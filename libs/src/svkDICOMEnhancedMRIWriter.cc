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


#include <svkDICOMEnhancedMRIWriter.h>
#include <svkTypeUtils.h>
#include <svkImageReader2.h>
#include </usr/include/vtk/vtkErrorCode.h>
#include </usr/include/vtk/vtkCellData.h>
#include </usr/include/vtk/vtkExecutive.h>
#include </usr/include/vtk/vtkImageAccumulate.h>


using namespace svk;


//vtkCxxRevisionMacro(svkDICOMEnhancedMRIWriter, "$Rev$");
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
    this->SetFilePrefix("DCM");

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

    //  Make sure the series is unique:
    this->GetImageDataInput(0)->GetDcmHeader()->MakeDerivedDcmHeader();

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
    string internalFileNameString; 

    this->FileNumber = 0;
    this->MinimumFileNumber = this->FileNumber;
    this->FilesDeleted = 0;
    this->UpdateProgress(0.0);

    this->MaximumFileNumber = this->FileNumber;

    // determine the name
    if (this->FileName) {
        //sprintf(this->InternalFileName,"%s",this->FileName);
        internalFileNameString = this->FileName; 
    } else {
        if (this->FilePrefix) {
            //sprintf(this->InternalFileName, this->FilePattern,
                    //this->FilePrefix, this->FileNumber);
            internalFileNameString = string(this->FilePattern) + string(this->FilePrefix) + svkTypeUtils::IntToString(this->FileNumber); 
        } else {
            //sprintf(this->InternalFileName, this->FilePattern,this->FileNumber);
            internalFileNameString = string(this->FilePattern) + svkTypeUtils::IntToString(this->FileNumber); 
        }
    }

    this->InitPixelData( 
        this->GetImageDataInput(0)->GetDcmHeader() 
    );  


    //  Make sure there is an extension:
    internalFileNameString.append(".dcm");
    
    if ( this->useLosslessCompression ) {
        this->GetImageDataInput(0)->GetDcmHeader()->
                WriteDcmFileCompressed(internalFileNameString); 
    } else {
        this->GetImageDataInput(0)->GetDcmHeader()->WriteDcmFile(internalFileNameString); 
    }

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
 *  Write the pixel data to the PixelData DICOM element.       
 *  if a slice number (starting at 0 index) is specified, will 
 *  init only that block of PixelData in the DCM file.  
 * 
 */
void svkDICOMEnhancedMRIWriter::InitPixelData( svkDcmHeader* dcmHeader )
{

    vtkDebugMacro( << this->GetClassName() << "::InitPixelData()" );

    int numSlices = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfSlices();
    int numFrames = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "NumberOfFrames" );
    int numVolumes = numFrames / numSlices;

    //  size of all volumes together 
    int dataLength = this->GetDataLength();

    int offset = 0; 

    int vtkDataType = vtkImageData::GetScalarType( this->GetImageDataInput(0)->GetInformation() );
    dcmHeader->InitPixelValueTransformationMacro();
    switch ( this->GetImageDataInput(0)->GetDcmHeader()->GetPixelDataType( vtkDataType ) ) {

        case svkDcmHeader::UNSIGNED_INT_1:
        {
            // Dicom does not support the writing of 8 bit words so we will convert to 16
            unsigned short* shortPixelData = new unsigned short[dataLength*numVolumes];
            for (int volume = 0; volume < numVolumes; volume++ ) {
                offset = dataLength * volume;
                unsigned char* pixelData = static_cast<unsigned char*>(
                    static_cast<vtkUnsignedCharArray*>(this->GetImageDataInput(0)->GetPointData()->GetArray(volume))->GetPointer(0)
                );
                for (int i = 0; i < dataLength; i++) {
                    shortPixelData[offset + i] = pixelData[i];
                }
                dcmHeader->SetPixelDataType( svkDcmHeader::UNSIGNED_INT_2 );
            }
            dcmHeader->SetValue(
                  "PixelData",
                  shortPixelData,
                  dataLength * numVolumes
            );
            delete[] shortPixelData;
        }
        break;

        case svkDcmHeader::UNSIGNED_INT_2:
        {
            unsigned short* pixelData = new unsigned short[dataLength*numVolumes];
            for (int volume = 0; volume < numVolumes; volume++ ) {
                offset = dataLength * volume;
                unsigned short* vol = static_cast<unsigned short*>(
                    static_cast<vtkUnsignedShortArray*>(this->GetImageDataInput(0)->GetPointData()->GetArray(volume))->GetPointer(0)
                );
                memcpy( &pixelData[offset], vol, dataLength * 2 ) ;
            }
            dcmHeader->SetValue(
                  "PixelData",
                  pixelData, 
                  dataLength*numVolumes
            );
        }
        break; 

        case svkDcmHeader::SIGNED_FLOAT_4:
        case svkDcmHeader::SIGNED_FLOAT_8:
        {

            unsigned short* pixelData = new unsigned short[dataLength*numVolumes];

            

            //  Init Rescale Attributes:    
            //  For Enhanced MR Image Storage use the PixelValueTransformation Macro
            //  For MR Image Stroage use VOI LUT Module. 
            double inputRangeMin = DBL_MAX;
            double inputRangeMax = DBL_MIN;
            double inputRangeMinTmp;
            double inputRangeMaxTmp;
            for (int volume = 0; volume < numVolumes; volume++ ) {
                this->GetPixelRange(inputRangeMinTmp, inputRangeMaxTmp, volume);
                if ( inputRangeMinTmp < inputRangeMin ) {  
                    inputRangeMin = inputRangeMinTmp;
                }
                if ( inputRangeMaxTmp > inputRangeMax ) {  
                    inputRangeMax = inputRangeMaxTmp;
                }
                    
            }

            double deltaRangeIn = inputRangeMax - inputRangeMin;

            //  Get the output range for scaling:
            int shortMin = VTK_UNSIGNED_SHORT_MIN;
            int shortMax = VTK_UNSIGNED_SHORT_MAX;
            double deltaRangeOut = shortMax - shortMin;

            //  apply linear mapping from float range to signed short range;
            //  shortMax = inputRangeMax * m + b;
            //  shortMin = inputRangeMin * m + b;
            double slope = deltaRangeOut/deltaRangeIn;
            double intercept = shortMin - inputRangeMin * ( deltaRangeOut/deltaRangeIn );

            for (int volume = 0; volume < numVolumes; volume++ ) {
                offset = (dataLength) * volume;
                this->GetScaledPixels( pixelData+offset, slope, intercept, -1, volume);
            }
            dcmHeader->SetValue(
                  "PixelData",
                  pixelData,
                  dataLength*numVolumes
            );

            //  Fix BitsAllocated, etc to be represent
            //  signed short data
            dcmHeader->SetPixelDataType(svkDcmHeader::UNSIGNED_INT_2);

            //  Enhanced MR Image Storage:
            //      slope and intercept are for real -> short, we need the
            //      inverse transformation here that a downstream application:
            //      can use to regenerate the original values:
            float slopeReverse = 1/slope;
            float interceptReverse = inputRangeMin - shortMin/slope;
            dcmHeader->InitPixelValueTransformationMacro( slopeReverse, interceptReverse );


            delete[] pixelData; 
        }
        break;

        case svkDcmHeader::SIGNED_INT_2: 
        {
            short* pixelData = new short[dataLength*numVolumes];
            for (int volume = 0; volume < numVolumes; volume++ ) {
                offset = dataLength * volume;

                short* vol = static_cast<short*>(
                    static_cast<vtkShortArray*>(this->GetImageDataInput(0)->GetPointData()->GetArray(volume))->GetPointer(0)
                );
                memcpy( &pixelData[offset], vol, dataLength * 2 ) ;
            }
            dcmHeader->SetValue(
                  "PixelData",
                  pixelData, 
                  dataLength*numVolumes
            );
        }
        default:
          vtkErrorMacro("Undefined or unsupported pixel data type");
    }
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

    int numSlices = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfSlices();
    int numFrames = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "NumberOfFrames" );
    int numVolumes = numFrames / numSlices;

    int dataLength = cols * rows * slices * numComponents;
    return dataLength; 
}


