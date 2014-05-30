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
    this->SetNumberOfInputPorts(3);
    this->SetNumberOfOutputPorts(1);
    bool required = true;
    bool repeatable = true;
    this->GetPortMapper()->InitializeInputPort( INPUT_IMAGE, "IMAGE", svkAlgorithmPortMapper::SVK_XML, required, repeatable );
    this->GetPortMapper()->InitializeInputPort( INPUT_ROI, "ROI", svkAlgorithmPortMapper::SVK_XML, required, repeatable );
    this->GetPortMapper()->InitializeInputPort( MEASURES, "measures", svkAlgorithmPortMapper::SVK_XML, required );
    cout << "Constructing svkImageStatisticsCollector." << endl;
    this->RootName = NULL;
    this->reader = NULL;
    this->pipelineFilter = NULL;
    this->results = vtkXMLDataElement::New();
    vtkInstantiator::RegisterInstantiator("svkXML",  svkXML::NewObject);
}


//! Destructor
svkImageStatisticsCollector::~svkImageStatisticsCollector()
{
    cout << "Delete pipeline..." << endl;
    if( this->pipelineFilter != NULL ) {
        this->pipelineFilter->Delete();
        this->pipelineFilter = NULL;
    }
    cout << "Delete reader..." << endl;
    if( this->reader != NULL ) {
        this->reader->Delete();
        this->reader = NULL;
    }
    cout << "Delete images..." << endl;
    map<string,svkMriImageData*>::iterator imageHashIter;
    for (imageHashIter = this->images.begin(); imageHashIter != this->images.end(); ++imageHashIter) {
        imageHashIter->second->Delete();
    }
    cout << "Delete rois..." << endl;
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
vtkXMLDataElement* svkImageStatisticsCollector::GetXMLResults( )
{
    this->results->RemoveAllNestedElements();
    this->results->RemoveAllAttributes();
    this->results->SetName("svk_image_stats_results");

    this->results->SetAttribute("version", string(SVK_RELEASE_VERSION).c_str());
    time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    string timeString = asctime(timeinfo);
    // Chop off the return character from asctime:
    timeString = timeString.substr(0, timeString.size()-1);
    this->results->SetAttribute("date", timeString.c_str());

    this->LoadImagesAndROIS( );

    map<string,svkMriImageData*>::iterator imageIter;
    map<string,svkMriImageData*>::iterator roiIter;
    for (imageIter = this->images.begin(); imageIter != this->images.end(); ++imageIter) {
        for (roiIter = this->rois.begin(); roiIter != this->rois.end(); ++roiIter) {
            cout << "Calculating statistics..." << endl;
            svkImageStatistics* statsCalculator = svkImageStatistics::New();
            statsCalculator->SetInputPortsFromXML(this->config->FindNestedElementWithName("svkArgument:measures")->FindNestedElementWithName(statsCalculator->GetPortMapper()->GetXMLTagForAlgorithm().c_str()));
            statsCalculator->GetPortMapper()->SetAlgorithmInputPort(svkImageStatistics::INPUT_IMAGE, imageIter->second );
            statsCalculator->GetPortMapper()->SetAlgorithmInputPort(svkImageStatistics::INPUT_ROI, roiIter->second );
            statsCalculator->Update();
            vtkXMLDataElement* nextResult = vtkXMLDataElement::New();
            nextResult->SetName("results");
            svkUtils::CreateNestedXMLDataElement( nextResult, "ROI",roiIter->first.c_str() );
            svkUtils::CreateNestedXMLDataElement( nextResult, "IMAGE",imageIter->first.c_str() );
            vtkXMLDataElement* statistics = vtkXMLDataElement::New();
            vtkIndent indent;
            statsCalculator->GetXMLResults(statistics);
            if( statistics == NULL ) {
                cout << "STATISTICS ARE NULL!" << endl;
            } else {
                cout<< "Printing statistics..." << endl;
                statistics->PrintXML(cout, indent);

            }
            nextResult->AddNestedElement( statistics );
            this->results->AddNestedElement( nextResult );
            cout<< "Printing result..." << endl;
            nextResult->PrintXML(cout, indent);
            nextResult->Delete();
            statistics->Delete();
            statsCalculator->Delete();
        }
    }
    return this->results;
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
 *  Default output type is same concrete sub class type as the input data.  Override with
 *  specific concrete type in sub-class if necessary.
 */
int svkImageStatisticsCollector::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkXML");
    return 1;
}


/*!
 *  RequestData pass the input through the algorithm, and copies the dcos and header
 *  to the output.
 */
int svkImageStatisticsCollector::RequestData( vtkInformation* request,
                                              vtkInformationVector** inputVector,
                                              vtkInformationVector* outputVector )
{
    cout << "MADE IT TO REQUEST DATA!" << endl;
    svkXML::SafeDownCast(this->GetOutputDataObject(0))->SetValue( this->results );
    cout << "OutputData object:" << *this->GetOutputDataObject(0) << endl;
}


/*!
 * This method loads the images and ROIs from the input configuration, applies the necessary filters
 * using svkImageStatisticsCollector::ApplyFiltersFromXML, and stores them in a hash using the label
 * found in the ROI/IMAGE tag.
 */
svkImageData* svkImageStatisticsCollector::LoadImagesAndROIS( )
{
    for( int i = 0; i< this->GetNumberOfInputConnections( INPUT_IMAGE ); i++ ) {
        svkXML* imageXML = svkXML::SafeDownCast(this->GetPortMapper()->GetAlgorithmInputPort( INPUT_IMAGE, i ));
        if( imageXML != NULL ) {
            svkImageData* image = this->ReadImageFromXML( imageXML->GetValue() );
            string label = imageXML->GetValue()->GetAttribute("label");
            svkImageData* filteredData = this->ApplyFiltersFromXML( image, imageXML->GetValue() );
            this->images[label] = svkMriImageData::SafeDownCast(filteredData);
            this->images[label]->Register(this);
        }
    }
    for( int i = 0; i< this->GetNumberOfInputConnections( INPUT_ROI ); i++ ) {
        svkXML* imageXML = svkXML::SafeDownCast(this->GetPortMapper()->GetAlgorithmInputPort( INPUT_ROI, i ));
        if( imageXML != NULL ) {
            svkImageData* image = this->ReadImageFromXML( imageXML->GetValue() );
            string label = imageXML->GetValue()->GetAttribute("label");
            svkImageData* filteredData = this->ApplyFiltersFromXML( image, imageXML->GetValue() );
            this->rois[label] = svkMriImageData::SafeDownCast(filteredData);
            this->rois[label]->Register(this);
        }
    }
}


/*!
 *  This method takes an svkImageData object and a vtkXMLDataElement containing the filter tags
 *  to be applied to the data. It then returns the result of the pipeline.
 */
svkImageData* svkImageStatisticsCollector::ApplyFiltersFromXML( svkImageData* inputImage, vtkXMLDataElement* imageElement )
{
    svkImageData* filteredData = NULL;
    if( this->pipelineFilter != NULL ) {
        this->pipelineFilter->Delete();
    }
    this->pipelineFilter = svkImageAlgorithmPipeline::New();
    vtkIndent indent;
    imageElement->PrintXML(cout, indent);
    vtkXMLDataElement* filters = imageElement->FindNestedElementWithName(this->pipelineFilter->GetPortMapper()->GetXMLTagForAlgorithm().c_str());
    if( filters != NULL ) {
        this->pipelineFilter->SetInput(svkImageAlgorithmPipeline::INPUT_IMAGE, inputImage);
        this->pipelineFilter->SetInputPortsFromXML( filters );
        this->pipelineFilter->Update();
        filteredData = this->pipelineFilter->GetOutput();
    } else {
        filteredData = inputImage;
    }
    return filteredData;

}
