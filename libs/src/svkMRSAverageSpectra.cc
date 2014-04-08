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
 *      Beck Olson
 */



#include <svkMRSAverageSpectra.h>


using namespace svk;


vtkCxxRevisionMacro(svkMRSAverageSpectra, "$Rev$");
vtkStandardNewMacro(svkMRSAverageSpectra);


svkMRSAverageSpectra::svkMRSAverageSpectra()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    this->useMask = false;
    this->SetNumberOfInputPorts(2); // the 2nd is optional

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
 *  Averages spectra over input ROI. 
 */
void svkMRSAverageSpectra::AverageSpectraInROI()
{


    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    svkMriImageData* maskImage = svkMriImageData::SafeDownCast( this->GetImageDataInput(1) );
    svkDcmHeader* hdr = data->GetDcmHeader(); 
    svkDcmHeader* avHdr = this->averageData->GetDcmHeader(); 
    svkDcmHeader* maskHdr = maskImage->GetDcmHeader(); 

    unsigned short* mask = static_cast<vtkUnsignedShortArray*>( 
                        maskImage->GetPointData()->GetArray(0) )->GetPointer(0) ; 

    svkDcmHeader::DimensionVector dimVec      = hdr->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector loopVec     = dimVec; 
    svkDcmHeader::DimensionVector avDimVec    = avHdr->GetDimensionIndexVector(); 
    svkDcmHeader::DimensionVector avLoopVec   = avDimVec; 
    svkDcmHeader::DimensionVector maskDimVec  = maskHdr->GetDimensionIndexVector(); 
    svkDcmHeader::DimensionVector maskLoopVec = maskDimVec; 

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
        for ( int dim = 0; dim < 3; dim++ ) {
            svkDcmHeader::DimensionIndexLabel indexLabel=  svkDcmHeader::GetDimensionLabelFromIndex( &loopVec, dim ); 
            int indexValue = svkDcmHeader::GetDimensionVectorValue( &loopVec, indexLabel ); 
            svkDcmHeader::SetDimensionVectorValue( &maskLoopVec, indexLabel, indexValue); 
        }

        int maskCellID = svkDcmHeader::GetCellIDFromDimensionVectorIndex( &maskDimVec, &maskLoopVec ); 

        if ( mask[maskCellID] != 0 ) {

            // Get loop values, and set first 3 dimensions to 1 to create correct output 
            //  loop
            svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimVec, &loopVec, cellID ); 
            vtkFloatArray* spectrumIn = static_cast<vtkFloatArray*>( data->GetSpectrum( cellID ) );

            //  ==========================================================
            //  Get the corresponding average spectrum to write into
            //  Set the non spatial dimensions to match those in the input data: 
            //  ==========================================================
            for ( int dim = 4; dim < avDimVec.size(); dim++ ) {
                svkDcmHeader::DimensionIndexLabel indexLabel=  svkDcmHeader::GetDimensionLabelFromIndex( &loopVec, dim ); 
                int indexValue = svkDcmHeader::GetDimensionVectorValue( &loopVec, indexLabel ); 
                svkDcmHeader::SetDimensionVectorValue( &avLoopVec, indexLabel, indexValue); 
            }
            int avCellID = svkDcmHeader::GetCellIDFromDimensionVectorIndex( &avDimVec, &avLoopVec ); 
            vtkFloatArray* spectrumAv = static_cast<vtkFloatArray*>( this->averageData->GetSpectrum( avCellID ) );

            for (int i = 0; i < numTimePoints; i++ ) {
                spectrumIn->GetTuple(i, tupleIn);     
                spectrumAv->GetTuple(i, tupleAv); 
                tupleAv[0] += tupleIn[0]; 
                tupleAv[1] += tupleIn[1]; 
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
    }
    return 1;
}


/*!
 *  PrintSelf method calls parent class PrintSelf, then prints the dcos.
 *
 */
void svkMRSAverageSpectra::PrintSelf( ostream &os, vtkIndent indent )
{
    Superclass::PrintSelf( os, indent );
    os << "only use selection box:" << this->useMask << endl;
}



