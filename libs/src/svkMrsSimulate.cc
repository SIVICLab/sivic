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



#include <svkMrsSimulate.h>


using namespace svk;


//vtkCxxRevisionMacro(svkMrsSimulate, "$Rev$");
vtkStandardNewMacro(svkMrsSimulate);


svkMrsSimulate::svkMrsSimulate()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");
    this->type = 3;

}


/*!
 *  Destructor. 
 */
svkMrsSimulate::~svkMrsSimulate()
{
}


/*! 
 *
 */
int svkMrsSimulate::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    return 1;
}


/*! 
 *  Each data array in the input will be multiplied by the window.
 */
int svkMrsSimulate::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    svkDcmHeader* hdr = data->GetDcmHeader();
    int numPoints    = hdr->GetIntValue( "DataPointColumns" );

    //  Get the Dimension Index and index values  
    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector loopVector = dimensionVector; 
    

    //  GetNumber of cells in the image:
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );

    double spec[2]; 

    float T = 1./8.;     // (s-1) (damping factor or linewidth factor)
    float wres1 = numPoints/2.;  // (s-1)  resonant frequency
    float wres2 = numPoints/3.;  // (s-1)  resonant frequency
    for (int cellID = 0; cellID < numCells; cellID++ ) {

        svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimensionVector, &loopVector, cellID );
        int colIndex = svkDcmHeader::GetDimensionVectorValue(&loopVector, svkDcmHeader::COL_INDEX); 
        int rowIndex = svkDcmHeader::GetDimensionVectorValue(&loopVector, svkDcmHeader::ROW_INDEX); 

        vtkFloatArray* spectrum = static_cast<vtkFloatArray*>(
            svkMrsImageData::SafeDownCast(data)->GetSpectrum( cellID )
        );

        //  square image: 
        //if (( colIndex > 4 && colIndex <= 7 ) &&  (rowIndex > 4 && rowIndex <= 7 )) { 
        //  DC: 
        //this->type = 0; // 2 lorentzian frequency domain peaks at wres1 and wres2 in every spatial voxel. 
        //this->type = 1; // 1 lorentzian time domain peaks at 0 frequency.  FT should produce peakcentered at 0 frequency
        
        if ( this->type == 0 ) { 
            // kludge to create "k=0 DC k-space for FORTRAN . symmetric sampling 10x10
            //if (( colIndex > 3 && colIndex < 6 ) &&  (rowIndex > 3 && rowIndex < 6 )) { 
            //  simulate complex frequency spectrum
            for ( int w = 0; w < numPoints; w++ ) {
                float wmwres1 = (w - wres1); 
                float wmwres2 = (w - wres2); 
                spec[0] = T / ( T * T + wmwres1 *wmwres1 ) + T / ( T * T + wmwres2 *wmwres2 ); 
                spec[1] = wmwres1 / (T * T + wmwres1 * wmwres1)  +  wmwres2 / (T * T + wmwres2 * wmwres2); 
                spectrum->SetTuple2( w, spec[0], spec[1] ); 
            }
        } else if ( this->type == 1 ) { 
            cout << "===========" << endl;
            for ( int w = 0; w < numPoints; w++ ) {
                spec[0] = 1000 * exp(-1 * (T/100) * w); 
                spec[1] = 0; 
                spectrum->SetTuple2( w, spec[0], spec[1] ); 
            }
        } else if ( this->type == 3 ) { 
            //  DC offset in time -> single frequency at 0 hz
            for ( int w = 0; w < numPoints; w++ ) {
                spec[0] = 1000;
                spec[1] =    0; 
                spectrum->SetTuple2( w, spec[0], spec[1] ); 
            }
        } else {
        }
    }

    return 1; 
} 


/*!
 *
 */
int svkMrsSimulate::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


