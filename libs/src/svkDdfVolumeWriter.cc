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



#include <svkDdfVolumeWriter.h>


using namespace svk;


vtkCxxRevisionMacro(svkDdfVolumeWriter, "$Rev$");
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

    //this->WriteData();
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
    out << "date of birth: " << setw(7) << hdr->GetStringValue( "PatientsBirthDate" ) <<  endl;
    out << "sex: " << setw(7) << hdr->GetStringValue( "PatientsSex" ) <<  endl;
    out << "study id: " << setw(7) << hdr->GetStringValue( "StudyID" ) <<  endl;
    out << "study code: " << setw(7) << "" <<  endl;
    out << "study date: " << setw(7) << hdr->GetStringValue( "StudyDate" ) <<  endl;
    out << "accession number: " << setw(7) << hdr->GetStringValue( "AccessionNumber" ) <<  endl;
    out << "root name: " << setw(7) << "FILENAME" <<  endl;
    out << "series number: " << setw(7) << hdr->GetStringValue( "SeriesNumber" ) <<  endl;
    out << "series description: " << setw(7) << hdr->GetStringValue( "SeriesDescription" ) <<  endl;
    out << "comment: " << setw(7) << "Comment" <<  endl;

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

    double orientation[2][3];
    hdr->GetOrientation(orientation);
    string orientationString; 

    if ( ( fabs( orientation[0][0] ) == 1 && fabs( orientation[1][1] ) == 1 ) ||
         ( fabs(orientation[0][1]) ==1 && fabs( orientation[1][0] ) == 1 ) ) {
        orientationString.assign( "axial" ); 
    } else if ( ( fabs( orientation[0][1] ) == 1 && fabs( orientation[1][2] ) == 1 ) ||
        ( fabs( orientation[0][2] ) == 1 && fabs( orientation[1][1] ) == 1 ) ) {
        orientationString.assign( "sagittal" ); 
    } else if ( ( fabs( orientation[0][0] ) == 1 && fabs( orientation[1][2] ) == 1) ||
        ( fabs( orientation[0][2] ) == 1 && fabs( orientation[1][0] ) == 1 ) ) {
        orientationString.assign( "coronal" ); 
    } else {
        orientationString.assign( "oblique" ); 
    }

    out << "orientation: " << orientationString << endl;
    out << "data type: floating point" << endl;
   
    int numComponents = 0;      
    if ( hdr->GetStringValue("DataRepresentation").compare("COMPLEX") == 0 ) {
        numComponents = 2;     
    }
    out << "number of components: " << setw(2) << numComponents << endl; 

    out << "source description: " << endl;

    int numDims = 4; 
    if ( this->GetImageDataInput(0)->GetNumberOfTimePoints()  > 1 ) { 
        numDims = 5; 
    }
    out << "number of dimensions: " << numDims << endl; 

    string specDomain;  
    string sigDomain = hdr->GetStringValue( "SignalDomainColumns" );
    if ( sigDomain.compare( "FREQUENCY" ) == 0 ) {
        specDomain.assign( "frequency" ); 
    } else if (sigDomain.compare( "TIME" ) == 0 ) {
        specDomain.assign( "time" ); 
    }
   
    int numVoxels[3];  
    numVoxels[0] = hdr->GetIntValue( "Columns" ); 
    numVoxels[1] = hdr->GetIntValue( "Rows" ); 
    numVoxels[2] = hdr->GetNumberOfSlices(); 
    
    double voxelSpacing[3]; 
    hdr->GetPixelSpacing( voxelSpacing );  
    out << "dimension 1: type: " << specDomain << " npoints: " << hdr->GetIntValue( "DataPointColumns" ) << endl;
    out << "dimension 2: type: " << "space "<< " npoints: " << numVoxels[0] << " pixel spacing(mm): " <<  setw(10) << setprecision(5) << voxelSpacing[0] << endl;
    out << "dimension 3: type: " << "space "<< " npoints: " << numVoxels[1] << " pixel spacing(mm): " <<  setw(10) << setprecision(5) << voxelSpacing[1] << endl;
    out << "dimension 4: type: " << "space "<< " npoints: " << numVoxels[2] << " pixel spacing(mm): " <<  setw(10) << setprecision(5) << voxelSpacing[2] << endl;
    if ( numDims == 5 ) {    
        out << "dimension 5: type: time" << endl ; 
    }

    float center[3];
    this->GetDDFCenter( center );
    out << "center(lps, mm):" << fixed<<setw(14) << setprecision(5) << center[0]
        << setw(14) << center[1] << setw(14) << center[2] << endl;

    double positionFirst[3]; 
    hdr->GetOrigin(positionFirst, 0);
    out << "toplc(lps, mm):  " << fixed << setw(14) << setprecision(5)
        << positionFirst[0]
        << setw(14) << positionFirst[1]
        << setw(14) << positionFirst[2] <<endl;

    double dcos[3][3];
    this->GetImageDataInput(0)->GetDcos(dcos);
    out << "dcos0:  " << fixed << setw(14) << setprecision(5) << dcos[0][0] << setw(14) << dcos[0][1]
        << setw(14) << dcos[0][2] << endl;
    out << "dcos1:  " << fixed << setw(14) << setprecision(5) << dcos[1][0]
        << setw(14) << dcos[1][1] << setw(14) << dcos[1][2] << endl;
    out << "dcos2:  " << fixed << setw(14) << setprecision(5) << dcos[2][0] << setw(14)
        << dcos[2][1] << setw(14) << dcos[2][2] << endl;

    out << "===================================================" << endl; 
    out << "MR Parameters" << endl; 

    string coilName = hdr->GetStringSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "ReceiveCoilName",
        "SharedFunctionalGroupsSequence",
        0
    );
    out << "coil name: " << coilName << endl; 

    out << "slice gap(mm): " << endl;

    float TE = hdr->GetFloatSequenceItemElement(
        "MREchoSequence",
        0,
        "EffectiveEchoTime",
        "SharedFunctionalGroupsSequence",
        0
    );
    out << "echo time(ms): " << TE << endl; 

    float TR = hdr->GetFloatSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "RepetitionTime",
        "SharedFunctionalGroupsSequence",
        0
    );
    out << "repetition time(ms): " << TR << endl;

    out << "inversion time(ms): " << endl; 
    
    float flipAngle =  hdr->GetFloatSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "FlipAngle",
        "SharedFunctionalGroupsSequence",
        0
    );
    out << "flip angle: " << flipAngle << endl;

    out << "pulse sequence name: " << hdr->GetStringValue( "PulseSequenceName" ) << endl;
    out << "transmitter frequency(MHz): " << hdr->GetFloatValue( "TransmitterFrequency" ) << endl; 
    out << "isotope: " << hdr->GetStringValue( "ResonantNucleus" ) << endl;
    out << "field strength(T): " <<  hdr->GetFloatValue( "MagneticFieldStrength" ) << endl; 
    out << "number of sat bands: " << endl;

    out << "===================================================" << endl; 
    out << "Spectroscopy Parameters" << endl; 

    out << "localization type: " << hdr->GetStringValue( "VolumeLocalizationTechnique" ) << endl;
    out << "center frequency(MHz): " << 127.718941 << endl;
    out << "ppm reference: " << hdr->GetFloatValue( "ChemicalShiftReference" ) << endl;
    out << "sweepwidth(Hz): " << hdr->GetFloatValue( "SpectralWidth") << endl;
    out << "dwelltime(ms): " << endl;
    out << "frequency offset(Hz): " << endl;
    out << "centered on water: " << endl;
    out << "suppression technique: " << endl;
    out << "residual water:" << endl;
    out << "number of acquisitions: " << endl;
    out << "chop: " << endl;
    out << "even symmetry: " << endl;
    out << "data reordered: " << endl;

    out << "acq. toplc(lps, mm): " << endl;
    out << "acq. spacing(mm): " << endl;
    out << "acq. number of data points: " << endl;
    out << "acq. number of points: " << endl;
    out << "acq. dcos1: " << endl;
    out << "acq. dcos2: " << endl;
    out << "acq. dcos3: " << endl;

    float selBoxSize[3]; 
    float selBoxCenter[3]; 

    for (int i = 0; i < 3; i++) {

        selBoxSize[i] = hdr->GetFloatSequenceItemElement(
            "VolumeLocalizationSequence",
            i,
            "SlabThickness"
        );

        selBoxCenter[i] = hdr->GetFloatSequenceItemElement(
            "VolumeLocalizationSequence",
            i,
            "MidSlabPosition"
        );

    }

    float selBoxOrientation[3][3]; 
    float tmpSelBoxOrientation[6]; 
    for (int i = 0; i < 6; i++) {

        tmpSelBoxOrientation[i] = hdr->GetFloatSequenceItemElement(
            "VolumeLocalizationSequence",
            0,
            "SlabOrientation",
            i 
        );
    }

    selBoxOrientation[0][0] = tmpSelBoxOrientation[0]; 
    selBoxOrientation[0][1] = tmpSelBoxOrientation[1]; 
    selBoxOrientation[0][2] = tmpSelBoxOrientation[2]; 
    selBoxOrientation[1][0] = tmpSelBoxOrientation[3]; 
    selBoxOrientation[1][1] = tmpSelBoxOrientation[4]; 
    selBoxOrientation[1][2] = tmpSelBoxOrientation[5]; 

    out << "selection center(lps, mm): " << fixed << setw(14) << setprecision(5) << selBoxCenter[0] 
                                         << fixed << setw(14) << setprecision(5) << selBoxCenter[1]  
                                         << fixed << setw(14) << setprecision(5) << selBoxCenter[2] 
                                         << endl;  

    out << "selection size(mm): " << fixed << setw(14) << setprecision(5) << selBoxSize[0] 
                                  << fixed << setw(14) << setprecision(5) << selBoxSize[1]  
                                  << fixed << setw(14) << setprecision(5) << selBoxSize[2] 
                                  << endl;  

    out << "selection dcos1: " << fixed << setw(14) << setprecision(5) << selBoxOrientation[0][0] 
                               << fixed << setw(14) << setprecision(5) << selBoxOrientation[0][1]  
                               << fixed << setw(14) << setprecision(5) << selBoxOrientation[0][2] 
                               << endl;  

    out << "selection dcos2: " << fixed << setw(14) << setprecision(5) << selBoxOrientation[1][0] 
                               << fixed << setw(14) << setprecision(5) << selBoxOrientation[1][1]  
                               << fixed << setw(14) << setprecision(5) << selBoxOrientation[1][2] 
                               << endl;  

    out << "selection dcos2: " << fixed << setw(14) << setprecision(5) << selBoxOrientation[1][0] 
                               << fixed << setw(14) << setprecision(5) << selBoxOrientation[1][1]  
                               << fixed << setw(14) << setprecision(5) << selBoxOrientation[1][2] 
                               << endl;  

    out << "reordered toplc(lps, mm): " << endl;
    out << "reordered center(lps, mm): " << endl;
    out << "reordered spacing(mm): " << endl;
    out << "reordered number of points: " << endl; 

    out << "reordered dcos1: " << fixed << setw(14) << setprecision(5) << selBoxOrientation[0][0] 
                               << fixed << setw(14) << setprecision(5) << selBoxOrientation[0][1]  
                               << fixed << setw(14) << setprecision(5) << selBoxOrientation[0][2] 
                               << endl;  

    out << "reordered dcos2: " << fixed << setw(14) << setprecision(5) << selBoxOrientation[1][0] 
                               << fixed << setw(14) << setprecision(5) << selBoxOrientation[1][1]  
                               << fixed << setw(14) << setprecision(5) << selBoxOrientation[1][2] 
                               << endl;  

    out << "reordered dcos2: " << fixed << setw(14) << setprecision(5) << selBoxOrientation[1][0] 
                               << fixed << setw(14) << setprecision(5) << selBoxOrientation[1][1]  
                               << fixed << setw(14) << setprecision(5) << selBoxOrientation[1][2] 
                               << endl;  

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
    hdr->GetOrigin(slicePositionLast, hdr->GetNumberOfSlices() - 1  );

    double dcos[3][3];
    this->GetImageDataInput(0)->GetDcos(dcos);

    float numPix[3]; 
    numPix[0] =  hdr->GetFloatValue("Columns");
    numPix[1] =  hdr->GetFloatValue("Rows");
    numPix[2] =  hdr->GetNumberOfSlices();

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


