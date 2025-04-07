# Install script for directory: /home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/applications/cmd_line/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/sivic_build")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "None")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_file_convert")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_image_threshold")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_dilate_erode")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_interpolate_spectra")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_quick_view")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_multi_view")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_gepfile_reader")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_variable_flip_scaler")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_gepfile_anon")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_lcmodel_reader")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_lcmodel_writer")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_spec_diff")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_integrate_dynamic")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_quantify")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_zscore")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_extract_spec")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_combine_spec")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_reslice")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_transform")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_dcmdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_point_selector")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_phase_spec")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_fft")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_freq_correct")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_scale_image")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_image_mathematics")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_average_spec")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_noise")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_mrs_combine")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_apodize")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_image_stats")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_image_pipeline")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_peak_pick")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_volume_diff")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_psd_prescription_convert")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_reorder_epsi")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/bin" TYPE PROGRAM FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/src/svk_zerofill")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_file_convert" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_file_convert")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_file_convert"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_file_convert")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_file_convert" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_file_convert")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_file_convert"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_file_convert")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_threshold" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_threshold")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_threshold"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_image_threshold")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_threshold" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_threshold")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_threshold"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_threshold")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_dilate_erode" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_dilate_erode")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_dilate_erode"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_dilate_erode")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_dilate_erode" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_dilate_erode")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_dilate_erode"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_dilate_erode")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_interpolate_spectra" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_interpolate_spectra")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_interpolate_spectra"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_interpolate_spectra")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_interpolate_spectra" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_interpolate_spectra")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_interpolate_spectra"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_interpolate_spectra")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_dcmdump" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_dcmdump")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_dcmdump"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_dcmdump")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_dcmdump" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_dcmdump")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_dcmdump"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_dcmdump")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_quick_view" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_quick_view")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_quick_view"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_quick_view")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_quick_view" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_quick_view")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_quick_view"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_quick_view")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_point_selector" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_point_selector")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_point_selector"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_point_selector")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_point_selector" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_point_selector")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_point_selector"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_point_selector")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_multi_view" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_multi_view")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_multi_view"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_multi_view")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_multi_view" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_multi_view")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_multi_view"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_multi_view")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_gepfile_reader" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_gepfile_reader")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_gepfile_reader"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_gepfile_reader")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_gepfile_reader" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_gepfile_reader")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_gepfile_reader"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_gepfile_reader")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_variable_flip_scaler" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_variable_flip_scaler")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_variable_flip_scaler"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_variable_flip_scaler")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_variable_flip_scaler" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_variable_flip_scaler")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_variable_flip_scaler"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_variable_flip_scaler")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_lcmodel_writer" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_lcmodel_writer")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_lcmodel_writer"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_lcmodel_writer")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_lcmodel_writer" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_lcmodel_writer")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_lcmodel_writer"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_lcmodel_writer")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_lcmodel_reader" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_lcmodel_reader")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_lcmodel_reader"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_lcmodel_reader")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_lcmodel_reader" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_lcmodel_reader")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_lcmodel_reader"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_lcmodel_reader")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_gepfile_anon" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_gepfile_anon")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_gepfile_anon"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_gepfile_anon")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_gepfile_anon" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_gepfile_anon")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_gepfile_anon"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_gepfile_anon")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_spec_diff" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_spec_diff")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_spec_diff"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_spec_diff")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_spec_diff" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_spec_diff")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_spec_diff"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_spec_diff")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_integrate_dynamic" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_integrate_dynamic")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_integrate_dynamic"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_integrate_dynamic")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_integrate_dynamic" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_integrate_dynamic")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_integrate_dynamic"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_integrate_dynamic")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_quantify" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_quantify")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_quantify"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_quantify")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_quantify" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_quantify")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_quantify"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_quantify")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_reslice" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_reslice")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_reslice"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_reslice")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_reslice" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_reslice")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_reslice"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_reslice")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_fft" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_fft")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_fft"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_fft")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_fft" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_fft")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_fft"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_fft")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_noise" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_noise")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_noise"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_noise")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_noise" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_noise")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_noise"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_noise")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_peak_pick" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_peak_pick")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_peak_pick"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_peak_pick")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_peak_pick" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_peak_pick")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_peak_pick"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_peak_pick")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_reorder_epsi" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_reorder_epsi")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_reorder_epsi"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_reorder_epsi")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_reorder_epsi" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_reorder_epsi")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_reorder_epsi"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_reorder_epsi")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_phase_spec" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_phase_spec")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_phase_spec"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_phase_spec")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_phase_spec" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_phase_spec")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_phase_spec"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_phase_spec")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_freq_correct" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_freq_correct")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_freq_correct"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_freq_correct")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_freq_correct" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_freq_correct")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_freq_correct"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_freq_correct")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_mathematics" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_mathematics")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_mathematics"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_image_mathematics")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_mathematics" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_mathematics")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_mathematics"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_mathematics")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_zscore" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_zscore")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_zscore"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_zscore")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_zscore" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_zscore")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_zscore"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_zscore")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_extract_spec" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_extract_spec")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_extract_spec"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_extract_spec")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_extract_spec" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_extract_spec")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_extract_spec"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_extract_spec")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_combine_spec" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_combine_spec")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_combine_spec"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_combine_spec")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_combine_spec" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_combine_spec")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_combine_spec"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_combine_spec")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_scale_image" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_scale_image")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_scale_image"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_scale_image")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_scale_image" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_scale_image")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_scale_image"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_scale_image")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_volume_diff" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_volume_diff")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_volume_diff"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_volume_diff")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_volume_diff" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_volume_diff")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_volume_diff"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_volume_diff")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_average_spec" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_average_spec")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_average_spec"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_average_spec")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_average_spec" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_average_spec")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_average_spec"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_average_spec")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_mrs_combine" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_mrs_combine")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_mrs_combine"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_mrs_combine")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_mrs_combine" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_mrs_combine")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_mrs_combine"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_mrs_combine")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_apodize" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_apodize")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_apodize"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_apodize")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_apodize" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_apodize")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_apodize"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_apodize")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_zerofill" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_zerofill")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_zerofill"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_zerofill")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_zerofill" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_zerofill")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_zerofill"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_zerofill")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_psd_prescription_convert" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_psd_prescription_convert")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_psd_prescription_convert"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_psd_prescription_convert")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_psd_prescription_convert" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_psd_prescription_convert")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_psd_prescription_convert"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_psd_prescription_convert")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_stats" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_stats")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_stats"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_image_stats")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_stats" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_stats")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_stats"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_stats")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_pipeline" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_pipeline")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_pipeline"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_image_pipeline")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_pipeline" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_pipeline")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_pipeline"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_image_pipeline")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_transform" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_transform")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_transform"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/sivic" TYPE EXECUTABLE FILES "/home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3/applications/cmd_line/Linux_x86_64/svk_transform")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_transform" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_transform")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_transform"
         OLD_RPATH "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3/../..:../Linux_x86_64:../../../libs/Linux_x86_64:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib:/mnt/nfs/rad/apps/netopt/versions/dcmtk/dcmtk-3.6.7-devel/lib64:/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/sivic/svk_transform")
    endif()
  endif()
endif()

