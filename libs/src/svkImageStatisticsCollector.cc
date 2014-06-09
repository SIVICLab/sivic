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
    ///this->SetNumberOfOutputPorts(1);
    bool required = true;
    bool repeatable = true;
    this->GetPortMapper()->InitializeInputPort( INPUT_IMAGE, "INPUT_IMAGE", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, required, repeatable );
    this->GetPortMapper()->InitializeInputPort( INPUT_ROI, "INPUT_ROI", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, required, repeatable );
    this->GetPortMapper()->InitializeInputPort( MEASURES, "measures", svkAlgorithmPortMapper::SVK_XML, required );
    cout << "Constructing svkImageStatisticsCollector." << endl;
    vtkInstantiator::RegisterInstantiator("svkXML",  svkXML::NewObject);
}


//! Destructor
svkImageStatisticsCollector::~svkImageStatisticsCollector()
{
}


/*!
*  Get's the output as a vtkXMLDataElement.
*/
vtkXMLDataElement* svkImageStatisticsCollector::GetOutput()
{
   vtkXMLDataElement* output = NULL;
   svkXML* outputDataObject = svkXML::SafeDownCast(this->GetOutputDataObject(0));
   if( outputDataObject != NULL ) {
       output = outputDataObject->GetValue();
   }
   return output;
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
            svkImageStatistics* statsCalculator = svkImageStatistics::New();
            statsCalculator->SetInputPortsFromXML(this->GetPortMapper()->GetXMLInputPortValue(MEASURES)->GetValue());
            svkMriImageData* image = this->GetPortMapper()->GetMRImageInputPortValue(INPUT_IMAGE, imageIndex);
            svkMriImageData* roi   = this->GetPortMapper()->GetMRImageInputPortValue(INPUT_ROI, roiIndex);
            statsCalculator->GetPortMapper()->SetAlgorithmInputPort(svkImageStatistics::INPUT_IMAGE, image );
            statsCalculator->GetPortMapper()->SetAlgorithmInputPort(svkImageStatistics::INPUT_ROI, roi);
            statsCalculator->Update();
            vtkXMLDataElement* nextResult = vtkXMLDataElement::New();
            nextResult->SetName("results");
            string imageLabel = image->GetDcmHeader()->GetStringValue("SeriesDescription");
            string roiLabel = roi->GetDcmHeader()->GetStringValue("SeriesDescription");
            cout << "Image: " << imageLabel << endl;
            cout << "ROI: " << roiLabel << endl;
            svkUtils::CreateNestedXMLDataElement( nextResult, "ROI",   imageLabel);
            svkUtils::CreateNestedXMLDataElement( nextResult, "IMAGE", roiLabel);
            vtkXMLDataElement* statistics = statsCalculator->GetOutput();
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
            statsCalculator->Delete();
        }
    }

    vtkIndent indent;
    results->PrintXML(cout , indent);
}
