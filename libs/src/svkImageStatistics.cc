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
    this->results = NULL;
    this->accumulator = NULL;
    this->config = NULL;
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
 * Sets the input data to calculate the statistics upon.
 */
void svkImageStatistics::SetInput( svkMriImageData* image )
{
    this->image = image;
    this->image->Register(this);
}


/*!
 * Sets the roi within which to calculate the statistics.
 */
void svkImageStatistics::SetROI( svkMriImageData* roi )
{
    this->roi = roi;
    this->roi->Register(this);
}


void svkImageStatistics::SetConfig( vtkXMLDataElement* config )
{
    this->config = config;
}


/*!
 *  This method does the computation for this class.
 */
void svkImageStatistics::Update( )
{
    if( this->config != NULL ) {
        if( this->results != NULL ) {
            this->results->Delete();
        }
        this->results = vtkXMLDataElement::New();
        this->results->SetName("measures");
        this->Execute( );
    }
}


void svkImageStatistics::Execute( )
{
    if( this->image != NULL ) {
        double* spacing = this->image->GetSpacing();
        double pixelVolume = spacing[0] * spacing[1] * spacing[2];
        if( this->accumulator != NULL ) {
            this->accumulator->Delete();
        }
        this->accumulator = vtkImageAccumulate::New();
        accumulator->SetInput( this->image );
        if( this->roi != NULL ) {
            vtkImageToImageStencil* stencil = vtkImageToImageStencil::New();
            stencil->SetInput( this->roi );
            stencil->ThresholdByUpper(1);
            stencil->Update();
            accumulator->SetStencil( stencil->GetOutput() );
            stencil->Delete();
        }
        accumulator->Update( );
        accumulator->SetIgnoreZero( false );
        int numberOfBins = 1000;
        double startBin  = 0;
        double binSize   = 1;
        if( this->config->FindNestedElementWithName("histogram") != NULL ) {
            vtkXMLDataElement* binsElement = this->config->FindNestedElementWithName("histogram")->FindNestedElementWithName("bins");
            if( binsElement != NULL ) {
                numberOfBins = svkUtils::StringToInt(binsElement->GetCharacterData());
            }
            vtkXMLDataElement* startBinElement = this->config->FindNestedElementWithName("histogram")->FindNestedElementWithName("start_bin");
            if( startBinElement != NULL ) {
                startBin = svkUtils::StringToDouble(startBinElement->GetCharacterData());
            }
            vtkXMLDataElement* binSizeElement = this->config->FindNestedElementWithName("histogram")->FindNestedElementWithName("bin_size");
            if( binSizeElement != NULL ) {
                binSize = svkUtils::StringToDouble(binSizeElement->GetCharacterData());
            }
        }
        accumulator->SetComponentExtent(0,numberOfBins-1,0,0,0,0 );
        accumulator->SetComponentOrigin(startBin, 0,0 );
        accumulator->SetComponentSpacing(binSize, 0,0);
        accumulator->Update();
        vtkXMLDataElement* element = NULL;
        if( this->config->FindNestedElementWithName("volume") != NULL ) {
            element = vtkXMLDataElement::New();
            element->SetName("volume");
            element->SetAttribute("units", "mm^3");
            string volumeString = svkUtils::DoubleToString( accumulator->GetVoxelCount()*pixelVolume );
            element->SetCharacterData( volumeString.c_str(), volumeString.size());
            this->results->AddNestedElement( element );
            element->Delete();
        }

        if( this->config->FindNestedElementWithName("max") != NULL ) {
            element = vtkXMLDataElement::New();
            element->SetName("max");
            string maxString = svkUtils::DoubleToString( *accumulator->GetMax() );
            element->SetCharacterData( maxString.c_str(), maxString.size());
            this->results->AddNestedElement( element );
            element->Delete();
        }


        if( this->config->FindNestedElementWithName("min") != NULL ) {
            element = vtkXMLDataElement::New();
            element->SetName("min");
            string minString = svkUtils::DoubleToString( *accumulator->GetMin() );
            element->SetCharacterData( minString.c_str(), minString.size());
            this->results->AddNestedElement( element );
            element->Delete();
        }

        if( this->config->FindNestedElementWithName("mean") != NULL ) {
            element = vtkXMLDataElement::New();
            element->SetName("mean");
            string meanString = svkUtils::DoubleToString( *accumulator->GetMean() );
            element->SetCharacterData( meanString.c_str(), meanString.size());
            this->results->AddNestedElement( element );
            element->Delete();
        }

        if( this->config->FindNestedElementWithName("stdev") != NULL ) {
            element = vtkXMLDataElement::New();
            element->SetName("stdev");
            string stdevString = svkUtils::DoubleToString( *accumulator->GetStandardDeviation() );
            element->SetCharacterData( stdevString.c_str(), stdevString.size());
            this->results->AddNestedElement( element );
            element->Delete();
        }

        if( this->config->FindNestedElementWithName("histogram") != NULL ) {
            vtkDataArray* histData = accumulator->GetOutput()->GetPointData()->GetScalars();
            cout << "Accumulator data: " << *accumulator->GetOutput() << endl;
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
                cout << "tuple value:" << histData->GetTuple1(i) << endl;
                string valueString = svkUtils::DoubleToString(histData->GetTuple1(i));
                element->SetCharacterData(valueString.c_str(), valueString.size());
                histogram->AddNestedElement(element);
                element->Delete();
            }
            this->results->AddNestedElement(histogram);
            histogram->Delete();
        }
    }

}


void svkImageStatistics::GetXMLResults( vtkXMLDataElement* results )
{
    if( results != NULL ) {
        results->DeepCopy( this->results );
    }
}


void svkImageStatistics::PrintStatistics( )
{
    cout << "###########################################################" << endl << endl;
    vtkIndent indent;
    this->results->PrintXML(cout, indent);
    cout << endl << "###########################################################" << endl << endl;
}
