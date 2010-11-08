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



#include <svkIOD.h>


using namespace svk;


vtkCxxRevisionMacro(svkIOD, "$Rev$");

//  
const string svkIOD::NA_STRING = "NA";
const string svkIOD::NA_DATE_STRING = "00000000";
const string svkIOD::NA_TIME_STRING = "0000";


/*!
 *
 */
svkIOD::svkIOD()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->dcmHeader = NULL; 
}


/*!
 *
 */
svkIOD::~svkIOD()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}


/*
 *
 */
void svkIOD::SetDcmHeader(svkDcmHeader* header)
{
    this->dcmHeader = header; 
}


/*!
 *
 */
void svkIOD::InitPatientModule()
{
    this->dcmHeader->InsertEmptyElement( "PatientsName" );
    this->dcmHeader->InsertEmptyElement( "PatientID" );
    this->dcmHeader->InsertEmptyElement( "PatientsBirthDate" );
    this->dcmHeader->InsertEmptyElement( "PatientsSex" );
}


/*!
 *
 */
void svkIOD::InitPatientModule(vtkstd::string patientsName, vtkstd::string patientID, vtkstd::string patientsBirthDate, vtkstd::string patientsSex)
{
    if ( !patientsName.empty() ) {
        this->dcmHeader->SetValue(
            "PatientsName",
            patientsName 
        );
    }

    if ( !patientID.empty() ) {
        this->dcmHeader->SetValue(
            "PatientID",
            patientID
        );
    }
    if ( !patientsBirthDate.empty() ) {
        this->dcmHeader->SetValue(
            "PatientsBirthDate",
            patientsBirthDate
        );
    }
    if ( !patientsSex.empty() ) {
        this->dcmHeader->SetValue(
            "PatientsSex",
            patientsSex
        );
    }
}


/*!
 *
 */
void svkIOD::InitGeneralStudyModule()
{
    this->dcmHeader->InsertUniqueUID( "StudyInstanceUID" );
    this->dcmHeader->InsertEmptyElement( "StudyDate" );
    this->dcmHeader->InsertEmptyElement( "StudyTime" );
    this->dcmHeader->InsertEmptyElement( "ReferringPhysiciansName" );
    this->dcmHeader->InsertEmptyElement( "StudyID" );
    this->dcmHeader->InsertEmptyElement( "AccessionNumber" );
}


/*!
 *  Initialize the General Study Module (Study IE):
 */
void svkIOD::InitGeneralStudyModule(vtkstd::string studyDate, vtkstd::string studyTime, vtkstd::string referringPhysiciansName, vtkstd::string studyID, vtkstd::string accessionNumber)
{

    if ( !studyDate.empty() ) {
        this->dcmHeader->SetValue(
            "StudyDate",
            studyDate
        );
    }

    if ( !studyTime.empty() ) {
        this->dcmHeader->SetValue(
            "StudyTime",
            studyTime
        );
    }

    if ( !referringPhysiciansName.empty() ) {
        this->dcmHeader->SetValue(
            "ReferringPhysiciansName",
            referringPhysiciansName
        );
    }

    if ( !studyID.empty() ) {
        this->dcmHeader->SetValue(
            "StudyID",
            studyID 
        );
    }

    if ( !accessionNumber.empty() ) {
        this->dcmHeader->SetValue(
            "AccessionNumber",
            accessionNumber 
        );
    }
}


/*!
 *
 */
void svkIOD::InitGeneralSeriesModule()
{
    this->dcmHeader->SetValue(
        "Modality",
        GetModality()
    );
    this->dcmHeader->InsertUniqueUID( "SeriesInstanceUID" );
    this->dcmHeader->InsertEmptyElement( "SeriesNumber" );
    if (this->GetModality() == "MR") {
        this->dcmHeader->InsertEmptyElement( "PatientPosition" );
    }
}


/*!
 *  Initialize the General Series Module (Series IE):
 *  Modality is set on initialztion of IOD type   
 */
void svkIOD::InitGeneralSeriesModule(vtkstd::string seriesNumber, vtkstd::string seriesDescription, vtkstd::string patientPosition)
{

    if ( !seriesNumber.empty() ) {
        this->dcmHeader->SetValue(
            "SeriesNumber",
            seriesNumber
        );
    }

    if ( !seriesDescription.empty() ) {
        this->dcmHeader->SetValue(
            "SeriesDescription",
            seriesDescription
        );
    }

    if ( !patientPosition.empty() ) {
        this->dcmHeader->SetValue(
            "PatientPosition",
            patientPosition 
        );
    }
}


/*!
 *
 */
void svkIOD::InitMRSeriesModule()
{
    this->dcmHeader->SetValue( "Modality", this->GetModality() );
}


/*!
 *
 */
void svkIOD::InitFrameOfReferenceModule()
{
    this->dcmHeader->InsertUniqueUID( "FrameOfReferenceUID" );
    this->dcmHeader->InsertEmptyElement( "PositionReferenceIndicator" );
}


/*!
 *
 */
void svkIOD::InitGeneralEquipmentModule()
{
    this->dcmHeader->SetValue( "Manufacturer", this->GetManufacturer() );
}


/*!
 *
 */
void svkIOD::InitEnhancedGeneralEquipmentModule()
{
    this->dcmHeader->SetValue( "Manufacturer", svkIOD::NA_STRING );
    this->dcmHeader->SetValue( "ManufacturersModelName",svkIOD::NA_STRING );
    this->dcmHeader->SetValue( "DeviceSerialNumber",svkIOD::NA_STRING );
    this->dcmHeader->SetValue( "SoftwareVersions",  svkIOD::NA_STRING );
}


/*!
 *
 */
void svkIOD::InitMultiFrameFunctionalGroupsModule()
{
    this->dcmHeader->InsertEmptyElement( "SharedFunctionalGroupsSequence" );

    //  This sequence contains the same number of items as frames, so contains
    //  no items at default initialization.
    this->dcmHeader->InsertEmptyElement( "PerFrameFunctionalGroupsSequence" );

    this->dcmHeader->SetValue( "InstanceNumber",  1);
    this->dcmHeader->SetValue( "ContentDate",  svkIOD::NA_DATE_STRING );
    this->dcmHeader->SetValue( "ContentTime",  svkIOD::NA_TIME_STRING );
    this->dcmHeader->SetValue( "NumberOfFrames", 0 );
}


/*!
 *
 */
void svkIOD::InitPerFrameFunctionalGroupSequence(double toplc[3], double voxelSpacing[3],
                                             double dcos[3][3], int numSlices, int numTimePts, int numCoils)
{

    this->dcmHeader->ClearSequence( "PerFrameFunctionalGroupsSequence" );
    this->dcmHeader->SetValue( "NumberOfFrames", numSlices * numTimePts * numCoils );

    //this->dcmHeader->InitMultiFrameDimensionModule( numSlices, numTimePts, numCoils );
    //this->dcmHeader->InitFrameContentMacro( numSlices, numTimePts, numCoils );
    //this->dcmHeader->InitPlanePositionMacro( toplc, voxelSpacing, dcos, numSlices, numTimePts, numCoils);

}


/*!
 *
 */
void svkIOD::InitMultiFrameDimensionModule()
{
    this->dcmHeader->InsertEmptyElement( "DimensionOrganizationSequence" );
    this->dcmHeader->InsertEmptyElement( "DimensionIndexSequence" );
}


/*!
 *
 */
void svkIOD::InitPlaneOrientationMacro( vtkstd::string orientationString )
{

    this->dcmHeader->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PlaneOrientationSequence"
    );


    this->dcmHeader->AddSequenceItemElement(
        "PlaneOrientationSequence",
        0,
        "ImageOrientationPatient",
        orientationString,
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *  Pixel Spacing:
 */
void svkIOD::InitPixelMeasuresMacro( vtkstd::string pixelSizes, vtkstd::string sliceThickness )
{

    this->dcmHeader->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PixelMeasuresSequence"
    );


    this->dcmHeader->AddSequenceItemElement(
        "PixelMeasuresSequence",
        0,
        "PixelSpacing",
        pixelSizes, 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "PixelMeasuresSequence",
        0,
        "SliceThickness",
        sliceThickness, 
        "SharedFunctionalGroupsSequence",
        0
    );
}



/*!
 *
 */
void svkIOD::InitAcquisitionContextModule()
{
    this->dcmHeader->InsertEmptyElement( "AcquisitionContextSequence" );
}


/*!
 *
 */
void svkIOD::InitGeneralImageModule()
{
    this->dcmHeader->InsertEmptyElement( "InstanceNumber" );
    this->dcmHeader->InsertEmptyElement( "PatientOrientation" );
}


/*!
 *
 */
void svkIOD::InitImagePixelModule()
{
    this->dcmHeader->SetValue( "SamplesPerPixel", 1 );
    this->dcmHeader->SetValue( "PhotometricInterpretation", "MONOCHROME2" );
    this->dcmHeader->SetValue( "Rows", 0 );
    this->dcmHeader->SetValue( "Columns", 0 );
    this->dcmHeader->SetValue( "BitsAllocated", 0 );
    this->dcmHeader->SetValue( "BitsStored", 0 );
    this->dcmHeader->SetValue( "HighBit", 0 );
    this->dcmHeader->InsertEmptyElement( "PixelRepresentation" );
    this->dcmHeader->InsertEmptyElement( "PixelData" );
}


/*!
 *
 */
void svkIOD::InitImagePixelModule( int rows, int columns, svkDcmHeader::DcmPixelDataFormat dataType)
{
    this->dcmHeader->SetValue( "Rows", rows );
    this->dcmHeader->SetValue( "Columns", columns );
    this->dcmHeader->SetPixelDataType( dataType );
}


/*!
 *
 */
string svkIOD::GetManufacturer()
{
    return svkIOD::NA_STRING; 
}


/*!
 *
 */
string svkIOD::GetModality()
{
    return "MR";
}


/*!
 *
 */
void svkIOD::SetReplaceOldElements(bool replaceElements)
{
    if ( this->dcmHeader != NULL ) {
        this->dcmHeader->ReplaceOldElements( replaceElements );
    } else {
        vtkWarningWithObjectMacro(this, "SetReplaceOldElements: dcmHeader Not set yet ");   
    } 
}

