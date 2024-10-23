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


#include <svkBrukerRawMRSMapper.h>
#include <svkVarianReader.h>
#include <svkFreqCorrect.h>
#include <svkTypeUtils.h>

#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkTransform.h>
#include </usr/include/vtk/vtkMatrix4x4.h>
#include </usr/include/vtk/vtkByteSwap.h>
#include </usr/include/vtk/vtkCallbackCommand.h>


using namespace svk;


vtkStandardNewMacro(svkBrukerRawMRSMapper);



/*!
 *
 */
svkBrukerRawMRSMapper::svkBrukerRawMRSMapper()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkBrukerRawMRSMapper");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->specData = NULL;
    this->numFrames = 1;

}


/*!
 *
 */
svkBrukerRawMRSMapper::~svkBrukerRawMRSMapper()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if ( this->specData != NULL )  {
        delete [] (char*)specData;
        this->specData = NULL;
    }

}


/*!
 *  Initializes the svkDcmHeader adapter to a specific IOD type      
 *  and initizlizes the svkDcmHeader member of the svkImageData 
 *  object.    
 */
void svkBrukerRawMRSMapper::InitializeDcmHeader(map <string, vector < string> >  paramMap, 
    svkDcmHeader* header, svkMRSIOD* iod, int swapBytes) 
{
    this->paramMap = paramMap; 
    this->dcmHeader = header; 
    this->iod = iod;   
    this->swapBytes = swapBytes; 

    this->InitPatientModule();
    this->InitGeneralStudyModule();
    this->InitGeneralSeriesModule();
    this->InitGeneralEquipmentModule();

    this->InitMultiFrameFunctionalGroupsModule();
    this->InitMultiFrameDimensionModule();
//    this->InitAcquisitionContextModule();
    this->InitMRSpectroscopyModule();
    this->InitMRSpectroscopyPulseSequenceModule();

    this->InitMRSpectroscopyDataModule();

    this->dcmHeader->SetValue( "SVK_PRIVATE_TAG",  "SVK_PRIVATE_CREATOR"); 

}


/*!
 *
 */
void svkBrukerRawMRSMapper::InitPatientModule()
{

    this->dcmHeader->InitPatientModule(
        this->dcmHeader->GetDcmPatientName( this->GetHeaderValueAsString("SUBJECT_name_string") ),
        this->GetHeaderValueAsString("SUBJECT_id"), 
        this->GetHeaderValueAsString("SUBJECT_dbirth"), 
        this->GetHeaderValueAsString("SUBJECT_sex") 
    );

}


/*!
 *
 */
void svkBrukerRawMRSMapper::InitGeneralStudyModule()
{
    string timeDate = this->GetHeaderValueAsString( "SUBJECT_date" ); 
    size_t delim = timeDate.find("T"); 
    string date = timeDate.substr(0, delim); 

    this->dcmHeader->InitGeneralStudyModule(
        date, 
        "",
        "",
        this->GetHeaderValueAsString("SUBJECT_study_instance_uid"), 
        "", 
        ""
    );

}


/*!
 *
 */
void svkBrukerRawMRSMapper::InitGeneralSeriesModule()
{
    this->dcmHeader->InitGeneralSeriesModule(
        "0", 
        "BRUKER SER Data", 
        this->GetDcmPatientPositionString()
    );
}


/*!
 *  DDF is historically the UCSF representation of a GE raw file so
 *  initialize to svkBrukerRawMRSMapper::MFG_STRING.
 */
void svkBrukerRawMRSMapper::InitGeneralEquipmentModule()
{
    this->dcmHeader->SetValue(
        "Manufacturer",
        "Bruker"
    );
}


/*!
 *
 */
void svkBrukerRawMRSMapper::InitMultiFrameFunctionalGroupsModule()
{

    this->InitSharedFunctionalGroupMacros();

    this->dcmHeader->SetValue(
        "InstanceNumber",
        1
    );

    string timeDate = this->GetHeaderValueAsString( "SUBJECT_date" );
    size_t delim = timeDate.find("T");
    string date = timeDate.substr(0, delim);

    this->dcmHeader->SetValue(
        "ContentDate",
        date
    );

    this->numSlices = 1; 
    int numEchoes = this->GetHeaderValueAsInt("NECHOES");

    this->numFrames = this->numSlices * numEchoes;
    this->dcmHeader->SetValue(
        "NumberOfFrames",
        this->numFrames
    );

    this->InitPerFrameFunctionalGroupMacros();

}


/*!
 *
 */
void svkBrukerRawMRSMapper::InitSharedFunctionalGroupMacros()
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
 *  The FID toplc is the center of the first voxel.
 */
void svkBrukerRawMRSMapper::InitPerFrameFunctionalGroupMacros()
{

    double dcos[3][3];
    this->dcmHeader->SetSliceOrder( this->dataSliceOrder );
    this->dcmHeader->GetDataDcos( dcos );

    double pixelSpacing[3];
    this->dcmHeader->GetPixelSize(pixelSpacing);

    int numPixels[3];
    numPixels[0] = this->GetHeaderValueAsInt("ACQ_spatial_size_0");
    numPixels[1] = this->GetHeaderValueAsInt("ACQ_spatial_size_1");
    numPixels[2] = 1; 

    //  Get center coordinate float array from fidMap and use that to generate
    //  Displace from that coordinate by 1/2 fov - 1/2voxel to get to the center of the
    //  toplc from which the individual frame locations are calculated

    //  If volumetric 3D (not 2D), get the center of the TLC voxel in LPS coords:
    double* volumeTlcLPSFrame = new double[3];
    //  if more than 1 slice:
    if ( numPixels[2] > 1 ) {

        //  Get the volumetric center in acquisition frame coords:
        double volumeCenterAcqFrame[3];
        volumeCenterAcqFrame[0] = 0.; 
        volumeCenterAcqFrame[1] = 0.; 
        //  Invert about origin in SI: 
        volumeCenterAcqFrame[2] = -1 * this->GetHeaderValueAsFloat("ACQ_slice_offset"); 
        cout << "OFFSET: " << volumeCenterAcqFrame[2] << endl;

        double* volumeTlcAcqFrame = new double[3];
        for (int i = 0; i < 3; i++) {
            volumeTlcAcqFrame[i] = volumeCenterAcqFrame[i]
                                 + (100 - pixelSpacing[i] )/2;
        }
        svkVarianReader::UserToMagnet(volumeTlcAcqFrame, volumeTlcLPSFrame, dcos);
        delete [] volumeTlcAcqFrame;

    }


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
        centerAcqFrame[0] = 0.0; 
        centerAcqFrame[1] = 0.0; 
        //  Invert about origin in SI: 
        centerAcqFrame[2] = -1 * this->GetHeaderValueAsFloat("ACQ_slice_offset"); 
        cout << "OFFSET: " << centerAcqFrame[2] << endl;

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
    
    } else {

        for(int j = 0; j < 3; j++) { //L, P, S
            toplc[j] = volumeTlcLPSFrame[j]; 
        }
    
    }



    svkDcmHeader::DimensionVector dimensionVector = this->dcmHeader->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, this->numFrames-1);
    int numTimePts = this->GetNumTimePoints();
    this->dcmHeader->AddDimensionIndex( &dimensionVector, svkDcmHeader::TIME_INDEX, numTimePts - 1 );


    this->dcmHeader->InitPerFrameFunctionalGroupSequence(
        toplc, pixelSpacing, dcos, &dimensionVector
    );

    delete volumeTlcLPSFrame;
}


/*!
 *  
 *
 */
void svkBrukerRawMRSMapper::InitPlaneOrientationMacro()
{

    this->dcmHeader->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PlaneOrientationSequence"
    );

    
    string orientationString; 
    float dcos[3][3]; 
    this->GetDcmOrientation(dcos, &orientationString); 

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
        dcosSliceOrder[j] = dcos[2][j];
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
 *  Initializes the numeric and string representation of the orientation
 */
void svkBrukerRawMRSMapper::GetDcmOrientation(float dcos[3][3], string* orientationString) 
{

    //  Get the dcos from the SPackArrGradOrient field
    *orientationString = this->GetHeaderValueAsString("PVM_SPackArrGradOrient", 0);
    char* cstr = new char [orientationString->length()+1];
    strcpy( cstr, orientationString->c_str() );
    float dcosTmp[9]; 
    char* element; 
    dcosTmp[0] = svkTypeUtils::StringToFloat( strtok( cstr, " ") ); 
    for ( int i = 1; i < 9; i++ ) {    
        dcosTmp[i] = svkTypeUtils::StringToFloat( strtok( NULL, " ") ); 
    }

    //  RAS 
    dcos[0][0] = dcosTmp[0]; 
    dcos[0][1] = dcosTmp[1]; 
    dcos[0][2] = dcosTmp[2]; 
    dcos[1][0] = dcosTmp[3]; 
    dcos[1][1] = dcosTmp[4]; 
    dcos[1][2] = dcosTmp[5]; 
    dcos[2][0] = dcosTmp[6]; 
    dcos[2][1] = dcosTmp[7]; 
    dcos[2][2] = dcosTmp[8]; 

    string encoding = this->GetHeaderValueAsString("PVM_EncOrder");
    if ( encoding.find("CENTRIC_ENC") != string::npos ) { 
        encoding = "CENTRIC"; 
    } else if ( encoding.find("LINEAR_ENC") != string::npos  ) { 
        encoding = "LINEAR"; 
    }  else {
        //  make this the default.  Not sure if would be specified for all sequences. 
        encoding = "LINEAR"; 
    }

    if (encoding.compare("LINEAR") == 0 ) {

        //  Invert about origin in SI: 
        float inverter[3][3]; 
        inverter[0][0] = 1;   
        //inverter[0][0] = -1;   
        inverter[0][1] = 0;  
        inverter[0][2] = 0;  
        inverter[1][0] = 0;  
        inverter[1][1] = 1;  
        //inverter[1][1] = -1;  
        inverter[1][2] = 0;  
        inverter[2][0] = 0;  
        inverter[2][1] = 0;  
        //inverter[2][2] = 1;  
        inverter[2][2] = -1;  

        this->MatMult( dcos, inverter );  
    }

    this->FixBrukerOrientationAnomalies( dcos ); 

    //orientationString->assign(""); 
    orientationString->append( svkTypeUtils::DoubleToString( dcos[0][0] ) ); 
    orientationString->append("\\");
    orientationString->append( svkTypeUtils::DoubleToString( dcos[0][1] ) ); 
    orientationString->append("\\");
    orientationString->append( svkTypeUtils::DoubleToString( dcos[0][2] ) ); 
    orientationString->append("\\");
    orientationString->append( svkTypeUtils::DoubleToString( dcos[1][0] ) ); 
    orientationString->append("\\");
    orientationString->append( svkTypeUtils::DoubleToString( dcos[1][1] ) ); 
    orientationString->append("\\");
    orientationString->append( svkTypeUtils::DoubleToString( dcos[1][2] ) ); 
    orientationString->append("\\");
    orientationString->append( svkTypeUtils::DoubleToString( dcos[2][0] ) ); 
    orientationString->append("\\");
    orientationString->append( svkTypeUtils::DoubleToString( dcos[2][1] ) ); 
    orientationString->append("\\");
    orientationString->append( svkTypeUtils::DoubleToString( dcos[2][2] ) ); 
}


/*!
 *  Heuristic adjustments to Bruker orientation to account for orientation "anomolies"
 *  observed in validation phantoms: 
 *      1.  Centric Coronal is inverted through all 3 directions (LP and S) 
 */
void svkBrukerRawMRSMapper::FixBrukerOrientationAnomalies( float dcos[3][3] )
{
    string encoding = this->GetHeaderValueAsString("PVM_EncOrder");
    if ( encoding.find("CENTRIC_ENC") != string::npos ) { 
        encoding = "CENTRIC"; 
    } else if ( encoding.find("LINEAR_ENC") != string::npos  ) { 
        encoding = "LINEAR"; 
    }  else {
        //  make this the default.  Not sure if would be specified for all sequences. 
        encoding = "LINEAR"; 
    }

    //  invert about LPS 
    //  axial if dcos[2][2] = 1 (assuming non oblique data)
    //  coronal if dcos[2][1] = 1 (assuming non oblique data)
    if (encoding.compare("CENTRIC") == 0  ) { 
        float inverter[3][3]; 
        inverter[0][0] = 1;   
        inverter[0][1] = 0;  
        inverter[0][2] = 0;  
        inverter[1][0] = 0;  
        inverter[1][1] = 1;  
        inverter[1][2] = 0;  
        inverter[2][0] = 0;  
        inverter[2][1] = 0;  
        inverter[2][2] = 1;  
        if ( dcos[2][1] == 1 || dcos[2][1] == -1 ) {
            //  CORONAL: Invert about origin in L: 
            inverter[0][0] = -1;   
        } 
        if ( dcos[2][2] == 1 || dcos[2][2] == -1 ) {
            //  AXIAL: Invert about origin in LPS: 
            inverter[0][0] = -1;   
            inverter[1][1] = -1;  
            inverter[2][2] = -1;  
        }
        this->MatMult( dcos, inverter );  
    }
}


/*!
 *  Probably implemented elsewhere, stupid. Returns product in the original 
 *  A matrix
 */
void svkBrukerRawMRSMapper::MatMult(float A[3][3], float B[3][3] )
{
    float tmp[3][3];
    tmp[0][0] = 0; 
    tmp[0][1] = 0; 
    tmp[0][2] = 0; 
    tmp[1][0] = 0; 
    tmp[1][1] = 0; 
    tmp[1][2] = 0; 
    tmp[2][0] = 0; 
    tmp[2][1] = 0; 
    tmp[2][2] = 0; 

    for ( int j = 0; j < 3; j++ ) { 
        for ( int i = 0; i < 3; i++ ) { 
            for ( int k = 0; k < 3; k++ ) { 
                tmp[j][i] += A[j][k] * B[k][i];  
            }
        }
    }

    A[0][0] = tmp[0][0]; 
    A[0][1] = tmp[0][1]; 
    A[0][2] = tmp[0][2]; 
    A[1][0] = tmp[1][0]; 
    A[1][1] = tmp[1][1]; 
    A[1][2] = tmp[1][2]; 
    A[2][0] = tmp[2][0]; 
    A[2][1] = tmp[2][1]; 
    A[2][2] = tmp[2][2]; 

}
     

/*!
 *
 */
void svkBrukerRawMRSMapper::InitMRTimingAndRelatedParametersMacro()
{
    this->dcmHeader->InitMRTimingAndRelatedParametersMacro(
        1000,
        0
        //this->GetHeaderValueAsFloat( "tr" ),
        //this->GetHeaderValueAsFloat("fliplist", 0)
    ); 
}


/*!
 *
 */
void svkBrukerRawMRSMapper::InitMRSpectroscopyFOVGeometryMacro()
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
        this->GetHeaderValueAsInt("PVM_DigNp"), 
        "SharedFunctionalGroupsSequence",
        0
    );


    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionPhaseColumns",
        this->GetHeaderValueAsInt("ACQ_spatial_size_0"),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionPhaseRows",
        this->GetHeaderValueAsInt("ACQ_spatial_size_1"),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionOutOfPlanePhaseSteps",
        1,
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

    //pixelSpacing: 
    float pixelSize[3]; 
    this->GetBrukerPixelSize( pixelSize ); 

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionPixelSpacing",
        svkTypeUtils::DoubleToString(pixelSize[0]) + '\\' + svkTypeUtils::DoubleToString(pixelSize[1]) , 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionSliceThickness",
        this->GetHeaderValueAsFloat("PVM_SliceThick", 0),
        "SharedFunctionalGroupsSequence",
        0
    );

    string orientationString; 
    float dcos[3][3]; 
    this->GetDcmOrientation(dcos, &orientationString); 

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionOrientation",
        orientationString, 
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
        this->GetHeaderValueAsInt("ACQ_spatial_size_0"),
        "SharedFunctionalGroupsSequence",
        0
    );
    
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                               
        "SVK_SpectroscopyAcqReorderedPhaseRows",
        this->GetHeaderValueAsInt("ACQ_spatial_size_1"),
        "SharedFunctionalGroupsSequence",
        0
    );
    
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                               
        "SVK_SpectroscopyAcqReorderedOutOfPlanePhaseSteps",
        1,
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
        svkTypeUtils::DoubleToString(pixelSize[0]) + '\\' + svkTypeUtils::DoubleToString(pixelSize[1]), 
        "SharedFunctionalGroupsSequence",
        0
    );
    
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,                                      
        "SVK_SpectroscopyAcqReorderedSliceThickness",
        this->GetHeaderValueAsFloat("PVM_SliceThick", 0),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedOrientation",
        orientationString, 
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
void svkBrukerRawMRSMapper::InitMREchoMacro()
{

    this->dcmHeader->InitMREchoMacro( 1 * 1000.); 
}


/*!
 *  Override in concrete mapper for specific acquisitino
 */
void svkBrukerRawMRSMapper::InitMRModifierMacro()
{
    float inversionTime = 0; 
    this->dcmHeader->InitMRModifierMacro( inversionTime );
}


/*!
 *
 */
void svkBrukerRawMRSMapper::InitMRTransmitCoilMacro()
{
    this->dcmHeader->InitMRTransmitCoilMacro("Bruker", "UNKNOWN", "BODY");
}


/*! 
 *  Receive Coil:
 */
void svkBrukerRawMRSMapper::InitMRReceiveCoilMacro()
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
        "Bruker Coil",
        "SharedFunctionalGroupsSequence",
        0
    );

}


/*!
 *
 */
void svkBrukerRawMRSMapper::InitMRAveragesMacro()
{
    int numAverages = 1; 
    this->dcmHeader->InitMRAveragesMacro(numAverages);
}


/*!
 *
 */
void svkBrukerRawMRSMapper::InitMultiFrameDimensionModule()
{
    int indexCount = 0; 
    this->dcmHeader->AddSequenceItemElement(
        "DimensionIndexSequence",
        indexCount,
        "DimensionDescriptionLabel",
        "Slice"
    );


    if (this->GetNumTimePoints() > 1) {
        indexCount++; 
        this->dcmHeader->AddSequenceItemElement(
            "DimensionIndexSequence",
            indexCount,
            "DimensionDescriptionLabel",
            "Time Point"
        );
    }


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
void svkBrukerRawMRSMapper::InitAcquisitionContextModule()
{
}


/*!
 *
 */
void svkBrukerRawMRSMapper::InitMRSpectroscopyModule()
{

    /*  =======================================
     *  MR Image and Spectroscopy Instance Macro
     *  ======================================= */
    string timeDate = this->GetHeaderValueAsString( "SUBJECT_date" );

    this->dcmHeader->SetValue(
        "AcquisitionDateTime",
        timeDate
    );
    
    this->dcmHeader->SetValue(
        "AcquisitionDuration",
        0
    );


    string nucleus = this->GetHeaderValueAsString("NUC1"); 
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


    //  The field looks like this, so parse out the numeric field value: 
    //  BioSpec 3T
    string field = this->GetHeaderValueAsString("ACQ_station", 0); 
    size_t pos = field.find(" ");  
    if ( pos != string::npos ) {
        string sub1 = field.substr(  pos + 1); 
        size_t posT = sub1.find("T"); 
        if ( posT != string::npos ) {
            field = sub1.substr(0, posT); 
        }
    }
    this->dcmHeader->SetValue(
        "MagneticFieldStrength",
        field
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
        this->GetHeaderValueAsFloat( "BF1", 0 )
    );

    this->dcmHeader->SetValue(
        "SpectralWidth",
        this->GetHeaderValueAsFloat( "PVM_SpecSWH", 0 )
    );

    this->dcmHeader->SetValue(
        "SVK_FrequencyOffset",
        this->GetHeaderValueAsFloat("PVM_SpecOffsetHz", 0)
    );

    this->dcmHeader->SetValue(
        "ChemicalShiftReference",
        this->GetHeaderValueAsFloat("PVM_FrqWorkPpm", 0)
    );

    this->dcmHeader->SetValue(
        "VolumeLocalizationTechnique",
        ""
    );


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
void svkBrukerRawMRSMapper::InitMRSpectroscopyDataModule()
{
    this->dcmHeader->SetValue( "Columns", this->GetHeaderValueAsInt("ACQ_spatial_size_0") );
    this->dcmHeader->SetValue( "Rows", this->GetHeaderValueAsInt("ACQ_spatial_size_1") );
    this->dcmHeader->SetValue( "DataPointRows", 0 );
    this->dcmHeader->SetValue( "DataPointColumns", this->GetHeaderValueAsInt("PVM_DigNp", 0) );
    this->dcmHeader->SetValue( "DataRepresentation", "COMPLEX" );
    this->dcmHeader->SetValue( "SignalDomainColumns", "TIME" );
    this->dcmHeader->SetValue( "SVK_ColumnsDomain", "KSPACE" );
    this->dcmHeader->SetValue( "SVK_RowsDomain", "KSPACE" );
    this->dcmHeader->SetValue( "SVK_SliceDomain", "KSPACE" );
}


/*!
 *  Reads spec data from Bruker raw file (e.g. "ser" or "fid").
 */
void svkBrukerRawMRSMapper::ReadSerFile( string serFileName, svkMrsImageData* data )
{
    
    vtkDebugMacro( << this->GetClassName() << "::ReadSerFile()" );

    ifstream* serDataIn = new ifstream();
    serDataIn->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

    string rawFormat = this->GetHeaderValueAsString("GO_raw_data_format"); 
    int pixelWordSize = 4;
    if ( rawFormat.find("32_BIT") != string::npos ) {
        pixelWordSize = 4;
    }

    int numComponents = 2;
    int numSpecPoints = this->dcmHeader->GetIntValue( "DataPointColumns" );

    int numVoxels[3]; 
    numVoxels[0] = this->dcmHeader->GetIntValue( "Columns" ); 
    numVoxels[1] = this->dcmHeader->GetIntValue( "Rows" ); 
    numVoxels[2] = this->dcmHeader->GetNumberOfSlices( ); 

    int numPixInVolume = numVoxels[0] * numVoxels[1] * numVoxels[2]; 

    int numBytesInVol = ( numPixInVolume * pixelWordSize * numComponents * numSpecPoints );

    serDataIn->open( serFileName.c_str(), ios::binary );

    /*
     *   Flatten the data volume into one dimension
     */
    if (this->specData == NULL) {
        this->specData = new char[ numBytesInVol ];
    }

    serDataIn->seekg(0, ios::beg);
    serDataIn->read( (char *)(this->specData), numBytesInVol);

    /*
     *  FID files are bigendian.
     */
    if ( this->swapBytes ) {
        vtkByteSwap::SwapVoidRange((void *)this->specData, numBytesInVol/pixelWordSize, pixelWordSize);
    }

    svkDcmHeader* hdr = this->dcmHeader;

    double progress = 0;

    string encoding = this->GetHeaderValueAsString("PVM_EncOrder");
    if ( encoding.find("CENTRIC_ENC") != string::npos ) { 
        encoding = "CENTRIC"; 
    } else if ( encoding.find("LINEAR_ENC") != string::npos  ) { 
        encoding = "LINEAR"; 
    }  else {
        //  make this the default.  Not sure if would be specified for all sequences. 
        encoding = "LINEAR"; 
    }
             
    int numTimePts = hdr->GetNumberOfTimePoints(); 
    int numCoils = hdr->GetNumberOfCoils(); 
    for (int coilNum = 0; coilNum < numCoils; coilNum++) {
        for (int timePt = 0; timePt < numTimePts; timePt++) {

            for (int z = 0; z < numVoxels[2] ; z++) {
                for (int y = 0; y < numVoxels[1]; y++) {
                    for (int x = 0; x < numVoxels[0]; x++) {
                        SetCellSpectrum(data, x, y, z, timePt, coilNum);
                    }
                }
            } 
            if( timePt % 2 == 0 ) { // only update every other index
                progress = (timePt * coilNum)/((double)(numTimePts*numCoils));
                this->InvokeEvent(vtkCommand::ProgressEvent, static_cast<void *>(&progress));
            }
        }
    }

    if (encoding.compare("CENTRIC") == 0 ) {
        this->ReorderKSpace( data ); 
    }

    //cout << *data << endl; 
    this->ApplyGroupDelay( data ); 

    progress = 1;
	this->InvokeEvent(vtkCommand::ProgressEvent,static_cast<void *>(&progress));

    serDataIn->close();
    delete serDataIn;

}


/*!
 *  Determine number of time points in the fid or ser file. 
 */
int svkBrukerRawMRSMapper::GetNumTimePoints()
{
    return this->GetHeaderValueAsInt("NR");  
}


/*!
 *  Apply group delay shift in FID if necessary. Was required originally, but Bruker appears to 
 *  have fixed this so the FIDs start at the correct time point now. 
 */
void svkBrukerRawMRSMapper::ApplyGroupDelay( svkMrsImageData* data )
{

    // not required anymore
    return; 

    //  Bruker FIDs are shifted by a group delay number of points defined by PVM_DigShiftDbl.  
    //  Apply this global shift to correct the data here: 
    svkFreqCorrect* freqShift = svkFreqCorrect::New(); 
    freqShift->SetInputData( data ); 
    freqShift->SetCircularShift(); 
    freqShift->SetGlobalFrequencyShift( 
        -1 * this->GetHeaderValueAsInt("PVM_DigShiftDbl") 
    ); 
    freqShift->Update(); 
    freqShift->Delete(); 
}


/*!
 *  Reorder centric (or reverse centric) k-space data to standard linear encoding
 */
void svkBrukerRawMRSMapper::ReorderKSpace( svkMrsImageData* data )
{
        
    svkMrsImageData* tmpData = svkMrsImageData::New();
    tmpData->DeepCopy( data );

    int numVoxels[3]; 
    numVoxels[0] = this->dcmHeader->GetIntValue( "Columns" ); 
    numVoxels[1] = this->dcmHeader->GetIntValue( "Rows" ); 
    numVoxels[2] = this->dcmHeader->GetNumberOfSlices( ); 

    int numPts   = this->dcmHeader->GetIntValue( "DataPointColumns" );

    svkDcmHeader::DimensionVector dimVector    = data->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector loopVector   = dimVector; 
    svkDcmHeader::DimensionVector targetVector = dimVector; 

    int numTimePts = this->GetNumTimePoints();
    int coilNum = 0; 

    int signX= -1; 
    int signY= -1; 
    int signZ= -1; 
    int zc = numVoxels[2]/2; 

    int numCells = svkDcmHeader::GetNumberOfCells( &dimVector );
    int cellID = numCells - 1;     
    float specTuple[2];

    //for ( int time = 0; time < numTimePts; time++ ) {
        for ( int z = 0; z < numVoxels[2] ; z++ ) {
            signZ *= -1; 
            int yc = numVoxels[1]/2; 
            zc =  zc + (z * signZ); 
            for ( int y = 0; y < numVoxels[1]; y++ ) {
                signY *= -1; 
                int xc = numVoxels[0]/2; 
                yc =  yc + (y * signY); 
                for ( int x = 0; x < numVoxels[0]; x++ ) {

                    signX *= -1; 
                    xc =  xc + (x * signX); 

                    //  Get dim vector for location where the data should go (reordered to standard linear encoding) 
                    svkDcmHeader::SetDimensionVectorValue(&targetVector, svkDcmHeader::COL_INDEX,   xc); 
                    svkDcmHeader::SetDimensionVectorValue(&targetVector, svkDcmHeader::ROW_INDEX,   yc); 
                    svkDcmHeader::SetDimensionVectorValue(&targetVector, svkDcmHeader::SLICE_INDEX, zc); 
                    //svkDcmHeader::SetDimensionVectorValue(&targetVector, svkDcmHeader::TIME_INDEX,  time); 
                    int targetCellID = svkDcmHeader::GetCellIDFromDimensionVectorIndex( &dimVector, &targetVector );

                    //  order of mapping from input cell ordering, to output cell ordering
                    //cout << "CELL: " << cellID << " -> " << targetCellID << " x: " << xc << " y: " << yc << " z: " << zc << endl;

                    //  get the loopVector spectrum and write the contents into the targetVector spectrum
                    vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast(
                        svkMrsImageData::SafeDownCast(data)->GetSpectrum( cellID)
                    );
                    vtkFloatArray* targetSpectrum = vtkFloatArray::SafeDownCast(
                        svkMrsImageData::SafeDownCast(tmpData)->GetSpectrum( targetCellID )
                    );

                    for ( int freq = 0; freq < numPts; freq++ ) {
                        spectrum->GetTuple( freq, specTuple);
                        //cout << "ST: " << specTuple[0] << endl;
                        targetSpectrum->InsertTuple(freq, specTuple);
                    }

                    cellID--; 
                }
            }
        }
    //}
    data->DeepCopy( tmpData );
}


/*!
 *
 */
void svkBrukerRawMRSMapper::SetCellSpectrum(vtkImageData* data, int x, int y, int z, int timePt, int coilNum)
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

    x = numVoxels[0] - x - 1; 
    y = numVoxels[1] - y - 1; 

    int offset = (numPts * numComponents) *  (
                     ( numVoxels[0] * numVoxels[1] * numVoxels[2] ) * timePt
                    +( numVoxels[0] * numVoxels[1] ) * z
                    +  numVoxels[0] * y
                    +  x
                 );


    string rawFormat = this->GetHeaderValueAsString("GO_raw_data_format"); 
    if ( rawFormat.find("SGN_INT") != string::npos ) {
        for (int i = 0; i < numPts; i++) {
            int intValRe     = static_cast<int*>(  this->specData)[offset + (i * 2)];
            int intValIm     = static_cast<int*>(  this->specData)[offset + (i * 2 + 1)];
            float floatVal[2]; 
            floatVal[0] = intValRe; 
            floatVal[1] = -1 * intValIm; 
            //  Bruker data points are in reverse order: 
            //  chop in freq space?  
            dataArray->SetTuple( i,  floatVal ); 
        }
    } else { 
        for (int i = 0; i < numPts; i++) {
            dataArray->SetTuple( i, &(static_cast<float*>(this->specData)[offset + (i * 2)]));
        }
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
string svkBrukerRawMRSMapper::GetDcmPatientPositionString()
{
    string dcmPatientPosition;

    string position1 = this->GetHeaderValueAsString("SUBJECT_entry");
    if( position1.find("HeadFirst") != string::npos ) {
        dcmPatientPosition.assign("HF");
    } else if( position1.find("FeetFirst") != string::npos ) {
        dcmPatientPosition.assign("FF");
    } else {
        dcmPatientPosition.assign("UNKNOWN");
    }

    string position2 = this->GetHeaderValueAsString("ACQ_patient_pos");
    if( position2.find("Supine") != string::npos ) {
        dcmPatientPosition += "S";
    } else if( position2.find("Prone") != string::npos ) {
        dcmPatientPosition += "P";
    } else if( position2.find("Decubitus left") != string::npos ) {
        dcmPatientPosition += "DL";
    } else if( position2.find("Decubitus right") != string::npos ) {
        dcmPatientPosition += "DR";
    } else {
        dcmPatientPosition += "UNKNOWN";
    }

    return dcmPatientPosition;
}


/*!
 *
 */
int svkBrukerRawMRSMapper::GetHeaderValueAsInt(string keyString, int valueIndex )
{

    istringstream* iss = new istringstream();
    int value;

    iss->str( (this->paramMap[keyString])[valueIndex]);
    *iss >> value;

    delete iss; 

    return value;
}


/*!
 *
 */
float svkBrukerRawMRSMapper::GetHeaderValueAsFloat(string keyString, int valueIndex )
{

    istringstream* iss = new istringstream();
    float value;
    iss->str( (this->paramMap[keyString])[valueIndex]);
    *iss >> value;

    delete iss; 

    return value;
}


/*!
 *
 */
string svkBrukerRawMRSMapper::GetHeaderValueAsString(string keyString, int valueIndex )
{

    map< string, vector < string > >::iterator it;
    it = this->paramMap.find(keyString);
    if ( it != this->paramMap.end() ) {
        return (this->paramMap[keyString])[valueIndex];
    } else {
        return ""; 
    }
}



/*!
 *  Pixel Spacing:
 */
void svkBrukerRawMRSMapper::InitPixelMeasuresMacro()
{

    float pixelSize[3];
    this->GetBrukerPixelSize( pixelSize ); 

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


void svkBrukerRawMRSMapper::InitMRSpectroscopyPulseSequenceModule() 
{
    //  I would have thought this would be no for linear and an even number of 
    //  phase encodes, but I thought the centric encoding would have sampled k0
    //  assymetrically, however both trajectories yield best spatial overlap with 
    //  reference image when this is set to NO. 
    this->dcmHeader->SetValue( "SVK_K0Sampled", "NO");
} 


/*!
 *
 */
void svkBrukerRawMRSMapper::GetBrukerPixelSize( float pixelSize[3] )
{
    float numPixels[3];
    numPixels[0] = this->GetHeaderValueAsInt("ACQ_spatial_size_0");
    numPixels[1] = this->GetHeaderValueAsInt("ACQ_spatial_size_1");
    numPixels[2] = 1;

    string fovStr = this->GetHeaderValueAsString("PVM_Fov", 0); 
    char* cstr = new char [fovStr.length()+1];
    strcpy( cstr, fovStr.c_str() );

    for ( int i = 0; i < 3; i++ ) {    
        pixelSize[i] = 0; 
    }

    char*  pch;
    pch = strtok (cstr," ");
    int i = 0; 
    float fov; 
    while (pch != NULL)
    {
        fov = svkTypeUtils::StringToFloat ( string(pch) ); 
        pixelSize[i] = fov / numPixels[i]; 
        pch = strtok (NULL, " ");
        i++; 
    }
    //  2D so should require gettting the 3rd dimension fromthe slice thickness
    if ( i == 2 ) {
         pixelSize[i] = this->GetHeaderValueAsFloat("ACQ_slice_thick");
    }
    //for ( int i = 0; i < 3; i++ ) {    
        //cout << "PS: " << pixelSize[i] << endl;; 
    //}

}
