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


#include <svkSiemensRdaReader.h>
#include <svkSpecUtils.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkByteSwap.h>

#include <limits>
#include <sys/stat.h>


using namespace svk;


//vtkCxxRevisionMacro(svkSiemensRdaReader, "$Rev$");
vtkStandardNewMacro(svkSiemensRdaReader);


/*!
 *
 */
svkSiemensRdaReader::svkSiemensRdaReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkSiemensRdaReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    // Set the byte ordering, as little-endian.
    this->SetDataByteOrderToLittleEndian();

    this->specData = NULL;
    this->dataArray = NULL; 
    this->rdaFile = NULL;
    this->fileSize = 0;
    this->numSlices = 1;
    this->numCoils = 1;
    this->numTimePts = 1;
    this->iod = NULL;
    this->endOfHeaderPos = 0;

}


/*!
 *
 */
svkSiemensRdaReader::~svkSiemensRdaReader()
{

    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if (this->dataArray != NULL) {
        this->dataArray->Delete();
        this->dataArray = NULL; 
    }

    if ( this->rdaFile != NULL )  {
        delete rdaFile; 
        this->rdaFile = NULL; 
    }
}


/*!
 *  Check to see if the extension indicates a Siemens rda file.  If so, try 
 *  to open the file for reading.  If that works, then return a success code. 
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkSiemensRdaReader::CanReadFile(const char* fname)
{

    string fileToCheck(fname);

    if( fileToCheck.size() > 4 ) {

        // Also see "vtkImageReader2::GetFileExtensions method" 
        if (  fileToCheck.substr( fileToCheck.size() - 4 ) == ".rda" ) {

            FILE *fp = fopen(fname, "rb");

            if (fp) {
                fclose(fp);
                // Adding some more smarts to this function.
                // Check for the existence of ">>> Begin of header <<<"
                // and ">>> End of header <<<" text values in the rda
                // file.  If they don't exist, then this is not an 
                // rda file!

                ifstream* tempRdaFile = new ifstream();
                tempRdaFile->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
                istringstream* iss = new istringstream();
                try {

                    tempRdaFile->open( fname, ifstream::in );
                    if ( ! tempRdaFile->is_open() ) {
                        throw runtime_error( "Could not open rda file: " + fileToCheck );
                    }

                    // First line should be ">>> Begin of header <<<".
                    this->ReadLine(tempRdaFile, iss);
                    if ( iss->str().find(">>> Begin of header <<<") 
                      == string::npos ) {
                        vtkDebugMacro(
                          << this->GetClassName() << "::CanReadFile(): It's NOT a Siemens RDA File: " << fileToCheck << 
                          ": Could not find >>> Begin of header <<< text"
                        );
                        tempRdaFile->close();
                        delete tempRdaFile;
                        delete iss;
                        return 0;
                    } 
                    // Read until ">>> End of header <<<" is found or EOF.
                    while (! tempRdaFile->eof() ) {
                        this->ReadLine(tempRdaFile, iss);
                        if ( iss->str().find(">>> End of header <<<") 
                        != string::npos ) {
                            break; 
                        }
                    }
                    
                    // Was ">>> End of header <<<" found?
                    if ( tempRdaFile->eof() ) {
                        vtkDebugMacro(
                          << this->GetClassName() << "::CanReadFile(): It's NOT a Siemens RDA File: " << fileToCheck << 
                          ": Could not find >>> End of header <<< text"
                        );
                        tempRdaFile->close();
                        delete tempRdaFile;
                        delete iss;
                        return 0;
                    } 

                    tempRdaFile->close();

                } catch (const exception& e) {
                    vtkDebugMacro(
                        << this->GetClassName() << "::CanReadFile(): It's NOT a Siemens RDA File: " << fileToCheck << 
                        ": " << e.what()
                    );
                    delete tempRdaFile;
                    delete iss;
                    return 0;
                }
                vtkDebugMacro(
                    << this->GetClassName() << "::CanReadFile(): It's a Siemens RDA File: " << fileToCheck
                );
                delete tempRdaFile;
                delete iss;
                return 1;
            }

        } else {

            vtkDebugMacro(
                << this->GetClassName() << "::CanReadFile(): It's NOT a Siemens RDA File: " << fileToCheck
            );

            return 0;

        }

    } else {

        vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): is NOT a valid file: " << fileToCheck);
        return 0;

    }

}


/*!
 *  Reads pixel data from all rda files. 
 *  For .rda series, each file contains 1/num_files_in_series worth of pixels. 
 */
void svkSiemensRdaReader::ReadRdaFiles(vtkImageData* data)
{

    vtkDebugMacro( << this->GetClassName() << "::ReadRdaFiles()" );

    int coilNum = 0;

    //  Read in data from 1 coil:
    int numComponents =  2;
    int numPts = this->GetHeaderValueAsInt( "VectorSize" );
    int numBytesInVol = this->GetNumPixelsInVol() * numPts * numComponents * sizeof(double) * this->numTimePts;
    this->specData      = new float[ numBytesInVol/sizeof(float) ];
    double* specDataDbl = new double[ numBytesInVol/sizeof(double) ];

    ifstream* rdaDataIn = new ifstream();
    rdaDataIn->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
    rdaDataIn->open( this->FileName, ios::binary );

    rdaDataIn->seekg(this->endOfHeaderPos, ios::beg); 
    rdaDataIn->read( (char*)(specDataDbl), numBytesInVol );

    if ( this->GetSwapBytes() ) {
        vtkByteSwap::SwapVoidRange((void *)specDataDbl, numBytesInVol/sizeof(double), sizeof(double));
    }


    this->MapDoubleValuesToFloat( specDataDbl, this->specData, numBytesInVol/sizeof(double));

    for (int timePt = 0; timePt < this->numTimePts ; timePt++) {
        for (int z = 0; z < (this->GetDataExtent())[5] ; z++) {
            for (int y = 0; y < (this->GetDataExtent())[3]; y++) {
                for (int x = 0; x < (this->GetDataExtent())[1]; x++) {
                    SetCellSpectrum(data, x, y, z, timePt, coilNum);
                }
            }
        }
    }

    rdaDataIn->close();
    delete rdaDataIn;
    delete [] specData;
    delete [] specDataDbl;
}


/*!
 *
 */
void svkSiemensRdaReader::SetCellSpectrum(vtkImageData* data, int x, int y, int z, int timePt, int coilNum)
{

    //  Set XY points to plot - 2 components per tuble for complex data sets:
    vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
    dataArray->SetNumberOfComponents(2);

    int numPts = this->GetHeaderValueAsInt( "VectorSize" );
    int numComponents = 2; 
    dataArray->SetNumberOfTuples(numPts);
    char arrayName[30];

    sprintf(arrayName, "%d %d %d %d %d", x, y, z, timePt, coilNum);
    dataArray->SetName(arrayName);

    //  preallocate float array for spectrum and use to iniitialize the vtkDataArray:
    //  don't do this for each call
    int numVoxels[3];
    numVoxels[0] = this->GetHeaderValueAsInt( "CSIMatrixSize[0]" );
    numVoxels[1] = this->GetHeaderValueAsInt( "CSIMatrixSize[1]" );
    numVoxels[2] = this->GetHeaderValueAsInt( "CSIMatrixSize[2]" );

    int offset = (numPts * numComponents) *  (
                     ( numVoxels[0] * numVoxels[1] * numVoxels[2] ) * timePt
                    +( numVoxels[0] * numVoxels[1] ) * z
                    +  numVoxels[0] * y
                    +  x
                 );

    for (int i = 0; i < numPts; i++) {
        dataArray->SetTuple(i, &(this->specData[offset + (i * 2)]));
    }

    //  Add the spectrum's dataArray to the CellData:
    //  vtkCellData is a subclass of vtkFieldData
    data->GetCellData()->AddArray(dataArray);

    dataArray->Delete();

    return;
}



/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called after ExecuteInformation()
 */
void svkSiemensRdaReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
{

    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );

    svkImageData* data = svkImageData::SafeDownCast( this->AllocateOutputData(output, outInfo) );

    if (!this->FileName) {
        vtkErrorMacro("A valid FileName must be specified.");
        return;
    }

    vtkDebugMacro( << this->GetClassName() << " FileName: " << this->FileName );
    struct stat fs;
    if ( stat(this->FileName, &fs) ) {
        vtkErrorMacro("Unable to open file " << string(this->FileName) );
        return;
    }
    this->ReadRdaFiles( data );

    double dcos[3][3];
    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
    this->GetOutput()->SetDcos(dcos);

    //  SetNumberOfIncrements is supposed to call this, but only works if the data has already
    //  been allocated. but that requires the number of components to be specified.
    this->GetOutput()->GetIncrements();

    if (this->GetDebug()) {
        cout << "SIEMENS RDA READER HEADER " << *data << endl;
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }

}


/*!
 *  Side effect of Update() method.  Used to initialize the svkDcmHeader member of 
 *  the target svkImageData object and uses the header to set up the Output Informatin.
 *  Called before ExecuteData()
 */
void svkSiemensRdaReader::ExecuteInformation()
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

}


/*!
 *  Initializes the svkDcmHeader adapter to a specific IOD type
 *  and initizlizes the svkDcmHeader member of the svkImageData
 *  object.
 */
void svkSiemensRdaReader::InitDcmHeader()
{
    vtkDebugMacro( << this->GetClassName() << "::InitDcmHeader()" );

    this->iod = svkMRSIOD::New();
    this->iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    this->iod->InitDcmHeader();

    //  Read the rda header into a map of values used to initialize the
    //  DICOM header. 
    this->ParseRda(); 
    this->PrintKeyValuePairs();


    //  Fill in data set specific values:
    this->InitPatientModule();
    this->InitGeneralStudyModule();
    this->InitGeneralSeriesModule();
    this->InitGeneralEquipmentModule();
    this->InitMultiFrameFunctionalGroupsModule();
//    this->InitMultiFrameDimensionModule();
//    this->InitAcquisitionContextModule();
    this->InitMRSpectroscopyModule();
    this->InitMRSpectroscopyPulseSequenceModule();
    this->InitMRSpectroscopyDataModule();

    if (this->GetDebug()) {
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }

    this->iod->Delete();
}



/*!
 *  Returns the file type enum 
 */
svkDcmHeader::DcmPixelDataFormat svkSiemensRdaReader::GetFileType()
{
    return svkDcmHeader::SIGNED_FLOAT_8;
}


/*!
 *
 */
void svkSiemensRdaReader::InitPatientModule()
{

    this->GetOutput()->GetDcmHeader()->InitPatientModule(
        this->GetHeaderValueAsString("PatientName"), 
        this->GetHeaderValueAsString("PatientID"),
        "", 
        ""
    );

}


/*!
 *
 */
void svkSiemensRdaReader::InitGeneralStudyModule()
{
    this->GetOutput()->GetDcmHeader()->InitGeneralStudyModule(
        this->GetHeaderValueAsString("StudyDate"), 
        "",
        "",
        this->GetHeaderValueAsString("PatientID"), 
        "", 
        ""
    );
}


/*!
 *
 */
void svkSiemensRdaReader::InitGeneralSeriesModule()
{
    this->GetOutput()->GetDcmHeader()->InitGeneralSeriesModule(
        this->GetHeaderValueAsString("SeriesNumber"), 
        this->GetHeaderValueAsString("SeriesDescription"), 
        this->GetHeaderValueAsString("PatientPosition") 
    );
}


/*!
 *  DDF is historically the UCSF representation of a GE raw file so
 *  initialize to svkSiemensRdaReader::MFG_STRING.
 */
void svkSiemensRdaReader::InitGeneralEquipmentModule()
{
    this->GetOutput()->GetDcmHeader()->SetValue(
        "Manufacturer",
        "Siemens" 
    );

}


/*! 
 *  
 */
void svkSiemensRdaReader::InitMultiFrameFunctionalGroupsModule()
{

   InitSharedFunctionalGroupMacros();

    this->GetOutput()->GetDcmHeader()->SetValue(
        "InstanceNumber",
        1
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ContentDate",
        this->GetHeaderValueAsString("StudyDate")
    );

    this->numSlices = this->GetHeaderValueAsInt("CSIMatrixSize[2]");  
    this->numCoils = 1; 
    this->numTimePts = 1; 

    InitPerFrameFunctionalGroupMacros();

}


/*! 
 *  
 */
void svkSiemensRdaReader::InitMultiFrameDimensionModule()
{
}


/*! 
 *  
 */
void svkSiemensRdaReader::InitAcquisitionContextModule()
{
}


/*! 
 *  
 */
void svkSiemensRdaReader::InitSharedFunctionalGroupMacros()
{

    this->InitPixelMeasuresMacro();
    this->InitPlaneOrientationMacro();
    this->InitMRTimingAndRelatedParametersMacro();
    this->InitMRSpectroscopyFOVGeometryMacro();
    this->InitMREchoMacro();
    this->InitMRModifierMacro();
    this->InitMRReceiveCoilMacro();
    this->InitMRTransmitCoilMacro();
    this->InitMRAveragesMacro();
    this->InitMRSpatialSaturationMacro();
    //this->InitMRSpatialVelocityEncodingMacro();

}


/*! 
 *  
 */
void svkSiemensRdaReader::InitPerFrameFunctionalGroupMacros()
{

    //  Get toplc float array from rdaMap and use that to generate
    //  frame locations:
    double toplc[3];
    int value;
    for (int i = 0; i < 3; i++) {
        ostringstream ossIndex;
        ossIndex << i;
        string indexString(ossIndex.str());
        toplc[i] = this->GetHeaderValueAsFloat( "PositionVector[" + indexString + "]" );
    }

    double pixelSize[3];
    pixelSize[0] = this->GetHeaderValueAsFloat( "PixelSpacingCol" );
    pixelSize[1] = this->GetHeaderValueAsFloat( "PixelSpacingRow" );
    pixelSize[2] = this->GetHeaderValueAsFloat( "PixelSpacing3D" );

    double dcos[3][3];
    this->GetDcosFromRda(dcos); 

    svkDcmHeader::DimensionVector dimensionVector = this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, numSlices-1);
    this->GetOutput()->GetDcmHeader()->InitPerFrameFunctionalGroupSequence(
        toplc, pixelSize, dcos, &dimensionVector
    );
}


/*!
 *  Extract dcos from rda header 
 */
void svkSiemensRdaReader::GetDcosFromRda(double dcos[3][3])
{

    // direction of increasing column number (X dir):
    double colVector[3];  
    colVector[0] = this->GetHeaderValueAsFloat( "RowVector[0]" );
    colVector[1] = this->GetHeaderValueAsFloat( "RowVector[1]" );
    colVector[2] = this->GetHeaderValueAsFloat( "RowVector[2]" );

    // direction of increasing row number (Y dir):
    double rowVector[3];  
    rowVector[0] = this->GetHeaderValueAsFloat( "ColumnVector[0]" );
    rowVector[1] = this->GetHeaderValueAsFloat( "ColumnVector[1]" );
    rowVector[2] = this->GetHeaderValueAsFloat( "ColumnVector[2]" );

    vtkMath* math = vtkMath::New();
    double normal[3];  
    math->Cross(colVector, rowVector, normal);
    math->Delete();

    dcos[0][0] = colVector[0];  
    dcos[0][1] = colVector[1]; 
    dcos[0][2] = colVector[2]; 
    dcos[1][0] = rowVector[0]; 
    dcos[1][1] = rowVector[1]; 
    dcos[1][2] = rowVector[2]; 
    dcos[2][0] = normal[0]; 
    dcos[2][1] = normal[1]; 
    dcos[2][2] = normal[2]; 
}


/*!
 *  Extract dcos from rda header 
 */
void svkSiemensRdaReader::GetDcosFromRda(float dcos[3][3])
{
    double dcosDbl[3][3]; 
    this->GetDcosFromRda(dcosDbl); 

    dcos[0][0] = dcosDbl[0][0];  
    dcos[0][1] = dcosDbl[0][1]; 
    dcos[0][2] = dcosDbl[0][2]; 
    dcos[1][0] = dcosDbl[1][0]; 
    dcos[1][1] = dcosDbl[1][1]; 
    dcos[1][2] = dcosDbl[1][2]; 
    dcos[2][0] = dcosDbl[2][0]; 
    dcos[2][1] = dcosDbl[2][1]; 
    dcos[2][2] = dcosDbl[2][2]; 
}


/*!
 *  Pixel Spacing:
 */
void svkSiemensRdaReader::InitPixelMeasuresMacro()
{
    this->GetOutput()->GetDcmHeader()->InitPixelMeasuresMacro(
        this->GetHeaderValueAsString( "PixelSpacingCol") + '\\' + this->GetHeaderValueAsString( "PixelSpacingRow"), 
        this->GetHeaderValueAsString( "PixelSpacing3D") 
    );
}


/*!
 *  
 */
void svkSiemensRdaReader::InitPlaneOrientationMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PlaneOrientationSequence"
    );

    string orientationString;
 
    //  varian appears to be LAI coords (rather than LPS), 
    //  so flip the 2nd and 3rd idndex.  Is there an "entry" indicator?
    //  HF vs FF should flip both RL and AP.  Supine/Prone should flip 
    //  RL and AP.  
    //  FF + Prone should flip AP + SI relative to HF + Supine 
    double dcos[3][3];
    this->GetDcosFromRda(dcos); 

    double inPlane[6]; 
    inPlane[0] = dcos[0][0]; 
    inPlane[1] = dcos[0][1]; 
    inPlane[2] = dcos[0][2]; 
    inPlane[3] = dcos[1][0]; 
    inPlane[4] = dcos[1][1]; 
    inPlane[5] = dcos[1][2]; 

    for (int i = 0; i < 6; i++) {
        ostringstream dcosOSS;
        dcosOSS << inPlane[i];
        orientationString.append( dcosOSS.str() );
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
    dcosSliceOrder[0] = dcos[2][0]; 
    dcosSliceOrder[1] = dcos[2][1]; 
    dcosSliceOrder[2] = dcos[2][2]; 

    //  Use the scalar product to determine whether the data in the .rda
    //  file is ordered along the slice normal or antiparalle to it.
    vtkMath* math = vtkMath::New();
    if (math->Dot(normal, dcosSliceOrder) > 0 ) {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
    } else {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_NEG_NORMAL;
    }
    math->Delete();

}


/*!
 *
 */
void svkSiemensRdaReader::InitMREchoMacro()
{
    this->GetOutput()->GetDcmHeader()->InitMREchoMacro( this->GetHeaderValueAsFloat("TE") ); 
}


/*!
 *
 */
void svkSiemensRdaReader::InitMRAveragesMacro()
{
    int numAverages = 1; 
    this->GetOutput()->GetDcmHeader()->InitMRAveragesMacro(numAverages);
}


/*!
 *
 */
void svkSiemensRdaReader::InitMRTimingAndRelatedParametersMacro()
{
    this->GetOutput()->GetDcmHeader()->InitMRTimingAndRelatedParametersMacro(
        this->GetHeaderValueAsFloat("TR"),
        this->GetHeaderValueAsFloat("FlipAngle")
    ); 
}


/*!
 *
 */
void svkSiemensRdaReader::InitMRModifierMacro()
{
    float inversionTime = this->GetHeaderValueAsFloat("TI");
    this->GetOutput()->GetDcmHeader()->InitMREchoMacro( inversionTime );
}


/*!
 *
 */
void svkSiemensRdaReader::InitMRTransmitCoilMacro()
{
    this->GetOutput()->GetDcmHeader()->InitMRTransmitCoilMacro(
        "Siemens", 
        "UNKNOWN", 
        this->GetHeaderValueAsString("TransmitCoil")
    );
}


/*!
 *
 */
void svkSiemensRdaReader::InitVolumeLocalizationSeq()
{
    float size[3];     
    //size[0] = this->GetHeaderValueAsFloat( "VOIPhaseFOV" );
    //size[1] = this->GetHeaderValueAsFloat( "VOIReadoutFOV" );
    size[1] = this->GetHeaderValueAsFloat( "VOIPhaseFOV" );
    size[0] = this->GetHeaderValueAsFloat( "VOIReadoutFOV" );
    size[2] = this->GetHeaderValueAsFloat( "VOIThickness" );

    float center[3];     
    center[0] = this->GetHeaderValueAsFloat( "VOIPositionSag" );
    center[1] = this->GetHeaderValueAsFloat( "VOIPositionCor" );
    center[2] = this->GetHeaderValueAsFloat( "VOIPositionTra" );

    //  See if the VOI normal is parallel to the acquisition dcos, and if so 
    //  use that dcos.  Oterhwise: 
    //      use Euler angles to obtain dcos
    //      1.  theta: angle between normal and the magnet z vector  (0,0,1)
    float normal[3];     
    normal[0] = this->GetHeaderValueAsFloat( "VOINormalSag" );
    normal[1] = this->GetHeaderValueAsFloat( "VOINormalCor" );
    normal[2] = this->GetHeaderValueAsFloat( "VOINormalTra" );

    float dcos[3][3];
    this->GetDcosFromRda(dcos); 

    float dcosNormal[3];     
    dcosNormal[0] = dcos[2][0];  
    dcosNormal[1] = dcos[2][1];  
    dcosNormal[2] = dcos[2][2];  
    cout <<  vtkMath::Dot(normal, dcosNormal) << endl; 
    if ( vtkMath::Dot(normal, dcosNormal) != 1 ) {
        cout << "Acquisition and VOI dcos differ. NEED TO CALC VOI ORIENTATION " << endl;
        //double theta = arcos( normal[2]/vtkMath::Norm(normal) ); 
        //  to do...
    }

    this->GetOutput()->GetDcmHeader()->InitVolumeLocalizationSeq(
        size, 
        center, 
        dcos
    ); 
}


/*!
 *  Receive Coil:
 */
void svkSiemensRdaReader::InitMRReceiveCoilMacro()
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
        "Siemens Coil",
        "SharedFunctionalGroupsSequence",
        0
    );

}


/*!
 *  Sat Bands not defined in rda
 */
void svkSiemensRdaReader::InitMRSpatialSaturationMacro()
{
    int numSatBands = 0; 

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRSpatialSaturationSequence"
    );
}


/*!
 *  
 */
void svkSiemensRdaReader::InitMRSpectroscopyFOVGeometryMacro()
{
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRSpectroscopyFOVGeometrySequence"
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionDataColumns",
        this->GetHeaderValueAsInt("VectorSize"), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionPhaseColumns",
        this->GetHeaderValueAsInt( "CSIMatrixSizeOfScan[0]" ),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionPhaseRows",
        this->GetHeaderValueAsInt( "CSIMatrixSizeOfScan[1]"),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionOutOfPlanePhaseSteps",
        this->GetHeaderValueAsInt( "CSIMatrixSizeOfScan[2]"),
        "SharedFunctionalGroupsSequence",
        0
    );

    double toplc[3];
    int value;
    for (int i = 0; i < 3; i++) {
        ostringstream ossIndex;
        ossIndex << i;
        string indexString(ossIndex.str());
        toplc[i] = this->GetHeaderValueAsFloat( "PositionVector[" + indexString + "]" );
    }

    string acqTlc = this->GetHeaderValueAsString( "PositionVector[0]" ) 
                    + '\\' + this->GetHeaderValueAsString( "PositionVector[1]" ) 
                    + '\\' + this->GetHeaderValueAsString( "PositionVector[2]" ) ;
    
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionTLC",
        acqTlc, 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionPixelSpacing",
        this->GetHeaderValueAsString( "PixelSpacingCol") + '\\' + this->GetHeaderValueAsString( "PixelSpacingRow"), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionSliceThickness",
        this->GetHeaderValueAsString( "PixelSpacing3D"), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionOrientation",
        "0\\0\\0\\0\\0\\0\\0\\0\\0", 
        "SharedFunctionalGroupsSequence",
        0
    );

    // ==================================================
    //  Reordered Params
    // ==================================================
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedPhaseColumns",
        this->GetHeaderValueAsInt( "NumberOfColumns"),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedPhaseRows",
        this->GetHeaderValueAsInt( "NumberOfRows"),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedOutOfPlanePhaseSteps",
        this->GetHeaderValueAsInt( "NumberOf3DParts"),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedTLC",
        acqTlc, 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedPixelSpacing",
        this->GetHeaderValueAsString( "PixelSpacingCol") + '\\' + this->GetHeaderValueAsString( "PixelSpacingRow"), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedSliceThickness",
        this->GetHeaderValueAsString( "PixelSpacing3D"), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedOrientation",
        "0\\0\\0\\0\\0\\0\\0\\0\\0", 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "PercentSampling",
        1,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "PercentPhaseFieldOfView",
        1,
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *
 */
void svkSiemensRdaReader::InitMRSpectroscopyModule()
{

    /*  =======================================
     *  MR Image and Spectroscopy Instance Macro
     *  ======================================= */

    this->GetOutput()->GetDcmHeader()->SetValue(
        "AcquisitionDateTime",
        this->GetHeaderValueAsString( "StudyDate") + "000000" 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "AcquisitionDuration",
        0
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ResonantNucleus",
        this->GetHeaderValueAsString( "Nucleus") 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "KSpaceFiltering",
        "NONE"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ApplicableSafetyStandardAgency",
        "Research"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "MagneticFieldStrength",
        this->GetHeaderValueAsFloat("MagneticFieldStrength")
    );

    /*  =======================================
     *  END: MR Image and Spectroscopy Instance Macro
     *  ======================================= */

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ImageType",
        string("ORIGINAL\\PRIMARY\\SPECTROSCOPY\\NONE")
    );

    /*  =======================================
     *  Spectroscopy Description Macro
     *  ======================================= */
    this->GetOutput()->GetDcmHeader()->SetValue(
        "VolumetricProperties",
        string("VOLUME")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "VolumeBasedCalculationTechnique",
        string("NONE")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ComplexImageComponent",
        string("COMPLEX")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "AcquisitionContrast",
        "UNKNOWN"
    );

    /*  =======================================
     *  END: Spectroscopy Description Macro
     *  ======================================= */


    this->GetOutput()->GetDcmHeader()->SetValue(
        "TransmitterFrequency",
        this->GetHeaderValueAsFloat( "MRFrequency" )
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SpectralWidth",
        1./this->GetHeaderValueAsFloat( "DwellTime") * 1e6
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_FrequencyOffset",
        0
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ChemicalShiftReference",
        this->GetPPMRef()     
   );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "VolumeLocalizationTechnique",
        "PRESS"
    );

    string volLocType = "UNKNOWN"; 

    if ( this->GetHeaderValueAsFloat( "VOIPhaseFOV" ) !=0 
        && this->GetHeaderValueAsFloat( "VOIReadoutFOV" ) !=0 
        && this->GetHeaderValueAsFloat( "VOIThickness" ) !=0 ) {

        this->InitVolumeLocalizationSeq();
        volLocType = "PRESS"; 
    } 

    this->GetOutput()->GetDcmHeader()->SetValue(
        "VolumeLocalizationTechnique",
        "PRESS"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "Decoupling",
        "NO"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "TimeDomainFiltering",
        "NONE"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "NumberOfZeroFills",
        0
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "BaselineCorrection",
        string("NONE")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "FrequencyCorrection",
        "NO"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "FirstOrderPhaseCorrection",
        string("NO")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "WaterReferencedPhaseCorrection",
        string("NO")
    );
}


/*!
 *
 */
void svkSiemensRdaReader::InitMRSpectroscopyPulseSequenceModule()
{
    this->GetOutput()->GetDcmHeader()->SetValue(
        "PulseSequenceName",
        this->GetHeaderValueAsString( "SequenceName" ) 
    );

    int numVoxels[3];
    numVoxels[0] = this->GetHeaderValueAsInt( "CSIMatrixSize[0]" );
    numVoxels[1] = this->GetHeaderValueAsInt( "CSIMatrixSize[1]" );
    numVoxels[2] = this->GetHeaderValueAsInt( "CSIMatrixSize[2]" );

    string acqType = "VOLUME";
    if (numVoxels[0] == 1 && numVoxels[1] == 1 &&  numVoxels[2] == 1) {
        acqType = "SINGLE VOXEL";
    }

    this->GetOutput()->GetDcmHeader()->SetValue(
        "MRSpectroscopyAcquisitionType",
        acqType
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "EchoPulseSequence",
        "SPIN"
    );
    this->GetOutput()->GetDcmHeader()->SetValue(
        "MultipleSpinEcho",
        "NO"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "MultiPlanarExcitation",
        "NO"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SteadyStatePulseSequence",
        "NONE"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "EchoPlanarPulseSequence",
        "NO"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SpectrallySelectedSuppression",
        "UNKNOWN"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "GeometryOfKSpaceTraversal",
        "RECTILINEAR"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "RectilinearPhaseEncodeReordering",
        "LINEAR"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SegmentedKSpaceTraversal",
        "SINGLE"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "CoverageOfKSpace",
        "FULL"
    );

    this->GetOutput()->GetDcmHeader()->SetValue( "NumberOfKSpaceTrajectories", 1 );
}



/*!
 *
 */
void svkSiemensRdaReader::InitMRSpectroscopyDataModule()
{
    int numVoxels[3];
    numVoxels[0] = this->GetHeaderValueAsInt( "CSIMatrixSize[0]" );
    numVoxels[1] = this->GetHeaderValueAsInt( "CSIMatrixSize[1]" );
    numVoxels[2] = this->GetHeaderValueAsInt( "CSIMatrixSize[2]" );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "Rows",
        numVoxels[1]
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "Columns",
        numVoxels[0]
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "DataPointRows",
        1
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "DataPointColumns",
        this->GetHeaderValueAsInt("VectorSize")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "DataRepresentation",
        "COMPLEX"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SignalDomainColumns",
        "TIME"
    );

    //  Private Attributes for spatial domain encoding.  rda data is already
    //  spatially reconstructed
    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_ColumnsDomain",
        "SPACE" 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_RowsDomain",
        "SPACE" 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_SliceDomain",
        "SPACE" 
    );
}


/*! 
 *  Use the RDA patient position string to set the DCM_PatientPosition data element.
 */
string svkSiemensRdaReader::GetDcmPatientPositionString(string patientPosition)
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
 *  Read RDA header fields into a string STL map for use during initialization 
 *  of DICOM header by Init*Module methods. 
 *  The RDA header consists of a list of "=" delimited key/value pairs. 
 */
void svkSiemensRdaReader::ParseRda()
{

    string rdaFileName( this->GetFileName() );

    try { 

        /*  Read in the RDA Header:
         */
        this->rdaFile = new ifstream();
        this->rdaFile->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        this->rdaFile->open( rdaFileName.c_str(), ifstream::in );
        if ( ! this->rdaFile->is_open() ) {
            throw runtime_error( "Could not open rda file: " + rdaFileName );
        } 

        this->ParseAndSetStringElements("FileName", rdaFileName);

        // determine how big the data buffer is (num pts * word size).  
        // header key-value pairs use total_bytes_in_file - sizeof_data_buffer
        // read key-value pairs from the top until start of data buffer. 
        this->fileSize = this->GetFileSize( this->rdaFile );

        while (! this->rdaFile->eof() ) {
            if ( this->GetRdaKeyValuePair() != 0 ) {
                break; 
            }
        }

        this->rdaFile->close();

        if (this->GetDebug()) {
            this->PrintKeyValuePairs(); 
        }

    } catch (const exception& e) {
        cerr << "ERROR opening or reading Siemens RDA file (" << rdaFileName << "): " << e.what() << endl;
    }
}


/*! 
 *  Utility function to read a single line from the RDA file and return 
 *  set the delimited key/value pair into the stl map.  
 *  Returns -1 if reading isn't successful. 
 */
int svkSiemensRdaReader::GetRdaKeyValuePair(  )    
{

    int status = 0; 

    istringstream* iss = new istringstream();

    string keyString;
    string valueString;

    try {

        this->ReadLine(this->rdaFile, iss); 

        size_t  position; 
        string  tmp; 

        //  Read only to the start of the pixel buffer, 
        //  i.e. no more than the header size, delimited 
        //  by "End of header":     
        tmp.assign( iss->str() );
        position = tmp.find(">>> End of header <<<"); 

        if ( position == string::npos ) {  

            //  find first white space position before "key" string: 
            if (this->GetDebug()) {
                cout << "DBG: " << tmp << endl;
            }
    
            //  Extract key and value strings:
            position = tmp.find_first_of(':');
            if (position != string::npos) {

                keyString.assign( tmp.substr(0, position) );
                keyString = StripWhite(keyString); 

                valueString.assign( tmp.substr(position + 2) );

                this->ParseAndSetStringElements(keyString, valueString);
            } 
            this->endOfHeaderPos = this->rdaFile->tellg();
            //cout << "EOF: " << this->rdaFile->tellg() << endl;

        } else { 

            this->endOfHeaderPos = this->rdaFile->tellg();
            //cout << "EOF2: " << this->rdaFile->tellg() << endl;
            this->rdaFile->seekg(0, ios::end);     

        }
    } catch (const exception& e) {

        if (this->GetDebug()) {
            cout <<  "ERROR reading line: " << e.what() << endl;
        }

        status = -1; 

    }

    delete iss; 
    return status; 
}


/*!
 *  Push key value pairs into the map's value vector: 
 *  For values that are comma separated lists, put each element into the value 
 *  vector. 
 */
void svkSiemensRdaReader::ParseAndSetStringElements(string key, string valueArrayString) 
{
    size_t pos;
    istringstream* iss = new istringstream();
    string tmpString;     

    while ( (pos = valueArrayString.find_first_of(',')) != string::npos) {  

        iss->str( valueArrayString.substr(0, pos) );
        *iss >> tmpString;
        this->rdaMap[key].push_back(tmpString); 
        iss->clear();

        valueArrayString.assign( valueArrayString.substr(pos + 1) ); 
    }
    iss->str( valueArrayString );
    *iss >> tmpString;
    this->rdaMap[key].push_back(tmpString); 
    delete iss; 
}


/*!
 *
 */
string svkSiemensRdaReader::GetStringFromFloat(float floatValue) 
{
    ostringstream tmpOss;
    tmpOss << floatValue; 
    return tmpOss.str(); 
}


/*!
 *
 */
int svkSiemensRdaReader::GetHeaderValueAsInt(string keyString, int valueIndex) 
{
    
    istringstream* iss = new istringstream();
    int value;

    iss->str( (this->rdaMap[keyString])[valueIndex]);
    *iss >> value;

    delete iss; 
    return value; 
}


/*!
 *
 */
float svkSiemensRdaReader::GetHeaderValueAsFloat(string keyString, int valueIndex) 
{
    
    istringstream* iss = new istringstream();
    float value;

    iss->str( (this->rdaMap[keyString])[valueIndex]);
    *iss >> value;
    delete iss; 
    return value; 
}


/*!
 *
 */
string svkSiemensRdaReader::GetHeaderValueAsString(string keyString, int valueIndex) 
{
    return (this->rdaMap[keyString])[valueIndex];
}


/*!
 *  Prints the key value pairs parsed from the header. 
 */
void svkSiemensRdaReader::PrintKeyValuePairs()
{

    //  Print out key value pairs parsed from header:
    map< string, vector<string> >::iterator mapIter;
    for ( mapIter = this->rdaMap.begin(); mapIter != this->rdaMap.end(); ++mapIter ) {
     
        cout << this->GetClassName() << " " << mapIter->first << " = ";

        vector<string>::iterator it;
        for ( it = this->rdaMap[mapIter->first].begin() ; it < this->rdaMap[mapIter->first].end(); it++ ) {
            cout << " " << *it ;
        }
        cout << endl;
    }
}


/*!
 *  Map 
 */
void svkSiemensRdaReader::MapDoubleValuesToFloat(double* specDataDbl, float* specData, int numVals)
{
    //  Get the input range for scaling: 
    double inputRange[2];
    inputRange[1] = std::numeric_limits<double>::max(); 
    inputRange[0] = -1 * inputRange[1];  
    inputRange[1] = 0; 
    inputRange[0] = 0;  
    for (int i = 0; i < numVals; i++) {
        if (specDataDbl[i] < inputRange[0] ) {
            inputRange[0] = specDataDbl[i]; 
        }
        if (specDataDbl[i] > inputRange[1] ) {
            inputRange[1] = specDataDbl[i]; 
        }
    }
        
    double deltaRangeIn = inputRange[1] - inputRange[0]; 

    //  Map to full dynamic range of target type:
    double outputRange[2];
    outputRange[1] = std::numeric_limits<float>::max(); 
    outputRange[0] = -1 * outputRange[1];  
    double deltaRangeOut = outputRange[1];// - outputRange[0];  
cout << "RO: " << deltaRangeOut<< endl;
cout << "Ri: " << deltaRangeIn<< endl;
    
    //  apply linear mapping from float range to short range using positive 
    //  values only. 
    //  maxShort = inputRangeMax * m + b , minShort = inputRangeMin * m + b

    double slope = deltaRangeOut/deltaRangeIn;
cout << "SLPE " << slope << endl;
slope = 1; 
    double scaledValue; 

    for (int i = 0; i < numVals; i++) {
        scaledValue =  slope * specDataDbl[i]; 
        specData[i] = scaledValue; 
        //cout << specDataDbl[i] << " -> " << specData[i] << endl;
    } 

}


/*!
 *
 */
float svkSiemensRdaReader::GetPPMRef()
{
    float freqOffset = 0;

    float transmitFreq = this->GetHeaderValueAsFloat( "MRFrequency" ); 

    float ppmRef = svkSpecUtils::GetPPMRef(transmitFreq, freqOffset);

    return ppmRef;
}


/*!
 *  Utility method returns the total number of Pixels in 3D.
 */
int svkSiemensRdaReader::GetNumPixelsInVol()
{
    return (
        ( (this->GetDataExtent())[1] ) *
        ( (this->GetDataExtent())[3] ) *
        ( (this->GetDataExtent())[5] )
    );
}


/*!
 *
 */
int svkSiemensRdaReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}

