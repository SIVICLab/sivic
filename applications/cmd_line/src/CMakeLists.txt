#
#   Copyright © 2009-2014 The Regents of the University of California.
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

cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


PROJECT(svk_applications)



#
#   Set build dir:
#
IF (CMAKE_SYSTEM_NAME MATCHES Linux)
    if(COMMAND cmake_policy)
        cmake_policy(SET CMP0015 OLD)
    endif(COMMAND cmake_policy)
    IF (CMAKE_SYSTEM_PROCESSOR MATCHES i686)
        SET(PLATFORM Linux_i686)
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
    ELSE (CMAKE_SYSTEM_PROCESSOR MATCHES i686)
        SET(PLATFORM Linux_x86_64)
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
    ENDIF (CMAKE_SYSTEM_PROCESSOR MATCHES i686)
	SET(PLATFORM_LIBS z)
ELSE (CMAKE_SYSTEM_NAME MATCHES Linux)
    IF (CMAKE_SYSTEM_NAME MATCHES Darwin)
        SET(PLATFORM Darwin_i386)
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(PLATFORM_LIBS z )
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
        IF (CMAKE_UNIVERSAL MATCHES TRUE)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDarwin -arch i386 -arch x86_64")
        ENDIF (CMAKE_UNIVERSAL MATCHES TRUE)
    ENDIF (CMAKE_SYSTEM_NAME MATCHES Darwin)
    IF (CMAKE_SYSTEM_NAME MATCHES SunOS)
        ADD_DEFINITIONS(-D__GNU_LIBRARY__) 
        SET(PLATFORM SunOS_sparc)
        SET(GETOPT_LIB CACHE FILEPATH "Library containing getop_long for solaris (if you are not using glibc)." )
        SET(PLATFORM_LIBS z ${GETOPT_LIB})
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
        SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "-xO3 -DNDEBUG")
        SET(CMAKE_CXX_LINK_FLAGS_RELEASE "-xO3 -DNDEBUG")
        SET(CMAKE_CXX_FLAGS_RELEASE "-xO3 -DNDEBUG")
        SET(CMAKE_CXX_FLAGS "-xO3 -DNDEBUG")
    ENDIF (CMAKE_SYSTEM_NAME MATCHES SunOS)
ENDIF (CMAKE_SYSTEM_NAME MATCHES Linux)


IF (CMAKE_BUILD_TYPE MATCHES Debug)
    SET(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}_debug)
    SET(LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH}_debug)
    ADD_DEFINITIONS(-DDEBUG_BUILD) 
    SET(PLATFORM ${PLATFORM}_debug)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)


SET(CMAKE_VERBOSE_MAKEFILE TRUE)

IF (UCSF_INTERNAL)
    ADD_DEFINITIONS(-DUCSF_INTERNAL)
ENDIF(UCSF_INTERNAL)


# if dcmtk 3.6, need to add liboflog:
SET(DCMTK_OFLOG "")
if ( EXISTS "${DCMTK_DIR}/lib/liboflog.a" OR EXISTS "${DCMTK_DIR}/lib/oflog.lib")
    SET(DCMTK_OFLOG oflog )
endif (EXISTS "${DCMTK_DIR}/lib/liboflog.a" OR EXISTS "${DCMTK_DIR}/lib/oflog.lib")

###################################
#   VTK
###################################
FIND_PACKAGE(VTK REQUIRED)

IF(VTK_FOUND)
  INCLUDE(${VTK_USE_FILE})
ENDIF(VTK_FOUND)


###################################
#   KWWidgets
###################################
INCLUDE(${KWWidgets_USE_FILE})


if( BUILD_PROFILER)
    ADD_DEFINITIONS( -pg )
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pg ")
endif( BUILD_PROFILER )


###################################
#   DCMTK (DCMTK_DIR is the root include dir
###################################
IF( NOT WIN32 )
	ADD_DEFINITIONS(-DHAVE_CONFIG_H)
ENDIF( NOT WIN32 )



#############################################
#    Build Test Application
#############################################

LINK_DIRECTORIES(
	${LIBRARY_OUTPUT_PATH}
    ../../../libs/${PLATFORM}
    ../${PLATFORM}
    ${DCMTK_DIR}/lib
    ${OPENSSL_DIR}/lib
)


if ( BUILD_ITK )

    ###################################
    #   ITK 
    ###################################
    ADD_DEFINITIONS(-DITK_BUILD)
    FIND_PACKAGE( ITK REQUIRED )
    IF(ITK_FOUND)
        INCLUDE(${ITK_USE_FILE})
    ELSE(ITK_FOUND)
        MESSAGE(FATAL_ERROR "Cannot build without ITK.  Please set ITK_DIR.")
    ENDIF(ITK_FOUND)

    LINK_DIRECTORIES( ${ITK_DIR}/lib )

endif( BUILD_ITK )


if ( BUILD_CLAPACK )
    IF(WIN32)
    	LINK_DIRECTORIES( ${CLAPACK_DIR}/lib )
    ELSE(WIN32)
    	LINK_DIRECTORIES( ${CLAPACK_DIR} )
    ENDIF(WIN32)
endif( BUILD_CLAPACK )


IF(WIN32)
	SET(PLATFORM_INCLUDE ../../../libs/getopt ${OPENSSL_DIR}/include)
ENDIF(WIN32)
INCLUDE_DIRECTORIES(
    ./
    ../../../libs/src/
    ${DCMTK_DIR}/include
    ${PLATFORM_INCLUDE}
)
if ( BUILD_ITK )
    INCLUDE_DIRECTORIES( ${ITK_DIR}/include )
endif( BUILD_ITK )

if ( BUILD_CLAPACK )
    INCLUDE_DIRECTORIES( ${CLAPACK_DIR}/INCLUDE)
endif( BUILD_CLAPACK )

IF(WIN32)
	SET(PLATFORM_LIBS getopt
	                  netapi32)
ENDIF(WIN32)

if ( BUILD_ITK )
    SET( ITK_LIBS 
        svkNumerical 
        #cminpack 
        ITKDICOMParser
    )
    SET( ITK_LIBS_PRE
        ITKOptimizers
        ITKCommon
        ITKDICOMParser
    ) 
endif( BUILD_ITK )

if ( BUILD_CLAPACK )
    INCLUDE_DIRECTORIES( ${CLAPACK_DIR}/INCLUDE)
    IF(WIN32)
        SET( CLAPACK_LIBS 
            svkNumerical2 
            lapack
            libf2c.lib
            lapack
            blas
        )
    ELSE(WIN32)
        SET( CLAPACK_LIBS 
            svkNumerical2 
            ${CLAPACK_DIR}/libclapack.a
            ${CLAPACK_DIR}/libf2c.a 
            ${CLAPACK_DIR}/libclapack.a
            ${CLAPACK_DIR}/libcblas.a 
        )
    ENDIF(WIN32)
endif( BUILD_CLAPACK )

SET(SVK_LIBS
    ${ITK_LIBS}
    ${CLAPACK_LIBS}
    svkPSD
    svk 
    svkPSD
    svkTypeUtils
    svkadapt 
    vtkImaging
    vtkIO
    vtkFiltering
    vtkCommon
    vtkInfovis
    dcmdata
    ${DCMTK_OFLOG}
    ofstd 
    ${PLATFORM_LIBS}
    ${ITK_LIBS_PRE}
)

SET(SVK_VIZ_LIBS
    svkviz
    vtkRendering
    vtkVolumeRendering
    vtkGraphics
    vtkHybrid
    vtkInfovis
)




ADD_EXECUTABLE(
    svk_file_convert
    svk_file_convert.cc
)

TARGET_LINK_LIBRARIES(
    svk_file_convert 
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_image_threshold
    svk_image_threshold.cc
)

TARGET_LINK_LIBRARIES(
    svk_image_threshold 
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_interpolate_spectra
    svk_interpolate_spectra.cc
)

TARGET_LINK_LIBRARIES(
    svk_interpolate_spectra 
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_dcmdump
    svk_dcmdump.cc
)

TARGET_LINK_LIBRARIES(
    svk_dcmdump 
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_quick_view
    svk_quick_view.cc
)
TARGET_LINK_LIBRARIES(
    svk_quick_view 
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_point_selector
    svk_point_selector.cc
)
TARGET_LINK_LIBRARIES(
    svk_point_selector 
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_multi_view
    svk_multi_view.cc
)
TARGET_LINK_LIBRARIES(
    svk_multi_view 
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_gepfile_reader
    svk_gepfile_reader.cc 
)
TARGET_LINK_LIBRARIES(
    svk_gepfile_reader
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_lcmodel
    svk_lcmodel.cc
)
TARGET_LINK_LIBRARIES(
    svk_lcmodel
	${SVK_LIBS}
)
ADD_EXECUTABLE(
    svk_lcmodel_reader
    svk_lcmodel_reader.cc
)
TARGET_LINK_LIBRARIES(
    svk_lcmodel_reader
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_gepfile_anon
    svk_gepfile_anon.cc 
)
TARGET_LINK_LIBRARIES(
    svk_gepfile_anon
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    svk_get_filetype
    svk_get_filetype.cc 
)
TARGET_LINK_LIBRARIES(
    svk_get_filetype
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_spec_diff
    svk_spec_diff.cc 
)
TARGET_LINK_LIBRARIES(
    svk_spec_diff
    ${SVK_LIBS}
)


ADD_EXECUTABLE(
    svk_integrate_dynamic
    svk_integrate_dynamic.cc
)
TARGET_LINK_LIBRARIES(
    svk_integrate_dynamic
    ${SVK_LIBS}
)


ADD_EXECUTABLE(
    svk_quantify
    svk_quantify.cc
)
TARGET_LINK_LIBRARIES(
    svk_quantify
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_reslice
    svk_reslice.cc
)
TARGET_LINK_LIBRARIES(
    svk_reslice
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_svrecon
    svk_svrecon.cc
)
TARGET_LINK_LIBRARIES(
    svk_svrecon
    ${SVK_LIBS}
)


ADD_EXECUTABLE(
    svk_surface_area
    svk_surface_area.cc
)
TARGET_LINK_LIBRARIES(
    svk_surface_area
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    svk_fft
    svk_fft.cc
)
TARGET_LINK_LIBRARIES(
    svk_fft
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    svk_noise
    svk_noise.cc
)
TARGET_LINK_LIBRARIES(
    svk_noise
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    svk_peak_pick
    svk_peak_pick.cc
)
TARGET_LINK_LIBRARIES(
    svk_peak_pick
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    svk_reorder_epsi
    svk_reorder_epsi.cc
)
TARGET_LINK_LIBRARIES(
    svk_reorder_epsi
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_phase_spec
    svk_phase_spec.cc
)
TARGET_LINK_LIBRARIES(
    svk_phase_spec
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_dcm_deid   
    svk_dcm_deid.cc
)
TARGET_LINK_LIBRARIES(
    svk_dcm_deid 
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    svk_scale_image
    svk_scale_image.cc
)
TARGET_LINK_LIBRARIES(
    svk_scale_image
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    svk_volume_diff
    svk_volume_diff.cc
)
TARGET_LINK_LIBRARIES(
    svk_volume_diff
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    svk_average_spec
    svk_average_spec.cc
)
TARGET_LINK_LIBRARIES(
    svk_average_spec
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    svk_mrs_combine
    svk_mrs_combine.cc
)
TARGET_LINK_LIBRARIES(
    svk_mrs_combine
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_apodize
    svk_apodize.cc
)
TARGET_LINK_LIBRARIES(
    svk_apodize
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_channel2time
    svk_channel2time.cc
)
TARGET_LINK_LIBRARIES(
    svk_channel2time
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_psd_prescription_convert
    svk_psd_prescription_convert.cc
)
TARGET_LINK_LIBRARIES(
    svk_psd_prescription_convert
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_image_stats
    svk_image_stats.cc
)
TARGET_LINK_LIBRARIES(
    svk_image_stats
	${SVK_LIBS}
    vtkInfovis
)

ADD_EXECUTABLE(
    svk_image_pipeline
    svk_image_pipeline.cc
)
TARGET_LINK_LIBRARIES(
    svk_image_pipeline
	${SVK_LIBS}
    vtkInfovis
)

if ( BUILD_ITK )

    ADD_EXECUTABLE(
        svk_met_kinetics
        svk_met_kinetics.cc
    )
    TARGET_LINK_LIBRARIES(
        svk_met_kinetics
        ${SVK_LIBS}
    )

    ADD_EXECUTABLE(
        svk_auto_phase 
        svk_auto_phase.cc 
    )
    TARGET_LINK_LIBRARIES(
        svk_auto_phase
        ${SVK_LIBS}
    )
 
endif ( BUILD_ITK )

if ( BUILD_CLAPACK )
    ADD_EXECUTABLE(
        svk_hsvd
        svk_hsvd.cc
    )
    TARGET_LINK_LIBRARIES(
        svk_hsvd
	    ${SVK_LIBS}
    )
endif ( BUILD_CLAPACK )


#   Your app here (set ADD_EXECUTABLE, TARGET_LINK_LIBRARIES)
ADD_EXECUTABLE(
    svk_app_template
    svk_app_template.cc
)
TARGET_LINK_LIBRARIES(
    svk_app_template
    ${SVK_LIBS}
)


IF( BUILD_CREATE_RAW )

    #   libcrypto doesn't exist on solaris or windows
    IF (CMAKE_SYSTEM_NAME MATCHES Linux)
        ADD_EXECUTABLE(
            svk_create_dcmraw
            svk_create_dcmraw.cc
        )
        TARGET_LINK_LIBRARIES(
            svk_create_dcmraw
            ${SVK_LIBS}
            crypto 
        )
    ELSE (CMAKE_SYSTEM_NAME MATCHES Linux)
        # Assume Windows 32
        ADD_EXECUTABLE(
            svk_create_dcmraw
            svk_create_dcmraw.cc
        )
        TARGET_LINK_LIBRARIES(
            svk_create_dcmraw
            ${SVK_LIBS}
            ssleay32
            libeay32

        )
    ENDIF (CMAKE_SYSTEM_NAME MATCHES Linux)
ENDIF ( BUILD_CREATE_RAW )

SET(APPS 
    svk_file_convert 
    svk_image_threshold 
    svk_interpolate_spectra
    svk_quick_view
    svk_multi_view
    svk_gepfile_reader 
    svk_lcmodel
    svk_spec_diff 
    svk_integrate_dynamic 
    svk_quantify 
    svk_reslice 
    svk_dcmdump 
    #svk_surface_area 
    svk_point_selector 
    svk_phase_spec 
    svk_fft
    svk_scale_image
    svk_average_spec
    svk_noise
    svk_mrs_combine
    svk_apodize
    svk_image_stats 
    svk_image_pipeline 
    svk_peak_pick
    svk_volume_diff
    svk_psd_prescription_convert
)
if ( BUILD_CLAPACK )
    LIST(APPEND APPS svk_hsvd)
endif ( BUILD_CLAPACK )
if ( BUILD_ITK )
    LIST(APPEND APPS svk_auto_phase)
    LIST(APPEND APPS svk_met_kinetics)
endif ( BUILD_ITK )

IF (NOT WIN32)	
    FOREACH(APP ${APPS})    
        configure_file( svk_cmd_start ./${APP} [COPYONLY])
        INSTALL_TARGETS( /local/sivic ${APP} )
        INSTALL_PROGRAMS( /local/bin FILES ${APP} )
    ENDFOREACH(APP)
ELSE (NOT WIN32)
    FOREACH(APP ${APPS})    
        configure_file( svk_cmd_start.bat ${APP}.bat [COPYONLY])
        INSTALL_TARGETS( /local/sivic ${APP} )
        INSTALL_PROGRAMS( /local/bin FILES ${APP}.bat )
    ENDFOREACH(APP)
ENDIF (NOT WIN32)



IF (CMAKE_SYSTEM_NAME MATCHES Darwin)
    SET_TARGET_PROPERTIES (svk_surface_area PROPERTIES LINK_FLAGS 
    "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib") 
    SET_TARGET_PROPERTIES (svk_file_convert PROPERTIES LINK_FLAGS 
    "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib") 
    SET_TARGET_PROPERTIES (svk_quick_view PROPERTIES LINK_FLAGS 
    "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib") 
    SET_TARGET_PROPERTIES (svk_gepfile_reader PROPERTIES LINK_FLAGS 
    "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib") 
    SET(VTK_USE_PARALLEL "OFF")
ENDIF (CMAKE_SYSTEM_NAME MATCHES Darwin)

IF (CMAKE_SYSTEM_NAME MATCHES SunOS)
    SET_TARGET_PROPERTIES (svk_file_convert PROPERTIES LINK_FLAGS "")
    SET_TARGET_PROPERTIES (svk_gepfile_reader PROPERTIES LINK_FLAGS "")
ENDIF (CMAKE_SYSTEM_NAME MATCHES SunOS)
