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
 *      Beck Olson,
 *      Nonlinear LS fit added by: Christine Leon (Jul 28, 2012)
 */


#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkInformationVector.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>

#include <svkDynamicMRIAlgoTemplate.h>
#include <cminpack.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

# define real __cminpack_real__

using namespace svk;


//vtkCxxRevisionMacro(svkDynamicMRIAlgoTemplate, "$Rev$");
vtkStandardNewMacro(svkDynamicMRIAlgoTemplate);

/* Use for user provided Jacobian*/
int fcn_lmder(void *p, int m, int n, const real *x, real *fvec, real *fjac, int ldfjac, int iflag);

/* Use for no Jacobian Provided */
int fcn_lmdif(void *p, int combinedNumberOfTimePoints, int numMets, const double *x, double *fvec, int iflag);


  /* the following struct defines the data points */
    typedef struct  {
        int m;
        real *y;
    } fcndata_t;


/*!
 *
 */
svkDynamicMRIAlgoTemplate::svkDynamicMRIAlgoTemplate()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    this->newSeriesDescription = ""; 
    //  3 required input ports: 
    this->SetNumberOfInputPorts(3);

}



/*!
 *
 */
svkDynamicMRIAlgoTemplate::~svkDynamicMRIAlgoTemplate()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}


/*!
 *  Set the series description for the DICOM header of the copy.  
 */
void svkDynamicMRIAlgoTemplate::SetSeriesDescription( std::string newSeriesDescription )
{
    this->newSeriesDescription = newSeriesDescription;
    this->Modified(); 
}


/*!
 *  Resets the origin and extent for correct initialization of output svkMriImageData object from input 
 *  svkMrsImageData object. 
 */
int svkDynamicMRIAlgoTemplate::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    int inWholeExt[6];
    inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), inWholeExt);
    double inSpacing[3]; 
    this->GetImageDataInput(0)->GetSpacing( inSpacing );
    

    //  MRI image data output map has the same extent as the input MRI 
    //  image data (points):
    int outUpExt[6];
    int outWholeExt[6];
    double outSpacing[3]; 
    for (int i = 0; i < 3; i++) {
        outUpExt[2*i]      = inWholeExt[2*i];
        outUpExt[2*i+1]    = inWholeExt[2*i+1];
        outWholeExt[2*i]   = inWholeExt[2*i];
        outWholeExt[2*i+1] = inWholeExt[2*i+1];

        outSpacing[i] = inSpacing[i];
    }

    //  MRS Input data has origin at first point (voxel corner).  Whereas output MRI image has origin at
    //  center of a point (point data).  In both cases this is the DICOM origin, but needs to be represented
    //  differently in VTK and DCM: 
    double outOrigin[3];
    this->GetImageDataInput(0)->GetDcmHeader()->GetOrigin( outOrigin ); 

    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), outWholeExt, 6);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), outUpExt, 6);
    outInfo->Set(vtkDataObject::SPACING(), outSpacing, 3);
    outInfo->Set(vtkDataObject::ORIGIN(), outOrigin, 3);

    return 1;
}


/*!
 *  Copy the Dcm Header and Provenance from the input to the output. 
 */
int svkDynamicMRIAlgoTemplate::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    //  Create the template data object by  
    //  extractng an svkMriImageData from the input svImageData object
    //  Use an arbitrary point for initialization of scalars.  Actual data 
    //  will be overwritten by algorithm. 
    int indexArray[1];
    indexArray[0] = 0;
    svkMriImageData::SafeDownCast( this->GetImageDataInput(0) )->GetCellDataRepresentation()->GetImage(
        svkMriImageData::SafeDownCast( this->GetOutput() ), 
        0, 
        this->newSeriesDescription, 
        indexArray, 
        0,
        VTK_DOUBLE
    ); 

    this->GenerateKineticParamMap();

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    hdr->InsertUniqueUID("SeriesInstanceUID");
    hdr->InsertUniqueUID("SOPInstanceUID");
    hdr->SetValue("SeriesDescription", this->newSeriesDescription);

    return 1; 
};


/*!
 *  Generate 3D image parameter map from analysis of dynamic data.  
 */
void svkDynamicMRIAlgoTemplate::GenerateKineticParamMap()
{

    this->ZeroData();

    int numVoxels[3];
    this->GetOutput()->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];


    //  Get the data array to initialize.  
    vtkDataArray* kineticsMapArray;
    kineticsMapArray = this->GetOutput()->GetPointData()->GetArray(0);

    //  Add the output volume array to the correct array in the svkMriImageData object
    std::string arrayNameString("pixels");

    kineticsMapArray->SetName( arrayNameString.c_str() );

    double voxelValue;
    for (int i = 0; i < totalVoxels; i++ ) {

        cout << "VOXEL NUMBER: " << i << endl;
        vtkFloatArray* kineticTrace0 = vtkFloatArray::SafeDownCast(
            svkMriImageData::SafeDownCast(this->GetImageDataInput(0))->GetCellDataRepresentation()->GetArray(i)
        );
        vtkFloatArray* kineticTrace1 = vtkFloatArray::SafeDownCast(
            svkMriImageData::SafeDownCast(this->GetImageDataInput(1))->GetCellDataRepresentation()->GetArray(i)
        );
        vtkFloatArray* kineticTrace2 = vtkFloatArray::SafeDownCast(
            svkMriImageData::SafeDownCast(this->GetImageDataInput(2))->GetCellDataRepresentation()->GetArray(i)
        );

        float* metKinetics0 = kineticTrace0->GetPointer(0);
		float* metKinetics1 = kineticTrace1->GetPointer(0);
		float* metKinetics2 = kineticTrace2->GetPointer(0);
		
        //cout << "NUM COMP: " << kineticTrace->GetNumberOfComponents() << endl;
        //cout << "NUM TUPS: " << kineticTrace->GetNumberOfTuples() << endl;
		
		//this->metKinetics0 = kineticTrace0->GetPointer(0);
        //this->metKinetics1 = kineticTrace1->GetPointer(0);
        //this->metKinetics2 = kineticTrace2->GetPointer(0);

        voxelValue = this->GetKineticsMapVoxelValue( metKinetics0, metKinetics1, metKinetics2  );
		//voxelValue = this->GetKineticsMapVoxelValue( );

        kineticsMapArray->SetTuple1(i, voxelValue);
    }
}
     

/*!  
 *  Fit the kinetics for a single voxel. 
 */
double svkDynamicMRIAlgoTemplate::GetKineticsMapVoxelValue(float* metKinetics0, float* metKinetics1, float* metKinetics2 )
{

    double voxelValue;
	
    //  get num points in kinetic trace: 
    int numPts = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfTimePoints();


    // Get other parameters outside of kinetics that describe kinetics 
	
    //  Get max(aka peak hieght) and min intensity data point for upper and lower bounds:
    float maxValue0 = metKinetics0[0];
    float maxValue1 = metKinetics1[0];
    float maxValue2 = metKinetics2[0];	
    float minValue0 = metKinetics0[0];
    float minValue1 = metKinetics1[0];
    float minValue2 = metKinetics2[0];

    //  Get arrival time:
    float arrival0 = 0;
    float arrival1 = 0;	
    float arrival2 = 0;

    //  Get area under curve time:
    float AreaUnderCurve0 = metKinetics0[0];
    float AreaUnderCurve1 = metKinetics1[0];	
    float AreaUnderCurve2 = metKinetics2[0];
	
    //  Get approximate Full Width Half Max:
    float t_start_FWHM0 = 0; float t_end_FWHM0 = 0; 
    float t_start_FWHM1 = 0; float t_end_FWHM1 = 0; 	
    float t_start_FWHM2 = 0; float t_end_FWHM2 = 0; 

    //  Get Mean Time:
    float MeanTime0 = 0;
    float MeanTime1 = 0;	
    float MeanTime2 = 0;
    float time = 1;
	
    for ( int t = 0; t < numPts; t++ ) {
        cout << "   val: " << t << " " << metKinetics0[t] << " " << metKinetics1[t] << " " <<  metKinetics2[t] << endl;

        if (t > 0){
            /* Calculate AUC */
            AreaUnderCurve0 = AreaUnderCurve0 + metKinetics0[t-1];
            AreaUnderCurve1 = AreaUnderCurve1 + metKinetics1[t-1];
            AreaUnderCurve2 = AreaUnderCurve2 + metKinetics2[t-1];
            time = 1 + time;
            /* Calculate MT */
            MeanTime0 = (time*metKinetics0[t]) + MeanTime0;
            MeanTime1 = (time*metKinetics1[t]) + MeanTime1;
            MeanTime2 = (time*metKinetics2[t]) + MeanTime2;
        }
          
        MeanTime0 = MeanTime0/AreaUnderCurve0;
        MeanTime1 = MeanTime1/AreaUnderCurve1;
        MeanTime2 = MeanTime2/AreaUnderCurve2;
        
        /* Calculate max(aka peak hieght), min, & arrival time */
        if ( metKinetics0[t] > maxValue0) {
            maxValue0 = metKinetics0[ t ];
            arrival0 = t;
        }
        if ( metKinetics0[t] < minValue0) {
            minValue0 = metKinetics0[ t ];
        }

        if ( metKinetics1[t] > maxValue1) {
            maxValue1 = metKinetics1[ t ];
            arrival1 = t;
        }
        if ( metKinetics1[t] < minValue1) {
            minValue1 = metKinetics1[ t ];
        }

        if ( metKinetics2[t] > maxValue2) {
            maxValue2 = metKinetics2[ t ];
            arrival2 = t;
        }
        if ( metKinetics2[t] < minValue2) {
            minValue2 = metKinetics2[ t ];
        }
              
    }

	// calculate approximate FWHM 

	// PYRUVATE 
	for ( int t = 0; t < numPts; t++ ) {
	    if ( metKinetics0[t] >  0.5*maxValue0){
		    t_start_FWHM0 = t;
		    break;
	    }
	}
	for ( int t = 0; t < numPts; t++ ) {
	    if ( metKinetics0[numPts-t] >  0.5*maxValue0){
		    t_end_FWHM0 = numPts-t;
		    break;
	    }
	}
	// LACTATE
	for ( int t = 0; t < numPts; t++ ) {
	    if ( metKinetics1[t] >  0.5*maxValue1){
		    t_start_FWHM1 = t;
		    break;
	    }
	}
	//UREA
	for ( int t = 0; t < numPts; t++ ) {
	    if ( metKinetics1[numPts-t] >  0.5*maxValue1){
		    t_end_FWHM1 = numPts-t;
		    break;
	    }
	}
    for ( int t = 0; t < numPts; t++ ) {
	    if ( metKinetics2[t] >  0.5*maxValue2){
		    t_start_FWHM2 = t;
		    break;
	    }
	}
    for ( int t = 0; t < numPts; t++ ) {
	    if ( metKinetics2[numPts-t] >  0.5*maxValue2){
		    t_end_FWHM2 = numPts-t;
		    break;
	    }
	}

	float FWHM0 = t_end_FWHM0- t_start_FWHM0;
	float FWHM1 = t_end_FWHM1- t_start_FWHM1;  
	float FWHM2 = t_end_FWHM2- t_start_FWHM2;	
	  

	
    //  Set up dynamic variable arrray for cmin_pack 
    const int numMets = 3;
    const int combinedNumberOfTimePoints = numMets * numPts; /* m = 3*numPts... should this be 3? 15? 20? 60? */
    int i, j, ldfjac, maxfev, mode, nprint, info, nfev, njev;
    int* ipvt =  new int[numMets]; 
    real ftol, xtol, gtol, factor, fnorm, epsfcn;
    double* x =  new double[numMets];
    double* fvec = new double[combinedNumberOfTimePoints];
    double* diag = new double[numMets];
    double* fjac = new double[combinedNumberOfTimePoints*numMets];
    double* qtf = new double[numMets];
    double* wa1 = new double[numMets];
    double* wa2 = new double[numMets];
    double* wa3 = new double[numMets];
    double* wa4 = new double[combinedNumberOfTimePoints];
    int k;
	
    real y[combinedNumberOfTimePoints];
	
    for (int met=0; met < numMets; met++){
        for (int t=0; t < numPts; t++){
            if(met==0) {
                y[met*numPts+t] = metKinetics0[t];
            }
            if(met==1) {
                y[met*numPts+t] = metKinetics1[t];
            }
            if(met==2) {
                y[met*numPts+t] = metKinetics2[t];
            }
        }
    }
		
    fcndata_t data;	
    data.m = combinedNumberOfTimePoints;
    data.y = y;
    // Not sure what this is for...based on examples from cminpack-1.3.0/examples/tlmdifc.c 

    real TR = 1; // sec 
	  
    // Set initial values (sec-1)
    //x[0] = this->metKinetics0[0]; // Pyr at time = 0 
    //x[1] = this->metKinetics1[0]; // Lac at time = 0 
    //x[2] = this->metKinetics2[0]; // Urea at time = 0 
    x[0] = 1/(20*TR);               // 1/T1 All 
    //x[4] = 1/(10*TR);             // T1,Pyr 
    //x[5] = 1/(10*TR);             // T1,Lac 
    x[2] = 1/(10*TR);               // T1,Urea 
    //x[2] = 1/TR;                  // Pyruvate bolus arrival time 
    x[1] = 0.5/TR;                  // Kpyr->lac 
	
    ldfjac = combinedNumberOfTimePoints;

    // Set ftol and xtol to the square root of the machine 
    // and gtol to zero. unless high solutions are 
    // required, these are the recommended settings. 
    //ftol =sqrt(__cminpack_func__(dpmpar)(1));
    xtol =sqrt(__cminpack_func__(dpmpar)(1));
    gtol =0.;

    maxfev = 2000;
    epsfcn = 0.;
    mode = 1;
    factor = 10;
    //nprint =0;
	
	
    // Set lower and upper bounds 
    // double lb[] = {minValue0, minValue1, minValue2, 1/50, 1/50, 1/50, 0};
    // double ub[] = {maxValue0, maxValue1, maxValue2, 1/5, 1/5, 1/5, 1};  
    // SI pyr, SI lac, SI urea, 1/Tp, 1/T1L, 1/T1U, rate lac-pyr...sec-1 

	
    // lmder: User defined Jacobian
    // based on examples from cminpack-1.3.0/examples/tlmderc.c
    //info = __cminpack_func__(lmder)(fcn_lmder, &data, m, n, x, fvec, fjac, ldfjac, ftol, xtol, gtol,
    //maxfev, diag, mode, factor, nprint, &nfev, &njev,
    //ipvt, qtf, wa1, wa2, wa3, wa4);

	
    // lmdif: Calculates Jacobian based on derivatives
    // based on examples from cminpack-1.3.0/examples/tlmdifc.c
	
    info = __cminpack_func__(lmdif)(fcn_lmdif, &data,combinedNumberOfTimePoints , numMets, x, fvec, ftol, xtol, gtol, maxfev, epsfcn, diag, mode, factor, nprint, &nfev, fjac, ldfjac,ipvt, qtf, wa1, wa2, wa3, wa4);
	
    fnorm = __cminpack_func__(enorm)(combinedNumberOfTimePoints, fvec);
	
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
    //for (j=0; j<n; ++j) {
    //  printf("                 Estimated value:%15.7g\n\n",x[j]);
    //  printf("\n");
    //	}

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
    k = __cminpack_func__(covar1)(combinedNumberOfTimePoints, numMets, fnorm*fnorm, fjac, ldfjac, ipvt, ftol, wa1);
    printf("      covariance\n");
    for (i=0; i < numMets; ++i) {
        for (j=0; j < numMets; ++j){
            printf("%s%15.7g", j%3==0?"\n     ":"", (double)fjac[i*ldfjac+j]);
        }
    }
    printf("\n");
    printf("\n");
    printf("      rank(J) = %d\n", k != 0 ? k : numMets); 
    printf("\n");  
    //double Mfit[] = fcn_lmdif(void *p, int m, int n, const real *x, real *fvec, int iflag);

  
    // Get parameters 
    //double Klp = 0;
    //double T1p = 1/x[4];
    //double T1l = 1/x[5];
    //double t_arrival = x[1]; 
    double T1u   = 1/x[2];
    double T1all = 1/x[0];
    double Kpl   = x[1];
	
    cout << " Two Site Exchange assuming back reaction is zero and acq starts after bolus" << endl;
    printf("\n");
    // Dealing with all of these parameters later 
    //cout << "   Klp: " << Klp   << endl;
    //cout << "   T1p: " << T1p << endl;
    //cout << "   Tl1: " << T1l << endl;
    //cout << "   Bolus arrival time: " << t_arrival  << endl;
    //printf("\n");
    cout << "   Tlu: " << T1u << endl;
    printf("\n");
    cout << "   Kpl: " << Kpl << endl;
    printf("\n");
    cout << "   T1 all metabolites: " << T1all  << endl;
    printf("\n");
    
    float* calculatedLacKinetics = new float[numPts];
    //float* calculatedPyrKinetics = new float[numPts];
    //float* calculatedUreaKinetics = new float[numPts];
    this->CalculateLactateKinetics(x, numPts, metKinetics0, metKinetics1, calculatedLacKinetics);	
    delete[] calculatedLacKinetics;
	
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
    
    
    // EDIT HERE!!! Christine
    // Add a reasonable mask 
    if (maxValue1 > 5*minValue1){
        voxelValue=MeanTime1;
    }else{
        voxelValue = 0;
    }
    return voxelValue;

}


/*! 
 *  Function to caluculate the lactate kinetic trace from the best fit params for this voxel
 */
void svkDynamicMRIAlgoTemplate::CalculateLactateKinetics(double* fittedModelParams, int numTimePts,float* metKinetics0, float* metKinetics1, float* calculatedLacKinetics ) 
{
    double T1all  = 1/fittedModelParams[0];
    double Kpl    = fittedModelParams[1];
    int t_arrival = 2;
  
    //  use fitted model params and initial concentration/intensity to calculate the lactacte intensity at 
    //  each time point
    //  solved met(t) = met(0)*invlaplace(phi(t)), where phi(t) = sI - x. x is the matrix of parameters.
  
    cout << " Calculating Lactate Kinetics from LS fit " << endl;
  
    for ( int t = 0; t < numTimePts; t++ ) {
        if (t<t_arrival){
            calculatedLacKinetics[t] = 0;
        }
        if (t >= t_arrival){  	  
            // PYRUVATE 
            // calculatedPyrKinetics[t] = metKinetics0[0]*exp(-((1/T1all)+Kpl-t_arrival)*t);
	    
            // UREA
            // calculatedUreaKinetics[t] = metKinetics2[0]*exp(-((1/T1all)-t_arrival)*t);
	    
            // LACTATE 
            calculatedLacKinetics[t] = metKinetics0[t_arrival]*(-exp(-t/T1all-t*Kpl)+exp(-t/T1all))+metKinetics1[t_arrival]*exp(-t/T1all);
        }
	
        cout << "Measured at time  t=" << t << " : "<< metKinetics1[t] << endl;
        cout << "Estimated at time t=" << t <<" : "<< calculatedLacKinetics[t] << endl;
	
    }
    printf("\n");
}


/*
 * lmder stub
 */
//	int fcn_lmder(void *p, int m, int n, const real *x, real *fvec, real *fjac, int ldfjac, int iflag)
//	{
//  return 0;
//	}


/*!
 * model exchange
 */
int fcn_lmdif(void *p, int combinedNumberOfTimePoints, int numMets, const double *x, double *fvec, int iflag) 
{

    //const real y* = ((fcndata_t*)p)->y;
  
    //int mets = 3;
    int pfa = 0; 
    float TR = 1; /* sec */
    double pi = vtkMath::Pi();//3.14159265358979323846;
    int numTimePoints = combinedNumberOfTimePoints/numMets;
  
    //cout <<  "numTimePoints: " << numTimePoints << endl;
  
    // Now extract the arrays from the fcndata_t struct:
    const real* y = ((fcndata_t*)p)->y;
  
    const real* metKinetics0 = y;
    const real* metKinetics1 = y+=numTimePoints;
    const real* metKinetics2 = y+=numTimePoints;

    //cout << " metKinetics0[0] = "<< metKinetics0[0] << endl;
    //cout << " metKinetics1[0] = "<< metKinetics1[0] << endl;
    //cout << " metKinetics2[0] = "<< metKinetics2[0] << endl;
  
    //cout << " metKinetics0[numTimePoints-1] = "<< metKinetics0[numTimePoints-1] << endl;
    //cout << " metKinetics1[numTimePoints-1] = "<< metKinetics1[numTimePoints-1] << endl;
    //cout << " metKinetics2[numTimePoints-1] = "<< metKinetics2[numTimePoints-1] << endl;

    // For now use a set bolus arrival time to simplify perfusion
    int t_arrival = 2;
  
    // set initial conditions and remove perfusion data for now 
    for (int mm = 0; mm<numMets; mm++){
        for  ( int t = 1; t < t_arrival; t++ ){
            fvec[(mm-1)*numTimePoints+t] = 0;
        }
        fvec[(mm-1)*numTimePoints+t_arrival] = y[(mm-1)*numTimePoints+t_arrival]-y[(mm-1)*numTimePoints+t_arrival];
    }
  
    // Use when inital conditions are not estimated (PYR, LAC, UREA) 
    // double K[] = {x[1]-x[0]-x[2],0,0,x[2],-x[0],0, 0,0,x[1]-x[0]};

    // Use when inital conditions are not estimated (PYR, LAC, LAC) 
    // double K[] = {x[1]-x[0]-x[2],0,0,x[2],-x[0],0, x[2],0,-x[0]};

    // Use when inital conditions are not estimated (PYR, LAC, LAC)  ignoring urea and perfusion for now
    double K[] = {-x[0]-x[1],0,0,x[1],-x[0],0, 0,0,-x[2]};
  
    // Test on pyruvate data only estimates T1p 
    // double K[] = {x[1]-x[0],0,0,   0,x[1]-x[0],0,    0,0,x[1]-x[0]};

    //cout<< " X[0] =  " << x[0] << endl;
    //cout<< " X[1] =  " << x[1] << endl;
    //cout<< " X[2] =  " << x[2] << endl;
  
    // need to define TR, flip earlier
    int j=1;


    // find residuals at x 
    for (int mm = 0; mm<numMets; mm++){
        for  ( int t = t_arrival+1; t < numTimePoints; t++ ){
            //if (t<t_arrival){
            //     fvec[(mm-1)*numTimePoints+t] = 0;
            // }
	  
            //if (t==t_arrival || t>t_arrival){
	  
            if ( pfa != 0){
            // correct for progressive flip angle
                real flip=atan(1/(sqrt(numTimePoints-j)));
                fvec[(mm-1)*numTimePoints+t] =y[(mm-1)*numTimePoints+t]- (exp(K[(mm-1)*numMets+0]*TR)+exp(K[(mm-1)*numMets+1]*TR)+exp(K[(mm-1)*numMets+2]*TR))*fvec[(mm-1)*numTimePoints+t-1]*cos(flip*pi/180);
            }

            if (pfa == 0){
				
                // PYRUVATE 
                if (mm==0){
                    fvec[(mm)*numTimePoints+t] = 
                        metKinetics0[t] - 
                        (exp(K[(mm-1)*numMets+0]*TR)+exp(K[(mm-1)*numMets+1]*TR)+exp(K[(mm-1)*numMets+2]*TR)) * fvec[(mm-1)*numTimePoints+t-1];
                }

                // LACTATE 
                if (mm==1){
                    fvec[(mm)*numTimePoints+t] = 
                        metKinetics1[t] - 
                        (exp(K[(mm-1)*numMets+0]*TR)+exp(K[(mm-1)*numMets+1]*TR)+exp(K[(mm-1)*numMets+2]*TR)) * fvec[(mm-1)*numTimePoints+t-1];
                }

                // UREA 
                if (mm==2){
                    fvec[(mm)*numTimePoints+t] = 
                        metKinetics2[t] - 
                        (exp(K[(mm-1)*numMets+0]*TR)+exp(K[(mm-1)*numMets+1]*TR)+exp(K[(mm-1)*numMets+2]*TR)) * fvec[(mm-1)*numTimePoints+t-1];
                }
                //	cout<< " fvec =  " << fvec[(mm-1)*numTimePoints+t] << " at " << t << " and metabolite "<< mm << endl;
            }
            //}
            j=j+1;
        }
    }

    cout << "    Kpl = "<< x[1] << endl;
    //cout << "1/T1all = "<< x[0] << endl;
    return 0;

}

/*!
 * Get residuals: Not sure if this package needs this 
 */
//double g(double* x,float* metKinetics0, float* metKinetics1, float* metKinetics2){
//  int mets = 3;
//  int Nt = sizeof(metKinetics0);
//  double res[3*numTimePoints];/*res[sizeof(X)];*/
//  double model[3*numTimePoints];
//  model = fcn_lmdif(void *p, int m, int n, const real *x, real *fvec, real *fjac,
//				int ldfjac, int iflag);
  
//  for (int m=1; m>mets; m++){
//	for (int t=1; t<numTimePoints; t++){
//	  if (m==1){
//		res[(m-1)*numTimePoints+t] = model[(m-1)*numTimePoints+t] - metKinetics0[t]; /*double check that this is right*/
//	  }
//	  if (m==2){
//		res[(m-1)*numTimePoints+t] = model[(m-1)*numTimePoints+t] - metKinetics1[t]; /*double check that this is right*/
//	  }
//	  if (m==3){
//		res[(m-1)*numTimePoints+t] = model[(m-1)*numTimePoints+t] - metKinetics2[t]; /*double check that this is right*/
//	  }
//	}
//  }
//  // return res;
//}

	

/*! 
 *  Zero data
 */
void svkDynamicMRIAlgoTemplate::ZeroData()
{

    int numVoxels[3];
    this->GetOutput()->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];
    double zeroValue = 0.;
    for (int i = 0; i < totalVoxels; i++ ) {
        this->GetOutput()->GetPointData()->GetScalars()->SetTuple1(i, zeroValue);
    }

}


/*!
 *
 */
void svkDynamicMRIAlgoTemplate::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
}


/*!
 *  input ports 0 - 2 are required. All input ports are for dynamic MRI data. 
 */
int svkDynamicMRIAlgoTemplate::FillInputPortInformation( int port, vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
    return 1;
}



/*!
 *  Output from this algo is an svkMriImageData object. 
 */
int svkDynamicMRIAlgoTemplate::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData"); 
    return 1;
}

