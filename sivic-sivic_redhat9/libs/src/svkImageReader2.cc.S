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



#include <svkImageReader2.h>
#include <vtkInformation.h>
#include <vtkInstantiator.h>
#include <vtkGlobFileNames.h>
#include <vtkSortFileNames.h>
#include <vtkStringArray.h>
#include <svkMriImageData.h>
#include <svkMrsImageData.h>
#include <svkImageReaderFactory.h>


using namespace svk;


//vtkCxxRevisionMacro(svkImageReader2, "$Rev$");


/*!
 *
 */
svkImageReader2::svkImageReader2()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() );

    vtkInstantiator::RegisterInstantiator("svkMriImageData", svkMriImageData::NewObject);
    vtkInstantiator::RegisterInstantiator("svkMrsImageData", svkMrsImageData::NewObject);

    this->dataArray = NULL;
    this->readOneInputFile = false;
    this->onlyGlobFiles = false;
    this->onlyReadHeader = false;

}


/*!
 *
 */
svkImageReader2::~svkImageReader2()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() );

    if ( this->GetDebug() ) {
        cout << "+++++++++++++++++++++++++++++++++++++++++" << endl;
        cout << "delete image reader 2" << endl;
        cout << "+++++++++++++++++++++++++++++++++++++++++" << endl;
    }

    if (this->dataArray != NULL) {
        this->dataArray->Delete();
        this->dataArray = NULL;
    }

}


/*!
 *  Only read the one specified input file.  Otherwise the default 
 *  behavior is to read all the files in the group.
 */
void svkImageReader2::OnlyReadOneInputFile()
{
    this->readOneInputFile = true;
}


/*!
 *  Only list the files in the data set, then exit. 
 */
void svkImageReader2::OnlyGlobFiles()
{
    this->onlyGlobFiles = true; 
}


/*!
 *  Returns the file root without extension (will include any path elements)
 */
vtkstd::string svkImageReader2::GetFileRoot(const char* fname)
{
    vtkstd::string volumeFileName(fname);

    size_t dotPosition;
    dotPosition = volumeFileName.find_last_of( "." );
    if ( dotPosition == string::npos) {
        dotPosition = 0; 
    }

    size_t lastPath = volumeFileName.find_last_of("/");
    if ( lastPath == string::npos) {
        lastPath = 0; 
    }

    vtkstd::string fileRoot( fname );

    // If there is an extension remove it
    if( dotPosition > lastPath ) {
        fileRoot = volumeFileName.substr(0, dotPosition);
    } 

    return fileRoot;
}


/*!
 *  Returns the file root without extension
 *  or NULL if no extension found. 
 */
vtkstd::string svkImageReader2::GetFileExtension(const char* fname)
{
    vtkstd::string volumeFileName(fname);

    //  should disregard leading dots in path, so first get 
    //  substring following last "/", if it exists. 
    size_t pathPosition = volumeFileName.find_last_of( "/" );
    if ( pathPosition != vtkstd::string::npos ) {
        volumeFileName.assign( volumeFileName.substr(pathPosition + 1) );
    }

    size_t position = volumeFileName.find_last_of( "." );
    vtkstd::string fileExtension(""); 
    if ( position != string::npos && position != 0) {
        fileExtension.assign( volumeFileName.substr(position + 1) );
    } 

    return fileExtension;
}


/*!
 *  Returns the file path:  everything before the last "/".   
 */
vtkstd::string svkImageReader2::GetFilePath(const char* fname)
{
    vtkstd::string volumeFileName(fname);
    size_t position;
    position = volumeFileName.find_last_of( "/" );
    vtkstd::string filePath; 
    if ( position != vtkstd::string::npos ) {
        filePath.assign( volumeFileName.substr(0, position) );
    } else {
        filePath.assign( "." );
    }
    return filePath;
}


/*!
 *  Returns the file name without path:  everything after the last "/".   
 */
vtkstd::string svkImageReader2::GetFileNameWithoutPath(const char* fname)
{
    vtkstd::string volumeFileName(fname);
    size_t position;
    position = volumeFileName.find_last_of( "/" );
    vtkstd::string fileNameNoPath; 
    if ( position != vtkstd::string::npos ) {
        fileNameNoPath.assign( volumeFileName.substr(position + 1) );
    } else {
        fileNameNoPath.assign( fname );
    }
    return fileNameNoPath;
}


/*!
 *  Return the file size.
 */
long svkImageReader2::GetFileSize(ifstream* fs)
{
    long begin;
    long end;
    fs->seekg(ios::beg);
    begin = fs->tellg();
    fs->seekg (0, ios::end);
    end = fs->tellg();
    fs->seekg(ios::beg);
    return end - begin;
}


/*
 *  Strips leading and trailing white space from string
 */
vtkstd::string svkImageReader2::StripWhite(vtkstd::string in)
{
    vtkstd::string stripped;
    vtkstd::string stripped_leading(in);
    size_t firstNonWhite;
    size_t lastWhite;

    //  Remove leading spaces:
    firstNonWhite = in.find_first_not_of(" \t");
    if (firstNonWhite != vtkstd::string::npos) {
        stripped_leading.assign( in.substr(firstNonWhite) );
    }

    //  Remove trailing spaces:
    lastWhite = stripped_leading.find_last_of(" \t");
    while ( (lastWhite != vtkstd::string::npos) && (lastWhite  == stripped_leading.length() - 1) ) {
        stripped_leading.assign( stripped_leading.substr(0, lastWhite) );
        lastWhite = stripped_leading.find_last_of(" \t");
    }

    stripped = stripped_leading;

    return stripped;

}


/*!
 *  Use svkDcmHeader content to set up Output Information about the
 *  target svkImageData object.
 */
void svkImageReader2::SetupOutputInformation()
{

    svkDcmHeader* dcmHdr = this->GetOutput()->GetDcmHeader();
    if( this->GetFileNames() != NULL && this->GetFileNames()->GetValue(0) != NULL ) {
        this->GetOutput()->SetSourceFileName( this->GetFileNames()->GetValue(0) );
    }

    //  ============================
    //  Set Dimensionality
    //  ============================
    this->SetFileDimensionality(3);

    //  ============================
    //  Set Extent
    //  ============================
    this->SetupOutputExtent(); 

    //  ============================
    //  Set Voxel Spacing (size)
    //  ============================
    double spacing[3];
    dcmHdr->GetPixelSpacing(spacing);
    this->SetDataSpacing(spacing);

    //  ============================
    //  Set Origin
    //  ============================
    double origin[3];
    dcmHdr->GetOrigin(origin);

    //  For MRS data the vtk point origin (origin of first point in data set is corner of voxel) 
    //  is 1/2 voxel away from DICOM origin. 
    if ( strcmp( this->GetOutput()->GetClassName(), "svkMrsImageData") == 0 ) {
        double pixelSize[3];
        dcmHdr->GetPixelSize(pixelSize);

        double dcos[3][3];
        dcmHdr->GetDataDcos(dcos); 

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                origin[i] -= (pixelSize[j]/2) * dcos[j][i];
            }
        }
    }

    this->SetDataOrigin( origin );

    this->SetupOutputScalarData(); 

    this->vtkImageReader2::ExecuteInformation();
}


/*!
 *  
 */
void svkImageReader2::SetupOutputExtent()
{
    int numVoxels[3];
    numVoxels[0] = this->GetOutput()->GetDcmHeader()->GetIntValue("Columns");
    numVoxels[1] = this->GetOutput()->GetDcmHeader()->GetIntValue("Rows");
    numVoxels[2] = this->GetOutput()->GetDcmHeader()->GetNumberOfSlices();

    if ( strcmp( this->GetOutput()->GetClassName(), "svkMrsImageData") == 0 ) {
        this->SetDataExtent(
            0,
            numVoxels[0],
            0,
            numVoxels[1],
            0,
            numVoxels[2]
        );
    } else if ( strcmp(this->GetOutput()->GetClassName(), "svkMriImageData") == 0 ) {
        this->SetDataExtent(
            0,
            numVoxels[0] - 1,
            0,
            numVoxels[1] - 1,
            0,
            numVoxels[2] - 1
        );
    }
}


/*!
 *  Set scalar data information for MRI data: 
 */
void svkImageReader2::SetupOutputScalarData()
{

    //  only MRI data has scalar data
    if ( strcmp(this->GetOutput()->GetClassName(), "svkMriImageData") == 0 ) {

        //  ============================
        //  Set data type:
        //  ============================
        vtkInformation* outInfo = this->GetOutput()->GetInformation(); 
            
        if (  this->GetFileType() == svkDcmHeader::UNSIGNED_INT_1 ) {
            this->SetDataScalarTypeToUnsignedChar();
            this->dataArray = vtkUnsignedCharArray::New();
            this->GetOutput()->GetDcmHeader()->SetPixelDataType( svkDcmHeader::UNSIGNED_INT_1 );
            vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_UNSIGNED_CHAR, 1);
        } else if (  this->GetFileType() == svkDcmHeader::UNSIGNED_INT_2 ) {
            this->SetDataScalarTypeToUnsignedShort();
            this->dataArray = vtkUnsignedShortArray::New();
            this->GetOutput()->GetDcmHeader()->SetPixelDataType( svkDcmHeader::UNSIGNED_INT_2 );
            vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_UNSIGNED_SHORT, 1);
        } else if (  this->GetFileType() == svkDcmHeader::SIGNED_INT_2 ) {
            this->SetDataScalarTypeToShort();
            this->dataArray = vtkShortArray::New();
            this->GetOutput()->GetDcmHeader()->SetPixelDataType( svkDcmHeader::SIGNED_INT_2 );
            vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_SHORT, 1);
        } else if (  this->GetFileType() == svkDcmHeader::SIGNED_FLOAT_4 ) {
            this->SetDataScalarTypeToFloat();
            this->dataArray = vtkFloatArray::New();
            this->GetOutput()->GetDcmHeader()->SetPixelDataType( svkDcmHeader::SIGNED_FLOAT_4 );
            vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_FLOAT, 1);
        } else if (  this->GetFileType() == svkDcmHeader::SIGNED_FLOAT_8 ) {
            this->SetDataScalarTypeToDouble();
            this->dataArray = vtkDoubleArray::New();
            this->GetOutput()->GetDcmHeader()->SetPixelDataType( svkDcmHeader::SIGNED_FLOAT_8 );
            vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_DOUBLE, 1);
        } else {
            vtkErrorWithObjectMacro( this, "Unsupported data type: " << this->GetFileType() );
        }

        this->dataArray->SetName("pixels");
        this->SetNumberOfScalarComponents(1);

    } 
}



/*!
 *
 */
int svkImageReader2::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), this->GetImageDataInput(0)->GetClassName() );
    return 1;
}


/*!
 *
 */
svkImageData* svkImageReader2::GetOutput()
{
    return this->GetOutput(0);
}


/*!
 *
 */
svkImageData* svkImageReader2::GetOutput(int port)
{
    return svkImageData::SafeDownCast(this->GetOutputDataObject(port));
}



/*!
 *  Remove slashes from idf date and reorder for DICOM compliance:
 *  07/25/2007 -> 20070725
 */
vtkstd::string svkImageReader2::RemoveSlashesFromDate(vtkstd::string* slashDate)
{

    //  string should not be empty or "blank"
    size_t pos = slashDate->find_first_not_of(" ");

    if ( slashDate->length() > 0 && pos != string::npos ) {
        size_t delim;
        delim = slashDate->find_first_of('/');
        vtkstd::string month = slashDate->substr(0, delim);
        month = StripWhite(month);
        if (month.size() != 2) {
            month = "0" + month;
        }

        vtkstd::string dateSub;
        dateSub = slashDate->substr(delim + 1);
        delim = dateSub.find_first_of('/');
        vtkstd::string day = dateSub.substr(0, delim);
        day = StripWhite(day);
        if (day.size() != 2) {
            day = "0" + day;
        }

        dateSub = dateSub.substr(delim + 1);
        delim = dateSub.find_first_of('/');
        vtkstd::string year = dateSub.substr(0, delim);
        year = StripWhite(year);

        return year+month+day;
    } else { 
        return ""; 
    }

}


/*!
 *  Utility function to read a single line from the volume file.
 */
void svkImageReader2::ReadLine(ifstream* hdr, istringstream* iss)
{
    char line[256];
    iss->clear();
    hdr->getline(line, 256);
    iss->str(vtkstd::string(line));
}


/*!
 *  Utility function to read a single line from the volume file.
 *  and ignore all characters up to the specified delimiting character.
 */
void svkImageReader2::ReadLineIgnore(ifstream* hdr, istringstream* iss, char delim)
{
    this->ReadLine(hdr, iss);
    iss->ignore(256, delim);
}


/*!
 *  Utility function for extracting a substring with white space removed from LHS.
 */
vtkstd::string svkImageReader2::ReadLineSubstr(ifstream* hdr, istringstream* iss, int start, int stop)
{
    vtkstd::string temp;
    vtkstd::string lineSubStr;
    size_t firstNonSpace;
    this->ReadLine(hdr, iss);
    try {
        temp.assign(iss->str().substr(start,stop));
        firstNonSpace = temp.find_first_not_of(' ');
        if (firstNonSpace != vtkstd::string::npos) {
            lineSubStr.assign( temp.substr(firstNonSpace) );
        }
    } catch (const exception& e) {
        cout <<  e.what() << endl;
    }
    return lineSubStr;
}


/*!
 *  Read the value part of a delimited key value line in a file:
 */
vtkstd::string svkImageReader2::ReadLineValue( ifstream* hdr, istringstream* iss, char delim)
{

    vtkstd::string value;
    this->ReadLine( hdr, iss );
    try {

        vtkstd::string line;
        line.assign( iss->str() );

        size_t delimPos = line.find_first_of(delim);
        vtkstd::string delimitedLine;
        if (delimPos != vtkstd::string::npos) {
            delimitedLine.assign( line.substr( delimPos + 1 ) );
        } else {
            delimitedLine.assign( line );
        }

        // remove leading white space:
        size_t firstNonSpace = delimitedLine.find_first_not_of( ' ' );
        if ( firstNonSpace != vtkstd::string::npos) {
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
 *
 */
svkDcmHeader* svkImageReader2::GetDcmHeader( const char* fileName)
{
    this->SetFileName( fileName );
    return this->GetOutput()->GetDcmHeader();
}


/*!
 *  Appends algo info to provenance record.
 */
void svkImageReader2::SetProvenance()
{
    svkImageData::SafeDownCast( this->GetOutput() )->GetProvenance()->AddAlgorithm( this->GetClassName() );
}


/*!
 *  *  If only the idf header is to be read.  Sometimes this acts as a template for processing, but 
 *   *  there isn't an associated data file.  
 *    */
void svkImageReader2::OnlyReadHeader(bool onlyReadHeader)
{
    this->onlyReadHeader = onlyReadHeader;
    if (this->GetDebug()) {
        vtkWarningWithObjectMacro(this, "onlyReadHeader: " << this->onlyReadHeader);
    }
}


/*
 *  Globs file names for multi-file data formats and sets them into a vtkStringArray 
 *  accessible via the GetFileNames() method. 
 */
void svkImageReader2::GlobFileNames()
{
    string fileName( this->GetFileName() );
    string fileExtension( this->GetFileExtension( this->GetFileName() ) );
    string filePath( this->GetFilePath( this->GetFileName() ) );
    vtkGlobFileNames* globFileNames = vtkGlobFileNames::New();
    globFileNames->SetDirectory( filePath.c_str() ); 
    if ( fileExtension.compare("") == 0 ) {
        globFileNames->AddFileNames(  "*" );
    } else {
        globFileNames->AddFileNames( string( "*." + fileExtension).c_str() );
    }

    vtkSortFileNames* sortFileNames = vtkSortFileNames::New();
    sortFileNames->GroupingOn();
    sortFileNames->SetInputFileNames( globFileNames->GetFileNames() );
    sortFileNames->NumericSortOn();
    sortFileNames->Update();

    //  =================================================================
    //  If globed file names are not similar, use only the 0th group. 
    //  If there is one group that group is used.
    //  If there are multiple groups and the input file cannot be associated with a group then use input filename only.
    //  If readOneInputFile is set, the groupsToUse remains -1 and only the literal input file is read.  
    //  If a group is going to be loaded, check that the series descriptions are consistent. 
    //  =================================================================
    
    //  by default do not use a group, but rather just the single input file name (groupToUse = -1).  
    int groupToUse = -1;    
cout << "BOOBOO 1 " << this->readOneInputFile << endl;
    if ( this->readOneInputFile == false ) {
        if (sortFileNames->GetNumberOfGroups() > 1 ) {
cout << "BOOBOO 2 " << this->readOneInputFile << endl;

            //  Get the group the selected file belongs to:
            vtkStringArray* group;
            for (int k = 0; k < sortFileNames->GetNumberOfGroups(); k++ ) {
cout << "BOOBOO 3 " << this->readOneInputFile << endl;
                group = sortFileNames->GetNthGroup(k); 
                for (int i = 0; i < group->GetNumberOfValues(); i++) {
cout << "BOOBOO 4 " << this->readOneInputFile << endl;
                    //  The glob was already limited to the directory where the input file was located
                    //  so to avoid any further relative vs absolute path issues just compare the
                    //  returned file names without path:
                    string groupFile ( this->GetFileNameWithoutPath( group->GetValue(i) ) ); 
                    if( this->GetDebug() ) {
                        cout << "Group: " << groupFile << endl;
                    }
                    if ( this->GetFileNameWithoutPath(fileName.c_str()).compare( groupFile ) == 0 ) {
                        groupToUse = k; 
                        break; 
                    }
                }
            }
        } else {
            groupToUse = 0;     
        }
    }
    
    if( groupToUse != -1 ) {

        vtkStringArray* group = sortFileNames->GetNthGroup(groupToUse); 
        int numFilesInGroup = group->GetNumberOfValues();
        cout << "NUM VALUES: " <<  numFilesInGroup << endl;

        //  Before setting the file names for this group, confirm that if more than
        //  one file in the group that they all have the same series description
        //  all have the same series description    
        if ( numFilesInGroup > 1 ) {   
            
            string referenceSeriesDescription = this->GetFileSeriesDescription( fileName ); 
            cout << "REF SERIES DESCRIPTION: " << referenceSeriesDescription << endl; 
      
            vtkStringArray* seriesGroup = vtkStringArray::New();  
            for (int i = 0; i < numFilesInGroup; i++) {
                string groupFileName = sortFileNames->GetNthGroup( groupToUse )->GetValue(i); 
                string seriesDescription = this->GetFileSeriesDescription( groupFileName ); 
                cout << "SERIES DESCRIPTION GROUP: " << seriesDescription << endl;
                if ( seriesDescription.compare( referenceSeriesDescription ) == 0 ) {
                    seriesGroup->InsertNextValue( groupFileName ); 
                    cout << "GROUP FN: " << groupFileName<< endl;
                }
            }
            this->SetFileNames( seriesGroup ); 
            seriesGroup->Delete(); 
    } else {
        this->SetFileNames( sortFileNames->GetNthGroup( groupToUse ) );
    }

    } else {
        vtkStringArray* inputFile = vtkStringArray::New();
        inputFile->InsertNextValue( fileName.c_str() );
        this->SetFileNames( inputFile );
        inputFile->Delete();
    }

    if (this->GetDebug() || ( this->onlyGlobFiles == true ) ) {
        for (int i = 0; i < this->GetFileNames()->GetNumberOfValues(); i++) {
            cout << "FN: " << this->GetFileNames()->GetValue(i) << endl;
        }
    }

    globFileNames->Delete(); 
    sortFileNames->Delete(); 

    if ( this->onlyGlobFiles) {
        cout << "Just Glob Files, exiting now" << endl;
        exit(0); 
    }
}


/*!
 *  get the series description for this file for globbing purposes: 
 */
string svkImageReader2::GetFileSeriesDescription( string fileName ) 
{

    svkImageReader2::ReaderType readerType = this->GetReaderType(); 
    svkImageReader2* readerLocal = svkImageReaderFactory::CreateImageReader2( readerType );
    readerLocal->SetFileName( fileName.c_str() );
    readerLocal->OnlyReadOneInputFile();
    readerLocal->OnlyReadHeader( true ); 
    readerLocal->Update();
cout << "HEADER: " << endl; readerLocal->GetOutput()->GetDcmHeader()->PrintDcmHeader(); 
cout << "HEADER: " << endl; svkGEPFileReader::SafeDownCast(readerLocal)->PrintHeader(); 
    string seriesDescription =  readerLocal->GetOutput()->GetDcmHeader()->GetStringValue( "SeriesDescription" ); 
    readerLocal->Delete(); 
    return seriesDescription; 
}
      
