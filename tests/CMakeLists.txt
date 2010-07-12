#
#   $URL$
#   $Rev$
#   $Author$
#   $Date$
#

cmake_minimum_required(VERSION 2.6)


PROJECT(svk_test)


#
#   Set build dir:
#
IF (CMAKE_SYSTEM_NAME MATCHES Linux)
    IF (CMAKE_SYSTEM_PROCESSOR MATCHES i686)
        SET(PLATFORM Linux_i686)
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
    ELSE (CMAKE_SYSTEM_PROCESSOR MATCHES i686)
        SET(PLATFORM Linux_x86_64)
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
    ENDIF (CMAKE_SYSTEM_PROCESSOR MATCHES i686)
    SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
ELSE (CMAKE_SYSTEM_NAME MATCHES Linux)
    IF (CMAKE_SYSTEM_NAME MATCHES Darwin)
        SET(PLATFORM Darwin_i386)
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
    ELSE (CMAKE_SYSTEM_NAME MATCHES Darwin)
        SET(PLATFORM SunOS_sparc)
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
        SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "-xO3 -DNDEBUG")

        SET(CMAKE_CXX_LINK_FLAGS_RELEASE "-xO3 -DNDEBUG")
        SET(CMAKE_CXX_FLAGS_RELEASE "-xO3 -DNDEBUG")
        SET(CMAKE_CXX_FLAGS "-xO3 -DNDEBUG")
    ENDIF (CMAKE_SYSTEM_NAME MATCHES Darwin)
ENDIF (CMAKE_SYSTEM_NAME MATCHES Linux)


IF (CMAKE_BUILD_TYPE MATCHES Debug)
    SET(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}_debug)
    SET(LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH}_debug)
    ADD_DEFINITIONS(-DDEBUG_BUILD) 
    SET(PLATFORM ${PLATFORM}_debug)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

SET(CMAKE_VERBOSE_MAKEFILE TRUE)

INCLUDE(${CMAKE_ROOT}/Modules/FindOpenGL.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/FindGLU.cmake)

###################################
#   VTK
###################################
INCLUDE(${CMAKE_ROOT}/Modules/FindVTK.cmake)
IF (USE_VTK_FILE)
    INCLUDE(${USE_VTK_FILE})
ENDIF (USE_VTK_FILE)

ADD_DEFINITIONS(-DHAVE_CONFIG_H) 

LINK_DIRECTORIES(
    ../${PLATFORM}
    ../lib/${PLATFORM}
    ${DCMTK_DIR}/lib
)

INCLUDE_DIRECTORIES(
    ./
    ../libs/src
    ${DCMTK_DIR}/include
)

ADD_LIBRARY( 
    svkTest

    svkTestUtils.cc
)


SET(SVK_LIBS
    vtkRendering
    vtkGraphics
    vtkHybrid
    vtkImaging
    vtkIO
    vtkFiltering
    vtkCommon
    vtkVolumeRendering
    svkTest
    svkadapt 
    svk 
    svkadapt 
    dcmdata
    ofstd 
    z
)




ADD_EXECUTABLE(
    vtkRenderLoopTest
    vtkRenderLoopTest.cc
)


TARGET_LINK_LIBRARIES(
    vtkRenderLoopTest
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
	${SVK_LIBS}
)

include( CMakeSivicTests.cmake )
