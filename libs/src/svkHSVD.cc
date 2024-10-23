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
 *      Stojan Maleschlijski
 */



#include <svkHSVD.h>
#include <svkSpecPoint.h>
#include <svkMrsImageFFT.h>
//#include <svkApodizationWindow.h>

#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkImageData.h>
#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkInformationVector.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>
#ifndef WIN32
#include <sys/time.h>
#endif
using namespace svk;


//vtkCxxRevisionMacro(svkHSVD, "$Rev$");
vtkStandardNewMacro(svkHSVD);


int* svkHSVD::progress; //  static pointer 

/*!
 */
svkHSVD::svkHSVD()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");
    this->isInputInTimeDomain = false; 
    this->exportFilterImage = false; 
    this->onlyFitInVolumeLocalization = false; 
    this->modelOrder = 25; 
    this->numTimePoints = -1;     
    //this->SetNumberOfThreads(1);
    svkHSVD::progress = NULL; 
    this->errorHandlingFlag = svkHSVD::SET_SIGNAL_TO_ZERO;
    this->thresholdRMSRatioDown = 0.075; // empirical value
    this->thresholdRMSRatioUp = 50; // doesnt represent the filter quality that much
    this->numberPtsToCheckQuality = 10;
    this->fitSuccessMap = NULL;
}


/*!
 */
svkHSVD::~svkHSVD()
{
    
    if ( this->filterImage != NULL )  {
        this->filterImage->Delete();
        this->filterImage = NULL;
    }

    if ( this->fitSuccessMap != NULL )  {
        this->fitSuccessMap->Delete();
        this->fitSuccessMap = NULL;
    }

} 


/*!
 *  Number of basis functions in HSVD model (default = 25). 
 */
void svkHSVD::SetModelOrder(int modelOrder)
{
    this->modelOrder = modelOrder; 
}


/*!
 *  Default is to fit all voxels. 
 */
void svkHSVD::OnlyFitSpectraInVolumeLocalization()
{
    this->onlyFitInVolumeLocalization = true; 
}



/*!
 *  Make sure output data in the same domain as the input. 
 */
void svkHSVD::ExportFilterImage() 
{
    this->exportFilterImage = true; 
}


/*!
 *  Make sure output data in the same domain as the input. 
 */
svkMrsImageData* svkHSVD::GetFilterImage() 
{
    return this->filterImage; 
}


svkMriImageData* svkHSVD::GetFitSuccessImage()
{
    return this->fitSuccessMap;
}

/*! 
 *
 */
int svkHSVD::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    return 1;
}


/*!
 *  Set the error handling behvior type
 */
void svkHSVD::SetErrorHandlingBehavior( HSVDBehaviorOnError errBehavior )
{ 
    this->errorHandlingFlag = errBehavior;
}

/*!
 *  Switch to single threading (relevant for grid submissions)
 */
void svkHSVD::SetSingleThreaded()
{ 
    this->SetNumberOfThreads(1);
}


/*!
 *  Sets the error handling behavior such that if a fitting error is detected, the output spectrum
 *  for that voxel is set to zero. This is the default error handling behavior. 
 */
void svkHSVD::SetErrorHandlingSignalToZeroOn()
{ 
    this->errorHandlingFlag = svkHSVD::SET_SIGNAL_TO_ZERO;
}


/*!
 *  Sets the error handling behavior such that if a fitting error is detected, 
 *  the filter for that voxel is set to zero and the output spectrum is set to 
 *  the original input data.  In other words if an error is 
 *  detected, then no filter is applied to that voxel. 
 */
void svkHSVD::SetErrorHandlingFilterToZeroOn()
{ 
    this->errorHandlingFlag = svkHSVD::SET_FILTER_TO_ZERO;
}


/*!
 *  Sets the error handling behavior such that if a fitting error is detected, 
 *  the filter is still subtracted even though the quality may be poor.  This 
 *  may be useuful to study the quality of the filter. 
 */
void svkHSVD::SetErrorHandlingIgnoreError()
{
    this->errorHandlingFlag = svkHSVD::IGNORE_ERROR;
}       


/*!
 *  
 */
void svkHSVD::SetThresholdModelDifference(float percentDifferenceUp, float percentDifferenceDown)
{
        this->thresholdRMSRatioUp = percentDifferenceUp;
        this->thresholdRMSRatioDown = percentDifferenceDown;
}




/*! 
 *  vtk algorithm method
 */
int svkHSVD::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 

    this->fitSuccessMap = svkMriImageData::New();
    data->GetZeroImage(this->fitSuccessMap);
    this->fitSuccessMap->GetDcmHeader()->SetValue("SeriesDescription", "HSVD Error Map");

    //  Make sure input spectra are in time domain for HSVD filter. 
    this->CheckInputSpectralDomain(); 

    this->numTimePoints = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );
    this->spectralWidth = data->GetDcmHeader()->GetFloatValue( "SpectralWidth" );


    //  make a copy of the input image to put the filter image into
    this->filterImage = svkMrsImageData::New(); 
    this->filterImage->DeepCopy( data ); 

    //   for each cell /spectrum: 
    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );

    float tolerance = .5;     
    this->selectionBoxMask = new short[numCells];

    data->GetSelectionBoxMask(selectionBoxMask, tolerance); 

    //===========================================
    //  try apodizing the spectrum here: 
    //===========================================
    //this->apodizationWindow = vtkFloatArray::New();
    //float fwhh = .1;
    //svkApodizationWindow::GetLorentzianWindow( this->apodizationWindow, data, fwhh );
    //===========================================

    if ( svkHSVD::progress == NULL ) {
        int numThreads = this->GetNumberOfThreads();
        svkHSVD::progress = new int[numThreads]; 
        for ( int t = 0; t < numThreads; t++ ) {
            svkHSVD::progress[t] = 0; 
        }
    }

    ostringstream progressStream;
    progressStream << "Executing HSVD ";
    this->SetProgressText( progressStream.str().c_str() );
    this->UpdateProgress(.0);

    //  This will call HSVDFitCellSpectrum foreach cell within the 
    //  sub-extent within each thread.
    this->Superclass::RequestData(
        request,
        inputVector,
        outputVector
    );

    if ( svkHSVD::progress != NULL ) {
        delete [] svkHSVD::progress; 
        svkHSVD::progress = NULL; 
    }

    //  Subtract Filter Spectrum from the input data
    this->SubtractFilter();

    //  Make sure input spectra are in time domain for HSVD filter. 
    this->CheckOutputSpectralDomain(); 

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();

    delete [] this->selectionBoxMask; 

    return 1; 
} 


/*!
 *  Computes the HSVD fit for the specified spectrum. 
 */
void svkHSVD::HSVDFitCellSpectrum( int cellID ) 
{

    //cout << "HSVD Cell: " << cellID << endl;
    vector< vector< double > >  hsvdModel;    

    bool cellContainsNaNs = this->HSVD(cellID, &hsvdModel);
   
    if ( this->GetDebug() ) {
        for ( int pole = 0; pole < hsvdModel.size(); pole++) {

            double amp   = (hsvdModel)[pole][0]; 
            double phase = (hsvdModel)[pole][1]; 
            double freq  = (hsvdModel)[pole][2]; 
            double damp  = (hsvdModel)[pole][3]; 
            //cout << "POLE " << pole << " : " << freq << "           " << amp << " " << phase << " " << damp << endl; 
            if ( this->GetDebug() ) {
                //cout << "    HSVD POLE " << setw(5) << pole << ": " << setw(10) << amp << " " << freq
                    //<< " " << setw(20) << " " << phase << " " << setw(20) << damp << endl; 
            }
        }
    }

    // Only if HSVD succefully generated the fit parameter, a model should be generated
    this->GenerateHSVDFilterModel( cellID, &hsvdModel, cellContainsNaNs );
   
}


/*!
 *
 */
int svkHSVD::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


/*!
 *  Make sure input data are transformed to time domain 
 *  before applying HSVD filter. 
 */
void svkHSVD::CheckInputSpectralDomain() 
{

    //  Get the domain of the input spectra:  time/frequency
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    string spectralDomain = data->GetDcmHeader()->GetStringValue( "SignalDomainColumns");
    if ( spectralDomain.compare("TIME") == 0 ) {
        this->isInputInTimeDomain = true; 
    } else {
        this->isInputInTimeDomain = false; 
    }

    //  if necessary, transform data to time domain: 
    if ( this->isInputInTimeDomain == false ) {

        svkMrsImageFFT* fft = svkMrsImageFFT::New();
        fft->SetInputData( data );

        fft->SetFFTDomain( svkMrsImageFFT::SPECTRAL ); 

        //  frequency to time: 
        fft->SetFFTMode( svkMrsImageFFT::REVERSE ); 

        if ( this->onlyFitInVolumeLocalization == true ) { 
            fft->OnlyUseSelectionBox();
        } 

        fft->Update();
        fft->Delete();

    } 

}


/*!
 *  Make sure output data in the same domain as the input. 
 */
void svkHSVD::CheckOutputSpectralDomain() 
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 

    //  Output spectra are in TIME domain at this point.  Compare with original
    //  input domain and transform if necessary. 

    if ( this->isInputInTimeDomain == false ) {

        svkMrsImageFFT* fft = svkMrsImageFFT::New();
        fft->SetInputData( data );

        fft->SetFFTDomain( svkMrsImageFFT::SPECTRAL ); 

        //  time to frequency: 
        fft->SetFFTMode( svkMrsImageFFT::FORWARD ); 
        if ( this->onlyFitInVolumeLocalization == true ) { 
            fft->OnlyUseSelectionBox();
        } 

        fft->Update();
        fft->Delete();

        if ( this->exportFilterImage ) {

            svkMrsImageFFT* fft = svkMrsImageFFT::New();
            fft->SetInputData( this->filterImage );

            fft->SetFFTDomain( svkMrsImageFFT::SPECTRAL ); 

            //  time to frequency: 
            fft->SetFFTMode( svkMrsImageFFT::FORWARD ); 
            if ( this->onlyFitInVolumeLocalization == true ) { 
                fft->OnlyUseSelectionBox();
            } 

            fft->Update();
            fft->Delete();
        }

    } 

}


/*!
 *  Returns true if cell was fit, or false if it could not be fit (e.g., no signal) or a fitting 
 *  error was detected. 
 */
bool svkHSVD::HSVD(int cellID, vector<vector <double > >* hsvdModel) 
{
    bool bCellFit = false; // cell not ok

    //cout << "FIT HSVD Cell: " << cellID << endl;
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    svkDcmHeader* hdr = data->GetDcmHeader(); 

    //  some information about the spectrum to be processed
    int numTimePoints = this->numTimePoints; 

    //  dwelltime in seconds: 
    double _RealDwellTime =  1./(this->spectralWidth); 

    integer       numTimePointsLong = (integer) numTimePoints;
    const double  dt                = _RealDwellTime;       // dwell time in seconds
    const double  dt2pi             = dt * 2. * M_PI;
    integer       l                 = numTimePointsLong / 4;
    integer       m                 = numTimePointsLong - l + 1;
    const integer l2                = l * l;

    // read complex signal
    doublecomplex* signal = new doublecomplex[numTimePointsLong];

    vtkFloatArray* spectrum = static_cast<vtkFloatArray*>( data->GetSpectrum( cellID ) );

    int x;
    int y;
    int z;
    float tupleIn[2];

    for( int t = 0; t < numTimePointsLong; t++ ){

        spectrum->GetTupleValue(t, tupleIn);

        signal[t].r = static_cast<doublereal>(tupleIn[0]); 
        signal[t].i = static_cast<doublereal>(tupleIn[1]);
    }

    //  if signal is entirely zero, then do not fit this voxel
    if ( ! this->CanFitSignal( signal,  numTimePointsLong ) ) {

        vector < double > poleParams; 
        double zero_val = 0.;

        //  amplitude
        poleParams.push_back(zero_val);

        //  phase
        poleParams.push_back(zero_val);

        //  frequency    
        poleParams.push_back(zero_val);
                  
        //  damping
        poleParams.push_back(zero_val);         

        hsvdModel->push_back(poleParams); 

        return bCellFit;
    }


    //////////////////////////////////////////////////////////////////////
    // start the processing of the individual spectra
    //////////////////////////////////////////////////////////////////////

    const int length_multiplications = l * ( m + numTimePointsLong )/2;
    doublecomplex *multiplications = new doublecomplex[length_multiplications];
    int idx = 0;
    // ??? useless part
    for( idx = 0; idx < numTimePointsLong; idx++ ){
        multiplications[idx].r = signal[idx].r * signal[idx].r + signal[idx].i * signal[idx].i;
        multiplications[idx].i = 0.;
    }

    // prepare multiplication matrix with conj complex multiplications
    for( int i = 1; i < l; ++i ){
        int length = numTimePointsLong - i;
        for( int j = 0; j < length; ++j ){
            double prod1 = (signal[i+j].r-signal[i+j].i) * (signal[j].r+signal[j].i);
            double prod2 = signal[i+j].r*signal[j].r;
            double prod3 = signal[i+j].i * signal[j].i;
            multiplications[idx].r = prod2 + prod3;
            multiplications[idx].i = (prod1 - prod2 + prod3);
            idx++;
        }
    }
    // generate hankel matrix
    doublecomplex *hankel2 = new doublecomplex[l2];
    const int step = l+1;
    int mult_elem = 0;
    for( int i = 0; i != l; ++i ){
        for( idx = i*l; idx < l2; idx += step ){
            if( idx % l == 0 ){
                hankel2[idx].r = hankel2[idx].i = 0.;
                for( int j = 0; j < m; ++j ){
                    hankel2[idx].r += multiplications[mult_elem].r;
                    hankel2[idx].i += multiplications[mult_elem].i;
                    mult_elem++;
                }
            } 
            else {
                hankel2[idx].r = hankel2[idx - step].r + multiplications[mult_elem].r - multiplications[mult_elem - m].r;
                hankel2[idx].i = hankel2[idx - step].i + multiplications[mult_elem].i - multiplications[mult_elem - m].i;
                mult_elem++;
            }
       }
    }
    delete[] multiplications;

    // Not quite sure what this part does
    // ----------------------------------------          
    char flag1 = 'V';    
    char flag2 = 'U';    
    integer workspace1_size = l * (l+48);
    integer info;       
    doublecomplex* workspace1   = new doublecomplex[workspace1_size];
    integer     workspace2_size = 1+8*l+2*l2;
    doublereal* workspace2      = new doublereal[workspace2_size];
    integer workspace3_size     = 8*l;
    integer* workspace3         = new integer[workspace3_size];
    doublereal *eigenvalues     = new doublereal[l];

    zheevd_( &flag1, &flag2, &l, hankel2, &l, eigenvalues, workspace1, 
            &workspace1_size, workspace2,&workspace2_size, workspace3, 
            &workspace3_size, &info );
    //cout << "INFO(" << cellID << "): 1 " << info << endl; 

    delete[] eigenvalues;
    delete[] workspace1;
    delete[] workspace2;
    delete[] workspace3;


    // ----------------------------------------
    //  PeakNumber is the number of basis functions 
    //  in the model.. number of polesused to model 
    //  the input spectrum. 
    // ----------------------------------------
    int peakNumber = this->modelOrder;

    // Extract the u, ut and ub matrices from the hankel2 matrix.
    integer k = peakNumber;
    doublecomplex *u = new doublecomplex[ l * k ];
    idx = 0;
    for(int i = l * ( l - k ); i < l2; ++i ){
        u[idx].r = hankel2[i].r;
        u[idx].i = hankel2[i].i;
        ++idx;
    }
    delete[] hankel2;

    // ----------------------------------------
    doublecomplex *ub    = new doublecomplex[ (l - 1) * k ];
    doublecomplex *ut    = new doublecomplex[ (l - 1) * k ];
    doublecomplex *ub_ub = new doublecomplex[ k * k ];
    doublecomplex *ub_ut = new doublecomplex[ k * k ];
    idx = 0; 
    for( int i = 1; i < l*k; ++i ){
        if( i % l == 0 )
            continue;
        ut[idx].r = u[i].r;
        ut[idx].i = u[i].i;
        ub[idx].r = u[i-1].r;
        ub[idx].i = u[i-1].i;
        ++idx;
    }
    delete[] u;
    // ub_ub = ub^H * ub
    memset(ub_ub, 0, sizeof(doublecomplex)*k*k);
    this->MatSq(ub, k, l-1, ub_ub);
    // ub_ut = ub^H * ut
    this->MatMat(ub,ut,k,l-1,ub_ut);
    delete[] ub;
    delete[] ut;





    // ----------------------------------------
    integer* pivot = new integer[k];  
    doublecomplex *scal_refl = new doublecomplex[k];
    integer workspace4_size = 2*k;
    doublecomplex *workspace4 = new doublecomplex[workspace4_size];
    doublereal *workspace5 = new doublereal[2*k];

    memset(pivot, 0, sizeof(integer)*k);
    memset(scal_refl, 0, sizeof(doublecomplex)*k);
    memset(workspace4, 0, sizeof(doublecomplex)*workspace4_size);
    memset(workspace5, 0, sizeof(doublereal)*2*k);

    //  Reference to zgeqp3 from lapack: 
    //  http://www.netlib.org/lapack/complex16/zgeqp3.f
    // QR Factorization with column pivoting of a complex k*k matrix
    zgeqp3_(&k,&k,ub_ub,&k,pivot,scal_refl,workspace4,&workspace4_size,workspace5,&info);
    //cout << "INFO(" << cellID << "): 2 " << info << endl; 
    delete[] workspace4;
    delete[] workspace5;


    // Obtain the Q matrix from the factorization
    // ----------------------------------------
    integer workspace6_size = 2*k;
    doublecomplex *workspace6 = new doublecomplex[workspace6_size];
    doublecomplex *q = new doublecomplex[k*k];
    memset(workspace6, 0, sizeof(doublecomplex)*workspace6_size);

    for( int i = 0; i < k*k; ++i ){
        q[i].r = ub_ub[i].r;
        q[i].i = ub_ub[i].i;
    }
    zungqr_(&k,&k,&k,q,&k,scal_refl,workspace6,&workspace6_size,&info);
    //cout << "INFO(" << cellID << "): 3 " << info << endl; 
    delete[] workspace6;

    // Complex matrix multiplication
    // ----------------------------------------
    doublecomplex *q_ub_ut = new doublecomplex[k*k];
    memset(q_ub_ut, 0, sizeof(doublecomplex)*k*k);
    this->MatMat(q,ub_ut,k,k,q_ub_ut);
    delete[] ub_ut;


    // Solves the system of equations A*X = B
    // A = ub_ub and B = q_ub_ut. The solution is saved in q_ub_ut
    // ----------------------------------------
    flag2 = 'U';                    // only use the upper triangular part of ub_ub
    char flag3 = 'N';               // R is not transposed
    char flag4 = 'N';               // diagonals are not 1
    ztrtrs_(&flag2,&flag3,&flag4,&k,&k,ub_ub,&k,q_ub_ut,&k,&info);
    //cout << "INFO(" << cellID << "): 4 " << info << endl; 
    delete[] ub_ub;

    doublecomplex *pivot_permutation = new doublecomplex[k*k];
    memset(pivot_permutation, 0, sizeof(doublecomplex)*k*k);
    for( int i = 0; i < k; ++i)             
        pivot_permutation[ (pivot[i]-1)*k+i ].r = 1.;
    doublecomplex *least_sq_solve = new doublecomplex[k*k];
    this->MatMat(pivot_permutation,q_ub_ut,k,k,least_sq_solve);
    delete[] q_ub_ut; 


    // ----------------------------------------
    char flag5 = 'V';  
    char flag6 = 'n';
    integer workspace7_size = k*36;
    doublecomplex *freq_damp = new doublecomplex[k];
    integer left_eigenvec_size = k;
    integer right_eigenvec_size = 1;
    doublecomplex *left_eigenvec = new doublecomplex[left_eigenvec_size*k];
    doublecomplex *right_eigenvec = new doublecomplex[right_eigenvec_size*k];
    doublecomplex *workspace7 = new doublecomplex[workspace7_size];
    doublereal *workspace8 = new doublereal[2*k];
    memset(workspace7, 0, sizeof(doublecomplex)*workspace7_size);
    memset(workspace8, 0, sizeof(doublereal)*2*k);
    // Computes the eigenvalues of the matrix least_sq_solve (basically Q)
    // the eigenvalues represent the signal pole estimates -> freq and damping factor
    zgeev_(&flag5,&flag6,&k,least_sq_solve,&k,freq_damp,left_eigenvec,&left_eigenvec_size,right_eigenvec,&right_eigenvec_size, workspace7,&workspace7_size,workspace8,&info);
    //cout << "INFO(" << cellID << "): 5 " << info << endl; 

    delete[] workspace7;
    delete[] workspace8;
    delete[] left_eigenvec;
    delete[] right_eigenvec;
    delete[] least_sq_solve;

    doublecomplex* log_freq_damp = new doublecomplex[k];
    for( int i = 0; i < k; ++i ){               // z_k = |z_k| exp(i phi_k) ==> log z_k = log |z_k| + i phi_k
        log_freq_damp[i].r = log(sqrt(freq_damp[i].r*freq_damp[i].r+freq_damp[i].i*freq_damp[i].i));
        log_freq_damp[i].i = atan2(freq_damp[i].i,freq_damp[i].r);
    }
    delete[] freq_damp;

    // The frequencies and the damping factors are immediatelz available. Next step is to obtain the amplitudes.
    // For this purpose the data is fitted with f and damping kept fixed.
    // ----------------------------------------
    doublecomplex *vandermonde = new doublecomplex[numTimePointsLong * k];
    idx = 0;
    for( int i = 0; i < k; ++i ){
        for( int j = 0; j < numTimePointsLong ; ++j ){
            vandermonde[idx].r = exp(j*log_freq_damp[i].r)*cos(j*log_freq_damp[i].i);
            vandermonde[idx].i = exp(j*log_freq_damp[i].r)*sin(j*log_freq_damp[i].i);
            idx++;
        }
    }

    // ----------------------------------------
    doublecomplex *vandermonde_signal = new doublecomplex[k];

    this->MatVec(vandermonde, signal, k, numTimePointsLong, vandermonde_signal);
    delete[] signal;
    doublecomplex *vandermonde2 = new doublecomplex[k*k];
    memset(vandermonde2, 0, sizeof(doublecomplex)*k*k);

    this->MatSq(vandermonde, k, numTimePointsLong, vandermonde2);
    delete[] vandermonde;
    memset(pivot, 0, sizeof(integer)*k);
    memset(scal_refl, 0, sizeof(doublecomplex)*k);

    integer workspace9_size = 2*k;
    doublecomplex *workspace9 = new doublecomplex[workspace9_size];
    doublereal *workspace10 = new doublereal[2*k];
    memset(workspace9, 0, sizeof(doublecomplex)*workspace9_size);
    memset(workspace10,0, sizeof(doublereal)*2*k);

    zgeqp3_(&k, &k, vandermonde2, &k, pivot, scal_refl, workspace9, &workspace9_size, workspace10, &info);
    //cout << "INFO(" << cellID << "): 6 " << info << endl; 
    delete[] workspace9;
    delete[] workspace10;

    // ----------------------------------------
    integer workspace11_size = k;
    doublecomplex *workspace11 = new doublecomplex[workspace11_size];
    memset(workspace11, 0, sizeof(doublecomplex)*workspace11_size);

    for( int i = 0; i < k*k; ++i ){
        q[i].r = vandermonde2[i].r;
        q[i].i = vandermonde2[i].i;
    }
    zungqr_( &k, &k, &k, q, &k,scal_refl, workspace11, &workspace11_size, &info);
    //cout << "INFO(" << cellID << "): 7 " << info << endl; 
    delete[] workspace11;
    delete[] scal_refl;
    // ----------------------------------------
    doublecomplex *q_vandermonde_signal = new doublecomplex[k];
    memset(q_vandermonde_signal, 0, sizeof(doublecomplex)*k);

    this->MatVec(q,vandermonde_signal,k,k,q_vandermonde_signal);
    delete[] q;
    delete[] vandermonde_signal;

    // ----------------------------------------
    flag2 = 'U';        // only use the upper triangular part of ub_ub
    flag3 = 'N';        // R is not transposed
    flag4 = 'N';        // diagonals are not 1
    integer dim_rhs = 1;
    ztrtrs_( &flag2, &flag3, &flag4, &k,&dim_rhs, vandermonde2, &k, q_vandermonde_signal, &k, &info);
    //cout << "INFO(" << cellID << "): 8 " << info << endl; 
    delete[] vandermonde2;

    doublecomplex* complex_amplitude = new doublecomplex[k];
    memset(complex_amplitude, 0, sizeof(doublecomplex)*k);
    memset(pivot_permutation, 0, sizeof(doublecomplex)*k*k);

    for( int i = 0; i < k; ++i)
        pivot_permutation[ (pivot[i]-1)*k+i ].r = 1.;
    delete[] pivot;
    this->MatVec( pivot_permutation, q_vandermonde_signal, k, k, complex_amplitude);
    delete[] pivot_permutation;
    delete[] q_vandermonde_signal;

    //=====================================================
    // end of the processing of the individual spectrum
    //=====================================================

    //=====================================================
    //  Write out the 4 parameters of each pole,
    //      index   parameter
    //      0       amplitude
    //      1       phase
    //      2       frequency 
    //      3       damping 
    //=====================================================

    double amp, phase, freq, damping;
    double Deviation;

    bool nanDetected = false;
    //bool fitErrorDetected = this->CheckQualityOfFit(complex_amplitude, log_freq_damp, k, spectrum, &Deviation);

    for( int i = 0; i < k; ++i ){  // loop over poles                    
        //cout << "POLE: " << i << endl;
        vector < double > poleParams;
        //  amplitude
        amp = (double)sqrt(
                        complex_amplitude[i].r * complex_amplitude[i].r +
                        complex_amplitude[i].i * complex_amplitude[i].i
                      );
        poleParams.push_back(amp);

        //  phase
        phase = (double) atan2(complex_amplitude[i].i, complex_amplitude[i].r);
        poleParams.push_back(phase);

        //  frequency    
        freq = (double) log_freq_damp[i].i / dt2pi;
        poleParams.push_back(freq);
                  
        //  damping
        damping = (double) -log_freq_damp[i].r / dt ;
        poleParams.push_back(damping);

        hsvdModel->push_back(poleParams); 

        if (isnan(amp) || isnan(phase) || isnan(freq) || isnan(damping)){
            nanDetected = true;
        }
    }
    bCellFit = !nanDetected;

    return bCellFit;
       
}

/*!
 *  Check the quality the fit by evaluating the amplitude of the first 10 t points of the FID
 *  signal and compare it to the first 10x points calculated by the fit. If they are too different
 *  return TRUE.
 *  
 *  Output: 
 * 
 *          returns  bool fitErrorDetected 
 */
bool svkHSVD::GetFilterFailStatus(int cellID, vtkFloatArray* filterSpec, float* qfactor)
{
    bool fitErrorDetected = true;   // filter failed
    float tuple[2], filtertuple[2];

    float rmsSignal = 0.f;
    float rmsFilter   = 0.f;

    if  (filterSpec == NULL || qfactor == NULL){
        return fitErrorDetected;
    }

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0));
    vtkFloatArray* signal = static_cast<vtkFloatArray*>( data->GetSpectrum( cellID ) );


    for (int i=0; i<this->numberPtsToCheckQuality; i++){

        signal->GetTupleValue(i, tuple); // get first amplitude of the time signal
        filterSpec->GetTupleValue(i, filtertuple);

        rmsSignal += pow(tuple[0],2.f);
        rmsFilter   += pow(filtertuple[0],2.f);
    }

    rmsSignal = sqrt(rmsSignal/this->numberPtsToCheckQuality);
    rmsFilter = sqrt(rmsFilter/this->numberPtsToCheckQuality);

    *qfactor = rmsSignal/rmsFilter;
    //cout<<"QFactor:" << cellID << "\t" << *qfactor << endl;
    // 0.1 - 20
    if ( (*qfactor >= (  this->thresholdRMSRatioDown )) &&
          (*qfactor <= (  this->thresholdRMSRatioUp ))){
        fitErrorDetected = false;
    }
    return fitErrorDetected; 
}


/*
 *  If the signal is identically zero everywhere, it can't be fit. 
 */
bool svkHSVD::CanFitSignal( const doublecomplex* signal, int numPts ) 
{
        
    bool canFit = false; 
    for( int t = 0; t < numPts; t++ ){
        if ( signal[t].r != 0 ||  signal[t].i != 0 ) {
            canFit = true; 
            break;  
        } 
    }
    return canFit; 
}


/*!
 *  Use components from the fitted HSVD model to construct a filter for the specified 
 *  frequency range, amplitude, damping, etc.   The filter Spectrum should be the same dimension
 *  as the input spectrum.         
 */
void svkHSVD::GenerateHSVDFilterModel( int cellID, vector< vector<double> >* hsvdModel, bool cellFit )
{
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    svkDcmHeader* hdr = data->GetDcmHeader(); 

    int numTimePoints = this->numTimePoints; 

    //  dwelltime in milisec: 
    double sweepWidth =  this->spectralWidth; 
    bool    addSVDComponent;

    bool filterFailed = false;
    vtkFloatArray* filterSpectrum = static_cast<vtkFloatArray*>( this->filterImage->GetSpectrum( cellID ) );
    // If cell was Fit with no Zeros or NaNs OR we are set to ignore_error in the fit generate the filter
    if ( cellFit || this->errorHandlingFlag == svkHSVD::IGNORE_ERROR ) {
        //  loop over each time point in FID: 
        for ( int i = 0; i < numTimePoints; i++ ) {

            //cout << "TIME POINT: " << i << endl;

            //  Initialize the filter spectrum to 0
            float filterTuple[2]; 
            filterTuple[0] = 0.; 
            filterTuple[1] = 0.; 

            //  loop over poles (peakNumber): 
            //cout << endl;
            for ( int pole = 0; pole < hsvdModel->size(); pole++) {

                //      0       amplitude
                //      1       phase
                //      2       frequency 
                //      3       damping 
                double amp   = (*hsvdModel)[pole][0]; 
                double phase = (*hsvdModel)[pole][1]; 
                double freq  = (*hsvdModel)[pole][2]; 
                double damp  = (*hsvdModel)[pole][3]; 

                if ( this->GetDebug() && i == 0 ) {
                    //cout << "pole: " << pole << " amp, phase, freq, damp: " << amp << " " << phase << " " << freq << " " << damp << endl;
                }

                addSVDComponent = false;

                //  Check to see if this HSVD components should contribute to the filter 
                //  spectrum or not.  There may be more than one filter rule, e.g. filter
                //  water and fat, etc.
                
                for (int filterRule = 0; filterRule < this->filterRules.size(); filterRule++) {
                    if (
                        //( sweepWidth * freq >= this->filterRules[filterRule][0]
                        //&& sweepWidth * freq <= this->filterRules[filterRule][1] )
                        ( -1 * freq >= this->filterRules[filterRule][0]
                        && -1 * freq <= this->filterRules[filterRule][1] )
                        || damp < -1. * this->filterRules[filterRule][2]
                    )  {
                        addSVDComponent = true;
                        if ( i == 0 ) {
                            //cout << "FILTER RULE: " << this->filterRules[filterRule][0] 
                            //<< " to " << this->filterRules[filterRule][1] << endl;
                            //cout << "   AMP: " << amp << " FREQ " << freq << endl ;
                        }
                    }
                }

                if ( addSVDComponent ) {

                    double PI      = vtkMath::Pi();
                    double dT      =  1./sweepWidth;

                    //  get angular frequency argument
                    double omegaT  = 2. * PI * freq * i * dT;

                    //  damping term
                    double damping = exp( -1 * damp * i * dT);

                    //  phase is in radians
                    double phi     = phase;

                    if ( this->GetDebug() ) {
                        if ( i == 0 ) {
                            //cout << "       amp frequency phi damping : " <<  amp
                            //<< " " << omegaT << " "  << phi << " " << damping<< endl;
                        }
                    }

                    filterTuple[0] += (amp * cos( phi + omegaT ) * damping);
                    filterTuple[1] += (amp * sin( phi + omegaT ) * damping);
                    //cout << "                  FT: " << filterTuple[0] << " " << filterTuple[1] << endl;
                }
            } // pole loop
            filterSpectrum->SetTuple( i, filterTuple );
            // If filter failed and not ignoring error break;
            if (i == this->numberPtsToCheckQuality){ // check first N points
                float qfactor;
                filterFailed = this->GetFilterFailStatus(cellID, filterSpectrum, &qfactor);
                if (filterFailed &&  this->errorHandlingFlag != svkHSVD::IGNORE_ERROR ){
                    break;
                }
            }
        } // numpoint loop
    } // if

    if ((!cellFit || filterFailed)){
        // cell was corrupt or filter failed
        float tuple[2];

        // we need to handle only if we are not ignoring the error!
        switch ( this->errorHandlingFlag ) {

            case svkHSVD::SET_SIGNAL_TO_ZERO: // Copy Signal Spectrum into Filter Spectrum
            {
                vtkFloatArray* spectrum = static_cast<vtkFloatArray*>( data->GetSpectrum( cellID ) );
                for (int i = 0; i < numTimePoints; i++) {
                    spectrum->GetTupleValue(i,tuple);
                    filterSpectrum->SetTuple(i, tuple);  // just case the filter spectrum gets output it will contain the input signal
                }
                break;
            }
            case svkHSVD::SET_FILTER_TO_ZERO: // Set Filter Spectrum to 0
            {
                for (int i = 0; i < numTimePoints; i++) {
                    tuple[0] = 0.;
                    tuple[1] = 0.;
                    filterSpectrum->SetTuple(i, tuple);
                }
                break;
            }
        }
    }

    if (this->fitSuccessMap != NULL){
        this->fitSuccessMap->SetImagePixel(cellID,  (cellFit && !filterFailed)?1.0:0.0);
    }
    return;
}

bool svkHSVD::GetFitSuccessStatus(){

    svkDcmHeader::DimensionVector dimensionVector =  this->fitSuccessMap->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector loopVector = dimensionVector;
    bool bResult = false;

    if (this->fitSuccessMap == NULL){
        return bResult;
    }
    bResult = true;
    int numCells = this->fitSuccessMap->GetNumberOfCells();

    for (int i=0; i<numCells && (bResult == true);i++){
        svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimensionVector, &loopVector, i );
        // If only PRESSBox we should only consider cells in the Box
        if ( this->onlyFitInVolumeLocalization == true ) {
            int spatialCellIndex = svkDcmHeader::GetSpatialCellIDFromDimensionVectorIndex( &dimensionVector, &loopVector);
            if ( this->selectionBoxMask[spatialCellIndex] == 1 ){
                bResult &=  (*(this->fitSuccessMap->GetImagePixel(i)) == 1.0?true:false);
            }
        }
    }
    return bResult;
}

/*!
 *  Subtract model filter from input
 */
void svkHSVD::SubtractFilter()
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    svkDcmHeader* hdr     = data->GetDcmHeader(); 

    //   for each cell /spectrum: 
    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );
    int firstCell = 0; 
    int numTimePoints = this->numTimePoints; 

    //for ( int cellID = firstCell; cellID < numCells; cellID+=4 ) {
    for ( int cellID = firstCell; cellID < numCells; cellID++ ) {
        vtkFloatArray* spectrum = static_cast<vtkFloatArray*>( data->GetSpectrum( cellID ) );
        float tuple[2];  
        vtkFloatArray* filterSpectrum = static_cast<vtkFloatArray*>( this->filterImage->GetSpectrum( cellID ) );
        float filterTuple[2]; 

        for (int i = 0; i < numTimePoints; i++) {
            filterSpectrum->GetTupleValue(i, filterTuple);
            spectrum->GetTupleValue(i, tuple);
            tuple[0] -= filterTuple[0]; 
            tuple[1] -= filterTuple[1]; 
            spectrum->SetTuple(i, tuple);  
        }
    }
}


/*!
 *  Add a filter rule, defined by a frequency range and damping threshold (all frequencies with 
 *  damping greather than the threshold will be filtered out (fast decaying components). 
 *  Specify frequency in PPM (order doesn't matter).  Filter includes limiting frequencies. 
 */
void svkHSVD::AddFrequencyAndDampingFilterRule( float frequencyLimit1PPM, float frequencyLimit2PPM, float dampingThreshold )
{
    if ( this->GetImageDataInput(0)== NULL ) { 
        cerr << " svkHSVD: Must set input data before specifying filters" << endl;
    }

    svkSpecPoint* point = svkSpecPoint::New();
    point->SetDcmHeader( this->GetImageDataInput(0)->GetDcmHeader() );

    float lowFrequencyLimitHz = point->ConvertPosUnits( frequencyLimit1PPM, svkSpecPoint::PPM, svkSpecPoint::Hz); 
    float highFrequencyLimitHz = point->ConvertPosUnits( frequencyLimit2PPM, svkSpecPoint::PPM, svkSpecPoint::Hz); 

    if ( highFrequencyLimitHz < lowFrequencyLimitHz ) {
        float tmpLimit = lowFrequencyLimitHz; 
        lowFrequencyLimitHz   = highFrequencyLimitHz; 
        highFrequencyLimitHz= tmpLimit; 
    }

    vector< float > filter; 
    filter.push_back( lowFrequencyLimitHz); 
    filter.push_back( highFrequencyLimitHz); 
    filter.push_back( dampingThreshold ); 

    this->filterRules.push_back( filter );

    point->Delete();

}


/*!
 *  Remove H20 from proton spectra (all frequencies downfield of 4.2PPM, i.e. higher PPM). 
 */
void svkHSVD::RemoveH20On()
{
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    if ( data == NULL ) { 
        cerr << " svkHSVD: Must set input data before specifying filters" << endl;
    }
    svkDcmHeader* hdr = data->GetDcmHeader(); 
       
    svkSpecPoint* point = svkSpecPoint::New();
    point->SetDcmHeader( hdr );
       
    float downfieldPPMLimit = point->ConvertPosUnits( 0, svkSpecPoint::PTS, svkSpecPoint::PPM);
    this->AddPPMFrequencyFilterRule( downfieldPPMLimit, 4.2 );
       
    point->Delete();
}


/*!
 *  Remove lipid from proton spectra (all frequencies upfield of 1.8 PPM, i.e. lower PPM). 
 */
void svkHSVD::RemoveLipidOn()
{
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    if ( data == NULL ) { 
        cerr << " svkHSVD: Must set input data before specifying filters" << endl;
    }
    svkDcmHeader* hdr = data->GetDcmHeader(); 

    svkSpecPoint* point = svkSpecPoint::New();
    point->SetDcmHeader( hdr );

    if ( this->numTimePoints < 0 ) {  
        this->numTimePoints = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );
    }
    int maxPoint = this->numTimePoints; 
    float upfieldPPMLimit = point->ConvertPosUnits( maxPoint-1, svkSpecPoint::PTS, svkSpecPoint::PPM);
    //cout << "MAX UPFIELD: " << maxPoint-1 << " " << upfieldPPMLimit << endl;
    this->AddPPMFrequencyFilterRule( 1.8, upfieldPPMLimit );

    point->Delete();
}



/*!
 *  Add a filter rule, defined by a frequency range (any damping value).  
 *  Specify frequency in PPM.  Filter includes limiting frequencies. 
 */
void svkHSVD::AddPPMFrequencyFilterRule( float frequencyLimit1PPM, float frequencyLimit2PPM )
{

    if ( this->GetImageDataInput(0)== NULL ) { 
        cerr << " svkHSVD: Must set input data before specifying filters" << endl;
    }

    svkSpecPoint* point = svkSpecPoint::New();
    point->SetDcmHeader( this->GetImageDataInput(0)->GetDcmHeader() );

    float lowFrequencyLimitHz  = point->ConvertPosUnits( frequencyLimit1PPM, svkSpecPoint::PPM, svkSpecPoint::Hz); 
    float highFrequencyLimitHz = point->ConvertPosUnits( frequencyLimit2PPM, svkSpecPoint::PPM, svkSpecPoint::Hz); 
    float dampingThreshold = 0.0; 

    if ( highFrequencyLimitHz < lowFrequencyLimitHz ) {
        float tmpLimit = lowFrequencyLimitHz; 
        lowFrequencyLimitHz = highFrequencyLimitHz; 
        highFrequencyLimitHz = tmpLimit; 
    }

    vector< float > filter; 
    filter.push_back( lowFrequencyLimitHz ); 
    filter.push_back( highFrequencyLimitHz ); 
    filter.push_back( dampingThreshold ); 
    cout << "AddPPMFrequencyFilterRule: " << frequencyLimit1PPM << " to " << frequencyLimit2PPM << endl;
    //cout << "SET FILTER: " << lowFrequencyLimitHz  << " to " << highFrequencyLimitHz << endl;
    this->filterRules.push_back( filter );

    point->Delete();
    
}


/*!
 *  Add a filter rule, defined by a damping value ( entire frequency range)
 */
void svkHSVD::AddDampingFilterRule( float dampingThreshold )
{

    if ( this->GetImageDataInput(0)== NULL ) { 
        cerr << " svkHSVD: Must set input data before specifying filters" << endl;
    }
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    svkDcmHeader* hdr = data->GetDcmHeader(); 
    int numTimePoints = this->numTimePoints; 

    svkSpecPoint* point = svkSpecPoint::New();
    point->SetDcmHeader( this->GetImageDataInput(0)->GetDcmHeader() );

    float lowFrequencyLimitHz  = point->ConvertPosUnits( 0,  svkSpecPoint::PPM, svkSpecPoint::Hz); 
    float highFrequencyLimitHz = point->ConvertPosUnits( numTimePoints, svkSpecPoint::PPM, svkSpecPoint::Hz); 
    dampingThreshold = 0.0; 

    if ( highFrequencyLimitHz < lowFrequencyLimitHz ) {
        float tmpLimit = lowFrequencyLimitHz; 
        lowFrequencyLimitHz  = highFrequencyLimitHz; 
        highFrequencyLimitHz = tmpLimit; 
    }

    vector< float > filter; 
    filter.push_back( lowFrequencyLimitHz ); 
    filter.push_back( highFrequencyLimitHz ); 
    filter.push_back( dampingThreshold ); 

    this->filterRules.push_back( filter );

    point->Delete();

}


/*!
 *
 */
void svkHSVD::MatVec(const doublecomplex* matrix, const doublecomplex* vector, int m, int n, doublecomplex* result)
{
    for( int i = 0; i < m; ++i ) {
        double res_r = 0.;
        double res_i = 0.;
        int ink = i * n;
        for(int k = 0; k < n; ++k ) {
            double prod1 = (matrix[ink].r - matrix[ink].i)*(vector[k].r+vector[k].i);
            double prod2 = matrix[ink].r*vector[k].r;
            double prod3 = matrix[ink].i*vector[k].i;
            res_r += (prod2+prod3);
            res_i += (prod1-prod2+prod3);
            ink++;
        }
        result[i].r = res_r;
        result[i].i = res_i;
    }
}


/*!
 *  This function calculates the product result = matrix^H*matrix, with ^H denoting the 
 *  Hermitian transpose matrix or conjugate transpose.
 */
void svkHSVD::MatSq(const doublecomplex* matrix, int m, int n, doublecomplex* result)
{
    doublecomplex a,b;
    for ( int targetCol = 0; targetCol< m; ++targetCol){
        for ( int targetRow = targetCol; targetRow< m; ++targetRow){ // calc only upper diagonal and copy values since symmetric!
            double res_r1 = 0.;
            double res_i1 = 0.;
            double res_r2 = 0.;
            double res_i2 = 0.;
            for (int sourceIdx = 0; sourceIdx < n; ++ sourceIdx){
                a = matrix[targetCol*n+sourceIdx]; 
                b = matrix[targetRow*n+sourceIdx];
                res_r1 += a.r*b.r+a.i*b.i;
                res_i1 += -a.r*b.i+a.i*b.r;
            
                res_i2 += a.r*b.i-a.i*b.r;
            }
            result[targetCol*m+targetRow].r = res_r1;
            result[targetCol*m+targetRow].i = res_i1;
            if ( targetRow != targetCol ) {
                result[targetRow*m+targetCol].r = res_r1;   //same as real1
                result[targetRow*m+targetCol].i = res_i2;
            }
        }
    }
}


/*!
 *
 */
void svkHSVD::MatMat(const doublecomplex* matrix1, const doublecomplex* matrix2, int m, int n, doublecomplex* result)
{
    const int mm1 = m*m-1;
    for( int i = 0; i < m; ++i ){
        int ink = i*n;
        int jmi = i;
        double res_r = 0.;
        double res_i = 0.;
        for( int jnk = 0; jnk < n*m; ++jnk) {
            // ink = i*n+k, jnk = j*n+k, jmi = j*m+i = (jnk/n)*m+i
            double prod1 = (matrix1[ink].r - matrix1[ink].i)*(matrix2[jnk].r + matrix2[jnk].i);
            double prod2 = matrix1[ink].r * matrix2[jnk].r;
            double prod3 = matrix1[ink].i * matrix2[jnk].i;
            res_r += (prod2+prod3);
            res_i += (prod1-prod2+prod3);
            ink++;
            if( (jnk+1) % n == 0 ){
                ink = i*n;
                result[jmi].r = res_r;
                result[jmi].i = res_i;
                jmi += m;
                // the next two lines should be unnecessary?
                if( jmi != mm1 ) {
                    jmi %= mm1;
                }
                res_r = res_i = 0.;
            }
        }
    }
}

/*!
 *  Loop through spectra within the specified sub-extent and apply HSVD to each. 
 */   
void svkHSVD::svkHSVDExecute(int ext[6], int id) 
{
    vtkIdType in1Inc0, in1Inc1, in1Inc2;

    // Get information to march through data
    this->GetImageDataInput(0)->GetContinuousIncrements(ext, in1Inc0, in1Inc1, in1Inc2);
    //cout << "   outExt sub: " << id << " " << ext[0] << " " << ext[1] << endl;
    //cout << "   outExt sub: " << id << " " << ext[2] << " " << ext[3] << endl;
    //cout << "   outExt sub: " << id << " " << ext[4] << " " << ext[5] << endl;
    //cout << endl <<  endl;
    

    // Loop through spectra in the given extent 
    svkDcmHeader::DimensionVector dimensionVector = this->GetImageDataInput(0)->GetDcmHeader()->GetDimensionIndexVector();
    //svkDcmHeader::PrintDimensionIndexVector(&dimensionVector); 
    svkDcmHeader::DimensionVector loopVector = dimensionVector;

    
    int numThreads = this->GetNumberOfThreads();
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );

    for (int cellID = 0; cellID < numCells; cellID++) {

        svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimensionVector, &loopVector, cellID );

        //  if restricting fit to selection box, then check to see if current cell is in box, otherwise continue:     
        if ( this->onlyFitInVolumeLocalization == true ) { 
            //  Get the 3D spatial index for comparing if a given cell is in the spatial selectin box maks:   
            int spatialCellIndex = svkDcmHeader::GetSpatialCellIDFromDimensionVectorIndex( &dimensionVector, &loopVector); 
            if ( this->selectionBoxMask[spatialCellIndex] == 0 ) {
                continue; 
            }
        }

        bool isCellInSubExtent = svk4DImageData::IsIndexInExtent( ext, &loopVector ); 
        if ( isCellInSubExtent ) { 
            //cout << "CELL TO FIT: " << cellID << endl;
            this->HSVDFitCellSpectrum( cellID ); 
            svkHSVD::progress[id]++; 

            int cellCount = 0; 
            for ( int t = 0; t < numThreads; t++ ) {
                cellCount = cellCount + svkHSVD::progress[t]; 
            }
            int percent = 100 * (double)cellCount/(double)numCells; 
            if ( id == 0 && percent % 1 == 0 ) {
                //cout << " CC: " << id << " " << percent << endl; 
                this->UpdateProgress(percent/100.);
            }
        }

    }

}


/*! 
 *  This method is passed an input and output Datas, and executes the filter
 *  algorithm to fill the output from the inputs.
 *  It just executes a switch statement to call the correct function for
 *  the Datas data types.
 */
void svkHSVD::ThreadedRequestData(
    vtkInformation * vtkNotUsed( request ), 
    vtkInformationVector ** vtkNotUsed( inputVector ), 
    vtkInformationVector * vtkNotUsed( outputVector ),
    vtkImageData ***inData, 
    vtkImageData **outData,
    int outExt[6], int id)
{

    cout << "THREADED EXECUTE " << id << endl ;
    this->svkHSVDExecute(outExt, id);
                          
}

