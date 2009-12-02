#!/bin/sh

#
#   Copyright © 2009 The Regents of the University of California.
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
#   $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/applications/osirix_plugin/sivic_plugin_post_install.sh $
#   $Rev: 1 $
#   $Author: jccrane $
#   $Date: 2009-11-25 02:04:25 -0800 (Wed, 25 Nov 2009) $
#
#   Authors:
#       Jason C. Crane, Ph.D.
#       Beck Olson
#

#
#   changes internal tck/tk names to point to bundled tcl distribution
#


current=`pwd`
depend_dir="./plugin_depends"
mkdir ${depend_dir} 

./vtk_dependencies.csh

cp -R /usr/local/lib/KWWidgets  ${depend_dir}
cp -R ./vtk-5.4 ${depend_dir}

tcl_system="/Library/Frameworks/Tcl.framework/Versions/8.5/Tcl"
tk_system="/Library/Frameworks/Tk.framework/Versions/8.5/Tk"

plugin_path_tmp="/Library/Application Support/OsiriX/Plugins/SIVIC_MRSI.osirixplugin/Contents/Resources"
plugin_path=`echo ${plugin_path_tmp} | sed 's/ /\\ /'`

tcl_plugin="Tcl.framework/Versions/8.5/Tcl"
tk_plugin="Tk.framework/Versions/8.5/Tk"

set -x
############################
#   sivic:
############################
cd ./build/Deployment/SIVIC_MRSI.bundle/Contents/Resources
install_name_tool -change "${tcl_system}" "${plugin_path}/${tcl_plugin}" sivic
install_name_tool -change "${tk_system}" "${plugin_path}/${tk_plugin}" sivic
cd ${current}


############################
#   kwwidgets
############################
cd "${depend_dir}/KWWidgets"
install_name_tool -change "${tcl_system}" "${plugin_path}/${tcl_plugin}" libKWWidgets.dylib
install_name_tool -change "${tk_system}" "${plugin_path}/${tk_plugin}" libKWWidgets.dylib
cd ${current}

############################
#   vtk
############################
cd "${depend_dir}/vtk-5.4"
for f in ./*.dylib
do
    install_name_tool -change "${tcl_system}" "${plugin_path}/${tcl_plugin}" $f
    install_name_tool -change "${tk_system}" "${plugin_path}/${tk_plugin}" $f
done

