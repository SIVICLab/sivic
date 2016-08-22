#!/bin/bash

# 
#   Copyright © 2009-2016 The Regents of the University of California.
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
#   $URL$
#   $Rev$
#   $Author$
#   $Date$
# 
#   Authors:
#       Jason C. Crane, Ph.D.
#       Beck Olson
# 


if [ -d '/Applications/SIVIC.app' ]; then
    plugin_path='/Applications/SIVIC.app/Contents/Resources'
elif [ -d '/Library/Application Support/OsiriX/Plugins/SIVIC_MRSI.osirixplugin' ]; then
    plugin_path='/Library/Application Support/OsiriX/Plugins/SIVIC_MRSI.osirixplugin/Contents/Resources'
else
    plugin_path_tmp="/Users/${USER}/Library/Application Support/OsiriX/Plugins/SIVIC_MRSI.osirixplugin/Contents/Resources"
    plugin_path=`echo ${plugin_path_tmp} | sed 's/ /\\ /'`
fi
#plugin_path='/Applications/SIVIC.app/Contents/Resources'
echo ${plugin_path}

DYLD_FRAMEWORK_PATH="${plugin_path}/Frameworks"
export DYLD_FRAMEWORK_PATH

#   add dyld path to DYLD_LIBRARY_PATH environment variable to fiind tcl/tk and X11 components: 
DYLD_LIBRARY_PATH="${plugin_path}:${plugin_path}/tcl8.5/lib:${plugin_path}/tk8.5/lib:${plugin_path}/X11"
export DYLD_LIBRARY_PATH
set | grep  DYLD_FRAMEWORK_PATH
set | grep  DYLD_LIBRARY_PATH

PATH="${plugin_path}:${plugin_path}/tcl8.5/lib:${plugin_path}/tk8.5/lib:${PATH}"
export PATH
set | grep PATH

TCL_LIBRARY="${plugin_path}/tcl8.5/lib/tcl8.5"
export TCL_LIBRARY
set | grep TCL_LIBRARY

TK_LIBRARY="${plugin_path}/tk8.5/lib/tk8.5"
export TK_LIBRARY
set | grep TK_LIBRARY 


DCMDICTPATH="${plugin_path}/dicom.dic"
export DCMDICTPATH

if [ $# == 0 ]; then
    echo "${plugin_path}/sivic"
    "${plugin_path}/sivic"
fi


if [ $# == 2 ]; then
    path1=`echo $1 | sed 's/ /\\ /'`
    path2=`echo $2 | sed 's/ /\\ /'`
    echo "${plugin_path}/sivic" "${path1}" "${path2}"
    "${plugin_path}/sivic" "${path1}" "${path2}"
fi


if [ $# == 4 ]; then
    path1=`echo $1 | sed 's/ /\\ /'`
    path2=`echo $2 | sed 's/ /\\ /'`
    path3=`echo $3 | sed 's/ /\\ /'`
    path4=`echo $4 | sed 's/ /\\ /'`
    echo "${plugin_path}/sivic" "${path1}" "${path2}" "${path3}" "${path4}"
    "${plugin_path}/sivic" "${path1}" "${path2}" "${path3}" "${path4}"
fi

if [ $# == 6 ]; then
    path1=`echo $1 | sed 's/ /\\ /'`
    path2=`echo $2 | sed 's/ /\\ /'`
    path3=`echo $3 | sed 's/ /\\ /'`
    path4=`echo $4 | sed 's/ /\\ /'`
    path3=`echo $5 | sed 's/ /\\ /'`
    path4=`echo $6 | sed 's/ /\\ /'`
    echo "${plugin_path}/sivic" "${path1}" "${path2}" "${path3} ${path4} "${path5}" "${path6}"
    "${plugin_path}/sivic" "${path1}" "${path2}" "${path3} "${path4}" "${path5}" "${path6}"
fi

