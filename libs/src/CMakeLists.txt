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
    SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -Wno-deprecated")
ELSE (CMAKE_SYSTEM_NAME MATCHES Linux)
    IF (CMAKE_SYSTEM_NAME MATCHES Darwin)
        SET(EXECUTABLE_OUTPUT_PATH ../Darwin_i386)
        SET(LIBRARY_OUTPUT_PATH ../Darwin_i386)
        SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -DDarwin -Wno-deprecated")
        SET(CMAKE_CXX_FLAGS "-O3 -DNDEBUG -DDarwin -Wno-deprecated")
        IF (CMAKE_UNIVERSAL MATCHES TRUE)    
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch i386 -arch x86_64")
        ENDIF (CMAKE_UNIVERSAL MATCHES TRUE)    
    ELSE (CMAKE_SYSTEM_NAME MATCHES Darwin)
        SET(EXECUTABLE_OUTPUT_PATH ../SunOS_sparc)
        SET(LIBRARY_OUTPUT_PATH ../SunOS_sparc)
        SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "-xO3 -DNDEBUG")
        SET(CMAKE_EXE_LINKER_FLAGS "-xO3 -DNDEBUG")

        SET(CMAKE_CXX_FLAGS_RELEASE "-xO3 -DNDEBUG -Wno-deprecated")
        SET(CMAKE_CXX_FLAGS "-xO3 -DNDEBUG -Wno-deprecated")
        SET(CMAKE_CXX_DEBUG "-xO3 -DNDEBUG -Wno-deprecated")
    ENDIF (CMAKE_SYSTEM_NAME MATCHES Darwin)
ENDIF (CMAKE_SYSTEM_NAME MATCHES Linux)

SET(CMAKE_VERBOSE_MAKEFILE TRUE)


IF (CMAKE_BUILD_TYPE MATCHES Debug)
    SET(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}_debug)
    SET(LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH}_debug)
    ADD_DEFINITIONS(-DVTK_DEBUG_ON)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)


INCLUDE(${CMAKE_ROOT}/Modules/FindOpenGL.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/FindGLU.cmake)


###################################
#   VTK
###################################
INCLUDE(${CMAKE_ROOT}/Modules/FindVTK.cmake)
SET(VTK_WRAP_TCL ON)
IF (USE_VTK_FILE)
    INCLUDE(${USE_VTK_FILE})
ENDIF (USE_VTK_FILE)

###################################
#   KWWidgets
###################################
FIND_PACKAGE(KWWidgets REQUIRED)
INCLUDE(${KWWidgets_USE_FILE})


###################################
#   DCMTK (DCMTK_DIR is the root include dir
###################################
ADD_DEFINITIONS(-DHAVE_CONFIG_H -DSVK_ADAPT_DCMTK) 

FIND_PATH(DCMTK_DIR dctk.h)


INCLUDE_DIRECTORIES(
    ./
    ${DCMTK_DIR}/include
)



ADD_LIBRARY(
    svk 

    svkImageDataFactory.cc
    svkSpecPoint.cc
    svkImageData.cc
    svkMrsImageData.cc
    svkMriImageData.cc
    svkDcmHeader.cc
    svkProvenance.cc
    svkImageWriter.cc
    svkIdfVolumeWriter.cc
    svkDdfVolumeWriter.cc
    svkDICOMSCWriter.cc
    svkDICOMMRSWriter.cc
    svkImageWriterFactory.cc
    svkMultiWindowToImageFilter.cc
    svkImageReaderFactory.cc
    svkImageReader2.cc
    svkSiemensRdaReader.cc
    svkGEPFileReader.cc
    svkGEPFileMapper.cc
    svkGEPFileMapperMBrease.cc
    svkGEPFileMapperUCSF.cc
    svkGEPFileMapperUCSFfidcsi.cc
    svkIdfVolumeReader.cc
    svkDdfVolumeReader.cc
    svkSdbmVolumeReader.cc
    svkDcmVolumeReader.cc
    svkDcmMrsVolumeReader.cc
    svkDcmMriVolumeReader.cc
    svkVarianReader.cc
    svkFdfVolumeReader.cc
    svkVarianFidReader.cc
    svkVarianCSFidMapper.cc
    svkVarianFidMapper.cc
    svkDataView.cc
    svkDataViewController.cc
    svkImageView2D.cc
    svkImageView2DController.cc
	svkDataModel.cc
	svkDataValidator.cc
    svkSatBandSet.cc
    svkAreaPicker.cc
    svkBoxPlot.cc
    svkPlotLine.cc
    svkPlotGrid.cc
    svkPlotLineGrid.cc
    svkSpecGridSelector.cc
    svkPlotGridView.cc
    svkPlotGridViewController.cc
    svkDetailedPlotView.cc
    svkDetailedPlotViewController.cc
    svkOverlaySelector.cc
    svkOverlayView.cc
    svkOverlayViewController.cc
    svkSpectraReferenceView.cc
    svkSpectraReferenceViewController.cc
    svkImageTopologyGenerator.cc
    svkImageViewer2.cc
    svkMrsTopoGenerator.cc
    svkImageInPlaceFilter.cc    
    svkPhaseSpec.cc
    svkMrsImageFFT.cc
    svkImageLinearPhase.cc
    svkImageMapToWindowLevelColors.cc
    svkImageMapToColors.cc
    svkImageAlgorithm.cc
    svkImageAlgorithmExecuter.cc
    svkImageCopy.cc
    svkImageClip.cc
    svkObliqueReslice.cc
    svkIOD.cc
    svkSCIOD.cc
    svkMRSIOD.cc
    svkMRIIOD.cc
    svkOpenGLOrientedImageActor.cc
    svkOrientedImageActorFactory.cc
    svkLookupTable.cc
    svkByteSwap.cc
    svkMultiCoilPhase.cc
    svkCoilCombine.cc
    svkSpecUtils.cc
    svkUCSFUtils.cc
    svkImageFourierCenter.cc
)


ADD_LIBRARY(
    svkadapt

    svkDcmtkAdapter.cc
    svkDcmtkIod.cc
    svkDcmtkUtils.cc
    svkDcmtkException.cc
)
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
