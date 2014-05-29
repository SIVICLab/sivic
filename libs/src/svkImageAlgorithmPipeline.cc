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


#include <svkImageAlgorithmPipeline.h>

using namespace svk;

vtkCxxRevisionMacro(svkImageAlgorithmPipeline, "$Rev$");
vtkStandardNewMacro(svkImageAlgorithmPipeline);

//! Constructor
svkImageAlgorithmPipeline::svkImageAlgorithmPipeline()
{
    this->SetNumberOfInputPorts(2);
    this->GetPortMapper()->InitializeInputPort( INPUT_IMAGE, "INPUT_IMAGE", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
    this->GetPortMapper()->InitializeInputPort( PIPELINE, "pipeline", svkAlgorithmPortMapper::SVK_XML);
    this->GetPortMapper()->SetXMLInputPortPrefix("svk");
    this->lastFilter = NULL;
    this->reader = NULL;
}


//! Destructor
svkImageAlgorithmPipeline::~svkImageAlgorithmPipeline()
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
 *  Creates the algorithm pipeline and executes it.
 */
int svkImageAlgorithmPipeline::RequestData( vtkInformation* request,
                                    vtkInformationVector** inputVector,
                                    vtkInformationVector* outputVector )
{
    svkImageData* filteredImage = this->GetPortMapper()->GetMRImageInputPortValue(INPUT_IMAGE);
    vtkXMLDataElement* pipeline = this->GetPortMapper()->GetXMLInputPortValue(PIPELINE)->GetValue();
    if( pipeline != NULL ) {
        int numberOfFilters = pipeline->GetNumberOfNestedElements();
        for( int i = 0; i < numberOfFilters; i++ ) {
            vtkXMLDataElement* filterParameters = pipeline->GetNestedElement(i);

            // Get the next filter
            svkImageAlgorithmWithPortMapper* filter = GetAlgorithmForFilterName(filterParameters->GetName());
            string xsd = filter->GetPortMapper()->GetXSD();
            cout << "############################   XSD   ##########################" << endl << endl;
            cout << xsd << endl;
            cout << "############################   XSD   ##########################" << endl << endl;
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
            } else {
                cout << "ERROR: Did not recognize algorithm:"<< filterParameters->GetName() << "!" << endl;
            }
        }
    }
    this->GetOutputDataObject(0)->ShallowCopy( filteredImage );
    return 1;
}


/*!
 * Factory method for getting the algorithms by class name.
 */
svkImageAlgorithmWithPortMapper* svkImageAlgorithmPipeline::GetAlgorithmForFilterName( string filterName )
{
    //TODO: Create algorithm factory class like svkImageReaderFactory
    cout << "Filter name:" << filterName << endl;
    svkImageAlgorithmWithPortMapper* algorithm = NULL;
    if( filterName == "svkAlgorithm:svkImageThreshold") {
        algorithm = svkImageThreshold::New();
    }
    return algorithm;
}
