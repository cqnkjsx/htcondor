/***************************Copyright-DO-NOT-REMOVE-THIS-LINE**
 * CONDOR Copyright Notice
 *
 * See LICENSE.TXT for additional notices and disclaimers.
 *
 * Copyright (c)1990-1998 CONDOR Team, Computer Sciences Department, 
 * University of Wisconsin-Madison, Madison, WI.  All Rights Reserved.  
 * No use of the CONDOR Software Program Source Code is authorized 
 * without the express consent of the CONDOR Team.  For more information 
 * contact: CONDOR Team, Attention: Professor Miron Livny, 
 * 7367 Computer Sciences, 1210 W. Dayton St., Madison, WI 53706-1685, 
 * (608) 262-0856 or miron@cs.wisc.edu.
 *
 * U.S. Government Rights Restrictions: Use, duplication, or disclosure 
 * by the U.S. Government is subject to restrictions as set forth in 
 * subparagraph (c)(1)(ii) of The Rights in Technical Data and Computer 
 * Software clause at DFARS 252.227-7013 or subparagraphs (c)(1) and 
 * (2) of Commercial Computer Software-Restricted Rights at 48 CFR 
 * 52.227-19, as applicable, CONDOR Team, Attention: Professor Miron 
 * Livny, 7367 Computer Sciences, 1210 W. Dayton St., Madison, 
 * WI 53706-1685, (608) 262-0856 or miron@cs.wisc.edu.
****************************Copyright-DO-NOT-REMOVE-THIS-LINE**/

#if !defined(_CONDOR_STARTER_H)
#define _CONDOR_STARTER_H

#include "../condor_daemon_core.V6/condor_daemon_core.h"
#include "list.h"
#include "user_proc.h"
#include "job_info_communicator.h"


/** The starter class.  Basically, this class does some initialization
	stuff and manages a set of UserProc instances, each of which 
	represent a running job.

	@see UserProc
 */
class CStarter : public Service
{
public:
		/// Constructor
	CStarter();
		/// Destructor
	virtual ~CStarter();

		/** This is called at the end of main_init().  It calls
			Config(), registers a bunch of signals, registers a
			reaper, makes the starter's working dir and moves there,
			sets resource limits, then calls StartJob()
		*/
	virtual bool Init( JobInfoCommunicator* my_jic, 
					   const char* orig_cwd, bool is_gridshell );

		/** Params for "EXECUTE" and other useful stuff 
		 */
	virtual void Config();

	virtual int SpawnJob( void );

		/** Walk through list of jobs, call ShutDownGraceful on each.
			@return 1 if no jobs running, 0 otherwise 
		*/
	virtual int ShutdownGraceful(int);

		/** Walk through list of jobs, call ShutDownFast on each.
			@return 1 if no jobs running, 0 otherwise 
		*/
	virtual int ShutdownFast(int);

		/** Create the execute/dir_<pid> directory and chdir() into
			it.  This can only be called once user_priv is initialized
			by the JobInfoCommunicator.
		*/
	virtual bool createTempExecuteDir( void );

		/** Called by the JobInfoCommunicator whenever the job
			execution environment is ready so we can actually spawn
			the job.
		*/
	virtual int jobEnvironmentReady( void );

		/** Does final cleanup once all the jobs (and post script, if
			any) have completed.  This deals with everything on the
			CleanedUpJobList, notifies the JIC, etc.
		*/
	virtual void allJobsDone( void );

		/** Call Suspend() on all elements in JobList */
	virtual int Suspend(int);

		/** Call Continue() on all elements in JobList */
	virtual int Continue(int);

		/** To do */
	virtual int PeriodicCkpt(int);

		/** Handles the exiting of user jobs.  If we're shutting down
			and there are no jobs left alive, we exit ourselves.
			@param pid The pid that died.
			@param exit_status The exit status of the dead pid
		*/
	virtual int Reaper(int pid, int exit_status);

		/** Return the Working dir */
	const char *GetWorkingDir() const { return WorkingDir; }

		/** Publish all attributes we care about for our job
			controller into the given ClassAd.  Walk through all our
			UserProcs and have them publish.
            @param ad pointer to the classad to publish into
			@return true if we published any info, false if not.
		*/
	bool publishUpdateAd( ClassAd* ad );

	bool publishPreScriptUpdateAd( ClassAd* ad );
	bool publishPostScriptUpdateAd( ClassAd* ad );

		/** Put all the environment variables we'd want a Proc to have
			into the given Env object.  This will figure out what Proc
			objects we've got and will call their respective
			PublishToEnv() methods
			@param proc_env The environment to publish to
		*/
	void PublishToEnv( Env* proc_env );
	
		/** Pointer to our JobInfoCommuniator object, which abstracts
			away any details about our communications with whatever
			entity is controlling our job.  This way, the starter can
			easily run without talking to a shadow, by instantiating a
			different kind of jic.  We want this public, since lots of
			parts of the starter need to get to this thing, and it's
			just easier this way. :)
		*/
	JobInfoCommunicator* jic;

		/** Returns the VM number we're running on
		*/
	int getMyVMNumber( void );

	bool isGridshell( void ) {return is_gridshell;};
	const char* origCwd( void ) {return (const char*) orig_cwd;};

protected:
	List<UserProc> JobList;
	List<UserProc> CleanedUpJobList;

private:

		// // // // // // // //
		// Private Data Members
		// // // // // // // //

	int jobUniverse;

	char *Execute;
	char WorkingDir[_POSIX_PATH_MAX]; // The iwd given to the job
	char ExecuteDir[_POSIX_PATH_MAX]; // The scratch dir created for the job
	char *orig_cwd;
	bool is_gridshell;
	int ShuttingDown;

	UserProc* pre_script;
	UserProc* post_script;
};

#endif


