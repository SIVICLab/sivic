/*
 *  Copyright © 2009-2011 The Regents of the University of California.
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
#include <vtkDebugLeaks.h>


using namespace svk;


vtkCxxRevisionMacro(svkGEPFileMapperUCSFfidcsiDev0, "$Rev$");
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

}


/*!
 *
 */
svkGEPFileMapperUCSFfidcsiDev0::~svkGEPFileMapperUCSFfidcsiDev0()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
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
vtkstd::string  svkGEPFileMapperUCSFfidcsiDev0::GetVolumeLocalizationTechnique()
{
    vtkstd::string localizationType("NONE");
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
    selBoxSize[ 0 ] = this->GetHeaderValueAsFloat( "rhr.rh_user24" );
    selBoxSize[ 1 ] = this->GetHeaderValueAsFloat( "rhr.rh_user25" );
    selBoxSize[ 2 ] = this->GetHeaderValueAsFloat( "rhr.rh_user26" );

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

    svkImageData* tmpImageDynamic = svkMrsImageData::New();
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

        svkImageData* tmpImage = svkMrsImageData::New();
        tmpImage->DeepCopy( data ); 

        //  Reset the progress bar to local progress
        this->progress = (float)timePt/numTimePts; 
        this->UpdateProgress( this->progress );

        //  Do all the usual data loading stuff:
        this->Superclass::ReadData( tmpArray, tmpImage);  //new
        this->progress = (float)timePt/numTimePts; 
        this->UpdateProgress( this->progress );

        //  Separate out EPSI sampled data into time and k-space dimensions:
        this->ReorderEPSIData( tmpImage );
        //data->DeepCopy( tmpImage ); 

        //  copy data to tmpImageDynamic and add appropriate arrays

        if ( fileNumber == 0 ) {

            //  First time around set up the target data struct
            tmpImageDynamic->DeepCopy(tmpImage); 
            this->RemoveArrays( tmpImageDynamic ); 

            //  ========================================
            //  Preallocate all arrays for data set.  
            //  Insert single time points at correct 
            //  location in AddReorderedTimePoint 
            //  ========================================
            svkDcmHeader* hdr = tmpImage->GetDcmHeader();
            int voxels[3]; 
            voxels[0] = hdr->GetIntValue( "Columns" );
            voxels[1] = hdr->GetIntValue( "Rows" );
            voxels[2] = hdr->GetNumberOfSlices();
            int totalNumArrays = numTimePts; 
            totalNumArrays *= voxels[0] * voxels[1] * voxels[2]; 
            totalNumArrays *= 2; // numLobes 
           
            vtkstd::string dataRepresentation = hdr->GetStringValue( "DataRepresentation" );
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
        } 
        //cout << "tmpDynamic: " << *tmpImageDynamic << endl;

        this->AddReorderedTimePoint(tmpImageDynamic, tmpImage, timePt, numTimePts); 
        //cout << "tmp: " << *tmpImage << endl;
        //tmpImage->GetDcmHeader()->PrintDcmHeader() ;
        //cout << "tmpDynamic: " << *tmpImageDynamic << endl;
        //tmpImageDynamic->GetDcmHeader()->PrintDcmHeader() ;

        tmpArray->Delete();
        tmpImage->Delete();

        
    }  	 

    data->DeepCopy( tmpImageDynamic ); 

    //cout << "data: " << *data << endl;
    data->GetDcmHeader()->PrintDcmHeader();

    // now that the header and dimensionality are set correctly, reset this param:
    data->SyncVTKImageDataToDcmHeader(); 
    this->InitK0Sampled(); 
    //cout << "data: " << *data << endl;
    data->GetDcmHeader()->PrintDcmHeader();
}


/*!
 *
 */
void svkGEPFileMapperUCSFfidcsiDev0::AddReorderedTimePoint(svkImageData* dynamicImage, svkImageData* tmpImage, int timePt, int numTimePts)
{

    svkDcmHeader* hdr = tmpImage->GetDcmHeader();
    vtkstd::string dataRepresentation = hdr->GetStringValue( "DataRepresentation" );
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

    int numCoils = 2;    //  num lobes = 2

    for (int coil = 0; coil < numCoils; coil++) {
        for (int z = 0; z < voxels[2]; z++) {
            for (int y = 0; y < voxels[1]; y++) {
                for (int x = 0; x < voxels[0]; x++) {

                    //  Get index of array in target image. 
                    int targetIndex =  x
                            + ( voxels[0] ) * y
                            + ( voxels[0] * voxels[1] ) * z
                            + ( voxels[0] * voxels[1] * voxels[2] ) * timePt 
                            + ( voxels[0] * voxels[1] * voxels[2] * numTimePts ) * coil; 

                    vtkDataArray* targetDataArray = dynamicImage->GetCellData()->GetArray(targetIndex);
    
                    char targetArrayName[30];
                    sprintf(targetArrayName, "%d %d %d %d %d", x, y, z, timePt, coil);
                    //cout << "ArrayName: " << targetArrayName << endl;    
                    targetDataArray->SetName( targetArrayName );

                    //  Add arrays to the new data set at the correct array index:
                    //  Get the data array from the single time point tmpImage:     
                    int index =  x
                            + ( voxels[0] ) * y
                            + ( voxels[0] * voxels[1] ) * z
                            + ( voxels[0] * voxels[1] * voxels[2] ) * coil; 

                    vtkDataArray* tmpDataArray = tmpImage->GetCellData()->GetArray(index);

                    double* tmpTuple; 
                    float dynamicTuple[2]; 
                    for (int i = 0; i < numFreqPts; i++) {
                        tmpTuple = tmpDataArray->GetTuple(i); 
                        dynamicTuple[0] = tmpTuple[0]; 
                        dynamicTuple[1] = tmpTuple[1]; 
                        targetDataArray->SetTuple( i, dynamicTuple );
                    }    

                }
            }
        }
    }

    int currentNumTimePts = timePt + 1; 

    //  This is the original origin based on the reduced dimensionality in the EPSI direction
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
        voxels[2], 
        currentNumTimePts,
        numCoils 
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
    //cout << "Num EPSI Pts: "<< numEPSIPts << endl;

    //  this is the number of lobes in the EPSI sampling. For symmetric 
    //  epsi this is twice the number of frequence points (pos + neg)    
    int numSymmetricEPSILobes = this->GetHeaderValueAsInt( "rhr.rh_user10" );
    int numFreqPts = numSymmetricEPSILobes / 2;     
    //cout << "Num Freq Pts: "<< numFreqPts << endl;


    //============================================================
    //  Read EPSI acquisition params from raw header: 
    //============================================================

    //  dwell time time between k-space points
    int deltaT = this->GetHeaderValueAsInt( "rhr.rh_user12" );

    //  time for plateau encoding (gradient duratation)
    int plateauTime = this->GetHeaderValueAsInt( "rhr.rh_user13" );

    //  time for ramp in one direction
    int rampTime = this->GetHeaderValueAsInt( "rhr.rh_user15" );

    //  number of samples at start
    int numSkip = this->GetHeaderValueAsInt( "rhr.rh_user22" );

    //  number of samples per lobe (ramps + plateau)  
    //  num spectral samples in FID is num time_pts / this value (
    int numRead = (plateauTime + 2 * rampTime)/deltaT;

    //============================================================
    //Q:  is the number of EPSI phase encodes equal to numRead or numEPSIkSpacePoints?
    //  I think it starts at numRead, and gets sampled down to numEPSIkSpacePoints (23-18)
    //  after ramp sampling... i think
    //============================================================
    //===========
    //  Reset hdr value (num cols?)
    //===========
    int numEPSIkSpacePts = this->GetHeaderValueAsInt( "rhr.rh_user9" );

    //  EPSI Axis (user9) defines which axis is epsi encoded.  Swap the value of 
    //  epsi k-space encodes into this field: 
    int epsiAxis = this->GetHeaderValueAsInt( "rhr.rh_user20" ) - 1;

    //  Get the input dimensionality (EPSI)
    int numEPSIVoxels[3]; 
    this->GetNumVoxels( numEPSIVoxels ); 

    //  Set the actual number of k-space points sampled:
    int numVoxels[3]; 
    numVoxels[0] = numEPSIVoxels[0]; 
    numVoxels[1] = numEPSIVoxels[1]; 
    numVoxels[2] = numEPSIVoxels[2]; 
    numVoxels[ epsiAxis ] = numRead; 
    //numVoxels[ epsiAxis ] = numEPSIkSpacePts; 

    //  Set the number of reordered voxels (target dimensionality of this method)
    int reorderedVoxels[3]; 
    reorderedVoxels[0] = numVoxels[0]; 
    reorderedVoxels[1] = numVoxels[1]; 
    reorderedVoxels[2] = numVoxels[2]; 
    reorderedVoxels[ epsiAxis ] = numVoxels[epsiAxis] - 2; //throw out first and last samples

    //cout << "full dimensionality = " << numVoxels[0] << " " << numVoxels[1] << " " << numVoxels[2] << endl;
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 

    //============================================================
    //  Reorder the data: 
    //============================================================
    int numCoils = 1; 
    int numTimePts = 1; 
    int timePoint = numTimePts - 1; 
    int coilNum = numCoils - 1; 

    //============================================================
    //  create a data set to store the new reordered data arrays. 
    //  Remove all the original arrays from it. 
    //============================================================
    svkImageData* reorderedImageData = svkMrsImageData::New();
    reorderedImageData->DeepCopy( data ); 
    this->RemoveArrays( reorderedImageData ); 

    //============================================================
    //  Preallocate data arrays for reordered data. The API only permits dynamic 
    //  assignmet at end of CellData, so for swapped cases where we need to insert 
    //  data out of order they need to be preallocated.
    //============================================================
    int numReorderedVoxels = 1;
    for (int i = 0; i < 3; i++) {
        numReorderedVoxels *= reorderedVoxels[i];  
    }
    numReorderedVoxels *= 2 * numCoils; //2 lobes
    for (int arrayNumber = 0; arrayNumber < numReorderedVoxels; arrayNumber++) {
        vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
        dataArray->SetNumberOfComponents( numComponents );
        dataArray->SetNumberOfTuples( numFreqPts );
        reorderedImageData->GetCellData()->AddArray(dataArray);
        dataArray->Delete();
    }

    
    for (int zEPSI = 0; zEPSI < numEPSIVoxels[2]; zEPSI++ ) { 
        for (int yEPSI = 0; yEPSI < numEPSIVoxels[1]; yEPSI++ ) { 
            for (int xEPSI = 0; xEPSI < numEPSIVoxels[0]; xEPSI++ ) { 

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
                rangeMin[epsiAxis] = 0; 
                rangeMax[epsiAxis] = numVoxels[epsiAxis] - 2; 

                
                //============================================================
                //  allocate an array for both the positive and negative lobes of 
                //  the symmetric EPSI encoding:
                //============================================================
                int lobeStride = numRead;  

                //  set the current index along the EPSI dimension
                //  Initialize first point to skip + throw out first point (zero crossing)
                int currentEPSIPt = numSkip + 1;   

                rangeMax[epsiAxis] -= 1;    //  other indices have zero range 
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
 
                                //cout << "   target index(xyzlobe): " << x << " " 
                                    //<< y << " " << z << " " << lobe  << " index: " << index << endl;
                                vtkDataArray* dataArray = reorderedImageData->GetCellData()->GetArray(index);

                                char arrayName[30];
                                sprintf(arrayName, "%d %d %d %d %d", x, y, z, timePoint, lobe);
                                //cout << "reordered Array name (" << index << ") = " << arrayName << endl; 
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

                                    //cout << "       extract spectrum  epsiOffset: " 
                                    //<< epsiOffset << " target index " << i << endl;
                                    //if ( i <  10 ) { 
                                        //cout << "fist 2 values or val0:  " << epsiTuple[0] << " " << epsiTuple[1] << endl;                                     //}
                                }

                                currentEPSIPt++; 
                            }
                        }
                    }
                    //  between lobes, throw out the last and first point before resuming sampling
                    //  These are the zero crossings in symmetric EPSI. 
                    currentEPSIPt +=2; 
                }
            }
        }
    }


    //  =================================================
    //  Redimension the meta data and set the new arrays:
    //  =================================================
    data->DeepCopy( reorderedImageData ); 
    numVoxels[epsiAxis] = numVoxels[epsiAxis] - 2; //   first and last point were thrown out
    int numVoxelsOriginal[3]; 
    this->GetNumVoxels( numVoxelsOriginal ); 
    this->RedimensionData( data, numVoxelsOriginal, numVoxels, numFreqPts ); 

    //  =================================================
    //  Apply linear phase correction to correct for EPSI sampling of 
    //  spectra: 
    //      foreach k-space point, apply a phase correction to each complex spectral point
    //          the correction is a function of the distance along the epsi-axis (k/t) with pivots 
    //          at the center of each dimension
    //  =================================================
    this->EPSIPhaseCorrection( data, numVoxels, numRead, epsiAxis);  
 
    //  =================================================
    //  reverse odd lobe k-space spectra along epsi axis
    //  =================================================
    this->ReverseOddEPSILobe( data, epsiAxis ); 

    //  =================================================
    //  resample ramp data 
    //  =================================================
    this->ResampleRamps( data, deltaT, plateauTime, rampTime, epsiAxis ); 

    //  =================================================
    //  if feet first entry, reverse RL and SI direction: 
    //  =================================================
    this->FlipAxis( data, 0); 
    this->FlipAxis( data, 2); 

    //  =================================================
    //  FFTShift:  put the origin of k-space at the image
    //  center so it's suitable for standard downstream 
    //  FFT.  Seems to be the case already
    //  =================================================
    //this->FFTShift( data ); 
   
    //  =================================================
    //  Zero fill in frequency domain (should be moved to
    //  post-processing step)
    //  =================================================
    //this->ZeroFill(data);

    //  =================================================
    //  combine even/odd lobes (separate post-processing 
    //  step). 
    //  =================================================


    //  =================================================
    //  Account for patient entry.. need to review, not sure this
    //   is necessary after the axis flips above. 
    //  =================================================
    this->ModifyForPatientEntry(data); 

    this->InitK0Sampled();

    reorderedImageData->Delete(); 

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
    epsiPhase->SetNumEPSIkRead( numRead );
    epsiPhase->SetEPSIAxis( epsiAxis );
    epsiPhase->SetInput( tmpData ); 
    epsiPhase->Update(); 

    data->DeepCopy( epsiPhase->GetOutput() ); 

    epsiPhase->Delete(); 
    tmpData->Delete(); 

}


/*!
 *  Reverses the specified axis. 
 */
void svkGEPFileMapperUCSFfidcsiDev0::FlipAxis( svkImageData* data, int axis ) 
{

    svkMrsImageData* tmpData = svkMrsImageData::New();
    tmpData->DeepCopy( data ); 

    svkMrsImageFlip* flip = svkMrsImageFlip::New(); 
    flip->SetFilteredAxis( axis ); 
    flip->SetInput( tmpData ); 
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
void svkGEPFileMapperUCSFfidcsiDev0::ResampleRamps( svkImageData* data, int deltaT, int plateauTime, int rampTime, int epsiAxis )
{
    //  get the EPSI sampling waveform: 
    svkDcmHeader* hdr = data->GetDcmHeader(); 
    int numVoxels[3];  
    numVoxels[0] = hdr->GetIntValue( "Columns" );
    numVoxels[1] = hdr->GetIntValue( "Rows" );
    numVoxels[2] = hdr->GetNumberOfSlices();

    int numEPSIVoxels = numVoxels[ epsiAxis ]; 

    float* waveFormIntegral = new float[ numEPSIVoxels ]; 
    this->GetWaveFormIntegral( waveFormIntegral, deltaT, plateauTime, rampTime ); 

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

        vtkstd::vector <int> iGrid; 
        vtkstd::vector <float> iGridDK; 

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
        vtkstd::vector<float> kbVals; 
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

    float* epsiKData = new float[ numEPSIVoxels * 2 ];
    float* overgrid = new float[ gridSize *2 ];
    for ( int lobe = 0; lobe < 2; lobe++) {
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

                            vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( 
                                svkMrsImageData::SafeDownCast(data)->GetSpectrum( col, row, slice, 0, lobe) 
                            );

                            epsiKData[ epsiK * 2 ]     = spectrum->GetValue( freq * 2 ); 
                            epsiKData[ epsiK * 2 + 1 ] = spectrum->GetValue( freq * 2 + 1 ); 
                            
                        }

                        // ========================================================
                        //  Calculate the overgrid vector here for each epsi row
                        //  fn_overgrid in MATLAB
                        // ========================================================
                        for ( int i = 0; i < gridSize; i++ ) {
                            overgrid[ i*2 ] = 0;  
                            overgrid[ i*2 + 1 ] = 0;  
                            for ( int j = 0; j < numEPSIVoxels; j++) {
                                overgrid[ i*2 ]    += grid[i][j] * epsiKData[ j * 2 ];
                                overgrid[ i*2 + 1] += grid[i][j] * epsiKData[ j * 2 + 1];
                            }
                        }

                        //================================================
                        //  ifft overgrid and divide by apodCor
                        //      -> convert to vtkImageComplex
                        //      ->  ifftShift (k=0 to origin)
                        //      ->  ifft   (k to spatial domain) 
                        //      ->  fftShift ( x=0 to middle)
                        //================================================
                        vtkImageComplex* epsiKData = new vtkImageComplex[ gridSize ];
                        vtkImageComplex* epsiXData = new vtkImageComplex[ gridSize ];
                        for( int i = 0; i < gridSize; i++ ) {
                            epsiKData[i].Real = overgrid[i*2];
                            epsiKData[i].Imag = overgrid[i*2+1];
                        }
                        svkMrsImageFFT::IFFTShift( epsiKData, gridSize );
                        vtkImageFourierFilter* rfft = vtkImageRFFT::New();
                        rfft->ExecuteRfft( epsiKData, epsiXData, gridSize );
                        svkMrsImageFFT::FFTShift( epsiXData, gridSize );

                        for( int i = 0; i < gridSize; i++ ) {
                            epsiXData[i].Real = epsiXData[i].Real / apodCor[i]; 
                            epsiXData[i].Imag = epsiXData[i].Imag / apodCor[i]; 
                        }

                        int offset = static_cast<int>(floor(integralMax/2.)); 
                        for( int i = 0; i < gridSize; i++ ) {
                            epsiXData[i].Real = epsiXData[i + offset].Real; 
                            epsiXData[i].Imag = epsiXData[i + offset].Imag; 
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

                            vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( 
                                svkMrsImageData::SafeDownCast(data)->GetSpectrum( col, row, slice, 0, lobe) 
                            );

                            tuple[0] = epsiKData[k].Real; 
                            tuple[1] = epsiKData[k].Imag; 
                            spectrum->SetTuple( k, tuple );
                            
                            //cout << "regridded tupple: " << tuple[0] << " + " << tuple[1] << endl;
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

    //  redimension the data set in the EPSI k-space axis (18 voxels, epsiKData):  
    //  Remove all arrays with epsiAxis dimension greater
    //  than  integralMax, and reinit the DcmHeader!!!
    int numTimePoints = data->GetDcmHeader()->GetNumberOfTimePoints();
    for ( int lobe = 0; lobe < 2; lobe++) { // lobes are stored as coils or channels
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

                            char arrayName[30];
                            sprintf(arrayName, "%d %d %d %d %d", col, row, slice, timePt, lobe);
                            //cout << "REGRID remove array: " << arrayName << endl;
                            data->GetCellData()->RemoveArray( arrayName );
                        }
                    }
                }
            }
        }
    }

    // Now reinit the DICOM header
    regridDims[epsiAxis] = integralMax; 
    this->RedimensionData( data, numVoxels, regridDims, numSpecPts); 

    delete [] waveFormIntegralNorm;

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
            apodCor[i] = sin( arg[0] ) / arg[0];
        } else {
            // all imag 
            arg[1] = static_cast<float>( pow( static_cast<double>( -1 * arg[0]), 0.5) ); 
            arg[0] = 0;     
            apodCor[i] = sinh( arg[1] ) / arg[1];
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
void svkGEPFileMapperUCSFfidcsiDev0::GetKaiserBesselValues( vtkstd::vector<float>* u, float width, float beta, vtkstd::vector<float>* kbVals )
{

    //  vector to hold indices for computing bessel coefficients. 
    vtkstd::vector <int> uz; 
    //vtkstd::vector<float> kb_vals; 
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

            //cout << "KB: " << (*kbVals)[i] << endl;

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
    besselTerm *= pow( (arg/2), 2*index ); 
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
void svkGEPFileMapperUCSFfidcsiDev0::GetWaveFormIntegral( float* waveFormIntegral, int deltaT, int plateauTime, int rampTime )
{
    //  4 microsecond sampling:     
    int sampleDeltaT= 4; 
    int numRampSamples = rampTime / sampleDeltaT;  
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
    int binSize = deltaT / sampleDeltaT; 
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
    fftShift->SetInput( tmpData ); 
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
 *  Reverse the k-space data along epsi axis in odd lobe.  
 */
void svkGEPFileMapperUCSFfidcsiDev0::ReverseOddEPSILobe( svkImageData* data, int epsiAxis )
{

    //  Make a target object to write reversed data into
    svkMrsImageData* reversedImageData = svkMrsImageData::New();
    reversedImageData->DeepCopy( data ); 
    this->RemoveArrays( reversedImageData ); 
    int numVoxels[3]; 
    data->GetNumberOfVoxels( numVoxels ); 

    //  allocate new arrays for reversed data: 
    for (int arrayNumber = 0; arrayNumber < data->GetCellData()->GetNumberOfArrays(); arrayNumber++) {
        vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
        dataArray->SetNumberOfComponents( 2 ); 
        dataArray->SetNumberOfTuples( data->GetCellData()->GetNumberOfTuples() );
        reversedImageData->GetCellData()->AddArray(dataArray);
        dataArray->Delete();
    }

    //  Lookup any data set attributes from header required for algorithm (See DICOM IOD for field names):
    svkDcmHeader* hdr = data->GetDcmHeader(); 
    int numSpecPts = hdr->GetIntValue( "DataPointColumns" );
    int cols       = hdr->GetIntValue( "Columns" );
    int rows       = hdr->GetIntValue( "Rows" );
    int slices     = hdr->GetNumberOfSlices();
    int numLobes   = hdr->GetNumberOfCoils();  // e.g. symmetric EPSI has pos + neg lobes
    int timePt     = 0; 

    //  Do not loop over epsiAxis:
    int loopLimits[3]; 
    loopLimits[0] = cols ; 
    loopLimits[1] = rows; 
    loopLimits[2] = slices; 
    int epsiAxisLimit; 
    if ( epsiAxis == 0 ) {
        loopLimits[0] = 1; 
        epsiAxisLimit = cols; 
    } else if ( epsiAxis == 1 ) {
        loopLimits[1] = 1; 
        epsiAxisLimit = rows; 
    } else if ( epsiAxis == 2 ) {
        loopLimits[2] = 1; 
        epsiAxisLimit = slices; 
    }

    //  Iterate through 3D spatial locations
    int xOutEven;     
    int yOutEven;     
    int zOutEven;     
    int xOutOdd;     
    int yOutOdd;     
    int zOutOdd;     
    for (int lobe = 0; lobe < numLobes-1; lobe++) {   // odd lobe only
        for (int z = 0; z < loopLimits[2]; z++) {
            for (int y = 0; y < loopLimits[1]; y++) {
                for (int x = 0; x < loopLimits[0]; x++) {

                    //  for the given non EPSI indices, loop over the epsi direction and reverse the 
                    //  array ordering
                    for ( int epsiAxisIndex = 0; epsiAxisIndex < epsiAxisLimit; epsiAxisIndex++ ) {
                        
                        xOutEven = x;
                        yOutEven = y;
                        zOutEven = z;
                        //  Index along epsiAxis used to get appropriate kPhaseArray values along epsiAxis
                        if ( epsiAxis == 0 ) {
                            x = epsiAxisIndex; 
                            xOutEven = epsiAxisLimit - epsiAxisIndex - 1; 
                        } else if ( epsiAxis == 1 ) {
                            y = epsiAxisIndex; 
                            yOutEven = epsiAxisLimit - epsiAxisIndex - 1; 
                        } else if ( epsiAxis == 2 ) {
                            z = epsiAxisIndex; 
                            zOutEven = epsiAxisLimit - epsiAxisIndex - 1; 
                        }
                        xOutOdd = x; 
                        yOutOdd = y; 
                        zOutOdd = z; 

                        int indexInEven =  x
                                  + ( numVoxels[0] ) * y
                                  + ( numVoxels[0] * numVoxels[1] ) * z
                                  + ( numVoxels[0] * numVoxels[1] * numVoxels[2] ) * (lobe + 1)
                                  + ( numVoxels[0] * numVoxels[1] * numVoxels[2] * 2 ) * timePt;
                        int indexInOdd =  x
                                  + ( numVoxels[0] ) * y
                                  + ( numVoxels[0] * numVoxels[1] ) * z
                                  + ( numVoxels[0] * numVoxels[1] * numVoxels[2] ) * lobe 
                                  + ( numVoxels[0] * numVoxels[1] * numVoxels[2] * 2 ) * timePt;
                        int indexOutEven = xOutEven
                                  + ( numVoxels[0] ) * yOutEven
                                  + ( numVoxels[0] * numVoxels[1] ) * zOutEven
                                  + ( numVoxels[0] * numVoxels[1] * numVoxels[2] ) * (lobe + 1) 
                                  + ( numVoxels[0] * numVoxels[1] * numVoxels[2] * 2 ) * timePt;
                        int indexOutOdd =  xOutOdd
                                  + ( numVoxels[0] ) * yOutOdd
                                  + ( numVoxels[0] * numVoxels[1] ) * zOutOdd
                                  + ( numVoxels[0] * numVoxels[1] * numVoxels[2] ) * lobe 
                                  + ( numVoxels[0] * numVoxels[1] * numVoxels[2] * 2 ) * timePt;

                        vtkFloatArray* dataArrayInEven  = vtkFloatArray::SafeDownCast( 
                                                                data->GetCellData()->GetArray( indexInEven ) 
                                                            );
                        vtkFloatArray* dataArrayInOdd   = vtkFloatArray::SafeDownCast( 
                                                                data->GetCellData()->GetArray( indexInOdd) 
                                                            );
                        vtkFloatArray* dataArrayOutEven = vtkFloatArray::SafeDownCast( 
                                                                reversedImageData->GetCellData()->GetArray( indexOutEven ) 
                                                            );
                        vtkFloatArray* dataArrayOutOdd  = vtkFloatArray::SafeDownCast( 
                                                                reversedImageData->GetCellData()->GetArray( indexOutOdd ) 
                                                            );

                        char arrayNameEven[30];
                        sprintf(arrayNameEven, "%d %d %d %d %d", xOutEven, yOutEven, zOutEven, timePt, lobe + 1);
                        dataArrayOutEven->SetName(arrayNameEven);
                        char arrayNameOdd[30];
                        sprintf(arrayNameOdd, "%d %d %d %d %d", xOutOdd, yOutOdd, zOutOdd, timePt, lobe);
                        dataArrayOutOdd->SetName(arrayNameOdd);

                        float tupleEven[2];
                        float tupleOdd[2];
                        for ( int f = 0 ; f < numSpecPts;  f++ ) {
                            dataArrayInEven->GetTupleValue( f, tupleEven );
                            dataArrayInOdd->GetTupleValue( f, tupleOdd);
                            dataArrayOutEven->SetTuple( f, tupleEven );
                            dataArrayOutOdd->SetTuple( f, tupleOdd );
                        }

                    }
                }
            }
        }
    }

    data->DeepCopy( reversedImageData ); 
    data->Modified();
    reversedImageData->Delete(); 
}


/*!
 *  Remove the original EPSI arrays from the svkImageData. 
 *  at this point. 
 */
void svkGEPFileMapperUCSFfidcsiDev0::RemoveArrays( svkImageData* data )
{

    int numCoils = 2;//data->GetDcmHeader()->GetNumberOfCoils();
    int numTimePts = data->GetDcmHeader()->GetNumberOfTimePoints(); 

    //  get the original EPSI dimensionality:
    int numVoxels[3];
    data->GetNumberOfVoxels(numVoxels);

    //  Remove all original arrays
    for (int coilNum = 0; coilNum < numCoils; coilNum++) {
        for (int timePt = 0; timePt < numTimePts; timePt++) {
            for (int z = 0; z < numVoxels[2]; z++) {
                for (int y = 0; y < numVoxels[1]; y++) {
                    for (int x = 0; x < numVoxels[0]; x++) {
                        char arrayName[30];
                        sprintf(arrayName, "%d %d %d %d %d", x, y, z, timePt, coilNum);
                        //cout << "remove array: " << arrayName << endl;
                        data->GetCellData()->RemoveArray( arrayName );
                    }
                }
            }
        }
    }
}



/*!
 *  Redimension after reordering epsi dimension.  Should have 2 lobes
 *  at this point. 
 */
void svkGEPFileMapperUCSFfidcsiDev0::RedimensionData( svkImageData* data, int* numVoxelsOriginal, int* numVoxelsReordered, int numFreqPts )
{

    int numTimePts = data->GetDcmHeader()->GetNumberOfTimePoints();
    int numCoils = 2; //data->GetDcmHeader()->GetNumberOfCoils(); //num lobes

    //  This is the original origin based on the reduced dimensionality in the EPSI direction
    double origin[3];
    data->GetDcmHeader()->GetOrigin( origin, 0 );

    double voxelSpacing[3];
    data->GetDcmHeader()->GetPixelSpacing( voxelSpacing );

    double dcos[3][3];
    data->GetDcmHeader()->GetDataDcos( dcos );

    double center[3]; 
    this->GetCenterFromOrigin( origin, numVoxelsOriginal, voxelSpacing, dcos, center); 

    //  Now calcuate the new origin based on the reordered dimensionality: 
    double newOrigin[3]; 
    this->GetOriginFromCenter( center, numVoxelsReordered, voxelSpacing, dcos, newOrigin ); 

    svkDcmHeader* hdr = data->GetDcmHeader();     
    hdr->SetValue( "Columns", numVoxelsReordered[0]);
    hdr->SetValue( "Rows", numVoxelsReordered[1]);
    hdr->SetValue( "DataPointColumns", numFreqPts );

    data->GetDcmHeader()->InitPerFrameFunctionalGroupSequence(
        newOrigin,
        voxelSpacing,
        dcos,
        numVoxelsReordered[2], 
        numTimePts,
        numCoils 
    );

    data->SyncVTKImageDataToDcmHeader(); 
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
    this->GetCenterFromOrigin( origin, numVoxels, voxelSpacing, dcos, center); 

    //  If feet first, swap LR & SI (first and third columns of 
    //  LPS/col,row, slice dcos matrix: 

    if( patientPosition.find("FF") != string::npos ) {

        //dcos[0][0] *=-1;
        //dcos[1][0] *=-1;
        //dcos[2][0] *=-1;

        //dcos[0][2] *=-1;
        //dcos[1][2] *=-1;
        //dcos[2][2] *=-1;

        dcos[0][0] *=-1;
        dcos[0][1] *=-1;
        dcos[0][2] *=-1;

        dcos[2][0] *=-1;
        dcos[2][1] *=-1;
        dcos[2][2] *=-1;
    }


    //  Now calcuate the new origin based on the new dcos: 
    double newOrigin[3]; 
    this->GetOriginFromCenter( center, numVoxels, voxelSpacing, dcos, newOrigin ); 


    data->GetDcmHeader()->InitPerFrameFunctionalGroupSequence(
        newOrigin,
        voxelSpacing,
        dcos,
        data->GetDcmHeader()->GetNumberOfSlices(),
        data->GetDcmHeader()->GetNumberOfTimePoints(),
        data->GetDcmHeader()->GetNumberOfCoils()
    );

    data->SyncVTKImageDataToDcmHeader(); 

}
