# 
#   Copyright © 2009-2010 The Regents of the University of California.
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

IF(COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0005 OLD)
ENDIF(COMMAND CMAKE_POLICY)

option( BUILD_LIBRARY "Build library." TRUE )
option( BUILD_SIVIC "Build stand-alone application." TRUE )
option( BUILD_EXAMPLES "Build examples." FALSE )
option( BUILD_APPS "Build mini-applications." FALSE )
option( BUILD_OSIRIX "Build OsiriX Plugin." FALSE )
option( UCSF_INTERNAL "For UCSF Internal builds only." FALSE )
option( SVK_USE_GL2PS "For exporting vector graphics formats, VTK must be built with VTK_USE_GL2PS." FALSE )
option( BUILD_PROFILER "Build stand-alone application." FALSE)

ADD_DEFINITIONS( -DSVK_RELEASE_VERSION='\"0.6.6\"' )
ADD_DEFINITIONS( -DSVK_MAJOR_VERSION='0' )
ADD_DEFINITIONS( -DSVK_MINOR_VERSION='6' )

###################################
#   VTK
###################################
FIND_PACKAGE(VTK)
IF(NOT VTK_FOUND)
  MESSAGE(FATAL_ERROR
    "VTK not found. Please set VTK_DIR.")
ENDIF(NOT VTK_FOUND)

###################################
#   KWWidgets
###################################
FIND_PACKAGE(KWWidgets REQUIRED)

###################################
#   DCMTK (DCMTK_DIR is the root include dir
###################################
FIND_PATH(DCMTK_DIR dctk.h)


if( BUILD_PROFILER)
    ADD_DEFINITIONS( -pg )
endif( BUILD_PROFILER )

if( BUILD_OSIRIX)
#    ADD_DEFINITIONS(-DCMAKE_UNIVERSAL=TRUE)
    SET (CMAKE_UNIVERSAL TRUE)
endif (BUILD_OSIRIX)

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

if( BUILD_TESTING)
    #add_subdirectory (tests/src)
endif ( BUILD_TESTING)

