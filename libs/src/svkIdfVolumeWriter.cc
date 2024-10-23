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



#include <svkIdfVolumeWriter.h>
#include <svkImageReader2.h>
#include <svkUtils.h>
#include <svkTypeUtils.h>
#include </usr/include/vtk/vtkExecutive.h>
#include </usr/include/vtk/vtkImageAccumulate.h>
#include </usr/include/vtk/vtkErrorCode.h>
#include </usr/include/vtk/vtkUnsignedCharArray.h>
#include </usr/include/vtk/vtkUnsignedShortArray.h>
#include </usr/include/vtk/vtkShortArray.h>
#include </usr/include/vtk/vtkFloatArray.h>
#include </usr/include/vtk/vtkDoubleArray.h>
#include </usr/include/vtk/vtkByteSwap.h>


using namespace svk;


//vtkCxxRevisionMacro(svkIdfVolumeWriter, "$Rev$");
vtkStandardNewMacro(svkIdfVolumeWriter);


/*!
 *
 */
svkIdfVolumeWriter::svkIdfVolumeWriter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->castDoubleToFloat = false;
    this->globalRangeMin = 0.; 
    this->globalRangeMax = 0.;
}


/*!
 *
 */
svkIdfVolumeWriter::~svkIdfVolumeWriter()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}



/*!
 *  Write the DICOM MR Spectroscopy multi-frame file.   Also initializes the 
 *  DICOM SpectroscopyData element from the svkImageData object. 
 */
void svkIdfVolumeWriter::Write()
{

    vtkDebugMacro( << this->GetClassName() << "::Write()" );
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

    this->WriteData();
    this->WriteHeader();

    if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError) {
        vtkErrorMacro("Ran out of disk space; deleting file(s) already written");
        this->DeleteFiles();
        return;
    }

    delete [] this->InternalFileName;
    this->InternalFileName = NULL;
}


/*!
 *  Write the image data pixels to the IDF data file (.int2, .real, .byt).       
 *  int2 is unsigned short.  
 */
void svkIdfVolumeWriter::WriteData()
{
    vtkDebugMacro( << this->GetClassName() << "::WriteData()" );

    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader(); 
    int numPixelsPerSlice = hdr->GetIntValue( "Rows" ) * hdr->GetIntValue( "Columns" );
    int numSlices = hdr->GetNumberOfSlices();
    int numVolumes = this->GetImageDataInput(0)->GetPointData()->GetNumberOfArrays();

    int vtkDataType = vtkImageData::GetScalarType( this->GetImageDataInput(0)->GetInformation() ); 
    int dataType = this->GetImageDataInput(0)->GetDcmHeader()->GetPixelDataType( vtkDataType ); 
    if ( dataType == svkDcmHeader::SIGNED_FLOAT_8 ) {
        this->InitDoublePixelRange(); 
    }

    //  Write out each volume:
    for ( int vol = 0; vol < numVolumes; vol++ ) {

        string extension; 
        int numBytesPerPixel; 
        void* pixels; 
        float* floatPixels = NULL;
        if (dataType == svkDcmHeader::UNSIGNED_INT_1) {
            extension = ".byt"; 
            numBytesPerPixel = 1; 
            pixels = static_cast<vtkUnsignedCharArray*>(this->GetImageDataInput(0)->GetPointData()->GetArray(vol))->GetPointer(0);
        } else if (dataType == svkDcmHeader::UNSIGNED_INT_2) {
            extension = ".int2"; 
            numBytesPerPixel = 2; 
            pixels = static_cast<vtkUnsignedShortArray*>(this->GetImageDataInput(0)->GetPointData()->GetArray(vol))->GetPointer(0);
        } else if (dataType == svkDcmHeader::SIGNED_FLOAT_4 || 
                   dataType == svkDcmHeader::SIGNED_INT_2 || 
                   dataType == svkDcmHeader::SIGNED_FLOAT_8) {
            extension = ".real"; 
            numBytesPerPixel = 4; 
            if ( dataType == svkDcmHeader::SIGNED_FLOAT_8 ) {
                //scale to float: 
                if (this->GetDebug()) {
                    cout << "SCALE Double to Float" << endl;
                }
                double* doublePixels; 
                doublePixels = vtkDoubleArray::SafeDownCast(this->GetImageDataInput(0)->GetPointData()->GetArray(vol))->GetPointer(0);
                floatPixels = new float[ numSlices * numPixelsPerSlice ];     
                if( this->castDoubleToFloat ) {
                    this->CastDoubleToFloat(doublePixels, floatPixels, numSlices * numPixelsPerSlice); 
                } else {
                    this->MapDoubleToFloat(doublePixels, floatPixels, numSlices * numPixelsPerSlice); 
                }
                pixels = floatPixels;
            }
            if ( dataType == svkDcmHeader::SIGNED_FLOAT_4 ) {
                pixels = static_cast<vtkFloatArray*>(this->GetImageDataInput(0)->GetPointData()->GetArray(vol))->GetPointer(0);
            }
            //  If input pixels are signed ints, convert them to reals : 
            if ( dataType == svkDcmHeader::SIGNED_INT_2 ) {
                short* shortPixels; 
                shortPixels = static_cast<vtkShortArray*>(this->GetImageDataInput(0)->GetPointData()->GetArray(vol))->GetPointer(0);
                floatPixels = new float[ numSlices * numPixelsPerSlice ];     
                this->MapSignedIntToFloat(shortPixels, floatPixels, numSlices * numPixelsPerSlice); 
                pixels = floatPixels;
            }
        }


        string volName = this->InternalFileName;
        if (numVolumes > 1 ) {
            volName += "_" + svkTypeUtils::IntToString(vol + 1);
        }
        volName += extension;

        ofstream pixelsOut( volName.c_str(), ios::binary);
        if( !pixelsOut ) {
            throw runtime_error( "Cannot open file for writing: " + string(this->InternalFileName) + extension ); 
        }

        // We want to hold the pixels here so we can do an endian swap without changing the original data
        void* bigEndianPixels = NULL; 
        // Swap bytes if the CPU is NOT big endian

    #ifndef VTK_WORDS_BIGENDIAN
        if (numBytesPerPixel > 1) {
            // Allocate our new array
            bigEndianPixels = malloc( numBytesPerPixel * numPixelsPerSlice * numSlices );
            memcpy( bigEndianPixels, pixels , numBytesPerPixel * numPixelsPerSlice * numSlices ); 
            vtkByteSwap::SwapVoidRange(bigEndianPixels, numPixelsPerSlice * numSlices, numBytesPerPixel);

            // These are the pixels to write out
            pixels = bigEndianPixels;
        }
    #endif

        pixelsOut.write( (char *)pixels, numSlices * numPixelsPerSlice * numBytesPerPixel );

        if (floatPixels != NULL) {
            delete[] floatPixels; 
        }

        if (bigEndianPixels != NULL) {
            free(bigEndianPixels);
        }

        pixelsOut.close();
    }

}


/*!
 *  Write the IDF header.
 */
void svkIdfVolumeWriter::WriteHeader()
{
    int numVolumes = this->GetImageDataInput(0)->GetPointData()->GetNumberOfArrays();

    //  Write out each volume:
    for ( int vol = 0; vol < numVolumes; vol++ ) {
        string header = this->GetHeaderString(vol);

        string volName = this->InternalFileName;
        if (numVolumes > 1 ) {
            volName += "_" +  svkTypeUtils::IntToString(vol + 1);
        }
        volName += string(".idf");

        ofstream out( volName.c_str() );
        if(!out) {
            throw runtime_error("Cannot open .idf file for writing");
        }
        out << header.c_str();
        out.close();

    }

}


/*!
 *  Write the IDF header.
 */
string svkIdfVolumeWriter::GetHeaderString( int vol )
{
        
    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader();
    int numVolumes = this->GetImageDataInput(0)->GetPointData()->GetNumberOfArrays();

    ostringstream out;

    out << "IMAGE DESCRIPTOR FILE version 5" << endl;
    if ( hdr->GetStringValue( "AccessionNumber" ).length() > 0 ) {
        out << "studyid: " << hdr->GetStringValue( "AccessionNumber" ) << endl;
    } else {
        out << "studyid: " << hdr->GetStringValue( "PatientID" ) << endl;
    }
    out << "study #: " << setw(7) << hdr->GetStringValue( "StudyID" ) << endl;
    out << "series #: " << setw(7) << hdr->GetIntValue( "SeriesNumber" ) << endl;
    out << "position: ";
    string positionString = hdr->GetStringValue( "PatientPosition" );
    if ( positionString.substr(0,2) == string( "HF" ) ){
        out << "Head First, ";
    } else if ( positionString.substr(0,2) == string( "FF" ) ) {
        out << "Feet First, ";
    } else {
        out << "UNKNOWN, ";
    }

    string orientationSubstring = ""; 
    if ( positionString.length() >= 3 ) {
        orientationSubstring = positionString.substr(2); 
    }
    if ( orientationSubstring == string( "S" ) ) {
        out << "Supine" << endl;
    } else if ( orientationSubstring == string( "P" ) ) {
        out << "Prone" << endl;
    } else if ( orientationSubstring == string( "DL" ) ) {
        out << "Decubitus Left" << endl;
    } else if ( orientationSubstring == string( "DR" ) ) {
        out << "Decubitus Right" << endl;
    } else {
        out << "UNKNOWN" << endl;;
    }

    string coilName;
    if ( hdr->ElementExists( "ReceiveCoilName" ) ) {
        coilName = hdr->GetStringSequenceItemElement(
            "MRReceiveCoilSequence",
            0,
            "ReceiveCoilName",
            "SharedFunctionalGroupsSequence",
            0
        );
    } else {
        coilName = "";
    }

    out << "coil: " << coilName << endl;

    out << "orientation: ";

    double orientation[2][3];
    hdr->GetOrientation(orientation);

    int xType;
    int yType;
    int zType;

    if ( ( fabs( orientation[0][0] ) == 1 && fabs( orientation[1][1] ) == 1 ) ||
         ( fabs(orientation[0][1]) ==1 && fabs( orientation[1][0] ) == 1 ) )
    {
        out << setw(3) << 13 << "     axial normal"<<endl;
        xType = 1;
        yType = 2;
        zType = 3;
    } else if ( ( fabs( orientation[0][1] ) == 1 && fabs( orientation[1][2] ) == 1 ) ||
        ( fabs( orientation[0][2] ) == 1 && fabs( orientation[1][1] ) == 1 ) )
    {
        out << setw(3) << 11 << "     sagittal normal" << endl;
        xType = 2;
        yType = 3;
        zType = 1;
    } else if ( ( fabs( orientation[0][0] ) == 1 && fabs( orientation[1][2] ) == 1) ||
        ( fabs( orientation[0][2] ) == 1 && fabs( orientation[1][0] ) == 1 ) )
    {
        out << setw(3) << 12 << "     coronal normal" << endl;
        xType = 1;
        yType = 3;
        zType = 2;
    } else {
        out << setw(3) << 9 << "     Oblique Plane" << endl;
        xType = 0;
        yType = 0;
        zType = 0;
    }

    out << "echo/time/met index:     1     value:       1.00" << endl;

    //  if there are multiple volumes append _volNum to rootname
    if ( numVolumes > 1 ) {
        out << "rootname: " << string(FileName).substr( string(FileName).rfind("/") + 1 )
        << "_" << svkTypeUtils::IntToString(vol+1) << endl;
    } else {
        out << "rootname: " << string(FileName).substr( string(FileName).rfind("/") + 1 ) << endl;
    }



    string date = hdr->GetStringValue( "StudyDate" );
    if ( date.length() == 0 ) {
        date.assign("        ");
    } else if ( date.length() < 8 ) {
        for( int i = 0; i < 8; i++ ) {
            if( i >= date.length() ) {
                date.append(" ");
            }
        }
    }

    out << "comment: " << this->GetIDFPatientName( hdr->GetStringValue( "PatientName" ) ) << " - "
        << hdr->GetStringValue( "SeriesDescription" ) << " - "
        << date[4] << date[5] << "/" << date[6] << date[7] << "/" << date[0] << date[1] << date[2] << date[3]
        << endl;

    int vtkDataType = vtkImageData::GetScalarType( this->GetImageDataInput(0)->GetInformation() ); 
    int dataType = this->GetImageDataInput(0)->GetDcmHeader()->GetPixelDataType( vtkDataType ); 
    if (dataType == svkDcmHeader::UNSIGNED_INT_1) {
        out << "filetype:   2     entry/pixel:  1     DICOM format images" << endl;
    } else if (dataType == svkDcmHeader::UNSIGNED_INT_2) {
        out << "filetype:   3     entry/pixel:  1     DICOM format images" << endl;
    } else if (dataType == svkDcmHeader::SIGNED_INT_2) {
        //  SIGNED_INT_2 gets converted to reals
        out << "filetype:   7     entry/pixel:  1     DICOM format images" << endl;
    } else if (dataType == svkDcmHeader::SIGNED_FLOAT_4) {
        out << "filetype:   7     entry/pixel:  1     DICOM format images" << endl;
    } else if (dataType == svkDcmHeader::SIGNED_FLOAT_8) {
        out << "filetype:   7     entry/pixel:  1     DICOM format images" << endl;
    } else {
        cout << "ERROR: Can not determine IDF filetype." << endl;
        exit(1);
    }

    double center[3];
    this->GetIDFCenter(center);
    double pixelSpacing[3];
    this->GetImageDataInput(0)->GetDcmHeader()->GetPixelSpacing(pixelSpacing);

    center[2] *= -1;
    out << "dimension:  1     columns     itype: " << setw(2) << xType<< endl;
    out << "npix: " << setw(5) << hdr->GetIntValue( "Columns" )
        << "   fov(mm): "<< fixed << setw(7) << setprecision(2) << (float)hdr->GetIntValue( "Columns") * pixelSpacing[0]
        << "  center(mm): " << setw(7) << ( (xType == 0) ? 0:center[xType-1])
        << "  pixelsize(mm): " << setw(10) << setprecision(5) << pixelSpacing[0] << endl;

    out << "dimension:  2     rows        itype: " << setw(2) << yType << endl;
    out << "npix: " << setw(5) << hdr->GetIntValue( "Rows" )
        << "   fov(mm): " << fixed << setw(7) << setprecision(2) << (float)hdr->GetIntValue( "Rows") * pixelSpacing[1]
        << "  center(mm): " << setw(7) << ( (yType==0)?0:center[yType-1] )
        << "  pixelsize(mm): " << setw(10) << setprecision(5) << pixelSpacing[1] << endl;

    //zfov must take into account that slice could be skipped

    out << "dimension:  3     slices      itype: " << setw(2) << zType << endl;
    out << "npix: " << setw(5) << hdr->GetNumberOfSlices()
        << "   fov(mm): " << fixed << setw(7) << setprecision(2)
        << pixelSpacing[2] * hdr->GetNumberOfSlices()
        << "  center(mm): " << setw(7) << ((zType==0)?0:center[zType-1])
        << "  pixelsize(mm): " << setw(10) << setprecision(5)
        << pixelSpacing[2] << endl;

    center[2] *= -1;

    double pixelSize[3];
    this->GetImageDataInput(0)->GetDcmHeader()->GetPixelSize(pixelSize);
    out << "slice thickness (mm): " << setw(14) << setprecision(5) << pixelSize[2] << endl;

    double inputRangeMin;
    double inputRangeMax;
    if (dataType == svkDcmHeader::SIGNED_FLOAT_8) {
        inputRangeMin = this->globalRangeMin;
        inputRangeMax = this->globalRangeMax;
    } else {
        vtkImageAccumulate* histo = vtkImageAccumulate::New();
        histo->SetInputData( this->GetImageDataInput(0) );
        histo->Update();
        inputRangeMin = (histo->GetMin())[0];
        inputRangeMax = (histo->GetMax())[0];
        histo->Delete();
    }

    out << "minimum: " << scientific << setw(12) << setprecision(4) << inputRangeMin
    << "     maximum: " << scientific << setw(12) << setprecision(4) << inputRangeMax << endl;

    out << "scale:     1.000" << endl;
    out << "first slice read: " << setw(4) << 1
        << "   last slice read: " << setw(4) << hdr->GetIntValue("NumberOfFrames")
        << "   sliceskip:    " << 1 << endl;
    out << "LOCATION DATA IN LPS COORDINATES" << endl;
    out << "center: " << fixed<<setw(14) << setprecision(5) << center[0]
        << setw(14) << center[1] << setw(14) << center[2] << endl;

    double positionFirst[3];
    hdr->GetOrigin(positionFirst, 0);
    out << "toplc:  " << fixed << setw(14) << setprecision(5)
        << positionFirst[0]
        << setw(14) << positionFirst[1]
        << setw(14) << positionFirst[2] <<endl;

    double dcos[3][3];
    this->GetImageDataInput(0)->GetDcos(dcos);
    out << "dcos1:  " << fixed << setw(14) << setprecision(5) << dcos[0][0] << setw(14) << dcos[0][1]
        << setw(14) << dcos[0][2] << endl;
    out << "dcos2:  " << fixed << setw(14) << setprecision(5) << dcos[1][0]
            << setw(14) << dcos[1][1] << setw(14) << dcos[1][2] << endl;
    out << "dcos3:  " << fixed << setw(14) << setprecision(5) << dcos[2][0] << setw(14)
            << dcos[2][1] << setw(14) << dcos[2][2] << endl;
    return out.str();

}



/*!
 *   
 */
void svkIdfVolumeWriter::GetIDFCenter(double center[3])
{
    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader(); 

    double pixelSpacing[3];
    hdr->GetPixelSpacing(pixelSpacing);

    double slicePositionFirst[3];
    hdr->GetOrigin(slicePositionFirst, 0);

    double dcos[3][3];
    this->GetImageDataInput(0)->GetDcos(dcos);

    int numPix[3]; 
    numPix[0] =  hdr->GetIntValue("Columns");
    numPix[1] =  hdr->GetIntValue("Rows");
    numPix[2] =  hdr->GetNumberOfSlices();

    double fov[3]; 
    for(int i = 0; i < 3; i++) {
        fov[i] = static_cast<double>(numPix[i] - 1) * pixelSpacing[i];  
    }

    for (int i = 0; i < 3; i++ ){
        center[i] = slicePositionFirst[i];
        for(int j = 0; j < 3; j++ ){
            center[i] += ( dcos[j][i] * fov[j]/2. );
        }
    }

    if (this->GetDebug()) {
        cout << "space: " << pixelSpacing[0] << " " << pixelSpacing[1] << " " << pixelSpacing[2] << endl;
        cout << "center: " << center[0] << " " << center[1] << " " << center[2] << endl;
    }
}


/*!
 *   
 */
string svkIdfVolumeWriter::GetIDFPatientName(string PatientName)
{

    //  Remove DICOM delimiters:
    for (int i = 0; i < PatientName.size(); i++) {
        if ( PatientName[i] == '^') {
            PatientName[i] = ' ';
        }
    }

    //  Remove multiple spaces:
    size_t pos; 
    while ( (pos = PatientName.find("  ")) != string::npos) {
        PatientName.erase(pos, 1);     
    }

    return svkImageReader2::StripWhite( PatientName );
}


/*!
 *  converts signed shorts to float for writing as .real files.  This enables signed data to be
 *  preserved. 
 */
void svkIdfVolumeWriter::MapSignedIntToFloat(short* shortPixels, float* floatPixels, int numPixels)
{
    for (int i = 0; i < numPixels; i++) {
        floatPixels[i] = static_cast<float>( shortPixels[i] );
    }
}



/*!
 *  Calculates the global range of pixel values over all volumes:
 */
void svkIdfVolumeWriter::InitDoublePixelRange()
{

    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader();
    int numVolumes = hdr->GetNumberOfTimePoints();
    int numPixels = hdr->GetIntValue( "Columns" ) * hdr->GetIntValue( "Rows" ) *  hdr->GetNumberOfSlices();

    double* doublePixels; 
    this->globalRangeMin = VTK_DOUBLE_MAX; 
    this->globalRangeMax = VTK_DOUBLE_MIN; 

    //  Write out each volume:
    for ( int vol = 0; vol < numVolumes; vol++ ) {

        doublePixels = vtkDoubleArray::SafeDownCast(
                    this->GetImageDataInput(0)->GetPointData()->GetArray(vol)
                )->GetPointer(0);

        for (int i = 0; i < numPixels; i++ ) {
            if ( doublePixels[i] > this->globalRangeMax ) {
                this->globalRangeMax = doublePixels[i]; 
            } 
            if ( doublePixels[i] < this->globalRangeMin ) {
                this->globalRangeMin = doublePixels[i]; 
            } 
        }
    }

}


/*!
 *  Set to true if you want double values to be simply cast to floats. The default
 *  behavior is to scale the doubles to the full dynamic range of the floats.
 *  If the doubles are over range for float, will throw a runtime error.
 */
void svkIdfVolumeWriter::SetCastDoubleToFloat( bool castDoubleToFloat )
{
    this->castDoubleToFloat = castDoubleToFloat;
}


/*!
 *  Casts double inputs to floating point outputs. Uses c++ static_cast<float>
 *  to accomplish this. If the range of the input is greater than the range
 *  of floats it will throw an error.
 */
void svkIdfVolumeWriter::CastDoubleToFloat(double* doublePixels, float* floatPixels, int numPixels)
{
    double inputRangeMin = 0. ; 
    double inputRangeMax = 0. ; 
    float floatMin = VTK_FLOAT_MIN;
    float floatMax = VTK_FLOAT_MAX;
    if( this->globalRangeMin < floatMin || this->globalRangeMax > floatMax ) {
        cout << "RANGE Min/Max: " << this->globalRangeMin << " - " << this->globalRangeMax << endl;
        throw runtime_error("ERROR: Cannot cast doubles to floats-- range is too large.");
    }
    for (int i = 0; i < numPixels; i++) {
        floatPixels[i] = static_cast<float>( doublePixels[i] );
    }
}


/*!
 *
 */
void svkIdfVolumeWriter::MapDoubleToFloat(double* doublePixels, float* floatPixels, int numPixels) 
{
    //  Scale this to 32 bit values and init RescaleIntercept and RescaleSlope:

    //  Get the input range for scaling:
    double deltaRangeIn = this->globalRangeMax - this->globalRangeMin;
    if (this->GetDebug()) {
        cout << "RANGE " << this->globalRangeMax << " " << this->globalRangeMin << endl;
    }

    //  Get the output range for scaling:
    float floatMin = VTK_FLOAT_MIN;
    float floatMax = VTK_FLOAT_MAX;
    double deltaRangeOut = floatMax - floatMin;

    //  apply linear mapping from float range to signed short range;
    //  floatMax = globalRangeMax * m + b;
    //  floatMin = globalRangeMin * m + b;
    double slope = deltaRangeOut/deltaRangeIn;
    double intercept = floatMin - this->globalRangeMin * ( deltaRangeOut/deltaRangeIn );
    if (this->GetDebug()) {
        cout << "IDF Writer double to float scaling (slope, intercept): " << slope << " " << intercept << endl;
    }
    for (int i = 0; i < numPixels; i++) {
        floatPixels[i] = static_cast<float>( slope * doublePixels[i] + intercept );
    }

}


/*!
 *
 */
void svkIdfVolumeWriter::MapUnsignedToSigned( void* pixels, int numPixels ) 
{
    //  Get input and output ranges: 
    vtkUnsignedShortArray* usArray = vtkUnsignedShortArray::New();
    int maxUShort =  static_cast<int>( usArray->GetDataTypeMax() );
    int minUShort =  static_cast<int>( usArray->GetDataTypeMin() );
    int deltaRangeIn = maxUShort - minUShort;
    usArray->Delete();

    vtkShortArray* sArray = vtkShortArray::New();
    int maxShort =  static_cast<int>( sArray->GetDataTypeMax() );
    int minShort =  static_cast<int>( sArray->GetDataTypeMin() );
    int deltaRangeOut = maxShort - minShort;
    sArray->Delete();

    //  Map values to range between 0 and 1, then scale to type max.
    //  apply linear mapping between unsigned and signed short ranges
    //  maxShort = maxUShort * m + b , minShort = minUShort * m + b
    for (int i = 0; i < numPixels; i++) {
        ((short*)(pixels))[i] = 
            static_cast<short> ( 
                (deltaRangeOut/deltaRangeIn) * ((unsigned short*)(pixels))[i] + minShort 
            ); 
    }
}


/*!
 *
 */
int svkIdfVolumeWriter::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(svkIdfVolumeWriter::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
    return 1;
}


/*!
 *
 */
//void svkIdfVolumeWriter::SetInput( vtkDataObject* input )
//{
    //this->SetInput(0, input);
//}


/*!
 *
 */
//void svkIdfVolumeWriter::SetInput(int index, vtkDataObject* input)
//{
    //if(input) {
        //this->SetInputConnection(index, input->GetProducerPort());
    //} else {
        //// Setting a NULL input removes the connection.
        //this->SetInputConnection(index, 0);
    //}
//}


/*!
 *
 */
vtkDataObject* svkIdfVolumeWriter::GetInput(int port)
{
    return this->GetExecutive()->GetInputData(port, 0);
}


/*!
 *
 */
svkImageData* svkIdfVolumeWriter::GetImageDataInput(int port)
{
    return svkImageData::SafeDownCast( this->GetInput(port) );
}


