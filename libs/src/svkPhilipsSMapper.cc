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

#ifdef WIN32
#include<signal.h>
#endif
#include<signal.h>
#include <svkPhilipsSMapper.h>
#include <svkSpecUtils.h>
#include <svkImageReader2.h>

#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkTransform.h>
#include </usr/include/vtk/vtkCallbackCommand.h>


using namespace svk;


vtkStandardNewMacro(svkPhilipsSMapper);


/*!
 *
 */
svkPhilipsSMapper::svkPhilipsSMapper()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkPhilipsSMapper");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->specData = NULL;
    this->numFrames = 1;

}


/*!
 *
 */
svkPhilipsSMapper::~svkPhilipsSMapper()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if ( this->specData != NULL )  {
        delete [] specData;
        this->specData = NULL;
    }

}


/*!
 *  Initializes the svkDcmHeader adapter to a specific IOD type      
 *  and initizlizes the svkDcmHeader member of the svkImageData 
 *  object.    
 */
void svkPhilipsSMapper::InitializeDcmHeader(map <string, string>  sparMap, 
    svkDcmHeader* header, svkMRSIOD* iod, int swapBytes) 
{
    this->sparMap = sparMap; 
    this->dcmHeader = header; 
    this->iod = iod;   
    this->swapBytes = swapBytes; 

    this->InitPatientModule();
    this->InitGeneralStudyModule();
    this->InitGeneralSeriesModule();
    this->InitGeneralEquipmentModule();

    this->InitMultiFrameFunctionalGroupsModule();
    //    this->InitMultiFrameDimensionModule();
    //    this->InitAcquisitionContextModule();
    this->InitMRSpectroscopyModule();
    this->InitMRSpectroscopyPulseSequenceModule();

    this->InitMRSpectroscopyDataModule();

    this->dcmHeader->SetValue( "SVK_PRIVATE_TAG",  "SVK_PRIVATE_CREATOR"); 

}


/*!
 *
 */
void svkPhilipsSMapper::InitPatientModule()
{

    this->dcmHeader->InitPatientModule(
        this->dcmHeader->GetDcmPatientName( this->sparMap["patient_name"] ),
        "", //  patientID NA in SPAR
        this->sparMap["patient_birth_date"], 
        ""  // gender NA in SPAR
    );

}


/*!
 *
 */
string svkPhilipsSMapper::GetDcmDate()
{
    string timeDate = this->sparMap["scan_date"]; 
    size_t delim = timeDate.find(" "); 
    string date = timeDate.substr(0, delim); 
    //  assumes order in is month, day, year 
    //  assumes order out is year, month, day
    //  However, philips date order is: scan_date : 2013.11.13 16:03:22
    string reorderedDate = date;
    reorderedDate[0] = date[5];  
    reorderedDate[1] = date[6];  
    reorderedDate[2] = date[7];  
    reorderedDate[3] = date[8];  
    reorderedDate[4] = date[9];  
    reorderedDate[5] = date[4];  
    reorderedDate[6] = date[0];  
    reorderedDate[7] = date[1];  
    reorderedDate[8] = date[2];  
    reorderedDate[9] = date[3];  

    string dcmDate = svkImageReader2::RemoveDelimFromDate(&reorderedDate, '.'); 
        
    return dcmDate; 
}


/*!
 *
 */
void svkPhilipsSMapper::InitGeneralStudyModule()
{
    string dcmDate = this->GetDcmDate(); 

    this->dcmHeader->InitGeneralStudyModule(
        dcmDate,   // studyDate
        "",     // study time 
        "",     // referring phys
        "",     // studyID  
        "",     // accession
        ""      // studyInstanceUID 
    );

}


/*!
 *
 */
void svkPhilipsSMapper::InitGeneralSeriesModule()
{
    this->dcmHeader->InitGeneralSeriesModule(
        "0", 
        "PHILIPS S Data", 
        this->GetDcmPatientPositionString()
    );
}


/*!
 *  DDF is historically the UCSF representation of a GE raw file so
 *  initialize to svkPhilipsSMapper::MFG_STRING.
 */
void svkPhilipsSMapper::InitGeneralEquipmentModule()
{
    this->dcmHeader->SetValue(
        "Manufacturer",
        "Philips"
    );
}


/*!
 *
 */
void svkPhilipsSMapper::InitMultiFrameFunctionalGroupsModule()
{

    this->InitSharedFunctionalGroupMacros();

    this->dcmHeader->SetValue(
        "InstanceNumber",
        1
    );

    this->dcmHeader->SetValue(
        "ContentDate",
        this->GetDcmDate() 
    );

    this->numSlices = this->GetHeaderValueAsInt("nr_of_slices_for_multislice"); 
    int numEchoes   = this->GetHeaderValueAsInt("echo_nr");   // this will probalby only work for n=1

    this->numFrames = this->numSlices * numEchoes;

    this->InitPerFrameFunctionalGroupMacros();

}


/*!
 *
 */
void svkPhilipsSMapper::InitSharedFunctionalGroupMacros()
{

    this->InitPixelMeasuresMacro();
    this->InitPlaneOrientationMacro();

    this->InitMRTimingAndRelatedParametersMacro();
    this->InitMRSpectroscopyFOVGeometryMacro();
    this->InitMREchoMacro();

    this->InitMRModifierMacro();

    this->InitMRReceiveCoilMacro();

    //this->InitMRTransmitCoilMacro();
    this->InitMRAveragesMacro();
    //this->InitMRSpatialSaturationMacro();
    //this->InitMRSpatialVelocityEncodingMacro();

}


/*!
 *  The SPAR toplc is the center of the first voxel.
 *  Best guess at field interpretation all dims in mm. Not positive about difference between two
 *  sets of fields, e.g. ap_off_center and si_ap_off_center, etc.    
 *      ap_size :   AP VOL_LOC FOV
 *      lr_size :   RL VOL_LOC FOV
 *      cc_size :   SI VOL LOC FOV
 *      ap_off_center :  AP displacment from center (from origin)  
 *      lr_off_center :  LR displacement from center (from origin) 
 *      cc_off_center :  SI displacment from center (from origin) 
 *      //  these may be euler angles. 
 *      ap_angulation :  angulation of or about AP axis (tilt forward/backwards) 
 *      lr_angulation :  angluation of or aboutLR axis (tilt left right) 
 *      cc_angulation :  angulation of or aboutSI axis( 
 *      volume_selection_method : 2
 *      si_ap_off_center : 1.225752473
 *      si_lr_off_center : 1.05771184
 *      si_cc_off_center : -2.013401001e-008
 *      si_ap_off_angulation : 0
 *      si_lr_off_angulation : 0
 *      si_cc_off_angulation : 0
 *      phase_encoding_fov :     
 *      slice_thickness : 10
 */
void svkPhilipsSMapper::InitPerFrameFunctionalGroupMacros()
{

    double dcos[3][3];
    this->dcmHeader->SetSliceOrder( this->dataSliceOrder );
    this->dcmHeader->GetDataDcos( dcos );

    double pixelSpacing[3];
    this->dcmHeader->GetPixelSize(pixelSpacing);

    int numPixels[3];
    this->GetDimPnts( numPixels ); 

    float fov[3]; 
    this->GetFOV( fov ); 

    //  Get center coordinate float array from sdatMap and use that to generate
    //  Displace from that coordinate by 1/2 fov - 1/2voxel to get to the center of the
    //  toplc from which the individual frame locations are calculated

    //  Get the volumetric center in acquisition frame coords:
    double center[3];
    center[0] = this->GetHeaderValueAsFloat("si_lr_off_center");
    center[1] = this->GetHeaderValueAsFloat("si_ap_off_center");
    center[2] = this->GetHeaderValueAsFloat("si_cc_off_center");

    //  Center of toplc (LPS) pixel in frame:
    double toplc[3];
    for (int i = 0; i < 3; i++) {
        toplc[i] = center[i]; 
        for (int j = 0; j < 3; j++) {
            toplc[i] -= dcos[j][i] * pixelSpacing[j] * (numPixels[j]/ 2.0 - 0.5 ); 
        }
    }
    
    svkDcmHeader::DimensionVector dimensionVector = this->dcmHeader->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, this->numFrames-1);

    this->dcmHeader->InitPerFrameFunctionalGroupSequence(
        toplc, pixelSpacing, dcos, &dimensionVector
    );

}


/*!
 *  The DICOM PlaneOrientationSequence is set from orientational params defined in the 
 *  Philips sdat file.  
 */
void svkPhilipsSMapper::InitPlaneOrientationMacro()
{

    this->dcmHeader->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PlaneOrientationSequence"
    );

    //  Get the euler angles for the acquisition coordinate system:
    float psi   = this->GetHeaderValueAsFloat("si_lr_angulation"); 
    float phi   = this->GetHeaderValueAsFloat("si_ap_angulation"); 
    float theta = this->GetHeaderValueAsFloat("si_cc_angulation"); 

    vtkMatrix4x4* dcos = vtkMatrix4x4::New();
    string orientationString;
    this->GetDcosFromAngulation( psi, phi, theta, dcos, &orientationString ); 

    this->dcmHeader->AddSequenceItemElement(
        "PlaneOrientationSequence",
        0,
        "ImageOrientationPatient",
        orientationString,
        "SharedFunctionalGroupsSequence",
        0
    );

    //  Determine whether the data is ordered with or against the slice normal direction.
    double normal[3];
    this->dcmHeader->GetNormalVector(normal);

    double dcosSliceOrder[3];
    for (int j = 0; j < 3; j++) {
        dcosSliceOrder[j] = dcos->GetElement(2, j);
    }

    //  Use the scalar product to determine whether the data in the .cmplx
    //  file is ordered along the slice normal or antiparalle to it.
    vtkMath* math = vtkMath::New();
    if (math->Dot(normal, dcosSliceOrder) > 0 ) {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
    } else {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_NEG_NORMAL;
    }
}


/*!
 *  convert angluation to dcos:
 *  Returns a vtkMatrix4x4 representing thd dcos matrix and a string representing the DICOM Orientation
 */
void svkPhilipsSMapper::GetDcosFromAngulation( float psi, float phi, float theta, vtkMatrix4x4* dcos, string* orientationString )
{

    vtkTransform* eulerTransform = vtkTransform::New();
    eulerTransform->RotateX( theta);
    eulerTransform->RotateY( phi );
    eulerTransform->RotateZ( psi );
    eulerTransform->GetMatrix( dcos );

    if (this->GetDebug()) {
        cout << *dcos << endl;
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            ostringstream dcosOss;
            dcosOss.setf(ios::fixed);
            dcosOss << dcos->GetElement(i, j);
            orientationString->append( dcosOss.str() );
            if (i != 1 || j != 2  ) {
                orientationString->append( "\\");
            }
        }
    }

}



/*!
 *
 */
void svkPhilipsSMapper::InitMRTimingAndRelatedParametersMacro()
{
    this->dcmHeader->InitMRTimingAndRelatedParametersMacro(
        this->GetHeaderValueAsFloat( "repetition_time" )
    ); 
}


/*!
 *
 */
void svkPhilipsSMapper::InitMRSpectroscopyFOVGeometryMacro()
{
    int numPixels[3];
    this->GetDimPnts( numPixels ); 

    this->dcmHeader->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRSpectroscopyFOVGeometrySequence"
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionDataColumns",
        this->GetHeaderValueAsInt("spec_num_col"),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionPhaseColumns",
        numPixels[0], 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionPhaseRows",
        numPixels[1], 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionOutOfPlanePhaseSteps",
        numPixels[2], 
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


    double pixelSpacing[3];
    this->dcmHeader->GetPixelSize(pixelSpacing);
    string pixelSizeString[2];
    for (int i = 0; i < 2; i++) {
        ostringstream oss;
        oss << pixelSpacing[i];
        pixelSizeString[i].assign( oss.str() );
    }
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionPixelSpacing",
        pixelSizeString[0] + '\\' + pixelSizeString[1],
        "SharedFunctionalGroupsSequence",
        0
    );

    float fov[3]; 
    this->GetFOV( fov ); 

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionSliceThickness",
        fov[2], 
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
        numPixels[0], 
        "SharedFunctionalGroupsSequence",
        0
    );
    
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                               
        "SVK_SpectroscopyAcqReorderedPhaseRows",
        numPixels[1], 
        "SharedFunctionalGroupsSequence",
        0
    );
    
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                               
        "SVK_SpectroscopyAcqReorderedOutOfPlanePhaseSteps",
        numPixels[2], 
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
        pixelSizeString[0] + '\\' + pixelSizeString[1],
        "SharedFunctionalGroupsSequence",
        0
    );
    
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,                                      
        "SVK_SpectroscopyAcqReorderedSliceThickness",
        //this->GetHeaderValueAsFloat("slice_thickness"),
        fov[2], 
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

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "PercentSampling",
        1,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
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
void svkPhilipsSMapper::InitMRSpectroscopyPulseSequenceModule()
{
}


/*!
 *
 */
void svkPhilipsSMapper::InitMREchoMacro()
{
    this->dcmHeader->InitMREchoMacro( this->GetHeaderValueAsFloat( "repetition_time" )  );
}


/*!
 *  Override in concrete mapper for specific acquisitino
 */
void svkPhilipsSMapper::InitMRModifierMacro()
{
    this->dcmHeader->InitMRModifierMacro( this->GetHeaderValueAsFloat("spectrum_inversion_time"));
}


/*!
 *
 */
void svkPhilipsSMapper::InitMRTransmitCoilMacro()
{
    this->dcmHeader->InitMRTransmitCoilMacro("Philips", "UNKNOWN", "BODY");
}


/*! 
 *  Receive Coil:
 */
void svkPhilipsSMapper::InitMRReceiveCoilMacro()
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
        "Philips Coil",
        "SharedFunctionalGroupsSequence",
        0
    );

}


/*!
 *
 */
void svkPhilipsSMapper::InitVolumeLocalizationSeq()
{

    float size[3];
    size[0] = this->GetHeaderValueAsFloat( "lr_size" );
    size[1] = this->GetHeaderValueAsFloat( "ap_size" );
    size[2] = this->GetHeaderValueAsFloat( "cc_size" );

    float center[3];
    center[0] = this->GetHeaderValueAsFloat( "lr_off_center" );
    center[1] = this->GetHeaderValueAsFloat( "ap_off_center" );
    center[2] = this->GetHeaderValueAsFloat( "cc_off_center" );


    //  Get the euler angles for the acquisition coordinate system:
    float psi   = this->GetHeaderValueAsFloat("lr_angulation"); 
    float phi   = this->GetHeaderValueAsFloat("ap_angulation"); 
    float theta = this->GetHeaderValueAsFloat("cc_angulation"); 

    vtkMatrix4x4* dcos = vtkMatrix4x4::New();
    string orientationString;
    this->GetDcosFromAngulation( psi, phi, theta, dcos, &orientationString ); 

    float dcos3x3[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            dcos3x3[i][j] = dcos->GetElement(i, j);
        }
    }

    this->dcmHeader->InitVolumeLocalizationSeq(
        size,
        center,
        dcos3x3
    );

}



/*!
 *
 */
void svkPhilipsSMapper::InitMRAveragesMacro()
{
    int numAverages = 1; 
    this->dcmHeader->InitMRAveragesMacro(numAverages);
}


/*!
 *
 */
void svkPhilipsSMapper::InitMultiFrameDimensionModule()
{
    int indexCount = 0; 
    this->dcmHeader->AddSequenceItemElement(
        "DimensionIndexSequence",
        indexCount,
        "DimensionDescriptionLabel",
        "Slice"
    );

/*
    if (this->GetNumTimePoints() > 1) {
        indexCount++; 
        this->dcmHeader->AddSequenceItemElement(
            "DimensionIndexSequence",
            indexCount,
            "DimensionDescriptionLabel",
            "Time Point"
        );
    }
*/

//      if (this->GetNumCoils() > 1) {
//          indexCount++; 
//          this->dcmHeader->AddSequenceItemElement(
//          "DimensionIndexSequence",
//          indexCount,
//          "DimensionDescriptionLabel",
//          "Coil Number"
//      );

//
//        this->dmHeader()->AddSequenceItemElement(
//            "DimensionIndexSequence",
//            1,
//            "DimensionIndexPointer",
//            "18H\\00H\\47H\\90"
//        );
//        this->dcmHeader->AddSequenceItemElement(
//            "DimensionIndexSequence",
//            1,
//            "FunctionalGroupPointer",
//            //"MultiCoilDefinitionSequence"
//            "18H\\00H\\47H\\90"
//        );
//  }

}


/*!
 *
 */
void svkPhilipsSMapper::InitAcquisitionContextModule()
{
}


/*!
 *
 */
void svkPhilipsSMapper::InitMRSpectroscopyModule()
{

    /*  =======================================
     *  MR Image and Spectroscopy Instance Macro
     *  ======================================= */

    this->dcmHeader->SetValue(
        "AcquisitionDateTime",
        this->sparMap["date"]
    );
    
    this->dcmHeader->SetValue(
        "AcquisitionDuration",
        0
    );


    this->dcmHeader->SetValue(
        "ResonantNucleus",
        this->sparMap["nucleus"] 
    );

    this->dcmHeader->SetValue(
        "KSpaceFiltering",
        "NONE"
    );

    this->dcmHeader->SetValue(
        "ApplicableSafetyStandardAgency",
        "Research"
    );

    //  B0 in Gauss?
    float fieldStrength = svkSpecUtils::GetFieldStrength( 
        this->sparMap["nucleus"],
        this->GetHeaderValueAsFloat( "synthesizer_frequency" ) / 1000000.
    ); 

    this->dcmHeader->SetValue(
        "MagneticFieldStrength",
        fieldStrength
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
        this->GetHeaderValueAsFloat( "synthesizer_frequency" ) / 1000000
    );

    this->dcmHeader->SetValue(
        "SpectralWidth",
        this->GetHeaderValueAsFloat( "sample_frequency" )
    );

    this->dcmHeader->SetValue(
        "SVK_FrequencyOffset",
        0
    );

    //  sp is the frequency in Hz at left side (downfield/High freq) 
    //  side of spectrum: 
    //
    float ppmRef = this->GetHeaderValueAsFloat( "sample_frequency" )/2.;
    ppmRef /= (this->GetHeaderValueAsFloat( "synthesizer_frequency" )/1000000.); 
    this->dcmHeader->SetValue(
        "ChemicalShiftReference",
        ppmRef 
    );

    this->dcmHeader->SetValue(
        "VolumeLocalizationTechnique",
        "PRESS"
    );

    this->InitVolumeLocalizationSeq();


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
 *  SPAR DATA is spatially reconstructed already, but spectral domain is in 
 *  time. 
 */
void svkPhilipsSMapper::InitMRSpectroscopyDataModule()
{
    int numPixels[3];
    this->GetDimPnts( numPixels ); 

    this->dcmHeader->SetValue( "Columns", numPixels[0] );
    this->dcmHeader->SetValue( "Rows",    numPixels[1] );
    this->dcmHeader->SetValue( "DataPointRows", 0 );
    this->dcmHeader->SetValue( "DataPointColumns", this->GetHeaderValueAsInt("dim1_pnts") );
    this->dcmHeader->SetValue( "DataRepresentation", "COMPLEX" );
    this->dcmHeader->SetValue( "SignalDomainColumns", "TIME" );
    this->dcmHeader->SetValue( "SVK_ColumnsDomain", "SPACE" );
    this->dcmHeader->SetValue( "SVK_RowsDomain", "SPACE" );
    this->dcmHeader->SetValue( "SVK_SliceDomain", "SPACE" );
}


/*!
 *  Reads spec data from sdat file.
 */
void svkPhilipsSMapper::ReadSDATFile( string sdatFileName, svkImageData* data )
{
    
    vtkDebugMacro( << this->GetClassName() << "::ReadSDATFile()" );


    int pixelWordSize = 4;
    int numComponents = 2;
    int numSpecPoints = this->dcmHeader->GetIntValue( "DataPointColumns" );

    int numVoxels[3]; 
    numVoxels[0] = this->dcmHeader->GetIntValue( "Columns" ); 
    numVoxels[1] = this->dcmHeader->GetIntValue( "Rows" ); 
    numVoxels[2] = this->dcmHeader->GetNumberOfSlices( ); 

    int numPixInVolume = numVoxels[0] * numVoxels[1] * numVoxels[2]; 
    int numBytesInVol = ( numPixInVolume * pixelWordSize * numComponents * numSpecPoints );

    /*
     *   Flatten the data volume into one dimension
     */
    int numWordsInSDAT = numBytesInVol/pixelWordSize; 
    float* specDataVax = new float[ numWordsInSDAT ];
    if (this->specData == NULL) {
        this->specData = new float[ numWordsInSDAT ];
    }

    string sdatFile = svkImageReader2::GetFileRoot( sdatFileName.c_str() ) + ".SDAT";

    ifstream* sdatDataIn = new ifstream();

    try {

        sdatDataIn->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
        sdatDataIn->open( sdatFile.c_str(), ifstream::binary );
        if ( ! sdatDataIn->is_open() ) {
            throw runtime_error( "Could not open sdat file: " + sdatFile);
        }

        sdatDataIn->seekg(0, ios::beg);
        cout << "TELL:" << sdatDataIn->tellg(); 
        sdatDataIn->read( (char *)(specDataVax), numBytesInVol);
        
        this->VaxToFloat( (void*)(specDataVax), (void*)(this->specData), &numWordsInSDAT); 

        delete [] specDataVax; 
        
    } catch (const exception& e) {
        cout <<  "ERROR reading file: " << sdatFile << " : "  << e.what() << endl;
        exit(1); 
    }

    svkDcmHeader* hdr = this->dcmHeader;

    double progress = 0;

    int numTimePts = hdr->GetNumberOfTimePoints(); 
    int numCoils = hdr->GetNumberOfCoils(); 
    for (int coilNum = 0; coilNum < numCoils; coilNum++) {
        for (int timePt = 0; timePt < numTimePts; timePt++) {

            for (int z = 0; z < numVoxels[2] ; z++) {
                for (int y = 0; y < numVoxels[1]; y++) {
                    for (int x = 0; x < numVoxels[0]; x++) {
                        SetCellSpectrum(data, x, y, z, timePt, coilNum);

                        if( timePt % 2 == 0 ) { // only update every other index
				            progress = (timePt * coilNum)/((double)(numTimePts*numCoils));
				            this->InvokeEvent(vtkCommand::ProgressEvent, static_cast<void *>(&progress));
        	            }
                    }
                }
            }
        }
    }

    progress = 1;
	this->InvokeEvent(vtkCommand::ProgressEvent,static_cast<void *>(&progress));

    if ( ! sdatDataIn->is_open() ) {
        sdatDataIn->close();
    }
    delete sdatDataIn;

}


/*!
 *
 */
void svkPhilipsSMapper::SetCellSpectrum(vtkImageData* data, int x, int y, int z, int timePt, int coilNum)
{

    int numComponents = 1;
    string representation =  this->dcmHeader->GetStringValue( "DataRepresentation" );
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
    //int xTmp = x; 
    //x = y; 
    //y = xTmp; 
    //x = numVoxels[0] - x - 1; 
    //y = numVoxels[1] - y - 1; 

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


/*!
 *  Use the Procpar patient position string to set the DCM_PatientPosition data element.
 */
string svkPhilipsSMapper::GetDcmPatientPositionString()
{
    string dcmPatientPosition;

    string position1 = this->sparMap["patient_position"]; 
    if( position1.find("head_first") != string::npos ) {
        dcmPatientPosition.assign("HF");
    } else if( position1.find("feet_first") != string::npos ) {
        dcmPatientPosition.assign("FF");
    } else {
        dcmPatientPosition.assign("UNKNOWN");
    }

    string position2 = this->sparMap["patient_orientation"]; 
    if( position2.find("supine") != string::npos ) {
        dcmPatientPosition += "S";
    } else if( position2.find("prone") != string::npos ) {
        dcmPatientPosition += "P";
    } else if( position2.find("decubitus left") != string::npos ) {
        dcmPatientPosition += "DL";
    } else if( position2.find("decubitus right") != string::npos ) {
        dcmPatientPosition += "DR";
    } else {
        dcmPatientPosition += "UNKNOWN";
    }

    return dcmPatientPosition;
}


/*!
 *  \param keystring
 *  \return value if key is found, or INT_MIN otherwise 
 */
int svkPhilipsSMapper::GetHeaderValueAsInt(string keyString)
{

    istringstream* iss = new istringstream();
    int value;

    if ( this->sparMap.count(keyString) != 0 ) {

        iss->str( (this->sparMap[keyString]) );
        *iss >> value;
    } else {
        cout << "WARNING:  could not find map key: " << keyString << endl;
        value = INT_MIN; 
    }

    delete iss; 

    return value;
}


/*!
 *
 */
float svkPhilipsSMapper::GetHeaderValueAsFloat(string keyString )
{

    istringstream* iss = new istringstream();
    float value;
    iss->str( (this->sparMap[keyString]) );
    *iss >> value;

    delete iss; 

    return value;
}


/*!
 *  Pixel Spacing:
 */
void svkPhilipsSMapper::InitPixelMeasuresMacro()
{
    int numPixels[3];
    this->GetDimPnts( numPixels ); 

    float fov[3];
    this->GetFOV( fov ); 

    float pixelSize[3];
    for (int i = 0; i < 3; i++ ) {
        pixelSize[i] = fov[i]/numPixels[i]; 
    }

    string pixelSizeString[3];

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


/*
 *  Sets the fov in 3 dimensions. Only applies to isotropic voxels for now.  
 *  Philips informs that only isotropic voxles are supported in the reconstructed
 *  S* files.  Are anisotropic voxels supported: 
 *  "No, since the "Reconstructed voxel size" is set by a single number, it is usually equal 
 *  to or less than the smallest voxel size set."
 */
void svkPhilipsSMapper::GetFOV(float* fov) 
{

    int numPixels[3];
    this->GetDimPnts( numPixels ); 

    fov[0] = this->GetHeaderValueAsFloat("phase_encoding_fov");
    float isotropicPixelSize = fov[0] / numPixels[0];  
    
    fov[1] = numPixels[1] * isotropicPixelSize; 
    fov[2] = numPixels[2] * this->GetHeaderValueAsFloat("slice_thickness");
}


/*
 *  Sets the number of spatial dimensions.  If a dimension isn't defined, 
 *  sets the value to 1
 */
void svkPhilipsSMapper::GetDimPnts(int* numPixels) 
{

    //  this should look at the value of "dimX_direction" to set the 3 indices correctly
    numPixels[0] = this->GetHeaderValueAsInt("dim2_pnts"); 
    numPixels[1] = this->GetHeaderValueAsInt("dim3_pnts"); 
    numPixels[2] = this->GetHeaderValueAsInt("dim4_pnts"); 
    for (int i = 0; i < 3; i++ ) {
        if ( numPixels[i] < 0 ) {
            numPixels[i] = 1; 
        }
    }
}




/* 
 * using VAX to Float conversion method implemented by Lawrence M. Baker of 
 * USGS: 
 * 
 * References:  ANSI/IEEE Std 754-1985, IEEE Standard for Binary Floating-    *
 *                 Point Arithmetic, Institute of Electrical and Electronics  *
 *                 Engineers                                                  *
 *              Brunner, Richard A., Ed., 1991, VAX Architecture Reference    *
 *                 Manual, Second Edition, Digital Press                      *
 *              Sites, Richard L., and Witek, Richard T., 1995, Alpha AXP     *
 *                 Architecture Reference Manual, Second Edition, Digital     *
 *                 Press                                                      *
 *                                                                            *
 * Author:      Lawrence M. Baker                                             *
 *              U.S. Geological Survey                                        *
 *              345 Middlefield Road  MS977                                   *
 *              Menlo Park, CA  94025                                         *
 *              baker@usgs.gov                                                *
 *                                                                            *
 * Citation:    Baker, L.M., 2005, libvaxdata: VAX Data Format Conversion     *
 *                 Routines: U.S. Geological Survey Open-File Report 2005-    *
 *                 1424 (http://pubs.usgs.gov/of/2005/1424/).                 *
 *                                                                            *
 *                                                                            *
 *                                 Disclaimer                                 *
 *                                                                            *
 * Although this program has been used by the USGS, no warranty, expressed or *
 * implied, is made by the USGS or the United States  Government  as  to  the *
 * accuracy  and functioning of the program and related program material, nor *
 * shall the fact of  distribution  constitute  any  such  warranty,  and  no *
 * responsibility is assumed by the USGS in connection therewith.             *
 *                                                                            *
 */
#define SIGN_BIT             0x80000000
#define VAX_F_EXPONENT_MASK  0x7F800000
#define VAX_F_MANTISSA_MASK  0x007FFFFF
#define VAX_F_MANTISSA_SIZE  23
#define VAX_F_EXPONENT_SIZE  8
#define VAX_F_EXPONENT_BIAS  ( 1 << ( VAX_F_EXPONENT_SIZE - 1 ) )
#define VAX_F_HIDDEN_BIT     ( 1 << VAX_F_MANTISSA_SIZE )
#define IEEE_S_EXPONENT_SIZE 8
#define IEEE_S_EXPONENT_BIAS ( ( 1 << ( IEEE_S_EXPONENT_SIZE - 1 ) ) - 1 )
#define IEEE_S_MANTISSA_SIZE 23
#define MANTISSA_MASK VAX_F_MANTISSA_MASK
#define MANTISSA_SIZE VAX_F_MANTISSA_SIZE
#define HIDDEN_BIT    VAX_F_HIDDEN_BIT
#define EXPONENT_ADJUSTMENT ( 1 + VAX_F_EXPONENT_BIAS - IEEE_S_EXPONENT_BIAS )
#define IN_PLACE_EXPONENT_ADJUSTMENT \
           ( EXPONENT_ADJUSTMENT << IEEE_S_MANTISSA_SIZE )
void svkPhilipsSMapper::VaxToFloat( const void *inbuf, void *outbuf,const int *count ) 
{

   register const unsigned short* in;   
   union { 
        unsigned short i[2]; 
        unsigned int l; 
   } vaxpart;
   register unsigned int* out;         
   unsigned int vaxpart1;
   int n;
   int e;


   in  = (const unsigned short *) inbuf;
   out = (unsigned int *) outbuf;

   for ( n = *count; n > 0; n-- ) {

      vaxpart.i[1] = *in++;
      vaxpart.i[0] = *in++;
      vaxpart1     = vaxpart.l;

      if ( ( e = ( vaxpart1 & VAX_F_EXPONENT_MASK ) ) == 0 ) {
                                  /* If the biased VAX exponent is zero [e=0] */

         if ( ( vaxpart1 & SIGN_BIT ) == SIGN_BIT ) {    /* If negative [s=1] */
            raise( SIGFPE );/* VAX reserved operand fault; fixup to IEEE zero */
         }
           /* Set VAX dirty [m<>0] or true [m=0] zero to IEEE +zero [s=e=m=0] */
         *out++ = 0;

      } else {                  /* The biased VAX exponent is non-zero [e<>0] */

         e >>= MANTISSA_SIZE;               /* Obtain the biased VAX exponent */

         /* The  biased  VAX  exponent  has to be adjusted to account for the */
         /* right shift of the IEEE mantissa binary point and the  difference */
         /* between  the biases in their "excess n" exponent representations. */
         /* If the resulting biased IEEE exponent is less than  or  equal  to */
         /* zero, the converted IEEE S_float must use subnormal form.         */

         if ( ( e -= EXPONENT_ADJUSTMENT ) > 0 ) {
                                            /* Use IEEE normalized form [e>0] */

            /* Both mantissas are 23 bits; adjust the exponent field in place */
            *out++ = vaxpart1 - IN_PLACE_EXPONENT_ADJUSTMENT;

         } else {                       /* Use IEEE subnormal form [e=0, m>0] */

            /* In IEEE subnormal form, even though the biased exponent is 0   */
            /* [e=0], the effective biased exponent is 1.  The mantissa must  */
            /* be shifted right by the number of bits, n, required to adjust  */
            /* the biased exponent from its current value, e, to 1.  I.e.,    */
            /* e + n = 1, thus n = 1 - e.  n is guaranteed to be at least 1   */
            /* [e<=0], which guarantees that the hidden 1.m bit from the ori- */
            /* ginal mantissa will become visible, and the resulting subnor-  */
            /* mal mantissa will correctly be of the form 0.m.                */

            *out++ = ( vaxpart1 & SIGN_BIT ) |
                     ( ( HIDDEN_BIT | ( vaxpart1 & MANTISSA_MASK ) ) >>
                       ( 1 - e ) );

         }
      }
   }

}

#undef SIGN_BIT
#undef VAX_F_EXPONENT_MASK
#undef VAX_F_MANTISSA_MASK
#undef VAX_F_MANTISSA_SIZE
#undef VAX_F_EXPONENT_SIZE
#undef VAX_F_EXPONENT_BIAS
#undef VAX_F_HIDDEN_BIT
#undef IEEE_S_EXPONENT_SIZE
#undef IEEE_S_EXPONENT_BIAS
#undef IEEE_S_MANTISSA_SIZE
#undef MANTISSA_MASK
#undef MANTISSA_SIZE
#undef HIDDEN_BIT
#undef EXPONENT_ADJUSTMENT
#undef IN_PLACE_EXPONENT_ADJUSTMENT

