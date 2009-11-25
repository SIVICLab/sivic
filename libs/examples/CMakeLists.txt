#
#   $URL$
#   $Rev$
#   $Author$
#   $Date$
#

cmake_minimum_required(VERSION 2.6)


PROJECT(testing)

FIND_PACKAGE(KWWidgets REQUIRED)
INCLUDE(${KWWidgets_USE_FILE})


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
    SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
ELSE (CMAKE_SYSTEM_NAME MATCHES Linux)
    IF (CMAKE_SYSTEM_NAME MATCHES Darwin)
        SET(PLATFORM Darwin_i386)
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
    ELSE (CMAKE_SYSTEM_NAME MATCHES Darwin)
        SET(PLATFORM SunOS_sparc)
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
INCLUDE(${CMAKE_ROOT}/Modules/FindVTK.cmake)
IF (USE_VTK_FILE)
    INCLUDE(${USE_VTK_FILE})
ENDIF (USE_VTK_FILE)

ADD_DEFINITIONS(-DHAVE_CONFIG_H) 

LINK_DIRECTORIES(
    ../${PLATFORM}
    ${DCMTK_DIR}/lib
)

INCLUDE_DIRECTORIES(
    ./
    ../src
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
    ${KWWidgets_LIBRARIES}
    z
)



ADD_EXECUTABLE(
    test_read_write 
    test_read_write.cc
)


TARGET_LINK_LIBRARIES(
    test_read_write 
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_mvc
    test_mvc.cc
)

TARGET_LINK_LIBRARIES(
    test_mvc
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_new_del
    test_new_del.cc
)

TARGET_LINK_LIBRARIES(
    test_new_del
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    test_svkImageData
    test_svkImageData.cc
)

TARGET_LINK_LIBRARIES(
    test_svkImageData
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkOrientedImageCopy
    test_svkOrientedImageCopy.cc
)

TARGET_LINK_LIBRARIES(
    test_svkOrientedImageCopy
	${SVK_LIBS}
)

#ADD_EXECUTABLE(
    #test_OverlayError
    #test_OverlayError.cc
#)
#
#TARGET_LINK_LIBRARIES(
    #test_OverlayError
	#${SVK_LIBS}
#)

ADD_EXECUTABLE(
    test_svkImageReaderFactory
    test_svkImageReaderFactory.cc
)

TARGET_LINK_LIBRARIES(
    test_svkImageReaderFactory
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    test_svkPlotGridView 
    test_svkPlotGridView.cc
)


TARGET_LINK_LIBRARIES(
    test_svkPlotGridView 
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkOverlayView 
    test_svkOverlayView.cc
)


TARGET_LINK_LIBRARIES(
    test_svkOverlayView 
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkBoxPlot 
    test_svkBoxPlot.cc
)


TARGET_LINK_LIBRARIES(
    test_svkBoxPlot 
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    test_svkImageTopologyGenerator 
    test_svkImageTopologyGenerator.cc
)


TARGET_LINK_LIBRARIES(
    test_svkImageTopologyGenerator 
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    test_grid_topo_order
    test_grid_topo_order.cc 
)


TARGET_LINK_LIBRARIES(
    test_grid_topo_order
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    test_svkMultiWindowToImageFilter
    test_svkMultiWindowToImageFilter.cc
)


TARGET_LINK_LIBRARIES(
    test_svkMultiWindowToImageFilter
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_vtkRenderLoop
    test_vtkRenderLoop.cc
)


TARGET_LINK_LIBRARIES(
    test_vtkRenderLoop
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_actorSizeRectGrid
    test_actorSizeRectGrid.cc
    ${RC_FILENAME}
)


TARGET_LINK_LIBRARIES(
    test_actorSizeRectGrid
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_actorSizeIndividual
    test_actorSizeIndividual.cc
    ${RC_FILENAME}
)


TARGET_LINK_LIBRARIES(
    test_actorSizeIndividual
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    test_rectilnearGrid
    test_rectilnearGrid.cc
)


TARGET_LINK_LIBRARIES(
    test_rectilnearGrid
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    test_svkDICOMMRSWriter
    test_svkDICOMMRSWriter.cc
)

TARGET_LINK_LIBRARIES(
    test_svkDICOMMRSWriter
    ${SVK_LIBS}
)

#ADD_EXECUTABLE(
    #test_unstructuredImageViewer
    #test_unstructuredImageViewer.cc
#)
#
#TARGET_LINK_LIBRARIES(
    #test_unstructuredImageViewer
    #${SVK_LIBS}
#)

ADD_EXECUTABLE(
    test_svkOrientedImageData
    test_svkOrientedImageData.cc
)

TARGET_LINK_LIBRARIES(
    test_svkOrientedImageData
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkImageViewer2
    test_svkImageViewer2.cc
)

TARGET_LINK_LIBRARIES(
    test_svkImageViewer2
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkOrientedImageActor
    test_svkOrientedImageActor.cc
)

TARGET_LINK_LIBRARIES(
    test_svkOrientedImageActor
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkPhase
    test_svkPhase.cc
)

TARGET_LINK_LIBRARIES(
    test_svkPhase
    ${SVK_LIBS}
)


ADD_EXECUTABLE(
    test_svkIdfWriter
    test_svkIdfWriter.cc
)

TARGET_LINK_LIBRARIES(
    test_svkIdfWriter
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkSpectraPhaserWidget
    test_svkSpectraPhaserWidget.cc
)

TARGET_LINK_LIBRARIES(
    test_svkSpectraPhaserWidget
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkSinglePlotExtraction
    test_svkSinglePlotExtraction.cc
)

TARGET_LINK_LIBRARIES(
    test_svkSinglePlotExtraction
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkDataModel
    test_svkDataModel.cc
)

TARGET_LINK_LIBRARIES(
    test_svkDataModel
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkFormatConverter
    test_svkFormatConverter.cc
)

TARGET_LINK_LIBRARIES(
    test_svkFormatConverter
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkDetailedPlotView
    test_svkDetailedPlotView.cc
)

TARGET_LINK_LIBRARIES(
    test_svkDetailedPlotView
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkImageCopy
    test_svkImageCopy.cc
)

TARGET_LINK_LIBRARIES(
    test_svkImageCopy
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkObliqueReslice
    test_svkObliqueReslice.cc
)

TARGET_LINK_LIBRARIES(
    test_svkObliqueReslice
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkMrsImageFFT
    test_svkMrsImageFFT.cc
)

TARGET_LINK_LIBRARIES(
    test_svkMrsImageFFT
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkMCPhase
    test_svkMCPhase.cc
)

TARGET_LINK_LIBRARIES(
    test_svkMCPhase
    ${SVK_LIBS}
)

