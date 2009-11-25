#
#   $URL: https://intrarad.ucsf.edu/svn/rad_software/surbeck/brain/sivic/trunk/applications/slicer_plugin/MRSpectroscopy/CMakeLists.txt $
#   $Rev: 15111 $
#   $Author: jasonc@RADIOLOGY.UCSF.EDU $
#   $Date: 2009-09-28 22:20:25 -0700 (Mon, 28 Sep 2009) $
#

PROJECT(SIVIC_MRS)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Find Slicer3

SET(Slicer3_DIR /Users/jasonc/slicer3_2_build/Slicer3-build)

if(NOT Slicer3_SOURCE_DIR)
    find_package(Slicer3 REQUIRED)
    include(${Slicer3_USE_FILE})
    slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)


# --------------------------------------------------------------------------
# Include dirs

ADD_DEFINITIONS(-DHAVE_CONFIG_H -DMRK_ADAPT_DCMTK -D_DARWIN_C_SOURCE)

set(PATHROOT /usr/local)

include_directories(
    /Users/jasonc/svn/surbeck/brain/visualization/trunk/libs/src
    ${PATHROOT}/share/include
    ${PATHROOT}/share/include/svk
    ${PATHROOT}/share/include/raw
    ${PATHROOT}/share/include/csi
    ${PATHROOT}/share/include/mrsc
    ${PATHROOT}/share/include/util
    ${PATHROOT}/share/include/mrsc/internal
    ${PATHROOT}/share/include/dicom/dev
    ${PATHROOT}/dicom/include/dcmtk/dcmdata
    ${PATHROOT}/dicom/include/dcmtk/ofstd
    ${PATHROOT}/../include/dcmtk/dcmdata
    ${PATHROOT}/../include/dcmtk/ofstd
    ${PATHROOT}/../include/dcmdata
    ${PATHROOT}/../include/ofstd
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Slicer3_Libs_INCLUDE_DIRS}
    ${Slicer3_Base_INCLUDE_DIRS}
)


#
# Configure include file
#
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/vtkMRSpectroscopyConfigure.h.in 
    ${CMAKE_CURRENT_BINARY_DIR}/vtkMRSpectroscopyConfigure.h
  )

file(GLOB headers 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  )

install(FILES 
    ${headers} 
    "${CMAKE_CURRENT_BINARY_DIR}/vtkMRSpectroscopyConfigure.h"
    DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(MRSpectroscopy_SRCS 
    vtkMRSpectroscopyLogic.cxx
    vtkMRSpectroscopyGUI.cxx
)


# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(MRSpectroscopy MRSpectroscopy_TCL_SRCS "${MRSpectroscopy_SRCS}" "")



#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(MRSpectroscopy_SRCS MRSpectroscopy.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name MRSpectroscopy)
add_library(${lib_name}
    ${MRSpectroscopy_SRCS} 
    ${MRSpectroscopy_TCL_SRCS}
  )

slicer3_set_modules_output_path(${lib_name})

IF (CMAKE_SYSTEM_NAME MATCHES Darwin)
    SET_TARGET_PROPERTIES (MRSpectroscopy PROPERTIES LINK_FLAGS "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
ENDIF (CMAKE_SYSTEM_NAME MATCHES Darwin)


target_link_libraries(${lib_name}
    ${PATHROOT}/lib/local/libutil.a
/Users/jasonc/svn/surbeck/brain/visualization/trunk/libs/Darwin_i386/libsvk.a
/Users/jasonc/svn/surbeck/brain/visualization/trunk/libs/Darwin_i386/libsvkadapt.a
#    ${PATHROOT}/lib/local/libsvk.a	
#    ${PATHROOT}/lib/local/libsvkadapt.a	
    ${PATHROOT}/lib/local/dev/libdicom.a
    /usr/lib/libdcmdata.a
    /usr/lib/libofstd.a
    ${PATHROOT}/lib/local/libcsi.a
    ${PATHROOT}/lib/local/libmrsc.a
    ${PATHROOT}/lib/local/libgnumath.a
    /usr/lib/libz.a
    ${Slicer3_Libs_LIBRARIES}
    ${Slicer3_Base_LIBRARIES}
    ${KWWidgets_LIBRARIES}
 )

install(TARGETS ${lib_name}
    RUNTIME DESTINATION ${Slicer3_INSTALL_MODULES_BIN_DIR} COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT Development
  )

slicer3_install_modules(${lib_name})
