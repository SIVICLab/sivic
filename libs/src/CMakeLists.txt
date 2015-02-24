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

#
#   $URL$
#   $Rev$
#   $Author$
#   $Date$
#

cmake_minimum_required(VERSION 2.4)

PROJECT(svk)

#
#   Set build dir:
#
IF (CMAKE_SYSTEM_NAME MATCHES Linux)
    IF (CMAKE_SYSTEM_PROCESSOR MATCHES i686)
        SET(EXECUTABLE_OUTPUT_PATH ../Linux_i686)
        SET(LIBRARY_OUTPUT_PATH ../Linux_i686)
    ELSE (CMAKE_SYSTEM_PROCESSOR MATCHES i686)
        SET(EXECUTABLE_OUTPUT_PATH ../Linux_x86_64)
        SET(LIBRARY_OUTPUT_PATH ../Linux_x86_64)
    ENDIF (CMAKE_SYSTEM_PROCESSOR MATCHES i686)
ELSE (CMAKE_SYSTEM_NAME MATCHES Linux)
    IF (CMAKE_SYSTEM_NAME MATCHES Darwin)
        SET(EXECUTABLE_OUTPUT_PATH ../Darwin_i386)
        SET(LIBRARY_OUTPUT_PATH ../Darwin_i386)
        SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELESAE} -O3 -DNDEBUG -DDarwin -Wno-deprecated")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -DDarwin -Wno-deprecated")
        IF (CMAKE_UNIVERSAL MATCHES TRUE)    
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch i386 -arch x86_64")
        ENDIF (CMAKE_UNIVERSAL MATCHES TRUE)    
    ENDIF (CMAKE_SYSTEM_NAME MATCHES Darwin)
    IF(CMAKE_SYSTEM_NAME MATCHES SunOS)    
        SET(EXECUTABLE_OUTPUT_PATH ../SunOS_sparc)
        SET(LIBRARY_OUTPUT_PATH ../SunOS_sparc)
        SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "-xO3 -DNDEBUG")
        SET(CMAKE_EXE_LINKER_FLAGS "-xO3 -DNDEBUG")

        SET(CMAKE_CXX_FLAGS_RELEASE "-xO3 -DNDEBUG -Wno-deprecated")
        SET(CMAKE_CXX_FLAGS "-xO3 -DNDEBUG -Wno-deprecated")
        SET(CMAKE_CXX_DEBUG "-xO3 -DNDEBUG -Wno-deprecated")
    ENDIF (CMAKE_SYSTEM_NAME MATCHES SunOS)
ENDIF (CMAKE_SYSTEM_NAME MATCHES Linux)

SET(CMAKE_VERBOSE_MAKEFILE TRUE)

IF ( BUILD_EPIC )
    SET(LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH}_epic)
ENDIF ( BUILD_EPIC )

IF (CMAKE_BUILD_TYPE MATCHES Debug)
    SET(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}_debug)
    SET(LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH}_debug)
    ADD_DEFINITIONS(-DVTK_DEBUG_ON)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)



###################################
#   VTK
###################################
FIND_PACKAGE( VTK REQUIRED )
IF(VTK_FOUND)
  INCLUDE(${VTK_USE_FILE})
ENDIF(VTK_FOUND)



###################################
#   KWWidgets
###################################
#INCLUDE(${KWWidgets_USE_FILE})


###################################
#   DCMTK (DCMTK_DIR is the root include dir
##################################
IF( NOT CMAKE_SYSTEM_NAME MATCHES Windows)
	ADD_DEFINITIONS(-DHAVE_CONFIG_H)
ENDIF( NOT CMAKE_SYSTEM_NAME MATCHES Windows)
ADD_DEFINITIONS(-DSVK_ADAPT_DCMTK) 

IF(WIN32)
	SET(PLATFORM_INCLUDE ../getopt)
ENDIF(WIN32)

INCLUDE_DIRECTORIES(
    ./
    ${DCMTK_DIR}/include
	${PLATFORM_INCLUDE}
)

if ( BUILD_ITK )
    INCLUDE_DIRECTORIES ( ${ITK_DIR}/include )
endif( BUILD_ITK )

if ( BUILD_CLAPACK )
    INCLUDE_DIRECTORIES ( ${CLAPACK_DIR}/INCLUDE)
endif( BUILD_CLAPACK )


IF(WIN32)
	ADD_LIBRARY( getopt
			     STATIC
				 ../getopt/getopt.c
				 ../getopt/getopt_long.c
			    )
ENDIF(WIN32)

ADD_LIBRARY(
    svk 

    svkImageDataFactory.cc
    svkSpecPoint.cc
    svkImageData.cc
    svk4DImageData.cc
    svkFastCellData.cc
    svkMrsImageData.cc
    svkMriImageData.cc
    svkDcmHeader.cc
    svkProvenance.cc
    svkImageWriter.cc
    svkIdfVolumeWriter.cc
    svkDdfVolumeWriter.cc
    svkDICOMSCWriter.cc
    svkDICOMWriter.cc
    svkDICOMImageWriter.cc
    svkDICOMMRSWriter.cc
    svkDICOMEnhancedMRIWriter.cc
    svkDICOMMRIWriter.cc
    svkDICOMRawDataWriter.cc
    svkImageWriterFactory.cc
    svkImageReaderFactory.cc
    svkImageReader2.cc
    svkImageMathematics.cc
    svkSiemensRdaReader.cc
    svkGEPFileReader.cc
    svkGEPFileMapper.cc
    svkGEPFileMapperMBrease.cc
    svkGEPFileMapperUCSF.cc
    svkGEPFileMapperUCSFfidcsi.cc
    svkGEPFileMapperUCSFfidcsiDev0.cc
    svkGEPFileMapperUCSFfidcsiDev07t.cc
    svkGEPFileMapperMPCSIOBL.cc
    svkIdfVolumeReader.cc
    svkDdfVolumeReader.cc
    svkSdbmVolumeReader.cc
    svkDcmVolumeReader.cc
    svkDcmMrsVolumeReader.cc
    svkDcmMriVolumeReader.cc
    svkDcmEnhancedVolumeReader.cc
    svkDcmSegmentationVolumeReader.cc
    svkDcmRawDataReader.cc
    svkGEPostageStampReader.cc
    svkBrukerDCMMRSReader.cc
    svkVarianReader.cc
    svkFdfVolumeReader.cc
    svkVarianFidReader.cc
    svkVarianCSFidMapper.cc
    svkVarianUCSF2DcsiMapper.cc
    svkVarianUCSFEPSI2DMapper.cc
    svkVarianFidMapper.cc
    svkGESigna5XReader.cc
    svkGESignaLX2Reader.cc
    svkPACSInterface.cc
    svkUCSFPACSInterface.cc
    svkGEConsolePACSInterface.cc
    svkDataModel.cc
    svkDataValidator.cc
    svkImageInPlaceFilter.cc    
    svkThreadedImageAlgorithm.cc
    svkPhaseSpec.cc
    svkAlgoTemplate.cc
    svkEPSIPhaseCorrect.cc
    svkEPSIReorder.cc
    svkBurnResearchPixels.cc
    svkMrsImageFFT.cc
    svkMriImageFFT.cc
    svkMriZeroFill.cc
    svkMrsZeroFill.cc
    svkMrsLinearPhase.cc
    svkSincInterpolationFilter.cc
    svkMrsSingleVoxelSincExtraction.cc
    svkMrsImageFlip.cc
    svkMrsImageFourierCenter.cc
    svkImageLinearPhase.cc
    svkImageFourierCenter.cc
    svkImageAlgorithm.cc
    svkImageAlgorithmExecuter.cc
    svkImageThreshold.cc
    svkAlgorithmPortMapper.cc
    svkImageAlgorithmWithPortMapper.cc
    svkImageAlgorithmPipeline.cc
    svkGenericAlgorithmWithPortMapper.cc
    svkDSCDeltaR2.cc
    svkImageCopy.cc
    svkQuantifyMetabolites.cc
    svkQuantifyDSC.cc
    svkMetaboliteMap.cc
    svkDSCMap.cc
    svkDSCPeakHeight.cc
    svkDSCRecovery.cc
    svkMetaboliteRatioZScores.cc
    svkImageClip.cc
    svkObliqueReslice.cc
    svkIOD.cc
    svkSCIOD.cc
    svkMRSIOD.cc
    svkMRSNoise.cc
    svkMRSPeakPick.cc
    svkMRSAverageSpectra.cc
    svkMRIIOD.cc
    svkEnhancedMRIIOD.cc
    svkRawIOD.cc
    svkLookupTable.cc
    svkByteSwap.cc
    svkMultiCoilPhase.cc
    svkMRSCombine.cc
    svkCorrectDCOffset.cc
    svkSpecUtils.cc
    svkUCSFUtils.cc
    svkUtils.cc
    svkVoxelTaggingUtils.cc
    #svkImageFFT.cc
    #svkImageRFFT.cc
    svkIntegratePeak.cc
    svkApodizationWindow.cc
    svkMrsApodizationFilter.cc
    svkExtractVOI.cc
    svkDICOMParser.cc
    svkImageStatistics.cc
    svkDouble.cc
    svkBool.cc
    svkInt.cc
    svkString.cc
    svkXML.cc
    svkF2C.cc
)


ADD_LIBRARY(
    svkTypeUtils

    svkTypeUtils.cc
)


ADD_LIBRARY(
    svkPSD 

    svkFileUtils.cc
    svkSatBandsXML.cc
)



IF( BUILD_VIZ_LIBRARY )

    SET(SVK_ORIENTED_IMAGE_ACTOR_SRC svkImageActor.cc)
    IF (VTK_MAJOR_VERSION EQUAL 5 AND VTK_MINOR_VERSION LESS 10)
        SET(SVK_ORIENTED_IMAGE_ACTOR_SRC svkOpenGLOrientedImageActor.cc)
    ENDIF (VTK_MAJOR_VERSION EQUAL 5 AND VTK_MINOR_VERSION LESS 10)

    ADD_LIBRARY(
        svkviz

        svkDataView.cc
        svkDataViewController.cc
        svkPlotGridView.cc
        svkPlotGridViewController.cc
        svkDetailedPlotDirector.cc
        svkXYPlotActor.cc
        svkOverlaySelector.cc
        svkOverlayView.cc
        svkOverlayViewController.cc
        svkLabeledDataMapper.cc
        svkSatBandSet.cc
        svkImageTopologyGenerator.cc
        svkMrsTopoGenerator.cc
        svkPlotLine.cc
        svkPlotLineGrid.cc
        svkSpecGridSelector.cc
        svkImageViewer2.cc
        svkImageMapToWindowLevelColors.cc
        svkImageMapToColors.cc
        svkLabeledDataMapper.cc
        svkVizUtils.cc
        ${SVK_ORIENTED_IMAGE_ACTOR_SRC}
    )
ENDIF( BUILD_VIZ_LIBRARY )


ADD_LIBRARY(
    svkadapt

    svkDcmtkAdapter.cc
    svkDcmtkIod.cc
    svkDcmtkUtils.cc
    svkDcmtkException.cc
)

IF ( BUILD_ITK )

    ADD_DEFINITIONS(-DITK_BUILD)
    ###################################
    #   ITK 
    ###################################
    FIND_PACKAGE( ITK REQUIRED )
    IF(ITK_FOUND)
        INCLUDE(${ITK_USE_FILE})
    ELSE(ITK_FOUND)
        MESSAGE(FATAL_ERROR "Cannot build without ITK.  Please set ITK_DIR.")
    ENDIF(ITK_FOUND)

    ADD_LIBRARY(
        svkNumerical

        svkMRSAutoPhase.cc
        svkMRSFirstPointPhase.cc
        svkMRSZeroOrderPhase.cc
        #svkMRSFirstOrderPhase.cc
        svkMRSKinetics.cc 
    )
ENDIF ( BUILD_ITK )

IF ( BUILD_CLAPACK )
    ADD_LIBRARY(
        svkNumerical2

        svkHSVD.cc 
    )
ENDIF ( BUILD_CLAPACK )

#IF (BUILD_TESTS)
#    ENABLE_TESTING()
#
#    ADD_TEST(VTK_RENDER_LOOP ${EXECUTABLE_OUTPUT_PATH}/vtkRenderLoopTest)
#    ADD_TEST(RUNTIME_EXCEPTION ${EXECUTABLE_OUTPUT_PATH}/runtimeException)
#    ADD_TEST(SCRIPT_FAILURE ${EXECUTABLE_OUTPUT_PATH}/../tests/scripts/scriptFailTest)
#    ADD_TEST(ASSERT_FAILURE ${EXECUTABLE_OUTPUT_PATH}/assertFailTest)
#    ADD_TEST(RETURN_FAILURE ${EXECUTABLE_OUTPUT_PATH}/returnFailure)
#    ADD_TEST(READ_WRITE_SIVIC_IMAGE ${EXECUTABLE_OUTPUT_PATH}/../tests/scripts/idfReadWriteTest.pl
#                                    ${EXECUTABLE_OUTPUT_PATH}/svkIdfWriterTest ${SVK_DATA_ROOT}/sivic_out)
#    ADD_TEST(READ_WRITE_NON_SIVIC_IMAGE ${EXECUTABLE_OUTPUT_PATH}/../tests/scripts/idfReadWriteTest.pl
#                                    ${EXECUTABLE_OUTPUT_PATH}/svkIdfWriterTest ${SVK_DATA_ROOT}/t0000_fla)
#
#ENDIF(BUILD_TESTS)
