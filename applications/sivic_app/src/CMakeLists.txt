##
#  Copyright © 2009-2014 The Regents of the University of California.
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
    if(COMMAND cmake_policy)
        cmake_policy(SET CMP0015 OLD)
    endif(COMMAND cmake_policy)
    IF (CMAKE_SYSTEM_PROCESSOR MATCHES i686)
        SET(PLATFORM Linux_i686)
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
    ELSE (CMAKE_SYSTEM_PROCESSOR MATCHES i686)
        SET(PLATFORM Linux_x86_64)
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
    ENDIF (CMAKE_SYSTEM_PROCESSOR MATCHES i686)
ELSE (CMAKE_SYSTEM_NAME MATCHES Linux)
    IF (CMAKE_SYSTEM_NAME MATCHES Darwin)
        ADD_DEFINITIONS( -DDarwin )
        SET(PLATFORM Darwin_i386)
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
        IF (CMAKE_UNIVERSAL MATCHES TRUE)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDarwin -arch x86_64")
            #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDarwin -arch i386 -arch x86_64")
        ENDIF (CMAKE_UNIVERSAL MATCHES TRUE)
    ENDIF (CMAKE_SYSTEM_NAME MATCHES Darwin)
    IF (CMAKE_SYSTEM_NAME MATCHES SunOS)
        SET(PLATFORM SunOS_sparc)
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
        SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "-xO3 -DNDEBUG")

        SET(CMAKE_CXX_LINK_FLAGS_RELEASE "-xO3 -DNDEBUG")
        SET(CMAKE_CXX_FLAGS_RELEASE "-xO3 -DNDEBUG")
        SET(CMAKE_CXX_FLAGS "-xO3 -DNDEBUG")
    ENDIF (CMAKE_SYSTEM_NAME MATCHES SunOS)
    IF (WIN32)
		SET(EXECUTABLE_OUTPUT_PATH Win32)
	endif(WIN32)
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

# if dcmtk 3.6, need to add liboflog: 
SET(DCMTK_OFLOG "")
if ( EXISTS "${DCMTK_DIR}/lib/liboflog.a" OR EXISTS "${DCMTK_DIR}/lib/oflog.lib")
    SET(DCMTK_OFLOG oflog )
endif (EXISTS "${DCMTK_DIR}/lib/liboflog.a" OR EXISTS "${DCMTK_DIR}/lib/oflog.lib")

###################################
#   VTK
###################################
FIND_PACKAGE(VTK REQUIRED)
IF(VTK_FOUND)
  INCLUDE(${VTK_USE_FILE})
ENDIF(VTK_FOUND)


###################################
#   KWWidgets
###################################
INCLUDE(${KWWidgets_USE_FILE})
include("${KWWidgets_CMAKE_DIR}/KWWidgetsTclTkMacros.cmake")

#
#  This section creates a resource file that will be linked in to make static applications possible.
#
IF(WIN32 AND NOT BORLAND AND NOT CYGWIN)
  INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsResourceMacros.cmake")
  SET(RC_FILENAME "${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.rc")
  kwwidgets_create_rc_file(
    RC_FILENAME "${RC_FILENAME}"
    RC_APPLICATION_NAME "${EXE_NAME}"
    RC_COMPANY_NAME "Kitware, Inc.")
ENDIF(WIN32 AND NOT BORLAND AND NOT CYGWIN)

set(LIB_NAME "SivicKWCallbacksLib")
set(LIB_SRCS
  vtkSivicController.cc
)
include("${KWWidgets_CMAKE_DIR}/KWWidgetsWrappingMacros.cmake")
kwwidgets_wrap_tcl(${LIB_NAME} LIB_TCL_SRCS "${LIB_SRCS}" "")
add_library(${LIB_NAME} ${LIB_TCL_SRCS} ${LIB_SRCS})
target_link_libraries(${LIB_NAME} ${KWWidgets_LIBRARIES})


###################################
#   Windows/Unix options
###################################
IF( NOT CMAKE_SYSTEM_NAME MATCHES Windows)
	ADD_DEFINITIONS(-DHAVE_CONFIG_H)
    SET( ZLIB "z" )
ELSE( NOT CMAKE_SYSTEM_NAME MATCHES Windows)
    SET( NETAPI "netapi32" )
ENDIF( NOT CMAKE_SYSTEM_NAME MATCHES Windows)




#############################################
#    Build Test Application
#############################################

LINK_DIRECTORIES(
    ../../../libs/${PLATFORM}
    ../${PLATFORM}
    ${DCMTK_DIR}/lib
)

if ( BUILD_ITK )

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

endif( BUILD_ITK )


if ( BUILD_CLAPACK )
    IF(WIN32)
    	LINK_DIRECTORIES( ${CLAPACK_DIR}/lib )
    ELSE(WIN32)
    	LINK_DIRECTORIES( ${CLAPACK_DIR} )
    ENDIF(WIN32)
endif( BUILD_CLAPACK )

IF(WIN32)
	SET(PLATFORM_LIBS getopt)
	SET(PLATFORM_INCLUDE ../../../libs/getopt)
ENDIF(WIN32)

INCLUDE_DIRECTORIES(
    ./
    ../../../libs/src/
    ${DCMTK_DIR}/include
    ${PLATFORM_INCLUDE}
)

if ( BUILD_ITK )
    SET( ITK_LIBS 
        svkNumerical 
        ITKDICOMParser
    )
    SET( ITK_LIBS_PRE
        ITKOptimizers
        ITKCommon
        ITKDICOMParser
    )
endif( BUILD_ITK )


if ( BUILD_CLAPACK )
    INCLUDE_DIRECTORIES( ${CLAPACK_DIR}/INCLUDE)
    IF(WIN32)
        SET( CLAPACK_LIBS 
            svkNumerical2 
            lapack
            libf2c.lib
            lapack
            blas
        )
    ELSE(WIN32)
        SET( CLAPACK_LIBS 
            svkNumerical2 
            ${CLAPACK_DIR}/libclapack.a
            ${CLAPACK_DIR}/libf2c.a 
            ${CLAPACK_DIR}/libclapack.a
            ${CLAPACK_DIR}/libcblas.a 
        )
    ENDIF(WIN32)
endif( BUILD_CLAPACK )



ADD_LIBRARY( 
    sivicGUI

    STATIC

    sivicApp.cc
    sivicImageViewWidget.cc
    sivicSpectraViewWidget.cc
    sivicSpectraRangeWidget.cc
    sivicViewRenderingWidget.cc
    sivicProcessingWidget.cc
    sivicPhaseWidget.cc
    sivicPreprocessingWidget.cc
    sivicPostprocessingWidget.cc
    sivicDataWidget.cc
    sivicImageDataWidget.cc
    sivicCombineWidget.cc
    sivicDSCWidget.cc
    sivicQuantificationWidget.cc
    sivicWindowLevelWidget.cc
    sivicPreferencesWidget.cc
    sivicVoxelTaggingWidget.cc
    sivicKWCompositeWidget.cc
    vtkSivicController.cc
    sivicBasicTest.cc
    sivicTestCase.cc
    sivicTestSuite.cc
    svkSecondaryCaptureFormatter.cc
)


ADD_EXECUTABLE(
    sivic
    sivic.cc
    ${RC_FILENAME}
)



TARGET_LINK_LIBRARIES(
    sivic

    sivicGUI
    ${ITK_LIBS}
    ${CLAPACK_LIBS}
    svkviz
    svk 
    svkadapt
    vtkRendering
    vtkGraphics
    vtkHybrid
    vtkImaging
    vtkIO
    vtkFiltering
    vtkCommon
    dcmdata
    ${DCMTK_OFLOG}
    ofstd 
    ${KWWidgets_LIBRARIES}
    #KWCallbacksExampleLib
    SivicKWCallbacksLib
	${ZLIB}
    ${NETAPI}
    ${PLATFORM_LIBS}
    ${ITK_LIBS_PRE}

)

IF (NOT WIN32)
	INSTALL_PROGRAMS( /local/sivic FILES ${EXECUTABLE_OUTPUT_PATH}/sivic )
    INSTALL( DIRECTORY  ${VTK_DIR}/tk8.5 DESTINATION  local/sivic/vtk/lib)
    INSTALL( DIRECTORY  ${VTK_DIR}/tcl8.5 DESTINATION  local/sivic/vtk/lib)
ELSE (NOT WIN32)
	INSTALL_TARGETS( /local/sivic sivic )
    INSTALL( DIRECTORY  ${VTK_DIR}/lib/tk8.5 DESTINATION  local/sivic/vtk/lib)
    INSTALL( DIRECTORY  ${VTK_DIR}/lib/tcl8.5 DESTINATION  local/sivic/vtk/lib)
ENDIF (NOT WIN32)

INSTALL_FILES( /local/sivic/dcmtk/lib FILES ${DCMTK_DIR}/share/dcmtk/dicom.dic )
IF(WIN32)
	INSTALL_PROGRAMS( /local/bin FILES sivic.bat )
ELSE (WIN32)
	INSTALL_PROGRAMS( /local/bin FILES sivic )
ENDIF (WIN32)

IF (CMAKE_SYSTEM_NAME MATCHES Darwin)
    SET_TARGET_PROPERTIES (sivic PROPERTIES LINK_FLAGS 
    "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib") 
    SET(VTK_USE_PARALLEL "OFF")
ENDIF (CMAKE_SYSTEM_NAME MATCHES Darwin)

IF (CMAKE_SYSTEM_NAME MATCHES SunOS)
    SET_TARGET_PROPERTIES (sivic PROPERTIES LINK_FLAGS "")
ENDIF (CMAKE_SYSTEM_NAME MATCHES SunOS)
