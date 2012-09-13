/*
 *  Copyright © 2009-2012 The Regents of the University of California.
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



#include <svkEPSIPhaseCorrect.h>
#include <svkMrsLinearPhase.h> 


using namespace svk;


vtkCxxRevisionMacro(svkEPSIPhaseCorrect, "$Rev$");
vtkStandardNewMacro(svkEPSIPhaseCorrect);


/*!
 *  Constructor.  Initialize any member variables. 
 */
svkEPSIPhaseCorrect::svkEPSIPhaseCorrect()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    //  Initialize any member variables
    this->numEPSIkRead = 0;
    this->epsiAxis = -1;
    this->epsiOrigin = -1;
    this->epsiSpatialPhaseCorrection = NULL;

}


/*!
 *  Clean up any allocated member variables. 
 */
svkEPSIPhaseCorrect::~svkEPSIPhaseCorrect()
{
}


/*!
 *  Set the number of k-space samples along the EPSI encoding 
 *  direction (number of samples per lobe). This is the number
 *  of samples per echo in the EPSI acquisition trajetory (not 
 *  necessarily the final k-space dimensionality). 
 */
void svkEPSIPhaseCorrect::SetNumEPSIkRead( int numKspacePoints )
{
    this->numEPSIkRead = numKspacePoints;
}


/*!
 *  Set the axis index corresponding to the EPSI encoding (0,1 or 2). 
 */
void svkEPSIPhaseCorrect::SetEPSIAxis( int epsiAxis)
{
    this->epsiAxis = epsiAxis;
}


/*!
 *  Set the origin index along the EPSI encoding axis 
 *  default = (numEPSIkRead-1)/2. See notes for 
 *  GetEPSIOrigin.  
 */
void svkEPSIPhaseCorrect::SetEPSIOrigin( float epsiOrigin )
{
    this->epsiOrigin = epsiOrigin;
}


/*!
 *  Get the origin index along the EPSI encoding axis 
 *  default = (numEPSIkRead-1)/2. This is the c-lang  
 *  index, thus the -1, e.g.: if numEPSIkRead is 8,  
 *  and data index varies from 0-7, the default origin 
 *  index is 3.5.  However, should depend on whether 
 *  k=0 was sampled or not.  If not, (default, then origin
 *  is .5 higher. 
 */
float svkEPSIPhaseCorrect::GetEPSIOrigin()
{
    bool k0Sampled = false; 
    if ( this->epsiOrigin < 0 ) { 
        if (k0Sampled) { 
            this->epsiOrigin = (this->numEPSIkRead - 1) / 2.;
        } else { 
            this->epsiOrigin = (this->numEPSIkRead) / 2.;
        }
    }
    return this->epsiOrigin; 
}


/*! 
 *  This method is called during pipeline execution.  Calculates the 2D array of linear phase correction factors, which
 *  are a function of the epsi k-space index, as well as the spectral index.  Applies these to all spectra in data set
 *  to generate a rectilinear spectral/spatial (k-space, time-domain) data set.  
 */
int svkEPSIPhaseCorrect::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    if ( this->numEPSIkRead == 0  || this->epsiAxis < 0 ) {
        cout << "ERROR, must specify the epsiAxis and number of sample k-space points per lobe" << endl;
        return 1; 
    }

    //  Get pointer to input data set. 
    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 

    //  Get pointer to data's meta-data header (DICOM object). 
    svkDcmHeader* hdr = mrsData->GetDcmHeader();  

    //  Lookup any data set attributes from header required for algorithm (See DICOM IOD for field names):
    int numSpecPts = hdr->GetIntValue( "DataPointColumns" );
    int cols       = hdr->GetIntValue( "Columns" );
    int rows       = hdr->GetIntValue( "Rows" );
    int slices     = hdr->GetNumberOfSlices();
    int numLobes   = hdr->GetNumberOfCoils();  // e.g. symmetric EPSI has pos + neg lobes

    //  Initialize the spatial and spectral factor in the EPSI phase correction: 
    //  One phase factor for each value of k in EPSI axis
    vtkImageComplex** epsiPhaseArray = new vtkImageComplex*[ this->numEPSIkRead ];  
    for (int i = 0; i < this->numEPSIkRead; i++ ) {
        epsiPhaseArray[i] = new vtkImageComplex[ numSpecPts ];  
    }
    this->CreateEPSIPhaseCorrectionFactors( epsiPhaseArray, numSpecPts ); 

    float cmplxPtIn[2]; 
    float cmplxPtPhased[2]; 
    float epsiPhase[2]; 
    int   epsiIndex; 
    vtkImageComplex* ktCorrection = new vtkImageComplex[2]; 

    //  Inverse Fourier Transform spectral data to frequency domain to 
    //  apply linear phase shift: 
    this->SpectralFFT( svkMrsImageFFT::FORWARD); 

    //  Iterate through 3D spatial locations
    for (int lobe = 0; lobe < numLobes; lobe++) {
        for (int z = 0; z < slices; z++) {
            for (int y = 0; y < rows; y++) {
                for (int x = 0; x < cols; x++) {

                    //  Index along epsiAxis used to get appropriate kPhaseArray values along epsiAxis 
                    if ( this->epsiAxis == 2 ) {
                        epsiIndex = z; 
                    } else if ( this->epsiAxis == 1 ) {
                        epsiIndex = y; 
                    } else if ( this->epsiAxis == 0 ) {
                        epsiIndex = x; 
                    }

                    vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( mrsData->GetSpectrum( x, y, z, 0, lobe) );

                    //  Iterate over frequency points in spectrum and apply phase correction:
                    for ( int freq = 0; freq < numSpecPts; freq++ ) {
                    
                        spectrum->GetTupleValue(freq, cmplxPtIn);

                        epsiPhase[0] = epsiPhaseArray[epsiIndex][freq].Real; 
                        epsiPhase[1] = epsiPhaseArray[epsiIndex][freq].Imag; 

                        //  data * e(i * X)
                        //cmplxPtPhased[0] = cmplxPtIn[0] * epsiPhase[0] - cmplxPtIn[1] * epsiPhase[1]; 
                        //cmplxPtPhased[1] = cmplxPtIn[1] * epsiPhase[0] + cmplxPtIn[0] * epsiPhase[1]; 

                        //  data * e(-i * X)
                        cmplxPtPhased[0] = cmplxPtIn[0] * epsiPhase[0] + cmplxPtIn[1] * epsiPhase[1]; 
                        cmplxPtPhased[1] = cmplxPtIn[1] * epsiPhase[0] - cmplxPtIn[0] * epsiPhase[1]; 

                        //cout << "spec: " << cmplxPtIn[0] << " " << cmplxPtIn[1] << " -> " << cmplxPtPhased[0] 
                        //  << " " << cmplxPtPhased[1] << endl;

                        spectrum->SetTuple(freq, cmplxPtPhased); 
    
                    }

                }
            }
        }
    }


    //  Forward Fourier Transform spectral data to back to time domain, should now be shifted.   
    this->SpectralFFT( svkMrsImageFFT::REVERSE); 


    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->GetInput()->Update();

    for (int i = 0; i < this->numEPSIkRead; i++ ) {
        delete [] epsiPhaseArray[i]; 
    }
    delete [] epsiPhaseArray; 

    return 1; 
} 


/*!
 *  This method initializes an array of N, vtkImageComplex values, where vtkImageComplex
 *  is a struct with a double real and double imaginary component representing
 *  a single complex value.  N is the number of k-space points sampled in the EPSI 
 *  direction (epsiAxis) (per lobe).  The function sets the spatial component of the linear 
 *  EPSI phase correction. The pivot in the first order correction given by the origin 
 *  (default middle index of array) and linear factor
 *  that increments by 2*pi/N.  These are multiplied by the factor derived by the 
 *  frequency encoding to obtain the final linear EPSI phase correctin (spatial/spectral). 
 *
 *      This is an implementation of equation 3 from (MRM 54:1286, 2005):   
 *      Here, I'm using lobe and echo to mean a full gradient echo period
 *          Sphased(mn) = S(mn) * exp i*( 2 * pi * m * n * Dt * Bs / N )
 *              - Dt    = the time between k-space samples 
 *              - Bs    = spectral bandwith (or 1/time_per_echo)
 *              - N     = num gradient echoes
 *          Dt*Bs is therefore 1/num_sample_per_echo, aka this->numEPSIkRead. 
 *          N is numSpecPts (i.e. number of lobes or half that for symmetric EPSI).   
 *
 *
 *  current issues 
 *  1.  + or - phase shift eix or e-ix) ? 
 *  2.  Origin of k-space sampling based on num_read or current number of k-space samples (with ends thrown out)
 *  3.  Matlab origin is off by 1 (N/2 vs (N-1)/2 for zero based array indexing 
 *  4.  Is there an additional phase shift required for the even lobes due to the points thrown out between lobes?
 *  5.  finally I think the numerator is based on the number of lobes per spectrum, rather than the total number
 *      of lobes (echoes). the MATLAB currently uses the total number of lobes * numread
 */
void svkEPSIPhaseCorrect::CreateEPSIPhaseCorrectionFactors( vtkImageComplex** epsiPhaseArray, int numSpecPts )
{

    double numKPts = this->numEPSIkRead;
    double kOrigin = this->GetEPSIOrigin(); 
    float  fOrigin = (numSpecPts)/2.; 
    //float  fOrigin = (numSpecPts-1)/2.; 
    double Pi      = vtkMath::Pi();
    double kIncrement;
    double freqIncrement;
    double mult;
    cout <<  "num spec pts: " << numSpecPts << endl;
    cout <<  "num k pts read: " << numEPSIkRead << endl;
    cout << " EPSI ORIGIN: " << kOrigin << endl;
    cout << " FREQ ORIGIN: " << fOrigin << endl;
    cout << "DENOM " << numSpecPts * numKPts * 2 << endl;
    for( int k = 0; k < numKPts ; k++ ) {
        for( int f = 0; f <  numSpecPts; f++ ) {
            kIncrement = ( k - kOrigin )/( numKPts * 2);
            freqIncrement = ( f - fOrigin )/( numSpecPts );
            mult = 2 * Pi * kIncrement * freqIncrement; 
            epsiPhaseArray[k][f].Real = cos( mult );
            epsiPhaseArray[k][f].Imag = sin( mult );
        }
    }

}


/*!
 *  To shift spectra in time a linear phase correction is appplied to spectra in the 
 *  frequency domain.  This method transforms spectral domain from time to frequency. 
 */
int svkEPSIPhaseCorrect::SpectralFFT( svkMrsImageFFT::FFTMode direction )
{

    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 

    svkMrsImageFFT* fft = svkMrsImageFFT::New();
    fft->SetInput( mrsData );
    fft->SetFFTDomain( svkMrsImageFFT::SPECTRAL );
    fft->SetFFTMode( direction ); 
    fft->Update();
    fft->Delete();
    return 0;
}


/*!
 *  Set the input data type, e.g. svkMrsImageData for an MRS algorithm.
 */
int svkEPSIPhaseCorrect::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


