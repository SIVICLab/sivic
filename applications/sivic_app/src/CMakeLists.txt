##
#  Copyright © 2009 The Regents of the University of California.
#  All Rights Reserved.
#
#  Redistribution and use in source and binary forms, with or without 
#  modification, are permitted provided that the following conditions are met:
#  •   Redistributions of source code must retain the above copyright notice, 
#      this list of conditions and the following disclaimer.
#  •   Redistributions in binary form must reproduce the above copyright notice, 
#      this list of conditions and the following disclaimer in the documentation 
#      and/or other materials provided with the distribution.
#  •   None of the names of any campus of the University of California, the name 
#      "The Regents of the University of California," or the names of any of its 
#      contributors may be used to endorse or promote products derived from this 
#      software without specific prior written permission.
#  
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
#  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
#  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
#  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
#  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
#  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
#  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
#  OF SUCH DAMAGE.
##

#
#   $URL$
#   $Rev$
#   $Author$
#   $Date$
#

cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


PROJECT(sivic_application)



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
        IF (CMAKE_UNIVERSAL MATCHES TRUE)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDarwin -arch i386 -arch x86_64")
        ENDIF (CMAKE_UNIVERSAL MATCHES TRUE)
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

IF (UCSF_INTERNAL)
    ADD_DEFINITIONS(-DUCSF_INTERNAL)
ENDIF(UCSF_INTERNAL) 

IF (SVK_USE_GL2PS)
    ADD_DEFINITIONS(-DSVK_USE_GL2PS)
ENDIF(SVK_USE_GL2PS) 

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


###################################
#   KWWidgets
###################################
FIND_PACKAGE(KWWidgets REQUIRED)
INCLUDE(${KWWidgets_USE_FILE})


set(LIB_NAME "SivicKWCallbacksLib")
set(LIB_SRCS
  vtkSivicController.cc
)
include("${KWWidgets_CMAKE_DIR}/KWWidgetsWrappingMacros.cmake")
kwwidgets_wrap_tcl(${LIB_NAME} LIB_TCL_SRCS "${LIB_SRCS}" "")
add_library(${LIB_NAME} ${LIB_TCL_SRCS} ${LIB_SRCS})
target_link_libraries(${LIB_NAME} ${KWWidgets_LIBRARIES})


###################################
#   DCMTK (DCMTK_DIR is the root include dir
###################################
ADD_DEFINITIONS(-DHAVE_CONFIG_H) 
FIND_PATH(DCMTK_DIR dctk.h)




#############################################
#    Build Test Application
#############################################

LINK_DIRECTORIES(
    ../../../libs/${PLATFORM}
    ../${PLATFORM}
    ${DCMTK_DIR}/lib
)

INCLUDE_DIRECTORIES(
    ./
    ../../../libs/src/
    ${DCMTK_DIR}/include
)


ADD_LIBRARY( 
    sivicGUI

    sivicApp.cc
    sivicGlobalWidget.cc
    sivicImageViewWidget.cc
    sivicSpectraViewWidget.cc
    sivicSpectraRangeWidget.cc
    sivicViewRenderingWidget.cc
    sivicProcessingWidget.cc
    sivicKWCompositeWidget.cc
    vtkSivicController.cc
    sivicAppGUI.cc
    sivicBasicTest.cc
    sivicTestCase.cc
    sivicTestSuite.cc
)


ADD_EXECUTABLE(
    sivic
    sivic.cc
)



TARGET_LINK_LIBRARIES(
    sivic
    
    vtkRendering
    vtkGraphics
    vtkHybrid
    vtkImaging
    vtkIO
    vtkFiltering
    vtkCommon
    sivicGUI
    svk 
    svkadapt
    dcmdata
    ofstd 
    ${KWWidgets_LIBRARIES}
    #KWCallbacksExampleLib
    SivicKWCallbacksLib
    z
)


IF (CMAKE_SYSTEM_NAME MATCHES Darwin)
    SET_TARGET_PROPERTIES (sivic PROPERTIES LINK_FLAGS 
    "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib") 
    SET(VTK_USE_PARALLEL "OFF")
ENDIF (CMAKE_SYSTEM_NAME MATCHES Darwin)

IF (CMAKE_SYSTEM_NAME MATCHES SunOS)
    SET_TARGET_PROPERTIES (sivic PROPERTIES LINK_FLAGS "")
ENDIF (CMAKE_SYSTEM_NAME MATCHES SunOS)
