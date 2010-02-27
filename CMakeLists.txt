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

cmake_minimum_required(VERSION 2.4)
option( BUILD_LIBRARY "Build library." TRUE )
option( BUILD_SIVIC "Build stand-alone application." TRUE )
option( BUILD_EXAMPLES "Build examples." FALSE )
option( BUILD_APPS "Build mini-applications." FALSE )
option( BUILD_OSIRIX "Build OsiriX Plugin." FALSE )
option( UCSF_INTERNAL "For UCSF Internal builds only." FALSE )
option( SVK_USE_GL2PS "For exporting vector graphics formats, VTK must be built with VTK_USE_GL2PS." FALSE )

ADD_DEFINITIONS( -DSVK_RELEASE_VERSION='\"0.5.0\"' )
ADD_DEFINITIONS( -DSVK_MAJOR_VERSION='0' )
ADD_DEFINITIONS( -DSVK_MINOR_VERSION='5' )

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

