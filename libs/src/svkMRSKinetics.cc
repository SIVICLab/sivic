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
 *      Christine Leon 
 */


#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkInformationVector.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>

#include <svkMRSKinetics.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <svkKineticModelCostFunction.h>
#include <svk2SiteExchangeCostFunction.h>
#include <svk2SitePerfCostFunction.h>
#include <svk2SiteIMCostFunction.h>
#include <svk2SiteIMPyrCostFunction.h>



using namespace svk;


vtkStandardNewMacro(svkMRSKinetics);


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

    this->modelType = svkMRSKinetics::UNDEFINED;     
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
void svkMRSKinetics::SetSeriesDescription( std::string newSeriesDescription )
{
    this->newSeriesDescription = newSeriesDescription;
    this->Modified(); 
}


/*!
 *  Set model 
 */
void svkMRSKinetics::SetModelType( svkMRSKinetics::MODEL_TYPE modelType)
{
    if ( modelType < svkMRSKinetics::FIRST_MODEL || modelType > svkMRSKinetics::LAST_MODEL ) {
        cout << "ERROR: invalid model type: " << modelType << endl;
        exit(1); 
    }
    this->modelType = modelType;

    int numModelSignals = this->GetNumberOfModelSignals(); 
    int numMasks = 1; 
    this->SetNumberOfInputPorts(numModelSignals + numMasks ); //3 input metabolites + roi mask
    this->SetNumberOfOutputPorts( this->GetNumberOfModelOutputPorts() ); 
    this->InitModelOutputDescriptionVector(); 

}


/*!
 *  Set the TR for the experiment (in seconds).  This is the time between 
 *  kinetic samples and is used to scale the and params 
 */
void svkMRSKinetics::SetTR( float TR )
{
    this->TR = TR; 
}


/*!
 *  Get the TR for the experiment (in seconds).  This is the time between 
 *  kinetic samples and is used to scale the and params 
 */
float svkMRSKinetics::GetTR( )
{
    return this->TR;
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
    vtkFloatArray* templateArray = vtkFloatArray::New(); 
    templateArray->DeepCopy(
        vtkFloatArray::SafeDownCast(
                svkMriImageData::SafeDownCast(this->GetImageDataInput(0))->GetCellDataRepresentation()->GetArray(0)
        )
    ); 
    for ( int time = 0; time < this->numTimePoints; time++ ) {
        templateArray->SetTuple1(time, 0); 
    }

    int numVoxels[3];
    this->GetOutput(1)->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];

    int numSignalsInModel = this->GetNumberOfModelSignals();
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
    int numSignals = GetNumberOfModelSignals(); 
    for ( int i = numSignals; i < this->GetNumberOfModelOutputPorts();  i++ ) { 
        this->GetOutput(i)->GetPointData()->GetArray(0)->SetName( this->modelOutputDescriptionVector[i].c_str() );
    }

    double voxelValue;
    int numVoxels[3];
    this->GetOutput(1)->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];

    //  set up mask.  If no input mask was provided, then set the mask to   
    svkMriImageData* maskImage;
    unsigned short* mask;
    bool  hasMask = false;       
    int maskIndex = numSignals; // input after last mask
    if (this->GetInput( maskIndex )) {
        maskImage = svkMriImageData::SafeDownCast(this->GetImageDataInput(maskIndex ) );
        mask      = static_cast<vtkUnsignedShortArray*>( 
                        maskImage->GetPointData()->GetArray(0) )->GetPointer(0) ; 
        hasMask = true; 
    } 

    this->InitAverageDynamics(hasMask, totalVoxels, mask);

    this->PrintInitialParamBounds(); 

    for (int voxelID = 0; voxelID < totalVoxels; voxelID++ ) {

        //cout << "VOXEL NUMBER: " << voxelID << endl;
        //cout << "MASK VALUE: " << mask[voxelID] << endl;

        //  If there is a mask check it.  If no mask provided, the compute all voxels. 
        if ( ((hasMask == true) && (mask[voxelID] != 0 )) || ( hasMask == false )  ) {
            cout << endl;
            //cout << "Fit Voxel " << voxelID << endl;
            this->FitVoxelKinetics( voxelID );
        }
    }

    //  This syncs the cell data back to the point data, should be in svkMriImageData class
    this->SyncPointsFromCells(); 

}


/*!
 *  Creates an average dynamic trace that can be used to estimate initial param values
 *  for fitting.
 *      @param hasMask  true or false
 *      @totalVoxels    total number of voxels in data set.
 */
void svkMRSKinetics::InitAverageDynamics(bool hasMask, int totalVoxels, unsigned short* mask)
{

    //  Initialize an array for the average dynamic trace
    float cellValueAtTime[1];
    cellValueAtTime[0] = 0;

    int numSignalsInModel = this->GetNumberOfModelSignals();
    for ( int sigNum = 0; sigNum < numSignalsInModel; sigNum++ ) {
        vtkFloatArray* averageSignal;
        averageSignal = vtkFloatArray::New();
        averageSignal->DeepCopy(
            vtkFloatArray::SafeDownCast(
                svkMriImageData::SafeDownCast(
                    this->GetImageDataInput(sigNum))->GetCellDataRepresentation()->GetArray(0)
            )
        );
        for ( int time = 0; time < this->numTimePoints; time++ ) {
            averageSignal->SetTuple(time, cellValueAtTime);
        }
        this->averageSignalVector.push_back(averageSignal); 
    }


    //  average signals over all voxels
    double averageValue;  
    float averageCell[1]; 
    for ( int sigNum = 0; sigNum < numSignalsInModel; sigNum++ ) {
        for ( int time = 0; time < this->numTimePoints; time++ ) {

            int numVoxelsInMask = 0; 

            for (int voxelID = 0; voxelID < totalVoxels; voxelID++ ) {

                if (((hasMask == true) && (mask[voxelID] != 0)) || (hasMask == false)) {

                    numVoxelsInMask += 1; 

                    vtkFloatArray* kineticSignal = vtkFloatArray::SafeDownCast(
                        svkMriImageData::SafeDownCast(
                            this->GetImageDataInput(sigNum))->GetCellDataRepresentation()->GetArray(voxelID)
                    );
                    averageValue = this->averageSignalVector[sigNum]->GetTuple1( time );
                    averageValue += kineticSignal->GetTuple1( time ); 
                    averageCell[0] = averageValue; 
                    this->averageSignalVector[sigNum]->SetTuple( time, averageCell );
                }
            }

            averageValue = this->averageSignalVector[sigNum]->GetTuple1( time );
            averageValue /= numVoxelsInMask; 
            averageCell[0] = averageValue; 
            //cout << "average: " << averageValue << endl;
            this->averageSignalVector[sigNum]->SetTuple( time, averageCell );
        }
    }

}


/*!
 *  Initialize the cost function with the input signals 
 */
void svkMRSKinetics::InitCostFunction(  svkKineticModelCostFunction::Pointer& costFunction,  int voxelID )
{
    this->GetCostFunction( costFunction ); 

    int numSignalsInModel = this->GetNumberOfModelSignals();
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
 *  Factory to get the correct cost function.
 */
void svkMRSKinetics::GetCostFunction( svkKineticModelCostFunction::Pointer& costFunction)
{
    if ( this->modelType == svkMRSKinetics::TWO_SITE_EXCHANGE ) { 
        costFunction = svk2SiteExchangeCostFunction::New();
    } else if ( this->modelType == svkMRSKinetics::TWO_SITE_EXCHANGE_PERF ) {
        costFunction = svk2SitePerfCostFunction::New();
    } else if ( this->modelType == svkMRSKinetics::TWO_SITE_IM) {
        //  2 site piecewise Integrated Model
        costFunction = svk2SiteIMCostFunction::New();
    } else if ( this->modelType == svkMRSKinetics::TWO_SITE_IM_PYR) {
        costFunction = svk2SiteIMPyrCostFunction::New();
    }
}


/*!
 *  Get the number of signals required for the specified cost function
 */
int svkMRSKinetics::GetNumberOfModelSignals()
{
    svkKineticModelCostFunction::Pointer costFunction;
    this->GetCostFunction( costFunction ); 
    return costFunction->GetNumberOfSignals(); 
}


/*!
 *  Get the number of otuput ports for the const function
 *  This is the sum of the :
 *      number of signals
 *      number of parameters
 *      +1 for the cost function map 
 */
int svkMRSKinetics::GetNumberOfModelOutputPorts()
{
    svkKineticModelCostFunction::Pointer costFunction;
    this->GetCostFunction( costFunction ); 
    return costFunction->GetNumberOfOutputPorts() + 1; 
}


/*!
 *  Get the number of otuput parameters (parameter maps)
 *  This is the sum of the :
 *      number of parameters
 */
int svkMRSKinetics::GetNumberOfModelParameters()
{
    svkKineticModelCostFunction::Pointer costFunction;
    this->GetCostFunction( costFunction ); 
    return costFunction->GetNumberOfParameters(); 
}


/*! 
 *  Initialize the output description vector in the specific model/cost function
 */
void svkMRSKinetics::InitModelOutputDescriptionVector()
{
    svkKineticModelCostFunction::Pointer costFunction;
    this->GetCostFunction( costFunction ); 
    costFunction->InitOutputDescriptionVector( &this->modelOutputDescriptionVector ); 
   
    //  finally add one for the cost function output map 
    //int numPorts = this->GetNumberOfModelOutputPorts(); 
    this->modelOutputDescriptionVector.push_back("rss");

}


/*
 *  Get the description of the model's n'th output 
 */
string svkMRSKinetics::GetModelOutputDescription( int outputIndex )
{
    return this->modelOutputDescriptionVector[outputIndex]; 
}


void svkMRSKinetics::PrintInitialParamBounds()
{   

    svkKineticModelCostFunction::Pointer costFunction;
    this->InitCostFunction( costFunction, 0);

    const unsigned int paramSpaceDimensionality = costFunction->GetNumberOfParameters();
    typedef svkKineticModelCostFunction::ParametersType ParametersType;
    ParametersType  initialPosition( paramSpaceDimensionality );

    int numParams = this->GetNumberOfModelParameters();
    vector<float> lowerBounds(numParams);
    vector<float> upperBounds(numParams);

    costFunction->SetTR( this->GetTR() );
    costFunction->InitScaledParamBounds( &lowerBounds, &upperBounds, &(this->averageSignalVector) );
    costFunction->SetCustomizedScaledParamBounds( 
            &lowerBounds, 
            &upperBounds, 
            *this->customBoundsParamNumbers, 
            *this->customLowerBounds, 
            *this->customUpperBounds  
    ); 
    costFunction->InitParamInitialPosition( &initialPosition, &lowerBounds, &upperBounds ); 
    costFunction->PrintParmBounds( &initialPosition, &lowerBounds, &upperBounds, &this->modelOutputDescriptionVector ); 

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

    //  ============================================================
    //  Set model's parameters search bounds
    //  ============================================================
    itk::ParticleSwarmOptimizer::ParameterBoundsType bounds;

    //  Set bounds in dimensionless units (each time point is a TR)
    int numParams = this->GetNumberOfModelParameters(); 
    vector<float> lowerBounds(numParams); 
    vector<float> upperBounds(numParams); 

    //  Set the temporal resolution of the data
    costFunction->SetTR( this->GetTR() ); 
    costFunction->InitScaledParamBounds( &lowerBounds, &upperBounds, &(this->averageSignalVector) ); 
    costFunction->SetCustomizedScaledParamBounds( 
            &lowerBounds, 
            &upperBounds, 
            *this->customBoundsParamNumbers, 
            *this->customLowerBounds, 
            *this->customUpperBounds  
    ); 

    for (int paramNum  = 0; paramNum < numParams; paramNum++ ) {
        //cout << "BOUNDS: " << lowerBounds[paramNum] << " " << upperBounds[paramNum] << endl;
        bounds.push_back( std::make_pair( lowerBounds[paramNum], upperBounds[paramNum] ) );    
    }
    itkOptimizer->SetParameterBounds( bounds );


    //  ============================================================
    //  Set initial guesses
    //  ============================================================
    costFunction->InitParamInitialPosition( &initialPosition, &lowerBounds, &upperBounds ); 


    //  ============================================================
    //  Set optimizer convergence parameters
    //  ============================================================
    itk::ParticleSwarmOptimizer::ParametersType initialParameters( paramSpaceDimensionality), finalParameters;
           
    unsigned int numberOfParticles = 200;
    itkOptimizer->SetNumberOfParticles( numberOfParticles );

    unsigned int maxIterations = 3000;
    itkOptimizer->SetMaximalNumberOfIterations( maxIterations );

    double xTolerance = 0.000001;
    itkOptimizer->SetParametersConvergenceTolerance( xTolerance, costFunction->GetNumberOfParameters() );
                                                  
    double fTolerance = 0.000001;
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

    //  ===================================================
    //  Save fitted kinetics into algorithm output object cell data
    //  Get the scaled values fo the final fitted params to insert into the 
    //  algo outputs:
    //  ===================================================
    typedef svkKineticModelCostFunction::ParametersType ParametersType;
    ParametersType finalPosition = itkOptimizer->GetCurrentPosition();


    // check results to see if it is within range
    bool pass = true;
    // Exercise various member functions.
    // itkOptimizer->Print( cout );
    if( !pass ) {
        cout << "Test failed." << endl;
        return;
    }
    // cout << "Test passed." << endl;


    int numSignalsInModel = this->GetNumberOfModelSignals();

    svkKineticModelCostFunction::Pointer costFunction;
    this->InitCostFunction( costFunction, voxelID ); 
    costFunction->SetTR( this->GetTR() ); 
    cout << "=========================================" << endl;
    cout << "FINAL VALUES VOXEL = " << voxelID << endl;
    costFunction->GetKineticModel(  finalPosition ); 

    //  ==================================================================
    //  write out fitted signals from kinteic model to imageDataOutput 
    //  ==================================================================
    for (int sigNum = 0; sigNum < numSignalsInModel; sigNum++ ) {
        vtkFloatArray* outputDynamics = vtkFloatArray::SafeDownCast(
            svkMriImageData::SafeDownCast(this->GetOutput(sigNum))->GetCellDataRepresentation()->GetArray(voxelID)
        );

        for ( int t = 0; t < this->numTimePoints; t++ ) {
            //cout << "Fitted Pyruvate(" << t << "): " <<  costFunction->GetModelSignalAtTime(sigNum, t) <<  endl; 
            outputDynamics->SetTuple1(t, costFunction->GetModelSignalAtTime(sigNum, t) ); 
        }
    }

    //  ==================================================================
    //  write out maps of final fitted parameters form kinteic model to imageDataOutput 
    //  the signals and model are in unitless space (point space).  only scale the paramets as a 
    //  final step to write them out in physical units.      
    //  ==================================================================

    const unsigned int paramSpaceDimensionality = this->GetNumberOfModelParameters(); 
    int num3DOutputMaps = paramSpaceDimensionality; 

    //  And write out the cost function map as well 
    double residual = costFunction->GetValue( finalPosition ); 
    cout << setw(10) << left << "RESIDUAL: " << " = " << residual << endl;
    cout << "===========================" << endl;
    int residualMapIndex = num3DOutputMaps + numSignalsInModel;    
    this->GetOutput(residualMapIndex)->GetPointData()->GetArray(0)->SetTuple1( voxelID, residual); 

    //  Now write out individual param maps for the specified model
    costFunction->GetParamFinalScaledPosition( &finalPosition ); 
    for (int index = 0; index < num3DOutputMaps; index++ ) {
        //  the maps are the last set of outputs after the fitted signals: 
        int mapIndex = index + numSignalsInModel;    
        this->GetOutput(mapIndex)->GetPointData()->GetArray(0)->SetTuple1( voxelID, finalPosition[index]); 
        string paramName = this->modelOutputDescriptionVector[ numSignalsInModel + index]; 
        cout <<  setw(10) << left << paramName << " = " << finalPosition[index] << endl; 
    }
    cout << "===========================" << endl;

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

    int numSignalsInModel = this->GetNumberOfModelSignals();

    const unsigned int paramSpaceDimensionality = this->GetNumberOfModelParameters(); 
    int num3DOutputMaps = paramSpaceDimensionality; 
    num3DOutputMaps += 1; //    for the cost function map
    //cout << "NUM3D: " << num3DOutputMaps << endl;

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
            //cout << "MN: " << mapNum << endl;
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

    int numSignalsInModel = this->GetNumberOfModelSignals();
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


/*
 *  Initialize custom param search bounds for specific parameters 
 */
void svkMRSKinetics::SetCustomParamSearchBounds(vector<int>* customBoundsParamNumbers, vector<float>* customLowerBounds, vector<float>* customUpperBounds)
{
    this->customBoundsParamNumbers  = customBoundsParamNumbers;
    this->customLowerBounds         = customLowerBounds;
    this->customUpperBounds         = customUpperBounds ;
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
    int numModelSignals = this->GetNumberOfModelSignals(); 
    if ( port < numModelSignals ) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
    } 
    //  the last input is an optional mask
    int maskIndex = numModelSignals; 
    if ( port == maskIndex ) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
        info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), maskIndex);
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

