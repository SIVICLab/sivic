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



#include <svkEPSIReorder.h>
#include <svkMrsLinearPhase.h> 
#include <svkUtils.h> 
#include <svkTypeUtils.h> 


#define UNDEFINED_NUM_SAMPLES -1
#define UNDEFINED_NUM_LOBES -1

using namespace svk;


//vtkCxxRevisionMacro(svkEPSIReorder, "$Rev$");
vtkStandardNewMacro(svkEPSIReorder);


/*!
 *  Constructor.  Initialize any member variables. 
 */
svkEPSIReorder::svkEPSIReorder()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    //  Initialize any member variables
    this->epsiType = UNDEFINED_EPSI_TYPE;
    this->numSamplesPerLobe = UNDEFINED_NUM_SAMPLES; 
    this->numLobes = UNDEFINED_NUM_LOBES; 
    this->numSamplesToSkip = 0; 
    this->firstSample = 0; 
    this->epsiAxis = svkEPSIReorder::UNDEFINED_EPSI_AXIS;

}


/*!
 *  Clean up any allocated member variables. 
 */
svkEPSIReorder::~svkEPSIReorder()
{
}


/*!
 *  Set the axis index corresponding to the EPSI encoding (0,1 or 2). 
 */
void svkEPSIReorder::SetEPSIAxis( svkEPSIReorder::EPSIAxis epsiAxis )
{
    this->epsiAxis = epsiAxis;
}


/*!
 *  Get the axis index corresponding to the EPSI encoding (0,1 or 2). 
 */
svkEPSIReorder::EPSIAxis svkEPSIReorder::GetEPSIAxis()
{
    return this->epsiAxis;
}


/*!
 *  Set the type of EPSI sequence 
 */ 
void svkEPSIReorder::SetEPSIType( EPSIType epsiType )
{
    this->epsiType = epsiType; 
}


/*!
 *  Set the number of lobes in the EPSI acquisition
 */
void svkEPSIReorder::SetNumEPSILobes( int numLobes )
{
    this->numLobes = numLobes; 
}


/*!
 *  Set the number of lobes in the EPSI acquisition
 */
void svkEPSIReorder::SetNumSamplesPerLobe( int numSamplesPerLobe )
{
    this->numSamplesPerLobe = numSamplesPerLobe; 

}

/*!
 *  Get the number of lobes in the EPSI acquisition.  If the number
 *  of samples per lobe has not been explicitly set via SetNumEPSISamplesPerLobe, 
 *  then infer it by dividing the number of samples in the entire EPSI waveform
 *  by the number of lobes. 
 */
int svkEPSIReorder::GetNumSamplesPerLobe( )
{
    if ( this->numSamplesPerLobe == UNDEFINED_NUM_SAMPLES  ) {
        if ( this->numLobes != UNDEFINED_NUM_LOBES && this->GetImageDataInput(0) != NULL ) {
            this->numSamplesPerLobe = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue("DataPointColumns")/ this->numLobes; 
        } else {
            cout << "ERROR, can not determine the number of samples per lobe" << endl; 
        }    
    } 
    return this->numSamplesPerLobe;
}


/*!
 *  Set the number of samples to skip between lobes. Different from 
 *  the initial offset (firstSample).  This is 0 by default.  
 */
void svkEPSIReorder::SetNumSamplesToSkip( int numSamplesToSkip)
{
    this->numSamplesToSkip = numSamplesToSkip; 
}

/*!
 *  Set the first sample to read when reordering the EPSI data. A few 
 *  samples may be skipped at the begining. To include the first point
 *  set to 0 (default).    
 */
void svkEPSIReorder::SetFirstSample( int firstSample )
{
    this->firstSample = firstSample; 
}


/*!
 *  Virtual method that returns the number of spec frequency points
 *  in the acquisition.  May be reimplemented for specific EPSI 
 *  acquisitions. 
 */
int svkEPSIReorder::GetNumEPSIFrequencyPoints()
{
    int numFreqPts;
    if ( this->epsiType == UNDEFINED_EPSI_TYPE ) {
        cout << "ERROR( " << this->GetClassName() << "): EPSI TYPE is undefined" << endl;
        exit(1); 
    } else if ( this->epsiType == FLYBACK ) {
        numFreqPts = this->numLobes;     
    } else if ( this->epsiType == SYMMETRIC ) {
        numFreqPts = this->numLobes / 2;     
    } else if ( this->epsiType == INTERLEAVED ) {
        numFreqPts = this->numLobes;     
    } else {
        cout << "ERROR( " << this->GetClassName() << "): UNSUPPORTED EPSI TYPE " << endl;
        exit(1); 
    }
    return numFreqPts; 
}


/*!
 *  Virtual method that returns the number of acquisitions in the 
 *  sequence.  For example symmetric EPSI contains 2, one during the 
 *  positive gradient and one during the negative gradient. 
 */
int svkEPSIReorder::GetNumEPSIAcquisitions()
{
    int numAcquisitions; 

    if ( this->epsiType == UNDEFINED_EPSI_TYPE ) {
        cout << "ERROR( " << this->GetClassName() << "): EPSI TYPE is undefined" << endl;
        exit(1); 
    } else if ( this->epsiType == FLYBACK ) {
        numAcquisitions = 1; 
    } else if ( this->epsiType == SYMMETRIC ) {
        numAcquisitions = 2; 
    } else if ( this->epsiType == INTERLEAVED ) {
        numAcquisitions = 2; 
    } else {
        cout << "ERROR( " << this->GetClassName() << "): EPSI TYPE is undefined" << endl;
        exit(1); 
    }
    
    return numAcquisitions; 
}


/*!
 *  Virtual method that returns the number of acquisitions in each FID 
 *  1, except for symmetric which has interleaved pos/neg lobes. 
 */
int svkEPSIReorder::GetNumEPSIAcquisitionsPerFID()
{
    int numAcquisitionsPerFID; 

    if ( this->epsiType == UNDEFINED_EPSI_TYPE ) {
        cout << "ERROR( " << this->GetClassName() << "): EPSI TYPE is undefined" << endl;
        exit(1); 
    } else if ( this->epsiType == FLYBACK ) {
        numAcquisitionsPerFID = 1; 
    } else if ( this->epsiType == SYMMETRIC ) {
        numAcquisitionsPerFID = 2;  //for symmetric they are alternating in the same FID
    } else if ( this->epsiType == INTERLEAVED ) {
        numAcquisitionsPerFID = 1;  // here they are already separated out into 2 "time points"
    } else {
        cout << "ERROR( " << this->GetClassName() << "): EPSI TYPE is undefined" << endl;
        exit(1); 
    }
    
    return numAcquisitionsPerFID; 
}


/*! 
 *  This method is called during pipeline execution.  Calculates the 2D array of linear phase correction factors, which
 *  are a function of the epsi k-space index, as well as the spectral index.  Applies these to all spectra in data set
 *  to generate a rectilinear spectral/spatial (k-space, time-domain) data set.  
 */
int svkEPSIReorder::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    if ( this->epsiType == UNDEFINED_EPSI_TYPE || 
        this->epsiAxis == svkEPSIReorder::UNDEFINED_EPSI_AXIS || 
        this->numLobes == UNDEFINED_NUM_LOBES 
    ) {
        cout << "ERROR, must specify the epsiAxis and number of sample k-space points per lobe" << endl;
        return 1; 
    }

    //  Get pointer to input data set. 
    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    this->ReorderEPSIData( mrsData ); 

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    //this->Update();

    return 1; 
} 


/*!
 *  Reorder the data.  Input EPSI data has one dimension with both time (spectral) and k-space
 *  content.  The time domain data from each k-space point needs to be extracted and put into 
 *  a cell array.  This is done separately for positive and negative lobes.  So, for example, 
 *  a 1380 x 1 x 12 x 10 data set with 60 lobes (23 k-space samples) would get reordered to a
 *  two 30 x 23 x 12 x 10 data sets. Each will be encoded into a separate "channel" indicated
 *  by the dimension index sequence.  this is just a temporarary internal layout.
 */
void svkEPSIReorder::ReorderEPSIData( svkImageData* data )
{

    //  Reorder data.  This requires adding data arrays 
    //  to accommodate the spectra at the new k-space points 
    //  points extracted from the EPSI encoding 

    //  Preallocate data arrays. We can't override the original arrays, 
    //  until the reordering is complete, so use these as the 
    //  target arrays. 

    //  Allocate arrays for all How many k-space points were acquired in all?

    //  Allocate arrays for spectra at each phase encode:
    svkDcmHeader* hdr = data->GetDcmHeader();
    std::string dataRepresentation = hdr->GetStringValue( "DataRepresentation" );

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

    //  this is the number of lobes in the EPSI sampling. For symmetric 
    //  epsi this is twice the number of frequence points (pos + neg)    
    int numFreqPts = this->GetNumEPSIFrequencyPoints(); 

    int epsiDir  = static_cast<int>( this->GetEPSIAxis() ); 

    //  Set the actual number of k-space points sampled:
    //  Get the Dimension Vector and a loop index vector of the same dimensionality
    svkDcmHeader::DimensionVector inputDimensionVector = data->GetDcmHeader()->GetDimensionIndexVector(); 
    svkDcmHeader::DimensionVector inputLoopVector = inputDimensionVector;  
    //svkDcmHeader::PrintDimensionIndexVector(&inputDimensionVector); 

    int numVoxels[3]; 
    numVoxels[0] = svkDcmHeader::GetDimensionVectorValue( &inputDimensionVector, svkDcmHeader::COL_INDEX) + 1;
    numVoxels[1] = svkDcmHeader::GetDimensionVectorValue( &inputDimensionVector, svkDcmHeader::ROW_INDEX) + 1;
    numVoxels[2] = svkDcmHeader::GetDimensionVectorValue( &inputDimensionVector, svkDcmHeader::SLICE_INDEX) + 1;
    numVoxels[ epsiDir ] = this->GetNumSamplesPerLobe();

    int numChannels = svkDcmHeader::GetDimensionVectorValue( &inputDimensionVector, svkDcmHeader::CHANNEL_INDEX) + 1; 
    //cout << "NUM CHANNELS 1" << numChannels << endl; exit(1); 

    //  Set the number of reordered voxels (target dimensionality of this method)
    int reorderedVoxels[3]; 
    reorderedVoxels[0] = numVoxels[0]; 
    reorderedVoxels[1] = numVoxels[1]; 
    reorderedVoxels[2] = numVoxels[2]; 
    reorderedVoxels[ epsiDir ] = numVoxels[epsiDir] - this->numSamplesToSkip; 

    //cout << "full dimensionality = " << reorderedVoxels[0] << " " << reorderedVoxels[1] << " " << reorderedVoxels[2] << endl;
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 

    //============================================================
    //  Reorder the data: 
    //============================================================
    int numTimePoints = 1; 
    int timePoint = 0;
    int numEPSIAcquisitions = this->GetNumEPSIAcquisitions(); 
    int numEPSIAcquisitionsPerFID = this->GetNumEPSIAcquisitionsPerFID(); 

    //============================================================
    //  create a data set to store the new reordered data arrays. 
    //  Remove all the original arrays from it. 
    //============================================================
    svkMrsImageData* reorderedImageData = svkMrsImageData::New();
    reorderedImageData->DeepCopy( data ); 
    svkImageData::RemoveArrays( reorderedImageData ); 

    //  Add an index to the data set to represent the EPSI acq, for example sym EPSI contains data 
    //  from pos and neg gradient.  Interleaved also contains 2 data sets. 
    svkDcmHeader::DimensionVector outDimensionVector = reorderedImageData->GetDcmHeader()->GetDimensionIndexVector(); 
    svkDcmHeader::SetDimensionVectorValue(&outDimensionVector, svkDcmHeader::COL_INDEX, reorderedVoxels[0] - 1); 
    svkDcmHeader::SetDimensionVectorValue(&outDimensionVector, svkDcmHeader::ROW_INDEX, reorderedVoxels[1] - 1); 
    svkDcmHeader::SetDimensionVectorValue(&outDimensionVector, svkDcmHeader::SLICE_INDEX, reorderedVoxels[2] - 1); 
    svkDcmHeader::SetDimensionVectorValue(&outDimensionVector, svkDcmHeader::TIME_INDEX, numTimePoints - 1); 
    svkDcmHeader::SetDimensionVectorValue(&outDimensionVector, svkDcmHeader::CHANNEL_INDEX, numChannels - 1); 

    reorderedImageData->GetDcmHeader()->AddDimensionIndex( &outDimensionVector, svkDcmHeader::EPSI_ACQ_INDEX, numEPSIAcquisitions - 1 ); 
    svkDcmHeader::DimensionVector outLoopVector = outDimensionVector; 


    //============================================================
    //  Preallocate data arrays for reordered data. The API only permits dynamic 
    //  assignmet at end of CellData, so for swapped cases where we need to insert 
    //  data out of order they need to be preallocated.
    //============================================================
    int numReorderedVoxels = 1;
    for (int i = 0; i < 3; i++) {
        numReorderedVoxels *= reorderedVoxels[i];  
    }

    numReorderedVoxels *= numTimePoints * numEPSIAcquisitions * numChannels; 
    svkFastCellData* cellData = static_cast<svkFastCellData*>(reorderedImageData->GetCellData()); 
    cellData->AllocateArrays( numReorderedVoxels ); 
    vtkDataArray* dataArray = NULL;
    
    for (int arrayNumber = 0; arrayNumber < numReorderedVoxels; arrayNumber++) {
        dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
        dataArray->SetNumberOfComponents( numComponents );
        dataArray->SetNumberOfTuples( numFreqPts );
        cellData->FastAddArray( dataArray );
        dataArray->FastDelete();
    }
    cellData->FinishFastAdd();


    //  GetNumber of cells in the input image to loop over for reordering:
    int numCells = hdr->GetNumberOfCells( &inputDimensionVector ); 
    //cout << "NUMCELLS: " << numCells << endl;

    int percentTmp = 0; 
    for (int cellID = 0; cellID < numCells; cellID++ ) { 

        //cout << endl;    
        //cout << "loop 1 cellID: " << cellID << endl;

        int percent =  static_cast<int>(100.*cellID/numCells )  + 1;
        if ( percent % 10  ==  0 && percent != percentTmp ) {
            cout << "reordering : " << percent << "%" << endl;
            percentTmp = percent; 
        }

        //  Get the dimensionVector index for current cell -> indexVector: 
        svkDcmHeader::GetDimensionVectorIndexFromCellID( &inputDimensionVector, &inputLoopVector, cellID ); 
        //cout << "EPSI SPECTRUM IS FROM HERE: " << endl;
        //svkDcmHeader::PrintDimensionIndexVector(&inputDimensionVector); 
        //svkDcmHeader::PrintDimensionIndexVector(&inputLoopVector); 

        //  convert these indices to a frame number
        vtkFloatArray* epsiSpectrum = static_cast<vtkFloatArray*>(svkMrsImageData::SafeDownCast(data)->GetSpectrum( cellID) );
        //vtkFloatArray* epsiSpectrumAN = static_cast<vtkFloatArray*>(svkMrsImageData::SafeDownCast(data)->GetSpectrum( &inputLoopVector) );
        //cout << "CHECK ARRAY NAME (" << cellID <<  ") : "  <<  svkMrsImageData::SafeDownCast(data)->GetArrayName(&inputLoopVector) << endl;
        //cout << "CHECK in reorder: " << *epsiSpectrum->GetTuple(0) << endl; 
        //cout << "CHECK in reorder: " << *epsiSpectrumAN->GetTuple(0) << endl; 
    
        //  define range of spatial dimension to expand (i.e. epsiAxis):  
        int xEPSI   = svkDcmHeader::GetDimensionVectorValue( &inputLoopVector, svkDcmHeader::COL_INDEX); 
        int yEPSI   = svkDcmHeader::GetDimensionVectorValue( &inputLoopVector, svkDcmHeader::ROW_INDEX); 
        int zEPSI   = svkDcmHeader::GetDimensionVectorValue( &inputLoopVector, svkDcmHeader::SLICE_INDEX); 
        int channel = svkDcmHeader::GetDimensionVectorValue( &inputLoopVector, svkDcmHeader::CHANNEL_INDEX); 

        int rangeMin[4]; 
        rangeMin[0] = xEPSI; 
        rangeMin[1] = yEPSI; 
        rangeMin[2] = zEPSI; 
        rangeMin[3] = channel; 
        int rangeMax[4]; 
        rangeMax[0] = xEPSI; 
        rangeMax[1] = yEPSI;
        rangeMax[2] = zEPSI;
        rangeMax[3] = channel;
        //  throw out the first and last points (zero crossing)
        rangeMin[epsiDir] = 0; 
        rangeMax[epsiDir] = numVoxels[epsiDir] - this->numSamplesToSkip; 

        //============================================================
        //  allocate an array for both the positive and negative lobes of 
        //  the symmetric EPSI encoding, or both interleaves for interleaved EPSI:
        //============================================================
        int lobeStride = this->GetNumSamplesPerLobe();  

        //  set the current index along the EPSI dimension
        //  Initialize first point to skip + throw out first point (zero crossing)

        rangeMax[epsiDir] -= 1;    //  other indices have zero range 
        //  first loop over epsi-kspace points for each acquisition, i.e. 2 for symmetric epsi, 1 for flyback.  
        //  For sym epsi these are the pos/neg gradient sampling.
        //  init outLoopVector to 0, then reset values that increment: 
        int numDims = outLoopVector.size(); 
        for (int dim = 0; dim < numDims; dim++) {
            svkDcmHeader::SetDimensionVectorValue(&outLoopVector, dim, 0); 
        }

        int currentAcq; 
        currentAcq = svkDcmHeader::GetDimensionVectorValue(&inputLoopVector, svkDcmHeader::EPSI_ACQ_INDEX); //testing


        int currentEPSIPt = this->firstSample;   

        for (int acq = 0; acq < numEPSIAcquisitionsPerFID; acq++ ) { //testing

            //  For symmetric EPSI, EPSI_ACQ_INDEX isn't initialized in the input data, 
            //  so set it here: 
            if ( this->epsiType == SYMMETRIC ) {
                currentAcq = acq; //testing
            }
            //cout << "ACQ: " << acq << " " << currentAcq << endl;

            //  If numAcqs Per FID = 1, then reset the currentEPSIPt being read
            //  to the start of the FID: 
            //  if ( numEPSIAcquisitionsPerFID == 1 ) 
            if ( this->epsiType == INTERLEAVED ) {
                currentEPSIPt = this->firstSample;   
            }

            //cout << "loop 3: " << rangeMin[3] << " -> "   << rangeMax[3] + 1<< endl;
            //cout << "loop 2: " << rangeMin[2] << " -> "   << rangeMax[2] + 1<< endl;
            //cout << "loop 1: " << rangeMin[1] << " -> "   << rangeMax[1] + 1<< endl;
            //cout << "loop 0: " << rangeMin[0] << " -> "   << rangeMax[0] + 1<< endl;
            for (int channel = rangeMin[3]; channel < rangeMax[3] + 1; channel++ ) { 
                for (int z = rangeMin[2]; z < rangeMax[2] + 1; z++ ) { 
                    for (int y = rangeMin[1]; y < rangeMax[1] + 1; y++ ) { 
                        for (int x = rangeMin[0]; x < rangeMax[0] + 1; x++ ) { 

                            //  Initialize the dimensionVector with the current cell array indices: 
                            svkDcmHeader::SetDimensionVectorValue(&outLoopVector, svkDcmHeader::COL_INDEX, x); 
                            svkDcmHeader::SetDimensionVectorValue(&outLoopVector, svkDcmHeader::ROW_INDEX, y); 
                            svkDcmHeader::SetDimensionVectorValue(&outLoopVector, svkDcmHeader::SLICE_INDEX, z); 
                            svkDcmHeader::SetDimensionVectorValue(&outLoopVector, svkDcmHeader::CHANNEL_INDEX, channel); 
                            svkDcmHeader::SetDimensionVectorValue(&outLoopVector, svkDcmHeader::EPSI_ACQ_INDEX, currentAcq); 

                            //  Now get the cell ID for this set of indices: 
                            int cellIDInner = svkDcmHeader::GetCellIDFromDimensionVectorIndex( &outDimensionVector, &outLoopVector ); 
                            //svkDcmHeader::PrintDimensionIndexVector(&outDimensionVector); 
                            //cout << " CELL ID: " << cellIDInner << endl;                    
                            //svkDcmHeader::PrintDimensionIndexVector(&outLoopVector); 

                            vtkDataArray* dataArray = reorderedImageData->GetCellData()->GetArray( cellIDInner );
                            reorderedImageData->SetArrayName( dataArray, &outLoopVector); 
                            //cout << "test 2 CELL ID (" << cellIDInner << "): " << x << " " << y << " " << z << " " << channel << " " << currentAcq << endl;
                            //cout << "ARRAY NAME:  " << reorderedImageData->GetArrayName(&outLoopVector) << endl;

                            //  init the dimensionVector with the current value in each dimension and pass that to 
                            //  reorder EPSI data, throwing away the first and last 
                            //  point (zero crossing). 
                            double epsiTuple[2]; 
                            double tuple[2]; 
                            int epsiOffset; 

                            for (int i = 0; i < numFreqPts; i++) {
                                epsiOffset = (lobeStride * numEPSIAcquisitionsPerFID * i ) + currentEPSIPt;
                                epsiSpectrum->GetTuple (epsiOffset, epsiTuple); 
                                tuple[0] = epsiTuple[0]; 
                                tuple[1] = epsiTuple[1]; 
                                //if ( i < 1 ) {
                                    //cout << "REORDER TUPPLE: " << epsiOffset << " " << tuple[0] << " " << tuple[1] << endl;
                                //}

                                dataArray->SetTuple( i, tuple );
                            }
                            currentEPSIPt++; // increment the k-space point on the lobe
                        }
                    }
                }
            }
            //  between lobes, throw out the last and first point before resuming sampling
            //  These are the zero crossings in symmetric EPSI. 
            currentEPSIPt += this->numSamplesToSkip; 
        }  //testing
    }

    //  =================================================
    //  Redimension the meta data and set the new arrays:
    //  =================================================
    //cout << *reorderedImageData << endl;
    data->DeepCopy( reorderedImageData ); 
    //data->DeepCopy( reorderedImageData ); 

    // new version: 
    data->GetDcmHeader()->Redimension( &outDimensionVector );
    data->GetDcmHeader()->SetValue( "DataPointColumns", numFreqPts );
    data->SyncVTKImageDataToDcmHeader();

    //cout << "FINAL" << endl;
    //svkDcmHeader::PrintDimensionIndexVector(&outDimensionVector);

    //  if interleaved EPSI, then interleave here. 
    if ( this->epsiType == INTERLEAVED ) {

        //  combine interleaved data points here.  The spectra will be doubled in size, 
        //  but EPSI index will get dropped.so.. allocate a new data 
        //  set with 1/2 as many arrays, each with twice as many points.
        svkMrsImageData* dataTmp = svkMrsImageData::New();
        dataTmp->DeepCopy( data ); 
        svkImageData::RemoveArrays( dataTmp ); 

        //  Remove the EPSI dimension from the Dimension Index
        dataTmp->GetDcmHeader()->RemoveDimensionIndex( svkDcmHeader::EPSI_ACQ_INDEX ); 
        dataTmp->GetDcmHeader()->SetValue( "DataPointColumns", numFreqPts * 2 ); 
        float originalSweepWidth = dataTmp->GetDcmHeader()->GetFloatValue("SpectralWidth"); 
        dataTmp->GetDcmHeader()->SetValue( "SpectralWidth", originalSweepWidth * 2 ); 

        //  Allocate arrays for target data set: 
        svkFastCellData* cellDataTmp = static_cast<svkFastCellData*>(dataTmp->GetCellData()); 
        cellDataTmp->AllocateArrays( numReorderedVoxels/2 ); 
        vtkDataArray* dataArray = NULL;
        for (int arrayNumber = 0; arrayNumber < numReorderedVoxels/2; arrayNumber++) {
            dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
            dataArray->SetNumberOfComponents( numComponents );
            dataArray->SetNumberOfTuples( numFreqPts * 2 );
            cellDataTmp->FastAddArray( dataArray );
            dataArray->FastDelete();
        }
        cellDataTmp->FinishFastAdd();

        svkDcmHeader::DimensionVector outputDimensions = dataTmp->GetDcmHeader()->GetDimensionIndexVector(); 
        svkDcmHeader::DimensionVector outputLoopVector = outputDimensions; 
        
        //  Now, iterate over frames for 1 EPSI index, but get spectra from both indices
        //  and interleave the data into the new data set. 
        svkDcmHeader::DimensionVector epsiDimensionVector = data->GetDcmHeader()->GetDimensionIndexVector(); 
        svkDcmHeader::DimensionVector loopVector0 = epsiDimensionVector;    // interleave 0
        svkDcmHeader::DimensionVector loopVector1;                          // interleave 1
        svkDcmHeader::SetDimensionVectorValue(&epsiDimensionVector, svkDcmHeader::EPSI_ACQ_INDEX, 0); 
        int numCells = svkDcmHeader::GetNumberOfCells( &epsiDimensionVector ); 

        int percentTmp; 
        for (int cellID = 0; cellID < numCells; cellID++ ) { 

            int percent =  static_cast<int>(100.*cellID/numCells )  + 1;
            if ( percent % 10  ==  0 && percent != percentTmp ) {
                cout << "interleaving: " << percent << "%" << endl;
                percentTmp = percent; 
            }

            //cout << "CELLID: " << cellID << endl;
            //  Get the dimensionVector index for current cell for both EPSI interleaves: 
            svkDcmHeader::GetDimensionVectorIndexFromCellID( &epsiDimensionVector, &loopVector0, cellID ); 
            loopVector1 = loopVector0; 
            svkDcmHeader::SetDimensionVectorValue(&loopVector1, svkDcmHeader::EPSI_ACQ_INDEX, 1); 

            //for all indices except EPSI, map to outputDimensions 
            for ( int i = 0; i < loopVector0.size(); i++ ) {
                svkDcmHeader::DimensionIndexLabel dimLabel = svkDcmHeader::GetDimensionLabelFromIndex( &loopVector0, i );   
                if ( dimLabel != svkDcmHeader::EPSI_ACQ_INDEX ) {
                    int dimValue = svkDcmHeader::GetDimensionVectorValue(&loopVector0, i); 
                    svkDcmHeader::SetDimensionVectorValue(&outputLoopVector, dimLabel, dimValue);         
                }
            }

            //  Now get all 3 spectra: 
            int cellID0   = svkDcmHeader::GetCellIDFromDimensionVectorIndex(&epsiDimensionVector, &loopVector0); 
            int cellID1   = svkDcmHeader::GetCellIDFromDimensionVectorIndex(&epsiDimensionVector, &loopVector1); 
            int cellIDOut = svkDcmHeader::GetCellIDFromDimensionVectorIndex(&outputDimensions, &outputLoopVector); 
            //cout << "OD: " << cellID0 << endl;
            //svkDcmHeader::PrintDimensionIndexVector(&outputDimensions);
            //cout << "OLV: " << cellID << endl;
            //svkDcmHeader::PrintDimensionIndexVector(&loopVector0);
            //svkDcmHeader::PrintDimensionIndexVector(&loopVector1);

            //cout << *dataTmp << endl;

            vtkFloatArray* epsiSpectrum0  
                    = static_cast<vtkFloatArray*>(svkMrsImageData::SafeDownCast(data)->GetSpectrum( cellID0 ) );
            vtkFloatArray* epsiSpectrum1  
                    = static_cast<vtkFloatArray*>(svkMrsImageData::SafeDownCast(data)->GetSpectrum( cellID1 ) );
            vtkFloatArray* outputSpectrum 
                    = static_cast<vtkFloatArray*>(svkMrsImageData::SafeDownCast(dataTmp)->GetSpectrum( cellIDOut ) );

            //  Loop over frequency points and interleave
            double tuple0[2]; 
            double tuple1[2]; 
            double tupleOut0[2]; 
            double tupleOut1[2]; 

            vtkDataArray* outArray = dataTmp->GetCellData()->GetArray( cellIDOut );
            dataTmp->SetArrayName( outArray, &outputLoopVector ); 

            for ( int i = 0; i < numFreqPts; i++  ) {

                epsiSpectrum0->GetTuple(i, tuple0); 
                epsiSpectrum1->GetTuple(i, tuple1); 

                tupleOut0[0] = tuple0[0]; 
                tupleOut0[1] = tuple0[1]; 
                tupleOut1[0] = tuple1[0]; 
                tupleOut1[1] = tuple1[1]; 
                //if (i < 1)     {
                    //cout << "ARRAY NAME:  " <<  dataTmp->GetArrayName(&outputLoopVector) << endl;
                    //cout << "CHECK TUPLES: " << tupleOut0[0] << " " << tupleOut0[1] << " " << tupleOut1[0] << " " << tupleOut1[1] << endl;
                //}
                outArray->SetTuple( i*2 , tupleOut0 );
                outArray->SetTuple( i*2 + 1 , tupleOut1 );

            }

        }

        //cout << *dataTmp << endl;
        data->DeepCopy( dataTmp ); 

    }

    this->UpdateReorderedParams( data, reorderedVoxels ); 

    data->GetProvenance()->AddAlgorithm( this->GetClassName() );

    reorderedImageData->Delete();

}


/*!
 *  Static method to compute 
 *  - cmplx sum of lobes 
 *  - sum of squares combination of lobes 
 */
void svkEPSIReorder::CombineLobes(svkImageData* data, bool sumOfSquares )
{

    svkDcmHeader* hdr = data->GetDcmHeader();

    int numFreqPts = hdr->GetIntValue( "DataPointColumns" );

    svkDcmHeader::DimensionVector dimVector = hdr->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector loopVectorLobe0 = dimVector;

    int numCells = svkDcmHeader::GetNumberOfCells( &dimVector );

    //  Loop over all cellss where EPSI_ACQ_INDEX = 0 and combine with EPSI_ACQ_INDEX = 1 

    double specPtLobe0[2];
    double specPtLobe1[2];
    double specPtSum[2];
    
    int lastPrint = -1;
    for (int cellIDLobe0 = 0; cellIDLobe0 < numCells; cellIDLobe0++ ) {

        //  Get the dimensionVector index for the non-dynamic image: 
        svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimVector, &loopVectorLobe0, cellIDLobe0 );

        int lobe = hdr->GetDimensionVectorValue( &loopVectorLobe0, svkDcmHeader::EPSI_ACQ_INDEX);

        if ( lobe == 0 ) {

            int completed = 100 * ( static_cast<float>(cellIDLobe0) / static_cast<float>(numCells) );
            if( completed % 5 == 0 && completed != lastPrint ) {
                cout << "combine lobes " << completed << " %" << endl;
                lastPrint = completed;
            }

            //get spectrum from both lobes 
            svkDcmHeader::DimensionVector loopVectorLobe1 = loopVectorLobe0;
            svkDcmHeader::SetDimensionVectorValue( &loopVectorLobe1, svkDcmHeader::EPSI_ACQ_INDEX, 1);

            int cellIDLobe1 = svkDcmHeader::GetCellIDFromDimensionVectorIndex( &dimVector, &loopVectorLobe1);

            vtkFloatArray* spectrumLobe0 = vtkFloatArray::SafeDownCast(
                svkMrsImageData::SafeDownCast(data)->GetSpectrum( cellIDLobe0)
            );
            vtkFloatArray* spectrumLobe1 = vtkFloatArray::SafeDownCast(
                svkMrsImageData::SafeDownCast(data)->GetSpectrum( cellIDLobe1)
            );

            for ( int freq = 0; freq < numFreqPts; freq++ ) {
                spectrumLobe0->GetTuple( freq, specPtLobe0 );
                spectrumLobe1->GetTuple( freq, specPtLobe1 );
                if ( sumOfSquares == true ) {
                    //  add the magnitude data from each lobe: 
                    specPtSum[0]  = ( specPtLobe0[0] * specPtLobe0[0] + specPtLobe0[1] * specPtLobe0[1] );
                    specPtSum[0] += ( specPtLobe1[0] * specPtLobe1[0] + specPtLobe1[1] * specPtLobe1[1] );
                    specPtSum[0]  = pow( static_cast<float>(specPtSum[0]), static_cast<float>(0.5) );
                    specPtSum[1]  = 0;
                } else {
                    specPtSum[0]  = ( specPtLobe0[0] + specPtLobe1[0] ); 
                    specPtSum[1]  = ( specPtLobe0[1] + specPtLobe1[1] ); 
                }
                spectrumLobe0->SetTuple( freq, specPtSum );
            }
        }
    }

    //  redimension here: 
    //  remove EPSI_ACQ_INDEX = 1 arrays
    //  remove dimension index for EPSI_ACQ /? 
    svkDcmHeader::DimensionVector loopVector = dimVector;
    for (int cellID = 0; cellID < numCells; cellID++ ) {

        svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimVector, &loopVector, cellID);

        int lobeNum = hdr->GetDimensionVectorValue( &loopVector, svkDcmHeader::EPSI_ACQ_INDEX);

        if ( lobeNum == 1 ) {

            int completed = 100 * ( static_cast<float>(cellID) / static_cast<float>(numCells) );
            if( completed % 5 == 0 && completed != lastPrint ) {
                cout << "combine redimension " << completed << " %" << endl;
                lastPrint = completed;
            }

            //int cellIDLobe1 = svkDcmHeader::GetCellIDFromDimensionVectorIndex( &dimVector, &loopVector); 

            string arrayName = svk4DImageData::GetArrayName( &loopVector );
            data->GetCellData()->RemoveArray( arrayName.c_str() );
        }
    }
    data->GetDcmHeader()->RemoveDimensionIndex( svkDcmHeader::EPSI_ACQ_INDEX );
}



/*!
 *  Reset the "Reordered" parameters in the header, including the sweepwidth: 
 */
void svkEPSIReorder::UpdateReorderedParams( svkImageData* data, int numVoxels[3] )
{

    float sweepWidthOriginal = data->GetDcmHeader()->GetFloatValue( "SpectralWidth" ); 
    float sweepWidth = sweepWidthOriginal / this->GetNumSamplesPerLobe(); 
    data->GetDcmHeader()->SetValue(
        "SpectralWidth",
        sweepWidth
    );


    data->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedPhaseColumns",
        numVoxels[0], 
        "SharedFunctionalGroupsSequence",
        0
    );

    data->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedPhaseRows",
        numVoxels[1], 
        "SharedFunctionalGroupsSequence",
        0
    );

    data->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedOutOfPlanePhaseSteps",
        numVoxels[2], 
        "SharedFunctionalGroupsSequence",
        0
    );

    double toplc[3];
    data->GetDcmHeader()->GetOrigin(toplc, 0);
    string toplcString = 
        svkTypeUtils::DoubleToString(toplc[0]) + '\\' +
        svkTypeUtils::DoubleToString(toplc[1]) + '\\' +
        svkTypeUtils::DoubleToString(toplc[2]); 

    data->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedTLC",
        toplcString, 
        "SharedFunctionalGroupsSequence",
        0
    );

}


/*!
 *  Set the input data type, e.g. svkMrsImageData for an MRS algorithm.
 */
int svkEPSIReorder::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


