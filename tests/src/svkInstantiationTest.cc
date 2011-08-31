/*
 *  Copyright © 2009-2011 The Regents of the University of California.
 *  All Rights Reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  •   Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *  •   Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *  •   None of the names of any campus of the University of California, the name
 *      "The Regents of the University of California," or the names of any of its
 *      contributors may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 *  OF SUCH DAMAGE.
 */


/*
 *  $URL$ 
 *  $Rev$
 *  $Author$
 *  $Date$
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */



#include <svkAlgoTemplate.h>
#include <svkBoxPlot.h>
#include <svkBurnResearchPixels.h>
#include <svkByteSwap.h>
#include <svkCoilCombine.h>
#include <svkCorrectDCOffset.h>
#include <svkDataModel.h>
#include <svkDataValidator.h>
#include <svkDataViewController.h>
#include <svkDataView.h>
#include <svkDcmHeader.h>
#include <svkDcmMriVolumeReader.h>
#include <svkDcmMrsVolumeReader.h>
#include <svkDcmtkAdapter.h>
#include <svkDcmtkException.h>
#include <svkDcmtkIod.h>
#include <svkDcmtkUtils.h>
#include <svkDcmVolumeReader.h>
#include <svkDdfVolumeReader.h>
#include <svkDdfVolumeWriter.h>
#include <svkDetailedPlotViewController.h>
#include <svkDetailedPlotView.h>
#include <svkDICOMEnhancedMRIWriter.h>
#include <svkDICOMImageWriter.h>
#include <svkDICOMMRIWriter.h>
#include <svkDICOMMRSWriter.h>
#include <svkDICOMSCWriter.h>
#include <svkEnhancedMRIIOD.h>
#include <svkEPSIPhaseCorrect.h>
#include <svkFdfVolumeReader.h>
#include <svkGEImageHeader.h>
#include <svkGEPFileMapper.h>
#include <svkGEPFileMapperMBrease.h>
#include <svkGEPFileMapperMPCSIOBL.h>
#include <svkGEPFileMapperUCSFfidcsiDev0.h>
#include <svkGEPFileMapperUCSFfidcsi.h>
#include <svkGEPFileMapperUCSF.h>
#include <svkGEPFileReader.h>
#include <svkGEPostageStampReader.h>
#include <svkGESigna5XReader.h>
#include <svkGESignaLX2Reader.h>
#include <svkIdfVolumeReader.h>
#include <svkIdfVolumeWriter.h>
#include <svkImageAlgorithmExecuter.h>
#include <svkImageAlgorithm.h>
#include <svkImageClip.h>
#include <svkImageCopy.h>
#include <svkImageDataFactory.h>
#include <svkImageData.h>
#include <svkImageFourierCenter.h>
#include <svkImageInPlaceFilter.h>
#include <svkImageLinearPhase.h>
#include <svkImageMapToColors.h>
#include <svkImageMapToWindowLevelColors.h>
#include <svkImageReader2.h>
#include <svkImageReaderFactory.h>
#include <svkImageTopologyGenerator.h>
#include <svkImageView2DController.h>
#include <svkImageView2D.h>
#include <svkImageViewer2.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkIntegratePeak.h>
#include <svkIOD.h>
#include <svkLookupTable.h>
#include <svkMriImageData.h>
#include <svkMRIIOD.h>
#include <svkMrsImageData.h>
#include <svkMrsImageFFT.h>
#include <svkMRSIOD.h>
#include <svkMrsTopoGenerator.h>
#include <svkMultiCoilPhase.h>
#include <svkMultiWindowToImageFilter.h>
#include <svkObliqueReslice.h>
#include <svkOpenGLOrientedImageActor.h>
#include <svkOrientedImageActorFactory.h>
#include <svkOverlaySelector.h>
#include <svkOverlayViewController.h>
#include <svkOverlayView.h>
#include <svkPACSInterface.h>
#include <svkPhaseSpec.h>
#include <svkPlotGridViewController.h>
#include <svkPlotGridView.h>
#include <svkPlotLineGrid.h>
#include <svkPlotLine.h>
#include <svkProvenance.h>
#include <svkSatBandSet.h>
#include <svkSCIOD.h>
#include <svkSdbmVolumeReader.h>
#include <svkSiemensRdaReader.h>
#include <svkSpecGridSelector.h>
#include <svkSpecPoint.h>
#include <svkSpectraReferenceViewController.h>
#include <svkSpectraReferenceView.h>
#include <svkSpecUtils.h>
#include <svkUCSFPACSInterface.h>
#include <svkUCSFUtils.h>
#include <svkUtils.h>
#include <svkVarianCSFidMapper.h>
#include <svkVarianFidMapper.h>
#include <svkVarianFidReader.h>
#include <svkVarianReader.h>
    
using namespace svk;

int main ( int argc, char** argv )
{
    vtkObject* obj;
    obj = svkBurnResearchPixels::New();
    obj->Delete();
    obj = svkBoxPlot::New();
    obj->Delete();
    obj = svkByteSwap::New();
    obj->Delete();
    obj = svkCoilCombine::New();
    obj->Delete();
    obj = svkCorrectDCOffset::New();
    obj->Delete();
    obj = svkDataModel::New();
    obj->Delete();
    obj = svkDataValidator::New();
    obj->Delete();
    obj = svkDataViewController::New();
    obj->Delete();
    obj = svkDataView::New();
    obj->Delete();
    obj = svkDcmHeader::New();
    obj->Delete();
    obj = svkDcmMriVolumeReader::New();
    obj->Delete();
    obj = svkDcmMrsVolumeReader::New();
    obj->Delete();
    obj = svkDcmtkAdapter::New();
    obj->Delete();
    obj = svkDcmVolumeReader::New();
    obj->Delete();
    obj = svkDdfVolumeReader::New();
    obj->Delete();
    obj = svkDdfVolumeWriter::New();
    obj->Delete();
    obj = svkDetailedPlotViewController::New();
    obj->Delete();
    obj = svkDetailedPlotView::New();
    obj->Delete();
    obj = svkDICOMEnhancedMRIWriter::New();
    obj->Delete();
    obj = svkDICOMImageWriter::New();
    obj->Delete();
    obj = svkDICOMMRIWriter::New();
    obj->Delete();
    obj = svkDICOMMRSWriter::New();
    obj->Delete();
    obj = svkDICOMSCWriter::New();
    obj->Delete();
    obj = svkEnhancedMRIIOD::New();
    obj->Delete();
    obj = svkEPSIPhaseCorrect::New();
    obj->Delete();
    obj = svkFdfVolumeReader::New();
    obj->Delete();
    obj = svkGEPFileMapper::New();
    obj->Delete();
    obj = svkGEPFileMapperMBrease::New();
    obj->Delete();
    obj = svkGEPFileMapperMPCSIOBL::New();
    obj->Delete();
    obj = svkGEPFileMapperUCSFfidcsiDev0::New();
    obj->Delete();
    obj = svkGEPFileMapperUCSFfidcsi::New();
    obj->Delete();
    obj = svkGEPFileMapperUCSF::New();
    obj->Delete();
    obj = svkGEPFileReader::New();
    obj->Delete();
    obj = svkGEPostageStampReader::New();
    obj->Delete();
    obj = svkGESigna5XReader::New();
    obj->Delete();
    obj = svkGESignaLX2Reader::New();
    obj->Delete();
    obj = svkIdfVolumeReader::New();
    obj->Delete();
    obj = svkIdfVolumeWriter::New();
    obj->Delete();
    obj = svkImageAlgorithmExecuter::New();
    obj->Delete();
    obj = svkImageAlgorithm::New();
    obj->Delete();
    obj = svkImageClip::New();
    obj->Delete();
    obj = svkImageCopy::New();
    obj->Delete();
    obj = svkImageDataFactory::New();
    obj->Delete();
    obj = svkImageData::New();
    obj->Delete();
    obj = svkImageFourierCenter::New();
    obj->Delete();
    obj = svkImageInPlaceFilter::New();
    obj->Delete();
    obj = svkImageLinearPhase::New();
    obj->Delete();
    obj = svkImageMapToColors::New();
    obj->Delete();
    obj = svkImageMapToWindowLevelColors::New();
    obj->Delete();
    obj = svkImageReader2::New();
    obj->Delete();
    obj = svkImageReaderFactory::New();
    obj->Delete();
    obj = svkImageTopologyGenerator::New();
    obj->Delete();
    obj = svkImageView2DController::New();
    obj->Delete();
    obj = svkImageView2D::New();
    obj->Delete();
    obj = svkImageViewer2::New();
    obj->Delete();
    obj = svkImageWriterFactory::New();
    obj->Delete();
    obj = svkImageWriter::New();
    obj->Delete();
    obj = svkIntegratePeak::New();
    obj->Delete();
    obj = svkIOD::New();
    obj->Delete();
    obj = svkLookupTable::New();
    obj->Delete();
    obj = svkMriImageData::New();
    obj->Delete();
    obj = svkMRIIOD::New();
    obj->Delete();
    obj = svkMrsImageData::New();
    obj->Delete();
    obj = svkMrsImageFFT::New();
    obj->Delete();
    obj = svkMRSIOD::New();
    obj->Delete();
    obj = svkMrsTopoGenerator::New();
    obj->Delete();
    obj = svkMultiCoilPhase::New();
    obj->Delete();
    obj = svkMultiWindowToImageFilter::New();
    obj->Delete();
    obj = svkObliqueReslice::New();
    obj->Delete();
    obj = svkOpenGLOrientedImageActor::New();
    obj->Delete();
    obj = svkOrientedImageActorFactory::New();
    obj->Delete();
    obj = svkOverlaySelector::New();
    obj->Delete();
    obj = svkOverlayViewController::New();
    obj->Delete();
    obj = svkOverlayView::New();
    obj->Delete();
    obj = svkPACSInterface::New();
    obj->Delete();
    obj = svkPhaseSpec::New();
    obj->Delete();
    obj = svkPlotGridViewController::New();
    obj->Delete();
    obj = svkPlotGridView::New();
    obj->Delete();
    obj = svkPlotLineGrid::New();
    obj->Delete();
    obj = svkPlotLine::New();
    obj->Delete();
    obj = svkProvenance::New();
    obj->Delete();
    obj = svkSatBandSet::New();
    obj->Delete();
    obj = svkSCIOD::New();
    obj->Delete();
    obj = svkSdbmVolumeReader::New();
    obj->Delete();
    obj = svkSiemensRdaReader::New();
    obj->Delete();
    obj = svkSpecGridSelector::New();
    obj->Delete();
    obj = svkSpecPoint::New();
    obj->Delete();
    obj = svkSpecUtils::New();
    obj->Delete();
    obj = svkUCSFPACSInterface::New();
    obj->Delete();
    obj = svkUCSFUtils::New();
    obj->Delete();
    obj = svkUtils::New();
    obj->Delete();
    obj = svkVarianCSFidMapper::New();
    obj->Delete();
    obj = svkVarianFidMapper::New();
    obj->Delete();
    obj = svkVarianFidReader::New();
    obj->Delete();
    obj = svkVarianReader::New();
    obj->Delete();

    return 0;
  
}
