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



#include <svkPhaseSpec.h>


using namespace svk;


//vtkCxxRevisionMacro(svkPhaseSpec, "$Rev$");
vtkStandardNewMacro(svkPhaseSpec);


svkPhaseSpec::svkPhaseSpec()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    this->phase0 = 0.0;
    this->phase0Target = 0.0;
    this->linearPhase = 0.0;
    this->linearPhaseTarget = 0.0;
    cmplxSpec = new float[2];

    //  Start index for update extent
    this->updateExtent[0] = -1;  
    this->updateExtent[1] = -1;  
    this->updateExtent[2] = -1;  
    this->updateExtent[3] = -1;  
    this->updateExtent[4] = -1;  
    this->updateExtent[5] = -1;  
    this->linearPhasePivot = 0;
    this->linearPhasePivotTarget = 0;

    this->channelToPhase = -1; 

    this->SetNumberOfInputPorts(2); //1 mandatory, 1 optional 

}


svkPhaseSpec::~svkPhaseSpec()
{
    delete[] cmplxSpec; 
}


/*! 
 *
 */
int svkPhaseSpec::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    int* wholeExtent = this->GetOutputInformation(0)->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT());
    
    bool useWholeExtent = false; 
    //  If the specified update extent is outside the whole extent, just use
    //  the whole extent:

    //  Lower bounds of extent:
    for (int i = 0; i < 6; i+=2) {
        if ( updateExtent[i] < wholeExtent[i] ) {
            useWholeExtent = true;
        }
    }

    //  upper bounds of extent:
    for (int i = 1; i < 6; i+=2) {
        if ( updateExtent[i] > wholeExtent[i] ) {
            useWholeExtent = true;
        }
    }

    if (useWholeExtent) {
        wholeExtent[1] =  wholeExtent[1] - 1;  
        wholeExtent[3] =  wholeExtent[3] - 1;  
        wholeExtent[5] =  wholeExtent[5] - 1;  
        for (int i = 0; i < 6; i++) {
            vtkDebugMacro(<<this->GetClassName() << " Whole Extent " << wholeExtent[i]);
            this->updateExtent[i] = wholeExtent[i];  
        }
    }
    return 1;
}


/*! 
 *  Phase data, either using a phaseMap, or by applying a global phase
 */
int svkPhaseSpec::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    int retVal = 0; 

    if ( this->GetImageDataInput(1) ) {
        retVal = this->ApplyPhaseMap(); 
    } else {
        retVal = this->ApplyGlobalPhase(); 
    }

    svkMrsImageData::SafeDownCast(this->GetImageDataInput(0))->GetProvenance()->AddAlgorithm( this->GetClassName() );
    return retVal; 

}


/*! 
 *  If a phase map was provided, apply values 
 *  to each voxel.  
 */
int svkPhaseSpec::ApplyPhaseMap( )
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

    vtkDataArray* phaseValues = mriData->GetPointData()->GetArray(0); 
    
    for (int cellID = 0; cellID < numMRSCells; cellID++) {

        float phi0 = phaseValues->GetTuple1(cellID); 
        float phi0Radians = phi0 * vtkMath::Pi()/180.; 
        //cout << "PHI1: " << cellID << " = " << phi0 << endl;
        vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( mrsData->GetSpectrum( cellID ) ); 
        this->ZeroOrderPhase(phi0Radians, spectrum); 

    }

    return 1;

}


/*! 
 *  Apply global phase values
 */
int svkPhaseSpec::ApplyGlobalPhase( )
{
    if ( this->phase0Target == this->phase0 &&
         this->linearPhaseTarget == this->linearPhase && 
         this->linearPhasePivotTarget == this->linearPhasePivot 
    ) {
        return 1; 
    } 

    float deltaPhase0 =  this->phase0Target - this->GetPhase0();
    float deltaLinearPhase =  this->linearPhaseTarget - this->GetLinearPhase();

    float cosPhase = static_cast<float>( cos( deltaPhase0 ) );
    float sinPhase = static_cast<float>( sin( deltaPhase0 ) );


    //  =================================================    
    //  Iterate through spectral data from all cells.  
    //  Eventually for performance I should do this by visible
    //  =================================================    
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 

    //  TODO: Set Initial Extent, and catch up with invisible extents after rerendering (modified update).    
    int spatialDims[3]; 
    data->GetDimensions( spatialDims );
    spatialDims[0] -= 1;
    spatialDims[1] -= 1;
    spatialDims[2] -= 1;
  
    vtkCellData* cellData = data->GetCellData();   
    int numFrequencyPoints = cellData->GetNumberOfTuples();
    int numChannels  = data->GetNumberOfChannels();
    int numTimePts = data->GetDcmHeader()->GetNumberOfTimePoints();

    float re;
    float im;
    float* specPtr;

    int firstChannel = 0; 
    int lastChannel = numChannels; 

    if ( this->channelToPhase >= 0 ) {
        firstChannel = this->channelToPhase; 
        lastChannel = this->channelToPhase + 1; 
    }

    //  =================================================    
    //  Let's create the phase array for the linear phase
    //  =================================================    
    vtkImageComplex* linearPhaseArray = new vtkImageComplex[ numFrequencyPoints ];
    vtkImageComplex* reversePhaseArray = NULL;

    //  =================================================    
	//  If the pivot point has changed we'll need reverse 
    //  the current phase, then apply the new phase
    //  =================================================    
    if ( this->linearPhasePivotTarget == this->linearPhasePivot ) {

        svkSpecUtils::CreateLinearPhaseShiftArray(
                numFrequencyPoints, 
                linearPhaseArray, 
                deltaLinearPhase, 
                this->linearPhasePivot);

    } else {

        reversePhaseArray = new vtkImageComplex[ numFrequencyPoints ];

        svkSpecUtils::CreateLinearPhaseShiftArray(
                numFrequencyPoints, 
                reversePhaseArray, 
                -1 * this->linearPhase, 
                this->linearPhasePivot);

        svkSpecUtils::CreateLinearPhaseShiftArray(
                numFrequencyPoints, 
                linearPhaseArray, 
                this->linearPhaseTarget, 
                this->linearPhasePivotTarget);

        cosPhase = static_cast<float>( cos( this->phase0Target ) );
        sinPhase = static_cast<float>( sin( this->phase0Target ) );

    }

    float reverseCosPhase = static_cast<float>( cos( -1*this->phase0 ) );
    float reverseSinPhase = static_cast<float>( sin( -1*this->phase0 ) );

    for( int channel = firstChannel; channel < lastChannel; channel++ ) { 
        for( int timePt = 0; timePt < numTimePts; timePt++ ) { 
            for (int z = this->updateExtent[4]; z <= this->updateExtent[5]; z++) {
                for (int y = this->updateExtent[2]; y <= this->updateExtent[3]; y++) {
                    for (int x = this->updateExtent[0]; x <= this->updateExtent[1]; x++) {
                        vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast(data->GetSpectrum( x, y, z, timePt, channel ));
                        specPtr = spectrum->GetPointer(0); 

                        for (int i = 0; i < numFrequencyPoints; i++) {

                            //  =================================================    
                            //  If the pivot point has changed we'll need reverse 
                            //  the current phase, then apply the new phase
                            //  TODO: If possible make this algorithm not in place 
                            //  so that we do not have to reverse this.
                            //  =================================================    
                        	if( reversePhaseArray != NULL ) {
								re = specPtr[2*i] * reverseCosPhase - specPtr[2*i+1] * reverseSinPhase;
                                im = specPtr[2*i] * reverseSinPhase + specPtr[2*i+1] * reverseCosPhase;

                                // And apply the phase values
                                specPtr[2*i]   = ( reversePhaseArray[i].Real*re - reversePhaseArray[i].Imag*im );
                                specPtr[2*i+1] = ( reversePhaseArray[i].Real*im + reversePhaseArray[i].Imag*re );
                        	}

                            re = specPtr[2*i] * cosPhase - specPtr[2*i+1] * sinPhase;
                            im = specPtr[2*i] * sinPhase + specPtr[2*i+1] * cosPhase;

                            // And apply the phase values
                            specPtr[2*i]   = ( linearPhaseArray[i].Real*re - linearPhaseArray[i].Imag*im );
                            specPtr[2*i+1] = ( linearPhaseArray[i].Real*im + linearPhaseArray[i].Imag*re );
                        }
                    }
                }

            }
        }
    }
    delete[] linearPhaseArray;
    if( reversePhaseArray != NULL ) {
        delete[] reversePhaseArray;
    }

    this->phase0 =  this->phase0Target;
    this->linearPhase =  this->linearPhaseTarget;
    this->linearPhasePivot = this->linearPhasePivotTarget;
    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    return 1; 

} 


/*! 
 *  Applies a zero order phase to the complex values input tuple, representing the re and im
 *  intensity at a given frequency point. 
 *  inputs: 
 *      phi0, phase in radians
 *      cmplxPt, complex float array with 2 elements: real, imaginary 
 */
void svkPhaseSpec::ZeroOrderPhase(float phi0, float* cmplxPt)
{
    float cosPhase = static_cast<float>( cos( phi0) );
    float sinPhase = static_cast<float>( sin( phi0) );
    float re = ( cmplxPt[0] * cosPhase ) - ( cmplxPt[1] * sinPhase );
    float im = ( cmplxPt[0] * sinPhase ) + ( cmplxPt[1] * cosPhase );
    cmplxPt[0] = re; 
    cmplxPt[1] = im; 

}


/*! 
 *  Applies a zero order phase to the complex spectrum. 
 *  inputs: 
 *      phi0, phase in radians
 *      spectrum 
 */
void svkPhaseSpec::ZeroOrderPhase(float phi0, vtkFloatArray* spectrum)
{

    float cosPhase = static_cast<float>( cos( phi0) );
    float sinPhase = static_cast<float>( sin( phi0) );

    int numPoints = spectrum->GetNumberOfTuples();     

    float cmplxPt[2]; 
    for ( int i = 0; i < numPoints; i++ ) {

        spectrum->GetTuple(i, cmplxPt);
        svkPhaseSpec::ZeroOrderPhase( phi0, cmplxPt ); 
        spectrum->SetTuple(i, cmplxPt); 

    }

}


/*! 
 *  Applies  first order phase to the complex spectrum. 
 *  inputs: 
 *      phi0, phase in radians
 *      phi1, phase in radians (pivot at center of spectrum (assume 0Hz 
 *      spectrum 
 */
void svkPhaseSpec::FirstOrderPhase(float phi1, int pivotPoint, vtkFloatArray* spectrum)
{


    //  Get zero order phase factors, with 0 pivot.  
    int numPoints = spectrum->GetNumberOfTuples();     

    vtkImageComplex* linearPhaseArray = new vtkImageComplex[ numPoints ];
    svkSpecUtils::CreateLinearPhaseShiftArray( 
                numPoints, 
                linearPhaseArray, 
                phi1, 
                pivotPoint);

    double cmplxPt[2]; 
    for ( int i = 0; i < numPoints; i++ ) {

        spectrum->GetTuple(i, cmplxPt);

        //  apply first order phase
        float re1 = ( linearPhaseArray[i].Real * cmplxPt[0] - linearPhaseArray[i].Imag * cmplxPt[1] );
        float im1 = ( linearPhaseArray[i].Real * cmplxPt[1] + linearPhaseArray[i].Imag * cmplxPt[0] );

        //  update the spectrum; 
        cmplxPt[0] = re1; 
        cmplxPt[1] = im1; 
        spectrum->SetTuple(i, cmplxPt); 

    }

}


/*! 
 *  Applies a zero and first order phase to the complex spectrum. 
 *  inputs: 
 *      phi0, phase in radians
 *      phi1, phase in radians (pivot at center of spectrum (assume 0Hz 
 *      spectrum 
 */
void svkPhaseSpec::FirstOrderPhase( float phi0, vtkImageComplex* linearPhaseArray, vtkFloatArray* spectrum)
{

    //  Get global zero order phase factor
    float cosPhase = static_cast<float>( cos( phi0) );
    float sinPhase = static_cast<float>( sin( phi0) );

    int numPoints = spectrum->GetNumberOfTuples();     
    double cmplxPt[2]; 
    for ( int i = 0; i < numPoints; i++ ) {

        spectrum->GetTuple(i, cmplxPt);

        //  apply zero-order phase
        float re0 = ( cmplxPt[0] * cosPhase ) - ( cmplxPt[1] * sinPhase );
        float im0 = ( cmplxPt[0] * sinPhase ) + ( cmplxPt[1] * cosPhase );
        //cout << "phased values 0 " << re0 << endl;
        //cout << "lpa: " << linearPhaseArray[i].Real << endl;

        //  apply first order phase
        float re1 = ( linearPhaseArray[i].Real * re0 - linearPhaseArray[i].Imag * im0 );
        float im1 = ( linearPhaseArray[i].Real * im0 + linearPhaseArray[i].Imag * re0 );

        //  update the spectrum; 
        cmplxPt[0] = re1; 
        cmplxPt[1] = im1; 
        //cout << "phased values1 " << re1 << endl;
        spectrum->SetTuple(i, cmplxPt); 

    }


}


/*! 
 *  Applies a zero and first order phase to the complex spectrum. 
 *  inputs: 
 *      phi0, phase in radians
 *      phi1, phase in radians (pivot at center of spectrum (assume 0Hz 
 *      spectrum 
 */
void svkPhaseSpec::FirstOrderPhase(float phi0, float phi1, int pivotPoint, vtkFloatArray* spectrum)
{

    //  Get zero order phase factors, with 0 pivot.  
    int numPoints = spectrum->GetNumberOfTuples();     

    vtkImageComplex* linearPhaseArray = new vtkImageComplex[ numPoints ];
    svkSpecUtils::CreateLinearPhaseShiftArray( 
                numPoints, 
                linearPhaseArray, 
                phi1, 
                pivotPoint);

    svkPhaseSpec::FirstOrderPhase(phi0, linearPhaseArray, spectrum); 

}


/*! 
 *  Sets the zero order phase to apply to spectra.  Phase is input in degrees. 
 */
void svkPhaseSpec::SetPhase0(float phase0)
{
    this->phase0Target = phase0 * 3.14159/180.0; 
}


/*! 
 *  This returns the current data phase, which may differ from the
 *  value input to SetPhase0 until the data has been updated.
 */
float svkPhaseSpec::GetPhase0()
{
    return this->phase0;
}


/*! 
 *  Sets the linear phase to apply to spectra.
 */
void svkPhaseSpec::SetLinearPhase(float linearPhase)
{
    this->linearPhaseTarget = linearPhase;
}


/*!
 *  This returns the current data phase, which may differ from the
 *  value input to SetLinearPhase until the data has been updated.
 */
float svkPhaseSpec::GetLinearPhase()
{
    return this->linearPhase;
}

void svkPhaseSpec::SetLinearPhasePivot(int linearPhasePivot)
{
	this->linearPhasePivotTarget = linearPhasePivot;
}


/*!
 *  Sets the extent over which the phasing should be applied.      
 *  Takes 2 sets of x,y,z indices that specify the extent range 
 *  in 3D.  
 */
void svkPhaseSpec::SetUpdateExtent(int* start, int* end)
{
    this->updateExtent[0] =  start[0];  
    this->updateExtent[1] =  end[0];  
    this->updateExtent[2] =  start[1];  
    this->updateExtent[3] =  end[1];  
    this->updateExtent[4] =  start[2];  
    this->updateExtent[5] =  end[2];  

    /*  
     *  set modified time so that subsequent calls to Update() call RequestInformation() 
     *  and refresh the extent 
     */
    this->Modified();
}


/* 
 *
 */
void svkPhaseSpec::SetChannel(int channel)
{
    this->channelToPhase = channel; 
}


/* 
 *
 */
void svkPhaseSpec::PhaseAllChannels()
{
    this->channelToPhase = -1; 
}


/*!
 *  Port 0 is the required input port for the MRS data to be phased. 
 *  Port 1 is the optional input port for a phase map of values to apply 
 */
int svkPhaseSpec::FillInputPortInformation( int port, vtkInformation* info )
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
void svkPhaseSpec::PrintSelf( ostream &os, vtkIndent indent )
{
    Superclass::PrintSelf( os, indent );
    os << "channelToPhase        :" << this->channelToPhase << endl;
    os << "phase0                :" << this->phase0 << endl;
    os << "phase0Target          :" << this->phase0Target << endl;
    os << "linearPhase           :" << this->linearPhase << endl;
    os << "linearPhaseTarget     :" << this->linearPhaseTarget << endl;
    os << "linearPhasePivot      :" << this->linearPhasePivot << endl;
    os << "linearPhasePivotTarget:" << this->linearPhasePivotTarget << endl;
}



