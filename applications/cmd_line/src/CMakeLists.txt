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

cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


PROJECT(svk_applications)



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
	SET(PLATFORM_LIBS z)
ELSE (CMAKE_SYSTEM_NAME MATCHES Linux)
    IF (CMAKE_SYSTEM_NAME MATCHES Darwin)
        SET(PLATFORM Darwin_i386)
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
        IF (CMAKE_UNIVERSAL MATCHES TRUE)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDarwin -arch i386 -arch x86_64")
        ENDIF (CMAKE_UNIVERSAL MATCHES TRUE)
    ENDIF (CMAKE_SYSTEM_NAME MATCHES Darwin)
    IF (CMAKE_SYSTEM_NAME MATCHES SunOS)
        ADD_DEFINITIONS(-D__GNU_LIBRARY__) 
        SET(PLATFORM SunOS_sparc)
        SET(GETOPT_LIB CACHE FILEPATH "Library containing getop_long for solaris (if you are not using glibc)." )
        SET(PLATFORM_LIBS z ${GETOPT_LIB})
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
        SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "-xO3 -DNDEBUG")

        SET(CMAKE_CXX_LINK_FLAGS_RELEASE "-xO3 -DNDEBUG")
        SET(CMAKE_CXX_FLAGS_RELEASE "-xO3 -DNDEBUG")
        SET(CMAKE_CXX_FLAGS "-xO3 -DNDEBUG")
    ENDIF (CMAKE_SYSTEM_NAME MATCHES SunOS)
ENDIF (CMAKE_SYSTEM_NAME MATCHES Linux)


IF (CMAKE_BUILD_TYPE MATCHES Debug)
    SET(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}_debug)
    SET(LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH}_debug)
    ADD_DEFINITIONS(-DDEBUG_BUILD) 
    SET(PLATFORM ${PLATFORM}_debug)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)


SET(CMAKE_VERBOSE_MAKEFILE TRUE)

IF (UCSF_INTERNAL)
    ADD_DEFINITIONS(-DUCSF_INTERNAL)
ENDIF(UCSF_INTERNAL)


INCLUDE(${CMAKE_ROOT}/Modules/FindOpenGL.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/FindGLU.cmake)

###################################
#   VTK
###################################
IF(VTK_FOUND)
  INCLUDE(${VTK_USE_FILE})
ENDIF(VTK_FOUND)


###################################
#   KWWidgets
###################################
INCLUDE(${KWWidgets_USE_FILE})


###################################
#   DCMTK (DCMTK_DIR is the root include dir
###################################
IF( NOT WIN32 )
	ADD_DEFINITIONS(-DHAVE_CONFIG_H)
ENDIF( NOT WIN32 )



#############################################
#    Build Test Application
#############################################

LINK_DIRECTORIES(
	${LIBRARY_OUTPUT_PATH}
    ../../../libs/${PLATFORM}
    ../${PLATFORM}
    ${DCMTK_DIR}/lib
)
IF(WIN32)
	SET(PLATFORM_INCLUDE getopt)
ENDIF(WIN32)
INCLUDE_DIRECTORIES(
    ./
    ../../../libs/src/
    ${DCMTK_DIR}/include
    ${PLATFORM_INCLUDE}
)
IF(WIN32)
	ADD_LIBRARY( getopt
			     STATIC
				 getopt/getopt.c
				 getopt/getopt_long.c
			    )
	SET(PLATFORM_LIBS getopt
	                  netapi32)
ENDIF(WIN32)

SET(SVK_LIBS
    vtkRendering
    vtkGraphics
    vtkHybrid
    vtkImaging
    vtkIO
    vtkFiltering
    vtkCommon
    vtkVolumeRendering
    svkadapt 
    svk 
    svkadapt 
    dcmdata
    ofstd 
    ${PLATFORM_LIBS}
)



ADD_EXECUTABLE(
    svk_file_convert
    svk_file_convert.cc
)

TARGET_LINK_LIBRARIES(
    svk_file_convert 
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_dcmdump
    svk_dcmdump.cc
)

TARGET_LINK_LIBRARIES(
    svk_dcmdump 
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_quick_view
    svk_quick_view.cc
)

TARGET_LINK_LIBRARIES(
    svk_quick_view 
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_multi_view
    svk_multi_view.cc
)

TARGET_LINK_LIBRARIES(
    svk_multi_view 
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_gepfile_reader
    svk_gepfile_reader.cc 
)

TARGET_LINK_LIBRARIES(
    svk_gepfile_reader
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_spec_diff
    svk_spec_diff.cc 
)
 
TARGET_LINK_LIBRARIES(
    svk_spec_diff
    ${SVK_LIBS}
)


ADD_EXECUTABLE(
    svk_integrate_dynamic
    svk_integrate_dynamic.cc
)
 
TARGET_LINK_LIBRARIES(
    svk_integrate_dynamic
    ${SVK_LIBS}
)


ADD_EXECUTABLE(
    svk_quantify
    svk_quantify.cc
)
 
TARGET_LINK_LIBRARIES(
    svk_quantify
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_reslice
    svk_reslice.cc
)
 
TARGET_LINK_LIBRARIES(
    svk_reslice
    ${SVK_LIBS}
)

#   Your app here (set ADD_EXECUTABLE, TARGET_LINK_LIBRARIES)
ADD_EXECUTABLE(
    svk_app_template
    svk_app_template.cc
)
 
TARGET_LINK_LIBRARIES(
    svk_app_template
    ${SVK_LIBS}
)

IF (!WIN32)	
	INSTALL_PROGRAMS( /local/bin FILES ../${EXECUTABLE_OUTPUT_PATH}/svk_file_convert )
	INSTALL_PROGRAMS( /local/bin FILES ../${EXECUTABLE_OUTPUT_PATH}/svk_quick_view )
	INSTALL_PROGRAMS( /local/bin FILES ../${EXECUTABLE_OUTPUT_PATH}/svk_multi_view )
	INSTALL_PROGRAMS( /local/bin FILES ../${EXECUTABLE_OUTPUT_PATH}/svk_gepfile_reader )
	INSTALL_PROGRAMS( /local/bin FILES ../${EXECUTABLE_OUTPUT_PATH}/svk_spec_diff )
	INSTALL_PROGRAMS( /local/bin FILES ../${EXECUTABLE_OUTPUT_PATH}/svk_integrate_dynamic )
	INSTALL_PROGRAMS( /local/bin FILES ../${EXECUTABLE_OUTPUT_PATH}/svk_quantify )
	INSTALL_PROGRAMS( /local/bin FILES ../${EXECUTABLE_OUTPUT_PATH}/svk_reslice )
ELSE (!WIN32)
	INSTALL_TARGETS( /local/bin svk_file_convert )
	INSTALL_TARGETS( /local/bin svk_quick_view )
	INSTALL_TARGETS( /local/bin svk_multi_view )
	INSTALL_TARGETS( /local/bin svk_gepfile_reader )
	INSTALL_TARGETS( /local/bin svk_spec_diff )
	INSTALL_TARGETS( /local/bin svk_integrate_dynamic )
	INSTALL_TARGETS( /local/bin svk_quantify )
	INSTALL_TARGETS( /local/bin svk_reslice )
ENDIF (!WIN32)



IF (CMAKE_SYSTEM_NAME MATCHES Darwin)
    SET_TARGET_PROPERTIES (svk_file_convert PROPERTIES LINK_FLAGS 
    "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib") 
    SET_TARGET_PROPERTIES (svk_quick_view PROPERTIES LINK_FLAGS 
    "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib") 
    SET_TARGET_PROPERTIES (svk_gepfile_reader PROPERTIES LINK_FLAGS 
    "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib") 
    SET(VTK_USE_PARALLEL "OFF")
ENDIF (CMAKE_SYSTEM_NAME MATCHES Darwin)

IF (CMAKE_SYSTEM_NAME MATCHES SunOS)
    SET_TARGET_PROPERTIES (svk_file_convert PROPERTIES LINK_FLAGS "")
    SET_TARGET_PROPERTIES (svk_gepfile_reader PROPERTIES LINK_FLAGS "")
ENDIF (CMAKE_SYSTEM_NAME MATCHES SunOS)
