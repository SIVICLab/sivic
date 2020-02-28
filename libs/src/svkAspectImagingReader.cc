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


#include <svkAspectImagingReader.h>
#include <vtkCellData.h>
#include <vtkDebugLeaks.h>
#include <vtkGlobFileNames.h>
#include <vtkSortFileNames.h>
#include <vtkByteSwap.h>

#include <sys/stat.h>


using namespace svk;


//vtkCxxRevisionMacro(svkAspectImagingReader, "$Rev$");
vtkStandardNewMacro(svkAspectImagingReader);


const string svkAspectImagingReader::MFG_STRING = "GE MEDICAL SYSTEMS";


/*!
 *
 */
svkAspectImagingReader::svkAspectImagingReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkAspectImagingReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->specData = NULL; 
    this->aiHdr = NULL; 
    this->numCoils = 1; 
    this->numTimePts = 1;

    // Set the byte ordering, as big-endian.
    this->SetDataByteOrderToBigEndian(); 
}



/*!
 *
 */
svkAspectImagingReader::~svkAspectImagingReader()
{

    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if ( this->aiHdr != NULL )  {
        delete aiHdr;
        this->aiHdr = NULL;
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
int svkAspectImagingReader::CanReadFile(const char* fname)
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
int svkAspectImagingReader::GetNumSlices()
{
    return (this->GetDataExtent())[5] + 1;
}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called before ExecuteData()
 */
void svkAspectImagingReader::ExecuteInformation()
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
void svkAspectImagingReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
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
void svkAspectImagingReader::ReadComplexFile(vtkImageData* data)
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
        int numComponents =  this->GetHeaderValueAsInt( aiMap, "numberOfComponents" ); 
        int numPts = this->GetHeaderValueAsInt(aiMap, "dimensionNumberOfPoints0"); 
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
int svkAspectImagingReader::GetNumPixelsInVol()
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
void svkAspectImagingReader::SetCellSpectrum(vtkImageData* data, int x, int y, int z, int timePt, int coilNum)
{

    //  Set XY points to plot - 2 components per tuble for complex data sets:
    vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
    dataArray->SetNumberOfComponents(2);

    int numPts = this->GetHeaderValueAsInt(aiMap, "dimensionNumberOfPoints0"); 
    int numComponents =  this->GetHeaderValueAsInt( aiMap, "numberOfComponents" ); 
    dataArray->SetNumberOfTuples(numPts);
    char arrayName[30];

    sprintf(arrayName, "%d %d %d %d %d", x, y, z, timePt, coilNum);
    dataArray->SetName(arrayName);

    //  preallocate float array for spectrum and use to iniitialize the vtkDataArray:
    //  don't do this for each call
    int numVoxels[3]; 
    numVoxels[0] = this->GetHeaderValueAsInt(aiMap, "dimensionNumberOfPoints1"); 
    numVoxels[1] = this->GetHeaderValueAsInt(aiMap, "dimensionNumberOfPoints2"); 
    numVoxels[2] = this->GetHeaderValueAsInt(aiMap, "dimensionNumberOfPoints3"); 

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
void svkAspectImagingReader::InitDcmHeader()
{
    vtkDebugMacro( << this->GetClassName() << "::InitDcmHeader()" );

    this->iod = svkMRSIOD::New();
    this->iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    this->iod->InitDcmHeader();

    this->ParseAspectImaging(); 
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
 *  Read Aspect Imaging header fields into a string STL map for use during initialization
 *  of DICOM header by Init*Module methods.
 */
void svkAspectImagingReader::ParseAspectImaging()
{

    this->GlobFileNames();


    try {

        //  Read in the Aspect Imaging Header:
        this->aiHdr = new ifstream();
        this->aiHdr->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        int fileIndex = 0; 
        string currentAspectImagingFileName = this->GetFileRoot( this->GetFileNames()->GetValue( fileIndex ) ) + ".ddf";

        this->aiHdr->open( currentAspectImagingFileName.c_str(), ifstream::in );

        if ( ! this->aiHdr->is_open() ) {
            throw runtime_error( "Could not open volume file: " + string( this->GetFileName() ) );
        }
        istringstream* iss = new istringstream();

        //  Skip first line: 
        this->ReadLine(this->aiHdr, iss);

        // ASPECT_IMAGING _VERSION
        int aiVersion;
        this->ReadLine(this->aiHdr, iss);
        iss->ignore(29);
        *iss>>aiVersion;

        aiMap["objectType"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["patientId"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["patientName"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["patientCode"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["dateOfBirth"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["sex"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["studyId"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["studyCode"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["studyDate"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["accessionNumber"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["rootName"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["seriesNumber"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["seriesDescription"] = this->StripWhite( this->ReadLineValue( this->aiHdr, iss, ':') );
        aiMap["comment"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["patientEntry"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["patientPosition"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["orientation"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["dataType"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["numberOfComponents"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["sourceDescription"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["numberOfDimensions"] = this->ReadLineValue( this->aiHdr, iss, ':');
 
        // DIMENSIONS AND SPACING
        int numDimensions = this->GetHeaderValueAsInt(aiMap, "numberOfDimensions"); 

        for (int i = 0; i < numDimensions; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());

            //  Type
            string dimensionTypeString = "dimensionType" + indexString; 
            string dimensionLine( this->ReadLineSubstr(this->aiHdr, iss, 0, 100) );
            size_t start = dimensionLine.find( "type:" ); 
            string tmp   = dimensionLine.substr( start + 6 ); 
            size_t end   = tmp.find( "npoints" ); 
            aiMap[dimensionTypeString] = dimensionLine.substr( start + 6, end - 1); 

            // npoints 
            string nptsString = "dimensionNumberOfPoints" + indexString; 
            start = dimensionLine.find( "npoints:" ); 
            start += 8; 
            end   = dimensionLine.find( "pixel" ); 
            aiMap[nptsString] = dimensionLine.substr( start , end - start ); 

            //  PixelSpacing
            string pixelSpacingString = "pixelSpacing" + indexString; 
            start = dimensionLine.find( "(mm):" ); 
            if (start != string::npos) {
                aiMap[pixelSpacingString] = dimensionLine.substr( start + 5 ); 
            }
        }

        //  CENTER(LPS):
        this->ReadLineIgnore( this->aiHdr, iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> aiMap[string("centerLPS" + indexString)];
        }

        //  TOPLC(LPS):
        this->ReadLineIgnore( this->aiHdr, iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> aiMap[string("toplcLPS" + indexString)];
        }

        //  DCOS:  
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndexI;
            ossIndexI << i;
            string indexStringI(ossIndexI.str());
            this->ReadLineIgnore( this->aiHdr, iss, 's' );
            for (int j = 0; j < 3; j++) {
                ostringstream ossIndexJ;
                ossIndexJ << j;
                string indexStringJ(ossIndexJ.str());
                iss->ignore(256, ' ');
                *iss >> aiMap[string("dcos" + indexStringI + indexStringJ)];
            }
        }

        //MR Parameters 
        this->ReadLine(this->aiHdr, iss);
        this->ReadLine(this->aiHdr, iss);
        
        aiMap["coilName"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["sliceGap"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["echoTime"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["repetitionTime"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["inversionTime"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["flipAngle"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["pulseSequenceName"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["transmitterFrequency"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["isotope"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["fieldStrength"] = this->ReadLineValue( this->aiHdr, iss, ':');

        aiMap["numberOfSatBands"] = this->ReadLineValue( this->aiHdr, iss, ':');
        for (int i = 0; i < this->GetHeaderValueAsInt( aiMap, "numberOfSatBands" ); i++) {

            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            aiMap["satBand" + indexString + "Thickness"] = this->ReadLineValue( this->aiHdr, iss, ':');

            //  Orientation:
            this->ReadLineIgnore( this->aiHdr, iss, 'r' );
            for (int j = 0; j < 3; j++) {
                ostringstream ossIndexJ;
                ossIndexJ << j;
                string indexStringJ(ossIndexJ.str());
                iss->ignore(256, ' ');
                *iss >> aiMap[string("satBand" + indexString + "Orientation" + indexStringJ)];
            }

            //  position:
            this->ReadLineIgnore( this->aiHdr, iss, ')' );
            for (int j = 0; j < 3; j++) {
                ostringstream ossIndexJ;
                ossIndexJ << j;
                string indexStringJ(ossIndexJ.str());
                iss->ignore(256, ' ');
                *iss >> aiMap[string("satBand" + indexString + "PositionLPS" + indexStringJ)];
            }

        }

        //Spectroscopy Parameters 
        this->ReadLine(this->aiHdr, iss);
        this->ReadLine(this->aiHdr, iss);
        aiMap["localizationType"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["centerFrequency"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["ppmReference"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["sweepwidth"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["dwelltime"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["frequencyOffset"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["centeredOnWater"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["suppressionTechnique"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["residualWater"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["numberOfAcquisitions"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["chop"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["evenSymmetry"] = this->ReadLineValue( this->aiHdr, iss, ':');
        aiMap["dataReordered"] = this->ReadLineValue( this->aiHdr, iss, ':');

        //  ACQ TOPLC(LPS):
        this->ReadLineIgnore( this->aiHdr, iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> aiMap[string("acqToplcLPS" + indexString)];
        }

        //  ACQ Spacing:
        this->ReadLineIgnore( this->aiHdr, iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> aiMap[string("acqSpacing" + indexString)];
        }

        aiMap["acqNumberOfDataPoints"] = this->ReadLineValue( this->aiHdr, iss, ':');

        //  Acq Num Data Pts:
        this->ReadLineIgnore( this->aiHdr, iss, 's' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> aiMap[string("acqNumberOfPoints" + indexString)];
        }

        //  ACQ DCOS:  
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndexI;
            ossIndexI << i;
            string indexStringI(ossIndexI.str());
            this->ReadLineIgnore( this->aiHdr, iss, 's' );
            for (int j = 0; j < 3; j++) {
                ostringstream ossIndexJ;
                ossIndexJ << j;
                string indexStringJ(ossIndexJ.str());
                iss->ignore(256, ' ');
                *iss >> aiMap[string("acqDcos" + indexStringI + indexStringJ)];
            }
        }

        //  Selection Center:
        this->ReadLineIgnore( this->aiHdr, iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> aiMap[string("selectionCenterLPS" + indexString)];
        }

        //  Selection Size:
        this->ReadLineIgnore( this->aiHdr, iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> aiMap[string("selectionSize" + indexString)];
        }

        //  Selection DCOS:  
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndexI;
            ossIndexI << i;
            string indexStringI(ossIndexI.str());
            this->ReadLineIgnore( this->aiHdr, iss, 'd' );
            for (int j = 0; j < 3; j++) {
                ostringstream ossIndexJ;
                ossIndexJ << j;
                string indexStringJ(ossIndexJ.str());
                iss->ignore(256, ' ');
                *iss >> aiMap[string("selectionDcos" + indexStringI + indexStringJ)];
            }
        }

        //  Reordered :
        this->ReadLineIgnore( this->aiHdr, iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> aiMap[string("reorderedToplcLPS" + indexString)];
        }

        //  Reordered:
        this->ReadLineIgnore( this->aiHdr, iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> aiMap[string("reorderedCenterLPS" + indexString)];
        }

        //  Reordered:
        this->ReadLineIgnore( this->aiHdr, iss, ')' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> aiMap[string("reorderedSpacing" + indexString)];
        }

        //  Reordered:
        this->ReadLineIgnore( this->aiHdr, iss, 's' );
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndex;
            ossIndex << i;
            string indexString(ossIndex.str());
            iss->ignore(256, ' ');
            *iss >> aiMap[string("reorderedNumberOfPoints" + indexString)];
        }

        //  Selection DCOS:  
        for (int i = 0; i < 3; i++) {
            ostringstream ossIndexI;
            ossIndexI << i;
            string indexStringI(ossIndexI.str());
            this->ReadLineIgnore( this->aiHdr, iss, 's' );
            for (int j = 0; j < 3; j++) {
                ostringstream ossIndexJ;
                ossIndexJ << j;
                string indexStringJ(ossIndexJ.str());
                iss->ignore(256, ' ');
                *iss >> aiMap[string("reorderedDcos" + indexStringI + indexStringJ)];
            }
        }

        delete iss;

    } catch (const exception& e) {
        cerr << "ERROR opening or reading aspect imaging file (" << this->GetFileName() << "): " << e.what() << endl;
    }

}



/*!
 *  Initializes the VolumeLocalizationSequence in the MRSpectroscopy
 *  DICOM object for PRESS excitation.  
 */
void svkAspectImagingReader::InitVolumeLocalizationSeq()
{

    //  Get Thickness Values
    float selBoxSize[3]; 
    selBoxSize[0] = this->GetHeaderValueAsFloat(aiMap, "selectionSize0"); 
    selBoxSize[1] = this->GetHeaderValueAsFloat(aiMap, "selectionSize1"); 
    selBoxSize[2] = this->GetHeaderValueAsFloat(aiMap, "selectionSize2"); 

    //  Get Center Location Values
    float selBoxCenter[3]; 
    selBoxCenter[0] = this->GetHeaderValueAsFloat(aiMap, "selectionCenterLPS0"); 
    selBoxCenter[1] = this->GetHeaderValueAsFloat(aiMap, "selectionCenterLPS1"); 
    selBoxCenter[2] = this->GetHeaderValueAsFloat(aiMap, "selectionCenterLPS2"); 

    //  Get Orientation Values 
    float selBoxOrientation[3][3]; 
    selBoxOrientation[0][0] = this->GetHeaderValueAsFloat(aiMap, "selectionDcos00"); 
    selBoxOrientation[0][1] = this->GetHeaderValueAsFloat(aiMap, "selectionDcos01"); 
    selBoxOrientation[0][2] = this->GetHeaderValueAsFloat(aiMap, "selectionDcos02"); 
    selBoxOrientation[1][0] = this->GetHeaderValueAsFloat(aiMap, "selectionDcos10"); 
    selBoxOrientation[1][1] = this->GetHeaderValueAsFloat(aiMap, "selectionDcos11"); 
    selBoxOrientation[1][2] = this->GetHeaderValueAsFloat(aiMap, "selectionDcos12"); 
    selBoxOrientation[2][0] = this->GetHeaderValueAsFloat(aiMap, "selectionDcos20"); 
    selBoxOrientation[2][1] = this->GetHeaderValueAsFloat(aiMap, "selectionDcos21"); 
    selBoxOrientation[2][2] = this->GetHeaderValueAsFloat(aiMap, "selectionDcos22"); 

    this->GetOutput()->GetDcmHeader()->InitVolumeLocalizationSeq(
        selBoxSize, 
        selBoxCenter, 
        selBoxOrientation
    ); 

}


/*!
 *
 */
void svkAspectImagingReader::InitPatientModule() 
{

    this->GetOutput()->GetDcmHeader()->InitPatientModule(
        this->GetOutput()->GetDcmHeader()->GetDcmPatientName(  aiMap["patientName"] ),  
        aiMap["patientId"], 
        this->RemoveDelimFromDate( &(aiMap["dateOfBirth"]) ), 
        aiMap["sex"] 
    );

}


/*!
 *
 */
void svkAspectImagingReader::InitGeneralStudyModule() 
{
    this->GetOutput()->GetDcmHeader()->InitGeneralStudyModule(
        this->RemoveDelimFromDate( &(aiMap["studyDate"]) ), 
        "", 
        "", 
        aiMap["studyId"], 
        aiMap["accessionNumber"], 
        ""
    );
}


/*!
 *
 */
void svkAspectImagingReader::InitGeneralSeriesModule() 
{

    string patientEntryPos; 
    string patientEntry( aiMap["patientEntry"]); 
    if ( patientEntry.compare("head first") == 0) {
        patientEntryPos = "HF";
    } else if ( patientEntry.compare("feet first") == 0) {
        patientEntryPos = "FF";
    }

    string patientPosition( aiMap["patientPosition"]); 
    if ( patientPosition.compare("supine") == 0 ) {
        patientEntryPos.append("S");
    } else if ( patientPosition.compare("prone") == 0 ) {
        patientEntryPos.append("P");
    }

    this->GetOutput()->GetDcmHeader()->InitGeneralSeriesModule(
        aiMap["seriesNumber"], 
        aiMap["seriesDescription"], 
        patientEntryPos 
    );

}


/*!
 *  initialize to svkAspectImagingReader::MFG_STRING.
 */
void svkAspectImagingReader::InitGeneralEquipmentModule()
{
    this->GetOutput()->GetDcmHeader()->SetValue(
        "Manufacturer", 
        svkAspectImagingReader::MFG_STRING 
    );
}


/*!
 *
 */
void svkAspectImagingReader::InitMultiFrameFunctionalGroupsModule()
{

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "InstanceNumber", 
        1 
    );

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "ContentDate", 
        this->RemoveDelimFromDate( &(aiMap["studyDate"]) ) 
    );

    this->numSlices = this->GetHeaderValueAsInt( aiMap, "dimensionNumberOfPoints3" ); 

    if ( this->GetHeaderValueAsInt( aiMap, "numberOfDimensions" ) == 5 ) {
        this->numTimePts = this->GetHeaderValueAsInt( aiMap, "dimensionNumberOfPoints4" ); 
    }

    this->numCoils = this->GetFileNames()->GetNumberOfValues();

    InitSharedFunctionalGroupMacros();
    InitPerFrameFunctionalGroupMacros();
}


/*!
 *
 */
void svkAspectImagingReader::InitSharedFunctionalGroupMacros()
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
void svkAspectImagingReader::InitPerFrameFunctionalGroupMacros()
{
    //  Get toplc float array from aiMap and use that to generate
    //  frame locations.  This position is off by 1/2 voxel, fixed below:
    double toplc[3];
    toplc[0] = this->GetHeaderValueAsFloat(aiMap, "toplcLPS0"); 
    toplc[1] = this->GetHeaderValueAsFloat(aiMap, "toplcLPS1"); 
    toplc[2] = this->GetHeaderValueAsFloat(aiMap, "toplcLPS2"); 
        
    double dcos[3][3];
    dcos[0][0] = this->GetHeaderValueAsFloat(aiMap, "dcos00"); 
    dcos[0][1] = this->GetHeaderValueAsFloat(aiMap, "dcos01"); 
    dcos[0][2] = this->GetHeaderValueAsFloat(aiMap, "dcos02"); 
    dcos[1][0] = this->GetHeaderValueAsFloat(aiMap, "dcos10"); 
    dcos[1][1] = this->GetHeaderValueAsFloat(aiMap, "dcos11"); 
    dcos[1][2] = this->GetHeaderValueAsFloat(aiMap, "dcos12"); 
    dcos[2][0] = this->GetHeaderValueAsFloat(aiMap, "dcos20"); 
    dcos[2][1] = this->GetHeaderValueAsFloat(aiMap, "dcos21"); 
    dcos[2][2] = this->GetHeaderValueAsFloat(aiMap, "dcos22"); 
    
    double pixelSize[3];
    pixelSize[0] = this->GetHeaderValueAsFloat(aiMap, "pixelSpacing1"); 
    pixelSize[1] = this->GetHeaderValueAsFloat(aiMap, "pixelSpacing2"); 
    pixelSize[2] = this->GetHeaderValueAsFloat(aiMap, "pixelSpacing3"); 

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
void svkAspectImagingReader::InitPixelMeasuresMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "PixelMeasuresSequence"
    );

    //  get the spacing for the specified index:
    float colSpacing = this->GetHeaderValueAsFloat(aiMap, "pixelSpacing1"); 
    float rowSpacing = this->GetHeaderValueAsFloat(aiMap, "pixelSpacing2"); 

    string pixelSpacing;
    ostringstream oss;
    oss << colSpacing;
    oss << '\\';
    oss << rowSpacing;
    pixelSpacing = oss.str();

    this->GetOutput()->GetDcmHeader()->InitPixelMeasuresMacro(
        pixelSpacing,
        aiMap["pixelSpacing3"] 
    );
}


/*!
 *
 */
void svkAspectImagingReader::InitPlaneOrientationMacro()
{
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement( 
        "SharedFunctionalGroupsSequence",
        0, 
        "PlaneOrientationSequence"
    );

    float dcos[3][3]; 
    dcos[0][0] = this->GetHeaderValueAsFloat(aiMap, "dcos00"); 
    dcos[0][1] = this->GetHeaderValueAsFloat(aiMap, "dcos01"); 
    dcos[0][2] = this->GetHeaderValueAsFloat(aiMap, "dcos02"); 
    dcos[1][0] = this->GetHeaderValueAsFloat(aiMap, "dcos10"); 
    dcos[1][1] = this->GetHeaderValueAsFloat(aiMap, "dcos11"); 
    dcos[1][2] = this->GetHeaderValueAsFloat(aiMap, "dcos12"); 
    dcos[2][0] = this->GetHeaderValueAsFloat(aiMap, "dcos20"); 
    dcos[2][1] = this->GetHeaderValueAsFloat(aiMap, "dcos21"); 
    dcos[2][2] = this->GetHeaderValueAsFloat(aiMap, "dcos22"); 

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
void svkAspectImagingReader::InitMRTimingAndRelatedParametersMacro()
{
    this->GetOutput()->GetDcmHeader()->InitMRTimingAndRelatedParametersMacro(
        this->GetHeaderValueAsFloat(aiMap, "repetitionTime"), 
        this->GetHeaderValueAsFloat(aiMap, "flipAngle")
    ); 
}


/*!
 *
 */
void svkAspectImagingReader::InitMRSpectroscopyFOVGeometryMacro()
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
        this->GetHeaderValueAsInt( aiMap, "acqNumberOfDataPoints"),
        "SharedFunctionalGroupsSequence",      
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SpectroscopyAcquisitionPhaseColumns",
        this->GetHeaderValueAsInt( aiMap, "acqNumberOfPoints0"),
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SpectroscopyAcquisitionPhaseRows",
        this->GetHeaderValueAsInt( aiMap, "acqNumberOfPoints1"),
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SpectroscopyAcquisitionOutOfPlanePhaseSteps",
        this->GetHeaderValueAsInt( aiMap, "acqNumberOfPoints2"),
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcquisitionTLC",
        aiMap["acqToplcLPS0"] + '\\' + aiMap["acqToplcLPS1"] + '\\' + aiMap["acqToplcLPS2"],
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcquisitionPixelSpacing",
        aiMap["acqSpacing0"] + '\\' + aiMap["acqSpacing1"], 
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcquisitionSliceThickness",
        aiMap["acqSpacing2"], 
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcquisitionOrientation",
        aiMap["acqDcos00"] + '\\' + aiMap["acqDcos01"] + '\\' + aiMap["acqDcos02"] + '\\' + 
        aiMap["acqDcos10"] + '\\' + aiMap["acqDcos11"] + '\\' + aiMap["acqDcos12"] + '\\' + 
        aiMap["acqDcos20"] + '\\' + aiMap["acqDcos21"] + '\\' + aiMap["acqDcos22"], 
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
        this->GetHeaderValueAsInt( aiMap, "reorderedNumberOfPoints0" ), 
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcqReorderedPhaseRows",
        this->GetHeaderValueAsInt( aiMap, "reorderedNumberOfPoints1" ), 
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcqReorderedOutOfPlanePhaseSteps",
        this->GetHeaderValueAsInt( aiMap, "reorderedNumberOfPoints2"), 
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcqReorderedTLC",
        aiMap["reorderedToplcLPS0"] + '\\' + aiMap["reorderedToplcLPS1"] + '\\' + aiMap["reorderedToplcLPS2"],
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcqReorderedPixelSpacing",
        aiMap["reorderedSpacing0"] + '\\' + aiMap["reorderedSpacing1"],
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcqReorderedSliceThickness",
        aiMap["reorderedSpacing2"], 
        "SharedFunctionalGroupsSequence",    
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcqReorderedOrientation",
        aiMap["reorderedDcos00"] + '\\' + aiMap["reorderedDcos01"] + '\\' + aiMap["reorderedDcos02"] + '\\' + 
        aiMap["reorderedDcos10"] + '\\' + aiMap["reorderedDcos11"] + '\\' + aiMap["reorderedDcos12"] + '\\' + 
        aiMap["reorderedDcos20"] + '\\' + aiMap["reorderedDcos21"] + '\\' + aiMap["reorderedDcos22"], 
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
void svkAspectImagingReader::InitMREchoMacro()
{
    this->GetOutput()->GetDcmHeader()->InitMREchoMacro( this->GetHeaderValueAsFloat(aiMap, "echoTime") );
}


/*!
 *
 */
void svkAspectImagingReader::InitMRModifierMacro()
{
    float inversionTime = this->GetHeaderValueAsFloat( aiMap, "inversionTime");
    this->GetOutput()->GetDcmHeader()->InitMRModifierMacro( inversionTime );
}


/*!
 *
 */
void svkAspectImagingReader::InitMRReceiveCoilMacro()
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
        aiMap["coilName"], 
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
void svkAspectImagingReader::InitMRTransmitCoilMacro()
{
    this->GetOutput()->GetDcmHeader()->InitMRTransmitCoilMacro("GE", "UNKNOWN", "BODY");
}


/*!
 *
 */
void svkAspectImagingReader::InitMRAveragesMacro()
{
    int numAverages = 1; 
    this->GetOutput()->GetDcmHeader()->InitMRAveragesMacro(numAverages);
}


/*!
 *
 */
void svkAspectImagingReader::InitMRSpatialSaturationMacro()
{

    int numSatBands =  this->GetHeaderValueAsInt( aiMap, "numberOfSatBands" ); 

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
                this->GetHeaderValueAsFloat(aiMap, "satBand" + indexString + "Thickness"), 
                "SharedFunctionalGroupsSequence",    
                0
            );
        
            float orientation[3];
            orientation[0] = this->GetHeaderValueAsFloat(aiMap, "satBand" + indexString + "Orientation0");     
            orientation[1] = this->GetHeaderValueAsFloat(aiMap, "satBand" + indexString + "Orientation1");     
            orientation[2] = this->GetHeaderValueAsFloat(aiMap, "satBand" + indexString + "Orientation2");     

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
            position[0] = this->GetHeaderValueAsFloat(aiMap, "satBand" + indexString + "PositionLPS0");     
            position[1] = this->GetHeaderValueAsFloat(aiMap, "satBand" + indexString + "PositionLPS1");     
            position[2] = this->GetHeaderValueAsFloat(aiMap, "satBand" + indexString + "PositionLPS2");     

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
void svkAspectImagingReader::InitMRSpatialVelocityEncodingMacro()
{
}


/*!
 *
 */
void svkAspectImagingReader::InitAcquisitionContextModule()
{
}


/*!
 *
 */
void svkAspectImagingReader::InitMRSpectroscopyModule()
{

    /*  =======================================
     *  MR Image and Spectroscopy Instance Macro
     *  ======================================= */

    this->GetOutput()->GetDcmHeader()->SetValue(
        "AcquisitionDateTime",
        this->RemoveDelimFromDate( &(aiMap["studyDate"]) ) + "000000"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "AcquisitionDuration",
        0
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ResonantNucleus", 
        aiMap["isotope"]
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
        this->GetHeaderValueAsFloat(aiMap, "fieldStrength")
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
        this->GetHeaderValueAsFloat(aiMap, "transmitterFrequency")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SpectralWidth", 
        this->GetHeaderValueAsFloat(aiMap, "sweepwidth")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_FrequencyOffset",
        this->GetHeaderValueAsFloat( aiMap, "frequencyOffset" )
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ChemicalShiftReference", 
        this->GetHeaderValueAsFloat( aiMap, "ppmReference" )
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "VolumeLocalizationTechnique", 
        aiMap["localizationType"]
    );

    if ( strcmp(aiMap["localizationType"].c_str(), "PRESS") == 0)  { 
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
void svkAspectImagingReader::InitMRSpectroscopyPulseSequenceModule()
{
    this->GetOutput()->GetDcmHeader()->SetValue(
        "PulseSequenceName", 
        aiMap["pulseSequenceName"]
    );

    int numVoxels[3];
    numVoxels[0] = this->GetHeaderValueAsInt(aiMap, "dimensionNumberOfPoints1"); 
    numVoxels[1] = this->GetHeaderValueAsInt(aiMap, "dimensionNumberOfPoints2"); 
    numVoxels[2] = this->GetHeaderValueAsInt(aiMap, "dimensionNumberOfPoints3"); 

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
        aiMap["suppressionTechnique"]
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
    string spatialDomain = this->GetDimensionDomain( aiMap["dimensionType1"] ); 
    if ( spatialDomain.compare("KSPACE") == 0) {

        string k0Sampled = "YES";
       
        string aiSym = this->aiMap[ "evenSymmetry" ]; 
        // data dims odd? 
        if (   (numVoxels[0] > 1 && numVoxels[0] % 2) 
            || (numVoxels[1] > 1 && numVoxels[1] % 2 ) 
            || (numVoxels[2] > 1 && numVoxels[2] % 2 ) ) {
            if ( aiSym.compare("yes") == 0 ) {
                k0Sampled = "YES"; 
            } else {
                k0Sampled = "NO";   
            }
        } else {
            if ( aiSym.compare("yes") == 0 ) {
                k0Sampled = "NO"; 
            } else {
                k0Sampled = "YES";   
            }
        }

        this->GetOutput()->GetDcmHeader()->SetValue( "SVK_K0Sampled", k0Sampled);
    }

    string chop = "no";
    if ( (this->aiMap[ "chop" ] ).compare("yes") == 0 ) {
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
void svkAspectImagingReader::InitMRSpectroscopyDataModule()
{
    int numVoxels[3];
    numVoxels[0] = this->GetHeaderValueAsInt(aiMap, "dimensionNumberOfPoints1"); 
    numVoxels[1] = this->GetHeaderValueAsInt(aiMap, "dimensionNumberOfPoints2"); 
    numVoxels[2] = this->GetHeaderValueAsInt(aiMap, "dimensionNumberOfPoints3"); 

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
        this->GetHeaderValueAsInt(aiMap, "dimensionNumberOfPoints0") 
    );

    int numComponents =  this->GetHeaderValueAsInt( aiMap, "numberOfComponents" ); 
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
        this->GetDimensionDomain( aiMap["dimensionType0"] )
    );


    //  Private Attributes for spatial domain encoding:
    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_ColumnsDomain", 
        this->GetDimensionDomain( aiMap["dimensionType1"] )
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_RowsDomain", 
        this->GetDimensionDomain( aiMap["dimensionType2"] )
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_SliceDomain", 
        this->GetDimensionDomain( aiMap["dimensionType3"] )
    );

}


/*!
 *  Returns the file root without extension
 */
svkDcmHeader::DcmPixelDataFormat svkAspectImagingReader::GetFileType()
{
    return svkDcmHeader::SIGNED_FLOAT_4;
}


/*! 
 *  Converts the aspect imaging dimension type to a string for DICOM domain tags: 
 */
string svkAspectImagingReader::GetDimensionDomain( string aiDomainString )
{
    //cout << "domain: " << aiDomainString << endl;
    string domain;  
    if ( aiDomainString.compare("time") == 0 )  { 
        domain.assign("TIME"); 
    } else if ( aiDomainString.compare("frequency") == 0 )  { 
        domain.assign("FREQUENCY"); 
    } else if ( aiDomainString.compare("space") == 0 )  { 
        domain.assign("SPACE"); 
    } else if ( aiDomainString.compare("k-space") == 0 )  { 
        domain.assign("KSPACE"); 
    }
    return domain; 
}


/*!
 *
 */
int svkAspectImagingReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}


/*!
 *  Prints the key value pairs parsed from the header.
 */
void svkAspectImagingReader::PrintKeyValuePairs()
{
    if (this->GetDebug()) {
        map< string, string >::iterator mapIter;
        for ( mapIter = aiMap.begin(); mapIter != aiMap.end(); ++mapIter ) {
            cout << this->GetClassName() << " " << mapIter->first << " = ";
            cout << aiMap[mapIter->first] << endl;
        }
    }
}


/*!
 *
 */
int svkAspectImagingReader::GetHeaderValueAsInt(map <string, string> hdrMap, 
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
float svkAspectImagingReader::GetHeaderValueAsFloat(map <string, string> hdrMap, 
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
bool svkAspectImagingReader::IsMultiCoil()
{
    bool isMultiCoil = false; 

    if (this->GetFileNames()->GetNumberOfValues() > 1 ) { 
        isMultiCoil = true; 
    }
   
    return isMultiCoil; 
}


