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

#include <vtkErrorCode.h>
#include <vtkCellData.h>
#include <vtkExecutive.h>
#include <vtkImageAccumulate.h>

#include <svkDICOMMRIWriter.h>
#include <svkMRIIOD.h>


using namespace svk;


vtkCxxRevisionMacro(svkDICOMMRIWriter, "$Rev$");
vtkStandardNewMacro(svkDICOMMRIWriter);


/*!
 *
 */
svkDICOMMRIWriter::svkDICOMMRIWriter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );
}


/*!
 *
 */
svkDICOMMRIWriter::~svkDICOMMRIWriter()
{
}


/*!
 *  Write DICOM MR Image Storage SOP class (single frame) files. 
 */
void svkDICOMMRIWriter::Write()
{

    this->SetErrorCode(vtkErrorCode::NoError);

    //  If a file name is not know then use a generic format (E#S#I#)
    if (! this->FileName ) {
        vtkstd::string prefix = 
                this->GetImageDataInput(0)->GetDcmHeader()->GetStringValue( "StudyID" ) + 
                this->GetImageDataInput(0)->GetDcmHeader()->GetStringValue( "SeriesNumber" ) +
                "I";
        this->SetFilePrefix( prefix.c_str() );
    }

    // Make sure the file name is allocated
    this->InternalFileName =
        new char[(this->FileName ? strlen(this->FileName) : 1) +
        (this->FilePrefix ? strlen(this->FilePrefix) : 1) +
        (this->FilePattern ? strlen(this->FilePattern) : 1) + 10];



    /*  
     *  Write each frame to a DICOM file:     
     *
     *  Convert the Enhanced header to a single frame header, then set the pixel 
     *  buffer from the appropriate frame and write to disk.
     *
     *  First, get a blank svkDcmHeader and initialize it to an MRImageStorage SOP
     *  instance (with details copied from the current Enhanced MRI header). 
     */

    svkDcmHeader* mriHeader; 
    if ( svkDcmHeader::adapter_type == svkDcmtkAdapter::DCMTK_API ) {
        mriHeader = svkDcmtkAdapter::New() ;
    }

    svkMRIIOD* iod = svkMRIIOD::New();
    iod->SetDcmHeader( mriHeader );
    iod->InitDcmHeader();
    iod->Delete();

    vtkIdType dataType = this->GetImageDataInput(0)->GetScalarType(); 

    this->GetImageDataInput(0)->GetDcmHeader()->ConvertEnhancedMriToMriHeader(
        mriHeader, 
        dataType
    );

    int cols = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "Columns" );
    int rows = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "Rows" );
    int pixelsPerSlice = cols * rows; 

    this->MaximumFileNumber = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "NumberOfFrames" ); 
    for (int frame = 0; frame < MaximumFileNumber; frame++ ) { 

        this->FileNumber = frame + 1;
        this->MinimumFileNumber = this->FileNumber;
        this->FilesDeleted = 0;
        this->UpdateProgress(0.0);

        // determine the name
        if (this->FileName) {
            sprintf(this->InternalFileName,"%sI%d.dcm",this->FileName, this->FileNumber);
        } else {
            if (this->FilePrefix) {
                sprintf(this->InternalFileName, "%s%d.dcm",
                        this->FilePrefix, this->FileNumber);
            } 
        }

        if (this->GetDebug()) {
            cout << this->GetClassName() << ": Writing DICOM file: " << this->InternalFileName << endl;
        }

        // SOP Instance UID and Media Storage Instance UID need to be the same
        mriHeader->SetValue( "InstanceNumber", this->FileNumber ); 
        mriHeader->InsertUniqueUID( "SOPInstanceUID" );
        mriHeader->SetValue(
            "MediaStorageSOPInstanceUID", 
            mriHeader->GetStringValue( "SOPInstanceUID" )
        );

        vtkstd::string imagePositionPatient = this->GetImageDataInput(0)->GetDcmHeader()->GetStringSequenceItemElement(
            "PlanePositionSequence",
            0,
            "ImagePositionPatient",
            "PerFrameFunctionalGroupsSequence",
            frame 
        );
        mriHeader->InitImagePlaneModule( imagePositionPatient ); 

        //  set pixel data
        this->InitPixelData(
            mriHeader, 
            frame
        );

        mriHeader->WriteDcmFile(this->InternalFileName); 

        if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError) {
            vtkErrorMacro("Ran out of disk space; deleting file(s) already written");
            this->DeleteFiles();
            return;
        }
    }

    mriHeader->Delete();
    delete [] this->InternalFileName;
    this->InternalFileName = NULL;

}


/*!
 *  Determines the length of the Pixel Data for the specific IOD (all frames or single frame).
 */
int svkDICOMMRIWriter::GetDataLength()
{
    int cols = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "Columns" );
    int rows = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "Rows" );
    int dataLength = cols * rows;
    return dataLength;
}

