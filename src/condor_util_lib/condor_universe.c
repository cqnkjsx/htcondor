/***************************Copyright-DO-NOT-REMOVE-THIS-LINE**
  *
  * Condor Software Copyright Notice
  * Copyright (C) 1990-2006, Condor Team, Computer Sciences Department,
  * University of Wisconsin-Madison, WI.
  *
  * This source code is covered by the Condor Public License, which can
  * be found in the accompanying LICENSE.TXT file, or online at
  * www.condorproject.org.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  * AND THE UNIVERSITY OF WISCONSIN-MADISON "AS IS" AND ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  * WARRANTIES OF MERCHANTABILITY, OF SATISFACTORY QUALITY, AND FITNESS
  * FOR A PARTICULAR PURPOSE OR USE ARE DISCLAIMED. THE COPYRIGHT
  * HOLDERS AND CONTRIBUTORS AND THE UNIVERSITY OF WISCONSIN-MADISON
  * MAKE NO MAKE NO REPRESENTATION THAT THE SOFTWARE, MODIFICATIONS,
  * ENHANCEMENTS OR DERIVATIVE WORKS THEREOF, WILL NOT INFRINGE ANY
  * PATENT, COPYRIGHT, TRADEMARK, TRADE SECRET OR OTHER PROPRIETARY
  * RIGHT.
  *
  ****************************Copyright-DO-NOT-REMOVE-THIS-LINE**/

#include "condor_common.h"
#include "condor_debug.h"
#include "condor_universe.h"

/*
Warning: This table should stay in sync
with the symbols in condor_universe.h
*/


typedef struct {
	const char	*uc;
	const char	*ucfirst;
} UniverseName;

static const UniverseName names [] =
{
	{ "NULL","NULL" },
	{ "STANDARD", "Standard" },
	{ "PIPE", "Pipe" },
	{ "LINDA", "Linda" },
	{ "PVM", "PVM" },
	{ "VANILLA", "Vanilla" },
	{ "PVMD", "PVMD" },
	{ "SCHEDULER", "Scheduler" },
	{ "MPI", "MPI" },
	{ "GLOBUS", "Globus" },
	{ "JAVA", "Java" },
	{ "PARALLEL", "Parallel" },
	{ "LOCAL", "Local" }
};

const char*
CondorUniverseName( int u )
{
	if( u<=CONDOR_UNIVERSE_MIN || u>=CONDOR_UNIVERSE_MAX ) {
		return "UNKNOWN";
	} else {
		return names[u].uc;
	}
}

const char*
CondorUniverseNameUcFirst( int u )
{
	if( u<=CONDOR_UNIVERSE_MIN || u>=CONDOR_UNIVERSE_MAX ) {
		return "Unknown";
	} else {
		return names[u].ucfirst;
	}
}

int
CondorUniverseNumber( const char* univ )
{
	if( ! univ ) {
		return 0;
	}

	if( stricmp(univ,"standard") == MATCH ) {
		return CONDOR_UNIVERSE_STANDARD;
	}
	if( stricmp(univ,"pipe") == MATCH ) {
		return CONDOR_UNIVERSE_PIPE;
	}
	if( stricmp(univ,"linda") == MATCH ) {
		return CONDOR_UNIVERSE_LINDA;
	}
	if( stricmp(univ,"pvm") == MATCH ) {
		return CONDOR_UNIVERSE_PVM;
	}
	if( stricmp(univ,"vanilla") == MATCH ) {
		return CONDOR_UNIVERSE_VANILLA;
	}
	if( stricmp(univ,"pvmd") == MATCH ) {
		return CONDOR_UNIVERSE_PVMD;
	}
	if( stricmp(univ,"scheduler") == MATCH ) {
		return CONDOR_UNIVERSE_SCHEDULER;
	}
	if( stricmp(univ,"mpi") == MATCH ) {
		return CONDOR_UNIVERSE_MPI;
	}
		// grid replaces globus, but keeping globus for backwards compat
	if( stricmp(univ,"globus") == MATCH ||
		stricmp(univ,"grid") == MATCH ) {
		return CONDOR_UNIVERSE_GRID;
	}
	if( stricmp(univ,"java") == MATCH ) {
		return CONDOR_UNIVERSE_JAVA;
	}
	if( stricmp(univ,"parallel") == MATCH ) {
		return CONDOR_UNIVERSE_MPI;
	}
	if( stricmp(univ,"local") == MATCH ) {
		return CONDOR_UNIVERSE_LOCAL;
	}
	return 0;
}


BOOLEAN
universeCanReconnect( int universe )
{
	switch (universe) {
	case CONDOR_UNIVERSE_STANDARD:
	case CONDOR_UNIVERSE_PVM:
	case CONDOR_UNIVERSE_SCHEDULER:
	case CONDOR_UNIVERSE_LOCAL:
	case CONDOR_UNIVERSE_MPI:
	case CONDOR_UNIVERSE_GRID:
		return FALSE;
	case CONDOR_UNIVERSE_VANILLA:
	case CONDOR_UNIVERSE_JAVA:
	case CONDOR_UNIVERSE_PARALLEL:
		return TRUE;
	default:
		EXCEPT( "Unknown universe (%d) in universeCanReconnect()", universe );
	}
	return FALSE;
}
