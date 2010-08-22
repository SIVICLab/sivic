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


SET( TEST_SCRIPT_PATH ${CMAKE_SOURCE_DIR}/trunk/tests/scripts)
SET( TEST_BIN_PATH ${CMAKE_SOURCE_DIR}/trunk/applications/cmd_line/${PLATFORM})


########################
#   20X GE Raw to DDF
########################
SET( TEST_NAME test_1)
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/ge_pfiles/20x)
ADD_TEST(TEST_1  ${TEST_SCRIPT_PATH}/test_svk_file_convert --platform ${PLATFORM} --bin_path ${TEST_BIN_PATH} --test_path ${SVK_TEST_ROOT} --test_name ${TEST_NAME} --baseline_path ${TEST_CASE_ROOT} --opts "-i ${TEST_CASE_ROOT}/20_x_raw -o 20x -t 2" )

########################
#   UCSF ddf to UCSF DDF
########################
SET( TEST_NAME test_2)
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/ddf_files/ddf_to_ddf)
ADD_TEST(TEST_2  ${TEST_SCRIPT_PATH}/test_svk_file_convert --platform ${PLATFORM} --bin_path ${TEST_BIN_PATH} --test_path ${SVK_TEST_ROOT} --test_name ${TEST_NAME} --baseline_path ${TEST_CASE_ROOT} --opts "-i ${TEST_CASE_ROOT}/20x_1.ddf -o 20x -t 2" )

########################
#   DICOM MRI to UCSF IDF 
########################
SET( TEST_NAME test_3)
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/DICOM/mri_phantom)
ADD_TEST(TEST_3  ${TEST_SCRIPT_PATH}/test_svk_file_convert --platform ${PLATFORM} --bin_path ${TEST_BIN_PATH} --test_path ${SVK_TEST_ROOT} --test_name ${TEST_NAME} --baseline_path ${TEST_CASE_ROOT} --opts "-i ${TEST_CASE_ROOT}/E0000S4I1.DCM -o vol -t 3" )

########################
#   UCSF IDF to UCSF IDF 
########################
SET( TEST_NAME test_4)
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/idf_files/idf_to_idf)
ADD_TEST(TEST_4  ${TEST_SCRIPT_PATH}/test_svk_file_convert --platform ${PLATFORM} --bin_path ${TEST_BIN_PATH} --test_path ${SVK_TEST_ROOT} --test_name ${TEST_NAME} --baseline_path ${TEST_CASE_ROOT} --opts "-i ${TEST_CASE_ROOT}/vol.idf -o out -t 3" )


########################
#   svk_gepfile_reader:   
########################
#ADD_TEST(READ_PFILE ../../applications/cmd_line/src/${PLATFORM}/svk_gepfile_reader -i ${SVK_TEST_ROOT}/ge_pfiles/20x/20_x_raw -o ${SVK_TEST_ROOT}/output/out )




#ADD_TEST(VTK_RENDER_LOOP ${EXECUTABLE_OUTPUT_PATH}/vtkRenderLoopTest)
#ADD_TEST(RUNTIME_EXCEPTION ${EXECUTABLE_OUTPUT_PATH}/runtimeException)

#ADD_TEST(SCRIPT_FAILURE ${EXECUTABLE_OUTPUT_PATH}/../tests/scripts/scriptFailTest)
#ADD_TEST(ASSERT_FAILURE ${EXECUTABLE_OUTPUT_PATH}/assertFailTest)
#ADD_TEST(RETURN_FAILURE ${EXECUTABLE_OUTPUT_PATH}/returnFailure)

#ADD_TEST(READ_WRITE_SIVIC_IMAGE ${EXECUTABLE_OUTPUT_PATH}/../tests/scripts/idfReadWriteTest.pl
#                                ${EXECUTABLE_OUTPUT_PATH}/svkIdfWriterTest ${SVK_TEST_ROOT}/sivic_out)
#IF( TEST_OUTPUT_DIR )
#    SET( TEST_OUTPUT_DIR -p${TEST_OUTPUT_DIR} )
#ENDIF(TEST_OUTPUT_DIR)

#ADD_TEST(PLOT_GRID_VIEW ${EXECUTABLE_OUTPUT_PATH}/../tests/scripts/plotGridViewRenderTest.pl
#                                -t ${EXECUTABLE_OUTPUT_PATH}/svkPlotGridViewTest -s${SVK_TEST_ROOT}/t0000_1_cor.ddf 
#                                ${TEST_OUTPUT_DIR} -r${TEST_REFERENCE_DIR})

#ADD_TEST(READ_WRITE_NON_SIVIC_IMAGE ${EXECUTABLE_OUTPUT_PATH}/../tests/scripts/idfReadWriteTest.pl
#                                    ${EXECUTABLE_OUTPUT_PATH}/svkIdfWriterTest ${SVK_TEST_ROOT}/t0000_fla)
