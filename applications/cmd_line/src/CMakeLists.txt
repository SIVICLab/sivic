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
    IF (CMAKE_SYSTEM_PROCESSOR MATCHES i686)
        SET(PLATFORM Linux_i686)
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
    ELSE (CMAKE_SYSTEM_PROCESSOR MATCHES i686)
        SET(PLATFORM Linux_x86_64)
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
    ENDIF (CMAKE_SYSTEM_PROCESSOR MATCHES i686)
ELSE (CMAKE_SYSTEM_NAME MATCHES Linux)
    IF (CMAKE_SYSTEM_NAME MATCHES Darwin)
        SET(PLATFORM Darwin_i386)
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
        IF (CMAKE_UNIVERSAL MATCHES TRUE)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDarwin -arch i386 -arch x86_64")
        ENDIF (CMAKE_UNIVERSAL MATCHES TRUE)
    ELSE (CMAKE_SYSTEM_NAME MATCHES Darwin)
        ADD_DEFINITIONS(-D__GNU_LIBRARY__) 
        SET(PLATFORM SunOS_sparc)
        SET(GETOPT_LIB CACHE FILEPATH "Library containing getop_long for solaris (if you are not using glibc)." )
        SET(PLATFORM_LIBS ${GETOPT_LIB})
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
        SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "-xO3 -DNDEBUG")

        SET(CMAKE_CXX_LINK_FLAGS_RELEASE "-xO3 -DNDEBUG")
        SET(CMAKE_CXX_FLAGS_RELEASE "-xO3 -DNDEBUG")
        SET(CMAKE_CXX_FLAGS "-xO3 -DNDEBUG")
    ENDIF (CMAKE_SYSTEM_NAME MATCHES Darwin)
ENDIF (CMAKE_SYSTEM_NAME MATCHES Linux)


IF (CMAKE_BUILD_TYPE MATCHES Debug)
    SET(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}_debug)
    SET(LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH}_debug)
    ADD_DEFINITIONS(-DDEBUG_BUILD) 
    SET(PLATFORM ${PLATFORM}_debug)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)


SET(CMAKE_VERBOSE_MAKEFILE TRUE)


INCLUDE(${CMAKE_ROOT}/Modules/FindOpenGL.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/FindGLU.cmake)

###################################
#   VTK
###################################
IF(VTK_FOUND)
  INCLUDE(${VTK_USE_FILE})
ENDIF(VTK_FOUND)


###################################
#   KWWidgets
###################################
INCLUDE(${KWWidgets_USE_FILE})


###################################
#   DCMTK (DCMTK_DIR is the root include dir
###################################
ADD_DEFINITIONS(-DHAVE_CONFIG_H)




#############################################
#    Build Test Application
#############################################

LINK_DIRECTORIES(
    ../../../libs/${PLATFORM}
    ../${PLATFORM}
    ${DCMTK_DIR}/lib
)

INCLUDE_DIRECTORIES(
    ./
    ../../../libs/src/
    ${DCMTK_DIR}/include
)


SET(SVK_LIBS
    vtkRendering
    vtkGraphics
    vtkHybrid
    vtkImaging
    vtkIO
    vtkFiltering
    vtkCommon
    vtkVolumeRendering
    svkadapt 
    svk 
    svkadapt 
    dcmdata
    ofstd 
    z
    ${PLATFORM_LIBS}
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
    svk_quick_view
    svk_quick_view.cc
)

TARGET_LINK_LIBRARIES(
    svk_quick_view 
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    svk_multi_view
    svk_multi_view.cc
)

TARGET_LINK_LIBRARIES(
    svk_multi_view 
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



IF (CMAKE_SYSTEM_NAME MATCHES Darwin)
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
