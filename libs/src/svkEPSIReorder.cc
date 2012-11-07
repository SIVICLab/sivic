/*
 *  Copyright © 2009-2012 The Regents of the University of California.
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
#include <svkRawMapperUtils.h> 


#define UNDEFINED_EPSI_AXIS -1

using namespace svk;


vtkCxxRevisionMacro(svkEPSIReorder, "$Rev$");
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
    this->epsiType = svkEPSIReorder::UNDEFINED_EPSI_TYPE; 
    this->numSamplesPerLobe = 0; 
    this->numSamplesToSkip = 0; 
    this->epsiAxis = UNDEFINED_EPSI_AXIS;

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
void svkEPSIReorder::SetEPSIAxis( int epsiAxis )
{
    this->epsiAxis = epsiAxis;
}


/*!
 *  Get the axis index corresponding to the EPSI encoding (0,1 or 2). 
 */
int svkEPSIReorder::GetEPSIAxis()
{
    return this->epsiAxis;
}


/*!
 *  Set the type of EPSI sequence 
 */ 
void svkEPSIReorder::SetEPSIType( svkEPSIReorder::EPSIType epsiType )
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
 *  Set the number of lobes in the EPSI acquisition
 */
int svkEPSIReorder::GetNumSamplesPerLobe( )
{
    return this->numSamplesPerLobe;
}


/*!
 *  Set the number of samples to skip between lobes. Different from 
 *  the initial offset (firstSample). 
 */
void svkEPSIReorder::SetNumSamplesToSkip( int numSamplesToSkip)
{
    this->numSamplesToSkip = numSamplesToSkip; 
}

/*!
 *  Set the first sample to read when reordering the EPSI data. A few 
 *  samples may be skipped at the begining. 
 */
void svkEPSIReorder::SetFirstSample( int firstSample )
{
    this->firstSample = firstSample; 
}


/*!
 *  Set the numVoxels before reordering
 */
void svkEPSIReorder::SetNumVoxelsOriginal( int numVoxels[3] )
{
    this->numVoxelsOriginal[0] = numVoxels[0]; 
    this->numVoxelsOriginal[1] = numVoxels[1]; 
    this->numVoxelsOriginal[2] = numVoxels[2]; 
}

/*!
 *  Virtual method that returns the number of spec frequency points
 *  in the acquisition.  May be reimplemented for specific EPSI 
 *  acquisitions. 
 */
int svkEPSIReorder::GetNumEPSIFrequencyPoints()
{
    if ( this->epsiType == svkEPSIReorder::UNDEFINED_EPSI_TYPE ) {
        cout << "ERROR( " << this->GetClassName() << "): EPSI TYPE is undefined" << endl;
        exit(1); 
    } else if ( this->epsiType == svkEPSIReorder::FLYBACK ) {
        int numFreqPts = this->numLobes;     
    } else if ( this->epsiType == svkEPSIReorder::SYMMETRIC ) {
        int numFreqPts = this->numLobes / 2;     
    } else if ( this->epsiType == svkEPSIReorder::INTERLEAVED ) {
        cout << "ERROR( " << this->GetClassName() << "): INTERLEAVED EPSI not yet supported." << endl;
        exit(1); 
    } else {
        cout << "ERROR( " << this->GetClassName() << "): UNSUPPORTED EPSI TYPE " << endl;
        exit(1); 
    }
}


/*!
 *  Virtual method that returns the number of acquisitions in the 
 *  sequence.  For example symmetric EPSI contains 2, one during the 
 *  positive gradient and one during the negative gradient. 
 */
int svkEPSIReorder::GetNumEPSIAcquisitions()
{
    if ( this->epsiType == svkEPSIReorder::UNDEFINED_EPSI_TYPE ) {
        cout << "ERROR( " << this->GetClassName() << "): EPSI TYPE is undefined" << endl;
        exit(1); 
    } else if ( this->epsiType == svkEPSIReorder::FLYBACK ) {
        return 1; 
    } else if ( this->epsiType == svkEPSIReorder::SYMMETRIC ) {
        return 2; 
        int numFreqPts = this->numLobes / 2;     
    } else if ( this->epsiType == svkEPSIReorder::INTERLEAVED ) {
        cout << "ERROR( " << this->GetClassName() << "): INTERLEAVED EPSI not yet supported." << endl;
        exit(1); 
    } 
}


/*! 
 *  This method is called during pipeline execution.  Calculates the 2D array of linear phase correction factors, which
 *  are a function of the epsi k-space index, as well as the spectral index.  Applies these to all spectra in data set
 *  to generate a rectilinear spectral/spatial (k-space, time-domain) data set.  
 */
int svkEPSIReorder::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    if ( this->epsiType == UNDEFINED_EPSI_TYPE  || this->epsiAxis == UNDEFINED_EPSI_AXIS ) {
        cout << "ERROR, must specify the epsiAxis and number of sample k-space points per lobe" << endl;
        return 1; 
    }

    //  Get pointer to input data set. 
    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    this->ReorderEPSIData( mrsData ); 

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->GetInput()->Update();

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
    //  until the reordering is complette, so use these as the 
    //  target arrays. 

    //  Allocate arrays for all How many k-space points were acquired in all?

    //  Allocate arrays for spectra at each phase encode:
    svkDcmHeader* hdr = data->GetDcmHeader();
    vtkstd::string dataRepresentation = hdr->GetStringValue( "DataRepresentation" );

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

    //  Set the actual number of k-space points sampled:
    int numVoxels[3]; 
    numVoxels[0] = this->numVoxelsOriginal[0]; 
    numVoxels[1] = this->numVoxelsOriginal[1]; 
    numVoxels[2] = this->numVoxelsOriginal[2]; 
    numVoxels[ this->epsiAxis ] = this->numSamplesPerLobe; 

    //  Set the number of reordered voxels (target dimensionality of this method)
    int reorderedVoxels[3]; 
    reorderedVoxels[0] = numVoxels[0]; 
    reorderedVoxels[1] = numVoxels[1]; 
    reorderedVoxels[2] = numVoxels[2]; 
    reorderedVoxels[ this->epsiAxis ] = numVoxels[this->epsiAxis] - this->numSamplesToSkip; 

    //cout << "full dimensionality = " << numVoxels[0] << " " << numVoxels[1] << " " << numVoxels[2] << endl;
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 

    //============================================================
    //  Reorder the data: 
    //============================================================
    int numTimePoints = 1; 
    int timePoint = 0;
    int numCoils = this->GetNumEPSIAcquisitions(); 
    int coilNum = 0; 

    //============================================================
    //  create a data set to store the new reordered data arrays. 
    //  Remove all the original arrays from it. 
    //============================================================
    svkImageData* reorderedImageData = svkMrsImageData::New();
    reorderedImageData->DeepCopy( data ); 
    svkImageData::RemoveArrays( reorderedImageData ); 

    //============================================================
    //  Preallocate data arrays for reordered data. The API only permits dynamic 
    //  assignmet at end of CellData, so for swapped cases where we need to insert 
    //  data out of order they need to be preallocated.
    //============================================================
    int numReorderedVoxels = 1;
    for (int i = 0; i < 3; i++) {
        numReorderedVoxels *= reorderedVoxels[i];  
    }

    numReorderedVoxels *= numTimePoints * numCoils; //remember, numCoils is number of acquisitions 
    for (int arrayNumber = 0; arrayNumber < numReorderedVoxels; arrayNumber++) {
        vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
        dataArray->SetNumberOfComponents( numComponents );
        dataArray->SetNumberOfTuples( numFreqPts );
        reorderedImageData->GetCellData()->AddArray(dataArray);
        dataArray->Delete();
    }

    
    for (int zEPSI = 0; zEPSI < this->numVoxelsOriginal[2]; zEPSI++ ) { 
        for (int yEPSI = 0; yEPSI < this->numVoxelsOriginal[1]; yEPSI++ ) { 
            for (int xEPSI = 0; xEPSI < this->numVoxelsOriginal[0]; xEPSI++ ) { 

                vtkFloatArray* epsiSpectrum = static_cast<vtkFloatArray*>(
                        svkMrsImageData::SafeDownCast(data)->GetSpectrum( xEPSI, yEPSI, zEPSI, timePoint, 0) );
    
                //  define range of spatial dimension to expand (i.e. epsiAxis):  
                int rangeMin[3]; 
                rangeMin[0] = xEPSI; 
                rangeMin[1] = yEPSI; 
                rangeMin[2] = zEPSI; 
                int rangeMax[3]; 
                rangeMax[0] = xEPSI; 
                rangeMax[1] = yEPSI;
                rangeMax[2] = zEPSI;
                //  throw out the first and last points (zero crossing)
                rangeMin[this->epsiAxis] = 0; 
                rangeMax[this->epsiAxis] = numVoxels[this->epsiAxis] - this->numSamplesToSkip; 

                
                //============================================================
                //  allocate an array for both the positive and negative lobes of 
                //  the symmetric EPSI encoding:
                //============================================================
                int lobeStride = this->numSamplesPerLobe;  

                //  set the current index along the EPSI dimension
                //  Initialize first point to skip + throw out first point (zero crossing)
                int currentEPSIPt = this->firstSample;   

                rangeMax[this->epsiAxis] -= 1;    //  other indices have zero range 
                //  first loop over epsi-kspace points for first lobe, then for 2nd lobe, etc.
                for (int lobe = 0; lobe < 2; lobe++ ) { 
                    for (int z = rangeMin[2]; z < rangeMax[2] + 1; z++ ) { 
                        for (int y = rangeMin[1]; y < rangeMax[1] + 1; y++ ) { 
                            for (int x = rangeMin[0]; x < rangeMax[0] + 1; x++ ) { 

                                //  Add arrays to the new data set at the correct array index: 
                                int index =  x
                                          + ( reorderedVoxels[0] ) * y
                                          + ( reorderedVoxels[0] * reorderedVoxels[1] ) * z
                                          + ( reorderedVoxels[0] * reorderedVoxels[1] * reorderedVoxels[2] ) * lobe 
                                          + ( reorderedVoxels[0] * reorderedVoxels[1] * reorderedVoxels[2] * 2 ) * timePoint;
 
                                vtkDataArray* dataArray = reorderedImageData->GetCellData()->GetArray(index);

                                char arrayName[30];
                                sprintf(arrayName, "%d %d %d %d %d", x, y, z, timePoint, lobe);
                                dataArray->SetName(arrayName);

                                //  reorder EPSI data, throwng away the first and last 
                                //  point (zero crossing). 
                                float epsiTuple[2]; 
                                float tuple[2]; 
                                int epsiOffset; 
                                for (int i = 0; i < numFreqPts; i++) {
                                    epsiOffset = (lobeStride * 2 * i ) + currentEPSIPt;
                                    epsiSpectrum->GetTupleValue(epsiOffset, epsiTuple); 
                                    tuple[0] = epsiTuple[0]; 
                                    tuple[1] = epsiTuple[1]; 
                                    dataArray->SetTuple( i, tuple );
                                }

                                currentEPSIPt++; 
                            }
                        }
                    }

                    //  between lobes, throw out the last and first point before resuming sampling
                    //  These are the zero crossings in symmetric EPSI. 
                    currentEPSIPt += this->numSamplesToSkip; 
                }
            }
        }
    }


    //  =================================================
    //  Redimension the meta data and set the new arrays:
    //  =================================================
    data->DeepCopy( reorderedImageData ); 
    numVoxels[this->epsiAxis] = numVoxels[this->epsiAxis] - this->numSamplesToSkip; //   first and last point were thrown out
    int numVoxelsOriginal[3]; 
    svkRawMapperUtils::RedimensionData( data, this->numVoxelsOriginal, numVoxels, numFreqPts ); 

    reorderedImageData->Delete();

}


/*!
 *  Set the input data type, e.g. svkMrsImageData for an MRS algorithm.
 */
int svkEPSIReorder::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


