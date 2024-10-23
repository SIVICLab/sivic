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


#include <svkVarianUCSFEPSI2DMapper.h>
#include <svkVarianReader.h>
#include <svkMrsImageData.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkTransform.h>
#include </usr/include/vtk/vtkMatrix4x4.h>
#include </usr/include/vtk/vtkByteSwap.h>


using namespace svk;


//vtkCxxRevisionMacro(svkVarianUCSFEPSI2DMapper, "$Rev$");
vtkStandardNewMacro(svkVarianUCSFEPSI2DMapper);



/*!
 *
 */
svkVarianUCSFEPSI2DMapper::svkVarianUCSFEPSI2DMapper()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkVarianUCSFEPSI2DMapper");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->specData = NULL;

}


/*!
 *
 */
svkVarianUCSFEPSI2DMapper::~svkVarianUCSFEPSI2DMapper()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if ( this->specData != NULL )  {
        delete [] specData;
        this->specData = NULL;
    }

}


/*
 * Gets the number of voxels from the fid/procpar header
 * this will correspond to the 3D Grid, not the original 
 * EPSI encoding
 */
void svkVarianUCSFEPSI2DMapper::GetNumVoxels(int* numVoxels)
{
    numVoxels[0] = this->GetHeaderValueAsInt("nv", 0);
    numVoxels[1] = this->GetHeaderValueAsInt("np", 0)/2;
    numVoxels[2] = this->GetHeaderValueAsInt("ns", 0);
}


/*!
 *  Pixel Spacing:
 */
void svkVarianUCSFEPSI2DMapper::InitPixelMeasuresMacro()
{

    int numPixels[3];
    this->GetNumVoxels(numPixels); 

    //  lpe (phase encode resolution in cm, already converted in base class)
    float pixelSize[3];
    pixelSize[0] = this->GetHeaderValueAsFloat("lpe", 0) / numPixels[0] ;
    pixelSize[1] = this->GetHeaderValueAsFloat("lro", 0) / numPixels[1];
    pixelSize[2] = this->GetHeaderValueAsFloat("thk", 0) / numPixels[2];

    std::string pixelSizeString[3];

    for (int i = 0; i < 3; i++) {
        ostringstream oss;
        oss << pixelSize[i];
        pixelSizeString[i].assign( oss.str() );
    }

    this->dcmHeader->InitPixelMeasuresMacro(
        pixelSizeString[0] + "\\" + pixelSizeString[1],
        pixelSizeString[2]
    );
}


/*!
 *
 */
void svkVarianUCSFEPSI2DMapper::InitMultiFrameFunctionalGroupsModule()
{

    this->InitSharedFunctionalGroupMacros();

    this->dcmHeader->SetValue(
        "InstanceNumber",
        1
    );

    this->dcmHeader->SetValue(
        "ContentDate",
        this->GetHeaderValueAsString( "time_svfdate" )
    );

    this->numSlices = this->GetHeaderValueAsInt("ns");

    this->numFrames = this->numSlices;
    this->dcmHeader->SetValue(
        "NumberOfFrames",
        this->numFrames
    );

    this->InitPerFrameFunctionalGroupMacros();

}


/*!
 *  The FID toplc is the center of the first voxel.
 */
void svkVarianUCSFEPSI2DMapper::InitPerFrameFunctionalGroupMacros()
{

    double dcos[3][3];
    this->dcmHeader->SetSliceOrder( this->dataSliceOrder );
    this->dcmHeader->GetDataDcos( dcos );
    dcos[0][0] *= -1; 
    dcos[0][1] *= -1; 
    dcos[0][2] *= -1; 
    double pixelSpacing[3];
    this->dcmHeader->GetPixelSize(pixelSpacing);

    int numPixels[3];
    this->GetNumVoxels(numPixels); 
    //numPixels[0] = this->GetHeaderValueAsInt("nv", 0);
    numPixels[1] = 1; 
    //numPixels[1] = this->GetHeaderValueAsInt("nv2", 0);
    //numPixels[2] = this->GetHeaderValueAsInt("ns", 0);

    //  Get center coordinate float array from fidMap and use that to generate
    //  Displace from that coordinate by 1/2 fov - 1/2voxel to get to the center of the
    //  toplc from which the individual frame locations are calculated


    //  Center of toplc (LPS) pixel in frame:
    double toplc[3];

    //
    //  If 3D vol, calculate slice position, otherwise use value encoded
    //  into slice header
    //

     //  If 2D (single slice)
     if ( numPixels[2] == 1 ) {

        //  Location is the center of the image frame in user (acquisition frame).
        double centerAcqFrame[3];
        centerAcqFrame[0] = this->GetHeaderValueAsFloat("ppe", 0);
        centerAcqFrame[1] = this->GetHeaderValueAsFloat("ppe2", 0);
        centerAcqFrame[2] = this->GetHeaderValueAsFloat("pro", 0);

        //  Account for slice offset (pss is offset in cm):
        int mmPerCm = 10; 
        //centerAcqFrame[2] += this->GetHeaderValueAsFloat("pss", 0) * mmPerCm ;

        //  Now get the center of the tlc voxel in the acq frame:
        double* tlcAcqFrame = new double[3];
        for (int j = 0; j < 2; j++) {
            tlcAcqFrame[j] = centerAcqFrame[j]
                - ( ( numPixels[j] * pixelSpacing[j] ) - pixelSpacing[j] )/2;
        }
        tlcAcqFrame[2] = centerAcqFrame[2];

        //  and convert to LPS (magnet) frame:
        svkVarianReader::UserToMagnet(tlcAcqFrame, toplc, dcos);

        delete [] tlcAcqFrame;

    } 

    svkDcmHeader::DimensionVector dimensionVector = this->dcmHeader->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, numFrames-1);

    this->dcmHeader->InitPerFrameFunctionalGroupSequence(
        toplc, pixelSpacing, dcos, &dimensionVector
    );

}


/*!
 *
 */
void svkVarianUCSFEPSI2DMapper::InitMRSpectroscopyDataModule()
{
    this->dcmHeader->SetValue( "Columns", this->GetHeaderValueAsInt("nv", 0) );
    this->dcmHeader->SetValue( "Rows", 1); 
    //this->dcmHeader->SetValue( "Rows", this->GetHeaderValueAsInt("nv2", 0) );
    this->dcmHeader->SetValue( "DataPointRows", 0 );

    // Number of EPSI data points is numEchoes * num k-space points     
    int numEchoes = this->GetHeaderValueAsInt("ne");
    int numFlybackKSpacePts = this->GetHeaderValueAsInt("np", 0)/2 ;
    this->dcmHeader->SetValue( "DataPointColumns", numEchoes * numFlybackKSpacePts); 

    this->dcmHeader->SetValue( "DataRepresentation", "COMPLEX" );
    this->dcmHeader->SetValue( "SignalDomainColumns", "TIME" );
    this->dcmHeader->SetValue( "SVK_ColumnsDomain", "KSPACE" );
    this->dcmHeader->SetValue( "SVK_RowsDomain", "KSPACE" );
    this->dcmHeader->SetValue( "SVK_SliceDomain", "KSPACE" );

    if ( this->HeaderFieldExists("epsiorigin") == true ) {
        this->dcmHeader->SetValue( "SVK_ECHO_CENTER_PT", this->GetHeaderValueAsFloat("epsiorigin")); 
    }
}


/*!
 *
 */
void svkVarianUCSFEPSI2DMapper::InitMRSpectroscopyPulseSequenceModule()
{
    this->dcmHeader->SetValue(
        "PulseSequenceName",
        this->GetHeaderValueAsString( "seqfil" )
    );

    int numVoxels[3];
    numVoxels[0] = this->GetHeaderValueAsInt("nv", 0);
    numVoxels[1] = 1; 
    //numVoxels[1] = this->GetHeaderValueAsInt("nv2", 0);
    numVoxels[2] = this->GetHeaderValueAsInt("ns", 0);

    string acqType = "VOLUME";
    if (numVoxels[0] == 1 && numVoxels[1] == 1 &&  numVoxels[2] == 1) {
        acqType = "SINGLE VOXEL";
    }

    this->dcmHeader->SetValue(
        "MRSpectroscopyAcquisitionType",
        acqType
    );

    this->dcmHeader->SetValue(
        "EchoPulseSequence",
        "SPIN"
    );

    this->dcmHeader->SetValue(
        "MultipleSpinEcho",
        "NO"
    );

    this->dcmHeader->SetValue(
        "MultiPlanarExcitation",
        "NO"
    );

    this->dcmHeader->SetValue(
        "SteadyStatePulseSequence",
        "NONE"
    );


    this->dcmHeader->SetValue(
        "EchoPlanarPulseSequence",
        "NO"
    );

    this->dcmHeader->SetValue(
        "SpectrallySelectedSuppression",
        ""
    );

    this->dcmHeader->SetValue(
        "GeometryOfKSpaceTraversal",
        "RECTILINEAR"
    );

    this->dcmHeader->SetValue(
        "RectilinearPhaseEncodeReordering",
        "LINEAR"
    );

    this->dcmHeader->SetValue(
        "SegmentedKSpaceTraversal",
        "SINGLE"
    );

    this->dcmHeader->SetValue(
        "CoverageOfKSpace",
        "FULL"
    );

    this->dcmHeader->SetValue( "NumberOfKSpaceTrajectories", 1 );

    //  k = 0 is sampled in the acquisition. 
    //string k0Sampled = "NO";
    string k0Sampled = "NO";
    this->dcmHeader->SetValue( "SVK_K0Sampled", k0Sampled);

}


/*!
 *  
 */
void svkVarianUCSFEPSI2DMapper::InitMRSpectroscopyModule()
{
    this->Superclass::InitMRSpectroscopyModule(); 

    float swf = 0; 
    if ( this->HeaderFieldExists("swf") == true ) {
        swf = this->GetHeaderValueAsFloat( "swf" ); 
    }

    this->dcmHeader->SetValue(
        "SpectralWidth",
        swf
    );

    //  sp is the frequency in Hz at left side (downfield/High freq) 
    //  side of spectrum: 
    //
    float ppmRef = 0;  
    if (
        ( this->HeaderFieldExists("spcenter") == true ) &&
        ( this->HeaderFieldExists("sfrq") == true ) 
    ) {
        ppmRef = this->GetHeaderValueAsFloat( "spcenter" ) /  this->GetHeaderValueAsFloat( "sfrq" );
    }
    this->dcmHeader->SetValue(
        "ChemicalShiftReference",
        ppmRef 
    );
}



/*!
 *  This method reads data from the pfile and puts the data into the CellData arrays.
 *  Non-uniform k-space sampling requires regridding to rectaliniear k-space array here. 
 */
void svkVarianUCSFEPSI2DMapper::ReadFidFile( std::string fidFileName, svkImageData* data )
{

    svkImageData* tmpImage = svkMrsImageData::New();
    tmpImage->DeepCopy( data ); 

    //  Do all the usual data loading stuff:
    this->Superclass::ReadFidFile( fidFileName, tmpImage); 

    //  Separate out EPSI sampled data into time and k-space dimensions:
    this->ReorderEPSIData( tmpImage );
    data->DeepCopy( tmpImage ); 

    tmpImage->Delete();

    //  =================================================
    //  Apply linear phase correction to correct for EPSI sampling of 
    //  spectra: 
    //      foreach k-space point, apply a phase correction to each complex 
    //      spectral point. The correction is a function of the distance along 
    //      the epsi-axis (k/t) with pivots at the center of each dimension.
    //  =================================================
//this->EPSIPhaseCorrection( data, numRead, epsiAxis);  

    // now that the header and dimensionality are set correctly, reset this param:
    //this->InitK0Sampled(); 
    data->SyncVTKImageDataToDcmHeader(); 

}


/*!
 *  apply linear phase correction to k-space points along epsi-axis:
 *  this would be easier if I actually redimension the metadata and set the new arrays 
 */
void svkVarianUCSFEPSI2DMapper::EPSIPhaseCorrection( svkImageData* data, int numRead, int epsiAxis)
{
/*
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
*/
}


/* 
 *  Reorder the data.  Input EPSI data has one dimension with both time (spectral) and k-space
 *  content.  The time domain data from each k-space point needs to be extracted and put into 
 *  a cell array.  This is done separately for positive and negative lobes.  So, for example, 
 *  a 1380 x 1 x 12 x 10 data set with 60 lobes (23 k-space samples) would get reordered to a
 *  two 30 x 23 x 12 x 10 data sets. Each will be encoded into a separate "channel" indicated
 *  by the dimension index sequence.  this is just a temporary internal layout.
 * 
 *  Refactor, inputs for reordering will be .... in progress( 
 *      int numEPSIKspacePts e.g 16;     
 *      int numEPSILobes = numEPSIPts / numEPSIKspacePts; 
 *      int skip (points betwen lobes 
 *      int offset points to skip at the begining
 */
void svkVarianUCSFEPSI2DMapper::ReorderEPSIData( svkImageData* data)
{

    //  Reorder data.  This requires adding data arrays 
    //  to accommodate the spectra at the new k-space points 
    //  points extracted from the EPSI encoding 

    //  Preallocate data arrays. We can't override the original arrays, 
    //  until the reordering is complette, so use these as the 
    //  target arrays. 

    //  Allocate arrays for all (How many k-space points were acquired in all)
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
    //  TODO get number of epsi k-space points in plateau from header
    int numEPSIKspacePts = 16;     
    int numEPSILobes = numEPSIPts / numEPSIKspacePts; 
    int numFreqPts = numEPSILobes;     

    //============================================================
    //  Read EPSI acquisition params from raw header: 
    //============================================================

    //  dwell time time between k-space points
    //int deltaT = this->GetHeaderValueAsInt( "rhr.rh_user12" );

    //  time for plateau encoding (gradient duratation)
    //int plateauTime = this->GetHeaderValueAsInt( "rhr.rh_user13" );

    //  time for ramp in one direction
    //int rampTime = this->GetHeaderValueAsInt( "rhr.rh_user15" );

    //  number of samples at start
    int numSkip = 0; 

    //  number of samples per lobe (ramps + plateau)  
    //  num spectral samples in FID is num time_pts / this value (
    //int numRead = (plateauTime + 2 * rampTime)/deltaT;

    //============================================================
    //Q:  is the number of EPSI phase encodes equal to numRead or numEPSIkSpacePoints?
    //  I think it starts at numRead, and gets sampled down to numEPSIkSpacePoints (23-18)
    //  after ramp sampling... i think
    //============================================================
    //===========
    //  Reset hdr value (num cols?)
    //===========
    //int numEPSIkSpacePts = this->GetHeaderValueAsInt( "rhr.rh_user9" );

    //  EPSI Axis (user9) defines which axis is epsi encoded.  Swap the value of 
    //  epsi k-space encodes into this field: 
    int epsiAxis = 1; 


    //  Get the input dimensionality (EPSI)
    int numEPSIVoxels[3]; 
    data->GetNumberOfVoxels(numEPSIVoxels);

    //  Set the actual number of k-space points sampled:
    int numVoxels[3]; 
    numVoxels[0] = numEPSIVoxels[0]; 
    numVoxels[1] = numEPSIVoxels[1]; 
    numVoxels[2] = numEPSIVoxels[2]; 
    numVoxels[ epsiAxis ] = numEPSIKspacePts; 

    //  Set the number of reordered voxels (target dimensionality of this method)
    int reorderedVoxels[3]; 
    reorderedVoxels[0] = numVoxels[0]; 
    reorderedVoxels[1] = numVoxels[1]; 
    reorderedVoxels[2] = numVoxels[2]; 

    //============================================================
    //  Reorder the data: 
    //============================================================
    int numCoils = 1; 
    int numTimePts = 1; 
    int timePoint = numTimePts - 1; 
    int coilNum = numCoils - 1; 

    //============================================================
    //  create a data set to store the new reordered data arrays. 
    //  Remove all the original arrays from it so we are starting with 
    //  a blank slate except for the header. 
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
    numReorderedVoxels *= numCoils; 
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
                //cout << "REORDER: " << xEPSI << " "  << yEPSI << " "  << zEPSI << " "  << endl;
    
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
                rangeMax[epsiAxis] = numVoxels[epsiAxis]; 
                //cout << "RANGE " << rangeMin[0] << " -> " << rangeMax[0] << endl;
                //cout << "RANGE " << rangeMin[1] << " -> " << rangeMax[1] << endl;
                //cout << "RANGE " << rangeMin[2] << " -> " << rangeMax[2] << endl;
                
                //============================================================
                //  allocate an array for both the positive and negative lobes of 
                //  the symmetric EPSI encoding:
                //============================================================
                int lobeStride = numEPSIKspacePts;  

                //  set the current index along the EPSI dimension
                //  Initialize first point to skip + throw out first point (zero crossing)
                int currentEPSIPt = numSkip;   

                rangeMax[epsiAxis] -= 1;    //  other indices have zero range 
                for (int z = rangeMin[2]; z < rangeMax[2] + 1; z++ ) { 
                    for (int y = rangeMin[1]; y < rangeMax[1] + 1; y++ ) { 
                        for (int x = rangeMin[0]; x < rangeMax[0] + 1; x++ ) { 

                    //cout << "  to : " << x << " "  << y << " "  << z << " "  << endl;

                            //  Add arrays to the new data set at the correct array index: 
                            int index =  x
                                  + ( reorderedVoxels[0] ) * y
                                  + ( reorderedVoxels[0] * reorderedVoxels[1] ) * z
                                  + ( reorderedVoxels[0] * reorderedVoxels[1] * reorderedVoxels[2] ) * coilNum;

                            //cout << "   target index(xyzlobe): " << x << " " << y << " " << z << " index: " << index << endl;
                                 
                            vtkDataArray* dataArray = reorderedImageData->GetCellData()->GetArray(index);

                            char arrayName[30];
                            int timePt = 0;     
                            sprintf(arrayName, "%d %d %d %d %d", x, y, z, coilNum, timePt);
                            //cout << "reordered Array name (" << index << ") = " << arrayName << endl; 
                            dataArray->SetName(arrayName);

                               //  reorder EPSI data, throwng away the first and last 
                               //  point (zero crossing). 
                            double epsiTuple[2]; 
                            double tuple[2]; 
                            int epsiOffset; 
                            for (int i = 0; i < numFreqPts; i++) {
                                epsiOffset = (lobeStride * i ) + currentEPSIPt;
                                epsiSpectrum->GetTuple(epsiOffset, epsiTuple); 
                                tuple[0] = epsiTuple[0]; 
                                tuple[1] = epsiTuple[1]; 
                                //cout << " epsi_spec to cartesian spec " << epsiOffset << " -> " << i << " " << epsiTuple[0] << " " << epsiTuple[1] << endl;
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
            }
        }
    }


    //  =================================================
    //  Redimension the meta data and set the new arrays:
    //  =================================================
    data->DeepCopy( reorderedImageData ); 
    numVoxels[epsiAxis] = numVoxels[epsiAxis]; //   first and last point were thrown out
    int numVoxelsOriginal[3]; 
    data->GetNumberOfVoxels(numVoxelsOriginal);
    this->RedimensionData( data, numVoxelsOriginal, numVoxels, numFreqPts ); 
    
}


/*!
 *  Remove the original EPSI arrays from the svkImageData. 
 *  at this point. 
 */
void svkVarianUCSFEPSI2DMapper::RemoveArrays( svkImageData* data )
{

    int numCoils = data->GetDcmHeader()->GetNumberOfCoils();
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
void svkVarianUCSFEPSI2DMapper::RedimensionData( svkImageData* data, int* numVoxelsOriginal, int* numVoxelsReordered, int numFreqPts )
{

    int numTimePts = data->GetDcmHeader()->GetNumberOfTimePoints();
    int numCoils = 1;

    //  This is the original origin based on the reduced dimensionality in the EPSI direction
    double origin[3];
    data->GetDcmHeader()->GetOrigin( origin, 0 );

    double voxelSpacing[3];
    data->GetDcmHeader()->GetPixelSpacing( voxelSpacing );

    double dcos[3][3];
    data->GetDcmHeader()->GetDataDcos( dcos );

    double center[3]; 
    svkDcmHeader::GetCenterFromOrigin( origin, numVoxelsOriginal, voxelSpacing, dcos, center); 

    //  Now calcuate the new origin based on the reordered dimensionality: 
    double newOrigin[3]; 
    svkDcmHeader::GetOriginFromCenter( center, numVoxelsReordered, voxelSpacing, dcos, newOrigin ); 

    svkDcmHeader* hdr = data->GetDcmHeader();     
    hdr->SetValue( "Columns", numVoxelsReordered[0]);
    hdr->SetValue( "Rows", numVoxelsReordered[1]);
    hdr->SetValue( "DataPointColumns", numFreqPts );

    svkDcmHeader::DimensionVector dimensionVector = hdr->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, numVoxelsReordered[2]-1);
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::TIME_INDEX, numTimePts-1);
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::CHANNEL_INDEX, numCoils-1);

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
void svkVarianUCSFEPSI2DMapper::SetCellSpectrum(vtkImageData* data, int x, int y, int z, int timePt, int coilNum)
{

    int numComponents = 1;
    std::string representation =  this->dcmHeader->GetStringValue( "DataRepresentation" );
    if (representation.compare( "COMPLEX" ) == 0 ) {
        numComponents = 2;
    }
    vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
    dataArray->SetNumberOfComponents( numComponents );

    int numPts = this->dcmHeader->GetIntValue( "DataPointColumns" );
    dataArray->SetNumberOfTuples(numPts);

    char arrayName[30];
    sprintf(arrayName, "%d %d %d %d %d", x, y, z, timePt, coilNum);
    dataArray->SetName(arrayName);

    int numVoxels[3];
    numVoxels[0] = this->dcmHeader->GetIntValue( "Columns" );
    numVoxels[1] = this->dcmHeader->GetIntValue( "Rows" );
    numVoxels[2] = this->dcmHeader->GetNumberOfSlices();
    
    //  if cornoal, swap z and x:
    //cout << "X,Y:" << x << " " << y << " -> ";
    //int xTmp = x; 
    //x = y; 
    //y = xTmp; 
    //x = numVoxels[0] - x - 1; 
    //y = numVoxels[1] - y - 1; 
    //cout << x << " " << y << endl; 

    int offset = (numPts * numComponents) *  (
                     ( numVoxels[0] * numVoxels[1] * numVoxels[2] ) * timePt
                    +( numVoxels[0] * numVoxels[1] ) * z
                    +  numVoxels[0] * y
                    +  x
                 );


    for (int i = 0; i < numPts; i++) {
        dataArray->SetTuple(i, &(this->specData[offset + (i * 2)]));
    }

    //  Add the spectrum's dataArray to the CellData:
    //  vtkCellData is a subclass of vtkFieldData
    data->GetCellData()->AddArray(dataArray);

    dataArray->Delete();

    return;
}

