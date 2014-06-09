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
    this->SetNumberOfInputPorts(12);
    bool required = true;
    bool repeatable = true;
    this->GetPortMapper()->InitializeInputPort( INPUT_IMAGE, "INPUT_IMAGE", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, required, repeatable );
    this->GetPortMapper()->InitializeInputPort( INPUT_ROI, "INPUT_ROI", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, required, repeatable );
    this->GetPortMapper()->InitializeInputPort( NUM_BINS, "NUM_BINS", svkAlgorithmPortMapper::SVK_INT);
    this->GetPortMapper()->InitializeInputPort( BIN_SIZE, "BIN_SIZE", svkAlgorithmPortMapper::SVK_DOUBLE);
    this->GetPortMapper()->InitializeInputPort( START_BIN, "START_BIN", svkAlgorithmPortMapper::SVK_DOUBLE);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_HISTOGRAM, "COMPUTE_HISTOGRAM", svkAlgorithmPortMapper::SVK_BOOL);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MEAN, "COMPUTE_MEAN", svkAlgorithmPortMapper::SVK_BOOL);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MAX, "COMPUTE_MAX", svkAlgorithmPortMapper::SVK_BOOL);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MIN, "COMPUTE_MIN", svkAlgorithmPortMapper::SVK_BOOL);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_STDEV, "COMPUTE_STDEV", svkAlgorithmPortMapper::SVK_BOOL);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_VOLUME, "COMPUTE_VOLUME", svkAlgorithmPortMapper::SVK_BOOL);
    this->GetPortMapper()->InitializeInputPort( OUTPUT_FILE_NAME, "OUTPUT_FILE_NAME", svkAlgorithmPortMapper::SVK_STRING);
    //vtkInstantiator::RegisterInstantiator("svkXML",  svkXML::NewObject);
}


//! Destructor
svkImageStatistics::~svkImageStatistics()
{
}

/*
vtkXMLDataElement* svkImageStatistics::GetOutput()
{
   vtkXMLDataElement* output = NULL;
   svkXML* outputDataObject = svkXML::SafeDownCast(this->GetOutputDataObject(0));
   if( outputDataObject != NULL ) {
       output = outputDataObject->GetValue();
   }
   return output;
}
*/


/*!
 *  Default output type is same concrete sub class type as the input data.  Override with
 *  specific concrete type in sub-class if necessary.
int svkImageStatistics::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkXML");
    return 1;
}
 */


/*!
 *  RequestData pass the input through the algorithm, and copies the dcos and header
 *  to the output.
 */
int svkImageStatistics::RequestData( vtkInformation* request,
                                              vtkInformationVector** inputVector,
                                              vtkInformationVector* outputVector )
{
    cout << "MADE IT TO REQUEST DATA!" << endl;
    //vtkXMLDataElement* results = this->GetOutput();
    vtkXMLDataElement* results = vtkXMLDataElement::New();

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
            cout << "Calculating statistics for image:" << imageIndex << " and roi " << roiIndex << endl;
            svkMriImageData* image = this->GetPortMapper()->GetMRImageInputPortValue(INPUT_IMAGE, imageIndex);
            svkMriImageData* roi   = this->GetPortMapper()->GetMRImageInputPortValue(INPUT_ROI, roiIndex);
            vtkXMLDataElement* nextResult = vtkXMLDataElement::New();
            nextResult->SetName("results");
            string imageLabel = image->GetDcmHeader()->GetStringValue("SeriesDescription");
            string roiLabel = roi->GetDcmHeader()->GetStringValue("SeriesDescription");
            cout << "Image: " << imageLabel << endl;
            cout << "ROI: " << roiLabel << endl;
            svkUtils::CreateNestedXMLDataElement( nextResult, "ROI",   imageLabel);
            svkUtils::CreateNestedXMLDataElement( nextResult, "IMAGE", roiLabel);
            vtkXMLDataElement* statistics = vtkXMLDataElement::New();
            this->ComputeStatistics(image,roi, statistics);
            vtkIndent indent;
            if( statistics != NULL ) {
                nextResult->AddNestedElement( statistics );
            } else {
                cout << "STATISTICS ARE NULL!" << endl;

            }
            results->AddNestedElement( nextResult );
            cout<< "Printing result..." << endl;
            nextResult->PrintXML(cout, indent);
            nextResult->Delete();
        }
    }

    vtkIndent indent;
    svkString* outputFileName = this->GetPortMapper()->GetStringInputPortValue(OUTPUT_FILE_NAME);
    if( outputFileName != NULL && outputFileName->GetValue() != "") {
        vtkXMLUtilities::WriteElementToFile( results, outputFileName->GetValue().c_str());
    }
    results->Delete();
    return 1;
}


void svkImageStatistics::ComputeStatistics(svkMriImageData* image, svkMriImageData* roi, vtkXMLDataElement* results)
{
    results->SetName("measures");
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
        if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_VOLUME)) {
            element = vtkXMLDataElement::New();
            element->SetName("volume");
            element->SetAttribute("units", "mm^3");
            string volumeString = svkUtils::DoubleToString( accumulator->GetVoxelCount()*pixelVolume );
            element->SetCharacterData( volumeString.c_str(), volumeString.size());
            results->AddNestedElement( element );
            element->Delete();
        }

        if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_MAX)) {
            element = vtkXMLDataElement::New();
            element->SetName("max");
            string maxString = svkUtils::DoubleToString( *accumulator->GetMax() );
            element->SetCharacterData( maxString.c_str(), maxString.size());
            results->AddNestedElement( element );
            element->Delete();
        }


        if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_MIN)) {
            element = vtkXMLDataElement::New();
            element->SetName("min");
            string minString = svkUtils::DoubleToString( *accumulator->GetMin() );
            element->SetCharacterData( minString.c_str(), minString.size());
            results->AddNestedElement( element );
            element->Delete();
        }

        if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_MEAN)) {
            element = vtkXMLDataElement::New();
            element->SetName("mean");
            string meanString = svkUtils::DoubleToString( *accumulator->GetMean() );
            element->SetCharacterData( meanString.c_str(), meanString.size());
            results->AddNestedElement( element );
            element->Delete();
        }

        if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_STDEV)) {
            element = vtkXMLDataElement::New();
            element->SetName("stdev");
            string stdevString = svkUtils::DoubleToString( *accumulator->GetStandardDeviation() );
            element->SetCharacterData( stdevString.c_str(), stdevString.size());
            results->AddNestedElement( element );
            element->Delete();
        }

        if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_HISTOGRAM)) {
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
