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


#include <svkGEPFileMapperUCSFfidcsiDev0.h>
#include <svkMrsImageData.h>
#include <svkMrsImageFlip.h>
#include <svkMrsImageFourierCenter.h>
#include <svkEPSIPhaseCorrect.h>
#include <svkEPSIReorder.h>
#include </usr/include/vtk/vtkDebugLeaks.h>


using namespace svk;


//vtkCxxRevisionMacro(svkGEPFileMapperUCSFfidcsiDev0, "$Rev$");
vtkStandardNewMacro(svkGEPFileMapperUCSFfidcsiDev0);


/*!
 *
 */
svkGEPFileMapperUCSFfidcsiDev0::svkGEPFileMapperUCSFfidcsiDev0()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkGEPFileMapperUCSFfidcsiDev0");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );
    this->dadFile = NULL;
}


/*!
 *
 */
svkGEPFileMapperUCSFfidcsiDev0::~svkGEPFileMapperUCSFfidcsiDev0()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
    if( this->dadFile != NULL ) {
        this->dadFile->Delete();
    }
}


/*!
 *  Chop is off for UCSF fid dev0 (EPSI)
 */
bool svkGEPFileMapperUCSFfidcsiDev0::IsChopOn()
{
    return false; 
}


/*
 *  Returns the volume localization type = NONE. 
 */
string  svkGEPFileMapperUCSFfidcsiDev0::GetVolumeLocalizationTechnique()
{
    string localizationType("NONE");
    return localizationType; 
}


/*!
 *
 */
void svkGEPFileMapperUCSFfidcsiDev0::GetSelBoxCenter( float selBoxCenter[3] )
{

    selBoxCenter[0] = -1 * this->GetHeaderValueAsFloat( "rhi.ctr_R" );
    selBoxCenter[1] = -1 * this->GetHeaderValueAsFloat( "rhi.ctr_A" );
    selBoxCenter[2] = this->GetHeaderValueAsFloat( "rhi.ctr_S" );

} 


/*!
 *  For FIDCSI make the press box occupy the whole volume. 
 *  There isnt't full 3D PRESS, just a single slab of selection 
 *  (1D selective). 
 */
void svkGEPFileMapperUCSFfidcsiDev0::GetSelBoxSize( float selBoxSize[3] )
{

    cout << "THIS IS NOT ENCODED CORRECTLY.  THE EPSI-AXIS Slab selection should be correctly encoded here" << endl;
    selBoxSize[ 0 ] = this->GetHeaderValueAsFloat( "rhr.rh_user24" );  // RL FOV
    selBoxSize[ 1 ] = this->GetHeaderValueAsFloat( "rhr.rh_user25" );  // AP FOV
    selBoxSize[ 2 ] = this->GetHeaderValueAsFloat( "rhr.rh_user26" );  // SI FOV

}


/*!
 *  Gets the center of the acquisition grid.  May vary between sequences.
 */
void svkGEPFileMapperUCSFfidcsiDev0::GetCenterFromRawFile( double* center )
{

    center[0] = -1 * this->GetHeaderValueAsFloat( "rhi.ctr_R" );
    center[1] = -1 * this->GetHeaderValueAsFloat( "rhi.ctr_A" );
    center[2] = this->GetHeaderValueAsFloat( "rhi.ctr_S" );
}



/*!
 *  This method reads data from the pfile and puts the data into the CellData arrays.
 *  Non-uniform k-space sampling requires regridding to rectaliniear k-space array here. 
 */
void svkGEPFileMapperUCSFfidcsiDev0::ReadData(vtkStringArray* pFileNames, svkImageData* data)
{

    svkMrsImageData* tmpImageDynamic = svkMrsImageData::New();
    tmpImageDynamic->DeepCopy( data ); 

    int numTimePts = pFileNames->GetNumberOfValues(); 

    for ( int fileNumber = 0; fileNumber < numTimePts; fileNumber++ ) {      	 	 

        int timePt = fileNumber; 

        ostringstream progressStream;
        progressStream <<"Reading Time Point " << fileNumber + 1 << "/" << numTimePts ;
        this->SetProgressText( progressStream.str().c_str() );

        cout << "list raw files EPSI: " << pFileNames->GetValue(fileNumber) << endl;  	 	 
        vtkStringArray* tmpArray = vtkStringArray::New();
        tmpArray->InsertNextValue( pFileNames->GetValue(fileNumber) );
        this->LoadDataAcquisitionDescriptionFile( pFileNames->GetValue(fileNumber) );
        svkImageData* tmpImage = svkMrsImageData::New();
        tmpImage->DeepCopy( data ); 

        //  Reset the progress bar to local progress
        this->progress = (float)timePt/numTimePts; 
        this->UpdateProgress( this->progress );

        //  Do all the usual data loading stuff:
        this->Superclass::ReadData( tmpArray, tmpImage);  //new

        this->progress = (float)timePt/numTimePts; 
        this->UpdateProgress( this->progress );

        // Detect symmetric EPSI based on equal positive and negative gradient lobe lengths (stored in rhuser 13 and 16)
        if (this->GetHeaderValueAsInt( "rhr.rh_user13" ) == this->GetHeaderValueAsInt( "rhr.rh_user16" )) { 
            //  Separate out EPSI sampled data into time and k-space dimensions:
            this->ReorderEPSIData( tmpImage );
        }
        //data->DeepCopy( tmpImage ); 

        //  copy data to tmpImageDynamic and add appropriate arrays

        if ( fileNumber == 0 ) {

            //  First time around set up the target data struct
            tmpImageDynamic->DeepCopy(tmpImage); 
            svkImageData::RemoveArrays( tmpImageDynamic ); 

            //  ========================================
            //  Preallocate all arrays for data set.  
            //  Insert single time points at correct 
            //  location in AddReorderedTimePoint 
            //  ========================================
            svkDcmHeader* hdr = tmpImage->GetDcmHeader();
            svkDcmHeader::DimensionVector dimensionVector = hdr->GetDimensionIndexVector(); 
            int totalNumArrays = svkDcmHeader::GetNumberOfCells( &dimensionVector ); 
            totalNumArrays *= numTimePts;  

            string dataRepresentation = hdr->GetStringValue( "DataRepresentation" );
            int numComponents;
            if ( dataRepresentation == "COMPLEX" ) {
                   numComponents = 2;
            } else {
                   numComponents = 1;
            }
            int numFreqPts = hdr->GetIntValue( "DataPointColumns" );
            for (int arrayNumber = 0; arrayNumber < totalNumArrays; arrayNumber++) {
                vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
                dataArray->SetNumberOfComponents( numComponents );
                dataArray->SetNumberOfTuples( numFreqPts );
                tmpImageDynamic->GetCellData()->AddArray(dataArray);
                dataArray->Delete();
            }
            tmpImageDynamic->GetDcmHeader()->SetDimensionIndexSize(svkDcmHeader::TIME_INDEX, numTimePts - 1);
            //tmpImageDynamic->GetDcmHeader()->PrintDcmHeader();
        } 

        this->AddReorderedTimePoint(tmpImageDynamic, tmpImage, timePt, numTimePts); 

        tmpArray->Delete();
        tmpImage->Delete();
    }  	 


    data->DeepCopy( tmpImageDynamic ); 

    /*if ( this->GetDebug() ) {
        for (int fn=0; fn<3; fn++) {
        int numFreqPts = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );
        svkDcmHeader::DimensionVector ddv = data->GetDcmHeader()->GetDimensionIndexVector();
        svkDcmHeader::DimensionVector dli = ddv;  //copy for specific loop 
        cout << endl << "DDV 1 " << ddv.size() << endl;
        svkDcmHeader::PrintDimensionIndexVector(&ddv); 
        cout << "DLI 1 " <<  dli.size() << endl;
        svkDcmHeader::PrintDimensionIndexVector(&dli); 
        //  reset the new spatial dims:
        svk4DImageData::SetDimensionVectorValue(&dli, svkDcmHeader::COL_INDEX, 0); 
        svk4DImageData::SetDimensionVectorValue(&dli, svkDcmHeader::ROW_INDEX, 0); 
        svk4DImageData::SetDimensionVectorValue(&dli, svkDcmHeader::SLICE_INDEX, 0); 
        svk4DImageData::SetDimensionVectorValue(&dli, svkDcmHeader::TIME_INDEX, fn); 
        cout << endl << "DDV 2" << endl;
        svkDcmHeader::PrintDimensionIndexVector(&ddv); 
        cout << "DLI 2" << endl;
        svkDcmHeader::PrintDimensionIndexVector(&dli); 

        cout << endl << "DDV" << endl;
        svkDcmHeader::PrintDimensionIndexVector(&ddv); 
        cout << "DLI" << endl;
        svkDcmHeader::PrintDimensionIndexVector(&dli); 

        svk4DImageData::SetDimensionVectorValue(&dli, svkDcmHeader::EPSI_ACQ_INDEX, 0); 
        this->PrintSpecPts(data, numFreqPts, &ddv, &dli, "resampled lobe"); 

        svk4DImageData::SetDimensionVectorValue(&dli, svkDcmHeader::EPSI_ACQ_INDEX, 1); 
        this->PrintSpecPts(data, numFreqPts, &ddv, &dli, "resampled lobe"); 
        }
    }*/


    // now that the header and dimensionality are set correctly, reset this param:
    data->SyncVTKImageDataToDcmHeader(); 
    this->InitK0Sampled( data->GetDcmHeader() ); 

    tmpImageDynamic->Delete();

}


/*!
 *
 */
void svkGEPFileMapperUCSFfidcsiDev0::AddReorderedTimePoint(svkMrsImageData* dynamicImage, svkImageData* tmpImage, int timePt, int numTimePts)
{
    //dynamicImage->GetDcmHeader()->PrintDcmHeader(); 
    //tmpImage->GetDcmHeader()->PrintDcmHeader(); 

    svkDcmHeader* hdr = tmpImage->GetDcmHeader();
    string dataRepresentation = hdr->GetStringValue( "DataRepresentation" );
    int numComponents;
    if ( dataRepresentation == "COMPLEX" ) {
        numComponents = 2;
    } else {
        numComponents = 1;
    }

    int numFreqPts = hdr->GetIntValue( "DataPointColumns" );

    //  loop over all xyz, coil, time points
    int voxels[3]; 
    voxels[0] = hdr->GetIntValue( "Columns" );
    voxels[1] = hdr->GetIntValue( "Rows" );
    voxels[2] = hdr->GetNumberOfSlices();

    //  Get the values from the single time point image (tmpImage): 
    svkDcmHeader::DimensionVector dimensionVector = hdr->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector indexVector = dimensionVector; 

    svkDcmHeader::DimensionVector dynamicDimensionVector = dynamicImage->GetDcmHeader()->GetDimensionIndexVector();  
    svkDcmHeader::DimensionVector dynamicIndexVector = dynamicDimensionVector; 

    //  GetNumber of cells in non-dynamic image:
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector ); 
     
    for (int cellID = 0; cellID < numCells; cellID++ ) { 

        //  Get the dimensionVector index for the non-dynamic image: 
        svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimensionVector, &indexVector, cellID ); 

        //  Take those indices and insert them into the target dynamicIndexVector together with the
        //  current time point: 
        svkDcmHeader::SetDimensionVectorValue( &dynamicIndexVector, svkDcmHeader::COL_INDEX, 
                hdr->GetDimensionVectorValue( &indexVector, svkDcmHeader::COL_INDEX ) ); 

        svkDcmHeader::SetDimensionVectorValue( &dynamicIndexVector, svkDcmHeader::ROW_INDEX, 
                hdr->GetDimensionVectorValue( &indexVector, svkDcmHeader::ROW_INDEX ) ); 

        svkDcmHeader::SetDimensionVectorValue( &dynamicIndexVector, svkDcmHeader::SLICE_INDEX, 
                hdr->GetDimensionVectorValue( &indexVector, svkDcmHeader::SLICE_INDEX ) ); 

        svkDcmHeader::SetDimensionVectorValue( &dynamicIndexVector, svkDcmHeader::CHANNEL_INDEX, 
                hdr->GetDimensionVectorValue( &indexVector, svkDcmHeader::CHANNEL_INDEX ) ); 

        svkDcmHeader::SetDimensionVectorValue( &dynamicIndexVector, svkDcmHeader::EPSI_ACQ_INDEX, 
                hdr->GetDimensionVectorValue( &indexVector, svkDcmHeader::EPSI_ACQ_INDEX) ); 

        svkDcmHeader::SetDimensionVectorValue( &dynamicIndexVector, svkDcmHeader::TIME_INDEX, timePt); 

        //  Get index of array in target dynamic image. 
        int targetCellIndex =  svkDcmHeader::GetCellIDFromDimensionVectorIndex( &dynamicDimensionVector, &dynamicIndexVector); 

        //  Get array from target dynamic image: 
        vtkDataArray* targetDataArray = dynamicImage->GetCellData()->GetArray(targetCellIndex);

        //  Set it's array name from the current target DimensionVector  
        //cout << "DIM" << endl;
        //svkDcmHeader::PrintDimensionIndexVector(&dynamicDimensionVector);
        //cout << "INDEX" << endl;
        //svkDcmHeader::PrintDimensionIndexVector(&dynamicIndexVector);
        dynamicImage->SetArrayName( targetDataArray, &dynamicIndexVector ); 
    
        //  Now get the array from the single time point input image and map it onto the
        //  array in the target image: 
        vtkDataArray* tmpDataArray = tmpImage->GetCellData()->GetArray( cellID );

        double* tmpTuple; 
        float dynamicTuple[2]; 
        for (int i = 0; i < numFreqPts; i++) {
            tmpTuple = tmpDataArray->GetTuple(i); 
            dynamicTuple[0] = tmpTuple[0]; 
            dynamicTuple[1] = tmpTuple[1]; 
            targetDataArray->SetTuple( i, dynamicTuple );
        }    

    }

    int currentNumTimePts = timePt + 1; 

    //  This is the original origin based on the reduced dimensionality in the EPSI direction
    //  the dynamicIndexVector should contain the current dimensionality limits for the 
    //  dynamic image with the new time point added: 
    hdr = dynamicImage->GetDcmHeader();
    double origin[3];
    hdr->GetOrigin( origin, 0 );

    double voxelSpacing[3];
    hdr->GetPixelSpacing( voxelSpacing );

    double dcos[3][3];
    hdr->GetDataDcos( dcos );
    
    hdr->InitPerFrameFunctionalGroupSequence(
        origin,
        voxelSpacing,
        dcos,
        &dynamicDimensionVector 
    );

    dynamicImage->SyncVTKImageDataToDcmHeader(); 
}


/*!
 *  Reorder the data.  Input EPSI data has one dimension with both time (spectral) and k-space
 *  content.  The time domain data from each k-space point needs to be extracted and put into 
 *  a cell array.  This is done separately for positive and negative lobes.  So, for example, 
 *  a 1380 x 1 x 12 x 10 data set with 60 lobes (23 k-space samples) would get reordered to a
 *  two 30 x 23 x 12 x 10 data sets. Each will be encoded into a separate "channel" indicated
 *  by the dimension index sequence.  this is just a temporarary internal layout.
 */
void svkGEPFileMapperUCSFfidcsiDev0::ReorderEPSIData( svkImageData* data )
{
    //  Reorder data.  This requires adding data arrays 
    //  to accommodate the spectra at the new k-space points 
    //  points extracted from the EPSI encoding 

    //  Preallocate data arrays. We can't override the original arrays, 
    //  until the reordering is complette, so use these as the 
    //  target arrays. 

    //  Allocate arrays for all How many k-space points were acquired in all?

    //  Allocate arrays for spectra at each phase encode:
    svkDcmHeader* hdr = data->GetDcmHeader();
    string dataRepresentation = hdr->GetStringValue( "DataRepresentation" );

    int numComponents;
    if ( dataRepresentation == "COMPLEX" ) {
        numComponents = 2;
    } else {
        numComponents = 1;
    }

    //===========
    //  Reset hdr value (DataPointColumns)
    //===========
    int numEPSIPts= hdr->GetIntValue( "DataPointColumns" );
    //cout << "Num EPSI Pts: "<< numEPSIPts << endl;

    //data->GetDcmHeader()->PrintDcmHeader(); 
    data->GetDcmHeader()->GetDimensionIndexVector(); 
    //====================================================
    //use tmp
    svkMrsImageData* tmpReorderData = svkMrsImageData::New();
    tmpReorderData->DeepCopy( data ); 

    svkEPSIReorder* reorder = svkEPSIReorder::New();    
    reorder->SetInputData( tmpReorderData );

    EPSIType epsiType = SYMMETRIC;
    if( this->dadFile != NULL && this->dadFile->GetEPSIType() != UNDEFINED_EPSI_TYPE ) {
        epsiType = this->dadFile->GetEPSIType();
    }
    reorder->SetEPSIType( epsiType );

    //  between lobes, throw out the last and first point before resuming sampling
    //  These are the zero crossings in symmetric EPSI. 
    cout << "reorder: NumSamplesToSkip: " << 2 << endl;
    reorder->SetNumSamplesToSkip( 2 ); 


    //============================================================
    //  Read EPSI acquisition params from raw header(legacy) or DAD file: 
    //============================================================

    //================================================
    //  this is the number of lobes in the EPSI sampling. 
    //  For symmetric epsi this is twice the number of 
    //  frequence points (pos + neg)    
    //================================================
    int numLobes; ; 
    if ( this->dadFile != NULL ) {
        //  DAD 
        int numLobesOdd  = this->dadFile->GetIntWithPath("/encoding/trajectoryDescription/epsiEncoding/numberOfLobesOdd");
        int numLobesEven = this->dadFile->GetIntWithPath("/encoding/trajectoryDescription/epsiEncoding/numberOfLobesEven");
        numLobes = numLobesOdd + numLobesEven; 
    } else {
        numLobes = this->GetHeaderValueAsInt( "rhr.rh_user10");
    }
    cout << "reorder: NumEPSILobes: " <<  numLobes << endl;
    reorder->SetNumEPSILobes( numLobes ); 


    //================================================
    //  dwell time time between k-space points
    //================================================
    int sampleSpacingTimeMs; 
    if ( this->dadFile != NULL ) {
        //  DAD 
        sampleSpacingTimeMs = 1000 * this->dadFile->GetFloatWithPath(
            "/encoding/trajectoryDescription/epsiEncoding/sampleSpacingTimeMs");
    } else {
        //  Legacy
        sampleSpacingTimeMs = this->GetHeaderValueAsInt( "rhr.rh_user12" );
    }    
    cout << "DELTA T: " << sampleSpacingTimeMs << endl;


    //================================================
    //  time for plateau encoding (gradient duratation)
    //================================================
    int plateauTime;
    if ( this->dadFile != NULL ) {
        //  DAD    
        plateauTime = 1000 * this->dadFile->GetFloatWithPath(
            "/encoding/trajectoryDescription/epsiEncoding/plateauDurationOddMs");
    } else {
        //  Legacy
        plateauTime = this->GetHeaderValueAsInt( "rhr.rh_user13" );
    }
    cout << "plateauTime: " << plateauTime << endl;

    
    //================================================
    //  time for ramp in one direction
    //================================================
    int rampDuration;
    if ( this->dadFile != NULL ) {
        //  DAD    
        rampDuration = 1000 * this->dadFile->GetFloatWithPath(
            "/encoding/trajectoryDescription/epsiEncoding/rampDurationOddMs");
    } else {
        //  Legacy 
        rampDuration = this->GetHeaderValueAsInt( "rhr.rh_user15" );
    }
    cout << "ramp duration: " << rampDuration << endl;
    

    //  number of samples per lobe (ramps + plateau)  
    //  num spectral samples in FID is num time_pts / this value (
    cout << "reorder: NumSamplesPerLobe: " <<  ( plateauTime + 2 * rampDuration) / sampleSpacingTimeMs  << endl;
    reorder->SetNumSamplesPerLobe( ( plateauTime + 2 * rampDuration) / sampleSpacingTimeMs );


    //================================================
    //  Number of samples at start.  add 1 for the 
    //  zero crossing
    //================================================
    int firstSampleOffset; 
    if ( this->dadFile != NULL ) {
        //  DAD    
        float acquisitionDelayTimeMs = 1000 * this->dadFile->GetFloatWithPath(
            "/encoding/trajectoryDescription/epsiEncoding/acquisitionDelayTimeMs");
        firstSampleOffset = acquisitionDelayTimeMs / sampleSpacingTimeMs; 
    } else {
        //  Legacy 
        firstSampleOffset = this->GetHeaderValueAsInt( "rhr.rh_user22" ); 
    }
    cout << "reorder: FirstSample: " <<  firstSampleOffset + 1 << endl;
    reorder->SetFirstSample( firstSampleOffset + 1 ); 


    //================================================
    //  EPSI Axis defines which axis is epsi 
    //  encoded.  Swap the value of 
    //  epsi k-space encodes into this field: 
    //================================================
    int epsiAxis_init = this->GetEPSIAxis();
    cout << "EPSI AXIS: " << epsiAxis_init<< endl;
    

    //  Swap X/Y dimensions if swap on and if epsi is on X or Y
    if ( this->IsSwapOn()  && epsiAxis_init == 0) {
        epsiAxis_init = 1;
    } else if ( this->IsSwapOn()  && epsiAxis_init == 1) {
        epsiAxis_init = 0;
    }

    reorder->SetEPSIAxis( static_cast<svkEPSIReorder::EPSIAxis>(epsiAxis_init)  );


    //  set the original input dimensionality:
    int numEPSIVoxels[3]; 
    this->GetNumVoxels( numEPSIVoxels ); 
    tmpReorderData->GetDcmHeader()->GetDimensionIndexVector(); 
    reorder->Update();

    //  =================================================
    //  Apply linear phase correction to correct for EPSI sampling of 
    //  spectra: 
    //      foreach k-space point, apply a phase correction to each complex spectral point
    //          the correction is a function of the distance along the epsi-axis (k/t) with pivots 
    //          at the center of each dimension
    //  =================================================
    int numVoxels[3]; 
    this->GetNumVoxels( numVoxels ); 
    int numSamplesPerLobe = reorder->GetNumSamplesPerLobe();
    int epsiAxis = static_cast<int>( reorder->GetEPSIAxis() );
    int numFreqPts = reorder->GetNumEPSIFrequencyPoints(); 

    //reorder->GetOutput()->GetDcmHeader()->PrintDcmHeader(); 
    //reorder->GetOutput()->GetDcmHeader()->GetDimensionIndexVector(); 

    data->DeepCopy( reorder->GetOutput() ); 

    if ( this->GetDebug() ) {
        svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
        svkDcmHeader::DimensionVector loopIndex = dimensionVector;  //copy for specific loop 
        //  reset the new spatial dims:
        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::COL_INDEX, 1); 
        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::ROW_INDEX, 0); 
        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::SLICE_INDEX, 0); 

        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::EPSI_ACQ_INDEX, 0); 
        this->PrintSpecPts(data, numFreqPts, &dimensionVector, &loopIndex, "before phase lobe"); 

        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::EPSI_ACQ_INDEX, 1); 
        this->PrintSpecPts(data, numFreqPts, &dimensionVector, &loopIndex, "before phase lobe"); 
    }

    //data->GetDcmHeader()->PrintDcmHeader(); 
    //data->GetDcmHeader()->GetDimensionIndexVector(); 

    this->EPSIPhaseCorrection( data, numVoxels, numSamplesPerLobe, epsiAxis);  

    //data->GetDcmHeader()->PrintDcmHeader(); 
    //data->GetDcmHeader()->GetDimensionIndexVector(); 

    //  EPSIReorder tries to automatically set the sweepwidth, but reset it here to the know value
    data->GetDcmHeader()->SetValue(
        "SpectralWidth",
        581
    );
 
    //  =================================================
    //  Reverse first, third, etc lobe k-space spectra along 
    //  epsi axis.. Initial graidient lobe is negative and 
    //  requires flipping of every other lobe. 
    //  =================================================

    if ( this->GetDebug() ) {
        svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
        svkDcmHeader::DimensionVector loopIndex = dimensionVector;  //copy for specific loop 
        //  reset the new spatial dims:
        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::COL_INDEX, 1); 
        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::ROW_INDEX, 0); 
        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::SLICE_INDEX, 0); 

        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::EPSI_ACQ_INDEX, 0); 
        this->PrintSpecPts(data, numFreqPts, &dimensionVector, &loopIndex, "before flip lobe"); 

        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::EPSI_ACQ_INDEX, 1); 
        this->PrintSpecPts(data, numFreqPts, &dimensionVector, &loopIndex, "before flip lobe"); 
    }

    //  flip first lobe along epsiAxis
    //  first gradient is negative in this sequence:
    //  This should get encoded in the DAD file
    map < string, void* >::iterator  it;
    it = this->inputArgs.find( "epsiFlipLobe" );
    int lobeToFlip = 0;
    if ( it != this->inputArgs.end() ) {
        lobeToFlip = *(static_cast<int*>( ( this->inputArgs[ it->first ] )));
    }
    
    if ( lobeToFlip == 0 ) {
        float pfileVersion = this->GetHeaderValueAsFloat( "rhr.rh_rdbm_rev" );
        if ( pfileVersion < 25 ) {
            lobeToFlip = 1;
        } else {
            lobeToFlip = 2;
        }

    }
    cout << "SVKGEPFileMapperUCSFfidcsiDev0: flip lobe: " << lobeToFlip << endl;
    
    this->FlipAxis( data, epsiAxis, lobeToFlip - 1);     //original

    //data->GetDcmHeader()->PrintDcmHeader(); 
    //data->GetDcmHeader()->GetDimensionIndexVector(); 

    if ( this->GetDebug() ) {
        svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
        svkDcmHeader::DimensionVector loopIndex = dimensionVector;  //copy for specific loop 
        //  reset the new spatial dims:
        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::COL_INDEX, 0); 
        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::ROW_INDEX, 0); 
        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::SLICE_INDEX, 0); 

        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::EPSI_ACQ_INDEX, 0); 
        this->PrintSpecPts(data, numFreqPts, &dimensionVector, &loopIndex, "before resampled lobe"); 

        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::EPSI_ACQ_INDEX, 1); 
        this->PrintSpecPts(data, numFreqPts, &dimensionVector, &loopIndex, "before resampled lobe"); 
    }

    //  =================================================
    //  resample ramp data 
    //  =================================================

    this->ResampleRamps( data, sampleSpacingTimeMs, plateauTime, rampDuration, epsiAxis ); 

    //  This results in a match to Matlab when comparing the k-space data output:     
    //  final_datap(:,:,:,:,t) = fftshift(fftn(kdatap));
    //  final_datan(:,:,:,:,t) = fftshift(fftn(kdatan));
    if ( this->GetDebug() ) {
        svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
        svkDcmHeader::DimensionVector loopIndex = dimensionVector;  //copy for specific loop 
        //  reset the new spatial dims:
        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::COL_INDEX, 1); 
        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::ROW_INDEX, 0); 
        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::SLICE_INDEX, 0); 

        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::EPSI_ACQ_INDEX, 0); 
        this->PrintSpecPts(data, numFreqPts, &dimensionVector, &loopIndex, "resampled lobe"); 

        svkDcmHeader::SetDimensionVectorValue(&loopIndex, svkDcmHeader::EPSI_ACQ_INDEX, 1); 
        this->PrintSpecPts(data, numFreqPts, &dimensionVector, &loopIndex, "resampled lobe"); 
    }

    //  =================================================
    //  FFTShift:  put the origin of k-space at the image
    //  center so it's suitable for standard downstream 
    //  FFT.  Seems to be the case already
    //  =================================================
    //this->FFTShift( data ); 
   
    //  =================================================
    //  combine even/odd lobes (separate post-processing 
    //  step). 
    //  =================================================

    svkMrsImageData* tmpData = svkMrsImageData::New();
    tmpData->DeepCopy( data );
    bool sumOfSquares = false;     
    svkEPSIReorder::CombineLobes( tmpData, sumOfSquares);
    data->DeepCopy( tmpData );
    tmpData->Delete();


    //  =================================================
    //  Account for patient entry.. need to review, not sure this
    //   is necessary after the axis flips above. 
    //  =================================================
    this->ModifyForPatientEntry(data); 

    this->InitK0Sampled( data->GetDcmHeader() );

    tmpReorderData->Delete();
    reorder->Delete(); 

}


/*!
 *  Use DAD or legacy userCV to get epsi gradient axis: 
 */
int svkGEPFileMapperUCSFfidcsiDev0::GetEPSIAxis( ) 
{
    int epsiAxis;
    if ( this->dadFile != NULL ) {
        //  DAD
        string epsiGradientAxis = this->dadFile->GetDataWithPath(
            "/encoding/trajectoryDescription/epsiEncoding/gradientAxis");
        if ( epsiGradientAxis.compare("dim1") == 0 ) {
            epsiAxis = 0; 
        } else if ( epsiGradientAxis.compare("dim2") == 0 ) {
            epsiAxis = 1; 
        } else if ( epsiGradientAxis.compare("dim3")  == 0 ) {
            epsiAxis = 2; 
        }
    } else {
        //  Legacy 
        epsiAxis = this->GetHeaderValueAsInt( "rhr.rh_user20" ) - 1 ;
    }
    return epsiAxis; 
}


/*  
 *  util method for debugging
 */
void svkGEPFileMapperUCSFfidcsiDev0::PrintSpecPts( svkImageData* data, int numFreqPts, svkDcmHeader::DimensionVector* dimensionVector, svkDcmHeader::DimensionVector* loopIndex, string comment )
{
    return; 
    //svkDcmHeader::PrintDimensionIndexVector( dimensionVector);
    //svkDcmHeader::PrintDimensionIndexVector(loopIndex);
    int cellIndex = svkDcmHeader::GetCellIDFromDimensionVectorIndex( dimensionVector, loopIndex ); 

    cout.setf(ios::fixed, ios::floatfield);
    cout.setf(ios::showpoint);

    vtkFloatArray* specLobe = vtkFloatArray::SafeDownCast(svkMrsImageData::SafeDownCast(data)->GetSpectrum( cellIndex )); 
    for (int p=0; p<numFreqPts; p++ ) {
        int epsiAcqNum = svkDcmHeader::GetDimensionVectorValue(loopIndex, svkDcmHeader::EPSI_ACQ_INDEX);
        cout << comment << " " << epsiAcqNum << ": " << setw(14) << setprecision(2) << specLobe->GetValue(p*2) << " " << setw(14) << setprecision(2) << specLobe->GetValue(p*2+1) << endl;
    }
    cout << endl;
}


/*!
 *  apply linear phase correction to k-space points along epsi-axis:
 *  this would be easier if I actually redimension the metadata and set the new arrays 
 */
void svkGEPFileMapperUCSFfidcsiDev0::EPSIPhaseCorrection( svkImageData* data, int* numVoxels, int numRead, int epsiAxis)
{

    svkMrsImageData* tmpData = svkMrsImageData::New();
    tmpData->DeepCopy( data ); 

    svkEPSIPhaseCorrect* epsiPhase = svkEPSIPhaseCorrect::New();
    if ( this->GetDebug() ) {
        cout << this->GetClassName() << "::epsiPhaseCorrection() " << endl; 
        cout << "   SetNumEPSIkread: " << numRead  << endl; 
        cout << "   SetEPSIAxis:     " << epsiAxis << endl; 
    }
    //  the first and last point have been thrown out
    numRead -= 2; 
    epsiPhase->SetNumEPSIkRead( numRead );
    epsiPhase->SetEPSIAxis( epsiAxis );
    epsiPhase->SetEPSIType( SYMMETRIC );
    epsiPhase->SetInputData( tmpData ); 
    //tmpData->GetDcmHeader()->PrintDcmHeader(); 
    epsiPhase->Update(); 

    data->DeepCopy( epsiPhase->GetOutput() ); 

    epsiPhase->Delete(); 
    tmpData->Delete(); 

}


/*!
 *  Reverses the specified axis and lobe.   
 *  lobe shoud be 0 or 1 depending on the starting polarity of the first lobe
 */
void svkGEPFileMapperUCSFfidcsiDev0::FlipAxis( svkImageData* data, int axis, int lobe) 
{

    //  Set the algorithm to only flip the odd lobes: 
    svkDcmHeader::DimensionVector filterDimVector = data->GetDcmHeader()->GetDimensionIndexVector(); 
    //  set all indices to -1 except for EPSI_ACQ_INDEX which is set to 1.  The index must match "1" in this dimension 
    //  otherwise the flip algorith will skip that volume
    for ( int i = 0; i < filterDimVector.size(); i++ ) {
        svkDcmHeader::SetDimensionVectorValue(&filterDimVector, i, -1);
    }
    svkDcmHeader::SetDimensionVectorValue(&filterDimVector, svkDcmHeader::EPSI_ACQ_INDEX, lobe ); 


    svkMrsImageData* tmpData = svkMrsImageData::New();
    tmpData->DeepCopy( data ); 

    svkMrsImageFlip* flip = svkMrsImageFlip::New(); 
    flip->SetFilteredAxis( axis ); 
    flip->SetFilterDomainIndices( &filterDimVector ); 
    //flip->SetFilteredChannel( lobe ); 

    flip->SetInputData( tmpData ); 
    flip->Update(); 

    data->DeepCopy( flip->GetOutput() ); 

    flip->Delete(); 
    tmpData->Delete(); 
}


/*!
 *  Resamples ramp portion of symmetric EPSI acquisition, regridding it 
 *  to a uniform cartesian grid. 
 *
 *  References: 
 *  
 *      John I. Jackson, Craig H. Meyer, Dwight G. Nishimura and Albert Macovski
 *      Selection of a Convolution Function for Fourier Inversion Using Gridding
 *      IEEE TRANSACTIONS ON MEDICAL IMAGING. VOL. IO. NO. 3 , SEPTEMBER 1991
 *
 *      Philip J. Beatty, Dwight G. Nishimura, John M. Pauly,
 *      Rapid Gridding Reconstruction With a Minimal Oversampling Ratio
 *      IEEE TRANSACTIONS ON MEDICAL IMAGING, VOL. 24, NO. 6, JUNE 2005
 */
void svkGEPFileMapperUCSFfidcsiDev0::ResampleRamps( svkImageData* data, int sampleSpacingTimeMs, int plateauTime, int rampDuration, int epsiAxis )
{

    int numLobes = 2; //for symmetric epsi pos/neg gradient

    //  get the EPSI sampling waveform: 
    svkDcmHeader* hdr = data->GetDcmHeader(); 
    int numVoxels[3];  
    numVoxels[0] = hdr->GetIntValue( "Columns" );
    numVoxels[1] = hdr->GetIntValue( "Rows" );
    numVoxels[2] = hdr->GetNumberOfSlices();

    int numEPSIVoxels = numVoxels[ epsiAxis ]; 

    float* waveFormIntegral = new float[ numEPSIVoxels ]; 
    this->GetWaveFormIntegral( waveFormIntegral, sampleSpacingTimeMs, plateauTime, rampDuration ); 

    //  =============================================
    //  MATLAB translation: 
    //  ks = waveFormIntegral  (21 points)
    //  ks_max = integralMaxFloat
    //  ks_norm =  waveFormIntegralNorm
    //  =============================================

    //  Max should be last point in cummulative integral: 
    float integralMaxFloat = waveFormIntegral[ numEPSIVoxels - 1 ]; 
    int integralMax = static_cast<int>( floor(  2 * integralMaxFloat ) ); 
    int overGridFactor = 2; 
    int gridSize = overGridFactor * integralMax; 
    float dkn = ( 2 * integralMaxFloat ) / gridSize; 
    float* kn     = new float[ gridSize ]; 
    float* knNorm = new float[ gridSize ]; 
    for ( int i = 0; i < gridSize; i++ ) {
        kn[i]     = ( (i + 0.5) - gridSize/2 ) * dkn; 
        knNorm[i] = kn[i] / dkn; 
        //cout << "knn: " << knNorm[i] << endl;
    }

    float* waveFormIntegralNorm = new float[ numEPSIVoxels ]; 
    for ( int i = 0; i < numEPSIVoxels; i++ ) {
        waveFormIntegralNorm[i] =  waveFormIntegral[i] / dkn; 
        //cout << "wfin: " << waveFormIntegralNorm[i] << endl;
    }

    //  =============================================
    //  Calculate density compensation factors
    //  =============================================
    float* waveFormTmp = new float[ numEPSIVoxels + 2]; 

    waveFormTmp[0] = 2 * waveFormIntegral[0] - waveFormIntegral[1]; 
    for ( int i = 1; i < numEPSIVoxels + 1; i++ ) {
        waveFormTmp[i] = waveFormIntegral[i - 1];
    }
    waveFormTmp[numEPSIVoxels +1] = 2 * waveFormIntegral[ numEPSIVoxels-1 ] - waveFormIntegral[ numEPSIVoxels -2 ]; 
    
    float* densityCompensationFactors = new float[ numEPSIVoxels ]; 
    for ( int i = 0; i < numEPSIVoxels; i++ ) {
        densityCompensationFactors[i] = waveFormTmp[i+2] - waveFormTmp[i];  
        //cout << "DCF: " << densityCompensationFactors[i] << endl;
    }


    //  =============================================
    //  Calculate   Kaise-Bessel kernel params:  
    //  Equation 5:
    //
    //      alpha = overGridFactor
    //      W     = kWidth 
    //
    //  Philip J. Beatty, Dwight G. Nishimura, John M. Pauly,
    //  Rapid Gridding Reconstruction With a Minimal Oversampling Ratio
    //  IEEE TRANSACTIONS ON MEDICAL IMAGING, VOL. 24, NO. 6, JUNE 2005
    //  =============================================
    float kWidth = 3.5; 
    float beta = ( kWidth * kWidth ) / ( overGridFactor * overGridFactor ) ; 
    beta *= ( overGridFactor - 0.5 ) * ( overGridFactor - 0.5 ); 
    beta -= 0.8 ; 
    beta = static_cast<float>( pow( static_cast<double>(beta), 0.5) ); 
    beta *= vtkMath::Pi(); 
    
   
    //  =============================================
    //  Make Gridding Matrix: 
    //  =============================================

    //  allocate and initialize to zero
    float** grid = new float*[gridSize];    //  gridSize rows
    for (int i = 0; i < gridSize; i++ ) {
        grid[i] = new float [numEPSIVoxels];   //   numEPSIVoxels columns 
        for ( int j = 0; j < numEPSIVoxels; j++) {
            grid[i][j] = 0;      
        }
    }

    float* dk = new float[ gridSize ]; 
    for ( int i = 0; i < numEPSIVoxels; i++ ) {

        std::vector <int> iGrid; 
        std::vector <float> iGridDK; 

        for ( int j = 0; j < gridSize; j++ ) {
            dk[j] = fabs( knNorm[j] - waveFormIntegralNorm[i] );  
            if ( dk[j] < (kWidth/2) ) {
                iGrid.push_back( j ); 
                iGridDK.push_back( dk[j] ); 
            }
        }

        //for (int k = 0; k < iGrid.size(); k++ ) {
            //cout << "IGRID: " << iGrid[k] << endl;
            //cout << "IGRID: " << iGridDK[k] << endl;
        //}
        //cout << endl; 


        //  get Kaiser Bessel Values: 
        std::vector<float> kbVals; 
        this->GetKaiserBesselValues( &iGridDK , kWidth, beta, &kbVals); 

        // Initialize non-zero grid elements:  
        //cout << "DCF: " << densityCompensationFactors[i] << endl; 
        int kbIndex = 0; 
        for (int m = iGrid[0]; m <= iGrid[iGrid.size() - 1]; m++ ) {
            //cout << "Row, Col: " << m << " " << i << endl;
            //cout << "grid: " << kbVals[kbIndex] * densityCompensationFactors[i] << endl; 
            grid[m][i] = kbVals[kbIndex] * densityCompensationFactors[i]; 
            kbIndex++; 
        }

    }


    //  ================================================
    //  Initialize the rolloff correction: Eq 15 Jackson
    //  ================================================
    float* apodCor = new float[gridSize]; 
    this->GetRolloffCorrection( gridSize, kWidth, beta, apodCor ); 


    //  ================================================
    //  Convolve, Iterate over the nonEPSI dimensions: 
    //  ================================================
    int numSpecPts = hdr->GetIntValue( "DataPointColumns" );
    int regridDims[3]; 
    regridDims[0] = numVoxels[0]; 
    regridDims[1] = numVoxels[1]; 
    regridDims[2] = numVoxels[2]; 
    // loop over all dims other than the epsi k-space dimension: 
    regridDims[epsiAxis] = 1;  

    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector(); 
    svkDcmHeader::DimensionVector loopVector = dimensionVector;  

    int numChannels = svkDcmHeader::GetDimensionVectorValue( &dimensionVector, svkDcmHeader::CHANNEL_INDEX) + 1;


    int timePt = 0; // always 0 for this methods, only handles one time point at a time
    float* epsiKData0 = new float[ numEPSIVoxels * 2 ];
    float* overgrid = new float[ gridSize *2 ];
    for ( int channel = 0; channel < numChannels; channel++) {
    for ( int lobe = 0; lobe < numLobes; lobe++) {
        //cout << "LOBE: " << lobe << endl;
        for ( int slice = 0; slice < regridDims[2]; slice++) {
            for ( int row = 0; row < regridDims[1]; row++) {
                for ( int col = 0; col < regridDims[0]; col++) {
                    for ( int freq = 0; freq < numSpecPts; freq++) {

                        for ( int epsiK = 0; epsiK < numVoxels[epsiAxis]; epsiK++ ) {

                            if ( epsiAxis == 0 ) { 
                                col = epsiK;         
                            } else if ( epsiAxis == 1 ) {
                                row = epsiK;         
                            } else if ( epsiAxis == 2 ) {
                                slice = epsiK;         
                            }

                            //  set the index values for this specific loop
                            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::COL_INDEX, col); 
                            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::ROW_INDEX, row); 
                            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::SLICE_INDEX, slice); 
                            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::CHANNEL_INDEX, channel); 
                            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::EPSI_ACQ_INDEX, lobe); 
                            int cellIndex = svkDcmHeader::GetCellIDFromDimensionVectorIndex( &dimensionVector, &loopVector); 

                            vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( 
                                svkMrsImageData::SafeDownCast(data)->GetSpectrum( cellIndex ) 
                            );

                            //  epsiKData0 is equivalent to kte3 and kto3 in Matlab implementation
                            epsiKData0[ epsiK * 2 ]     = spectrum->GetValue( freq * 2 ); 
                            epsiKData0[ epsiK * 2 + 1 ] = spectrum->GetValue( freq * 2 + 1 ); 
                            //cout << "Ramp Spectrum: " << epsiKData0[epsiK * 2 ] << " " << epsiKData0[epsiK * 2 +1]   << endl;
                            
                        }

                        // ========================================================
                        //  Calculate the overgrid vector here for each epsi row
                        //  fn_overgrid in MATLAB
                        // ========================================================
                        for ( int i = 0; i < gridSize; i++ ) {
                            overgrid[ i*2 ] = 0;  
                            overgrid[ i*2 + 1 ] = 0;  
                            for ( int j = 0; j < numEPSIVoxels; j++) {
                                overgrid[ i*2 ]    += grid[i][j] * epsiKData0[ j * 2 ];
                                overgrid[ i*2 + 1] += grid[i][j] * epsiKData0[ j * 2 + 1];
                            }
                        }

                        //for ( int i = 0; i < gridSize; i++ ) {
                        //    cout << "overgrid : " << overgrid[i* 2 ] << " " << overgrid[i* 2 +1]   << endl;
                        //}

                        //================================================
                        //  ifft overgrid and divide by apodCor
                        //      -> convert to vtkImageComplex
                        //      ->  ifftShift (k=0 to origin)
                        //      ->  ifft   (k to spatial domain) 
                        //      ->  fftShift ( x=0 to middle)
                        //================================================
                        int offset = static_cast<int>(floor(integralMax/2.)); 
                        vtkImageComplex* epsiKData = new vtkImageComplex[ gridSize + offset];
                        vtkImageComplex* epsiXData = new vtkImageComplex[ gridSize + offset];
                        for( int i = 0; i < gridSize; i++ ) {
                            epsiKData[i].Real = overgrid[i*2];
                            epsiKData[i].Imag = overgrid[i*2+1];
                        }

                        svkMrsImageFFT::IFFTShift( epsiKData, gridSize );
                        vtkImageFourierFilter* rfft = vtkImageRFFT::New();
                        rfft->ExecuteRfft( epsiKData, epsiXData, gridSize );
                        svkMrsImageFFT::FFTShift( epsiXData, gridSize );

                        //  epsiXData should correspond to Fn_overgrid in 
                        //  Matlab implementation
                        for( int i = 0; i < gridSize; i++ ) {
                            epsiXData[i].Real = epsiXData[i].Real / apodCor[i]; 
                            epsiXData[i].Imag = epsiXData[i].Imag / apodCor[i]; 
                        }
                        for( int i = 0; i < gridSize; i++ ) {
                            epsiXData[i].Real = epsiXData[i + offset].Real; 
                            epsiXData[i].Imag = epsiXData[i + offset].Imag; 
                            //  TODO:  the values of epsiXData above index 26 do not seem to be correct but also 
                            //  aren't used  
                            //cout << "EXD: " << i << " " << epsiXData[i].Real << " " << apodCor[i] << endl;
                        }

                        //================================================
                        //  ifftshift (move x = 0 back to origin), 
                        //  fft (x back to kspace)
                        //  fftshift (k=0 back to middle)
                        //================================================
                        svkMrsImageFFT::IFFTShift( epsiXData, integralMax);

                        vtkImageFourierFilter* fft = vtkImageFFT::New();
                        fft->ExecuteFft( epsiXData, epsiKData, integralMax);

                        svkMrsImageFFT::FFTShift( epsiKData, integralMax);

                        //  Set the Gridded k-space data back into the data set: 
                        float tuple[2]; 
                        for ( int k = 0; k < integralMax; k++ ) {

                            if ( epsiAxis == 0 ) { 
                                col = k;         
                            } else if ( epsiAxis == 1 ) {
                                row = k;         
                            } else if ( epsiAxis == 2 ) {
                                slice = k;         
                            }

                            //  set the index values for this specific loop
                            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::COL_INDEX, col); 
                            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::ROW_INDEX, row); 
                            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::SLICE_INDEX, slice); 
                            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::CHANNEL_INDEX, channel); 
                            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::EPSI_ACQ_INDEX, lobe); 
                            int cellIndex = svkDcmHeader::GetCellIDFromDimensionVectorIndex( &dimensionVector, &loopVector); 

                            vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( 
                                svkMrsImageData::SafeDownCast(data)->GetSpectrum( cellIndex ) 
                            );

                            tuple[0] = epsiKData[k].Real; 
                            tuple[1] = epsiKData[k].Imag; 
                            spectrum->SetTuple( freq, tuple );
                           
                            //  spectrum/tuple corresponds to gridkb_1d.m, fn in Matlab implementation 
                            //  up to here looks OK compared with Matlab
                            //  dbg: 
                            //  cout << "regridded tupple: " << tuple[0] << " + " << tuple[1] << endl;
                            //vtkFloatArray* testspectrum = vtkFloatArray::SafeDownCast( 
                                //svkMrsImageData::SafeDownCast(data)->GetSpectrum( col, row, slice, 0, lobe) ); 
                            //cout << "regridded spectrum check tuple: " 
                                //<< testspectrum->GetValue( freq * 2 ) << " " << testspectrum->GetValue( freq * 2 + 1 ) << endl;
                        }

                        rfft->Delete();
                        fft->Delete();
                        delete [] epsiXData; 
                        delete [] epsiKData; 
                    }
                }
            }
        }
    }
    }

    //  dbg: 
    //vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( svkMrsImageData::SafeDownCast(data)->GetSpectrum( 0, 0, 0, 0, 0) ); 
    //for (int xx = 0; xx<numSpecPts; xx++) {
        //cout << "regridded spectrum lobe0: " << spectrum->GetValue( xx * 2 ) << " " << spectrum->GetValue( xx * 2 + 1 ) << endl;
    //}

    //  redimension the data set in the EPSI k-space axis (18 voxels, epsiKData):  
    //  Remove all arrays with epsiAxis dimension greater
    //  than  integralMax, and reinit the DcmHeader!!!
    int numTimePoints = data->GetDcmHeader()->GetNumberOfTimePoints();
    for ( int channel = 0; channel < numChannels; channel++) {
    for ( int lobe = 0; lobe < numLobes; lobe++) { // lobes are stored as coils or channels
        for (int timePt = 0; timePt < numTimePoints; timePt++) {
            for ( int slice = 0; slice < regridDims[2]; slice++) {
                for ( int row = 0; row < regridDims[1]; row++) {
                    for ( int col = 0; col < regridDims[0]; col++) {

                        for ( int epsiK = integralMax; epsiK < numVoxels[epsiAxis]; epsiK++ ) {

                            if ( epsiAxis == 0 ) { 
                                col = epsiK;         
                            } else if ( epsiAxis == 1 ) {
                                row = epsiK;         
                            } else if ( epsiAxis == 2 ) {
                                slice = epsiK;         
                            }

                            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::COL_INDEX, col); 
                            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::ROW_INDEX, row); 
                            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::SLICE_INDEX, slice); 
                            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::TIME_INDEX, timePt); 
                            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::CHANNEL_INDEX, channel); 
                            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::EPSI_ACQ_INDEX, lobe); 

                            string arrayName = svk4DImageData::GetArrayName( &loopVector ); 
                            //cout << "REMOVE ARRAY: " << arrayName << endl;
                            data->GetCellData()->RemoveArray( arrayName.c_str() );

                        }
                    }
                }
            }
        }
    }
    }

    // Now reinit the DICOM header
    regridDims[epsiAxis] = integralMax; 

    //  reset the new spatial dims:
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::COL_INDEX, regridDims[0] - 1); 
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::ROW_INDEX, regridDims[1] - 1); 
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, regridDims[2] - 1); 

    // new version: 
    data->GetDcmHeader()->Redimension( &dimensionVector ); 
    data->GetDcmHeader()->SetValue( "DataPointColumns", numSpecPts );
    data->SyncVTKImageDataToDcmHeader();

    //data->GetDcmHeader()->PrintDcmHeader();    

    delete [] kn;
    delete [] knNorm;
    delete [] waveFormTmp;
    delete [] waveFormIntegralNorm;
    delete [] epsiKData0;
    delete [] overgrid; 

}


/*! 
 *  Initialize the rolloff correction: Eq 15 Jackson
 * 
 *  References: 
 *      John I. Jackson, Craig H. Meyer, Dwight G. Nishimura and Albert Macovski
 *      Selection of a Convolution Function for Fourier Inversion Using Gridding
 *      IEEE TRANSACTIONS ON MEDICAL IMAGING. VOL. IO. NO. 3 , SEPTEMBER 1991
 */
void svkGEPFileMapperUCSFfidcsiDev0::GetRolloffCorrection( int gridSize, float width, float beta, float* apodCor)
{

    float arg[2]; 
    float pi2 = vtkMath::Pi() * vtkMath::Pi();
    float width2 = width * width; 
    float beta2 = beta * beta; 
    float x2; 
    for ( int i = 0; i < gridSize; i++ ) {

        x2 = pow( (i - floor(gridSize/2.) ) / gridSize, 2);  
        arg[0]  = pi2 * width2 * x2 - beta2;  
        arg[1]  = 0; 

        // sin(a+ib) = sin(a) * cosh(b) + i cos(a) * sinh(b) 
        // which simplifies for all real or all imaginary
        if ( arg[0] >= 0 ) {
            // all real
            arg[0] = static_cast<float>( pow( static_cast<double>(arg[0]), 0.5) ); 
            apodCor[i] = static_cast<float>(sinf( arg[0] )) / arg[0];
        } else {
            // all imag 
            arg[1] = static_cast<float>( pow( static_cast<double>( -1 * arg[0]), 0.5) ); 
            arg[0] = 0;     
            apodCor[i] = sinh( static_cast<double>(arg[1]) ) / arg[1];
        }
    }

}    


/*!
 *
 *  Gridding: 
 *
 *  Computes the Kaiser-Bessel convolving function used for gridding, namely
 *
 *  y = f(u, width, beta) = I0 [ beta*sqrt(1-(2u/width)^2) ]/width
 *
 *  where 
 *      I0    = the zero-order modified Bessel function of the first kind.
 *      u     = vector of k-space locations for calculation.
 *      width = width parameter - see Jackson et al.
 *      beta  = beta parameter - see Jackson et al.
 *
 *  OUTPUT:
 *       y = vector of Kaiser-Bessel values.
 *
 *                                                         
 *  References: 
 *      John I. Jackson, Craig H. Meyer, Dwight G. Nishimura and Albert Macovski
 *      Selection of a Convolution Function for Fourier Inversion Using Gridding
 *      IEEE TRANSACTIONS ON MEDICAL IMAGING. VOL. IO. NO. 3 , SEPTEMBER 1991
 *  
 */
void svkGEPFileMapperUCSFfidcsiDev0::GetKaiserBesselValues( std::vector<float>* u, float width, float beta, std::vector<float>* kbVals )
{

    //  vector to hold indices for computing bessel coefficients. 
    std::vector <int> uz; 
    //std::vector<float> kb_vals; 
    // Indices where u < w/2.
    for ( int i = 0; i < u->size(); i++ ) {
        if ( fabs ( (*u)[i] ) < (width/2) ) {
            uz.push_back( i ); 
        }
        kbVals->push_back(0.); 
    }    

    // Calculate Modified Bessel Function of the first kind (order 0) at indices in uz vector.


    if ( uz.size() > 0) {

        for (int i = 0; i < uz.size(); i++) {

            //  Bessel argument: 
            float x =   1 - pow( ( 2. * (*u)[ uz[i] ] / width ), 2);    
            x = beta * pow( static_cast<double>(x), 0.5 );
            (*kbVals)[i] =  this->GetModifiedBessel0( x ) / width ; 

        }

    } 

}


/*
 *  Calculate the modified Bessel function (order 0)
 *  
 *  The zero-order functions simplify significantly and 
 *  are implemented here.  For the zero-order function, 
 *  the modified bessel function is the same as the 
 *  real component of the zero-order Bessel function
 *  of the first kind     
 *
 *  Ref: 
 *  Mathematical Methods for Physicists (3rd edition, 1985)
 *  George Arfken 
 *  Academic Press, INC
 *  equation 11.111
 */
double svkGEPFileMapperUCSFfidcsiDev0::GetModifiedBessel0( float arg )
{
         
    double modifiedBessel; 
    float besselArgImag = arg;  
    float besselReal = this->GetBessel0( besselArgImag ); 

    //  for order 0, the first term in 11.111 reduces to: 
    //  cos(0)*J(arg) - isin(0)*J(arg) = 1 * J(arg)
    modifiedBessel = besselReal; 
    return modifiedBessel; 
}


/*
 *  Calculate the Real component of the Bessel function of the first 
 *  kind (order 0) returns only the real component of an imaginary argument:
 *      i.e. arg is implicitly -i * arg.
 *
 *  Ref: 
 *  Mathematical Methods for Physicists (3rd edition, 1985)
 *  George Arfken 
 *  Academic Press, INC
 *  equation 11.5 
 */
double svkGEPFileMapperUCSFfidcsiDev0::GetBessel0( float arg)
{
    double bessel; 
    double besselTerm; 

    float tol; 
    float tolNum; 
    float tolDenom; 
   
    //  Calculate first term:  
    int index = 0; 
    bessel = this->GetBessel0Term( arg, index);  
    tolNum = bessel; 
    tolDenom = bessel; 
    tol = fabs(tolNum/tolDenom); 

    while ( tol > .00001) {
        index++; 
        besselTerm = this->GetBessel0Term( arg, index);
        bessel += besselTerm;
        tol = fabs(besselTerm/tolDenom); 
        tolDenom = tolNum; 
    }
    return bessel; 

}


/*
 *  Get term in series form of bessel function:
 *  this is a specialzed solution for implicit "i" 
 *  in the argument for computing modified bessel 0 solutions.  
 *
 *  Ref: 
 *  Mathematical Methods for Physicists (3rd edition, 1985)
 *  George Arfken 
 *  Academic Press, INC
 *  equation 11.5 
 */
double svkGEPFileMapperUCSFfidcsiDev0::GetBessel0Term( float arg, int index)
{

    float besselTerm = pow( static_cast<float>(-1), index );
    besselTerm *= pow( static_cast<double>(arg/2), 2*index ); 
    if (besselTerm == HUGE_VAL ) {
        cout << "ERROR: " << this->GetClassName() << " Can not get Gessel0Term for " << arg << " " << index << endl;
        cout << "ERROR: " << this->GetClassName() << " value out of range " << endl; 
        exit(1); 
    }
    besselTerm /= pow( static_cast<double>( vtkMath::Factorial( index) ), 2); 

    //  account for alternating i ^ (2*index)
    if ( index % 2 ) {
        besselTerm *= -1; 
    }

    return besselTerm; 
}


/*!
 *  Returns the waveform integral over a single period, normalized to 1 and centered 
 *  about 0, with both end points removed (zero crossing). 
 */
void svkGEPFileMapperUCSFfidcsiDev0::GetWaveFormIntegral( float* waveFormIntegral, int sampleSpacingTimeMs, int plateauTime, int rampDuration )
{
    //  4 microsecond sampling:     
    int sampleDeltaT= 4; 
    int numRampSamples = rampDuration / sampleDeltaT;  
    int numPlateauSamples = plateauTime / sampleDeltaT;  

    int numWaveFormPts = (numRampSamples * 2) + numPlateauSamples; 
    float* waveForm = new float[ numWaveFormPts ]; 
    float* waveFormIntegralAll = new float[ numWaveFormPts ]; 

    for ( int i = 0; i < numWaveFormPts; i++) { 

        if ( i <  numRampSamples ) { 
            //  leading ramp 
            waveForm[i] = ( i + 0.5 ) / numRampSamples; 
        } else if ( i >= ( numRampSamples + numPlateauSamples ) ) {
            //  trailing ramps:    
            int rampIndex = numRampSamples - (i -  (numRampSamples + numPlateauSamples) ); 
            waveForm[i] = ( rampIndex - 0.5 ) / numRampSamples; 
        } else {
            //  plateau
            waveForm[i] = 1; 
        }
  
        if ( i == 0 ) { 
            waveFormIntegralAll[i] = waveForm[ i ]; 
        } else if ( i > 0 ) {
            waveFormIntegralAll[i] = waveForm[i] + waveFormIntegralAll[ i-1 ]; 
        }
    }

    //  make symmetric about 0: 
    float max = waveFormIntegralAll[numWaveFormPts-1]; 
    for ( int i = 0; i < numWaveFormPts; i++) { 
        waveFormIntegralAll[i] -= max/2.; 
    }


    //  normalize to 1 and down sample full integral to EPSI k-space sampling interval: 
    int binSize = sampleSpacingTimeMs / sampleDeltaT; 
    int binIndex = 0 + binSize/2 -1; 

    // truncate first and last points: 
    int firstSample = binIndex + binSize;  
    int lastSample = numWaveFormPts - binSize; 
    int outputIndex = 0; 
    for ( int i = firstSample; i < lastSample; i+=binSize) { 
        waveFormIntegral[outputIndex] = waveFormIntegralAll[i]/ binSize; 
        outputIndex++; 
    }

    delete [] waveForm;
    delete [] waveFormIntegralAll;

}




/*!
 *  Shift the spatial k-space origin to the image center
 *  so the output data is suitable for standard FFT 
 *  recon which expects the origin to be in the center. 
 */
void svkGEPFileMapperUCSFfidcsiDev0::FFTShift( svkImageData* data )
{

    svkMrsImageData* tmpData = svkMrsImageData::New();
    tmpData->DeepCopy( data ); 

    svkMrsImageFourierCenter* fftShift = svkMrsImageFourierCenter::New(); 
    fftShift->SetShiftDomain( svkMrsImageFourierCenter::SPATIAL ); 
    fftShift->SetShiftDirection( svkMrsImageFourierCenter::FORWARD); 
    fftShift->SetInputData( tmpData ); 
    fftShift->Update(); 

    data->DeepCopy( fftShift->GetOutput() ); 

    fftShift->Delete(); 
    tmpData->Delete(); 
}


/*!
 *  Zero fill to 256 
 */
void svkGEPFileMapperUCSFfidcsiDev0::ZeroFill( svkImageData* data )
{

    svkDcmHeader* hdr = data->GetDcmHeader(); 
    int numSpecPts = hdr->GetIntValue( "DataPointColumns" );
    int cols       = hdr->GetIntValue( "Columns" );
    int rows       = hdr->GetIntValue( "Rows" );
    int slices     = hdr->GetNumberOfSlices();
    int numLobes   = 2; 

    float cmplxPt[2]; 
    cmplxPt[0] = 0.; 
    cmplxPt[1] = 0.; 

    for (int lobe = 0; lobe < numLobes; lobe++) {
        for (int z = 0; z < slices; z++) {
            for (int y = 0; y < rows; y++) {
                for (int x = 0; x < cols; x++) {

                    vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( svkMrsImageData::SafeDownCast(data)->GetSpectrum( x, y, z, 0, lobe) );

                    //  Iterate over frequency points in spectrum and apply phase correction:
                    for ( int freq = numSpecPts; freq < 256; freq++ ) {

                        spectrum->InsertTuple(freq, cmplxPt);

                    }
                }
            }
        }
    }

    data->GetDcmHeader()->SetValue( "DataPointColumns", 256);

}



/*!
 *  Reorder axes if patient entry is not HFS: 
 */
void svkGEPFileMapperUCSFfidcsiDev0::ModifyForPatientEntry( svkImageData* data )
{

    string patientPosition = data->GetDcmHeader()->GetStringValue( "PatientPosition" );

    double dcos[3][3];
    data->GetDcmHeader()->GetDataDcos( dcos );

    double origin[3];
    data->GetDcmHeader()->GetOrigin( origin, 0 );

    double voxelSpacing[3];
    data->GetDcmHeader()->GetPixelSpacing( voxelSpacing );

    svkDcmHeader* hdr = data->GetDcmHeader();
    int numVoxels[3]; 
    numVoxels[0] = hdr->GetIntValue( "Columns" ); 
    numVoxels[1] = hdr->GetIntValue( "Rows" ); 
    numVoxels[2] = hdr->GetNumberOfSlices();

    //  Get the current center: 
    double center[3]; 
    svkDcmHeader::GetCenterFromOrigin( origin, numVoxels, voxelSpacing, dcos, center); 

    //  If feet first, swap LR & SI (first and third columns of 
    //  LPS/col,row, slice dcos matrix: 

    if( patientPosition.find("FF") != string::npos ) {

        //  =================================================
        //  if feet first entry, reverse RL and SI direction: 
        //  =================================================

        //  fix dcos for epsiAxis
        int epsiAxis = this->GetEPSIAxis();
        dcos[epsiAxis][0] *=-1;
        dcos[epsiAxis][1] *=-1;
        dcos[epsiAxis][2] *=-1;

    }


    //  Now calcuate the new origin based on the new dcos: 
    double newOrigin[3]; 
    svkDcmHeader::GetOriginFromCenter( center, numVoxels, voxelSpacing, dcos, newOrigin ); 

    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector(); 

    data->GetDcmHeader()->InitPerFrameFunctionalGroupSequence(
        newOrigin,
        voxelSpacing,
        dcos,
        &dimensionVector
    );

    data->SyncVTKImageDataToDcmHeader(); 

}


/*
 *
 */
void svkGEPFileMapperUCSFfidcsiDev0::InitMRSpectroscopyModule()
{
    this->Superclass::InitMRSpectroscopyModule();

    this->dcmHeader->SetValue(
        "SpectralWidth",
        581
    );

}


/*!
 *  Gets the chemical shift reference taking into account acquisition frequency offset
 *  and the acquisition sample temperature.
 */
float svkGEPFileMapperUCSFfidcsiDev0::GetPPMRef()
{
    return 178; 
}

void svkGEPFileMapperUCSFfidcsiDev0::LoadDataAcquisitionDescriptionFile( string pfileName )
{
    if( this->dadFile == NULL ) {
        size_t lastPath = pfileName.find_last_of("/");
        string pfileDirectory = svkUtils::GetCurrentWorkingDirectory();
        if ( lastPath != pfileName.npos ) {
            pfileDirectory = pfileName.substr(0, lastPath);
        } else {
            pfileDirectory = svkUtils::GetCurrentWorkingDirectory();
        }
        string dadFileName = pfileDirectory;
        dadFileName.append("/P44544_dad_fidcsi_ucsf.xml");

        if ( svkUtils::FilePathExists( dadFileName.c_str() )){
            this->dadFile = svkDataAcquisitionDescriptionXML::New();
            //  search for dad files with .xml extension and find one with dad element: 
            cout << "DAD File Name: " << dadFileName << endl;
            int status = this->dadFile->SetXMLFileName( dadFileName );
            if ( status != 0 ) {
                cout << "ERROR: not a DAD file" << endl;
                exit(1); 
            }
            cout << "Loading parameters from Data Acquistion Description file... " << endl;
            cout << "Trajectory ID           : " << this->dadFile->GetTrajectoryID() << endl;
            cout << "EPSI Type               : " << this->dadFile->GetEPSITypeString() << endl;
            cout << "Interleaves             : " << this->dadFile->GetDataWithPath("/encoding/trajectoryDescription/epsiEncoding/numInterleaves") << endl;
            cout << "gradientAmplitudeOddMTM : " << this->dadFile->GetDataWithPath("/encoding/trajectoryDescription/epsiEncoding/gradientAmplitudeOddMTM") << " mT/m" << endl;
            cout << "gradientAmplitudeEvenMTM: " << this->dadFile->GetDataWithPath("/encoding/trajectoryDescription/epsiEncoding/gradientAmplitudeEvenMTM") << " mT/m" << endl;
            cout << "rampDurationOddMs       : " << this->dadFile->GetDataWithPath("/encoding/trajectoryDescription/epsiEncoding/rampDurationOddMs") << " ms" << endl;
            cout << "rampDurationEvenMs      : " << this->dadFile->GetDataWithPath("/encoding/trajectoryDescription/epsiEncoding/rampDurationEvenMs") << " ms" << endl;
            cout << "plateauDurationOddMs    : " << this->dadFile->GetDataWithPath("/encoding/trajectoryDescription/epsiEncoding/plateauDurationOddMs") << " ms" << endl;
            cout << "plateauDurationEvenMs   : " << this->dadFile->GetDataWithPath("/encoding/trajectoryDescription/epsiEncoding/plateauDurationEvenMs") << " ms" << endl;
            cout << "numLobesOdd             : " << this->dadFile->GetDataWithPath("/encoding/trajectoryDescription/epsiEncoding/numberOfLobesOdd") << endl;
            cout << "numLobesEven            : " << this->dadFile->GetDataWithPath("/encoding/trajectoryDescription/epsiEncoding/numberOfLobesEven") << endl;
            cout << "sampleSpacing_timeMs    : " << this->dadFile->GetDataWithPath("/encoding/trajectoryDescription/epsiEncoding/sampleSpacingTimeMs") << " ms" << endl;
            cout << "acquisitionDelayMs      : " << this->dadFile->GetDataWithPath("/encoding/trajectoryDescription/epsiEncoding/acquisitionDelayTimeMs") << " ms" << endl;
            cout << "gradientAxis            : " << this->dadFile->GetDataWithPath("/encoding/trajectoryDescription/epsiEncoding/gradientAxis") << " ms" << endl;
            cout << "echoDelayMs             : " << this->dadFile->GetDataWithPath("/encoding/trajectoryDescription/epsiEncoding/echoDelayTimeMs") << " ms" << endl;

        }
    }
}
