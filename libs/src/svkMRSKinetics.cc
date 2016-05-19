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
 *      Beck Olson,
 *      Christine Leon 
 */


#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <svkMRSKinetics.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <svkKineticModelCostFunction.h>
#include <svk2SiteExchangeCostFunction.h>
#include <svk2SitePerfCostFunction.h>


using namespace svk;


//vtkCxxRevisionMacro(svkMRSKinetics, "$Rev$");
vtkStandardNewMacro(svkMRSKinetics);

//#define MODEL 1     //2SiteExchange
#define MODEL 2     //2SitePerf


/*!
 *
 */
svkMRSKinetics::svkMRSKinetics()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    this->newSeriesDescription = "model"; 
    //  3 required input ports: 
    this->SetNumberOfInputPorts(4); //3 input metabolites + roi mask

    this->modelType = svkMRSKinetics::UNDEFINED;     
    //  Outputports:  0 for fitted pyruvate kinetics
    //  Outputports:  1 for fitted lactate kinetics
    //  Outputports:  2 for fitted urea kinetics
    //  Outputports:  3 for T1all map 
    //  Outputports:  4 for Kpl map 
    //  Outputports:  5 for Ktrans map 
    this->SetNumberOfOutputPorts(6); 

    this->modelOutputDescriptionVector.resize( this->GetNumberOfOutputPorts() ); 
    this->modelOutputDescriptionVector[0] = "pyr"; 
    this->modelOutputDescriptionVector[1] = "lac"; 
    this->modelOutputDescriptionVector[2] = "urea"; 
    this->modelOutputDescriptionVector[3] = "T1all"; 
    this->modelOutputDescriptionVector[4] = "Kpl"; 
    this->modelOutputDescriptionVector[5] = "Ktrans"; 
}



/*!
 *
 */
svkMRSKinetics::~svkMRSKinetics()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}


/*!
 *  Set the series description for the DICOM header of the copy.  
 */
void svkMRSKinetics::SetSeriesDescription( vtkstd::string newSeriesDescription )
{
    this->newSeriesDescription = newSeriesDescription;
    this->Modified(); 
}


/*!
 *  Set model 
 */
void svkMRSKinetics::SetModelType( svkMRSKinetics::MODEL_TYPE modelType)
{
    if ( modelType < 1 || modelType > 2 ) {
        cout << "ERROR: invalid model type: " << modelType << endl;
        exit(1); 
    }
    this->modelType = modelType;
}


/*!
 *  Resets the origin and extent for correct initialization of output svkMriImageData object from input 
 *  svkMrsImageData object. 
 */
int svkMRSKinetics::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
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
int svkMRSKinetics::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    //  Create the template data object by  
    //  extractng an svkMriImageData from the input svImageData object
    //  Use an arbitrary point for initialization of scalars.  Actual data 
    //  will be overwritten by algorithm. 
    //  GetImage initializes "GetOutput(N) with a template image. 

    //  Initialze the dim vector to all 0.  The non-spatial indices determine which volume is used to 
    //  initializet the output image. 
    svkDcmHeader::DimensionVector dimVector = this->GetImageDataInput(0)->GetDcmHeader()->GetDimensionIndexVector();
    for ( int i = 0; i < dimVector.size(); i++ ) {
        svkDcmHeader::SetDimensionVectorValue(&dimVector, i, 0);
    }

    //  this should really come from the model
    vector <string> outputSeriesDescriptionVector;
    outputSeriesDescriptionVector.resize( this->GetNumberOfOutputPorts() ); 
    for ( int i = 0 ; i < this->GetNumberOfOutputPorts(); i++ ) { 
        outputSeriesDescriptionVector[i] = this->newSeriesDescription + "_" + this->modelOutputDescriptionVector[i]; 
    }

    for ( int i = 0 ; i < this->GetNumberOfOutputPorts(); i++ ) { 
        string seriesDescription = outputSeriesDescriptionVector[i]; 
        svkMriImageData::SafeDownCast( this->GetImageDataInput(0) )->GetCellDataRepresentation()->GetImage(
               svkMriImageData::SafeDownCast( this->GetOutput(i) ), 
               0, 
               seriesDescription, 
               &dimVector, 
               0 
           ); 
    }

    //  ===============================================
    //  Create and initialize output data objects for fitted kinetics results: 
    //  ===============================================

    svkMriImageData* data = svkMriImageData::SafeDownCast(this->GetImageDataInput(0) );

    //  create a template array for output kinetics: 
    this->numTimePoints = data->GetDcmHeader()->GetNumberOfTimePoints();
    vtkFloatArray* templateArray = vtkFloatArray::SafeDownCast(
            svkMriImageData::SafeDownCast(this->GetImageDataInput(0))->GetCellDataRepresentation()->GetArray(0)
    );
    for ( int time = 0; time < this->numTimePoints; time++ ) {
        templateArray->SetTuple1(time, 0); 
    }

    int numVoxels[3];
    this->GetOutput(1)->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];

    int numSignalsInModel = this->GetNumSignals();
    for ( int sigNum = 0; sigNum < numSignalsInModel; sigNum++ ) {
        svkMriImageData* fittedKineticSignal  = svkMriImageData::SafeDownCast( this->GetOutput(sigNum) );
        fittedKineticSignal->DeepCopy(data);

        for (int i = 0; i < totalVoxels; i++ ) {

            vtkFloatArray* outputArray = vtkFloatArray::SafeDownCast(
                fittedKineticSignal->GetCellDataRepresentation()->GetArray(i)
            );
            outputArray->DeepCopy(templateArray); 

        }
    }

    //  ===============================================
    //  ===============================================
    this->GenerateKineticParamMap();
    //  ===============================================

    for ( int i = 0 ; i < this->GetNumberOfOutputPorts(); i++ ) { 
        svkDcmHeader* hdr = this->GetOutput(i)->GetDcmHeader();
        hdr->InsertUniqueUID("SeriesInstanceUID");
        hdr->InsertUniqueUID("SOPInstanceUID");
        hdr->SetValue("SeriesDescription", outputSeriesDescriptionVector[i] );
    }

    return 1; 
};


/*!
 *  Iterate over all spatial locations and fit each voxel to a kinetic model. 
 *  Generate 3D image parameter maps and model dynamics 
 */
void svkMRSKinetics::GenerateKineticParamMap()
{

    svkMriImageData* data = svkMriImageData::SafeDownCast(this->GetImageDataInput(0) );

    this->ZeroData();

    //  Set the parameter map data array names to initialize.  
    this->GetOutput(3)->GetPointData()->GetArray(0)->SetName( this->modelOutputDescriptionVector[3].c_str() );
    this->GetOutput(4)->GetPointData()->GetArray(0)->SetName( this->modelOutputDescriptionVector[4].c_str() );
    this->GetOutput(5)->GetPointData()->GetArray(0)->SetName( this->modelOutputDescriptionVector[5].c_str() );

    double voxelValue;
    int numVoxels[3];
    this->GetOutput(1)->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];

    //  set up mask.  If no input mask was provided, then set the mask to   
    svkMriImageData* maskImage;
    unsigned short* mask;
    bool  hasMask = false;       
    if (this->GetInput( svkMRSKinetics::MASK )) {
        maskImage = svkMriImageData::SafeDownCast(this->GetImageDataInput(svkMRSKinetics::MASK) );
        mask      = static_cast<vtkUnsignedShortArray*>( 
                        maskImage->GetPointData()->GetArray(0) )->GetPointer(0) ; 
        hasMask = true; 
    } 


    for (int voxelID = 0; voxelID < totalVoxels; voxelID++ ) {

        //cout << "VOXEL NUMBER: " << voxelID << endl;
        //cout << "MASK VALUE: " << mask[voxelID] << endl;

        //  If there is a mask check it.  If no mask provided, the compute all voxels. 
        if ( ((hasMask == true) && (mask[voxelID] != 0 )) || ( hasMask == false )  ) {
            cout << "Fit Voxel " << voxelID << endl;
            this->FitVoxelKinetics( voxelID );
        }
    }

    //  This syncs the cell data back to the point data, should be in svkMriImageData class
    this->SyncPointsFromCells(); 

}


/*!
 *  Initial the cost function with the input signals for the given number of sites.  
 */
void svkMRSKinetics::InitCostFunction(  svkKineticModelCostFunction::Pointer& costFunction,  int voxelID )
{
    this->GetCostFunction( costFunction ); 

    int numSignalsInModel = this->GetNumSignals();
    for (int sigNum = 0; sigNum < numSignalsInModel; sigNum++ ) {

        vtkFloatArray* kineticSignal = vtkFloatArray::SafeDownCast(
            svkMriImageData::SafeDownCast(
                    this->GetImageDataInput(sigNum))->GetCellDataRepresentation()->GetArray(voxelID)
        );

        float* kineticSignalPointer= kineticSignal->GetPointer(0);
        costFunction->SetSignal( kineticSignalPointer, sigNum, this->modelOutputDescriptionVector[sigNum]);
    }
    costFunction->SetNumTimePoints( this->numTimePoints );
}


/*!
 *  Factory to get the correct the cost function. 
 */
void svkMRSKinetics::GetCostFunction( svkKineticModelCostFunction::Pointer& costFunction)
{
    if ( this->modelType == svkMRSKinetics::TWO_SITE_EXCHANGE ) { 
        costFunction = svk2SiteExchangeCostFunction::New();
    } else if ( this->modelType == svkMRSKinetics::TWO_SITE_EXCHANGE_PERF ) {
        costFunction = svk2SitePerfCostFunction::New();
    }
}


/*!
 *  Get the number of signals required for the specified cost function
 */
int svkMRSKinetics::GetNumSignals()
{
    svkKineticModelCostFunction::Pointer costFunction;
    this->GetCostFunction( costFunction ); 
    return costFunction->GetNumSignals(); 
}


/*!
 * 
 */
void svkMRSKinetics::InitOptimizer( itk::ParticleSwarmOptimizer::Pointer itkOptimizer, int voxelID )
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0) );

    //========================================================
    //  ITK Optimization 
    //========================================================
    svkKineticModelCostFunction::Pointer costFunction;
    this->InitCostFunction( costFunction, voxelID); 
    itkOptimizer->SetCostFunction( costFunction.GetPointer() );

    //  set dimensionality of parameter space: 
    const unsigned int paramSpaceDimensionality = costFunction->GetNumberOfParameters();
    typedef svkKineticModelCostFunction::ParametersType ParametersType;
    ParametersType  initialPosition( paramSpaceDimensionality );


    //  THIS SHOULD PROBABLY BE MOVED TO THE MODEL
    float TR = 3.;

    //  Set bounds
    itk::ParticleSwarmOptimizer::ParameterBoundsType bounds;

    // first order range of values: 
    //  Set bounds in dimensionless units (each time point is a TR)
    float upperBound[4]; 
    float lowerBound[4]; 

    upperBound[0] = 28/TR;          //  T1all
    lowerBound[0] = 8/TR;           //  T1all
    if ( this->modelType == svkMRSKinetics::TWO_SITE_EXCHANGE_PERF ) {
        upperBound[0] = TR*1./20;     //  T1all
        lowerBound[0] = TR*1./40;     //  T1all
    }

    upperBound[1] = .05 * TR;       //  Kpl
    lowerBound[1] = 0.000 * TR;     //  Kpl
    if ( this->modelType == svkMRSKinetics::TWO_SITE_EXCHANGE_PERF ) {
        upperBound[1] = .5 * TR;       //  Kpl
        lowerBound[1] = 0 * TR;     //  Kpl
    }

    upperBound[2] = 0 * TR;       //  ktrans 
    lowerBound[2] = 0 * TR;       //  ktrans 
    if ( this->modelType == svkMRSKinetics::TWO_SITE_EXCHANGE_PERF ) {
        upperBound[2] = 100;       //  ktrans 
        lowerBound[2] = 0;       //  ktrans 
    }

    upperBound[3] = 1;              //  k2 
    lowerBound[3] = 0;              //  k2

    bounds.push_back( std::make_pair( lowerBound[0], upperBound[0] ) );    // bounds param 0
    bounds.push_back( std::make_pair( lowerBound[1], upperBound[1] ) );    // bounds param 1
    bounds.push_back( std::make_pair( lowerBound[2], upperBound[2] ) );    // bounds param 2
    bounds.push_back( std::make_pair( lowerBound[3], upperBound[3] ) );    // bounds param 3
    itkOptimizer->SetParameterBounds( bounds );

    //  Set initial guesses
    //  Scale by temporal resolution.  At end apply inverse scaling when writing out maps
    //initialPosition[0] =  (upperBound0 - lowerBound0)/2.;    // T1all  (s)
    //initialPosition[1] =  (upperBound1 - lowerBound1)/2.;    // Kpl    (1/s)  
    //initialPosition[2] =  (upperBound2 - lowerBound2)/2.;    // ktrans (1/s)
    //initialPosition[3] =  (upperBound3 - lowerBound3)/2.;    // k2     (1/s)
    //initialPosition[0] =  TR*1./35;     // T1all  (s)
    //initialPosition[1] =  0.01 * TR;    // Kpl    (1/s)  
    //initialPosition[2] =  1 * TR;       // ktrans (1/s)
    //initialPosition[3] =  TR*1./40;     // k2     (1/s)

    initialPosition[0] =  12   / TR;    // T1all  (s)
    initialPosition[1] =  0.01 * TR;    // Kpl    (1/s)  
    initialPosition[2] =  0.00 * TR;    // ktrans (1/s)
    if ( this->modelType == svkMRSKinetics::TWO_SITE_EXCHANGE_PERF ) {
        initialPosition[0] =  (1./35) * TR;     // T1all  (s)
        initialPosition[1] =  0.01    * TR;     // Kpl    (1/s)  
        initialPosition[2] =  1       * TR;     // ktrans (1/s)
        initialPosition[3] =  (1./40) * TR;     // k2     (1/s)
    }

    //for (int k=0; k<4; k++) {
        //cout << "INITIAL: " << initialPosition[k] << " : " << lowerBound[k] << " -> " << upperBound[k] << endl;
    //}


    itk::ParticleSwarmOptimizer::ParametersType initialParameters( paramSpaceDimensionality), finalParameters;
           
    unsigned int numberOfParticles = 100;
    itkOptimizer->SetNumberOfParticles( numberOfParticles );

    unsigned int maxIterations = 500;
    itkOptimizer->SetMaximalNumberOfIterations( maxIterations );

    double xTolerance = 0.0001;
    itkOptimizer->SetParametersConvergenceTolerance( xTolerance, costFunction->GetNumberOfParameters() );
                                                  
    double fTolerance = 0.0001;
    itkOptimizer->SetFunctionConvergenceTolerance( fTolerance );

    itkOptimizer->SetInitializeNormalDistribution( false ); // use uniform distribution
    itkOptimizer->SetInitialPosition( initialPosition );
   
    //  to get reproducible results for testing, use fixed seed. 
    itkOptimizer->SetUseSeed( true ); // use uniform distribution
    itkOptimizer->SetSeed( 0 ); // use uniform distribution


}

     

/*!  
 *  Fit the kinetics for a single voxel. 
 *      metKinetics0 = signal0
 *      metKinetics1 = signal1
 *      metKinetics2 = signal2
 *  Returns the best fitted metKinetics arrays. 
 */
void svkMRSKinetics::FitVoxelKinetics( int voxelID )
{


    typedef itk::ParticleSwarmOptimizer OptimizerType;
    OptimizerType::Pointer itkOptimizer = OptimizerType::New();
    this->InitOptimizer( itkOptimizer, voxelID );

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

    typedef svkKineticModelCostFunction::ParametersType ParametersType;
    ParametersType finalPosition = itkOptimizer->GetCurrentPosition();

    // check results to see if it is within range
    bool pass = true;
    // Exercise various member functions.
    //itkOptimizer->Print( cout );
    if( !pass ) {
        cout << "Test failed." << endl;
        return;
    }
    //cout << "Test passed." << endl;

    //  ===================================================
    //  Save fitted kinetics into algorithm output object cell data
    //  ===================================================
    float  TR     = 3.;          //   this shoudl be read from the data
    double T1all  = finalPosition[0] * TR;
    double Kpl    = finalPosition[1] / TR;
    double Ktrans = finalPosition[2] / TR;
    cout << "T1all(s):      " << T1all  << endl;
    cout << "Kpl(1/s):      " << Kpl    << endl;
    if ( this->modelType == svkMRSKinetics::TWO_SITE_EXCHANGE_PERF ) {
        cout << "Ktrans(1/s):   " << Ktrans << endl;
    }

    //  This isn't great.  I think this type of model specific initialization should be done in the
    //  specific model, not this generic driver class: 
    this->GetOutput(3)->GetPointData()->GetArray(0)->SetTuple1( voxelID, T1all ); 
    this->GetOutput(4)->GetPointData()->GetArray(0)->SetTuple1( voxelID, Kpl   );  
    this->GetOutput(5)->GetPointData()->GetArray(0)->SetTuple1( voxelID, Ktrans);  

    svkKineticModelCostFunction::Pointer costFunction;
    this->InitCostFunction( costFunction, voxelID ); 

    costFunction->GetKineticModel(  finalPosition); 


    //  write out results to imageDataOutput 
    int numSignalsInModel = this->GetNumSignals();
    for (int sigNum = 0; sigNum < numSignalsInModel; sigNum++ ) {
        vtkFloatArray* outputDynamics = vtkFloatArray::SafeDownCast(
            svkMriImageData::SafeDownCast(this->GetOutput(sigNum))->GetCellDataRepresentation()->GetArray(voxelID)
        );

        for ( int t = 0; t < this->numTimePoints; t++ ) {
            //cout << "Fitted Pyruvate(" << t << "): " <<  kineticModel0[t] << " " << signal0[t] << endl; 
            outputDynamics->SetTuple1(t, costFunction->GetModelSignalAtTime(sigNum, t) ); 
        }
    }

    return;
}



/*! 
 *  Zero data
 */
void svkMRSKinetics::ZeroData()
{
    int numVoxels[3];
    this->GetOutput(1)->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];

    int numSignalsInModel = this->GetNumSignals();

    svkKineticModelCostFunction::Pointer costFunction;
    this->GetCostFunction( costFunction ); 
    const unsigned int paramSpaceDimensionality = costFunction->GetNumberOfParameters();
    int num3DOutputMaps = 3; //paramSpaceDimensionality; 
    //int num3DOutputMaps = paramSpaceDimensionality; 

    double zeroValue = 0.;
    for (int i = 0; i < totalVoxels; i++ ) {
        for ( int time = 0; time < this->numTimePoints; time++ ) {
            //  zero each 4D dynamic output signal
            for (int sigNum = 0; sigNum < numSignalsInModel; sigNum++ ) {
                this->GetOutput(sigNum)->GetPointData()->GetArray(time)->SetTuple1(i, zeroValue);
            }
        }
        //  zero each 3D output map 
        for (int mapNum = numSignalsInModel; mapNum < num3DOutputMaps + numSignalsInModel; mapNum++) {
            cout << "MN: " << mapNum << endl;
            this->GetOutput(mapNum)->GetPointData()->GetArray(0)->SetTuple1(i, zeroValue);
        }
    }
}


/*! 
 *  Sync Point Data from Cell Data
 */
void svkMRSKinetics::SyncPointsFromCells()
{
    int numVoxels[3];
    this->GetOutput(1)->GetNumberOfVoxels(numVoxels);
    int numCells = numVoxels[0] * numVoxels[1] * numVoxels[2];

    int numSignalsInModel = this->GetNumSignals();
    for (int sigNum = 0; sigNum < numSignalsInModel; sigNum++ ) {

        svkImageData* imageData = this->GetOutput(sigNum); 
        float cellValueAtTime[1]; 

        for ( int timePoint = 0; timePoint < this->numTimePoints; timePoint++ ) { 

            imageData->GetPointData()->SetActiveScalars( 
                imageData->GetPointData()->GetArray( timePoint )->GetName() 
            );

            //  Loop over cells at this time point: 
            for (int cellID = 0; cellID < numCells; cellID++ ) {

                //  Get the value for the cell and time
                vtkFloatArray* outputCellData = vtkFloatArray::SafeDownCast(
                    svkMriImageData::SafeDownCast(imageData)->GetCellDataRepresentation()->GetArray( cellID )
                );
                outputCellData->GetTupleValue(timePoint, cellValueAtTime);

                //  insert it into the correct point data array: 
                imageData->GetPointData()->GetScalars()->SetTuple1(cellID, cellValueAtTime[0]);
            }
        }
    }
}



/*!
 *
 */
void svkMRSKinetics::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
}


/*!
 *  input ports 0 - 2 are required. All input ports are MRI data. 
 *      0:  pyruvate signal vs time
 *      1:  lactate signal vs time
 *      2:  urea signal vs time
 *      3:  spatial mask ROI (optional) 
 */
int svkMRSKinetics::FillInputPortInformation( int port, vtkInformation* info )
{
    if ( port < 3 ) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
    } 
    if ( port == 3 ) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
        info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 3);
    } 
    return 1;
}



/*!
 *  Output from this algo is an svkMriImageData object. 
 */
int svkMRSKinetics::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData"); 
    return 1;
}

