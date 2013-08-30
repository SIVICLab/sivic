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

#include <itkPowellOptimizer.h>
#include <vnl/vnl_math.h>
int POWELL_CALLS_TO_GET_VALUE = 0;

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkImageFourierFilter.h> // for vtkImageComplex struct

#include <svkMRSAutoPhase.h>
#include <svkSpecUtils.h>
#include <svkPhaseSpec.h>

#include <math.h>
#include <stdio.h>
#include <string.h>


using namespace svk;


vtkCxxRevisionMacro(svkMRSAutoPhase, "$Rev$");
vtkStandardNewMacro(svkMRSAutoPhase);
int* svkMRSAutoPhase::progress; //  static pointer 


/*
 *  Cost function for ITK optimizer: 
 */
class PowellBoundedCostFunction : public itk::SingleValuedCostFunction 
{
    public:

        typedef PowellBoundedCostFunction       Self;
        typedef itk::SingleValuedCostFunction   Superclass;
        typedef itk::SmartPointer<Self>         Pointer;
        typedef itk::SmartPointer<const Self>   ConstPointer;
        itkNewMacro( Self );
        itkTypeMacro( PowellBoundedCostFunction, SingleValuedCostFunction );

        //  number of parameters in model (dimensionality of parameter space)
        //  for zero order phase this is 1: 
        enum { NumParameters = 1 };
  
        typedef Superclass::ParametersType      ParametersType;
        typedef Superclass::DerivativeType      DerivativeType;
        typedef Superclass::MeasureType         MeasureType ;
        
        PowellBoundedCostFunction() 
        {
        }


        void GetDerivative( const ParametersType & ,
                            DerivativeType &  ) const
        {
        }


        /*  
         *  returns the cost function for the current param values: 
         *  typedef double MeasureType
         */
        MeasureType  GetValue( const ParametersType & parameters ) const
        { 
            ++POWELL_CALLS_TO_GET_VALUE;
        
            double phi0 = parameters[0];
            //double phi1 = parameters[1];
        
            cout << "      GetValue( " ;
            cout << phi0 * 180 / 3.1415 << " ";
            cout << ") = ";
        
            double peakHeight = FLT_MIN; 
            float cmplxPt[2];
            int minPt = 0; 
            int maxPt = this->numFreqPoints; 
            //int minPt = 191; 
            double tmp; 
            for  ( int t = minPt; t < maxPt; t++ ){

                // apply zero order phase to data:  
                this->spectrum->GetTupleValue(t, cmplxPt);
                svk::svkPhaseSpec::ZeroOrderPhase(phi0, cmplxPt);

                //  maximize positive peak height (minimize negative peak ht) 
                tmp = cmplxPt[0];
                if ( tmp >= peakHeight ) {
                    peakHeight = tmp; 
                }

            }
            MeasureType measure = peakHeight; 
            cout << "                          " << measure << endl; 
            return measure;
        }


        /*
         *
         */  
        unsigned int GetNumberOfParameters(void) const
        {
            return NumParameters;
        }

        /*
         *
         */  
        void SetSpectrum( vtkFloatArray* spectrum )  
        {
            this->spectrum = spectrum;
        }


        /*
         *
         */  
        void SetNumFreqPoints( int numFreqPoints )  
        {
            this->numFreqPoints = numFreqPoints;
        }


    private:

            vtkFloatArray*  spectrum;
            int             numFreqPoints; 

};


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

    //========================================================
    //  ITK Optimization 
    //==================
    typedef itk::PowellOptimizer        OptimizerType;
    typedef OptimizerType::ScalesType   ScalesType;
    // Declaration of a itkOptimizer
    OptimizerType::Pointer  itkOptimizer = OptimizerType::New();
    // Declaration of the CostFunction 
    PowellBoundedCostFunction::Pointer costFunction = PowellBoundedCostFunction::New();

    itkOptimizer->SetCostFunction( costFunction.GetPointer() );

    typedef PowellBoundedCostFunction::ParametersType ParametersType;

    const unsigned int spaceDimension = 
                      costFunction->GetNumberOfParameters();

    costFunction->SetSpectrum( spectrum ); 
    costFunction->SetNumFreqPoints(this->numTimePoints ); 

    // We start not so far from  | 2 -2 |
    ParametersType  initialPosition( spaceDimension );
    initialPosition[0] =  0;

    //  maximize total peak height
    itkOptimizer->SetMaximize( true );
    itkOptimizer->SetStepLength( 1 );
    itkOptimizer->SetStepTolerance( 0.1 );
    itkOptimizer->SetValueTolerance( 0.1 );
    itkOptimizer->SetMaximumIteration( 100 );

    itkOptimizer->SetInitialPosition( initialPosition );

    try { 
        itkOptimizer->StartOptimization();
    } catch( itk::ExceptionObject & e ) {
        cout << "Exception thrown ! " << endl;
        cout << "An error ocurred during Optimization" << endl;
        cout << "Location    = " << e.GetLocation()    << endl;
        cout << "Description = " << e.GetDescription() << endl;
        //return EXIT_FAILURE;
        return; 
    }

    ParametersType finalPosition = itkOptimizer->GetCurrentPosition();
    cout << "Solution        = (";
    cout << finalPosition[0] << "," ;
    cout << ")" << endl;  

    //
    // check results to see if it is within range
    //
    bool pass = true;
    //double trueParameters[2] = { 2, -2 };
    //for( unsigned int j = 0; j < 2; j++ ) {
        //if( vnl_math_abs( finalPosition[j] - trueParameters[j] ) > 0.01 ) {
            //pass = false;
        //}
    //}

    // Exercise various member functions.
    cout << "Maximize: " << itkOptimizer->GetMaximize() << endl;
    cout << "StepLength: " << itkOptimizer->GetStepLength();
    cout << endl;
    cout << "CurrentIteration: " << itkOptimizer->GetCurrentIteration();
    cout << endl;

    itkOptimizer->Print( cout );

    cout << "Calls to GetValue = " << POWELL_CALLS_TO_GET_VALUE << endl;

    if( !pass ) {
        cout << "Test failed." << endl;
        return;
        //return EXIT_FAILURE;
    }

    cout << "Test passed." << endl;

    //
    //  Apply fitted phase values to spectrum: 
    //
    float phi0Final = finalPosition[0]; 
    cout << "           PHI0 FINAL("<<cellID <<"): " << phi0Final * 180 / 3.1415<< endl;
    float cmplxPt[2];
    for (int i = 0; i < this->numTimePoints; i++) {
            
        spectrum->GetTupleValue(i, cmplxPt);
        svk::svkPhaseSpec::ZeroOrderPhase(phi0Final, cmplxPt); 
             
        spectrum->SetTuple(i, cmplxPt); 
    }

    return;

}


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

