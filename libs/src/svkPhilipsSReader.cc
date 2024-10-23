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


#include <svkPhilipsSReader.h>
#include <svkPhilipsSMapper.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include <algorithm>    // std::remove

#include <sys/stat.h>

using namespace svk;


//vtkCxxRevisionMacro(svkPhilipsSReader, "$Rev$");
vtkStandardNewMacro(svkPhilipsSReader);


/*!
 *
 */
svkPhilipsSReader::svkPhilipsSReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkPhilipsSReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->sdatFile = NULL;
    this->sparFile = NULL;
    this->mapper   = NULL;

//  data is written as big endian:
//    this->SetDataByteOrderToBigEndian();

    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgressCallback );
    this->progressCallback->SetClientData( (void*)this );
}


/*!
 *
 */
svkPhilipsSReader::~svkPhilipsSReader()
{

    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if ( this->sdatFile != NULL )  {
        delete sdatFile; 
        this->sdatFile = NULL; 
    }

    if ( this->sparFile != NULL )  {
        delete sparFile; 
        this->sparFile = NULL; 
    }

    if ( this->mapper != NULL )  {
        mapper->Delete(); 
        this->mapper = NULL; 
    }

    if( this->progressCallback != NULL ) {
        this->progressCallback->Delete();
        this->progressCallback = NULL;
    }
}



/*!
 *  Check to see if the extension indicates a Philips SPAR/SDAT file.  If so, try 
 *  to open the file for reading.  If that works, then return a success code. 
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkPhilipsSReader::CanReadFile(const char* fname)
{


    //  If file has an extension then check it:
    string fileExtension = this->GetFileExtension( fname );

    if( ! fileExtension.empty() ) {

        if (
            fileExtension.compare( "sdat" ) == 0 ||
            fileExtension.compare( "spar" ) == 0 || 
            fileExtension.compare( "SDAT" ) == 0 ||
            fileExtension.compare( "SPAR" ) == 0
        )  {

            FILE *fp = fopen(fname, "rb");
            if (fp) {
                fclose(fp);

                vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's a Philips S File: " << fname );
                return 1;
            }

        } else {
            vtkDebugMacro(
                << this->GetClassName() << "::CanReadFile(): It's NOT a Philips S File: " << fname 
            );
            return 0;
        }
    } else {
        vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): is NOT a valid Philips S file: " << fname);
        return 0;
    }
}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called after ExecuteInformation()
 */
void svkPhilipsSReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
{

    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );
    svkImageData* data = svkImageData::SafeDownCast( this->AllocateOutputData(output, outInfo) );

    if ( this->FileName ) {
        this->mapper->ReadSDATFile( this->FileName, data );
    }

    double dcos[3][3];
    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
    this->GetOutput()->SetDcos(dcos);

    //  SetNumberOfIncrements is supposed to call this, but only works if the data has already
    //  been allocated. but that requires the number of components to be specified.
    this->GetOutput()->GetIncrements();

    if (this->GetDebug()) {
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }

}


/*!
 *  Side effect of Update() method.  Used to initialize the svkDcmHeader member of 
 *  the target svkImageData object and uses the header to set up the Output Informatin.
 *  Called before ExecuteData()
 */
void svkPhilipsSReader::ExecuteInformation()
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
void svkPhilipsSReader::InitDcmHeader()
{

    vtkDebugMacro( << this->GetClassName() << "::InitDcmHeader()" );

    svkMRSIOD* iod = svkMRSIOD::New();
    iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    iod->InitDcmHeader();

    //  Read the fid header into a map of values used to initialize the
    //  DICOM header. 
    this->ParseSDAT(); 

    //  should be a mapper factory to get psd specific instance:
    if ( this->mapper != NULL )  {
        mapper->Delete(); 
        this->mapper = NULL; 
    }
    this->mapper = this->GetPhilipsSMapper();
    this->mapper->AddObserver(vtkCommand::ProgressEvent, progressCallback);

    //  all the IE initialization modules would be contained within the mapper
    this->mapper->InitializeDcmHeader(
        this->sparMap,
        this->GetOutput()->GetDcmHeader(),
        iod,
        this->GetSwapBytes()
    );
    this->RemoveObserver(progressCallback);


    if (this->GetDebug()) {
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }

    iod->Delete();
}


/*!
 *  Create an svkGEPFileMapper of the appropriate type for the present pfile
 */
svkPhilipsSMapper* svkPhilipsSReader::GetPhilipsSMapper()
{
    svkPhilipsSMapper* aMapper = NULL;

    string scanID = this->sparMap["scan_id"];
    if ( 
        ( scanID.find("PRESS") != string::npos ) || 
        ( scanID.find("csifid") != string::npos ) ||
        ( scanID.find("CSI_2D") != string::npos ) 
    ) {

        aMapper = svkPhilipsSMapper::New();

    }  else {
        cout << "Not a supported Philips sequence" << endl;
    }

    return aMapper; 
}


/*!
 *  Returns the file type enum 
 */
svkDcmHeader::DcmPixelDataFormat svkPhilipsSReader::GetFileType()
{
    svkDcmHeader::DcmPixelDataFormat format = svkDcmHeader::SIGNED_FLOAT_4;

    return format; 
}


/*
 *  Read SDAT header blocks into a string STL map for use during initialization 
 *  of DICOM header by Init*Module methods. 
 *  If a procpar file is present in the directory, parse that as well. 
 */
void svkPhilipsSReader::ParseSDAT()
{

    string sdatFileName( this->GetFileName() );  
    string sdatFilePath( this->GetFilePath( this->GetFileName() ) );  

    //string sparFile = sdatFilePath; 
    //sparFile       += "/"; 
    string sparFile = this->GetFileRoot( this->GetFileName() ) + ".SPAR";

    try { 

        this->ParseSPAR(sparFile);
        if (this->GetDebug()) {
            this->PrintSparKeyValuePairs();
        }

    } catch (const exception& e) {
        cerr << "ERROR opening or reading Philips S files (" << sparFile << "): " << e.what() << endl;
    }

}


/*! 
 *  Parses the spar file. 
 *  The format is:
 *      keyName : value
 */

void svkPhilipsSReader::ParseSPAR( string path )
{

    try {

        this->sparFile = new ifstream();
        this->sparFile->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        this->sparFile->open( path.c_str(), ifstream::in );
        if ( ! this->sparFile->is_open() ) {
            throw runtime_error( "Could not open spar file: " + path );
        }

        this->sparFileSize = this->GetFileSize( this->sparFile );

        this->sparFile->clear();
        this->sparFile->seekg( 0, ios_base::beg );

        while (! this->sparFile->eof() ) {
            this->GetSparKeyValuePair(); 
        }

        this->sparFile->close();

    } catch (const exception& e) {
        cerr << "ERROR opening or reading Philips spar file( " << path << ": " << e.what() << endl;
        exit(1); 
    }
    //this->PrintSparKeyValuePairs(); 

}


/*! 
 *  Utility function to read key/values from spar file 
 *  and set the delimited key/value pair into the stl map.  
 *  \return  0 if found key value pair, 1 otherwise
 */
int svkPhilipsSReader::GetSparKeyValuePair( )
{

    int status = 1;

    istringstream* iss = new istringstream();

    string keyString;
    string valueString;

    try {
        if ( this->sparFile->tellg() <= this->sparFileSize ) {

            status = this->ReadLineKeyValue( this->sparFile, iss, ':', &keyString, &valueString); 
            if ( status == 0 ) {
                valueString.erase( remove(valueString.begin(), valueString.end(), '\r'), valueString.end() );
                this->sparMap[keyString] = valueString; 
            }

        } else {
            this->sparFile->seekg(0, ios::end);
        }

    } catch (const exception& e) {
        if (this->GetDebug()) {
            cout <<  "ERROR reading line: " << e.what() << endl;
        }
    }

    delete iss;
    return status;
}


/*!
 *  Prints the key value pairs parsed from the header. 
 */
void svkPhilipsSReader::PrintSparKeyValuePairs()
{

    //  Print out key value pairs parsed from header:
    map< string, string >::iterator mapIter;
    for ( mapIter = this->sparMap.begin(); mapIter != this->sparMap.end(); ++mapIter ) {
        cout << this->GetClassName() << " " << mapIter->first << " = " << mapIter->second << endl;
    }
}



/*!
 *
 */
int svkPhilipsSReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}


/*!
 * 
 */
void svkPhilipsSReader::SetProgressText( string progressText ) 
{
    this->progressText = progressText;
}


/*!
 * 
 */
void svkPhilipsSReader::UpdateProgress(double amount)
{
    this->InvokeEvent(vtkCommand::ProgressEvent,static_cast<void *>(&amount));
}   


/*!
 * 
 */
void svkPhilipsSReader::UpdateProgressCallback(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<svkPhilipsSReader*>(thisObject)->UpdateProgress(*(double*)(callData)); 
}
