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


#include <svkGESigna5XReader.h>
#include </usr/include/vtk/vtkByteSwap.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkGlobFileNames.h>
#include </usr/include/vtk/vtkSortFileNames.h>
#include </usr/include/vtk/vtkInformation.h>
#include <svkIOD.h>
#include <svkEnhancedMRIIOD.h>
#include <svkMriImageData.h>
#include </usr/include/vtk/vtkMath.h>
#include </usr/include/vtk/vtkMatrix4x4.h>

#include <sys/stat.h>
#include <string>
#include <vector>
#include <sstream>
#include <time.h>
#include <math.h>

#define GE_SIGNA_5X_MAGIC_NUMBER  0x494d4746

using namespace svk;


//vtkCxxRevisionMacro(svkGESigna5XReader, "$Rev$");
vtkStandardNewMacro(svkGESigna5XReader);


/*!
 *
 */
svkGESigna5XReader::svkGESigna5XReader()
{
#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkGESigna5XReader");
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
svkGESigna5XReader::~svkGESigna5XReader()
{
    vtkDebugMacro(<<this->GetClassName() << "::~" << this->GetClassName() << "()");
    if( this->imageHeader != NULL ) {
        delete this->imageHeader;
        this->imageHeader = NULL;
    }
}


int svkGESigna5XReader::CanReadFile(const char* fname)
{ 
    FILE *fp = fopen(fname, "rb");
    if (!fp)
    {
        return 0;
    }
  
    int magic;
    fread(&magic, 4, 1, fp);
    vtkByteSwap::Swap4BE(&magic);
  
    if (magic != GE_SIGNA_5X_MAGIC_NUMBER) // "IMGF"
    {
        fclose(fp);
        return 0;
    }
    return 3;
}


#define GE_VERSION_FIX(var, value, version) \
if ( signaHeader->mainHeader.Version == version ) { \
    var =  value; \
} 


GESignaHeader* svkGESigna5XReader::ReadHeader(const char *FileNameToRead)
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
  
    if( !fread(&signaHeader->mainHeader, sizeof(GEImageMainHeader), 1, fp) ) {
        fclose(fp);
        delete signaHeader;
        vtkWarningWithObjectMacro(this, "Could not read main header!");
        return NULL;
    }
    vtkByteSwap::Swap4BERange(&signaHeader->mainHeader, 13);
    vtkByteSwap::Swap2BERange(&signaHeader->mainHeader.Version, 2);
    vtkByteSwap::Swap4BERange(&signaHeader->mainHeader.Pointer_ID, 25);
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
  
    if (signaHeader->mainHeader.Magic_Number != GE_SIGNA_5X_MAGIC_NUMBER) { // "IMGF"
        fclose(fp);
        delete signaHeader;
        vtkWarningWithObjectMacro(this, "Unknown file type! Not a GE ximg file!");
        return NULL;
    }

    // Accomodate as many versions of GE Signa 5X as possible.
    switch (signaHeader->mainHeader.Version) {
        case 2: // This version caused many GE Signa readers to crash
            // because the header is only 124 bytes long.  Need to 
            // manually set the file pointers for the header.
            signaHeader->mainHeader.Pointer_Suite = 124; // Version 3 is 2304
            signaHeader->mainHeader.Length_Suite = 116;  // Version 3 is 114
            signaHeader->mainHeader.Pointer_Exam = 240;  // Version 3 is 2418
            signaHeader->mainHeader.Length_Exam = 1040; // Version 3 is 1024
            signaHeader->mainHeader.Pointer_Series = 1280; // Version 3 is 3442
            signaHeader->mainHeader.Length_Series = 1028;  // Version 3 is 1020
            signaHeader->mainHeader.Pointer_Image = 2308;  // Version 3 is 4462
            signaHeader->mainHeader.Length_Image = 1044; // Don't know for sure?
            break;
        case 3: // This is the version that most GE Signa readers support.
            break;
        default:
            fclose(fp);
            delete signaHeader;
            vtkWarningWithObjectMacro(this, "This class cannot read this version of GE Signa 5X image!");
            return NULL;
    }
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

    valueOffset = 80;
    GE_VERSION_FIX(valueOffset,84,2)
    memcpy(&signaHeader->Exam_Magnet_Strength,buffer+valueOffset,sizeof(unsigned int));
    vtkByteSwap::Swap4BE(&signaHeader->Exam_Magnet_Strength);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Magnet_Strength = " 
        << signaHeader->Exam_Magnet_Strength);

    valueOffset = 84;
    GE_VERSION_FIX(valueOffset,88,2)
    numCopy = 13;
    memcpy(signaHeader->Exam_Patient_ID,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Patient_ID = " 
        << signaHeader->Exam_Patient_ID);

    valueOffset = 97;
    GE_VERSION_FIX(valueOffset,101,2)
    numCopy = 25;
    memcpy(signaHeader->Exam_Patient_Name,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Patient_Name = " 
        << signaHeader->Exam_Patient_Name);

    valueOffset = 122;
    GE_VERSION_FIX(valueOffset,126,2)
    memcpy(&signaHeader->Exam_Patient_Age,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->Exam_Patient_Age);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Patient_Age = " 
        << signaHeader->Exam_Patient_Age);

    valueOffset = 124;
    GE_VERSION_FIX(valueOffset,128,2)
    memcpy(&signaHeader->Exam_Patient_Age_Notation,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->Exam_Patient_Age_Notation);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Patient_Age_Notation = " 
        << signaHeader->Exam_Patient_Age_Notation);

    valueOffset = 126;
    GE_VERSION_FIX(valueOffset,130,2)
    memcpy(&signaHeader->Exam_Patient_Sex,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->Exam_Patient_Sex);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Patient_Sex = " 
        << signaHeader->Exam_Patient_Sex);

    valueOffset = 128;
    GE_VERSION_FIX(valueOffset,132,2)
    memcpy(&signaHeader->Exam_Patient_Weight_In_Grams,buffer+valueOffset,sizeof(unsigned int));
    vtkByteSwap::Swap4BE(&signaHeader->Exam_Patient_Weight_In_Grams);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Patient_Weight_In_Grams = " 
        << signaHeader->Exam_Patient_Weight_In_Grams);

    valueOffset = 134;
    numCopy = 61;
    GE_VERSION_FIX(valueOffset,138,2)
    GE_VERSION_FIX(numCopy,62,2)
    memcpy(signaHeader->Exam_Patient_History,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Patient_History = " 
        << signaHeader->Exam_Patient_History);

    valueOffset = 195;
    numCopy = 13;
    GE_VERSION_FIX(valueOffset,200,2)
    GE_VERSION_FIX(numCopy,12,2)
    memcpy(signaHeader->Exam_Requisition_Number,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Requisition_Number = " 
        << signaHeader->Exam_Requisition_Number);

    valueOffset = 208;
    GE_VERSION_FIX(valueOffset,212,2)
    memcpy(&signaHeader->Exam_Time_Stamp,buffer+valueOffset,sizeof(int));
    vtkByteSwap::Swap4BE(&signaHeader->Exam_Time_Stamp);
    this->statTimeAndDateToAscii(&signaHeader->Exam_Time_Stamp, 
        signaHeader->Exam_Time_Stamp_Time, signaHeader->Exam_Time_Stamp_Date);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Time_Stamp_Time = " 
        << signaHeader->Exam_Time_Stamp_Time);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Time_Stamp_Date = " 
        << signaHeader->Exam_Time_Stamp_Date);

    valueOffset = 212;
    numCopy = 33;
    GE_VERSION_FIX(valueOffset,216,2)
    memcpy(signaHeader->Exam_Referring_Physician,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Referring_Physician = " 
        << signaHeader->Exam_Referring_Physician);

    valueOffset = 245;
    numCopy = 33;
    GE_VERSION_FIX(valueOffset,249,2)
    memcpy(signaHeader->Exam_Radiologist,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Radiologist = " 
        << signaHeader->Exam_Radiologist);

    valueOffset = 278;
    numCopy = 4;
    GE_VERSION_FIX(valueOffset,282,2)
    memcpy(signaHeader->Exam_Operator,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Operator = " 
        << signaHeader->Exam_Operator);

    valueOffset = 282;
    numCopy = 23;
    GE_VERSION_FIX(valueOffset,286,2)
    memcpy(signaHeader->Exam_Description,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Description = " 
        << signaHeader->Exam_Description);

    valueOffset = 305;
    numCopy = 3;
    GE_VERSION_FIX(valueOffset,309,2)
    memcpy(signaHeader->Exam_Type,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Type = " 
        << signaHeader->Exam_Type);

    valueOffset = 352;
    numCopy = 2;
    GE_VERSION_FIX(valueOffset,364,2)
    memcpy(signaHeader->Exam_Software_Version,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Software_Version = " 
        << signaHeader->Exam_Software_Version);

    valueOffset = 438;
    numCopy = 16;
    GE_VERSION_FIX(valueOffset,450,2)
    memcpy(signaHeader->Exam_Unique_System_ID,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->Exam_Unique_System_ID = " 
        << signaHeader->Exam_Unique_System_ID);

    // Done with exam, delete buffer.
    delete[] buffer;
    buffer = NULL;
    
    // Check if this is a MR file.  If not exit!
    string examType(signaHeader->Exam_Type);
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

    valueOffset = 14;
    GE_VERSION_FIX(valueOffset,16,2)
    memcpy(&signaHeader->MR_Time_Stamp,buffer+valueOffset,sizeof(int));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Time_Stamp);
    this->statTimeAndDateToAscii(&signaHeader->MR_Time_Stamp, 
        signaHeader->MR_Time_Stamp_Time, signaHeader->MR_Time_Stamp_Date);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Time_Stamp_Time = " 
        << signaHeader->MR_Time_Stamp_Time);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Time_Stamp_Date = " 
        << signaHeader->MR_Time_Stamp_Date);

    valueOffset = 22;
    GE_VERSION_FIX(valueOffset,24,2)
    memcpy(&signaHeader->MR_Scan_Duration,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Scan_Duration);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Scan_Duration = " 
        << signaHeader->MR_Scan_Duration);

    valueOffset = 26;
    GE_VERSION_FIX(valueOffset,28,2)
    memcpy(&signaHeader->MR_Slice_Thickness,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Slice_Thickness);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Slice_Thickness = " 
        << signaHeader->MR_Slice_Thickness);

    valueOffset = 30;
    GE_VERSION_FIX(valueOffset,32,2)
    memcpy(&signaHeader->MR_Image_Matrix_Size_X,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Image_Matrix_Size_X);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Matrix_Size_X = " 
        << signaHeader->MR_Image_Matrix_Size_X);

    valueOffset = 32;
    GE_VERSION_FIX(valueOffset,34,2)
    memcpy(&signaHeader->MR_Image_Matrix_Size_Y,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Image_Matrix_Size_Y);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Matrix_Size_Y = " 
        << signaHeader->MR_Image_Matrix_Size_Y);

    valueOffset = 34;
    GE_VERSION_FIX(valueOffset,36,2)
    memcpy(&signaHeader->MR_FOV_X,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_FOV_X);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_FOV_X = " 
        << signaHeader->MR_FOV_X);

    valueOffset = 38;
    GE_VERSION_FIX(valueOffset,40,2)
    memcpy(&signaHeader->MR_FOV_Y,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_FOV_Y);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_FOV_Y = " 
        << signaHeader->MR_FOV_Y);

    valueOffset = 42;
    GE_VERSION_FIX(valueOffset,44,2)
    memcpy(&signaHeader->MR_Image_Dimension_X,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Image_Dimension_X);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Dimension_X = " 
        << signaHeader->MR_Image_Dimension_X);

    valueOffset = 46;
    GE_VERSION_FIX(valueOffset,48,2)
    memcpy(&signaHeader->MR_Image_Dimension_Y,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Image_Dimension_Y);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Dimension_Y = " 
        << signaHeader->MR_Image_Dimension_Y);

    valueOffset = 50;
    GE_VERSION_FIX(valueOffset,52,2)
    memcpy(&signaHeader->MR_Image_Pixel_Size_X,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Image_Pixel_Size_X);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Pixel_Size_X = " 
        << signaHeader->MR_Image_Pixel_Size_X);

    valueOffset = 54;
    GE_VERSION_FIX(valueOffset,56,2)
    memcpy(&signaHeader->MR_Image_Pixel_Size_Y,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Image_Pixel_Size_Y);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Pixel_Size_Y = " 
        << signaHeader->MR_Image_Pixel_Size_Y);

    valueOffset = 72;
    numCopy = 14;
    GE_VERSION_FIX(valueOffset,74,2)
    memcpy(signaHeader->MR_IV_Contrast_Agent,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_IV_Contrast_Agent = " 
        << signaHeader->MR_IV_Contrast_Agent);

    valueOffset = 89;
    numCopy = 17;
    GE_VERSION_FIX(valueOffset,91,2)
    memcpy(signaHeader->MR_Oral_Contrast_Agent,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_IV_Contrast_Agent = " 
        << signaHeader->MR_Oral_Contrast_Agent);

    valueOffset = 106;
    GE_VERSION_FIX(valueOffset,108,2)
    memcpy(&signaHeader->MR_Image_Contrast_Mode,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Image_Contrast_Mode);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Contrast_Mode = " 
        << hex << signaHeader->MR_Image_Contrast_Mode);

    valueOffset = 114;
    GE_VERSION_FIX(valueOffset,116,2)
    memcpy(&signaHeader->MR_Plane_Type,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Plane_Type);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Plane_Type = " 
        << hex << signaHeader->MR_Plane_Type);

    valueOffset = 116;
    GE_VERSION_FIX(valueOffset,120,2)
    memcpy(&signaHeader->MR_Slice_Spacing,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Slice_Spacing);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Slice_Spacing = " 
        << signaHeader->MR_Slice_Spacing);

    valueOffset = 126;
    GE_VERSION_FIX(valueOffset,132,2)
    memcpy(&signaHeader->MR_Image_Location,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Image_Location);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Location = " 
        << signaHeader->MR_Image_Location);

    valueOffset = 142;
    GE_VERSION_FIX(valueOffset,148,2)
    memcpy(&signaHeader->MR_R_Normal,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_R_Normal);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_R_Normal = " 
        << signaHeader->MR_R_Normal);

    valueOffset = 146;
    GE_VERSION_FIX(valueOffset,152,2)
    memcpy(&signaHeader->MR_A_Normal,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_A_Normal);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_A_Normal = " 
        << signaHeader->MR_A_Normal);

    valueOffset = 150;
    GE_VERSION_FIX(valueOffset,156,2)
    memcpy(&signaHeader->MR_S_Normal,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_S_Normal);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_S_Normal = " 
        << signaHeader->MR_S_Normal);

    valueOffset = 154;
    GE_VERSION_FIX(valueOffset,160,2)
    memcpy(&signaHeader->MR_R_Top_Left_Corner,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_R_Top_Left_Corner);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_R_Top_Left_Corner = " 
        << signaHeader->MR_R_Top_Left_Corner);

    valueOffset = 158;
    GE_VERSION_FIX(valueOffset,164,2)
    memcpy(&signaHeader->MR_A_Top_Left_Corner,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_A_Top_Left_Corner);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_A_Top_Left_Corner = " 
        << signaHeader->MR_A_Top_Left_Corner);

    valueOffset = 162;
    GE_VERSION_FIX(valueOffset,168,2)
    memcpy(&signaHeader->MR_S_Top_Left_Corner,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_S_Top_Left_Corner);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_S_Top_Left_Corner = " 
        << signaHeader->MR_S_Top_Left_Corner);

    valueOffset = 166;
    GE_VERSION_FIX(valueOffset,172,2)
    memcpy(&signaHeader->MR_R_Top_Right_Corner,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_R_Top_Right_Corner);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_R_Top_Right_Corner = " 
        << signaHeader->MR_R_Top_Right_Corner);

    valueOffset = 170;
    GE_VERSION_FIX(valueOffset,176,2)
    memcpy(&signaHeader->MR_A_Top_Right_Corner,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_A_Top_Right_Corner);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_A_Top_Right_Corner = " 
        << signaHeader->MR_A_Top_Right_Corner);

    valueOffset = 174;
    GE_VERSION_FIX(valueOffset,180,2)
    memcpy(&signaHeader->MR_S_Top_Right_Corner,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_S_Top_Right_Corner);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_S_Top_Right_Corner = " 
        << signaHeader->MR_S_Top_Right_Corner);

    valueOffset = 178;
    GE_VERSION_FIX(valueOffset,184,2)
    memcpy(&signaHeader->MR_R_Bottom_Right_Corner,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_R_Bottom_Right_Corner);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_R_Bottom_Right_Corner = " 
        << signaHeader->MR_R_Bottom_Right_Corner);

    valueOffset = 182;
    GE_VERSION_FIX(valueOffset,188,2)
    memcpy(&signaHeader->MR_A_Bottom_Right_Corner,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_A_Bottom_Right_Corner);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_A_Bottom_Right_Corner = " 
        << signaHeader->MR_A_Bottom_Right_Corner);

    valueOffset = 186;
    GE_VERSION_FIX(valueOffset,192,2)
    memcpy(&signaHeader->MR_S_Bottom_Right_Corner,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_S_Bottom_Right_Corner);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_S_Bottom_Right_Corner = " 
        << signaHeader->MR_S_Bottom_Right_Corner);

    valueOffset = 194;
    GE_VERSION_FIX(valueOffset,200,2)
    memcpy(&signaHeader->MR_Pulse_Repetition_Time,buffer+valueOffset,sizeof(unsigned int));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Pulse_Repetition_Time);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Pulse_Repetition_Time = " 
        << signaHeader->MR_Pulse_Repetition_Time);

    valueOffset = 198;
    GE_VERSION_FIX(valueOffset,204,2)
    memcpy(&signaHeader->MR_Pulse_Inversion_Time,buffer+valueOffset,sizeof(unsigned int));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Pulse_Inversion_Time);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Pulse_Inversion_Time = " 
        << signaHeader->MR_Pulse_Inversion_Time);

    valueOffset = 202;
    GE_VERSION_FIX(valueOffset,208,2)
    memcpy(&signaHeader->MR_Pulse_Echo_Time,buffer+valueOffset,sizeof(unsigned int));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Pulse_Echo_Time);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Pulse_Echo_Time = " 
        << signaHeader->MR_Pulse_Echo_Time);

    valueOffset = 210;
    GE_VERSION_FIX(valueOffset,216,2)
    memcpy(&signaHeader->MR_Number_Of_Echoes,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Number_Of_Echoes);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Number_Of_Echoes = " 
        << signaHeader->MR_Number_Of_Echoes);

    valueOffset = 212;
    GE_VERSION_FIX(valueOffset,218,2)
    memcpy(&signaHeader->MR_Echo_Number,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Echo_Number);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Echo_Number = " 
        << signaHeader->MR_Echo_Number);

    valueOffset = 218;
    GE_VERSION_FIX(valueOffset,224,2)
    memcpy(&signaHeader->MR_Number_Of_Averages,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Number_Of_Averages);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Number_Of_Averages = " 
        << signaHeader->MR_Number_Of_Averages);

    valueOffset = 224;
    GE_VERSION_FIX(valueOffset,230,2)
    memcpy(&signaHeader->MR_Cardiac_Heart_Rate,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Cardiac_Heart_Rate);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Cardiac_Heart_Rate = " 
        << signaHeader->MR_Cardiac_Heart_Rate);

    valueOffset = 230;
    GE_VERSION_FIX(valueOffset,236,2)
    memcpy(&signaHeader->MR_Average_SAR,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Average_SAR);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Average_SAR = " 
        << signaHeader->MR_Average_SAR);

    valueOffset = 240;
    GE_VERSION_FIX(valueOffset,246,2)
    memcpy(&signaHeader->MR_Trigger_Window,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Trigger_Window);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Trigger_Window = " 
        << signaHeader->MR_Trigger_Window);

    valueOffset = 246;
    GE_VERSION_FIX(valueOffset,252,2)
    memcpy(&signaHeader->MR_Images_Per_Cardiac_Cycle,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Images_Per_Cardiac_Cycle);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Images_Per_Cardiac_Cycle = " 
        << signaHeader->MR_Images_Per_Cardiac_Cycle);

    valueOffset = 254;
    GE_VERSION_FIX(valueOffset,260,2)
    memcpy(&signaHeader->MR_Flip_Angle,buffer+valueOffset,sizeof(short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Flip_Angle);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Flip_Angle = " 
        << signaHeader->MR_Flip_Angle);

    valueOffset = 278;
    GE_VERSION_FIX(valueOffset,288,2)
    memcpy(&signaHeader->MR_Center_Frequency,buffer+valueOffset,sizeof(int));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Center_Frequency);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Center_Frequency = " 
        << signaHeader->MR_Center_Frequency);

    valueOffset = 298;
    GE_VERSION_FIX(valueOffset,310,2)
    memcpy(&signaHeader->MR_Imaging_Mode,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Imaging_Mode);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Imaging_Mode = " 
        << signaHeader->MR_Imaging_Mode);

    valueOffset = 300;
    GE_VERSION_FIX(valueOffset,312,2)
    memcpy(&signaHeader->MR_Imaging_Options,buffer+valueOffset,sizeof(unsigned int));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Imaging_Options);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Imaging_Options = " 
        << hex << signaHeader->MR_Imaging_Options);

    valueOffset = 304;
    GE_VERSION_FIX(valueOffset,316,2)
    memcpy(&signaHeader->MR_Pulse_Sequence,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Pulse_Sequence);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Pulse_Sequence = " 
        << signaHeader->MR_Pulse_Sequence);

    valueOffset = 308;
    numCopy = 34;
    GE_VERSION_FIX(valueOffset,320,2)
    memcpy(signaHeader->MR_Pulse_Sequence_Name,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Pulse_Sequence_Name = " 
        << signaHeader->MR_Pulse_Sequence_Name);

    valueOffset = 362;
    numCopy = 18;
    GE_VERSION_FIX(valueOffset,376,2)
    memcpy(signaHeader->MR_Receive_Coil_Name,buffer+valueOffset,numCopy);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Receive_Coil_Name = " 
        << signaHeader->MR_Receive_Coil_Name);

    valueOffset = 384;
    GE_VERSION_FIX(valueOffset,402,2)
    memcpy(&signaHeader->MR_Raw_Data_Run_Number,buffer+valueOffset,sizeof(int));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Raw_Data_Run_Number);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Raw_Data_Run_Number = " 
        << signaHeader->MR_Raw_Data_Run_Number);

    valueOffset = 392;
    GE_VERSION_FIX(valueOffset,410,2)
    memcpy(&signaHeader->MR_Fat_Water_SAT,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Fat_Water_SAT);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Fat_Water_SAT = " 
        << signaHeader->MR_Fat_Water_SAT);

    valueOffset = 394;
    GE_VERSION_FIX(valueOffset,412,2)
    memcpy(&signaHeader->MR_Variable_Bandwidth,buffer+valueOffset,sizeof(float));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Variable_Bandwidth);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Variable_Bandwidth = " 
        << signaHeader->MR_Variable_Bandwidth);

    valueOffset = 398;
    GE_VERSION_FIX(valueOffset,416,2)
    memcpy(&signaHeader->MR_Number_Of_Slices,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Number_Of_Slices);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Number_Of_Slices = " 
        << signaHeader->MR_Number_Of_Slices);

    valueOffset = 520;
    GE_VERSION_FIX(valueOffset,540,2)
    memcpy(&signaHeader->MR_Timestamp_Of_Last_Change,buffer+valueOffset,sizeof(int));
    vtkByteSwap::Swap4BE(&signaHeader->MR_Timestamp_Of_Last_Change);
    this->statTimeAndDateToAscii(&signaHeader->MR_Timestamp_Of_Last_Change, 
        signaHeader->MR_Timestamp_Of_Last_Change_Time, signaHeader->MR_Timestamp_Of_Last_Change_Date);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Timestamp_Of_Last_Change_Time = " 
        << signaHeader->MR_Timestamp_Of_Last_Change_Time);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Timestamp_Of_Last_Change_Date = " 
        << signaHeader->MR_Timestamp_Of_Last_Change_Date);

    valueOffset = 552;
    GE_VERSION_FIX(valueOffset,572,2)
    memcpy(&signaHeader->MR_Bitmap_Of_Saturation_Selections,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Bitmap_Of_Saturation_Selections);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Bitmap_Of_Saturation_Selections = " 
        << signaHeader->MR_Bitmap_Of_Saturation_Selections);

    valueOffset = 554;
    GE_VERSION_FIX(valueOffset,574,2)
    memcpy(&signaHeader->MR_Surface_Coil_Intensity_Correction,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Surface_Coil_Intensity_Correction);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Surface_Coil_Intensity_Correction = " 
        << signaHeader->MR_Surface_Coil_Intensity_Correction);

    valueOffset = 584;
    GE_VERSION_FIX(valueOffset,604,2)
    memcpy(&signaHeader->MR_Image_Type,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Image_Type);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Image_Type = " 
        << signaHeader->MR_Image_Type);

    valueOffset = 586;
    GE_VERSION_FIX(valueOffset,606,2)
    memcpy(&signaHeader->MR_Vascular_Collapse,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Vascular_Collapse);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Vascular_Collapse = " 
        << signaHeader->MR_Vascular_Collapse);

    valueOffset = 596;
    GE_VERSION_FIX(valueOffset,616,2)
    memcpy(&signaHeader->MR_Projection_Algorithm,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Projection_Algorithm);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Projection_Algorithm = " 
        << signaHeader->MR_Projection_Algorithm);

    valueOffset = 640;
    GE_VERSION_FIX(valueOffset,660,2)
    memcpy(&signaHeader->MR_Echo_Train_Length,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Echo_Train_Length);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Echo_Train_Length = " 
        << signaHeader->MR_Echo_Train_Length);

    valueOffset = 642;
    GE_VERSION_FIX(valueOffset,662,2)
    memcpy(&signaHeader->MR_Fractional_Echo_Flag,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Fractional_Echo_Flag);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Fractional_Echo_Flag = " 
        << hex << signaHeader->MR_Fractional_Echo_Flag);

    valueOffset = 644;
    GE_VERSION_FIX(valueOffset,664,2)
    memcpy(&signaHeader->MR_Preparatory_Pulse_Option,buffer+valueOffset,sizeof(unsigned short));
    vtkByteSwap::Swap2BE(&signaHeader->MR_Preparatory_Pulse_Option);
    vtkDebugMacro( << this->GetClassName() << "::ReadHeader(): signaHeader->MR_Preparatory_Pulse_Option = " 
        << signaHeader->MR_Preparatory_Pulse_Option);

    valueOffset = 742;
    GE_VERSION_FIX(valueOffset,750,2)
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

void svkGESigna5XReader::statTimeAndDateToAscii (void *clock, char *time, char *date)
{
  char *asciiTime;
  unsigned int i;
  struct tm * timeinfo;
#ifdef SGI
  timespec_t *lclock;
#else

#endif

#ifdef SGI
  lclock = (timespec_t *) clock;
  asciiTime = ctime (&(lclock->tv_sec));
  timeinfo = gmtime(&(lclock->tv_sec));
#else
  time_t tclock = (time_t) *((int *) clock);
  asciiTime = ctime (&tclock);
  timeinfo = gmtime(&tclock);
#endif
  vtkDebugMacro( << this->GetClassName() << "::statTimeAndDateToAscii(): asciiTime = " 
        << asciiTime);
  
  strftime(time,7,"%H%M%S",timeinfo);
  strftime(date,9,"%Y%m%d",timeinfo);

}

/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called before ExecuteData()
 */
void svkGESigna5XReader::ExecuteInformation()
{

    vtkDebugMacro( << this->GetClassName() << "::ExecuteInformation()" );

    if (this->FileName == NULL) {
        return;
    }

    if ( this->FileName ) {

        struct stat fs;
        if ( stat(this->FileName, &fs) ) {
            vtkErrorMacro("Unable to open file " << this->FileName );
            return;
        }

        this->InitDcmHeader();
        this->numFrames = this->GetOutput()->GetDcmHeader()->GetIntValue( "NumberOfFrames");
        if (this->numFrames > 1) {
            double origin0[3];
            this->GetOutput()->GetDcmHeader()->GetOrigin(origin0, 0);
            double origin1[3];
            this->GetOutput()->GetDcmHeader()->GetOrigin(origin1, this->numFrames-1);

            //  Determine whether the data is ordered with or against the slice normal direction.
            double normal[3];
            this->GetOutput()->GetDcmHeader()->GetNormalVector(normal);
       
            //  Get vector from first to last image and get the dot product of that vector with the normal:
            double dcosSliceOrder[3];
            for (int j = 0; j < 3; j++) {
                dcosSliceOrder[j] =  origin1[j] - origin0[j];
            }
       
            //  Use the scalar product to determine whether the data in the .cmplx
            //  file is ordered along the slice normal or antiparalle to it.
            vtkMath* math = vtkMath::New();
            if (math->Dot(normal, dcosSliceOrder) > 0 ) {
                this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
            } else {
                this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_NEG_NORMAL;
            }
            math->Delete();
        } else {
            this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
        }

        double dcos[3][3];
        this->GetOutput()->GetDcmHeader()->SetSliceOrder( this->dataSliceOrder );

        this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
        this->GetOutput()->SetDcos(dcos);

        //  SetNumberOfIncrements is supposed to call this, but only works if the data has already
        //  been allocated. but that requires the number of components to be specified.
        this->GetOutput()->GetIncrements();
        this->SetupOutputInformation();

    }

}

bool svkGESigna5XReader::CopyGenesisImage(FILE *infp, int width, int height, int compress,
                         short *map_left, short *map_wide,
                         unsigned short *output)
{
    unsigned short row;
    unsigned short last_pixel=0;
    for (row=0; row<height; ++row) {
        unsigned short j;
        unsigned short start;
        unsigned short end;
      
        if (compress == 2 || compress == 4) { // packed/compacked
            start=map_left[row];
            end=start+map_wide[row];
        } else {
            start=0;
            end=width;
        }
        // Pad the first "empty" part of the line ...
        for (j=0; j<start; j++) {
            (*output) = 0;
            ++output;
        }

        if (compress == 3 || compress == 4) { // compressed/compacked
            while (start<end) {
                unsigned char byte;
                if (!fread(&byte,1,1,infp)) {
                    return false;
                }
                if (byte & 0x80) {
                    unsigned char byte2;
                    if (!fread(&byte2,1,1,infp)) {
                        return false;
                    }
                    if (byte & 0x40) {      // next word
                        if (!fread(&byte,1,1,infp)) {
                            return false;
                        }
                        last_pixel=
                          (((unsigned short)byte2<<8)+byte);
                    } else {                  // 14 bit delta
                        if (byte & 0x20) {
                            byte|=0xe0;
                        } else {
                            byte&=0x1f;
                        }
                        last_pixel+=
                          (((short)byte<<8)+byte2);
                    }
                } else {                          // 7 bit delta
                    if (byte & 0x40) {
                        byte|=0xc0;
                    }
                    last_pixel+=(signed char)byte;
                }
                (*output) = last_pixel;
                ++output;
                ++start;
            }
        } else {
            while (start<end) {
                unsigned short u;
                if (!fread(&u,2,1,infp)) {
                    return false;
                }
                vtkByteSwap::Swap2BE(&u);
                (*output) = u;
                ++output;
                ++start;
            }
        }
      
        // Pad the last "empty" part of the line ...
        for (j=end; j<width; j++) {
            (*output) = 0;
            ++output;
        }
    }
    return true;
}


bool svkGESigna5XReader::LoadData(const char *filename, unsigned short *outPtr, 
                             int *outExt, vtkIdType *)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        vtkGenericWarningMacro(<<"Couldn't open file: " << filename);
        return false;
    }

    int magic;
    bool isLX2 = false;
    if( !fread(&magic, 4, 1, fp) ) {
        vtkGenericWarningMacro(<<"Error reading file: " << filename);
        fclose(fp);
        return false;
    }
    vtkByteSwap::Swap4BE(&magic);
  
    if (magic != GE_SIGNA_5X_MAGIC_NUMBER) {
        fseek(fp, 3228, SEEK_SET);  // Could be an LX2!
        if( !fread(&magic, 4, 1, fp) ) {
            vtkGenericWarningMacro(<<"Error reading file: " << filename);
            fclose(fp);
            return false;
        }
        vtkByteSwap::Swap4BE(&magic);
        if (magic != GE_SIGNA_5X_MAGIC_NUMBER) {
            vtkGenericWarningMacro(<<"Unknown GE file type: " << filename);
            fclose(fp);
            return false;
        } else {
            isLX2 = true;
        }
    }

    // read in the pixel offset from the header
    int offset;
    if( !fread(&offset, 4, 1, fp) ) {
        vtkGenericWarningMacro(<<"Error reading file: " << filename);
        fclose(fp);
        return false;
    } 
    vtkByteSwap::Swap4BE(&offset);
    if ( isLX2 ) {
        offset += 3228;
    }

    int width, height, depth;
    if( !fread(&width, 4, 1, fp) ) {
        vtkGenericWarningMacro(<<"Error reading file: " << filename);
        fclose(fp);
        return false;
    } 
    vtkByteSwap::Swap4BE(&width);
    if( !fread(&height, 4, 1, fp) ) {
        vtkGenericWarningMacro(<<"Error reading file: " << filename);
        fclose(fp);
        return false;
    } 
    vtkByteSwap::Swap4BE(&height);
    // depth in bits
    if( !fread(&depth, 4, 1, fp) ) {
        vtkGenericWarningMacro(<<"Error reading file: " << filename);
        fclose(fp);
        return false;
    } 
    vtkByteSwap::Swap4BE(&depth);

    int compression;
    if( !fread(&compression, 4, 1, fp) ) {
        vtkGenericWarningMacro(<<"Error reading file: " << filename);
        fclose(fp);
        return false;
    } 
    vtkByteSwap::Swap4BE(&compression);

    short *leftMap = 0;
    short *widthMap = 0;

    if (compression == 2 || compression == 4) { // packed/compacked
        leftMap = new short [height];
        widthMap = new short [height];

        fseek(fp, 64, SEEK_SET);
        int packHdrOffset;
        if( !fread(&packHdrOffset, 4, 1, fp) ) {
            vtkGenericWarningMacro(<<"Error reading file: " << filename);
            delete [] leftMap;
            delete [] widthMap;
            fclose(fp);
            return false;
        }
        vtkByteSwap::Swap4BE(&packHdrOffset);
      
        // now seek to the pack header and read some values
        fseek(fp, packHdrOffset, SEEK_SET);
        // read in the maps
        int i;
        for (i = 0; i < height; i++) {
            if( !fread(leftMap+i, 2, 1, fp) ) {
                vtkGenericWarningMacro(<<"Error reading file: " << filename);
                delete [] leftMap;
                delete [] widthMap;
                fclose(fp);
                return false;
            }
            vtkByteSwap::Swap2BE(leftMap+i);
            if( !fread(widthMap+i, 2, 1, fp) ) {
                vtkGenericWarningMacro(<<"Error reading file: " << filename);
                delete [] leftMap;
                delete [] widthMap;
                fclose(fp);
                return false;
            }
            vtkByteSwap::Swap2BE(widthMap+i);
        }
    }

    // seek to pixel data
    fseek(fp, offset, SEEK_SET);

    // read in the pixels
    unsigned short *tmp = new unsigned short [width*height];
    int *dext = this->GetDataExtent();
    if (! this->CopyGenesisImage(fp, dext[1] + 1, dext[3] + 1, 
                      compression, leftMap, widthMap, tmp) ) {
        vtkGenericWarningMacro(<<"Error copying image");
        delete [] tmp;
        if (leftMap) {
            delete [] leftMap;
        }
        if (widthMap) {
            delete [] widthMap;
        }
        fclose(fp);
        return false;
    }
    memcpy(outPtr,tmp,2*width*height);

    delete [] tmp;
    if (leftMap) {
        delete [] leftMap;
    }
    if (widthMap) {
        delete [] widthMap;
    }
    fclose(fp);
    return true;
}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called after ExecuteInformation()
 */
void svkGESigna5XReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
{
    vtkIdType outIncr[3];
    int outExtent[6];

    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );

    vtkImageData *data = this->AllocateOutputData(output, outInfo);

    if ( this->GetFileNames()->GetNumberOfValues() < 1 ) {
        vtkErrorMacro("No files are available to read");
        return;
    }

    data->GetPointData()->GetScalars()->SetName("GESignaImage");

    this->ComputeDataIncrements();
  
    unsigned short *outPtr = (unsigned short *)data->GetScalarPointer();
    data->GetExtent(outExtent);
    data->GetIncrements(outIncr);
    int idx2;
    for (idx2 = 0; idx2 < this->GetFileNames()->GetNumberOfValues(); idx2++) {
        // read in file
        if( !this->LoadData(this->GetFileNames()->GetValue( idx2 ), outPtr, outExtent, outIncr) ) {
            vtkErrorMacro("Error loading data");
            return;
        }
        this->UpdateProgress((idx2 - outExtent[4])/
                         (outExtent[5] - outExtent[4] + 1.0));
        outPtr += outIncr[2];
    }

}


bool svkGESigna5XReader::CalculateTopLeftHandCornerAndRowColumnAndNormalVectors(GESignaHeader* hdr, 
    double tlhc[3], double row[3], double column[3], double normal[3])
{
    double tlhc_temp[4];
    double row_temp[4];
    double column_temp[4];
    double trhc[3];
    double brhc[3];
    if ( hdr == NULL || tlhc == NULL || row == NULL || column == NULL || normal == NULL) {
        return false;
    }

    // Need to convert from Genesis RAS coordinate system to DICOM LPS coordinate system.
    // Initialize the top left hand corner (TLHC).
    tlhc_temp[0] = -hdr->MR_R_Top_Left_Corner;
    tlhc_temp[1] = -hdr->MR_A_Top_Left_Corner;
    tlhc_temp[2] = hdr->MR_S_Top_Left_Corner;
    tlhc_temp[3] = 1.0;
    
    // Initialize top right hand corner (TRHC).
    trhc[0] = -hdr->MR_R_Top_Right_Corner;
    trhc[1] = -hdr->MR_A_Top_Right_Corner;
    trhc[2] = hdr->MR_S_Top_Right_Corner;

    // Initialize bottom right hand corner (BRHC).
    brhc[0] = -hdr->MR_R_Bottom_Right_Corner;
    brhc[1] = -hdr->MR_A_Bottom_Right_Corner;
    brhc[2] = hdr->MR_S_Bottom_Right_Corner;
 
    //Column vector is  BRHC-TRHC.
    column_temp[0] = brhc[0]-trhc[0];
    column_temp[1] = brhc[1]-trhc[1];
    column_temp[2] = brhc[2]-trhc[2];
    column_temp[3] = 1.0;

    // Row vector is TRHC-TLHC.
    row_temp[0] = trhc[0]-tlhc_temp[0];
    row_temp[1] = trhc[1]-tlhc_temp[1];
    row_temp[2] = trhc[2]-tlhc_temp[2];
    row_temp[3] = 1.0;

    // Now account for patient entry and position.  
    // If not head first and supine, then we need to 
    // rotate things around (used dicom3tools as a guide).
	// NOTE: VTK 5.4 does not have the vtkMatrix3x3 class,
	// so use the vtkMatrix4x4 class instead.
    vtkMatrix4x4 *rotations = NULL;
    switch (hdr->Series_Patient_Entry) {
        case 1: // Head first, do nothing
            break;
        case 2: // Feet first, rotate points by -180 degrees in the y direction
            rotations = vtkMatrix4x4::New();
            rotations->Identity();
            rotations->SetElement(0,0,-1.0);
            rotations->SetElement(2,2,-1.0);
            break;
        default: // Unknown, assume head first
            break;
    }
    switch (hdr->Series_Patient_Position) {
        case 1: // Supine, do nothing.
            break;
        case 2: // Prone, rotate points by -180 in the z direction.
            {
                if ( rotations == NULL ) {
                    rotations = vtkMatrix4x4::New();
                    rotations->Identity();
                    rotations->SetElement(0,0,-1.0);
                    rotations->SetElement(1,1,-1.0);
                } else {
                    vtkMatrix4x4 *tmp = vtkMatrix4x4::New();
                    tmp->Identity();
                    tmp->SetElement(0,0,-1.0);
                    tmp->SetElement(1,1,-1.0);
                    vtkMatrix4x4::Multiply4x4(rotations,tmp,rotations);
                }
            }
            break;
        case 4: // Left lateral decubitus, rotate points by -90 degrees in the z direction.
            {
                if ( rotations == NULL ) {
                    rotations = vtkMatrix4x4::New();
                    rotations->Identity();
                    rotations->SetElement(0,0,0.0);
                    rotations->SetElement(0,1,1.0);
                    rotations->SetElement(1,0,-1.0);
                    rotations->SetElement(1,1,0.0);
                } else {
                    vtkMatrix4x4 *tmp = vtkMatrix4x4::New();
                    tmp->Identity();
                    tmp->SetElement(0,0,0.0);
                    tmp->SetElement(0,1,1.0);
                    tmp->SetElement(1,0,-1.0);
                    tmp->SetElement(1,1,0.0);
                    vtkMatrix4x4::Multiply4x4(rotations,tmp,rotations);
                    tmp->Delete();
                }
            }
            break;
        case 8: // Right lateral decubitus, rotate points by 90 degrees in the z direction.
            {
                if ( rotations == NULL ) {
                    rotations = vtkMatrix4x4::New();
                    rotations->Identity();
                    rotations->SetElement(0,0,0.0);
                    rotations->SetElement(0,1,-1.0);
                    rotations->SetElement(1,0,1.0);
                    rotations->SetElement(1,1,0.0);
                } else {
                    vtkMatrix4x4 *tmp = vtkMatrix4x4::New();
                    tmp->Identity();
                    tmp->SetElement(0,0,0.0);
                    tmp->SetElement(0,1,-1.0);
                    tmp->SetElement(1,0,1.0);
                    tmp->SetElement(1,1,0.0);
                    vtkMatrix4x4::Multiply4x4(rotations,tmp,rotations);
                    tmp->Delete();
                }
            }
            break;
        default: // Unknown, assume supine.
            break;
    }

    // Rotate the points if rotations is not NULL.
    if ( rotations != NULL ) {
        rotations->MultiplyPoint(tlhc_temp,tlhc_temp);
        rotations->MultiplyPoint(row_temp,row_temp);
        rotations->MultiplyPoint(column_temp,column_temp);
        rotations->Delete();
        rotations = NULL;
    }
    
    // Copy the data back to 3D.
    tlhc[0] = tlhc_temp[0];
    tlhc[1] = tlhc_temp[1];
    tlhc[2] = tlhc_temp[2];
	
    row[0] = row_temp[0];
    row[1] = row_temp[1];
    row[2] = row_temp[2];
	
    column[0] = column_temp[0];
    column[1] = column_temp[1];
    column[2] = column_temp[2];

    // Noramlize row and column vectors.
    if ( vtkMath::Normalize(row) == 0.0 ) {
        vtkWarningWithObjectMacro(this, "Bad plane coordinate system in row vector");
        return false;
    }
    if ( vtkMath::Normalize(column) == 0.0 ) {
        vtkWarningWithObjectMacro(this, "Bad plane coordinate system in column vector");
        return false;
    }
    // Generate the normal.
    vtkMath::Cross(row,column,normal);

    // Finally shift the in-plane TLHC position by 1/2 voxel.
    for(int i=0; i<3; i++) {
        tlhc[i] += (0.5 * hdr->MR_Image_Pixel_Size_X * row[i] + 0.5 * hdr->MR_Image_Pixel_Size_Y * column[i]);
    }

    return true;
}


struct svkGESigna5XReaderSort_lt_pair_double_string
{
    bool operator()(const pair<double, string> s1, 
                  const pair<double, string> s2) const
    {
        return s1.first < s2.first;
    }
};


struct svkGESigna5XReaderSort_gt_pair_double_string
{
    bool operator()(const pair<double, string> s1, 
                  const pair<double, string> s2) const
    {
        return s1.first > s2.first;
    }
};


/*!
 * Sort the list of files in either ascending or descending order by ImagePositionPatient
 * and Series ID.  If the Series ID is not the same as the input, the file
 * is removed from the list.  Also return true if this is a multi-volume series. 
 */
bool svkGESigna5XReader::SortFilesByImagePositionPatient(GESignaHeader* selectedFile, 
    vtkStringArray* fileNames, bool ascending)
{
    bool multiVolumeSeries = false;
    vector<pair<double, string> > positionFilePairVector;
    double imagePosition = VTK_DOUBLE_MAX; // initialize to an "infinite" value
    
    for (int i = 0; i < fileNames->GetNumberOfValues(); i++) {
        double position[3];
        double row[3];
        double col[3];
        double normal[3];
        double tmpImagePosition = 0;
        GESignaHeader* tmp = ReadHeader(  fileNames->GetValue(i) );
        this->CalculateTopLeftHandCornerAndRowColumnAndNormalVectors(tmp,position,row,col,normal);
        // If input series matches this series, add to list for sorting later.
        if ( (selectedFile->Exam_Number == tmp->Exam_Number) &&
             (selectedFile->Series_Number == tmp->Series_Number) ) {
            pair<double, string> positionFilePair;
            tmpImagePosition = (normal[0]*position[0]) + (normal[1]*position[1]) 
                + (normal[2]*position[2]);
            // If the image position is the same for any given slice, then we have a multi-volume series.
            if( tmpImagePosition == imagePosition ) {
                multiVolumeSeries = true;
                delete tmp;
                return multiVolumeSeries;
            } else {
                imagePosition = tmpImagePosition;
                delete tmp;
            }
            positionFilePair.first = imagePosition;
            positionFilePair.second = fileNames->GetValue(i);
            positionFilePairVector.push_back(positionFilePair);
        } 
    }
    // Sort according to ascending/descending order.
    if (ascending) {
        sort(positionFilePairVector.begin(), 
            positionFilePairVector.end(), svkGESigna5XReaderSort_lt_pair_double_string());
    } else {
        sort(positionFilePairVector.begin(), 
            positionFilePairVector.end(), svkGESigna5XReaderSort_gt_pair_double_string());
    }
    // Finally, repopulate the file list.
    fileNames->SetNumberOfValues(positionFilePairVector.size());
    for (int i = 0; i < positionFilePairVector.size(); i++) {
        fileNames->SetValue(i, positionFilePairVector[i].second.c_str() );
#if VTK_DEBUG_ON
        cout << "FN: " << fileNames->GetValue(i) << endl;
#endif
    } 
    return multiVolumeSeries;
}


/*!
 *
 */
void svkGESigna5XReader::InitDcmHeader()
{
    string mrFileName( this->GetFileName() );
    string mrFilePath( this->GetFilePath( this->GetFileName() ) );  
    
    vtkGlobFileNames* globFileNames = vtkGlobFileNames::New();
    globFileNames->AddFileNames( string( mrFilePath + "/*.MR").c_str() );  // Should only find MR files, not CT.

    vtkSortFileNames* sortFileNames = vtkSortFileNames::New();
    sortFileNames->SetInputFileNames( globFileNames->GetFileNames() );
    sortFileNames->NumericSortOn();
    sortFileNames->SkipDirectoriesOn();
    sortFileNames->Update();

    // If ImageOrientationPatient is not the same for all of the slices,
    // then just use the specified file.
    GESignaHeader* selectedFile = ReadHeader(  this->GetFileName() );
    if ( selectedFile == NULL ) {
        vtkErrorMacro("Unable to read header of file " << this->GetFileName() );
        return;
    }
    vtkStringArray* fileNames =  sortFileNames->GetFileNames();
    for (int i = 0; i < fileNames->GetNumberOfValues(); i++) {
        GESignaHeader* tmp = ReadHeader(  fileNames->GetValue(i) );
        if ( selectedFile->MR_Plane_Type != tmp->MR_Plane_Type ) {
            
            vtkWarningWithObjectMacro(this, "Image orientation is not the same for all slices, using only specified file ");

            vtkStringArray* tmpFileNames = vtkStringArray::New(); 
            tmpFileNames->SetNumberOfValues(1);
            tmpFileNames->SetValue(0, this->GetFileName() );
            sortFileNames->SetInputFileNames( tmpFileNames );
            tmpFileNames->Delete();
            delete tmp;
            break; 
        } else {
            delete tmp;
        }
    }

    // Now sort the files according to Series and Position.
    fileNames =  sortFileNames->GetFileNames();
    if ( this->SortFilesByImagePositionPatient(selectedFile, fileNames, true) ){
        vtkWarningWithObjectMacro(this, "Multi-volume series are currently not supported, using only specified file ");

        vtkStringArray* tmpFileNames = vtkStringArray::New(); 
        tmpFileNames->SetNumberOfValues(1);
        tmpFileNames->SetValue(0, this->GetFileName() );
        sortFileNames->SetInputFileNames( tmpFileNames );
        tmpFileNames->Delete();
    }

    // Calling this method will set the DataExtents for the slice direction
    this->SetFileNames( sortFileNames->GetFileNames() );

    // Initialize the image header information stored in this class using the first image.
    if ( this->imageHeader != NULL ) {
        delete this->imageHeader;
        this->imageHeader = NULL;
    }
    this->imageHeader = ReadHeader(  this->GetFileNames()->GetValue( 0 ) );
    if ( this->imageHeader == NULL ) {
        vtkErrorMacro("Unable to read header of file " << this->GetFileNames()->GetValue( 0 ).c_str() );
        return;
    }

    // Check that the number of files, matches the number of slices in the header.
    if( this->GetFileNames()->GetNumberOfValues() != selectedFile->MR_Number_Of_Slices ) {
        vtkWarningWithObjectMacro(this, "The number of files to read does not match the number of slices reported in the header ");
    }

    //  Now override elements with Multi-Frame sequences and default details:
    this->iod = svkEnhancedMRIIOD::New();
    iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    iod->SetReplaceOldElements(false); 
    iod->InitDcmHeader();

    //  Now set DICOM header according to GE header.
    this->InitEnhancedMRImageModule();
    this->InitPatientModule();
    this->InitGeneralStudyModule();
    this->InitGeneralSeriesModule();
    this->InitGeneralEquipmentModule();
    this->InitEnhancedGeneralEquipmentModule();
    this->InitMultiFrameFunctionalGroupsModule();
    this->InitMRImageAndSpectroscopyInstanceMacro();
    this->InitMRPulseSequenceModule();

    /*
     *  odds and ends: 
     */
    int rows    = selectedFile->MR_Image_Matrix_Size_Y; // Y
    int columns = selectedFile->MR_Image_Matrix_Size_X; // X
    delete selectedFile;
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "Rows", 
        rows 
    );
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "Columns", 
        columns 
    );
    
    globFileNames->Delete();
    sortFileNames->Delete();

    this->iod->Delete();
}


/*!
 *
 */
void svkGESigna5XReader::InitEnhancedMRImageModule()
{
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "BurnedInAnnotation", 
        "NO" 
    );

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "LossyImageCompression", 
        "00" 
    );

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "PresentationLUTShape", 
        "IDENTITY" 
    );
}


/*!
 *
 */
void svkGESigna5XReader::InitPatientModule()
{
    if ( this->imageHeader == NULL ) {
        return;
    }

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "PatientID", 
        string(this->imageHeader->Exam_Patient_ID) 
    );

    // It appears like the patient's last and first name is 
    // separated by a comma.  Need to replace it with ^
    string name(this->imageHeader->Exam_Patient_Name);
    size_t found = name.find_first_of(',');
    if ( found != string::npos ) {
        name[found] = '^';
    }

    this->GetOutput()->GetDcmHeader()->SetDcmPatientName(name);

    string str;
    switch (this->imageHeader->Exam_Patient_Sex) {
        case 1:         str="M"; break;
        case 2:         str="F"; break;
        default:        str="";  break;
    }
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "PatientSex", 
        str 
    );

    ostringstream ost;
    ost << setfill('0') << setw(3) << dec
        << this->imageHeader->Exam_Patient_Age;
    switch (this->imageHeader->Exam_Patient_Age_Notation) {
        case 0: ost << "Y"; break;
        case 1: ost << "M"; break;
        case 2: ost << "D"; break;
        case 3: ost << "W"; break;
    }
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "PatientAge", 
        ost.str() 
    );

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "PatientWeight", 
        this->imageHeader->Exam_Patient_Weight_In_Grams/1000.0 
    );

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "AdditionalPatientHistory", 
        string(this->imageHeader->Exam_Patient_History) 
    );
}


/*!
 *
 */
void svkGESigna5XReader::InitGeneralStudyModule()
{
    if ( this->imageHeader == NULL ) {
        return;
    }
    
    this->GetOutput()->GetDcmHeader()->SetValue(
        "StudyDescription",
        string(this->imageHeader->Exam_Description) 
    );

    string name(this->imageHeader->Exam_Referring_Physician);
    size_t found = name.find_first_of(',');
    if ( found != string::npos ) {
        name[found] = '^';
    }
    this->GetOutput()->GetDcmHeader()->SetValue(
        "ReferringPhysicianName",
        name 
    );

    name = this->imageHeader->Exam_Radiologist;
    found = name.find_first_of(',');
    if ( found != string::npos ) {
        name[found] = '^';
    }
    this->GetOutput()->GetDcmHeader()->SetValue(
        "PerformingPhysicianName",
        name 
    );

    // Just the initials.
    this->GetOutput()->GetDcmHeader()->SetValue(
        "OperatorsName",
        string(this->imageHeader->Exam_Operator) 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "StudyDate",
        string(this->imageHeader->Exam_Time_Stamp_Date) 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "StudyTime",
        string(this->imageHeader->Exam_Time_Stamp_Time) 
    );

    ostringstream ost;
    ost << this->imageHeader->Exam_Number;

    this->GetOutput()->GetDcmHeader()->SetValue(
        "StudyID",
        ost.str() 
    );

    string uid = "1.2.276.0.7230010.3"; //OFFIS_UID_ROOT
    uid += ".1.2.";
    uid += ost.str();

    // Create StudyInstanceUID using study ID.
    this->GetOutput()->GetDcmHeader()->SetValue(
        "StudyInstanceUID",
        uid 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "AccessionNumber",
        string(this->imageHeader->Exam_Requisition_Number) 
    );
}


/*!
 *
 */
void svkGESigna5XReader::InitGeneralSeriesModule()
{
    if ( this->imageHeader == NULL ) {
        return;
    }

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ProtocolName",
        string(this->imageHeader->Series_Scan_Potocol_Name) 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SeriesNumber",
        (int)this->imageHeader->Series_Number 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SeriesDescription",
        string(this->imageHeader->Series_Description) 
    );
    
    this->GetOutput()->GetDcmHeader()->SetValue(
        "SeriesDate",
        string(this->imageHeader->Series_Time_Stamp_Date) 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SeriesTime",
        string(this->imageHeader->Series_Time_Stamp_Time) 
    );

    string hfff,ap,patientPosition;

    switch (this->imageHeader->Series_Patient_Entry) {
        case 1:
            hfff="HF";
            break;
        case 2:
            hfff="FF";
            break;
        default:        
            hfff="UNKNOWN";
    }

    switch (this->imageHeader->Series_Patient_Position) {
        case 1:
            ap="S";
            break;
        case 2:
            ap="P";
            break;
        case 4:
            ap="DL";
            break;
        case 8:
            ap="DR";
            break;
        default:        
            ap="UNKNOWN";
    }
    patientPosition = hfff + ap;

    this->GetOutput()->GetDcmHeader()->SetValue(
        "PatientPosition",
        patientPosition 
    );

}


/*!
 *  
 */
void svkGESigna5XReader::InitGeneralEquipmentModule()
{
    if ( this->imageHeader == NULL ) {
        return;
    }

    this->GetOutput()->GetDcmHeader()->SetValue(
        "Manufacturer",
        "GE MEDICAL SYSTEMS" 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "InstitutionName", 
        string(this->imageHeader->Exam_Hospital_Name)
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "StationName", 
        string(this->imageHeader->Exam_System_ID)
    );

}


/*!
 *  initialize 
 */
void svkGESigna5XReader::InitEnhancedGeneralEquipmentModule()
{
    if ( this->imageHeader == NULL ) {
        return;
    }

    this->GetOutput()->GetDcmHeader()->SetValue(
        "DeviceSerialNumber", 
        string(this->imageHeader->Exam_Unique_System_ID)
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ManufacturerModelName",
        string(this->imageHeader->Suite_Product_ID) 
    );

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "SoftwareVersions",
        string(this->imageHeader->Exam_Software_Version) 
    );
}


/*!
 *
 */
void svkGESigna5XReader::InitMultiFrameFunctionalGroupsModule()
{
    if ( this->imageHeader == NULL ) {
        return;
    }
 
    this->numFrames =  this->GetFileNames()->GetNumberOfValues();
 
    /*
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "NumberOfFrames", 
        this->numFrames 
    );

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "InstanceNumber", 
        (int)this->imageHeader->MR_Image_Number 
    );
    */

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "ContentDate", 
        string(this->imageHeader->MR_Timestamp_Of_Last_Change_Date) 
    );

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "ContentTime", 
        string(this->imageHeader->MR_Timestamp_Of_Last_Change_Time) 
    );

    this->InitSharedFunctionalGroupMacros();
    this->InitPerFrameFunctionalGroupMacros();

}

/*!
 *
 */
void svkGESigna5XReader::InitSharedFunctionalGroupMacros()
{
    this->InitPixelMeasuresMacro();
    this->InitPlaneOrientationMacro();
    this->InitPixelValueTransformationMacro();
    this->InitMRImageFrameTypeMacro();
    this->InitMRTimingAndRelatedParametersMacro();
    this->InitMRFOVGeometryMacro();
    this->InitMREchoMacro();
    this->InitMRModifierMacro();
    this->InitMRImagingModifierMacro();
    this->InitMRReceiveCoilMacro();
    this->InitMRTransmitCoilMacro();
    this->InitMRAveragesMacro();
}

/*!
 *  Pixel Spacing:
 */
void svkGESigna5XReader::InitPixelMeasuresMacro()
{

    if ( this->imageHeader == NULL ) {
        return;
    }

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PixelMeasuresSequence"
    );

    ostringstream ost;
    ost << fixed << setprecision(6)
        << this->imageHeader->MR_Image_Pixel_Size_X << "\\" 
        << this->imageHeader->MR_Image_Pixel_Size_Y;

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PixelMeasuresSequence",
        0,
        "PixelSpacing",
        ost.str(), 
        "SharedFunctionalGroupsSequence",
        0
    );
  
    ost.clear();
    ost.str( "" );
    ost << this->imageHeader->MR_Slice_Thickness;

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PixelMeasuresSequence",
        0,
        "SliceThickness",
        ost.str(), 
        "SharedFunctionalGroupsSequence",
        0
    );
}

/*!
 *
 */
void svkGESigna5XReader::InitPlaneOrientationMacro()
{
    if ( this->imageHeader == NULL ) {
        return;
    }

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PlaneOrientationSequence"
    );
 
    double position[3];
    double row[3];
    double col[3];
    double normal[3];
    this->CalculateTopLeftHandCornerAndRowColumnAndNormalVectors(this->imageHeader,position,row,col,normal);

    //  image orientation patient / dcos
    ostringstream ost;
    ost << fixed << setprecision(6) << row[0] 
        << "\\" << row[1] << "\\"<< row[2] 
        << "\\" << col[0] << "\\" << col[1] 
        << "\\" << col[2];    

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PlaneOrientationSequence",
        0,
        "ImageOrientationPatient",
        ost.str(), 
        "SharedFunctionalGroupsSequence",
        0
    );

}


void svkGESigna5XReader::InitPixelValueTransformationMacro()
{
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "PixelValueTransformationSequence"
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PixelValueTransformationSequence",       
        0,                             
        "RescaleIntercept",              
        0,                     
        "SharedFunctionalGroupsSequence", 
        0                                  
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PixelValueTransformationSequence",       
        0,                             
        "RescaleSlope",              
        1,                     
        "SharedFunctionalGroupsSequence", 
        0                                  
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PixelValueTransformationSequence",       
        0,                             
        "RescaleType",              
        string("US"),                     
        "SharedFunctionalGroupsSequence", 
        0                                  
    );

}


/*!
 *
 */
void svkGESigna5XReader::InitMRImageFrameTypeMacro()
{
    if ( this->imageHeader == NULL ) {
        return;
    }

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRImageFrameTypeSequence"
    );

    const char *value1,*value2,*value3,*value4;

    bool angio;
    switch (this->imageHeader->MR_Pulse_Sequence) {
        case 12:                // TF:TOF
        case 13:                // PC
        case 15:                // TOFGR:TOG/GR
        case 16:                // TOFSPGR:TOF/SPGR
        case 17:                // PCGR:PC/GR
        case 18:                // PCSPGR:PC/SPGR
            angio=true;
            break;
        default:
            angio=false;
            break;
    }

    bool collapsed = this->imageHeader->MR_Vascular_Collapse;

    bool projected;
    switch (this->imageHeader->MR_Projection_Algorithm) {
        case 0:                 // none
            value4="NONE";
            projected = false;
            break;
        case 2:                 // Minimum Pixel:Min
            value3="MIN_IP";
            value4="MINIMUM";
            projected = true;
            break;
        case 3:                 // Maximum Pixel:Max
            value3="MAX_IP";
            value4="MAXIMUM";
            projected = true;
            break;
        case 1:                 // Prototype
        default:
            value3="PROJECTION IMAGE";
            value4="UNKNOWN";
            projected = true;
            break;
    }

    switch (this->imageHeader->Series_Type) {
       case 1:         // Prospective
           value1="ORIGINAL";
           value2="PRIMARY";
           value3=(angio&&!projected&&!collapsed)?"ANGIO":"VOLUME";
           value4="NONE";
           break;
       case 2:         // Retrospective
           value1="ORIGINAL";
           value2="SECONDARY";
           value3=(angio&&!projected&&!collapsed)?"ANGIO":"VOLUME";
           value4="NONE";
           break;
       case 3:         // Scout
           value1="ORIGINAL";
           value2="PRIMARY";
           value3="LOCALIZER";     // CT IOD
           value4="NONE";
           break;
       case 4:         // Reformatted
       case 5:         // Screensave
           value1="DERIVED";
           value2="SECONDARY";
           value3=(angio&&!projected&&!collapsed)?"ANGIO":"OTHER";
           value4="NONE";
           break;
       case 6:         // Xenon
           value1="ORIGINAL";
           value2="PRIMARY";
           value3=(angio&&!projected&&!collapsed)?"ANGIO":"AXIAL";
           value4="NONE";
           break;
       case 7:         // Service
           value1="DERIVED";
           value2="SECONDARY";
           value3=(angio&&!projected&&!collapsed)?"ANGIO":"OTHER";
           value4="NONE";
           break;
       case 9:         // Projected
           value1="DERIVED";
           value2="SECONDARY";
           value3=(angio&&!projected&&!collapsed)?"ANGIO":value3; // MR IOD
           break;
       default:
           value1="UNKNOWN";
           value2="UNKNOWN";
           value3="UNKNOWN";
           value4="UNKNOWN";
           break;
    }

    string frameType = value1;
    frameType += "\\";
    frameType += value2;
    frameType += "\\";
    frameType += value3;
    frameType += "\\";
    frameType += value4;

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ImageType", 
        frameType
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRImageFrameTypeSequence",
        0,                               
        "FrameType",                    
        frameType,  
        "SharedFunctionalGroupsSequence",   
        0                                 
    );

    string VolumetricProperties = "VOLUME";
    string VolumeBasedCalculationTechnique = "NONE";
    switch (this->imageHeader->MR_Projection_Algorithm) {             
        case 2:                 // Minimum Pixel:Min
            VolumetricProperties = "SAMPLED";
            VolumeBasedCalculationTechnique = "MIN_IP";
            break;
        case 3:                 // Maximum Pixel:Max
            VolumetricProperties = "SAMPLED";
            VolumeBasedCalculationTechnique = "MAX_IP";
            break;
        case 0:                 // none
        case 1:                 // Prototype
        default:
            break;
    }

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRImageFrameTypeSequence",  
        0,                                 
        "PixelPresentation",           
        "MONOCHROME",  
        "SharedFunctionalGroupsSequence",
        0                              
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRImageFrameTypeSequence",  
        0,                                 
        "VolumetricProperties",           
        VolumetricProperties,  
        "SharedFunctionalGroupsSequence",
        0                              
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRImageFrameTypeSequence",  
        0,                                 
        "VolumeBasedCalculationTechnique",
        VolumeBasedCalculationTechnique,
        "SharedFunctionalGroupsSequence",
        0                              
    );

    string ComplexImageComponent;

    switch(this->imageHeader->MR_Image_Type) {
        case 0: // MAGNITUDE
            ComplexImageComponent = "MAGNITUDE";
            break;
        case 1: // PHASE
            ComplexImageComponent = "PHASE";
            break;
        case 2: // REAL
            ComplexImageComponent = "REAL";
            break;
        case 3: // IMAGINARY
            ComplexImageComponent = "IMAGINARY";
            break;
        default: // MAGNITUDE
            ComplexImageComponent = "MAGNITUDE";
    }

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRImageFrameTypeSequence",
        0,                                
        "ComplexImageComponent",           
        ComplexImageComponent,  
        "SharedFunctionalGroupsSequence",   
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRImageFrameTypeSequence",  
        0,                                 
        "AcquisitionContrast",            
        "UNKNOWN",
        "SharedFunctionalGroupsSequence",   
        0                                 
    );

    // MR Image Description Macro
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "ComplexImageComponent", 
        ComplexImageComponent 
    );

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "AcquisitionContrast", 
        "UNKNOWN" 
    );

    // Common CT/MR Image Description Level Macro
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "PixelPresentation", 
        "MONOCHROME" 
    );

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "VolumetricProperties", 
        VolumetricProperties 
    );

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "VolumeBasedCalculationTechnique", 
        VolumeBasedCalculationTechnique 
    );
}


/*!
 *
 *    
 */
void svkGESigna5XReader::InitMRTimingAndRelatedParametersMacro()
{
    if ( this->imageHeader == NULL ) {
        return;
    }

    // Figure out scanning sequence.
    bool spinEcho = false;
    bool gradientEcho = false;
    switch (this->imageHeader->MR_Pulse_Sequence) {
        case 0:                 // SE
            spinEcho = true;
            break;
        case 1:                 // IR
        case 2:                 // RM:RM
        case 3:                 // RMGE:none
            break;
        case 4:                 // GRE:GR
        case 5:                 // MPGR
            gradientEcho = true;
            break;
        case 6:                 // MPIRS:IR/s
        case 7:                 // MPIRI:IR
            break;
        case 8:                 // VOGRE:3D/GR
        case 9:                 // CINEGRE:Cine/GRE
        case 10:                // SPGR
        case 11:                // SSFP
        case 12:                // TF:TOF
        case 13:                // PC
        case 14:                // CINSPGR:Cine/SPGR
        case 15:                // TOFGR:TOG/GR
        case 16:                // TOFSPGR:TOF/SPGR
        case 17:                // PCGR:PC/GR
        case 18:                // PCSPGR:PC/SPGR
            gradientEcho = true;
            break;
        case 19:                // FSE
            spinEcho = true;
            break;
        case 20:                // FGR
        case 21:                // FMPGR
        case 22:                // FSPGR
        case 23:                // FMPSPGR
            gradientEcho = true;
            break;
        case 24:                // SPECT
        case 25:                // PSEQ_MIXED:MIXED
        default:
            break;
    }

    int etl = this->imageHeader->MR_Echo_Train_Length;
    int numecho = this->imageHeader->MR_Number_Of_Echoes;
    int useetl = etl ? etl : numecho;
    if( useetl == 0 ) {
        useetl = 1;
    }

    this->GetOutput()->GetDcmHeader()->InitMRTimingAndRelatedParametersMacro(
        (float)((double)this->imageHeader->MR_Pulse_Repetition_Time / 1000.0), 
        (float)this->imageHeader->MR_Flip_Angle, 
        useetl 
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence", 
        0,                                
        "RFEchoTrainLength",                     
        (spinEcho) ? useetl:0, 
        "SharedFunctionalGroupsSequence",  
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence", 
        0,                                
        "GradientEchoTrainLength",                     
        (gradientEcho) ? useetl:0,
        "SharedFunctionalGroupsSequence",  
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",      
        0,                          
        "SpecificAbsorptionRateSequence"   
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",      
        0,                          
        "OperatingModeSequence"   
    );
}


/*!
 *
 */
void svkGESigna5XReader::InitMRFOVGeometryMacro()
{
    if ( this->imageHeader == NULL ) {
        return;
    }

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRFOVGeometrySequence"
    );

    string str;
    if (this->imageHeader->MR_Frequency_Direction == 1) {
        str="COLUMN";
    } else if (this->imageHeader->MR_Frequency_Direction == 2
        || this->imageHeader->MR_Frequency_Direction == 0) {    // assume "unknown" is row
        str="ROW";
    } else {
        str="OTHER";
    }

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRFOVGeometrySequence",   
        0,                                    
        "InPlanePhaseEncodingDirection",     
        str, 
        "SharedFunctionalGroupsSequence",   
        0                                 
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRFOVGeometrySequence",   
        0,                                    
        "MRAcquisitionFrequencyEncodingSteps",     
        (int)this->imageHeader->MR_Image_Dimension_X, 
        "SharedFunctionalGroupsSequence",   
        0                                 
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRFOVGeometrySequence",   
        0,                                    
        "MRAcquisitionPhaseEncodingStepsInPlane",     
        (int)this->imageHeader->MR_Image_Dimension_Y, 
        "SharedFunctionalGroupsSequence",   
        0                                 
    );

    // Add out of plane phase encode if 3D.
    // Note that it's possible that the number of 
    // reconstrcuted slices and the number of 
    // slice-encoding steps are different. However,
    // there's nothing in the header that tells me
    // what the slice-encoding steps are, so I must
    // calculate using the scan duration, TR, number 
    // of averages, and number of phase encoding steps.
    if ( this->imageHeader->MR_Imaging_Mode == 2 ||
        this->imageHeader->MR_Imaging_Mode == 3 ) {
        double NumberOfAverages = this->imageHeader->MR_Number_Of_Averages;
        double AcquisitionDuration = this->imageHeader->MR_Scan_Duration;
        double PhaseEncodingSteps = this->imageHeader->MR_Image_Dimension_Y;
        double RepetitionTime = this->imageHeader->MR_Pulse_Repetition_Time;
        double SliceEncodingSteps = AcquisitionDuration/
            (NumberOfAverages*PhaseEncodingSteps*RepetitionTime);
        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "MRFOVGeometrySequence",   
            0,                                    
            "MRAcquisitionPhaseEncodingStepsOutOfPlane",     
            (int)SliceEncodingSteps, 
            "SharedFunctionalGroupsSequence",   
            0                                 
        );

        // Assume phase oversampling if slices is less
        // than slice-encode size.
        if ( ((unsigned short)SliceEncodingSteps >
             this->imageHeader->MR_Number_Of_Slices) && 
             ((unsigned short)this->imageHeader->MR_Image_Dimension_Y <= 
             this->imageHeader->MR_Image_Matrix_Size_Y ) ) {
            this->GetOutput()->GetDcmHeader()->SetValue( 
                "OversamplingPhase", 
                string ("3D") 
            );
        } else if ( ((unsigned short)SliceEncodingSteps >
             this->imageHeader->MR_Number_Of_Slices) && 
             ((unsigned short)this->imageHeader->MR_Image_Dimension_Y > 
             this->imageHeader->MR_Image_Matrix_Size_Y) ) {
             this->GetOutput()->GetDcmHeader()->SetValue( 
                "OversamplingPhase", 
                string ("2D_3D") 
            );
        }
    } else if ( (unsigned short)this->imageHeader->MR_Image_Dimension_Y > 
             this->imageHeader->MR_Image_Matrix_Size_Y ) {
        this->GetOutput()->GetDcmHeader()->SetValue( 
            "OversamplingPhase", 
            string ("2D") 
        );
    }

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRFOVGeometrySequence",   
        0,                                    
        "PercentSampling",     
        100, 
        "SharedFunctionalGroupsSequence",   
        0                                 
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRFOVGeometrySequence",   
        0,                                    
        "PercentPhaseFieldOfView",     
        (float)100.0*this->imageHeader->MR_FOV_Y/this->imageHeader->MR_FOV_X, 
        "SharedFunctionalGroupsSequence",   
        0                                 
    );
}

/*!
 *
 */
void svkGESigna5XReader::InitMREchoMacro()
{
    if ( this->imageHeader == NULL ) {
        return;
    }

    this->GetOutput()->GetDcmHeader()->InitMREchoMacro(
        (float)((double)this->imageHeader->MR_Pulse_Echo_Time/1000.0) 
    ); 
}

/*!
 * 
 */
void svkGESigna5XReader::InitMRModifierMacro()
{

    if ( this->imageHeader == NULL ) {
        return;
    }

    this->GetOutput()->GetDcmHeader()->InitMRModifierMacro( 
        (float)((double)this->imageHeader->MR_Pulse_Inversion_Time/1000.0)
    ); 

    string str = "NONE";
    if (this->imageHeader->MR_Imaging_Options & (1<<3)) {       // FC - Flow Compensated
        str = "OTHER"; // no way of knowing correct value
    }
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRModifierSequence",
        0,                        
        "FlowCompensation",       
        str,
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    if ( str == "OTHER" ) {
        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "MRModifierSequence",
            0,                        
            "FlowCompensationDirection",       
            str,
            "SharedFunctionalGroupsSequence",    
            0                      
        );
    }

    // These are educated guesses (http://www.mr-tip.com/serv1.php?type=cam).
    string Spoiling = "NONE";
    switch (this->imageHeader->MR_Pulse_Sequence) {
        case 10:                // SPGR
        case 14:                // CINSPGR:Cine/SPGR
        case 16:                // TOFSPGR:TOF/SPGR
        case 18:                // PCSPGR:PC/SPGR
        case 22:                // FSPGR
        case 23:                // FMPSPGR
            Spoiling = "RF";
            break;
        default:
            break;
    }

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRModifierSequence",
        0,                        
        "Spoiling",       
        Spoiling,
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRModifierSequence",
        0,                        
        "T2Preparation",       
        string("NO"),
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRModifierSequence",
        0,                        
        "SpectrallySelectedExcitation",       
        string("NONE"),
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    str = "NONE";
    if (this->imageHeader->MR_Imaging_Options & (1<<5)               // ST - Sat parameters
         || this->imageHeader->MR_Bitmap_Of_Saturation_Selections) { // if any sat bits set
        str = "SLAB";                    // spatial presaturation
    }

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRModifierSequence",
        0,                        
        "SpatialPresaturation",       
        str,
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    str = "NO";
    if (this->imageHeader->MR_Fractional_Echo_Flag & 1<<0) {
        str = "YES";                   // partial fourier freq
    }

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRModifierSequence",
        0,                        
        "PartialFourier",       
        str,
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    if ( str == "YES" ) {
        str = "PHASE";  // Assume phase encoding direction.
                        // Partial fourier imaging in the
                        // slice encoding direction is
                        // uncommon.
        if( this->imageHeader->MR_Image_Dimension_X < 
            this->imageHeader->MR_Image_Dimension_Y) {
            str = "FREQUENCY";
        }
        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "MRModifierSequence",
            0,                        
            "PartialFourierDirection",       
            str,
            "SharedFunctionalGroupsSequence",    
            0                      
        );
    }

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRModifierSequence",
        0,                        
        "ParallelAcquisition",       
        string("NO"),
        "SharedFunctionalGroupsSequence",    
        0                      
    );
}


/*!
 * 
 */
void svkGESigna5XReader::InitMRImagingModifierMacro()
{
    if ( this->imageHeader == NULL ) {
        return;
    }

    float transmitFreq = (float)((double)this->imageHeader->MR_Center_Frequency/10000000.0);
    float pixelBandwidth =  (float)((this->imageHeader->MR_Variable_Bandwidth*2.0*1000.0)
                            /this->imageHeader->MR_Image_Dimension_X);

    this->GetOutput()->GetDcmHeader()->InitMRImagingModifierMacro(
        transmitFreq, 
        pixelBandwidth
    );

}


/*!
 *  Should work for single vs multi-coil, but will not currently 
 *  differentiate between volume, surface, body coils
 */
void svkGESigna5XReader::InitMRReceiveCoilMacro()
{

    if ( this->imageHeader == NULL ) {
        return;
    }

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRReceiveCoilSequence"
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,                        
        "ReceiveCoilName",       
        string(this->imageHeader->MR_Receive_Coil_Name), 
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,                        
        "ReceiveCoilManufacturerName",       
        string(""),
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    string coilType( "VOLUME" ); 
    string quadCoil("YES"); 

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,                        
        "ReceiveCoilType",       
        coilType, 
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,                        
        "QuadratureReceiveCoil",       
        quadCoil, 
        "SharedFunctionalGroupsSequence",    
        0                      
    );

}

/*!
 *
 */
void svkGESigna5XReader::InitMRTransmitCoilMacro()
{
    if ( this->imageHeader == NULL ) {
        return;
    }

    this->GetOutput()->GetDcmHeader()->InitMRTransmitCoilMacro(
        "GE",
        string(this->imageHeader->MR_Receive_Coil_Name),
        "VOLME"
    );

}


/*!
 *
 */
void svkGESigna5XReader::InitMRAveragesMacro()
{
    if ( this->imageHeader == NULL ) {
        return;
    }

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRAveragesSequence"
    );

    ostringstream ost;
    ost << this->imageHeader->MR_Number_Of_Averages;

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRAveragesSequence",
        0,                        
        "NumberOfAverages",       
        ost.str(),
        "SharedFunctionalGroupsSequence",    
        0
    );

}

/*!
 *
 */
void svkGESigna5XReader::InitPerFrameFunctionalGroupMacros()
{

    /*
     *  Iterate over slices (frames) and copy ImagePositions
     */
    double toplc[3];
    double dcos[3][3];
    for (int i = 0; i < 1; i++ ) {
    //for (int i = 0; i < this->GetFileNames()->GetNumberOfValues(); i++) { 

        GESignaHeader* tmpImage = ReadHeader( this->GetFileNames()->GetValue( i ) );
        if( tmpImage == NULL ) {
            vtkWarningWithObjectMacro(this, "Unable to read GE Signa 5X header");
            return;
        }
        double row[3];
        double col[3];
        double normal[3];
        this->CalculateTopLeftHandCornerAndRowColumnAndNormalVectors(tmpImage, toplc, row, col, normal);
        dcos[0][0] = row[0]; 
        dcos[0][1] = row[1]; 
        dcos[0][2] = row[2]; 
        dcos[1][0] = col[0]; 
        dcos[1][1] = col[1]; 
        dcos[1][2] = col[2]; 
        dcos[2][0] = normal[0]; 
        dcos[2][1] = normal[1]; 
        dcos[2][2] = normal[2]; 

        delete tmpImage; 
    }

    double pixelSize[3];
    pixelSize[0] =  this->imageHeader->MR_Image_Pixel_Size_X; 
    pixelSize[1] =  this->imageHeader->MR_Image_Pixel_Size_Y;
    pixelSize[2] =  this->imageHeader->MR_Slice_Thickness;


    svkDcmHeader::DimensionVector dimensionVector = this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector(); 
    int numSlices = this->GetFileNames()->GetNumberOfValues(); 
    svkDcmHeader::SetDimensionVectorValue( &dimensionVector, svkDcmHeader::SLICE_INDEX, numSlices - 1 );

    this->GetOutput()->GetDcmHeader()->InitPerFrameFunctionalGroupSequence(
                toplc,        
                pixelSize,  
                dcos,  
                &dimensionVector
    );
}

/*!
 *
 */
void svkGESigna5XReader::InitMRImageAndSpectroscopyInstanceMacro()
{
    if ( this->imageHeader == NULL ) {
        return;
    }

    /*  =======================================
     *  MR Image and Spectroscopy Instance Macro
     *  ======================================= */

    string AcquisitionDateTime = this->imageHeader->MR_Time_Stamp_Date;
    AcquisitionDateTime += this->imageHeader->MR_Time_Stamp_Time;
    this->GetOutput()->GetDcmHeader()->SetValue(
        "AcquisitionDateTime",
        AcquisitionDateTime 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "AcquisitionDuration",
        this->imageHeader->MR_Scan_Duration/1000000.0
    );

    //  Use the mps freq and field strength to determine gamma which is 
    //  characteristic of the isotop:
    float Exam_Magnet_Strength = (float)this->imageHeader->Exam_Magnet_Strength/10000.0;
    float MR_Center_Frequency = (float)this->imageHeader->MR_Center_Frequency/10000000.0;
    float gamma = MR_Center_Frequency  / Exam_Magnet_Strength;
   
    string nucleus;  
    if ( fabs( gamma - 42.57 ) < 0.1 ) {
        nucleus.assign("1H");
    } else if ( fabs( gamma - 10.7 ) <0.1 ) {
        nucleus.assign("13C");
    } else {
         vtkErrorMacro("Unknown gyromagnetic ratio " << gamma );
    }

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ResonantNucleus", 
        nucleus
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "KSpaceFiltering", 
        "NONE" 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ApplicableSafetyStandardAgency", 
        "FDA" // For US
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "MagneticFieldStrength", 
        Exam_Magnet_Strength 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ContentQualification", 
        "PRODUCT" 
    );

    
    /*  =======================================
     *  END: MR Image and Spectroscopy Instance Macro
     *  ======================================= */
}

/*!
 *
 */
void svkGESigna5XReader::InitMRPulseSequenceModule()
{
    if ( this->imageHeader == NULL ) {
        return;
    }

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "PulseSequenceName", 
        string(this->imageHeader->MR_Pulse_Sequence_Name) 
    );

    string str;
    switch(this->imageHeader->MR_Imaging_Mode) {
        case 1:                 // Two D:2D
            str="2D";
            break;
        case 2:                 // Three D Volume:3D
        case 3:                 // Three D Fourier:
            str="3D";
            break;
         case 4:                 // Cine:Cine
         case 5:                 // Angiography:ANGIO
         case 6:                 // Spectroscopy:SPECT
         case 7:                 // Flouroscopy:FLOURO
         case 8:                 // Research Mode:RM
         default:
            str="UNKNOWN";
    }

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "MRAcquisitionType", 
        str 
    );

    // These are all educated guesses!
    string EchoPulseSequence = "",
        PhaseContrast = "NO", 
        TimeofFlightContrast = "NO",
        SteadyStatePulseSequence = "NONE",
        RectilinearPhaseEncodeReordering = "LINEAR",
        SegmentedKSpaceTraversal = "SINGLE";
    switch (this->imageHeader->MR_Pulse_Sequence) {
        case 0:                 // SE
        case 1:                 // IR (Assume SPIN)
        case 2:                 // RM:RM (Research Mode)
            EchoPulseSequence = "SPIN";
            break;
        case 3:                 // RMGE:none (Research Mode GE)
        case 4:                 // GRE:GR
        case 5:                 // MPGR
            EchoPulseSequence = "GRADIENT";
            break;
        case 6:                 // MPIRS:IR/s
        case 7:                 // MPIRI:IR
            EchoPulseSequence = "SPIN"; // Assume SPIN
            break;
        case 8:                 // VOGRE:3D/GR
        case 9:                 // CINEGRE:Cine/GRE
        case 10:                // SPGR
            EchoPulseSequence = "GRADIENT";
            break;
        case 11:                // SSFP
            EchoPulseSequence = "GRADIENT";
            SteadyStatePulseSequence = "FREE_PRECESSION";
            break;
        case 12:                // TF:TOF
            EchoPulseSequence = "GRADIENT";
            TimeofFlightContrast = "YES";
            break;
        case 13:                // PC
            EchoPulseSequence = "GRADIENT";
            PhaseContrast = "YES";
            break;
        case 14:                // CINSPGR:Cine/SPGR
            EchoPulseSequence = "GRADIENT";
            break;
        case 15:                // TOFGR:TOG/GR
        case 16:                // TOFSPGR:TOF/SPGR
            EchoPulseSequence = "GRADIENT";
            TimeofFlightContrast = "YES";
            break;
        case 17:                // PCGR:PC/GR
        case 18:                // PCSPGR:PC/SPGR
            EchoPulseSequence = "GRADIENT";
            PhaseContrast = "YES";
            break;
        case 19:                // FSE
            EchoPulseSequence = "SPIN";
            RectilinearPhaseEncodeReordering = "SEGMENTED";
            SegmentedKSpaceTraversal = "PARITAL";
            break;
        case 20:                // FGR
        case 21:                // FMPGR
        case 22:                // FSPGR
        case 23:                // FMPSPGR
            EchoPulseSequence = "GRADIENT";
            break;
        case 24:                // SPECT
            break;
        case 25:                // PSEQ_MIXED:MIXED
            EchoPulseSequence = "BOTH";
            break;
        default:
            break;
    }

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "EchoPulseSequence", 
        EchoPulseSequence 
    );

    // MP - Multiplanar
    string MultiPlanarExcitation = "NO";
    if (this->imageHeader->MR_Imaging_Options & (1<<15)) {
        MultiPlanarExcitation = "YES";      
    }
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "MultiPlanarExcitation", 
        MultiPlanarExcitation 
    ); 

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "PhaseContrast", 
        PhaseContrast 
    );

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "TimeOfFlightContrast", 
        TimeofFlightContrast 
    );

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "SteadyStatePulseSequence", 
        SteadyStatePulseSequence 
    );

    // EPI doesn't appear to be an allowed image type
    // for GE Signa 5X?
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "EchoPlanarPulseSequence", 
        string ("NO") 
    );

    // May have to figure this out later, but 
    // assume NO for now.
    //this->GetOutput()->GetDcmHeader()->SetValue( 
    //    "MultipleSpinEcho", 
    //    string ("NO") 
    //);

    // Saturation Recovery doesn't appear to in
    // GE Signa 5X header.
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "SaturationRecovery", 
        string ("NO") 
    );

    string SpectrallySelectedSuppression; 
    switch (this->imageHeader->MR_Fat_Water_SAT) {
        case 1:
            SpectrallySelectedSuppression = "FAT";            // fat saturation
            break;
        case 2:
            SpectrallySelectedSuppression = "WATER";          // water saturation
            break;                          
        default:
            SpectrallySelectedSuppression = "NONE";
    }
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "SpectrallySelectedSuppression", 
        SpectrallySelectedSuppression 
    );

    // Oversampling phase already handled in InitMRFOVGeometryMacro()
    /*string OversamplingPhase = "NONE";
    if ( (unsigned short)this->imageHeader->MR_Image_Dimension_Y > 
             this->imageHeader->MR_Image_Matrix_Size_Y ) {
        OversamplingPhase = "2D";
    }
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "OversamplingPhase", 
        OversamplingPhase 
    );*/

    // Don't know! Always assume RECTILINEAR.
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "GeometryOfKSpaceTraversal", 
        string ("RECTILINEAR") 
    );

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "RectilinearPhaseEncodeReordering", 
        RectilinearPhaseEncodeReordering 
    );

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "SegmentedKSpaceTraversal", 
        SegmentedKSpaceTraversal 
    );

    if (str == "3D") {
        this->GetOutput()->GetDcmHeader()->SetValue( 
            "CoverageOfKSpace", 
            string ("FULL") 
        );
    }

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "NumberOfKSpaceTrajectories", 
        1 
    );
}


/*!
 *  Returns the file root without extension
 */
svkDcmHeader::DcmPixelDataFormat svkGESigna5XReader::GetFileType()
{
    return svkDcmHeader::UNSIGNED_INT_2;
}


/*!
 *
 */
int svkGESigna5XReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData");
    return 1;
}






