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


#############################################################
#   Paths to binary applications and scripts
#############################################################
SET( TEST_SCRIPT_PATH ${CMAKE_SOURCE_DIR}/trunk/tests/scripts)
SET( TEST_BIN_PATH ${CMAKE_SOURCE_DIR}/trunk/applications/cmd_line/${PLATFORM})

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


#############################################################
#   Tests are in pairs:
#     - odd tests are calls to svk binaries which can be tested with valgrind
#       for memory checking (dynamic ctest analysis).  Files are generated from  
#       these tests into $(TEST_RESULTS_ROOT).  Even tests check for run-time errors
#       and memory allocation. 
#     - even tests diff result files generated in the odd test against a baseline 
#       file.  Even test check for processing correctness. 
#############################################################

########################
#   20X GE Raw to DDF
########################
SET( TEST_NAME TEST_1)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/ge_pfiles/20x)
ADD_TEST(${TEST_NAME}  ${TEST_BIN_PATH}/svk_file_convert -i ${TEST_CASE_ROOT}/input/20_x_raw -o${TEST_RESULTS_PATH}/out -t 2 )

SET( TEST_NAME TEST_2)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/${PLATFORM} )


########################
#   15X GE Raw to DDF
########################
SET( TEST_NAME TEST_3)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/ge_pfiles/15x)
ADD_TEST(${TEST_NAME}  ${TEST_BIN_PATH}/svk_file_convert -i ${TEST_CASE_ROOT}/input/15x_raw -o${TEST_RESULTS_PATH}/out -t 2 )

SET( TEST_NAME TEST_4)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/${PLATFORM} )


########################
#   14X GE Raw to DDF
########################
SET( TEST_NAME TEST_5)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/ge_pfiles/14x)
ADD_TEST(${TEST_NAME}  ${TEST_BIN_PATH}/svk_file_convert -i ${TEST_CASE_ROOT}/input/14x_raw -o${TEST_RESULTS_PATH}/out -t 2 )

SET( TEST_NAME TEST_6)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/${PLATFORM} )


########################
#   12X GE Raw to DDF
########################
SET( TEST_NAME TEST_7)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/ge_pfiles/12x)
ADD_TEST(${TEST_NAME}  ${TEST_BIN_PATH}/svk_file_convert -i ${TEST_CASE_ROOT}/input/12x_raw -o${TEST_RESULTS_PATH}/out -t 2 )

SET( TEST_NAME TEST_8)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/${PLATFORM} )


########################
#   11X GE Raw to DDF
########################
SET( TEST_NAME TEST_9)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/ge_pfiles/11x)
ADD_TEST(${TEST_NAME}  ${TEST_BIN_PATH}/svk_file_convert -i ${TEST_CASE_ROOT}/input/11x_raw -o${TEST_RESULTS_PATH}/out -t 2 )

SET( TEST_NAME TEST_10)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/${PLATFORM} )


########################
#   9X GE Raw to DDF
########################
#SET( TEST_NAME TEST_6)
#SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/ge_pfiles/9x)
#ADD_TEST(${TEST_NAME}  ${TEST_SCRIPT_PATH}/test_svk_file_convert --platform ${PLATFORM} --bin_path ${TEST_BIN_PATH} --test_path ${SVK_TEST_ROOT} --test_name ${TEST_NAME} --baseline_path ${TEST_CASE_ROOT} --opts "-i ${TEST_CASE_ROOT}/input/9x_raw -o out -t 2" )


########################
#   UCSF ddf to UCSF DDF
########################
SET( TEST_NAME TEST_11)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/ddf_files/ddf_to_ddf)
ADD_TEST(${TEST_NAME}  ${TEST_BIN_PATH}/svk_file_convert -i ${TEST_CASE_ROOT}/input/20x_1.ddf -o${TEST_RESULTS_PATH}/out -t 2 )

SET( TEST_NAME TEST_12)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/${PLATFORM} )


########################
#   DICOM MRI to UCSF IDF 
########################
SET( TEST_NAME TEST_13)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/DICOM/mri_phantom)
ADD_TEST(${TEST_NAME}  ${TEST_BIN_PATH}/svk_file_convert -i ${TEST_CASE_ROOT}/input/E0000S4I1.DCM -o${TEST_RESULTS_PATH}/out -t 3 )

SET( TEST_NAME TEST_14)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/${PLATFORM} )


########################
#   UCSF IDF to UCSF IDF 
########################
SET( TEST_NAME TEST_15)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/idf_files/idf_to_idf)
ADD_TEST(${TEST_NAME}  ${TEST_BIN_PATH}/svk_file_convert -i ${TEST_CASE_ROOT}/input/vol.idf -o${TEST_RESULTS_PATH}/out -t 3 )

SET( TEST_NAME TEST_16)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/${PLATFORM} )


########################
#   VARIAN fdf to UCSF IDF 
########################
SET( TEST_NAME TEST_17)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/varian/fdf)
ADD_TEST(${TEST_NAME}  ${TEST_BIN_PATH}/svk_file_convert -i ${TEST_CASE_ROOT}/input/lac.0001.fdf -o${TEST_RESULTS_PATH}/out -t 3 )

SET( TEST_NAME TEST_18)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/${PLATFORM} )


########################
#   VARIAN fid to UCSF DDF 
########################
SET( TEST_NAME TEST_19)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/varian/fid_cs)
ADD_TEST(${TEST_NAME}  ${TEST_BIN_PATH}/svk_file_convert -i ${TEST_CASE_ROOT}/input/fid -o${TEST_RESULTS_PATH}/out -t 2 )

SET( TEST_NAME TEST_20)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/${PLATFORM} )


########################
#   GE Signa 5x to UCSF IDF 
########################
SET( TEST_NAME TEST_21)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/ge_signa_files/Signa5x)
ADD_TEST(${TEST_NAME}  ${TEST_BIN_PATH}/svk_file_convert -i ${TEST_CASE_ROOT}/input/E8668S3I1.MR -o${TEST_RESULTS_PATH}/out -t 3 )

SET( TEST_NAME TEST_22)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/${PLATFORM} )


########################
#   GE Signa 5x V2 to UCSF IDF 
########################
SET( TEST_NAME TEST_23)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/ge_signa_files/Signa5XVersion2)
ADD_TEST(${TEST_NAME}  ${TEST_BIN_PATH}/svk_file_convert -i ${TEST_CASE_ROOT}/input/E18636S3I1.MR -o${TEST_RESULTS_PATH}/out -t 3 )

SET( TEST_NAME TEST_24)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/${PLATFORM} )


########################
#   GE Signa 5x V2 to UCSF IDF 
########################
SET( TEST_NAME TEST_25)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/ge_signa_files/SignaLX2)
ADD_TEST(${TEST_NAME}  ${TEST_BIN_PATH}/svk_file_convert -i ${TEST_CASE_ROOT}/input/E12916S3I1.MR -o${TEST_RESULTS_PATH}/out -t 3 )

SET( TEST_NAME TEST_26)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/${PLATFORM} )


########################
#   SIEMENS RDA to DICOM DDF
########################
SET( TEST_NAME TEST_27)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/siemens_rda/sv)
ADD_TEST(${TEST_NAME}  ${TEST_BIN_PATH}/svk_file_convert -i ${TEST_CASE_ROOT}/input/sample.rda -o${TEST_RESULTS_PATH}/out -t 2 )

SET( TEST_NAME TEST_28)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/${PLATFORM} )


########################
#   DICOM MRS to DICOM MRS 
########################
SET( TEST_NAME TEST_29)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/DICOM/mrs)
ADD_TEST(${TEST_NAME}  ${TEST_BIN_PATH}/svk_file_convert -i ${TEST_CASE_ROOT}/input/mrs.dcm -o${TEST_RESULTS_PATH}/out.dcm -t 4 )

SET( TEST_NAME TEST_30)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/${PLATFORM} )

