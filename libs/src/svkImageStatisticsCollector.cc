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


#include <svkImageStatisticsCollector.h>

using namespace svk;

vtkCxxRevisionMacro(svkImageStatisticsCollector, "$Rev$");
vtkStandardNewMacro(svkImageStatisticsCollector);

//! Constructor
svkImageStatisticsCollector::svkImageStatisticsCollector()
{
    cout << "Constructing svkImageStatisticsCollector." << endl;
    this->RootName = NULL;
}


//! Destructor
svkImageStatisticsCollector::~svkImageStatisticsCollector()
{
    if( this->lastFilter != NULL ) {
        this->lastFilter->Delete();
        this->lastFilter = NULL;
    }
    if( this->reader != NULL ) {
        this->reader->Delete();
        this->reader = NULL;
    }
    map<string,svkMriImageData*>::iterator imageHashIter;
    for (imageHashIter = this->images.begin(); imageHashIter != this->images.end(); ++imageHashIter) {
        imageHashIter->second->Delete();
    }
    for (imageHashIter = this->rois.begin(); imageHashIter != this->rois.end(); ++imageHashIter) {
        imageHashIter->second->Delete();
    }
}


/*!
 * Method sets the XML configuration file.
 */
void svkImageStatisticsCollector::SetXMLConfiguration( vtkXMLDataElement* config )
{
    this->config = config;
    this->reader = NULL;
    vtkIndent indent;
    cout << "COFIGURATION SET BY XML:" << endl;
    this->config->PrintXML(cout, indent);
}


/*!
 * Method computes and returns the results in xml.
 */
void svkImageStatisticsCollector::GetXMLResults( vtkXMLDataElement* results )
{
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

    this->LoadImagesAndROIS( );

    map<string,svkMriImageData*>::iterator mapIter;
    map<string,svkMriImageData*>::iterator roiIter;
    for (mapIter = this->images.begin(); mapIter != this->images.end(); ++mapIter) {
        for (roiIter = this->rois.begin(); roiIter != this->rois.end(); ++roiIter) {
            svkImageStatistics* statsCalculator = svkImageStatistics::New();
            cout << "SETTING INPUTS..." << endl;
            statsCalculator->SetInputPortsFromXML(this->config->FindNestedElementWithName("measures")->FindNestedElementWithName("svkImageStatistics"));
            statsCalculator->SetInput(svkImageStatistics::INPUT_IMAGE, mapIter->second );
            statsCalculator->SetInput(svkImageStatistics::INPUT_ROI, roiIter->second );
            cout << "SETTING UPDATING..." << endl;
            statsCalculator->Update();
            vtkXMLDataElement* nextResult = vtkXMLDataElement::New();
            nextResult->SetName("results");
            svkUtils::CreateNestedXMLDataElement( nextResult, "ROI",roiIter->first.c_str() );
            svkUtils::CreateNestedXMLDataElement( nextResult, "MAP",mapIter->first.c_str() );
            vtkXMLDataElement* statistics = vtkXMLDataElement::New();
            statsCalculator->GetXMLResults(statistics);
            nextResult->AddNestedElement( statistics );
            results->AddNestedElement( nextResult );
            nextResult->Delete();
            statistics->Delete();
            statsCalculator->Delete();
        }
    }

}


/*!
 * Method combines the basename with the 'suffix' and 'directory' tags found in the given
 * vtkXMLDataElement to read an svkImageData object which is then returned.
 */
svkImageData* svkImageStatisticsCollector::ReadImageFromXML( vtkXMLDataElement* imageElement )
{
    svkImageData* image = NULL;
    if( imageElement != NULL ) {
        string suffix = string(imageElement->FindNestedElementWithName("suffix")->GetCharacterData());
        string directory = string(imageElement->FindNestedElementWithName("directory")->GetCharacterData());
        string filename = directory;
        filename.append("/");
        filename.append(this->RootName);
        filename.append("_");
        filename.append(suffix);
        // READ THE IMAGE
        svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
        if( this->reader != NULL ) {
            this->reader->Delete();
        }
        this->reader = readerFactory->CreateImageReader2(filename.c_str());
        readerFactory->Delete();
        if (this->reader != NULL) {
            this->reader->SetFileName( filename.c_str() );
            this->reader->Update();
            image = this->reader->GetOutput();
        } else {
            cout << "ERROR: Could not read file: " << filename << endl;
        }
    } else {
        cout << "Image element is NULL!" << endl;
    }
    return image;
}


/*!
 * This method loads the images and ROIs from the input configuration, applies the necessary filters
 * using svkImageStatisticsCollector::ApplyFiltersFromXML, and stores them in a hash using the label
 * found in the ROI/MAP tag.
 */
svkImageData* svkImageStatisticsCollector::LoadImagesAndROIS( )
{
    int numberOfConfigElements = this->config->GetNumberOfNestedElements();
    for( int i = 0; i < numberOfConfigElements; i++ ) {
        vtkXMLDataElement* element = this->config->GetNestedElement(i);
        if( string(element->GetName()) ==  "ROI") {
            svkImageData* roi = this->ReadImageFromXML( element );
            string label = element->GetAttribute("label");
            svkImageData* filteredData = this->ApplyFiltersFromXML( roi, element );
            this->rois[label] = svkMriImageData::SafeDownCast(filteredData);
            this->rois[label]->Register(this);
        } else if( string(element->GetName()) ==  "MAP") {
            svkImageData* map = this->ReadImageFromXML( element );
            string label = element->GetAttribute("label");
            svkImageData* filteredData = this->ApplyFiltersFromXML( map, element );
            this->images[label] = svkMriImageData::SafeDownCast(filteredData);
            this->images[label]->Register(this);
        }
    }
}


/*!
 *  This method takes an svkImageData object and a vtkXMLDataElement containing the filter tags
 *  to be applied to the data. It then returns the result of the pipeline.
 */
svkImageData* svkImageStatisticsCollector::ApplyFiltersFromXML( svkImageData* inputImage, vtkXMLDataElement* imageElement )
{
    vtkXMLDataElement* filters = imageElement->FindNestedElementWithName("filters");
    svkImageData* filteredImage = inputImage;
    if( filters != NULL ) {
        int numberOfFilters = filters->GetNumberOfNestedElements();
        for( int i = 0; i < numberOfFilters; i++ ) {
            vtkXMLDataElement* filterParameters = filters->GetNestedElement(i);

            // Get the next filter
            svkXMLImageAlgorithm* filter = GetAlgorithmForFilterName(filterParameters->GetName());
            if( filter != NULL) {
                filter->SetInputPortsFromXML( filterParameters );
                filter->SetInput( svkImageThreshold::INPUT_IMAGE, filteredImage);

                // RUN THE ALGORITHM
                filter->Update();
                filteredImage = filter->GetOutput();

                // Let's hold onto a pointer of the last filter so the filteredImage does not get freed early
                if( this->lastFilter != NULL ) {
                    this->lastFilter->Delete();
                }
                this->lastFilter = filter;
                cout << "Running Algorithm:" << *filter << endl;
            }
        }
    }
    return filteredImage;
}


/*!
 * Factory method for getting the algorithms to be used in the statistics collection.
 */
svkXMLImageAlgorithm* svkImageStatisticsCollector::GetAlgorithmForFilterName( string filterName )
{
    svkXMLImageAlgorithm* algorithm;
    // TODO: Replace this with an algorithm factory method...
    if( filterName == "svkImageThreshold") {
        algorithm = svkImageThreshold::New();
    }
    return algorithm;
}
