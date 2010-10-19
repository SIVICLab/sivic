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
#   $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/tests/src/CMakeSivicTests.cmake $
#   $Rev: 608 $
#   $Author: jccrane $
#   $Date: 2010-09-11 00:01:42 -0700 (Sat, 11 Sep 2010) $
#
#   Authors:
#       Jason C. Crane, Ph.D.
#       Beck Olson

#SET(MEMORYCHECK_SUPPRESSIONS_FILE /home/bolson/Projects/sivic/trunk/tests/src/suppressions/renderSuppressions)
#INCLUDE(CTest)
#ENABLE_TESTING()
#############################################################
#   Paths to binary applications and scripts
#############################################################
SET( DEDICATED_TEST_BIN_PATH ${CMAKE_SOURCE_DIR}/trunk/tests/${PLATFORM})

#############################################################
#   Location where output files from individual tests 
#   are written to. 
#############################################################
SET( TEST_RESULTS_ROOT ${SVK_TEST_ROOT}/results_tmp)

#############################################################
#   Flags for diff to avoid errors from minor differences in 
#   paths and rootnames in header files. 
#############################################################
SET( DIFF_OPT --ignore-matching-lines=SVK_CMD --ignore-matching-lines=root)


SET( TEST_NAME VTK_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/vtk_baseline)
ADD_TEST(${TEST_NAME}  ${DEDICATED_TEST_BIN_PATH}/vtkBaselineTest ${TEST_RESULTS_PATH}/out.jpeg )

SET( TEST_NAME VTK_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/${PLATFORM} )

SET( TEST_NAME PHANTOM_SPECTRA_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/ddf_files/ddf_to_ddf)
ADD_TEST(${TEST_NAME}  ${DEDICATED_TEST_BIN_PATH}/svkPlotGridViewTest -t RenderingTest --spectra ${TEST_CASE_ROOT}/input/20x_1.ddf -p ${TEST_RESULTS_PATH}
 )

SET( TEST_NAME PHANTOM_SPECTRA_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${PLATFORM} )

SET( TEST_NAME PHANTOM_IMAGE_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/idf_files/idf_to_idf)
ADD_TEST(${TEST_NAME}  ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -t RenderingTest --image ${TEST_CASE_ROOT}/input/vol.idf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME PHANTOM_IMAGE_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/out_1/${PLATFORM} )

SET( TEST_NAME PHNTM_IMG_SPEC_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/idf_files/idf_to_idf)
ADD_TEST(${TEST_NAME}  ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -t RenderingTest --image ${TEST_CASE_ROOT}/input/vol.idf --spectra ${SVK_TEST_ROOT}/ddf_files/ddf_to_ddf/input/20x_1.ddf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME PHNTM_IMG_SPEC_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/out_2/${PLATFORM} )
