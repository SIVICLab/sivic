#!/bin/csh

#
#   Copyright © 2009-2010 The Regents of the University of California.
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

# 
#   Configures ctesting.  Assumes that:
#       1.  sivic has been checked out of svn and this is run from the 
#           project root dir. 
#       2.  test cases have been installed under SVK_TEST_ROOT
# 


set DCMTK_DIR=/netopt/dicom
set SVK_TEST_ROOT=/data/lhst3/sivic/test_data 
set GETOPT_LIB
if ( "`/bin/uname`" == "SunOS" ) then
    set GETOPT_LIB=/netopt/lib/local/libgnu.a
endif


rm CMakeCache.txt 
setenv CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage"
set cmake="echo cmake -DCMAKE_BUILD_TYPE=Release -DDCMTK_DIR=${DCMTK_DIR} -DUCSF_INTERNAL=ON -DBUILD_APPS=ON -DBUILD_EXAMPLES=OFF -DBUILD_OSIRIX=OFF -DBUILD_SIVIC=ON -DBUILD_TESTING=ON -DGETOPT_LIB=${GETOPT_LIB} -DSVK_TEST_ROOT=${SVK_TEST_ROOT} -DCMAKE_CXX_FLAGS='-g -O0 -fprofile-arcs -ftest-coverage' -DDART_TESTING_TIMEOUT=6000 ./"

${cmake}
`${cmake}` 

if ( $#argv == 0 ) then
    echo "Must specify test type: Nightly, Experimental. "
    exit 1
else if ( $argv[1] == "Experimental" ) then
    ctest -D Experimental --verbose --build-nocmake --output-log /data/lhst3/sivic/ctest_logs/atom
else if ( $argv[1] == "Nightly" ) then
    ctest -D Nightly --verbose --build-nocmake --output-log /data/lhst3/sivic/ctest_logs/atom
endif

