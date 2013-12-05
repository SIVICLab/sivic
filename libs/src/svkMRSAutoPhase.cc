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

//int POWELL_CALLS_TO_GET_VALUE = 0;

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkImageFourierFilter.h> // for vtkImageComplex struct
#include <vtkMath.h>

#include <svkMRSAutoPhase.h>
#include <svkMrsImageFFT.h>
#include <svkSpecUtils.h>
#include <svkPhaseSpec.h>

#include <math.h>
#include <stdio.h>
#include <string.h>


using namespace svk;


vtkCxxRevisionMacro(svkMRSAutoPhase, "$Rev$");
int* svkMRSAutoPhase::progress; //  static pointer 



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
    //this->SetPhasingModel(svkMRSAutoPhase::MAX_GLOBAL_PEAK_HT_0); 
    this->onlyUseSelectionBox = false; 
    this->linearPhaseArrays = NULL; 

}



/*! 
 *  Sets the linear phase to apply to spectra.
 */
void svkMRSAutoPhase::OnlyUseSelectionBox()
{
    this->onlyUseSelectionBox = true;
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
//void svkMRSAutoPhase::SetPhasingModel(svkMRSAutoPhase::phasingModel modelType) 
//{
    //this->phaseModelType = modelType;  
//}


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

    //   for each cell /spectrum: 
    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );
    cout << "NUMCELLS: " << numCells << endl;

    float tolerance = .5;     
    this->selectionBoxMask = new short[numCells];
    data->GetSelectionBoxMask(selectionBoxMask, tolerance);

    //  Setup:  if firstorder, then transform to time domain
    //  Initialize first order phase arrays (e.g. -1 to 1 in nuTimePoint intervals): 
    this->PrePhaseSetup();

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

    this->PostPhaseCleanup();

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->GetInput()->Update();

    delete [] this->selectionBoxMask; 


    return 1;
};


/*!
 *
 */
/*
void svkMRSAutoPhase::PrePhaseSetup()
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0));

    // don't need peaks for first point phasing
    if ( this->phaseModelType != FIRST_POINT_0 ) {
        //  Locate peaks in spectra:     
        this->peakPicker = svkMRSPeakPick::New();
        this->peakPicker->SetInput( data );
        if ( this->onlyUseSelectionBox ) { 
            this->peakPicker->OnlyUseSelectionBox();
        }
        this->peakPicker->Update();
        cout << "NUM PEAKS: " << this->peakPicker->GetNumPeaks() << endl;
    }

    if (this->phaseModelType >= LAST_ZERO_ORDER_MODEL) {
        this->InitLinearPhaseArrays(); 
    }
}
*/


/*!
 *
 */
/*
void svkMRSAutoPhase::PostPhaseCleanup()
{

    if (this->phaseModelType != FIRST_POINT_0) { 
        this->peakPicker->Delete(); 
    }

    if (this->phaseModelType >= LAST_ZERO_ORDER_MODEL) {
        this->InitLinearPhaseArrays(); 
    }
}
*/

/*
 * 
 */
int svkMRSAutoPhase::GetPivot()
{
    //int pivot = this->numTimePoints/2; 
    int start; 
    int peak; 
    int end; 
    int zeroOrderPhasePeak = this->GetZeroOrderPhasePeak( ); 
    this->peakPicker->GetPeakDefinition( zeroOrderPhasePeak, &start, &peak, &end ); 
    cout << "ZOPP: " << zeroOrderPhasePeak << " " << peak << endl;
    int pivot = peak; 
    return pivot; 
}

/*! 
 *  Initialize the array of linear phase correction factors for performance 
 */
void svkMRSAutoPhase::InitLinearPhaseArrays() 
{
    this->numFirstOrderPhaseValues = this->numTimePoints*4;  

    this->linearPhaseArrays = new vtkImageComplex*[ this->numFirstOrderPhaseValues ]; 

    int pivot = this->GetPivot(); 

    for (int i = 0; i < this->numFirstOrderPhaseValues; i++) {

        float phi1 = i - this->numFirstOrderPhaseValues/2;  
        phi1 = phi1/360.; 

        // Each phase value defines an array of linear phase correction factors for the spectra: 
        vtkImageComplex* linearPhaseArrayTmp = new vtkImageComplex[ this->numTimePoints ];
        svkSpecUtils::CreateLinearPhaseShiftArray( 
                this->numTimePoints, 
                linearPhaseArrayTmp, 
                phi1, 
                pivot);

        this->linearPhaseArrays[i] = linearPhaseArrayTmp; 
        //cout << "LPA Init: " << i << " = " << this->linearPhaseArrays[i][0].Real<< endl;
    }
}


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
   
        //  each thread operates on a sub-extent.  Check to see if the cell is in the 
        //  current cell's sub-extent.  If so, fit it.
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

    //  If only fitting within selection box, then skip over
    //  voxels outside. 
    if ( this->onlyUseSelectionBox == true ) { 
        if ( this->selectionBoxMask[cellID] == 0 ) {
               return; 
           }
    }

    //  if first order fitting, fit in two passes
    //if ( this->phaseModelType > svkMRSAutoPhase::LAST_ZERO_ORDER_MODEL)  {
        //this->FitPhase( cellID, svkMRSAutoPhase::MAX_PEAK_HT_0_ONE_PEAK); 
        ////this->FitPhase( cellID, svkMRSAutoPhase::MIN_DIFF_FROM_MAG_0_ONE_PEAK); 
        //this->FitPhase( cellID, svkMRSAutoPhase::MAX_PEAK_HTS_1); 
        ////this->FitPhase( cellID, svkMRSAutoPhase::MIN_DIFF_FROM_MAG_1); 
    //} else {
    this->FitPhase( cellID ); 
    //}

}

/*
*/
int svkMRSAutoPhase::GetZeroOrderPhasePeak( )
{
    int peakNum; 
    // Pick the peak for zero order phasing: 
    float height = 0; 
    for ( int i = 1; i < this->peakPicker->GetNumPeaks() - 1 ; i++ ) {
        float tmpHt = this->peakPicker->GetAvRMSPeakHeight(i); 
        if ( tmpHt > height ) { 
            peakNum = i; 
        }
    }
    return peakNum; 
}


/*!
 * 
 */
/*
#ifdef SWARM
void svkMRSAutoPhase::InitOptimizer( int cellID, itk::ParticleSwarmOptimizer::Pointer itkOptimizer )
#else
void svkMRSAutoPhase::InitOptimizer( int cellID, itk::PowellOptimizer::Pointer itkOptimizer )
#endif
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0));
    vtkFloatArray* spectrum = static_cast<vtkFloatArray*>( data->GetSpectrum( cellID ) );
    float cmplxPt[2];
    spectrum->GetTupleValue(0, cmplxPt);
    cout << " cell: " << cellID << " -> " << cmplxPt[0] << endl;

    //========================================================
    //  ITK Optimization 
    //========================================================
    PowellBoundedCostFunction::Pointer  costFunction = PowellBoundedCostFunction::New();
    itkOptimizer->SetCostFunction( costFunction.GetPointer() );

    costFunction->SetModel( model ); 
    costFunction->SetSpectrum( spectrum ); 
    costFunction->SetNumFreqPoints(this->numTimePoints ); 
    costFunction->SetPeakPicker( this->peakPicker ); 
    costFunction->SetLinearPhaseArrays( this->linearPhaseArrays ); 
    costFunction->SetNumFirstOrderPhaseValues( this->numFirstOrderPhaseValues ); 
    if ( model != svkMRSAutoPhase::FIRST_POINT_0 ) {
        costFunction->SetZeroOrderPhasePeak ( this->GetZeroOrderPhasePeak() ); 
    }

    //  set dimensionality of parameter space: 
    const unsigned int spaceDimension = costFunction->GetNumberOfParameters();
    typedef PowellBoundedCostFunction::ParametersType ParametersType;
    ParametersType  initialPosition( spaceDimension );
    initialPosition[0] =  0.;
    if ( model == svkMRSAutoPhase::MAX_PEAK_HTS_01) {
        initialPosition[1] =  0.;
    }

#ifdef SWARM 

        //  Set bounds
        itk::ParticleSwarmOptimizer::ParameterBoundsType bounds;
        // Zero order phase range radians
        if ( model == svkMRSAutoPhase::MAX_PEAK_HTS_1 ) {
            // first order range of values: 
            int upperBound =  this->numFirstOrderPhaseValues/2 - 1; 
            int lowerBound =  -1 * upperBound;  
            bounds.push_back( std::make_pair( lowerBound, upperBound) );
        } else {
            bounds.push_back( std::make_pair( -1 * vtkMath::Pi(), vtkMath::Pi()) ); 
            if ( spaceDimension == 2 ) {
                // first order range of values: 
                int upperBound =  this->numFirstOrderPhaseValues/2 - 1; 
                int lowerBound =  -1 * upperBound;  
                bounds.push_back( std::make_pair( lowerBound, upperBound) );
            }
        }
        itkOptimizer->SetParameterBounds( bounds );

        unsigned int numberOfParticles = 10;
        unsigned int maxIterations = 500;
        double xTolerance = 0.01;
        double fTolerance = 0.01;
        itk::ParticleSwarmOptimizer::ParametersType 
            initialParameters( spaceDimension ), finalParameters;
        itkOptimizer->SetNumberOfParticles( numberOfParticles );
        itkOptimizer->SetMaximalNumberOfIterations( maxIterations );
        itkOptimizer->SetParametersConvergenceTolerance( xTolerance,
                                                   costFunction->GetNumberOfParameters() );
        itkOptimizer->SetFunctionConvergenceTolerance( fTolerance );
        itkOptimizer->SetInitializeNormalDistribution( false ); // use uniform distribution


#else

        itkOptimizer->SetStepLength( 1 );
        itkOptimizer->SetStepTolerance( 1);
        itkOptimizer->SetValueTolerance( 1 );
        itkOptimizer->SetMaximumIteration( 10000 );
#endif 

    itkOptimizer->SetInitialPosition( initialPosition );

}
*/

/*!
 * 
 */
/*
void svkMRSAutoPhase::FitPhase( int cellID, svkMRSAutoPhase::phasingModel model)  
{
#ifdef SWARM 
        typedef itk::ParticleSwarmOptimizer        OptimizerType;
#else
        typedef itk::PowellOptimizer        OptimizerType;
#endif

    OptimizerType::Pointer              itkOptimizer = OptimizerType::New();
    this->InitOptimizer( cellID, model, itkOptimizer ); 

#ifndef SWARM 
        //  maximize total peak height
        if ( model == svkMRSAutoPhase::FIRST_POINT_0 ) {
            itkOptimizer->SetMaximize( true );
        } else if ( model == svkMRSAutoPhase::MAX_GLOBAL_PEAK_HT_0 ) {
            itkOptimizer->SetMaximize( true );
        } else if ( model == svkMRSAutoPhase::MAX_PEAK_HTS_0 ) {
            itkOptimizer->SetMaximize( true );
        } else if ( model == svkMRSAutoPhase::MIN_DIFF_FROM_MAG_0 ) {
            itkOptimizer->SetMaximize( false );
        } else if ( model == svkMRSAutoPhase::MAX_PEAK_HTS_1 ) {
            itkOptimizer->SetMaximize( true );
        } else if ( model == svkMRSAutoPhase::MIN_DIFF_FROM_MAG_0) {
            itkOptimizer->SetMaximize( false );
        } else if ( model == svkMRSAutoPhase::MAX_PEAK_HT_0_ONE_PEAK) {
            itkOptimizer->SetMaximize( true );
        } else if ( model == svkMRSAutoPhase::MIN_DIFF_FROM_MAG_0_ONE_PEAK) {
            itkOptimizer->SetMaximize( false );
        } else if ( model == svkMRSAutoPhase::MAX_PEAK_HTS_01) {
            itkOptimizer->SetMaximize( true );
        }
#endif

    try { 
        cout << "CELL ID: " << cellID << endl;
        itkOptimizer->StartOptimization();
    } catch( itk::ExceptionObject & e ) {
        cout << "Exception thrown ! " << endl;
        cout << "An error ocurred during Optimization" << endl;
        cout << "Location    = " << e.GetLocation()    << endl;
        cout << "Description = " << e.GetDescription() << endl;
        //return EXIT_FAILURE;
        return; 
    }

    typedef PowellBoundedCostFunction::ParametersType ParametersType;
    ParametersType finalPosition = itkOptimizer->GetCurrentPosition();

    //
    // check results to see if it is within range
    //
    bool pass = true;

    // Exercise various member functions.
    cout << endl;

    itkOptimizer->Print( cout );

    if( !pass ) {
        cout << "Test failed." << endl;
        return;
    }

    cout << "Test passed." << endl;

    //  ===================================================
    //  Apply fitted phase values to spectrum: 
    //  ===================================================
      
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0));
    vtkFloatArray* spectrum = static_cast<vtkFloatArray*>( data->GetSpectrum( cellID ) );

    if ( model == svkMRSAutoPhase::MAX_PEAK_HTS_1  || model == svkMRSAutoPhase::MIN_DIFF_FROM_MAG_1 ) {

        float phi1Final = finalPosition[0] + this->numFirstOrderPhaseValues/2; 
        cout << "           PHI1 FINAL(" <<cellID << "): " << phi1Final << endl;
        int pivot = this->GetPivot(); 
        svkPhaseSpec::FirstOrderPhase( phi1Final, pivot, spectrum); 

    } else if ( model == svkMRSAutoPhase::MAX_PEAK_HTS_01 ) { 

        double phi0Final = finalPosition[0]; 
        double phi1Final = finalPosition[1] + this->numFirstOrderPhaseValues/2; 
        cout << " FINAL: " << phi0Final * 180/vtkMath::Pi() << ", " << phi1Final <<  endl; 
        svkPhaseSpec::FirstOrderPhase( phi0Final, this->linearPhaseArrays[static_cast<int>(phi1Final)], spectrum); 

    } else {

        float phi0Final = finalPosition[0]; 
        cout << "           PHI0 FINAL("<<cellID <<"): " << phi0Final * 180. / vtkMath::Pi() << endl;
        svkPhaseSpec::ZeroOrderPhase( phi0Final, spectrum); 

    }

    return;
}
*/


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

