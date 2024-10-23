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


#include <svkSdbmVolumeReader.h>
#include </usr/include/vtk/vtkCellData.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkGlobFileNames.h>
#include </usr/include/vtk/vtkSortFileNames.h>
#include </usr/include/vtk/vtkByteSwap.h>

#include <sys/stat.h>

#include <cerrno>



using namespace svk;


//vtkCxxRevisionMacro(svkSdbmVolumeReader, "$Rev$");
vtkStandardNewMacro(svkSdbmVolumeReader);


const string svkSdbmVolumeReader::MFG_STRING = "GE MEDICAL SYSTEMS";


/*!
 *
 */
svkSdbmVolumeReader::svkSdbmVolumeReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkSdbmVolumeReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->specData = NULL; 
    this->shfHdr = NULL; 
    this->numCoils = 1; 
    this->iod = NULL; 

    // Set the byte ordering, as big-endian.
    this->SetDataByteOrderToBigEndian();
}



/*!
 *
 */
svkSdbmVolumeReader::~svkSdbmVolumeReader()
{

    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if ( this->shfHdr != NULL )  {
        delete shfHdr;
        this->shfHdr = NULL;
    }

    if ( this->specData != NULL )  {
        delete [] specData;
        this->specData = NULL;
    }

}


/*!
 *  Check to see if the extension indicates a GE SDBM file.  If so, try
 *  to open the file for reading.  If that works, then return a success code.
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkSdbmVolumeReader::CanReadFile(const char* fname)
{

    string fileToCheck(fname);

    if( fileToCheck.size() > 4 ) {

        // Also see "vtkImageReader2::GetFileExtensions method" 
        if ( 
            fileToCheck.substr( fileToCheck.size() - 4 ) == ".shf" 
        )  {
            FILE *fp = fopen(fname, "rb");
            if (fp) {
                fclose(fp);

                vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's a GE SDBM File: " << fileToCheck);
                return 1;
            }
        } else {
            vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's NOT a GE SDBM File: " << fileToCheck);
            return 0;
        }

    } else {
        vtkDebugMacro(<< this->GetClassName() <<"::CanReadFile(): It's NOT a valid file name : " << fileToCheck);
        return 0;
    }
}


/*!
 *
 */
int svkSdbmVolumeReader::GetNumVoxelsInVol()
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
int svkSdbmVolumeReader::GetNumSlices()
{
    return (this->GetDataExtent())[5] + 1;
}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called before ExecuteData()
 */
void svkSdbmVolumeReader::ExecuteInformation()
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
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called after ExecuteInformation()
 */
void svkSdbmVolumeReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
{

    this->FileNames = vtkStringArray::New();
    this->FileNames->DeepCopy(this->tmpFileNames);
    this->tmpFileNames->Delete();
    this->tmpFileNames = NULL;

    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );

    svkImageData* data = svkImageData::SafeDownCast( this->AllocateOutputData(output, outInfo) );

    if ( this->GetFileNames()->GetNumberOfValues() ) {
        vtkDebugMacro( << this->GetClassName() << " FileName: " << FileName );
        struct stat fs;
        if ( stat(this->GetFileNames()->GetValue(0), &fs) ) {
            vtkErrorMacro("Unable to open file " << string(this->GetFileNames()->GetValue(0)) );
            return;
        }
        this->ReadComplexFile(data);
    }

    double dcos[3][3];
    this->GetOutput()->GetDcmHeader()->SetSliceOrder( this->dataSliceOrder );
    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
    this->GetOutput()->SetDcos(dcos);

    //  SetNumberOfIncrements is supposed to call this, but only works if the data has already
    //  been allocated. but that requires the number of components to be specified.
    this->GetOutput()->GetIncrements();

}


/*!
 *
 */
void svkSdbmVolumeReader::ReadComplexFile(vtkImageData* data)
{

    string shfFilePath( this->GetFilePath( this->GetFileNames()->GetValue(0) ) );
    string cmplxFile = shfFilePath + "/" +  shfMap["data_file_name"]; 

    ifstream* cmplxDataIn = new ifstream();
    //cmplxDataIn->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );


    cmplxDataIn->open( cmplxFile.c_str(), ios::in);
    if( !cmplxDataIn ) {
        cout << strerror(errno) << '\n'; // displays "Permission denied"
        exit(1);
    }

    int numComponents = 2;
    int numPts = this->GetHeaderValueAsInt(shfMap, "num_pts_0"); 
    int numBytesInVol = this->GetNumPixelsInVol() * numPts * numComponents * sizeof(float) ; 

    for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {

        int coilNum = 0; 
        if ( this->numCoils > 1 ) {
            coilNum = fileIndex;     
        }

        this->specData = new float[ numBytesInVol/sizeof(float) ];  
        int offsetToData = this->GetHeaderValueAsInt(shfMap, "offset_to_data"); 
        
        cmplxDataIn->seekg(offsetToData);
        cmplxDataIn->read( (char*)(this->specData), numBytesInVol );

        #ifdef VTK_WORDS_BIGENDIAN
            vtkByteSwap::SwapVoidRange((void *)this->specData, this->GetNumPixelsInVol() * numPts * numComponents, sizeof(float));
        #endif

        for (int z = 0; z < (this->GetDataExtent())[5] ; z++) {
            for (int y = 0; y < (this->GetDataExtent())[3]; y++) {
                for (int x = 0; x < (this->GetDataExtent())[1]; x++) {
                    SetCellSpectrum(data, x, y, z, coilNum);
                }
            }
        }

        delete [] this->specData; 
        specData = NULL;
        
    }

    cmplxDataIn->close(); 
    delete cmplxDataIn;

}


/*!
 *  Utility method returns the total number of Pixels in 3D.
 */
int svkSdbmVolumeReader::GetNumPixelsInVol()
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
void svkSdbmVolumeReader::SetCellSpectrum(vtkImageData* data, int x, int y, int z, int coilNum, int timepoint)
{

    //  Set XY points to plot - 2 components per tuble for complex data sets:
    vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
    dataArray->SetNumberOfComponents(2);

    int numPts = this->GetHeaderValueAsInt(shfMap, "num_pts_0"); 
    int numComponents = 2;  
    //int numComponents =  this->GetHeaderValueAsInt( shfMap, "numberOfComponents" ); 
    dataArray->SetNumberOfTuples(numPts);
    char arrayName[30];

    sprintf(arrayName, "%d %d %d 0 %d", x, y, z, coilNum);
    dataArray->SetName(arrayName);

    //  preallocate float array for spectrum and use to iniitialize the vtkDataArray:
    //  don't do this for each call
    int numVoxels[3]; 
    numVoxels[0] = this->GetHeaderValueAsInt(shfMap, "num_pts_1"); 
    numVoxels[1] = this->GetHeaderValueAsInt(shfMap, "num_pts_2"); 
    numVoxels[2] = this->GetHeaderValueAsInt(shfMap, "num_pts_2"); 

    int offset = (numPts * numComponents) *  (
                        (numVoxels[0] * numVoxels[1]) * z
                        + numVoxels[0] * y
                        + x ); 

    for (int i = 0; i < numPts; i++) {
        dataArray->SetTuple(i, &(specData[offset + (i * 2)]));
    }

    //  Add the spectrum's dataArray to the CellData:
    //  vtkCellData is a subclass of vtkFieldData
    data->GetCellData()->AddArray(dataArray);

    dataArray->Delete();

    return;
}


/*!
 *  Initializes the svkDcmHeader adapter to a specific IOD type      
 *  and initizlizes the svkDcmHeader member of the svkImageData 
 *  object.    
 */
void svkSdbmVolumeReader::InitDcmHeader()
{
    vtkDebugMacro( << this->GetClassName() << "::InitDcmHeader()" );

    this->iod = svkMRSIOD::New();
    this->iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    this->iod->InitDcmHeader();

    this->ParseShf(); 
    this->PrintKeyValuePairs();

    //  Fill in data set specific values:
    this->InitPatientModule(); 
    this->InitGeneralStudyModule(); 
    this->InitGeneralSeriesModule(); 
    this->InitGeneralEquipmentModule(); 
    this->InitMultiFrameFunctionalGroupsModule();
    this->InitMultiFrameDimensionModule();
    this->InitAcquisitionContextModule();
    this->InitMRSpectroscopyModule(); 
    this->InitMRSpectroscopyPulseSequenceModule(); 
    this->InitMRSpectroscopyDataModule(); 

    if (this->GetDebug()) { 
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }

    iod->Delete();
}


/*!
 *  Read DDF header fields into a string STL map for use during initialization
 *  of DICOM header by Init*Module methods.
 */
void svkSdbmVolumeReader::ParseShf()
{

    string shfFileName( this->GetFileName() );
    string shfFileExtension( this->GetFileExtension( this->GetFileName() ) );
    string shfFilePath( this->GetFilePath( this->GetFileName() ) );
    vtkGlobFileNames* globFileNames = vtkGlobFileNames::New();
    globFileNames->AddFileNames( string( shfFilePath + "/*." + shfFileExtension).c_str() );

    vtkSortFileNames* sortFileNames = vtkSortFileNames::New();
    sortFileNames->GroupingOn();
    sortFileNames->SetInputFileNames( globFileNames->GetFileNames() );
    sortFileNames->NumericSortOn();
    sortFileNames->Update();

    //  If globed file names are not similar, use only the 0th group. 
    if (sortFileNames->GetNumberOfGroups() > 1 ) {

        vtkWarningWithObjectMacro(this, "Found Multiple shf file groups, using only specified file ");

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



    try {

        //  Read in the SHF Header:
        this->shfHdr = new ifstream();
        this->shfHdr->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        int fileIndex = 0; 
        string currentShfFileName( this->GetFileNames()->GetValue( fileIndex ) );

        this->shfHdr->open( currentShfFileName.c_str(), ifstream::in );

        if ( ! this->shfHdr->is_open() ) {
            throw runtime_error( "Could not open volume file: " + shfFileName );
        }


        istringstream* iss = new istringstream();

        while ( ! this->shfHdr->eof() ) {

            string key; 
            string value; 

            if ( this->ReadLineKeyValue(iss, ' ', &key, &value ) == 0 ) {

                //cout << "key: " << key << " = " << value << endl;
                shfMap[key] = value; 

            } else {

                break;

            }

            if ( key.compare("dimen_num") == 0)  {


                //  parse dims: 

                //  DIM 0
                this->ParseShfDim( value ); 

                //  Skip 5 lines: 
                for (int i = 0; i < 5; i++) {
                    this->ReadLine(this->shfHdr, iss);
                }
                //  DIM 1
                this->ParseShfDim( "" ); 
        
                //  Skip 5 lines: 
                for (int i = 0; i < 5; i++) {
                    this->ReadLine(this->shfHdr, iss);
                }
                //  DIM 2
                this->ParseShfDim( "" ); 
        
                //  Skip 5 lines: 
                for (int i = 0; i < 5; i++) {
                    this->ReadLine(this->shfHdr, iss);
                }
                //  DIM 3
                this->ParseShfDim( "" ); 

                break; 
            } 

        }

        delete iss;

    } catch (const exception& e) {
        cerr << "ERROR opening or reading shf file (" << shfFileName << "): " << e.what() << endl;
    }

    sortFileNames->Delete();
    globFileNames->Delete();
}


/*!
 *
 */
void svkSdbmVolumeReader::ParseShfDim( string dimenNum )
{
    istringstream* iss = new istringstream();

    string key; 
    string value; 
    //  first time the dimension number is alrady know, otherwise get it: 
    if ( dimenNum.size() == 0 ) {
        this->ReadLineKeyValue(iss, ' ', &key, &value ); 
        shfMap["dimen_num"] =  value; 
    } else { 
        shfMap["dimen_num"] =  dimenNum; 
    }

    int dimNum = this->GetHeaderValueAsInt(shfMap, "dimen_num"); 
    ostringstream ossIndex;
    ossIndex << dimNum;
    string indexString(ossIndex.str());
    string ruString = "rhuser" + indexString; 

    this->ReadLineKeyValue(iss, ' ', &key, &value ); 
    shfMap[key + "_" + indexString] = value; 
    this->ReadLineKeyValue(iss, ' ', &key, &value ); 
    shfMap[key + "_" + indexString] = value;
    this->ReadLineKeyValue(iss, ' ', &key, &value ); 
    shfMap[key + "_" + indexString] = value;
    this->ReadLineKeyValue(iss, ' ', &key, &value ); 
    shfMap[key + "_" + indexString] = value;
    this->ReadLineKeyValue(iss, ' ', &key, &value ); 
    shfMap[key + "_" + indexString] = value;
    this->ReadLineKeyValue(iss, ' ', &key, &value ); 
    shfMap[key + "_" + indexString] = value;
    this->ReadLineKeyValue(iss, ' ', &key, &value ); 
    shfMap[key + "_" + indexString] = value;
    this->ReadLineKeyValue(iss, ' ', &key, &value ); 
    shfMap[key + "_" + indexString] = value;
    this->ReadLineKeyValue(iss, ' ', &key, &value ); 
    shfMap[key + "_" + indexString] = value;
    this->ReadLineKeyValue(iss, ' ', &key, &value ); 
    shfMap[key + "_" + indexString] = value;

    delete iss;
}


/*!
 *  Initializes the VolumeLocalizationSequence in the MRSpectroscopy
 *  DICOM object for PRESS excitation.  
 */
void svkSdbmVolumeReader::InitVolumeLocalizationSeq()
{

    //  Get Thickness Values
    float selBoxSize[3]; 
    selBoxSize[0] = this->GetHeaderValueAsFloat(shfMap, "sl_thick_1"); 
    selBoxSize[1] = this->GetHeaderValueAsFloat(shfMap, "sl_thick_2"); 
    selBoxSize[2] = this->GetHeaderValueAsFloat(shfMap, "sl_thick_3"); 

    //  if the sel box is 0 in all 3 dimensions, set localization to none
    //  and reset sel box to full fov.  Otherwise, if some directions 
    //   have sl_thick = 0, set those to the full fov
    if ( selBoxSize[0] == 0 && selBoxSize[1] &&  selBoxSize[2] ) {
        this->GetOutput()->GetDcmHeader()->SetValue(
            "VolumeLocalizationTechnique", 
            "NONE" 
        );
        return;
    }
    //  If any 1 or 2 dims are zero and localization is only say a slab, set the localization in 
    //  other 2 dims to full fov:
    int numVoxels[3]; 
    numVoxels[0] = this->GetHeaderValueAsInt(shfMap, "num_pts_1"); 
    numVoxels[1] = this->GetHeaderValueAsInt(shfMap, "num_pts_2"); 
    numVoxels[2] = this->GetHeaderValueAsInt(shfMap, "num_pts_2"); 
    double voxelSpacing[3];
    this->GetOutput()->GetDcmHeader()->GetPixelSpacing( voxelSpacing );
    for (int i = 0; i < 3; i++ ) {
        if (selBoxSize[i] == 0 ) {
            selBoxSize[i] = numVoxels[i] * voxelSpacing[i];
        }
    }

    //  Get Center Location Values
    float selBoxCenter[3]; 
    selBoxCenter[0] = this->GetHeaderValueAsFloat(shfMap, "sl_loc_1"); 
    selBoxCenter[1] = this->GetHeaderValueAsFloat(shfMap, "sl_loc_2"); 
    selBoxCenter[2] = this->GetHeaderValueAsFloat(shfMap, "sl_loc_3"); 

    //  Get Orientation Values 
    float selBoxOrientation[3][3]; 
    selBoxOrientation[0][0] = this->dcos[0][0]; 
    selBoxOrientation[0][1] = this->dcos[0][1]; 
    selBoxOrientation[0][2] = this->dcos[0][2]; 
    selBoxOrientation[1][0] = this->dcos[1][0]; 
    selBoxOrientation[1][1] = this->dcos[1][1]; 
    selBoxOrientation[1][2] = this->dcos[1][2]; 
    selBoxOrientation[2][0] = this->dcos[2][0]; 
    selBoxOrientation[2][1] = this->dcos[2][1]; 
    selBoxOrientation[2][2] = this->dcos[2][2]; 

    this->GetOutput()->GetDcmHeader()->InitVolumeLocalizationSeq(
        selBoxSize,
        selBoxCenter,
        selBoxOrientation 
    );

}


/*!
 *
 */
void svkSdbmVolumeReader::InitPatientModule() 
{
    this->GetOutput()->GetDcmHeader()->InitPatientModule(
        shfMap["patient_name"], 
        shfMap["patient_id"], 
        "",
        "" 
    );
}


/*!
 *
 */
void svkSdbmVolumeReader::InitGeneralStudyModule() 
{

    this->GetOutput()->GetDcmHeader()->InitGeneralStudyModule(
        this->RemoveDelimFromDate( &(shfMap["date_acquired"]) ), 
        "",
        "",
        shfMap["study_number"], 
        "", 
        "" 
    );
}


/*!
 *
 */
void svkSdbmVolumeReader::InitGeneralSeriesModule() 
{

    string patientEntryPos; 

    //  entry:       Patient Entry, 1=head first, 2=feet first.
    int entry = this->GetHeaderValueAsInt(shfMap, "entry"); 
    if ( entry == 1) {
        patientEntryPos = "HF";
    } else if ( entry == 2 ) {
        patientEntryPos = "FF";
    }

    //  position:    Patient Pos, 1=supine, 2=prone, 4=decub L, 8=decub R.
    int position = this->GetHeaderValueAsInt(shfMap, "position"); 
    if ( position == 1 ) {
        patientEntryPos.append("S");
    } else if ( position == 2 ) {
        patientEntryPos.append("P");
    } else if ( position == 4 ) {
        patientEntryPos.append("DL");
    } else if ( position == 8 ) {
        patientEntryPos.append("DR");
    }

    this->GetOutput()->GetDcmHeader()->InitGeneralSeriesModule(
        shfMap["series_number"], 
        "SDBM MRSI",
        patientEntryPos 
    );

}


/*!
 *  DDF is historically the UCSF representation of a GE raw file so 
 *  initialize to svkSdbmVolumeReader::MFG_STRING.
 */
void svkSdbmVolumeReader::InitGeneralEquipmentModule()
{
    this->GetOutput()->GetDcmHeader()->SetValue(
        "Manufacturer", 
        svkSdbmVolumeReader::MFG_STRING 
    );
}


/*!
 *
 */
void svkSdbmVolumeReader::InitMultiFrameFunctionalGroupsModule()
{
    InitSharedFunctionalGroupMacros();

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "InstanceNumber", 
        1 
    );

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "ContentDate", 
        this->RemoveDelimFromDate( &(shfMap["date_acquired"]) ) 
    );

    this->numSlices = this->GetHeaderValueAsInt(shfMap, "num_pts_3"); 

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "NumberOfFrames", 
        this->numSlices * this->numCoils
    );

    InitPerFrameFunctionalGroupMacros();
}


/*!
 *
 */
void svkSdbmVolumeReader::InitSharedFunctionalGroupMacros()
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
}


/*!
 *
 */
void svkSdbmVolumeReader::InitPerFrameFunctionalGroupMacros()
{
    double voxelSpacing[3];
    this->GetOutput()->GetDcmHeader()->GetPixelSpacing( voxelSpacing );

    //  Get toplc float array from shfMap and use that to generate
    //  frame locations.  This position is off by 1/2 voxel, fixed below:
    double toplc[3];
    toplc[0] = -1 * this->GetHeaderValueAsFloat(shfMap, "tlhc_R"); 
    toplc[1] = -1 * this->GetHeaderValueAsFloat(shfMap, "tlhc_A"); 
    toplc[2] = this->GetHeaderValueAsFloat(shfMap, "tlhc_S"); 

    svkDcmHeader::DimensionVector dimensionVector = this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, this->numSlices-1);
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::CHANNEL_INDEX, this->numCoils-1);

    this->GetOutput()->GetDcmHeader()->InitPerFrameFunctionalGroupSequence(
            toplc,
            voxelSpacing,
            this->dcos,
            &dimensionVector
    );

}


/*!
 *  Pixel Spacing:
 */
void svkSdbmVolumeReader::InitPixelMeasuresMacro()
{

    //  get the spacing for the specified index:

    float colSpacing = this->GetHeaderValueAsInt(shfMap, "width_1") / this->GetHeaderValueAsInt(shfMap, "num_pts_1"); 
    float rowSpacing = this->GetHeaderValueAsInt(shfMap, "width_2") / this->GetHeaderValueAsInt(shfMap, "num_pts_2"); 
    float sliceThickness = this->GetHeaderValueAsInt(shfMap, "width_3") / this->GetHeaderValueAsInt(shfMap, "num_pts_3"); 

    string pixelSpacing;
    ostringstream oss;
    oss << colSpacing;
    oss << '\\';
    oss << rowSpacing;
    pixelSpacing = oss.str();

    ostringstream ossThickness;
    ossThickness << sliceThickness;
    string sliceThicknessString = ossThickness.str();

    this->GetOutput()->GetDcmHeader()->InitPixelMeasuresMacro(
        pixelSpacing,
        sliceThicknessString
    );

}


/*!
 *  Mandatory, Must be a per-frame functional group
 */
void svkSdbmVolumeReader::InitFrameContentMacro()
{

    unsigned int* indexValues;
    int numIndices = 1; 
    if ( this->numCoils == 1 ) {
        indexValues = new unsigned int[1]; 
    } else if ( this->numCoils > 1 ) {
        indexValues = new unsigned int[2]; 
        numIndices = 2; 
    }
    
    int frame = 0; 

    for (int j = 0; j < this->numCoils; j++) {

        for (int i = 0; i < this->numSlices; i++) {

            indexValues[0] = i; 
            if (this->numCoils > 1) {
                indexValues[1] = j; 
            }

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
                numIndices,         // num values in array
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

    delete[] indexValues;

}


/*!
 *
 */
void svkSdbmVolumeReader::InitPlaneOrientationMacro()
{
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "PlaneOrientationSequence"
    );

    //  plane:       Scan Plane, 2=Axial, 4=Sagittal, 8=Coronal, 16=Oblique.
    //  obplane:     18=Axial Oblique, 20=Sag Oblique, 24=Coronal Oblique.
    //  rotation:    Rotation, where 0=0, 1=90, 2=180, 3=270 degrees CCW.
    //  tlhc:        Top Left Hand Corner.
    //  trhc:        Top Right Hand Corner.
    //  brhc:        Bottom Right Hand Corner.

    /*  
     *  ImageOrientation is a 6 vector describing a vector along the columns and 
     *  rows directions of the data.  Calculate this from the tlhc, trhc, brhc values. 
     *  Should correspond with the orientation descriptors (plane, oblane)
     */
    float tlhc[3]; 
    tlhc[0] = -1 * this->GetHeaderValueAsFloat(shfMap, "tlhc_R"); 
    tlhc[1] = -1 * this->GetHeaderValueAsFloat(shfMap, "tlhc_A"); 
    tlhc[2] = this->GetHeaderValueAsFloat(shfMap, "tlhc_S"); 

    float trhc[3]; 
    trhc[0] = -1 * this->GetHeaderValueAsFloat(shfMap, "trhc_R"); 
    trhc[1] = -1 * this->GetHeaderValueAsFloat(shfMap, "trhc_A"); 
    trhc[2] = this->GetHeaderValueAsFloat(shfMap, "trhc_S"); 

    float brhc[3]; 
    brhc[0] = -1 * this->GetHeaderValueAsFloat(shfMap, "brhc_R"); 
    brhc[1] = -1 * this->GetHeaderValueAsFloat(shfMap, "brhc_A"); 
    brhc[2] = this->GetHeaderValueAsFloat(shfMap, "brhc_S"); 

    //  Determine vectors in LPS (DICOM) rather than RAS 
    float colDir[3]; 
    for (int i = 0; i < 3; i++) {
        colDir[i] = trhc[i] - tlhc[i];
    }
    float colNorm = vtkMath::Normalize(colDir); 

    float rowDir[3]; 
    for (int i = 0; i < 3; i++) {
        rowDir[i] = brhc[i] - trhc[i];
    }
    float rowNorm = vtkMath::Normalize(rowDir); 

    ostringstream ossDcos;
    ossDcos << colDir[0];
    ossDcos<< "\\";
    ossDcos << colDir[1];
    ossDcos<< "\\";
    ossDcos << colDir[2];
    ossDcos<< "\\";
    ossDcos << rowDir[0];
    ossDcos<< "\\";
    ossDcos << rowDir[1];
    ossDcos<< "\\";
    ossDcos << rowDir[2];

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PlaneOrientationSequence",         
        0,                                 
        "ImageOrientationPatient",        
        ossDcos.str(), 
        "SharedFunctionalGroupsSequence",
        0                               
    );


    //  Determine whether the data is ordered with or against the slice normal direction.
    double normal[3]; 
    this->GetOutput()->GetDcmHeader()->GetNormalVector(normal); 

    for (int i = 0; i < 3; i++) {
        this->dcos[0][i] = colDir[i]; 
        this->dcos[1][i] = rowDir[i]; 
        this->dcos[2][i] = normal[i]; 
    }


    double dcosSliceOrder[3]; 
    for (int i = 0; i < 3; i++) {
        dcosSliceOrder[i] = dcos[2][i]; 
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

}


/*!
 *
 */
void svkSdbmVolumeReader::InitMRTimingAndRelatedParametersMacro()
{
    this->GetOutput()->GetDcmHeader()->InitMRTimingAndRelatedParametersMacro(
        this->GetHeaderValueAsFloat(shfMap, "TR"), 
        999
    ); 
}


/*!
 *
 */
void svkSdbmVolumeReader::InitMRSpectroscopyFOVGeometryMacro()
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
        this->GetHeaderValueAsInt(shfMap, "num_pts_0"), 
        "SharedFunctionalGroupsSequence",
        0
    );

//
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionPhaseColumns",
        this->GetHeaderValueAsInt(shfMap, "num_pts_1"), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionPhaseRows",
        this->GetHeaderValueAsInt(shfMap, "num_pts_2"), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionOutOfPlanePhaseSteps",
        this->GetHeaderValueAsInt(shfMap, "num_pts_3"),
        "SharedFunctionalGroupsSequence",
        0
    );

//
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                    
        "InPlanePhaseEncodingDirection",     
        string("COLUMN"),
        "SharedFunctionalGroupsSequence",   
        0                                 
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence", 
        0,                                  
        "MRAcquisitionFrequencyEncodingSteps", 
        1,
        "SharedFunctionalGroupsSequence",      
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "MRAcquisitionPhaseEncodingStepsInPlane",
        1,
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "MRAcquisitionPhaseEncodingStepsOutOfPlane",
        1,
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
void svkSdbmVolumeReader::InitMREchoMacro()
{
    this->GetOutput()->GetDcmHeader()->InitMREchoMacro( this->GetHeaderValueAsFloat( shfMap, "TE") );
}


/*!
 *
 */
void svkSdbmVolumeReader::InitMRModifierMacro()
{
    float inversionTime = 0;  
    this->GetOutput()->GetDcmHeader()->InitMREchoMacro( inversionTime );
}


/*!
 *
 */
void svkSdbmVolumeReader::InitMRReceiveCoilMacro()
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
        shfMap["coil_name"], 
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,                        
        "ReceiveCoilManufacturerName",       
        string("GE"),
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    string coilType("VOLUME"); 
    this->numCoils =  this->GetHeaderValueAsInt(shfMap, "num_coils");  
    if ( this->numCoils < 0 ) {
        this->numCoils = 1; 
    }
    if ( this->numCoils > 1 ) {
        coilType.assign("MULTICOIL"); 
    }

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
        string("YES"),
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    //=====================================================
    //  Multi Coil Sequence
    //=====================================================
    if ( strcmp(coilType.c_str(), "MULTICOIL") == 0)  { 

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
            "MRReceiveCoilSequence",
            0, 
            "MultiCoilDefinitionSequence"
        );

        /*
         *  If multi-channel coil, assume each file corresponds to an individual channel:
         */
        for (int coilIndex = 0; coilIndex < this->numCoils; coilIndex++) {

            ostringstream ossIndex;
            ossIndex << coilIndex;
            string indexString(ossIndex.str());

            this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
                "MultiCoilDefinitionSequence",
                coilIndex,                        
                "MultiCoilElementName",       
                coilIndex, //"9", //indexString, 
                "MRReceiveCoilSequence",
                0                      
            );


            this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
                "MultiCoilDefinitionSequence",
                coilIndex,                        
                "MultiCoilElementUsed",       
                "YES", 
                "MRReceiveCoilSequence",
                0                      
            );
        }

    }

}


/*!
 *
 */
void svkSdbmVolumeReader::InitMRTransmitCoilMacro()
{
    this->GetOutput()->GetDcmHeader()->InitMRTransmitCoilMacro("GE", "UNKNOWN", "BODY");
}


/*!
 *
 */
void svkSdbmVolumeReader::InitMRAveragesMacro()
{
    int numAverages = 1; 
    this->GetOutput()->GetDcmHeader()->InitMRAveragesMacro(numAverages);
}


/*!
 *  Shf doesn't support sat band encoding: 
 */
void svkSdbmVolumeReader::InitMRSpatialSaturationMacro()
{

    int numSatBands =  0; 

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRSpatialSaturationSequence"
    );

}


/*!
 *
 */
void svkSdbmVolumeReader::InitMultiFrameDimensionModule()
{
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "DimensionIndexSequence",
        0,
        "DimensionDescriptionLabel",
        "Slice"
    );

    if (this->numCoils > 1) {
        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "DimensionIndexSequence",
            1,
            "DimensionDescriptionLabel",
            "Coil Number"
        );
    }

/*
    if ( this->numCoils > 1 ) {
        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "DimensionIndexSequence",
            1,
            "DimensionIndexPointer",
            "18H\\00H\\47H\\90"
        );
        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "DimensionIndexSequence",
            1,
            "FunctionalGroupPointer",
            //"MultiCoilDefinitionSequence"
            "18H\\00H\\47H\\90"
        );
    }
*/
}


/*!
 *
 */
void svkSdbmVolumeReader::InitAcquisitionContextModule()
{
}


/*!
 *
 */
void svkSdbmVolumeReader::InitMRSpectroscopyModule()
{

    /*  =======================================
     *  MR Image and Spectroscopy Instance Macro
     *  ======================================= */

    this->GetOutput()->GetDcmHeader()->SetValue(
        "AcquisitionDateTime",
        this->RemoveDelimFromDate( &(shfMap["date_acquired"]) ) + "000000"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "AcquisitionDuration",
        0
    );


    // ==========================
    //  gyromatnetic ratios:
    //  1H  1   0   1/2     42.58 
    //  2H  1   1   1       6.54 
    //  31P     1   0   1/2 17.25 
    //  23Na    1   2   3/2 11.27 
    //  14N     1   1   1   3.08 
    //  13C     0   1   1/2 10.71 
    //  19F     1   0   1/2 40.08 
    // ==========================
    float magStrength = this->GetHeaderValueAsFloat(shfMap, "magstrength"); 

    string isotope; 
    if ( magStrength == 1.5 ) {
        if ( ( shfMap["center_freq"] ).find("63") != string::npos) { 
            isotope.assign("1H"); 
        }
    } else if (magStrength == 3 ) {
        if ( ( shfMap["center_freq"] ).find("127") != string::npos) { 
            isotope.assign("1H"); 
        } else if (( shfMap["center_freq"] ).find("51") != string::npos) {
            isotope.assign("31P"); 
        } else if (( shfMap["center_freq"] ).find("32") != string::npos) {
            isotope.assign("13C"); 
        }
    }
    
    this->GetOutput()->GetDcmHeader()->SetValue(
        "ResonantNucleus", 
        isotope
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
        this->GetHeaderValueAsFloat(shfMap, "magstrength")
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

    string dataType; 
    if ( ( shfMap["data_type"] ).find("Complex") != string::npos) { 
        dataType.assign("COMPLEX"); 
    }
    
    this->GetOutput()->GetDcmHeader()->SetValue(
        "ComplexImageComponent", 
        dataType 
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
        this->GetHeaderValueAsFloat(shfMap, "center_freq")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SpectralWidth", 
        this->GetHeaderValueAsFloat(shfMap, "width_0")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_FrequencyOffset",
        0
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ChemicalShiftReference", 
        this->GetHeaderValueAsFloat(shfMap, "ppm_offset")
    );

    //  hardcode for now
    this->GetOutput()->GetDcmHeader()->SetValue(
        "VolumeLocalizationTechnique", 
        "PRESS" 
    );
    this->InitVolumeLocalizationSeq();

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
void svkSdbmVolumeReader::InitMRSpectroscopyPulseSequenceModule()
{
    this->GetOutput()->GetDcmHeader()->SetValue(
        "PulseSequenceName", 
        shfMap["psd_name"]
    );

    int numVoxels[3];
    numVoxels[0] = this->GetHeaderValueAsInt(shfMap, "num_pts_1"); 
    numVoxels[1] = this->GetHeaderValueAsInt(shfMap, "num_pts_2"); 
    numVoxels[2] = this->GetHeaderValueAsInt(shfMap, "num_pts_3"); 

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
        "WATER" 
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
void svkSdbmVolumeReader::InitMRSpectroscopyDataModule()
{
    int numVoxels[3];
    numVoxels[0] = this->GetHeaderValueAsInt(shfMap, "num_pts_1"); 
    numVoxels[1] = this->GetHeaderValueAsInt(shfMap, "num_pts_2"); 
    numVoxels[2] = this->GetHeaderValueAsInt(shfMap, "num_pts_3"); 

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
        this->GetHeaderValueAsInt(shfMap, "num_pts_0") 
    );

    string dataType; 
    if ( ( shfMap["data_type"] ).find("Complex") != string::npos) { 
        dataType.assign("COMPLEX"); 
    } else {
        dataType.assign("REAL"); 
    }

    this->GetOutput()->GetDcmHeader()->SetValue(
        "DataRepresentation", 
        dataType 
    );

    string signalDomain; 
    if ( strcmp(shfMap["domain_0"].c_str(), "Time") == 0)  { 
        signalDomain.assign("TIME"); 
    } else if ( strcmp(shfMap["domain_0"].c_str(), "Frequency") == 0)  { 
        signalDomain.assign("FREQUENCY"); 
    }

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SignalDomainColumns", 
        signalDomain 
    );

    //  Private Attributes for spatial domain encoding:

    if ( strcmp(shfMap["domain_1"].c_str(), "Space") == 0)  { 
        signalDomain.assign("SPACE"); 
    } else { 
        signalDomain.assign("KSPACE"); 
    }
    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_ColumnsDomain",
        signalDomain
    );

    if ( strcmp(shfMap["domain_2"].c_str(), "Space") == 0)  { 
        signalDomain.assign("SPACE"); 
    } else { 
        signalDomain.assign("KSPACE"); 
    }
    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_RowsDomain",
        signalDomain
    );

    if ( strcmp(shfMap["domain_3"].c_str(), "Space") == 0)  { 
        signalDomain.assign("SPACE"); 
    } else { 
        signalDomain.assign("KSPACE"); 
    }
    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_SliceDomain",
        signalDomain
    );

}


/*!
 *  Returns the file root without extension
 */
svkDcmHeader::DcmPixelDataFormat svkSdbmVolumeReader::GetFileType()
{
    return svkDcmHeader::SIGNED_FLOAT_4;
}




/*!
 *
 */
int svkSdbmVolumeReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}


/*!
 *  Prints the key value pairs parsed from the header.
 */
void svkSdbmVolumeReader::PrintKeyValuePairs()
{
    if (this->GetDebug()) {
        map< string, string >::iterator mapIter;
        for ( mapIter = shfMap.begin(); mapIter != shfMap.end(); ++mapIter ) {
            cout << this->GetClassName() << " " << mapIter->first << " = ";
            cout << shfMap[mapIter->first] << endl;
        }
    }
}


string svkSdbmVolumeReader::ReadLineIgnore(istringstream* iss, char delim)
{
    this->ReadLine(this->shfHdr, iss);
    iss->ignore(256, delim);
    string value;
    *iss >> value; 
    return value; 
}


/*! 
 *  Read the value part of a delimited key value line in a file: 
 */
int svkSdbmVolumeReader::ReadLineKeyValue(istringstream* iss, char delim, string* key, string* value)
{

    this->ReadLine(this->shfHdr, iss);

    try {

        string line;
        line.assign( iss->str() );

        size_t delimPos = line.find_first_of(delim);
        string valueTmp; 
        string keyTmp; 

        if (delimPos != string::npos) {
            keyTmp.assign( line.substr( 0, delimPos ) );
            valueTmp.assign( line.substr( delimPos + 1 ) );
        } else {
            keyTmp.assign( line );
            valueTmp.assign( line );
        }

        // remove trailing white space from key: 
        size_t firstSpace = keyTmp.find_first_of( ' ' );  
        if ( firstSpace != string::npos) {
            key->assign( keyTmp.substr( 0, firstSpace ) );
        } else {
            key->assign( keyTmp ); 
        }
   
        // remove leading white space from value: 
        size_t firstNonSpace = valueTmp.find_first_not_of( ' ' );  
        if ( firstNonSpace != string::npos) {
            value->assign( valueTmp.substr( firstNonSpace ) );
        } else {
            value->assign( valueTmp ); 
        }

    } catch (const exception& e) {
        cout <<  e.what() << endl;
        return -1; 
    }

    return 0; 
}


/*!
 *  Utility function for extracting a substring with white space removed from LHS.
 */
string svkSdbmVolumeReader::ReadLineSubstr(istringstream* iss, int start, int stop)
{
    string temp;
    string lineSubStr;
    size_t firstNonSpace;
    this->ReadLine(this->shfHdr, iss);
    try {
        temp.assign(iss->str().substr(start,stop));
        firstNonSpace = temp.find_first_not_of(' ');
        if (firstNonSpace != string::npos) {
            lineSubStr.assign( temp.substr(firstNonSpace) );
        }
    } catch (const exception& e) {
        cout <<  e.what() << endl;
    }
    return lineSubStr;
}


/*!
 *
 */
int svkSdbmVolumeReader::GetHeaderValueAsInt(map <string, string> hdrMap, 
    string keyString, int valueIndex)
{
    istringstream* iss = new istringstream();
    int value;

    iss->str( hdrMap[keyString] );
    *iss >> value;
    delete iss; 
    return value;
}


/*!
 *
 */
float svkSdbmVolumeReader::GetHeaderValueAsFloat(map <string, string> hdrMap, 
    string keyString, int valueIndex)
{
    istringstream* iss = new istringstream();
    float value;
    iss->str( hdrMap[keyString] );
    *iss >> value;
    delete iss; 
    return value;
}

