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


#include <svkDICOMEnhancedMRIWriter.h>
#include <svkImageReader2.h>
#include <vtkErrorCode.h>
#include <vtkCellData.h>
#include <vtkExecutive.h>
#include <vtkImageAccumulate.h>


using namespace svk;


vtkCxxRevisionMacro(svkDICOMEnhancedMRIWriter, "$Rev$");
vtkStandardNewMacro(svkDICOMEnhancedMRIWriter);


/*!
 *
 */
svkDICOMEnhancedMRIWriter::svkDICOMEnhancedMRIWriter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );


}


/*!
 *
 */
svkDICOMEnhancedMRIWriter::~svkDICOMEnhancedMRIWriter()
{
}



/*!
 *  Write the DICOM Enahnced MR Image multi-frame file.   Also initializes the 
 */
void svkDICOMEnhancedMRIWriter::Write()
{

    this->SetErrorCode(vtkErrorCode::NoError);

    if (! this->FileName ) {
        vtkErrorMacro(<<"Write:Please specify either a FileName or a file prefix and pattern");
        this->SetErrorCode(vtkErrorCode::NoFileNameError);
        return;
    }

    // Make sure the file name is allocated
    this->InternalFileName =
        new char[(this->FileName ? strlen(this->FileName) : 1) +
        (this->FilePrefix ? strlen(this->FilePrefix) : 1) +
        (this->FilePattern ? strlen(this->FilePattern) : 1) + 10];

    this->FileNumber = 0;
    this->MinimumFileNumber = this->FileNumber;
    this->FilesDeleted = 0;
    this->UpdateProgress(0.0);

    this->MaximumFileNumber = this->FileNumber;

    // determine the name
    if (this->FileName) {
        sprintf(this->InternalFileName,"%s",this->FileName);
    } else {
        if (this->FilePrefix) {
            sprintf(this->InternalFileName, this->FilePattern,
                    this->FilePrefix, this->FileNumber);
        } else {
            sprintf(this->InternalFileName, this->FilePattern,this->FileNumber);
        }
    }

    this->InitPixelData( 
        this->GetImageDataInput(0)->GetDcmHeader() 
    );  


    //  Make sure there is an extension:
    vtkstd::string fileRoot = svkImageReader2::GetFileRoot( this->InternalFileName );
    sprintf(this->InternalFileName, "%s.dcm", fileRoot.c_str() );
     
    this->GetImageDataInput(0)->GetDcmHeader()->WriteDcmFile(this->InternalFileName); 

    if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError) {
        vtkErrorMacro("Ran out of disk space; deleting file(s) already written");
        this->DeleteFiles();
        return;
    }

    delete [] this->InternalFileName;
    this->InternalFileName = NULL;

    //  Clear the PixelData element: 
    //this->GetImageDataInput(0)->GetDcmHeader()->ClearElement( "PixelData" ); 
}


/*!
 *  Determines the length of the Pixel Data for the specific IOD (all frames or single frame).
 */
int svkDICOMEnhancedMRIWriter::GetDataLength()
{
    int cols = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "Columns" );
    int rows = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "Rows" );
    int slices = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfSlices();
    int numComponents = 1;
    int dataLength = cols * rows * slices * numComponents;
    return dataLength; 
}
