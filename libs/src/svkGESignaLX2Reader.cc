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
 *      Don C. Bigler, Ph.D.
 */


#include <svkGESignaLX2Reader.h>
#include </usr/include/vtk/vtkByteSwap.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include <svkMriImageData.h>

#define GE_SIGNA_5X_MAGIC_NUMBER  0x494d4746

using namespace svk;


//vtkCxxRevisionMacro(svkGESignaLX2Reader, "$Rev$");
vtkStandardNewMacro(svkGESignaLX2Reader);


/*!
 *
 */
svkGESignaLX2Reader::svkGESignaLX2Reader()
{
#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkGESignaLX2Reader");
#endif
    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    // Set the byte ordering, as big-endian.
    this->SetDataByteOrderToBigEndian();
    this->imageHeader = NULL;
    this->numFrames = 1;
}


/*!
 *
 */
svkGESignaLX2Reader::~svkGESignaLX2Reader()
{
    vtkDebugMacro(<<this->GetClassName() << "::~" << this->GetClassName() << "()");
    if( this->imageHeader != NULL ) {
        delete this->imageHeader;
        this->imageHeader = NULL;
    }
}

int svkGESignaLX2Reader::CanReadFile(const char* fname)
{ 
    FILE *fp = fopen(fname, "rb");
    if (!fp)
    {
        return 0;
    }
  
    int magic;
    fseek(fp, 3228, SEEK_SET);
    fread(&magic, 4, 1, fp);
    vtkByteSwap::Swap4BE(&magic);
  
    if (magic != GE_SIGNA_5X_MAGIC_NUMBER) // "IMGF"
    {
        fclose(fp);
        return 0;
    }
    return 3;
}


GESignaHeader* svkGESignaLX2Reader::ReadHeader(const char *FileNameToRead)
{
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): FileNameToRead = " << FileNameToRead);

    GESignaHeader *signaHeader = new GESignaHeader;

    if ( signaHeader == NULL ) {
        vtkWarningWithObjectMacro(this, "Unable to allocate memory for GESignaHeader!");
        return NULL;
    }
    memset(signaHeader,0,sizeof(GESignaHeader));

    FILE *fp = fopen(FileNameToRead, "rb");
    if (!fp) {
        delete signaHeader;
        vtkWarningWithObjectMacro(this, "Unable to open file!");
        return NULL;
    }

    // Seek to the correct location for LX2.
    fseek(fp, 3228, SEEK_SET);
  
    if( !fread(&signaHeader->mainHeader, sizeof(GEImageMainHeader), 1, fp) ) {
        fclose(fp);
        delete signaHeader;
        vtkWarningWithObjectMacro(this, "Could not read main header!");
        return NULL;
    }
    vtkByteSwap::Swap4BERange(&signaHeader->mainHeader, 13);
    vtkByteSwap::Swap2BERange(&signaHeader->mainHeader.Version, 2);
    vtkByteSwap::Swap4BERange(&signaHeader->mainHeader.Pointer_ID, 25);
  
    if (signaHeader->mainHeader.Magic_Number != GE_SIGNA_5X_MAGIC_NUMBER) { // "IMGF"
        fclose(fp);
        delete signaHeader;
        vtkWarningWithObjectMacro(this, "Unknown file type! Not a GE ximg file!");
        return NULL;
    }

    // Like version 2 Signa 5X MR files, LX2 does not define the file pointers.
    // Do it here.
    signaHeader->mainHeader.Pointer_Suite = 0;
    signaHeader->mainHeader.Length_Suite = 116;
    signaHeader->mainHeader.Pointer_Exam =
        signaHeader->mainHeader.Pointer_Suite + signaHeader->mainHeader.Length_Suite;
    signaHeader->mainHeader.Length_Exam = 1040;
    signaHeader->mainHeader.Pointer_Series =
        signaHeader->mainHeader.Pointer_Exam + signaHeader->mainHeader.Length_Exam;
    signaHeader->mainHeader.Length_Series = 1028;
    signaHeader->mainHeader.Pointer_Image =
        signaHeader->mainHeader.Pointer_Series + signaHeader->mainHeader.Length_Series;
    signaHeader->mainHeader.Length_Image = 1044;
    signaHeader->mainHeader.Header_Length += 3228;  // For some reason I need to add the IMGF offset!

    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Magic_Number = " 
        << hex << signaHeader->mainHeader.Magic_Number);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Header_Length = " 
        << signaHeader->mainHeader.Header_Length);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Width = " 
        << signaHeader->mainHeader.Width);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Height = " 
        << signaHeader->mainHeader.Height);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Depth = " 
        << signaHeader->mainHeader.Depth);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Compressed = " 
        << signaHeader->mainHeader.Compressed);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Dwindow = " 
        << signaHeader->mainHeader.Dwindow);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Dlevel = " 
        << signaHeader->mainHeader.Dlevel);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Background_Shade = " 
        << signaHeader->mainHeader.Background_Shade);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Overflow = " 
        << signaHeader->mainHeader.Overflow);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Underflow = " 
        << signaHeader->mainHeader.Underflow);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Top_Offset = " 
        << signaHeader->mainHeader.Top_Offset);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Bottom_Offset = " 
        << signaHeader->mainHeader.Bottom_Offset);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Version = " 
        << signaHeader->mainHeader.Version);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Checksum = " 
        << signaHeader->mainHeader.Checksum);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Pointer_ID = " 
        << signaHeader->mainHeader.Pointer_ID);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Length_ID = " 
        << signaHeader->mainHeader.Length_ID);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Pointer_Unpack = " 
        << signaHeader->mainHeader.Pointer_Unpack);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Length_Unpack = " 
        << signaHeader->mainHeader.Length_Unpack);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Pointer_Compress = " 
        << signaHeader->mainHeader.Pointer_Compress);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Length_Compress = " 
        << signaHeader->mainHeader.Length_Compress);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Pointer_Histogram = " 
        << hex << signaHeader->mainHeader.Pointer_Histogram);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Length_Histogram = " 
        << signaHeader->mainHeader.Length_Histogram);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Pointer_Text = " 
        << signaHeader->mainHeader.Pointer_Text);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Length_Text = " 
        << signaHeader->mainHeader.Length_Text);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Pointer_Graphics = " 
        << signaHeader->mainHeader.Pointer_Graphics);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Length_Graphics = " 
        << signaHeader->mainHeader.Length_Graphics);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Pointer_DBHeader = " 
        << signaHeader->mainHeader.Pointer_DBHeader);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Length_DBHeader = " 
        << signaHeader->mainHeader.Length_DBHeader);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Level_Offset = " 
        << signaHeader->mainHeader.Level_Offset);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Pointer_User = " 
        << signaHeader->mainHeader.Pointer_User);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Length_User = " 
        << signaHeader->mainHeader.Length_User);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Pointer_Suite = " 
        << signaHeader->mainHeader.Pointer_Suite);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Length_Suite = " 
        << signaHeader->mainHeader.Length_Suite);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Pointer_Exam = " 
        << signaHeader->mainHeader.Pointer_Exam);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Length_Exam = " 
        << signaHeader->mainHeader.Length_Exam);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Pointer_Series = " 
        << signaHeader->mainHeader.Pointer_Series);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Length_Series = " 
        << signaHeader->mainHeader.Length_Series);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Pointer_Image = " 
        << signaHeader->mainHeader.Pointer_Image);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->mainHeader.Length_Image = " 
        << signaHeader->mainHeader.Length_Image);

    // Seek to the suite header and read product ID.
    int valueOffset = 7;
    fseek(fp, signaHeader->mainHeader.Pointer_Suite + valueOffset, SEEK_SET);
    if( !fread(signaHeader->Suite_Product_ID, sizeof(char), 13, fp) ) {
        fclose(fp);
        delete signaHeader;
        vtkWarningWithObjectMacro(this, "Could not read product ID from suite header!");
        return NULL;
    }
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Suite_Product_ID = " 
        << signaHeader->Suite_Product_ID);

    // Create a buffer to read the exam header.
    char* buffer = new char[signaHeader->mainHeader.Length_Exam];
    if ( buffer == NULL ) {
        fclose(fp);
        delete signaHeader;
        vtkWarningWithObjectMacro(this, "Unable to allocate memory for exam header!");
        return NULL;
    }

    // Now seek to the exam header and read the data into the buffer.
    fseek(fp, signaHeader->mainHeader.Pointer_Exam, SEEK_SET);
    if( !fread(buffer, sizeof(char), signaHeader->mainHeader.Length_Exam, fp) ) {
        fclose(fp);
        delete signaHeader;
        vtkWarningWithObjectMacro(this, "Could not read exam header!");
        return NULL;
    }

    // Now extract the exam information from the buffer.
    size_t numCopy = 4;
    memcpy(signaHeader->Exam_System_ID,buffer,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_System_ID = " 
        << signaHeader->Exam_System_ID);

    valueOffset = 8;
    memcpy(&signaHeader->Exam_Number,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->Exam_Number);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Number = " 
        << signaHeader->Exam_Number);

    valueOffset = 10;
    numCopy = 34;
    memcpy(signaHeader->Exam_Hospital_Name,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Hospital_Name = " 
        << signaHeader->Exam_Hospital_Name);

    valueOffset = 84;
    memcpy(&signaHeader->Exam_Magnet_Strength,buffer+valueOffset,sizeof(unsigned int));
    vtkByteSwap::Swap4BE(&signaHeader->Exam_Magnet_Strength);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Magnet_Strength = " 
        << signaHeader->Exam_Magnet_Strength);

    valueOffset = 88;
    numCopy = 13;
    memcpy(signaHeader->Exam_Patient_ID,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Patient_ID = " 
        << signaHeader->Exam_Patient_ID);

    valueOffset = 101;
    numCopy = 25;
    memcpy(signaHeader->Exam_Patient_Name,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Patient_Name = " 
        << signaHeader->Exam_Patient_Name);

    valueOffset = 126;
    memcpy(&signaHeader->Exam_Patient_Age,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->Exam_Patient_Age);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Patient_Age = " 
        << signaHeader->Exam_Patient_Age);

    valueOffset = 128;
    memcpy(&signaHeader->Exam_Patient_Age_Notation,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->Exam_Patient_Age_Notation);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Patient_Age_Notation = " 
        << signaHeader->Exam_Patient_Age_Notation);

    valueOffset = 130;
    memcpy(&signaHeader->Exam_Patient_Sex,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->Exam_Patient_Sex);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Patient_Sex = " 
        << signaHeader->Exam_Patient_Sex);

    valueOffset = 132;
    memcpy(&signaHeader->Exam_Patient_Weight_In_Grams,buffer+valueOffset,sizeof(unsigned int));
    vtkByteSwap::Swap4BE(&signaHeader->Exam_Patient_Weight_In_Grams);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Patient_Weight_In_Grams = " 
        << signaHeader->Exam_Patient_Weight_In_Grams);

    valueOffset = 138;
    numCopy = 62;
    memcpy(signaHeader->Exam_Patient_History,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Patient_History = " 
        << signaHeader->Exam_Patient_History);

    valueOffset = 200;
    numCopy = 12;
    memcpy(signaHeader->Exam_Requisition_Number,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Requisition_Number = " 
        << signaHeader->Exam_Requisition_Number);

    valueOffset = 212;
    memcpy(&signaHeader->Exam_Time_Stamp,buffer+valueOffset,sizeof(int));
    vtkByteSwap::Swap4BE(&signaHeader->Exam_Time_Stamp);
    this->statTimeAndDateToAscii(&signaHeader->Exam_Time_Stamp, 
        signaHeader->Exam_Time_Stamp_Time, signaHeader->Exam_Time_Stamp_Date);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Time_Stamp_Time = " 
        << signaHeader->Exam_Time_Stamp_Time);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Time_Stamp_Date = " 
        << signaHeader->Exam_Time_Stamp_Date);

    valueOffset = 216;
    numCopy = 33;
    memcpy(signaHeader->Exam_Referring_Physician,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Referring_Physician = " 
        << signaHeader->Exam_Referring_Physician);

    valueOffset = 249;
    numCopy = 33;
    memcpy(signaHeader->Exam_Radiologist,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Radiologist = " 
        << signaHeader->Exam_Radiologist);

    valueOffset = 282;
    numCopy = 4;
    memcpy(signaHeader->Exam_Operator,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Operator = " 
        << signaHeader->Exam_Operator);

    valueOffset = 286;
    numCopy = 23;
    memcpy(signaHeader->Exam_Description,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Description = " 
        << signaHeader->Exam_Description);

    valueOffset = 309;
    numCopy = 3;
    memcpy(signaHeader->Exam_Type,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Type = " 
        << signaHeader->Exam_Type);

    valueOffset = 364;
    numCopy = 2;
    memcpy(signaHeader->Exam_Software_Version,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Software_Version = " 
        << signaHeader->Exam_Software_Version);

    valueOffset = 450;
    numCopy = 16;
    memcpy(signaHeader->Exam_Unique_System_ID,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Unique_System_ID = " 
        << signaHeader->Exam_Unique_System_ID);

    // Done with exam, delete buffer.
    delete[] buffer;
    buffer = NULL;
    
    // Check if this is a MR file.  If not exit!
    std::string examType(signaHeader->Exam_Type);
    if ( examType != "MR" ) {
        fclose(fp);
        delete signaHeader;
        vtkWarningWithObjectMacro(this, "This reader only handles the MR exam type!");
        return NULL;
    }

    // Allocate buffer for series header.
    buffer = new char[signaHeader->mainHeader.Length_Series];
    if ( buffer == NULL ) {
        fclose(fp);
        delete signaHeader;
        vtkWarningWithObjectMacro(this, "Unable to allocate memory for series header!");
        return NULL;
    }

    // Now seek to the series header and read the data into the buffer.
    fseek(fp, signaHeader->mainHeader.Pointer_Series, SEEK_SET);
    if( !fread(buffer, sizeof(char), signaHeader->mainHeader.Length_Series, fp) ) {
        fclose(fp);
        delete signaHeader;
        vtkWarningWithObjectMacro(this, "Could not read exam header!");
        return NULL;
    }

    // Now extract the series information from the buffer.
    valueOffset = 10;
    memcpy(&signaHeader->Series_Number,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->Series_Number);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Series_Number = " 
        << signaHeader->Series_Number);

    valueOffset = 12;
    memcpy(&signaHeader->Series_Time_Stamp,buffer+valueOffset,sizeof(int));
    vtkByteSwap::Swap4BE(&signaHeader->Series_Time_Stamp);
    this->statTimeAndDateToAscii(&signaHeader->Series_Time_Stamp, 
        signaHeader->Series_Time_Stamp_Time, signaHeader->Series_Time_Stamp_Date);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Series_Time_Stamp_Time = " 
        << signaHeader->Series_Time_Stamp_Time);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Series_Time_Stamp_Date = " 
        << signaHeader->Series_Time_Stamp_Date);

    valueOffset = 20;
    numCopy = 30;
    memcpy(signaHeader->Series_Description,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Series_Description = " 
        << signaHeader->Series_Description);
    valueOffset = 68;
    memcpy(&signaHeader->Series_Type,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->Series_Type);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Series_Type = " 
        << signaHeader->Series_Type);

    valueOffset = 76;
    memcpy(&signaHeader->Series_Patient_Position,buffer+valueOffset,sizeof(unsigned int));
    vtkByteSwap::Swap4BE(&signaHeader->Series_Patient_Position);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Series_Patient_Position = " 
        << signaHeader->Series_Patient_Position);

    valueOffset = 80;
    memcpy(&signaHeader->Series_Patient_Entry,buffer+valueOffset,sizeof(unsigned int));
    vtkByteSwap::Swap4BE(&signaHeader->Series_Patient_Entry);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Series_Patient_Entry = " 
        << signaHeader->Series_Patient_Entry);

    valueOffset = 92;
    numCopy = 26;
    memcpy(signaHeader->Series_Scan_Potocol_Name,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Series_Scan_Potocol_Name = " 
        << signaHeader->Series_Scan_Potocol_Name);

    // Done with series, delete buffer and allocate for MR header.
    delete[] buffer;
    buffer = NULL;
    buffer = new char[signaHeader->mainHeader.Length_Image];
    if ( buffer == NULL ) {
        fclose(fp);
        delete signaHeader;
        vtkWarningWithObjectMacro(this, "Unable to allocate memory for MR header!");
        return NULL;
    }
  
    // Now seek to the MR header and read the data into the buffer.
    fseek(fp, signaHeader->mainHeader.Pointer_Image, SEEK_SET);
    if( !fread(buffer, sizeof(char), signaHeader->mainHeader.Length_Image, fp) ) {
        fclose(fp);
        delete signaHeader;
        vtkWarningWithObjectMacro(this, "Could not read exam header!");
        return NULL;
    }
    // Won't need anymore info from the file after this, so close file.
    fclose(fp);

    // Now extract the MR information from the buffer.
    // This is the largest header!
    valueOffset = 12;
    memcpy(&signaHeader->MR_Image_Number,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Image_Number);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Number = " 
        << signaHeader->MR_Image_Number);

    valueOffset = 16;
    memcpy(&signaHeader->MR_Time_Stamp,buffer+valueOffset,sizeof(int));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Time_Stamp);
    this->statTimeAndDateToAscii(&signaHeader->MR_Time_Stamp, 
        signaHeader->MR_Time_Stamp_Time, signaHeader->MR_Time_Stamp_Date);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Time_Stamp_Time = " 
        << signaHeader->MR_Time_Stamp_Time);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Time_Stamp_Date = " 
        << signaHeader->MR_Time_Stamp_Date);

    valueOffset = 24;
    memcpy(&signaHeader->MR_Scan_Duration,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Scan_Duration);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Scan_Duration = " 
        << signaHeader->MR_Scan_Duration);

    valueOffset = 28;
    memcpy(&signaHeader->MR_Slice_Thickness,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Slice_Thickness);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Slice_Thickness = " 
        << signaHeader->MR_Slice_Thickness);

    valueOffset = 32;
    memcpy(&signaHeader->MR_Image_Matrix_Size_X,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Image_Matrix_Size_X);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Matrix_Size_X = " 
        << signaHeader->MR_Image_Matrix_Size_X);

    valueOffset = 34;
    memcpy(&signaHeader->MR_Image_Matrix_Size_Y,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Image_Matrix_Size_Y);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Matrix_Size_Y = " 
        << signaHeader->MR_Image_Matrix_Size_Y);

    valueOffset = 36;
    memcpy(&signaHeader->MR_FOV_X,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_FOV_X);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_FOV_X = " 
        << signaHeader->MR_FOV_X);

    valueOffset = 40;
    memcpy(&signaHeader->MR_FOV_Y,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_FOV_Y);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_FOV_Y = " 
        << signaHeader->MR_FOV_Y);

    valueOffset = 44;
    memcpy(&signaHeader->MR_Image_Dimension_X,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Image_Dimension_X);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Dimension_X = " 
        << signaHeader->MR_Image_Dimension_X);

    valueOffset = 48;
    memcpy(&signaHeader->MR_Image_Dimension_Y,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Image_Dimension_Y);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Dimension_Y = " 
        << signaHeader->MR_Image_Dimension_Y);

    valueOffset = 52;
    memcpy(&signaHeader->MR_Image_Pixel_Size_X,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Image_Pixel_Size_X);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Pixel_Size_X = " 
        << signaHeader->MR_Image_Pixel_Size_X);

    valueOffset = 56;
    memcpy(&signaHeader->MR_Image_Pixel_Size_Y,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Image_Pixel_Size_Y);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Pixel_Size_Y = " 
        << signaHeader->MR_Image_Pixel_Size_Y);

    valueOffset = 74;
    numCopy = 14;
    memcpy(signaHeader->MR_IV_Contrast_Agent,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_IV_Contrast_Agent = " 
        << signaHeader->MR_IV_Contrast_Agent);

    valueOffset = 91;
    numCopy = 17;
    memcpy(signaHeader->MR_Oral_Contrast_Agent,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_IV_Contrast_Agent = " 
        << signaHeader->MR_Oral_Contrast_Agent);

    valueOffset = 108;
    memcpy(&signaHeader->MR_Image_Contrast_Mode,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Image_Contrast_Mode);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Contrast_Mode = " 
        << hex << signaHeader->MR_Image_Contrast_Mode);

    valueOffset = 116;
    memcpy(&signaHeader->MR_Plane_Type,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Plane_Type);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Plane_Type = " 
        << hex << signaHeader->MR_Plane_Type);

    valueOffset = 120;
    memcpy(&signaHeader->MR_Slice_Spacing,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Slice_Spacing);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Slice_Spacing = " 
        << signaHeader->MR_Slice_Spacing);

    valueOffset = 132;
    memcpy(&signaHeader->MR_Image_Location,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Image_Location);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Location = " 
        << signaHeader->MR_Image_Location);

    valueOffset = 148;
    memcpy(&signaHeader->MR_R_Normal,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_R_Normal);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_R_Normal = " 
        << signaHeader->MR_R_Normal);

    valueOffset = 152;
    memcpy(&signaHeader->MR_A_Normal,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_A_Normal);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_A_Normal = " 
        << signaHeader->MR_A_Normal);

    valueOffset = 156;
    memcpy(&signaHeader->MR_S_Normal,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_S_Normal);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_S_Normal = " 
        << signaHeader->MR_S_Normal);

    valueOffset = 160;
    memcpy(&signaHeader->MR_R_Top_Left_Corner,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_R_Top_Left_Corner);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_R_Top_Left_Corner = " 
        << signaHeader->MR_R_Top_Left_Corner);

    valueOffset = 164;
    memcpy(&signaHeader->MR_A_Top_Left_Corner,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_A_Top_Left_Corner);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_A_Top_Left_Corner = " 
        << signaHeader->MR_A_Top_Left_Corner);

    valueOffset = 168;
    memcpy(&signaHeader->MR_S_Top_Left_Corner,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_S_Top_Left_Corner);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_S_Top_Left_Corner = " 
        << signaHeader->MR_S_Top_Left_Corner);

    valueOffset = 172;
    memcpy(&signaHeader->MR_R_Top_Right_Corner,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_R_Top_Right_Corner);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_R_Top_Right_Corner = " 
        << signaHeader->MR_R_Top_Right_Corner);

    valueOffset = 176;
    memcpy(&signaHeader->MR_A_Top_Right_Corner,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_A_Top_Right_Corner);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_A_Top_Right_Corner = " 
        << signaHeader->MR_A_Top_Right_Corner);

    valueOffset = 180;
    memcpy(&signaHeader->MR_S_Top_Right_Corner,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_S_Top_Right_Corner);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_S_Top_Right_Corner = " 
        << signaHeader->MR_S_Top_Right_Corner);

    valueOffset = 184;
    memcpy(&signaHeader->MR_R_Bottom_Right_Corner,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_R_Bottom_Right_Corner);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_R_Bottom_Right_Corner = " 
        << signaHeader->MR_R_Bottom_Right_Corner);

    valueOffset = 188;
    memcpy(&signaHeader->MR_A_Bottom_Right_Corner,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_A_Bottom_Right_Corner);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_A_Bottom_Right_Corner = " 
        << signaHeader->MR_A_Bottom_Right_Corner);

    valueOffset = 192;
    memcpy(&signaHeader->MR_S_Bottom_Right_Corner,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_S_Bottom_Right_Corner);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_S_Bottom_Right_Corner = " 
        << signaHeader->MR_S_Bottom_Right_Corner);

    valueOffset = 200;
    memcpy(&signaHeader->MR_Pulse_Repetition_Time,buffer+valueOffset,sizeof(unsigned int));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Pulse_Repetition_Time);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Pulse_Repetition_Time = " 
        << signaHeader->MR_Pulse_Repetition_Time);

    valueOffset = 204;
    memcpy(&signaHeader->MR_Pulse_Inversion_Time,buffer+valueOffset,sizeof(unsigned int));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Pulse_Inversion_Time);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Pulse_Inversion_Time = " 
        << signaHeader->MR_Pulse_Inversion_Time);

    valueOffset = 208;
    memcpy(&signaHeader->MR_Pulse_Echo_Time,buffer+valueOffset,sizeof(unsigned int));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Pulse_Echo_Time);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Pulse_Echo_Time = " 
        << signaHeader->MR_Pulse_Echo_Time);

    valueOffset = 216;
    memcpy(&signaHeader->MR_Number_Of_Echoes,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Number_Of_Echoes);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Number_Of_Echoes = " 
        << signaHeader->MR_Number_Of_Echoes);

    valueOffset = 218;
    memcpy(&signaHeader->MR_Echo_Number,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Echo_Number);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Echo_Number = " 
        << signaHeader->MR_Echo_Number);

    valueOffset = 224;
    memcpy(&signaHeader->MR_Number_Of_Averages,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Number_Of_Averages);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Number_Of_Averages = " 
        << signaHeader->MR_Number_Of_Averages);

    valueOffset = 230;
    memcpy(&signaHeader->MR_Cardiac_Heart_Rate,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Cardiac_Heart_Rate);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Cardiac_Heart_Rate = " 
        << signaHeader->MR_Cardiac_Heart_Rate);

    valueOffset = 236;
    memcpy(&signaHeader->MR_Average_SAR,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Average_SAR);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Average_SAR = " 
        << signaHeader->MR_Average_SAR);

    valueOffset = 246;
    memcpy(&signaHeader->MR_Trigger_Window,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Trigger_Window);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Trigger_Window = " 
        << signaHeader->MR_Trigger_Window);

    valueOffset = 252;
    memcpy(&signaHeader->MR_Images_Per_Cardiac_Cycle,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Images_Per_Cardiac_Cycle);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Images_Per_Cardiac_Cycle = " 
        << signaHeader->MR_Images_Per_Cardiac_Cycle);

    valueOffset = 260;
    memcpy(&signaHeader->MR_Flip_Angle,buffer+valueOffset,sizeof(short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Flip_Angle);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Flip_Angle = " 
        << signaHeader->MR_Flip_Angle);

    valueOffset = 288;
    memcpy(&signaHeader->MR_Center_Frequency,buffer+valueOffset,sizeof(int));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Center_Frequency);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Center_Frequency = " 
        << signaHeader->MR_Center_Frequency);

    valueOffset = 310;
    memcpy(&signaHeader->MR_Imaging_Mode,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Imaging_Mode);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Imaging_Mode = " 
        << signaHeader->MR_Imaging_Mode);

    valueOffset = 312;
    memcpy(&signaHeader->MR_Imaging_Options,buffer+valueOffset,sizeof(unsigned int));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Imaging_Options);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Imaging_Options = " 
        << hex << signaHeader->MR_Imaging_Options);

    valueOffset = 304;
    memcpy(&signaHeader->MR_Pulse_Sequence,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Pulse_Sequence);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Pulse_Sequence = " 
        << signaHeader->MR_Pulse_Sequence);

    valueOffset = 320;
    numCopy = 34;
    memcpy(signaHeader->MR_Pulse_Sequence_Name,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Pulse_Sequence_Name = " 
        << signaHeader->MR_Pulse_Sequence_Name);

    valueOffset = 376;
    numCopy = 18;
    memcpy(signaHeader->MR_Receive_Coil_Name,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Receive_Coil_Name = " 
        << signaHeader->MR_Receive_Coil_Name);

    valueOffset = 402;
    memcpy(&signaHeader->MR_Raw_Data_Run_Number,buffer+valueOffset,sizeof(int));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Raw_Data_Run_Number);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Raw_Data_Run_Number = " 
        << signaHeader->MR_Raw_Data_Run_Number);

    valueOffset = 410;
    memcpy(&signaHeader->MR_Fat_Water_SAT,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Fat_Water_SAT);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Fat_Water_SAT = " 
        << signaHeader->MR_Fat_Water_SAT);

    valueOffset = 412;
    memcpy(&signaHeader->MR_Variable_Bandwidth,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Variable_Bandwidth);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Variable_Bandwidth = " 
        << signaHeader->MR_Variable_Bandwidth);

    valueOffset = 416;
    memcpy(&signaHeader->MR_Number_Of_Slices,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Number_Of_Slices);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Number_Of_Slices = " 
        << signaHeader->MR_Number_Of_Slices);

    valueOffset = 540;
    memcpy(&signaHeader->MR_Timestamp_Of_Last_Change,buffer+valueOffset,sizeof(int));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Timestamp_Of_Last_Change);
    this->statTimeAndDateToAscii(&signaHeader->MR_Timestamp_Of_Last_Change, 
        signaHeader->MR_Timestamp_Of_Last_Change_Time, signaHeader->MR_Timestamp_Of_Last_Change_Date);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Timestamp_Of_Last_Change_Time = " 
        << signaHeader->MR_Timestamp_Of_Last_Change_Time);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Timestamp_Of_Last_Change_Date = " 
        << signaHeader->MR_Timestamp_Of_Last_Change_Date);

    valueOffset = 572;
    memcpy(&signaHeader->MR_Bitmap_Of_Saturation_Selections,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Bitmap_Of_Saturation_Selections);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Bitmap_Of_Saturation_Selections = " 
        << signaHeader->MR_Bitmap_Of_Saturation_Selections);

    valueOffset = 574;
    memcpy(&signaHeader->MR_Surface_Coil_Intensity_Correction,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Surface_Coil_Intensity_Correction);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Surface_Coil_Intensity_Correction = " 
        << signaHeader->MR_Surface_Coil_Intensity_Correction);

    valueOffset = 604;
    memcpy(&signaHeader->MR_Image_Type,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Image_Type);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Type = " 
        << signaHeader->MR_Image_Type);

    valueOffset = 606;
    memcpy(&signaHeader->MR_Vascular_Collapse,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Vascular_Collapse);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Vascular_Collapse = " 
        << signaHeader->MR_Vascular_Collapse);

    valueOffset = 616;
    memcpy(&signaHeader->MR_Projection_Algorithm,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Projection_Algorithm);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Projection_Algorithm = " 
        << signaHeader->MR_Projection_Algorithm);

    valueOffset = 660;
    memcpy(&signaHeader->MR_Echo_Train_Length,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Echo_Train_Length);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Echo_Train_Length = " 
        << signaHeader->MR_Echo_Train_Length);

    valueOffset = 662;
    memcpy(&signaHeader->MR_Fractional_Echo_Flag,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Fractional_Echo_Flag);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Fractional_Echo_Flag = " 
        << hex << signaHeader->MR_Fractional_Echo_Flag);

    valueOffset = 664;
    memcpy(&signaHeader->MR_Preparatory_Pulse_Option,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Preparatory_Pulse_Option);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Preparatory_Pulse_Option = " 
        << signaHeader->MR_Preparatory_Pulse_Option);

    valueOffset = 750;
    memcpy(&signaHeader->MR_Frequency_Direction,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Frequency_Direction);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Frequency_Direction = " 
        << signaHeader->MR_Frequency_Direction);
    
    // Delete the buffer and return the pointer to the header.
    // The function that receives the pointer must do memory
    // cleanup or a memory leak will occur. 
    delete[] buffer;

    return signaHeader;
}



