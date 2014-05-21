#!/usr/bin/perl -w

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

#   $URL$
#   $Rev$
#   $Author$
#   $Date$
# 
#   Authors:
#       Jason C. Crane, Ph.D.
#       Beck Olson
# 


package svk_ge_data_browser_conf;
use strict;
use lib "$ENV{HOME}/svk/console/lib";
use svk_ge_util;

sub svk_ge_get_tail($); 

#
#   Global lookup table between local values of
#   $HOST and registered hostnames on svk_ge network.
#
%svk_ge_data_browser_conf::network_hostnames = (
    'qb3-7t'                    =>  'qb3-mr7t',
    'icn1'                      =>  'icn1'
);


#
#   The remote host acting as the submit host:  should be accessible from scanner:
#
my $remote_host_site = "";

%svk_ge_data_browser_conf::remote_host = (
    $ENV{HOST}                    =>  "$remote_host_site"
);


#
#   VRE (volume recon engine) compute node (aka ICN - image compute node).  
#
my $VRE_host = "icn1";
%svk_ge_data_browser_conf::VRE_host = (
    $ENV{HOST}                    =>  "icn1",
);


# 
#   set up work path tail: scanner_host_name/script_name 
# 
my $script_name = svk_ge_get_tail($0);
my $dir_tail = "$svk_ge_data_browser_conf::network_hostnames{$ENV{HOST}}/$script_name";


#
#   The remote data directory 
#
$svk_ge_data_browser_conf::remote_data_dir = "/data/lhst3/scanner_data/$dir_tail";


#
#   The work dir on the scanner:
#	/root/host_name/script_name
#
if ( defined $ENV{SDCHOME} ) {
    $svk_ge_data_browser_conf::scanner_work_dir = "$ENV{SDCHOME}/svk/console/data/$dir_tail";
} else {
    # VRE: 	
    $svk_ge_data_browser_conf::scanner_work_dir = "/export/home/sdc/svk/console/data/$dir_tail";
}


#
#   The work dir on the scanner:
#	/root/host_name/script_name
#
$svk_ge_data_browser_conf::vre_bin_dir = "/export/home/sdc/svk/console/bin";
$svk_ge_data_browser_conf::vre_svk_bin_dir = "/export/home/sdc/svk/local/bin";


sub svk_ge_get_tail($)
{
    my $path = $_[0];
    my $tail;

    if ($path =~ /.*\/(\S+)$/) {
        $tail = $1;
    }
    else {
        $tail = $path;
    }

    return $tail;
}

