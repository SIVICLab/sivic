# 
#   Copyright © 2009-2011 The Regents of the University of California.
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

#SET(MEMORYCHECK_SUPPRESSIONS_FILE /home/bolson/Projects/sivic/trunk/tests/src/suppressions/renderSuppressions)
#INCLUDE(CTest)
#ENABLE_TESTING()
#############################################################
#   Paths to binary applications and scripts
#############################################################
SET( DEDICATED_TEST_BIN_PATH ${CMAKE_BINARY_DIR}/trunk/tests/${PLATFORM})

#############################################################
#   Location where output files from individual tests 
#   are written to. 
#############################################################
SET( TEST_RESULTS_ROOT ${SVK_TEST_ROOT}/results_tmp)

#############################################################
#   Flags for diff to avoid errors from minor differences in 
#   paths and rootnames in header files. 
#############################################################
SET( DIFF_OPT --ignore-matching-lines=SVK_ --ignore-matching-lines=root --exclude=.svn)

#############################################################
# This is a basic test to see if VTK is working
#############################################################
SET( TEST_NAME VTK_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/vtk_baseline)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER} ${DEDICATED_TEST_BIN_PATH}/vtkBaselineTest ${TEST_RESULTS_PATH}/out.jpeg )

SET( TEST_NAME VTK_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/${PLATFORM} )
SET_TESTS_PROPERTIES(VTK_RENDER_DIFF PROPERTIES DEPENDS VTK_RENDER_MCHK)


#############################################################
# Check to see if you can render spectra from a phantom 
#############################################################
SET( TEST_NAME PHANTOM_SPECTRA_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/ddf_files/ddf_to_ddf)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER} ${DEDICATED_TEST_BIN_PATH}/svkPlotGridViewTest -t RenderingTest --spectra ${TEST_CASE_ROOT}/input/20x_1.ddf -p ${TEST_RESULTS_PATH}
 )

SET( TEST_NAME PHANTOM_SPECTRA_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${PLATFORM} )
SET_TESTS_PROPERTIES(PHANTOM_SPECTRA_RENDER_DIFF PROPERTIES DEPENDS PHANTOM_SPECTRA_RENDER_MCHK)

#############################################################
# Memory Test for svkOverlayView 
#############################################################
SET( TEST_NAME OVERLAY_MEMORY_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER}  ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -d -t MemoryTest --image ${TEST_CASE_ROOT}/input/refImage.idf --spectra ${TEST_CASE_ROOT}/input/spec.ddf --overlay ${TEST_CASE_ROOT}/input/met.idf --second_image ${TEST_CASE_ROOT}/input/refImage.idf --second_spectra ${TEST_CASE_ROOT}/input/spec.ddf --second_overlay ${TEST_CASE_ROOT}/input/met.idf)

#############################################################
# Load Order Test for svkOverlayView 
#############################################################
SET( TEST_NAME OVERLAY_LOAD_ORDER_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/reslicing/axial_to_oblique)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER}  ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -d -t LoadOrderTest --image ${TEST_CASE_ROOT}/input/axial_vol.idf --spectra ${TEST_CASE_ROOT}/input/oblique_mrsvol.ddf  --overlay ${TEST_CASE_ROOT}/input/oblique_vol.idf --second_image ${TEST_CASE_ROOT}/input/oblique_vol.idf --second_spectra ${TEST_CASE_ROOT}/input/axial_mrsvol.ddf --second_overlay ${TEST_CASE_ROOT}/input/axial_vol.idf -p ${TEST_RESULTS_PATH} ) 

SET( TEST_NAME OVERLAY_LOAD_ORDER_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${PLATFORM} )
SET_TESTS_PROPERTIES(OVERLAY_LOAD_ORDER_DIFF PROPERTIES DEPENDS OVERLAY_LOAD_ORDER_RENDER_MCHK)

#############################################################
# Check to see if you can render an image from a phantom 
#############################################################
SET( TEST_NAME PHANTOM_IMAGE_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/idf_files/idf_to_idf)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER}  ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -d -t RenderingTest --image ${TEST_CASE_ROOT}/input/vol.idf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME PHANTOM_IMAGE_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/out_1/${PLATFORM} )
SET_TESTS_PROPERTIES(PHANTOM_IMAGE_RENDER_DIFF PROPERTIES DEPENDS PHANTOM_IMAGE_RENDER_MCHK)

#############################################################
# Check to see if you can render an image and spectra from a 
# phantom.
#############################################################
SET( TEST_NAME PHNTM_IMG_SPEC_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/idf_files/idf_to_idf)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER} ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -d -t RenderingTest --image ${TEST_CASE_ROOT}/input/vol.idf --spectra ${SVK_TEST_ROOT}/ddf_files/ddf_to_ddf/input/20x_1.ddf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME PHNTM_IMG_SPEC_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/out_2/${PLATFORM} )
SET_TESTS_PROPERTIES(PHNTM_IMG_SPEC_RENDER_DIFF PROPERTIES DEPENDS PHNTM_IMG_SPEC_RENDER_MCHK)

#############################################################
# Check to see if you can render an image with spectra and an 
# overlay from a phantom.
#############################################################
SET( TEST_NAME OVERLAY_MET_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER} ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -d -t RenderingTest --image ${TEST_CASE_ROOT}/input/refImage.idf --spectra ${TEST_CASE_ROOT}/input/spec.ddf --overlay ${TEST_CASE_ROOT}/input/met.idf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME OVERLAY_MET_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/out_3/${PLATFORM} )
SET_TESTS_PROPERTIES(OVERLAY_MET_RENDER_DIFF PROPERTIES DEPENDS OVERLAY_MET_RENDER_MCHK)

#############################################################
# Check to see if you can see all the color maps.
#############################################################
SET( TEST_NAME COLOR_MAP_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER} ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -d -t ColorMapTest --image ${TEST_CASE_ROOT}/input/refImage.idf --spectra ${TEST_CASE_ROOT}/input/spec.ddf --overlay ${TEST_CASE_ROOT}/input/met.idf --cni_overlay  ${TEST_CASE_ROOT}/input/met_cni.idf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME COLOR_MAP_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/out_7/${PLATFORM} )
SET_TESTS_PROPERTIES(COLOR_MAP_RENDER_DIFF PROPERTIES DEPENDS COLOR_MAP_RENDER_MCHK)

#############################################################
# Check to see if you can render spectra and an overlay from
# a phantom.
#############################################################
SET( TEST_NAME PLOT_GRID_MET_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER}  ${DEDICATED_TEST_BIN_PATH}/svkPlotGridViewTest -t RenderingTest --spectra ${TEST_CASE_ROOT}/input/spec.ddf --overlay ${TEST_CASE_ROOT}/input/met.idf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME PLOT_GRID_MET_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/out_5/${PLATFORM} )
SET_TESTS_PROPERTIES(PLOT_GRID_MET_RENDER_DIFF PROPERTIES DEPENDS PLOT_GRID_MET_RENDER_MCHK)

#############################################################
# Check to see if the validation will catch the origin shift
# in the overlay-- no overlay should be rendered. 
#############################################################
SET( TEST_NAME PLOT_GRID_MET_SHIFT_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER}  ${DEDICATED_TEST_BIN_PATH}/svkPlotGridViewTest -t RenderingTest --spectra ${TEST_CASE_ROOT}/input/spec.ddf --overlay ${TEST_CASE_ROOT}/input/met_shifted.idf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME PLOT_GRID_MET_SHIFT_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/out_6/${PLATFORM} )
SET_TESTS_PROPERTIES(PLOT_GRID_MET_SHIFT_RENDER_DIFF PROPERTIES DEPENDS PLOT_GRID_MET_SHIFT_RENDER_MCHK)


#############################################################
# Check to see if the we can render our phantom in multiple orientations. 
#############################################################
SET( TEST_NAME ORIENTATION_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER} ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -d -t OrientationTest --image ${TEST_CASE_ROOT}/input/refImage.idf --spectra ${TEST_CASE_ROOT}/input/spec.ddf --overlay ${TEST_CASE_ROOT}/input/met.idf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME ORIENTATION_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${TEST_NAME}/${PLATFORM} )
SET_TESTS_PROPERTIES(ORIENTATION_RENDER_DIFF PROPERTIES DEPENDS ORIENTATION_RENDER_MCHK)


#############################################################
# Check to see if you can render spectra and an overlay from
# a phantom in different orientations.
#############################################################
SET( TEST_NAME ORIENTATION_SPEC_MET_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER}  ${DEDICATED_TEST_BIN_PATH}/svkPlotGridViewTest -t OrientationTest --spectra ${TEST_CASE_ROOT}/input/spec.ddf --overlay ${TEST_CASE_ROOT}/input/met.idf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME ORIENTATION_SPEC_MET_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${TEST_NAME}/${PLATFORM} )
SET_TESTS_PROPERTIES(ORIENTATION_SPEC_MET_DIFF PROPERTIES DEPENDS ORIENTATION_SPEC_MET_MCHK)


#############################################################
# Check to see if you can render spectra and an overlay from
# a phantom of an individual voxel with the detailed plot.
#############################################################
SET( TEST_NAME PLOT_GRID_DETAILED_VOXEL_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER}  ${DEDICATED_TEST_BIN_PATH}/svkPlotGridViewTest -l 44 -r 44 -t RenderingTest --spectra ${TEST_CASE_ROOT}/input/spec.ddf --overlay ${TEST_CASE_ROOT}/input/met.idf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME PLOT_GRID_DETAILED_VOXEL_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/out_8/${PLATFORM} )
SET_TESTS_PROPERTIES(PLOT_GRID_DETAILED_VOXEL_DIFF PROPERTIES DEPENDS PLOT_GRID_DETAILED_VOXEL_MCHK)


#############################################################
# Check to see if the we can render a sagittal phantom  image and spectra in multiple orientations. 
#############################################################
SET( TEST_NAME SAG_IMAGE_SPEC_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/orientations)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER} ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -d -t OrientationTest --image ${TEST_CASE_ROOT}/input/sag_phantom.idf --spectra ${TEST_CASE_ROOT}/input/sag_recon.ddf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME SAG_IMAGE_SPEC_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${TEST_NAME}/${PLATFORM} )
SET_TESTS_PROPERTIES(SAG_IMAGE_SPEC_RENDER_DIFF PROPERTIES DEPENDS SAG_IMAGE_SPEC_RENDER_MCHK)


#############################################################
# Check to see if you can render sagittal spectra
#############################################################
SET( TEST_NAME SAG_SPEC_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/orientations)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER}  ${DEDICATED_TEST_BIN_PATH}/svkPlotGridViewTest -t OrientationTest --spectra ${TEST_CASE_ROOT}/input/sag_recon.ddf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME SAG_SPEC_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${TEST_NAME}/${PLATFORM} )
SET_TESTS_PROPERTIES(SAG_SPEC_RENDER_DIFF PROPERTIES DEPENDS SAG_SPEC_RENDER_MCHK)


#############################################################
# Check to see if the we can render a coronal phantom  image and spectra in multiple orientations. 
#############################################################
SET( TEST_NAME COR_IMAGE_SPEC_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/orientations)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER} ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -d -t OrientationTest --image ${TEST_CASE_ROOT}/input/cor_phantom.idf --spectra ${TEST_CASE_ROOT}/input/cor_phased.ddf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME COR_IMAGE_SPEC_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${TEST_NAME}/${PLATFORM} )
SET_TESTS_PROPERTIES(COR_IMAGE_SPEC_RENDER_DIFF PROPERTIES DEPENDS COR_IMAGE_SPEC_RENDER_MCHK)


#############################################################
# Check to see if you can render coronal spectra
#############################################################
SET( TEST_NAME COR_SPEC_RENDER_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/orientations)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER}  ${DEDICATED_TEST_BIN_PATH}/svkPlotGridViewTest -t OrientationTest --spectra ${TEST_CASE_ROOT}/input/cor_phased.ddf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME COR_SPEC_RENDER_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${TEST_NAME}/${PLATFORM} )
SET_TESTS_PROPERTIES(COR_SPEC_RENDER_DIFF PROPERTIES DEPENDS COR_SPEC_RENDER_MCHK)


#############################################################
# Check to see if you can render spectra from a phantom with sat bands
#############################################################
SET( TEST_NAME PHANTOM_SPECTRA_SAT_BANDS_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/sat_bands)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER} ${DEDICATED_TEST_BIN_PATH}/svkPlotGridViewTest -t SatBandTest --spectra ${TEST_CASE_ROOT}/input/sag_recon.ddf -p ${TEST_RESULTS_PATH}
 )

SET( TEST_NAME PHANTOM_SPECTRA_SAT_BANDS_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${TEST_NAME}/${PLATFORM} )
SET_TESTS_PROPERTIES(PHANTOM_SPECTRA_SAT_BANDS_DIFF PROPERTIES DEPENDS PHANTOM_SPECTRA_SAT_BANDS_MCHK)

#############################################################
# Test for the svk_multi_view command line tool 
#############################################################
SET( TEST_NAME SVK_MULTI_VIEW_TEST_1_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/svk_multi_view)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER}  ${TEST_BIN_PATH_CMD_LINE}/svk_multi_view -s ${SVK_TEST_ROOT}/ge_pfiles/24x/input/24x -s ${SVK_TEST_ROOT}/ge_pfiles/24x/input/24x -j ${TEST_RESULTS_PATH}/out -b 6 -e 51 -l -5.5e+05 -u 1.5e+06 -c 6 -p 1 -t 2 -w 600
 )

SET( TEST_NAME SVK_MULTI_VIEW_TEST_1_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/out1 )
SET_TESTS_PROPERTIES(SVK_MULTI_VIEW_TEST_1_DIFF PROPERTIES DEPENDS SVK_MULTI_VIEW_TEST_1_MCHK)

#############################################################
# Test for the svk_multi_view command line tool 
#############################################################
SET( TEST_NAME SVK_MULTI_VIEW_TEST_2_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/svk_multi_view)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER}  ${TEST_BIN_PATH_CMD_LINE}/svk_multi_view -s ${SVK_TEST_ROOT}/ge_pfiles/24x/input/24x -j ${TEST_RESULTS_PATH}/out -b 101 -e 151 -w 600
 )

SET( TEST_NAME SVK_MULTI_VIEW_TEST_2_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/out2 )
SET_TESTS_PROPERTIES(SVK_MULTI_VIEW_TEST_2_DIFF PROPERTIES DEPENDS SVK_MULTI_VIEW_TEST_2_MCHK)

#############################################################
# Test for the svk_multi_view command line tool 
#############################################################
SET( TEST_NAME SVK_MULTI_VIEW_TEST_3_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/svk_multi_view)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER}  ${TEST_BIN_PATH_CMD_LINE}/svk_multi_view -s ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets/input/spec.ddf -j ${TEST_RESULTS_PATH}/out ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets/input/refImage.idf -o ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets/input/met.idf -w 500
 )

SET( TEST_NAME SVK_MULTI_VIEW_TEST_3_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/out3 )
SET_TESTS_PROPERTIES(SVK_MULTI_VIEW_TEST_3_DIFF PROPERTIES DEPENDS SVK_MULTI_VIEW_TEST_3_MCHK)

#############################################################
# Test for the svk_multi_view command line tool 
#############################################################
SET( TEST_NAME SVK_MULTI_VIEW_TEST_4_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/svk_multi_view)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER}  ${TEST_BIN_PATH_CMD_LINE}/svk_multi_view -s ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets/input/spec.ddf -j ${TEST_RESULTS_PATH}/out ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets/input/refImage.idf -o ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets/input/met.idf -i 27 -w 500
 )

SET( TEST_NAME SVK_MULTI_VIEW_TEST_4_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/out4 )
SET_TESTS_PROPERTIES(SVK_MULTI_VIEW_TEST_4_DIFF PROPERTIES DEPENDS SVK_MULTI_VIEW_TEST_4_MCHK)

#############################################################
# Test for the svk_multi_view command line tool 
#############################################################
SET( TEST_NAME SVK_MULTI_VIEW_TEST_5_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/svk_multi_view)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER}  ${TEST_BIN_PATH_CMD_LINE}/svk_multi_view -s ${SVK_TEST_ROOT}/overlay_validation/ddf_idf_mets/input/spec.ddf -j ${TEST_RESULTS_PATH}/out -b 425 -e 650 -l -8.0e+07 -u 4.0e+08 -i 6 -w 500
 )

SET( TEST_NAME SVK_MULTI_VIEW_TEST_5_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/out5 )
SET_TESTS_PROPERTIES(SVK_MULTI_VIEW_TEST_5_DIFF PROPERTIES DEPENDS SVK_MULTI_VIEW_TEST_5_MCHK)

#############################################################
# Test for the svk_multi_view command line tool 
#############################################################
SET( TEST_NAME SVK_MULTI_VIEW_TEST_6_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/svk_multi_view)
ADD_TEST(${TEST_NAME}  ${GRAPHICS_WRAPPER}  ${TEST_BIN_PATH_CMD_LINE}/svk_multi_view -s ${SVK_TEST_ROOT}/ge_pfiles/24x/input/24x -s ${SVK_TEST_ROOT}/ge_pfiles/24x/input/24x -j ${TEST_RESULTS_PATH}/out -b 5 -e 50 -l -1.5e+06 -u 3.75e+06 -c 8 -p 1 -t 1 -c 3 -p 2 -t 2 -w 600
 )

SET( TEST_NAME SVK_MULTI_VIEW_TEST_6_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/out6 )
SET_TESTS_PROPERTIES(SVK_MULTI_VIEW_TEST_6_DIFF PROPERTIES DEPENDS SVK_MULTI_VIEW_TEST_6_MCHK)

#############################################################
# Check to see if you can render an image from a phantom with sat bands
#############################################################
SET( TEST_NAME PHANTOM_IMAGE_SAT_BANDS_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/sat_bands)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER}  ${DEDICATED_TEST_BIN_PATH}/svkOverlayViewTest -d -t SatBandTest --image ${TEST_CASE_ROOT}/input/sag_phantom.idf --spectra ${TEST_CASE_ROOT}/input/sag_recon.ddf -p ${TEST_RESULTS_PATH} )

SET( TEST_NAME PHANTOM_IMAGE_SAT_BANDS_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${TEST_NAME}/${PLATFORM} )
SET_TESTS_PROPERTIES(PHANTOM_IMAGE_SAT_BANDS_DIFF PROPERTIES DEPENDS PHANTOM_IMAGE_SAT_BANDS_MCHK)


#############################################################
# Check to see if you can render image data as 4D lines 
#############################################################
SET( TEST_NAME MRI_4D_IMAGE_MCHK)
SET( TEST_RESULTS_PATH ${TEST_RESULTS_ROOT}/${TEST_NAME})
FILE( REMOVE_RECURSE ${TEST_RESULTS_PATH} )
FILE( MAKE_DIRECTORY ${TEST_RESULTS_PATH} )
SET( TEST_CASE_ROOT ${SVK_TEST_ROOT}/multi_volume)
ADD_TEST(${TEST_NAME} ${GRAPHICS_WRAPPER}  ${DEDICATED_TEST_BIN_PATH}/svkMri4DImageDataTest ${TEST_CASE_ROOT}/input/E10755S2I1.DCM ${TEST_RESULTS_PATH}  62094 65009 4)

SET( TEST_NAME MRI_4D_IMAGE_DIFF)
ADD_TEST(${TEST_NAME}  diff ${DIFF_OPT} -r ${TEST_RESULTS_PATH} ${TEST_CASE_ROOT}/render_results/${TEST_NAME}/${PLATFORM} )
SET_TESTS_PROPERTIES(MRI_4D_IMAGE_DIFF PROPERTIES DEPENDS MRI_4D_IMAGE_MCHK)
