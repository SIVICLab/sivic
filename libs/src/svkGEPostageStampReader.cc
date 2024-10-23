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

#include <svkGEPostageStampReader.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkStringArray.h>

#include <svkMrsImageData.h>
#include <svkIOD.h>
#include <svkMRSIOD.h>
#include <svkUtils.h>
#include <svkTypeUtils.h>


using namespace svk;


//vtkCxxRevisionMacro(svkGEPostageStampReader, "$Rev$");
vtkStandardNewMacro(svkGEPostageStampReader);


/*!
 *
 */
svkGEPostageStampReader::svkGEPostageStampReader()
{
#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkGEPostageStampReader");
#endif
    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");
}


/*!
 *
 */
svkGEPostageStampReader::~svkGEPostageStampReader()
{
    vtkDebugMacro(<<this->GetClassName() << "::~" << this->GetClassName() << "()");
}


/*!
 *  Mandatory, must be overridden to get the factory to check for proper
 *  type of vtkImageReader2 to return. 
 */
int svkGEPostageStampReader::CanReadFile(const char* fname)
{

    std::string fileToCheck(fname);
    bool isGEPostage = false; 

    if ( svkDcmHeader::IsFileDICOM( fname ) ) {
 
        svkImageData* tmp = svkMrsImageData::New(); 
        tmp->GetDcmHeader()->ReadDcmFile( fname ); 
        std::string SOPClassUID = tmp->GetDcmHeader()->GetStringValue( "SOPClassUID" ) ; 

        // Check for MR Image Storage
        if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.4" ) {

            //  Check for proprietary use of DICOM MR ImageStorage:
            if ( this->ContainsProprietaryContent( tmp ) == svkDcmVolumeReader::GE_POSTAGE_STAMP_SOP ) {
                this->SetFileName(fname);
                isGEPostage = true;
            }

        }

        tmp->Delete(); 
    } 

    if ( isGEPostage ) { 
            cout << this->GetClassName() << "::CanReadFile(): It's a GE Postage Stamp File: " <<  fileToCheck << endl;
            this->SetFileName(fname);
            return 1;

    } else {

        vtkDebugMacro(<<this->GetClassName() << "::CanReadFile() It's Not a GE Postage Stamp file " << fileToCheck );

    }

    return 0;

}


/*
 *  returns false, meaning it's ok to have a multi-volumetric data set
 *  since real and imaginary components are stored in separate PS images 
 *  with the same imagePositionPatient values. 
 */
bool svkGEPostageStampReader::CheckForMultiVolume() {
    return false;
}


/*!
 *
 */
void svkGEPostageStampReader::InitDcmHeader()
{

    this->InitFileNames();

    // Read the first file and load the header as the starting point
    this->GetOutput()->GetDcmHeader()->ReadDcmFile( this->GetFileNames()->GetValue(0) );

    this->imageCols = this->GetOutput()->GetDcmHeader()->GetIntValue( "Columns" );
    this->imageRows = this->GetOutput()->GetDcmHeader()->GetIntValue( "Rows" );

    //  Now override elements with Multi-Frame sequences and default details:
    svkIOD* iod = svkMRSIOD::New();
    iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    iod->SetReplaceOldElements(false);
    iod->InitDcmHeader();
    iod->Delete();

    std::string studyInstanceUID( this->GetOutput()->GetDcmHeader()->GetStringValue("StudyInstanceUID"));
    this->GetOutput()->GetDcmHeader()->SetValue( "StudyInstanceUID", studyInstanceUID.c_str() );

    //  Now move info from original MRImageStorage header elements to flesh out enhanced
    //  SOP class elements (often this is just a matter of copying elements from the top
    //  level to a sequence item.
    this->InitMultiFrameFunctionalGroupsModule();
    this->InitMRSpectroscopyModule();
    this->InitMRSpectroscopyDataModule();


    if (this->GetDebug()) {
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }

}


/*!
 *  Initializes any private DICOM attributes that are needed internally
 */
void svkGEPostageStampReader::InitPrivateHeader()
{
}



/*!
 *
 */
void svkGEPostageStampReader::InitMultiFrameFunctionalGroupsModule()
{

    //  num frames is 1/2 num files for postage stamp data, since real and
    //  imaginary components of each slice are in separate files. 
    this->numFrames =  this->GetFileNames()->GetNumberOfValues() / 2;
    this->GetOutput()->GetDcmHeader()->SetValue(
        "NumberOfFrames",
        this->numFrames
    );

    this->InitSharedFunctionalGroupMacros();
    this->InitPerFrameFunctionalGroupMacros();

}


/*!
 *
 */
void svkGEPostageStampReader::InitPerFrameFunctionalGroupMacros()
{

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    
    double pixelSpacing[3];
    hdr->GetPixelSpacing(pixelSpacing);

    pixelSpacing[2] = hdr->GetFloatValue( "SliceThickness" ); 
    if ( this->numFrames > 2 ) {

        double origin0[3];
        hdr->GetOrigin(origin0, 0); 

        svkImageData* tmpImage = svkMriImageData::New();
        tmpImage->GetDcmHeader()->ReadDcmFile( this->GetFileNames()->GetValue( 2 ) );

        double origin1[3];
        tmpImage->GetDcmHeader()->GetOrigin(origin1, 0);
        tmpImage->Delete();

        double sliceSpacing = 0;
        for (int i = 0; i < 3; i++ ) {
            sliceSpacing += pow(origin1[i] - origin0[i], 2);
        }
        sliceSpacing = pow(sliceSpacing, .5);

        pixelSpacing[2] = sliceSpacing;
    }

    hdr->SetSliceOrder( svkDcmHeader::INCREMENT_ALONG_NEG_NORMAL );

    double dcos[3][3];
    hdr->GetDataDcos(dcos);


    //  Get toplc from image position patient
    //  frame locations:         
    double toplc[3];         
    for (int i = 0; i < 3; i++) {        
        toplc[i] = svkTypeUtils::StringToDouble( hdr->GetStringValue( "ImagePositionPatient", i ) );         
    } 

    svkDcmHeader::DimensionVector dimensionVector = hdr->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, this->numFrames-1);

    hdr->InitPerFrameFunctionalGroupSequence(
        toplc, pixelSpacing, dcos, &dimensionVector
    );

}




/*!
 *
 */
void svkGEPostageStampReader::InitSharedFunctionalGroupMacros()
{

    this->InitPixelMeasuresMacro();
    //this->InitPlaneOrientationMacro();
    this->InitMRTimingAndRelatedParametersMacro();
    this->InitMRSpectroscopyFOVGeometryMacro();
    this->InitMREchoMacro();
    //this->InitMRModifierMacro();
    this->InitMRReceiveCoilMacro();
    //this->InitMRTransmitCoilMacro();
    this->InitMRAveragesMacro();
    this->InitMRSpatialSaturationMacro();
    //this->InitMRSpatialVelocityEncodingMacro();
}


/*!
 *  Pixel Spacing:
 */
void svkGEPostageStampReader::InitPixelMeasuresMacro()
{

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();

    float diameter = hdr->GetFloatValue( "ReconstructionDiameter" );
    int numCols; 
    int numRows; 
    this->GetColsAndRows(&numCols, &numRows); 

    ostringstream ossCol;
    ossCol << diameter/numCols;
    std::string pixelSizeString(ossCol.str());

    pixelSizeString.append("\\"); 

    ostringstream ossRow;
    ossRow << diameter/numRows;
    pixelSizeString.append(ossRow.str());

    hdr->InitPixelMeasuresMacro(
        pixelSizeString, 
        hdr->GetStringValue( "SliceThickness" )
    );
}



/*!
 *
 */
void svkGEPostageStampReader::InitMRTimingAndRelatedParametersMacro()
{
    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    float repTime = hdr->GetFloatValue("RepetitionTime"); 
    hdr->ClearElement("RepetitionTime");
    float flipAngle = hdr->GetFloatValue("FlipAngle"); 
    hdr->ClearElement("FlipAngle");
    hdr->InitMRTimingAndRelatedParametersMacro(
        repTime, 
        flipAngle
    );
}


/*!
 *
 */
void svkGEPostageStampReader::InitMREchoMacro()
{

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    float echoTime = hdr->GetFloatValue("EchoTime"); 
    hdr->ClearElement("EchoTime");
    hdr->InitMREchoMacro( echoTime ); 
}


/*!
 *
 */
void svkGEPostageStampReader::InitMRReceiveCoilMacro()
{

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();

    hdr->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRReceiveCoilSequence"
    );

    string rcvCoilName = hdr->GetStringValue("ReceiveCoilName"); 
    hdr->ClearElement("ReceiveCoilName");
    hdr->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "ReceiveCoilName",
        rcvCoilName, 
        "SharedFunctionalGroupsSequence",
        0
    );

    hdr->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "ReceiveCoilManufacturerName",
        std::string("GE"),
        "SharedFunctionalGroupsSequence",
        0
    );

    std::string coilType("VOLUME");
    if ( rcvCoilName.compare("8HRBRAIN") == 0 ) {
        coilType.assign("MULTICOIL");
    }

    hdr->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "ReceiveCoilType",
        coilType,
        "SharedFunctionalGroupsSequence",
        0
    );

    hdr->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "QuadratureReceiveCoil",
        std::string("YES"),
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *
 */
void svkGEPostageStampReader::InitMRSpectroscopyModule()
{


    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();


    /*  =======================================
     *  MR Image and Spectroscopy Instance Macro
     *  ======================================= */
    string studyDate = hdr->GetStringValue("StudyDate");
    string acqTime = hdr->GetStringValue("AcquisitionTime");
    hdr->SetValue(
        "AcquisitionDateTime",
        studyDate + acqTime
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "AcquisitionDuration",
        0
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ResonantNucleus",
        "1H" 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "KSpaceFiltering",
        "NONE"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ApplicableSafetyStandardAgency",
        "Research"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "MagneticFieldStrength",
        hdr->GetStringValue("MagneticFieldStrength")
    );
    /*  =======================================
     *  END: MR Image and Spectroscopy Instance Macro
     *  ======================================= */

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ImageType",
        std::string("ORIGINAL\\PRIMARY\\SPECTROSCOPY\\NONE")
    );


    /*  =======================================
     *  Spectroscopy Description Macro
     *  ======================================= */
    this->GetOutput()->GetDcmHeader()->SetValue(
        "VolumetricProperties",
        std::string("VOLUME")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "VolumeBasedCalculationTechnique",
        std::string("NONE")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ComplexImageComponent",
        std::string("COMPLEX")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "AcquisitionContrast",
        "UNKNOWN"
    );
    /*  =======================================
     *  END: Spectroscopy Description Macro
     *  ======================================= */


    float transmitterFreq = 63;
    if ( hdr->GetFloatValue("MagneticFieldStrength") == 1.5 ) { 
        transmitterFreq = 63;
    }
    this->GetOutput()->GetDcmHeader()->SetValue(
        "TransmitterFrequency",
        transmitterFreq
    );

    //  original pts/hz:
    //  (0019,10a7) DS [2000.000000]      
    //  (0019,10a8) DS [1024.000000],
    //  we subsample 256 frequency pts, but these fields
    //  aren't present in all  PS data sets, so just hardcode for now
    this->GetOutput()->GetDcmHeader()->SetValue(
        "SpectralWidth",
        "250"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_FrequencyOffset",
        "0"
    );  

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ChemicalShiftReference",
        "2.3"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "VolumeLocalizationTechnique", 
        "PRESS"
    );  
    
    this->InitVolumeLocalizationSeq();

    this->GetOutput()->GetDcmHeader()->SetValue(
        "Decoupling", 
        "NO" 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "TimeDomainFiltering", 
        "NONE" 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "NumberOfZeroFills", 
        0 
    );
    
    this->GetOutput()->GetDcmHeader()->SetValue(
        "BaselineCorrection", 
        std::string("NONE")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "FrequencyCorrection",
        "NO"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "FirstOrderPhaseCorrection",
        std::string("NO")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "WaterReferencedPhaseCorrection",
        std::string("NO")
    );
}


/*!
 *  Initializes the VolumeLocalizationSequence in the MRSpectroscopy
 *  DICOM object for PRESS excitation.
 */
void svkGEPostageStampReader::InitVolumeLocalizationSeq()
{
    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();

    //  =========================
    //  Get Orientation Values
    //  =========================
    double dcosD[3][3];
    hdr->GetDataDcos(dcosD);
    float dcos[3][3];
    for (int i=0; i<3; i++) {
        for (int j=0; j<3; j++) {
            dcos[i][j] = static_cast<float>(dcosD[i][j]); 
        }
    }


    //  =========================
    //  Get Thickness Values:
    //  (0019,10af) DS [69.170731]                              
    //  (0019,10b0) DS [108.975609]                            
    //  (0019,10b1) DS [13.400000]    
    //  =========================
    float selBoxSize[3];

    selBoxSize[0] = 0.0;
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
   
    //  If these fields do not exist, then don't initialize the volume localization 
    try {


        selBoxSize[ lIndex ] = hdr->GetFloatValue( "GE_PS_SEL_BOX_SIZE_1" ); 
        selBoxSize[ pIndex ] = hdr->GetFloatValue( "GE_PS_SEL_BOX_SIZE_2" );  
        selBoxSize[ sIndex ] = hdr->GetFloatValue( "GE_PS_SEL_BOX_SIZE_3" ); 



        //  =========================
        //  Get Center Location Values
        //  This may be GE_PS_CENER_R,A,S
        //  =========================
        float selBoxCenter[3];
        selBoxCenter[0] = -1 * hdr->GetFloatValue( "GE_PS_SEL_CENTER_R" ); 
        selBoxCenter[1] = -1 * hdr->GetFloatValue( "GE_PS_SEL_CENTER_A" ); 
        selBoxCenter[2] = hdr->GetFloatValue( "GE_PS_SEL_CENTER_S" ); 
    
    
        this->GetOutput()->GetDcmHeader()->InitVolumeLocalizationSeq(
            selBoxSize,
            selBoxCenter,
            dcos
        );

    } catch (const exception& e) {
        cerr << "Warning, GEPostage Stamp fields describing volume localization are not readable: " << e.what() << endl;
    }

}


/*!
 *  sat band information  image.user25-48
 */
void svkGEPostageStampReader::InitMRSpatialSaturationMacro()
{

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();

    hdr->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRSpatialSaturationSequence"
    );


    //  if this field is 3, then sat bands are defined in user CVs.
    //if ( this->GetHeaderValueAsInt( "rhr.rh_user_usage_tag" ) == 3 ) {

        //  Sat band info (up to 6) is contained in 4 sequential user CVs:
        int satBandNumber = 0;
        for ( int i = 25; i < 49; i += 4 ) {

            int index = i - 25; 

            float satRAS[3];
            satRAS[0] = svkTypeUtils::StringToFloat( hdr->GetStringValue( "GE_PS_SAT_BANDS", index ) );
            satRAS[1] = svkTypeUtils::StringToFloat( hdr->GetStringValue( "GE_PS_SAT_BANDS", index + 1 ) );
            satRAS[2] = svkTypeUtils::StringToFloat( hdr->GetStringValue( "GE_PS_SAT_BANDS", index + 2 ) );
            float translation = svkTypeUtils::StringToFloat( hdr->GetStringValue( "GE_PS_SAT_BANDS", index + 3 ) );
        
            if ( satRAS[0] != 0 || satRAS[1] != 0 || satRAS[2] != 0 || translation != 0) {
                this->InitSatBand(satRAS, translation, satBandNumber);
                satBandNumber++;
            }

        }
    //}
}


/*!
 *  Add a sat band to the SpatialSaturationSequence:
 *  RAS:          vector of the normal to the sat band with lenght
 *                equal to the band thickness in RAS coordinates.
 *  translation : translation along that vector to sat band location (slab middle)
 */
void svkGEPostageStampReader::InitSatBand( float satRAS[3], float translation, int satBandNumber )
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


    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();

    hdr->AddSequenceItemElement(
        "MRSpatialSaturationSequence",
        satBandNumber,
        "SlabThickness",
        satThickness,
        "SharedFunctionalGroupsSequence",
        0
    );

    std::string slabOrientation;
    for (int j = 0; j < 3; j++) {
        ostringstream ossOrientation;
        ossOrientation << orientation[j];
        slabOrientation += ossOrientation.str();
        if (j < 2) {
            slabOrientation += '\\';
        }
    }

    hdr->AddSequenceItemElement(
        "MRSpatialSaturationSequence",
        satBandNumber,
        "SlabOrientation",
        slabOrientation,
        "SharedFunctionalGroupsSequence",
        0
    );

    std::string slabPosition;
    for (int j = 0; j < 3; j++) {
        ostringstream ossPosition;
        ossPosition << position[j];
        slabPosition += ossPosition.str();
        if (j < 2) {
            slabPosition += '\\';
        }
    }
    hdr->AddSequenceItemElement(
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
void svkGEPostageStampReader::InitMRAveragesMacro()
{
    int numAverages = 1;
    this->GetOutput()->GetDcmHeader()->InitMRAveragesMacro(numAverages);
}


/*!
 *
 */
void svkGEPostageStampReader::InitMRSpectroscopyFOVGeometryMacro()
{

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();

    hdr->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRSpectroscopyFOVGeometrySequence"
    );

    hdr->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionDataColumns",
        256, 
        "SharedFunctionalGroupsSequence",
        0
    );


    int numCols = 0;
    int numRows = 0;
    this->GetColsAndRows( &numCols, &numRows );

    hdr->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionPhaseColumns",
        numCols, 
        "SharedFunctionalGroupsSequence",
        0
    );

    hdr->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionPhaseRows",
        numRows, 
        "SharedFunctionalGroupsSequence",
        0
    );

    hdr->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionOutOfPlanePhaseSteps",
        hdr->GetIntValue( "NumberOfFrames" ),
        "SharedFunctionalGroupsSequence",
        0
    );
/*
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionTLC",
        ddfMap["acqToplcLPS0"] + '\\' + ddfMap["acqToplcLPS1"] + '\\' + ddfMap["acqToplcLPS2"],
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionPixelSpacing",
        ddfMap["acqSpacing0"] + '\\' + ddfMap["acqSpacing1"],
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionSliceThickness",
        ddfMap["acqSpacing2"],
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionOrientation",
        ddfMap["acqDcos00"] + '\\' + ddfMap["acqDcos01"] + '\\' + ddfMap["acqDcos02"] + '\\' +
        ddfMap["acqDcos10"] + '\\' + ddfMap["acqDcos11"] + '\\' + ddfMap["acqDcos12"] + '\\' +
        ddfMap["acqDcos20"] + '\\' + ddfMap["acqDcos21"] + '\\' + ddfMap["acqDcos22"],
        "SharedFunctionalGroupsSequence",    
        0
    );
    

    // ==================================================
    //  Reordered Params
    // ==================================================
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                               
        "SVK_SpectroscopyAcqReorderedPhaseColumns",
        this->GetHeaderValueAsInt( ddfMap, "reorderedNumberOfPoints0" ),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                               
        "SVK_SpectroscopyAcqReorderedPhaseRows",
        this->GetHeaderValueAsInt( ddfMap, "reorderedNumberOfPoints1" ),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                               
        "SVK_SpectroscopyAcqReorderedOutOfPlanePhaseSteps",
        this->GetHeaderValueAsInt( ddfMap, "reorderedNumberOfPoints2"),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,                               
        "SVK_SpectroscopyAcqReorderedTLC",
        ddfMap["reorderedToplcLPS0"] + '\\' + ddfMap["reorderedToplcLPS1"] + '\\' + ddfMap["reorderedToplcLPS2"],
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedPixelSpacing",
        ddfMap["reorderedSpacing0"] + '\\' + ddfMap["reorderedSpacing1"],
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedSliceThickness",
        ddfMap["reorderedSpacing2"],
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedOrientation",
        ddfMap["reorderedDcos00"] + '\\' + ddfMap["reorderedDcos01"] + '\\' + ddfMap["reorderedDcos02"] + '\\' +
        ddfMap["reorderedDcos10"] + '\\' + ddfMap["reorderedDcos11"] + '\\' + ddfMap["reorderedDcos12"] + '\\' +
        ddfMap["reorderedDcos20"] + '\\' + ddfMap["reorderedDcos21"] + '\\' + ddfMap["reorderedDcos22"],
        "SharedFunctionalGroupsSequence",
        0
    );
*/
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "PercentSampling",
        1,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "PercentPhaseFieldOfView",
        1,
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *
 */
void svkGEPostageStampReader::GetColsAndRows(int* numCols, int* numRows)
{

    *numCols = 0; 
    *numRows = 0; 
    for (int i = 0; i < 4; i++ ) {    
        int val = svkTypeUtils::StringToInt( this->GetOutput()->GetDcmHeader()->GetStringValue( "AcquisitionMatrix", i ) );
        if ( val != 0 ) {
            if ( *numCols == 0 ) {  
                *numCols = val; 
            } else {
                *numRows = val; 
            }
        }
    }
}


/*!
 *
 */
void svkGEPostageStampReader::InitMRSpectroscopyDataModule()
{

    int numCols = 0; 
    int numRows = 0; 
    this->GetColsAndRows( &numCols, &numRows ); 

    this->GetOutput()->GetDcmHeader()->SetValue(
        "Rows", 
        numRows 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "Columns",
        numCols 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "DataPointRows",
        1
    );

    //  Postage stamp appears to be 256 frequency points:
    //  16x16 grid of single frequency images
    this->GetOutput()->GetDcmHeader()->SetValue(
        "DataPointColumns",
        256
    );

    int numComponents =  2; 
    std::string representation;
    if (numComponents == 1) {
        representation = "REAL";
    } else if (numComponents == 2) {
        representation = "COMPLEX";
    }

    this->GetOutput()->GetDcmHeader()->SetValue(
        "DataRepresentation",
        representation
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SignalDomainColumns",
        "FREQUENCY"
    );


    //  Private Attributes for spatial domain encoding:
    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_ColumnsDomain",
        "SPACE"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_RowsDomain",
        "SPACE"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_SliceDomain",
        "SPACE"
    );
}




/*! 
 *  
 */
void svkGEPostageStampReader::LoadData( svkImageData* data )
{

    std::string dataRepresentation = this->GetOutput()->GetDcmHeader()->GetStringValue( "DataRepresentation" ); 
    int numComponents; 
    if ( dataRepresentation == "COMPLEX" ) {
        numComponents = 2; 
    } else {
        numComponents = 1; 
    }

    this->numFreqPts = this->GetOutput()->GetDcmHeader()->GetIntValue( "DataPointColumns" ); 
    this->numTimePts = this->GetOutput()->GetDcmHeader()->GetNumberOfTimePoints();
    int numCoils = this->GetOutput()->GetDcmHeader()->GetNumberOfCoils();

    int numVoxels[3]; 
    data->GetNumberOfVoxels(numVoxels); 

    //  the num pixels in plane
    long unsigned int dataLength = this->imageCols * this->imageCols * numComponents; 

    short* specData = new short[ dataLength ];

    for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {
        
        int realImag = fileIndex % 2; 
        svkImageData* tmpImage = svkMriImageData::New();
        tmpImage->GetDcmHeader()->ReadDcmFile( this->GetFileNames()->GetValue( fileIndex ) );
        //  if this is the 2nd component of complex pair, adjust the offset of 
        //  the specData by 1/2 data length:
        tmpImage->GetDcmHeader()->GetShortValue( "PixelData", specData + realImag * (dataLength/2) , dataLength/2);  

        //  determine voxel range of image within postage stamp:
        //  image may be smaller than postage stamp and padded with 
        //  zeros
        int stampCols = this->imageCols / 16; 
        int stampRows = this->imageRows / 16; 
        int paddingCols = (stampCols - numVoxels[0]) / 2; 
        int paddingRows = (stampRows - numVoxels[1]) / 2; 
        int xStart = paddingCols; 
        int xEnd  = xStart + numVoxels[0]; 
        int yStart = paddingRows; 
        int yEnd  = yStart + numVoxels[1]; 
        
        //  once both components of complex pair have been read in, then set the spectrum
        if ( realImag == 1 ) {
            for (int coilNum = 0; coilNum < numCoils; coilNum ++) {
                for (int timePt = 0; timePt < this->numTimePts; timePt ++) {
                    for (int y = yStart; y < yEnd; y++) {
                        for (int x = xStart; x < xEnd; x++ ) { 
                            int z = fileIndex / 2; 
                            SetCellSpectrum( data, x, y, z, timePt, coilNum, numComponents, specData);
                        }
                    }
                }
            }
        }

        tmpImage->Delete();
    }

    delete [] specData; 

    //this->GetOutput()->GetDcmHeader()->PrintDcmHeader(); 
}


/*!
 *
 */
void svkGEPostageStampReader::SetCellSpectrum(svkImageData* data, int x, int y, int z, int timePt, int coilNum, int numComponents, short* specData)
{

    //  Set XY points to plot - 2 components per tuble for complex data sets:
    vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
    dataArray->SetNumberOfComponents( numComponents );

    int numVoxels[3]; 
    data->GetNumberOfVoxels(numVoxels); 
    int stampXVoxels = this->imageCols / 16; 
    int stampYVoxels = this->imageRows / 16; 
    int paddingCols = (stampXVoxels - numVoxels[0]) / 2; 
    int paddingRows = (stampYVoxels - numVoxels[1]) / 2; 

    dataArray->SetNumberOfTuples(this->numFreqPts);
    char arrayName[30];
    sprintf(arrayName, "%d %d %d %d %d", x - paddingCols, y - paddingRows, z, timePt, coilNum);
    dataArray->SetName(arrayName);

    //  each of the 256 stamps is imageSize/16, e.g. 512/16 = 32.  However the actual image may only be 24x24 voxels centered 
    //  within that stamp.  

    int dx = stampXVoxels;      //  stride along columns
    int dy = this->imageCols * stampYVoxels ;   //  stride along rows (stamp to stamp) 16 stamps per row * row ht

    int componentOffset = this->imageRows * this->imageCols;    // total size of 1 image
    int startIndex = x + y * this->imageCols; 
    float specVal[2];
    int freqPt = 0; 

    // loop over 256 frequency points 
    for ( int rows = 0; rows < 16; rows++) {
        for ( int cols = 0; cols < 16; cols++) {
            int offset = startIndex + cols * dx + rows * dy; 
            specVal[0] =  static_cast<float>(specData[ offset ]);
            specVal[1] =  static_cast<float>(specData[ offset + componentOffset ]);
            dataArray->SetTuple( freqPt, specVal );
            freqPt++; 
        }
    }

    //  Add the spectrum's dataArray to the CellData:
    //  vtkCellData is a subclass of vtkFieldData
    data->GetCellData()->AddArray(dataArray);

    //  Should these be a member var, deleted in destructor?
    dataArray->Delete();

    return;
}


/*!
 *  Returns the file root without extension
 */
svkDcmHeader::DcmPixelDataFormat svkGEPostageStampReader::GetFileType()
{
    return svkDcmHeader::SIGNED_FLOAT_4;
}


/*!
 *
 */
int svkGEPostageStampReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}

