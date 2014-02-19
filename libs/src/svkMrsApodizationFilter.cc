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



#include <svkMrsApodizationFilter.h>


using namespace svk;


vtkCxxRevisionMacro(svkMrsApodizationFilter, "$Rev$");
vtkStandardNewMacro(svkMrsApodizationFilter);


svkMrsApodizationFilter::svkMrsApodizationFilter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

}


/*!
 *  Destructor. 
 */
svkMrsApodizationFilter::~svkMrsApodizationFilter()
{
}


/*!
 * Sets the window to be used for the apodization.
 */
void svkMrsApodizationFilter::SetWindow( vtkFloatArray* window )
{
    this->window = window;
}


/*! 
 *
 */
int svkMrsApodizationFilter::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    return 1;
}


/*! 
 *  Each data array in the input will be multiplied by the window.
 */
int svkMrsApodizationFilter::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    svkDcmHeader* hdr = data->GetDcmHeader();
    int numPoints    = hdr->GetIntValue( "DataPointColumns" );

    int cols            = hdr->GetIntValue( "Columns" );
    int rows            = hdr->GetIntValue( "Rows" );
    int slices          = hdr->GetNumberOfSlices();
    int numChannels     = hdr->GetNumberOfCoils();
    int numTimePts      = hdr->GetNumberOfTimePoints();
    double* windowTuple = NULL;
    double* specTuple   = NULL;

    for( int channel = 0; channel < numChannels; channel++ ) { 
        for( int timePt = 0; timePt < numTimePts; timePt++ ) { 
            for (int z = 0; z < slices; z++) {
                for (int y = 0; y < rows; y++) {
                    for (int x = 0; x < cols; x++) {

                        vtkFloatArray* spectrum = 
                            vtkFloatArray::SafeDownCast( svkMrsImageData::SafeDownCast(data)->GetSpectrum( x, y, z, timePt, channel) );
                        //  Iterate over frequency points in spectrum and apply phase the window:
                        for ( int i = 0; i < numPoints; i++ ) {
                            windowTuple = this->window->GetTuple( i );
                            specTuple = spectrum->GetTuple( i );
                            spectrum->SetTuple2( i, specTuple[0] * windowTuple[0], specTuple[1] * windowTuple[1] ); 
                        }
                    }
                }
            }
        }
    }

    return 1; 
} 


/*!
 *
 */
int svkMrsApodizationFilter::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


