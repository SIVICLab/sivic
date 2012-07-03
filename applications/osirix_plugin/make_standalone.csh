#!/bin/csh

mkdir -p SIVIC.app/Contents/Resources

cp -rp plugin_depends/vtk-5.6           SIVIC.app/Contents/Resources
cp -rp plugin_depends/tk8.5             SIVIC.app/Contents/Resources
cp -rp plugin_depends/tcl8.5            SIVIC.app/Contents/Resources
cp ../sivic_app/Darwin_i386/sivic       SIVIC.app/Contents/Resources
cp sivic.sh                             SIVIC.app/Contents/Resources
cp SIVIC                                SIVIC.app/
cp -rp plugin_depends/libtcl8.5.dylib   SIVIC.app/Contents/Resources
cp -rp plugin_depends/libtk8.5.dylib    SIVIC.app/Contents/Resources
cp standalone/Info.plist                SIVIC.app/Contents
cp tmpicns.rsrc                         SIVIC.app/Contents/Resources

chmod -R 775 SIVIC.app

