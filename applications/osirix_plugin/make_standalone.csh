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


#   Add standalone tools: 
mkdir -p SIVIC.app/Contents/sivic
mkdir -p SIVIC.app/Contents/sivic/local
mkdir -p SIVIC.app/Contents/sivic/local/bin
mkdir -p SIVIC.app/Contents/sivic/local/sivic
mkdir -p SIVIC.app/Contents/sivic/local/sivic/dcmtk/lib

cp /usr/local/dicom/share/dcmtk/dicom.dic   SIVIC.app/Contents/sivic/local/sivic/dcmtk/lib/

cp ../cmd_line/Darwin_i386/svk_file_convert         SIVIC.app/Contents/sivic/local/sivic/
cp ../cmd_line/Darwin_i386/svk_quick_view           SIVIC.app/Contents/sivic/local/sivic/
cp ../cmd_line/Darwin_i386/svk_multi_view           SIVIC.app/Contents/sivic/local/sivic/
cp ../cmd_line/Darwin_i386/svk_gepfile_reader       SIVIC.app/Contents/sivic/local/sivic/
cp ../cmd_line/Darwin_i386/svk_spec_diff            SIVIC.app/Contents/sivic/local/sivic/
cp ../cmd_line/Darwin_i386/svk_integrate_dynamic    SIVIC.app/Contents/sivic/local/sivic/
cp ../cmd_line/Darwin_i386/svk_quantify             SIVIC.app/Contents/sivic/local/sivic/
cp ../cmd_line/Darwin_i386/svk_reslice              SIVIC.app/Contents/sivic/local/sivic/
cp ../cmd_line/Darwin_i386/svk_phase_spec           SIVIC.app/Contents/sivic/local/sivic/
cp ../cmd_line/Darwin_i386/svk_auto_phase           SIVIC.app/Contents/sivic/local/sivic/
cp ../cmd_line/Darwin_i386/svk_met_kinetics         SIVIC.app/Contents/sivic/local/sivic/
cp ../cmd_line/Darwin_i386/svk_fft                  SIVIC.app/Contents/sivic/local/sivic/
cp ../cmd_line/Darwin_i386/svk_scale_image          SIVIC.app/Contents/sivic/local/sivic/
cp ../cmd_line/Darwin_i386/svk_hsvd                 SIVIC.app/Contents/sivic/local/sivic/
cp ../cmd_line/Darwin_i386/svk_point_selector       SIVIC.app/Contents/sivic/local/sivic/
cp ../cmd_line/Darwin_i386/svk_dcmdump              SIVIC.app/Contents/sivic/local/sivic/

cp ../cmd_line/src/svk_file_convert         SIVIC.app/Contents/sivic/local/bin/
cp ../cmd_line/src/svk_quick_view           SIVIC.app/Contents/sivic/local/bin/
cp ../cmd_line/src/svk_multi_view           SIVIC.app/Contents/sivic/local/bin/
cp ../cmd_line/src/svk_gepfile_reader       SIVIC.app/Contents/sivic/local/bin/
cp ../cmd_line/src/svk_spec_diff            SIVIC.app/Contents/sivic/local/bin/
cp ../cmd_line/src/svk_integrate_dynamic    SIVIC.app/Contents/sivic/local/bin/
cp ../cmd_line/src/svk_quantify             SIVIC.app/Contents/sivic/local/bin/
cp ../cmd_line/src/svk_reslice              SIVIC.app/Contents/sivic/local/bin/
cp ../cmd_line/src/svk_phase_spec           SIVIC.app/Contents/sivic/local/bin/
cp ../cmd_line/src/svk_auto_phase           SIVIC.app/Contents/sivic/local/bin/
cp ../cmd_line/src/svk_met_kinetics         SIVIC.app/Contents/sivic/local/bin/
cp ../cmd_line/src/svk_fft                  SIVIC.app/Contents/sivic/local/bin/
cp ../cmd_line/src/svk_scale_image          SIVIC.app/Contents/sivic/local/bin/
cp ../cmd_line/src/svk_hsvd                 SIVIC.app/Contents/sivic/local/bin/
cp ../cmd_line/src/svk_point_selector       SIVIC.app/Contents/sivic/local/bin/
cp ../cmd_line/src/svk_dcmdump              SIVIC.app/Contents/sivic/local/bin/


chmod -R 775 SIVIC.app

