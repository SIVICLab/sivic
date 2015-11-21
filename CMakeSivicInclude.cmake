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

#SIVIC version variables

SET(GET_REVISION_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/git_revision_tool.pl")

#GET MAJOR REVISION
execute_process(COMMAND ${GET_REVISION_COMMAND} -p 0 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE SVK_MAJOR_VERSION RESULT_VARIABLE MAJOR_RESULT)
if(NOT ${MAJOR_RESULT} EQUAL 0 )
    message( WARNING "WARNING: Could not determine major revision using ${GET_REVISION_COMMAND}. Setting to 0.")
    SET(SVK_MAJOR_VERSION 0)
endif(NOT ${MAJOR_RESULT} EQUAL 0  )

#GET MINOR REVISION
execute_process(COMMAND ${GET_REVISION_COMMAND} -p 1 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE SVK_MINOR_VERSION RESULT_VARIABLE MINOR_RESULT)
if(NOT ${MINOR_RESULT} EQUAL 0 )
    message( WARNING "WARNING: Could not determine minor revision using ${GET_REVISION_COMMAND}. Setting to 0.")
    SET(SVK_MINOR_VERSION 0)
endif(NOT ${MINOR_RESULT} EQUAL 0  )

#GET PATCH REVISION
execute_process(COMMAND ${GET_REVISION_COMMAND} -p 2 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE SVK_PATCH_VERSION RESULT_VARIABLE PATCH_RESULT)
if(NOT ${PATCH_RESULT} EQUAL 0 )
    message( WARNING "WARNING: Could not determine patch revision using ${GET_REVISION_COMMAND}. Setting to 0.")
    SET(SVK_PATCH_VERSION 0)
endif(NOT ${PATCH_RESULT} EQUAL 0  )

SET(SVK_RELEASE_VERSION "${SVK_MAJOR_VERSION}.${SVK_MINOR_VERSION}.${SVK_PATCH_VERSION}")

message("SVK_RELEASE_VERSION = <${SVK_RELEASE_VERSION}>")
