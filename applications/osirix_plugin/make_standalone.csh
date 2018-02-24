#!/bin/csh

if ($#argv != 1) then
    echo
    echo "Usage: $0 horos | osirix"
    echo
    exit(1)
endif
set plugin=$1

if (${plugin} != "osirix" && ${plugin} != "horos") then
    echo
    echo "Usage: $0 horos | osirix"
    echo
    exit(1)
endif

#   Add Resources, e.g for tcl/tk, dicom, icons, X11 libs: 
rm -rf ./SIVIC.app
mkdir -p SIVIC.app/Contents/Resources

cp -rp plugin_depends/tk8.5                                         SIVIC.app/Contents/Resources
cp -rp plugin_depends/tcl8.5                                        SIVIC.app/Contents/Resources
cp ../../working/${plugin}/applications/sivic_app/Darwin_i386/sivic SIVIC.app/Contents/Resources
cp detect_xquartz.scpt                                              SIVIC.app/Contents/Resources
cp sivic.sh                                                         SIVIC.app/Contents/Resources
cp SIVIC                                                            SIVIC.app/SIVIC.sh
cp standalone/Info.plist                                            SIVIC.app/Contents
cp Icons/prism_blue.icns                                            SIVIC.app/Contents/Resources/prism.icns
cp /usr/local/dicom/share/dcmtk/dicom.dic                           SIVIC.app/Contents/Resources/

mkdir -p SIVIC.app/Contents/Resources/X11
cp /opt/X11/lib/libfontconfig.1.dylib                               SIVIC.app/Contents/Resources/X11
cp /opt/X11/lib/libGLU.1.dylib                                      SIVIC.app/Contents/Resources/X11
cp /opt/X11/lib/libGL.1.dylib                                       SIVIC.app/Contents/Resources/X11
cp /opt/X11/lib/libSM.6.dylib                                       SIVIC.app/Contents/Resources/X11
cp /opt/X11/lib/libICE.6.dylib                                      SIVIC.app/Contents/Resources/X11
cp /opt/X11/lib/libX11.6.dylib                                      SIVIC.app/Contents/Resources/X11
cp /opt/X11/lib/libXext.6.dylib                                     SIVIC.app/Contents/Resources/X11
cp /opt/X11/lib/libXt.6.dylib                                       SIVIC.app/Contents/Resources/X11
cp /opt/X11/lib/libXss.1.dylib                                      SIVIC.app/Contents/Resources/X11
cp /opt/X11/lib/libXft.2.dylib                                      SIVIC.app/Contents/Resources/X11
cp /opt/X11/lib/libOSMesa.8.dylib                                   SIVIC.app/Contents/Resources/X11


#   Add standalone tools: 
mkdir -p SIVIC.app/Contents/sivic
mkdir -p SIVIC.app/Contents/sivic/local
mkdir -p SIVIC.app/Contents/sivic/local/bin
mkdir -p SIVIC.app/Contents/sivic/local/sivic
mkdir -p SIVIC.app/Contents/sivic/local/sivic/dcmtk/lib

cp /usr/local/dicom/share/dcmtk/dicom.dic                                               SIVIC.app/Contents/sivic/local/sivic/dcmtk/lib/

cp ../../working/release/applications/cmd_line/Darwin_i386/svk_file_convert             SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_quick_view               SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_multi_view               SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_gepfile_reader           SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_spec_diff                SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_integrate_dynamic        SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_quantify                 SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_reslice                  SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_phase_spec               SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_auto_phase               SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_met_kinetics             SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_fft                      SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_scale_image              SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_hsvd                     SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_point_selector           SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_dcmdump                  SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_image_threshold          SIVIC.app/Contents/sivic/local/sivic/ 
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_interpolate_spectra      SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_gepfile_anon             SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_lcmodel_reader           SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_lcmodel_writer           SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_zscore                   SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_extract_spec             SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_transform                SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_image_mathematics        SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_average_spec             SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_noise                    SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_mrs_combine              SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_apodize                  SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_image_stats              SIVIC.app/Contents/sivic/local/sivic/ 
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_image_pipeline           SIVIC.app/Contents/sivic/local/sivic/ 
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_peak_pick                SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_volume_diff              SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_psd_prescription_convert SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_zerofill                 SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_reorder_epsi             SIVIC.app/Contents/sivic/local/sivic/
cp ../../working/release/applications/cmd_line/Darwin_i386/svk_variable_flip_scaler     SIVIC.app/Contents/sivic/local/sivic/


cp ../../working/release/applications/cmd_line/src/svk_file_convert             SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_quick_view               SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_multi_view               SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_gepfile_reader           SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_spec_diff                SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_integrate_dynamic        SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_quantify                 SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_reslice                  SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_phase_spec               SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_auto_phase               SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_met_kinetics             SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_fft                      SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_scale_image              SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_hsvd                     SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_point_selector           SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_dcmdump                  SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_image_threshold          SIVIC.app/Contents/sivic/local/bin/  
cp ../../working/release/applications/cmd_line/src/svk_interpolate_spectra      SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_gepfile_anon             SIVIC.app/Contents/sivic/local/bin/ 
cp ../../working/release/applications/cmd_line/src/svk_lcmodel_reader           SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_lcmodel_writer           SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_zscore                   SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_extract_spec             SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_transform                SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_image_mathematics        SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_average_spec             SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_noise                    SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_mrs_combine              SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_apodize                  SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_image_stats              SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_image_pipeline           SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_peak_pick                SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_volume_diff              SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_psd_prescription_convert SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_zerofill                 SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_reorder_epsi             SIVIC.app/Contents/sivic/local/bin/
cp ../../working/release/applications/cmd_line/src/svk_variable_flip_scaler     SIVIC.app/Contents/sivic/local/bin/

chmod -R 775 SIVIC.app
rm -rf   ./SIVIC_DISTRIBUTION_${plugin}
mkdir -p ./SIVIC_DISTRIBUTION_${plugin} 
mkdir -p ./SIVIC_DISTRIBUTION_${plugin}/.background
mv ./SIVIC.app                                                                  ./SIVIC_DISTRIBUTION_${plugin}/
cp -r sivic_logo.icns                                                           ./SIVIC_DISTRIBUTION_${plugin}/.background/
cp -r ~/Library/Developer/Xcode/DerivedData/SIVIC_MRSI-gfzhgwmtudqatzemvsdhqbsdogzb/Build/Products/Deployment/SIVIC_MRSI.bundle ./SIVIC_DISTRIBUTION_${plugin}/SIVIC_MRSI.osirixplugin

#   add alias
cp /Applications\ alias                                                         ./SIVIC_DISTRIBUTION_${plugin}/Applications
if (${plugin} == "osirix") then
    cp ~/Library/Application\ Support/OsiriX/Sivic_Plugins\ alias                   ./SIVIC_DISTRIBUTION_${plugin}/Sivic_Plugins
endif
if (${plugin} == "horos") then
    cp ~/Library/Application\ Support/Horos/Horos_Plugins\ alias                    ./SIVIC_DISTRIBUTION_${plugin}/Horos_Plugins
endif

