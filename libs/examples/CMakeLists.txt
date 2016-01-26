#
#   Copyright © 2009-2010 The Regents of the University of California.
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


cmake_minimum_required(VERSION 2.6)


PROJECT(testing)



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
    SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
ELSE (CMAKE_SYSTEM_NAME MATCHES Linux)
    IF (CMAKE_SYSTEM_NAME MATCHES Darwin)
        SET(PLATFORM Darwin_i386)
        SET(EXECUTABLE_OUTPUT_PATH ../${PLATFORM})
        SET(LIBRARY_OUTPUT_PATH ../${PLATFORM})
    ENDIF (CMAKE_SYSTEM_NAME MATCHES Darwin)
    IF(CMAKE_SYSTEM_NAME MATCHES SunOS)   
        SET(PLATFORM SunOS_sparc)
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

INCLUDE(${CMAKE_ROOT}/Modules/FindOpenGL.cmake)

###################################
#   VTK
###################################
#INCLUDE(${CMAKE_ROOT}/Modules/FindVTK.cmake)
#IF (USE_VTK_FILE)
#    INCLUDE(${USE_VTK_FILE})
#ENDIF (USE_VTK_FILE)

###################################
#   Windows/Unix options
###################################
IF( NOT CMAKE_SYSTEM_NAME MATCHES Windows)
	ADD_DEFINITIONS(-DHAVE_CONFIG_H)
    SET( PLATFORM_LIBS "z" )
ELSE( NOT CMAKE_SYSTEM_NAME MATCHES Windows)
    SET(PLATFORM_LIBS getopt
	                  netapi32)
	SET(PLATFORM_INCLUDE ../getopt)
ENDIF( NOT CMAKE_SYSTEM_NAME MATCHES Windows)

# if dcmtk 3.6, need to add liboflog:
SET(DCMTK_OFLOG "")
if ( EXISTS "${DCMTK_DIR}/lib/liboflog.a" OR EXISTS "${DCMTK_DIR}/lib/oflog.lib")
    SET(DCMTK_OFLOG oflog )
endif (EXISTS "${DCMTK_DIR}/lib/liboflog.a" OR EXISTS "${DCMTK_DIR}/lib/oflog.lib")



LINK_DIRECTORIES(
    ../${PLATFORM}
    ${DCMTK_DIR}/lib
)

INCLUDE_DIRECTORIES(
    ./
    ../src
    ${PLATFORM_INCLUDE}
    ${DCMTK_DIR}/include
)

SET(SVK_LIBS
    vtkImaging
    vtkIO
    vtkFiltering
    vtkCommon
    svk 
    svkadapt 
    vtkHybrid
    dcmdata
    ${DCMTK_OFLOG}
    ofstd 
    ${PLATFORM_LIBS}    
)


SET(SVK_VIZ_LIBS
    svkviz
    vtkRendering
    vtkVolumeRendering
    vtkGraphics
    vtkHybrid
)


ADD_EXECUTABLE(
    test_read_write 
    test_read_write.cc
)


TARGET_LINK_LIBRARIES(
    test_read_write 
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_mvc
    test_mvc.cc
)

TARGET_LINK_LIBRARIES(
    test_mvc
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_new_del
    test_new_del.cc
)

TARGET_LINK_LIBRARIES(
    test_new_del
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    test_svkImageData
    test_svkImageData.cc
)

TARGET_LINK_LIBRARIES(
    test_svkImageData
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkOrientedImageCopy
    test_svkOrientedImageCopy.cc
)

TARGET_LINK_LIBRARIES(
    test_svkOrientedImageCopy
    ${SVK_VIZ_LIBS}
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
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    test_svkBoxPlot 
    test_svkBoxPlot.cc
)


TARGET_LINK_LIBRARIES(
    test_svkBoxPlot 
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkPlotLine 
    test_svkPlotLine.cc
)


TARGET_LINK_LIBRARIES(
    test_svkPlotLine 
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    test_svkImageTopologyGenerator 
    test_svkImageTopologyGenerator.cc
)


TARGET_LINK_LIBRARIES(
    test_svkImageTopologyGenerator 
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    test_grid_topo_order
    test_grid_topo_order.cc 
)


TARGET_LINK_LIBRARIES(
    test_grid_topo_order
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    test_svkMultiWindowToImageFilter
    test_svkMultiWindowToImageFilter.cc
)


TARGET_LINK_LIBRARIES(
    test_svkMultiWindowToImageFilter
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_vtkRenderLoop
    test_vtkRenderLoop.cc
)


TARGET_LINK_LIBRARIES(
    test_vtkRenderLoop
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)

IF (SVK_USE_GL2PS)
 
    ADD_EXECUTABLE(
        test_vtkGL2PSExporter
        test_vtkGL2PSExporter.cc
    )


    TARGET_LINK_LIBRARIES(
        test_vtkGL2PSExporter
        ${SVK_VIZ_LIBS}
        ${SVK_LIBS}
    )

    ADD_EXECUTABLE(
        test_vtkGL2PSExporterImages
        test_vtkGL2PSExporterImages.cc
    )


    TARGET_LINK_LIBRARIES(
        test_vtkGL2PSExporterImages
        ${SVK_VIZ_LIBS}
        ${SVK_LIBS}
    )
ENDIF(SVK_USE_GL2PS)

ADD_EXECUTABLE(
    test_actorSizeRectGrid
    test_actorSizeRectGrid.cc
    ${RC_FILENAME}
)


TARGET_LINK_LIBRARIES(
    test_actorSizeRectGrid
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_actorSizeIndividual
    test_actorSizeIndividual.cc
    ${RC_FILENAME}
)


TARGET_LINK_LIBRARIES(
    test_actorSizeIndividual
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    test_rectilnearGrid
    test_rectilnearGrid.cc
)


TARGET_LINK_LIBRARIES(
    test_rectilnearGrid
    ${SVK_VIZ_LIBS}
	${SVK_LIBS}
)


ADD_EXECUTABLE(
    test_svkDICOMMRSWriter
    test_svkDICOMMRSWriter.cc
)

TARGET_LINK_LIBRARIES(
    test_svkDICOMMRSWriter
    ${SVK_VIZ_LIBS}
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
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkImageViewer2
    test_svkImageViewer2.cc
)

TARGET_LINK_LIBRARIES(
    test_svkImageViewer2
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkOrientedImageActor
    test_svkOrientedImageActor.cc
)

TARGET_LINK_LIBRARIES(
    test_svkOrientedImageActor
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkPhase
    test_svkPhase.cc
)

TARGET_LINK_LIBRARIES(
    test_svkPhase
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)


ADD_EXECUTABLE(
    test_svkIdfWriter
    test_svkIdfWriter.cc
)

TARGET_LINK_LIBRARIES(
    test_svkIdfWriter
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkSpectraPhaserWidget
    test_svkSpectraPhaserWidget.cc
)

TARGET_LINK_LIBRARIES(
    test_svkSpectraPhaserWidget
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkSinglePlotExtraction
    test_svkSinglePlotExtraction.cc
)

TARGET_LINK_LIBRARIES(
    test_svkSinglePlotExtraction
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkDataModel
    test_svkDataModel.cc
)

TARGET_LINK_LIBRARIES(
    test_svkDataModel
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkFormatConverter
    test_svkFormatConverter.cc
)

TARGET_LINK_LIBRARIES(
    test_svkFormatConverter
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkImageCopy
    test_svkImageCopy.cc
)

TARGET_LINK_LIBRARIES(
    test_svkImageCopy
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkObliqueReslice
    test_svkObliqueReslice.cc
)

TARGET_LINK_LIBRARIES(
    test_svkObliqueReslice
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkMrsImageFFT
    test_svkMrsImageFFT.cc
)

TARGET_LINK_LIBRARIES(
    test_svkMrsImageFFT
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkMrsImageFFTSpatial
    test_svkMrsImageFFTSpatial.cc
)

TARGET_LINK_LIBRARIES(
    test_svkMrsImageFFTSpatial
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkMrsImageExtract
    test_svkMrsImageExtract.cc
)

TARGET_LINK_LIBRARIES(
    test_svkMrsImageExtract
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_vtkImageFFT
    test_vtkImageFFT.cc
)

TARGET_LINK_LIBRARIES(
    test_vtkImageFFT
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_vtkImageFourierCenter
    test_vtkImageFourierCenter.cc
)

TARGET_LINK_LIBRARIES(
    test_vtkImageFourierCenter
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkMCPhase
    test_svkMCPhase.cc
)

TARGET_LINK_LIBRARIES(
    test_svkMCPhase
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkUCSFUtils
    test_svkUCSFUtils.cc
)

TARGET_LINK_LIBRARIES(
    test_svkUCSFUtils
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkImageAlgorithmExecuter
    test_svkImageAlgorithmExecuter.cc
)

TARGET_LINK_LIBRARIES(
    test_svkImageAlgorithmExecuter
    ${SVK_VIZ_LIBS}
    ${SVK_LIBS}
)

ADD_EXECUTABLE(
    test_svkDcmQuickRead
    test_svkDcmQuickRead.cc
)

TARGET_LINK_LIBRARIES(
    test_svkDcmQuickRead
    ${SVK_LIBS}
)
