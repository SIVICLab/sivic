/*
 *  Copyright © 2009-2013 The Regents of the University of California.
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
 *      Beck Olson,
 */


#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <svkMRSAutoPhase.h>
#include <svkSpecUtils.h>
#include <svkPhaseSpec.h>
#include <vtkImageFourierFilter.h> // for vtkImageComplex struct


#include <cminpack.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

# define real __cminpack_real__

using namespace svk;


vtkCxxRevisionMacro(svkMRSAutoPhase, "$Rev$");
vtkStandardNewMacro(svkMRSAutoPhase);
int* svkMRSAutoPhase::progress; //  static pointer 

/* 
 *  Use for user provided Jacobian
 */
int fcn_lmder(void* p, int m, int n, const real* x, real* fvec, real* fjac, int ldfjac, int iflag);

/* 
 *  Use for no Jacobian Provided 
 */
int fcn_lmdif(void* p, int numCriteria, int numParams, const double* x, double* fvec, int iflag);


/*
 */
int fcn_hyb(void* fcnData, int numParams, const double* x, double* fvec, int iflag); 


/* 
 *  the following struct defines the data points 
 */
typedef struct  {
    svkMRSAutoPhase::phasingModel phasingModelType; 
    int             numTimePoints;
    vtkFloatArray*  spectrum;
    float           maxValue;
} fcndata_t;


/*!
 *
 */
svkMRSAutoPhase::svkMRSAutoPhase()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    //  1 required input ports: 
    this->SetNumberOfInputPorts(1);
    this->SetNumberOfThreads(16);
    this->SetNumberOfThreads(1);
    svkMRSAutoPhase::progress = NULL;
    this->SetPhasingModel(svkMRSAutoPhase::MIN_AREA_0); 

}



/*!
 *
 */
svkMRSAutoPhase::~svkMRSAutoPhase()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}


/*!
 *
 *  MIN_AREA_0: 
 *      fit zero order phase based in minizing the integrated 
 *      area of spectrum. 
 *  MIN_AREA_MAX_POSITIVE_0:  
 *      fit zero order phase based in minizing the 
 *      integrated area of spectrum and maximizing the positive peak
 *      height  
 *  MIN_MAGNITUDE_DIFF_0 
 *      fit zero order phase based in minizing sum of squares difference between  
 *      real and complex spectrum 
 */
void svkMRSAutoPhase::SetPhasingModel(svkMRSAutoPhase::phasingModel modelType) 
{
    this->phaseModelType = modelType;  
}


/*!
 *  Resets the origin and extent for correct initialization of output svkMriImageData object from input 
 *  svkMrsImageData object. 
 */
int svkMRSAutoPhase::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    return 1; 

}


/*!
 *  Copy the Dcm Header and Provenance from the input to the output. 
 */
int svkMRSAutoPhase::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0));

    //  Make sure input spectra are in frequency domain
    //this->CheckInputSpectralDomain();

    this->numTimePoints = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );

    if ( svkMRSAutoPhase::progress == NULL ) {
        int numThreads = this->GetNumberOfThreads();
        svkMRSAutoPhase::progress = new int[numThreads];
        for ( int t = 0; t < numThreads; t++ ) {
            svkMRSAutoPhase::progress[t] = 0;
        }
    }

    ostringstream progressStream;
    progressStream << "Executing Phase Correction";
    this->SetProgressText( progressStream.str().c_str() );
    this->UpdateProgress(.0);

    //  This will call AutoPhaseCellSpectrum foreach cell within the 
    //  sub-extent within each thread.
    this->Superclass::RequestData(
        request,
        inputVector,
        outputVector
    );

    if ( svkMRSAutoPhase::progress != NULL ) {
        delete [] svkMRSAutoPhase::progress;
        svkMRSAutoPhase::progress = NULL;
    }

    //  Make sure input spectra are in time domain for HSVD filter. 
    //this->CheckOutputSpectralDomain();

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->GetInput()->Update();

    return 1;
};


/*! 
 *  This method is passed an input and output Data, and executes the filter
 *  algorithm to fill the output from the inputs.
 *  id = id of thread
 */
void svkMRSAutoPhase::ThreadedRequestData(
  vtkInformation * vtkNotUsed( request ),
  vtkInformationVector ** vtkNotUsed( inputVector ),
  vtkInformationVector * vtkNotUsed( outputVector ),
  vtkImageData ***inData,
  vtkImageData **outData,
  int outExt[6], int id)
{

    cout << "THREADED EXECUTE " << id << endl ;
    this->AutoPhaseExecute(outExt, id);

}



/*!
 *  Loop through spectra within the specified sub-extent and apply auto phase
 *  algo to each. 
 */
void svkMRSAutoPhase::AutoPhaseExecute(int* ext, int id)
{

    vtkIdType in1Inc0, in1Inc1, in1Inc2;

    // Get information to march through data
    this->GetImageDataInput(0)->GetContinuousIncrements(ext, in1Inc0, in1Inc1, in1Inc2);

    // Loop through spectra in the given extent 
    svkDcmHeader::DimensionVector dimensionVector = this->GetImageDataInput(0)->GetDcmHeader()->GetDimensionIndexVector();
   svkDcmHeader::DimensionVector loopVector = dimensionVector;

    int numThreads = this->GetNumberOfThreads();
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );
    for (int cellID = 0; cellID < numCells; cellID++) {

        svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimensionVector, &loopVector, cellID );
        bool isCellInSubExtent = svk4DImageData::IsIndexInExtent( ext, &loopVector ); 
        if ( isCellInSubExtent ) { 

            cout << "CELL TO FIT: " << cellID << endl;
            this->AutoPhaseSpectrum( cellID );

            //  Update progress: 
            svkMRSAutoPhase::progress[id]++;

            int cellCount = 0;
            for ( int t = 0; t < numThreads; t++ ) {
                   cellCount = cellCount + svkMRSAutoPhase::progress[t];
            }
            int percent = static_cast<int>(100 * (double)cellCount/(double)numCells);
            if ( id == 0 && percent % 1 == 0 ) {
                this->UpdateProgress(percent/100.);
            }
        }
    }

}


/*
 *  Apply auto-phase algo to a single spectrum identified by its cellID
 */
void svkMRSAutoPhase::AutoPhaseSpectrum( int cellID )
{

    if ( this->GetDebug() ) {
        cout << "AutoPhase Spectrum: " << cellID << endl;
        cout << "TIME POINTS: " << this->numTimePoints << endl;
    }

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0));
    vtkFloatArray* spectrum = static_cast<vtkFloatArray*>( data->GetSpectrum( cellID ) );
	
    //  Set up dynamic variable arrray for cmin_pack 
    int numCriteria;
    int numParamsToFit; 
    //  Initialize the function data to be passed to the cost function 
    if ( this->phaseModelType == svkMRSAutoPhase::MIN_AREA_0 ) {

        //  fit zero order phase
        //  minimize the area (1 criterion) 
        numCriteria = 1;  
        numParamsToFit = 1; 

    } else if ( this->phaseModelType == svkMRSAutoPhase::MIN_AREA_MAX_POSITIVE_0 ) {

        //  fit zero order phase
        //  minimize the area and maximize positive peak height (2 criterion) 
        numCriteria = 2;  
        numParamsToFit = 1; 

    } else if ( this->phaseModelType == svkMRSAutoPhase::MIN_MAGNITUDE_DIFF_0 ) {

        //  fit zero order phase
        //  minimize the area and maximize positive peak height (2 criterion) 
        numCriteria = 1;    //  just differ between mag and real
        numParamsToFit = 1; 
    } 

    //find max spec value 
    float maxVal = FLT_MIN;
    float cmplxPt[2];
    for  ( int t = 0; t < this->numTimePoints; t++ ){
        // apply zero order phase to data:  
        spectrum->GetTupleValue(t, cmplxPt);
        float tmp = pow( cmplxPt[0]*cmplxPt[0] + cmplxPt[1]*cmplxPt[1], float(0.5)); 
        if ( tmp > maxVal )  {
            maxVal = tmp; 
        }
    }

	fcndata_t fcnData;	
    fcnData.phasingModelType = this->phaseModelType; 
	fcnData.numTimePoints = this->numTimePoints;
    fcnData.spectrum = spectrum; 
    fcnData.maxValue = maxVal; 

    int*    ipvt = new int[numParamsToFit]; 
    double* x    = new double[numParamsToFit];
	double* fvec = new double[numCriteria];
	double* diag = new double[numParamsToFit];
	double* fjac = new double[numCriteria];
	double* qtf  = new double[numParamsToFit];
	double* wa1  = new double[numParamsToFit];
	double* wa2  = new double[numParamsToFit];
	double* wa3  = new double[numParamsToFit];
	double* wa4  = new double[numCriteria];


    float phi0 = 0.; 
    float phi1 = 0.; 
	x[0] = phi0; 
	x[1] = phi1; 
	
	int ldfjac = numCriteria; 

	//  Set ftol and xtol to the square root of the machine 
	//  and gtol to zero. unless high solutions are 
	//  required, these are the recommended settings. 
	real ftol = sqrt(__cminpack_func__(dpmpar)(1));
	real xtol = sqrt(__cminpack_func__(dpmpar)(1));
    real gtol = 0.; 
    real factor = 100; 
    real fnorm; 
    real epsfcn = 1.;  // critical for fitting ( exploring param space sufficiently? )
	int  maxfev = 2000;
    int  mode = 1;
	
	
    //  http://devernay.free.fr/hacks/cminpack/lmdif_.html
 	//  see examples from cminpack-1.3.0/examples/tlmderc.c
    //
	//  lmder: User defined Jacobian
    //      - Not used here. 
	//  lmdif: Calculates Jacobian based on derivatives
    //      - used in this case
	
    int nprint, nfev, njev;
    /*  remove temp
 	int info = __cminpack_func__(lmdif)(
                    fcn_lmdif, 
                    &fcnData, 
                    numCriteria, 
                    numParamsToFit, 
                    x, 
                    fvec, 
                    ftol, 
                    xtol, 
                    gtol, 
                    maxfev, 
                    epsfcn, 
                    diag, 
                    mode, 
                    factor, 
                    nprint, 
                    &nfev, 
                    fjac, 
                    ldfjac,
                    ipvt, 
                    qtf, 
                    wa1, 
                    wa2, 
                    wa3, 
                    wa4);
     */

// hybrd1 version: 
real tol = sqrt(__cminpack_func__(dpmpar)(1));
real wa[2660];
const int lwa = 2660;
int info = __cminpack_func__(hybrd1)(fcn_hyb, &fcnData, numParamsToFit, x, fvec, tol, wa, lwa);

/* remove temp	
    fnorm = __cminpack_func__(enorm)(numCriteria, fvec);
	
	//
	//  Look at rank, covariance and residuals to ensure goodness of fit 
	// 
	printf("    final 12 norm of the residuals:%15.7g\n\n",(double)fnorm);
	printf("    number of function evaulations:%15.7g\n\n",fvec);
	printf("    exit parameter                %10i\n\n",info);
	switch (info){
	    case 0: printf(" improper iin put parameters "); break;
	    case 1: printf(" F_error < ftol "); break;
	    case 2: printf(" delta < xtol*xnorm "); break;
	    case 3: printf(" both_error < tol "); break;
	    case 4: printf(" cos(angle fvec Jacobian < gtol "); break;
	    case 5: printf(" n ftn evaluation > maxfev "); break;
	    case 6: printf(" too small ftol for F_error "); break;
	    case 7: printf(" too small xtol for x_error "); break;
	    case 99: printf(" too small gtol for cos(angle) "); break;
	}
	printf("\n");
	printf("\n"); 

	ftol = __cminpack_func__(dpmpar)(1);
	
#ifdef TEST_COVAR
{
    // test the original covar from MINPACK 
    real covfac = fnorm*fnorm/(combinedNumberOfTimePoints-numMets);
    real fjac1[15*3];
    memcpy(fjac1, fjac, sizeof(fjac));
    covar(n, fjac1, ldfjac, ipvt, ftol, wa1);
    printf("      covariance (using covar)\n");
    for (i=0; i < numMets; ++i) {
        for (j=0; j < numMets; ++j){
            printf("%s%15.7g", j%3==1?"\n     ":"", (double)fjac1[i*ldfjac+j]*covfac);
        }
    }
    printf("\n");
}
#endif
  
    // test covar1, which also estimates the rank of the Jacobian 
    int k = __cminpack_func__(covar1)(
                numCriteria, 
                numParamsToFit, 
                fnorm*fnorm, 
                fjac, 
                ldfjac, 
                ipvt, 
                ftol, 
                wa1);

    printf("      covariance\n");
    for (int i = 0; i < numParamsToFit; ++i) {
        for (int j = 0; j < numParamsToFit; ++j){
            printf("%s%15.7g", j%3==0?"\n     ":"", (double)fjac[i*ldfjac+j]);
	    }
    }
    printf("\n");
    printf("\n");
    printf("      rank(J) = %d\n", k != 0 ? k : numParamsToFit); 
    printf("\n");  

*/
    //
    //  Apply fitted phase values to spectrum: 
    //
    cout << "           PHI0 FINAL("<<cellID <<"): " << x[0] * 180 / 3.1415 << endl;
    //float cmplxPt[2]; 
    float phi0Final = x[0]; 
    for (int i = 0; i < this->numTimePoints; i++) {
            
        spectrum->GetTupleValue(i, cmplxPt);
        //cout << "cmplxPt: " << cmplxPt[0] << " " << cmplxPt[1] << endl;
        svk::svkPhaseSpec::ZeroOrderPhase(phi0Final, cmplxPt); 
             
        spectrum->SetTuple(i, cmplxPt); 
        //cout << "cmplxPt: " << cmplxPt[0] << " " << cmplxPt[1]<< endl;
    }

	// clean up memory:
	delete[] ipvt;
	delete[] x;
	delete[] fvec;
	delete[] diag;
	delete[] fjac;
	delete[] qtf;
	delete[] wa1;
	delete[] wa2;
	delete[] wa3;
	delete[] wa4;

	return;
}




/*
 * lmder stub
 */
int fcn_lmder(void *p, int m, int n, const real *x, real *fvec, real *fjac, int ldfjac, int iflag)
{
    return 0;
}


/*!
 *  model 
 *  Calculate the functions at x and return 
 *  the values in fvec[0] through fvec[m-1]
 *  The value of iflag should not be changed by fcn unless 
 *  the user wants to terminate execution of lmdif_. 
 *  In this case set iflag to a negative integer. 
 *
 *  fcnData    is a positive integer input variable set to the number of functions.
 *  m          is a positive integer with number of values to base fit on ("oberservations"), in this case
 *             m is 1 if only area is being considered, or 2 if area and pk ht, for example. 
 *  n          is a positive integer input variable set to the number of variables. n must not exceed m.
 *  x          is an array of length n. On input x must contain an initial estimate of the solution vector. 
 *             On output x contains the final estimate of the solution vector.
 *  fvec       is an output array of length m which contains the functions evaluated at the output x. 
 */
int fcn_lmdif(void* fcnData, int numCriteria, int numParams, const double* x, double* fvec, int iflag)
{

    // extract the arrays from the fcndata_t struct:
    svkMRSAutoPhase::phasingModel phasingModelType = ((fcndata_t*)fcnData)->phasingModelType;
    vtkFloatArray* spectrum = ((fcndata_t*)fcnData)->spectrum;
    int numTimePoints = ((fcndata_t*)fcnData)->numTimePoints;
    float maxValue = ((fcndata_t*)fcnData)->maxValue;
    cout << "MV: " << maxValue << endl;


    // obtain the current values for phi0 and phi1
    if ( phasingModelType == svkMRSAutoPhase::MIN_AREA_0 ) { 

        float phi0 = x[0]; 
        //float phi1 = x[1]; 
        //cout << "fcn_lmdif: phi0:           " << phi0 * 180/ 3.1415 << endl;
        //cout << "fcn_lmdif: phi1:           " << phi1 << endl;
  
        //  Create the first order phase shift factors
        //vtkImageComplex* phi1Array = new vtkImageComplex[ numTimePoints];
        //int pivotFrequency = 0; 
        //svkSpecUtils::CreateLinearPhaseShiftArray(numTimePoints, phi1Array, phi1, pivotFrequency); 

        //  find total area of spectrum with given zero and first order phase values. 
        //  This is the metric being minimized.  
        double area = 0.; 
        float cmplxPt[2];
        for  ( int t = 0; t < numTimePoints; t++ ){

            // apply zero order phase to data:  
            spectrum->GetTupleValue(t, cmplxPt);
            svk::svkPhaseSpec::ZeroOrderPhase(phi0, cmplxPt); 
            
            //  minimize abs area.  should correspond to absorption spectrum
            area = area + abs( cmplxPt[0]); 
        }
    
        fvec[0] = area; 

    } else if ( phasingModelType == svkMRSAutoPhase::MIN_AREA_MAX_POSITIVE_0 ) {

        float phi0 = x[0]; 
  
        //  find total area of spectrum with given zero and first order phase values. 
        //  This is the metric being minimized.  
        double area = 0.; 
        double negativePeakHeight = 0.; 
        float cmplxPt[2];
        for  ( int t = 0; t < numTimePoints; t++ ){

            // apply zero order phase to data:  
            spectrum->GetTupleValue(t, cmplxPt);
            svk::svkPhaseSpec::ZeroOrderPhase(phi0, cmplxPt); 

            //  minimize abs area.  should correspond to absorption spectrum
            area = area + abs( cmplxPt[0]); 
            
            //  maximize positive peak height (minimize negative peak ht) 
            float liftedValue = cmplxPt[0] + maxValue; 
            negativePeakHeight = (negativePeakHeight - liftedValue); 

        }
        cout << "phi0: " << phi0 * 180. / 3.1415 ;
        cout << "   pkHt: " << negativePeakHeight << endl;
    
        //fvec[0] = area; 
        fvec[0] = area; 
        fvec[0] = negativePeakHeight; 
        fvec[1] = negativePeakHeight; 

    } else if ( phasingModelType == svkMRSAutoPhase::MIN_MAGNITUDE_DIFF_0 ) {

        float phi0 = x[0]; 
  
        //  find total area of spectrum with given zero and first order phase values. 
        //  This is the metric being minimized.  
        double sumOfSquares = 0.; 
        float cmplxPt[2];
        for  ( int t = 0; t < numTimePoints; t++ ){

            // apply zero order phase to data:  
            spectrum->GetTupleValue(t, cmplxPt);
            svk::svkPhaseSpec::ZeroOrderPhase(phi0, cmplxPt); 

            //  minimize abs area.  should correspond to absorption spectrum
            sumOfSquares = sumOfSquares 
                        + ( pow(cmplxPt[0]*cmplxPt[0] + cmplxPt[1]*cmplxPt[1],float(0.5)) - cmplxPt[0]); 
            
        }

        cout << "phi0: " << phi0 * 180. / 3.1415 ;
        cout << "   SOSDIFF: " << sumOfSquares << endl;
    
        //fvec[0] = area; 
        fvec[0] = sumOfSquares; 
    }

    return 0;

}


/*!
 *  model 
 *  Calculate the functions at x and return 
 *  the values in fvec[0] through fvec[m-1]
 *  The value of iflag should not be changed by fcn unless 
 *  the user wants to terminate execution of lmdif_. 
 *  In this case set iflag to a negative integer. 
 *
 *  fcnData    is a positive integer input variable set to the number of functions.
 *  m          is a positive integer with number of values to base fit on ("oberservations"), in this case
 *             m is 1 if only area is being considered, or 2 if area and pk ht, for example. 
 *  n          is a positive integer input variable set to the number of variables. n must not exceed m.
 *  x          is an array of length n. On input x must contain an initial estimate of the solution vector. 
 *             On output x contains the final estimate of the solution vector.
 *  fvec       is an output array of length m which contains the functions evaluated at the output x. 
 */
int fcn_hyb(void* fcnData, int numParams, const double* x, double* fvec, int iflag)
{

    // extract the arrays from the fcndata_t struct:
    svkMRSAutoPhase::phasingModel phasingModelType = ((fcndata_t*)fcnData)->phasingModelType;
    vtkFloatArray* spectrum = ((fcndata_t*)fcnData)->spectrum;
    int numTimePoints = ((fcndata_t*)fcnData)->numTimePoints;
    float maxValue = ((fcndata_t*)fcnData)->maxValue;
    //cout << "MV: " << maxValue << endl;


    // obtain the current values for phi0 and phi1
    if ( phasingModelType == svkMRSAutoPhase::MIN_AREA_0 ) { 

        float phi0 = x[0]; 
        //float phi1 = x[1]; 
        //cout << "fcn_lmdif: phi0:           " << phi0 * 180/ 3.1415 << endl;
        //cout << "fcn_lmdif: phi1:           " << phi1 << endl;
  
        //  Create the first order phase shift factors
        //vtkImageComplex* phi1Array = new vtkImageComplex[ numTimePoints];
        //int pivotFrequency = 0; 
        //svkSpecUtils::CreateLinearPhaseShiftArray(numTimePoints, phi1Array, phi1, pivotFrequency); 

        //  find total area of spectrum with given zero and first order phase values. 
        //  This is the metric being minimized.  
        double area = 0.; 
        float cmplxPt[2];
        for  ( int t = 0; t < numTimePoints; t++ ){

            // apply zero order phase to data:  
            spectrum->GetTupleValue(t, cmplxPt);
            svk::svkPhaseSpec::ZeroOrderPhase(phi0, cmplxPt); 
            
            //  minimize abs area.  should correspond to absorption spectrum
            area = area + abs( cmplxPt[0]); 
        }
    
        fvec[0] = area; 

    } else if ( phasingModelType == svkMRSAutoPhase::MIN_AREA_MAX_POSITIVE_0 ) {

        float phi0 = x[0]; 
  
        //  find total area of spectrum with given zero and first order phase values. 
        //  This is the metric being minimized.  
        double area = 0.; 
        double negativePeakHeight = 0.; 
        float cmplxPt[2];
        for  ( int t = 0; t < numTimePoints; t++ ){

            // apply zero order phase to data:  
            spectrum->GetTupleValue(t, cmplxPt);
            //if (t==0) { cout << "initial: " << cmplxPt[0] ;}
            svk::svkPhaseSpec::ZeroOrderPhase(phi0, cmplxPt); 
            //if (t==0) { cout << " final: " << cmplxPt[0] << endl;}

            //  minimize abs area.  should correspond to absorption spectrum
            area = area + abs( cmplxPt[0]); 
            
            //  maximize positive peak height (minimize negative peak ht) 
            negativePeakHeight = negativePeakHeight - cmplxPt[0]; 

        }
        cout << "   phi0: " << phi0 * 180. / 3.1415 ;
        cout << "   pkHt: " << negativePeakHeight << endl;
    
        //fvec[0] = area; 
        fvec[0] = area; 
        fvec[0] = negativePeakHeight; 
        //fvec[1] = negativePeakHeight; 

    } else if ( phasingModelType == svkMRSAutoPhase::MIN_MAGNITUDE_DIFF_0 ) {

        float phi0 = x[0]; 
  
        //  find total area of spectrum with given zero and first order phase values. 
        //  This is the metric being minimized.  
        double sumOfSquares = 0.; 
        float cmplxPt[2];
        for  ( int t = 0; t < numTimePoints; t++ ){

            // apply zero order phase to data:  
            spectrum->GetTupleValue(t, cmplxPt);
            svk::svkPhaseSpec::ZeroOrderPhase(phi0, cmplxPt); 

            //  minimize abs area.  should correspond to absorption spectrum
            sumOfSquares = sumOfSquares 
                        + ( pow(cmplxPt[0]*cmplxPt[0] + cmplxPt[1]*cmplxPt[1],float(0.5)) - cmplxPt[0]); 
            
        }

        cout << "phi0: " << phi0 * 180. / 3.1415 ;
        cout << "   SOSDIFF: " << sumOfSquares << endl;
    
        //fvec[0] = area; 
        fvec[0] = sumOfSquares; 
    }

    return 0;

}


/*!


/*!
 *
 */
void svkMRSAutoPhase::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
}


/*!
 *  input ports 0 - 2 are required. All input ports are for dynamic MRI data. 
 */
int svkMRSAutoPhase::FillInputPortInformation( int port, vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData");
    return 1;
}



/*!
 *  Output from this algo is an svkMriImageData object. 
 */
int svkMRSAutoPhase::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData"); 
    return 1;
}

