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
    this->SetNumberOfInputPorts(11);
    this->SetNumberOfOutputPorts(1);
    this->GetPortMapper()->InitializeInputPort( INPUT_IMAGE, "INPUT_IMAGE", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
    this->GetPortMapper()->InitializeInputPort( INPUT_ROI, "INPUT_ROI", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
    this->GetPortMapper()->InitializeInputPort( NUM_BINS, "NUM_BINS", svkAlgorithmPortMapper::SVK_INT);
    this->GetPortMapper()->InitializeInputPort( BIN_SIZE, "BIN_SIZE", svkAlgorithmPortMapper::SVK_DOUBLE);
    this->GetPortMapper()->InitializeInputPort( START_BIN, "START_BIN", svkAlgorithmPortMapper::SVK_DOUBLE);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_HISTOGRAM, "COMPUTE_HISTOGRAM", svkAlgorithmPortMapper::SVK_BOOL);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MEAN, "COMPUTE_MEAN", svkAlgorithmPortMapper::SVK_BOOL);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MAX, "COMPUTE_MAX", svkAlgorithmPortMapper::SVK_BOOL);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MIN, "COMPUTE_MIN", svkAlgorithmPortMapper::SVK_BOOL);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_STDEV, "COMPUTE_STDEV", svkAlgorithmPortMapper::SVK_BOOL);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_VOLUME, "COMPUTE_VOLUME", svkAlgorithmPortMapper::SVK_BOOL);
    this->results = NULL;
    this->accumulator = NULL;

}


//! Destructor
svkImageStatistics::~svkImageStatistics()
{
    // TODO: FRee images, rois vectors

    if( this->results != NULL ) {
        this->results->Delete();
        this->results = NULL;
    }
}


/*!
 *  RequestData pass the input through the algorithm, and copies the dcos and header
 *  to the output.
 */
int svkImageStatistics::RequestData( vtkInformation* request,
                                    vtkInformationVector** inputVector,
                                    vtkInformationVector* outputVector )
{
    cout << "REQUEST DATA IN SVK IMAGE STATISTICS" << endl;
    if( this->results != NULL ) {
        this->results->Delete();
    }
    this->results = vtkXMLDataElement::New();
    this->results->SetName("measures");
    if( this->GetPortMapper()->GetMRImageInputPortValue( INPUT_IMAGE ) != NULL ) {
        double* spacing = this->GetPortMapper()->GetMRImageInputPortValue( INPUT_IMAGE )->GetSpacing();
        double pixelVolume = spacing[0] * spacing[1] * spacing[2];
        if( this->accumulator != NULL ) {
            this->accumulator->Delete();
        }
        this->accumulator = vtkImageAccumulate::New();
        accumulator->SetInput( this->GetPortMapper()->GetMRImageInputPortValue( INPUT_IMAGE)  );
        if( this->GetPortMapper()->GetMRImageInputPortValue(INPUT_ROI) != NULL ) {
            vtkImageToImageStencil* stencil = vtkImageToImageStencil::New();
            stencil->SetInput( this->GetPortMapper()->GetMRImageInputPortValue(INPUT_ROI) );
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
            this->results->AddNestedElement( element );
            element->Delete();
        }

        if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_MAX)) {
            element = vtkXMLDataElement::New();
            element->SetName("max");
            string maxString = svkUtils::DoubleToString( *accumulator->GetMax() );
            element->SetCharacterData( maxString.c_str(), maxString.size());
            this->results->AddNestedElement( element );
            element->Delete();
        }


        if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_MIN)) {
            element = vtkXMLDataElement::New();
            element->SetName("min");
            string minString = svkUtils::DoubleToString( *accumulator->GetMin() );
            element->SetCharacterData( minString.c_str(), minString.size());
            this->results->AddNestedElement( element );
            element->Delete();
        }

        if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_MEAN)) {
            element = vtkXMLDataElement::New();
            element->SetName("mean");
            string meanString = svkUtils::DoubleToString( *accumulator->GetMean() );
            element->SetCharacterData( meanString.c_str(), meanString.size());
            this->results->AddNestedElement( element );
            element->Delete();
        }

        if( this->GetPortMapper()->GetBoolInputPortValue(COMPUTE_STDEV)) {
            element = vtkXMLDataElement::New();
            element->SetName("stdev");
            string stdevString = svkUtils::DoubleToString( *accumulator->GetStandardDeviation() );
            element->SetCharacterData( stdevString.c_str(), stdevString.size());
            this->results->AddNestedElement( element );
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
            this->results->AddNestedElement(histogram);
            histogram->Delete();
        }
    }
    return 1;
}


/*!
 * Deep copies the resutls into the given xml input element.
 */
void svkImageStatistics::GetXMLResults( vtkXMLDataElement* results )
{
    if( results != NULL ) {
        vtkIndent indent;
        cout << "Printing results from inside if image statistics" << endl;
        results->PrintXML(cout,indent);
        results->DeepCopy( this->results );
    }
}


/*!
 *  Default output type is same concrete sub class type as the input data.  Override with
 *  specific concrete type in sub-class if necessary.
 */
int svkImageStatistics::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkXML" );
    return 1;
}


/*!
 *  Prints the result statistics.
 */
void svkImageStatistics::PrintStatistics( )
{
    cout << "###########################################################" << endl << endl;
    vtkIndent indent;
    this->results->PrintXML(cout, indent);
    cout << endl << "###########################################################" << endl << endl;
}
