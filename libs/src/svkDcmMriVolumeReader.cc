/*
 *  Copyright © 2009 The Regents of the University of California.
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
    this->imageData = NULL;  
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

    string fileToCheck(fname);

    if( fileToCheck.size() > 4 ) {

        if ( 
            fileToCheck.substr( fileToCheck.size() - 4 ) == ".dcm"  || 
            fileToCheck.substr( fileToCheck.size() - 4 ) == ".DCM" 
        )  {

            svkImageData* tmp = svkMriImageData::New(); 
            tmp->GetDcmHeader()->ReadDcmFile( fname ); 
            string SOPClassUID = tmp->GetDcmHeader()->GetStringValue( "SOPClassUID" ) ; 
            tmp->Delete(); 

            if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.4" ) {           

                cout << this->GetClassName() << "::CanReadFile(): It's a DICOM MRI File: " <<  fileToCheck << endl;

                SetFileName(fname);

                return 1;
            }

        }

    } 

    vtkDebugMacro(<<this->GetClassName() << "::CanReadFile() It's Not a DICOM MRI file " << fileToCheck );

    return 0;
}

/*!
 *
 */
void svkDcmMriVolumeReader::InitDcmHeader()
{

    string dcmFileName( this->GetFileName() );
    string dcmFileExtension( this->GetFileExtension( this->GetFileName() ) );
    string dcmFilePath( this->GetFilePath( this->GetFileName() ) );  
    
    vtkGlobFileNames* globFileNames = vtkGlobFileNames::New();
    globFileNames->AddFileNames( string( dcmFilePath + "/*." + dcmFileExtension).c_str() );

    vtkSortFileNames* sortFileNames = vtkSortFileNames::New();
    sortFileNames->GroupingOn(); 
    sortFileNames->SetInputFileNames( globFileNames->GetFileNames() );
    sortFileNames->NumericSortOn();
    sortFileNames->Update();
    
    //  If globed file names are not similar, use only the specified file
    if (sortFileNames->GetNumberOfGroups() > 1 ) {

        vtkWarningWithObjectMacro(this, "Found Multiple dcm file groups, using only specified file ");

        vtkStringArray* fileNames = vtkStringArray::New(); 
        fileNames->SetNumberOfValues(1);
        fileNames->SetValue(0, this->GetFileName() );
        sortFileNames->SetInputFileNames( fileNames );
        fileNames->Delete(); 
    }

    this->SetFileNames( sortFileNames->GetFileNames() );
    vtkStringArray* fileNames =  sortFileNames->GetFileNames();
    for (int i = 0; i < fileNames->GetNumberOfValues(); i++) {
        cout << "FN: " << fileNames->GetValue(i) << endl;
    }

    // Read the first file and load the header as the starting point
    this->GetOutput()->GetDcmHeader()->ReadDcmFile( this->GetFileNames()->GetValue(0) );

    //  Now override elements with Multi-Frame sequences and default details:
    svkIOD* iod = svkMRIIOD::New();
    iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    iod->SetReplaceOldElements(false); 
    iod->InitDcmHeader();
    iod->Delete();

    //  Now move info from original MRImageStorage header elements to flesh out enhanced
    //  SOP class elements (often this is just a matter of coping elements from the top 
    //  level to a sequence item. 
    this->InitMultiFrameFunctionalGroupsModule(); 

    /*
     *  odds and ends: 
     */
    svkImageData* tmp = svkMriImageData::New(); 
    tmp->GetDcmHeader()->ReadDcmFile(  this->GetFileNames()->GetValue( 0 ) ); 
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "Rows", 
        tmp->GetDcmHeader()->GetIntValue( "Rows" ) 
    );
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "Columns", 
        tmp->GetDcmHeader()->GetIntValue( "Columns" ) 
    );
    tmp->Delete(); 


    this->GetOutput()->GetDcmHeader()->PrintDcmHeader(); 
    
    globFileNames->Delete();
    sortFileNames->Delete();
}


/*! 
 *  Init the Shared and PerFrame sequences and load the pixel data into the svkImageData object. 
 */
void svkDcmMriVolumeReader::LoadData( svkImageData* data )
{

    int numComponents = 1; 

    int numVoxels[3]; 
    data->GetNumberOfVoxels(numVoxels); 

    long unsigned int dataLength = numVoxels[0] * numVoxels[1] * numVoxels[2] * numComponents;

    this->imageData = new short[ dataLength ];

    int rows    = this->GetOutput()->GetDcmHeader()->GetIntValue( "Rows" ); 
    int columns = this->GetOutput()->GetDcmHeader()->GetIntValue( "Columns" ); 
    int numPixelsInSlice = rows * columns; 

    /*
     *  Iterate over slices (frames) and copy ImagePositions
     */
    for (int i = 0; i < this->GetFileNames()->GetNumberOfValues(); i++) { 
        svkImageData* tmpImage = svkMriImageData::New(); 
        tmpImage->GetDcmHeader()->ReadDcmFile( this->GetFileNames()->GetValue( i ) ); 
        tmpImage->GetDcmHeader()->GetShortValue( "PixelData", imageData + (i * numPixelsInSlice), numPixelsInSlice );  
    }

    //vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_UNSIGNED_SHORT);
    this->dataArray->SetNumberOfComponents( 1 );
    dataArray->SetVoidArray( (void*)( imageData ), dataLength, 0 ); 

    data->GetPointData()->SetScalars(this->dataArray);

    //this->GetOutput()->GetDcmHeader()->PrintDcmHeader(); 
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

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PixelMeasuresSequence"
    );

    svkImageData* tmp = svkMriImageData::New(); 
    tmp->GetDcmHeader()->ReadDcmFile(  this->GetFileNames()->GetValue( 0 ) ); 

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PixelMeasuresSequence",
        0,
        "PixelSpacing",
        tmp->GetDcmHeader()->GetStringValue( "PixelSpacing" ), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PixelMeasuresSequence",
        0,
        "SliceThickness",
        tmp->GetDcmHeader()->GetStringValue( "SliceThickness" ), 
        "SharedFunctionalGroupsSequence",
        0
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
            "FrameAcquisitionDateTime",
            "EMPTY_ELEMENT",
            "PerFrameFunctionalGroupsSequence",
            i
        );

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "FrameContentSequence",
            0,
            "FrameReferenceDateTime",
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
 *  Returns the file root without extension
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

