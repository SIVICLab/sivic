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
 *      Nonlinear LS fit added by: Christine Leon (Jul 28, 2012)
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


vtkCxxRevisionMacro(svkMRSKinetics, "$Rev$");
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

    this->newSeriesDescription = ""; 
    //  3 required input ports: 
    this->SetNumberOfInputPorts(4); //3 input metabolites + roi mask

    //  Outputports:  0 for fitted pyruvate kinetics
    //  Outputports:  1 for fitted lactate kinetics
    //  Outputports:  2 for fitted urea kinetics
    //  Outputports:  3 for Kpl map 
    //  Outputports:  4 for T1all map 
    this->SetNumberOfOutputPorts(5); 
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
    int indexArray[1];
    indexArray[0] = 0;
    svkMriImageData::SafeDownCast( this->GetImageDataInput(0) )->GetCellDataRepresentation()->GetImage(
        svkMriImageData::SafeDownCast( this->GetOutput(0) ), 
        0, 
        this->newSeriesDescription, 
        indexArray, 
        0 
    ); 
    svkMriImageData::SafeDownCast( this->GetImageDataInput(0) )->GetCellDataRepresentation()->GetImage(
        svkMriImageData::SafeDownCast( this->GetOutput(1) ), 
        0, 
        this->newSeriesDescription, 
        indexArray, 
        0 
    ); 
    svkMriImageData::SafeDownCast( this->GetImageDataInput(0) )->GetCellDataRepresentation()->GetImage(
        svkMriImageData::SafeDownCast( this->GetOutput(2) ), 
        0, 
        this->newSeriesDescription, 
        indexArray, 
        0 
    ); 
    svkMriImageData::SafeDownCast( this->GetImageDataInput(0) )->GetCellDataRepresentation()->GetImage(
        svkMriImageData::SafeDownCast( this->GetOutput(3) ), 
        0, 
        this->newSeriesDescription, 
        indexArray, 
        0 
    ); 
    svkMriImageData::SafeDownCast( this->GetImageDataInput(0) )->GetCellDataRepresentation()->GetImage(
        svkMriImageData::SafeDownCast( this->GetOutput(4) ), 
        0, 
        this->newSeriesDescription, 
        indexArray, 
        0 
    ); 


    //  ===============================================
    //  Create and initialize output data objects for fitted results: 
    //  ===============================================
    svkMriImageData* data = svkMriImageData::SafeDownCast(this->GetImageDataInput(0) );
    svkMriImageData* fittedPyrKinetics  = svkMriImageData::SafeDownCast( this->GetOutput(0) );
    svkMriImageData* fittedLacKinetics  = svkMriImageData::SafeDownCast( this->GetOutput(1) );
    svkMriImageData* fittedUreaKinetics = svkMriImageData::SafeDownCast( this->GetOutput(2) );
    fittedPyrKinetics->DeepCopy(data);
    fittedLacKinetics->DeepCopy(data);
    fittedUreaKinetics->DeepCopy(data);

    this->numTimePoints = data->GetDcmHeader()->GetNumberOfTimePoints();
    
    int numVoxels[3];
    this->GetOutput(1)->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];

    vtkFloatArray* templateArray = vtkFloatArray::SafeDownCast(
            svkMriImageData::SafeDownCast(this->GetImageDataInput(0))->GetCellDataRepresentation()->GetArray(0)
    );
    cout << " TEMPLATEARRAY: " << *templateArray << endl;
    for ( int time = 0; time < this->numTimePoints; time++ ) {
        templateArray->SetTuple1(time, 0); 
    }


    for (int i = 0; i < totalVoxels; i++ ) {

        vtkFloatArray* outputArray;

        outputArray = vtkFloatArray::SafeDownCast(
            fittedPyrKinetics->GetCellDataRepresentation()->GetArray(i)
        );
        outputArray->DeepCopy(templateArray); 

        outputArray = vtkFloatArray::SafeDownCast(
            fittedLacKinetics->GetCellDataRepresentation()->GetArray(i)
        );
        outputArray->DeepCopy(templateArray); 

        outputArray = vtkFloatArray::SafeDownCast(
            fittedUreaKinetics->GetCellDataRepresentation()->GetArray(i)
        );
        outputArray->DeepCopy(templateArray); 
    }


    //  ===============================================
    //  ===============================================
    this->GenerateKineticParamMap();
    //  ===============================================

    svkDcmHeader* hdr = this->GetOutput(1)->GetDcmHeader();
    hdr->InsertUniqueUID("SeriesInstanceUID");
    hdr->InsertUniqueUID("SOPInstanceUID");
    hdr->SetValue("SeriesDescription", this->newSeriesDescription);

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

    //  Get the parameter map data arrays to initialize.  
    this->mapArrayKpl   = this->GetOutput(3)->GetPointData()->GetArray(0);
    this->mapArrayT1all = this->GetOutput(4)->GetPointData()->GetArray(0);

    //  Add the output volume array to the correct array in the svkMriImageData object
    vtkstd::string arrayNameStringKpl("Kpl");
    vtkstd::string arrayNameStringT1all("T1all");
    this->mapArrayKpl->SetName( arrayNameStringKpl.c_str() );
    this->mapArrayT1all->SetName( arrayNameStringT1all.c_str() );

    double voxelValue;
    int numVoxels[3];
    this->GetOutput(1)->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];

    //  set up mask:  
    svkMriImageData* maskImage = svkMriImageData::SafeDownCast(this->GetImageDataInput(svkMRSKinetics::MASK) );
    cout << "MASKIM" << *maskImage << endl;
    unsigned short* mask = static_cast<vtkUnsignedShortArray*>( 
        maskImage->GetPointData()->GetArray(0) )->GetPointer(0) ; 

    for (int i = 0; i < totalVoxels; i++ ) {

        cout << "VOXEL NUMBER: " << i << endl;
        cout << "MASK VALUE: " << mask[i] << endl;

        if ( mask[i] != 0  ) {

            vtkFloatArray* kineticTrace0 = vtkFloatArray::SafeDownCast(
                svkMriImageData::SafeDownCast(
                    this->GetImageDataInput(0))->GetCellDataRepresentation()->GetArray(i)
            );
            cout << *kineticTrace0 << endl; 
            vtkFloatArray* kineticTrace1 = vtkFloatArray::SafeDownCast(
                svkMriImageData::SafeDownCast(
                    this->GetImageDataInput(1))->GetCellDataRepresentation()->GetArray(i)
            );
            vtkFloatArray* kineticTrace2 = vtkFloatArray::SafeDownCast(
                svkMriImageData::SafeDownCast(
                    this->GetImageDataInput(2))->GetCellDataRepresentation()->GetArray(i)
            );

            float* metKinetics0 = kineticTrace0->GetPointer(0);
            float* metKinetics1 = kineticTrace1->GetPointer(0);
            float* metKinetics2 = kineticTrace2->GetPointer(0);

            this->FitVoxelKinetics( metKinetics0, metKinetics1, metKinetics2, i );
        }
    }

    //  This syncs the cell data back to the point data, should be in svkMriImageData class
    this->SyncPointsFromCells(); 

}


/*!
 * 
 */
void svkMRSKinetics::InitOptimizer( float* metKinetics0, float* metKinetics1, float* metKinetics2, itk::ParticleSwarmOptimizer::Pointer itkOptimizer )
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0) );

    //========================================================
    //  ITK Optimization 
    //========================================================
    //svkKineticModelCostFunction::Pointer costFunction = svk2SiteExchangeCostFunction::New();
    svkKineticModelCostFunction::Pointer costFunction = svk2SitePerfCostFunction::New();
    itkOptimizer->SetCostFunction( costFunction.GetPointer() );

    costFunction->SetSignal0( metKinetics0 );
    costFunction->SetSignal1( metKinetics1 );
    costFunction->SetSignal2( metKinetics2 );
    costFunction->SetNumTimePoints( this->numTimePoints );

    //  set dimensionality of parameter space: 
    const unsigned int paramSpaceDimensionality = costFunction->GetNumberOfParameters();
    typedef svkKineticModelCostFunction::ParametersType ParametersType;
    ParametersType  initialPosition( paramSpaceDimensionality );

    int TR = 5;
    initialPosition[0] =  40/TR;
    initialPosition[1] =  0.05/TR;
    initialPosition[2] =  1;
    initialPosition[3] =  1;

    //  Set bounds
    itk::ParticleSwarmOptimizer::ParameterBoundsType bounds;

    // first order range of values: 
    float upperBound0 = 50;  
    float lowerBound0 = 1;
    float upperBound1 = 1;  
    float lowerBound1 = 0;
    float upperBound2 = 100;  
    float lowerBound2 = -100;
    float upperBound3 = 100;  
    float lowerBound3 = -100;
    bounds.push_back( std::make_pair( lowerBound0, upperBound0 ) );    // bounds param 0
    bounds.push_back( std::make_pair( lowerBound1, upperBound1 ) );    // bounds param 0
    bounds.push_back( std::make_pair( lowerBound2, upperBound2 ) );    // bounds param 0
    bounds.push_back( std::make_pair( lowerBound3, upperBound3 ) );    // bounds param 0
    itkOptimizer->SetParameterBounds( bounds );

    itk::ParticleSwarmOptimizer::ParametersType initialParameters( paramSpaceDimensionality), finalParameters;
           
    unsigned int numberOfParticles = 10;
    itkOptimizer->SetNumberOfParticles( numberOfParticles );

    unsigned int maxIterations = 500;
    itkOptimizer->SetMaximalNumberOfIterations( maxIterations );

    double xTolerance = 0.01;
    itkOptimizer->SetParametersConvergenceTolerance( xTolerance, costFunction->GetNumberOfParameters() );
                                                  
    double fTolerance = 0.01;
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
void svkMRSKinetics::FitVoxelKinetics(float* metKinetics0, float* metKinetics1, float* metKinetics2, int voxelIndex )
{

    typedef itk::ParticleSwarmOptimizer OptimizerType;
    OptimizerType::Pointer itkOptimizer = OptimizerType::New();
    this->InitOptimizer(  metKinetics0,  metKinetics1,  metKinetics2, itkOptimizer );

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
    //  Save fitted kinetics into algorithm output object cell data
    //  ===================================================
    double T1all  = 1/finalPosition[0];
    double Kpl    = finalPosition[1];
    //cout << "T1all: " << T1all << endl;
    //cout << "Kpl:   " << Kpl   << endl;
    mapArrayKpl->SetTuple1(voxelIndex, Kpl);
    mapArrayT1all->SetTuple1(voxelIndex, T1all);

    // print out fitted results: 
    vtkFloatArray* kineticTrace0 = vtkFloatArray::SafeDownCast(
        svkMriImageData::SafeDownCast(this->GetImageDataInput(0))->GetCellDataRepresentation()->GetArray(voxelIndex) ); 
    vtkFloatArray* kineticTrace1 = vtkFloatArray::SafeDownCast(
        svkMriImageData::SafeDownCast(this->GetImageDataInput(1))->GetCellDataRepresentation()->GetArray(voxelIndex) ); 
    vtkFloatArray* kineticTrace2 = vtkFloatArray::SafeDownCast(
        svkMriImageData::SafeDownCast(this->GetImageDataInput(2))->GetCellDataRepresentation()->GetArray(voxelIndex) ); 

    float* signal0 = kineticTrace0->GetPointer(0);
    float* signal1 = kineticTrace1->GetPointer(0);
    float* signal2 = kineticTrace2->GetPointer(0);

    // assign new data to voxelarray
    float* kineticModel0 = new float [this->numTimePoints];
    float* kineticModel1 = new float [this->numTimePoints];
    float* kineticModel2 = new float [this->numTimePoints];
    //svkKineticModelCostFunction::Pointer costFunction = svk2SiteExchangeCostFunction::New();
    svkKineticModelCostFunction::Pointer costFunction = svk2SitePerfCostFunction::New();

    costFunction->GetKineticModel(  finalPosition, 
                                    kineticModel0, 
                                    kineticModel1, 
                                    kineticModel2, 
                                    signal0, 
                                    signal1, 
                                    signal2, 
                                    this->numTimePoints ); 

    //  write out results to imageDataOutput 
    vtkFloatArray* outputDynamics0 = vtkFloatArray::SafeDownCast(
        svkMriImageData::SafeDownCast(this->GetOutput(0))->GetCellDataRepresentation()->GetArray(voxelIndex)
    );
    vtkFloatArray* outputDynamics1 = vtkFloatArray::SafeDownCast(
        svkMriImageData::SafeDownCast(this->GetOutput(1))->GetCellDataRepresentation()->GetArray(voxelIndex)
    );
    vtkFloatArray* outputDynamics2 = vtkFloatArray::SafeDownCast(
        svkMriImageData::SafeDownCast(this->GetOutput(2))->GetCellDataRepresentation()->GetArray(voxelIndex)
    );
    for ( int t = 0; t < this->numTimePoints; t++ ) {
        cout << "Fitted Pyruvate(" << t << "): " <<  kineticModel0[t] << " " << signal0[t] << endl; 
        outputDynamics0->SetTuple1(t, kineticModel0[t]);
        outputDynamics1->SetTuple1(t, kineticModel1[t]);
        outputDynamics2->SetTuple1(t, kineticModel2[t]);
    }

    delete[] kineticModel0; 
    delete[] kineticModel1; 
    delete[] kineticModel2; 

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

    double zeroValue = 0.;
    //for ( int time = 0; time < this->numTimePoints; time++ ) {
        //for (int i = 0; i < totalVoxels; i++ ) {
            //this->GetOutput(0)->GetPointData()->GetArray(time)->SetTuple1(i, zeroValue);
            //this->GetOutput(1)->GetPointData()->GetArray(time)->SetTuple1(i, zeroValue);
            //this->GetOutput(2)->GetPointData()->GetArray(time)->SetTuple1(i, zeroValue);
        //}
    //}
    for (int i = 0; i < totalVoxels; i++ ) {
        for ( int time = 0; time < this->numTimePoints; time++ ) {
            this->GetOutput(0)->GetPointData()->GetArray(time)->SetTuple1(i, zeroValue);
            this->GetOutput(1)->GetPointData()->GetArray(time)->SetTuple1(i, zeroValue);
            this->GetOutput(2)->GetPointData()->GetArray(time)->SetTuple1(i, zeroValue);
        }
        this->GetOutput(3)->GetPointData()->GetArray(0)->SetTuple1(i, zeroValue);
        this->GetOutput(4)->GetPointData()->GetArray(0)->SetTuple1(i, zeroValue);
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

    svkImageData* imageData0 = this->GetOutput(0); 
    svkImageData* imageData1 = this->GetOutput(1); 
    svkImageData* imageData2 = this->GetOutput(2); 
    float cellValueAtTime0[1]; 
    float cellValueAtTime1[1]; 
    float cellValueAtTime2[1]; 

    for ( int timePoint = 0; timePoint < this->numTimePoints; timePoint++ ) { 

        imageData0->GetPointData()->SetActiveScalars( 
            imageData0->GetPointData()->GetArray( timePoint )->GetName() 
        );
        imageData1->GetPointData()->SetActiveScalars( 
            imageData1->GetPointData()->GetArray( timePoint )->GetName() 
        );
        imageData2->GetPointData()->SetActiveScalars( 
            imageData2->GetPointData()->GetArray( timePoint )->GetName() 
        );

        //  Loop over cells at this time point: 
        for (int cellID = 0; cellID < numCells; cellID++ ) {

            //  Get the value for the cell and time
            vtkFloatArray* outputCellData0 = vtkFloatArray::SafeDownCast(
                svkMriImageData::SafeDownCast(imageData0)->GetCellDataRepresentation()->GetArray( cellID )
            );
            vtkFloatArray* outputCellData1 = vtkFloatArray::SafeDownCast(
                svkMriImageData::SafeDownCast(imageData1)->GetCellDataRepresentation()->GetArray( cellID )
            );
            vtkFloatArray* outputCellData2 = vtkFloatArray::SafeDownCast(
                svkMriImageData::SafeDownCast(imageData2)->GetCellDataRepresentation()->GetArray( cellID )
            );
            outputCellData0->GetTupleValue(timePoint, cellValueAtTime0);
            outputCellData1->GetTupleValue(timePoint, cellValueAtTime1);
            outputCellData2->GetTupleValue(timePoint, cellValueAtTime2);

            //  insert it into the correct point data array: 
            imageData0->GetPointData()->GetScalars()->SetTuple1(cellID, cellValueAtTime0[0]);
            imageData1->GetPointData()->GetScalars()->SetTuple1(cellID, cellValueAtTime1[0]);
            imageData2->GetPointData()->GetScalars()->SetTuple1(cellID, cellValueAtTime2[0]);
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

