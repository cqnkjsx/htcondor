##**************************************************************
##
## Copyright (C) 1990-2007, Condor Team, Computer Sciences Department,
## University of Wisconsin-Madison, WI.
## 
## Licensed under the Apache License, Version 2.0 (the "License"); you
## may not use this file except in compliance with the License.  You may
## obtain a copy of the License at
## 
##    http://www.apache.org/licenses/LICENSE-2.0
## 
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
##
##**************************************************************

#!/usr/bin/env perl
use IO::Socket;
use IO::Handle;
use Socket;

my $SockAddr = $ARGV[0];
my $LogFile = $ARGV[1];

open(OLDOUT, ">&STDOUT");
open(OLDERR, ">&STDERR");
open(STDOUT, ">$LogFile") or die "Could not open $LogFile: $!";
open(STDERR, ">&STDOUT");
select(STDERR); $| = 1;
select(STDOUT); $| = 1;

unlink($SockAddr);

my $server = IO::Socket::UNIX->new(Local => $SockAddr,
								Type  => SOCK_DGRAM)
or die "Can't bind socket: $!\n";

$server->setsockopt(SOL_SOCKET, SO_RCVBUF, 65440);


while ( 1 )
{
	my $newmsg;
	my $MAXLEN = 1024;
	#$server->recv($newmsg,$MAXLEN) || die "Recv: $!";
	$server->recv($newmsg,$MAXLEN);
	print "$newmsg\n";
	if($newmsg eq "quit")
	{
		exit(0);
	}
}

my $stat = 0;
my $returnval = shift;
print "Server exiting\n";
