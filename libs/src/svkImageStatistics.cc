/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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
 *
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


#include <svkImageStatistics.h>

using namespace svk;

vtkCxxRevisionMacro(svkImageStatistics, "$Rev$");
vtkStandardNewMacro(svkImageStatistics);

//! Constructor
svkImageStatistics::svkImageStatistics()
{
    this->SetNumberOfInputPorts(15);
    this->SetNumberOfOutputPorts(1);
    bool required = true;
    bool repeatable = true;
    this->GetPortMapper()->InitializeInputPort( INPUT_IMAGE, "INPUT_IMAGE", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, required, repeatable );
    this->GetPortMapper()->InitializeInputPort( INPUT_ROI, "INPUT_ROI", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, required, repeatable );
    this->GetPortMapper()->InitializeInputPort( NUM_BINS, "NUM_BINS", svkAlgorithmPortMapper::SVK_INT);
    this->GetPortMapper()->InitializeInputPort( BIN_SIZE, "BIN_SIZE", svkAlgorithmPortMapper::SVK_DOUBLE);
    this->GetPortMapper()->InitializeInputPort( START_BIN, "START_BIN", svkAlgorithmPortMapper::SVK_DOUBLE);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_HISTOGRAM, "COMPUTE_HISTOGRAM", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MEAN, "COMPUTE_MEAN", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MAX, "COMPUTE_MAX", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MIN, "COMPUTE_MIN", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_STDEV, "COMPUTE_STDEV", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_VOLUME, "COMPUTE_VOLUME", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MODE, "COMPUTE_MODE", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_QUARTILES, "COMPUTE_QUARTILES", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MEDIAN, "COMPUTE_MEDIAN", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( OUTPUT_FILE_NAME, "OUTPUT_FILE_NAME", svkAlgorithmPortMapper::SVK_STRING, !required);
    this->GetPortMapper()->InitializeOutputPort( 0, "XML_RESULTS", svkAlgorithmPortMapper::SVK_XML);
}


//! Destructor
svkImageStatistics::~svkImageStatistics()
{
}


/*
*/
vtkXMLDataElement* svkImageStatistics::GetOutput()
{
   vtkXMLDataElement* output = NULL;
   svkXML* outputDataObject = svkXML::SafeDownCast(this->GetOutputDataObject(0));
   if( outputDataObject != NULL ) {
       output = outputDataObject->GetValue();
   }
   return output;
}


/*!
 *  RequestData pass the input through the algorithm, and copies the dcos and header
 *  to the output.
 */
int svkImageStatistics::RequestData( vtkInformation* request,
                                              vtkInformationVector** inputVector,
                                              vtkInformationVector* outputVector )
{
    vtkXMLDataElement* results = this->GetOutput();

    results->RemoveAllNestedElements();
    results->RemoveAllAttributes();
    results->SetName("svk_image_stats_results");

    results->SetAttribute("version", string(SVK_RELEASE_VERSION).c_str());
    time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    string timeString = asctime(timeinfo);
    // Chop off the return character from asctime:
    timeString = timeString.substr(0, timeString.size()-1);
    results->SetAttribute("date", timeString.c_str());

    for (int imageIndex = 0; imageIndex < this->GetNumberOfInputConnections(INPUT_IMAGE); imageIndex++) {
        for (int roiIndex = 0; roiIndex < this->GetNumberOfInputConnections(INPUT_ROI); roiIndex++) {
            svkMriImageData* image = this->GetPortMapper()->GetMRImageInputPortValue(INPUT_IMAGE, imageIndex);
            svkMriImageData* roi   = this->GetPortMapper()->GetMRImageInputPortValue(INPUT_ROI, roiIndex);
            vtkXMLDataElement* nextResult = vtkXMLDataElement::New();
            nextResult->SetName("results");
            string imageLabel = image->GetDcmHeader()->GetStringValue("SeriesDescription");
            string roiLabel = roi->GetDcmHeader()->GetStringValue("SeriesDescription");
            svkUtils::CreateNestedXMLDataElement( nextResult, "IMAGE", imageLabel);
            svkUtils::CreateNestedXMLDataElement( nextResult, "ROI",   roiLabel);
            vtkXMLDataElement* statistics = vtkXMLDataElement::New();
            bool geometriesMatch = true;
            if( roi != NULL ) {
                svkDataValidator* validator = svkDataValidator::New();
                if( !validator->AreDataGeometriesSame( image, roi ) ) {
                    cout << "ERROR: Data geometries do not match between image and ROI. Cannot compute order statistics:" << validator->resultInfo << endl;
                    geometriesMatch = false;
                }
                validator->Delete();
            }
            statistics->SetName("measures");
            if( geometriesMatch ) {
                this->ComputeOrderStatistics(image,roi, statistics);
                this->ComputeStatistics(image,roi, statistics);
            }
            vtkIndent indent;
            if( statistics != NULL ) {
                nextResult->AddNestedElement( statistics );
            }
            results->AddNestedElement( nextResult );
            nextResult->Delete();
        }
    }

    vtkIndent indent;
    svkString* outputFileName = this->GetPortMapper()->GetStringInputPortValue(OUTPUT_FILE_NAME);
    if( outputFileName != NULL && outputFileName->GetValue() != "") {
        vtkXMLUtilities::WriteElementToFile( results, outputFileName->GetValue().c_str());
    }
    return 1;
}


/*!
 * Computes basic statistics using vtkImageAccumulate.
 */
void svkImageStatistics::ComputeStatistics(svkMriImageData* image, svkMriImageData* roi, vtkXMLDataElement* results)
{
    if( image != NULL ) {
        double* spacing = image->GetSpacing();
        double pixelVolume = spacing[0] * spacing[1] * spacing[2];
        vtkImageAccumulate* accumulator = vtkImageAccumulate::New();
        accumulator->SetInput( image );
        if( roi != NULL ) {
            vtkImageToImageStencil* stencil = vtkImageToImageStencil::New();
            stencil->SetInput( roi );
            stencil->ThresholdByUpper(1);
            stencil->Update();
            accumulator->SetStencil( stencil->GetOutput() );
            stencil->Delete();
        }
        accumulator->Update( );
        accumulator->SetIgnoreZero( false );
        int numberOfBins = this->GetPortMapper()->GetIntInputPortValue( NUM_BINS )->GetValue();
        double startBin  = this->GetPortMapper()->GetDoubleInputPortValue( START_BIN )->GetValue();
        double binSize   = this->GetPortMapper()->GetDoubleInputPortValue( BIN_SIZE )->GetValue();
        accumulator->SetComponentExtent(0,numberOfBins-1,0,0,0,0 );
        accumulator->SetComponentOrigin(startBin, 0,0 );
        accumulator->SetComponentSpacing(binSize, 0,0);
        accumulator->Update();
        vtkXMLDataElement* element = NULL;
        if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_VOLUME) && this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_VOLUME)->GetValue()) {
            element = vtkXMLDataElement::New();
            element->SetName("volume");
            element->SetAttribute("units", "mm^3");
            string volumeString = svkUtils::DoubleToString( accumulator->GetVoxelCount()*pixelVolume );
            element->SetCharacterData( volumeString.c_str(), volumeString.size());
            results->AddNestedElement( element );
            element->Delete();
        }

        if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_MAX) && this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_MAX)->GetValue()) {
            element = vtkXMLDataElement::New();
            element->SetName("max");
            string maxString = svkUtils::DoubleToString( *accumulator->GetMax() );
            element->SetCharacterData( maxString.c_str(), maxString.size());
            results->AddNestedElement( element );
            element->Delete();
        }


        if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_MIN) && this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_MIN)->GetValue()) {
            element = vtkXMLDataElement::New();
            element->SetName("min");
            string minString = svkUtils::DoubleToString( *accumulator->GetMin() );
            element->SetCharacterData( minString.c_str(), minString.size());
            results->AddNestedElement( element );
            element->Delete();
        }

        if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_MEAN) && this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_MEAN)->GetValue()) {
            element = vtkXMLDataElement::New();
            element->SetName("mean");
            string meanString = svkUtils::DoubleToString( *accumulator->GetMean() );
            element->SetCharacterData( meanString.c_str(), meanString.size());
            results->AddNestedElement( element );
            element->Delete();
        }

        if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_STDEV) && this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_STDEV)->GetValue()) {
            element = vtkXMLDataElement::New();
            element->SetName("stdev");
            string stdevString = svkUtils::DoubleToString( *accumulator->GetStandardDeviation() );
            element->SetCharacterData( stdevString.c_str(), stdevString.size());
            results->AddNestedElement( element );
            element->Delete();
        }

        if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_MODE) &&  this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_MODE)->GetValue()) {
            vtkDataArray* histData = accumulator->GetOutput()->GetPointData()->GetScalars();
            double max = *accumulator->GetMax();
            double min = *accumulator->GetMin();
            int numBins =  histData->GetNumberOfTuples();
            accumulator->Update();
            if( numBins > 0 && histData != NULL ) {
                double mode = 0;
                int binMax = 0;
                for( int i = 0; i < numBins; i++ ) {
                    if( histData->GetTuple1(i) > binMax ) {
                        binMax = histData->GetTuple1(i);
                        mode = startBin + (i+0.5)*binSize;
                    }
                }
                element = vtkXMLDataElement::New();
                element->SetName("mode");
                string valueString = svkUtils::DoubleToString(mode);
                element->SetCharacterData(valueString.c_str(), valueString.size());
                results->AddNestedElement(element);
                element->Delete();
            }
        }

        if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_HISTOGRAM) &&  this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_HISTOGRAM)->GetValue()) {
            vtkDataArray* histData = accumulator->GetOutput()->GetPointData()->GetScalars();
            double max = *accumulator->GetMax();
            double min = *accumulator->GetMin();
            int numBins =  histData->GetNumberOfTuples();
            accumulator->Update();
            vtkXMLDataElement* histogram = vtkXMLDataElement::New();
            histogram->SetName("histogram");
            histogram->SetAttribute("bins", svkUtils::IntToString(numBins).c_str());
            for( int i = 0; i < numBins; i++ ) {
                element = vtkXMLDataElement::New();
                element->SetName("bin");
                element->SetAttribute("index", svkUtils::IntToString(i).c_str());
                element->SetAttribute("min", svkUtils::DoubleToString(startBin + i*binSize).c_str());
                element->SetAttribute("max", svkUtils::DoubleToString(startBin + (i+1)*binSize).c_str());
                string valueString = svkUtils::DoubleToString(histData->GetTuple1(i));
                element->SetCharacterData(valueString.c_str(), valueString.size());
                histogram->AddNestedElement(element);
                element->Delete();
            }
            results->AddNestedElement(histogram);
            histogram->Delete();
        }
        accumulator->Delete();
    }
}


/*!
 * Computes statistics using the vtkOrderStatistics class.
 */
void svkImageStatistics::ComputeOrderStatistics(svkMriImageData* image, svkMriImageData* roi, vtkXMLDataElement* results)
{
    if( image != NULL ) {
        vtkDataArray* pixels = image->GetPointData()->GetScalars();
        vtkDataArray* mask   = NULL;
        if( roi != NULL ) {
            mask  = roi->GetPointData()->GetScalars();
        }
        vtkDataArray* pixelsInROI = vtkDataArray::CreateDataArray( pixels->GetDataType());
        pixelsInROI->SetName("PixelsInROI");
        pixelsInROI->SetNumberOfComponents(1);

        for( int i = 0; i < pixels->GetNumberOfTuples(); i++ ) {
            if( mask == NULL || mask->GetTuple1(i) > 0 ) {
                pixelsInROI->InsertNextTuple1( pixels->GetTuple1(i));
            }
        }
        vtkTable* table = vtkTable::New();
        table->AddColumn( pixelsInROI );
        vtkOrderStatistics* orderStats = vtkOrderStatistics::New();
        orderStats->SetInput( vtkStatisticsAlgorithm::INPUT_DATA, table );
        orderStats->AddColumn(pixelsInROI->GetName());
        orderStats->SetLearnOption(true);
        orderStats->SetAssessOption(false);
        orderStats->Update();
        vtkTable* statResults = orderStats->GetOutput(1);
        /*
         // This is usefull for debuging. It prints all the results of the algroithm out.
        cout << "statResults: " << *statResults << endl;
        for( int i = 0; i < statResults->GetNumberOfRows(); i++) {
            for( int j = 0; j < statResults->GetNumberOfColumns(); j++) {
                cout << statResults->GetColumnName(j) << ": " << statResults->GetValue(i,j) << endl;
            }
        }
        */

        vtkXMLDataElement* element = NULL;
        if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_QUARTILES) && this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_QUARTILES)->GetValue()) {
            element = vtkXMLDataElement::New();
            element->SetName("firstquartile");
            string elementString = svkUtils::DoubleToString( statResults->GetValueByName(0,"First Quartile").ToDouble() );
            element->SetCharacterData( elementString.c_str(), elementString.size());
            results->AddNestedElement( element );
            element->Delete();
            element = vtkXMLDataElement::New();
            element->SetName("median");
            elementString = svkUtils::DoubleToString( statResults->GetValueByName(0,"Median").ToDouble() );
            element->SetCharacterData( elementString.c_str(), elementString.size());
            results->AddNestedElement( element );
            element->Delete();
            element = vtkXMLDataElement::New();
            element->SetName("thirdquartile");
            elementString = svkUtils::DoubleToString( statResults->GetValueByName(0,"Third Quartile").ToDouble() );
            element->SetCharacterData( elementString.c_str(), elementString.size());
            results->AddNestedElement( element );
            element->Delete();
        } else if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_MEDIAN) && this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_MEDIAN)->GetValue()) {
            element = vtkXMLDataElement::New();
            element->SetName("median");
            string elementString = svkUtils::DoubleToString( statResults->GetValueByName(0,"Median").ToDouble() );
            element->SetCharacterData( elementString.c_str(), elementString.size());
            results->AddNestedElement( element );
            element->Delete();
        }

        table->Delete();
        pixelsInROI->Delete();
    }
}
