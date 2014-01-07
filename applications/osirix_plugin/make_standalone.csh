#!/bin/csh

mkdir -p SIVIC.app/Contents/Resources

cp -rp plugin_depends/tk8.5                 SIVIC.app/Contents/Resources
cp -rp plugin_depends/tcl8.5                SIVIC.app/Contents/Resources
cp ../sivic_app/Darwin_i386/sivic           SIVIC.app/Contents/Resources
cp sivic.sh                                 SIVIC.app/Contents/Resources
cp SIVIC                                    SIVIC.app/
cp standalone/Info.plist                    SIVIC.app/Contents
cp Icons/prism_blue.icns                    SIVIC.app/Contents/Resources/prism.icns
cp /usr/local/dicom/share/dcmtk/dicom.dic   SIVIC.app/Contents/Resources/

chmod -R 775 SIVIC.app

