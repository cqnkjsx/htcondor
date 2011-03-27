/***************************************************************
 *
 * Copyright (C) 2009-2011 Red Hat, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License.  You may
 * obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************/

// condor includes
#include "condor_common.h"
#include "condor_debug.h"
#include "condor_attributes.h"
#include "condor_parser.h"
#include "proc.h"

// c++ includes
#include <sstream>

// local includes
#include "Globals.h"
#include "AviaryUtils.h"

// Any key that begins with the '0' char is either the
// header or a cluster, i.e. not a job
#define IS_JOB(key) ((key) && '0' != (key)[0])

// summary attributes
 const char *ATTRS[] = {ATTR_CLUSTER_ID,
                           ATTR_PROC_ID,
                           ATTR_GLOBAL_JOB_ID,
                           ATTR_Q_DATE,
                           ATTR_ENTERED_CURRENT_STATUS,
                           ATTR_JOB_STATUS,
                           ATTR_JOB_CMD,
                           ATTR_JOB_ARGUMENTS1,
                           ATTR_JOB_ARGUMENTS2,
                           ATTR_RELEASE_REASON,
                           ATTR_HOLD_REASON,
                           NULL
                          };

// TODO: C++ utils which may very well exist elsewhere :-)
template <class T>
bool from_string ( T& t,
                   const std::string& s,
                   std::ios_base& ( *f ) ( std::ios_base& ) )
{
    std::istringstream iss ( s );
    return ! ( iss >> f >> t ).fail();
}

template <class T>
std::string to_string ( T t, std::ios_base & ( *f ) ( std::ios_base& ) )
{
    std::ostringstream oss;
    oss << f << t;
    return oss.str();
}

using namespace aviary::query;
using namespace aviary::util;

//////////////
// LiveJobImpl
//////////////
LiveJobImpl::LiveJobImpl (const char* cluster_proc, ClusterJobImpl* parent)
{
    m_job = NULL;
    m_parent = NULL;
    m_summary_ad = NULL;
    m_full_ad = new ClassAd();
    if ( parent)
    {
        m_parent = parent;
        m_parent->incrementRef();
        m_full_ad->ChainToAd ( parent->m_full_ad );
    }

    dprintf ( D_FULLDEBUG, "LiveJobImpl created for '%s'\n", cluster_proc);
}

LiveJobImpl::~LiveJobImpl()
{
    // TODO: do we have to unchain our parent?
    if (m_full_ad) {
        m_full_ad->Unchain();
        delete m_full_ad;
        m_full_ad = NULL;
    }

    if (m_summary_ad) {
        delete m_summary_ad;
        m_summary_ad = NULL;
    }

    if (m_parent) {
        m_parent->decrementRef();
    }

    dprintf ( D_FULLDEBUG, "LiveJobImpl destroyed: key '%s'\n", m_job->getKey());
}

// TODO: this code doesn't work as expected
// everything seems to get set to EXPR_TYPE
bool
LiveJobImpl::get ( const char *_name, const AviaryAttribute *&_attribute ) const
{
    // our job ad is chained so lookups will
    // encompass our parent ad as well as the child

    // parse the type
    ExprTree *expr = NULL;
    if ( ! ( expr = m_full_ad->Lookup ( _name ) ) )
    {
	    dprintf ( D_FULLDEBUG,
                  "warning: failed to lookup attribute %s in job '%s'\n", _name, m_job->getKey() );
        return false;
    }
    // decode the type
    classad::Value value;
    m_full_ad->EvaluateExpr(expr,value);
    switch ( value.GetType() )
    {
        case classad::Value::INTEGER_VALUE:
        {
            int i;
            if ( !m_full_ad->LookupInteger ( _name, i ) )
            {
                return false;
            }
            _attribute = new AviaryAttribute ( AviaryAttribute::INTEGER_TYPE, to_string<int> ( i,std::dec ).c_str() );
            return true;
        }
        case classad::Value::REAL_VALUE:
        {
            float f;
            if ( !m_full_ad->LookupFloat ( _name, f ) )
            {
                return false;
            }
            _attribute = new AviaryAttribute ( AviaryAttribute::FLOAT_TYPE, to_string<float> ( f,std::dec ).c_str() );
            return true;
        }
        case classad::Value::STRING_VALUE:
        {
            MyString str;
            if ( !m_full_ad->LookupString ( _name, str ) )
            {
                return false;
            }
            _attribute = new AviaryAttribute ( AviaryAttribute::STRING_TYPE, str.StrDup() );
            return true;
        }
        default:
        {
            ExprTree* tree = NULL;
            if ( ! ( tree = m_full_ad->Lookup ( _name ) ) )
            {
                return false;
            }
            const char* rhs;
            rhs = ExprTreeToString( expr );
            _attribute = new AviaryAttribute ( AviaryAttribute::EXPR_TYPE, rhs );
            return true;
        }
    }

    return false;
}

int LiveJobImpl::getStatus() const
{
    const AviaryAttribute* attr;

    if ( !this->get ( ATTR_JOB_STATUS, attr ) )
    {
	// TODO: assume we might get cluster jobs here also
	return JOB_STATUS_MIN;
    }

    return strtol ( attr->getValue(), ( char ** ) NULL, 10 );
}

void
LiveJobImpl::set ( const char *_name, const char *_value )
{

    if ( strcasecmp ( _name, ATTR_JOB_SUBMISSION ) == 0 )
    {
        std::string val = trimQuotes( _value );
        // TODO: grab the cluster from our key
        PROC_ID id = getProcByString(m_job->getKey());
	if (m_job) {
		m_job->setSubmission ( val.c_str(), id.cluster );
	}
    }

    // our status is changing...decrement for old one
    if ( strcasecmp ( _name, ATTR_JOB_STATUS ) == 0 )
    {
	if ( m_job )
        {
	    m_job->setStatus(this->getStatus());
            m_job->decrementSubmission ();
        }
    }

    if ( strcasecmp ( _name, ATTR_OWNER ) == 0 )
    {
	// need to leave an owner for this job
	// to be picked up soon
	// if we are in here, we don't have m_submission
	PROC_ID id = getProcByString(m_job->getKey());
	std::string val = trimQuotes( _value );
	g_ownerless[id.cluster] = strdup( val.c_str() );
    }

    // parse the type
    ExprTree *expr;
    if ( ParseClassAdRvalExpr ( _value, expr ) )
    {
        dprintf ( D_ALWAYS,
                  "error: parsing %s[%s] = %s, skipping\n",
                  m_job->getKey(), _name, _value );
        return;
    }
    // add this value to the classad
    classad::Value value;
    expr->Evaluate(value);
    switch ( value.GetType() )
    {
        case classad::Value::INTEGER_VALUE:
            int i;
            from_string<int> ( i, std::string ( _value ), std::dec );
            m_full_ad->Assign ( _name, i );
            break;
        case classad::Value::REAL_VALUE:
            float f;
            from_string<float> ( f, std::string ( _value ), std::dec );
            m_full_ad->Assign ( _name, f );
            break;
        case classad::Value::STRING_VALUE:
            m_full_ad->Assign ( _name, _value );
            break;
        default:
            m_full_ad->AssignExpr ( _name, _value );
            break;
    }
    delete expr; expr = NULL;

    // our status has changed...increment for new one
    if ( strcasecmp ( _name, ATTR_JOB_STATUS ) == 0 )
    {
        if ( m_job )
        {
	    m_job->setStatus(this->getStatus());
            m_job->incrementSubmission ();
        }
    }
}

void
LiveJobImpl::remove ( const char *_name )
{
	// TODO: seems we implode if we don't unchain first
	classad::ClassAd* cp = m_full_ad->GetChainedParentAd();
	m_full_ad->Delete ( _name );
	m_full_ad->ChainToAd(cp);
}

const ClassAd* LiveJobImpl::getSummary () const
{
	ClassAd* _summary_ad = NULL;
	if (!m_summary_ad) {
		_summary_ad = new ClassAd();
		_summary_ad->ResetExpr();
		int i = 0;
		while (NULL != ATTRS[i]) {
			const AviaryAttribute* attr = NULL;
			if (this->get(ATTRS[i],attr)) {
				switch (attr->getType()) {
					case AviaryAttribute::FLOAT_TYPE:
						_summary_ad->Assign(ATTRS[i], atof(attr->getValue()));
						break;
					case AviaryAttribute::INTEGER_TYPE:
						_summary_ad->Assign(ATTRS[i], atol(attr->getValue()));
						break;
					case AviaryAttribute::EXPR_TYPE:
					case AviaryAttribute::STRING_TYPE:
					default:
						_summary_ad->Assign(ATTRS[i], strdup(attr->getValue()));
				}
			}
		i++;
		}
	} else {
		_summary_ad = m_summary_ad;
	}

	return _summary_ad;
}

const ClassAd* LiveJobImpl::getFullAd () const
{
    return m_full_ad;
}

/////////////////
// ClusterJobImpl
/////////////////
ClusterJobImpl::ClusterJobImpl(const char* key): LiveJobImpl(key, NULL)
{
        m_ref_count = 0;
}

ClusterJobImpl::~ClusterJobImpl()
{
        dprintf ( D_FULLDEBUG, "ClusterJobImpl destroyed: key '%s'\n", m_job->getKey());
}

void
ClusterJobImpl::incrementRef()
{
        m_ref_count++;
}

void
ClusterJobImpl::decrementRef()
{
        --m_ref_count;
}

/////////////////
// HistoryJobImpl
/////////////////
HistoryJobImpl::HistoryJobImpl ( const HistoryEntry& _he):
	m_he(_he)
{
    m_job = NULL;
    g_ownerless[_he.cluster] = strdup(_he.owner.c_str());
    dprintf ( D_FULLDEBUG, "HistoryJobImpl created for '%d.%d'\n", _he.cluster, _he.proc );
}

HistoryJobImpl::~HistoryJobImpl ()
{
	dprintf ( D_FULLDEBUG, "HistoryJobImpl destroyed: key '%s'\n", m_job->getKey());
}

int HistoryJobImpl::getStatus() const
{
    return m_he.status;
}

int HistoryJobImpl::getCluster() const
{
	return m_he.cluster;
}

const char* HistoryJobImpl::getSubmissionId() const
{
	return m_he.submission.c_str();
}

void HistoryJobImpl::getSummary ( ClassAd& _ad ) const
{
	_ad.ResetExpr();
	// use HistoryEntry data only
	_ad.Assign(ATTR_GLOBAL_JOB_ID,m_he.id.c_str());
	_ad.Assign(ATTR_CLUSTER_ID,m_he.cluster);
	_ad.Assign(ATTR_PROC_ID,m_he.proc);
	_ad.Assign(ATTR_Q_DATE,m_he.q_date);
	_ad.Assign(ATTR_JOB_STATUS,m_he.status);
	_ad.Assign(ATTR_ENTERED_CURRENT_STATUS,m_he.entered_status);
	_ad.Assign(ATTR_JOB_SUBMISSION,m_he.submission.c_str());
	_ad.Assign(ATTR_OWNER,m_he.owner.c_str());
	_ad.Assign(ATTR_JOB_CMD,m_he.cmd.c_str());

	// beyond here these may be empty so don't
	// automatically add to summary
	if (!m_he.args1.empty()) {
		_ad.Assign(ATTR_JOB_ARGUMENTS1,m_he.args1.c_str());
	}
	if (!m_he.args2.empty()) {
		_ad.Assign(ATTR_JOB_ARGUMENTS2,m_he.args2.c_str());
	}
	if (!m_he.release_reason.empty()) {
		_ad.Assign(ATTR_RELEASE_REASON,m_he.release_reason.c_str());
	}
	if (!m_he.hold_reason.empty()) {
		_ad.Assign(ATTR_HOLD_REASON,m_he.hold_reason.c_str());
	}

}

// specialization: this GetFullAd has to retrieve its classad attributes
// from the history file based on index pointers
void
 HistoryJobImpl::getFullAd ( ClassAd& _ad) const
{
    // fseek to he.start
    // ClassAd method to deserialize from a file with "***"

    FILE * hFile;
    int end = 0;
    int error = 0;
    int empty = 0;
	std::ostringstream buf;
	// placeholder in case we want to expose error details to UI
	std::string text;

    // TODO: move the ClassAd/file deserialize back to HistoryFile???
    const char* fName = m_he.file.c_str();
    if ( ! ( hFile = safe_fopen_wrapper ( fName, "r" ) ) )
    {
		buf <<  "unable to open history file " << m_he.file;
		text = buf.str();
        dprintf ( D_ALWAYS, text.c_str(), "\n");
	_ad.Assign("JOB_AD_ERROR",text.c_str());
	return;
    }
    if ( fseek ( hFile , m_he.start , SEEK_SET ) )
    {
		buf << "bad seek in " << m_he.file << " at " << m_he.start;
		text = buf.str();
        dprintf ( D_ALWAYS, text.c_str(), "\n");
	_ad.Assign("JOB_AD_ERROR",text.c_str());
        return;
    }

    ClassAd myJobAd ( hFile, "***", end, error, empty );
    fclose ( hFile );

	// TODO: debug logging and error to i/f for now
	// we might not have our original history file anymore
    if ( error )
    {
		buf <<  "malformed ad for job '" << m_job->getKey() << "' in " << m_he.file;
		text = buf.str();
        dprintf ( D_FULLDEBUG, "%s\n", text.c_str());
	_ad.Assign("JOB_AD_ERROR",text.c_str());
		return;
    }
    if ( empty )
    {
		buf << "empty ad for job '" << m_job->getKey() << "' in " << m_he.file;
		text = buf.str();
        dprintf ( D_FULLDEBUG,"%s\n", text.c_str());
	_ad.Assign("JOB_AD_ERROR",text.c_str());
		return;
    }

	_ad = myJobAd;

}

//////
// Job
//////
Job::Job(const char* _key):  m_submission(NULL), m_live_job(NULL), m_history_job(NULL), m_key(_key) 
{
        dprintf (D_FULLDEBUG,"Job::Job of '%s'\n", m_key);
}

Job::~Job() {
	// TODO: Destroy will be safe way to
	// cleanup

	dprintf (D_FULLDEBUG,"Job::~Job of '%s'\n", m_key);

    this->decrementSubmission();

	delete m_live_job;
	delete m_history_job;

	delete m_key;
	// submissions are shared and can't be deleted here
}

void Job::setImpl (LiveJobImpl* lji)
{
	lji->setJob(this);
	// probably shouldn't happen
	if (m_live_job) {
		delete m_live_job;
	}
	m_live_job = lji;

	// status of a live job always has precedence
	// so decrement if the history job got in ahead of it
	if (m_history_job) {
		m_status = m_history_job->getStatus();
		m_submission->decrement(this);
	}
	m_status = m_live_job->getStatus();
}

void Job::setImpl (HistoryJobImpl* hji)
{
	hji->setJob(this);
	// probably shouldn't happen
	if (m_history_job) {
		delete m_history_job;
	}
	m_history_job = hji;

	// stay away from extra inc/decs if the live job is still doing its thing
	if (!m_submission) {
		m_status = m_history_job->getStatus();
		setSubmission(m_history_job->getSubmissionId(),m_history_job->getCluster());
	}

	// call Destroy to see if we can clean up the live job
	this->destroy();
}

void Job::setStatus(int status)
{
	m_status = status;
}

const char* Job::getKey() const
{
	return m_key;
}

void Job::set ( const char *_name, const char *_value ) {
	if (m_live_job) {
		m_live_job->set(_name,_value);
	}
	// ignore for history jobs
}

void Job::remove ( const char *_name ) {
	if (m_live_job) {
		m_live_job->remove(_name);
	}
	// ignore for history jobs
}

void Job::incrementSubmission() {
	if (m_submission) {
		m_submission->increment(this);
	}
}

void Job::decrementSubmission() {
	if (m_submission) {
		m_submission->decrement(this);
	}
}

void
Job::setSubmission ( const char* _subName, int cluster )
{
	const char* owner = NULL;

	// need to see if someone has left us an owner
	OwnerlessClusterType::const_iterator it = g_ownerless.find ( cluster );
	if ( g_ownerless.end() == it )
	{
		dprintf ( D_FULLDEBUG, "warning: unable to resolve owner for Job key '%s' and cluster '%d'\n", getKey(), cluster );
	}
	else {
		owner = ( *it ).second ;
	}

	SubmissionCollectionType::const_iterator element = g_submissions.find ( _subName );
	SubmissionObject *submission;
	if ( g_submissions.end() == element )
	{
		submission = new SubmissionObject ( _subName, owner );
		g_submissions[strdup ( _subName ) ] = submission;
	}
	else
	{
		submission = ( *element ).second;
	}
	m_submission = submission;

	m_submission->increment(this);

	if (owner) {
		// ensure that the submission has an owner
		m_submission->setOwner ( owner );
		g_ownerless.erase ( cluster );
	}

}

bool ClusterJobImpl::destroy()
{
        return m_ref_count == 0;
}

bool
LiveJobImpl::destroy()
{
        return ((this->getStatus() == COMPLETED) || (this->getStatus() == REMOVED));
}

bool
Job::destroy()
{

        bool live_destroy = (m_live_job && m_live_job->destroy());

        if (m_history_job && live_destroy) {
                // must be a proc
                delete m_live_job;
                m_live_job = NULL;
                return false;
        }

        // hint to caller
        return live_destroy;
}

// space v. speed tradeoff here - we want to keep the size of the job server down
// since we may have to support millions of jobs in memory
void Job::getFullAd ( ClassAd& _ad) const
{
	// history ads are always reconstructed
	// so we deep copy the live job ad to be consistent
	// on the interface

	if (m_live_job) {
		_ad = *(m_live_job->getFullAd());
	}
	else {
		m_history_job->getFullAd(_ad);
	}

}

// poke a hole for live job ad chaining
const ClassAd* Job::getClassAd() const
{
	const ClassAd* ad_ptr = NULL;
	if (m_live_job) {
		ad_ptr = m_live_job->getFullAd();
	}
	return ad_ptr;

}

void Job::getSummary ( ClassAd& _ad) const
{
	//same thing as full ad
	if (m_live_job) {
		_ad = *(m_live_job->getSummary());
	}
	else {
		m_history_job->getSummary(_ad);
	}
}

int Job::getStatus() const
{
	return m_status;
}

JobImpl* Job::getImpl() {
        // only will provide live job impl
        // if it's NULL the caller needs to deal
        return m_live_job;
}
