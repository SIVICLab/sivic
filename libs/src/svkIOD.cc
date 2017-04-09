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



#include <svkIOD.h>


using namespace svk;


//vtkCxxRevisionMacro(svkIOD, "$Rev$");

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
    this->dcmHeader->InsertEmptyElement( "PatientName" );
    this->dcmHeader->InsertEmptyElement( "PatientID" );
    this->dcmHeader->InsertEmptyElement( "PatientBirthDate" );
    this->dcmHeader->InsertEmptyElement( "PatientSex" );
}


/*!
 *
 */
void svkIOD::InitGeneralStudyModule()
{
    this->dcmHeader->InsertUniqueUID( "StudyInstanceUID" );
    this->dcmHeader->InsertEmptyElement( "StudyDate" );
    this->dcmHeader->InsertEmptyElement( "StudyTime" );
    this->dcmHeader->InsertEmptyElement( "ReferringPhysicianName" );
    this->dcmHeader->InsertEmptyElement( "StudyID" );
    this->dcmHeader->InsertEmptyElement( "AccessionNumber" );
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
    this->dcmHeader->SetValue( "ManufacturerModelName",svkIOD::NA_STRING );
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
 *  Defaults to a single frame Dimension representing the slice 
 */
void svkIOD::InitMultiFrameDimensionModule()
{
    this->dcmHeader->InsertEmptyElement( "DimensionOrganizationSequence" );
    this->dcmHeader->InsertEmptyElement( "DimensionIndexSequence" );
    //  Every object exists in 3D and therefore has a slice index 
    this->dcmHeader->SetValue( "Rows", 1 );
    this->dcmHeader->SetValue( "Columns", 1 );
    svkDcmHeader::DimensionVector dimensionVector = this->dcmHeader->GetDimensionIndexVector();
    this->dcmHeader->AddDimensionIndex(&dimensionVector, svkDcmHeader::SLICE_INDEX);  
    this->dcmHeader->InitMultiFrameDimensionModule( &dimensionVector ); 
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
 *  SNOMED Coding:
 *      value   scheme  meaning
 *      T-A0100 SNM3    Brain
 *      T-9200B SNM3    Prostate
 */
void svkIOD::InitFrameAnatomyMacro()
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
        std::string("U"),
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

