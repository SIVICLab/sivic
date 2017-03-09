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



#include <svkFreqCorrect.h>


using namespace svk;


vtkStandardNewMacro(svkFreqCorrect);


/*!
 *
 */
svkFreqCorrect::svkFreqCorrect()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    this->units         = svkSpecPoint::PTS;
    this->globalShift   = 0.0;

    this->SetNumberOfInputPorts(2); //1 mandatory, 1 optional 

}


/*!
 *
 */
svkFreqCorrect::~svkFreqCorrect()
{
}


/*! 
 *
 */
int svkFreqCorrect::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    return 1;
}


/*! 
 *  Frequency shift data, either using a shiftMap, or by applying a global shift 
 */
int svkFreqCorrect::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    int retVal = 0; 

    //  If a map was provided, apply frequency corrections from that, otherwise apply a global correction
    if ( this->GetImageDataInput(1) ) {
        retVal = this->ApplyFrequencyCorrectionMap(); 
    } else {
        retVal = this->ApplyGlobalCorrection(); 
    }

    svkMrsImageData::SafeDownCast(this->GetImageDataInput(0))->GetProvenance()->AddAlgorithm( this->GetClassName() );
    return retVal; 

}


/*! 
 *  If a phase map was provided, apply values 
 *  to each voxel.  
 */
int svkFreqCorrect::ApplyFrequencyCorrectionMap( )
{

    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) ); 
    svkMriImageData* mriData = svkMriImageData::SafeDownCast( this->GetImageDataInput(1) ); 

    svkDcmHeader::DimensionVector mrsDimensionVector = mrsData->GetDcmHeader()->GetDimensionIndexVector();
    int numMRSCells = svkDcmHeader::GetNumberOfCells( &mrsDimensionVector );

    svkDcmHeader::DimensionVector mriDimensionVector = mriData->GetDcmHeader()->GetDimensionIndexVector();
    int numMRICells = svkDcmHeader::GetNumberOfCells( &mriDimensionVector );

    if ( numMRICells != numMRSCells ) {
        cout << "ERROR: Number of cells in MRS object does not match number in phase map image. " << endl;
        exit(1); 
    }

    vtkDataArray* freqShiftValues = mriData->GetPointData()->GetArray(0); 
    
    for (int cellID = 0; cellID < numMRSCells; cellID++) {

        float shift = freqShiftValues->GetTuple1(cellID); 
        //cout << "PHI1: " << cellID << " = " << phi0 << endl;
        vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( mrsData->GetSpectrum( cellID ) ); 
        //this->ZeroOrderPhase(phi0Radians, spectrum); 

    }

    return 1;

}


/*! 
 *  Apply global shift
 */
int svkFreqCorrect::ApplyGlobalCorrection( )
{
    if ( this->globalShift == 0 ) {
        return 1; 
    } 

    //  =================================================    
    //  Iterate through spectral data from all cells.  
    //  =================================================    
    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 

    svkDcmHeader::DimensionVector mrsDimensionVector = mrsData->GetDcmHeader()->GetDimensionIndexVector();
    int numMRSCells = svkDcmHeader::GetNumberOfCells( &mrsDimensionVector );

    vtkCellData* cellData = mrsData->GetCellData();   
    int numFrequencyPoints = cellData->GetNumberOfTuples();

    float re;
    float im;
    float newRe;
    float newIm;
    float* specPtr;
    float* origSpecPtr;

    for (int cellID = 0; cellID < numMRSCells; cellID++) {

        vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( mrsData->GetSpectrum( cellID ) ); 
        vtkFloatArray* originalSpectrum = vtkFloatArray::New(); 
        originalSpectrum->DeepCopy(spectrum); 

        specPtr     = spectrum->GetPointer(0); 
        origSpecPtr = originalSpectrum->GetPointer(0); 

        for (int i = 0; i < numFrequencyPoints; i++) {

            int shiftedIndex = i + this->globalShift; 
            if ( shiftedIndex > 0 && shiftedIndex < numFrequencyPoints ) {
                newRe = origSpecPtr[ 2*shiftedIndex ]; 
                newIm = origSpecPtr[ 2*shiftedIndex + 1];
            } else {
                newRe = 0.; 
                newIm = 0.; 
            }    

            // And apply the phase values
            specPtr[2*i]   = newRe; 
            specPtr[2*i+1] = newIm;  
        }
    }

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    return 1; 

} 



/*! 
 *  Sets the zero order phase to apply to spectra.  Phase is input in degrees. 
 */
void svkFreqCorrect::SetUnits( svkSpecPoint::UnitType units )
{
    this->units = units; 
}


/*! 
 *  This returns the global frequency shfit value. 
 */
float svkFreqCorrect::GetGlobalFrequencyShift()
{
    return this->globalShift;
}


/*! 
 *  Sets the global frequency shift 
 */
void svkFreqCorrect::SetGlobalFrequencyShift( float shift )
{
    this->globalShift = shift;
}


/*!
 *  Port 0 is the required input port for the MRS data to be phased. 
 *  Port 1 is the optional input port for a phase map of values to apply 
 */
int svkFreqCorrect::FillInputPortInformation( int port, vtkInformation* info )
{
    if ( port == 0 ) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    }
    if ( port == 1 ) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
        info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }

    return 1;
}


/*!
 *  PrintSelf method calls parent class PrintSelf, then prints the dcos.
 *
 */
void svkFreqCorrect::PrintSelf( ostream &os, vtkIndent indent )
{
    Superclass::PrintSelf( os, indent );
}



