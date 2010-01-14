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
 *  $URL: https://intrarad.ucsf.edu/svn/rad_software/surbeck/brain/sivic/trunk/libs/src/svkDdfVolumeWriter.cc $
 *  $Rev: 15561 $
 *  $Author: jasonc@RADIOLOGY.UCSF.EDU $
 *  $Date: 2009-11-05 10:52:19 -0800 (Thu, 05 Nov 2009) $
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */



#include <svkDdfVolumeWriter.h>


using namespace svk;


vtkCxxRevisionMacro(svkDdfVolumeWriter, "$Rev: 15561 $");
vtkStandardNewMacro(svkDdfVolumeWriter);


/*!
 *
 */
svkDdfVolumeWriter::svkDdfVolumeWriter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

}


/*!
 *
 */
svkDdfVolumeWriter::~svkDdfVolumeWriter()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}


/*!
 *  Write the UCSF DDF spectroscopy file.  Should support multiple coils (files) and multi-time point data 
 */
void svkDdfVolumeWriter::Write()
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

// based on number of coils of data:
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
 *  Write the image data pixels to the DDF data file (.int2, .real, .byt).       
 */
void svkDdfVolumeWriter::WriteData()
{
    vtkDebugMacro( << this->GetClassName() << "::WriteData()" );

    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader(); 
    int numPixelsPerSlice = hdr->GetIntValue( "Rows" ) * hdr->GetIntValue( "Columns" );
    int numSlices = hdr->GetIntValue( "NumberOfFrames" );

    int dataWordSize = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "BitsAllocated" );
    string extension = ".cmplx"; 
    int numBytesPerPixel = 4; 
    void* pixels = static_cast<vtkFloatArray*>(this->GetImageDataInput(0)->GetPointData()->GetScalars())->GetPointer(0);

    ofstream pixels_out( (this->InternalFileName + extension).c_str(), ios::binary);
    if(!pixels_out) {
        throw runtime_error("Cannot open .int2 file for writing");
    }


#if defined (linux) || defined (Darwin)
    svkByteSwap::SwapBufferEndianness((float*)pixels, numPixelsPerSlice * numSlices);
#endif

    pixels_out.write( (char *)pixels, numSlices * numPixelsPerSlice * numBytesPerPixel );

}


/*!
 *  Write the DDF header.

 */
void svkDdfVolumeWriter::WriteHeader()
{

    //write the ddf file
    ofstream out( (this->InternalFileName+string(".ddf")).c_str());
    if(!out) {
        throw runtime_error("Cannot open .ddf file for writing");
    }

    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader(); 

    out << "DATA DESCRIPTOR FILE" << endl;
    out << "version: 6.1" << endl;
    out << "object type: MR Spectroscopy" << endl;
    out << "patient id: " << setw(7) << hdr->GetStringValue( "PatientID" ) << endl;
    out << "patient name: " << setw(7) << this->GetDDFPatientsName( hdr->GetStringValue( "PatientsName" ) ) << endl;
    out << "patient code: " << setw(7) <<  endl;
    out << "date of birth: " << setw(7) << hdr->GetStringValue( "DateOfBirth" ) <<  endl;
    out << "sex: " << setw(7) << hdr->GetStringValue( "DateOfBirth" ) <<  endl;
    out << "study id: " << setw(7) << hdr->GetStringValue( "DateOfBirth" ) <<  endl;
    out << "study code: " << setw(7) << hdr->GetStringValue( "DateOfBirth" ) <<  endl;
    out << "study date: " << setw(7) << hdr->GetStringValue( "DateOfBirth" ) <<  endl;
    out << "accession number: " << setw(7) << hdr->GetStringValue( "DateOfBirth" ) <<  endl;
    out << "root name: " << setw(7) << hdr->GetStringValue( "DateOfBirth" ) <<  endl;
    out << "series number: " << setw(7) << hdr->GetStringValue( "DateOfBirth" ) <<  endl;
    out << "series description: " << setw(7) << hdr->GetStringValue( "DateOfBirth" ) <<  endl;
    out << "comment: " << setw(7) << hdr->GetStringValue( "DateOfBirth" ) <<  endl;

//  The following could be in a base class of UCSF file format utils (getorientation, position, patient entry, etc. ):

    out << "patient entry: ";
    string position_string = hdr->GetStringValue( "PatientPosition" );
    if ( position_string.substr(0,2) == string( "HF" ) ){
        out << "Head First, ";
    } else if ( position_string.substr(0,2) == string( "FF" ) ) {
        out << "Feet First, ";
    } else {
        out << "UNKNOWN, ";
    }
    out << endl;

    out << "patient position: ";
    if ( position_string.substr(2) == string( "S" ) ) {
        out << "Supine" << endl;
    } else if ( position_string.substr(2) == string( "P" ) ) {
        out << "Prone" << endl;
    } else if ( position_string.substr(2) == string( "DL" ) ) {
        out << "Decubitus Left" << endl;
    } else if ( position_string.substr(2) == string( "DR" ) ) {
        out << "Decubitus Right" << endl;
    } else {
        out << "UNKNOWN" << endl;;
    }
    out << endl;

//orientation: axial
//data type: floating point
//number of components: 2
//source description:
//number of dimensions: 4
//dimension 1: type: frequency npoints: 512
//dimension 2: type: space npoints: 12 pixel spacing(mm): 10.000000
//dimension 3: type: space npoints: 12 pixel spacing(mm): 10.000000
//dimension 4: type: space npoints: 8 pixel spacing(mm): 10.000000
//center(lps, mm):       16.39050     -28.51190      52.09760
//toplc(lps, mm):       -38.60950     -83.51190      87.09760
//dcos0:        1.00000       0.00000       0.00000
//dcos1:        0.00000       1.00000       0.00000
//dcos2:        0.00000       0.00000      -1.00000

    out << "===================================================" << endl; 
    out << "MR Parameters<< endl; 

    string coilName = hdr->GetStringSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "ReceiveCoilName",
        "SharedFunctionalGroupsSequence",
        0
    );

    out << "coil name: " << coilName << endl; 

    float orientation[6];
    this->GetDDFOrientation( orientation );    

    int x_type, y_type, z_type;

    if ( (fabs(orientation[0])==1 && fabs(orientation[4])==1) || 
         (fabs(orientation[1])==1 && fabs(orientation[3])==1) )
    {
        out << setw(3) << 13 << "     axial normal"<<endl;
        x_type = 1;
        y_type = 2;
        z_type = 3;
    } else if ( (fabs(orientation[1])==1 && fabs(orientation[5])==1) || 
        (fabs(orientation[2])==1 && fabs(orientation[4])==1) )
    {
        out << setw(3) << 11 << "     sagittal normal" << endl;
        x_type = 2;
        y_type = 3;
        z_type = 1;
    } else if ( (fabs(orientation[0])==1 && fabs(orientation[5])==1) || 
        (fabs(orientation[2])==1 && fabs(orientation[3])==1) )
    {
        out << setw(3) << 12 << "     coronal normal" << endl;
        x_type = 1;
        y_type = 3;
        z_type = 2;
    } else {
        out << setw(3) << 9 << "     Oblique Plane" << endl;
        x_type = 0;
        y_type = 0;
        z_type = 0;
    }


    string date = hdr->GetStringValue( "StudyDate" );
    out << "comment: " << this->GetDDFPatientsName( hdr->GetStringValue( "PatientsName" ) ) << "- "
        << hdr->GetStringValue( "SeriesDescription" ) << " - "
        << date[4] << date[5] << "/" << date[6] << date[7] << "/" << date[0] << date[1] << date[2] << date[3]
        << endl;
    
    int dataType = this->GetImageDataInput(0)->GetDcmHeader()->GetPixelDataType();
    if (dataType == svkDcmHeader::UNSIGNED_INT_1) {
        out << "filetype:   2     entry/pixel:  1     DICOM format images" << endl;
    } else if (dataType == svkDcmHeader::UNSIGNED_INT_2) {
        out << "filetype:   3     entry/pixel:  1     DICOM format images" << endl;
    } else if (dataType == svkDcmHeader::SIGNED_FLOAT_4) {
        out << "filetype:   7     entry/pixel:  1     DICOM format images" << endl;
    }

    
    float center[3];
    this->GetDDFCenter(center);
    double pixelSpacing[3];
    this->GetImageDataInput(0)->GetDcmHeader()->GetPixelSpacing(pixelSpacing);
    
    center[2] *= -1;
    out << "dimension:  1     columns     itype: " << setw(2) << x_type<< endl;
    out << "npix: " << setw(5) << hdr->GetIntValue( "Columns" )
        << "   fov(mm): "<< fixed << setw(7) << setprecision(2) << (float)hdr->GetIntValue( "Columns") * pixelSpacing[0] 
        << "  center(mm): " << setw(7) << ( (x_type == 0) ? 0:center[x_type-1])
        << "  pixelsize(mm): " << setw(10) << setprecision(5) << pixelSpacing[0] << endl;

    out << "dimension:  2     rows        itype: " << setw(2) << y_type << endl;
    out << "npix: " << setw(5) << hdr->GetIntValue( "Rows" )
        << "   fov(mm): " << fixed << setw(7) << setprecision(2) << (float)hdr->GetIntValue( "Rows") * pixelSpacing[1] 
        << "  center(mm): " << setw(7) << ( (y_type==0)?0:center[y_type-1] )
        << "  pixelsize(mm): " << setw(10) << setprecision(5) << pixelSpacing[1] << endl;

    //zfov must take into account that slice could be skipped
    
    out << "dimension:  3     slices      itype: " << setw(2) << z_type << endl;
    out << "npix: " << setw(5) << hdr->GetIntValue("NumberOfFrames")
        << "   fov(mm): " << fixed << setw(7) << setprecision(2)
        << pixelSpacing[2] * hdr->GetFloatValue("NumberOfFrames")
        << "  center(mm): " << setw(7) << ((z_type==0)?0:center[z_type-1])
        << "  pixelsize(mm): " << setw(10) << setprecision(5)  
        << pixelSpacing[2] << endl;

    center[2] *= -1;
    
    double pixelSize[3];
    this->GetImageDataInput(0)->GetDcmHeader()->GetPixelSize(pixelSize);
    out << "slice thickness (mm): " << setw(14) << setprecision(5) << pixelSize[2] << endl;

    vtkImageAccumulate* histo = vtkImageAccumulate::New();
cout << "HISTO INPUT: " << *(this->GetImageDataInput(0) ) << endl;
    histo->SetInput( this->GetImageDataInput(0) );
    histo->Update();

    out << "minimum: " << scientific << setw(12) << setprecision(4) << (histo->GetMin())[0]
    << "     maximum: " << scientific << setw(12) << setprecision(4) << (histo->GetMax())[0] << endl;
    histo->Delete();

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

}


/*!
 *   
 */
void svkDdfVolumeWriter::GetDDFOrientation(float orientation[6])
{

    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader(); 

    for(int j=0; j<6; j++) {
        istringstream* iss = new istringstream();
        iss->str( 
            hdr->GetStringSequenceItemElement( 
                "PlaneOrientationSequence",
                0,
                "ImageOrientationPatient",
                j,
                "SharedFunctionalGroupsSequence",
                0
            ) 
        );
        *iss >> orientation[j];
        delete iss; 
    }
}


/*!
 *   
 */
void svkDdfVolumeWriter::GetDDFCenter(float center[3])
{

    float centerFirst[3];
    float centerLast[3];
    double pixelSpacing[3];
    this->GetImageDataInput(0)->GetDcmHeader()->GetPixelSpacing(pixelSpacing);

    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader(); 

    double slicePositionFirst[3];
    hdr->GetOrigin(slicePositionFirst, 0);
    double slicePositionLast[3];
    hdr->GetOrigin(slicePositionLast, hdr->GetIntValue("NumberOfFrames") - 1  );

    double dcos[3][3];
    this->GetImageDataInput(0)->GetDcos(dcos);

    float numPix[3]; 
    numPix[0] =  hdr->GetFloatValue("Columns");
    numPix[1] =  hdr->GetFloatValue("Rows");
    numPix[2] =  hdr->GetFloatValue("NumberOfFrames");

    float fov[3]; 
    for(int i = 0; i < 3; i++) {
        fov[i] = (numPix[i] - 1) * pixelSpacing[i];  
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
string svkDdfVolumeWriter::GetDDFPatientsName(string patientsName)
{

    //  Remove DICOM delimiters:
    for (int i = 0; i < patientsName.size(); i++) {
        if ( patientsName[i] == '^') {
            patientsName[i] = ' ';
        }
    }

    //  Remove multiple spaces:
    size_t pos; 
    while ( (pos = patientsName.find("  ")) != string::npos) {
        patientsName.erase(pos, 1);     
    }

    return patientsName;
}


/*!
 *
 */
int svkDdfVolumeWriter::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(svkDdfVolumeWriter::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData");
    return 1;
}


/*!
 *
 */
vtkDataObject* svkDdfVolumeWriter::GetInput(int port)
{
    return this->GetExecutive()->GetInputData(port, 0);
}


/*!
 *
 */
svkImageData* svkDdfVolumeWriter::GetImageDataInput(int port)
{
    return svkImageData::SafeDownCast( this->GetInput(port) );
}


