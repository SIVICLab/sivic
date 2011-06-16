/*
 *  Copyright © 2009-2011 The Regents of the University of California.
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



#include <svkMrsZeroFill.h>


using namespace svk;


vtkCxxRevisionMacro(svkMrsZeroFill, "$Rev$");
vtkStandardNewMacro(svkMrsZeroFill);


svkMrsZeroFill::svkMrsZeroFill()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    this->numTargetPts = 0;

}


/*!
 *  Destructor. 
 */
svkMrsZeroFill::~svkMrsZeroFill()
{
}


/*! 
 *
 */
int svkMrsZeroFill::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    return 1;
}


/*! 
 *
 */
int svkMrsZeroFill::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 

    svkDcmHeader* hdr = data->GetDcmHeader();
    int numSpecPts    = hdr->GetIntValue( "DataPointColumns" );

    // If the numTargetPts is < current dimensionality, do nothing:
    if ( this->numTargetPts <= numSpecPts ) { 
        return 0; // is this the correct return value ?
    }

    int cols          = hdr->GetIntValue( "Columns" );
    int rows          = hdr->GetIntValue( "Rows" );
    int slices        = hdr->GetNumberOfSlices();
    int numChannels   = hdr->GetNumberOfCoils();
    int numTimePts    = hdr->GetNumberOfTimePoints();

    float cmplxPt[2];
    cmplxPt[0] = 0.;
    cmplxPt[1] = 0.;

    for( int channel = 0; channel < numChannels; channel++ ) { 
        for( int timePt = 0; timePt < numTimePts; timePt++ ) { 
            for (int z = 0; z < slices; z++) {
                for (int y = 0; y < rows; y++) {
                    for (int x = 0; x < cols; x++) {

                        vtkFloatArray* spectrum = 
                            vtkFloatArray::SafeDownCast( svkMrsImageData::SafeDownCast(data)->GetSpectrum( x, y, z, timePt, channel) );

                        //  Iterate over frequency points in spectrum and apply phase correction:
                        for ( int freq = numSpecPts; freq < this->numTargetPts; freq++ ) {

                            spectrum->InsertTuple(freq, cmplxPt);

                        }
                    }
                }
            }
        }
    }

    hdr->SetValue( "DataPointColumns", this->numTargetPts);

    return 1; 
} 


/*! 
 *  Sets the total number of spectral points after zero-filling. 
 *  If the number of points is less than the current number of 
 *  spectral points the algorithm will do nothing.    
 */
void svkMrsZeroFill::SetNumPoints(int numPts)
{
    this->numTargetPts = numPts; 
}


/*!
 *
 */
int svkMrsZeroFill::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


