/*
 *  Copyright © 2009-2017 The Regents of the University of California.
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



#include <svkAlgoTemplate.h>


using namespace svk;


//vtkCxxRevisionMacro(svkAlgoTemplate, "$Rev$");
vtkStandardNewMacro(svkAlgoTemplate);


/*!
 *  Constructor.  Initialize any member variables. 
 */
svkAlgoTemplate::svkAlgoTemplate()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    //  Initialize any member variables
}


/*!
 *  Clean up any allocated member variables. 
 */
svkAlgoTemplate::~svkAlgoTemplate()
{
}


/*! 
 *  This method is called during pipeline execution.  This is where you should implement your algorithm. 
 */
int svkAlgoTemplate::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    //  Get pointer to input data set. 
    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 

    //  Get pointer to data's meta-data header (DICOM object). 
    svkDcmHeader* hdr = mrsData->GetDcmHeader();  

    //  Lookup any data set attributes from header required for algorithm (See DICOM IOD for field names):
    int numSpecPts = hdr->GetIntValue( "DataPointColumns" );
    int cols       = hdr->GetIntValue( "Columns" );
    int rows       = hdr->GetIntValue( "Rows" );
    int slices     = hdr->GetNumberOfSlices();

    float cmplxPt[2]; 

    //  Iterate through 3D spatial locations
    for (int z = 0; z < slices; z++) {
        for (int y = 0; y < rows; y++) {
            for (int x = 0; x < cols; x++) {

                vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( mrsData->GetSpectrum( x, y, z) );

                //  Iterate over frequency points in spectrum and do something:
                for ( int freq = 0; freq < numSpecPts; freq++ ) {

                    spectrum->GetTuple(freq, cmplxPt);

                    cmplxPt[0] = cmplxPt[0] + 0;     
                    cmplxPt[1] = cmplxPt[1] + 0;     

                    spectrum->SetTuple(freq, cmplxPt); 

                }

            }
        }
    }

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->Update();

    return 1; 
} 


/*!
 *  Set the input data type, e.g. svkMrsImageData for an MRS algorithm.
 */
int svkAlgoTemplate::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


