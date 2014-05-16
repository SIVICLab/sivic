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

void svkImageStatistics::GenerateDefaultConfig( vtkXMLDataElement* config )
{
    if( config != NULL ) {
        config->SetName("svk_image_statistics_config");
        config->RemoveAllAttributes();
        config->RemoveAllNestedElements();
        vtkIndent indent;
        config->PrintXML(cout, indent);
    }
}

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
    if( this->config == NULL ) {
        this->config = vtkXMLDataElement::New();
        svkImageStatistics::GenerateDefaultConfig( this->config );
    }
    if( this->results != NULL ) {
        this->results->Delete();
    }
    this->results = vtkXMLDataElement::New();
    this->results->SetName("measures");
    this->GetImageAccumulateStats( this->image, this->roi, this->results );

}


void svkImageStatistics::GetImageAccumulateStats( svkMriImageData* data, svkMriImageData* roi, vtkXMLDataElement* stats )
{
    if( data != NULL ) {
        double* spacing = data->GetSpacing();
        double pixelVolume = spacing[0] * spacing[1] * spacing[2];
        if( this->accumulator != NULL ) {
            this->accumulator->Delete();
        }
        this->accumulator = vtkImageAccumulate::New();
        accumulator->SetInput( data );
        if( roi != NULL ) {
            vtkImageToImageStencil* stencil = vtkImageToImageStencil::New();
            stencil->SetInput( roi );
            stencil->Update();
            accumulator->SetStencil( stencil->GetOutput() );
            stencil->Delete();
        }
        accumulator->Update( );
        accumulator->SetIgnoreZero( true );
        int numberOfBins = 5;
        if( this->config->FindNestedElementWithName("histogram") != NULL ) {
            vtkXMLDataElement* binsElement = this->config->FindNestedElementWithName("histogram")->FindNestedElementWithName("bins");
            if( binsElement != NULL ) {
                numberOfBins = svkUtils::StringToInt(binsElement->GetCharacterData());
            }
        }
        accumulator->SetComponentExtent(0,numberOfBins-1,0,0,0,0 );
        accumulator->Update();
        vtkXMLDataElement* element = NULL;
        if( this->config->FindNestedElementWithName("volume") != NULL ) {
            element = vtkXMLDataElement::New();
            element->SetName("volume");
            element->SetAttribute("units", "mm^3");
            string volumeString = svkUtils::DoubleToString( accumulator->GetVoxelCount()*pixelVolume );
            element->SetCharacterData( volumeString.c_str(), volumeString.size());
            stats->AddNestedElement( element );
            element->Delete();
        }

        if( this->config->FindNestedElementWithName("max") != NULL ) {
            element = vtkXMLDataElement::New();
            element->SetName("max");
            string maxString = svkUtils::DoubleToString( *accumulator->GetMax() );
            element->SetCharacterData( maxString.c_str(), maxString.size());
            stats->AddNestedElement( element );
            element->Delete();
        }


        if( this->config->FindNestedElementWithName("min") != NULL ) {
            element = vtkXMLDataElement::New();
            element->SetName("min");
            string minString = svkUtils::DoubleToString( *accumulator->GetMin() );
            element->SetCharacterData( minString.c_str(), minString.size());
            stats->AddNestedElement( element );
            element->Delete();
        }

        if( this->config->FindNestedElementWithName("mean") != NULL ) {
            element = vtkXMLDataElement::New();
            element->SetName("mean");
            string meanString = svkUtils::DoubleToString( *accumulator->GetMean() );
            element->SetCharacterData( meanString.c_str(), meanString.size());
            stats->AddNestedElement( element );
            element->Delete();
        }

        if( this->config->FindNestedElementWithName("stdev") != NULL ) {
            element = vtkXMLDataElement::New();
            element->SetName("stdev");
            string stdevString = svkUtils::DoubleToString( *accumulator->GetStandardDeviation() );
            element->SetCharacterData( stdevString.c_str(), stdevString.size());
            stats->AddNestedElement( element );
            element->Delete();
        }

        if( this->config->FindNestedElementWithName("histogram") != NULL ) {
            vtkDataArray* histData = accumulator->GetOutput()->GetPointData()->GetScalars();
            double max = *accumulator->GetMax();
            double min = *accumulator->GetMin();
            int numBins =  histData->GetNumberOfTuples();
            double binSize = (max-min)/numBins;
            vtkXMLDataElement* histogram = vtkXMLDataElement::New();
            histogram->SetName("histogram");
            histogram->SetAttribute("bins", svkUtils::IntToString(numBins).c_str());
            for( int i = 0; i < numBins; i++ ) {
                element = vtkXMLDataElement::New();
                element->SetName("bin");
                element->SetAttribute("index", svkUtils::IntToString(i).c_str());
                element->SetAttribute("min", svkUtils::DoubleToString(i*binSize).c_str());
                element->SetAttribute("max", svkUtils::DoubleToString((i+1)*binSize).c_str());
                string valueString = svkUtils::DoubleToString(histData->GetTuple1(i));
                element->SetCharacterData(valueString.c_str(), valueString.size());
                histogram->AddNestedElement(element);
                element->Delete();
            }
            stats->AddNestedElement(histogram);
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
