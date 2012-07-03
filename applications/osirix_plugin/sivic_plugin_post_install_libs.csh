#!/bin/csh

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
#   Authors:
#       Jason C. Crane, Ph.D.
#       Beck Olson
#

#   
#   Post installation script for plugin in users home directory, required because packagemaker doesn't expand "~" but 
#   creates a literal ~ directory. 
#

set plugin_name="SIVIC_MRSI.osirixplugin"
if (-d /tmp/vtk-5.6) then 
    mv /tmp/vtk-5.6 /Users/${USER}/Library/Application\ Support/OsiriX/Plugins/SIVIC_MRSI.osirixplugin/Contents/Resources/
endif
if (-d /tmp/KWWidgets) then 
    mv /tmp/KWWidgets /Users/${USER}/Library/Application\ Support/OsiriX/Plugins/SIVIC_MRSI.osirixplugin/Contents/Resources/
endif
