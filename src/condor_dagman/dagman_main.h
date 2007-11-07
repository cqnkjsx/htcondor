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

#ifndef DAGMAN_MAIN_H
#define DAGMAN_MAIN_H

#include "dag.h"
#include "string_list.h"

	// Don't change these values!  Doing so would break some DAGs.
enum exit_value {
	EXIT_OKAY = 0,
	EXIT_ERROR = 1,
	EXIT_ABORT = 2, // condor_rm'ed or hit special abort DAG exit code
	EXIT_RESTART = 3,	// exit but indicate that we should be restarted
};

int main_shutdown_rescue( int exitVal );

class Dagman {
  public:
	Dagman();
	~Dagman();
    inline void CleanUp () { 
		if ( dag != NULL ) {
			delete dag; 
			dag = NULL;
		}
	}
    Dag * dag;
    int maxIdle;  // Maximum number of idle DAG nodes
    int maxJobs;  // Maximum number of Jobs to run at once
    int maxPreScripts;  // max. number of PRE scripts to run at once
    int maxPostScripts;  // max. number of POST scripts to run at once
    char *rescue_file;
	bool paused;

	char* condorSubmitExe;
	char* condorRmExe;
	char* storkSubmitExe;
	char* storkRmExe;

	// number of seconds to wait before consecutive calls to
	// condor_submit (or dap_submit, etc.)
    int submit_delay;
		// number of times in a row to attempt to execute
		// condor_submit (or dap_submit) before giving up
    int max_submit_attempts;
		// maximum number of jobs to submit in a single periodic timer
		// interval
    int max_submits_per_interval;

		// "Primary" DAG file -- if we have multiple DAG files this is
		// the first one.  The lock file name, rescue DAG name, etc., 
		// are based on this name.
	char *primaryDagFile;

		// The list of all DAG files to be run by this invocation of
		// condor_dagman.
	StringList dagFiles;

		// whether to peform expensive cycle-detection at startup
		// (note: we perform run-time cycle-detection regardless)
	bool startup_cycle_detect;

		// Allow the job to execute even if we have an error determining
		// the log files (e.g., the log file is missing from one of the
		// node submit files).
	bool allowLogError;

		// Whether to treat the dirname portion of any DAG file paths
		// as a directory that the DAG should effectively be run from.
	bool useDagDir;

		// What "bad" events to treat as non-fatal (as opposed to fatal)
		// errors; see check_events.h for values.
	int allow_events;

		// If this is true, nodes for which the job submit fails are retried
		// before any other ready nodes; otherwise a submit failure puts
		// a node at the back of the ready queue.  (Default is true.)
	bool retrySubmitFirst;

		// If this is true, nodes for which the node fails (and the node
		// has retries) are retried before any other ready nodes; 
		// otherwise a node failure puts a node at the back of the ready 
		// queue.  (Default is false.)
	bool retryNodeFirst;

		// Whether to munge the node names for multi-DAG runs to make
		// sure they're unique.  The default is true, but the user can
		// turn this off if their node names are globally unique.
	bool mungeNodeNames;

		// whether or not to remove any existing userlogs before
		// starting to ensure we don't see events from previous
		// instances of the same DAG and think they're ours
	bool deleteOldLogs;

		// whether or not to prohibit multiple job proc submits (e.g.,
		// node jobs that create more than one job proc)
	bool prohibitMultiJobs;

		// Whether to abort duplicates DAGMans (if multiple DAGMans are
		// run on the same DAG at the same time on the same machine,
		// all but the first will be aborted).
	bool abortDuplicates;

		// Whether to submit ready nodes in depth-first order (as opposed
		// to breadth-first).
	bool submitDepthFirst;

		// Whether to abort on a "scary" submit event (Condor ID doesn't
		// match expected value).
	bool abortOnScarySubmit;

		// The interval (in seconds) between reports on what nodes
		// are pending.
	int pendingReportInterval;

		// the Condor job id of the DAGMan job
	CondorID DAGManJobId;

    bool Config();

		// The DAGMan configuration file (NULL if none is specified).
	char *_dagmanConfigFile;
};

#endif	// ifndef DAGMAN_MAIN_H
