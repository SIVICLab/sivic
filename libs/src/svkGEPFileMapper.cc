/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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


using namespace svk;


vtkCxxRevisionMacro(svkGEPFileMapper, "$Rev$");
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
    this->SetMapperBehavior( svkGEPFileMapper::UNDEFINED );
}


/*!
 *
 */
svkGEPFileMapper::~svkGEPFileMapper()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if( this->progressCallback != NULL ) {
        this->progressCallback->Delete();
        this->progressCallback = NULL;
    }

}



/*!
 *  Initializes the svkDcmHeader adapter to a specific IOD type      
 *  and initizlizes the svkDcmHeader member of the svkImageData 
 *  object.    
 */
void svkGEPFileMapper::InitializeDcmHeader(map <string, vector< string > >  pfMap, svkDcmHeader* header, float pfileVersion) 
{
    this->pfMap = pfMap; 
    this->dcmHeader = header; 
    this->pfileVersion = pfileVersion; 

    this->InitPatientModule(); 
    this->InitGeneralStudyModule(); 
    this->InitGeneralSeriesModule(); 
    this->InitFrameOfReferenceModule(); 
    this->InitGeneralEquipmentModule(); 
    this->InitEnhancedGeneralEquipmentModule(); 
    this->InitMultiFrameFunctionalGroupsModule();
    this->InitAcquisitionContextModule();
    this->InitMRSpectroscopyModule(); 
    this->InitMRSpectroscopyPulseSequenceModule(); 
    this->InitMRSpectroscopyDataModule(); 
}


/*!
 *  Sets data loading behavior.  
 */
void svkGEPFileMapper::SetMapperBehavior(MapperBehavior behaviorFlag)
{
    this->behaviorFlag = behaviorFlag;
}


/*!
 *  Initializes the VolumeLocalizationSequence in the MRSpectroscopy
 *  DICOM object for PRESS excitation.  
 */
void svkGEPFileMapper::InitVolumeLocalizationSeq()
{

    this->dcmHeader->InsertEmptyElement( "VolumeLocalizationSequence" );

    //  Get Center Location Values
    float selBoxCenter[3]; 
    if ( this->GetHeaderValueAsString( "rhi.psdname" ).compare( "fidcsi" ) == 0 )  {

        selBoxCenter[0] = -1 * this->GetHeaderValueAsFloat( "rhi.ctr_R" );
        selBoxCenter[1] = -1 * this->GetHeaderValueAsFloat( "rhi.ctr_A" );
        selBoxCenter[2] = this->GetHeaderValueAsFloat( "rhi.ctr_S" );

    } else { 

        //  Center position is taken from user variables.  The Z "slice"
        //  position used to be taken from the image header "image.loc",
        //  but with the LX architecture, this held the table position only,
        //  so if Graphic RX was used to introduce an offset, it wouldn't
        //  be successfully extracted.
        selBoxCenter[0] = -1 * this->GetHeaderValueAsFloat( "rhi.user11" ); 
        selBoxCenter[1] = -1 * this->GetHeaderValueAsFloat( "rhi.user12" ); 
        selBoxCenter[2] =  this->GetHeaderValueAsFloat( "rhi.user13" ); 
    }

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
    float selBoxSize[3]; 
    selBoxSize[0] = 0.0;
    double dcos[3][3]; 
    this->GetDcos(dcos); 
    if ( this->pfileVersion > 9  ) {

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


/*!
 *
 */
void svkGEPFileMapper::InitPatientModule() 
{

    this->dcmHeader->SetDcmPatientsName( 
            this->GetHeaderValueAsString( "rhe.patname" )
    );

    this->dcmHeader->SetValue(
        "PatientID", 
        this->GetHeaderValueAsString( "rhe.patid" )
    );

    this->dcmHeader->SetValue(
        "PatientsBirthDate", 
        this->GetHeaderValueAsString( "rhe.dateofbirth" )
    );

    int patsex = this->GetHeaderValueAsInt( "rhe.patsex" ); 
    string gender("O"); 
    if ( patsex == 1 ) {
        gender.assign("M"); 
    } else if ( patsex == 2 ) {
        gender.assign("F"); 
    }

    this->dcmHeader->SetValue(
        "PatientsSex", 
        gender
    );
}


/*!
 *
 */
void svkGEPFileMapper::InitGeneralStudyModule() 
{

    this->dcmHeader->SetValue(
        "StudyInstanceUID", 
        this->GetHeaderValueAsString( "rhe.study_uid" )
    );

    this->dcmHeader->SetValue(
        "StudyDate", 
        this->GetHeaderValueAsString( "rhr.rh_scan_date" )
    );

    this->dcmHeader->SetValue(
        "StudyTime", 
        this->GetHeaderValueAsString( "rhr.rh_scan_time" )
    );

    this->dcmHeader->SetValue(
        "ReferringPhysiciansName", 
        this->GetHeaderValueAsString( "rhe.refphy" )
    );

    this->dcmHeader->SetValue(
        "StudyID", 
        this->GetHeaderValueAsString( "rhe.ex_no" )
    );

    this->dcmHeader->SetValue(
        "AccessionNumber", 
        this->GetHeaderValueAsString( "rhe.reqnum" )
    );
}


/*!
 *
 */
void svkGEPFileMapper::InitGeneralSeriesModule() 
{

    this->dcmHeader->SetValue(
        "SeriesNumber", 
        this->GetHeaderValueAsString( "rhs.se_no" )
    );

    this->dcmHeader->SetValue(
        "SeriesDescription", 
        this->GetHeaderValueAsString( "rhs.se_desc" )
    );

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

    this->dcmHeader->SetValue(
        "PatientPosition", 
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

    this->dcmHeader->SetValue( 
        "ContentDate", 
        this->GetHeaderValueAsString( "rhr.rh_scan_date" ) 
        //this->RemoveSlashesFromDate( this->GetHeaderValueAsString( "rhr.rh_scan_date" )  ) 
    );


    this->dcmHeader->SetValue( 
        "NumberOfFrames", 
        this->GetNumFrames() 
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
    this->InitFrameAnatomyMacro();
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

    double center[3]; 
    //toplc[0] = -1 * this->GetHeaderValueAsFloat( "rhi.tlhc_R" );
    //toplc[1] = -1 * this->GetHeaderValueAsFloat( "rhi.tlhc_A" );
    //toplc[2] = this->GetHeaderValueAsFloat( "rhi.tlhc_S" );

    center[0] = -1 * this->GetHeaderValueAsFloat( "rhi.user11" );
    center[1] = -1 * this->GetHeaderValueAsFloat( "rhi.user12" );
    center[2] = this->GetHeaderValueAsFloat( "rhi.user13" );

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

    this->dcmHeader->InitPerFrameFunctionalGroupSequence(
            toplc, 
            voxelSpacing,
            dcos, 
            numSlices, 
            numTimePts, 
            numCoils
    );

}


/*!
 *  Pixel Spacing:
 */
void svkGEPFileMapper::InitPixelMeasuresMacro()
{

    this->dcmHeader->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "PixelMeasuresSequence"
    );

    //  rhi.scanspacing (space in mm between scans)
    //  Need to get the FOV and the number of phase encodes to get the spatial resolution:
    //  should the methods for obtaining the resolution and spacing be virtual in a subclass?
    
    double voxelSpacing[3]; 
    this->GetVoxelSpacing( voxelSpacing); 

    string pixelSpacing;
    ostringstream oss;
    oss << voxelSpacing[0];
    oss << '\\';
    oss << voxelSpacing[1];
    pixelSpacing = oss.str();

    this->dcmHeader->AddSequenceItemElement(
        "PixelMeasuresSequence",            
        0,                                 
        "PixelSpacing",                   
        pixelSpacing,                    
        "SharedFunctionalGroupsSequence",   
        0                                  
    );

    string sliceThickness;
    oss.clear();
    oss.str( "" );
    oss << voxelSpacing[2];

    this->dcmHeader->AddSequenceItemElement(
        "PixelMeasuresSequence",          
        0,                               
        "SliceThickness",               
        oss.str(),
        "SharedFunctionalGroupsSequence",   
        0                                 
    );
}


/*!
 *  Get the voxel spacing in 3D. Note that the slice spacing may 
 *  include a skip. 
 */
void svkGEPFileMapper::GetVoxelSpacing( double voxelSpacing[3] )
{

    float user19 =  this->GetHeaderValueAsFloat( "rhi.user19" ); 

    if ( user19 > 0  && this->pfileVersion > 9 ) {

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

    if ( this->pfileVersion > 9  ) {

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
    if ( this->pfileVersion > 9  &&  numVoxels[0] != numVoxels[1] ) {

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

    if (this->GetDebug()) {
        cout << "NUM VOXELS: " << numVoxels[0] << " " << numVoxels[1] << " " << numVoxels[2] << endl;
    }
}


/*!
 *  Get the data dcos 
 */
void svkGEPFileMapper::GetDcos( double dcos[3][3] )
{

    dcos[0][0] = -( this->GetHeaderValueAsFloat( "rhi.trhc_R" ) - this->GetHeaderValueAsFloat( "rhi.tlhc_R" ) );
    dcos[0][1] = -( this->GetHeaderValueAsFloat( "rhi.trhc_A" ) - this->GetHeaderValueAsFloat( "rhi.tlhc_A" ) );
    dcos[0][2] = -( this->GetHeaderValueAsFloat( "rhi.trhc_S" ) - this->GetHeaderValueAsFloat( "rhi.tlhc_S" ) );

    float dcosLengthX = sqrt( dcos[0][0] * dcos[0][0]
                            + dcos[0][1] * dcos[0][1]
                            + dcos[0][2] * dcos[0][2] );

    dcos[0][0] /= dcosLengthX;
    dcos[0][1] /= dcosLengthX;
    dcos[0][2] /= dcosLengthX;


    dcos[1][0] = -( this->GetHeaderValueAsFloat( "rhi.brhc_R" ) - this->GetHeaderValueAsFloat( "rhi.trhc_R" ) );
    dcos[1][1] = -( this->GetHeaderValueAsFloat( "rhi.brhc_A" ) - this->GetHeaderValueAsFloat( "rhi.trhc_A" ) );
    dcos[1][2] = -( this->GetHeaderValueAsFloat( "rhi.brhc_S" ) - this->GetHeaderValueAsFloat( "rhi.trhc_S" ) );

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
void svkGEPFileMapper::InitFrameAnatomyMacro()
{
    this->dcmHeader->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "FrameAnatomySequence"
    );

    this->dcmHeader->AddSequenceItemElement(
        "FrameAnatomySequence",       
        0,                             
        "FrameLaterality",              
        string("U"),                     
        "SharedFunctionalGroupsSequence", 
        0                                  
    );

    this->dcmHeader->AddSequenceItemElement(
        "FrameAnatomySequence",      
        0,                          
        "AnatomicRegionSequence"   
    );


    this->dcmHeader->AddSequenceItemElement(
        "AnatomicRegionSequence",       
        0,                             
        "CodeValue",              
        1,
        "FrameAnatomySequence", 
        0                                  
    );

    this->dcmHeader->AddSequenceItemElement(
        "AnatomicRegionSequence",       
        0,                             
        "CodingSchemeDesignator",              
        0,
        "FrameAnatomySequence", 
        0                                  
    );

    this->dcmHeader->AddSequenceItemElement(
        "AnatomicRegionSequence",       
        0,                             
        "CodeMeaning",              
        0,
        "FrameAnatomySequence", 
        0                                  
    );

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
    this->dcmHeader->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRTimingAndRelatedParametersSequence"
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence", 
        0,                                
        "RepetitionTime",                     
        this->GetHeaderValueAsFloat( "rhi.tr" ) / 1000, 
        "SharedFunctionalGroupsSequence",  
        0 
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence", 
        0,                                
        "FlipAngle",                     
        this->GetHeaderValueAsFloat( "rhi.mr_flip" ), 
        "SharedFunctionalGroupsSequence",  
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence", 
        0,                                
        "EchoTrainLength",                     
        this->GetHeaderValueAsInt( "rhi.numecho" ), 
        "SharedFunctionalGroupsSequence",  
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence", 
        0,                                
        "RFEchoTrainLength",                     
        this->GetHeaderValueAsInt( "rhi.numecho" ), 
        "SharedFunctionalGroupsSequence",  
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence", 
        0,                                
        "GradientEchoTrainLength",                     
        0,
        "SharedFunctionalGroupsSequence",  
        0
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

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionTLC",
        "0\\0\\0",
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionPixelSpacing",
        "0\\0", 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionSliceThickness",
        "0", 
        "SharedFunctionalGroupsSequence",
        0
    );


    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionOrientation",
        "0\\0\\0\\0\\0\\0\\0\\0\\0", 
        "SharedFunctionalGroupsSequence",
        0
    );


    // ==================================================
    //  Reordered Params
    // ==================================================
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedPhaseColumns",
        0,  
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedPhaseRows",
        0, 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedOutOfPlanePhaseSteps",
        0, 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedTLC",
        "0\\0\\0", 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedPixelSpacing",
        "0\\0", 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedSliceThickness",
        0, 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedOrientation",
        "0\\0\\0\\0\\0\\0\\0\\0\\0", 
        "SharedFunctionalGroupsSequence",
        0
    );



}


/*!
 *
 */
void svkGEPFileMapper::InitMREchoMacro()
{
    this->dcmHeader->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MREchoSequence"
    );

    this->dcmHeader->AddSequenceItemElement(
        "MREchoSequence",        
        0,                        
        "EffectiveEchoTime",       
        this->GetHeaderValueAsFloat("rhi.te")/1000, 
        "SharedFunctionalGroupsSequence",    
        0
    );
}


/*!
 *  Override in concrete mapper for specific acquisitino
 */
void svkGEPFileMapper::InitMRModifierMacro()
{
    this->dcmHeader->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRModifierSequence"
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRModifierSequence",
        0,                        
        "InversionRecovery",       
        string("NO"),
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRModifierSequence",
        0,                        
        "SpatialPreSaturation",       
        string("SLAB"),
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRModifierSequence",
        0,                        
        "ParallelAcquisition",       
        string("NO"),
        "SharedFunctionalGroupsSequence",    
        0                      
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
    this->dcmHeader->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRTransmitCoilSequence"
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRTransmitCoilSequence",
        0,                        
        "TransmitCoilName",       
        "coil name",
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRTransmitCoilSequence",
        0,                        
        "TransmitCoilManufacturerName",       
        "GE",
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRTransmitCoilSequence",
        0,                        
        "TransmitCoilType",       
        "BODY",
        "SharedFunctionalGroupsSequence",    
        0                      
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

    this->dcmHeader->AddSequenceItemElement(
        "MRAveragesSequence",
        0,                        
        "NumberOfAverages",       
        "1",
        "SharedFunctionalGroupsSequence",    
        0
    );

}


/*!
 *  sat band information  image.user25-48 
 */
void svkGEPFileMapper::InitMRSpatialSaturationMacro()
{

    this->dcmHeader->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRSpatialSaturationSequence"
    );


    //  if this field is 3, then sat bands are defined in user CVs.    
    if ( this->GetHeaderValueAsInt( "rhr.rh_user_usage_tag" ) == 3 ) { 

        //  Sat band info (up to 6) is contained in 4 sequential user CVs:
        int satBandNumber = 0; 
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
                this->InitSatBand(satRAS, translation, satBandNumber);
                satBandNumber++; 
            }

        }
    }
}


/*!  
 *  Add a sat band to the SpatialSaturationSequence: 
 *  RAS:          vector of the normal to the sat band with lenght 
 *                equal to the band thickness in RAS coordinates. 
 *  translation : translation along that vector to sat band location (slab middle)
 */
void svkGEPFileMapper::InitSatBand( float satRAS[3], float translation, int satBandNumber )
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


    this->dcmHeader->AddSequenceItemElement(
        "MRSpatialSaturationSequence",
        satBandNumber,                        
        "SlabThickness",       
        satThickness, 
        "SharedFunctionalGroupsSequence",    
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
        "MRSpatialSaturationSequence",
        satBandNumber,                        
        "SlabOrientation",       
        slabOrientation,
        "SharedFunctionalGroupsSequence",    
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
        "MRSpatialSaturationSequence",
        satBandNumber,                        
        "MidSlabPosition",       
        slabPosition,
        "SharedFunctionalGroupsSequence",    
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

    this->dcmHeader->SetValue(
        "AcquisitionDatetime",
        this->GetHeaderValueAsString( "rhr.rh_scan_date" ) + "000000" 
    );

    this->dcmHeader->SetValue(
        "AcquisitionDuration",
        0
    );

    //  Use the mps freq and field strength to determine gamma which is 
    //  characteristic of the isotop:
    float gamma = ( this->GetHeaderValueAsFloat( "rhr.rh_ps_mps_freq" ) * 1e-7 )  
            / ( this->GetHeaderValueAsFloat( "rhe.magstrength" ) / 10000. );
   
    string nucleus;  
    if ( fabs( gamma - 42.57 ) < 0.1 ) {
        nucleus.assign("1H");
    } else if ( fabs( gamma - 10.7 ) <0.1 ) {
        nucleus.assign("13C");
    }

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
        0 
    );

    this->dcmHeader->SetValue(
        "ChemicalShiftReference", 
        this->GetPPMRef() 
    );

    string localizationType; 

    if ( this->pfileVersion > 9 ) {
        localizationType.assign( "PRESS" ); 
    } else {
        float user0 = this->GetHeaderValueAsFloat( "rhr.rh_user0" );
        if( user0 == 3 || user0 == 1 ) {
            localizationType.assign( "PRESS" ); 
        }
    }


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
float svkGEPFileMapper::GetPPMRef() 
{
    float freqOffset;

    if ( this->pfileVersion > 9 ) {
        freqOffset = 0.0; 
    } else {
        freqOffset = this->GetHeaderValueAsFloat( "rhr.rh_user13" ); 
    }

    float transmitFreq = this->GetHeaderValueAsFloat( "rhr.rh_ps_mps_freq" ) * 1e-7; 

    float ppmRef = svkSpecUtils::GetPPMRef(transmitFreq, freqOffset);

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
        "WATER" 
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

    this->dcmHeader->SetValue( 
        "NumberOfKSpaceTrajectories", 
        1 
    );

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
    if ( numVoxels[0] * numVoxels[1] * numVoxels[2] == 1 ) {
        spatialDomain = "SPACE"; 
    } 

    this->dcmHeader->SetValue( "SVK_ColumnsDomain", spatialDomain );
    this->dcmHeader->SetValue( "SVK_RowsDomain", spatialDomain);
    this->dcmHeader->SetValue( "SVK_SliceDomain", spatialDomain );

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

    int passSize = this->GetHeaderValueAsInt( "rhr.rh_raw_pass_size" ); 
    int numCoils = this->GetNumCoils(); 
    int numVoxels = this->GetNumVoxelsInVol(); 
    int dataWordSize = this->GetHeaderValueAsInt( "rhr.rh_point_size" ); 
    int numFreqPoints = this->GetHeaderValueAsInt( "rhr.rh_frame_size" ); 

    int numTimePoints = static_cast<int>(
        static_cast<float>( passSize ) / static_cast<float>( numCoils * 2 * dataWordSize * numFreqPoints )  - 1 
    ) / this->GetNumKSpacePoints(); 

    if ( this->GetDebug() ) {
        cout << "NUM TIME POINTS: " <<  numTimePoints << endl;
    }

    return numTimePoints; 
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
 *  sampling strategy was employed. 
 */
int svkGEPFileMapper::GetNumKSpacePoints()
{
    int numKSpacePts;

    //  Image user21 indicates that an elliptical k-space sampling radius is 
    //  was used.  If true, then number of k-space points differs from numVoxels
    //  in rectaliniear grid. 
    if (this->GetHeaderValueAsInt( "rhi.user21" ) != 0 ) { 

        float ellipticalRad =  this->GetHeaderValueAsFloat( "rhi.user22" ); 
        numKSpacePts = 0; 

        int numVoxels[3]; 
        this->GetNumVoxels( numVoxels ); 

        for( int z = 1; z <= numVoxels[2]; z++ ) {

            float zCorner = 2.0 * z - numVoxels[2] - 1.0;
            if (zCorner > 0) {
                zCorner += 1.0;
            } else {
                zCorner -= 1.0;
            }

            for( int y = 1; y <= numVoxels[1]; y++ ) {

                float yCorner = 2.0 * y - numVoxels[1] - 1.0;
                if (yCorner > 0) {
                    yCorner += 1.0;
                } else {
                    yCorner -= 1.0;
                }

                for( int x = 1; x <= numVoxels[0]; x++ ) {

                    float xCorner = 2.0 * x - numVoxels[0] - 1.0;
                    if (xCorner > 0) {
                        xCorner += 1.0;
                    } else {
                        xCorner -= 1.0;
                    }

                    float test = 
                        ( ( xCorner * xCorner ) / (float)( numVoxels[0] * numVoxels[0] ) )
                      + ( ( yCorner * yCorner ) / (float)( numVoxels[1] * numVoxels[1] ) ) 
                      + ( ( zCorner * zCorner ) / (float)( numVoxels[2] * numVoxels[2] ) );

                    // is it inside ellipse?
                    if ( test <= ellipticalRad ) {
                        numKSpacePts++;
                    }
                }     // for xstep
            }     // for ystep
        }     // for zstep

    } else {
        numKSpacePts = GetNumVoxelsInVol(); 
    }

    if ( this->GetDebug() ) {
        cout << "NUM KSPACE PTS: " << numKSpacePts << endl; 
    }

    return numKSpacePts; 
}


/*!
 *
 */
void svkGEPFileMapper::ReadData(string pFileName, svkImageData* data)
{

    ifstream* pFile = new ifstream();
    pFile->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

    pFile->open( pFileName.c_str(), ios::binary);

    int numComponents =  2; 
    int numCoils = this->GetNumCoils(); 
    int numTimePts = this->GetNumTimePoints(); 
    int numSpecPts = this->GetHeaderValueAsInt( "rhr.rh_frame_size" ); 
    int dataWordSize = this->GetHeaderValueAsInt( "rhr.rh_point_size" ); 
        
    int numBytesInVol = this->GetNumVoxelsInVol() * numSpecPts * numComponents * dataWordSize; 
    int numBytesInPFile = numBytesInVol * numTimePts * numCoils; 
    int numDummyWords = numCoils * numSpecPts * numComponents; 
    this->specData = new int[ numBytesInPFile / dataWordSize  + numDummyWords];  

    pFile->seekg(0, ios::beg);
    int readOffset = this->GetHeaderValueAsInt( "rhr.rdb_hdr_off_data" );
    pFile->seekg(readOffset, ios::beg);
    pFile->read( (char*)(this->specData), numBytesInPFile);

#if !defined (linux) && !defined(Darwin)
    svkByteSwap::SwapBufferEndianness( specData, numBytesInPFile/dataWordSize );
#endif

    int numVoxels[3]; 
    this->GetNumVoxels( numVoxels ); 

    int cols = numVoxels[0];
    int rows = numVoxels[1];
    int slices = numVoxels[2];
    int coilOffset = cols * rows * slices * numTimePts;     //number of spectra per coil
    int timePtOffset = cols * rows * slices;


    //  Preallocate data arrays. The API only permits dynamic assignmet at end of CellData, so for
    //  swapped cases where we need to insert data out of order they need to be preallocated.
    for (int arrayNumber = 0; arrayNumber < cols * rows * slices * numTimePts * numCoils; arrayNumber++) { 
        vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
        data->GetCellData()->AddArray(dataArray); 
        dataArray->Delete(); 
    }
    cout << *(data->GetCellData()) << endl;

    int offset = 0; 
    //  Blank scan, one spectrum per channel.
    int dummyOffset = 0; 
    int numPtsPerSpectrum = numSpecPts * numComponents;
    dummyOffset = numPtsPerSpectrum; 

    //  If Chop On, then reinitialize chopVal:
    if ( IsChopOn() ) {
        this->chopVal = -1; 
    }
    int denominator = numVoxels[2] * numVoxels[1]  * numVoxels[0] + numVoxels[1]*numVoxels[0] + numVoxels[0];
    double progress = 0;

    int x; 
    int y; 
    int z; 
    int index; 

    for (int coilNum = 0; coilNum < numCoils; coilNum++) {

        offset += dummyOffset;  

        for (int timePt = 0; timePt < numTimePts; timePt++) {

            ostringstream progressStream;
            progressStream <<"Reading Time Point " << timePt+1 << "/"
                           << numTimePts << " and Channel: " << coilNum+1 << "/" << numCoils;
            this->SetProgressText( progressStream.str().c_str() );

            for (int index2 = 0; index2 < numVoxels[2] ; index2++) {
                for (int index1 = 0; index1 < numVoxels[1]; index1++) {
                    for (int index0 = 0; index0 < numVoxels[0]; index0++) {
                        
                        this->GetXYZIndices(index0, index1, index2, &x, &y, &z);  

                        //  Linear index of current cell in target svkImageData 
                        int index =  x 
                                   + ( numVoxels[0] ) * y  
                                   + ( numVoxels[0] * numVoxels[1] ) * z  
                                   + ( numVoxels[0] * numVoxels[1] * numVoxels[2] ) * timePt  
                                   + ( numVoxels[0] * numVoxels[1] * numVoxels[2] * numTimePts ) * coilNum; 

                        SetCellSpectrum(data, offset, index, x, y, z, timePt, coilNum);
                        offset += numPtsPerSpectrum; 

                    }

                    progress = (((z) * (numVoxels[0]) * (numVoxels[1]) ) + ( (y) * (numVoxels[0]) ))
                                       /((double)denominator);
                    this->UpdateProgress( progress );

                }
            }
        }
    }

    pFile->close(); 
    delete pFile;
    delete [] specData; 

    this->ModifyBehavior( data );

}


/*! 
 *  If swapping is turned on, the data will need to get mapped correctly 
 *  from the input data buffer read from disk (specData) to the correct 
 *  svkImageData arrays. If swap is true, then the data indices are swapped 
 *  and ky is flipped. 
 */
void svkGEPFileMapper::GetXYZIndices(int index0, int index1, int index2, int* x, int* y, int* z)
{
    if ( this->IsSwapOn() ) {

        int numVoxels[3]; 
        this->GetNumVoxels( numVoxels ); 

        *x = index1; 
        *y = numVoxels[1] - index0 - 1; 
        *z = index2; 

    } else {

        *x = index0; 
        *y = index1; 
        *z = index2; 

    }

    return; 
}


/*!
 *  
 */
void svkGEPFileMapper::SetCellSpectrum(vtkImageData* data, int offset, int index, int x, int y, int z, int timePoint, int coilNum)
{

    //  Set XY points to plot 
    //vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);

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
    if ( IsChopOn() ) {
       chopVal *= -1;  
    }

    float tuple[2];
    for (int i = 0; i < numFreqPts; i++) {

        tuple[0] = chopVal * specData[ offset + (i * numComponents) ]; 
        tuple[1] = chopVal * specData[ offset + (i * numComponents) + 1 ]; 
        dataArray->SetTuple( i, tuple);  

    }

    return;
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
    int numUnsuppressed;
    int numSuppressed;

    //  
    //  If behavior flag isn't set then use default behaviors:
    //
    if ( this->behaviorFlag == svkGEPFileMapper::UNDEFINED ) {

        numUnsuppressed = this->GetNumberUnsuppressedAcquisitions(); 
        numSuppressed = this->GetNumberSuppressedAcquisitions(); 

        if ( (numVoxels[0] * numVoxels[1] * numVoxels[2] == 1) 
            && (numTimePts > 1) 
            && (numSuppressed > 1) 
        ) {
            this->behaviorFlag = svkGEPFileMapper::LOAD_AVG_SUPPRESSED; 
        } else {
            this->behaviorFlag = svkGEPFileMapper::LOAD_RAW; 
        }
    }

    if ( this->behaviorFlag == svkGEPFileMapper::LOAD_RAW ) {

        //  no need to modiy anything:
        return; 

    } else if ( this->behaviorFlag == svkGEPFileMapper::LOAD_AVG_SUPPRESSED ) {

        cout << "LOAD_AVG_SUPPRESSED" << endl;

        float cmplxPt[2];
        float cmplxPtAv[2];

        int numFreqPts = this->dcmHeader->GetIntValue( "DataPointColumns" );
        for ( int freq = 0; freq < numFreqPts; freq++ ) { 

            int numCoils = this->GetNumCoils(); 
            for ( int coil = 0; coil < numCoils; coil++ ) { 

                cmplxPtAv[0] = 0; 
                cmplxPtAv[1] = 0; 

                //  start averaging after unsuppressed acquisitions:   
                for ( int acq = numUnsuppressed; acq < numTimePts; acq++ ) { 

                    //  Average the suppressed time points
                    vtkFloatArray* spectrum = static_cast<vtkFloatArray*>(
                        mrsData->GetSpectrum( 0, 0, 0, acq, coil)
                    ); 
                    spectrum->GetTupleValue(freq, cmplxPt);
                    cmplxPtAv[0] += cmplxPt[0]; 
                    cmplxPtAv[1] += cmplxPt[1]; 
    
                }

                cmplxPtAv[0] /= numSuppressed; 
                cmplxPtAv[1] /= numSuppressed; 
            
                //  Output only has numCoil spectra
                vtkFloatArray* spectrumOut = static_cast<vtkFloatArray*>(
                    mrsData->GetSpectrum( coil )
                ); 

                spectrumOut->SetTuple( freq, cmplxPtAv );
            }
        }

        //  delete the unnecessary data arrays  and
        //  reset header to indicate only 1 time point of data in output
        this->RedimensionModifiedSVData( data ); 
    }
                    
}


/*!
 *  Remove extra arrays and redimension the DICOM frames. 
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

    data->GetDcmHeader()->InitPerFrameFunctionalGroupSequence(
        origin,
        voxelSpacing,
        dcos,
        data->GetDcmHeader()->GetNumberOfSlices(),
        numTimePts,
        numCoils 
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
