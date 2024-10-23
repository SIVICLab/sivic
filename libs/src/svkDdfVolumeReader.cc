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


#include <svkDdfVolumeReader.h>
#include </usr/include/vtk/vtkCellData.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkGlobFileNames.h>
#include </usr/include/vtk/vtkSortFileNames.h>
#include </usr/include/vtk/vtkByteSwap.h>

#include <sys/stat.h>


using namespace svk;


//vtkCxxRevisionMacro(svkDdfVolumeReader, "$Rev$");
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

    // Set the byte ordering, as big-endian.
    this->SetDataByteOrderToBigEndian(); 
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

    if ( this->specData != NULL )  {
        delete [] specData;
        this->specData = NULL;
    }

}



/*!
 *  Check to see if the extension indicates a UCSF IDF file.  If so, try
 *  to open the file for reading.  If that works, then return a success code.
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkDdfVolumeReader::CanReadFile(const char* fname)
{

    //  If file has an extension then check it:
    string fileExtension = this->GetFileExtension( fname );  
    if( ! fileExtension.empty() ) {

        // Also see "vtkImageReader2::GetFileExtensions method" 
        
        if ( 
            fileExtension.compare( "cmplx" ) == 0 || 
            fileExtension.compare( "ddf" ) == 0 
        )  {
            FILE *fp = fopen(fname, "rb");
            if (fp) {
                fclose(fp);

                vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's a UCSF Complex File: " << fname );
                return 1;
            }
        } else {
            vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's NOT a UCSF Complex File: " << fname );
            return 0;
        }

    } else {
        vtkDebugMacro(<< this->GetClassName() <<"::CanReadFile(): It's NOT a valid file name : " << fname );
        return 0;
    }
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
		if (this->GetDebug()) {
			this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
		}
        this->SetupOutputInformation();

    }

    //  This is a workaround required since the vtkImageAlgo executive
    //  for the reader resets the Extent[5] value to the number of files
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
void svkDdfVolumeReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
{

    this->FileNames = vtkStringArray::New();
    this->FileNames->DeepCopy(this->tmpFileNames);
    this->tmpFileNames->Delete();
    this->tmpFileNames = NULL;

    int numVoxels[3] = { this->GetDataExtent()[1], this->GetDataExtent()[3], this->GetDataExtent()[5] }; 

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
    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
    this->GetOutput()->SetDcos(dcos);

    //  SetNumberOfIncrements is supposed to call this, but only works if the data has already
    //  been allocated. but that requires the number of components to be specified.
    this->GetOutput()->GetIncrements();

    this->GetOutput()->GetProvenance()->AddAlgorithm( this->GetClassName() );

}


/*!
 *
 */
void svkDdfVolumeReader::ReadComplexFile(vtkImageData* data)
{

    if ( this->onlyReadHeader == true ) {
        return;
    }


    for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {

        if (this->GetDebug()) { 
            cout << "read data from file " << fileIndex << " / " << this->GetFileNames()->GetNumberOfValues() << endl;
        }
        //cout << "read data from file " << fileIndex << " / " << this->GetFileNames()->GetNumberOfValues() << endl;
        //cout << "FN: " << this->GetFileNames()->GetValue( fileIndex ) << endl;

        int coilNum = 0; 
        if ( this->numCoils > 1 ) {
            coilNum = fileIndex;     
        }

        ifstream* cmplxDataIn = new ifstream();
        cmplxDataIn->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        string cmplxFile = this->GetFileRoot( this->GetFileNames()->GetValue( fileIndex ) ) + ".cmplx"; 
        if( svkUtils::IsFileCompressed( cmplxFile )) {
        	svkUtils::UncompressFile( cmplxFile );
        }

        cmplxDataIn->open( cmplxFile.c_str(), ios::binary);

        //  Read in data from 1 coil:
        int numComponents =  this->GetHeaderValueAsInt( ddfMap, "numberOfComponents" ); 
        int numPts = this->GetHeaderValueAsInt(ddfMap, "dimensionNumberOfPoints0"); 
        int numBytesInVol = this->GetNumPixelsInVol() * numPts * numComponents * sizeof(float) * this->numTimePts; 

        this->specData = new float[ numBytesInVol/sizeof(float) ];  
        cmplxDataIn->read( (char*)(this->specData), numBytesInVol );

        if ( this->GetSwapBytes() ) {
            vtkByteSwap::SwapVoidRange((void *)this->specData, numBytesInVol/sizeof(float), sizeof(float));
        }

        int numVoxels[3] = { this->GetDataExtent()[1], this->GetDataExtent()[3], this->GetDataExtent()[5] }; 
        int denominator = numVoxels[2] * numVoxels[1]  * numVoxels[0] + numVoxels[1]*numVoxels[0] + numVoxels[0];
        double progress = 0;


        for (int timePt = 0; timePt < this->numTimePts ; timePt++) {
            ostringstream progressStream;
            progressStream <<"Reading Time Point " << timePt+1 << "/"
                           << numTimePts << " and Channel: " << coilNum+1 << "/" << numCoils;
            this->SetProgressText( progressStream.str().c_str() );
            for (int z = 0; z < (this->GetDataExtent())[5] ; z++) {
                for (int y = 0; y < (this->GetDataExtent())[3]; y++) {

                    for (int x = 0; x < (this->GetDataExtent())[1]; x++) {
                        SetCellSpectrum(data, x, y, z, timePt, coilNum);
                    }
                    progress = (((z) * (numVoxels[0]) * (numVoxels[1]) ) + ( (y) * (numVoxels[0]) ))
                                       /((double)denominator);
                    this->UpdateProgress( progress );
                }
            }
        }

        svkFastCellData::SafeDownCast(data->GetCellData())->FinishFastAdd();

        cmplxDataIn->close(); 
        delete cmplxDataIn;
        delete [] specData; 
        specData = NULL;
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
    svkFastCellData::SafeDownCast(data->GetCellData())->FastAddArray(dataArray); 

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

    this->iod = svkMRSIOD::New();
    this->iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    this->iod->InitDcmHeader();

    this->ParseDdf(); 
    this->PrintKeyValuePairs();

    //  Fill in data set specific values:
    this->InitPatientModule(); 
    this->InitGeneralStudyModule(); 
    this->InitGeneralSeriesModule(); 
    this->InitGeneralEquipmentModule(); 
    this->InitMultiFrameFunctionalGroupsModule();
    this->InitAcquisitionContextModule();
    this->InitMRSpectroscopyModule(); 
    this->InitMRSpectroscopyPulseSequenceModule(); 
    this->InitMRSpectroscopyDataModule(); 

    if (this->GetDebug()) { 
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }

    this->iod->Delete();

    this->GetOutput()->GetDcmHeader()->SetValue( "SVK_PRIVATE_TAG",  "SVK_PRIVATE_CREATOR");

}


/*!
 *  Read DDF header fields into a string STL map for use during initialization
 *  of DICOM header by Init*Module methods.
 */
void svkDdfVolumeReader::ParseDdf()
{

    this->GlobFileNames();


    try {

        //  Read in the DDF Header:
        this->ddfHdr = new ifstream();
        this->ddfHdr->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        int fileIndex = 0; 
        string currentDdfFileName = this->GetFileRoot( this->GetFileNames()->GetValue( fileIndex ) ) + ".ddf";

        this->ddfHdr->open( currentDdfFileName.c_str(), ifstream::in );

        if ( ! this->ddfHdr->is_open() ) {
            throw runtime_error( "Could not open volume file: " + string( this->GetFileName() ) );
        }
        istringstream* iss = new istringstream();

        //  Skip first line: 
        this->ReadLine(this->ddfHdr, iss);

        // DDF_VERSION
        int ddfVersion;
        this->ReadLine(this->ddfHdr, iss);
        iss->ignore(29);
        *iss>>ddfVersion;

        ddfMap["objectType"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["patientId"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["patientName"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["patientCode"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["dateOfBirth"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["sex"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["studyId"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["studyCode"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["studyDate"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["accessionNumber"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["rootName"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["seriesNumber"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["seriesDescription"] = this->StripWhite( this->ReadLineValue( this->ddfHdr, iss, ':') );
        ddfMap["comment"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["patientEntry"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["patientPosition"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["orientation"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["dataType"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["numberOfComponents"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["sourceDescription"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["numberOfDimensions"] = this->ReadLineValue( this->ddfHdr, iss, ':');
 
        // DIMENSIONS AND SPACING
        int numDimensions = this->GetHeaderValueAsInt(ddfMap, "numberOfDimensions"); 

        for (int i = 0; i < numDimensions; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());

            //  Type
            string dimensionTypeString = "dimensionType" + indexString; 
            string dimensionLine( this->ReadLineSubstr(this->ddfHdr, iss, 0, 100) );
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
        this->ReadLineIgnore( this->ddfHdr, iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> ddfMap[string("centerLPS" + indexString)];
        }

        //  TOPLC(LPS):
        this->ReadLineIgnore( this->ddfHdr, iss, ')' );
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
            this->ReadLineIgnore( this->ddfHdr, iss, 's' );
            for (int j = 0; j < 3; j++) {
                ostringstream ossIndexJ;
                ossIndexJ << j;
                string indexStringJ(ossIndexJ.str());
                iss->ignore(256, ' ');
                *iss >> ddfMap[string("dcos" + indexStringI + indexStringJ)];
            }
        }

        //MR Parameters 
        this->ReadLine(this->ddfHdr, iss);
        this->ReadLine(this->ddfHdr, iss);
        
        ddfMap["coilName"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["sliceGap"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["echoTime"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["repetitionTime"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["inversionTime"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["flipAngle"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["pulseSequenceName"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["transmitterFrequency"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["isotope"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["fieldStrength"] = this->ReadLineValue( this->ddfHdr, iss, ':');

        ddfMap["numberOfSatBands"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        for (int i = 0; i < this->GetHeaderValueAsInt( ddfMap, "numberOfSatBands" ); i++) {

            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            ddfMap["satBand" + indexString + "Thickness"] = this->ReadLineValue( this->ddfHdr, iss, ':');

            //  Orientation:
            this->ReadLineIgnore( this->ddfHdr, iss, 'r' );
            for (int j = 0; j < 3; j++) {
                ostringstream ossIndexJ;
                ossIndexJ << j;
                string indexStringJ(ossIndexJ.str());
                iss->ignore(256, ' ');
                *iss >> ddfMap[string("satBand" + indexString + "Orientation" + indexStringJ)];
            }

            //  position:
            this->ReadLineIgnore( this->ddfHdr, iss, ')' );
            for (int j = 0; j < 3; j++) {
                ostringstream ossIndexJ;
                ossIndexJ << j;
                string indexStringJ(ossIndexJ.str());
                iss->ignore(256, ' ');
                *iss >> ddfMap[string("satBand" + indexString + "PositionLPS" + indexStringJ)];
            }

        }

        //Spectroscopy Parameters 
        this->ReadLine(this->ddfHdr, iss);
        this->ReadLine(this->ddfHdr, iss);
        ddfMap["localizationType"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["centerFrequency"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["ppmReference"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["sweepwidth"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["dwelltime"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["frequencyOffset"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["centeredOnWater"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["suppressionTechnique"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["residualWater"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["numberOfAcquisitions"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["chop"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["evenSymmetry"] = this->ReadLineValue( this->ddfHdr, iss, ':');
        ddfMap["dataReordered"] = this->ReadLineValue( this->ddfHdr, iss, ':');

        //  ACQ TOPLC(LPS):
        this->ReadLineIgnore( this->ddfHdr, iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> ddfMap[string("acqToplcLPS" + indexString)];
        }

        //  ACQ Spacing:
        this->ReadLineIgnore( this->ddfHdr, iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> ddfMap[string("acqSpacing" + indexString)];
        }

        ddfMap["acqNumberOfDataPoints"] = this->ReadLineValue( this->ddfHdr, iss, ':');

        //  Acq Num Data Pts:
        this->ReadLineIgnore( this->ddfHdr, iss, 's' );
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
            this->ReadLineIgnore( this->ddfHdr, iss, 's' );
            for (int j = 0; j < 3; j++) {
                ostringstream ossIndexJ;
                ossIndexJ << j;
                string indexStringJ(ossIndexJ.str());
                iss->ignore(256, ' ');
                *iss >> ddfMap[string("acqDcos" + indexStringI + indexStringJ)];
            }
        }

        //  Selection Center:
        this->ReadLineIgnore( this->ddfHdr, iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> ddfMap[string("selectionCenterLPS" + indexString)];
        }

        //  Selection Size:
        this->ReadLineIgnore( this->ddfHdr, iss, ')' );
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
            this->ReadLineIgnore( this->ddfHdr, iss, 'd' );
            for (int j = 0; j < 3; j++) {
                ostringstream ossIndexJ;
                ossIndexJ << j;
                string indexStringJ(ossIndexJ.str());
                iss->ignore(256, ' ');
                *iss >> ddfMap[string("selectionDcos" + indexStringI + indexStringJ)];
            }
        }

        //  Reordered :
        this->ReadLineIgnore( this->ddfHdr, iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> ddfMap[string("reorderedToplcLPS" + indexString)];
        }

        //  Reordered:
        this->ReadLineIgnore( this->ddfHdr, iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> ddfMap[string("reorderedCenterLPS" + indexString)];
        }

        //  Reordered:
        this->ReadLineIgnore( this->ddfHdr, iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> ddfMap[string("reorderedSpacing" + indexString)];
        }

        //  Reordered:
        this->ReadLineIgnore( this->ddfHdr, iss, 's' );
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
            this->ReadLineIgnore( this->ddfHdr, iss, 's' );
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
        cerr << "ERROR opening or reading ddf file (" << this->GetFileName() << "): " << e.what() << endl;
    }

}



/*!
 *  Initializes the VolumeLocalizationSequence in the MRSpectroscopy
 *  DICOM object for PRESS excitation.  
 */
void svkDdfVolumeReader::InitVolumeLocalizationSeq()
{

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

    this->GetOutput()->GetDcmHeader()->InitVolumeLocalizationSeq(
        selBoxSize, 
        selBoxCenter, 
        selBoxOrientation
    ); 

}


/*!
 *
 */
void svkDdfVolumeReader::InitPatientModule() 
{

    this->GetOutput()->GetDcmHeader()->InitPatientModule(
        this->GetOutput()->GetDcmHeader()->GetDcmPatientName(  ddfMap["patientName"] ),  
        ddfMap["patientId"], 
        this->RemoveDelimFromDate( &(ddfMap["dateOfBirth"]) ), 
        ddfMap["sex"] 
    );

}


/*!
 *
 */
void svkDdfVolumeReader::InitGeneralStudyModule() 
{
    this->GetOutput()->GetDcmHeader()->InitGeneralStudyModule(
        this->RemoveDelimFromDate( &(ddfMap["studyDate"]) ), 
        "", 
        "", 
        ddfMap["studyId"], 
        ddfMap["accessionNumber"], 
        ""
    );
}


/*!
 *
 */
void svkDdfVolumeReader::InitGeneralSeriesModule() 
{

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

    this->GetOutput()->GetDcmHeader()->InitGeneralSeriesModule(
        ddfMap["seriesNumber"], 
        ddfMap["seriesDescription"], 
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

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "InstanceNumber", 
        1 
    );

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "ContentDate", 
        this->RemoveDelimFromDate( &(ddfMap["studyDate"]) ) 
    );

    this->numSlices = this->GetHeaderValueAsInt( ddfMap, "dimensionNumberOfPoints3" ); 

    if ( this->GetHeaderValueAsInt( ddfMap, "numberOfDimensions" ) == 5 ) {
        this->numTimePts = this->GetHeaderValueAsInt( ddfMap, "dimensionNumberOfPoints4" ); 
    }

    this->numCoils = this->GetFileNames()->GetNumberOfValues();

    InitSharedFunctionalGroupMacros();
    InitPerFrameFunctionalGroupMacros();
}


/*!
 *
 */
void svkDdfVolumeReader::InitSharedFunctionalGroupMacros()
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
void svkDdfVolumeReader::InitPerFrameFunctionalGroupMacros()
{
    //  Get toplc float array from ddfMap and use that to generate
    //  frame locations.  This position is off by 1/2 voxel, fixed below:
    double toplc[3];
    toplc[0] = this->GetHeaderValueAsFloat(ddfMap, "toplcLPS0"); 
    toplc[1] = this->GetHeaderValueAsFloat(ddfMap, "toplcLPS1"); 
    toplc[2] = this->GetHeaderValueAsFloat(ddfMap, "toplcLPS2"); 
        
    double dcos[3][3];
    dcos[0][0] = this->GetHeaderValueAsFloat(ddfMap, "dcos00"); 
    dcos[0][1] = this->GetHeaderValueAsFloat(ddfMap, "dcos01"); 
    dcos[0][2] = this->GetHeaderValueAsFloat(ddfMap, "dcos02"); 
    dcos[1][0] = this->GetHeaderValueAsFloat(ddfMap, "dcos10"); 
    dcos[1][1] = this->GetHeaderValueAsFloat(ddfMap, "dcos11"); 
    dcos[1][2] = this->GetHeaderValueAsFloat(ddfMap, "dcos12"); 
    dcos[2][0] = this->GetHeaderValueAsFloat(ddfMap, "dcos20"); 
    dcos[2][1] = this->GetHeaderValueAsFloat(ddfMap, "dcos21"); 
    dcos[2][2] = this->GetHeaderValueAsFloat(ddfMap, "dcos22"); 
    
    double pixelSize[3];
    pixelSize[0] = this->GetHeaderValueAsFloat(ddfMap, "pixelSpacing1"); 
    pixelSize[1] = this->GetHeaderValueAsFloat(ddfMap, "pixelSpacing2"); 
    pixelSize[2] = this->GetHeaderValueAsFloat(ddfMap, "pixelSpacing3"); 

    svkDcmHeader::DimensionVector dimensionVector = this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector(); 
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, this->numSlices-1);

    this->GetOutput()->GetDcmHeader()->AddDimensionIndex(
            &dimensionVector, svkDcmHeader::TIME_INDEX, this->numTimePts - 1 );

    this->GetOutput()->GetDcmHeader()->AddDimensionIndex(
            &dimensionVector, svkDcmHeader::CHANNEL_INDEX, this->numCoils - 1 );

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

    string pixelSpacing;
    ostringstream oss;
    oss << colSpacing;
    oss << '\\';
    oss << rowSpacing;
    pixelSpacing = oss.str();

    this->GetOutput()->GetDcmHeader()->InitPixelMeasuresMacro(
        pixelSpacing,
        ddfMap["pixelSpacing3"] 
    );
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
void svkDdfVolumeReader::InitMRTimingAndRelatedParametersMacro()
{
    this->GetOutput()->GetDcmHeader()->InitMRTimingAndRelatedParametersMacro(
        this->GetHeaderValueAsFloat(ddfMap, "repetitionTime"), 
        this->GetHeaderValueAsFloat(ddfMap, "flipAngle")
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
        "SVK_SpectroscopyAcqReorderedPhaseColumns",
        this->GetHeaderValueAsInt( ddfMap, "reorderedNumberOfPoints0" ), 
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcqReorderedPhaseRows",
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
    this->GetOutput()->GetDcmHeader()->InitMREchoMacro( this->GetHeaderValueAsFloat(ddfMap, "echoTime") );
}


/*!
 *
 */
void svkDdfVolumeReader::InitMRModifierMacro()
{
    float inversionTime = this->GetHeaderValueAsFloat( ddfMap, "inversionTime");
    this->GetOutput()->GetDcmHeader()->InitMRModifierMacro( inversionTime );
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
void svkDdfVolumeReader::InitMRTransmitCoilMacro()
{
    this->GetOutput()->GetDcmHeader()->InitMRTransmitCoilMacro("GE", "UNKNOWN", "BODY");
}


/*!
 *
 */
void svkDdfVolumeReader::InitMRAveragesMacro()
{
    int numAverages = 1; 
    this->GetOutput()->GetDcmHeader()->InitMRAveragesMacro(numAverages);
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
        "AcquisitionDateTime",
        this->RemoveDelimFromDate( &(ddfMap["studyDate"]) ) + "000000"
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
        "Research" 
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

    //  If k-space data and sym is even: set to NO (assume GE data with even num pts)
    //  method: bool isK0Sampled( sym(even/odd), dimension(even/odd) ); 
    string spatialDomain = this->GetDimensionDomain( ddfMap["dimensionType1"] ); 
    if ( spatialDomain.compare("KSPACE") == 0) {

        string k0Sampled = "YES";
       
        string ddfSym = this->ddfMap[ "evenSymmetry" ]; 
        // data dims odd? 
        if (   (numVoxels[0] > 1 && numVoxels[0] % 2) 
            || (numVoxels[1] > 1 && numVoxels[1] % 2 ) 
            || (numVoxels[2] > 1 && numVoxels[2] % 2 ) ) {
            if ( ddfSym.compare("yes") == 0 ) {
                k0Sampled = "YES"; 
            } else {
                k0Sampled = "NO";   
            }
        } else {
            if ( ddfSym.compare("yes") == 0 ) {
                k0Sampled = "NO"; 
            } else {
                k0Sampled = "YES";   
            }
        }

        this->GetOutput()->GetDcmHeader()->SetValue( "SVK_K0Sampled", k0Sampled);
    }

    string chop = "no";
    if ( (this->ddfMap[ "chop" ] ).compare("yes") == 0 ) {
        chop = "YES";
    }
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "SVK_AcquisitionChop", 
        chop 
    );

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
        "SVK_ColumnsDomain", 
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
    //cout << "domain: " << ddfDomainString << endl;
    string domain;  
    if ( ddfDomainString.compare("time") == 0 )  { 
        domain.assign("TIME"); 
    } else if ( ddfDomainString.compare("frequency") == 0 )  { 
        domain.assign("FREQUENCY"); 
    } else if ( ddfDomainString.compare("space") == 0 )  { 
        domain.assign("SPACE"); 
    } else if ( ddfDomainString.compare("k-space") == 0 )  { 
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


/*!
 *
 */
int svkDdfVolumeReader::GetHeaderValueAsInt(map <string, string> hdrMap, 
    string keyString, int valueIndex)
{
    istringstream* iss = new istringstream();
    int value = 0;

    iss->str( hdrMap[keyString] );
    *iss >> value;
    delete iss; 
    return value;
}


/*!
 *
 */
float svkDdfVolumeReader::GetHeaderValueAsFloat(map <string, string> hdrMap, 
    string keyString, int valueIndex)
{
    istringstream* iss = new istringstream();
    float value = 0;
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

    if (this->GetFileNames()->GetNumberOfValues() > 1 ) { 
        isMultiCoil = true; 
    }
   
    return isMultiCoil; 
}


