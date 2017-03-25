#
#  Copyright © 2009-2017 The Regents of the University of California.
#  All Rights Reserved.
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
#   verify XQuartz is installed
#   osascript detect_xquartz 
#

try
   set appID to "org.x.x11"
   tell application id appID to set APPVersion to version
   #display dialog "Apple X11:" & APPVersion
on error errmsg
   try
       set appID to "org.macosforge.xquartz.X11"
       tell application id appID to set APPVersion to version
       #display dialog "XQuartz :" & APPVersion
   on error errmsg
       set APPVersion to null
   end try
end try

if APPVersion is null then
    display alert "SIVIC INFO" message "XQuartz must be installed to run SIVIC.\n\nIf SIVIC still doesn't run after installing XQuartz, please submit a ticket https://sourceforge.net/projects/sivic/" as critical buttons {"Cancel", "Download XQuartz"} default button "Download XQuartz" 
    set response to button returned of the result 
    if response is "Download XQuartz" then open location "https://www.xquartz.org"
    error 22
else
    #display dialog "XQuartz installed"
end if
