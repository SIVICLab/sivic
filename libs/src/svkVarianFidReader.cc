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


#include <svkVarianFidReader.h>


using namespace svk;


vtkCxxRevisionMacro(svkVarianFidReader, "$Rev$");
vtkStandardNewMacro(svkVarianFidReader);


/*!
 *
 */
svkVarianFidReader::svkVarianFidReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkVarianFidReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->pixelData = NULL;
    this->dataArray = NULL; 
    this->fidFile = NULL;
    this->procparFile = NULL;
    this->fileSize = 0;
}


/*!
 *
 */
svkVarianFidReader::~svkVarianFidReader()
{

    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if (this->dataArray != NULL) {
        this->dataArray->Delete();
        this->dataArray = NULL; 
    }

    if ( this->fidFile != NULL )  {
        delete fidFile; 
        this->fidFile = NULL; 
    }

    if ( this->procparFile != NULL )  {
        delete procparFile; 
        this->procparFile = NULL; 
    }
}



/*!
 *  Check to see if the extension indicates a Varian FDF file.  If so, try 
 *  to open the file for reading.  If that works, then return a success code. 
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkVarianFidReader::CanReadFile(const char* fname)
{

    string fileToCheck(fname);

    if( fileToCheck.size() >= 3 ) {

        // Also see "vtkImageReader2::GetFileExtensions method" 
        if (  fileToCheck.substr( fileToCheck.size() - 3 ) == "fid" ) {
            FILE *fp = fopen(fname, "rb");
            if (fp) {
                fclose(fp);
                vtkDebugMacro(
                    << this->GetClassName() << "::CanReadFile(): It's a Varian FID File: " << fileToCheck
                );
                return 1;
            }
        } else {
            vtkDebugMacro(
                << this->GetClassName() << "::CanReadFile(): It's NOT a Varian FID File: " << fileToCheck
            );
            return 0;
        }
    } else {
        vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): is NOT a valid file: " << fileToCheck);
        return 0;
    }
}


/*!
 *  Reads pixel data from all fid files. 
 *  For .fid series, each file contains 1/num_files_in_series worth of pixels. 
 */
void svkVarianFidReader::ReadFidFiles()
{

    vtkDebugMacro( << this->GetClassName() << "::ReadFidFiles()" );

    for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {

        ifstream* volumeDataIn = new ifstream();
        volumeDataIn->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        int numBitsPerByte = 8;
        int pixelWordSize = this->GetHeaderValueAsInt("bits")/numBitsPerByte;

        int numBytesInVol = (this->GetNumPixelsInVol() * pixelWordSize);
        int numFilesInVol = this->GetFileNames()->GetNumberOfValues(); 
        int numBytesInFile = numBytesInVol/numFilesInVol;  
        volumeDataIn->open( this->GetFileNames()->GetValue( fileIndex ), ios::binary );

        /*
        *   Flatten the data volume into one dimension
        */
        if (this->pixelData == NULL) {
            this->pixelData = (void* ) malloc( numBytesInVol); 
        }

        this->fidFile->seekg(0, ios::end);     
        volumeDataIn->seekg(-1 * numBytesInFile, ios::end);
        int offset = (fileIndex * numBytesInFile);
        volumeDataIn->read( (char *)(this->pixelData) + offset, numBytesInFile);
        volumeDataIn->close();
        delete volumeDataIn;
    }

    /*  
     *  If this is running on linux, and the input is bigendian, then swap bytes:
     *  Otherwise, if this is runnin on Solaris/Sparc and the input is NOT bigendian
     *  also swap bytes: 
     */
#if defined (linux) || defined(Darwin)
    if ( this->GetHeaderValueAsInt("bigendian") != 0 ) {
        svkByteSwap::SwapBufferEndianness( (float*)pixelData, this->GetNumPixelsInVol() );
    }
#else
    if ( this->GetHeaderValueAsInt("bigendian") != 1 ) {
        svkByteSwap::SwapBufferEndianness( (float*)pixelData, this->GetNumPixelsInVol() );
    }
#endif
  
}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called after ExecuteInformation()
 */
void svkVarianFidReader::ExecuteData(vtkDataObject* output)
{

    this->FileNames = vtkStringArray::New(); 
    this->FileNames->DeepCopy(this->tmpFileNames); 
    this->tmpFileNames->Delete(); 
    this->tmpFileNames = NULL; 

    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );
    svkImageData* data = svkImageData::SafeDownCast( this->AllocateOutputData(output) );

    if ( this->GetFileNames()->GetNumberOfValues() ) {
        vtkDebugMacro( << this->GetClassName() << " FileName: " << FileName );
        struct stat fs;
        if ( stat(this->GetFileNames()->GetValue(0), &fs) ) {
            vtkErrorMacro("Unable to open file " << string(this->GetFileNames()->GetValue(0)) );
            return;
        }

        this->ReadFidFiles();

        //  If input is float, convert to short int (16 bit depth):     
        this->dataArray->SetVoidArray( (void*)(this->pixelData), GetNumPixelsInVol(), 0);

        this->Superclass::Superclass::Superclass::GetOutput()->SetScalarType(VTK_FLOAT);

        data->GetPointData()->SetScalars(this->dataArray);

    }

    /* 
     * We need to make a shallow copy of the output, otherwise we would have it
     * registered twice to the same reader which would cause the reader to never delete.
     */
    double dcos[3][3];
    this->GetOutput()->GetDcmHeader()->SetSliceOrder( this->dataSliceOrder );
    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
    this->GetOutput()->SetDcos(dcos);

    //  SetNumberOfIncrements is supposed to call this, but only works if the data has already
    //  been allocated. but that requires the number of components to be specified.
    this->GetOutput()->GetIncrements();

}


/*!
 *  Side effect of Update() method.  Used to initialize the svkDcmHeader member of 
 *  the target svkImageData object and uses the header to set up the Output Informatin.
 *  Called before ExecuteData()
 */
void svkVarianFidReader::ExecuteInformation()
{
    vtkDebugMacro( << this->GetClassName() << "::ExecuteInformation()" );

    if (this->FileName == NULL) {
        return;
    }

    if ( this->FileName ) {
        vtkDebugMacro( << this->GetClassName() << " FileName: " << FileName );

        struct stat fs;
        if ( stat(this->FileName, &fs) ) {
            vtkErrorMacro("Unable to open file " << this->FileName );
            return;
        }

        this->InitDcmHeader();
        this->SetupOutputInformation();
    }

    //  This is a workaround required since the vtkImageAlgo executive 
    //  for the reder resets the Extent[5] value to the number of files
    //  which is not correct for 3D multislice volume files. So store
    //  the files in a temporary array until after ExecuteData has been 
    //  called, then reset the array.  
    this->tmpFileNames = vtkStringArray::New(); 
    this->tmpFileNames->DeepCopy(this->FileNames); 
    this->FileNames->Delete(); 
    this->FileNames = NULL; 
}


/*!
 *  Initializes the svkDcmHeader adapter to a specific IOD type
 *  and initizlizes the svkDcmHeader member of the svkImageData
 *  object.
 */
void svkVarianFidReader::InitDcmHeader()
{

    vtkDebugMacro( << this->GetClassName() << "::InitDcmHeader()" );

    svkIOD* iod = svkMRSIOD::New();
    iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    iod->InitDcmHeader();
    iod->Delete();

    //  Read the fid header into a map of values used to initialize the
    //  DICOM header. 
    this->ParseFid(); 

    this->InitPatientModule();
    this->InitGeneralStudyModule();
    this->InitGeneralSeriesModule();
    this->InitGeneralEquipmentModule();

    this->InitMultiFrameFunctionalGroupsModule();
//    this->InitMultiFrameDimensionModule();
//    this->InitAcquisitionContextModule();
//    this->InitMRSpectroscopyModule();
//    this->InitMRSpectroscopyPulseSequenceModule();

    this->InitMRSpectroscopyDataModule();

    if (this->GetDebug()) {
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }
}


/*!
 *  Returns the file type enum 
 */
svkDcmHeader::DcmPixelDataFormat svkVarianFidReader::GetFileType()
{
    svkDcmHeader::DcmPixelDataFormat format = svkDcmHeader::SIGNED_FLOAT_4;

    return format; 
}


/*!
 *
 */
void svkVarianFidReader::InitPatientModule()
{
    this->GetOutput()->GetDcmHeader()->SetDcmPatientsName( this->GetHeaderValueAsString("samplename") );
    this->GetOutput()->GetDcmHeader()->SetValue( "PatientID", this->GetHeaderValueAsString("dataid"));
    this->GetOutput()->GetDcmHeader()->SetValue( "PatientsBirthDate", this->GetHeaderValueAsString("birthday") );
    this->GetOutput()->GetDcmHeader()->SetValue( "PatientsSex", this->GetHeaderValueAsString("gender") );
}


/*!
 *
 */
void svkVarianFidReader::InitGeneralStudyModule()
{
    this->GetOutput()->GetDcmHeader()->SetValue(
        "StudyDate",
        this->GetHeaderValueAsString("date") 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "StudyID",
        this->GetHeaderValueAsString("studyid_") 
    );
}


/*!
 *
 */
void svkVarianFidReader::InitGeneralSeriesModule()
{

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SeriesNumber",
        0 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SeriesDescription",
        "Varian FID Data"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "PatientPosition",
        "UNKNOWN" 
    );


}


/*!
 *  DDF is historically the UCSF representation of a GE raw file so
 *  initialize to svkVarianFidReader::MFG_STRING.
 */
void svkVarianFidReader::InitGeneralEquipmentModule()
{
    this->GetOutput()->GetDcmHeader()->SetValue(
        "Manufacturer",
        "Varian"
    );
}


/*!
 *
 */
void svkVarianFidReader::InitMRSpectroscopyDataModule()
{
    this->GetOutput()->GetDcmHeader()->SetValue( "Columns", this->GetHeaderValueAsInt("nv", 0) );
    this->GetOutput()->GetDcmHeader()->SetValue( "Rows", this->GetHeaderValueAsInt("nv2", 0) );
    this->GetOutput()->GetDcmHeader()->SetValue( "DataPointRows", 0 );
    this->GetOutput()->GetDcmHeader()->SetValue( "DataPointColumns", this->GetHeaderValueAsInt("np", 0)/2 );
    this->GetOutput()->GetDcmHeader()->SetValue( "DataRepresentation", "COMPLEX" );
    this->GetOutput()->GetDcmHeader()->SetValue( "SignalDomainColumns", "TIME" );
    this->GetOutput()->GetDcmHeader()->SetValue( "SVK_ColumnsDomain", "KSPACE" );
    this->GetOutput()->GetDcmHeader()->SetValue( "SVK_RowsDomain", "KSPACE" );
    this->GetOutput()->GetDcmHeader()->SetValue( "SVK_SliceDomain", "KSPACE" );
}



/*! 
 *  
 */
void svkVarianFidReader::InitMultiFrameFunctionalGroupsModule()
{

    this->InitSharedFunctionalGroupMacros();

    this->GetOutput()->GetDcmHeader()->SetValue(
        "InstanceNumber",
        1
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ContentDate",
        this->GetHeaderValueAsString( "time_svfdate" )
    );

    this->numSlices = this->GetHeaderValueAsInt("ns");
    int numEchoes = this->GetHeaderValueAsInt("ne");

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "NumberOfFrames", 
        this->numSlices * numEchoes 
    ); 

    this->InitPerFrameFunctionalGroupMacros();

}


/*! 
 *  
 */
void svkVarianFidReader::InitMultiFrameDimensionModule()
{
}


/*! 
 *  
 */
void svkVarianFidReader::InitAcquisitionContextModule()
{
}


/*! 
 *  
 */
void svkVarianFidReader::InitSharedFunctionalGroupMacros()
{

    this->InitPixelMeasuresMacro();
    this->InitPlaneOrientationMacro();
/*
    this->InitFrameAnatomyMacro();
    this->InitMRSpectroscopyFrameTypeMacro();
    this->InitMRTimingAndRelatedParametersMacro();
    this->InitMRSpectroscopyFOVGeometryMacro();
    this->InitMREchoMacro();
    this->InitMRModifierMacro();
    this->InitMRReceiveCoilMacro();
    this->InitMRTransmitCoilMacro();
    this->InitMRAveragesMacro();
    this->InitMRSpatialSaturationMacro();
    //this->InitMRSpatialVelocityEncodingMacro();
*/

}


/*! 
 *  
 */
void svkVarianFidReader::InitPerFrameFunctionalGroupMacros()
{

    this->InitFrameContentMacro();
    this->InitPlanePositionMacro();

}



/*!
 *  Mandatory, Must be a per-frame functional group
 */
void svkVarianFidReader::InitFrameContentMacro()
{

    int numCoils = 1; 
    int numTimePts = 1; 
    int numFrameIndices = svkDcmHeader::GetNumberOfDimensionIndices( numTimePts, numCoils ) ;

    unsigned int* indexValues = new unsigned int[numFrameIndices];

    int frame = 0;

    for (int coilNum = 0; coilNum < numCoils; coilNum++) {

        for (int timePt = 0; timePt < numTimePts; timePt++) {

            for (int sliceNum = 0; sliceNum < this->numSlices; sliceNum++) {

                svkDcmHeader::SetDimensionIndices(
                    indexValues, numFrameIndices, sliceNum, timePt, coilNum, numTimePts, numCoils
                );

                this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
                    "PerFrameFunctionalGroupsSequence",
                    frame,
                    "FrameContentSequence"
                );

                this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
                    "FrameContentSequence",
                    0,
                    "DimensionIndexValues",
                    indexValues,        //  array of vals
                    numFrameIndices,    // num values in array
                    "PerFrameFunctionalGroupsSequence",
                    frame
                );

                this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
                    "FrameContentSequence",
                    0,
                    "FrameAcquisitionDateTime",
                    "EMPTY_ELEMENT",
                    "PerFrameFunctionalGroupsSequence",
                    frame 
                );
    
                this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
                    "FrameContentSequence",
                    0,
                    "FrameReferenceDateTime",
                    "EMPTY_ELEMENT",
                    "PerFrameFunctionalGroupsSequence",
                    frame 
                );
    
                this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
                    "FrameContentSequence",
                    0,
                    "FrameAcquisitionDuration",
                    "-1",
                    "PerFrameFunctionalGroupsSequence",
                    frame 
                );
    
                frame++; 
            }
        }
    }

    delete[] indexValues;

}


/*!
 *  The FID toplc is the center of the first voxel. 
 */
void svkVarianFidReader::InitPlanePositionMacro()
{

    double dcos[3][3];
    this->GetOutput()->GetDcmHeader()->SetSliceOrder( this->dataSliceOrder );
    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
    double pixelSpacing[3];
    this->GetOutput()->GetDcmHeader()->GetPixelSize(pixelSpacing); 

    //  Get center coordinate float array from fidMap and use that to generate 
    //  Displace from that coordinate by 1/2 fov - 1/2voxel to get to the center of the
    //  toplc from which the individual frame locations are calculated

    //  If volumetric 3D, get the center of the TLC voxel in LPS coords: 
    float* volumeTlcLPSFrame = new float[3];  
    if (GetHeaderValueAsInt("rank") == 3) {

        //  Get the volumetric center in acquisition frame coords: 
        float volumeCenterAcqFrame[3];  
        for (int i = 0; i < 3; i++) {
            volumeCenterAcqFrame[i] = this->GetHeaderValueAsFloat("location[]", i); 
        }

        float* volumeTlcAcqFrame = new float[3];  
        for (int i = 0; i < 3; i++) {
            volumeTlcAcqFrame[i] = volumeCenterAcqFrame[i] 
                                 + ( this->GetHeaderValueAsFloat("span[]", i) - pixelSpacing[i] )/2; 
        }
        this->AcqToLPS(volumeTlcAcqFrame, volumeTlcLPSFrame, dcos);  
        delete [] volumeTlcAcqFrame;
        
    }

    float displacement[3];
    //  Center of toplc (LPS) pixel in frame:  
    float frameLPSPosition[3];

    /*  
     *  Iterate over slices (frames)
     *  If 3D vol, calculate slice position, otherwise use value encoded 
     *  into slice header
     */
    for (int i = 0; i < this->GetHeaderValueAsInt("matrix[]", 2); i++) {

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "PerFrameFunctionalGroupsSequence",
            i,
            "PlanePositionSequence"
        );

        //  Need to displace along normal from tlc of slice: 
        //  add displacement along normal vector to get toplc for each frame:
        for (int j = 0; j < 3; j++) {
            displacement[j] = dcos[2][j] * pixelSpacing[2] * i;
        }

        string imagePositionPatient;

        if (GetHeaderValueAsInt("rank") == 2) {

            //  Location is the center of the image frame in user (acquisition frame). 
            float centerAcqFrame[3];  
            for ( int j = 0; j < 3; j++) {
                centerAcqFrame[j] = this->GetHeaderValueAsFloat("location[]", i * 3 + j ) ;
            }

            //  Now get the center of the tlc voxel in the acq frame: 
            float* tlcAcqFrame = new float[3];  
            for (int j = 0; j < 2; j++) {
                tlcAcqFrame[j] = centerAcqFrame[j] 
                                 - ( this->GetHeaderValueAsFloat("span[]", j) - pixelSpacing[j] )/2; 
            }
            tlcAcqFrame[2] = centerAcqFrame[2]; 

            //  and convert to LPS (magnet) frame: 
            this->AcqToLPS(tlcAcqFrame, frameLPSPosition, dcos);  
                
            delete [] tlcAcqFrame; 

        } else {

            for(int j = 0; j < 3; j++) { //L, P, S
                frameLPSPosition[j] = volumeTlcLPSFrame[j] +  displacement[j] ;
            }

        }

        for (int j = 0; j < 3; j++) {
            ostringstream oss;
            oss.precision(8);
            oss << frameLPSPosition[j];
            imagePositionPatient += oss.str();
            if (j < 2) {
                imagePositionPatient += '\\';
            }
        }

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "PlanePositionSequence",
            0,
            "ImagePositionPatient",
            imagePositionPatient,
            "PerFrameFunctionalGroupsSequence",
            i
        );
    }
}


/*!
 *  Pixel Spacing:
 */
void svkVarianFidReader::InitPixelMeasuresMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PixelMeasuresSequence"
    );


    float numPixels[3]; 
    numPixels[0] = this->GetHeaderValueAsInt("nv", 0);
    numPixels[1] = this->GetHeaderValueAsInt("nv2", 0);
    numPixels[2] = this->GetHeaderValueAsInt("ns", 0);

    float pixelSize[3]; 
    pixelSize[0] = this->GetHeaderValueAsFloat("vox1", 0);
    pixelSize[1] = this->GetHeaderValueAsFloat("vox2", 0);
    pixelSize[2] = this->GetHeaderValueAsFloat("vox3", 0);

    string pixelSizeString[3]; 

    for (int i = 0; i < 3; i++) {
        ostringstream oss;
        oss << pixelSize[i];
        pixelSizeString[i].assign( oss.str() );
    }

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PixelMeasuresSequence",
        0,
        "PixelSpacing",
        pixelSizeString[0] + "\\" + pixelSizeString[1], 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PixelMeasuresSequence",
        0,
        "SliceThickness",
        pixelSize[2],
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *  
 */
void svkVarianFidReader::InitPlaneOrientationMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PlaneOrientationSequence"
    );

    string orientationString;

    for (int i = 0; i < 6; i++) {
        orientationString.append( GetHeaderValueAsString("orientation[]", i) );
        if (i < 5) {
            orientationString.append( "\\");
        }
    }

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PlaneOrientationSequence",
        0,
        "ImageOrientationPatient",
        orientationString,
        "SharedFunctionalGroupsSequence",
        0
    );


    //  Determine whether the data is ordered with or against the slice normal direction.
    double normal[3];
    this->GetOutput()->GetDcmHeader()->GetNormalVector(normal);

    double dcosSliceOrder[3];
    for (int j = 0; j < 3; j++) {
        dcosSliceOrder[j] =  this->GetHeaderValueAsFloat("orientation[]", j + 6 );
    }

    //  Use the scalar product to determine whether the data in the .cmplx
    //  file is ordered along the slice normal or antiparalle to it.
    vtkMath* math = vtkMath::New();
    if (math->Dot(normal, dcosSliceOrder) > 0 ) {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
    } else {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_NEG_NORMAL;
    }


}


/*!
 *  Receive Coil:
 */
void svkVarianFidReader::InitMRReceiveCoilMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRReceiveCoilSequence"
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "ReceiveCoilName",
        "Varian Coil",
        "SharedFunctionalGroupsSequence",
        0
    );

}



/*! 
 *  Use the FDF patient position string to set the DCM_PatientPosition data element.
 */
string svkVarianFidReader::GetDcmPatientPositionString(string patientPosition)
{
    size_t delim = patientPosition.find_first_of(',');
    string headFeetFirst( patientPosition.substr(0, delim) );

    for(int i = 0; i < headFeetFirst.size(); i++){
        headFeetFirst[i] = tolower( headFeetFirst[i] );
    }

    string dcmPatientPosition;
    if( headFeetFirst.find("head first") != string::npos ) {
        dcmPatientPosition.assign("HF");
    } else if( headFeetFirst.find("feet first") != string::npos ) {
        dcmPatientPosition.assign("FF");
    } else {
        dcmPatientPosition.assign("UNKNOWN");
    }

    //  skip ", ":
    string spd( patientPosition.substr(delim + 2) );
    for(int i = 0; i < spd.size(); i++){
        spd[i] = tolower( spd[i] );
    }

    if( spd.find("supine") != string::npos ) {
        dcmPatientPosition += "S";
    } else if( spd.find("prone") != string::npos ) {
        dcmPatientPosition += "P";
    } else if( spd.find("decubitus left") != string::npos ) {
        dcmPatientPosition += "DL";
    } else if( spd.find("decubitus right") != string::npos ) {
        dcmPatientPosition += "DR";
    } else {
        dcmPatientPosition += "UNKNOWN";
    }

    return dcmPatientPosition; 
}


/*
 *  Read FID header blocks into a string STL map for use during initialization 
 *  of DICOM header by Init*Module methods. 
 *  If a procpar file is present in the directory, parse that as well. 
 */
void svkVarianFidReader::ParseFid()
{

    string fidFileName( this->GetFileName() );  
    string fidFileExtension( this->GetFileExtension( this->GetFileName() ) );  
    string fidFilePath( this->GetFilePath( this->GetFileName() ) );  

    vtkGlobFileNames* globFileNames = vtkGlobFileNames::New();
    globFileNames->AddFileNames( string( fidFilePath + "/*." + fidFileExtension).c_str() );

    vtkSortFileNames* sortFileNames = vtkSortFileNames::New();
    sortFileNames->GroupingOn(); 
    sortFileNames->SetInputFileNames( globFileNames->GetFileNames() );
    sortFileNames->NumericSortOn();
    sortFileNames->Update();

    //  If globed file names are not similar, use only the specified file
    if (sortFileNames->GetNumberOfGroups() > 1 ) {

        vtkWarningWithObjectMacro(this, "Found Multiple fid file groups, using only specified file ");   

        vtkStringArray* fileNames = vtkStringArray::New(); 
        fileNames->SetNumberOfValues(1);  
        fileNames->SetValue(0, this->GetFileName() ); 
        sortFileNames->SetInputFileNames( fileNames ); 
        fileNames->Delete(); 

    } 

    this->SetFileNames( sortFileNames->GetFileNames() );
    vtkStringArray* fileNames =  sortFileNames->GetFileNames();
    for (int i = 0; i < fileNames->GetNumberOfValues(); i++) {
        cout << "FN: " << fileNames->GetValue(i) << endl; 
    } 
    //this->SetDataByteOrderToLittleEndian

    try { 

        /*  Read in the FDF Header:
         *  for image 1 read everything.  
         *  for subsequent images in the series get "slice_no" and "location[]" elements
         *  and append these to the existing map value in the order read.  Also, add a 
         *  file name element to map also in this order. 
         */
/*
        this->fidFile = new ifstream();
        this->fidFile->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {

            string currentFidFileName( this->GetFileNames()->GetValue( fileIndex ) ); 

            this->fidFile->open( currentFidFileName.c_str(), ifstream::in );
            if ( ! this->fidFile->is_open() ) {
                throw runtime_error( "Could not open fid file: " + currentFidFileName );
            } 

            this->ParseAndSetStringElements("FileName", currentFidFileName);

            // determine how big the data buffer is (num pts * word size).  
            // header key-value pairs use total_bytes_in_file - sizeof_data_buffer
            // read key-value pairs from the top until start of data buffer. 
            this->fileSize = this->GetFileSize( this->fidFile );

            vtkStringArray* keysToFind = vtkStringArray::New(); 
            SetKeysToSearch(keysToFind, fileIndex);

            while (! this->fidFile->eof() ) {
                this->GetFidKeyValuePair(keysToFind);
            }

            this->fidFile->close();
        }

        if (GetHeaderValueAsInt("rank") == 2) {
            this->AddDimensionTo2DData();
        }
*/

        this->ParseProcpar(fidFilePath);

        this->PrintProcparKeyValuePairs();

    } catch (const exception& e) {
        cerr << "ERROR opening or reading Varian fid file (" << fidFileName << "): " << e.what() << endl;
    }

    globFileNames->Delete();
    sortFileNames->Delete();
}


/*! 
 *  Utility function to read a single line from the fid file and return 
 *  set the delimited key/value pair into the stl map.  
 */
void svkVarianFidReader::GetFidKeyValuePair( vtkStringArray* keySet )    
{

    istringstream* iss = new istringstream();

    string keyString;
    string valueString;

    try {

        this->ReadLine(this->fidFile, iss); 

        size_t  position; 
        string  tmp; 
        string  dataType; 
        long    headerSize;

        int dataBufferSize = this->GetDataBufferSize();

        //  Read only to the start of the pixel buffer, 
        //  i.e. no more than the header size:     
        headerSize = this->fileSize - dataBufferSize; 
        if ( this->fidFile->tellg() < headerSize - 1 ) {

            //  find first white space position before "key" string: 
            position = iss->str().find_first_of(' ');
            if (position != string::npos) {
                tmp.assign( iss->str().substr(position) );
                dataType.assign( iss->str().substr(0, position) ) ; 
            } 
    
            //  If necessary, remove pointer indicator: 
            position = tmp.find_first_of('*');
            if (position != string::npos) {
                tmp.assign( tmp.substr(position + 1) );
            } 
    
            //  Extract key and value strings:
            position = tmp.find_first_of('=');
            if (position != string::npos) {
                keyString.assign( tmp.substr(0, position - 1) );
                keyString = StripWhite(keyString); 
                // Check for key match if doing a limited search: 
                int parseValue = 1;
                if (keySet != NULL) { 
                    if (keySet->GetNumberOfValues() > 0) { 
                        parseValue = 0; 
                        for (int i = 0; i < keySet->GetNumberOfValues(); i++) { 
                            if ( keySet->GetValue(i) == keyString ) {
                                parseValue = 1; 
                            } 
                        } 
                    } 
                }
                if ( !parseValue )  {
                    return; 
                }   

                valueString.assign( tmp.substr(position + 2) );
                // Remove terminating ; 
                position = valueString.find_first_of(';');
                valueString.assign( valueString.substr(0, position) );

                // Remove string quotes
                this->RemoveStringQuotes( &valueString );
                while ( ( position = valueString.find('"') ) != string::npos) {
                    valueString.erase( position, 1 );
                }
    
                //  Parse elements into vector: remove matrix brackets 
                //  and assign elements to vector: 
                position = valueString.find_first_of('{');
                if (position != string::npos) {

                    valueString.assign( valueString.substr(position + 1) );
                    position = valueString.find_first_of('}');

                    if (position != string::npos) {
                        valueString.assign( valueString.substr(0, position) );
                    } 
                } 

                this->ParseAndSetStringElements(keyString, valueString);
            } 

        } else { 
            this->fidFile->seekg(0, ios::end);     
        }
    } catch (const exception& e) {
        cout <<  "ERROR reading line: " << e.what() << endl;
    }

    delete iss; 
}


/*!
 *  Attempts to determine the pixel data buffer size from the currently 
 *  available header information.  If the size can be determined return that, 
 *  otherwise return 0. 
 */
int svkVarianFidReader::GetDataBufferSize()
{
    int bufferSize = 0; 
    map<string, string>::iterator it;

    if (fidMap.find("bits") != fidMap.end() && 
        fidMap.find("rank") != fidMap.end() && 
        fidMap.find("matrix[]") != fidMap.end() ) {

        int numDims = this->GetHeaderValueAsInt("rank");

        int numBitsPerByte = 8;
        int pixelWordSize = this->GetHeaderValueAsInt("bits")/numBitsPerByte;

        bufferSize = pixelWordSize; 
        for (int i = 0; i < numDims; i++) { 
            bufferSize *= this->GetHeaderValueAsInt("matrix[]", i);
        }
    }

    return bufferSize;
}


/*!
 *  Push key value pairs into the map's value vector: 
 *  mapFor values that are comma separated lists, put each element into the value 
 *  vector. 
 */
void svkVarianFidReader::ParseAndSetStringElements(string key, string valueArrayString) 
{
    size_t pos;
    istringstream* iss = new istringstream();
    string tmpString;     

    while ( (pos = valueArrayString.find_first_of(',')) != string::npos) {  

        iss->str( valueArrayString.substr(0, pos) );
        *iss >> tmpString;
        fidMap[key].push_back(tmpString); 
        iss->clear();

        valueArrayString.assign( valueArrayString.substr(pos + 1) ); 
    }
    iss->str( valueArrayString );
    *iss >> tmpString;
    fidMap[key].push_back(tmpString); 
    delete iss; 
}


/*!
 *
 */
int svkVarianFidReader::GetHeaderValueAsInt(string keyString, int valueIndex, int procparRow) 
{
    
    istringstream* iss = new istringstream();
    int value;

    iss->str( (procparMap[keyString])[procparRow][valueIndex]);
    *iss >> value;
    return value; 
}


/*!
 *
 */
float svkVarianFidReader::GetHeaderValueAsFloat(string keyString, int valueIndex, int procparRow) 
{
    
    istringstream* iss = new istringstream();
    float value;

    iss->str( (procparMap[keyString])[procparRow][valueIndex]);
    *iss >> value;
    return value; 
}


/*!
 *
 */
string svkVarianFidReader::GetHeaderValueAsString(string keyString, int valueIndex, int procparRow) 
{
    return (procparMap[keyString])[procparRow][valueIndex];
}


/*!
 *  If this is a 2D header, add explicit 3rd dimension with numSlices , 
 *  and modify slice roi and span 3rd dimension too: 
 */
void svkVarianFidReader::AddDimensionTo2DData() 
{

    ostringstream numSlicesOss;
    numSlicesOss << this->GetFileNames()->GetNumberOfValues();
    fidMap["matrix[]"].push_back( numSlicesOss.str() );     

    //  Get Min and Max location value in 3rd dimension: 
    float sliceMin = this->GetHeaderValueAsFloat("location[]", 2) ; 
    float sliceMax = this->GetHeaderValueAsFloat("location[]", 2) ; 
       
    float val;  
    int numSlices = this->GetHeaderValueAsInt("matrix[]", 2);
    for (int i = 0; i < numSlices; i++) {
        val = this->GetHeaderValueAsFloat("location[]", (i * 3) + 2 ) ;
        if (val > sliceMax ) {
            sliceMax = val; 
        }    
        if (val < sliceMin ) {
            sliceMin = val; 
        }    
    }
    float sliceThickness = (sliceMax - sliceMin)/(numSlices - 1); 
    float sliceFOV = numSlices * sliceThickness; 
    ostringstream sliceFOVoss;
    sliceFOVoss << sliceFOV; 
    fidMap["roi[]"][2] = sliceFOVoss.str(); 
    fidMap["span[]"].push_back( sliceFOVoss.str() );     
}


/*!
 *  Prints the key value pairs parsed from the header. 
 */
void svkVarianFidReader::PrintKeyValuePairs()
{

    //  Print out key value pairs parsed from header:
    map< string, vector<string> >::iterator mapIter;
    for ( mapIter = fidMap.begin(); mapIter != fidMap.end(); ++mapIter ) {
     
        cout << this->GetClassName() << " " << mapIter->first << " = ";

        vector<string>::iterator it;
        for ( it = fidMap[mapIter->first].begin() ; it < fidMap[mapIter->first].end(); it++ ) {
            cout << " " << *it ;
        }
        cout << endl;
    }
}


/*!
 *  Sets the keys to search for in the file header.  The first file all values are parsed, but
 *  only certain key values are parsed for subsequent files. 
 */
void svkVarianFidReader::SetKeysToSearch(vtkStringArray* fltArray, int fileIndex)
{
    if (fileIndex > 0) {  
        fltArray->InsertNextValue("location[]"); 
        fltArray->InsertNextValue("slice_no"); 
        fltArray->InsertNextValue("display_order"); 
    }
}


/*!
 *
 */
int svkVarianFidReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}

