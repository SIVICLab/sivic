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


#include <svkXMLImagePipeline.h>

using namespace svk;

vtkCxxRevisionMacro(svkXMLImagePipeline, "$Rev$");
vtkStandardNewMacro(svkXMLImagePipeline);

//! Constructor
svkXMLImagePipeline::svkXMLImagePipeline()
{
    this->xmlInterpreter->InitializeInputPort( INPUT_IMAGE, "INPUT_IMAGE", svkXMLInputInterpreter::SVK_MR_IMAGE_DATA);
    cout << "Constructing svkXMLImagePipeline." << endl;
    this->lastFilter = NULL;
    this->reader = NULL;
}


//! Destructor
svkXMLImagePipeline::~svkXMLImagePipeline()
{
    if( this->lastFilter != NULL ) {
        this->lastFilter->Delete();
        this->lastFilter = NULL;
    }
    if( this->reader != NULL ) {
        this->reader->Delete();
        this->reader = NULL;
    }
}


/*!
 * Method sets the XML configuration file.
 */
void svkXMLImagePipeline::SetXMLPipeline( vtkXMLDataElement* pipeline )
{
    this->pipeline = pipeline;
    this->reader = NULL;
    vtkIndent indent;
    cout << "COFIGURATION SET BY XML:" << endl;
    this->pipeline->PrintXML(cout, indent);
}


/*!
 *  RequestData pass the input through the algorithm, and copies the dcos and header
 *  to the output.
 */
int svkXMLImagePipeline::RequestData( vtkInformation* request,
                                    vtkInformationVector** inputVector,
                                    vtkInformationVector* outputVector )
{
    svkImageData* filteredImage = this->xmlInterpreter->GetMRImageInputPortValue(INPUT_IMAGE);
    if( pipeline != NULL ) {
        int numberOfFilters = pipeline->GetNumberOfNestedElements();
        for( int i = 0; i < numberOfFilters; i++ ) {
            vtkXMLDataElement* filterParameters = pipeline->GetNestedElement(i);

            // Get the next filter
            svkXMLImageAlgorithm* filter = GetAlgorithmForFilterName(filterParameters->GetName());
            if( filter != NULL) {
                vtkIndent indent;
                filterParameters->PrintXML(cout, indent);
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
            } else {
                cout << "COULD NOT DETERMINE FILTER!!" << endl;
            }
        }
    }
    this->GetOutputDataObject(0)->DeepCopy( filteredImage );
    return 1;
}


/*!
 * Factory method for getting the algorithms to be used in the statistics collection.
 */
svkXMLImageAlgorithm* svkXMLImagePipeline::GetAlgorithmForFilterName( string filterName )
{
    svkXMLImageAlgorithm* algorithm;
    // TODO: Replace this with an algorithm factory method...
    if( filterName == "svkImageThreshold") {
        algorithm = svkImageThreshold::New();
    }
    return algorithm;
}
