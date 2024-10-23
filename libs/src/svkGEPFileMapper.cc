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


#include <svkGEPFileMapper.h>
#include <svkMrsImageData.h>
#include </usr/include/vtk/vtkCellData.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include <svkSpecUtils.h>
#include </usr/include/vtk/vtkByteSwap.h>
#include </usr/include/vtk/vtkMatrix3x3.h>

#include "svkEPSIReorder.h"

using namespace svk;


//vtkCxxRevisionMacro(svkGEPFileMapper, "$Rev$");
vtkStandardNewMacro(svkGEPFileMapper);


/*!
 *
 */
svkGEPFileMapper::svkGEPFileMapper()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkGEPFileMapper");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->chopVal = 1;     
    this->progress = 0;
    this->acqDad = NULL; 

}


/*!
 *
 */
svkGEPFileMapper::~svkGEPFileMapper()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}


/*!
 *  Initializes the svkDcmHeader adapter to a specific IOD type      
 *  and initizlizes the svkDcmHeader member of the svkImageData 
 *  object.    
 */
void svkGEPFileMapper::InitializeDcmHeader(map <string, vector< string > >  pfMap, 
    svkDcmHeader* header, float pfileVersion, int swapBytes, map < string, void* > inputArgs)
{
    this->pfMap = pfMap; 
    this->dcmHeader = header; 
    this->pfileVersion = pfileVersion; 
    this->swapBytes = swapBytes;
    this->inputArgs = inputArgs;

    this->iod = svkMRSIOD::New();
    this->iod->SetDcmHeader( this->dcmHeader ); 

    this->InitPatientModule();
    this->InitGeneralStudyModule();
    this->InitGeneralSeriesModule();
    this->InitFrameOfReferenceModule();
    this->InitGeneralEquipmentModule();
    this->InitEnhancedGeneralEquipmentModule();
    this->InitMRSpectroscopyModule();
    this->InitMultiFrameFunctionalGroupsModule();
    this->InitAcquisitionContextModule();
    this->InitMRSpectroscopyPulseSequenceModule();
    this->InitMRSpectroscopyDataModule();

    this->iod->Delete();
}



/*!
 *  Initializes the VolumeLocalizationSequence in the MRSpectroscopy
 *  DICOM object for PRESS excitation.  
 */
void svkGEPFileMapper::InitVolumeLocalizationSeq()
{

    this->dcmHeader->InsertEmptyElement( "VolumeLocalizationSequence" );

    //  Get Center Location Values
    double selBoxCenter[3]; 
    this->GetSelBoxCenter( selBoxCenter );

    string midSlabPosition;
    for (int i = 0; i < 3; i++) {
        ostringstream oss;
        oss << selBoxCenter[i];
        midSlabPosition += oss.str();
        if (i < 2) {
            midSlabPosition += '\\';
        }
    }

    //  Get Thickness Values
    double selBoxSize[3]; 
    this->GetSelBoxSize( selBoxSize );

    double dcos[3][3]; 
    this->GetDcos(dcos); 

    //  Volume Localization (PRESS BOX)
    for (int i = 0; i < 3; i++) {

        this->dcmHeader->AddSequenceItemElement(
            "VolumeLocalizationSequence", 
            i,
            "SlabThickness",
            selBoxSize[i] 
        );

        this->dcmHeader->AddSequenceItemElement(
            "VolumeLocalizationSequence", 
            i,
            "MidSlabPosition",
            midSlabPosition
        );


        string slabOrientation;
        for (int j = 0; j < 3; j++) {
            ostringstream oss;
            oss << dcos[i][j];
            slabOrientation += oss.str();
            if (j < 2) {
                slabOrientation += '\\';
            }
        }

        this->dcmHeader->AddSequenceItemElement(
            "VolumeLocalizationSequence", 
            i,
            "SlabOrientation",
            slabOrientation 
        );

    }

}


/*
 *
 */
void svkGEPFileMapper::GetSelBoxCenter( double selBoxCenter[3] )
{
    //  Center position is taken from user variables.  The Z "slice"
    //  position used to be taken from the image header "image.loc",
    //  but with the LX architecture, this held the table position only,
    //  so if Graphic RX was used to introduce an offset, it wouldn't
    //  be successfully extracted.
    selBoxCenter[0] = -1 * this->GetHeaderValueAsFloat( "rhi.user11" ); 
    selBoxCenter[1] = -1 * this->GetHeaderValueAsFloat( "rhi.user12" ); 
    selBoxCenter[2] =  this->GetHeaderValueAsFloat( "rhi.user13" ); 

}


/*
 *
 */
void svkGEPFileMapper::GetSelBoxSize( double selBoxSize[3] )
{
    selBoxSize[0] = 0.0;
    double dcos[3][3]; 
    this->GetDcos(dcos); 
    if ( this->pfileVersion >= 9  ) {

        float lMax = 0; 
        float pMax = 0; 
        float sMax = 0;
        int lIndex; 
        int pIndex; 
        int sIndex;     
        for ( int i = 0; i < 3; i++ ) {
            if( fabs( dcos[i][0] ) > lMax ) {
                lIndex = i;
                lMax = fabs( dcos[i][0] );
            }
            if( fabs( dcos[i][1] ) > pMax) {
                pIndex = i;
                pMax = fabs( dcos[i][1] );
            }
            if( fabs( dcos[i][2] ) > sMax ) {
                sIndex = i;
                sMax = fabs( dcos[i][2] );
            }
        }
        selBoxSize[ lIndex ] = this->GetHeaderValueAsFloat( "rhi.user8" ); 
        selBoxSize[ pIndex ] = this->GetHeaderValueAsFloat( "rhi.user9" ); 
        selBoxSize[ sIndex ] = this->GetHeaderValueAsFloat( "rhi.user10" ); 

    } else {

        selBoxSize[0] = this->GetHeaderValueAsFloat( "rhr.roilenx" );
        selBoxSize[1] = this->GetHeaderValueAsFloat( "rhr.roileny" );
        selBoxSize[2] = this->GetHeaderValueAsFloat( "rhr.roilenz" );

        if ( this->IsSwapOn() ) {
            float ftemp = selBoxSize[0];
            selBoxSize[0] = selBoxSize[1];
            selBoxSize[1] = ftemp;
        }
    }
}


/*!
 *
 */
void svkGEPFileMapper::InitPatientModule() 
{

    int patsex = this->GetHeaderValueAsInt( "rhe.patsex" ); 
    string gender("O"); 
    if ( patsex == 1 ) {
        gender.assign("M"); 
    } else if ( patsex == 2 ) {
        gender.assign("F"); 
    }

    this->dcmHeader->InitPatientModule(
        this->GetHeaderValueAsString( "rhe.patname" ), 
        this->GetHeaderValueAsString( "rhe.patid" ), 
        this->GetHeaderValueAsString( "rhe.dateofbirth" ), 
        gender
    );

}


/*!
 *
 */
void svkGEPFileMapper::InitGeneralStudyModule() 
{

    string dcmDate = this->ConvertGEDateToDICOM( this->GetHeaderValueAsString( "rhr.rh_scan_date" ) );

    this->dcmHeader->InitGeneralStudyModule(
        svkImageReader2::RemoveDelimFromDate( &dcmDate ),  
        this->GetHeaderValueAsString( "rhr.rh_scan_time" ), 
        this->GetHeaderValueAsString( "rhe.refphy" ), 
        this->GetHeaderValueAsString( "rhe.ex_no" ), 
        this->GetHeaderValueAsString( "rhe.reqnum" ), 
        this->GetHeaderValueAsString( "rhe.study_uid" )
    );

}


/*!
 *
 */
void svkGEPFileMapper::InitGeneralSeriesModule() 
{

    string patientEntryPos; 
    int patientEntry( this->GetHeaderValueAsInt( "rhs.entry" ) ); 
    if ( patientEntry == 0) {
        patientEntryPos = "Unknown";
    } else if ( patientEntry == 1) {
        patientEntryPos = "HF";
    } else if ( patientEntry == 2) {
        patientEntryPos = "FF";
    }

    int patientPosition( this->GetHeaderValueAsInt( "rhs.position" ) ); 
    if ( patientPosition == 0 ) {
        patientEntryPos.append("Unknown");
    } else if ( patientPosition == 1 ) {
        patientEntryPos.append("S");
    } else if ( patientPosition == 2 ) {
        patientEntryPos.append("P");
    }

    this->dcmHeader->InitGeneralSeriesModule(
        this->GetHeaderValueAsString( "rhs.se_no" ), 
        this->GetHeaderValueAsString( "rhs.se_desc" ), 
        patientEntryPos
    );

}


/*!
 *  initialize 
 */
void svkGEPFileMapper::InitFrameOfReferenceModule()
{
    this->dcmHeader->SetValue(
        "FrameOfReferenceUID", 
        this->GetHeaderValueAsString( "rhs.landmark_uid" )
    );

    this->dcmHeader->SetValue(
        "PositionReferenceIndicator", 
        this->GetHeaderValueAsString( "rhs.anref" )
    );
}


/*!
 *  initialize 
 */
void svkGEPFileMapper::InitGeneralEquipmentModule()
{
    this->dcmHeader->SetValue(
        "Manufacturer", 
        "GE MEDICAL SYSTEMS" 
    );

    this->dcmHeader->SetValue(
        "InstitutionName", 
        this->GetHeaderValueAsString( "rhe.hospname" )
    );

    this->dcmHeader->SetValue(
        "StationName", 
        this->GetHeaderValueAsString( "rhe.ex_sysid" )
    );

}

/*!
 *  initialize 
 */
void svkGEPFileMapper::InitEnhancedGeneralEquipmentModule()
{
    this->dcmHeader->SetValue(
        "DeviceSerialNumber", 
        this->GetHeaderValueAsString( "rhe.uniq_sys_id" )
    );

    // need to null terminate this string:
    string versionNumber = this->GetHeaderValueAsString( "rhe.ex_verscre" ); 

    char* terminatedVersionString = const_cast<char*>(versionNumber.c_str()); 
    terminatedVersionString[2] = '\0'; 

    this->dcmHeader->SetValue(
        "SoftwareVersions", 
        terminatedVersionString
    );
}


/*!
 *
 */
void svkGEPFileMapper::InitMultiFrameFunctionalGroupsModule()
{
    InitSharedFunctionalGroupMacros();

    this->dcmHeader->SetValue( 
        "InstanceNumber", 
        1 
    );

    string dcmDate = this->ConvertGEDateToDICOM( this->GetHeaderValueAsString( "rhr.rh_scan_date" ) );
    this->dcmHeader->SetValue( 
        "ContentDate", 
        svkImageReader2::RemoveDelimFromDate( &dcmDate ) 
    );

    InitPerFrameFunctionalGroupMacros();

}


/*!
 *
 */
void svkGEPFileMapper::InitSharedFunctionalGroupMacros()
{
    this->InitPixelMeasuresMacro();
    this->InitPlaneOrientationMacro();
    this->InitMRSpectroscopyFrameTypeMacro();
    this->InitMRTimingAndRelatedParametersMacro();
    this->InitMRSpectroscopyFOVGeometryMacro();
    this->InitMREchoMacro();
    this->InitMRModifierMacro();
    this->InitMRReceiveCoilMacro();
    this->InitMRTransmitCoilMacro();
    this->InitMRAveragesMacro();
    this->InitMRSpatialSaturationMacro();
}


/*!
 *
 */
void svkGEPFileMapper::InitPerFrameFunctionalGroupMacros()
{

    double* center = new double[3]; 
    this->GetCenterFromRawFile( center ); 

    double voxelSpacing[3]; 
    this->GetVoxelSpacing( voxelSpacing ); 

    double dcos[3][3]; 
    this->GetDcos( dcos ); 

    int numVoxels[3]; 
    this->GetNumVoxels( numVoxels ); 
    int numSlices = numVoxels[2]; 

    double toplc[3]; 
    for( int i = 0; i < 3; i++ ) {
        toplc[i] = center[i];
        for( int j = 0; j < 3; j++ ) {
            toplc[i] -= dcos[j][i] * voxelSpacing[j] * ( numVoxels[j] / 2.0 - 0.5 );
        }    
    }

    int numCoils = this->GetNumCoils();
    int numTimePts = this->GetNumTimePoints();

    svkDcmHeader::DimensionVector dimensionVector = this->dcmHeader->GetDimensionIndexVector(); 
    svkDcmHeader::SetDimensionVectorValue( &dimensionVector, svkDcmHeader::SLICE_INDEX, numSlices - 1 );
    this->dcmHeader->AddDimensionIndex( &dimensionVector, svkDcmHeader::TIME_INDEX, numTimePts - 1 );
    this->dcmHeader->AddDimensionIndex( &dimensionVector, svkDcmHeader::CHANNEL_INDEX, numCoils - 1 );

    this->dcmHeader->InitPerFrameFunctionalGroupSequence(
            toplc, 
            voxelSpacing,
            dcos, 
            &dimensionVector
    );

    delete[] center; 

}


/*!
 *  Pixel Spacing:
 */
void svkGEPFileMapper::InitPixelMeasuresMacro()
{

    //  rhi.scanspacing (space in mm between scans)
    //  Need to get the FOV and the number of phase encodes to get the spatial resolution:
    //  should the methods for obtaining the resolution and spacing be virtual in a subclass?

    double voxelSpacing[3]; 

    int numVoxels[3];
    this->GetNumVoxels( numVoxels ); 
    int totalVoxels =  numVoxels[0] * numVoxels[1] * numVoxels[2]; 

    if ( totalVoxels > 1 ) { 
    
        this->GetVoxelSpacing( voxelSpacing); 

    } else {

        //  If PRESS SV, then set pixel size to PRESS box dimensions: 
        string localizationType; 
        localizationType = this->GetVolumeLocalizationTechnique(); 

        if ( localizationType.compare("PRESS") == 0 )  { 
            //  Get Thickness Values
            this->GetSelBoxSize( voxelSpacing );
        } else {
            // Single voxel, not press set size to full FOV
            float fov[3]; 
            this->GetFOV( fov ); 
            for ( int i = 0; i < 3; i++ ) {
                voxelSpacing[i] = fov[i]; 
            }
        }

    }

    string pixelSpacing;
    ostringstream oss;
    oss << voxelSpacing[0];
    oss << '\\';
    oss << voxelSpacing[1];
    pixelSpacing = oss.str();

    string sliceThickness;
    oss.clear();
    oss.str( "" );
    oss << voxelSpacing[2];

    this->dcmHeader->InitPixelMeasuresMacro(
        pixelSpacing,                    
        oss.str()
    );
}


/*
 *  Converts GE date year to 4 digit representation, i.e. 
 *  converts year from 1XX to 20XX
 */
string svkGEPFileMapper::ConvertGEDateToDICOM( string geDate )
{ 
    string dcmDate = "";

    //   if the data is deidentified, date will have length 0: 
    if (geDate.length() > 0) {
        size_t yearPos; 
        string yearPrefix = "";  
        if ( (yearPos = geDate.find_last_of("\\/") ) != string::npos ) { 
            if ( geDate[yearPos + 1] == '1' ) { 
                yearPrefix = "20";  
            } else if ( geDate[yearPos + 1] == '0' ) { 
                yearPrefix = "19";  
            }
        }
        dcmDate = geDate.replace(6, 1, yearPrefix);  
    }

    return dcmDate; 
}


/*!
 *  Get the voxel spacing in 3D. Note that the slice spacing may 
 *  include a skip. 
 */
void svkGEPFileMapper::GetVoxelSpacing( double voxelSpacing[3] )
{

    float user19 =  this->GetHeaderValueAsFloat( "rhi.user19" ); 

    if ( user19 > 0  && this->pfileVersion >= 9 ) {

        voxelSpacing[0] = user19; 
        voxelSpacing[1] = user19; 
        voxelSpacing[2] = user19; 

    } else {
       
        float fov[3];  
        this->GetFOV( fov ); 

        int numVoxels[3]; 
        this->GetNumVoxels( numVoxels ); 

        voxelSpacing[0] = fov[0]/numVoxels[0];
        voxelSpacing[1] = fov[1]/numVoxels[1];
        voxelSpacing[2] = fov[2]/numVoxels[2];
    }
}


/*!
 *  Get the voxel spacing in 3D. Note that the slice spacing may 
 *  include a skip. 
 *  Swaps the FOV if necessary based on freq_dir setting. 
 */
void svkGEPFileMapper::GetFOV( float fov[3] )
{
    int numVoxels[3]; 
    this->GetNumVoxels( numVoxels ); 

    float dfov = this->GetHeaderValueAsFloat( "rhi.dfov" ); 

    if ( this->pfileVersion >= 9  ) {

        fov[0] = dfov; 
        fov[1] = dfov; 

        // 2D case vs 3D cases 
        if ( this->Is2D() ) {
            fov[2] = this->GetHeaderValueAsFloat( "rhi.user10" ); 
        } else {
            fov[2] =  this->GetHeaderValueAsFloat( "rhi.scanspacing" ) 
                    * this->GetHeaderValueAsFloat( "rhr.zcsi" ); 
        }

    } else {

        fov[0] = this->GetHeaderValueAsFloat( "rhr.rh_user7" ); 
        fov[1] = this->GetHeaderValueAsFloat( "rhr.rh_user8" ); 
        fov[2] = this->GetHeaderValueAsFloat( "rhr.rh_user9" ); 

    }
 
    //  Anisotropic voxels:   
    if ( this->pfileVersion >= 9  &&  numVoxels[0] != numVoxels[1] ) {

        //  CSI has already been reordered if needed - so fov 
        //  calculated with this CSI will not need reordering
        //  next power of 2:
        int xDim = 2;

        while( xDim < numVoxels[0] ) {
           xDim *= 2;
        }

        int yDim = 2;
        while( yDim < numVoxels[1] ) {
           yDim *= 2;
        }

        float fovSpatialResolution;
        if( yDim > xDim ) {
            fovSpatialResolution = dfov/yDim;
        } else {
            fovSpatialResolution = dfov/xDim;
        }
        fov[1] = fovSpatialResolution * yDim;
        fov[0] = fovSpatialResolution * xDim;

    } else if ( this->IsSwapOn() ) {

        //  Swap the FOV if necessary based on freq dir: 
        float temp = fov[0];
        fov[0] = fov[1];
        fov[1] = temp;

    }

    if ( this->GetDebug() ) {
        cout << "FOV: " << fov[0] << " " << fov[1] << " " << fov[2] << endl;
    }
}


/*!
 *  Get the 3D spatial dimensionality of the data set 
 *  Returns an int array with 3 dimensions.  Swaps
 *  if necessary based on freq_dir setting. 
 */
void svkGEPFileMapper::GetNumVoxels( int numVoxels[3] )
{
    if ( this->GetHeaderValueAsInt( "rhr.rh_file_contents" ) == 0 ) {
        numVoxels[0] = 1;
        numVoxels[1] = 1;
        numVoxels[2] = 1;
    } else {
        numVoxels[0] = this->GetHeaderValueAsInt( "rhr.xcsi" );
        numVoxels[1] = this->GetHeaderValueAsInt( "rhr.ycsi" );
        numVoxels[2] = this->GetHeaderValueAsInt( "rhr.zcsi" );
    }

    //  Swap dimensions if necessary:
    if ( this->IsSwapOn() ) {
        int temp = numVoxels[0];
        numVoxels[0] = numVoxels[1];
        numVoxels[1] = temp;
    }
}


/*!
 *  Get the data dcos 
 */
void svkGEPFileMapper::GetDcos( double dcos[3][3] )
{

    dcos[0][0] = -( this->GetHeaderValueAsFloat( "rhi.trhc_R" ) - this->GetHeaderValueAsFloat( "rhi.tlhc_R" ) );
    dcos[0][1] = -( this->GetHeaderValueAsFloat( "rhi.trhc_A" ) - this->GetHeaderValueAsFloat( "rhi.tlhc_A" ) );
    dcos[0][2] =  ( this->GetHeaderValueAsFloat( "rhi.trhc_S" ) - this->GetHeaderValueAsFloat( "rhi.tlhc_S" ) );

    float dcosLengthX = sqrt( dcos[0][0] * dcos[0][0]
                            + dcos[0][1] * dcos[0][1]
                            + dcos[0][2] * dcos[0][2] );

    dcos[0][0] /= dcosLengthX;
    dcos[0][1] /= dcosLengthX;
    dcos[0][2] /= dcosLengthX;


    dcos[1][0] = -( this->GetHeaderValueAsFloat( "rhi.brhc_R" ) - this->GetHeaderValueAsFloat( "rhi.trhc_R" ) );
    dcos[1][1] = -( this->GetHeaderValueAsFloat( "rhi.brhc_A" ) - this->GetHeaderValueAsFloat( "rhi.trhc_A" ) );
    dcos[1][2] =  ( this->GetHeaderValueAsFloat( "rhi.brhc_S" ) - this->GetHeaderValueAsFloat( "rhi.trhc_S" ) );

    float dcosLengthY = sqrt( dcos[1][0] * dcos[1][0]
                            + dcos[1][1] * dcos[1][1]
                            + dcos[1][2] * dcos[1][2] );

    dcos[1][0] /= dcosLengthY;
    dcos[1][1] /= dcosLengthY;
    dcos[1][2] /= dcosLengthY;


    // third row is the vector product of the first two rows
    // actually, -1* vector product, at least for the axial and axial oblique
    // which is all that we support now
    dcos[2][0] = - dcos[0][1] * dcos[1][2] + dcos[0][2] * dcos[1][1];
    dcos[2][1] = - dcos[0][2] * dcos[1][0] + dcos[0][0] * dcos[1][2];
    dcos[2][2] = - dcos[0][0] * dcos[1][1] + dcos[0][1] * dcos[1][0];

    //
    //  change -0.0000 to 0.0000
    //
    for( int i = 0; i <= 2; i++ ) {
        for( int j = 0; j <= 2; j++ ) {
            dcos[i][j] = (dcos[i][j] == 0.0)? 0.0 : dcos[i][j];
        }
    }

    this->ModifyForPatientEntry( dcos ); 

}


/*!
 *  Modifies the dcos to reflect the patient entry. 
 */
void svkGEPFileMapper::ModifyForPatientEntry( double dcos[3][3] )
{

    //  Create the vtkMatrix3x3 version of dcos: 
    vtkMatrix3x3* dcosMatrix = vtkMatrix3x3::New();
    dcosMatrix->Identity();

    for (int i = 0; i < 3; i++ ) {
        for (int j = 0; j < 3; j++ ) {
            dcosMatrix->SetElement(i, j, dcos[i][j] ); 
            //cout << dcosMatrix->GetElement(i, j) << " " ;
        }
        //cout << endl ;
    }

    //  Apply the necessary transformations based on patient entry: 
    vtkMatrix3x3* rotations = vtkMatrix3x3::New(); 
    rotations->Identity();

    int patientPosition( this->GetHeaderValueAsInt( "rhs.position" ) ); 
    //  PRONE: patientPosition == 2
    //  rotate RL(x) and AP(y) axis by 180 degrees, i.e. rotate frame about SI (z) axis: 
    if ( patientPosition == 2 ) {
        rotations->SetElement(0, 0, -1.0);
        rotations->SetElement(1, 1, -1.0);
    }

    vtkMatrix3x3* transformedDcos = vtkMatrix3x3::New(); 
    vtkMatrix3x3::Multiply3x3(rotations, dcosMatrix, transformedDcos); 

    for (int i = 0; i < 3; i++ ) {
        for (int j = 0; j < 3; j++ ) {
            //cout << transformedDcos->GetElement(i, j) << " " ;
        }
        //cout << endl ;
    }
    

    //  Do I need to fix the ImagePositionPatient values?
 
    //  Write the transformed DCOS matrix to the input arg: 
    for (int i = 0; i < 3; i++ ) {
        for (int j = 0; j < 3; j++ ) {
            dcos[i][j] = transformedDcos->GetElement(i, j);
        }
    }
    
}



/*!
 *  Is freq dir swapped?
 */
bool svkGEPFileMapper::IsSwapOn()
{
    bool swap = false; 

    if ( this->GetHeaderValueAsInt( "rhi.freq_dir" ) != 1 ) {
        swap = true; 
    }

    return swap; 
}


/*!
 *  Is this a 2D or 3D data set (spatial dimensions)? 
 */
bool svkGEPFileMapper::Is2D()
{
    bool is2D = false;

    int numDims = this->GetHeaderValueAsInt( "rhr.csi_dims" );

    if (numDims == 0) {
        if ( this->GetHeaderValueAsInt( "rhr.xcsi" ) >= 0 ) {
            numDims++; 
        }
        if ( this->GetHeaderValueAsInt( "rhr.ycsi" ) >= 0 ) {
            numDims++; 
        }
        if ( this->GetHeaderValueAsInt( "rhr.zcsi" ) >= 0 ) {
            numDims++; 
        }
    }

    if ( numDims == 2 ) {
        is2D = true; 
    } 

    return is2D; 
}


/*!
 *  Is data chopped?
 */
bool svkGEPFileMapper::IsChopOn()
{
    bool chop = false;

    double nex = static_cast<double> (this->GetHeaderValueAsFloat( "rhi.nex" ) ); 
    int numEcho = this->GetHeaderValueAsInt( "rhi.numecho" ); 

    if ( (static_cast<float>( ceil( nex ) ) * numEcho ) <= 1 ) {
        chop = true; 
    }

    //  reset if manually overridden by user:
    this->GetInputArgBoolValue("chop", &chop); 

    return chop;
}


/*!
 *
 */
void svkGEPFileMapper::InitPlaneOrientationMacro()
{

    this->dcmHeader->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "PlaneOrientationSequence"
    );

    double dcos[3][3]; 
    this->GetDcos( dcos ); 

    ostringstream ossDcos;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            ossDcos << dcos[i][j];
            if (i * j != 2) {
                ossDcos<< "\\";
            }
        }
    }

    this->dcmHeader->AddSequenceItemElement(
        "PlaneOrientationSequence",         
        0,                                 
        "ImageOrientationPatient",        
        ossDcos.str(), 
        "SharedFunctionalGroupsSequence",
        0                               
    );


    //  Determine whether the data is ordered with or against the slice normal direction.
    double normal[3]; 
    this->dcmHeader->GetNormalVector(normal); 

    double dcosSliceOrder[3]; 
    for (int i = 0; i < 3; i++) {
        dcosSliceOrder[i] = dcos[2][i]; 
    }

    //  Use the scalar product to determine whether the data in the pfile 
    //  file is ordered along the slice normal or antiparalle to it. 
    vtkMath* math = vtkMath::New(); 
    if (math->Dot(normal, dcosSliceOrder) > 0 ) {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
    } else {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_NEG_NORMAL;
    }
    this->dcmHeader->SetSliceOrder( this->dataSliceOrder );
    math->Delete();
}


/*!
 *
 */
void svkGEPFileMapper::InitMRSpectroscopyFrameTypeMacro()
{
    this->dcmHeader->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRSpectroscopyFrameTypeSequence"
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFrameTypeSequence",
        0,                               
        "FrameType",                    
        string("ORIGINAL\\PRIMARY\\SPECTROSCOPY\\NONE"),  
        "SharedFunctionalGroupsSequence",   
        0                                 
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFrameTypeSequence",  
        0,                                 
        "VolumetricProperties",           
        string("VOLUME"),  
        "SharedFunctionalGroupsSequence",
        0                              
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFrameTypeSequence",  
        0,                                 
        "VolumeBasedCalculationTechnique",
        string("NONE"),
        "SharedFunctionalGroupsSequence",
        0                              
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFrameTypeSequence",
        0,                                
        "ComplexImageComponent",           
        string("COMPLEX"),  
        "SharedFunctionalGroupsSequence",   
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFrameTypeSequence",  
        0,                                 
        "AcquisitionContrast",            
        "UNKNOWN",
        "SharedFunctionalGroupsSequence",   
        0                                 
    );
}


/*!
 *  Specific to sequence, may need to be overridden in sub-class
 *  for a specific acquisition.    
 */
void svkGEPFileMapper::InitMRTimingAndRelatedParametersMacro()
{
    this->dcmHeader->InitMRTimingAndRelatedParametersMacro(
        this->GetHeaderValueAsFloat( "rhi.tr" ) / 1000, 
        this->GetHeaderValueAsFloat( "rhi.mr_flip" ), 
        this->GetHeaderValueAsInt( "rhi.numecho" ) 
    );
}


/*!
 *
 */
void svkGEPFileMapper::InitMRSpectroscopyFOVGeometryMacro()
{

    this->dcmHeader->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRSpectroscopyFOVGeometrySequence"
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                    
        "SpectroscopyAcquisitionDataColumns",     
        this->GetHeaderValueAsInt( "rhr.rh_frame_size" ), 
        "SharedFunctionalGroupsSequence",   
        0                                 
    );

    int numVoxels[3]; 
    this->GetNumVoxels( numVoxels ); 

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence", 
        0,                                  
        "SpectroscopyAcquisitionPhaseRows", 
        numVoxels[1],
        "SharedFunctionalGroupsSequence",      
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SpectroscopyAcquisitionPhaseColumns",
        numVoxels[0],
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SpectroscopyAcquisitionOutOfPlanePhaseSteps",
        numVoxels[2],
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "PercentSampling",
        this->GetHeaderValueAsFloat( "rhi.nex" ), 
        "SharedFunctionalGroupsSequence",    
        0
    );

    //  Is this supposed to be the in plane aspect ratio of the 2 phase encode fovs?
    float fov[3];  
    this->GetFOV( fov ); 
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "PercentPhaseFieldOfView",
        fov[0]/fov[1],
        "SharedFunctionalGroupsSequence",    
        0
    );

    double* center = new double[3]; 
    this->GetCenterFromRawFile( center ); 

    //===============================
    //  Spacing
    //===============================
    double voxelAcqSpacing[3]; 
    this->GetVoxelSpacing( voxelAcqSpacing ); 
    if ( this->IsSwapOn() ) {
        double tmp = voxelAcqSpacing[0];
        voxelAcqSpacing[0] = voxelAcqSpacing[1];
        voxelAcqSpacing[1] = tmp;
    } 

    ostringstream ossAcqSpacing;
    ostringstream ossAcqSliceThickness;
    for (int i = 0; i < 2; i++) {
        ossAcqSpacing << voxelAcqSpacing[i];
        if (i != 1 ) {
            ossAcqSpacing << "\\";
        }
    }
    ossAcqSliceThickness << voxelAcqSpacing[2];

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionPixelSpacing",
        ossAcqSpacing.str(),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionSliceThickness",
        ossAcqSliceThickness.str(), 
        "SharedFunctionalGroupsSequence",
        0
    );

    //===============================
    //  DCOS 
    //===============================
    double acqDcos[3][3]; 
    this->GetDcos(acqDcos); 
    if ( this->IsSwapOn() ) {

        for( int i = 0; i < 3; i++ ) {
            if( acqDcos[1][i] != 0 ) {
                acqDcos[1][i] = -1 * acqDcos[1][i];
            }
        }

        double tmp;  
        for(int i = 0; i < 3; i++){
            tmp = acqDcos[0][i];
            acqDcos[0][i] = acqDcos[1][i];
            acqDcos[1][i] = tmp;
        }
    }

    ostringstream ossAcqDcos;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            ossAcqDcos << acqDcos[i][j];
            if (i * j != 4 ) {
                ossAcqDcos<< "\\";
            }
        }
    }

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionOrientation",
        ossAcqDcos.str(), 
        "SharedFunctionalGroupsSequence",
        0
    );

    //===============================
    // TLC 
    //===============================
    double acqToplc[3]; 
    for( int i = 0; i < 3; i++ ) {
        acqToplc[i] = center[i];
        for( int j = 0; j < 3; j++ ) {
            acqToplc[i] -= acqDcos[j][i] * voxelAcqSpacing[j] * ( numVoxels[j] / 2.0 - 0.5 );
        }    
    }

    ostringstream ossAcqTlc;
    for (int i = 0; i < 3; i++) {
        ossAcqTlc<< acqToplc[i];
        if (i != 2 ) {
            ossAcqTlc << "\\";
        }
    }

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionTLC",
        ossAcqTlc.str(), 
        "SharedFunctionalGroupsSequence",
        0
    );


    // ==================================================
    // ==================================================
    //  Reordered Params
    // ==================================================
    // ==================================================
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedPhaseColumns",
        numVoxels[0],  
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedPhaseRows",
        numVoxels[1], 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedOutOfPlanePhaseSteps",
        numVoxels[2], 
        "SharedFunctionalGroupsSequence",
        0
    );

    double voxelSpacing[3]; 
    this->GetVoxelSpacing( voxelSpacing ); 

    double dcos[3][3];
    this->GetDcos(dcos);   

    //===============================
    // TLC 
    //===============================
    double toplc[3]; 
    for( int i = 0; i < 3; i++ ) {
        toplc[i] = center[i];
        for( int j = 0; j < 3; j++ ) {
            toplc[i] -= dcos[j][i] * voxelSpacing[j] * ( numVoxels[j] / 2.0 - 0.5 );
        }    
    }

    ostringstream ossOrigin;
    for (int i = 0; i < 3; i++) {
        ossOrigin << toplc[i];
        if (i != 2 ) {
            ossOrigin << "\\";
        }
    }

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedTLC",
        ossOrigin.str(), 
        "SharedFunctionalGroupsSequence",
        0
    );

    //===============================
    //  Spacing
    //===============================
    ostringstream ossSpacing;
    for (int i = 0; i < 2; i++) {
        ossSpacing << voxelSpacing[i];
        if (i != 1 ) {
            ossSpacing << "\\";
        }
    }

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedPixelSpacing",
        ossSpacing.str(), 
        "SharedFunctionalGroupsSequence",
        0
    );

    ostringstream ossSliceThickness;
    ossSliceThickness << voxelSpacing[2]; 

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedSliceThickness",
        ossSliceThickness.str(), 
        "SharedFunctionalGroupsSequence",
        0
    );

    //===============================
    //  DCOS 
    //===============================
    ostringstream ossDcos;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            ossDcos << dcos[i][j];
            if (i * j != 4 ) {
                ossDcos<< "\\";
            }
        }
    }

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedOrientation",
        ossDcos.str(), 
        "SharedFunctionalGroupsSequence",
        0
    );

    delete[] center; 
}


/*!
 *
 */
void svkGEPFileMapper::InitMREchoMacro()
{
    this->dcmHeader->InitMREchoMacro( 
        this->GetHeaderValueAsFloat("rhi.te")/1000 
    ); 
}


/*!
 *  Inversion timing and other acquisition params 
 */
void svkGEPFileMapper::InitMRModifierMacro()
{
    this->dcmHeader->InitMRModifierMacro(
        this->GetHeaderValueAsFloat( "rhi.ti" )
    );
}


/*!
 *  Should work for single vs multi-coil, but will not currently 
 *  differentiate between volume, surface, body coils
 */
void svkGEPFileMapper::InitMRReceiveCoilMacro()
{

    this->dcmHeader->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRReceiveCoilSequence"
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,                        
        "ReceiveCoilName",       
        this->GetHeaderValueAsString( "rhi.cname" ), 
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,                        
        "ReceiveCoilManufacturerName",       
        string(""),
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    string coilType( "VOLUME" ); 
    string quadCoil("YES"); 
    if ( this->GetNumCoils() > 1 ) {
        coilType.assign("MULTICOIL"); 
        quadCoil.assign("NO"); 
        
    }

    this->dcmHeader->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,                        
        "ReceiveCoilType",       
        coilType, 
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,                        
        "QuadratureReceiveCoil",       
        quadCoil, 
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    //=====================================================
    //  Multi Coil Sequence
    //=====================================================
    if ( this->GetNumCoils() > 1 ) {

        this->dcmHeader->AddSequenceItemElement( 
            "MRReceiveCoilSequence",
            0, 
            "MultiCoilDefinitionSequence"
        );

        for (int coilIndex = 0; coilIndex < this->GetNumCoils(); coilIndex++) {

            ostringstream ossIndex;
            ossIndex << coilIndex;
            string indexString(ossIndex.str());

            this->dcmHeader->AddSequenceItemElement(
                "MultiCoilDefinitionSequence",
                coilIndex,                        
                "MultiCoilElementName",       
                coilIndex, 
                "MRReceiveCoilSequence",
                0                      
            );


            this->dcmHeader->AddSequenceItemElement(
                "MultiCoilDefinitionSequence",
                coilIndex,                        
                "MultiCoilElementUsed",       
                "YES", 
                "MRReceiveCoilSequence",
                0                      
            );
        }

    }

}


/*!
 *
 */
void svkGEPFileMapper::InitMRTransmitCoilMacro()
{

    this->dcmHeader->InitMRTransmitCoilMacro(
        "GE",
        "UNKNOWN",
        "BODY"
    );

}


/*!
 *
 */
void svkGEPFileMapper::InitMRAveragesMacro()
{
    this->dcmHeader->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRAveragesSequence"
    );

    //  GE overloads the meaning of nex so that it also
    //  implicitly reflects any reduced k-space sampling.  
    //  for example 1 avererage with 50% of the rectilinear
    //  k-space grid sampled would give nex = 0.5.  
    //  for now I'm using ceil to round up, but may not be
    //  accurate in all cases. 
    this->dcmHeader->AddSequenceItemElement(
        "MRAveragesSequence",
        0,                        
        "NumberOfAverages",       
        static_cast<int> ( ceil( this->GetHeaderValueAsFloat( "rhi.nex" ) ) ), 
        "SharedFunctionalGroupsSequence",    
        0
    );

}


/*!
 *  Initialize sat band information:
 *      1.  from pfile Header:   image.user25-48 
 *      2.  additional sat bands encoded in xml (.dat) file
 */
void svkGEPFileMapper::InitMRSpatialSaturationMacro()
{

    this->dcmHeader->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRSpatialSaturationSequence"
    );

    //  First init the sat bands in the header: 
    //  if this field is 3, then sat bands are defined in user CVs.    
    if ( this->GetHeaderValueAsInt( "rhr.rh_user_usage_tag" ) == 3 ) { 

        //  Sat band info (up to 6) is contained in 4 sequential user CVs:
        for ( int i = 25; i < 49; i += 4 ) {

            ostringstream index;
            index << i;
            string ras0 = "rhi.user" + index.str(); 

            index.clear(); 
            index.str( "" ); 
            index << i + 1;
            string ras1 = "rhi.user" + index.str(); 
    
            index.clear(); 
            index.str( "" ); 
            index << i + 2;
            string ras2 = "rhi.user" + index.str(); 
    
            index.clear(); 
            index.str( "" ); 
            index << i + 3;
            string t = "rhi.user" + index.str(); 
    
            float satRAS[3];
            satRAS[0] = this->GetHeaderValueAsFloat( ras0 ); 
            satRAS[1] = this->GetHeaderValueAsFloat( ras1 ); 
            satRAS[2] = this->GetHeaderValueAsFloat( ras2 ); 
            float translation = this->GetHeaderValueAsFloat( t ); 
    
            if ( satRAS[0] != 0 || satRAS[1] != 0 || satRAS[2] != 0 || translation != 0) {
                this->InitSatBand(satRAS, translation);
            }

        }
    }

    //  Now init any sat bands stored in .dat/xml file:        
    this->InitSatBandsFromXML();
}


/*!
 * 
 */
void svkGEPFileMapper::InitSatBandsFromXML()
{
    return; 
}


/*!  
 *  Add a sat band to the SpatialSaturationSequence: 
 *  RAS:          vector of the normal to the sat band with length 
 *                equal to the band thickness in RAS coordinates. 
 *  translation : translation along that vector from origin to sat band location (slab farthest from
 *                origin)
 */
void svkGEPFileMapper::InitSatBand( float satRAS[3], float translation)
{

    float satThickness = sqrt( 
                            satRAS[0] * satRAS[0]
                          + satRAS[1] * satRAS[1] 
                          + satRAS[2] * satRAS[2]
                        );

    float orientation[3]; 
    float position[3];
    for(int i = 0; i < 3; i++) {

        //  orientation (normal vector): 
        //  Back to LPS.  
        orientation[i] = ( i < 2 ) ? ( -satRAS[i] / satThickness ) : ( satRAS[i] / satThickness );

        //  translation given is towards the "farther" edge of the sat band. 
        //  we need to get the center plane = that's why it is
        //  -0.5*thickness
        //  quotes, since it's not necessarily the "farther" plane - when 
        //  the sat band is rotated around, it will be the closer plane,
        //  but then the normal vector will be in the opposite direction 
        //  also, so our formula will still work

        position[i] = orientation[i] * ( translation - 0.5 * satThickness );
    }

    string sequenceName       = "MRSpatialSaturationSequence"; 
    string parentSequenceName = "SharedFunctionalGroupsSequence"; 

    int satBandNumber = this->dcmHeader->GetNumberOfItemsInSequence(
        sequenceName.c_str(), 
        parentSequenceName.c_str(), 
        0
    );
 
    this->dcmHeader->AddSequenceItemElement(
        sequenceName.c_str(), 
        satBandNumber,                        
        "SlabThickness",       
        satThickness, 
        parentSequenceName.c_str(), 
        0
    );
        
    string slabOrientation;
    for (int j = 0; j < 3; j++) {
        ostringstream ossOrientation;
        ossOrientation << orientation[j];
        slabOrientation += ossOrientation.str();
        if (j < 2) {
            slabOrientation += '\\';
        }
    }

    this->dcmHeader->AddSequenceItemElement(
        sequenceName.c_str(), 
        satBandNumber,                        
        "SlabOrientation",       
        slabOrientation,
        parentSequenceName.c_str(), 
        0                      
    );

    string slabPosition;
    for (int j = 0; j < 3; j++) {
        ostringstream ossPosition;
        ossPosition << position[j];
        slabPosition += ossPosition.str();
        if (j < 2) {
            slabPosition += '\\';
        }
    }
    this->dcmHeader->AddSequenceItemElement(
        sequenceName.c_str(), 
        satBandNumber,                        
        "MidSlabPosition",       
        slabPosition,
        parentSequenceName.c_str(), 
        0                      
    );

}


/*!
 *
 */
void svkGEPFileMapper::InitAcquisitionContextModule()
{
}


/*!
 *
 */
void svkGEPFileMapper::InitMRSpectroscopyModule()
{

    /*  =======================================
     *  MR Image and Spectroscopy Instance Macro
     *  ======================================= */

    string dcmDate = this->ConvertGEDateToDICOM(this->GetHeaderValueAsString( "rhr.rh_scan_date" ));
    this->dcmHeader->SetValue(
        "AcquisitionDateTime",
        svkImageReader2::RemoveDelimFromDate( &dcmDate ) + "000000"
    );

    this->dcmHeader->SetValue(
        "AcquisitionDuration",
        0
    );

    string nucleus = this->GetNucleus();

    this->dcmHeader->SetValue(
        "ResonantNucleus", 
        nucleus
    );

    this->dcmHeader->SetValue(
        "KSpaceFiltering", 
        "NONE" 
    );

    this->dcmHeader->SetValue(
        "ApplicableSafetyStandardAgency", 
        "Research" 
    );

    this->dcmHeader->SetValue(
        "MagneticFieldStrength", 
        this->GetHeaderValueAsFloat( "rhe.magstrength" ) / 10000. 
    );
    /*  =======================================
     *  END: MR Image and Spectroscopy Instance Macro
     *  ======================================= */

    this->dcmHeader->SetValue(
        "ImageType", 
        string("ORIGINAL\\PRIMARY\\SPECTROSCOPY\\NONE") 
    );


    /*  =======================================
     *  Spectroscopy Description Macro
     *  ======================================= */
    this->dcmHeader->SetValue(
        "VolumetricProperties", 
        string("VOLUME")  
    );

    this->dcmHeader->SetValue(
        "VolumeBasedCalculationTechnique", 
        string("NONE")
    );

    this->dcmHeader->SetValue(
        "ComplexImageComponent", 
        string("COMPLEX")  
    );

    this->dcmHeader->SetValue(
        "AcquisitionContrast", 
        "UNKNOWN"  
    );
    /*  =======================================
     *  END: Spectroscopy Description Macro
     *  ======================================= */


    this->dcmHeader->SetValue(
        "TransmitterFrequency", 
        this->GetHeaderValueAsFloat( "rhr.rh_ps_mps_freq" ) * 1e-7
    );

    this->dcmHeader->SetValue(
        "SpectralWidth", 
        this->GetHeaderValueAsFloat( "rhr.spectral_width" )
    );

    this->dcmHeader->SetValue(
        "SVK_FrequencyOffset", 
        this->GetFrequencyOffset() 
    );

    this->dcmHeader->SetValue(
        "ChemicalShiftReference", 
        this->GetPPMRef() 
    );

    string localizationType; 
    localizationType = this->GetVolumeLocalizationTechnique(); 

    this->dcmHeader->SetValue(
        "VolumeLocalizationTechnique", 
        localizationType
    );

    if ( localizationType.compare("PRESS") == 0 )  { 
        this->InitVolumeLocalizationSeq();
    }


    this->dcmHeader->SetValue(
        "Decoupling", 
        "NO" 
    );

    this->dcmHeader->SetValue(
        "TimeDomainFiltering", 
        "NONE" 
    );

    this->dcmHeader->SetValue(
        "NumberOfZeroFills", 
        0 
    );

    this->dcmHeader->SetValue(
        "BaselineCorrection", 
        string("NONE")
    );

    this->dcmHeader->SetValue(
        "FrequencyCorrection", 
        "NO"
    );

    this->dcmHeader->SetValue(
        "FirstOrderPhaseCorrection", 
        string("NO")
    );

    this->dcmHeader->SetValue(
        "WaterReferencedPhaseCorrection", 
        string("NO")
    );
}


/*!
 *
 */
string svkGEPFileMapper::GetNucleus()
{

    //  Use the mps freq and field strength to determine gamma which is 
    //  characteristic of the isotop:
    float gamma = ( this->GetHeaderValueAsFloat( "rhr.rh_ps_mps_freq" ) * 1e-7 )  
            / ( this->GetHeaderValueAsFloat( "rhe.magstrength" ) / 10000. );
   
    string nucleus;  
    if ( fabs( gamma - 42.57 ) < 0.1 ) {
        nucleus.assign("1H");
    } else if ( fabs( gamma - 10.7 ) <0.1 ) {
        nucleus.assign("13C");
    } else if ( fabs( gamma - 17.235 ) <0.1 ) {
        nucleus.assign("31P");
    }

    return nucleus; 
}


/*
 *  Returns the volume localization type, e.g. PRESS. 
 */
string  svkGEPFileMapper::GetVolumeLocalizationTechnique()
{
    string localizationType; 

    if ( this->pfileVersion >= 9 ) {
        localizationType.assign( "PRESS" ); 
    } else {
        float user0 = this->GetHeaderValueAsFloat( "rhr.rh_user0" );
        if( user0 == 3 || user0 == 1 ) {
            localizationType.assign( "PRESS" ); 
        }
    }

    return localizationType;     
}


/*!
 *  Returns the spectral frquency offset
 */
float svkGEPFileMapper::GetFrequencyOffset() 
{
    if ( this->pfileVersion >= 9  ) {
        return 0.;
    } else {
        return this->GetHeaderValueAsFloat( "rhr.rh_user13" );
    }

}


/*!
 *  Gets the chemical shift reference taking into account acquisition frequency offset
 *  and the acquisition sample temperature. 
 */
float svkGEPFileMapper::GetPPMRef() 
{

    string nucleus = this->GetNucleus(); 

    float ppmRef;
    if ( nucleus.compare("13C") == 0 ) {

        ppmRef = 178; 

    } else {

        float freqOffset;

        if ( this->pfileVersion >= 9 ) {
            freqOffset = 0.0; 
        } else {
            freqOffset = this->GetHeaderValueAsFloat( "rhr.rh_user13" ); 
        }

        float transmitFreq = this->GetHeaderValueAsFloat( "rhr.rh_ps_mps_freq" ) * 1e-7; 

        //  Get mapper behavior flag value:
        map < string, void* >::iterator  it;
        it = this->inputArgs.find( "temperature" );

        if ( it != this->inputArgs.end() ) {
            float temp = *( static_cast< float* >( this->inputArgs[ it->first ] ) );
            ppmRef = svkSpecUtils::GetPPMRef(transmitFreq, freqOffset, temp);
        } else {
            ppmRef = svkSpecUtils::GetPPMRef(transmitFreq, freqOffset);
        }
    }

    return ppmRef; 
}


/*!
 *
 */
void svkGEPFileMapper::InitMRSpectroscopyPulseSequenceModule()
{
    this->dcmHeader->SetValue(
        "PulseSequenceName", 
        this->GetHeaderValueAsString( "rhi.psdname" )
    );

    int numVoxels[3]; 
    this->GetNumVoxels( numVoxels ); 

    string acqType = "VOLUME"; 
    if (numVoxels[0] == 1 && numVoxels[1] == 1 &&  numVoxels[2] == 1) {
        acqType = "SINGLE VOXEL";
    }
    this->dcmHeader->SetValue( "MRSpectroscopyAcquisitionType", acqType ); 

    this->dcmHeader->SetValue( "EchoPulseSequence", "SPIN" ); 
    this->dcmHeader->SetValue( "MultipleSpinEcho", "NO" );   
    this->dcmHeader->SetValue( "MultiPlanarExcitation", "NO" ); 
    this->dcmHeader->SetValue( "SteadyStatePulseSequence", "NONE" ); 
    this->dcmHeader->SetValue( "EchoPlanarPulseSequence", "NO" );  
    this->dcmHeader->SetValue( "SpectrallySelectedSuppression", "WATER" ); 
    this->dcmHeader->SetValue( "GeometryOfKSpaceTraversal", "RECTILINEAR" ); 
    this->dcmHeader->SetValue( "RectilinearPhaseEncodeReordering", "LINEAR" );  
    this->dcmHeader->SetValue( "SegmentedKSpaceTraversal", "SINGLE" ); 

    string kspaceCoverage; 
    if ( this->GetNumKSpacePoints() <  this->GetNumVoxelsInVol() ) {
        kspaceCoverage = "ELLIPSOIDAL"; 
    } else {
        kspaceCoverage = "FULL"; 
    }
    this->dcmHeader->SetValue( "CoverageOfKSpace", kspaceCoverage ); 

    this->dcmHeader->SetValue( "NumberOfKSpaceTrajectories", 1); 

    string chop = "NO"; 
    if ( IsChopOn() ) {
        chop = "YES"; 
    }
    this->dcmHeader->SetValue( "SVK_AcquisitionChop", chop ); 
}


/*!
 *
 */
void svkGEPFileMapper::InitMRSpectroscopyDataModule()
{
    int numVoxels[3];
    this->GetNumVoxels( numVoxels ); 

    this->dcmHeader->SetValue(
        "Rows", 
        numVoxels[1]
    );

    this->dcmHeader->SetValue(
        "Columns", 
        numVoxels[0]
    );

    this->dcmHeader->SetValue(
        "DataPointRows", 
        1
    );

    this->dcmHeader->SetValue(
        "DataPointColumns", 
        this->GetHeaderValueAsInt( "rhr.rh_frame_size" )
    );

    string representation( "COMPLEX" );

    this->dcmHeader->SetValue(
        "DataRepresentation", 
        representation 
    );

    string signalDomain("TIME"); 

    this->dcmHeader->SetValue(
        "SignalDomainColumns", 
        signalDomain 
    );

    //  Single Voxel doesn't require spatial tranform 
    string spatialDomain = "KSPACE"; 

    this->dcmHeader->SetValue( "SVK_ColumnsDomain", spatialDomain );
    this->dcmHeader->SetValue( "SVK_RowsDomain", spatialDomain);
    this->dcmHeader->SetValue( "SVK_SliceDomain", spatialDomain );

    this->InitK0Sampled( this->dcmHeader );
}


/*!
 *  Initializes the parameter SVK_K0Sampled based on symmetry and
 *  dimensions of the dataset (odd/even).
 *
 */
void svkGEPFileMapper::InitK0Sampled( svkDcmHeader* hdr )
{
    string kSpaceSymmetry;
    if ( this->pfileVersion >= 9 ) {
        kSpaceSymmetry = "EVEN";
    } else {
        kSpaceSymmetry = this->GetHeaderValueAsString( "rhr.rh_user15" );
    }


    //  If k-space data and sym is even: set to NO (assume GE data with even num pts)
    //  method: bool isK0Sampled( sym(even/odd), dimension(even/odd) );
    int numVoxels[3];
    numVoxels[0] = hdr->GetIntValue( "Columns");
    numVoxels[1] = hdr->GetIntValue( "Rows");
    numVoxels[2] = hdr->GetNumberOfSlices();

    string k0Sampled = "YES";
    //cout << "Num Voxels :" << numVoxels[0] << " " << numVoxels[1] << " " << numVoxels[2] << endl;
    //cout << "Num Voxels :" << numVoxels[0] %2 << " " << numVoxels[1] %2 << " " << numVoxels[2] %2 << endl;
    // data dims odd?
    if ( (numVoxels[0] > 1 && numVoxels[0] % 2) 
            || (numVoxels[1] > 1 && numVoxels[1] % 2 ) 
            || (numVoxels[2] > 1 && numVoxels[2] % 2 ) ) {
        if ( kSpaceSymmetry.compare("EVEN") == 0 ) {
            k0Sampled = "YES";
        } else {
            k0Sampled = "NO";
        }
    } else {
        if ( kSpaceSymmetry.compare("EVEN") == 0 ) {
            k0Sampled = "NO";
        } else {
            k0Sampled = "YES";
        }
    }
    hdr->SetValue( "SVK_K0Sampled", k0Sampled);
}


/*
 *  Gets the center of the acquisition grid.  May vary between sequences.
 */
void svkGEPFileMapper::GetCenterFromRawFile( double* center )
{   
    if ( this->pfileVersion < 11  ) {
        center[0] = 0;
        center[1] = 0;
        center[2] = this->GetHeaderValueAsFloat( "rhi.user13" );
    } else {
        center[0] = -1 * this->GetHeaderValueAsFloat( "rhi.user11" );
        center[1] = -1 * this->GetHeaderValueAsFloat( "rhi.user12" );
        center[2] = this->GetHeaderValueAsFloat( "rhi.user13" );
    }
}


/*!
 *  returns the value for the specified key as an int. 
 */
int svkGEPFileMapper::GetHeaderValueAsInt(string key)
{

    istringstream* iss = new istringstream();
    int value;

    iss->str( pfMap[key][3] );
    *iss >> value;
    delete iss; 
    return value;
}


/*!
 *  returns the value for the specified key as a long long int. 
 *  An int may be 4 or 8 bytes, but long int should be 8 as needed.
 */
long long int svkGEPFileMapper::GetHeaderValueAsLongInt(string key)
{

    istringstream* iss = new istringstream();
    long long int value;

    iss->str( pfMap[key][3] );
    *iss >> value;
    delete iss; 
    return value;
}


/*!
 *  returns the value for the specified key as a float. 
 */
float svkGEPFileMapper::GetHeaderValueAsFloat(string key)
{

    istringstream* iss = new istringstream();
    float value;

    iss->str( pfMap[key][3] );
    *iss >> value;
    delete iss; 
    return value;
}


/*!
 *  returns the value for the specified key as a string. 
 */
string svkGEPFileMapper::GetHeaderValueAsString( string key )
{         
    return this->pfMap[key][3];
}         


/*!
 *  Determine number of coils of data in the PFile. 
 */
int svkGEPFileMapper::GetNumCoils()
{
    int numCoils = 0;

    for ( int i = 0; i < 4 ; i++ ) {

        ostringstream index;
        index << i;
        string start_rcv = "rhr.rh_dab[" + index.str() + "].start_rcv"; 
        string stop_rcv  = "rhr.rh_dab[" + index.str() + "].stop_rcv"; 

        int startRcv = this->GetHeaderValueAsInt( start_rcv ); 
        int stopRcv  = this->GetHeaderValueAsInt( stop_rcv ); 
        
        if ( startRcv != 0 || stopRcv != 0) {
            numCoils += 
                ( stopRcv - startRcv ) + 1;
        }         
    }

    //  Otherwise 1 
    if( numCoils == 0) {
        numCoils = 1; 
    }

    return numCoils; 
}


/*!
 *  Determine number of time points in the PFile. 
 *  Number of time points is determined from the file size, 
 *  number of voxels and number of coils.
 */
int svkGEPFileMapper::GetNumTimePoints()
{

    long long int passSize = this->GetHeaderValueAsLongInt( "rhr.rh_raw_pass_size" ); 
    int numCoils = this->GetNumCoils(); 
    int numVoxels = this->GetNumVoxelsInVol(); 
    int dataWordSize = this->GetHeaderValueAsInt( "rhr.rh_point_size" ); 
    int numFreqPoints = this->GetHeaderValueAsInt( "rhr.rh_frame_size" ); 

    int numTimePoints = static_cast<int>(
        static_cast<float>( passSize ) / static_cast<float>( numCoils * 2 * dataWordSize * numFreqPoints )  - 1 
    ) / this->GetNumKSpacePoints(); 

    return numTimePoints; 
}


/*!
 */
int svkGEPFileMapper::GetNumEPSIAcquisitions() 
{

    //  If this is not an EPSI acquisition return 1
    int numAcquisitions = 1;     
    map < string, void* >::iterator  it;
    it = this->inputArgs.find( "epsiType" );

    int epsiType;
    if ( it != this->inputArgs.end() ) {
        EPSIType epsiType = *( static_cast< EPSIType* >( this->inputArgs[ it->first ] ) );
        if ( epsiType == SYMMETRIC || epsiType == INTERLEAVED ) {
            numAcquisitions = 2; 
        }
    } 

    return numAcquisitions; 

}



/*!
 *  Determine whether to add a dummy scan. The assumption is that
 *  the number of dummy scans should be equal to the number of coils 
 *  or numCoils * numTimePts (e.g. for a spectral editing sequence). 
 *  If true, then the an FID worth of data should be skipped over when 
 *  reading data (e.g. frame_size * numComponents, or numFreqPts * numComponents) 
 */
bool svkGEPFileMapper::AddDummy( int offset, int coilNum, int timePt ) 
{
    int numDummyScans = this->GetNumDummyScans();  
    int numCoils = this->GetNumCoils(); 
    int numTimePoints = this->GetNumTimePoints();
    int numSampledVoxels = this->GetNumKSpacePoints(); 
    int numFreqPoints = this->GetHeaderValueAsInt( "rhr.rh_frame_size" ); 
    int numComponents = 2; 
    int numPointsPerFID = numFreqPoints * numComponents; 

    int numWordsBetweenDummies; 
    //  subtract the number of dummy words from the current offset to see if another 
    //  dummy scan should be skipped or not 
    if ( numDummyScans == numCoils )  {
        numWordsBetweenDummies = numSampledVoxels * numPointsPerFID * numTimePoints;  
        offset = offset - (coilNum * numPointsPerFID);
        //  additional time points have an offset that includes the 
        //  per-coil dummy 
        if (timePt > 1 ) {
            offset = offset - (numPointsPerFID);
        }
    } else if ( numDummyScans == (numCoils * numTimePoints) )  {
        numWordsBetweenDummies = numSampledVoxels * numPointsPerFID;
        offset = offset - (coilNum * numPointsPerFID) - ( ( coilNum + timePt ) * numPointsPerFID );
    } else {
        cerr << "ERROR: Can not determine placement of dummy scans in raw file reader. \n"; 
        exit(1); 
    }

    bool addDummy = false; 
    if ( ( offset % numWordsBetweenDummies ) == 0 ) {
        addDummy = true; 
    }

    return addDummy; 

}


/*!
 *  Determine number of dummy scans (FIDs) in the data block. 
 *  This is the difference between the raw pass size and the 
 *  expected size of the data based on numCoils, numTimePts, numKSpacePts
 *  and numFreqPts. 
 */
int svkGEPFileMapper::GetNumDummyScans()
{

    long long int passSize = this->GetHeaderValueAsLongInt( "rhr.rh_raw_pass_size" ); 
    int numCoils = this->GetNumCoils(); 
    int numTimePoints = this->GetNumTimePoints();
    int numSampledVoxels = this->GetNumKSpacePoints(); 
    int numFreqPoints = this->GetHeaderValueAsInt( "rhr.rh_frame_size" ); 
    int dataWordSize = this->GetHeaderValueAsInt( "rhr.rh_point_size" ); 

    string dataRepresentation = this->dcmHeader->GetStringValue( "DataRepresentation" );
    int numComponents;
    if ( dataRepresentation == "COMPLEX" ) {
        numComponents = 2;
    } else {
        numComponents = 1;
    }

    //  Calc the diff between the size of the data buffer and the number of real data points
    //  then divide by the number of bytes in a single fid to get the number of dummy FIDs
    int numDummyScans =
         passSize  - ( numCoils * numTimePoints * numSampledVoxels * numFreqPoints * numComponents * dataWordSize ); 

    numDummyScans = numDummyScans / ( numFreqPoints * numComponents * dataWordSize);

    //cout << "NumCoils:      " << numCoils << endl;
    //cout << "numTimePoints: " << numTimePoints << endl;
    //cout << "numDummyScans: " << numDummyScans << endl;

    return numDummyScans; 

}


/*!
 *   Number of frames is number of slices * numCoils * numTimePoints
 */
int svkGEPFileMapper::GetNumFrames()
{

    int numVoxels[3]; 
    this->GetNumVoxels( numVoxels ); 

    int numFrames = 
              numVoxels[2]  
            * this->GetNumCoils() 
            * this->GetNumTimePoints(); 

    return numFrames; 
}


/*!
 *
 */
int svkGEPFileMapper::GetNumVoxelsInVol()
{
    int numVoxels[3]; 
    this->GetNumVoxels( numVoxels ); 

    return  numVoxels[0] * numVoxels[1] * numVoxels[2] ;  
}


/*!
 *  Determine the number of sampled k-space points in the data set. 
 *  This may differ from the number of voxels in the rectalinear grid, 
 *  for example if elliptical or another non rectangular acquisition 
 *  sampling strategy was employed.  GE product sequences pad the
 *  reduced k-space data with zeros so the number of k-space points 
 *  is the same as the number of voxels, but that may not be true for
 *  custom sequences.  
 */
int svkGEPFileMapper::GetNumKSpacePoints()
{
    int numKSpacePts = GetNumVoxelsInVol(); 

    if ( this->GetDebug() ) {
        cout << "NUM KSPACE PTS: " << numKSpacePts << endl; 
    }

    return numKSpacePts; 
}


/*!
 *  Determines whether a voxel (index) was sampled (or a zero padded
 *  point is present in the data set), or not, i.e. was it within 
 *  the elliptical sampling volume if reduced k-space elliptical sampling
 *  was used. Could be extended to support other sparse sampling 
 *  trajectories. Note that for product sequences this always returns true 
 *  since GE  zero-pads reduced k-space data to a full rectilinear grid.  
 */
bool svkGEPFileMapper::WasIndexSampled(int indexX, int indexY, int indexZ)
{
    bool wasSampled = true;
    return wasSampled; 
}


/*!
 *  This method reads data from the pfile and puts the data into the CellData arrays.
 *  if elliptical k-space sampling was used, the data is zero-padded.  Other reduced
 *  k-space sampling strategies aren't supported yet. 
 */
void svkGEPFileMapper::ReadData(vtkStringArray* pFileNames, svkImageData* data)
{
    int numCoils = this->GetNumCoils(); 
    int numTimePts = this->GetNumTimePoints(); 
    int numSpecPts = this->GetHeaderValueAsInt( "rhr.rh_frame_size" ); 
    int numComponents =  2; 
    int dataWordSize = this->GetHeaderValueAsInt( "rhr.rh_point_size" ); 
        
    int numBytesInVol = this->GetNumKSpacePoints() * numSpecPts * numComponents * dataWordSize; 
    int numBytesPerCoil = numBytesInVol * numTimePts; 
    int numBytesInPFile = numBytesInVol * numCoils; 

    int numPtsPerSpectrum = numSpecPts * numComponents;

    //  one dummy spectrum per volume/coil:
    int numDummyBytes = this->GetNumDummyScans() * numPtsPerSpectrum * dataWordSize; 
    int numDummyBytesPerCoil = numDummyBytes/numCoils;  

    numBytesPerCoil += numDummyBytesPerCoil;  

    //  Only read in one coil at a time to reduce memory footprint
    this->specData = new int[ numBytesPerCoil / dataWordSize ];  

    ifstream* pFile = new ifstream();
    pFile->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

    try {
        pFile->open( pFileNames->GetValue(0).c_str(), ios::binary);
    
        pFile->seekg(0, ios::beg);
        int readOffset = this->GetHeaderValueAsInt( "rhr.rdb_hdr_off_data" );
        pFile->seekg(readOffset, ios::beg);

    } catch (ifstream::failure e) {
        cout << "ERROR: Exception opening/reading file " << pFileNames->GetValue(0) << " => " << e.what() << endl;
        exit(1); 
    }
    
    int numVoxels[3]; 
    this->GetNumVoxels( numVoxels ); 

    int cols = numVoxels[0];
    int rows = numVoxels[1];
    int slices = numVoxels[2];
    int coilOffset = cols * rows * slices * numTimePts;     //number of spectra per coil
    int timePtOffset = cols * rows * slices;
	int arraysPerVolume = cols * rows * slices;


    //  Preallocate data arrays. The API only permits dynamic assignmet at end of CellData, so for
    //  swapped cases where we need to insert data out of order they need to be preallocated.
    for (int arrayNumber = 0; arrayNumber < arraysPerVolume * numTimePts * numCoils; arrayNumber++) {
        vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
        data->GetCellData()->AddArray(dataArray); 
        dataArray->Delete(); 
    }

    //  Offset is the offset within the current block of loaded data (i.e. within
    //  a given coil.  
    int offset = 0; 
    //  Blank scan prepended to data blocks.
    int dummyOffset = 0; 
    dummyOffset = numPtsPerSpectrum; 

    //  If Chop On, then reinitialize chopVal:
    if ( IsChopOn() ) {
        this->chopVal = -1; 
    }
    int denominator = numVoxels[2] * numVoxels[1]  * numVoxels[0] + numVoxels[1]*numVoxels[0] + numVoxels[0];

    int x; 
    int y; 
    int z; 
    int index; 

    //  pFileOOffset is the offset of the current data set within the pFile (i.e. a global data offset). 
    //  a given coil.  
    int pFileOffset = 0;  

    for (int coilNum = 0; coilNum < numCoils; coilNum++) {
        offset = 0; 

        pFile->read( (char*)(this->specData), numBytesPerCoil);
        if ( this->swapBytes ) {
            vtkByteSwap::SwapVoidRange((void *)this->specData, numBytesPerCoil / dataWordSize, dataWordSize);
        }

        for (int timePt = 0; timePt < numTimePts; timePt++) {

            //  Should a dummy scan be skipped over? 
            if ( this->AddDummy( pFileOffset, coilNum, timePt ) ) {
                offset += dummyOffset;  
                pFileOffset += dummyOffset;
            }

            ostringstream progressStream;
            progressStream <<"Reading Time Point " << timePt+1 << "/"
                           << numTimePts << " and Channel: " << coilNum+1 << "/" << numCoils;
            this->SetProgressText( progressStream.str().c_str() );
			for (int arrayNumber = 0; arrayNumber < arraysPerVolume; arrayNumber++) {
				this->GetXYZIndices(arrayNumber, &x, &y, &z);
                                //cout << " indices: " << x << " " << y << " " << z << endl;

				//  Linear index of current cell in target svkImageData
				int linearIndex =  x
						   + ( numVoxels[0] ) * y
						   + ( numVoxels[0] * numVoxels[1] ) * z
						   + ( numVoxels[0] * numVoxels[1] * numVoxels[2] ) * timePt
						   + ( numVoxels[0] * numVoxels[1] * numVoxels[2] * numTimePts ) * coilNum;

				//  if k-space sampling was used check if the point was sampled, or if it needs
				//  to be zero-padded in the grid.
				//  if zero-padding don't increment the data pointer offset.
				bool wasSampled = this->WasIndexSampled(x, y, z);
				SetCellSpectrum(data, wasSampled, offset, linearIndex, x, y, z, timePt, coilNum);
				if ( wasSampled ) {
					offset += numPtsPerSpectrum;
					pFileOffset += numPtsPerSpectrum;
				}

				this->progress = arrayNumber/((double)arraysPerVolume);
				this->UpdateProgress( this->progress );
			}
		}
	}

    pFile->close(); 
    delete pFile;
    delete [] this->specData; 
    this->ModifyBehavior( data );

    if ( this->GetDebug() ) {
        cout << *data << endl;
    }
}


/*! 
 *  If swapping is turned on, the data will need to get mapped correctly 
 *  from the input data buffer read from disk (specData) to the correct 
 *  svkImageData arrays. If swap is true, then the data indices are swapped 
 *  and ky is flipped. 
 */
void svkGEPFileMapper::GetXYZIndices(int dataIndex, int* x, int* y, int* z)
{
    int numVoxels[3];

    this->GetNumVoxels(numVoxels);

    *z = static_cast <int> ( dataIndex/(numVoxels[0] * numVoxels[1]) );

    if ( this->IsSwapOn() ) {

    	// If swap is on use numVoxels[1] for x dimension and numVoxels[0] for y dimension
		*x = static_cast <int> ((dataIndex - (*z * numVoxels[0] * numVoxels[1]))/numVoxels[1]);

    	// In addition to swapping reverse the y direction
		*y = numVoxels[1] - static_cast <int> ( dataIndex%numVoxels[1] ) - 1;

    } else {
		*x = static_cast <int> ( dataIndex%numVoxels[0] );
		*y = static_cast <int> ((dataIndex - (*z * numVoxels[0] * numVoxels[1]))/numVoxels[0]);
    }
}


/*!
 *
 */
void svkGEPFileMapper::SetCellSpectrum(vtkImageData* data, bool wasSampled, int offset, int index, int x, int y, int z, int timePoint, int coilNum)
{

    //  Add the spectrum's dataArray to the CellData:
    //  vtkCellData is a subclass of vtkFieldData
    //  If the data is swapped, the array index needs 
    //  to be set correctly too.  The following index
    //  formula should be general:
    vtkDataArray* dataArray = data->GetCellData()->GetArray(index); 

    string dataRepresentation = this->dcmHeader->GetStringValue( "DataRepresentation" );
    int numComponents;
    if ( dataRepresentation == "COMPLEX" ) {
        numComponents = 2;
    } else {
        numComponents = 1;
    }
    dataArray->SetNumberOfComponents( numComponents );

    int numFreqPts = this->dcmHeader->GetIntValue( "DataPointColumns" );
    dataArray->SetNumberOfTuples(numFreqPts);

    char arrayName[30];
    sprintf(arrayName, "%d %d %d %d %d", x, y, z, timePoint, coilNum);
    dataArray->SetName(arrayName);

    //  Default, chop is off, so multiply all values by 1
    //  Only chop sampled data points.
    if ( IsChopOn() && wasSampled ) {
       this->chopVal *= -1;  
    }

    float tuple[2];
    if ( wasSampled ) {
        for (int i = 0; i < numFreqPts; i++) {
            tuple[0] = this->chopVal * this->specData[ offset + (i * numComponents) ]; 
            tuple[1] = this->chopVal * this->specData[ offset + (i * numComponents) + 1 ]; 
            this->InitSpecTuple(numFreqPts, i, tuple, dataArray); 
        }
    } else {
        tuple[0] = 0;  
        tuple[1] = 0; 
        for (int i = 0; i < numFreqPts; i++) {
            dataArray->SetTuple( i, tuple );  
        }
    }

    return;
}


/*!
 *  Virtual method for initializing the spectrum array for a given cell. 
 *  Some data sets have time/frequency reversed
 */
void svkGEPFileMapper::InitSpecTuple( int numFreqPts, int freqPt, float* tuple, vtkDataArray* dataArray )
{
    dataArray->SetTuple( freqPt, tuple );  
}


/*!
 *  Modify the data loading behavior.  For single voxel multi-acq data 
 *  this means return the averaged (suppresssed data, if applicable). 
 */
void svkGEPFileMapper::ModifyBehavior( svkImageData* data )
{

    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast( data );  

    int numVoxels[3]; 
    this->GetNumVoxels( numVoxels ); 
    int numTimePts = this->GetNumTimePoints();
    int numUnsuppressed = this->GetNumberUnsuppressedAcquisitions(); 
    int numSuppressed = this->GetNumberSuppressedAcquisitions(); 

    //  Get mapper behavior flag value: 
    map < string, void* >::iterator  it;
    it = this->inputArgs.find( "SetMapperBehavior" );
    MapperBehavior behaviorFlag = svkGEPFileMapper::UNDEFINED;  
    if ( it != this->inputArgs.end() ) {
        behaviorFlag = *( static_cast< svkGEPFileMapper::MapperBehavior* >( this->inputArgs[ it->first ] ) ); 
    }

    //  
    //  If behavior flag isn't set then use default behaviors:
    //
    if ( behaviorFlag == svkGEPFileMapper::UNDEFINED ) {

        //  If single voxel default to loading the averaged 
        //  of the suppressed acquisitions. 
        if ( (numVoxels[0] * numVoxels[1] * numVoxels[2] == 1) 
            && (numTimePts > 1) 
            && (numSuppressed > 1) 
        ) {
            behaviorFlag = svkGEPFileMapper::LOAD_AVG_SUPPRESSED; 
        } else {
            behaviorFlag = svkGEPFileMapper::LOAD_RAW; 
        }
    }

    if ( behaviorFlag == svkGEPFileMapper::LOAD_RAW ) {

        //  no need to modiy anything:
        return; 

    } else if ( behaviorFlag == svkGEPFileMapper::LOAD_RAW_SUPPRESSED ) {

        cout << "LOAD_RAW_SUPPRESSED" << endl;

    } else if ( 
                ( behaviorFlag == svkGEPFileMapper::LOAD_AVG_SUPPRESSED   ) || 
                ( behaviorFlag == svkGEPFileMapper::LOAD_AVG_UNSUPPRESSED ) )
    {

        cout << "LOAD_AVG OF ACQUISITIONS" << endl;

        double cmplxPt[2];
        float cmplxPtAv[2];

        int numFreqPts = this->dcmHeader->GetIntValue( "DataPointColumns" );
        for ( int freq = 0; freq < numFreqPts; freq++ ) { 

            int numCoils = this->GetNumCoils(); 
            for ( int coil = 0; coil < numCoils; coil++ ) { 

                cmplxPtAv[0] = 0; 
                cmplxPtAv[1] = 0; 

                int startAcq; 
                int endAcq; 
                int numAcqsAveraged; 
                if ( behaviorFlag == svkGEPFileMapper::LOAD_AVG_SUPPRESSED   ) {
                    //  start averaging after unsuppressed acquisitions:   
                    startAcq = numUnsuppressed;  
                    endAcq   = numTimePts; 
                    numAcqsAveraged = numSuppressed; 
                } else if ( behaviorFlag == svkGEPFileMapper::LOAD_AVG_UNSUPPRESSED ) {
                    startAcq = 0; 
                    endAcq   = numUnsuppressed; 
                    numAcqsAveraged = numUnsuppressed; 
                } 

                for ( int acq = startAcq; acq < endAcq; acq++ ) { 
                    //  Average the time points
                    vtkFloatArray* spectrum = static_cast<vtkFloatArray*>(
                        mrsData->GetSpectrum( 0, 0, 0, acq, coil)
                    ); 
                    spectrum->GetTuple(freq, cmplxPt);
                    cmplxPtAv[0] += cmplxPt[0]; 
                    cmplxPtAv[1] += cmplxPt[1]; 
                    if ( freq == 0) {
                        cout << "Coil (inputdata)" << coil << " time " << acq << " => " << cmplxPt[0] << " " << cmplxPt[1] << endl;
                    }
    
                }

                cmplxPtAv[0] /= numAcqsAveraged; 
                cmplxPtAv[1] /= numAcqsAveraged; 
            
                //  Output only has numCoil spectra
                vtkFloatArray* spectrumOut = static_cast<vtkFloatArray*>(
                    mrsData->GetSpectrum( 0, 0, 0, 0, coil )
                ); 

                spectrumOut->SetTuple( freq, cmplxPtAv );
                if ( freq == 0) {
                    cout << "AV Coil " << coil << " => " << cmplxPtAv[0] << endl;
                }
            }
        }

        //  delete the unnecessary data arrays  and
        //  reset header to indicate only 1 time point of data in output
        this->RedimensionModifiedSVData( data ); 

        if ( behaviorFlag == svkGEPFileMapper::LOAD_AVG_UNSUPPRESSED ) {
            //  Since it's only unsuppressed data in the file now, set the 
            //  SpectrallySelectedSuppression accordingly:
            this->dcmHeader->SetValue( "SpectrallySelectedSuppression", "NONE" );
        }

    } else if ( behaviorFlag == svkGEPFileMapper::LOAD_EPSI ) {

        cout << "LOAD_EPSI" << endl;
        //  if we are manually reordering EPSI data, then redimension the data 
        //  dimensions if necessary so that they correctly reflect the organization, 
        //  then call svkEPSIReorder with the correct params.
        this->ReorderEPSI( svkMrsImageData::SafeDownCast( data) );

    } else {

        cout << "UNSUPPORTED BEHAVIOR" << endl;

    }

}


/*!
 *  Modify behavior for EPSI data.  
 */
void svkGEPFileMapper::ReorderEPSI( svkMrsImageData* data )
{

    svkDcmHeader* hdr = data->GetDcmHeader();

    int numAcquisitions = this->GetNumEPSIAcquisitions(); 

    //  if more than 1 EPSI acquisition (i.e interleaved or symmetric, the time index should be rewritten as 
    //  EPSIAcq.  

    if ( numAcquisitions > 1 ) {



        svkDcmHeader::DimensionVector dimensionVectorIn = hdr->GetDimensionIndexVector();

        //  Also, currently doesn't support dynamic data, so if more than one time point, exit: 
        int numTimePts = svkDcmHeader::GetDimensionVectorValue(&dimensionVectorIn, svkDcmHeader::TIME_INDEX); 
        if (numTimePts > 1 ) {
            cout << "ERROR:  svkGEPFileMapper does not currently support dynamic EPSI reordering " << endl;
            exit(1);
        }


        //  -----------------------------------------------------------------------------
        //  Allocate new svkMrsImageData object for reordered cell data time->epsi_acq 
        //  dimension.  Changing loop order requires renumbering cell data arrays.
        //  -----------------------------------------------------------------------------
        svkMrsImageData* dataTmp = svkMrsImageData::New();
        dataTmp->DeepCopy( data ); 
        svkImageData::RemoveArrays( dataTmp ); 

        svkDcmHeader::DimensionVector dimensionVectorTmp = dataTmp->GetDcmHeader()->GetDimensionIndexVector(); 
        dataTmp->GetDcmHeader()->AddDimensionIndex( &dimensionVectorTmp, svkDcmHeader::EPSI_ACQ_INDEX, 1); 
        svkDcmHeader::SetDimensionVectorValue( &dimensionVectorTmp, svkDcmHeader::TIME_INDEX, 0);

        //  Allocate arrays for target data set: 
        svkFastCellData* cellDataTmp = static_cast<svkFastCellData*>(dataTmp->GetCellData()); 
        int numCellsTmp = svkDcmHeader::GetNumberOfCells(&dimensionVectorTmp); 

        cellDataTmp->AllocateArrays( numCellsTmp ); 
        vtkDataArray* dataArray = NULL;
        svkDcmHeader::DimensionVector loopIndexTmp = dimensionVectorTmp; 
        for (int arrayNumber = 0; arrayNumber < numCellsTmp; arrayNumber++) {
            dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
            cellDataTmp->FastAddArray( dataArray );
            dataArray->FastDelete();

            //  Set the array name:     
            svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimensionVectorTmp, &loopIndexTmp, arrayNumber);
            dataTmp->SetArrayName( dataArray, &loopIndexTmp );
        }
        cellDataTmp->FinishFastAdd();
        //  -----------------------------------------------------------------------------



        //  Add EPSI_ACQ_INDEX, initially with only one point (default)
        hdr->AddDimensionIndex( &dimensionVectorIn, svkDcmHeader::EPSI_ACQ_INDEX, 0); 

        svkDcmHeader::DimensionVector indexVector = dimensionVectorIn;

        //  GetNumber of cells  and rewrite their meta-data (dimension indices and array names: 
        //  Need to iterate over every cell in order to rewrite the array names.  The number of cells
        //  will not change, but their dimension labels will (time->epsiACQ
        int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVectorIn);
       
        for (int cellID = 0; cellID < numCells; cellID++ ) {
       
            //  Get the dimensionVector index for the non-dynamic image: 
            svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimensionVectorIn, &indexVector, cellID );

            int timePt = svkDcmHeader::GetDimensionVectorValue(&indexVector, svkDcmHeader::TIME_INDEX); 

            //  relabel the 2nd time point as the 2nd EPSI Acq point
            if ( timePt == 1 ) {

                svkDcmHeader::SetDimensionVectorValue( &indexVector, svkDcmHeader::EPSI_ACQ_INDEX, timePt); 
                svkDcmHeader::SetDimensionVectorValue( &indexVector, svkDcmHeader::TIME_INDEX, 0);
            }

            //  Get array from target dynamic image: 
            vtkDataArray* dataArray = data->GetCellData()->GetArray(cellID);
            data->SetArrayName( dataArray, &indexVector );

            int targetCellID = svkDcmHeader::GetCellIDFromDimensionVectorIndex( &dimensionVectorTmp, &indexVector); 
            dataTmp->GetCellData()->GetArray( targetCellID )->DeepCopy(dataArray); 

        } 

        data->DeepCopy( dataTmp ); 

        hdr->SetDimensionIndexSize(svkDcmHeader::TIME_INDEX, 0); 
        hdr->SetDimensionIndexSize(svkDcmHeader::EPSI_ACQ_INDEX, 1); 


        map < string, void* >::iterator  it;

        it = this->inputArgs.find( "epsiType" );
        EPSIType epsiType;
        if ( it != this->inputArgs.end() ) {
            epsiType = *( static_cast< EPSIType* >( this->inputArgs[ it->first ] ) );
        }

        it = this->inputArgs.find( "epsiAxis" );
        svkEPSIReorder::EPSIAxis epsiAxis; 
        if ( it != this->inputArgs.end() ) {
            epsiAxis= *( static_cast< svkEPSIReorder::EPSIAxis* >( this->inputArgs[ it->first ] ) );
        }

        it = this->inputArgs.find( "epsiNumSkip" );
        int epsiNumSkip;
        if ( it != this->inputArgs.end() ) {
            epsiNumSkip = *( static_cast< int* >( this->inputArgs[ it->first ] ) );
        }

        it = this->inputArgs.find( "epsiNumLobes" );
        int epsiNumLobes;
        if ( it != this->inputArgs.end() ) {
            epsiNumLobes = *( static_cast< int* >( this->inputArgs[ it->first ] ) );
        }

        it = this->inputArgs.find( "epsiFirst" );
        int epsiFirst;
        if ( it != this->inputArgs.end() ) {
            epsiFirst = *( static_cast< int* >( this->inputArgs[ it->first ] ) );
        }

        svkMrsImageData* tmpData = svkMrsImageData::New();
        tmpData->ShallowCopy( data );

        svkEPSIReorder* reorder = svkEPSIReorder::New();
        reorder->SetInputData( tmpData ); 
        reorder->SetEPSIType( epsiType );
        reorder->SetEPSIAxis( epsiAxis );
        reorder->SetNumSamplesToSkip( epsiNumSkip);
        reorder->SetNumEPSILobes( epsiNumLobes );
        reorder->SetFirstSample( epsiFirst );
        reorder->Update();

        data->ShallowCopy( reorder->GetOutput() );

        dataTmp->Delete();


    }

}


/*!
 *  Remove extra arrays and redimension the DICOM frames to reflect that the 
 *  single voxel acquisitions have been averaged. 
 */
void svkGEPFileMapper::RedimensionModifiedSVData( svkImageData* data )
{

    int numCoils = data->GetDcmHeader()->GetNumberOfCoils();
    int numTimePts = this->GetNumTimePoints(); 

    int numVoxels[3];
    data->GetNumberOfVoxels(numVoxels);

    int numArraysOriginal = numVoxels[0] * numVoxels[1] * numVoxels[2] * numTimePts * numCoils; 

    numTimePts = 1;  
    int numArraysOut      = numVoxels[0] * numVoxels[1] * numVoxels[2] * numTimePts * numCoils; 

   
    //  get the array from each coil and set it into the 1st numCoil arrays of the data set:    
    for (int coilNum = 0; coilNum < numCoils; coilNum++) {
        svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast( data );
        vtkFloatArray* spectrum = static_cast<vtkFloatArray*>(
            mrsData->GetSpectrum( 0, 0, 0, 0, coilNum )
        );

        vtkDataArray* dataArray = data->GetCellData()->GetArray(coilNum); 
        dataArray->DeepCopy( spectrum ); 
    }

    //  Modified data is set in first N arrays.  Remove all other arrays with higher index: 
    for (int i = numArraysOriginal - 1; i >= numArraysOut; i--) {
        data->GetCellData()->RemoveArray( 
            data->GetCellData()->GetArrayName( i ) 
        );
    }

    for (int coilNum = 0; coilNum < numArraysOut; coilNum++) {
        char arrayName[30];
        sprintf(arrayName, "%d %d %d %d %d", 0, 0, 0, 0, coilNum);
        vtkDataArray* dataArray = data->GetCellData()->GetArray(coilNum); 
        dataArray->SetName(arrayName);
    }

    double origin[3];
    data->GetDcmHeader()->GetOrigin( origin, 0 );

    double voxelSpacing[3];
    data->GetDcmHeader()->GetPixelSpacing( voxelSpacing );

    double dcos[3][3];
    data->GetDcmHeader()->GetDataDcos( dcos );

    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, data->GetDcmHeader()->GetNumberOfSlices()-1);
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::TIME_INDEX, numTimePts-1); 
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::CHANNEL_INDEX, numCoils-1); 
    data->GetDcmHeader()->InitPerFrameFunctionalGroupSequence(
        origin,
        voxelSpacing,
        dcos,
        &dimensionVector
    );

    if ( this->GetDebug() ) {
        data->GetDcmHeader()->PrintDcmHeader( );
    }
}


/*!
 *  For single voxel acquisitions, return the number of 
 *  unsuppressed acquisitions. 
 */
int svkGEPFileMapper::GetNumberUnsuppressedAcquisitions( )
{
    float nex = this->GetHeaderValueAsFloat( "rhi.nex" ); 
    int numUnsuppressed = static_cast< int > (16 / nex ); 
    return numUnsuppressed; 
}


/*!
 *  For single voxel acquisitions, return the number of 
 *  suppressed acquisitions. 
 */
int svkGEPFileMapper::GetNumberSuppressedAcquisitions( )
{
    float nex = this->GetHeaderValueAsFloat( "rhi.nex" ); 
    float user4 = this->GetHeaderValueAsFloat( "rhi.user4" ) ; 
    int numSuppressed = static_cast< int > ( user4 / nex ); 
    return numSuppressed; 
}


/*!
 *  Returns true if the inputArgs map key is set
 */
bool svkGEPFileMapper::isInputArgSet(string argName)
{
    bool isArgDefined;
    
    map < string, void* >::iterator  it;
    it = this->inputArgs.find( argName );
    if ( it != this->inputArgs.end() ) {
        isArgDefined = true;            
    } else {
        isArgDefined = false;            
    }
    return isArgDefined; 
}


/*!
 *  Sets the inputArg value for the specified key as a string.  If the value is not set, then argValue
 *  is not modified. Return value is true if the arg was found, or false if not set. 
 */
bool svkGEPFileMapper::GetInputArgStringValue(string argName, string* argValue)
{
    if ( this->isInputArgSet(argName) ) {

        map < string, void* >::iterator  it;
        it = this->inputArgs.find( argName);
        if ( it != this->inputArgs.end() ) {
            argValue->assign(*( static_cast< string* >( this->inputArgs[ it->first ] ) ));
        }
        return true; 

    } else {

        return false; 

    }
}


/*!
 *  Sets the inputArg value for the specified key as a string.  If the value is not set, then argValue
 *  is not modified. Return value is true if the arg was found, or false if not set. 
 */
bool svkGEPFileMapper::GetInputArgBoolValue(string argName, bool* argValue)
{
    if ( this->isInputArgSet(argName) ) {

        map < string, void* >::iterator  it;
        it = this->inputArgs.find( argName);
        if ( it != this->inputArgs.end() ) {
            *argValue = (*( static_cast< bool* >( this->inputArgs[ it->first ] ) ) );
        }
        return true; 

    } else {

        return false; 

    }

}


/*!
 *
 */
void svkGEPFileMapper::SetProgressText( string progressText )
{
    this->progressText = progressText;
}


/*!
 *
 */
string svkGEPFileMapper::GetProgressText( )
{
    return this->progressText;
}


void svkGEPFileMapper::UpdateProgress(double amount)
{
    this->InvokeEvent(vtkCommand::ProgressEvent,static_cast<void *>(&amount));
}


void svkGEPFileMapper::SetPfileName( string pfileName )
{
    this->pfileName = pfileName; 
    cout << "PFILE NAME: " << this->pfileName << endl;
}
