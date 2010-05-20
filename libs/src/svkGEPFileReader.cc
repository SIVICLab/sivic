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


#include <svkGEPFileReader.h>


using namespace svk;


vtkCxxRevisionMacro(svkGEPFileReader, "$Rev$");
vtkStandardNewMacro(svkGEPFileReader);


/*!
 *
 */
svkGEPFileReader::svkGEPFileReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkGEPFileReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->gepf= NULL; 
    this->mapper = NULL; 
    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgressCallback );
    this->progressCallback->SetClientData( (void*)this );

}



/*!
 *
 */
svkGEPFileReader::~svkGEPFileReader()
{

    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if ( this->gepf != NULL )  {
        delete gepf;
        this->gepf = NULL;
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
 *  Check to see if this is a GE pfile.  If so, try
 *  to open the file for reading.  If that works, then return a success code.
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkGEPFileReader::CanReadFile(const char* fname)
{

    this->gepf = new ifstream();
    this->gepf->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
    this->gepf->open( fname, ios::binary);
    bool    isGEPFile = false; 

    float pfileVersion = 0; 

    if ( this->gepf->is_open() ) {

        this->pfileVersion = this->GetPFileVersion(); 
        
        if ( this->pfileVersion ) {

            this->InitOffsetsMap( this->pfileVersion ); 

            if ( this->pfileVersion < 12 ) {
                string geLogo = this->GetFieldAsString( "rhr.rh_logo" );
                if ( geLogo.find("GE") != string::npos) {
                    isGEPFile = true; 
                }
            } else {
                string offset = this->GetFieldAsString( "rhr.rdb_hdr_off_data" );
                if ( offset.compare("66072") == 0 || offset.compare("145908") == 0 || offset.compare("149788") == 0 ) {
                    isGEPFile = true; 
                }
            }
        }
        
        this->gepf->close();
    }

    if ( isGEPFile ) {
        vtkDebugMacro(
            << this->GetClassName() << "::CanReadFile(): It's a GE " 
            << this->pfileVersion << " PFile: " << fname 
        );
        return 1;
    } else {
        vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's NOT a GE PFile: " << fname);
        return 0;
    }

}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called before ExecuteData()
 */
void svkGEPFileReader::ExecuteInformation()
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

}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called after ExecuteInformation()
 */
void svkGEPFileReader::ExecuteData(vtkDataObject* output)
{

    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );

    svkImageData* data = svkImageData::SafeDownCast( this->AllocateOutputData(output) );

    vtkDebugMacro( << this->GetClassName() << " FileName: " << this->FileName );
    this->mapper->AddObserver(vtkCommand::ProgressEvent, progressCallback);

    this->mapper->ReadData(this->GetFileName(), data);
    this->mapper->RemoveObserver(progressCallback);

    //  Set the orientation in the svkImageData object, synchronized from the dcm header:
    double dcos[3][3];
    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
    this->GetOutput()->SetDcos(dcos);

    //  SetNumberOfIncrements is supposed to call this, but only works if the data has already
    //  been allocated. but that requires the number of components to be specified.
    this->GetOutput()->GetIncrements();

}


/*!
 *  Initializes the svkDcmHeader adapter to a specific IOD type      
 *  and initizlizes the svkDcmHeader member of the svkImageData 
 *  object.    
 */
void svkGEPFileReader::InitDcmHeader()
{
    vtkDebugMacro( << this->GetClassName() << "::InitDcmHeader()" );

    //  This could be MRI or MRS:  
    svkIOD* iod = svkMRSIOD::New();
    iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    iod->InitDcmHeader();
    iod->Delete();

    //  Parse the GE PFile into a string map, the
    //  contents of which will be used to initialize
    //  the svkImageData's DICOM header.
    this->ReadGEPFile(); 


    //  Fill in data set specific values using the appropriate mapper type:
    this->mapper = this->GetPFileMapper(); 

    //  all the IE initialization modules would be contained within the 
    this->mapper->InitializeDcmHeader( 
        this->pfMap, 
        this->GetOutput()->GetDcmHeader(), 
        this->pfileVersion 
    ); 

    if (this->GetDebug()) { 
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }
}


/*!
 *  Create an svkGEPFileMapper of the appropriate type for the present pfile 
 */
svkGEPFileMapper* svkGEPFileReader::GetPFileMapper() 
{
    svkGEPFileMapper* aMapper = NULL; 

    string psd = this->pfMap["rhi.psdname"][3];

    if ( psd.compare("PROBE-P") == 0 ) {
        aMapper = svkGEPFileMapper::New();
    } else {
        vtkErrorMacro("No PFile mapper available for " << psd );
        exit(1);
    }

    return aMapper;          
}


/*!
 *  Read GE Pfile  header fields into a string STL map for use during initialization
 *  of DICOM header by Init*Module methods.
 */
void svkGEPFileReader::ReadGEPFile()
{

    cout << "FN: " << this->GetFileName() << endl;

    try {

        //  Read in the GE Pfile Header:
        this->gepf = new ifstream();
        this->gepf->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        this->gepf->open( this->GetFileName(), ios::binary );

        //  Iterate through map, get offset and read the appropriate length at that offset.  fix endianness. 
        this->ParsePFile(); 

        this->gepf->close();     

    } catch (const exception& e) {
        cerr << "ERROR opening or reading GE P-File (" << this->GetFileName() << "): " << e.what() << endl;
    }

}


/*!
 *  Initializes the map of pfile offsets and data types, parses the pfile header
 *  and sets the field value in the map to a string representation of the header field. 
 *
 *  key        pfMap[key][0]    pfMap[key][1] pfMap[key][2] pfMap[key][3]
 *  ----------------------------------------------------------------------
 *  fieldName  nativeType       numElements   offset        stringValue
 */
void svkGEPFileReader::ParsePFile()
{

    this->pfileVersion = this->GetPFileVersion(); 
    this->InitOffsetsMap( this->pfileVersion ); 

    map< string, vector<string> >::iterator mapIter;
    string key; 
    for ( mapIter = this->pfMap.begin(); mapIter != this->pfMap.end(); ++mapIter ) {
        key = mapIter->first; 
        this->pfMap[key].push_back( this->GetFieldAsString( key ) );
    }

}


/*!
 *
 */
int svkGEPFileReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}


/*!
 *  Prints the key value pairs parsed from the header.
 */
void svkGEPFileReader::PrintOffsets()
{
    //  Print out key value pairs parsed from header:
    map< string, vector<string> >::iterator mapIter;
    for ( mapIter = this->pfMap.begin(); mapIter != this->pfMap.end(); ++mapIter ) {

        cout << this->GetClassName() << " " << mapIter->first << " = ";
    
        vector<string>::iterator it;
        for ( it = this->pfMap[mapIter->first].begin() ; it < this->pfMap[mapIter->first].end(); it++ ) {
            cout << " " << *it ;           
        }
        cout << endl;
    }
}


/*!
 *  Prints the key value pairs parsed from the header.
 */
void svkGEPFileReader::PrintKeyValuePairs()
{

    //  Print out key value pairs parsed from header:
    map< string, vector<string> >::iterator mapIter;
    for ( mapIter = this->pfMap.begin(); mapIter != this->pfMap.end(); ++mapIter ) {

        cout << this->GetClassName() << " " << mapIter->first << " = ";

        vector<string>::iterator it;
        for ( it = this->pfMap[mapIter->first].begin() ; it < this->pfMap[mapIter->first].end(); it++ ) {
            cout << " " << *it ;
        }
        cout << endl;
    }
}



/*!
 *  
 *  Initialize map of field names to dataType, size and offset.  This map is used to parse 
 *  GE Pfile: 
 *
 *  //  fieldName           nativeType  numElements offset stringValue
 *  //  ----------------------------------------------------------
 *  //  rhr.rh_rdbm_rev   , FLOAT_4,    1   ,       0       value
 *  //  rhr.rh_logo       , CHAR   ,    10  ,       34      value
 * 
 */
void svkGEPFileReader::InitOffsetsMap( float pfileVersion )
{
    string offsets = this->GetOffsetsString( pfileVersion );

    size_t delim;

    string fieldName;
    string val;

    while ( (delim = offsets.find(",")) != string::npos ) {

        fieldName = this->StripWhite( offsets.substr( 0, delim ) );

        offsets = offsets.substr( delim +1 );

        for ( int i = 0; i < 3; i++ ) {
            delim = offsets.find(",");
            val = offsets.substr( 0, delim );
            this->pfMap[fieldName].push_back(val);
            offsets = offsets.substr( delim +1 );
        }
    }

    this->PrintOffsets();
    this->PrintKeyValuePairs(); 
}


/*!
 *  Returns the file root without extension
 */
svkDcmHeader::DcmPixelDataFormat svkGEPFileReader::GetFileType()
{
    return svkDcmHeader::SIGNED_FLOAT_4;
}


/*!
 *
 *  Read from file, data of specified type and offset into a string.  
 */
string svkGEPFileReader::GetFieldAsString( string key )
{

    //  Seek to offset for the specified key:
    int offset;
    istringstream* issOffset = new istringstream();
    issOffset->str( this->pfMap[key][2] );
    *issOffset >> offset;
    delete issOffset; 
    this->gepf->seekg( offset , ios_base::beg );

    ostringstream ossValue;

    string type = this->StripWhite( this->pfMap[key][0] ); 

    int numElements;
    istringstream* iss = new istringstream();
    iss->str( this->pfMap[key][1] );
    *iss >> numElements;

    if ( type.compare("FLOAT_4") == 0) {
        float floatVal; 
        this->gepf->read( (char*)(&floatVal), 4 * numElements );
        ossValue << floatVal;
    } else if ( type.compare("INT_2") == 0 ) {
        short shortVal; 
        this->gepf->read( (char*)(&shortVal), 2 * numElements );
        ossValue << shortVal;
    } else if ( type.compare("UINT_2") == 0 ) {
        unsigned short ushortVal; 
        this->gepf->read( (char*)(&ushortVal), 2 * numElements );
        ossValue << ushortVal;
    } else if ( type.compare("INT_4") == 0) {
        int intVal; 
        this->gepf->read( (char*)(&intVal), 4 * numElements );
        ossValue << intVal;
    } else if ( type.compare("LINT_4") == 0) {
        int intVal; 
        this->gepf->read( (char*)(&intVal), 4 * numElements );
        ossValue << intVal;
    } else if ( type.compare("ULINT_4") == 0) {
        unsigned int uintVal; 
        this->gepf->read( (char*)(&uintVal), 4 * numElements );
        ossValue << uintVal;
    } else if ( type.compare("CHAR") == 0) {
        char* charVal = new char[numElements];  
        this->gepf->read( charVal, 1 * numElements );
        ossValue << charVal;
        delete [] charVal; 
    } else if ( type.compare("UID") == 0) {
        char* charVal = new char[numElements];  
        this->gepf->read( charVal, 1 * numElements );
        ossValue << charVal;
        delete [] charVal; 
        //  Uncompress UIDfields:
        string tmp = this->UncompressUID( ossValue.str().c_str() );  
        ossValue.clear();     
        ossValue.str("");     
        ossValue << tmp ; 
    } else {
        cout << "CAN NOT DETERMINE TYPE " << endl;
    }
    
    delete iss; 
    cout << key << ":" <<  ossValue.str() << endl; 

    return ossValue.str();
}


/*!
 *  The following code for uncompressing the UIDs may be directly 
 *  from GE.  Get clearance.  
 */
#define UID_FAIL 0
#define UID_OK 1
#define UID_LEN 64
#define RT_FOUR_BITS        0x0f
#define LT_FOUR_BITS        0xf0
#define DB_UID_LEN 32
int svkGEPFileReader::GEUncompressUID(unsigned char *short_uid, char *long_uid)
{
    int i, len;

    memset(long_uid, '\0', UID_LEN + 1);

    if((len = strlen((char *)short_uid)) > DB_UID_LEN)
    {
        return(UID_FAIL);
    }

    for(i = 0; i < len * 2; i++) /* expanding the uid */
    {
        /* get the proper value from the short_uid/compressed string */
        if(i % 2 == 0) /* an even number */
        {
            long_uid[i] = short_uid[i/2] & LT_FOUR_BITS;
            long_uid[i] >>= 4;
            long_uid[i] &= 0x0f;
        }
        else
        {
            long_uid[i] = short_uid[i/2] & RT_FOUR_BITS;
        }

        /* we have the proper value, now expand/decompress it */
        if(0x1 <= long_uid[i] && long_uid[i] <= 0xa)
        {
            long_uid[i] += '0' - 1;
        }
        else if (long_uid[i] == 0xb)
        {
            long_uid[i] = '.';
        }
        else if (long_uid[i] == 0x0)
        {
            break;  /* This is the end, no need to look further */
        }
        else
        {
            memset(long_uid, '\0', UID_LEN + 1);
            return(UID_FAIL);
        }
    }
    return(UID_OK);
}


/*
 *  Uncompresses the UID from the raw header. 
 */
string svkGEPFileReader::UncompressUID(const char* compressedUID)
{
    char uncompressedUID[BUFSIZ];
    this->GEUncompressUID((unsigned char*)compressedUID, uncompressedUID);
    return string(uncompressedUID);
}


/*!  
 *  Get the pfile version number from already opened pfile file. 
 *  Returns 0 if version can't be determined. 
 */
float svkGEPFileReader::GetPFileVersion() 
{
    
    float version = 0;
    float rdbmRev;;

    if ( this->gepf->is_open() ) {
        gepf->seekg( 0, ios_base::beg );
        gepf->read( (char*)(&rdbmRev), 4);
        cout << "rdbm rev" << rdbmRev << endl;
    }

    //  
    //  Map the rdbm revision to a platform number (e.g. 11.x, 12.x, etc.) 
    //
    if ( rdbmRev > 6.95 && rdbmRev < 8.0 ) { 
        version = 9.0; 
    } else if ( rdbmRev == 9.0 ) { 
        version = 11.0;
    } else if ( rdbmRev == 11.0 ) { 
        version = 12.0;
    } else if ( (int)rdbmRev == 14 ) { 
        version = 14.0;
    } else if ( (int)rdbmRev == 15 ) { 
        version = 15.0;
    } else if ( (int)rdbmRev == 20 ) { 
        version = 20.0;
    }

    return version; 
}



/*!
 *  Get a string containing a mapping of offsets and datatype for pfile fields. 
 *  for the specified pfile version. 
 */
string svkGEPFileReader::GetOffsetsString( float pfileVersion )
{

    //  fieldName                           nativeType  numElements offset stringValue
    //  ----------------------------------------------------------
    //  rhr.rh_rdbm_rev                    , FLOAT_4,   1   ,       0       value
    //  rhr.rh_logo                        , CHAR   ,   10  ,       34

    string offsets; 

    if ( (int)pfileVersion == 9 ) {

        offsets.assign (" \
            rhr.rh_rdbm_rev                    , FLOAT_4, 1   , 0,\
            rhr.rh_scan_date                   , CHAR   , 10  , 16,\
            rhr.rh_scan_time                   , CHAR   , 8   , 26,\
            rhr.csi_dims                       , INT_2  , 1   , 372,\
            rhr.rh_dab[0].start_rcv            , INT_2  , 1   , 200,\
            rhr.rh_dab[0].stop_rcv             , INT_2  , 1   , 202,\
            rhr.rh_dab[1].start_rcv            , INT_2  , 1   , 204,\
            rhr.rh_dab[1].stop_rcv             , INT_2  , 1   , 206,\
            rhr.rh_dab[2].start_rcv            , INT_2  , 1   , 208,\
            rhr.rh_dab[2].stop_rcv             , INT_2  , 1   , 210,\
            rhr.rh_dab[3].start_rcv            , INT_2  , 1   , 212,\
            rhr.rh_dab[3].stop_rcv             , INT_2  , 1   , 214,\
            rhr.rh_data_collect_type           , INT_2  , 1   , 56,\
            rhr.rh_file_contents               , INT_2  , 1   , 44,\
            rhr.rh_frame_size                  , INT_2  , 1   , 80,\
            rhr.rh_point_size                  , INT_2  , 1   , 82,\
            rhr.rh_ps_mps_freq                 , LINT_4 , 1   , 424,\
            rhr.rh_user_usage_tag              , ULINT_4, 1   , 988,\
            rhr.roilenx                        , FLOAT_4, 1   , 380,\
            rhr.roileny                        , FLOAT_4, 1   , 384,\
            rhr.roilenz                        , FLOAT_4, 1   , 388,\
            rhr.spectral_width                 , FLOAT_4, 1   , 368,\
            rhr.xcsi                           , INT_2  , 1   , 374,\
            rhr.ycsi                           , INT_2  , 1   , 376,\
            rhr.zcsi                           , INT_2  , 1   , 378,\
            rhr.rh_logo                        , CHAR   , 10  , 34,\
            rhr.rh_raw_pass_size               , LINT_4 , 1   , 116,\
            rhr.rh_user0                       , FLOAT_4, 1   , 216,\
            rhr.rh_user1                       , FLOAT_4, 1   , 220,\
            rhr.rh_user2                       , FLOAT_4, 1   , 224,\
            rhr.rh_user3                       , FLOAT_4, 1   , 228,\
            rhr.rh_user4                       , FLOAT_4, 1   , 232,\
            rhr.rh_user5                       , FLOAT_4, 1   , 236,\
            rhr.rh_user6                       , FLOAT_4, 1   , 240,\
            rhr.rh_user7                       , FLOAT_4, 1   , 244,\
            rhr.rh_user8                       , FLOAT_4, 1   , 248,\
            rhr.rh_user9                       , FLOAT_4, 1   , 252,\
            rhr.rh_user10                      , FLOAT_4, 1   , 256,\
            rhr.rh_user11                      , FLOAT_4, 1   , 260,\
            rhr.rh_user12                      , FLOAT_4, 1   , 264,\
            rhr.rh_user13                      , FLOAT_4, 1   , 268,\
            rhr.rh_user14                      , FLOAT_4, 1   , 272,\
            rhr.rh_user15                      , FLOAT_4, 1   , 276,\
            rhr.rh_user16                      , FLOAT_4, 1   , 280,\
            rhr.rh_user17                      , FLOAT_4, 1   , 284,\
            rhr.rh_user18                      , FLOAT_4, 1   , 288,\
            rhr.rh_user19                      , FLOAT_4, 1   , 292,\
            rhr.rh_user20                      , FLOAT_4, 1   , 1000,\
            rhr.rh_user21                      , FLOAT_4, 1   , 1004,\
            rhr.rh_user22                      , FLOAT_4, 1   , 1008,\
            rhr.rh_user23                      , FLOAT_4, 1   , 1012,\
            rhr.rh_user24                      , FLOAT_4, 1   , 1016,\
            rhr.rh_user25                      , FLOAT_4, 1   , 1020,\
            rhr.rh_user26                      , FLOAT_4, 1   , 1024,\
            rhr.rh_user27                      , FLOAT_4, 1   , 1028,\
            rhr.rh_user28                      , FLOAT_4, 1   , 1032,\
            rhr.rh_user29                      , FLOAT_4, 1   , 1036,\
            rhr.rh_user30                      , FLOAT_4, 1   , 1040,\
            rhr.rh_user31                      , FLOAT_4, 1   , 1044,\
            rhr.rh_user32                      , FLOAT_4, 1   , 1048,\
            rhr.rh_user33                      , FLOAT_4, 1   , 1052,\
            rhr.rh_user34                      , FLOAT_4, 1   , 1056,\
            rhr.rh_user35                      , FLOAT_4, 1   , 1060,\
            rhr.rh_user36                      , FLOAT_4, 1   , 1064,\
            rhr.rh_user37                      , FLOAT_4, 1   , 1068,\
            rhr.rh_user38                      , FLOAT_4, 1   , 1072,\
            rhr.rh_user39                      , FLOAT_4, 1   , 1076,\
            rhr.rh_user40                      , FLOAT_4, 1   , 1080,\
            rhr.rh_user41                      , FLOAT_4, 1   , 1084,\
            rhr.rh_user42                      , FLOAT_4, 1   , 1088,\
            rhr.rh_user43                      , FLOAT_4, 1   , 1092,\
            rhr.rh_user44                      , FLOAT_4, 1   , 1096,\
            rhr.rh_user45                      , FLOAT_4, 1   , 1100,\
            rhr.rh_user46                      , FLOAT_4, 1   , 1104,\
            rhr.rh_user47                      , FLOAT_4, 1   , 1108,\
            rhr.rh_user48                      , FLOAT_4, 1   , 1112,\
            rhi.psdname                        , CHAR   , 33  , 39260,\
            rhi.scanspacing                    , FLOAT_4, 1   , 39060,\
            rhi.te                             , INT_4  , 1   , 39148,\
            rhi.ti                             , INT_4  , 1   , 39144,\
            rhi.tr                             , INT_4  , 1   , 39140,\
            rhi.tlhc_A                         , FLOAT_4, 1   , 39104,\
            rhi.tlhc_R                         , FLOAT_4, 1   , 39100,\
            rhi.tlhc_S                         , FLOAT_4, 1   , 39108,\
            rhi.t                              , INT_4  , 1   , 39140,\
            rhi.trhc_A                         , FLOAT_4, 1   , 39116,\
            rhi.trhc_R                         , FLOAT_4, 1   , 39112,\
            rhi.trhc_S                         , FLOAT_4, 1   , 39120,\
            rhi.user0                          , FLOAT_4, 1   , 39364,\
            rhi.user1                          , FLOAT_4, 1   , 39368,\
            rhi.user2                          , FLOAT_4, 1   , 39372,\
            rhi.user3                          , FLOAT_4, 1   , 39376,\
            rhi.user4                          , FLOAT_4, 1   , 39380,\
            rhi.user5                          , FLOAT_4, 1   , 39384,\
            rhi.user6                          , FLOAT_4, 1   , 39388,\
            rhi.user7                          , FLOAT_4, 1   , 39392,\
            rhi.user8                          , FLOAT_4, 1   , 39396,\
            rhi.user9                          , FLOAT_4, 1   , 39400,\
            rhi.user10                         , FLOAT_4, 1   , 39404,\
            rhi.user11                         , FLOAT_4, 1   , 39408,\
            rhi.user12                         , FLOAT_4, 1   , 39412,\
            rhi.user13                         , FLOAT_4, 1   , 39416,\
            rhi.user14                         , FLOAT_4, 1   , 39420,\
            rhi.user15                         , FLOAT_4, 1   , 39424,\
            rhi.user16                         , FLOAT_4, 1   , 39428,\
            rhi.user17                         , FLOAT_4, 1   , 39432,\
            rhi.user18                         , FLOAT_4, 1   , 39436,\
            rhi.user19                         , FLOAT_4, 1   , 39440,\
            rhi.user20                         , FLOAT_4, 1   , 39444,\
            rhi.user21                         , FLOAT_4, 1   , 39448,\
            rhi.user22                         , FLOAT_4, 1   , 39452,\
            rhi.user23                         , FLOAT_4, 1   , 39456,\
            rhi.user24                         , FLOAT_4, 1   , 39460,\
            rhi.user25                         , FLOAT_4, 1   , 39840,\
            rhi.user26                         , FLOAT_4, 1   , 39844,\
            rhi.user27                         , FLOAT_4, 1   , 39848,\
            rhi.user28                         , FLOAT_4, 1   , 39852,\
            rhi.user29                         , FLOAT_4, 1   , 39856,\
            rhi.user30                         , FLOAT_4, 1   , 39860,\
            rhi.user31                         , FLOAT_4, 1   , 39864,\
            rhi.user32                         , FLOAT_4, 1   , 39868,\
            rhi.user33                         , FLOAT_4, 1   , 39872,\
            rhi.user34                         , FLOAT_4, 1   , 39876,\
            rhi.user35                         , FLOAT_4, 1   , 39880,\
            rhi.user36                         , FLOAT_4, 1   , 39884,\
            rhi.user37                         , FLOAT_4, 1   , 39888,\
            rhi.user38                         , FLOAT_4, 1   , 39892,\
            rhi.user39                         , FLOAT_4, 1   , 39896,\
            rhi.user40                         , FLOAT_4, 1   , 39900,\
            rhi.user41                         , FLOAT_4, 1   , 39904,\
            rhi.user42                         , FLOAT_4, 1   , 39908,\
            rhi.user43                         , FLOAT_4, 1   , 39912,\
            rhi.user44                         , FLOAT_4, 1   , 39916,\
            rhi.user45                         , FLOAT_4, 1   , 39920,\
            rhi.user46                         , FLOAT_4, 1   , 39924,\
            rhi.user47                         , FLOAT_4, 1   , 39928,\
            rhi.user48                         , FLOAT_4, 1   , 39932,\
            rhi.cname                          , CHAR   , 17  , 39316,\
            rhi.brhc_A                         , FLOAT_4, 1   , 39128,\
            rhi.brhc_R                         , FLOAT_4, 1   , 39124,\
            rhi.brhc_S                         , FLOAT_4, 1   , 39132,\
            rhi.ctr_A                          , FLOAT_4, 1   , 39080,\
            rhi.ctr_R                          , FLOAT_4, 1   , 39076,\
            rhi.ctr_S                          , FLOAT_4, 1   , 39084,\
            rhi.dfov                           , FLOAT_4, 1   , 38976,\
            rhi.freq_dir                       , INT_2  , 1   , 39704,\
            rhi.ctyp                           , INT_2  , 1   , 39314,\
            rhi.loc                            , FLOAT_4, 1   , 39072,\
            rhi.mr_flip                        , INT_2  , 1   , 39200,\
            rhi.nex                            , FLOAT_4, 1   , 39164,\
            rhi.numecho                        , INT_2  , 1   , 39156,\
            rhe.ex_datetime                    , INT_4  , 1   , 37084,\
            rhe.ex_no                          , UINT_2 , 1   , 36880,\
            rhe.magstrength                    , INT_4  , 1   , 36956,\
            rhe.patid                          , CHAR   , 13  , 36960,\
            rhe.patname                        , CHAR   , 25  , 36973,\
            rhe.refphy                         , CHAR   , 33  , 37088,\
            rhe.reqnum                         , CHAR   , 13  , 37071,\
            rhe.study_uid                      , UID    , 32  , 37358,\
            rhe.dateofbirth                    , CHAR   , 9   , 37603,\
            rhe.patsex                         , INT_2  , 1   , 37002,\
            rhe.hospname                       , CHAR   , 33  , 36882,\
            rhe.ex_sysid                       , CHAR   , 9   , 37200,\
            rhe.uniq_sys_id                    , CHAR   , 16  , 37322,\
            rhe.ex_verscre                     , CHAR   , 2   , 37236,\
            rhs.se_no                          , INT_2  , 1   , 37922,\
            rhs.se_desc                        , CHAR   , 30  , 37932,\
            rhs.entry                          , INT_4  , 1   , 37992,\
            rhs.position                       , INT_4  , 1   , 37988,\
            rhs.series_uid                     , UID    , 32  , 38256,\
            rhs.landmark_uid                   , UID    , 32  , 38288,\
            rhs.anref                          , CHAR   , 3   , 37996,\
        "); 

    } else if ( (int)pfileVersion == 11 ) {

        offsets.assign (" \
            rhr.rh_rdbm_rev                    , FLOAT_4, 1   , 0,\
            rhr.rh_scan_date                   , CHAR   , 10  , 16,\
            rhr.rh_scan_time                   , CHAR   , 8   , 26,\
            rhr.csi_dims                       , INT_2  , 1   , 372,\
            rhr.rh_dab[0].start_rcv            , INT_2  , 1   , 200,\
            rhr.rh_dab[0].stop_rcv             , INT_2  , 1   , 202,\
            rhr.rh_dab[1].start_rcv            , INT_2  , 1   , 204,\
            rhr.rh_dab[1].stop_rcv             , INT_2  , 1   , 206,\
            rhr.rh_dab[2].start_rcv            , INT_2  , 1   , 208,\
            rhr.rh_dab[2].stop_rcv             , INT_2  , 1   , 210,\
            rhr.rh_dab[3].start_rcv            , INT_2  , 1   , 212,\
            rhr.rh_dab[3].stop_rcv             , INT_2  , 1   , 214,\
            rhr.rh_data_collect_type           , INT_2  , 1   , 56,\
            rhr.rh_file_contents               , INT_2  , 1   , 44,\
            rhr.rh_frame_size                  , UINT_2 , 1   , 80,\
            rhr.rh_point_size                  , INT_2  , 1   , 82,\
            rhr.rh_ps_mps_freq                 , LINT_4 , 1   , 424,\
            rhr.rh_user_usage_tag              , ULINT_4, 1   , 988,\
            rhr.roilenx                        , FLOAT_4, 1   , 380,\
            rhr.roileny                        , FLOAT_4, 1   , 384,\
            rhr.roilenz                        , FLOAT_4, 1   , 388,\
            rhr.spectral_width                 , FLOAT_4, 1   , 368,\
            rhr.xcsi                           , INT_2  , 1   , 374,\
            rhr.ycsi                           , INT_2  , 1   , 376,\
            rhr.zcsi                           , INT_2  , 1   , 378,\
            rhr.rh_logo                        , CHAR   , 10  , 34,\
            rhr.rh_raw_pass_size               , LINT_4 , 1   , 116,\
            rhr.rh_user0                       , FLOAT_4, 1   , 216,\
            rhr.rh_user1                       , FLOAT_4, 1   , 220,\
            rhr.rh_user2                       , FLOAT_4, 1   , 224,\
            rhr.rh_user3                       , FLOAT_4, 1   , 228,\
            rhr.rh_user4                       , FLOAT_4, 1   , 232,\
            rhr.rh_user5                       , FLOAT_4, 1   , 236,\
            rhr.rh_user6                       , FLOAT_4, 1   , 240,\
            rhr.rh_user7                       , FLOAT_4, 1   , 244,\
            rhr.rh_user8                       , FLOAT_4, 1   , 248,\
            rhr.rh_user9                       , FLOAT_4, 1   , 252,\
            rhr.rh_user10                      , FLOAT_4, 1   , 256,\
            rhr.rh_user11                      , FLOAT_4, 1   , 260,\
            rhr.rh_user12                      , FLOAT_4, 1   , 264,\
            rhr.rh_user13                      , FLOAT_4, 1   , 268,\
            rhr.rh_user14                      , FLOAT_4, 1   , 272,\
            rhr.rh_user15                      , FLOAT_4, 1   , 276,\
            rhr.rh_user16                      , FLOAT_4, 1   , 280,\
            rhr.rh_user17                      , FLOAT_4, 1   , 284,\
            rhr.rh_user18                      , FLOAT_4, 1   , 288,\
            rhr.rh_user19                      , FLOAT_4, 1   , 292,\
            rhr.rh_user20                      , FLOAT_4, 1   , 1000,\
            rhr.rh_user21                      , FLOAT_4, 1   , 1004,\
            rhr.rh_user22                      , FLOAT_4, 1   , 1008,\
            rhr.rh_user23                      , FLOAT_4, 1   , 1012,\
            rhr.rh_user24                      , FLOAT_4, 1   , 1016,\
            rhr.rh_user25                      , FLOAT_4, 1   , 1020,\
            rhr.rh_user26                      , FLOAT_4, 1   , 1024,\
            rhr.rh_user27                      , FLOAT_4, 1   , 1028,\
            rhr.rh_user28                      , FLOAT_4, 1   , 1032,\
            rhr.rh_user29                      , FLOAT_4, 1   , 1036,\
            rhr.rh_user30                      , FLOAT_4, 1   , 1040,\
            rhr.rh_user31                      , FLOAT_4, 1   , 1044,\
            rhr.rh_user32                      , FLOAT_4, 1   , 1048,\
            rhr.rh_user33                      , FLOAT_4, 1   , 1052,\
            rhr.rh_user34                      , FLOAT_4, 1   , 1056,\
            rhr.rh_user35                      , FLOAT_4, 1   , 1060,\
            rhr.rh_user36                      , FLOAT_4, 1   , 1064,\
            rhr.rh_user37                      , FLOAT_4, 1   , 1068,\
            rhr.rh_user38                      , FLOAT_4, 1   , 1072,\
            rhr.rh_user39                      , FLOAT_4, 1   , 1076,\
            rhr.rh_user40                      , FLOAT_4, 1   , 1080,\
            rhr.rh_user41                      , FLOAT_4, 1   , 1084,\
            rhr.rh_user42                      , FLOAT_4, 1   , 1088,\
            rhr.rh_user43                      , FLOAT_4, 1   , 1092,\
            rhr.rh_user44                      , FLOAT_4, 1   , 1096,\
            rhr.rh_user45                      , FLOAT_4, 1   , 1100,\
            rhr.rh_user46                      , FLOAT_4, 1   , 1104,\
            rhr.rh_user47                      , FLOAT_4, 1   , 1108,\
            rhr.rh_user48                      , FLOAT_4, 1   , 1112,\
            rhi.psdname                        , CHAR   , 33  , 60248,\
            rhi.scanspacing                    , FLOAT_4, 1   , 60048,\
            rhi.te                             , INT_4  , 1   , 60136,\
            rhi.ti                             , INT_4  , 1   , 60132,\
            rhi.tr                             , INT_4  , 1   , 60128,\
            rhi.tlhc_A                         , FLOAT_4, 1   , 60092,\
            rhi.tlhc_R                         , FLOAT_4, 1   , 60088,\
            rhi.tlhc_S                         , FLOAT_4, 1   , 60096,\
            rhi.t                              , INT_4  , 1   , 60128,\
            rhi.trhc_A                         , FLOAT_4, 1   , 60104,\
            rhi.trhc_R                         , FLOAT_4, 1   , 60100,\
            rhi.trhc_S                         , FLOAT_4, 1   , 60108,\
            rhi.user0                          , FLOAT_4, 1   , 60352,\
            rhi.user1                          , FLOAT_4, 1   , 60356,\
            rhi.user2                          , FLOAT_4, 1   , 60360,\
            rhi.user3                          , FLOAT_4, 1   , 60364,\
            rhi.user4                          , FLOAT_4, 1   , 60368,\
            rhi.user5                          , FLOAT_4, 1   , 60372,\
            rhi.user6                          , FLOAT_4, 1   , 60376,\
            rhi.user7                          , FLOAT_4, 1   , 60380,\
            rhi.user8                          , FLOAT_4, 1   , 60384,\
            rhi.user9                          , FLOAT_4, 1   , 60388,\
            rhi.user10                         , FLOAT_4, 1   , 60392,\
            rhi.user11                         , FLOAT_4, 1   , 60396,\
            rhi.user12                         , FLOAT_4, 1   , 60400,\
            rhi.user13                         , FLOAT_4, 1   , 60404,\
            rhi.user14                         , FLOAT_4, 1   , 60408,\
            rhi.user15                         , FLOAT_4, 1   , 60412,\
            rhi.user16                         , FLOAT_4, 1   , 60416,\
            rhi.user17                         , FLOAT_4, 1   , 60420,\
            rhi.user18                         , FLOAT_4, 1   , 60424,\
            rhi.user19                         , FLOAT_4, 1   , 60428,\
            rhi.user20                         , FLOAT_4, 1   , 60432,\
            rhi.user21                         , FLOAT_4, 1   , 60436,\
            rhi.user22                         , FLOAT_4, 1   , 60440,\
            rhi.user23                         , FLOAT_4, 1   , 60536,\
            rhi.user24                         , FLOAT_4, 1   , 60540,\
            rhi.user25                         , FLOAT_4, 1   , 60828,\
            rhi.user26                         , FLOAT_4, 1   , 60832,\
            rhi.user27                         , FLOAT_4, 1   , 60836,\
            rhi.user28                         , FLOAT_4, 1   , 60840,\
            rhi.user29                         , FLOAT_4, 1   , 60844,\
            rhi.user30                         , FLOAT_4, 1   , 60848,\
            rhi.user31                         , FLOAT_4, 1   , 60852,\
            rhi.user32                         , FLOAT_4, 1   , 60856,\
            rhi.user33                         , FLOAT_4, 1   , 60860,\
            rhi.user34                         , FLOAT_4, 1   , 60864,\
            rhi.user35                         , FLOAT_4, 1   , 60868,\
            rhi.user36                         , FLOAT_4, 1   , 60872,\
            rhi.user37                         , FLOAT_4, 1   , 60876,\
            rhi.user38                         , FLOAT_4, 1   , 60880,\
            rhi.user39                         , FLOAT_4, 1   , 60884,\
            rhi.user40                         , FLOAT_4, 1   , 60888,\
            rhi.user41                         , FLOAT_4, 1   , 60892,\
            rhi.user42                         , FLOAT_4, 1   , 60896,\
            rhi.user43                         , FLOAT_4, 1   , 60900,\
            rhi.user44                         , FLOAT_4, 1   , 60904,\
            rhi.user45                         , FLOAT_4, 1   , 60908,\
            rhi.user46                         , FLOAT_4, 1   , 60912,\
            rhi.user47                         , FLOAT_4, 1   , 60916,\
            rhi.user48                         , FLOAT_4, 1   , 60920,\
            rhi.cname                          , CHAR   , 17  , 60304,\
            rhi.brhc_A                         , FLOAT_4, 1   , 60116,\
            rhi.brhc_R                         , FLOAT_4, 1   , 60112,\
            rhi.brhc_S                         , FLOAT_4, 1   , 60120,\
            rhi.ctr_A                          , FLOAT_4, 1   , 60068,\
            rhi.ctr_R                          , FLOAT_4, 1   , 60064,\
            rhi.ctr_S                          , FLOAT_4, 1   , 60072,\
            rhi.dfov                           , FLOAT_4, 1   , 59964,\
            rhi.freq_dir                       , INT_2  , 1   , 60692,\
            rhi.ctyp                           , INT_2  , 1   , 60302,\
            rhi.loc                            , FLOAT_4, 1   , 60060,\
            rhi.mr_flip                        , INT_2  , 1   , 60188,\
            rhi.nex                            , FLOAT_4, 1   , 60152,\
            rhi.numecho                        , INT_2  , 1   , 60144,\
            rhe.ex_datetime                    , INT_4  , 1   , 57564,\
            rhe.ex_no                          , UINT_2 , 1   , 57360,\
            rhe.magstrength                    , INT_4  , 1   , 57436,\
            rhe.patid                          , CHAR   , 13  , 57440,\
            rhe.patname                        , CHAR   , 25  , 57453,\
            rhe.refphy                         , CHAR   , 33  , 57568,\
            rhe.reqnum                         , CHAR   , 13  , 57551,\
            rhe.study_uid                      , UID    , 32  , 57838,\
            rhe.dateofbirth                    , CHAR   , 9   , 58083,\
            rhe.patsex                         , INT_2  , 1   , 57482,\
            rhe.hospname                       , CHAR   , 33  , 57362,\
            rhe.ex_sysid                       , CHAR   , 9   , 57680,\
            rhe.uniq_sys_id                    , CHAR   , 16  , 57802,\
            rhe.ex_verscre                     , CHAR   , 2   , 57716,\
            rhs.se_no                          , INT_2  , 1   , 58402,\
            rhs.se_desc                        , CHAR   , 30  , 58412,\
            rhs.entry                          , INT_4  , 1   , 58472,\
            rhs.position                       , INT_4  , 1   , 58468,\
            rhs.series_uid                     , UID    , 32  , 58736,\
            rhs.landmark_uid                   , UID    , 32  , 58768,\
            rhs.anref                          , CHAR   , 3   , 58476,\
        "); 

    } else if ( (int)pfileVersion == 12 ) {

        offsets.assign (" \
            rhr.rh_rdbm_rev                    , FLOAT_4, 1   , 0,\
            rhr.rh_scan_date                   , CHAR   , 10  , 16,\
            rhr.rh_scan_time                   , CHAR   , 8   , 26,\
            rhr.csi_dims                       , INT_2  , 1   , 372,\
            rhr.rh_dab[0].start_rcv            , INT_2  , 1   , 200,\
            rhr.rh_dab[0].stop_rcv             , INT_2  , 1   , 202,\
            rhr.rh_dab[1].start_rcv            , INT_2  , 1   , 204,\
            rhr.rh_dab[1].stop_rcv             , INT_2  , 1   , 206,\
            rhr.rh_dab[2].start_rcv            , INT_2  , 1   , 208,\
            rhr.rh_dab[2].stop_rcv             , INT_2  , 1   , 210,\
            rhr.rh_dab[3].start_rcv            , INT_2  , 1   , 212,\
            rhr.rh_dab[3].stop_rcv             , INT_2  , 1   , 214,\
            rhr.rh_data_collect_type           , INT_2  , 1   , 56,\
            rhr.rh_file_contents               , INT_2  , 1   , 44,\
            rhr.rdb_hdr_off_data               , INT_4  , 1   , 1468,\
            rhr.rh_frame_size                  , UINT_2 , 1   , 80,\
            rhr.rh_point_size                  , INT_2  , 1   , 82,\
            rhr.rh_ps_mps_freq                 , LINT_4 , 1   , 424,\
            rhr.rh_user_usage_tag              , ULINT_4, 1   , 988,\
            rhr.roilenx                        , FLOAT_4, 1   , 380,\
            rhr.roileny                        , FLOAT_4, 1   , 384,\
            rhr.roilenz                        , FLOAT_4, 1   , 388,\
            rhr.spectral_width                 , FLOAT_4, 1   , 368,\
            rhr.xcsi                           , INT_2  , 1   , 374,\
            rhr.ycsi                           , INT_2  , 1   , 376,\
            rhr.zcsi                           , INT_2  , 1   , 378,\
            rhr.rh_logo                        , CHAR   , 10  , 34,\
            rhr.rh_raw_pass_size               , LINT_4 , 1   , 116,\
            rhr.rh_user0                       , FLOAT_4, 1   , 216,\
            rhr.rh_user1                       , FLOAT_4, 1   , 220,\
            rhr.rh_user2                       , FLOAT_4, 1   , 224,\
            rhr.rh_user3                       , FLOAT_4, 1   , 228,\
            rhr.rh_user4                       , FLOAT_4, 1   , 232,\
            rhr.rh_user5                       , FLOAT_4, 1   , 236,\
            rhr.rh_user6                       , FLOAT_4, 1   , 240,\
            rhr.rh_user7                       , FLOAT_4, 1   , 244,\
            rhr.rh_user8                       , FLOAT_4, 1   , 248,\
            rhr.rh_user9                       , FLOAT_4, 1   , 252,\
            rhr.rh_user10                      , FLOAT_4, 1   , 256,\
            rhr.rh_user11                      , FLOAT_4, 1   , 260,\
            rhr.rh_user12                      , FLOAT_4, 1   , 264,\
            rhr.rh_user13                      , FLOAT_4, 1   , 268,\
            rhr.rh_user14                      , FLOAT_4, 1   , 272,\
            rhr.rh_user15                      , FLOAT_4, 1   , 276,\
            rhr.rh_user16                      , FLOAT_4, 1   , 280,\
            rhr.rh_user17                      , FLOAT_4, 1   , 284,\
            rhr.rh_user18                      , FLOAT_4, 1   , 288,\
            rhr.rh_user19                      , FLOAT_4, 1   , 292,\
            rhr.rh_user20                      , FLOAT_4, 1   , 1000,\
            rhr.rh_user21                      , FLOAT_4, 1   , 1004,\
            rhr.rh_user22                      , FLOAT_4, 1   , 1008,\
            rhr.rh_user23                      , FLOAT_4, 1   , 1012,\
            rhr.rh_user24                      , FLOAT_4, 1   , 1016,\
            rhr.rh_user25                      , FLOAT_4, 1   , 1020,\
            rhr.rh_user26                      , FLOAT_4, 1   , 1024,\
            rhr.rh_user27                      , FLOAT_4, 1   , 1028,\
            rhr.rh_user28                      , FLOAT_4, 1   , 1032,\
            rhr.rh_user29                      , FLOAT_4, 1   , 1036,\
            rhr.rh_user30                      , FLOAT_4, 1   , 1040,\
            rhr.rh_user31                      , FLOAT_4, 1   , 1044,\
            rhr.rh_user32                      , FLOAT_4, 1   , 1048,\
            rhr.rh_user33                      , FLOAT_4, 1   , 1052,\
            rhr.rh_user34                      , FLOAT_4, 1   , 1056,\
            rhr.rh_user35                      , FLOAT_4, 1   , 1060,\
            rhr.rh_user36                      , FLOAT_4, 1   , 1064,\
            rhr.rh_user37                      , FLOAT_4, 1   , 1068,\
            rhr.rh_user38                      , FLOAT_4, 1   , 1072,\
            rhr.rh_user39                      , FLOAT_4, 1   , 1076,\
            rhr.rh_user40                      , FLOAT_4, 1   , 1080,\
            rhr.rh_user41                      , FLOAT_4, 1   , 1084,\
            rhr.rh_user42                      , FLOAT_4, 1   , 1088,\
            rhr.rh_user43                      , FLOAT_4, 1   , 1092,\
            rhr.rh_user44                      , FLOAT_4, 1   , 1096,\
            rhr.rh_user45                      , FLOAT_4, 1   , 1100,\
            rhr.rh_user46                      , FLOAT_4, 1   , 1104,\
            rhr.rh_user47                      , FLOAT_4, 1   , 1108,\
            rhr.rh_user48                      , FLOAT_4, 1   , 1112,\
            rhi.psdname                        , CHAR   , 33  , 65374,\
            rhi.scanspacing                    , FLOAT_4, 1   , 64552,\
            rhi.te                             , INT_4  , 1   , 65032,\
            rhi.ti                             , INT_4  , 1   , 65028,\
            rhi.tr                             , INT_4  , 1   , 65024,\
            rhi.tlhc_A                         , FLOAT_4, 1   , 64956,\
            rhi.tlhc_R                         , FLOAT_4, 1   , 64952,\
            rhi.tlhc_S                         , FLOAT_4, 1   , 64960,\
            rhi.t                              , INT_4  , 1   , 65024,\
            rhi.trhc_A                         , FLOAT_4, 1   , 64968,\
            rhi.trhc_R                         , FLOAT_4, 1   , 64964,\
            rhi.trhc_S                         , FLOAT_4, 1   , 64972,\
            rhi.user0                          , FLOAT_4, 1   , 64588,\
            rhi.user1                          , FLOAT_4, 1   , 64592,\
            rhi.user2                          , FLOAT_4, 1   , 64596,\
            rhi.user3                          , FLOAT_4, 1   , 64600,\
            rhi.user4                          , FLOAT_4, 1   , 64604,\
            rhi.user5                          , FLOAT_4, 1   , 64608,\
            rhi.user6                          , FLOAT_4, 1   , 64612,\
            rhi.user7                          , FLOAT_4, 1   , 64616,\
            rhi.user8                          , FLOAT_4, 1   , 64620,\
            rhi.user9                          , FLOAT_4, 1   , 64624,\
            rhi.user10                         , FLOAT_4, 1   , 64628,\
            rhi.user11                         , FLOAT_4, 1   , 64632,\
            rhi.user12                         , FLOAT_4, 1   , 64636,\
            rhi.user13                         , FLOAT_4, 1   , 64640,\
            rhi.user14                         , FLOAT_4, 1   , 64644,\
            rhi.user15                         , FLOAT_4, 1   , 64648,\
            rhi.user16                         , FLOAT_4, 1   , 64652,\
            rhi.user17                         , FLOAT_4, 1   , 64656,\
            rhi.user18                         , FLOAT_4, 1   , 64660,\
            rhi.user19                         , FLOAT_4, 1   , 64664,\
            rhi.user20                         , FLOAT_4, 1   , 64668,\
            rhi.user21                         , FLOAT_4, 1   , 64672,\
            rhi.user22                         , FLOAT_4, 1   , 64676,\
            rhi.user23                         , FLOAT_4, 1   , 64688,\
            rhi.user24                         , FLOAT_4, 1   , 64692,\
            rhi.user25                         , FLOAT_4, 1   , 64756,\
            rhi.user26                         , FLOAT_4, 1   , 64760,\
            rhi.user27                         , FLOAT_4, 1   , 64764,\
            rhi.user28                         , FLOAT_4, 1   , 64768,\
            rhi.user29                         , FLOAT_4, 1   , 64772,\
            rhi.user30                         , FLOAT_4, 1   , 64776,\
            rhi.user31                         , FLOAT_4, 1   , 64780,\
            rhi.user32                         , FLOAT_4, 1   , 64784,\
            rhi.user33                         , FLOAT_4, 1   , 64788,\
            rhi.user34                         , FLOAT_4, 1   , 64792,\
            rhi.user35                         , FLOAT_4, 1   , 64796,\
            rhi.user36                         , FLOAT_4, 1   , 64800,\
            rhi.user37                         , FLOAT_4, 1   , 64804,\
            rhi.user38                         , FLOAT_4, 1   , 64808,\
            rhi.user39                         , FLOAT_4, 1   , 64812,\
            rhi.user40                         , FLOAT_4, 1   , 64816,\
            rhi.user41                         , FLOAT_4, 1   , 64820,\
            rhi.user42                         , FLOAT_4, 1   , 64824,\
            rhi.user43                         , FLOAT_4, 1   , 64828,\
            rhi.user44                         , FLOAT_4, 1   , 64832,\
            rhi.user45                         , FLOAT_4, 1   , 64836,\
            rhi.user46                         , FLOAT_4, 1   , 64840,\
            rhi.user47                         , FLOAT_4, 1   , 64844,\
            rhi.user48                         , FLOAT_4, 1   , 64848,\
            rhi.cname                          , CHAR   , 17  , 65491,\
            rhi.brhc_A                         , FLOAT_4, 1   , 64980,\
            rhi.brhc_R                         , FLOAT_4, 1   , 64976,\
            rhi.brhc_S                         , FLOAT_4, 1   , 64984,\
            rhi.ctr_A                          , FLOAT_4, 1   , 64932,\
            rhi.ctr_R                          , FLOAT_4, 1   , 64928,\
            rhi.ctr_S                          , FLOAT_4, 1   , 64936,\
            rhi.dfov                           , FLOAT_4, 1   , 64536,\
            rhi.freq_dir                       , INT_2  , 1   , 65334,\
            rhi.ctyp                           , INT_2  , 1   , 65268,\
            rhi.loc                            , FLOAT_4, 1   , 64556,\
            rhi.mr_flip                        , INT_2  , 1   , 65244,\
            rhi.nex                            , FLOAT_4, 1   , 64564,\
            rhi.numecho                        , INT_2  , 1   , 65206,\
            rhe.ex_datetime                    , INT_4  , 1   , 61568,\
            rhe.ex_no                          , UINT_2 , 1   , 61576,\
            rhe.magstrength                    , INT_4  , 1   , 61560,\
            rhe.patid                          , CHAR   , 13  , 61884,\
            rhe.patname                        , CHAR   , 25  , 61897,\
            rhe.refphy                         , CHAR   , 33  , 61690,\
            rhe.reqnum                         , CHAR   , 13  , 61677,\
            rhe.study_uid                      , UID    , 32  , 61966,\
            rhe.dateofbirth                    , CHAR   , 9   , 62209,\
            rhe.patsex                         , INT_2  , 1   , 61600,\
            rhe.hospname                       , CHAR   , 33  , 61851,\
            rhe.ex_sysid                       , CHAR   , 9   , 61828,\
            rhe.uniq_sys_id                    , CHAR   , 16  , 61930,\
            rhe.ex_verscre                     , CHAR   , 2   , 61926,\
            rhs.se_no                          , INT_2  , 1   , 62710,\
            rhs.se_desc                        , CHAR   , 65  , 62786,\
            rhs.entry                          , INT_4  , 1   , 62644,\
            rhs.position                       , INT_4  , 1   , 62640,\
            rhs.series_uid                     , UID    , 32  , 62899,\
            rhs.landmark_uid                   , UID    , 32  , 62931,\
            rhs.anref                          , CHAR   , 3   , 62869,\
        "); 

    } else if ( (int)pfileVersion == 14 ) {

        offsets.assign (" \
            rhr.rh_rdbm_rev                    , FLOAT_4, 1   , 0,\
            rhr.rh_scan_date                   , CHAR   , 10  , 16,\
            rhr.rh_scan_time                   , CHAR   , 8   , 26,\
            rhr.csi_dims                       , INT_2  , 1   , 372,\
            rhr.rh_dab[0].start_rcv            , INT_2  , 1   , 200,\
            rhr.rh_dab[0].stop_rcv             , INT_2  , 1   , 202,\
            rhr.rh_dab[1].start_rcv            , INT_2  , 1   , 204,\
            rhr.rh_dab[1].stop_rcv             , INT_2  , 1   , 206,\
            rhr.rh_dab[2].start_rcv            , INT_2  , 1   , 208,\
            rhr.rh_dab[2].stop_rcv             , INT_2  , 1   , 210,\
            rhr.rh_dab[3].start_rcv            , INT_2  , 1   , 212,\
            rhr.rh_dab[3].stop_rcv             , INT_2  , 1   , 214,\
            rhr.rh_data_collect_type           , INT_2  , 1   , 56,\
            rhr.rh_file_contents               , INT_2  , 1   , 44,\
            rhr.rdb_hdr_off_data               , INT_4  , 1   , 1468,\
            rhr.rh_frame_size                  , UINT_2 , 1   , 80,\
            rhr.rh_point_size                  , INT_2  , 1   , 82,\
            rhr.rh_ps_mps_freq                 , LINT_4 , 1   , 424,\
            rhr.rh_user_usage_tag              , ULINT_4, 1   , 988,\
            rhr.roilenx                        , FLOAT_4, 1   , 380,\
            rhr.roileny                        , FLOAT_4, 1   , 384,\
            rhr.roilenz                        , FLOAT_4, 1   , 388,\
            rhr.spectral_width                 , FLOAT_4, 1   , 368,\
            rhr.xcsi                           , INT_2  , 1   , 374,\
            rhr.ycsi                           , INT_2  , 1   , 376,\
            rhr.zcsi                           , INT_2  , 1   , 378,\
            rhr.rh_logo                        , CHAR   , 10  , 34,\
            rhr.rh_raw_pass_size               , ULINT_4, 1   , 116,\
            rhr.rh_user0                       , FLOAT_4, 1   , 216,\
            rhr.rh_user1                       , FLOAT_4, 1   , 220,\
            rhr.rh_user2                       , FLOAT_4, 1   , 224,\
            rhr.rh_user3                       , FLOAT_4, 1   , 228,\
            rhr.rh_user4                       , FLOAT_4, 1   , 232,\
            rhr.rh_user5                       , FLOAT_4, 1   , 236,\
            rhr.rh_user6                       , FLOAT_4, 1   , 240,\
            rhr.rh_user7                       , FLOAT_4, 1   , 244,\
            rhr.rh_user8                       , FLOAT_4, 1   , 248,\
            rhr.rh_user9                       , FLOAT_4, 1   , 252,\
            rhr.rh_user10                      , FLOAT_4, 1   , 256,\
            rhr.rh_user11                      , FLOAT_4, 1   , 260,\
            rhr.rh_user12                      , FLOAT_4, 1   , 264,\
            rhr.rh_user13                      , FLOAT_4, 1   , 268,\
            rhr.rh_user14                      , FLOAT_4, 1   , 272,\
            rhr.rh_user15                      , FLOAT_4, 1   , 276,\
            rhr.rh_user16                      , FLOAT_4, 1   , 280,\
            rhr.rh_user17                      , FLOAT_4, 1   , 284,\
            rhr.rh_user18                      , FLOAT_4, 1   , 288,\
            rhr.rh_user19                      , FLOAT_4, 1   , 292,\
            rhr.rh_user20                      , FLOAT_4, 1   , 1000,\
            rhr.rh_user21                      , FLOAT_4, 1   , 1004,\
            rhr.rh_user22                      , FLOAT_4, 1   , 1008,\
            rhr.rh_user23                      , FLOAT_4, 1   , 1012,\
            rhr.rh_user24                      , FLOAT_4, 1   , 1016,\
            rhr.rh_user25                      , FLOAT_4, 1   , 1020,\
            rhr.rh_user26                      , FLOAT_4, 1   , 1024,\
            rhr.rh_user27                      , FLOAT_4, 1   , 1028,\
            rhr.rh_user28                      , FLOAT_4, 1   , 1032,\
            rhr.rh_user29                      , FLOAT_4, 1   , 1036,\
            rhr.rh_user30                      , FLOAT_4, 1   , 1040,\
            rhr.rh_user31                      , FLOAT_4, 1   , 1044,\
            rhr.rh_user32                      , FLOAT_4, 1   , 1048,\
            rhr.rh_user33                      , FLOAT_4, 1   , 1052,\
            rhr.rh_user34                      , FLOAT_4, 1   , 1056,\
            rhr.rh_user35                      , FLOAT_4, 1   , 1060,\
            rhr.rh_user36                      , FLOAT_4, 1   , 1064,\
            rhr.rh_user37                      , FLOAT_4, 1   , 1068,\
            rhr.rh_user38                      , FLOAT_4, 1   , 1072,\
            rhr.rh_user39                      , FLOAT_4, 1   , 1076,\
            rhr.rh_user40                      , FLOAT_4, 1   , 1080,\
            rhr.rh_user41                      , FLOAT_4, 1   , 1084,\
            rhr.rh_user42                      , FLOAT_4, 1   , 1088,\
            rhr.rh_user43                      , FLOAT_4, 1   , 1092,\
            rhr.rh_user44                      , FLOAT_4, 1   , 1096,\
            rhr.rh_user45                      , FLOAT_4, 1   , 1100,\
            rhr.rh_user46                      , FLOAT_4, 1   , 1104,\
            rhr.rh_user47                      , FLOAT_4, 1   , 1108,\
            rhr.rh_user48                      , FLOAT_4, 1   , 1112,\
            rhi.psdname                        , CHAR   , 33  , 145132,\
            rhi.scanspacing                    , FLOAT_4, 1   , 143940,\
            rhi.te                             , INT_4  , 1   , 144580,\
            rhi.ti                             , INT_4  , 1   , 144576,\
            rhi.tr                             , INT_4  , 1   , 144572,\
            rhi.tlhc_A                         , FLOAT_4, 1   , 144344,\
            rhi.tlhc_R                         , FLOAT_4, 1   , 144340,\
            rhi.tlhc_S                         , FLOAT_4, 1   , 144348,\
            rhi.t                              , INT_4  , 1   , 144572,\
            rhi.trhc_A                         , FLOAT_4, 1   , 144356,\
            rhi.trhc_R                         , FLOAT_4, 1   , 144352,\
            rhi.trhc_S                         , FLOAT_4, 1   , 144360,\
            rhi.user0                          , FLOAT_4, 1   , 143976,\
            rhi.user1                          , FLOAT_4, 1   , 143980,\
            rhi.user2                          , FLOAT_4, 1   , 143984,\
            rhi.user3                          , FLOAT_4, 1   , 143988,\
            rhi.user4                          , FLOAT_4, 1   , 143992,\
            rhi.user5                          , FLOAT_4, 1   , 143996,\
            rhi.user6                          , FLOAT_4, 1   , 144000,\
            rhi.user7                          , FLOAT_4, 1   , 144004,\
            rhi.user8                          , FLOAT_4, 1   , 144008,\
            rhi.user9                          , FLOAT_4, 1   , 144012,\
            rhi.user10                         , FLOAT_4, 1   , 144016,\
            rhi.user11                         , FLOAT_4, 1   , 144020,\
            rhi.user12                         , FLOAT_4, 1   , 144024,\
            rhi.user13                         , FLOAT_4, 1   , 144028,\
            rhi.user14                         , FLOAT_4, 1   , 144032,\
            rhi.user15                         , FLOAT_4, 1   , 144036,\
            rhi.user16                         , FLOAT_4, 1   , 144040,\
            rhi.user17                         , FLOAT_4, 1   , 144044,\
            rhi.user18                         , FLOAT_4, 1   , 144048,\
            rhi.user19                         , FLOAT_4, 1   , 144052,\
            rhi.user20                         , FLOAT_4, 1   , 144056,\
            rhi.user21                         , FLOAT_4, 1   , 144060,\
            rhi.user22                         , FLOAT_4, 1   , 144064,\
            rhi.user23                         , FLOAT_4, 1   , 144076,\
            rhi.user24                         , FLOAT_4, 1   , 144080,\
            rhi.user25                         , FLOAT_4, 1   , 144144,\
            rhi.user26                         , FLOAT_4, 1   , 144148,\
            rhi.user27                         , FLOAT_4, 1   , 144152,\
            rhi.user28                         , FLOAT_4, 1   , 144156,\
            rhi.user29                         , FLOAT_4, 1   , 144160,\
            rhi.user30                         , FLOAT_4, 1   , 144164,\
            rhi.user31                         , FLOAT_4, 1   , 144168,\
            rhi.user32                         , FLOAT_4, 1   , 144172,\
            rhi.user33                         , FLOAT_4, 1   , 144176,\
            rhi.user34                         , FLOAT_4, 1   , 144180,\
            rhi.user35                         , FLOAT_4, 1   , 144184,\
            rhi.user36                         , FLOAT_4, 1   , 144188,\
            rhi.user37                         , FLOAT_4, 1   , 144192,\
            rhi.user38                         , FLOAT_4, 1   , 144196,\
            rhi.user39                         , FLOAT_4, 1   , 144200,\
            rhi.user40                         , FLOAT_4, 1   , 144204,\
            rhi.user41                         , FLOAT_4, 1   , 144208,\
            rhi.user42                         , FLOAT_4, 1   , 144212,\
            rhi.user43                         , FLOAT_4, 1   , 144216,\
            rhi.user44                         , FLOAT_4, 1   , 144220,\
            rhi.user45                         , FLOAT_4, 1   , 144224,\
            rhi.user46                         , FLOAT_4, 1   , 144228,\
            rhi.user47                         , FLOAT_4, 1   , 144232,\
            rhi.user48                         , FLOAT_4, 1   , 144236,\
            rhi.cname                          , CHAR   , 17  , 145249,\
            rhi.brhc_A                         , FLOAT_4, 1   , 144368,\
            rhi.brhc_R                         , FLOAT_4, 1   , 144364,\
            rhi.brhc_S                         , FLOAT_4, 1   , 144372,\
            rhi.ctr_A                          , FLOAT_4, 1   , 144320,\
            rhi.ctr_R                          , FLOAT_4, 1   , 144316,\
            rhi.ctr_S                          , FLOAT_4, 1   , 144324,\
            rhi.dfov                           , FLOAT_4, 1   , 143924,\
            rhi.freq_dir                       , INT_2  , 1   , 145018,\
            rhi.ctyp                           , INT_2  , 1   , 144952,\
            rhi.loc                            , FLOAT_4, 1   , 143944,\
            rhi.mr_flip                        , INT_2  , 1   , 144928,\
            rhi.nex                            , FLOAT_4, 1   , 143952,\
            rhi.numecho                        , INT_2  , 1   , 144890,\
            rhe.ex_datetime                    , INT_4  , 1   , 140988,\
            rhe.ex_no                          , UINT_2 , 1   , 141044,\
            rhe.magstrength                    , INT_4  , 1   , 140980,\
            rhe.patid                          , CHAR   , 13  , 141368,\
            rhe.patname                        , CHAR   , 25  , 141381,\
            rhe.refphy                         , CHAR   , 33  , 141174,\
            rhe.reqnum                         , CHAR   , 13  , 141161,\
            rhe.study_uid                      , UID    , 32  , 141450,\
            rhe.dateofbirth                    , CHAR   , 9   , 141693,\
            rhe.patsex                         , INT_2  , 1   , 141068,\
            rhe.hospname                       , CHAR   , 33  , 141335,\
            rhe.ex_sysid                       , CHAR   , 9   , 141312,\
            rhe.uniq_sys_id                    , CHAR   , 16  , 141414,\
            rhe.ex_verscre                     , CHAR   , 2   , 141410,\
            rhs.se_no                          , INT_2  , 1   , 142194,\
            rhs.se_desc                        , CHAR   , 65  , 142318,\
            rhs.entry                          , INT_4  , 1   , 142064,\
            rhs.position                       , INT_4  , 1   , 142060,\
            rhs.series_uid                     , UID    , 32  , 142431,\
            rhs.landmark_uid                   , UID    , 32  , 142463,\
            rhs.anref                          , CHAR   , 3   , 142401,\
        "); 

    } else if ( (int)pfileVersion == 15 ) {

        offsets.assign (" \
            rhr.rh_rdbm_rev                    , FLOAT_4, 1   , 0,\
            rhr.rh_scan_date                   , CHAR   , 10  , 16,\
            rhr.rh_scan_time                   , CHAR   , 8   , 26,\
            rhr.csi_dims                       , INT_2  , 1   , 372,\
            rhr.rh_dab[0].start_rcv            , INT_2  , 1   , 200,\
            rhr.rh_dab[0].stop_rcv             , INT_2  , 1   , 202,\
            rhr.rh_dab[1].start_rcv            , INT_2  , 1   , 204,\
            rhr.rh_dab[1].stop_rcv             , INT_2  , 1   , 206,\
            rhr.rh_dab[2].start_rcv            , INT_2  , 1   , 208,\
            rhr.rh_dab[2].stop_rcv             , INT_2  , 1   , 210,\
            rhr.rh_dab[3].start_rcv            , INT_2  , 1   , 212,\
            rhr.rh_dab[3].stop_rcv             , INT_2  , 1   , 214,\
            rhr.rh_data_collect_type           , INT_2  , 1   , 56,\
            rhr.rh_file_contents               , INT_2  , 1   , 44,\
            rhr.rdb_hdr_off_data               , INT_4  , 1   , 1468,\
            rhr.rh_frame_size                  , UINT_2 , 1   , 80,\
            rhr.rh_point_size                  , INT_2  , 1   , 82,\
            rhr.rh_ps_mps_freq                 , LINT_4 , 1   , 424,\
            rhr.rh_user_usage_tag              , ULINT_4, 1   , 988,\
            rhr.roilenx                        , FLOAT_4, 1   , 380,\
            rhr.roileny                        , FLOAT_4, 1   , 384,\
            rhr.roilenz                        , FLOAT_4, 1   , 388,\
            rhr.spectral_width                 , FLOAT_4, 1   , 368,\
            rhr.xcsi                           , INT_2  , 1   , 374,\
            rhr.ycsi                           , INT_2  , 1   , 376,\
            rhr.zcsi                           , INT_2  , 1   , 378,\
            rhr.rh_logo                        , CHAR   , 10  , 34,\
            rhr.rh_raw_pass_size               , ULINT_4, 1   , 116,\
            rhr.rh_user0                       , FLOAT_4, 1   , 216,\
            rhr.rh_user1                       , FLOAT_4, 1   , 220,\
            rhr.rh_user2                       , FLOAT_4, 1   , 224,\
            rhr.rh_user3                       , FLOAT_4, 1   , 228,\
            rhr.rh_user4                       , FLOAT_4, 1   , 232,\
            rhr.rh_user5                       , FLOAT_4, 1   , 236,\
            rhr.rh_user6                       , FLOAT_4, 1   , 240,\
            rhr.rh_user7                       , FLOAT_4, 1   , 244,\
            rhr.rh_user8                       , FLOAT_4, 1   , 248,\
            rhr.rh_user9                       , FLOAT_4, 1   , 252,\
            rhr.rh_user10                      , FLOAT_4, 1   , 256,\
            rhr.rh_user11                      , FLOAT_4, 1   , 260,\
            rhr.rh_user12                      , FLOAT_4, 1   , 264,\
            rhr.rh_user13                      , FLOAT_4, 1   , 268,\
            rhr.rh_user14                      , FLOAT_4, 1   , 272,\
            rhr.rh_user15                      , FLOAT_4, 1   , 276,\
            rhr.rh_user16                      , FLOAT_4, 1   , 280,\
            rhr.rh_user17                      , FLOAT_4, 1   , 284,\
            rhr.rh_user18                      , FLOAT_4, 1   , 288,\
            rhr.rh_user19                      , FLOAT_4, 1   , 292,\
            rhr.rh_user20                      , FLOAT_4, 1   , 1000,\
            rhr.rh_user21                      , FLOAT_4, 1   , 1004,\
            rhr.rh_user22                      , FLOAT_4, 1   , 1008,\
            rhr.rh_user23                      , FLOAT_4, 1   , 1012,\
            rhr.rh_user24                      , FLOAT_4, 1   , 1016,\
            rhr.rh_user25                      , FLOAT_4, 1   , 1020,\
            rhr.rh_user26                      , FLOAT_4, 1   , 1024,\
            rhr.rh_user27                      , FLOAT_4, 1   , 1028,\
            rhr.rh_user28                      , FLOAT_4, 1   , 1032,\
            rhr.rh_user29                      , FLOAT_4, 1   , 1036,\
            rhr.rh_user30                      , FLOAT_4, 1   , 1040,\
            rhr.rh_user31                      , FLOAT_4, 1   , 1044,\
            rhr.rh_user32                      , FLOAT_4, 1   , 1048,\
            rhr.rh_user33                      , FLOAT_4, 1   , 1052,\
            rhr.rh_user34                      , FLOAT_4, 1   , 1056,\
            rhr.rh_user35                      , FLOAT_4, 1   , 1060,\
            rhr.rh_user36                      , FLOAT_4, 1   , 1064,\
            rhr.rh_user37                      , FLOAT_4, 1   , 1068,\
            rhr.rh_user38                      , FLOAT_4, 1   , 1072,\
            rhr.rh_user39                      , FLOAT_4, 1   , 1076,\
            rhr.rh_user40                      , FLOAT_4, 1   , 1080,\
            rhr.rh_user41                      , FLOAT_4, 1   , 1084,\
            rhr.rh_user42                      , FLOAT_4, 1   , 1088,\
            rhr.rh_user43                      , FLOAT_4, 1   , 1092,\
            rhr.rh_user44                      , FLOAT_4, 1   , 1096,\
            rhr.rh_user45                      , FLOAT_4, 1   , 1100,\
            rhr.rh_user46                      , FLOAT_4, 1   , 1104,\
            rhr.rh_user47                      , FLOAT_4, 1   , 1108,\
            rhr.rh_user48                      , FLOAT_4, 1   , 1112,\
            rhi.psdname                        , CHAR   , 33  , 145132,\
            rhi.scanspacing                    , FLOAT_4, 1   , 143940,\
            rhi.te                             , INT_4  , 1   , 144580,\
            rhi.ti                             , INT_4  , 1   , 144576,\
            rhi.tr                             , INT_4  , 1   , 144572,\
            rhi.tlhc_A                         , FLOAT_4, 1   , 144344,\
            rhi.tlhc_R                         , FLOAT_4, 1   , 144340,\
            rhi.tlhc_S                         , FLOAT_4, 1   , 144348,\
            rhi.t                              , INT_4  , 1   , 144572,\
            rhi.trhc_A                         , FLOAT_4, 1   , 144356,\
            rhi.trhc_R                         , FLOAT_4, 1   , 144352,\
            rhi.trhc_S                         , FLOAT_4, 1   , 144360,\
            rhi.user0                          , FLOAT_4, 1   , 143976,\
            rhi.user1                          , FLOAT_4, 1   , 143980,\
            rhi.user2                          , FLOAT_4, 1   , 143984,\
            rhi.user3                          , FLOAT_4, 1   , 143988,\
            rhi.user4                          , FLOAT_4, 1   , 143992,\
            rhi.user5                          , FLOAT_4, 1   , 143996,\
            rhi.user6                          , FLOAT_4, 1   , 144000,\
            rhi.user7                          , FLOAT_4, 1   , 144004,\
            rhi.user8                          , FLOAT_4, 1   , 144008,\
            rhi.user9                          , FLOAT_4, 1   , 144012,\
            rhi.user10                         , FLOAT_4, 1   , 144016,\
            rhi.user11                         , FLOAT_4, 1   , 144020,\
            rhi.user12                         , FLOAT_4, 1   , 144024,\
            rhi.user13                         , FLOAT_4, 1   , 144028,\
            rhi.user14                         , FLOAT_4, 1   , 144032,\
            rhi.user15                         , FLOAT_4, 1   , 144036,\
            rhi.user16                         , FLOAT_4, 1   , 144040,\
            rhi.user17                         , FLOAT_4, 1   , 144044,\
            rhi.user18                         , FLOAT_4, 1   , 144048,\
            rhi.user19                         , FLOAT_4, 1   , 144052,\
            rhi.user20                         , FLOAT_4, 1   , 144056,\
            rhi.user21                         , FLOAT_4, 1   , 144060,\
            rhi.user22                         , FLOAT_4, 1   , 144064,\
            rhi.user23                         , FLOAT_4, 1   , 144076,\
            rhi.user24                         , FLOAT_4, 1   , 144080,\
            rhi.user25                         , FLOAT_4, 1   , 144144,\
            rhi.user26                         , FLOAT_4, 1   , 144148,\
            rhi.user27                         , FLOAT_4, 1   , 144152,\
            rhi.user28                         , FLOAT_4, 1   , 144156,\
            rhi.user29                         , FLOAT_4, 1   , 144160,\
            rhi.user30                         , FLOAT_4, 1   , 144164,\
            rhi.user31                         , FLOAT_4, 1   , 144168,\
            rhi.user32                         , FLOAT_4, 1   , 144172,\
            rhi.user33                         , FLOAT_4, 1   , 144176,\
            rhi.user34                         , FLOAT_4, 1   , 144180,\
            rhi.user35                         , FLOAT_4, 1   , 144184,\
            rhi.user36                         , FLOAT_4, 1   , 144188,\
            rhi.user37                         , FLOAT_4, 1   , 144192,\
            rhi.user38                         , FLOAT_4, 1   , 144196,\
            rhi.user39                         , FLOAT_4, 1   , 144200,\
            rhi.user40                         , FLOAT_4, 1   , 144204,\
            rhi.user41                         , FLOAT_4, 1   , 144208,\
            rhi.user42                         , FLOAT_4, 1   , 144212,\
            rhi.user43                         , FLOAT_4, 1   , 144216,\
            rhi.user44                         , FLOAT_4, 1   , 144220,\
            rhi.user45                         , FLOAT_4, 1   , 144224,\
            rhi.user46                         , FLOAT_4, 1   , 144228,\
            rhi.user47                         , FLOAT_4, 1   , 144232,\
            rhi.user48                         , FLOAT_4, 1   , 144236,\
            rhi.cname                          , CHAR   , 17  , 145249,\
            rhi.brhc_A                         , FLOAT_4, 1   , 144368,\
            rhi.brhc_R                         , FLOAT_4, 1   , 144364,\
            rhi.brhc_S                         , FLOAT_4, 1   , 144372,\
            rhi.ctr_A                          , FLOAT_4, 1   , 144320,\
            rhi.ctr_R                          , FLOAT_4, 1   , 144316,\
            rhi.ctr_S                          , FLOAT_4, 1   , 144324,\
            rhi.dfov                           , FLOAT_4, 1   , 143924,\
            rhi.freq_dir                       , INT_2  , 1   , 145014,\
            rhi.ctyp                           , INT_2  , 1   , 144948,\
            rhi.loc                            , FLOAT_4, 1   , 143944,\
            rhi.mr_flip                        , INT_2  , 1   , 144924,\
            rhi.nex                            , FLOAT_4, 1   , 143952,\
            rhi.numecho                        , INT_2  , 1   , 144890,\
            rhe.ex_datetime                    , INT_4  , 1   , 140988,\
            rhe.ex_no                          , UINT_2 , 1   , 141044,\
            rhe.magstrength                    , INT_4  , 1   , 140980,\
            rhe.patid                          , CHAR   , 13  , 141368,\
            rhe.patname                        , CHAR   , 25  , 141381,\
            rhe.refphy                         , CHAR   , 33  , 141174,\
            rhe.reqnum                         , CHAR   , 13  , 141161,\
            rhe.study_uid                      , UID    , 32  , 141450,\
            rhe.dateofbirth                    , CHAR   , 9   , 141693,\
            rhe.patsex                         , INT_2  , 1   , 141068,\
            rhe.hospname                       , CHAR   , 33  , 141335,\
            rhe.ex_sysid                       , CHAR   , 9   , 141312,\
            rhe.uniq_sys_id                    , CHAR   , 16  , 141414,\
            rhe.ex_verscre                     , CHAR   , 2   , 141410,\
            rhs.se_no                          , INT_2  , 1   , 142194,\
            rhs.se_desc                        , CHAR   , 65  , 142318,\
            rhs.entry                          , INT_4  , 1   , 142064,\
            rhs.position                       , INT_4  , 1   , 142060,\
            rhs.series_uid                     , UID    , 32  , 142431,\
            rhs.landmark_uid                   , UID    , 32  , 142463,\
            rhs.anref                          , CHAR   , 3   , 142401,\
        "); 

    } else if ( (int)pfileVersion == 20 ) {

        offsets.assign (" \
            rhr.rh_rdbm_rev                    , FLOAT_4, 1   , 0,\
            rhr.rh_scan_date                   , CHAR   , 10  , 16,\
            rhr.rh_scan_time                   , CHAR   , 8   , 26,\
            rhr.csi_dims                       , INT_2  , 1   , 372,\
            rhr.rh_dab[0].start_rcv            , INT_2  , 1   , 200,\
            rhr.rh_dab[0].stop_rcv             , INT_2  , 1   , 202,\
            rhr.rh_dab[1].start_rcv            , INT_2  , 1   , 204,\
            rhr.rh_dab[1].stop_rcv             , INT_2  , 1   , 206,\
            rhr.rh_dab[2].start_rcv            , INT_2  , 1   , 208,\
            rhr.rh_dab[2].stop_rcv             , INT_2  , 1   , 210,\
            rhr.rh_dab[3].start_rcv            , INT_2  , 1   , 212,\
            rhr.rh_dab[3].stop_rcv             , INT_2  , 1   , 214,\
            rhr.rh_data_collect_type           , INT_2  , 1   , 56,\
            rhr.rh_file_contents               , INT_2  , 1   , 44,\
            rhr.rdb_hdr_off_data               , INT_4  , 1   , 1468,\
            rhr.rh_frame_size                  , UINT_2 , 1   , 80,\
            rhr.rh_point_size                  , INT_2  , 1   , 82,\
            rhr.rh_ps_mps_freq                 , UINT_4 , 1   , 424,\
            rhr.rh_user_usage_tag              , UINT_4 , 1   , 988,\
            rhr.roilenx                        , FLOAT_4, 1   , 380,\
            rhr.roileny                        , FLOAT_4, 1   , 384,\
            rhr.roilenz                        , FLOAT_4, 1   , 388,\
            rhr.spectral_width                 , FLOAT_4, 1   , 368,\
            rhr.xcsi                           , INT_2  , 1   , 374,\
            rhr.ycsi                           , INT_2  , 1   , 376,\
            rhr.zcsi                           , INT_2  , 1   , 378,\
            rhr.rh_logo                        , CHAR   , 10  , 34,\
            rhr.rh_raw_pass_size               , LINT_4 , 1   , 1660,\
            rhr.rh_user0                       , FLOAT_4, 1   , 216,\
            rhr.rh_user1                       , FLOAT_4, 1   , 220,\
            rhr.rh_user2                       , FLOAT_4, 1   , 224,\
            rhr.rh_user3                       , FLOAT_4, 1   , 228,\
            rhr.rh_user4                       , FLOAT_4, 1   , 232,\
            rhr.rh_user5                       , FLOAT_4, 1   , 236,\
            rhr.rh_user6                       , FLOAT_4, 1   , 240,\
            rhr.rh_user7                       , FLOAT_4, 1   , 244,\
            rhr.rh_user8                       , FLOAT_4, 1   , 248,\
            rhr.rh_user9                       , FLOAT_4, 1   , 252,\
            rhr.rh_user10                      , FLOAT_4, 1   , 256,\
            rhr.rh_user11                      , FLOAT_4, 1   , 260,\
            rhr.rh_user12                      , FLOAT_4, 1   , 264,\
            rhr.rh_user13                      , FLOAT_4, 1   , 268,\
            rhr.rh_user14                      , FLOAT_4, 1   , 272,\
            rhr.rh_user15                      , FLOAT_4, 1   , 276,\
            rhr.rh_user16                      , FLOAT_4, 1   , 280,\
            rhr.rh_user17                      , FLOAT_4, 1   , 284,\
            rhr.rh_user18                      , FLOAT_4, 1   , 288,\
            rhr.rh_user19                      , FLOAT_4, 1   , 292,\
            rhr.rh_user20                      , FLOAT_4, 1   , 1000,\
            rhr.rh_user21                      , FLOAT_4, 1   , 1004,\
            rhr.rh_user22                      , FLOAT_4, 1   , 1008,\
            rhr.rh_user23                      , FLOAT_4, 1   , 1012,\
            rhr.rh_user24                      , FLOAT_4, 1   , 1016,\
            rhr.rh_user25                      , FLOAT_4, 1   , 1020,\
            rhr.rh_user26                      , FLOAT_4, 1   , 1024,\
            rhr.rh_user27                      , FLOAT_4, 1   , 1028,\
            rhr.rh_user28                      , FLOAT_4, 1   , 1032,\
            rhr.rh_user29                      , FLOAT_4, 1   , 1036,\
            rhr.rh_user30                      , FLOAT_4, 1   , 1040,\
            rhr.rh_user31                      , FLOAT_4, 1   , 1044,\
            rhr.rh_user32                      , FLOAT_4, 1   , 1048,\
            rhr.rh_user33                      , FLOAT_4, 1   , 1052,\
            rhr.rh_user34                      , FLOAT_4, 1   , 1056,\
            rhr.rh_user35                      , FLOAT_4, 1   , 1060,\
            rhr.rh_user36                      , FLOAT_4, 1   , 1064,\
            rhr.rh_user37                      , FLOAT_4, 1   , 1068,\
            rhr.rh_user38                      , FLOAT_4, 1   , 1072,\
            rhr.rh_user39                      , FLOAT_4, 1   , 1076,\
            rhr.rh_user40                      , FLOAT_4, 1   , 1080,\
            rhr.rh_user41                      , FLOAT_4, 1   , 1084,\
            rhr.rh_user42                      , FLOAT_4, 1   , 1088,\
            rhr.rh_user43                      , FLOAT_4, 1   , 1092,\
            rhr.rh_user44                      , FLOAT_4, 1   , 1096,\
            rhr.rh_user45                      , FLOAT_4, 1   , 1100,\
            rhr.rh_user46                      , FLOAT_4, 1   , 1104,\
            rhr.rh_user47                      , FLOAT_4, 1   , 1108,\
            rhr.rh_user48                      , FLOAT_4, 1   , 1112,\
            rhi.psdname                        , CHAR   , 33  , 148972,\
            rhi.scanspacing                    , FLOAT_4, 1   , 147660,\
            rhi.te                             , INT_4  , 1   , 148404,\
            rhi.ti                             , INT_4  , 1   , 148400,\
            rhi.tr                             , INT_4  , 1   , 148396,\
            rhi.tlhc_A                         , FLOAT_4, 1   , 148064,\
            rhi.tlhc_R                         , FLOAT_4, 1   , 148060,\
            rhi.tlhc_S                         , FLOAT_4, 1   , 148068,\
            rhi.t                              , INT_4  , 1   , 148396,\
            rhi.trhc_A                         , FLOAT_4, 1   , 148076,\
            rhi.trhc_R                         , FLOAT_4, 1   , 148072,\
            rhi.trhc_S                         , FLOAT_4, 1   , 148080,\
            rhi.user0                          , FLOAT_4, 1   , 147696,\
            rhi.user1                          , FLOAT_4, 1   , 147700,\
            rhi.user2                          , FLOAT_4, 1   , 147704,\
            rhi.user3                          , FLOAT_4, 1   , 147708,\
            rhi.user4                          , FLOAT_4, 1   , 147712,\
            rhi.user5                          , FLOAT_4, 1   , 147716,\
            rhi.user6                          , FLOAT_4, 1   , 147720,\
            rhi.user7                          , FLOAT_4, 1   , 147724,\
            rhi.user8                          , FLOAT_4, 1   , 147728,\
            rhi.user9                          , FLOAT_4, 1   , 147732,\
            rhi.user10                         , FLOAT_4, 1   , 147736,\
            rhi.user11                         , FLOAT_4, 1   , 147740,\
            rhi.user12                         , FLOAT_4, 1   , 147744,\
            rhi.user13                         , FLOAT_4, 1   , 147748,\
            rhi.user14                         , FLOAT_4, 1   , 147752,\
            rhi.user15                         , FLOAT_4, 1   , 147756,\
            rhi.user16                         , FLOAT_4, 1   , 147760,\
            rhi.user17                         , FLOAT_4, 1   , 147764,\
            rhi.user18                         , FLOAT_4, 1   , 147768,\
            rhi.user19                         , FLOAT_4, 1   , 147772,\
            rhi.user20                         , FLOAT_4, 1   , 147776,\
            rhi.user21                         , FLOAT_4, 1   , 147780,\
            rhi.user22                         , FLOAT_4, 1   , 147784,\
            rhi.user23                         , FLOAT_4, 1   , 147796,\
            rhi.user24                         , FLOAT_4, 1   , 147800,\
            rhi.user25                         , FLOAT_4, 1   , 147864,\
            rhi.user26                         , FLOAT_4, 1   , 147868,\
            rhi.user27                         , FLOAT_4, 1   , 147872,\
            rhi.user28                         , FLOAT_4, 1   , 147876,\
            rhi.user29                         , FLOAT_4, 1   , 147880,\
            rhi.user30                         , FLOAT_4, 1   , 147884,\
            rhi.user31                         , FLOAT_4, 1   , 147888,\
            rhi.user32                         , FLOAT_4, 1   , 147892,\
            rhi.user33                         , FLOAT_4, 1   , 147896,\
            rhi.user34                         , FLOAT_4, 1   , 147900,\
            rhi.user35                         , FLOAT_4, 1   , 147904,\
            rhi.user36                         , FLOAT_4, 1   , 147908,\
            rhi.user37                         , FLOAT_4, 1   , 147912,\
            rhi.user38                         , FLOAT_4, 1   , 147916,\
            rhi.user39                         , FLOAT_4, 1   , 147920,\
            rhi.user40                         , FLOAT_4, 1   , 147924,\
            rhi.user41                         , FLOAT_4, 1   , 147928,\
            rhi.user42                         , FLOAT_4, 1   , 147932,\
            rhi.user43                         , FLOAT_4, 1   , 147936,\
            rhi.user44                         , FLOAT_4, 1   , 147940,\
            rhi.user45                         , FLOAT_4, 1   , 147944,\
            rhi.user46                         , FLOAT_4, 1   , 147948,\
            rhi.user47                         , FLOAT_4, 1   , 147952,\
            rhi.user48                         , FLOAT_4, 1   , 147956,\
            rhi.cname                          , CHAR   , 17  , 149089,\
            rhi.brhc_A                         , FLOAT_4, 1   , 148088,\
            rhi.brhc_R                         , FLOAT_4, 1   , 148084,\
            rhi.brhc_S                         , FLOAT_4, 1   , 148092,\
            rhi.ctr_A                          , FLOAT_4, 1   , 148040,\
            rhi.ctr_R                          , FLOAT_4, 1   , 148036,\
            rhi.ctr_S                          , FLOAT_4, 1   , 148044,\
            rhi.dfov                           , FLOAT_4, 1   , 147644,\
            rhi.freq_dir                       , INT_2  , 1   , 148840,\
            rhi.ctyp                           , INT_2  , 1   , 148774,\
            rhi.loc                            , FLOAT_4, 1   , 147664,\
            rhi.mr_flip                        , INT_2  , 1   , 148752,\
            rhi.nex                            , FLOAT_4, 1   , 147672,\
            rhi.numecho                        , INT_2  , 1   , 148718,\
            rhe.ex_datetime                    , INT_4  , 1   , 143400,\
            rhe.ex_no                          , UINT_2 , 1   , 143516,\
            rhe.magstrength                    , INT_4  , 1   , 143392,\
            rhe.patid                          , CHAR   , 65  , 144401,\
            rhe.patname                        , CHAR   , 65  , 144336,\
            rhe.refphy                         , CHAR   , 65  , 143877,\
            rhe.reqnum                         , CHAR   , 17  , 144466,\
            rhe.study_uid                      , UID    , 32  , 144240,\
            rhe.dateofbirth                    , CHAR   , 9   , 144483,\
            rhe.patsex                         , INT_2  , 1   , 143540,\
            rhe.hospname                       , CHAR   , 33  , 144163,\
            rhe.ex_sysid                       , CHAR   , 9   , 144140,\
            rhe.uniq_sys_id                    , CHAR   , 16  , 144204,\
            rhe.ex_verscre                     , CHAR   , 2   , 144200,\
            rhs.se_no                          , INT_2  , 1   , 145622,\
            rhs.se_desc                        , CHAR   , 65  , 145762,\
            rhs.entry                          , INT_4  , 1   , 145424,\
            rhs.position                       , INT_4  , 1   , 145420,\
            rhs.series_uid                     , UID    , 32  , 145875,\
            rhs.landmark_uid                   , UID    , 32  , 145907,\
            rhs.anref                          , CHAR   , 3   , 145845,\
        "); 

    }

    return offsets; 

}


/*!
 *
 */
void svkGEPFileReader::UpdateProgressCallback(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<svkGEPFileReader*>(thisObject)->SetProgressText( ( static_cast<svkGEPFileMapper*>(subject)->GetProgressText()).c_str() );
    static_cast<svkGEPFileReader*>(thisObject)->UpdateProgress(*(double*)(callData));

}
