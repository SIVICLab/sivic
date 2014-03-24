/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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


#include <svkIdfVolumeReader.h>
#include <svkDcmMriVolumeReader.h>
#include <vtkDebugLeaks.h>
#include <vtkByteSwap.h>

#include <sys/stat.h>


using namespace svk;


vtkCxxRevisionMacro(svkIdfVolumeReader, "$Rev$");
vtkStandardNewMacro(svkIdfVolumeReader);


/*!
 *  
 */
svkIdfVolumeReader::svkIdfVolumeReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkIdfVolumeReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->pixelData = NULL;
    this->volumeHdr = NULL;

    this->numSlices = 1; 
    this->numVolumes = 1; 
    this->onlyReadHeader = false;

    //  If there are multiple volumes, by default treat as separate
    //  channels of data. 
    this->multiVolumeType = svkIdfVolumeReader::MULTI_CHANNEL_DATA; 

    // IDF files are always big-endian.
    this->SetDataByteOrderToBigEndian();

}


/*!
 *
 */
svkIdfVolumeReader::~svkIdfVolumeReader()
{

    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if ( this->volumeHdr != NULL )  {
        delete volumeHdr; 
        this->volumeHdr = NULL; 
    }

}



/*!
 *  Check to see if the extension indicates a UCSF IDF file.  If so, try 
 *  to open the file for reading.  If that works, then return a success code. 
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkIdfVolumeReader::CanReadFile(const char* fname)
{

    vtkstd::string fileToCheck(fname);

    if( fileToCheck.size() > 4 ) {

        // Also see "vtkImageReader2::GetFileExtensions method" 
        if ( 
            fileToCheck.substr( fileToCheck.size() - 4 ) == ".byt"  ||
            fileToCheck.substr( fileToCheck.size() - 5 ) == ".int2" || 
            fileToCheck.substr( fileToCheck.size() - 5 ) == ".real" || 
            fileToCheck.substr( fileToCheck.size() - 4 ) == ".idf" 
        )  {
            FILE *fp = fopen(fname, "rb");
            if (fp) {
                fclose(fp);
                vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's a UCSF Volume File: " << fileToCheck);
                return 1;
            }
        } else {
            vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's NOT a UCSF Volume File: " << fileToCheck);
            return 0;
        }
    } else {
        vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): s NOT a valid file: " << fileToCheck);
        return 0;
    }
}


/*!
 *  Interpret the volumes as time points or channels.       
 */
void svkIdfVolumeReader::SetMultiVolumeType(svkIdfVolumeReader::MultiVolumeType volumeType)   
{
    this->multiVolumeType = volumeType; 
}


/*!
 *  If only the idf header is to be read.  Sometimes this acts as a template for processing, but 
 *  there isn't an associated data file.  
 */
void svkIdfVolumeReader::OnlyReadHeader(bool onlyReadHeader)
{
    this->onlyReadHeader = onlyReadHeader;
    vtkWarningWithObjectMacro(this, "onlyReadHeader: " << this->onlyReadHeader);   
}


/*!
 *  Reads the IDf data file (.byt, .int2, .real)
 */
void svkIdfVolumeReader::ReadVolumeFile()
{

    vtkDebugMacro( << this->GetClassName() << "::ReadVolumeFile()" );

    svkImageData* data = this->GetOutput(); 

    for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {

        vtkstd::string volFileName = this->GetFileRoot( this->GetFileNames()->GetValue( fileIndex ) );
        int dataUnitSize; 
        vtkDataArray* array;
        if ( this->GetFileType() == svkDcmHeader::UNSIGNED_INT_1 ) {
            array = vtkUnsignedCharArray::New();
            volFileName.append( ".byt" );
            dataUnitSize = 1;
        } else if ( this->GetFileType() == svkDcmHeader::UNSIGNED_INT_2 ) {
            array = vtkUnsignedShortArray::New();
            volFileName.append( ".int2" );
            dataUnitSize = 2;
        } else if ( this->GetFileType() == svkDcmHeader::SIGNED_FLOAT_4 ) {
            array =  vtkFloatArray::New();
            volFileName.append( ".real" );
            dataUnitSize = 4;
        }

        /*
        *   Flatten the data volume into one dimension
        */
        int numBytesInVol = this->GetNumPixelsInVol() * dataUnitSize; 
        this->pixelData = (void* ) malloc( numBytesInVol ); 

        ifstream* volumeDataIn = new ifstream();
        volumeDataIn->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        if( svkUtils::IsFileCompressed( volFileName )) {
        	svkUtils::UncompressFile( volFileName );
        }
        volumeDataIn->open( volFileName.c_str(), ios::binary);
        volumeDataIn->read( (char *)(this->pixelData), numBytesInVol );

        if ( this->GetSwapBytes() ) {
            if ( this->GetFileType() == svkDcmHeader::UNSIGNED_INT_2 ) {
                vtkByteSwap::SwapVoidRange(pixelData, this->GetNumPixelsInVol(), sizeof(short));
            } else if ( this->GetFileType() == svkDcmHeader::SIGNED_FLOAT_4 ) {
                vtkByteSwap::SwapVoidRange(pixelData, this->GetNumPixelsInVol(), sizeof(float));
            }
        }

        array->SetVoidArray( (void*)(this->pixelData), GetNumPixelsInVol(), 0);

        if ( this->GetFileType() == svkDcmHeader::UNSIGNED_INT_1 ) {
            this->Superclass::Superclass::GetOutput()->SetScalarType(VTK_UNSIGNED_CHAR);
        } else if ( this->GetFileType() == svkDcmHeader::UNSIGNED_INT_2 ) {
            this->Superclass::Superclass::GetOutput()->SetScalarType(VTK_UNSIGNED_SHORT);
        } else if ( this->GetFileType() == svkDcmHeader::SIGNED_FLOAT_4 ) {
            this->Superclass::Superclass::GetOutput()->SetScalarType(VTK_FLOAT);
        }

        ostringstream number;
        number << fileIndex ; 
        vtkstd::string arrayNameString("pixels"); 
        arrayNameString.append(number.str());

        array->SetName( arrayNameString.c_str() ); 

        if (fileIndex == 0 ) {
            data->GetPointData()->SetScalars(array);
        } else {
            data->GetPointData()->AddArray(array);
        }

        // We can now get rid of of our local reference to the array
        array->Delete();

        volumeDataIn->close();
        delete volumeDataIn;
    }
}


/*!
 *  Utility method returns the total number of Pixels in 3D. 
 */
int svkIdfVolumeReader::GetNumPixelsInVol()
{
    return (
        ( (this->GetDataExtent())[1] + 1 ) * 
        ( (this->GetDataExtent())[3] + 1 ) * 
        ( (this->GetDataExtent())[5] + 1 )  
    );
}


/*!
 *  Not sure if this is always extent 5 if the data is coronal and sagital for example
 */
int svkIdfVolumeReader::GetNumSlices()
{
    vtkWarningWithObjectMacro(this, "GetNumSlices: May not be correct for non axial data.");   
    return (this->GetDataExtent())[5] + 1;
}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called after ExecuteInformation()
 */
void svkIdfVolumeReader::ExecuteData(vtkDataObject* output)
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
            vtkErrorMacro("Unable to open file " << vtkstd::string(this->GetFileNames()->GetValue(0)) );
            return;
        }

        this->ReadVolumeFile();

        this->GetOutput()->SetDataRange( data->GetScalarRange(), svkImageData::REAL );
        double imaginaryRange[2] = {0,0}; 

        // Imaginary values are zeroes-- since images only have real components
        this->GetOutput()->SetDataRange( imaginaryRange, svkImageData::IMAGINARY );

        // Magnitudes are the same as the reals since the imaginaries are zero
        this->GetOutput()->SetDataRange( data->GetScalarRange(), svkImageData::MAGNITUDE );
    }

    /* 
     * We need to make a shallow copy of the output, otherwise we would have it
     * registered twice to the same reader which would cause the reader to never delete.
     */
    double dcos[3][3];
    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
    this->GetOutput()->SetDcos(dcos); 

    //  SetNumberOfIncrements is supposed to call this, but only works if the data has already
    //  been allocated. but that requires the number of components to be specified. 
    this->GetOutput()->GetIncrements(); 
    this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector();

}


/*!
 *  Side effect of Update() method.  Used to initialize the svkDcmHeader member of 
 *  the target svkImageData object and uses the header to set up the Output Informatin.
 *  Called before ExecuteData()
 */
void svkIdfVolumeReader::ExecuteInformation()
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
void svkIdfVolumeReader::InitDcmHeader()
{

    vtkDebugMacro( << this->GetClassName() << "::InitDcmHeader()" );

    this->iod = svkEnhancedMRIIOD::New();
    this->iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    this->iod->InitDcmHeader();

    this->ParseIdf(); 
    this->PrintKeyValuePairs(); 

    this->InitPatientModule();
    this->InitGeneralStudyModule();
    this->InitGeneralSeriesModule();
    this->InitGeneralEquipmentModule();
    this->InitImagePixelModule();
    this->InitMultiFrameFunctionalGroupsModule();
    this->InitAcquisitionContextModule();

    if (this->GetDebug()) {
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }

    this->iod->Delete();
}


/*!
 *  Returns the file root without extension
 */
svkDcmHeader::DcmPixelDataFormat svkIdfVolumeReader::GetFileType()
{
    istringstream* iss = new istringstream();
    iss->str(idfMap["fileType"]);  

    int fileType; 
    *iss >> fileType; 
    delete iss; 

    if ( fileType == 2 ) { 
        return svkDcmHeader::UNSIGNED_INT_1;
    } else if ( fileType == 3 ) { 
        return svkDcmHeader::UNSIGNED_INT_2;
    } else if ( fileType == 7 ) { 
        return svkDcmHeader::SIGNED_FLOAT_4;
    }

}


/*!
 *  If the IDF studyID field contains an accession number/t-number than
 *  we don't know the PatientID
 */
void svkIdfVolumeReader::InitPatientModule()
{

    vtkstd::string patientID;
    if ( this->IsIdfStudyIdAccessionNumber() ) {
        patientID = "";  
    } else {
        patientID = idfMap[ "studyId" ];  
    }

    this->GetOutput()->GetDcmHeader()->InitPatientModule(
        this->GetOutput()->GetDcmHeader()->GetDcmPatientName( idfMap["patientName"] ),
        patientID, 
        "",
        "" 
    );

}


/*!
 *
 */
void svkIdfVolumeReader::InitGeneralStudyModule()
{
    vtkstd::string accessionNumber;
    if ( this->IsIdfStudyIdAccessionNumber() ) {
        accessionNumber = idfMap[ "studyId" ];  
    } else {
        accessionNumber = ""; 
    }


    this->GetOutput()->GetDcmHeader()->InitGeneralStudyModule(
        idfMap[ "studyDate" ], 
        "",
        "",
        idfMap[ "studyNum" ], 
        accessionNumber, 
        "" 
    );

}


/*!
 *
 */
void svkIdfVolumeReader::InitGeneralSeriesModule()
{

    this->GetOutput()->GetDcmHeader()->InitGeneralSeriesModule(
        idfMap[ "seriesNum" ], 
        idfMap[ "seriesDescription" ], 
        GetDcmPatientPositionString( idfMap[ "patientPosition" ] )
    );

}


/*!
 *  DDF is historically the UCSF representation of a GE raw file so
 *  initialize to svkIdfVolumeReader::MFG_STRING.
 */
void svkIdfVolumeReader::InitGeneralEquipmentModule()
{
    // No way to know what type of scanner the images were acquired on. 
}


/*!
 *
 */
void svkIdfVolumeReader::InitImagePixelModule()
{        
    int value;
    istringstream* iss = new istringstream();

    iss->str(idfMap["numPixels_1"]);  
    *iss >> value; 
    this->GetOutput()->GetDcmHeader()->SetValue( "Rows", value ); 

    iss->clear();
    iss->str(idfMap["numPixels_0"]);  
    *iss >> value; 
    this->GetOutput()->GetDcmHeader()->SetValue( "Columns", value ); 

    delete iss; 
}


/*! 
 *  
 */
void svkIdfVolumeReader::InitMultiFrameFunctionalGroupsModule()
{

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ContentDate",
        idfMap[ "studyDate" ] 
    );

    int value;
    istringstream* iss = new istringstream();
    iss->str(idfMap["numPixels_2"]);  
    *iss >> this->numSlices; 

    this->numVolumes = this->GetFileNames()->GetNumberOfValues();
    this->numFrames = this->numSlices * this->numVolumes; 

    this->InitSharedFunctionalGroupMacros();
    this->InitPerFrameFunctionalGroupMacros();
    delete iss; 
}


/*! 
 *  
 */
void svkIdfVolumeReader::InitAcquisitionContextModule()
{
}


/*! 
 *  
 */
void svkIdfVolumeReader::InitSharedFunctionalGroupMacros()
{
    this->InitPixelMeasuresMacro();
    this->InitPlaneOrientationMacro();
    this->InitMRImagingModifierMacro(); 
    this->InitMRReceiveCoilMacro();
}


/*! 
 *  
 */
void svkIdfVolumeReader::InitPerFrameFunctionalGroupMacros()
{

    //  Get toplc float array from idfMap and use that to generate 
    //  frame locations:
    double toplc[3];
    int value;
    for (int i = 0; i < 3; i++) {
        ostringstream ossIndex;
        ossIndex << i;     
        vtkstd::string indexString(ossIndex.str());
        istringstream* iss = new istringstream();
        iss->setf(ios::fixed, ios::floatfield); 
        iss->precision(3);
        iss->str( idfMap[ vtkstd::string("toplc_" + indexString) ] );  
        *iss >> setiosflags(std::ios::fixed) >> setprecision(3) >>  toplc[i]; 
        delete iss; 
    }

    double dcos[3][3];
    double pixelSize[3];
    for (int i = 0; i < 3; i++) {

        ostringstream ossIndexI;
        ossIndexI << i;
        vtkstd::string indexStringI(ossIndexI.str());

        istringstream* issSize = new istringstream();
        issSize->str( idfMap[ vtkstd::string( "pixelSize_" + indexStringI ) ] );
        *issSize >> pixelSize[i];

        for (int j = 0; j < 3; j++) {
            ostringstream ossIndexJ;
            ossIndexJ << j;
            vtkstd::string indexStringJ(ossIndexJ.str());

            istringstream* iss = new istringstream();
            iss->str( idfMap[ vtkstd::string("dcos_" + indexStringI + "_" + indexStringJ)] ); 
            *iss >> dcos[i][j];
            delete iss; 
        }
        delete issSize; 
    }

    svkDcmHeader::DimensionVector dimensionVector = this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector(); 
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, this->numSlices-1);

    if ( this->multiVolumeType == svkIdfVolumeReader::MULTI_CHANNEL_DATA ) { 
        this->GetOutput()->GetDcmHeader()->AddDimensionIndex(
            &dimensionVector, svkDcmHeader::CHANNEL_INDEX, this->numVolumes-1);
    } else if ( this->multiVolumeType == svkIdfVolumeReader::TIME_SERIES_DATA ) { 
        this->GetOutput()->GetDcmHeader()->AddDimensionIndex(
            &dimensionVector, svkDcmHeader::TIME_INDEX, this->numVolumes-1);
    } 

    this->GetOutput()->GetDcmHeader()->InitPerFrameFunctionalGroupSequence(
                toplc,        
                pixelSize,  
                dcos,  
                &dimensionVector
    );
}


/*!
 *  Pixel Spacing:
 */
void svkIdfVolumeReader::InitPixelMeasuresMacro()
{
    float pixelSize[3];
    for (int i = 0; i < 3; i++) {
        ostringstream ossIndex;
        ossIndex << i;
        vtkstd::string indexString(ossIndex.str());

        istringstream* issSize = new istringstream();
        issSize->str( idfMap[ vtkstd::string( "pixelSize_" + indexString ) ] );
        *issSize >> pixelSize[i];
        delete issSize; 
    }

    this->GetOutput()->GetDcmHeader()->InitPixelMeasuresMacro(
        idfMap[ vtkstd::string( "pixelSize_0" ) ] + "\\" + idfMap[ vtkstd::string( "pixelSize_1" ) ],
        idfMap[ vtkstd::string( "sliceThickness" ) ]
    );
}


/*!
 *  
 */
void svkIdfVolumeReader::InitPlaneOrientationMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PlaneOrientationSequence"
    );

    vtkstd::string orientationString;
    for (int i = 0; i < 2; i++) {
        ostringstream ossIndexI;
        ossIndexI << i;
        vtkstd::string indexStringI(ossIndexI.str());
        for (int j = 0; j < 3; j++) {
            ostringstream ossIndexJ;
            ossIndexJ << j;
            vtkstd::string indexStringJ(ossIndexJ.str());
            orientationString.append( idfMap["dcos_" + indexStringI + "_" + indexStringJ ]) ;
            if (i + j < 3) {
                orientationString.append( "\\") ;
            }
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
    vtkstd::string indexStringI("2");
    for (int j = 0; j < 3; j++) {
       
        ostringstream ossIndexJ;
        ossIndexJ << j;
        vtkstd::string indexStringJ(ossIndexJ.str());

        istringstream* iss = new istringstream();
        vtkstd::string newone ("dcos_" + indexStringI + "_" + indexStringJ  );
        iss->str( idfMap["dcos_" + indexStringI + "_" + indexStringJ ] );
        *iss >> dcosSliceOrder[j];
        delete iss;
    }

    //  Use the scalar product to determine whether the data in the .cmplx
    //  file is ordered along the slice normal or antiparalle to it.
    vtkMath* math = vtkMath::New();
    if (math->Dot(normal, dcosSliceOrder) > 0 ) {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
    } else {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_NEG_NORMAL;
    }
    this->GetOutput()->GetDcmHeader()->SetSliceOrder( this->dataSliceOrder );
    math->Delete();

}


/*!
 *
 */
void svkIdfVolumeReader::InitMRTransmitCoilMacro()
{
    this->GetOutput()->GetDcmHeader()->InitMRTransmitCoilMacro("GE", "UNKNOWN", "BODY");
}


/*!
 *
 */
void svkIdfVolumeReader::InitMRImagingModifierMacro()
{

    float transmitFreq = -1;
    float pixelBandwidth = -1; 

    this->GetOutput()->GetDcmHeader()->InitMRImagingModifierMacro( transmitFreq, pixelBandwidth ); 
}


/*!
 *  Receive Coil:
 */
void svkIdfVolumeReader::InitMRReceiveCoilMacro()
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
        idfMap[ vtkstd::string( "coilName" ) ],
        "SharedFunctionalGroupsSequence",
        0
    );

}


/*! 
 *  Use the IDF patient position string to set the DCM_PatientPosition data element.
 */
vtkstd::string svkIdfVolumeReader::GetDcmPatientPositionString(vtkstd::string patientPosition)
{
    size_t delim = patientPosition.find_first_of(',');
    vtkstd::string headFeetFirst( patientPosition.substr(0, delim) );

    for(int i = 0; i < headFeetFirst.size(); i++){
        headFeetFirst[i] = tolower( headFeetFirst[i] );
    }

    vtkstd::string dcmPatientPosition;
    if( headFeetFirst.find("head first") != vtkstd::string::npos ) {
        dcmPatientPosition.assign("HF");
    } else if( headFeetFirst.find("feet first") != vtkstd::string::npos ) {
        dcmPatientPosition.assign("FF");
    } else {
        dcmPatientPosition.assign("UNKNOWN");
    }

    //  skip ", ":
    vtkstd::string spd( patientPosition.substr(delim + 2) );
    for(int i = 0; i < spd.size(); i++){
        spd[i] = tolower( spd[i] );
    }

    if( spd.find("supine") != vtkstd::string::npos ) {
        dcmPatientPosition += "S";
    } else if( spd.find("prone") != vtkstd::string::npos ) {
        dcmPatientPosition += "P";
    } else if( spd.find("decubitus left") != vtkstd::string::npos ) {
        dcmPatientPosition += "DL";
    } else if( spd.find("decubitus right") != vtkstd::string::npos ) {
        dcmPatientPosition += "DR";
    } else {
        dcmPatientPosition += "UNKNOWN";
    }

    return dcmPatientPosition; 
}


/*! 
 *  Parses the IDF comment field for:
 *      PatientName
 *      seriesDescription
 *      studyDate
 */
void svkIdfVolumeReader::ParseIdfComment(vtkstd::string comment, vtkstd::string* PatientName, 
    vtkstd::string* seriesDescription, vtkstd::string* studyDate)
{

    PatientName->assign(""); 
    studyDate->assign(""); 
    seriesDescription->assign(""); 

    size_t delim;
    if ( (delim = comment.find_first_of('-')) != vtkstd::string::npos)
    { 
        delim = delim -1;
        PatientName->assign( svkImageReader2::StripWhite( comment.substr(0, delim) ) );

        vtkstd::string commentSub;
        commentSub = comment.substr(delim + 3);
        delim = commentSub.find_last_of('-') - 1;
        seriesDescription->assign( commentSub.substr(0, delim) );
    
        studyDate->assign( commentSub.substr(delim + 3) );

        studyDate->assign( this->RemoveSlashesFromDate(studyDate) );
    }
}


/*
 *  Read IDF header fields into a string STL map for use during initialization 
 *  of DICOM header by Init*Module methods. 
 */
void svkIdfVolumeReader::ParseIdf()
{
   
    this->GlobFileNames();


    try { 

        //  Read in the IDF Header:
        this->volumeHdr = new ifstream();
        this->volumeHdr->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        int fileIndex = 0;
        vtkstd::string currentIdfFileName = this->GetFileRoot( this->GetFileNames()->GetValue( fileIndex ) ) + ".idf";

        this->volumeHdr->open( currentIdfFileName.c_str(), ifstream::in );

        if ( ! this->volumeHdr->is_open() ) {
            throw runtime_error( "Could not open volume file: " + currentIdfFileName );
        } 
        istringstream* iss = new istringstream();

        // IDF_VERSION 
        int idfVersion;
        this->ReadLine(this->volumeHdr, iss);
        iss->ignore(29);
        *iss>>idfVersion;

        // STUDY_ID
        idfMap["studyId"] = this->ReadLineValue( this->volumeHdr, iss, ':'); 

        // STUDY #
        idfMap["studyNum"] = this->ReadLineValue( this->volumeHdr, iss, ':');

        // SERIES #
        idfMap["seriesNum"] = this->ReadLineValue( this->volumeHdr, iss, ':');

        // POSITION
        idfMap["patientPosition"] = this->ReadLineValue( this->volumeHdr, iss, ':');

        // COILNAME
        idfMap["coilName"] = this->ReadLineValue( this->volumeHdr, iss, ':');

        // ORIENTATION
        int orientation;
        this->ReadLineIgnore(this->volumeHdr, iss, ':');
        *iss>>orientation;

        // ECHO/TIME/MET INDEX
        // This appears to be hardcoded in volume write function      
        idfMap["echoTimeMetIndex"] = this->ReadLineValue( this->volumeHdr, iss, ':');

        // ROOTNAME
        vtkstd::string rootname = this->ReadLineValue( this->volumeHdr, iss, ':');

        // COMMENT 
        vtkstd::string* patientName = new vtkstd::string(); 
        vtkstd::string* seriesDescription = new vtkstd::string(); 
        vtkstd::string* studyDate = new vtkstd::string(); 
        vtkstd::string  comment = this->ReadLineValue( this->volumeHdr, iss, ':');
        ParseIdfComment(comment, patientName, seriesDescription, studyDate);
        idfMap["patientName"] = *patientName; 
        idfMap["seriesDescription"] = *seriesDescription; 
        idfMap["studyDate"] = *studyDate; 

        // FILETYPE / ENTRY NUM / PIXEL
        idfMap["fileType"] = this->ReadLineValue( this->volumeHdr, iss, ':');

        // DIMENSIONS AND SPACING 
        const int numDimensions = 3;    
        vtkstd::string    dimensionString[numDimensions];
        float     fov[numDimensions];
        for (int i = 0; i < numDimensions; i++) {
            ostringstream ossIndex;
            ossIndex << i;     
            vtkstd::string indexString(ossIndex.str());
            dimensionString[i] = this->ReadLineSubstr(this->volumeHdr, iss, 10, 256);
            this->ReadLineIgnore(this->volumeHdr, iss, ':');
            *iss >> idfMap[ vtkstd::string( "numPixels_" + indexString ) ];
            iss->ignore(256, ':');
            *iss >> fov[i];
            iss->ignore(256, ':');
            iss->ignore(256, ':');
            *iss >> idfMap[ vtkstd::string( "pixelSize_" + indexString ) ];
        } 

        // Set DICOM Column and Row info:    
        //setDcmPixelSpacing(pixelSize, dcmImage);

        //  SLICE THICKNESS -> DCM_SliceThickness;
        idfMap["sliceThickness"] = this->ReadLineValue( this->volumeHdr, iss, ':');

        //  MIN + MAX 
        this->ReadLineIgnore(this->volumeHdr, iss, ':');
        *iss >> idfMap["minIntensity"];
        iss->ignore(256, ':');
        *iss >> idfMap["maxIntensity"];

/*
    // Set pixel intensity values here.  these aren't strictly mandatory 
    // attributes, so are not included in the MRImage class. 
        dcmImage->setValue( 
            DcmTag (DCM_SmallestImagePixelValue, EVR_US), 
            static_cast<int>(minIntensity)
        );
        dcmImage->setValue( 
            DcmTag (DCM_LargestImagePixelValue, EVR_US), 
            static_cast<int>(maxIntensity)
        );

        dcmImage->setValue(DCM_WindowCenter, static_cast<int>(maxIntensity)/2);
        dcmImage->setValue(DCM_WindowWidth, static_cast<int>(maxIntensity));
*/


        //  SCALE
        float scale;
        this->ReadLineIgnore(this->volumeHdr, iss, ':');
        *iss>>scale;

        //  FIRST, LAST, SKIP 
        //      firstSlice -> DCM_InstanceNumber of first DICOM image
        //      Set insance numbers of remaining DICOM images in volume by 
        //      incrementing this value by skip ???
        int firstSlice;
        int lastSlice;
        int sliceSkip;
        this->ReadLineIgnore(this->volumeHdr, iss, ':');
        *iss>>firstSlice;
        iss->ignore(256, ':');
        *iss>>lastSlice;
        iss->ignore(256, ':');
        *iss>>sliceSkip;

        //  LOCATION DATA IN LPS COORDINATES
        this->ReadLine(this->volumeHdr, iss);
        
        //  CENTER(LPS): 
        float center[numDimensions];
        this->ReadLine(this->volumeHdr, iss);
        for (int i = 0; i < numDimensions; i++) {
            ostringstream ossIndex;
            ossIndex << i;     
            vtkstd::string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> idfMap[vtkstd::string("center_" + indexString)];
        }

        //  TOPLC(LPS): 
        //      -> DCM_ImagePositionPatient for image0 (sorted?)
        //      Varies for each DICOM image in volume!
        this->ReadLine(this->volumeHdr, iss);
        for (int i = 0; i < numDimensions; i++) {
            ostringstream ossIndex;
            ossIndex << i;     
            vtkstd::string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> idfMap[vtkstd::string("toplc_" + indexString)]; 
        }

        //  DCOS: 
        for (int i = 0; i < numDimensions; i++) {
            ostringstream ossIndexI;
            ossIndexI << i;     
            vtkstd::string indexStringI(ossIndexI.str());
            this->ReadLine(this->volumeHdr, iss);
            for (int j = 0; j < numDimensions; j++) {
                ostringstream ossIndexJ;
                ossIndexJ << j;     
                vtkstd::string indexStringJ(ossIndexJ.str());
                iss->ignore(256, ' ');
                *iss >> idfMap[vtkstd::string("dcos_" + indexStringI + "_" + indexStringJ)]; 
            }
        }

        this->volumeHdr->close();

        // Delete allocated memory:
        delete iss;
        delete seriesDescription;
        delete studyDate;
        delete patientName;

    } catch (const exception& e) {
        cerr << "ERROR opening or reading volume file (" << this->GetFileName() << "): " << e.what() << endl;
    }

}


/*!
 *  Prints the key value pairs parsed from the header.
 */
void svkIdfVolumeReader::PrintKeyValuePairs()
{
    if (this->GetDebug()) {
        vtkstd::map< vtkstd::string, vtkstd::string >::iterator mapIter;
        for ( mapIter = idfMap.begin(); mapIter != idfMap.end(); ++mapIter ) {
            cout << this->GetClassName() << " " << mapIter->first << " = ";
            cout << idfMap[mapIter->first] << endl;
        }
    }
}


/*!
 *  Tries to determine whether an idf studyId field is a t-number 
 *  aka study id or a b# aka patient id based on a leading "t#_". 
 */
bool svkIdfVolumeReader::IsIdfStudyIdAccessionNumber()
{
    bool isAccession = false; 
    vtkstd::string idfStudyId = idfMap[ "studyId" ]; 

    size_t pos = idfStudyId.find( "t" ); 
    if ( pos == 0 ) {
        isAccession = true; 
    }

    return isAccession; 
}


/*!
 *
 */
int svkIdfVolumeReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData");
    return 1;
}


