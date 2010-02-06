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


#include <svkDdfVolumeReader.h>


using namespace svk;


vtkCxxRevisionMacro(svkDdfVolumeReader, "$Rev$");
vtkStandardNewMacro(svkDdfVolumeReader);


const string svkDdfVolumeReader::MFG_STRING = "GE MEDICAL SYSTEMS";


/*!
 *
 */
svkDdfVolumeReader::svkDdfVolumeReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkDdfVolumeReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->specData = NULL; 
    this->ddfHdr = NULL; 
    this->numCoils = 1; 
    this->numTimePts = 1; 
}



/*!
 *
 */
svkDdfVolumeReader::~svkDdfVolumeReader()
{

    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if ( this->ddfHdr != NULL )  {
        delete ddfHdr;
        this->ddfHdr = NULL;
    }
}



/*!
 *  Check to see if the extension indicates a UCSF IDF file.  If so, try
 *  to open the file for reading.  If that works, then return a success code.
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkDdfVolumeReader::CanReadFile(const char* fname)
{

    string fileToCheck(fname);

    if( fileToCheck.size() > 4 ) {

        // Also see "vtkImageReader2::GetFileExtensions method" 
        
        
        if ( 
            fileToCheck.substr( fileToCheck.size() - 6 ) == ".cmplx" || 
            fileToCheck.substr( fileToCheck.size() - 4 ) == ".ddf" 
        )  {
            FILE *fp = fopen(fname, "rb");
            if (fp) {
                fclose(fp);

                vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's a UCSF Complex File: " << fileToCheck);
                return 1;
            }
        } else {
            vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's NOT a UCSF Complex File: " << fileToCheck);
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
int svkDdfVolumeReader::GetNumVoxelsInVol()
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
int svkDdfVolumeReader::GetNumSlices()
{
    return (this->GetDataExtent())[5] + 1;
}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called before ExecuteData()
 */
void svkDdfVolumeReader::ExecuteInformation()
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
void svkDdfVolumeReader::ExecuteData(vtkDataObject* output)
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
        this->ReadComplexFile(data);
    }

    double dcos[3][3];
    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
    this->GetOutput()->SetDcos(dcos);

    //  SetNumberOfIncrements is supposed to call this, but only works if the data has already
    //  been allocated. but that requires the number of components to be specified.
    this->GetOutput()->GetIncrements();

}


/*!
 *
 */
void svkDdfVolumeReader::ReadComplexFile(vtkImageData* data)
{

    for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {

        int coilNum = 0; 
        if ( this->numCoils > 1 ) {
            coilNum = fileIndex;     
        }

        ifstream* cmplxDataIn = new ifstream();
        cmplxDataIn->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        string cmplxFile = this->GetFileRoot( this->GetFileNames()->GetValue( fileIndex ) ) + ".cmplx"; 

        cmplxDataIn->open( cmplxFile.c_str(), ios::binary);

        int numComponents =  this->GetHeaderValueAsInt( ddfMap, "numberOfComponents" ); 
        int numPts = this->GetHeaderValueAsInt(ddfMap, "dimensionNumberOfPoints0"); 
        int numBytesInVol = this->GetNumPixelsInVol() * numPts * numComponents * 4 ; 
        this->specData = new float[ numBytesInVol/4 ];  
        cmplxDataIn->read( (char*)(this->specData), numBytesInVol );


#if defined (linux) || defined(Darwin)
        svkByteSwap::SwapBufferEndianness( specData, this->GetNumPixelsInVol() * numPts * numComponents);
#endif

    
        for (int timePt = 0; timePt < this->numTimePts ; timePt++) {
            for (int z = 0; z < (this->GetDataExtent())[5] ; z++) {
                for (int y = 0; y < (this->GetDataExtent())[3]; y++) {
                    for (int x = 0; x < (this->GetDataExtent())[1]; x++) {
                        SetCellSpectrum(data, x, y, z, timePt, coilNum);
                    }
                }
            }
        }

        cmplxDataIn->close(); 
        delete cmplxDataIn;
        delete [] specData; 
    }
}


/*!
 *  Utility method returns the total number of Pixels in 3D.
 */
int svkDdfVolumeReader::GetNumPixelsInVol()
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
void svkDdfVolumeReader::SetCellSpectrum(vtkImageData* data, int x, int y, int z, int timePt, int coilNum)
{

    //  Set XY points to plot - 2 components per tuble for complex data sets:
    vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
    dataArray->SetNumberOfComponents(2);

    int numPts = this->GetHeaderValueAsInt(ddfMap, "dimensionNumberOfPoints0"); 
    int numComponents =  this->GetHeaderValueAsInt( ddfMap, "numberOfComponents" ); 
    dataArray->SetNumberOfTuples(numPts);
    char arrayName[30];

    sprintf(arrayName, "%d %d %d %d %d", x, y, z, timePt, coilNum);
    dataArray->SetName(arrayName);

    //  preallocate float array for spectrum and use to iniitialize the vtkDataArray:
    //  don't do this for each call
    int numVoxels[3]; 
    numVoxels[0] = this->GetHeaderValueAsInt(ddfMap, "dimensionNumberOfPoints1"); 
    numVoxels[1] = this->GetHeaderValueAsInt(ddfMap, "dimensionNumberOfPoints2"); 
    numVoxels[2] = this->GetHeaderValueAsInt(ddfMap, "dimensionNumberOfPoints3"); 

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
void svkDdfVolumeReader::InitDcmHeader()
{
    vtkDebugMacro( << this->GetClassName() << "::InitDcmHeader()" );

    svkIOD* iod = svkMRSIOD::New();
    iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    iod->InitDcmHeader();
    iod->Delete();

    this->ParseDdf(); 
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
}


/*!
 *  Read DDF header fields into a string STL map for use during initialization
 *  of DICOM header by Init*Module methods.
 */
void svkDdfVolumeReader::ParseDdf()
{

    string ddfFileName( this->GetFileName() );
    string ddfFileExtension( this->GetFileExtension( this->GetFileName() ) );
    string ddfFilePath( this->GetFilePath( this->GetFileName() ) );
    vtkGlobFileNames* globFileNames = vtkGlobFileNames::New();
    globFileNames->AddFileNames( string( ddfFilePath + "/*." + ddfFileExtension).c_str() );

    vtkSortFileNames* sortFileNames = vtkSortFileNames::New();
    sortFileNames->GroupingOn();
    sortFileNames->SetInputFileNames( globFileNames->GetFileNames() );
    sortFileNames->NumericSortOn();
    sortFileNames->Update();

    //  If globed file names are not similar, use only the 0th group. 
    if (sortFileNames->GetNumberOfGroups() > 1 ) {

        vtkWarningWithObjectMacro(this, "Found Multiple ddf file groups, using only specified file ");

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

        //  Read in the DDF Header:
        this->ddfHdr = new ifstream();
        this->ddfHdr->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        int fileIndex = 0; 
        string currentDdfFileName( this->GetFileNames()->GetValue( fileIndex ) );

        this->ddfHdr->open( currentDdfFileName.c_str(), ifstream::in );

        if ( ! this->ddfHdr->is_open() ) {
            throw runtime_error( "Could not open volume file: " + ddfFileName );
        }
        istringstream* iss = new istringstream();

        //  Skip first line: 
        this->ReadLine(iss);

        // DDF_VERSION
        int ddfVersion;
        this->ReadLine(iss);
        iss->ignore(29);
        *iss>>ddfVersion;

        ddfMap["objectType"] = this->ReadLineValue(iss, ':');
        ddfMap["patientId"] = this->ReadLineValue(iss, ':');
        ddfMap["patientName"] = this->ReadLineValue(iss, ':');
        ddfMap["patientCode"] = this->ReadLineValue(iss, ':');
        ddfMap["dateOfBirth"] = this->ReadLineValue(iss, ':');
        ddfMap["sex"] = this->ReadLineValue(iss, ':');
        ddfMap["studyId"] = this->ReadLineValue(iss, ':');
        ddfMap["studyCode"] = this->ReadLineValue(iss, ':');
        ddfMap["studyDate"] = this->ReadLineValue(iss, ':');
        ddfMap["accessionNumber"] = this->ReadLineValue(iss, ':');
        ddfMap["rootName"] = this->ReadLineValue(iss, ':');
        ddfMap["seriesNumber"] = this->ReadLineValue(iss, ':');
        ddfMap["seriesDescription"] = this->StripWhite( this->ReadLineValue(iss, ':') );
        ddfMap["comment"] = this->ReadLineValue(iss, ':');
        ddfMap["patientEntry"] = this->ReadLineValue(iss, ':');
        ddfMap["patientPosition"] = this->ReadLineValue(iss, ':');
        ddfMap["orientation"] = this->ReadLineValue(iss, ':');
        ddfMap["dataType"] = this->ReadLineValue(iss, ':');
        ddfMap["numberOfComponents"] = this->ReadLineValue(iss, ':');
        ddfMap["sourceDescription"] = this->ReadLineValue(iss, ':');
        ddfMap["numberOfDimensions"] = this->ReadLineValue(iss, ':');
 
        // DIMENSIONS AND SPACING
        int numDimensions = this->GetHeaderValueAsInt(ddfMap, "numberOfDimensions"); 

        for (int i = 0; i < numDimensions; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());

            //  Type
            string dimensionTypeString = "dimensionType" + indexString; 
            string dimensionLine( this->ReadLineSubstr(iss, 0, 100) );
            size_t start = dimensionLine.find( "type:" ); 
            string tmp   = dimensionLine.substr( start + 6 ); 
            size_t end   = tmp.find( "npoints" ); 
            ddfMap[dimensionTypeString] = dimensionLine.substr( start + 6, end - 1); 

            // npoints 
            string nptsString = "dimensionNumberOfPoints" + indexString; 
            start = dimensionLine.find( "npoints:" ); 
            start += 8; 
            end   = dimensionLine.find( "pixel" ); 
            ddfMap[nptsString] = dimensionLine.substr( start , end - start ); 

            //  PixelSpacing
            string pixelSpacingString = "pixelSpacing" + indexString; 
            start = dimensionLine.find( "(mm):" ); 
            if (start != string::npos) {
                ddfMap[pixelSpacingString] = dimensionLine.substr( start + 5 ); 
            }
        }

        //  CENTER(LPS):
        this->ReadLineIgnore( iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> ddfMap[string("centerLPS" + indexString)];
        }

        //  TOPLC(LPS):
        this->ReadLineIgnore( iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> ddfMap[string("toplcLPS" + indexString)];
        }

        //  DCOS:  
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndexI;
            ossIndexI << i;
            string indexStringI(ossIndexI.str());
            this->ReadLineIgnore( iss, 's' );
            for (int j = 0; j < 3; j++) {
                ostringstream ossIndexJ;
                ossIndexJ << j;
                string indexStringJ(ossIndexJ.str());
                iss->ignore(256, ' ');
                *iss >> ddfMap[string("dcos" + indexStringI + indexStringJ)];
            }
        }

        //MR Parameters 
        this->ReadLine(iss);
        this->ReadLine(iss);
        
        ddfMap["coilName"] = this->ReadLineValue(iss, ':');
        ddfMap["sliceGap"] = this->ReadLineValue(iss, ':');
        ddfMap["echoTime"] = this->ReadLineValue(iss, ':');
        ddfMap["repetitionTime"] = this->ReadLineValue(iss, ':');
        ddfMap["inversionTime"] = this->ReadLineValue(iss, ':');
        ddfMap["flipAngle"] = this->ReadLineValue(iss, ':');
        ddfMap["pulseSequenceName"] = this->ReadLineValue(iss, ':');
        ddfMap["transmitterFrequency"] = this->ReadLineValue(iss, ':');
        ddfMap["isotope"] = this->ReadLineValue(iss, ':');
        ddfMap["fieldStrength"] = this->ReadLineValue(iss, ':');

        ddfMap["numberOfSatBands"] = this->ReadLineValue(iss, ':');
        for (int i = 0; i < this->GetHeaderValueAsInt( ddfMap, "numberOfSatBands" ); i++) {

            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            ddfMap["satBand" + indexString + "Thickness"] = this->ReadLineValue(iss, ':');

            //  Orientation:
            this->ReadLineIgnore( iss, 'r' );
            for (int j = 0; j < 3; j++) {
                ostringstream ossIndexJ;
                ossIndexJ << j;
                string indexStringJ(ossIndexJ.str());
                iss->ignore(256, ' ');
                *iss >> ddfMap[string("satBand" + indexString + "Orientation" + indexStringJ)];
            }

            //  position:
            this->ReadLineIgnore( iss, ')' );
            for (int j = 0; j < 3; j++) {
                ostringstream ossIndexJ;
                ossIndexJ << j;
                string indexStringJ(ossIndexJ.str());
                iss->ignore(256, ' ');
                *iss >> ddfMap[string("satBand" + indexString + "PositionLPS" + indexStringJ)];
            }

        }

        //Spectroscopy Parameters 
        this->ReadLine(iss);
        this->ReadLine(iss);
        ddfMap["localizationType"] = this->ReadLineValue(iss, ':');
        ddfMap["centerFrequency"] = this->ReadLineValue(iss, ':');
        ddfMap["ppmReference"] = this->ReadLineValue(iss, ':');
        ddfMap["sweepwidth"] = this->ReadLineValue(iss, ':');
        ddfMap["dwelltime"] = this->ReadLineValue(iss, ':');
        ddfMap["frequencyOffset"] = this->ReadLineValue(iss, ':');
        ddfMap["centeredOnWater"] = this->ReadLineValue(iss, ':');
        ddfMap["suppressionTechnique"] = this->ReadLineValue(iss, ':');
        ddfMap["residualWater"] = this->ReadLineValue(iss, ':');
        ddfMap["numberOfAcquisitions"] = this->ReadLineValue(iss, ':');
        ddfMap["chop"] = this->ReadLineValue(iss, ':');
        ddfMap["evenSymmetry"] = this->ReadLineValue(iss, ':');
        ddfMap["dataReordered"] = this->ReadLineValue(iss, ':');

        //  ACQ TOPLC(LPS):
        this->ReadLineIgnore( iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> ddfMap[string("acqToplcLPS" + indexString)];
        }

        //  ACQ Spacing:
        this->ReadLineIgnore( iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> ddfMap[string("acqSpacing" + indexString)];
        }

        ddfMap["acqNumberOfDataPoints"] = this->ReadLineValue(iss, ':');

        //  Acq Num Data Pts:
        this->ReadLineIgnore( iss, 's' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> ddfMap[string("acqNumberOfPoints" + indexString)];
        }

        //  ACQ DCOS:  
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndexI;
            ossIndexI << i;
            string indexStringI(ossIndexI.str());
            this->ReadLineIgnore( iss, 's' );
            for (int j = 0; j < 3; j++) {
                ostringstream ossIndexJ;
                ossIndexJ << j;
                string indexStringJ(ossIndexJ.str());
                iss->ignore(256, ' ');
                *iss >> ddfMap[string("acqDcos" + indexStringI + indexStringJ)];
            }
        }

        //  Selection Center:
        this->ReadLineIgnore( iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> ddfMap[string("selectionCenterLPS" + indexString)];
        }

        //  Selection Size:
        this->ReadLineIgnore( iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> ddfMap[string("selectionSize" + indexString)];
        }

        //  Selection DCOS:  
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndexI;
            ossIndexI << i;
            string indexStringI(ossIndexI.str());
            this->ReadLineIgnore( iss, 'd' );
            for (int j = 0; j < 3; j++) {
                ostringstream ossIndexJ;
                ossIndexJ << j;
                string indexStringJ(ossIndexJ.str());
                iss->ignore(256, ' ');
                *iss >> ddfMap[string("selectionDcos" + indexStringI + indexStringJ)];
            }
        }

        //  Reordered :
        this->ReadLineIgnore( iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> ddfMap[string("reorderedToplcLPS" + indexString)];
        }

        //  Reordered:
        this->ReadLineIgnore( iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> ddfMap[string("reorderedCenterLPS" + indexString)];
        }

        //  Reordered:
        this->ReadLineIgnore( iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> ddfMap[string("reorderedSpacing" + indexString)];
        }

        //  Reordered:
        this->ReadLineIgnore( iss, 's' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> ddfMap[string("reorderedNumberOfPoints" + indexString)];
        }

        //  Selection DCOS:  
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndexI;
            ossIndexI << i;
            string indexStringI(ossIndexI.str());
            this->ReadLineIgnore( iss, 's' );
            for (int j = 0; j < 3; j++) {
                ostringstream ossIndexJ;
                ossIndexJ << j;
                string indexStringJ(ossIndexJ.str());
                iss->ignore(256, ' ');
                *iss >> ddfMap[string("reorderedDcos" + indexStringI + indexStringJ)];
            }
        }

        delete iss;

    } catch (const exception& e) {
        cerr << "ERROR opening or reading ddf file (" << ddfFileName << "): " << e.what() << endl;
    }
    sortFileNames->Delete();
    globFileNames->Delete();

}



/*!
 *  Initializes the VolumeLocalizationSequence in the MRSpectroscopy
 *  DICOM object for PRESS excitation.  
 */
void svkDdfVolumeReader::InitVolumeLocalizationSeq()
{

    this->GetOutput()->GetDcmHeader()->InsertEmptyElement( "VolumeLocalizationSequence" );

    //  Get Thickness Values
    float selBoxSize[3]; 
    selBoxSize[0] = this->GetHeaderValueAsFloat(ddfMap, "selectionSize0"); 
    selBoxSize[1] = this->GetHeaderValueAsFloat(ddfMap, "selectionSize1"); 
    selBoxSize[2] = this->GetHeaderValueAsFloat(ddfMap, "selectionSize2"); 

    //  Get Center Location Values
    float selBoxCenter[3]; 
    selBoxCenter[0] = this->GetHeaderValueAsFloat(ddfMap, "selectionCenterLPS0"); 
    selBoxCenter[1] = this->GetHeaderValueAsFloat(ddfMap, "selectionCenterLPS1"); 
    selBoxCenter[2] = this->GetHeaderValueAsFloat(ddfMap, "selectionCenterLPS2"); 

    string midSlabPosition;
    for (int i = 0; i < 3; i++) {
        ostringstream oss;
        oss << selBoxCenter[i];
        midSlabPosition += oss.str();
        if (i < 2) {
            midSlabPosition += '\\';
        }
    }

    //  Get Orientation Values 
    float selBoxOrientation[3][3]; 
    selBoxOrientation[0][0] = this->GetHeaderValueAsFloat(ddfMap, "selectionDcos00"); 
    selBoxOrientation[0][1] = this->GetHeaderValueAsFloat(ddfMap, "selectionDcos01"); 
    selBoxOrientation[0][2] = this->GetHeaderValueAsFloat(ddfMap, "selectionDcos02"); 
    selBoxOrientation[1][0] = this->GetHeaderValueAsFloat(ddfMap, "selectionDcos10"); 
    selBoxOrientation[1][1] = this->GetHeaderValueAsFloat(ddfMap, "selectionDcos11"); 
    selBoxOrientation[1][2] = this->GetHeaderValueAsFloat(ddfMap, "selectionDcos12"); 
    selBoxOrientation[2][0] = this->GetHeaderValueAsFloat(ddfMap, "selectionDcos20"); 
    selBoxOrientation[2][1] = this->GetHeaderValueAsFloat(ddfMap, "selectionDcos21"); 
    selBoxOrientation[2][2] = this->GetHeaderValueAsFloat(ddfMap, "selectionDcos22"); 

    //  Volume Localization (PRESS BOX)
    for (int i = 0; i < 3; i++) {

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "VolumeLocalizationSequence", 
            i,
            "SlabThickness",
            selBoxSize[i] 
        );

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "VolumeLocalizationSequence", 
            i,
            "MidSlabPosition",
            midSlabPosition
        );


        string slabOrientation;
        for (int j = 0; j < 3; j++) {
            ostringstream oss;
            oss << selBoxOrientation[i][j];
            slabOrientation += oss.str();
            if (j < 2) {
                slabOrientation += '\\';
            }
        }

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "VolumeLocalizationSequence", 
            i,
            "SlabOrientation",
            slabOrientation 
        );

    }

}


/*!
 *
 */
void svkDdfVolumeReader::InitPatientModule() 
{
    this->GetOutput()->GetDcmHeader()->SetDcmPatientsName( 
            ddfMap["patientName"] 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "PatientID", 
        ddfMap["patientId"] 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "PatientsBirthDate", 
        ddfMap["dateOfBirth"] 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "PatientsSex", 
        ddfMap["sex"] 
    );
}


/*!
 *
 */
void svkDdfVolumeReader::InitGeneralStudyModule() 
{
    this->GetOutput()->GetDcmHeader()->SetValue(
        "StudyDate", 
        this->RemoveSlashesFromDate( &(ddfMap["studyDate"]) )
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "StudyID", 
        ddfMap["studyId"]
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "AccessionNumber", 
        ddfMap["accessionNumber"]
    );
}


/*!
 *
 */
void svkDdfVolumeReader::InitGeneralSeriesModule() 
{

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SeriesNumber", 
        ddfMap["seriesNumber"] 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SeriesDescription", 
        ddfMap["seriesDescription"] 
    );

    string patientEntryPos; 
    string patientEntry( ddfMap["patientEntry"]); 
    if ( patientEntry.compare("head first") == 0) {
        patientEntryPos = "HF";
    } else if ( patientEntry.compare("feet first") == 0) {
        patientEntryPos = "FF";
    }

    string patientPosition( ddfMap["patientPosition"]); 
    if ( patientPosition.compare("supine") == 0 ) {
        patientEntryPos.append("S");
    } else if ( patientPosition.compare("prone") == 0 ) {
        patientEntryPos.append("P");
    }

    this->GetOutput()->GetDcmHeader()->SetValue(
        "PatientPosition", 
        patientEntryPos 
    );

}


/*!
 *  DDF is historically the UCSF representation of a GE raw file so 
 *  initialize to svkDdfVolumeReader::MFG_STRING.
 */
void svkDdfVolumeReader::InitGeneralEquipmentModule()
{
    this->GetOutput()->GetDcmHeader()->SetValue(
        "Manufacturer", 
        svkDdfVolumeReader::MFG_STRING 
    );
}


/*!
 *
 */
void svkDdfVolumeReader::InitMultiFrameFunctionalGroupsModule()
{
    InitSharedFunctionalGroupMacros();

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "InstanceNumber", 
        1 
    );

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "ContentDate", 
        this->RemoveSlashesFromDate( &(ddfMap["studyDate"]) ) 
    );

    int numVoxels[3];
    this->numSlices = this->GetHeaderValueAsInt( ddfMap, "dimensionNumberOfPoints3" ); 

    if ( this->GetHeaderValueAsInt( ddfMap, "numberOfDimensions" ) == 5 ) {
        this->numTimePts = this->GetHeaderValueAsInt( ddfMap, "dimensionNumberOfPoints4" ); 
    }

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "NumberOfFrames", 
        this->numSlices * this->numCoils * this->numTimePts
    );

    InitPerFrameFunctionalGroupMacros();
}

/*!
 *
 */
void svkDdfVolumeReader::InitSharedFunctionalGroupMacros()
{

    this->InitPixelMeasuresMacro();
    this->InitPlaneOrientationMacro();
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
}


/*!
 *
 */
void svkDdfVolumeReader::InitPerFrameFunctionalGroupMacros()
{
    this->InitFrameContentMacro();
    this->InitPlanePositionMacro();
}


/*!
 *  Pixel Spacing:
 */
void svkDdfVolumeReader::InitPixelMeasuresMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "PixelMeasuresSequence"
    );

    //  get the spacing for the specified index:
    float colSpacing = this->GetHeaderValueAsFloat(ddfMap, "pixelSpacing1"); 
    float rowSpacing = this->GetHeaderValueAsFloat(ddfMap, "pixelSpacing2"); 
    float sliceThickness = this->GetHeaderValueAsFloat(ddfMap, "pixelSpacing3"); 

    string pixelSpacing;
    ostringstream oss;
    oss << colSpacing;
    oss << '\\';
    oss << rowSpacing;
    pixelSpacing = oss.str();

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PixelMeasuresSequence",            
        0,                                 
        "PixelSpacing",                   
        pixelSpacing,                    
        "SharedFunctionalGroupsSequence",   
        0                                  
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PixelMeasuresSequence",          
        0,                               
        "SliceThickness",               
        sliceThickness, 
        "SharedFunctionalGroupsSequence",   
        0                                 
    );
}


/*!
 *  Mandatory, Must be a per-frame functional group
 */
void svkDdfVolumeReader::InitFrameContentMacro()
{

    int numFrameIndices = svkDcmHeader::GetNumberOfDimensionIndices( this->numTimePts, this->numCoils ) ;

    unsigned int* indexValues = new unsigned int[numFrameIndices]; 

    int frame = 0; 

    for (int coilNum = 0; coilNum < numCoils; coilNum++) {

        for (int timePt = 0; timePt < numTimePts; timePt++) {

            for (int sliceNum = 0; sliceNum < numSlices; sliceNum++) {

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
                    2,                  // num values in array
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
 *
 */
void svkDdfVolumeReader::InitPlanePositionMacro()
{


    //  Get toplc float array from ddfMap and use that to generate
    //  frame locations.  This position is off by 1/2 voxel, fixed below:
    float toplc[3];
    toplc[0] = this->GetHeaderValueAsFloat(ddfMap, "toplcLPS0"); 
    toplc[1] = this->GetHeaderValueAsFloat(ddfMap, "toplcLPS1"); 
    toplc[2] = this->GetHeaderValueAsFloat(ddfMap, "toplcLPS2"); 
        
    float dcos[3][3];
    dcos[0][0] = this->GetHeaderValueAsFloat(ddfMap, "dcos00"); 
    dcos[0][1] = this->GetHeaderValueAsFloat(ddfMap, "dcos01"); 
    dcos[0][2] = this->GetHeaderValueAsFloat(ddfMap, "dcos02"); 
    dcos[1][0] = this->GetHeaderValueAsFloat(ddfMap, "dcos10"); 
    dcos[1][1] = this->GetHeaderValueAsFloat(ddfMap, "dcos11"); 
    dcos[1][2] = this->GetHeaderValueAsFloat(ddfMap, "dcos12"); 
    dcos[2][0] = this->GetHeaderValueAsFloat(ddfMap, "dcos20"); 
    dcos[2][1] = this->GetHeaderValueAsFloat(ddfMap, "dcos21"); 
    dcos[2][2] = this->GetHeaderValueAsFloat(ddfMap, "dcos22"); 
    
    float pixelSize[3];
    pixelSize[0] = this->GetHeaderValueAsFloat(ddfMap, "pixelSpacing1"); 
    pixelSize[1] = this->GetHeaderValueAsFloat(ddfMap, "pixelSpacing2"); 
    pixelSize[2] = this->GetHeaderValueAsFloat(ddfMap, "pixelSpacing3"); 

    float displacement[3]; 
    float frameLPSPosition[3]; 

    int frame = 0; 

    for (int coilNum = 0; coilNum < this->numCoils; coilNum++) {

        for (int timePt = 0; timePt < this->numTimePts; timePt++) {
    
            for (int sliceNum = 0; sliceNum < this->numSlices; sliceNum++) {
    
                this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
                    "PerFrameFunctionalGroupsSequence",
                    sliceNum, 
                    "PlanePositionSequence"
                );
    
                //add displacement along normal vector:
                for (int j = 0; j < 3; j++) {
                    displacement[j] = dcos[2][j] * pixelSize[2] * sliceNum;
                }
                for(int j = 0; j < 3; j++) { //L, P, S
                    frameLPSPosition[j] = toplc[j] +  displacement[j] ;
                }
    
                string imagePositionPatient;
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
                    frame 
                );
            
                frame++;     
            }
        }
    }
}


/*!
 *
 */
void svkDdfVolumeReader::InitPlaneOrientationMacro()
{
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "PlaneOrientationSequence"
    );

    float dcos[3][3]; 
    dcos[0][0] = this->GetHeaderValueAsFloat(ddfMap, "dcos00"); 
    dcos[0][1] = this->GetHeaderValueAsFloat(ddfMap, "dcos01"); 
    dcos[0][2] = this->GetHeaderValueAsFloat(ddfMap, "dcos02"); 
    dcos[1][0] = this->GetHeaderValueAsFloat(ddfMap, "dcos10"); 
    dcos[1][1] = this->GetHeaderValueAsFloat(ddfMap, "dcos11"); 
    dcos[1][2] = this->GetHeaderValueAsFloat(ddfMap, "dcos12"); 
    dcos[2][0] = this->GetHeaderValueAsFloat(ddfMap, "dcos20"); 
    dcos[2][1] = this->GetHeaderValueAsFloat(ddfMap, "dcos21"); 
    dcos[2][2] = this->GetHeaderValueAsFloat(ddfMap, "dcos22"); 

    ostringstream ossDcos;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            ossDcos << dcos[i][j];
            if (i * j != 2) {
                ossDcos<< "\\";
            }
        }
    }

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
    this->GetOutput()->GetDcmHeader()->SetSliceOrder( this->dataSliceOrder );
    math->Delete();

}





/*!
 *
 */
void svkDdfVolumeReader::InitFrameAnatomyMacro()
{
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "FrameAnatomySequence"
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "FrameAnatomySequence",       
        0,                             
        "FrameLaterality",              
        string("U"),                     
        "SharedFunctionalGroupsSequence", 
        0                                  
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "FrameAnatomySequence",      
        0,                          
        "AnatomicRegionSequence"   
    );


    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "AnatomicRegionSequence",       
        0,                             
        "CodeValue",              
        1,
        "FrameAnatomySequence", 
        0                                  
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "AnatomicRegionSequence",       
        0,                             
        "CodingSchemeDesignator",              
        0,
        "FrameAnatomySequence", 
        0                                  
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "AnatomicRegionSequence",       
        0,                             
        "CodeMeaning",              
        0,
        "FrameAnatomySequence", 
        0                                  
    );

}


/*!
 *
 */
void svkDdfVolumeReader::InitMRSpectroscopyFrameTypeMacro()
{
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRSpectroscopyFrameTypeSequence"
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFrameTypeSequence",
        0,                               
        "FrameType",                    
        string("ORIGINAL\\PRIMARY\\SPECTROSCOPY\\NONE"),  
        "SharedFunctionalGroupsSequence",   
        0                                 
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFrameTypeSequence",  
        0,                                 
        "VolumetricProperties",           
        string("VOLUME"),  
        "SharedFunctionalGroupsSequence",
        0                              
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFrameTypeSequence",  
        0,                                 
        "VolumeBasedCalculationTechnique",
        string("NONE"),
        "SharedFunctionalGroupsSequence",
        0                              
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFrameTypeSequence",
        0,                                
        "ComplexImageComponent",           
        string("COMPLEX"),  
        "SharedFunctionalGroupsSequence",   
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFrameTypeSequence",  
        0,                                 
        "AcquisitionContrast",            
        "UNKNOWN",
        "SharedFunctionalGroupsSequence",   
        0                                 
    );
}


/*!
 *
 */
void svkDdfVolumeReader::InitMRTimingAndRelatedParametersMacro()
{
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRTimingAndRelatedParametersSequence"
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence", 
        0,                                
        "RepetitionTime",                     
        this->GetHeaderValueAsFloat(ddfMap, "repetitionTime"), 
        "SharedFunctionalGroupsSequence",  
        0 
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence", 
        0,                                
        "FlipAngle",                     
        this->GetHeaderValueAsFloat(ddfMap, "flipAngle"), 
        "SharedFunctionalGroupsSequence",  
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence", 
        0,                                
        "EchoTrainLength",                     
        1,
        "SharedFunctionalGroupsSequence",  
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence", 
        0,                                
        "RFEchoTrainLength",                     
        1,
        "SharedFunctionalGroupsSequence",  
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence", 
        0,                                
        "GradientEchoTrainLength",                     
        1,
        "SharedFunctionalGroupsSequence",  
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence", 
        0,                                
        "GradientEchoTrainLength",                     
        1,
        "SharedFunctionalGroupsSequence",  
        0                                  
    );
}


/*!
 *
 */
void svkDdfVolumeReader::InitMRSpectroscopyFOVGeometryMacro()
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
        this->GetHeaderValueAsInt( ddfMap, "acqNumberOfDataPoints"),
        "SharedFunctionalGroupsSequence",      
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SpectroscopyAcquisitionPhaseColumns",
        this->GetHeaderValueAsInt( ddfMap, "acqNumberOfPoints0"),
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SpectroscopyAcquisitionPhaseRows",
        this->GetHeaderValueAsInt( ddfMap, "acqNumberOfPoints1"),
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SpectroscopyAcquisitionOutOfPlanePhaseSteps",
        this->GetHeaderValueAsInt( ddfMap, "acqNumberOfPoints2"),
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcquisitionTLC",
        ddfMap["acqToplcLPS0"] + '\\' + ddfMap["acqToplcLPS1"] + '\\' + ddfMap["acqToplcLPS2"],
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcquisitionPixelSpacing",
        ddfMap["acqSpacing0"] + '\\' + ddfMap["acqSpacing1"], 
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcquisitionSliceThickness",
        ddfMap["acqSpacing2"], 
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcquisitionOrientation",
        ddfMap["acqDcos00"] + '\\' + ddfMap["acqDcos01"] + '\\' + ddfMap["acqDcos02"] + '\\' + 
        ddfMap["acqDcos10"] + '\\' + ddfMap["acqDcos11"] + '\\' + ddfMap["acqDcos12"] + '\\' + 
        ddfMap["acqDcos20"] + '\\' + ddfMap["acqDcos21"] + '\\' + ddfMap["acqDcos22"], 
        "SharedFunctionalGroupsSequence",    
        0
    );


    // ==================================================
    //  Reordered Params
    // ==================================================
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcqReorderedPhaseRows",
        this->GetHeaderValueAsInt( ddfMap, "reorderedNumberOfPoints0" ), 
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcqReorderedPhaseColumns",
        this->GetHeaderValueAsInt( ddfMap, "reorderedNumberOfPoints1" ), 
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcqReorderedOutOfPlanePhaseSteps",
        this->GetHeaderValueAsInt( ddfMap, "reorderedNumberOfPoints2"), 
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcqReorderedTLC",
        ddfMap["reorderedToplcLPS0"] + '\\' + ddfMap["reorderedToplcLPS1"] + '\\' + ddfMap["reorderedToplcLPS2"],
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcqReorderedPixelSpacing",
        ddfMap["reorderedSpacing0"] + '\\' + ddfMap["reorderedSpacing1"],
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcqReorderedSliceThickness",
        ddfMap["reorderedSpacing2"], 
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcqReorderedOrientation",
        ddfMap["reorderedDcos00"] + '\\' + ddfMap["reorderedDcos01"] + '\\' + ddfMap["reorderedDcos02"] + '\\' + 
        ddfMap["reorderedDcos10"] + '\\' + ddfMap["reorderedDcos11"] + '\\' + ddfMap["reorderedDcos12"] + '\\' + 
        ddfMap["reorderedDcos20"] + '\\' + ddfMap["reorderedDcos21"] + '\\' + ddfMap["reorderedDcos22"], 
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
void svkDdfVolumeReader::InitMREchoMacro()
{
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MREchoSequence"
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MREchoSequence",        
        0,                        
        "EffectiveEchoTime",       
        this->GetHeaderValueAsFloat(ddfMap, "echoTime"), 
        "SharedFunctionalGroupsSequence",    
        0
    );
}


/*!
 *
 */
void svkDdfVolumeReader::InitMRModifierMacro()
{
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRModifierSequence"
    );

    string invRecov = "NO"; 
    float inversionTime = this->GetHeaderValueAsFloat( ddfMap, "inversionTime");
    if ( inversionTime >= 0 ) { 
        invRecov.assign("YES"); 
        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "MRModifierSequence",
            0,                        
            "InversionTimes",       
            inversionTime,
            "SharedFunctionalGroupsSequence",    
            0                      
        );
    } 

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRModifierSequence",
        0,                        
        "InversionRecovery",       
        invRecov,
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRModifierSequence",
        0,                        
        "SpatialPreSaturation",       
        string("SLAB"),
        "SharedFunctionalGroupsSequence",    
        0                      
    );

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
void svkDdfVolumeReader::InitMRReceiveCoilMacro()
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
        ddfMap["coilName"], 
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
    if ( this->IsMultiCoil() ) {
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
        this->numCoils = this->GetFileNames()->GetNumberOfValues();

        for (int coilIndex = 0; coilIndex < this->GetFileNames()->GetNumberOfValues(); coilIndex++) {

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
void svkDdfVolumeReader::InitMRTransmitCoilMacro()
{
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRTransmitCoilSequence"
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRTransmitCoilSequence",
        0,                        
        "TransmitCoilName",       
        "coil name",
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRTransmitCoilSequence",
        0,                        
        "TransmitCoilManufacturerName",       
        "GE",
        "SharedFunctionalGroupsSequence",    
        0                      
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRTransmitCoilSequence",
        0,                        
        "TransmitCoilType",       
        "BODY",
        "SharedFunctionalGroupsSequence",    
        0                      
    );

}


/*!
 *
 */
void svkDdfVolumeReader::InitMRAveragesMacro()
{
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRAveragesSequence"
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRAveragesSequence",
        0,                        
        "NumberOfAverages",       
        "1",
        "SharedFunctionalGroupsSequence",    
        0
    );

}


/*!
 *
 */
void svkDdfVolumeReader::InitMRSpatialSaturationMacro()
{

    int numSatBands =  this->GetHeaderValueAsInt( ddfMap, "numberOfSatBands" ); 

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "MRSpatialSaturationSequence"
    );

    if (numSatBands > 0) {

        for (int i=0; i < numSatBands; i++) {

            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());

            this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
                "MRSpatialSaturationSequence",
                i,                        
                "SlabThickness",       
                this->GetHeaderValueAsFloat(ddfMap, "satBand" + indexString + "Thickness"), 
                "SharedFunctionalGroupsSequence",    
                0
            );
        
            float orientation[3];
            orientation[0] = this->GetHeaderValueAsFloat(ddfMap, "satBand" + indexString + "Orientation0");     
            orientation[1] = this->GetHeaderValueAsFloat(ddfMap, "satBand" + indexString + "Orientation1");     
            orientation[2] = this->GetHeaderValueAsFloat(ddfMap, "satBand" + indexString + "Orientation2");     

            string slabOrientation;
            for (int j = 0; j < 3; j++) {
                ostringstream ossOrientation;
                ossOrientation << orientation[j];
                slabOrientation += ossOrientation.str();
                if (j < 2) {
                    slabOrientation += '\\';
                }
            }
            this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
                "MRSpatialSaturationSequence",
                i,                        
                "SlabOrientation",       
                slabOrientation,
                "SharedFunctionalGroupsSequence",    
                0                      
            );
        
            float position[3];
            position[0] = this->GetHeaderValueAsFloat(ddfMap, "satBand" + indexString + "PositionLPS0");     
            position[1] = this->GetHeaderValueAsFloat(ddfMap, "satBand" + indexString + "PositionLPS1");     
            position[2] = this->GetHeaderValueAsFloat(ddfMap, "satBand" + indexString + "PositionLPS2");     

            string slabPosition;
            for (int j = 0; j < 3; j++) {
                ostringstream ossPosition;
                ossPosition << position[j];
                slabPosition += ossPosition.str();
                if (j < 2) {
                    slabPosition += '\\';
                }
            }
            this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
                "MRSpatialSaturationSequence",
                i,                        
                "MidSlabPosition",       
                slabPosition,
                "SharedFunctionalGroupsSequence",    
                0                      
            );

        }
    }
}


/*!
 *
 */
void svkDdfVolumeReader::InitMRSpatialVelocityEncodingMacro()
{
}


/*!
 *
 */
void svkDdfVolumeReader::InitMultiFrameDimensionModule()
{

    int indexCount = 0;

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "DimensionIndexSequence",
        indexCount,
        "DimensionDescriptionLabel",
        "Slice"
    );

    if (this->numTimePts > 1) {
        indexCount++;
        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "DimensionIndexSequence",
            indexCount,
            "DimensionDescriptionLabel",
            "Time Point"
        );
    }

    if (this->numCoils > 1) {
        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "DimensionIndexSequence",
            indexCount,
            "DimensionDescriptionLabel",
            "Coil Number"
        );
    }

/*
    if ( this->IsMultiCoil() ) {
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
void svkDdfVolumeReader::InitAcquisitionContextModule()
{
}


/*!
 *
 */
void svkDdfVolumeReader::InitMRSpectroscopyModule()
{

    /*  =======================================
     *  MR Image and Spectroscopy Instance Macro
     *  ======================================= */

    this->GetOutput()->GetDcmHeader()->SetValue(
        "AcquisitionDatetime",
        this->RemoveSlashesFromDate( &(ddfMap["studyDate"]) ) + "000000"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "AcquisitionDuration",
        0
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ResonantNucleus", 
        ddfMap["isotope"]
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "KSpaceFiltering", 
        "NONE" 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ApplicableSafetyStandardAgency", 
        "FDA" 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "MagneticFieldStrength", 
        this->GetHeaderValueAsFloat(ddfMap, "fieldStrength")
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
        this->GetHeaderValueAsFloat(ddfMap, "transmitterFrequency")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SpectralWidth", 
        this->GetHeaderValueAsFloat(ddfMap, "sweepwidth")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_FrequencyOffset",
        this->GetHeaderValueAsFloat( ddfMap, "frequencyOffset" )
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ChemicalShiftReference", 
        this->GetHeaderValueAsFloat( ddfMap, "ppmReference" )
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "VolumeLocalizationTechnique", 
        ddfMap["localizationType"]
    );

    if ( strcmp(ddfMap["localizationType"].c_str(), "PRESS") == 0)  { 
        this->InitVolumeLocalizationSeq();
    }

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
void svkDdfVolumeReader::InitMRSpectroscopyPulseSequenceModule()
{
    this->GetOutput()->GetDcmHeader()->SetValue(
        "PulseSequenceName", 
        ddfMap["pulseSequenceName"]
    );

    int numVoxels[3];
    numVoxels[0] = this->GetHeaderValueAsInt(ddfMap, "dimensionNumberOfPoints1"); 
    numVoxels[1] = this->GetHeaderValueAsInt(ddfMap, "dimensionNumberOfPoints2"); 
    numVoxels[2] = this->GetHeaderValueAsInt(ddfMap, "dimensionNumberOfPoints3"); 

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
        ddfMap["suppressionTechnique"]
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
void svkDdfVolumeReader::InitMRSpectroscopyDataModule()
{
    int numVoxels[3];
    numVoxels[0] = this->GetHeaderValueAsInt(ddfMap, "dimensionNumberOfPoints1"); 
    numVoxels[1] = this->GetHeaderValueAsInt(ddfMap, "dimensionNumberOfPoints2"); 
    numVoxels[2] = this->GetHeaderValueAsInt(ddfMap, "dimensionNumberOfPoints3"); 

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
        this->GetHeaderValueAsInt(ddfMap, "dimensionNumberOfPoints0") 
    );

    int numComponents =  this->GetHeaderValueAsInt( ddfMap, "numberOfComponents" ); 
    string representation; 
    if (numComponents == 1) {
        representation = "REAL";
    } else if (numComponents == 2) {
        representation = "COMPLEX";
    }

    this->GetOutput()->GetDcmHeader()->SetValue(
        "DataRepresentation", 
        representation 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SignalDomainColumns", 
        this->GetDimensionDomain( ddfMap["dimensionType0"] )
    );


    //  Private Attributes for spatial domain encoding:
    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_ColsDomain", 
        this->GetDimensionDomain( ddfMap["dimensionType1"] )
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_RowsDomain", 
        this->GetDimensionDomain( ddfMap["dimensionType2"] )
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_SliceDomain", 
        this->GetDimensionDomain( ddfMap["dimensionType3"] )
    );

}


/*!
 *  Returns the file root without extension
 */
svkDcmHeader::DcmPixelDataFormat svkDdfVolumeReader::GetFileType()
{
    return svkDcmHeader::SIGNED_FLOAT_4;
}


/*! 
 *  Converts the ddf dimension type to a string for DICOM domain tags: 
 */
string svkDdfVolumeReader::GetDimensionDomain( string ddfDomainString )
{
    string domain;  
    if ( ddfDomainString.compare("time") == 0 )  { 
        domain.assign("TIME"); 
    } else if ( ddfDomainString.compare("frequency") == 0 )  { 
        domain.assign("FREQUENCY"); 
    } else if ( ddfDomainString.compare("space") == 0 )  { 
        domain.assign("SPACE"); 
    } else if ( ddfDomainString.compare("kspace") == 0 )  { 
        domain.assign("KSPACE"); 
    }
    return domain; 
}


/*!
 *
 */
int svkDdfVolumeReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}


/*!
 *  Prints the key value pairs parsed from the header.
 */
void svkDdfVolumeReader::PrintKeyValuePairs()
{
    if (this->GetDebug()) {
        map< string, string >::iterator mapIter;
        for ( mapIter = ddfMap.begin(); mapIter != ddfMap.end(); ++mapIter ) {
            cout << this->GetClassName() << " " << mapIter->first << " = ";
            cout << ddfMap[mapIter->first] << endl;
        }
    }
}


string svkDdfVolumeReader::ReadLineIgnore(istringstream* iss, char delim)
{
    this->ReadLine(iss);
    iss->ignore(256, delim);
    string value;
    *iss >> value; 
    return value; 
}


/*! 
 *  Read the value part of a delimited key value line in a file: 
 */
string svkDdfVolumeReader::ReadLineValue(istringstream* iss, char delim)
{

    string value;
    this->ReadLine(iss);
    try {

        string line;
        line.assign( iss->str() );

        size_t delimPos = line.find_first_of(delim);
        string delimitedLine; 
        if (delimPos != string::npos) {
            delimitedLine.assign( line.substr( delimPos + 1 ) );
        } else {
            delimitedLine.assign( line );
        }
   
        // remove leading white space: 
        size_t firstNonSpace = delimitedLine.find_first_not_of( ' ' );  
        if ( firstNonSpace != string::npos) {
            value.assign( delimitedLine.substr( firstNonSpace ) );
        } else {
            value.assign( delimitedLine ); 
        }

    } catch (const exception& e) {
        cout <<  e.what() << endl;
    }

    return value;

}




/*!
 *  Utility function to read a single line from the volume file.
 */
void svkDdfVolumeReader::ReadLine(istringstream* iss)
{
    char line[256];
    iss->clear();
    this->ddfHdr->getline(line, 256);
    iss->str(string(line));
}


/*!
 *  Utility function for extracting a substring with white space removed from LHS.
 */
string svkDdfVolumeReader::ReadLineSubstr(istringstream* iss, int start, int stop)
{
    string temp;
    string lineSubStr;
    size_t firstNonSpace;
    this->ReadLine(iss);
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
int svkDdfVolumeReader::GetHeaderValueAsInt(map <string, string> hdrMap, string keyString, int valueIndex)
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
float svkDdfVolumeReader::GetHeaderValueAsFloat(map <string, string> hdrMap, string keyString, int valueIndex)
{
    istringstream* iss = new istringstream();
    float value;
    iss->str( hdrMap[keyString] );
    *iss >> value;
    delete iss; 
    return value;
}


/*!
 *  Determine whether the data is multi-coil, based on number of files and coil name:
 */
bool svkDdfVolumeReader::IsMultiCoil()
{
    bool isMultiCoil = false; 

    //if ( ddfMap["coilName"].find("8HRBRAIN") != string::npos || ddfMap["coilName"].find("HDBreastRight") != string::npos )  { 
        if (this->GetFileNames()->GetNumberOfValues() > 1 ) { 
            isMultiCoil = true; 
        }
    //} 
    return isMultiCoil; 
}
