cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0028 OLD)    # Suppress warnings for double-colon targets
project(svk_applications)

# ------------------------------------------------------------------------------
# MANUALLY SPECIFY X11 AND OPENGL INCLUDE/LIB PATHS
# ------------------------------------------------------------------------------
set(X11_INCLUDE_DIR "/usr/include/X11")
set(X11_LIB_DIR     "/usr/lib64")
include_directories("${X11_INCLUDE_DIR}")
link_directories("${X11_LIB_DIR}")

# Adjust if Mesa/OpenGL is in non-default locations
set(OPENGL_INCLUDE_DIR "/usr/include/GL")
set(OPENGL_LIB_DIR     "/usr/lib64")
include_directories("${OPENGL_INCLUDE_DIR}")
link_directories("${OPENGL_LIB_DIR}")

# ------------------------------------------------------------------------------
# If you want CMake to define OpenGL::GL / X11::X11, uncomment these:
#find_package(OpenGL REQUIRED)
#find_package(X11 REQUIRED)

# ------------------------------------------------------------------------------
# Set Build Directories and Platform-specific variables
# ------------------------------------------------------------------------------
if(CMAKE_SYSTEM_NAME MATCHES Linux)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES i686)
        set(PLATFORM Linux_i686)
        set(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        set(LIBRARY_OUTPUT_PATH ../${PLATFORM})
    else()
        set(PLATFORM Linux_x86_64)
        set(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        set(LIBRARY_OUTPUT_PATH ../${PLATFORM})
    endif()

    # Using old-school library names:
    set(PLATFORM_LIBS
        z
        GLU     # typically separate for GLU
        GL      # -lGL
        SM
        ICE
        X11     # -lX11
        Xext
        Xt
        Xss
        Xft
    )
elseif(CMAKE_SYSTEM_NAME MATCHES Darwin)
    set(PLATFORM Darwin_i386)
    set(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
    set(PLATFORM_LIBS z)
    set(LIBRARY_OUTPUT_PATH ../${PLATFORM})
    if(CMAKE_UNIVERSAL MATCHES TRUE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDarwin -arch i386 -arch x86_64")
    endif()
elseif(CMAKE_SYSTEM_NAME MATCHES SunOS)
    add_definitions(-D__GNU_LIBRARY__)
    set(PLATFORM SunOS_sparc)
    set(GETOPT_LIB CACHE FILEPATH "Library containing getopt_long for Solaris (if not using glibc).")
    set(PLATFORM_LIBS z ${GETOPT_LIB})
    set(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
    set(LIBRARY_OUTPUT_PATH ../${PLATFORM})
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-xO3 -DNDEBUG")
    set(CMAKE_CXX_LINK_FLAGS_RELEASE "-xO3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-xO3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS "-xO3 -DNDEBUG")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}_debug)
    set(LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH}_debug)
    add_definitions(-DDEBUG_BUILD)
    set(PLATFORM ${PLATFORM}_debug)
endif()

set(CMAKE_VERBOSE_MAKEFILE TRUE)

if(DEFINED UCSF_INTERNAL)
    add_definitions(-DUCSF_INTERNAL)
endif()

# ------------------------------------------------------------------------------
# DCMTK Settings (if applicable)
# ------------------------------------------------------------------------------
set(DCMTK_OFLOG "")
if(EXISTS "${DCMTK_DIR}/lib/liboflog.a" OR EXISTS "${DCMTK_DIR}/lib/oflog.lib")
    set(DCMTK_OFLOG oflog)
endif()
if(EXISTS "${DCMTK_DIR}/lib64/liboflog.a" OR EXISTS "${DCMTK_DIR}/lib64/oflog.lib")
    set(DCMTK_OFLOG oflog)
endif()

if(DEFINED DCMTK_DIR)
    if(EXISTS "${DCMTK_DIR}/lib")
        link_directories(${DCMTK_DIR}/lib)
    elseif(EXISTS "${DCMTK_DIR}/lib64")
        link_directories(${DCMTK_DIR}/lib64)
    endif()
endif()

# ------------------------------------------------------------------------------
# VTK Configuration
# ------------------------------------------------------------------------------
set(VTK_DIR "/mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/lib64/cmake/vtk-9.3")
find_package(VTK 9.3.0 REQUIRED)

# If VTK sets "OpenGL::GLX" or "OpenGL::OpenGL" or "X11::X11" in VTK_LIBRARIES
# and you see link errors or warnings, remove them:
list(REMOVE_ITEM VTK_LIBRARIES "OpenGL::GLX" "OpenGL::OpenGL" "X11::X11")

# (Optional) If you *do* need to link them explicitly:
# list(APPEND VTK_LIBRARIES GL X11)

# ------------------------------------------------------------------------------
# Additional Third-Party Packages (ITK, CLAPACK, etc.)
# ------------------------------------------------------------------------------
if(BUILD_ITK)
    add_definitions(-DITK_BUILD)
    find_package(ITK REQUIRED)
    if(ITK_FOUND)
        include(${ITK_USE_FILE})
    else()
        message(FATAL_ERROR "Cannot build without ITK. Please set ITK_DIR.")
    endif()
    link_directories(${ITK_DIR}/lib)
endif()

if(BUILD_CLAPACK)
    if(WIN32)
        link_directories(${CLAPACK_DIR}/lib)
    else()
        link_directories(${CLAPACK_DIR})
    endif()
endif()

if(WIN32)
    set(PLATFORM_INCLUDE ../../../libs/getopt ${OPENSSL_DIR}/include)
endif()

include_directories(
    ./
    ../../../libs/src/
    ${DCMTK_DIR}/include
    ${PLATFORM_INCLUDE}
)
if(BUILD_ITK)
    include_directories(${ITK_DIR}/include)
endif()
if(BUILD_CLAPACK)
    include_directories(${CLAPACK_DIR}/INCLUDE)
endif()

if(WIN32)
    set(PLATFORM_LIBS getopt netapi32)
endif()

if(BUILD_ITK)
    set(ITK_LIBS
        svkNumerical
        ITKDICOMParser
    )
    set(ITK_LIBS_PRE
        ITKOptimizers
        ITKCommon
        ITKDICOMParser
    )
endif()

if(BUILD_CLAPACK)
    if(WIN32)
        set(CLAPACK_LIBS
            svkNumerical2
            lapack
            libf2c.lib
            lapack
            blas
        )
    else()
        set(CLAPACK_LIBS
            svkNumerical2
            ${CLAPACK_DIR}/libclapack.a
            ${CLAPACK_DIR}/libf2c.a
            ${CLAPACK_DIR}/libclapack.a
            ${CLAPACK_DIR}/libcblas.a
        )
    endif()
endif()

# ------------------------------------------------------------------------------
# SVK Libraries
# ------------------------------------------------------------------------------
if(CMAKE_SYSTEM_NAME MATCHES Darwin)
    set(SVK_LIBS
        ${ITK_LIBS}
        ${CLAPACK_LIBS}
        svkPSD
        svk
        svkTypeUtils
        svkadapt
        svkXMLUtils
        ${VTK_LIBRARIES}
        dcmdata
        ${DCMTK_OFLOG}
        ofstd
        ${ITK_LIBS_PRE}
        GLU
        GL
        SM
        ICE
        X11
        Xext
        Xt
        Xss
        Xft
    )
else()
    # On Linux, we link the "classic" library names for OpenGL/X11:
    set(SVK_LIBS
        ${ITK_LIBS}
        ${CLAPACK_LIBS}
        svkPSD
        svk
        svkTypeUtils
        svkadapt
        svkXMLUtils
        ${VTK_LIBRARIES}
        dcmdata
        ${DCMTK_OFLOG}
        ofstd
        ${ITK_LIBS_PRE}
        ${PLATFORM_LIBS}   # includes GL, X11, etc.
    )
endif()

set(SVK_INFOVIS_LIBS "")

if(CMAKE_SYSTEM_NAME MATCHES Darwin)
    set(SVK_VIZ_LIBS
        svkviz
        OSMesa
    )
    set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -framework OpenGL")
else()
    set(SVK_VIZ_LIBS svkviz)
endif()

# ------------------------------------------------------------------------------
# Define Executable Targets
# ------------------------------------------------------------------------------
add_executable(svk_file_convert svk_file_convert.cc)
target_link_libraries(svk_file_convert ${SVK_LIBS})

add_executable(svk_print_image_info svk_print_image_info.cc)
target_link_libraries(svk_print_image_info ${SVK_LIBS})

add_executable(svk_create_secondary_capture svk_create_secondary_capture.cc)
target_link_libraries(svk_create_secondary_capture ${SVK_LIBS})

add_executable(svk_image_threshold svk_image_threshold.cc)
target_link_libraries(svk_image_threshold ${SVK_LIBS})

add_executable(svk_dilate_erode svk_dilate_erode.cc)
target_link_libraries(svk_dilate_erode ${SVK_LIBS})

add_executable(svk_interpolate_spectra svk_interpolate_spectra.cc)
target_link_libraries(svk_interpolate_spectra ${SVK_LIBS})

add_executable(svk_dcmdump svk_dcmdump.cc)
target_link_libraries(svk_dcmdump ${SVK_LIBS})

add_executable(svk_quick_view svk_quick_view.cc)
target_link_libraries(svk_quick_view ${SVK_VIZ_LIBS} ${SVK_LIBS})

add_executable(svk_point_selector svk_point_selector.cc)
target_link_libraries(svk_point_selector ${SVK_VIZ_LIBS} ${SVK_LIBS})

add_executable(svk_multi_view svk_multi_view.cc)
target_link_libraries(svk_multi_view ${SVK_VIZ_LIBS} ${SVK_LIBS})

add_executable(svk_gepfile_reader svk_gepfile_reader.cc)
target_link_libraries(svk_gepfile_reader ${SVK_LIBS})

add_executable(svk_variable_flip_scaler svk_variable_flip_scaler.cc)
target_link_libraries(svk_variable_flip_scaler ${SVK_LIBS})

add_executable(svk_lcmodel_writer svk_lcmodel_writer.cc)
target_link_libraries(svk_lcmodel_writer ${SVK_LIBS})

add_executable(svk_lcmodel_reader svk_lcmodel_reader.cc)
target_link_libraries(svk_lcmodel_reader ${SVK_LIBS})

add_executable(svk_gepfile_anon svk_gepfile_anon.cc)
target_link_libraries(svk_gepfile_anon ${SVK_LIBS})

add_executable(svk_get_filetype svk_get_filetype.cc)
target_link_libraries(svk_get_filetype ${SVK_LIBS})

add_executable(svk_spec_diff svk_spec_diff.cc)
target_link_libraries(svk_spec_diff ${SVK_LIBS})

add_executable(svk_integrate_dynamic svk_integrate_dynamic.cc)
target_link_libraries(svk_integrate_dynamic ${SVK_LIBS})

add_executable(svk_quantify svk_quantify.cc)
target_link_libraries(svk_quantify ${SVK_LIBS})

add_executable(svk_dce_quantify svk_dce_quantify.cc)
target_link_libraries(svk_dce_quantify ${SVK_LIBS})

add_executable(svk_reslice svk_reslice.cc)
target_link_libraries(svk_reslice ${SVK_LIBS})

add_executable(svk_svrecon svk_svrecon.cc)
target_link_libraries(svk_svrecon ${SVK_LIBS})

add_executable(svk_surface_area svk_surface_area.cc)
target_link_libraries(svk_surface_area ${SVK_VIZ_LIBS} ${SVK_LIBS})

add_executable(svk_fft svk_fft.cc)
target_link_libraries(svk_fft ${SVK_LIBS})

add_executable(svk_noise svk_noise.cc)
target_link_libraries(svk_noise ${SVK_LIBS})

add_executable(svk_peak_pick svk_peak_pick.cc)
target_link_libraries(svk_peak_pick ${SVK_LIBS})

add_executable(svk_reorder_epsi svk_reorder_epsi.cc)
target_link_libraries(svk_reorder_epsi ${SVK_LIBS})

add_executable(svk_phase_spec svk_phase_spec.cc)
target_link_libraries(svk_phase_spec ${SVK_LIBS})

add_executable(svk_freq_correct svk_freq_correct.cc)
target_link_libraries(svk_freq_correct ${SVK_LIBS})

add_executable(svk_dcm_deid svk_dcm_deid.cc)
target_link_libraries(svk_dcm_deid ${SVK_LIBS})

add_executable(svk_image_mathematics svk_image_mathematics.cc)
target_link_libraries(svk_image_mathematics ${SVK_LIBS})

add_executable(svk_zscore svk_zscore.cc)
target_link_libraries(svk_zscore ${SVK_LIBS})

add_executable(svk_extract_spec svk_extract_spec.cc)
target_link_libraries(svk_extract_spec ${SVK_LIBS})

add_executable(svk_combine_spec svk_combine_spec.cc)
target_link_libraries(svk_combine_spec ${SVK_LIBS})

add_executable(svk_scale_image svk_scale_image.cc)
target_link_libraries(svk_scale_image ${SVK_LIBS})

add_executable(svk_volume_diff svk_volume_diff.cc)
target_link_libraries(svk_volume_diff ${SVK_LIBS})

add_executable(svk_average_spec svk_average_spec.cc)
target_link_libraries(svk_average_spec ${SVK_LIBS})

add_executable(svk_mrs_combine svk_mrs_combine.cc)
target_link_libraries(svk_mrs_combine ${SVK_LIBS})

add_executable(svk_apodize svk_apodize.cc)
target_link_libraries(svk_apodize ${SVK_LIBS})

add_executable(svk_simulate svk_simulate.cc)
target_link_libraries(svk_simulate ${SVK_LIBS})

add_executable(svk_zerofill svk_zerofill.cc)
target_link_libraries(svk_zerofill ${SVK_LIBS})

add_executable(svk_channel2time svk_channel2time.cc)
target_link_libraries(svk_channel2time ${SVK_LIBS})

add_executable(svk_psd_prescription_convert svk_psd_prescription_convert.cc)
target_link_libraries(svk_psd_prescription_convert ${SVK_LIBS})

add_executable(svk_image_stats svk_image_stats.cc)
target_link_libraries(svk_image_stats ${SVK_LIBS} ${SVK_INFOVIS_LIBS})

add_executable(svk_image_pipeline svk_image_pipeline.cc)
target_link_libraries(svk_image_pipeline ${SVK_LIBS} ${SVK_INFOVIS_LIBS})

add_executable(svk_transform svk_transform.cc)
target_link_libraries(svk_transform ${SVK_LIBS})

if(BUILD_ITK)
    add_executable(svk_met_kinetics svk_met_kinetics.cc)
    target_link_libraries(svk_met_kinetics ${SVK_LIBS})

    add_executable(svk_auto_phase svk_auto_phase.cc)
    target_link_libraries(svk_auto_phase ${SVK_LIBS})
endif()

if(BUILD_CLAPACK)
    add_executable(svk_hsvd svk_hsvd.cc)
    target_link_libraries(svk_hsvd ${SVK_LIBS})
endif()

add_executable(svk_app_template svk_app_template.cc)
target_link_libraries(svk_app_template ${SVK_LIBS})

if(BUILD_CREATE_RAW)
    if(CMAKE_SYSTEM_NAME MATCHES Linux)
        add_executable(svk_create_dcmraw svk_create_dcmraw.cc)
        target_link_libraries(svk_create_dcmraw ${SVK_LIBS} crypto)
    else()
        add_executable(svk_create_dcmraw svk_create_dcmraw.cc)
        target_link_libraries(svk_create_dcmraw ${SVK_LIBS} ssleay32 libeay32)
    endif()
endif()

set(APPS
    svk_file_convert
    svk_image_threshold
    svk_dilate_erode
    svk_interpolate_spectra
    svk_quick_view
    svk_multi_view
    svk_gepfile_reader
    svk_variable_flip_scaler
    svk_gepfile_anon
    svk_lcmodel_reader
    svk_lcmodel_writer
    svk_spec_diff
    svk_integrate_dynamic
    svk_quantify
    svk_zscore
    svk_extract_spec
    svk_combine_spec
    svk_reslice
    svk_transform
    svk_dcmdump
    svk_point_selector
    svk_phase_spec
    svk_fft
    svk_freq_correct
    svk_scale_image
    svk_image_mathematics
    svk_average_spec
    svk_noise
    svk_mrs_combine
    svk_apodize
    svk_image_stats
    svk_image_pipeline
    svk_peak_pick
    svk_volume_diff
    svk_psd_prescription_convert
    svk_reorder_epsi
    svk_zerofill
)
if(BUILD_CLAPACK)
    list(APPEND APPS svk_hsvd)
endif()
if(BUILD_ITK)
    list(APPEND APPS svk_auto_phase)
    list(APPEND APPS svk_met_kinetics)
endif()

if(CREATE_WRAPPER_SCRIPTS)
    if(NOT WIN32)
        foreach(APP ${APPS})
            configure_file(svk_cmd_start ./${APP} COPYONLY)
            install(TARGETS ${APP} DESTINATION /local/sivic)
            install(PROGRAMS ${APP} DESTINATION /local/bin)
        endforeach()
    else()
        foreach(APP ${APPS})
            configure_file(svk_cmd_start.bat ${APP}.bat COPYONLY)
            install(TARGETS ${APP} DESTINATION /local/sivic)
            install(PROGRAMS ${APP}.bat DESTINATION /local/bin)
        endforeach()
    endif()
endif()

# ------------------------------------------------------------------------------
# MacOS / Darwin fixups
# ------------------------------------------------------------------------------
if(CMAKE_SYSTEM_NAME MATCHES Darwin)
    set_target_properties(svk_surface_area PROPERTIES LINK_FLAGS
        "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
    set_target_properties(svk_file_convert PROPERTIES LINK_FLAGS
        "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
    set_target_properties(svk_quick_view PROPERTIES LINK_FLAGS
        "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
    set_target_properties(svk_gepfile_reader PROPERTIES LINK_FLAGS
        "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
    set(VTK_USE_PARALLEL "OFF")
endif()

if(CMAKE_SYSTEM_NAME MATCHES SunOS)
    set_target_properties(svk_file_convert PROPERTIES LINK_FLAGS "")
    set_target_properties(svk_gepfile_reader PROPERTIES LINK_FLAGS "")
endif()
