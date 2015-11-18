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

#######################################################
# CPack variables.
#######################################################

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "sivic built using CMake")
SET(CPACK_PACKAGE_FILE_NAME "sivic_${SVK_RELEASE_VERSION}_${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "sivic ${SVK_RELEASE_VERSION}")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "sivic ${SVK_RELEASE_VERSION}")
SET(CPACK_PACKAGE_NAME "sivic")
SET(CPACK_PACKAGE_RELOCATABLE "true")
IF (WIN32)
	SET(CPACK_GENERATOR "ZIP;NSIS")
	SET(CPACK_NSIS_MENU_LINKS "local/bin/sivic.bat" "sivic" )
	SET(CPACK_PACKAGE_EXECUTABLES "sivic" "sivic")
ELSE (WIN32)
    IF( BUILD_GE_CONSOLE )
        SET(CPACK_PACKAGE_FILE_NAME "sivic_GE_console_${SVK_RELEASE_VERSION}_${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}")
	    SET(CPACK_GENERATOR "TGZ")
    ELSE ( BUILD_GE_CONSOLE )
	    SET(CPACK_GENERATOR "STGZ;TGZ;DEB")
    ENDIF( BUILD_GE_CONSOLE )
ENDIF (WIN32)
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "UCSF Nelson Laboratory")
IF (${CMAKE_SYSTEM_PROCESSOR} MATCHES x86_64 )
    SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
ELSE (${CMAKE_SYSTEM_PROCESSOR} MATCHES x86_64 )
    SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386)
ENDIF (${CMAKE_SYSTEM_PROCESSOR} MATCHES x86_64 )
    
SET(CPACK_RPM_PACKAGE_RELOCATABLE TRUE)
SET(CPACK_NSIS_DISPLAY_NAME "sivic ${SVK_RELEASE_VERSION}")
SET(CPACK_PACKAGE_VERSION "${SVK_RELEASE_VERSION}")
SET(CPACK_PACKAGE_VERSION_MAJOR "${SVK_MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${SVK_MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${SVK_PATCH_VERSION}")
SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_HOME_DIRECTORY}/LICENSE)
SET(CPACK_RESOURCE_FILE_README ${CMAKE_HOME_DIRECTORY}/README)
