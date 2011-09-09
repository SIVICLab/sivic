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


#include <svkDcmMriVolumeReader.h>
#include <svkMriImageData.h>
//#include <vtkByteSwap.h>
#include <vtkType.h>
#include <vtkDebugLeaks.h>
#include <vtkStringArray.h>
#include <vtkInformation.h>
#include <vtkMath.h>
#include <svkIOD.h>
#include <svkEnhancedMRIIOD.h>
#include <vtkstd/vector>
#include <vtkstd/utility>
#include <vtkstd/algorithm>
#include <sstream>


using namespace svk;


vtkCxxRevisionMacro(svkDcmMriVolumeReader, "$Rev$");
vtkStandardNewMacro(svkDcmMriVolumeReader);


/*!
 *
 */
svkDcmMriVolumeReader::svkDcmMriVolumeReader()
{
#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkDcmMriVolumeReader");
#endif
    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");
}


/*!
 *
 */
svkDcmMriVolumeReader::~svkDcmMriVolumeReader()
{
    vtkDebugMacro(<<this->GetClassName() << "::~" << this->GetClassName() << "()");
}


/*!
 *  Mandator, must be overridden to get the factory to check for proper
 *  type of vtkImageReader2 to return. 
 */
int svkDcmMriVolumeReader::CanReadFile(const char* fname)
{

    vtkstd::string fileToCheck(fname);

    bool isDcmMri = false; 

    if ( svkDcmHeader::IsFileDICOM( fname ) ) {

        svkImageData* tmp = svkMriImageData::New(); 

        tmp->GetDcmHeader()->ReadDcmFile( fname );
        vtkstd::string SOPClassUID = tmp->GetDcmHeader()->GetStringValue( "SOPClassUID" ) ;

        //verify that this isn't a proprietary use of DICOM MR ImageStorage: 
        if ( this->ContainsProprietaryContent( tmp ) == false ) {
                    
            // Check for MR Image Storage (and for now CTImageStorage too, see SIVIC tickets in trac)
            if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.4" || SOPClassUID == "1.2.840.10008.5.1.4.1.1.2") {           
                this->SetFileName(fname);
                isDcmMri = true; 
            }
        
        }

        tmp->Delete(); 
    }

    if ( isDcmMri ) {
        cout << this->GetClassName() << "::CanReadFile(): It's a DICOM MRI File: " <<  fileToCheck << endl;
        return 1;
    }

    vtkDebugMacro(<<this->GetClassName() << "::CanReadFile() It's Not a DICOM MRI file " << fileToCheck );

    return 0;
}


/*!
 *
 */
void svkDcmMriVolumeReader::InitDcmHeader()
{

    this->InitFileNames(); 

    // Read the first file and load the header as the starting point
    this->GetOutput()->GetDcmHeader()->ReadDcmFile( this->GetFileNames()->GetValue(0) );

    vtkstd::string studyInstanceUID( this->GetOutput()->GetDcmHeader()->GetStringValue("StudyInstanceUID"));
    int rows    = this->GetOutput()->GetDcmHeader()->GetIntValue( "Rows" ); // Y
    int columns = this->GetOutput()->GetDcmHeader()->GetIntValue( "Columns" ); // X
     
    //  Now override elements with Multi-Frame sequences and default details:
    svkIOD* iod = svkEnhancedMRIIOD::New();
    iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    iod->SetReplaceOldElements(false); 
    iod->InitDcmHeader();
    iod->Delete();


    this->GetOutput()->GetDcmHeader()->SetValue( "StudyInstanceUID", studyInstanceUID.c_str() );


    //  Now move info from original MRImageStorage header elements to flesh out enhanced
    //  SOP class elements (often this is just a matter of copying elements from the top 
    //  level to a sequence item. 
    this->InitMultiFrameFunctionalGroupsModule(); 

    /*
     *  odds and ends: 
     */
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "Rows", 
        rows 
    );
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "Columns", 
        columns 
    );

    if (this->GetDebug()) {
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader(); 
    }
    
}


/*!
 *  Initializes any private DICOM attributes that are needed internally
 */
void svkDcmMriVolumeReader::InitPrivateHeader()
{
}


/*! 
 *  Init the Shared and PerFrame sequences and load the pixel data into the svkImageData object. 
 */
void svkDcmMriVolumeReader::LoadData( svkImageData* data )
{
    void *imageData = data->GetScalarPointer();

    int rows    = this->GetOutput()->GetDcmHeader()->GetIntValue( "Rows" ); 
    int columns = this->GetOutput()->GetDcmHeader()->GetIntValue( "Columns" ); 
    int numPixelsInSlice = rows * columns; 

    /*
     *  Iterate over slices (frames) and copy ImagePositions
     */
    for (int i = 0; i < this->GetFileNames()->GetNumberOfValues(); i++) { 
        svkImageData* tmpImage = svkMriImageData::New(); 
        tmpImage->GetDcmHeader()->ReadDcmFile( this->GetFileNames()->GetValue( i ) ); 
        tmpImage->GetDcmHeader()->GetShortValue( "PixelData", ((short *)imageData) + (i * numPixelsInSlice), numPixelsInSlice );
        tmpImage->Delete(); 
    }

}


/*!
 *
 */
void svkDcmMriVolumeReader::InitMultiFrameFunctionalGroupsModule()
{

    this->numFrames =  this->GetFileNames()->GetNumberOfValues(); 

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
void svkDcmMriVolumeReader::InitSharedFunctionalGroupMacros()
{
    this->InitPixelMeasuresMacro();
    this->InitPlaneOrientationMacro();
    this->InitMRReceiveCoilMacro();
}


/*!
 *  Pixel Spacing:
 */
void svkDcmMriVolumeReader::InitPixelMeasuresMacro()
{

    svkImageData* tmp = svkMriImageData::New(); 
    tmp->GetDcmHeader()->ReadDcmFile(  this->GetFileNames()->GetValue( 0 ) ); 

    this->GetOutput()->GetDcmHeader()->InitPixelMeasuresMacro(
        tmp->GetDcmHeader()->GetStringValue( "PixelSpacing" ), 
        tmp->GetDcmHeader()->GetStringValue( "SliceThickness" ) 
    );

    tmp->Delete(); 
}


/*!
 *
 */
void svkDcmMriVolumeReader::InitPlaneOrientationMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PlaneOrientationSequence"
    );

    svkImageData* tmp = svkMriImageData::New(); 
    tmp->GetDcmHeader()->ReadDcmFile(  this->GetFileNames()->GetValue( 0 ) ); 

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PlaneOrientationSequence",
        0,
        "ImageOrientationPatient",
        tmp->GetDcmHeader()->GetStringValue( "ImageOrientationPatient" ), 
        "SharedFunctionalGroupsSequence",
        0
    );

    tmp->Delete(); 

}


/*!
 *  Receive Coil:
 */
void svkDcmMriVolumeReader::InitMRReceiveCoilMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRReceiveCoilSequence"
    );

    svkImageData* tmp = svkMriImageData::New(); 
    tmp->GetDcmHeader()->ReadDcmFile(  this->GetFileNames()->GetValue( 0 ) ); 

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "ReceiveCoilName",
        tmp->GetDcmHeader()->GetStringValue( "ReceiveCoilName" ), 
        "SharedFunctionalGroupsSequence",
        0
    );

    tmp->Delete(); 
}


/*!
 *
 */
void svkDcmMriVolumeReader::InitPerFrameFunctionalGroupMacros()
{
    this->InitFrameContentMacro();
    this->InitPlanePositionMacro();
}


/*!
 *  Mandatory, Must be a per-frame functional group
 */
void svkDcmMriVolumeReader::InitFrameContentMacro()
{
    for (int i = 0; i < this->numFrames; i++) {

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "PerFrameFunctionalGroupsSequence",
            i,
            "FrameContentSequence"
        );

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "FrameContentSequence",
            0,
            "FrameAcquisitionDatetime",
            "EMPTY_ELEMENT",
            "PerFrameFunctionalGroupsSequence",
            i
        );

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "FrameContentSequence",
            0,
            "FrameReferenceDatetime",
            "EMPTY_ELEMENT",
            "PerFrameFunctionalGroupsSequence",
            i
        );

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "FrameContentSequence",
            0,
            "FrameAcquisitionDuration",
            "-1",
            "PerFrameFunctionalGroupsSequence",
            i
        );
    }
}


/*!
 *  The FDF toplc is the center of the first voxel.
 */
void svkDcmMriVolumeReader::InitPlanePositionMacro()
{

    /*
     *  Iterate over slices (frames) and copy ImagePositions
     */
    for (int i = 0; i < this->GetFileNames()->GetNumberOfValues(); i++) { 

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "PerFrameFunctionalGroupsSequence",
            i,
            "PlanePositionSequence"
        );

        svkImageData* tmpImage = svkMriImageData::New(); 
        tmpImage->GetDcmHeader()->ReadDcmFile( this->GetFileNames()->GetValue( i ) ); 

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "PlanePositionSequence",
            0,
            "ImagePositionPatient", 
            tmpImage->GetDcmHeader()->GetStringValue( "ImagePositionPatient" ),
            "PerFrameFunctionalGroupsSequence",
            i
        );

        tmpImage->Delete(); 
    }
}


/*!
 *  Returns the pixel type 
 */
svkDcmHeader::DcmPixelDataFormat svkDcmMriVolumeReader::GetFileType()
{
    return svkDcmHeader::UNSIGNED_INT_2;
}


/*!
 *
 */
int svkDcmMriVolumeReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData");
    return 1;
}

