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
#   $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/applications/sivic_app/src/CMakeLists.txt $
#   $Rev: 10 $
#   $Author: beckn8tor $
#   $Date: 2009-12-02 15:08:59 -0800 (Wed, 02 Dec 2009) $
#

cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


PROJECT(SIVIC_MRS_UPDATE)

INCLUDE(CheckCCompilerFlag)
IF(UNIX)
  #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Werror -Wconversion")
  # No "-Wall -Werror" for C flags, to avoid third_party code break.
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
ENDIF(UNIX)

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
    SET(CMAKE_EXE_LINKER_FLAGS 
    "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib") 
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
#   Slicer
###################################

if(NOT Slicer3_SOURCE_DIR)
    find_package(Slicer3 REQUIRED)
    include(${Slicer3_USE_FILE})
    slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)


###################################
#   VTK
###################################
INCLUDE(${CMAKE_ROOT}/Modules/FindVTK.cmake)
IF (USE_VTK_FILE)
    INCLUDE(${USE_VTK_FILE})
ENDIF (USE_VTK_FILE)



###################################
#   DCMTK (DCMTK_DIR is the root include dir
###################################
#ADD_DEFINITIONS(-DHAVE_CONFIG_H) 
ADD_DEFINITIONS(-DHAVE_CONFIG_H -DSVK_ADAPT_DCMTK -D_DARWIN_C_SOURCE)

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
    ../../../libs/src
    ${DCMTK_DIR}/include
    ${Slicer3_Libs_INCLUDE_DIRS}
    ${Slicer3_Base_INCLUDE_DIRS}
)

##############################################
# Configure include file
##############################################
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/vtkMRSpectroscopyConfigure.h.in 
    ${CMAKE_CURRENT_BINARY_DIR}/vtkMRSpectroscopyConfigure.h
  )

file(GLOB headers 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  )

install(FILES 
    ${headers} 
    "${CMAKE_CURRENT_BINARY_DIR}/vtkMRSpectroscopyConfigure.h"
    DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )


# --------------------------------------------------------------------------
# Sources

set(MRSpectroscopy_SRCS 
    vtkMRSpectroscopyLogic.cxx
    vtkMRSpectroscopyGUI.cxx
)


# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(MRSpectroscopy MRSpectroscopy_TCL_SRCS "${MRSpectroscopy_SRCS}" "")



#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(MRSpectroscopy_SRCS MRSpectroscopy.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name MRSpectroscopy_update)
add_library(${lib_name}
    ${MRSpectroscopy_SRCS} 
    ${MRSpectroscopy_TCL_SRCS}
  )

slicer3_set_modules_output_path(${lib_name})


TARGET_LINK_LIBRARIES(
    ${lib_name}

    vtkRendering
    vtkGraphics
    vtkHybrid
    vtkImaging
    vtkIO
    vtkFiltering
    vtkCommon
    svk 
    svkadapt
    dcmdata
    ofstd 
    ${Slicer3_Libs_LIBRARIES}
    ${Slicer3_Base_LIBRARIES}
    ${KWWidgets_LIBRARIES}
    z
)

install(TARGETS ${lib_name}
    RUNTIME DESTINATION ${Slicer3_INSTALL_MODULES_BIN_DIR} COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT Development
  )

slicer3_install_modules(${lib_name})


IF (CMAKE_SYSTEM_NAME MATCHES Darwin)
#    SET_TARGET_PROPERTIES (sivic PROPERTIES LINK_FLAGS 
#    "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib") 
    SET(VTK_USE_PARALLEL "OFF")
ENDIF (CMAKE_SYSTEM_NAME MATCHES Darwin)

IF (CMAKE_SYSTEM_NAME MATCHES SunOS)
    SET_TARGET_PROPERTIES (sivic PROPERTIES LINK_FLAGS "")
ENDIF (CMAKE_SYSTEM_NAME MATCHES SunOS)
