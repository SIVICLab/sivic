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
void svkImageStatistics::AddInput( svkMriImageData* image )
{
    this->images.push_back(image);
    this->Register(image);
}


/*!
 * Sets the roi within which to calculate the statistics.
 */
void svkImageStatistics::AddROI( svkMriImageData* roi )
{
    this->rois.push_back(roi);
    this->Register(roi);
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
    this->results->SetName("svk_image_stats");
    this->results->SetAttribute("version", string(SVK_RELEASE_VERSION).c_str());
    time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    string timeString = asctime(timeinfo);
    // Chop off the return character from asctime:
    timeString = timeString.substr(0, timeString.size()-1);
    this->results->SetAttribute("date", timeString.c_str());

    for( int i = 0; i < this->images.size(); i++) {
        for( int j = 0; j <= this->rois.size(); j++) {
            svkMriImageData* roi = NULL;
            if( j < this->rois.size() ) {
                roi = this->rois[j];
            }
            vtkXMLDataElement* imageElement = vtkXMLDataElement::New();
            vtkXMLDataElement* roiImageStats = vtkXMLDataElement::New();
            imageElement->SetName("image");
            svkUtils::CreateNestedXMLDataElement( imageElement, "description",this->images[i]->GetDcmHeader()->GetStringValue("SeriesDescription"));
            roiImageStats->AddNestedElement( imageElement );
            imageElement->Delete();
            if( roi != NULL ) {
                vtkXMLDataElement* roiElement = vtkXMLDataElement::New();
                roiElement->SetName("roi");
                svkUtils::CreateNestedXMLDataElement( roiElement, "description",this->rois[i]->GetDcmHeader()->GetStringValue("SeriesDescription"));
                roiImageStats->AddNestedElement( roiElement );
                roiElement->Delete();
            }
            this->GetImageAccumulateStats( this->images[i], roi, roiImageStats);
            this->results->AddNestedElement( roiImageStats  );
            roiImageStats->Delete();
        }
    }

}


void svkImageStatistics::GetImageAccumulateStats( svkMriImageData* data, svkMriImageData* roi, vtkXMLDataElement* stats )
{
    if( data != NULL ) {
        double* spacing = data->GetSpacing();
        double pixelVolume = spacing[0] * spacing[1] * spacing[2];
        double thresholdMin = 0.5;
        double thresholdMax = 1.5;
        if( this->accumulator != NULL ) {
            this->accumulator->Delete();
        }
        this->accumulator = vtkImageAccumulate::New();
        accumulator->SetInput( data );
        if( roi != NULL ) {
            vtkImageToImageStencil* stencil = vtkImageToImageStencil::New();
            stencil->SetInput( roi );
            stencil->ThresholdBetween( thresholdMin, thresholdMax );
            stencil->Update();
            accumulator->SetStencil( stencil->GetOutput() );
            stencil->Delete();
        }
        accumulator->Update( );
        accumulator->SetIgnoreZero( false );
        accumulator->SetComponentExtent(0,4,0,0,0,0 );
        accumulator->Update();
        stats->SetName("roi_image_stats");
        stats->SetAttribute("algorithm", "vtkImageAccumulate");

        // Add parameters
        vtkXMLDataElement* parameter = vtkXMLDataElement::New();
        parameter->SetName("param");
        parameter->SetAttribute("name", "ignoreZero");
        parameter->SetCharacterData(svkUtils::IntToString(accumulator->GetIgnoreZero()).c_str(), svkUtils::IntToString(accumulator->GetIgnoreZero()).size());
        stats->AddNestedElement( parameter );
        parameter->Delete();

        if( roi != NULL ) {
            parameter = vtkXMLDataElement::New();
            parameter->SetName("param");
            parameter->SetAttribute("name", "ROIthresholdMin");
            parameter->SetCharacterData(svkUtils::DoubleToString(thresholdMin).c_str(), svkUtils::DoubleToString(thresholdMin).size());
            stats->AddNestedElement( parameter );
            parameter->Delete();

            parameter = vtkXMLDataElement::New();
            parameter->SetName("param");
            parameter->SetAttribute("name", "ROIthresholdMax");
            parameter->SetCharacterData(svkUtils::DoubleToString(thresholdMax).c_str(), svkUtils::DoubleToString(thresholdMax).size());
            stats->AddNestedElement( parameter );
            parameter->Delete();
        }

        vtkXMLDataElement* measurements = vtkXMLDataElement::New();
        measurements->SetName("statistics");
        stats->AddNestedElement( measurements );
        vtkXMLDataElement* element = vtkXMLDataElement::New();
        element->SetName("value");
        element->SetAttribute("name", "volume");
        element->SetAttribute("units", "mm^3");
        string volumeString = svkUtils::DoubleToString( accumulator->GetVoxelCount()*pixelVolume );
        element->SetCharacterData( volumeString.c_str(), volumeString.size());
        measurements->AddNestedElement( element );
        element->Delete();

        element = vtkXMLDataElement::New();
        element->SetName("value");
        element->SetAttribute("name", "max");
        string maxString = svkUtils::DoubleToString( *accumulator->GetMax() );
        element->SetCharacterData( maxString.c_str(), maxString.size());
        measurements->AddNestedElement( element );
        element->Delete();


        element = vtkXMLDataElement::New();
        element->SetName("value");
        element->SetAttribute("name", "min");
        string minString = svkUtils::DoubleToString( *accumulator->GetMin() );
        element->SetCharacterData( minString.c_str(), minString.size());
        measurements->AddNestedElement( element );
        element->Delete();

        element = vtkXMLDataElement::New();
        element->SetName("value");
        element->SetAttribute("name", "mean");
        string meanString = svkUtils::DoubleToString( *accumulator->GetMean() );
        element->SetCharacterData( meanString.c_str(), meanString.size());
        measurements->AddNestedElement( element );
        element->Delete();

        element = vtkXMLDataElement::New();
        element->SetName("value");
        element->SetAttribute("name", "stdev");
        string stdevString = svkUtils::DoubleToString( *accumulator->GetStandardDeviation() );
        element->SetCharacterData( stdevString.c_str(), stdevString.size());
        measurements->AddNestedElement( element );
        element->Delete();

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
        measurements->AddNestedElement(histogram);
        measurements->Delete();
        histogram->Delete();

    }

}


void svkImageStatistics::PrintStatistics( )
{
    cout << "###########################################################" << endl << endl;
    vtkIndent indent;
    this->results->PrintXML(cout, indent);
    cout << endl << "###########################################################" << endl << endl;
}
