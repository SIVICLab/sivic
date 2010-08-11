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
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/libs/src/svkDICOMMRWriter.cc $
 *  $Rev: 153 $
 *  $Author: beckn8tor $
 *  $Date: 2010-02-16 22:34:03 -0500 (Tue, 16 Feb 2010) $
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#include <svkDICOMMRWriter.h>
#include <vtkErrorCode.h>
#include <vtkCellData.h>
#include <vtkExecutive.h>


using namespace svk;


vtkCxxRevisionMacro(svkDICOMMRWriter, "$Rev: 153 $");
vtkStandardNewMacro(svkDICOMMRWriter);


/*!
 *
 */
svkDICOMMRWriter::svkDICOMMRWriter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< "svkDICOMMRWriter::svkDICOMMRWriter");

}


/*!
 *
 */
svkDICOMMRWriter::~svkDICOMMRWriter()
{
}



/*!
 *  Write the DICOM MR Spectroscopy multi-frame file.   Also initializes the 
 *  DICOM SpectroscopyData element from the svkImageData object. 
 */
void svkDICOMMRWriter::Write()
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

    this->InitPixelData();

    this->GetImageDataInput(0)->GetDcmHeader()->WriteDcmFile(this->InternalFileName); 

    if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError) {
        vtkErrorMacro("Ran out of disk space; deleting file(s) already written");
        this->DeleteFiles();
        return;
    }

    delete [] this->InternalFileName;
    this->InternalFileName = NULL;

    //  Clear the PixelData element: 
    this->GetImageDataInput(0)->GetDcmHeader()->ClearElement( "PixelData" ); 
}


/*!
 *
 */
int svkDICOMMRWriter::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(svkDICOMMRWriter::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
    return 1;
}


/*!
 *
 */
void svkDICOMMRWriter::SetInput( vtkDataObject* input )
{
    this->SetInput(0, input);
}


/*!
 *
 */
void svkDICOMMRWriter::SetInput(int index, vtkDataObject* input)
{
    if(input) {
        this->SetInputConnection(index, input->GetProducerPort());
    } else {
        // Setting a NULL input removes the connection.
        this->SetInputConnection(index, 0);
    }
}


/*!
 *
 */
vtkDataObject* svkDICOMMRWriter::GetInput(int port)
{
    return this->GetExecutive()->GetInputData(port, 0);
}


/*!
 *
 */
svkImageData* svkDICOMMRWriter::GetImageDataInput(int port)
{
    return svkImageData::SafeDownCast( this->GetInput(port) );
}


/*!
 *  Write the spectral data points to the PixelData DICOM element.       
 */
void svkDICOMMRWriter::InitPixelData()
{

    vtkDebugMacro(<<"svkDICOMMRWriter::InitPixelData()");
    int cols = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "Columns" ); 
    int rows = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "Rows" ); 
    int slices = (this->GetImageDataInput(0)->GetExtent() ) [5] - (this->GetImageDataInput(0)->GetExtent() ) [4] + 1;
    cout << "slices = " << slices << endl;
    int numComponents = 1;

    int dataLength = cols * rows * slices * numComponents;

    switch ( this->GetImageDataInput(0)->GetDcmHeader()->GetPixelDataType() ) {
        case svkDcmHeader::UNSIGNED_INT_1:
          {
              unsigned char *pixelData = (unsigned char *)this->GetImageDataInput(0)->GetScalarPointer();
              this->GetImageDataInput(0)->GetDcmHeader()->SetValue(
                  "PixelData",
                  pixelData, 
                  dataLength 
              );
          }
          break;
        case svkDcmHeader::UNSIGNED_INT_2:
          {
              unsigned short *pixelData = (unsigned short *)this->GetImageDataInput(0)->GetScalarPointer();
              this->GetImageDataInput(0)->GetDcmHeader()->SetValue(
                  "PixelData",
                  pixelData, 
                  dataLength 
              );
          }
          break; 
        case svkDcmHeader::SIGNED_FLOAT_4:
          {
              float *pixelData = (float *)this->GetImageDataInput(0)->GetScalarPointer();
              this->GetImageDataInput(0)->GetDcmHeader()->SetValue(
                  "PixelData",
                  pixelData, 
                  dataLength 
              );
          }
          break;
        case svkDcmHeader::SIGNED_INT_2: 
        case svkDcmHeader::SIGNED_FLOAT_8:
        default:
          vtkErrorMacro("Undefined or unsupported pixel data type");
    }
}

