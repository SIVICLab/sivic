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

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

PROJECT( sivic )

add_definitions(-D__STDC_FORMAT_MACROS)
add_definitions(-DVTK_LEGACY_REMOVE)
add_definitions(-Dvtkstd=std)

# Detect the compiler and set the C++ standard flags accordingly
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
  # For GCC or Clang
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wno-deprecated -Wno-deprecated-declarations")
elseif(MSVC)
  # For Microsoft Visual C++
  # Note: MSVC uses different flags and supports C++14 from Visual Studio 2015 Update 3 onwards
  # Adjust the flag accordingly if using an older version
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++14")
endif()

# For all targets

option( BUILD_TESTING "Build tests." FALSE )

INCLUDE( CMakeSivicInclude.cmake )
include(CMakeSivicInstall.cmake)
include(CPack)

    SET(MEMORYCHECK_COMMAND "/usr/bin/valgrind")
    SET(CTEST_MEMORYCHECK_COMMAND_OPTIONS "--tool=memcheck --leak-check=yes
--show-reachable=yes --workaround-gcc296-bugs=yes --num-callers=50")
    SET(MEMORYCHECK_COMMAND_OPTIONS "--tool=memcheck --leak-check=yes
--show-reachable=yes --workaround-gcc296-bugs=yes --num-callers=50")

    SET(MEMORYCHECK_SUPPRESSIONS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/tests/all.sup)

if( BUILD_TESTING )
    INCLUDE(CTest)
    ENABLE_TESTING()

    add_subdirectory( tests/src )
    FIND_PATH(SVK_TEST_ROOT "path to testing data root" )
    option( BUILD_TESTING "Build tests." TRUE)
    SET (BUILD_TESTING TRUE)

endif ( BUILD_TESTING )


file( MAKE_DIRECTORY Build )
file( MAKE_DIRECTORY Build/Release )
file( MAKE_DIRECTORY Build/Debug)

INCLUDE(CheckCCompilerFlag)

IF(COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0005 OLD)
ENDIF(COMMAND CMAKE_POLICY)


option( BUILD_LIBRARY "Build library." TRUE )
option( BUILD_VIZ_LIBRARY "Build visualization library." TRUE )
option( BUILD_SIVIC "Build stand-alone application." TRUE )
option( BUILD_EXAMPLES "Build examples." FALSE )
option( BUILD_APPS "Build mini-applications." FALSE )
option( BUILD_OSIRIX "Build OsiriX Plugin." FALSE )
option( BUILD_SLICER "Build Slicer Plugin." FALSE )
option( BUILD_GE_CONSOLE "Build SIVIC for GE Console." FALSE )
option( UCSF_INTERNAL "For UCSF Internal builds only." FALSE )
option( SVK_USE_GL2PS "For exporting vector graphics formats, VTK must be built with VTK_USE_GL2PS." FALSE )
option( BUILD_ITK "For building classes that rely on ITK fitting routines."  TRUE )
option( BUILD_CLAPACK "For building classes that rely on clapack fitting routines."  FALSE )
option( BUILD_PROFILER "Build for profiling with gprof." FALSE)
option( BUILD_TESTS "Build test applications." FALSE)
option( BUILD_CREATE_RAW "Build svk_create_raw. Requires open ssl" FALSE)
option( BUILD_EPIC "Build for EPIC PSD linking." FALSE)
option( CREATE_WRAPPER_SCRIPTS "Create wrapper scripts for executables." TRUE)

ADD_DEFINITIONS(-DSVK_RELEASE_VERSION="\\"${SVK_RELEASE_VERSION}\\"" )
ADD_DEFINITIONS( -DSVK_MAJOR_VERSION="${SVK_MAJOR_VERSION}" )
ADD_DEFINITIONS( -DSVK_MINOR_VERSION="${SVK_MINOR_VERSION}" )

IF(UNIX)
    IF( NOT BUILD_EPIC )
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    ENDIF( NOT BUILD_EPIC )
    IF( BUILD_EPIC )
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    ENDIF( BUILD_EPIC )
ENDIF(UNIX)
IF (WIN32)
    FIND_PATH(TK_INCLUDE "TK Include Path" )
ENDIF(WIN32)

###################################
#   VTK
###################################
set(VTK_INCLUDE_DIRS "/usr/include/vtk")

FIND_PACKAGE(VTK 6.0 REQUIRED COMPONENTS
    CommonCore
    CommonDataModel
    CommonSystem
    IOImage
    ImagingCore
    ImagingFourier
    ImagingGeneral
    FiltersCore
    FiltersGeneral
    FiltersSources
    # Add other required modules as needed
)

include_directories(${VTK_INCLUDE_DIRS})

# Remove deprecated include
#INCLUDE(${VTK_USE_FILE})


###################################
#   KWWidgets
###################################
IF( BUILD_SIVIC OR BUILD_EXAMPLES OR BUILD_APPS OR BUILD_OSIRIX OR BUILD_SLICER )
    FIND_PACKAGE(KWWidgets REQUIRED PATHS ${KWWidgets_DIR} HINTS ${KWWidgets_DIR} )
    MESSAGE(${KWWidgets_USE_FILE})
    INCLUDE(${KWWidgets_USE_FILE})
    IF( NOT BUILD_VIZ_LIBRARY  )
        message( FATAL_ERROR "ERROR: You must turn BUILD_VIZ_LIBRARY on to build requested components (BUILD_SIVIC, BUILD_EXAMPLES, BUILD_APPS, BUILD_OSIRIX, BUILD_SLICER)")
    ENDIF( NOT BUILD_VIZ_LIBRARY )
    
ENDIF( BUILD_SIVIC OR BUILD_EXAMPLES OR BUILD_APPS OR BUILD_OSIRIX OR BUILD_SLICER )

#   DCMTK
###################################
# Set DCMTK include and library directories
set(DCMTK_INCLUDE_DIRS "/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7/include")
set(DCMTK_LIBRARY_DIRS "/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7/lib64")

# Include DCMTK headers
include_directories(${DCMTK_INCLUDE_DIRS})

# List of required DCMTK libraries
set(DCMTK_LIB_NAMES
  ofstd
  oflog
  dcmdata
  dcmimgle
  dcmimage
  # Add other libraries as needed
)

# Find each DCMTK library
foreach(lib ${DCMTK_LIB_NAMES})
  find_library(DCMTK_${lib}_LIBRARY
    NAMES ${lib}
    PATHS ${DCMTK_LIBRARY_DIRS}
    NO_DEFAULT_PATH
  )
  if(NOT DCMTK_${lib}_LIBRARY)
    message(FATAL_ERROR "Cannot find DCMTK library: ${lib}")
  else()
    message(STATUS "Found DCMTK library: ${lib} at ${DCMTK_${lib}_LIBRARY}")
  endif()
  list(APPEND DCMTK_LIBRARIES ${DCMTK_${lib}_LIBRARY})
endforeach()

# Ensure proper linking order
set(DCMTK_LIBRARIES
  ${DCMTK_dcmimage_LIBRARY}
  ${DCMTK_dcmimgle_LIBRARY}
  ${DCMTK_dcmdata_LIBRARY}
  ${DCMTK_oflog_LIBRARY}
  ${DCMTK_ofstd_LIBRARY}
)

# If you have a specific target that requires DCMTK, link the libraries
# Replace 'your_target_name' with the actual target name(s)
# target_link_libraries(your_target_name ${DCMTK_LIBRARIES})

# For example, if you have a library target named 'svk'
# You might link it like this:
# target_link_libraries(svk ${DCMTK_LIBRARIES})
####################

if( BUILD_PROFILER)
    ADD_DEFINITIONS( -pg )
endif( BUILD_PROFILER )

if( BUILD_OSIRIX)
    #   ADD_DEFINITIONS(-DCMAKE_UNIVERSAL=TRUE)
    #   SET (CMAKE_UNIVERSAL TRUE)
endif (BUILD_OSIRIX)

if( BUILD_SLICER )
    add_subdirectory (applications/slicer_plugin/MRSpectroscopy)
endif (BUILD_SLICER)

if( BUILD_GE_CONSOLE )
    ADD_DEFINITIONS(-DBUILD_GE_CONSOLE )
    SET( UCSF_INTERNAL OFF CACHE BOOL "ucsf internal off" FORCE )
    add_subdirectory (applications/scanner/GE_console)
endif( BUILD_GE_CONSOLE )


if( Slicer3_DIR )
    SET(VTK_DIR FORCE)
    SET(KWWidgets_DIR FORCE)
    FIND_PACKAGE(VTK REQUIRED NO_DEFAULT_PATH PATHS ${Slicer3_DIR}/../Slicer3-lib/VTK-build)
    FIND_PACKAGE(KWWidgets REQUIRED NO_DEFAULT_PATH PATHS ${Slicer3_DIR}/../Slicer3-lib/KWWidgets-build)
endif ( Slicer3_DIR )

if( BUILD_LIBRARY )
    add_subdirectory (libs/src)
endif (BUILD_LIBRARY)

if( BUILD_SIVIC )
    add_subdirectory (applications/sivic_app/src)
endif ( BUILD_SIVIC )

if( BUILD_APPS )
    add_subdirectory (applications/cmd_line/src)
endif ( BUILD_APPS )

if( BUILD_EXAMPLES )
    add_subdirectory (libs/examples)
endif ( BUILD_EXAMPLES )

if( BUILD_TESTS )
    add_subdirectory (tests/src)
    FIND_PATH( SVK_TEST_ROOT REQUIRED)
endif ( BUILD_TESTS )

if( BUILD_ITK)
    FIND_PATH( ITK_DIR REQUIRED )
endif ( BUILD_ITK )

if( BUILD_CLAPACK )
    FIND_PATH( CLAPACK_DIR REQUIRED )
endif ( BUILD_CLAPACK )

if( BUILD_CREATE_RAW )
    FIND_PATH( OPENSSL_DIR REQUIRED )
endif ( BUILD_CREATE_RAW )

IF(CLION_BUILD)
    FILE(GLOB CLION_VTK_INCLUDE ${VTK_DIR}/../../../include/*)
    message(CLION_VTK_INCLUDE = ${CLION_VTK_INCLUDE})
ENDIF(CLION_BUILD)

INSTALL_FILES( /local/sivic FILES README LICENSE)
