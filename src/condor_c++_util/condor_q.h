/***************************************************************
 *
 * Copyright (C) 1990-2007, Condor Team, Computer Sciences Department,
 * University of Wisconsin-Madison, WI.
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

#ifndef __CONDOR_Q_H__
#define __CONDOR_Q_H__

#include "condor_common.h"
#include "generic_query.h"
#include "CondorError.h"

#define MAXOWNERLEN 20
#define MAXSCHEDDLEN 255

// This is for the getFilterAndProcess function
typedef bool    (*process_function)(ClassAd *);

/* a list of all types of direct DB query defined here */
enum CondorQQueryType
{
	AVG_TIME_IN_QUEUE
};

enum
{
	Q_NO_SCHEDD_IP_ADDR = 20,
	Q_SCHEDD_COMMUNICATION_ERROR
};

enum CondorQIntCategories
{
	CQ_CLUSTER_ID,
	CQ_PROC_ID,
	CQ_STATUS,
	CQ_UNIVERSE,

	CQ_INT_THRESHOLD
};

enum CondorQStrCategories
{
	CQ_OWNER,

	CQ_STR_THRESHOLD
};

enum CondorQFltCategories
{
	CQ_FLT_THRESHOLD
};


class CondorQ
{
  public:
	// ctor/dtor
	CondorQ ();
	// CondorQ (const CondorQ &);
	~CondorQ ();

	// initialize defaults, etc.
	bool init();  

	// add constraints
	int add (CondorQIntCategories, int);
	int add (CondorQStrCategories, char *);
	int add (CondorQFltCategories, float);
	int addAND (char *);  // custom
	int addOR (char *);  // custom
	int addDBConstraint (CondorQIntCategories, int);

	int addSchedd (char *);  // what schedd are we querying?
	int addScheddBirthdate (time_t value);  // what 
	// fetch the job ads from the schedd corresponding to the given classad
	// which pass the criterion specified by the constraints; default is
	// from the local schedd
	int fetchQueue (ClassAdList &, StringList &attrs, ClassAd * = 0, CondorError* errstack = 0);
	int fetchQueueFromHost (ClassAdList &, StringList &attrs, char * = 0, CondorError* errstack = 0);
	int fetchQueueFromHostAndProcess ( char *, StringList &attrs, process_function process_func, bool useFastPath, CondorError* errstack = 0);
	
		// fetch the job ads from database 	
	int fetchQueueFromDB (ClassAdList &, char * = 0, CondorError* errstack = 0);
	int fetchQueueFromDBAndProcess ( char *, process_function process_func, CondorError* errstack = 0);

		// return the results from a DB query directly to user
	void rawDBQuery(char *, CondorQQueryType);

  private:
	GenericQuery query;
	
	// default timeout when talking the schedd (via ConnectQ())
	int connect_timeout;
	
	int *clusterarray;
	int *procarray;
	int clusterprocarraysize;
	int numclusters;
	int numprocs;
	char owner[MAXOWNERLEN];
	char schedd[MAXSCHEDDLEN];
	time_t scheddBirthdate;
	
	// helper functions
	int getAndFilterAds( ClassAd &, StringList &attrs, ClassAdList &, bool useAll );
	int getFilterAndProcessAds( ClassAd &, StringList &attrs, process_function, bool useAll );
};

int JobSort(ClassAd *job1, ClassAd *job2, void *data);

const char encode_status( int status );

#endif
