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
void svkGEPFileMapperUCSFfidcsiDev0::ReadData(vtkstd::string pFileName, svkImageData* data)
{
    //  Do all the usual data loading stuff:
    this->Superclass::ReadData( pFileName, data );

    //  Separate out EPSI sampled data into time and k-space dimensions:
    this->ReorderEPSIData( data );

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
    vtkstd::string dataRepresentation = this->dcmHeader->GetStringValue( "DataRepresentation" );
    int numComponents;
    if ( dataRepresentation == "COMPLEX" ) {
        numComponents = 2;
    } else {
        numComponents = 1;
    }

    //===========
    //  Reset hdr value (DataPointColumns)
    //===========
    int numEPSIPts= this->dcmHeader->GetIntValue( "DataPointColumns" );
    cout << "Num EPSI Pts: "<< numEPSIPts << endl;

    //  this is the number of lobes in the EPSI sampling. For symmetric 
    //  epsi this is twice the number of frequence points (pos + neg)    
    int numSymmetricEPSILobes = this->GetHeaderValueAsInt( "rhr.rh_user10" );
    int numFreqPts = numSymmetricEPSILobes / 2;     
    cout << "Num Freq Pts: "<< numFreqPts << endl;


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

    cout << "full dimensionality = " << numVoxels[0] << " " << numVoxels[1] << " " << numVoxels[2] << endl;
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
                                          + ( reorderedVoxels[0] * reorderedVoxels[1] * reorderedVoxels[2] * 2 ) * coilNum;
 
                                //cout << "   target index(xyzlobe): " << x << " " 
                                    //<< y << " " << z << " " << lobe  << " index: " << index << endl;
                                vtkDataArray* dataArray = reorderedImageData->GetCellData()->GetArray(index);

                                char arrayName[30];
                                sprintf(arrayName, "%d %d %d %d %d", x, y, z, lobe, coilNum);
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
    this->RedimensionData( data, numVoxels, numFreqPts ); 


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
    //  if feet first entry, reverse LP and SI direction: 
    //  =================================================
    this->FlipAxis( data, 0); 
    this->FlipAxis( data, 2); 


    //  =================================================
    //  resample ramp data 
    //  =================================================


    //  =================================================
    //  Zero fill in frequency domain
    //  =================================================
    this->ZeroFill(data); 


    //  =================================================
    //  combine even/odd lobes
    //  =================================================


    //  =================================================
    //  Account for patient entry
    //  =================================================
//    this->ModifyForPatientEntry(data); 

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

                    vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( svkMrsImageData::SafeDownCast(data)->GetSpectrum( x, y, z, lobe, 0) );

                    //  Iterate over frequency points in spectrum and apply phase correction:
                    for ( int freq = numSpecPts; freq < 256; freq++ ) {

                        spectrum->InsertTuple(freq, cmplxPt);

                    }
                }
            }
        }
    }

    this->dcmHeader->SetValue( "DataPointColumns", 256);

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
    int numLobes   = hdr->GetNumberOfTimePoints();  // e.g. symmetric EPSI has pos + neg lobes
    int coilNum    = 0; 

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
                        
                        xOutOdd = x; 
                        yOutOdd = y; 
                        zOutOdd = z; 
                        //  Index along epsiAxis used to get appropriate kPhaseArray values along epsiAxis
                        if ( epsiAxis == 0 ) {
                            x = epsiAxisIndex; 
                            xOutOdd = epsiAxisLimit - epsiAxisIndex - 1; 
                        } else if ( epsiAxis == 1 ) {
                            y = epsiAxisIndex; 
                            yOutOdd = epsiAxisLimit - epsiAxisIndex - 1; 
                        } else if ( epsiAxis == 2 ) {
                            z = epsiAxisIndex; 
                            zOutOdd = epsiAxisLimit - epsiAxisIndex - 1; 
                        }
                        xOutEven = x; 
                        yOutEven = y; 
                        zOutEven = z; 

                        int indexInEven =  x
                                  + ( numVoxels[0] ) * y
                                  + ( numVoxels[0] * numVoxels[1] ) * z
                                  + ( numVoxels[0] * numVoxels[1] * numVoxels[2] ) * (lobe + 1)
                                  + ( numVoxels[0] * numVoxels[1] * numVoxels[2] * 2 ) * coilNum;
                        int indexInOdd =  x
                                  + ( numVoxels[0] ) * y
                                  + ( numVoxels[0] * numVoxels[1] ) * z
                                  + ( numVoxels[0] * numVoxels[1] * numVoxels[2] ) * lobe 
                                  + ( numVoxels[0] * numVoxels[1] * numVoxels[2] * 2 ) * coilNum;
                        int indexOutEven = xOutEven
                                  + ( numVoxels[0] ) * yOutEven
                                  + ( numVoxels[0] * numVoxels[1] ) * zOutEven
                                  + ( numVoxels[0] * numVoxels[1] * numVoxels[2] ) * (lobe + 1) 
                                  + ( numVoxels[0] * numVoxels[1] * numVoxels[2] * 2 ) * coilNum;
                        int indexOutOdd =  xOutOdd
                                  + ( numVoxels[0] ) * yOutOdd
                                  + ( numVoxels[0] * numVoxels[1] ) * zOutOdd
                                  + ( numVoxels[0] * numVoxels[1] * numVoxels[2] ) * lobe 
                                  + ( numVoxels[0] * numVoxels[1] * numVoxels[2] * 2 ) * coilNum;

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
                        sprintf(arrayNameEven, "%d %d %d %d %d", xOutEven, yOutEven, zOutEven, lobe + 1, coilNum);
                        dataArrayOutEven->SetName(arrayNameEven);
                        char arrayNameOdd[30];
                        sprintf(arrayNameOdd, "%d %d %d %d %d", xOutOdd, yOutOdd, zOutOdd, lobe, coilNum);
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

    int numCoils = data->GetDcmHeader()->GetNumberOfCoils();
    int numTimePts = 2; 

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
void svkGEPFileMapperUCSFfidcsiDev0::RedimensionData( svkImageData* data, int* numVoxelsReordered, int numFreqPts )
{

    int numCoils = data->GetDcmHeader()->GetNumberOfCoils();
    int numTimePts = 2; //num lobes

    //  This is the original origin based on the reduced dimensionality in the EPSI direction
    double origin[3];
    data->GetDcmHeader()->GetOrigin( origin, 0 );

    double voxelSpacing[3];
    data->GetDcmHeader()->GetPixelSpacing( voxelSpacing );

    double dcos[3][3];
    data->GetDcmHeader()->GetDataDcos( dcos );

    int numOriginalVoxels[3]; 
    this->GetNumVoxels( numOriginalVoxels ); 

    double center[3]; 
    this->GetCenterFromOrigin( origin, numOriginalVoxels, voxelSpacing, dcos, center); 

    //  Now calcuate the new origin based on the reordered dimensionality: 
    double newOrigin[3]; 
    this->GetOriginFromCenter( center, numVoxelsReordered, voxelSpacing, dcos, newOrigin ); 

    this->dcmHeader->SetValue( "Columns", numVoxelsReordered[0]);
    this->dcmHeader->SetValue( "Rows", numVoxelsReordered[1]);
    this->dcmHeader->SetValue( "DataPointColumns", numFreqPts );

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

    int numVoxels[3]; 
    numVoxels[0] = this->dcmHeader->GetIntValue( "Columns" ); 
    numVoxels[1] = this->dcmHeader->GetIntValue( "Rows" ); 
    numVoxels[2] = this->dcmHeader->GetNumberOfSlices();

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
