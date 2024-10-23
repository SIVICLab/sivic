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



#include <svkEPSIPhaseCorrect.h>
#include <svkMrsLinearPhase.h> 


using namespace svk;


//vtkCxxRevisionMacro(svkEPSIPhaseCorrect, "$Rev$");
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
    this->symEPSIPhaseArray = NULL; 
    this->epsiType =  FLYBACK;
    this->phaseSlope = 1;   //positive slope shifts points to left
}


/*!
 *  Clean up any allocated member variables. 
 */
svkEPSIPhaseCorrect::~svkEPSIPhaseCorrect()
{
}


/*!
 *  Set the epsi type 
 */
void svkEPSIPhaseCorrect::SetEPSIType( EPSIType type )
{
    this->epsiType = type;
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

    //  Initialize the spatial and spectral factor in the EPSI phase correction: 
    //  One phase factor for each value of k in EPSI axis
    vtkImageComplex** epsiPhaseArray = new vtkImageComplex*[ this->numEPSIkRead ];  
    for (int i = 0; i < this->numEPSIkRead; i++ ) {
        epsiPhaseArray[i] = new vtkImageComplex[ numSpecPts ];  
    }
    this->CreateEPSIPhaseCorrectionFactors( epsiPhaseArray, numSpecPts ); 

    double cmplxPtIn[2]; 
    float cmplxPtPhased[2]; 
    float epsiPhase[2]; 
    int   epsiIndex; 
    vtkImageComplex* ktCorrection = new vtkImageComplex[2]; 

    //  Inverse Fourier Transform spectral data to frequency domain to 
    //  apply linear phase shift for EPSI correction:
    string specDomain = hdr->GetStringValue( "SignalDomainColumns");
    bool applySpecFFTs = false;
    if ( specDomain.compare("TIME") == 0 ) {
        applySpecFFTs = true;
    }
    if ( applySpecFFTs == true ) {
        this->SpectralFFT(svkMrsImageFFT::FORWARD);
    }

     //  Get the Dimension Index and index values  
    svkDcmHeader::DimensionVector dimensionVector = hdr->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector indexVector = dimensionVector; 

    //  GetNumber of cells in the image:
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector ); 
     
    for (int cellID = 0; cellID < numCells; cellID++ ) { 
        //cout << "CELLID: " << cellID << endl;
        //  Get the dimensionVector index for current cell -> indexVector: 
        svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimensionVector, &indexVector, cellID ); 

        //  Index along epsiAxis used to get appropriate kPhaseArray values along epsiAxis 
        if ( this->epsiAxis == 2 ) {
            epsiIndex = svkDcmHeader::GetDimensionVectorValue( &indexVector, svkDcmHeader::SLICE_INDEX);  
        } else if ( this->epsiAxis == 1 ) {
            epsiIndex = svkDcmHeader::GetDimensionVectorValue( &indexVector, svkDcmHeader::ROW_INDEX);  
        } else if ( this->epsiAxis == 0 ) {
            epsiIndex = svkDcmHeader::GetDimensionVectorValue( &indexVector, svkDcmHeader::COL_INDEX);  
        }
       
        //cout << "cellID " << cellID << endl;
        //svkDcmHeader::PrintDimensionIndexVector(&indexVector); 

        vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( mrsData->GetSpectrum( cellID ) );

        //  Iterate over frequency points in spectrum and apply phase correction:
        for ( int freq = 0; freq < numSpecPts; freq++ ) {
                    
            spectrum->GetTuple(freq, cmplxPtIn);

            epsiPhase[0] = epsiPhaseArray[epsiIndex][freq].Real; 
            epsiPhase[1] = epsiPhaseArray[epsiIndex][freq].Imag; 
            //cout << freq << " " << cmplxPtIn[0] << " " << epsiPhase[0] << " " << epsiPhase[1] << endl;

            cmplxPtPhased[0] = cmplxPtIn[0] * epsiPhase[0] + cmplxPtIn[1] * epsiPhase[1]; 
            cmplxPtPhased[1] = cmplxPtIn[1] * epsiPhase[0] - cmplxPtIn[0] * epsiPhase[1]; 

            spectrum->SetTuple(freq, cmplxPtPhased); 
    
        }

        //  If this is a symmetric EPSI acquisition, then apply an additional phase correction to the 
        //  second lobe to account for 1/2 the spectral bandwidth shifts between pos and neg lobes 
        this->PhaseAlternatingSymmetricEPSILobes( cellID ); 

    }

    //  Forward Fourier Transform spectral data to back to time domain, should now be shifted.   
    if ( applySpecFFTs == true ) {
        this->SpectralFFT(svkMrsImageFFT::REVERSE);
    }

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    //this->Update();

    for (int i = 0; i < this->numEPSIkRead; i++ ) {
        delete [] epsiPhaseArray[i]; 
    }
    delete [] epsiPhaseArray; 

    return 1; 
} 


/*!
 *  If this is a symmetric EPSI acquisition, then apply an additional phase correction to the 
 *  second lobe to account for 1/2 the spectral bandwidth shifts between pos and neg lobes. 
 *  Apply a 1/2 cycle phase shift to alternating sym EPSI lobes.  Unlike the
 *  phase correction to account for the time delat (dt) across the readout lobes and 
 *  which varies by k, this is the same correction for each lobe, regardless of 
 *  the k value. 
 */
void svkEPSIPhaseCorrect::PhaseAlternatingSymmetricEPSILobes( int cellID )
{

    if ( this->epsiType == SYMMETRIC ) {
        //  Get pointer to input data set. 
        svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    
        //  Get pointer to data's meta-data header (DICOM object). 
        svkDcmHeader* hdr = mrsData->GetDcmHeader();  
    
        int numSpecPts = hdr->GetIntValue( "DataPointColumns" );
    
        svkDcmHeader::DimensionVector dimensionVector = hdr->GetDimensionIndexVector();
        svkDcmHeader::DimensionVector indexVector = dimensionVector; 
        svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimensionVector, &indexVector, cellID ); 
        int epsiLobeIndex = svkDcmHeader::GetDimensionVectorValue( &indexVector, svkDcmHeader::EPSI_ACQ_INDEX);  
        //cout << "EPSI LOBE INDEX: " << epsiLobeIndex << endl;
        if ( epsiLobeIndex == 1 ) {
   
            //  initialize the phase correction array if necessary 
            if ( this->symEPSIPhaseArray == NULL ) {
                cout << "Apply sym epsi phase correction" << endl;
                double freqIncrement;
                float  fOrigin = (numSpecPts)/2; 
                double mult;
                double Pi      = vtkMath::Pi();
                double factor = this->phaseSlope * Pi; 
                this->symEPSIPhaseArray = new vtkImageComplex[ numSpecPts ];
                for( int f = 0; f <  numSpecPts; f++ ) {
                    freqIncrement = ( f - fOrigin ) / ( numSpecPts );
                    mult = factor * freqIncrement;
                    this->symEPSIPhaseArray[f].Real = cos( mult );
                    this->symEPSIPhaseArray[f].Imag = sin( mult );
                }
            }
    
            vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( mrsData->GetSpectrum( cellID ) );
            double cmplxPtIn[2];
            double cmplxPtPhased[2];
            double epsiPhase[2];
    
            for ( int freq = 0; freq < numSpecPts; freq++ ) {
                spectrum->GetTuple(freq, cmplxPtIn);
                //cout << " phase " << freq << " " <<  this->symEPSIPhaseArray[freq].Real << endl;
                epsiPhase[0] = this->symEPSIPhaseArray[freq].Real; 
                epsiPhase[1] = this->symEPSIPhaseArray[freq].Imag; 
    
                cmplxPtPhased[0] = cmplxPtIn[0] * epsiPhase[0] + cmplxPtIn[1] * epsiPhase[1]; 
                cmplxPtPhased[1] = cmplxPtIn[1] * epsiPhase[0] - cmplxPtIn[0] * epsiPhase[1]; 
    
                spectrum->SetTuple(freq, cmplxPtPhased); 
            }
        
        }
    }
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
    float  fOrigin = (numSpecPts)/2; 
    //float  fOrigin = (numSpecPts - 1)/2.; 
    //float  fOrigin = (numSpecPts)/2. - 1; 
    double Pi      = vtkMath::Pi();
    double kIncrement;
    double freqIncrement;
    double mult;
    cout <<  "num spec pts: " << numSpecPts << endl;
    cout <<  "num k pts read: " << numEPSIkRead << endl;
    cout << " EPSI ORIGIN: " << kOrigin << endl;
    cout << " FREQ ORIGIN: " << fOrigin << endl;

    double dtBs = 1./static_cast<float>(this->numEPSIkRead);
    dtBs *= this->phaseSlope * 2 * Pi; 
    //  if sym EPSI divide mult by 2 since the cycle (num points in spectral bandwith is 
    //  twice as big)
    if ( this->epsiType == SYMMETRIC ) {
        dtBs /= 2; 
    }

    //  certainly need a factor of 2 for interleaved, but a factor of 4?  Not sure
    cout << "Need to resolve this factor in different implementations" << endl;
    cout << "DTBS: " << dtBs << endl;
    //  apply a positively sloping linear phase to shift data points back to the left.     
    //  shift should be 
    for( int k = 0; k < numKPts ; k++ ) {
        for( int f = 0; f <  numSpecPts; f++ ) {
            kIncrement = ( k - kOrigin );
            freqIncrement = ( f - fOrigin ) / ( numSpecPts );
            mult = dtBs * kIncrement * freqIncrement;
            epsiPhaseArray[k][f].Real = cos( mult );
            epsiPhaseArray[k][f].Imag = sin( mult );

            //cout << "fI: " << freqIncrement << endl;
            //cout << "FACTOR( " << k << "," << f << "): " << mult << " " <<  epsiPhaseArray[k][f].Real << " " << epsiPhaseArray[k][f].Imag << endl;
            //cout << "   Korigin: " << kOrigin << " numKPts " << numKPts << endl; 
            //cout << "   KI: " << -1 * 360 * kIncrement * dtBs << endl;
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
    fft->SetInputData( mrsData );
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


