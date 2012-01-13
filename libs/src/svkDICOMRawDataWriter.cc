/*
 *  Copyright © 2009-2011 The Regents of the University of California.
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



#include <svkDICOMRawDataWriter.h>
#include <svkRawIOD.h>
#include <svkGEPFileReader.h>
#include <svkImageReaderFactory.h>


using namespace svk;


vtkCxxRevisionMacro(svkDICOMRawDataWriter, "$Rev$");
vtkStandardNewMacro(svkDICOMRawDataWriter);


/*!
 *
 */
svkDICOMRawDataWriter::svkDICOMRawDataWriter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( <<  this->GetClassName() << "::" << this->GetClassName() << "()");

    this->seriesNumber = 0; 
    this->instanceNumber = 1; 
    this->sha1Digest = ""; 

    this->SetErrorCode(vtkErrorCode::NoError);
    this->computedPFileSize = 0; 
}


/*!
 *
 */
svkDICOMRawDataWriter::~svkDICOMRawDataWriter()
{
}


/*!
 *  Add filename for file associated with the PFile and to be 
 *  included in RawData SOP instance. 
 *
 */
void svkDICOMRawDataWriter::AddAssociatedFile( string fileName, string sha1DigestVal )
{

    if ( fileName.size() == 0 || sha1DigestVal.size() != 40 ) {
        vtkErrorMacro("Missing FileName or valid SHA1 Digest.  Can Not generate DICOM Raw Data File. " );
        this->SetErrorCode(vtkErrorCode::UnknownError);
    } else {
        vector < string > file; 
        file.push_back( fileName ); 
        file.push_back( sha1DigestVal ); 
        file.push_back( "Associated File" ); 
        this->associatedFiles.push_back( file ); 
    }
}



/*!
 *  The main method which triggers the writer to create DICOM Raw Data file.
 */
void svkDICOMRawDataWriter::Write()
{


    svkImageReaderFactory* readerFactory =svkImageReaderFactory::New();
    readerFactory->QuickParse();
    svkGEPFileReader* reader = svkGEPFileReader::SafeDownCast( readerFactory->CreateImageReader2(this->FileName) );
    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << this->FileName << endl;
        exit(1);
    }
    reader->OnlyParseHeader();
    reader->SetFileName( this->FileName );
    reader->Update();
    this->pfMap = reader->GetPFMap();

    this->InitDcmHeader();

    if ( this->GetErrorCode() != vtkErrorCode::NoError ) { 
        vtkErrorMacro(<< "Could not initialize DICOM Raw Data file for writing." );
        return; 
    }


    if ( this->FileName ) { 
        string fileNameString( this->FileName );
        size_t pos = fileNameString.rfind(".dcm");
        if( pos == string::npos ) {
            pos = fileNameString.rfind(".DCM");
            if( pos == string::npos ) {
                fileNameString+=".dcm";
                delete this->FileName;
                this->FileName = new char[fileNameString.size() + 1];
                strcpy(this->FileName, fileNameString.c_str());
            }
        }
    } else if ( this->FilePattern ) {
        string filePatternString( this->FilePattern );
        size_t pos = filePatternString.rfind(".dcm");
        if( pos == string::npos ) {
            pos = filePatternString.rfind(".DCM");
            if( pos == string::npos ) {
                filePatternString+=".dcm";
                delete this->FilePattern;
                this->FilePattern = new char[filePatternString.size() + 1];
                strcpy(this->FilePattern, filePatternString.c_str());
            }
        } 
        if( this->FilePrefix ) {
            string filePrefixString( this->FilePrefix );
            size_t pos = filePrefixString.rfind(".dcm");
            if( pos != string::npos ) {
                filePrefixString.replace(pos, pos + 4, "");
                delete this->FilePrefix;
                this->FilePrefix = new char[filePrefixString.size() + 1];
                strcpy(this->FilePrefix, filePrefixString.c_str());
            }
            pos = filePrefixString.rfind(".DCM");
            if( pos != string::npos ) {
                filePrefixString.replace(pos, pos + 4, "");
                delete this->FilePrefix;
                this->FilePrefix = new char[filePrefixString.size() + 1];
                strcpy(this->FilePrefix, filePrefixString.c_str());
            }
        }
    } else {
        vtkErrorMacro(<<"Write:Please specify either a FileName or a file prefix and pattern");
        this->SetErrorCode(vtkErrorCode::NoFileNameError);
        return;
    }
   
     
    // Make sure the file name is allocated
    this->InternalFileName =
        new char[(this->FileName ? strlen(this->FileName) : 1) +
            (this->FilePrefix ? strlen(this->FilePrefix) : 1) +
            (this->FilePattern ? strlen(this->FilePattern) : 1) + 10];
    

    this->FileNumber = 1; 
    this->MinimumFileNumber = this->MaximumFileNumber = this->FileNumber;
    this->FilesDeleted = 0;
    this->UpdateProgress(0.0);

 
    this->dcmHeader->InsertUniqueUID( "SOPInstanceUID" );

    this->dcmHeader->SetValue(
        "InstanceNumber",
        this->FileNumber 
    );

    // determine the name
    if (this->FileName) {
        sprintf(this->InternalFileName,"%s",this->FileName);
    } else {
        if (this->FilePrefix) {
            sprintf(this->InternalFileName, this->FilePattern, this->FilePrefix, this->FileNumber );
        } else {
            sprintf(this->InternalFileName, this->FilePattern,this->FileNumber);
        }
    }

    this->dcmHeader->WriteDcmFile(this->InternalFileName);

    if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError) {
        vtkErrorMacro("Ran out of disk space; deleting file(s) already written");
        this->DeleteFiles();
        return;
    }

    delete [] this->InternalFileName;
    this->InternalFileName = NULL;

}


/*!
 *  Generates a new svkDcmHeader initialized to a default DICOM Secondary
 *  Capture IOD, then fills in instance specific values.       
 *  The StudyInstanceUID and other patient/exam specific attributes are copied from the 
 *  template to attach the images to the data from which it was derived.    
 */
void svkDICOMRawDataWriter::InitDcmHeader()
{

    if ( svkDcmHeader::adapter_type == svkDcmtkAdapter::DCMTK_API ) {
        this->dcmHeader =  svkDcmtkAdapter::New() ;
    }

    /*  
     *  Get Default populated SOP Instance:
     */
    svkIOD* iod = svkRawIOD::New();
    iod->SetDcmHeader( this->dcmHeader );
    iod->InitDcmHeader();
    iod->Delete();


    this->InitPatientModule(); 
    this->InitGeneralStudyModule(); 
    this->InitGeneralSeriesModule(); 
    this->InitGeneralEquipmentModule(); 
    this->InitRawDataModule(); 

    if (this->GetDebug()) {
        //this->dcmHeader->PrintDcmHeader();
    }

}


/*!
 *
 */
void svkDICOMRawDataWriter::InitPatientModule()
{

    int patsex = this->GetHeaderValueAsInt("rhe.patsex");

    vtkstd::string gender("O");
    if ( patsex == 1 ) {
        gender.assign("M");
    } else if ( patsex == 2 ) {
        gender.assign("F");
    }

    this->dcmHeader->InitPatientModule(
        this->pfMap["rhe.patname"][3],  
        this->pfMap["rhe.patid"][3],  
        this->pfMap["rhe.dateofbirth"][3],  
        gender
    );

}


/*!
 *
 */
void svkDICOMRawDataWriter::InitGeneralStudyModule()
{

    vtkstd::string dcmDate = svkGEPFileMapper::ConvertGEDateToDICOM( this->pfMap["rhr.rh_scan_date"][3] ); 

    this->dcmHeader->InitGeneralStudyModule(
        svkImageReader2::RemoveSlashesFromDate( &dcmDate ),
        this->pfMap["rhr.rh_scan_time"][3],  
        this->pfMap["rhe.refphy"][3],  
        this->pfMap["rhe.ex_no"][3],  
        this->pfMap["rhe.reqnum"][3],  
        this->pfMap["rhe.study_uid"][3]  
    );

}


/*!
 *
 */
void svkDICOMRawDataWriter::InitGeneralSeriesModule()
{

    vtkstd::string patientEntryPos;
    int patientEntry( this->GetHeaderValueAsInt( "rhs.entry" ) );
    if ( patientEntry == 0) {
        patientEntryPos = "Unknown";
    } else if ( patientEntry == 1) {
        patientEntryPos = "HF";
    } else if ( patientEntry == 2) {
        patientEntryPos = "FF";
    }

    int patientPosition( this->GetHeaderValueAsInt( "rhs.position" ) );
    if ( patientPosition == 0 ) {
        patientEntryPos.append("Unknown");
    } else if ( patientPosition == 1 ) {
        patientEntryPos.append("S");
    } else if ( patientPosition == 2 ) {
        patientEntryPos.append("P");
    }

    this->dcmHeader->InitGeneralSeriesModule(
        this->pfMap["rhs.se_no"][3],  
        this->pfMap["rhs.se_desc"][3],  
        patientEntryPos
    );

    #   Retain the SeriesInstanceUID from the raw file as well.
    this->dcmHeader->SetValue( "SeriesInstanceUID", this->pfMap["rhs.series_uid"] ); 

}


/*!
 *  initialize
 */
void svkDICOMRawDataWriter::InitGeneralEquipmentModule()
{
    this->dcmHeader->SetValue(
        "Manufacturer",
        "GE MEDICAL SYSTEMS"
    );

    this->dcmHeader->SetValue(
        "InstitutionName",
        this->pfMap["rhe.hospname"][3]  
    );

    this->dcmHeader->SetValue(
        "StationName",
        this->pfMap["rhe.ex_sysid"][3]  
    );

}


/*!
 *  returns the value for the specified key as an int.
 */
void svkDICOMRawDataWriter::InitRawDataModule()
{

    int raw_pass_size = this->GetHeaderValueAsInt("rhr.rh_raw_pass_size");
    int raw_offset = this->GetHeaderValueAsInt("rhr.rdb_hdr_off_data");
    this->computedPFileSize = raw_pass_size + raw_offset; 

    if ( string(this->FileName).size() == 0 || this->sha1Digest.size() != 40  || this->computedPFileSize == 0 ) {
        vtkErrorMacro("Missing FileName or valid SHA1 Digest.  Can Not generate DICOM Raw Data File. " );
        this->SetErrorCode(vtkErrorCode::UnknownError);
    } else {
        vector < string > file; 
        file.push_back( this->FileName ); 
        file.push_back( this->sha1Digest ); 
        file.push_back( "GE PFile" ); 
        this->associatedFiles.push_back( file ); 
    }

    this->dcmHeader->SetValue(
        "InstanceNumber",
        1 
    );

    vtkstd::string dcmDate = svkGEPFileMapper::ConvertGEDateToDICOM( this->pfMap["rhr.rh_scan_date"][3] ); 

    this->dcmHeader->SetValue(
        "ContentDate",
        svkImageReader2::RemoveSlashesFromDate( &dcmDate )
    ); 


    time_t time = this->GetHeaderValueAsInt("rhe.ex_datetime");

    //convert to Pacific time:  subtract 8 hours
    struct tm * timeinfo;
    timeinfo = localtime ( &time );
    bool isDaylightSavingsTime = timeinfo->tm_isdst;
    if ( isDaylightSavingsTime ) { 
        time += 7 * 60 * 60;
    } else {
        time += 8 * 60 * 60;
    }
    char timeBuf[80];
    strftime (timeBuf, 80,"%H%M", timeinfo);

    this->dcmHeader->SetValue(
        "ContentTime",
        string(timeBuf)
    ); 


    for ( int fileNum = 0; fileNum < this->associatedFiles.size(); fileNum++ ) {

        ifstream* pFile = new ifstream();
        pFile->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        try {

            pFile->open( (this->associatedFiles[fileNum][0]).c_str(), ios::binary);

            pFile->seekg(0, ios::end);
            // get-ptr position is now same as file size
            ios::pos_type pfileSize = pFile->tellg();  
            cout << "PFILE SIZE = " << static_cast<int>(pfileSize) << endl;

            //  for the pfile, make sure the file size is as expceted: 
            if ( this->associatedFiles[fileNum][2].compare("GE PFile") == 0 ) { 
                if ( static_cast<int>(pfileSize) != this->computedPFileSize ) { 
                    cout << "GEPFile Size on disk doesn't match expectation based on header info: " << endl; 
                    cout << pfileSize << " vs " << this->computedPFileSize  << endl; 
                    this->SetErrorCode( vtkErrorCode::PrematureEndOfFileError); 
                    continue; 
                }
            }

            void* pfileBuffer = new char[ static_cast<int>(pfileSize) ]; 
            pFile->seekg(0, ios::beg);
            pFile->read( static_cast<char*>(pfileBuffer), pfileSize );

            this->dcmHeader->SetValue(
                "SVK_PRIVATE_TAG",
                "SVK_PRIVATE_CREATOR"
            );

            this->dcmHeader->InsertEmptyElement( "SVK_FILE_SET_SEQUENCE" );

            this->dcmHeader->AddSequenceItemElement(
                "SVK_FILE_SET_SEQUENCE",
                fileNum,
                "SVK_FILE_TYPE",
                this->associatedFiles[fileNum][2]
            );

            this->dcmHeader->AddSequenceItemElement(
                "SVK_FILE_SET_SEQUENCE",
                fileNum,
                "SVK_FILE_NAME",
                this->associatedFiles[fileNum][0]
            );

            this->dcmHeader->AddSequenceItemElement(
                "SVK_FILE_SET_SEQUENCE",
                fileNum,
                "SVK_FILE_SHA1_DIGEST",
                this->associatedFiles[fileNum][1]
            );

            this->dcmHeader->AddSequenceItemElement(
                "SVK_FILE_SET_SEQUENCE",
                fileNum,
                "SVK_FILE_NUM_BYTES",
                static_cast<int>(pfileSize) 
            );

            this->dcmHeader->AddSequenceItemElement(
                "SVK_FILE_SET_SEQUENCE",
                fileNum,
                "SVK_FILE_CONTENTS",
                static_cast<float*>(pfileBuffer),
                pfileSize/sizeof(float) 
            );

        } catch (ifstream::failure e) {
            cout << "ERROR: Exception opening/reading file " << this->associatedFiles[fileNum][0] << " => " << e.what() << endl;
            this->SetErrorCode( vtkErrorCode::CannotOpenFileError );
        }

        pFile->close();
    }

}


/*!
 *
 */
void svkDICOMRawDataWriter::SetSHA1Digest( string sha1Digest )
{
    this->sha1Digest = sha1Digest; 
}


/*!
 *  returns the value for the specified key as an int.
 */
int svkDICOMRawDataWriter::GetHeaderValueAsInt(vtkstd::string key)
{

    istringstream* iss = new istringstream();
    int value;

    iss->str( this->pfMap[key][3] );
    *iss >> value;
    delete iss;
    return value;
}



/*!
 *
 */
int svkDICOMRawDataWriter::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(svkDICOMRawDataWriter::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
    return 1;
}


