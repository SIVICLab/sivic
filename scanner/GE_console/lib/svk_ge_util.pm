#!/usr/bin/perl 

#
#   Copyright © 2009-2014 The Regents of the University of California.
#   All Rights Reserved.
# 
#   Redistribution and use in source and binary forms, with or without 
#   modification, are permitted provided that the following conditions are met:
#   •   Redistributions of source code must retain the above copyright notice, 
#       this list of conditions and the following disclaimer.
#   •   Redistributions in binary form must reproduce the above copyright notice, 
#       this list of conditions and the following disclaimer in the documentation 
#       and/or other materials provided with the distribution.
#   •   None of the names of any campus of the University of California, the name 
#       "The Regents of the University of California," or the names of any of its 
#       contributors may be used to endorse or promote products derived from this 
#       software without specific prior written permission.
#   
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
#   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
#   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
#   IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
#   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
#   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
#   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
#   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
#   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
#   OF SUCH DAMAGE.
# 

#   $URL: svn+ssh://jccrane@svn.code.sf.net/p/sivic/code/trunk/libs/src/svkImageData.cc $
#   $Rev: 1741 $
#   $Author: jccrane $
#   $Date: 2014-02-19 11:28:48 -0800 (Wed, 19 Feb 2014) $
# 
#   Authors:
#       Jason C. Crane, Ph.D.
#       Beck Olson
# 


package svk_ge_util;

use Exporter;
use strict;
use threads;
use svk_ge_data_browser_conf;
use IO::Socket;


our @ISA = qw(Exporter);
our @EXPORT = qw(&svk_ge_browser_get_command 
                 &svk_ge_run_program 
                 &svk_ge_run_program_remote_local
                 &svk_ge_finalize 
                 &svk_ge_write_local_status
                 &svk_ge_get_series_number
                 &svk_ge_stage_job
                );


$ENV{'DCMDICTPATH'} = "/export/home/sdc/svk/lib/dicom.dic";
#$ENV{'DCMDICTPATH'} = "$ENV{SDCHOME}/svk/lib/dicom.dic";

my $useSocket = 1;	#only set to false if using quick job staging without GUI 

my $local_dir  = "$svk_ge_data_browser_conf::scanner_work_dir";
my $remote_dir = "$svk_ge_data_browser_conf::remote_data_dir";

my $statusfile  = "$local_dir/STATUS_FILE";
my $local_host  = "$ENV{HOST}";
my $VRE_host    = "$svk_ge_data_browser_conf::VRE_host{$ENV{HOST}}";
my $remote_host = "$svk_ge_data_browser_conf::remote_host{$ENV{HOST}}";

my $browser_thread;
my $browser_socket;
my $remoteprocess_thread;
my $command_string;
my $files_from_browser;

my $config_file;
my $svk_ge_data_browser = "svk_ge_data_browser.sh";

sub svk_ge_browser_get_command($);
sub svk_ge_run_program($$);
sub svk_ge_run_program_remote_local($$);
sub svk_ge_write_local_status($$);
sub svk_ge_finalize();
sub svk_ge_get_series_number($);

sub svk_ge_stage_job($$);

use vars qw($server);



##################################################
# PUBLIC
# open socket and run the browser as a separate thread

sub svk_ge_browser_get_command($) 
{

    unlink("$local_dir/PEER_PROCESS");

    $server = new IO::Socket::INET ( LocalPort => '9900',
                                    Type   => SOCK_STREAM,
                                    Proto  => 'tcp',
                                    Listen => 1,
                                    Reuse  => 1);
    unless($server) {die "Could not create server";}


    $config_file = shift;
    unless(-e $config_file) {die "Config file does not exist $config_file";}

    # initiate the thread for the browser and set up
    # the socket for communication
    $browser_thread = threads->new(\&run_browser);
    $browser_socket = $server->accept();

    my %results;

    while(<$browser_socket>) {
        if(m/exit/) {
            $browser_thread->join;
            exit;
        } else {
            chomp;
            my @results = split /\|/;
            print "svk_ge_browser_get_command  results: @results\n";
            $results{prog} = $results[0];
            $results{flags} = $results[1];
            $results{files} = $results[2];
            $files_from_browser = $results{files};
            last;
        }
    }
    if ($results{files} =~ m/\/usr\/g\/mrraw/) {
        $results{files}=~ s/\/usr\/g\/mrraw\///g; 
    }
    return %results
}



#################################################
# PRIVATE
# this is a private function for use by svk_ge_browser_get_command.
# it is run in a new thread.

sub run_browser 
{
    unless($config_file and (-e $config_file)) {
        die "Config file not found or undefined";
    }
    print "run_browser: $svk_ge_data_browser $config_file $local_dir\n";
    system "$svk_ge_data_browser $config_file $local_dir";
}



##################################################
#PUBLIC
# run the program after staging the input files

sub svk_ge_run_program($$) 
{
    my($location, $command) = @_;
    unless($command)  {
        die "command undefined";
    }
    unless($location) {
        die "location undefined";
    }

    unlink($statusfile);

    print "svk_ge_run_program called with $location and $command\n"; 
  
    if ($location eq 'local' || $location eq 'VRE') {
        echocommand("rm -rf $local_dir/*");
    } elsif ($location eq 'remote') {
        echocommand("ssh $remote_host rm -rf $remote_dir/");
    } 

    copy_files($location);

    if($location eq 'local') {
        $command_string = $command;
    } elsif ( $location eq 'remote' ) {
        $command_string = "ssh -X $remote_host \"cd $remote_dir/;  $command\"";
    } elsif ($location eq 'VRE') {
        #   requires setting HOSTNAME on VRE to that of parent
        my $calling_host = "$ENV{HOST}";
        $command_string = "rsh $VRE_host \"export HOST=$calling_host; cd $local_dir/;  $command\"";
    }

    open(PROCESS_FILE, ">$local_dir/PEER_PROCESS") or
      die("could not open $local_dir/PEER_PROCESS");
    print PROCESS_FILE "$command_string\n";
    close PROCESS_FILE;

    $remoteprocess_thread = threads->new(\&echocommand, $command_string);
    my $counter = 0;
    my $status = 0;

    sleep 1 until(-e $statusfile or $counter++ == 3600);

    if (-e $statusfile) {
        open(STATUS_FILE, "$statusfile") || die "could not open $statusfile\n"; 
        $statusfile = <STATUS_FILE>;
        close(STATUS_FILE);
        chomp($statusfile);
        print "statusfile:  $statusfile\n";
        $status = $statusfile;
    } else {
        $status = -1;
    }

    #
    #   return status:  -1: didn't return, 0:success, >1:failed
    #
    return $status;
}



sub svk_ge_run_program_remote_local($$)
{
    my($remote_command, $local_command) = @_;
    unless($remote_command)  {
        die "remote command undefined";
    }
    unless($local_command) {
        die "local command undefined";
    }

    unlink($statusfile);

    ####################################
    # run remote command
    ####################################
    print "svk_ge_run_program called with $remote_command and $local_command\n"; 
  
    echocommand("ssh $remote_host rm -rf $remote_dir/");

    copy_files('remote');

    $command_string = "ssh -X $remote_host \"cd $remote_dir/;  $remote_command\"";

    open(PROCESS_FILE, ">$local_dir/PEER_PROCESS") or
      die("could not open $local_dir/PEER_PROCESS");
    print PROCESS_FILE "$command_string\n";
    close PROCESS_FILE;

    $remoteprocess_thread = threads->new(\&echocommand, $command_string);
    my $counter = 0;
    my $status = 0;

    sleep 1 until(-e $statusfile or $counter++ == 3600);


    ####################################
    # run local command
    ####################################
    $command_string = "cd $local_dir/; $local_command";

    open(PROCESS_FILE, ">$local_dir/PEER_PROCESS") or
      die("could not open $local_dir/PEER_PROCESS");
    print PROCESS_FILE "$command_string\n";
    close PROCESS_FILE;

    unlink($statusfile);

    $remoteprocess_thread->join();

    $remoteprocess_thread = threads->new(\&echocommand, $command_string);
    $counter = 0;
    $status = 0;

    sleep 1 until(-e $statusfile or $counter++ == 3600);

    if (-e $statusfile) {
        open(STATUS_FILE, "$statusfile") || die "could not open $statusfile\n"; 
        $statusfile = <STATUS_FILE>;
        close(STATUS_FILE);
        chomp($statusfile);
        print "statusfile:  $statusfile\n";
        $status = $statusfile;
    } else {
        $status = -1;
    }

    #
    #   return status:  -1: didn't return, 0:success, >1:failed
    #
    return $status;
}



#	location = local, remote, VRE
#	data_type = exam series
#	returns the local work directory; 
sub svk_ge_stage_job($$) 
{
    my ($location, $data_type) = @_;
    if ($data_type ne 'exam' && $data_type ne 'series') {
        print "Must specify data_type = exam or series \n";
        exit(1);
    }
    $useSocket = 0;
    get_sdc_selection_file($data_type);

    # remove any residual data sets from local work dir:
    clean_work_dir(); 

    # stage the selected data in local work dir 
    copy_files($location);

    return $local_dir;
}


##################################################
# PRIVATE
# depending on the location (local, remote, or VRE), 
sub get_sdc_selection_file($) 
{
    my ($data_type) = @_; 	
    open(SSF, "$ENV{'SDC_SELECTION_FILE'}") || die ("couldnt open SDC_SELECTION_FILE");
    my @selection_file = <SSF>;
    close(SSF);
   
    if ($data_type eq 'exam') { 	 
        if ($selection_file[0] =~ m/^(.*e\d+).*/) {	
       	    $files_from_browser = $1; 
	    }
    } elsif ($data_type eq 'series') { 	 
        if ($selection_file[0] =~ m/^(.*s\d+).*/) {	
       	    $files_from_browser = $1; 
	    }
    }
}	
    	

##################################################
# PRIVATE
# depending on the location (local, remote, or VRE), 
# copy files to the right place with the right 
# commands.
sub copy_files 
{
    my $location = shift;
    my($mkdirprog, $cpprog, $data_dir, $cphost, $mkdirhost);
    my $script_name = $0;

    if($location eq 'local' || $location eq 'VRE') {
        $data_dir = "$local_dir/";
        $mkdirprog= "mkdir -p";
        $cpprog = "cp -L";
        $cphost = "";
        $mkdirhost = ""; 
    } elsif($location eq 'remote') {
        $data_dir = "$remote_dir/";
        $mkdirprog = "ssh ${remote_host} mkdir -p";
        $cpprog = "scp ";
        $cphost = "${remote_host}:";
        $mkdirhost = ${remote_host};
    } else {
        die "location not recognized - remote, local or VRE";
    }

    echocommand("mkdir -p $local_dir/");
    echocommand("chmod -R 777 $local_dir/");

    # if exam files present, then create a remote local_host/exam_dir, 
    # otherwise only create a local_host dir.

    my $exam_dir = "";

    foreach my $obj ( split(/\s+/, $files_from_browser) ) {
        if ($obj =~ m/\/(e\d{1,})/) {
            $exam_dir = $1;
            print      ("$mkdirprog $data_dir/$exam_dir\n");
            echocommand("$mkdirprog $data_dir/$exam_dir");
        } else {
            print      ("$mkdirprog $data_dir\n");
            echocommand("$mkdirprog $data_dir");
        }
    }

    foreach my $obj ( split(/\s+/, $files_from_browser) ) {

        #
        # cp exam data (but not P file data) to remote exam_dir:
        #		
        if ($obj =~ m/\/(e\d{1,})/) {
            $exam_dir = $1;
            print      ("$cpprog -r $obj ${cphost}$data_dir/$exam_dir\n");
            echocommand("$cpprog -r $obj ${cphost}$data_dir/$exam_dir");
            next;
        }

        #
        # cp raw files to remote exam_dir:
        #		
        print      ("$cpprog -r $obj ${cphost}$data_dir/\n");
        echocommand("$cpprog -r $obj ${cphost}$data_dir/");

        #
        # cp any associated .dat files to remote exam_dir:
        #		
        if ( $obj =~ m/(\/usr\/g\/mrraw\/.*P.*)\.7/ ) {
            my $raw_root = $1; 
            print      ("$cpprog -r ${raw_root}*.dat ${cphost}$data_dir/\n");
            echocommand("$cpprog -r ${raw_root}*.dat ${cphost}$data_dir/");
        }
    }

    #  set dicom dictionary so dcmdump prints usable information
    $ENV{'DCMDICTPATH'} = "$ENV{SDCHOME}/svk/lib/dicom.dic";
    if ( !defined $ENV{DCMDICTPATH} ) {
    	$ENV{'DCMDICTPATH'} = "/export/home/sdc/svk/lib/dicom.dic";
    }	

    if($location eq 'local' || $location eq 'VRE') {
	my $rename_cmd = "svk_ge_rename_scanner_dcm_files -d $data_dir"; 
	#my $rename_cmd = "setenv DCMDICTPATH $ENV{DCMDICTPATH}; svk_ge_rename_scanner_dcm_files -d $data_dir"; 
        print      ("$rename_cmd\n"); 
        echocommand("$rename_cmd"); 
    } elsif($location eq 'remote') {
        print      ("ssh $remote_host svk_ge_rename_scanner_dcm_files -d $data_dir\n"); 
        echocommand("ssh $remote_host svk_ge_rename_scanner_dcm_files -d $data_dir"); 
    }

}

##################################################
#PRIVATE
# print the text of the command to the browser socket
# and print the output of the text command to the
# socket too. 
sub echocommand 
{
    my $command = shift;
    if ( $useSocket ) {
        print $browser_socket "$command\n";
    } else {
        print "$command\n";
    }
    open(CMD, "$command|") or die "could not open CMD $command";
    while(<CMD>) {
        if ( $useSocket ) {
            print $browser_socket $_;
        } else {
            print "$_";
        }	
    }
    close(CMD); 	
}


##################################################
# PUBLIC
# call this to write a local status file in specified 
# directory on the scanner
sub svk_ge_write_local_status($$) 
{

    my ($path, $status) = @_;

    my $status_file = "STATUS_FILE";
    print "echo $status > $path/$status_file\n";
    system("echo $status > $path/$status_file");

    return;
}


##################################################
#PRIVATE
# cleans the work directory on the scanner. 
sub clean_work_dir() 
{
    print "rm -rf ${local_dir}/\n";
    system("rm -rf ${local_dir}/");
    return;
}

##################################################
# PUBLIC
# call this to join the browser and 
# remote process threads to exit early
# this replaces the calling program (svk_ge_start...)
# with the browser, so when the user clicks done
# in the browser, everything is guaranteed to be over.
# Also, we hope that when finalize is called, the 
# remote process will already have finished.  Is there a 
# way to check if it exists before calling this?
sub svk_ge_finalize() 
{
    $remoteprocess_thread->join();
    print $browser_socket "RRC_DATA_BROWSER_QUIT\n";
    $browser_thread->join();
    clean_work_dir();
    return;
}


sub svk_ge_get_series_number($) 
{

    my ($series_number, $dcmdump);
    my ($series_dir) = @_;

    $dcmdump = "dcmdump";

    $series_number = `$dcmdump ${series_dir}/*.1 | grep SeriesNumber`;
    $series_number =~ s/.*\[(\d+)\].*/$1/;

    chomp $series_number;

    return $series_number;
}


