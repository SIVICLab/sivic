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

cmake_minimum_required(VERSION 2.6)

#cmake_policy(SET CMP0002 OLD)
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0015 OLD)
endif(COMMAND cmake_policy)


PROJECT(svk_test)

option( BUILD_GRAPHICS_TESTING "Run graphics test." FALSE )
option( USE_OFF_SCREEN_TESTING "Use offscreen rendering if available." TRUE )

#
#   Set build dir:
#
IF (CMAKE_SYSTEM_NAME MATCHES Linux)
    IF (USE_OFF_SCREEN_TESTING )
        SET( GRAPHICS_WRAPPER ${CMAKE_SOURCE_DIR}/trunk/tests/scripts/xvfb-run )
    ENDIF (USE_OFF_SCREEN_TESTING )
    IF (CMAKE_SYSTEM_PROCESSOR MATCHES i686)
        SET(PLATFORM Linux_i686)
    ELSE (CMAKE_SYSTEM_PROCESSOR MATCHES i686)
        SET(PLATFORM Linux_x86_64)
    ENDIF (CMAKE_SYSTEM_PROCESSOR MATCHES i686)
    SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    SET(PLATFORM_LIBS z)
ELSE (CMAKE_SYSTEM_NAME MATCHES Linux)
    IF (CMAKE_SYSTEM_NAME MATCHES Darwin)
        SET(PLATFORM Darwin_i386)
    ENDIF (CMAKE_SYSTEM_NAME MATCHES Darwin)
    IF (CMAKE_SYSTEM_NAME MATCHES SunOS)
        SET(PLATFORM SunOS_sparc)
        SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "-xO3 -DNDEBUG")

        SET(CMAKE_CXX_LINK_FLAGS_RELEASE "-xO3 -DNDEBUG")
        SET(CMAKE_CXX_FLAGS_RELEASE "-xO3 -DNDEBUG")
        SET(CMAKE_CXX_FLAGS "-xO3 -DNDEBUG")
    ENDIF (CMAKE_SYSTEM_NAME MATCHES SunOS)
ENDIF (CMAKE_SYSTEM_NAME MATCHES Linux)


IF(WIN32)
    SET(GNU_DIFFUTILS_PATH CACHE PATH "Location of gnu diffutils installation. Provides diff command for testing on Windows." )
ENDIF(WIN32)


SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})


IF (CMAKE_BUILD_TYPE MATCHES Debug)
    SET(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}_debug)
    SET(LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH}_debug)
    ADD_DEFINITIONS(-DDEBUG_BUILD) 
    SET(PLATFORM ${PLATFORM}_debug)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

SET(CMAKE_VERBOSE_MAKEFILE TRUE)

INCLUDE(${CMAKE_ROOT}/Modules/FindOpenGL.cmake)

SET(DCMTK_OFLOG "")
if ( EXISTS "${DCMTK_DIR}/lib/liboflog.a" OR EXISTS "${DCMTK_DIR}/lib/oflog.lib")
    SET(DCMTK_OFLOG oflog )
endif (EXISTS "${DCMTK_DIR}/lib/liboflog.a" OR EXISTS "${DCMTK_DIR}/lib/oflog.lib")

###################################
#   VTK
###################################
FIND_PACKAGE(VTK)

IF (USE_VTK_FILE)
    INCLUDE(${USE_VTK_FILE})
ENDIF (USE_VTK_FILE)

FIND_PATH(DCMTK_DIR dctk.h)

IF( NOT WIN32 )
	ADD_DEFINITIONS(-DHAVE_CONFIG_H)
ENDIF( NOT WIN32 )



if ( BUILD_CMINPACK )

    ###################################
    #   ITK 
    ###################################
    ADD_DEFINITIONS(-DITK_BUILD)
    FIND_PACKAGE( ITK REQUIRED )
    IF(ITK_FOUND)
        INCLUDE(${ITK_USE_FILE})
    ELSE(ITK_FOUND)
        MESSAGE(FATAL_ERROR "Cannot build without ITK.  Please set ITK_DIR.")
    ENDIF(ITK_FOUND)

endif( BUILD_CMINPACK )



###################################
#   CTEST 
###################################
ENABLE_TESTING()
INCLUDE(CTest)


LINK_DIRECTORIES(
    ../trunk/${PLATFORM}
    ../trunk/lib/${PLATFORM}
    ${DCMTK_DIR}/lib
)

IF(WIN32)
	SET(PLATFORM_INCLUDE ../../libs/getopt)
ENDIF(WIN32)

INCLUDE_DIRECTORIES(
    ./
    ../../libs/src
    ${DCMTK_DIR}/include
    ${PLATFORM_INCLUDE}
)

IF(WIN32)
	SET(PLATFORM_LIBS getopt
	                  netapi32)
ENDIF(WIN32)

ADD_LIBRARY( 
    svkTest
    svkTestUtils.cc
)


if ( BUILD_CMINPACK )
    SET( CMINPACK_LIBS 
        svkNumerical 
        ITKDICOMParser
    )
    SET( CMINPACK_LIBS_PRE
        ITKOptimizers
        ITKCommon
        ITKDICOMParser
    )
endif( BUILD_CMINPACK )


SET(SVK_LIBS
    ${CMINPACK_LIBS}
    svkTest
    svk 
    svkadapt 
    vtkIO
    vtkImaging
    vtkFiltering
    vtkCommon
    dcmdata
    ${DCMTK_OFLOG}
    ofstd 
    ${PLATFORM_LIBS}
    ${CMINPACK_LIBS_PRE}
)

SET(SVK_VIZ_LIBS
    svkviz
    vtkRendering
    vtkGraphics
    vtkHybrid
)


ADD_EXECUTABLE(
    vtkBaselineTest
    vtkBaselineTest.cc
)

TARGET_LINK_LIBRARIES(
    vtkBaselineTest
	${SVK_VIZ_LIBS}
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    runtimeException
    runtimeException.cc
)

TARGET_LINK_LIBRARIES(
    runtimeException
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    assertFailTest
    assertFailTest.cc
)

TARGET_LINK_LIBRARIES(
    assertFailTest
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    returnFailure
    returnFailure.cc
)

TARGET_LINK_LIBRARIES(
    returnFailure
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    svkIdfWriterTest
    svkIdfWriterTest.cc
)

TARGET_LINK_LIBRARIES(
    svkIdfWriterTest
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    svkPlotGridViewTest
    svkPlotGridViewTest.cc
)

TARGET_LINK_LIBRARIES(
    svkPlotGridViewTest
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svkOverlayViewTest
    svkOverlayViewTest.cc
)

TARGET_LINK_LIBRARIES(
    svkOverlayViewTest
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svkInstantiationTest
    svkInstantiationTest.cc
)

TARGET_LINK_LIBRARIES(
    svkInstantiationTest
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svkPlotGridViewMultiTraceTest
    svkPlotGridViewMultiTraceTest.cc
)

TARGET_LINK_LIBRARIES(
    svkPlotGridViewMultiTraceTest
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svkMrsImageFFT
    svkMrsImageFFT.cc
)

TARGET_LINK_LIBRARIES(
    svkMrsImageFFT
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svkMriImageFFTTest
    svkMriImageFFTTest.cc
)

TARGET_LINK_LIBRARIES(
    svkMriImageFFTTest
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svkMriZeroFillTest
    svkMriZeroFillTest.cc
)

TARGET_LINK_LIBRARIES(
    svkMriZeroFillTest
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svkImageLinearPhaseTest
    svkImageLinearPhaseTest.cc
)

TARGET_LINK_LIBRARIES(
    svkImageLinearPhaseTest
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svkImageFourierCenterTest
    svkImageFourierCenterTest.cc
)

TARGET_LINK_LIBRARIES(
    svkImageFourierCenterTest
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svkSincInterpolateTest
    svkSincInterpolateTest.cc
)

TARGET_LINK_LIBRARIES(
    svkSincInterpolateTest
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svkMrsCombine
    svkMrsCombine.cc
)

TARGET_LINK_LIBRARIES(
    svkMrsCombine
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svkMrsZeroFillTest
    svkMrsZeroFillTest.cc
)

TARGET_LINK_LIBRARIES(
    svkMrsZeroFillTest
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svkMrsLinearPhaseTest
    svkMrsLinearPhaseTest.cc
)

TARGET_LINK_LIBRARIES(
    svkMrsLinearPhaseTest
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svkMrsFFTVoxelShiftTest
    svkMrsFFTVoxelShiftTest.cc
)

TARGET_LINK_LIBRARIES(
    svkMrsFFTVoxelShiftTest
    ${SVK_LIBS}
)
ADD_EXECUTABLE(
     svkMrsApodizationFilterTest
     svkMrsApodizationFilterTest.cc
 )
 
 TARGET_LINK_LIBRARIES(
     svkMrsApodizationFilterTest
       ${SVK_LIBS}
 )

ADD_EXECUTABLE(
    svkZeroLinearPhaseTest
    svkZeroLinearPhaseTest.cc
)

TARGET_LINK_LIBRARIES(
    svkZeroLinearPhaseTest
	${SVK_LIBS}
)
ADD_EXECUTABLE(
     svkMri4DImageDataTest
     svkMri4DImageDataTest.cc
 )
TARGET_LINK_LIBRARIES(
    svkMri4DImageDataTest
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

include(CMakeSivicTests.cmake)

IF (BUILD_GRAPHICS_TESTING)
    include(CMakeSivicGraphicsTests.cmake)
ENDIF (BUILD_GRAPHICS_TESTING)
