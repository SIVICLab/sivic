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



#include <svkMRSAverageSpectra.h>


using namespace svk;


//vtkCxxRevisionMacro(svkMRSAverageSpectra, "$Rev$");
vtkStandardNewMacro(svkMRSAverageSpectra);


svkMRSAverageSpectra::svkMRSAverageSpectra()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    this->useMaskFile = false;
    this->useSelectionBoxMask = false;
    this->useMagnitudeSpectra = false;
    this->SetNumberOfInputPorts(2); // the 2nd is optional
    this->averageOverNonSpatialDims = false;

}


svkMRSAverageSpectra::~svkMRSAverageSpectra()
{
}


/*! 
 *
 */
int svkMRSAverageSpectra::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    return 1;
}


/*! 
 *
 */
int svkMRSAverageSpectra::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    this->SetAverageDataDimensions(); 

    this->InitAverageSpectrum(); 

    this->InitMask();

    this->AverageSpectraInROI();

    // copy the average data back onto the original: 
    svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) )->DeepCopy( this->averageData ); 

    this->averageData->Delete(); 

    return 1; 
}


/*
 *  Resets the spatial dimensions to be a single voxel.  
 */
void svkMRSAverageSpectra::SetAverageDataDimensions()
{
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    svkDcmHeader::DimensionVector dimVec = data->GetDcmHeader()->GetDimensionIndexVector();

    //  Make a single voxel copy of the input data: 
    //      - redimension the spatial dimensions in the header and remove 
    //        but one array for each non-spatial index. 
    this->averageData = svkMrsImageData::New();
    this->averageData->DeepCopy( data ); 
    svkDcmHeader* averageHdr = this->averageData->GetDcmHeader();

    svkDcmHeader::PrintDimensionIndexVector( &dimVec );

    //  =======================================================
    //  Redimension the average data to a single spatial voxel  
    //  (may have more than one voxel if multi-channel, etc. 
    //  =======================================================
    averageHdr->SetDimensionIndexSize( svkDcmHeader::COL_INDEX, 0 );
    averageHdr->SetDimensionIndexSize( svkDcmHeader::ROW_INDEX, 0);
    averageHdr->SetDimensionIndexSize( svkDcmHeader::SLICE_INDEX, 0);

    /*
     *  If averaging over time/channels, etc then set remaining dims to 0 as well: 
     */
    if ( this->averageOverNonSpatialDims == true ) {
        for ( int dim = 3; dim < dimVec.size(); dim++ ) {
            averageHdr->SetDimensionIndexSize( averageHdr->GetDimensionLabelFromIndex(&dimVec, dim), 0); 
        }
    }

    this->averageData->SyncVTKImageDataToDcmHeader(); 

    //  =======================================================
    //  First, remove all arrays, then add one in for each of 
    //  the single voxels. 
    //  =======================================================
    int numCellsIn = svkDcmHeader::GetNumberOfCells( &dimVec ); 
    svkDcmHeader::DimensionVector loopVec = dimVec;     
    for (int i = 0; i < numCellsIn; i++ ) {
        svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimVec, &loopVec, i ); 
        string arrayName = svk4DImageData::GetArrayName( &loopVec );
        this->averageData->GetCellData()->RemoveArray( arrayName.c_str() );
    }

    //  =======================================================
    //  Allocate arrays for average data set: 
    //  =======================================================
    svkDcmHeader::DimensionVector avDimVec = averageHdr->GetDimensionIndexVector(); 
    svkDcmHeader::DimensionVector avLoopVec = avDimVec; 
    int numAverageCells = svkDcmHeader::GetNumberOfCells( &avDimVec ); 

    svkFastCellData* cellDataTmp = static_cast<svkFastCellData*>(this->averageData->GetCellData()); 
    cellDataTmp->AllocateArrays( numAverageCells); 
    vtkDataArray* dataArray = NULL;
    int numComponents = 2; 
    int numFreqPts = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );
    for (int cell = 0; cell < numAverageCells; cell++) {

        dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
        dataArray->SetNumberOfComponents( numComponents );
        dataArray->SetNumberOfTuples( numFreqPts );

        svkDcmHeader::GetDimensionVectorIndexFromCellID( &avDimVec, &avLoopVec, cell ); 
        string arrayName = svk4DImageData::GetArrayName( &avLoopVec );
        dataArray->SetName( arrayName.c_str() ); 
        this->averageData->SetArrayName(dataArray, &avLoopVec ); 

        cellDataTmp->FastAddArray( dataArray );
        dataArray->FastDelete();

    }
    cellDataTmp->FinishFastAdd();
}


/*!
 *  Initializes an average svkMrsImageData to zero. 
 */
void svkMRSAverageSpectra::InitAverageSpectrum()    
{

    svkDcmHeader* avHdr = this->averageData->GetDcmHeader(); 
    svkDcmHeader::DimensionVector avDimVec = avHdr->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector avLoopVec = avDimVec; 

    int numTimePoints = avHdr->GetIntValue( "DataPointColumns" );
    int numAverageCells = svkDcmHeader::GetNumberOfCells( &avDimVec ); 
    for (int cell = 0; cell < numAverageCells; cell++) {

        //  Create 0 valued vtkFloatArray for average spectrum 
        vtkFloatArray* spectrum = static_cast<vtkFloatArray*>( this->averageData->GetSpectrum( 0 ) );
        float tuple[2];
        for (int i = 0; i < numTimePoints; i++ ) {
            tuple[0] = 0.; 
            tuple[1] = 0.; 
            spectrum->SetTuple(i, tuple); 
        }
    }

    return; 
} 


/*
 *  Get the mask ROI from file or the selection box.
 */
void svkMRSAverageSpectra::InitMask()
{
    if( this->GetImageDataInput(1) != NULL) {
        this->useMaskFile == true; 
        this->useSelectionBoxMask = false;

        svkMriImageData* maskImage = svkMriImageData::SafeDownCast( this->GetImageDataInput(1) );
        this->maskROI = maskImage->GetPointData()->GetArray(0); 

    } else if ( this->useSelectionBoxMask ) {

        svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
        svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
        int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );

        float tolerance = .5;     
        short* mask = new short[numCells];
        data->GetSelectionBoxMask( mask, tolerance); 
        this->maskROI = vtkShortArray::New(); 
        for ( int i = 0; i < numCells; i++ ) {
            float maskVal = static_cast<float>(mask[i]);
            this->maskROI->InsertTuple(i, &maskVal );
        }
    } else {
        // default is use all cells: 
        svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
        svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
        int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );
        this->maskROI = vtkShortArray::New(); 
        for ( int i = 0; i < numCells; i++ ) {
            float maskVal = 1;
            this->maskROI->InsertTuple(i, &maskVal );
        }
    }

}


/*
 *  Averages spectra over input ROI. 
 */
void svkMRSAverageSpectra::AverageSpectraInROI()
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    svkDcmHeader* hdr = data->GetDcmHeader(); 
    svkDcmHeader* avHdr = this->averageData->GetDcmHeader(); 

    svkDcmHeader::DimensionVector dimVec      = hdr->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector loopVec     = dimVec; 
    svkDcmHeader::DimensionVector avDimVec    = avHdr->GetDimensionIndexVector(); 
    svkDcmHeader::DimensionVector avLoopVec   = avDimVec; 

    int numTimePoints = hdr->GetIntValue( "DataPointColumns" );
    int numInputCells = svkDcmHeader::GetNumberOfCells( &dimVec ); 

    //  iterate over input data set, and assign average to output. 
    double tupleIn[2];
    double tupleAv[2];
    int numCellsInMask = 0; 
    for (int cellID = 0; cellID < numInputCells; cellID++) {
        
        svkDcmHeader::GetDimensionVectorIndexFromCellID(&dimVec, &loopVec, cellID); 

        //  ==========================================================
        //  Get the corresponding mask cell (only look at spatial dims). 
        //  Then check to see if this cell is within the mask. 
        //  ==========================================================
        int maskCellID = svkDcmHeader::GetSpatialCellIDFromDimensionVectorIndex( &dimVec, &loopVec ); 
        //cout << "MASK CELL ID: " << maskCellID << " = " << this->maskROI->GetTuple(maskCellID)[0] << endl;

        if ( this->maskROI->GetTuple(maskCellID)[0] != 0 ) {

            // Get loop values, and set first 3 dimensions to 1 to create correct output 
            //  loop
            svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimVec, &loopVec, cellID ); 
            vtkFloatArray* spectrumIn = static_cast<vtkFloatArray*>( data->GetSpectrum( cellID ) );

            //  ==========================================================
            //  Get the corresponding average spectrum to write into
            //  Set the non spatial dimensions to match those in the input data: 
            //  ==========================================================
            if ( this->averageOverNonSpatialDims == false ) { 
                for ( int dim = 3; dim < avDimVec.size(); dim++ ) {
                    svkDcmHeader::DimensionIndexLabel indexLabel=  svkDcmHeader::GetDimensionLabelFromIndex( &loopVec, dim ); 
                    int indexValue = svkDcmHeader::GetDimensionVectorValue( &loopVec, indexLabel ); 
                    svkDcmHeader::SetDimensionVectorValue( &avLoopVec, indexLabel, indexValue); 
                }
            }
            int avCellID = svkDcmHeader::GetCellIDFromDimensionVectorIndex( &avDimVec, &avLoopVec ); 
            vtkFloatArray* spectrumAv = static_cast<vtkFloatArray*>( this->averageData->GetSpectrum( avCellID ) );
            //cout << "Input cell, av cell: " << cellID << " " << avCellID << endl;

            for (int i = 0; i < numTimePoints; i++ ) {
                spectrumIn->GetTuple(i, tupleIn);     
                spectrumAv->GetTuple(i, tupleAv); 

                if ( this->useMagnitudeSpectra == true ) {
                    double rms = ( tupleIn[0] * tupleIn[0] + tupleIn[1] * tupleIn[1] ); 
                    rms = pow(rms, 0.5); 
                    tupleAv[0] += rms; 
                    tupleAv[1] += rms; 
                } else {
                    tupleAv[0] += tupleIn[0]; 
                    tupleAv[1] += tupleIn[1]; 
                }
                spectrumAv->SetTuple(i, tupleAv); 
            }
            numCellsInMask++; 
        }

    }

    if ( numCellsInMask > 1 ) {
        int numAvCells = svkDcmHeader::GetNumberOfCells( &avDimVec ); 
        for (int avCellID = 0; avCellID < numAvCells; avCellID++) {
            vtkFloatArray* spectrumAv = static_cast<vtkFloatArray*>( this->averageData->GetSpectrum( avCellID ) );
            for (int i = 0; i < numTimePoints; i++ ) {
                spectrumAv->GetTuple(i, tupleAv);     
                tupleAv[0] /= numCellsInMask; 
                tupleAv[1] /= numCellsInMask; 
                spectrumAv->SetTuple(i, tupleAv); 
            }
        }
    }

}


/*!
 *  Port 0 is the input MRS data
 *  Port 1 is the optional ROI mask (MRI data)
 */
int svkMRSAverageSpectra::FillInputPortInformation( int port, vtkInformation* info )
{
    if ( port == 0 ) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    }
    if ( port == 1 ) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
        info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);

    }
    return 1;
}


/*!
 *  Use selection box mask rather than mask from file  
 */
void svkMRSAverageSpectra::LimitToSelectionBox()
{
    this->useMaskFile = false;
    this->useSelectionBoxMask = true;
}


/*!
 *  Average the magnitude spectra within the ROI. 
 */
void svkMRSAverageSpectra::AverageMagnitudeSpectra()
{
    this->useMagnitudeSpectra = true;
}


/*
 * Sets algorithm to average over spatial ROI in all non-Spatial DIMS.  For example if 
 * this is a multi-coil data set will result in the average spectrum within the 
 * specified spatial ROI and over all channels.  By default the average spatial average
 * spectrum for each volume (time, channel, etc) are returned separately.  
 */
void svkMRSAverageSpectra::AverageOverNonSpatialDims()
{
    this->averageOverNonSpatialDims = true; 
}


/*!
 *  PrintSelf method calls parent class PrintSelf, then prints the dcos.
 *
 */
void svkMRSAverageSpectra::PrintSelf( ostream &os, vtkIndent indent )
{
    Superclass::PrintSelf( os, indent );
    os << "only use selection box:" << this->useMaskFile << endl;
}
