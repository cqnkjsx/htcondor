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

int
copy_file(const char *old_filename, const char *new_filename)
{
#if defined(WIN32)
   		// overwrite destination path
   BOOL retval;
   retval = CopyFile(old_filename, new_filename, TRUE);

   if (retval == 0) {
   		// failure
	  dprintf(D_ALWAYS, "CopyFile() failed with error=%li\n",
			  GetLastError());
	  return -1; 
   }
   else { return 0; } // success

#else
	int rc;
	int num_bytes;
	int in_fd = -1;
	int out_fd = -1;
	int new_file_created = 0;
	char buff[1024];
	struct stat fs;
	mode_t old_umask;

	old_umask = umask(0);

	rc = stat( old_filename, &fs );
	if ( rc < 0 ) {
		dprintf( D_ALWAYS, "stat(%s) failed with errno %d\n",
				 old_filename, errno );
		goto copy_file_err;
	}
	fs.st_mode &= S_IRWXU | S_IRWXG | S_IRWXO;

	in_fd = open( old_filename, O_RDONLY );
	if ( in_fd < 0 ) {
		dprintf( D_ALWAYS, "open(%s, O_RDONLY) failed with errno %d\n",
				 old_filename, errno );
		goto copy_file_err;
	}

	out_fd = open( new_filename, O_WRONLY | O_CREAT | O_TRUNC, fs.st_mode );
	if ( out_fd < 0 ) {
		dprintf( D_ALWAYS, "open(%s, O_WRONLY|O_CREAT|O_TRUNC, %d) failed with errno %d\n",
				 new_filename, fs.st_mode, errno );
		goto copy_file_err;
	}

	new_file_created = 1;

	errno = 0;
	rc = read( in_fd, buff, sizeof(buff) );
	while ( rc > 0 ) {
		num_bytes = rc;
		rc = write( out_fd, buff, num_bytes );
		if ( rc < num_bytes ) {
			dprintf( D_ALWAYS, "write(%d) to file %s return %d, errno %d\n",
					 num_bytes, new_filename, rc, errno );
			goto copy_file_err;
		}
		rc = read( in_fd, buff, sizeof(buff) );
	}

	if ( rc < 0 ) {
		dprintf( D_ALWAYS, "read() from file %s failed with errno %d\n",
				 old_filename, errno );
		goto copy_file_err;
	}

	close (in_fd);
	close (out_fd);

	umask( old_umask );

	return 0;

 copy_file_err:
	if ( in_fd != -1 ) {
		close( in_fd );
	}
	if ( out_fd != -1 ) {
		close( out_fd );
	}
	if ( new_file_created ) {
		unlink( new_filename );
	}
	umask( old_umask );
	return -1;
#endif
}

int
hardlink_or_copy_file(const char *old_filename, const char *new_filename)
{
#if defined(WIN32)
	/* There are no hardlinks under Windows, so just copy the file. */
	return copy_file(old_filename,new_filename);
#else
	if(link(old_filename,new_filename) == -1) {
		// Hardlink may fail for a number of reasons, some of which
		// can be solved by doing a plain old copy instead.  No harm
		// in trying.
		return copy_file(old_filename,new_filename);
	}
	return 0;
#endif
}
