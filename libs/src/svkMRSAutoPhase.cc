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
 */


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


//vtkCxxRevisionMacro(svkMRSAutoPhase, "$Rev$");
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
    this->SetNumberOfOutputPorts(1); 
    //this->SetNumberOfThreads(1);
    svkMRSAutoPhase::progress = NULL;
    //this->SetPhasingModel(svkMRSAutoPhase::MAX_GLOBAL_PEAK_HT_0); 
    this->onlyUseSelectionBox = false; 
}



/*! 
 *  Only fit phase inside selection box
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
 *  Resets the origin and extent for correct initialization of output svkMriImageData object from input 
 *  svkMrsImageData object. 
 */
int svkMRSAutoPhase::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
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
int svkMRSAutoPhase::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0));

    //  Initialize the output object (1) that will contain the image map of phases
    int indexArray[1];
    indexArray[0] = 0;

    //  Initialize the output metabolite map using a single frequency image from the 
    //  input MRS object. Extract an image from the input and set it as the outputimage: 
    mrsData->GetZeroImage( svkMriImageData::SafeDownCast(this->GetOutput(0)) ); 
    svkMriImageData* mriData = svkMriImageData::SafeDownCast(this->GetOutput(0));

    //  if there are multiple input volumes, then copy those to additional zeroed point data arrays: 
    svkDcmHeader::DimensionVector dimensionVector = mrsData->GetDcmHeader()->GetDimensionIndexVector();
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );
    int numSpatialVoxels = svkDcmHeader::GetNumSpatialVoxels(&dimensionVector); 

    int numVolumes = numCells / numSpatialVoxels;
    if ( numVolumes > 1 ) { 
        mriData->GetPointData()->GetArray(0)->SetName("pixels0"); 
    }
    for ( int vol = 1; vol < numVolumes; vol++ ) {

        //  Get the point data from the initialized output image: 
        int vtkDataType = mrsData->GetCellData()->GetArray(0)->GetDataType();
        vtkDataArray* newVolumeArray;

        if ( vtkDataType == VTK_FLOAT ) {
            newVolumeArray =  vtkFloatArray::New();
        } else if ( vtkDataType == VTK_DOUBLE ) {
            newVolumeArray =  vtkDoubleArray::New();
        }
        newVolumeArray->DeepCopy( mriData->GetPointData()->GetScalars() );
        string arrayName("pixels"); 
        ostringstream volString; 
        volString << vol; 
        arrayName.append(volString.str()); 
        newVolumeArray->SetName( arrayName.c_str() );

        mriData->GetPointData()->AddArray( newVolumeArray );
    }

    //  ensure that the ZeroImage has the same dimensions as the MRS object template  
    //  (Coy dimensionVector and redimension)
    svkDcmHeader::DimensionVector mriDimensionVector = dimensionVector; 
    mriData->GetDcmHeader()->Redimension( &mriDimensionVector ); 


    svkDcmHeader* hdr = this->GetOutput(0)->GetDcmHeader();
    hdr->InsertUniqueUID("SeriesInstanceUID");
    hdr->InsertUniqueUID("SOPInstanceUID");
    this->SetMapSeriesDescription(); 

    this->numTimePoints = mrsData->GetDcmHeader()->GetIntValue( "DataPointColumns" );


    float tolerance = .5;     
    this->selectionBoxMask = new short[numSpatialVoxels];
    mrsData->GetSelectionBoxMask(this->selectionBoxMask, tolerance);

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
    //  sub-extent within each thread, via ThreadedRequestData. 
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

    this->UpdateProvenance();

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->Update();


    //this->SyncPointsFromCells(); 

    delete [] this->selectionBoxMask; 

    return 1;
};


/*! 
 *  Sync Point Data from Cell Data
 */
void svkMRSAutoPhase::SyncPointsFromCells()
{
    int numVoxels[3];
    this->GetOutput(0)->GetNumberOfVoxels(numVoxels);
    int numCells = numVoxels[0] * numVoxels[1] * numVoxels[2];

    svkImageData* imageData0 = this->GetOutput(0); 
    float cellValueAtTime0[1]; 
    //cout << *imageData0 << endl; 

    for ( int timePoint = 0; timePoint < 1; timePoint++ ) { 

        imageData0->GetPointData()->SetActiveScalars( 
            imageData0->GetPointData()->GetArray( timePoint )->GetName() 
        );

        //  Loop over cells at this time point: 
        for (int cellID = 0; cellID < numCells; cellID++ ) {

            //  Get the value for the cell and time
            vtkFloatArray* outputCellData0 = vtkFloatArray::SafeDownCast(
                svkMriImageData::SafeDownCast(imageData0)->GetCellData()->GetArray( cellID )
            );
            outputCellData0->GetTupleValue(timePoint, cellValueAtTime0);

            //  insert it into the correct point data array: 
            imageData0->GetPointData()->GetScalars()->SetTuple1(cellID, cellValueAtTime0[0]);
        }
    }
}


/*! 
 *  Zero output phase image map 
 */
void svkMRSAutoPhase::ZeroData()
{

    int numVoxels[3];
    this->GetOutput(0)->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];

    double zeroValue = 0.;

    for (int i = 0; i < totalVoxels; i++ ) {
        this->GetOutput(0)->GetPointData()->GetArray(0)->SetTuple1(i, zeroValue);
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

    if ( this->GetDebug() ) {
        cout << "THREADED EXECUTE " << id << endl ;
    }
    this->AutoPhaseExecute(outExt, id);

}


void svkMRSAutoPhase::ValidateInput()
{
    svkDcmHeader::DimensionVector mrsDimensionVector = this->GetImageDataInput(0)->GetDcmHeader()->GetDimensionIndexVector();
    int numMRSCells = svkDcmHeader::GetNumberOfCells( &mrsDimensionVector );

    svkDcmHeader::DimensionVector mriDimensionVector = this->GetOutput(0)->GetDcmHeader()->GetDimensionIndexVector();
    int numMRICells = svkDcmHeader::GetNumberOfCells( &mriDimensionVector );
    if ( numMRICells != numMRSCells ) {
        cout << "ERROR: Number of MRI and MRS Cells do not match" << endl;
        exit(1);
    }    
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
    svkDcmHeader::DimensionVector mrsDimensionVector = this->GetImageDataInput(0)->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector loopVector = mrsDimensionVector;

    int numThreads = this->GetNumberOfThreads();
    int numMRSCells = svkDcmHeader::GetNumberOfCells( &mrsDimensionVector );

    this->ValidateInput(); 


    for (int cellID = 0; cellID < numMRSCells; cellID++) {

        svkDcmHeader::GetDimensionVectorIndexFromCellID( &mrsDimensionVector, &loopVector, cellID );
        bool isCellInSubExtent = svkMrsImageData::IsIndexInExtent( ext, &loopVector ); 
   
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
            int percent = static_cast<int>(100 * (double)cellCount/(double)numMRSCells);
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

        //  Determine if the spatial location of the current cell is within the 
        //  selection box:     
        svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0));
        svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
        svkDcmHeader::DimensionVector indexVector = dimensionVector; 
        svkDcmHeader::GetDimensionVectorIndexFromCellID(&dimensionVector, &indexVector, cellID);

        //  Get the 3D spatial index for comparing if a given cell is in the spatial selectin box maks:   
        int spatialCellIndex = svkDcmHeader::GetSpatialCellIDFromDimensionVectorIndex( &dimensionVector, &indexVector);

        if ( this->selectionBoxMask[spatialCellIndex] == 0 ) {
            return; 
        }
    }

    this->FitPhase( cellID ); 

}



/*!
 *
 */
void svkMRSAutoPhase::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
    this->GetOutput(0)->GetProvenance()->AddAlgorithm( this->GetClassName() );
    svkMrsImageData::SafeDownCast(this->GetImageDataInput(0))->GetProvenance()->AddAlgorithm( this->GetClassName() );

}


/*!
 *  
 */
svkImageData* svkMRSAutoPhase::GetOutput(int port)
{
    return svkImageData::SafeDownCast( this->GetOutputDataObject(port) ); 
}



/*!
 *  input port 0 is required. . 
 */
int svkMRSAutoPhase::FillInputPortInformation( int vtkNotUsed( port ), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData");
    return 1;
}


/*!
 * 
 */
void svkMRSAutoPhase::SetMapSeriesDescription( )  
{
    this->GetOutput(0)->GetDcmHeader()->SetValue("SeriesDescription", this->seriesDescription); 
}


/*!
 *  Output from this algo is 
 *      1:  image of applied phases
 *      The phased MRS data is in place applied to the input object. 
 */
int svkMRSAutoPhase::FillOutputPortInformation( int port, vtkInformation* info )
{
    if ( port == 0 ) {
        info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData");
    } 
    return 1;
}

