/*
 *  Copyright © 2009-2012 The Regents of the University of California.
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
    int numSlices = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfSlices();
    int numFrames = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "NumberOfFrames" ); 
    this->MaximumFileNumber = numFrames;  
    int numVolumes = numFrames / numSlices; 
    int pixelsPerSlice = cols * rows; 

    int frame = -1;
    for (int slice = 0; slice < numSlices; slice++ ) { 
        for (int volume = 0; volume < numVolumes; volume++ ) { 
            frame++;
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

            //  SOP Instance UID and Media Storage Instance UID need to be the same, 
            //  but DCMTK manages this on write: 
            mriHeader->SetValue( "InstanceNumber", this->FileNumber ); 
            mriHeader->InsertUniqueUID( "SOPInstanceUID" );

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
                slice,
                volume 
            );

            mriHeader->WriteDcmFile(this->InternalFileName); 

            if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError) {
                vtkErrorMacro("Ran out of disk space; deleting file(s) already written");
                this->DeleteFiles();
                return;
            }
        }
    }

    mriHeader->Delete();
    delete [] this->InternalFileName;
    this->InternalFileName = NULL;

}


/*!
 *  Write the pixel data to the PixelData DICOM element.       
 *  if a slice number (starting at 0 index) is specified, will 
 *  init only that block of PixelData in the DCM file.  
 * 
 */
void svkDICOMMRIWriter::InitPixelData( svkDcmHeader* dcmHeader, int sliceNumber, int volNumber )
{

    vtkDebugMacro( << this->GetClassName() << "::InitPixelData()" );

    int dataLength = this->GetDataLength();
    int offset = 0; 
    if (sliceNumber >= 0 ) {
        offset = dataLength * sliceNumber; 
    }

    switch ( this->GetImageDataInput(0)->GetDcmHeader()->GetPixelDataType( this->GetImageDataInput(0)->GetScalarType() ) ) {

        case svkDcmHeader::UNSIGNED_INT_1:
        {
            // Dicom does not support the writing of 8 bit words so we will convert to 16
            unsigned char* pixelData = static_cast<unsigned char*>(
                static_cast<vtkUnsignedCharArray*>(this->GetImageDataInput(0)->GetPointData()->GetArray(volNumber))->GetPointer(0)
            );
            unsigned short* shortPixelData = new unsigned short[dataLength];
            for (int i = 0; i < dataLength; i++) {
                shortPixelData[i] = pixelData[offset + i];
            }
            dcmHeader->SetPixelDataType( svkDcmHeader::UNSIGNED_INT_2 );
            dcmHeader->SetValue(
                  "PixelData",
                  shortPixelData,
                  dataLength 
            );
            delete[] shortPixelData;
        }
        break;

        case svkDcmHeader::UNSIGNED_INT_2:
        {
            unsigned short* pixelData = static_cast<unsigned short*>(
                static_cast<vtkUnsignedShortArray*>(this->GetImageDataInput(0)->GetPointData()->GetArray(volNumber))->GetPointer(0)
            );
                cout << *this->GetImageDataInput(0) << endl;
            dcmHeader->SetValue(
                  "PixelData",
                  &(pixelData[offset]), 
                  dataLength 
            );
        }
        break; 

        case svkDcmHeader::SIGNED_FLOAT_4:
        case svkDcmHeader::SIGNED_FLOAT_8:
        {
            //  Fix BitsAllocated, etc to be represent 
            //  signed short data
            dcmHeader->SetPixelDataType(svkDcmHeader::UNSIGNED_INT_2);

            unsigned short* pixelData = new unsigned short[dataLength];  
            float slope; 
            float intercept; 
            this->GetShortScaledPixels( pixelData, slope, intercept, sliceNumber, volNumber ); 
            
            dcmHeader->SetValue(
                  "PixelData",
                  pixelData, 
                  dataLength 
            );

            //  Init Rescale Attributes:    
            //  For Enhanced MR Image Storage use the PixelValueTransformation Macro
            //  For MR Image Stroage use VOI LUT Module. 
            double inputRangeMin;
            double inputRangeMax;
            this->GetPixelRange(inputRangeMin, inputRangeMax, volNumber);
            vtkstd::string SOPClassUID = dcmHeader->GetStringValue( "SOPClassUID" ) ;
            if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.4" ) {

                //  MR Image Storage: 
                //      convert slope and intercept to center and width:     
                //      Get the pixel value range (defines the WL of VOI LUT):
                double width = inputRangeMax - inputRangeMin;
                double center = (inputRangeMax + inputRangeMin)/2;
                dcmHeader->InitVOILUTModule( center, width ); 

            } else {

                //  Enhanced MR Image Storage: 
                //      slope and intercept are for real -> short, we need the 
                //      inverse transformation here that a downstream application:
                //      can use to regenerate the original values:
                float slopeReverse = 1/slope;  
                int shortMin = VTK_UNSIGNED_SHORT_MIN; 
                float interceptReverse = inputRangeMin - shortMin/slope;
                dcmHeader->InitPixelValueTransformationMacro( slopeReverse, interceptReverse ); 

            }

            delete[] pixelData; 
        }
        break;

        case svkDcmHeader::SIGNED_INT_2: 
        {
            short* pixelData = static_cast<short*>(
                static_cast<vtkShortArray*>(this->GetImageDataInput(0)->GetPointData()->GetArray(volNumber))->GetPointer(0)
            );
            dcmHeader->SetValue(
                  "PixelData",
                  &(pixelData[offset]), 
                  dataLength 
            );
        }
        default:
          vtkErrorMacro("Undefined or unsupported pixel data type");
    }
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

