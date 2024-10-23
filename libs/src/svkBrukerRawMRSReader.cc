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


#include <svkBrukerRawMRSReader.h>
#include <svkTypeUtils.h>
#include </usr/include/vtk/vtkGlobFileNames.h>
#include </usr/include/vtk/vtkSortFileNames.h>
#include </usr/include/vtk/vtkStringArray.h>
#include </usr/include/vtk/vtkDebugLeaks.h>

#include <sys/stat.h>


using namespace svk;


vtkStandardNewMacro(svkBrukerRawMRSReader);


/*!
 *
 */
svkBrukerRawMRSReader::svkBrukerRawMRSReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkBrukerRawMRSReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->mapper = NULL;
    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgressCallback );
    this->progressCallback->SetClientData( (void*)this );

}


/*!
 *
 */
svkBrukerRawMRSReader::~svkBrukerRawMRSReader()
{

    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
/*
    if ( this->procparFile != NULL )  {
        delete procparFile; 
        this->procparFile = NULL; 
    }
*/
}

/*!
 *  Check to see if the extension indicates a Bruker Raw MRS file.  If so, try 
 *  to open the file for reading.  If that works, then return a success code. 
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkBrukerRawMRSReader::CanReadFile(const char* fname)
{

    string fileToCheck(fname);

    if( fileToCheck.size() >= 3 ) {

        // Also see "vtkImageReader2::GetFileExtensions method" 
        if (
            (  fileToCheck.substr( fileToCheck.size() - 3 ) == "ser" ) || 
            (  fileToCheck.substr( fileToCheck.size() - 3 ) == "fid" )  
        ) {
            FILE *fp = fopen(fname, "rb");
            if (fp) {
                fclose(fp);
                vtkDebugMacro(
                    << this->GetClassName() << "::CanReadFile(): It's a Bruker RAW MRS (ser) File: " << fileToCheck
                );

                //  Now check to see if it's a supported sequence: 
                this->SetFileName( fname );
                this->ParseParamFiles();
                if ( this->mapper != NULL )  {
                    mapper->Delete();
                    this->mapper = NULL;
                }
                //  should be a mapper factory to get psd specific instance:
                this->mapper = this->GetBrukerRawMRSMapper();
                if ( this->mapper == NULL ) {
                    cout  << " Not a know Bruker MRS sequnce.  Can not read file."   << endl;
                    return 0;
                }

                return 1;
            }
        } else {
            vtkDebugMacro(
                << this->GetClassName() << "::CanReadFile(): It's NOT a Bruker MRS File: " << fileToCheck
            );
            return 0;
        }
    } else {
        vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): is NOT a valid file: " << fileToCheck);
        return 0;
    }
}




/*!
 *
 */
int svkBrukerRawMRSReader::GetNumPixelsInVol()
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
int svkBrukerRawMRSReader::GetNumSlices()
{
    return (this->GetDataExtent())[5] + 1;
}


/*! 
 *  Parses the param files (method, subject, etc.) 
 *  and initializes an stl map of key value pairs to pass to the mapper 
 *  for initialization of the DICOM header. 
 */
void svkBrukerRawMRSReader::ParseParamFiles( )    
{
    string path = svkImageReader2::GetFilePath( this->GetFileName() );
    string paramFilePath;

    paramFilePath = path + "/../subject"; 
    this->ParseParamFileToMap( paramFilePath ); 

    paramFilePath = path + "/method"; 
    this->ParseParamFileToMap( paramFilePath ); 

    paramFilePath = path + "/acqp"; 
    this->ParseParamFileToMap( paramFilePath ); 


    if (this->GetDebug()) {
        this->PrintParamKeyValuePairs(); 
    }
}


/*!
 *
 */
void svkBrukerRawMRSReader::ParseParamFileToMap( string paramFileName )    
{
    
    vtkGlobFileNames* paramFileGlob = vtkGlobFileNames::New();
    paramFileGlob->AddFileNames( paramFileName.c_str() );

    if ( paramFileGlob->GetFileNames()->GetNumberOfValues() == 1 ) {

        try { 

            this->paramFile = new ifstream();
            this->paramFile->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

            string paramFileName( paramFileGlob->GetNthFileName(0) ); 

            this->paramFile->open( paramFileName.c_str(), ifstream::in );
            if ( ! this->paramFile->is_open() ) {
                throw runtime_error( "Could not open Bruker param file: " + paramFileName);
            } 

            this->paramFileSize = this->GetFileSize( this->paramFile );
            while (! this->paramFile->eof() ) {
                if (this->GetParamKeyValuePair() != 0 ) {
                    break;
                }
            }

            this->paramFile->close();

        } catch (const exception& e) {
            cerr << "ERROR opening or reading Bruker param file: " << e.what() << endl;
        }


    } else {
        cout << "WARNING: " << this->GetClassName() << " Can't parse Bruker param file" << endl; 
        cout << "         " << paramFileName  << endl;
    }
    paramFileGlob->Delete();

    return; 
}


/*! 
 *  Utility function to read key/values meta data pairs from Bruker param files 
 *  and set the delimited key/value pair into the stl map.  
 *  Returns -1 if can't parse line. 
 */
int svkBrukerRawMRSReader::GetParamKeyValuePair( )    
{
    int status = 0; 

    istringstream* iss = new istringstream();

    string keyString;
    string* valueString = new string("");

    try {

        this->ReadLine(this->paramFile, iss); 

        size_t  delimPosition; 
        size_t  position; 
        size_t  commentPosition; 
        size_t  commaPosition; 
        size_t  parenthesisPosition; 
        size_t  braPosition; 
        size_t  ketPosition; 
        string  tmp; 

        if ( this->paramFile->tellg() < this->paramFileSize - 1 ) {

            //  check for comment line: $$ at start: 
            commentPosition = iss->str().find_first_of("$$");
            if ( commentPosition == 0 ) {
                return status; 
            }

            //  find first white space position before "key" string: 
            delimPosition = iss->str().find_first_of('=');
            if (delimPosition != string::npos ) {
                //  skip two to get rid of leading ##
                keyString.assign( iss->str().substr(2, delimPosition-2) );
                position = iss->str().find_first_of('$');
                if ( position != string::npos ) {
                    keyString.assign( keyString.substr(1, delimPosition-3) );
                }
            } 
            //cout << "PARAM KEY: " << keyString << endl;

            //  ====================================================================
            //  Now check if the value is scalar or an array of values: 
            //  If it's an array of values, the array size will be encoded in parenthesis. 
            //  Parse the following lines for the value 
            //  Heuristically, it looks like: 
            //       =( val ) indicates the size of the array on following lines
            //       =(vals) indicates the value (no space after parenthesis).   
            //  ====================================================================
            parenthesisPosition = iss->str().find("( ");
            if ( parenthesisPosition != string::npos) {
                //  It's an array on the subsequent lines

                string tmpString; 
                string numElementsString = "1";
                string sizeOfElementsString;
                //  is there a comma
                tmpString.assign( iss->str().substr( parenthesisPosition + 2 ) );
                commaPosition = tmpString.find_first_of(','); 
                if ( commaPosition != string::npos ) { 
                    numElementsString.assign( tmpString.substr( 0, commaPosition ) ); 
                    tmpString.assign( tmpString.substr(commaPosition + 1) ); 
                }     
                position = tmpString.find_last_of(' ');
                sizeOfElementsString.assign( tmpString.substr( 0, position ) );
                //cout << "NUM, SIZE: " << numElementsString << " " <<  sizeOfElementsString << endl;
                this->ReadLine(this->paramFile, iss); 

                //  Is it a string ( in side angle brackets <>)?
                //  This can be a multi line string
                braPosition = iss->str().find_first_of('<');    
                if ( braPosition != string::npos) {
                    valueString->assign(""); 
                    ketPosition = string::npos; 
                    int numElements = 0; 
                    while ( ketPosition == string::npos || numElements < svkTypeUtils::StringToInt(numElementsString)) {
                        ketPosition = iss->str().find_last_of('>');    
                        valueString->append( iss->str() ); 
                        if ( ketPosition == string::npos || numElements < svkTypeUtils::StringToInt(numElementsString) ) {
                            this->ReadLine(this->paramFile, iss); 
                        } 
                        if (ketPosition != string::npos) { 
                            numElements += 1; 
                            //cout << "NUM ELEMENTS: " << numElements << endl;
                        } 
                        //cout << "LINE: " << iss->str() << endl;
                    } 
                } else {
                    valueString->append( iss->str() ); 
                    char lastCharacter = *(valueString->rbegin());
                    // If the last character is a space, then its continued on the next line
                    while( lastCharacter == ' ' ) {
                        this->ReadLine(this->paramFile, iss);
                        valueString->append( iss->str() );
                        lastCharacter = *(valueString->rbegin());
                    }
                }

            } else { 
                //  if the value is in parenthesis, read until end of parenthesis: 
                parenthesisPosition = iss->str().find('(');
                if ( parenthesisPosition == string::npos ) {
                    valueString->assign(iss->str().substr(delimPosition + 1)); 
                } else { 
                    valueString->assign(iss->str().substr(delimPosition + 1)); 
                    parenthesisPosition = iss->str().find(')');
                    while ( parenthesisPosition == string::npos ) {  
                        this->ReadLine(this->paramFile, iss); 
                        valueString->append( iss->str() ); 
                        parenthesisPosition = iss->str().find(')');
                    } 
                }  
            
            }

            //cout << "VALUE: " << *valueString << endl;

            //this->GetParamValueArray( valueString );

            //  Assign elements to map vectors: 
            this->ParseAndSetParamStringElements(keyString, *valueString);

        } else { 
            this->paramFile->seekg(0, ios::end);     
        }

    } catch (const exception& e) {
        if (this->GetDebug()) {
            cout <<  "ERROR reading line: " << e.what() << endl;
        }
        status = -1;  
    }

    delete valueString; 
    delete iss; 

    return status; 
}


/*! 
 *  Utility function to read a single line from the fdf file stream.
 */
void svkBrukerRawMRSReader::ReadLine(ifstream* fs, istringstream* iss)    
{
    char line[2000];
    iss->clear();    
    fs->getline(line, 2000);
    iss->str(string(line));
}


/*!
 *  Push key value pairs into the map's value vector: 
 *  mapFor values that are space delimited lists, put each element into the value 
 *  vector. 
 */
void svkBrukerRawMRSReader::ParseAndSetParamStringElements(string key, string valueArray ) 
{

    vector <string> paramVector;

    this->AssignParamVectorElements(&paramVector, valueArray);

    this->paramMap[key] = paramVector;
}


/*!
 *  Utility method to determine how many space delimited elements 
 *  are in current propcar string. 
 */
int svkBrukerRawMRSReader::GetNumberOfParamElements( string* valueString )
{
    int numElementsParsed = 0; 
    size_t  position; 
    string tmpString = string(*valueString);


    //  If it's a quoted string, ignore spaces:
    int endQuote = 1;
    if ( ( position = tmpString.find('"') ) != string::npos) {
        while ( ( position = tmpString.find('"') ) != string::npos) {
            endQuote *= -1;
            tmpString = tmpString.substr( position + 1 );
            if (endQuote == 1) {
                numElementsParsed++;  
            }
        } 

    } else {
   
        //  If no space delimiter is found and the string has content, then
        //  assume 1 element.  Otherwise, iterate through space delimited 
        //  elements:
        if ( tmpString.find(' ') == string::npos) {
            if (tmpString.length() >= 1) {
                numElementsParsed++;  
            }
        } else {
            //  if a space was found increment by 1, then see how many more spaces can be found:
            numElementsParsed++;  
            while ( ( position = tmpString.find(' ') ) != string::npos) {
                tmpString.erase( position, 1 );
                numElementsParsed++;  
            } 
        }
    }

    return numElementsParsed; 
}


/*!
 *  Read lines from propcar until all elements of value have been parsed. 
 */
void svkBrukerRawMRSReader::GetParamValueArray( string* valueString )
{
/*
    istringstream* iss = new istringstream();
    this->ReadLine(this->procparFile, iss); 

    size_t  position; 
    position = iss->str().find_first_of(' ');
    if (position != string::npos) {

        istringstream* issTmp = new istringstream();
        issTmp->str( iss->str().substr(0, position) );
        int    numElements;
        *issTmp >> numElements; 
        delete issTmp;

        //  Now parse that many elements: 
        if ( position != iss->str().length() ) {
            valueString->append( iss->str().substr(position + 1) );
        }

        valueString->assign( this->StripWhite(*valueString) );

        //  parse additional lines if num elements in value hasn't been extracted yet:
        while ( this->GetNumberOfProcparElements( valueString ) < numElements ) {
            this->ReadLine(this->procparFile, iss); 
            valueString->append( " " );
            valueString->append( iss->str() );
            valueString->assign( this->StripWhite(*valueString) );
        }
    } 
    delete iss;     
*/
}



/*!
 *  Prints the key value pairs parsed from the header. 
 */
void svkBrukerRawMRSReader::PrintParamKeyValuePairs()
{

    //  Print out key value pairs parsed from header:
    map< string, vector< string > >::iterator mapIter;
    for ( mapIter = this->paramMap.begin(); mapIter != this->paramMap.end(); ++mapIter ) {
     
        cout << this->GetClassName() << " " << mapIter->first << " = " << endl;

        vector < string >::iterator it;
        for ( it = this->paramMap[mapIter->first].begin() ; it < this->paramMap[mapIter->first].end(); it++ ) {

            //vector<string> tmpVector = *it;  
            //vector<string>::iterator vecIter;

            //for ( vecIter = tmpVector.begin(); vecIter != tmpVector.end(); ++vecIter ) {
                cout << "       ->" << *it<< endl ;
            //}
            cout << endl;
        }
        cout << endl;
    }
}



/*!
 *  Create an svkBrukerRawMRS mapper of the appropriate type for the present sequence.  
 */
svkBrukerRawMRSMapper* svkBrukerRawMRSReader::GetBrukerRawMRSMapper()
{
    svkBrukerRawMRSMapper* aMapper = NULL;

    string sequence = this->paramMap["Method"][0];

    //convert to lower case:
    string::iterator it;
    for ( it = sequence.begin(); it < sequence.end(); it++ ) {
        *it = (char)tolower(*it);
    }

    if ( sequence.find("mm_csi") != string::npos ) {

        aMapper = svkBrukerRawMRSMapper::New();

    } else {
        cout <<  endl;
        cout << "===============================" << endl;
        cout << "===============================" << endl;
        cout << "Not a supported Bruker sequence" << endl;
        cout << "===============================" << endl;
        cout << "===============================" << endl;
        cout <<  endl;
    }

    return aMapper;
}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called after ExecuteInformation()
 */
void svkBrukerRawMRSReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
{

    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );
    svkMrsImageData* data = svkMrsImageData::SafeDownCast( this->AllocateOutputData(output, outInfo) );

    if ( this->FileName ) {
        this->mapper->ReadSerFile( this->FileName, data );
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
void svkBrukerRawMRSReader::ExecuteInformation()
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
void svkBrukerRawMRSReader::InitDcmHeader()
{

    vtkDebugMacro( << this->GetClassName() << "::InitDcmHeader()" );

    svkMRSIOD* iod = svkMRSIOD::New();
    iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    iod->InitDcmHeader();

    //  Read the fid header into a map of values used to initialize the
    //  DICOM header. 
    this->ParseParamFiles();

    //  should be a mapper factory to get psd specific instance:
    if ( this->mapper != NULL )  {
        this->mapper->Delete();
        this->mapper = NULL;
    }
    this->mapper = this->GetBrukerRawMRSMapper();
    this->mapper->AddObserver(vtkCommand::ProgressEvent, this->progressCallback);

    //  all the IE initialization modules would be contained within the mapper
    this->mapper->InitializeDcmHeader(
        paramMap,
        this->GetOutput()->GetDcmHeader(),
        iod,
        this->GetSwapBytes()
    );
    this->RemoveObserver(this->progressCallback);


    if (this->GetDebug()) {
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }

    iod->Delete();
}


/*!
 *  paramVector is the vector of values parsed from valueArray that get set 
 *  into map data structure. 
 */
void svkBrukerRawMRSReader::AssignParamVectorElements(vector<string>* paramVector, string valueArray)
{
    size_t pos = 0;
    istringstream* iss = new istringstream();
    string tmpString;

    int endQuote = 1;
    //  Parse bracketed strings separately from unquoted values:    

    if ( ( pos = valueArray.find('<', pos) ) != string::npos) {
        endQuote *= -1;
        while ( ( pos = valueArray.find('>', pos + 1) ) != string::npos) {
            endQuote *= -1;
            if (endQuote == 1) {
                tmpString.assign( valueArray.substr(1, pos - 1) );
                paramVector->push_back(tmpString);
                valueArray.assign( valueArray.substr(pos + 1) );
                pos = string::npos;
            }
        }

    } else {
        paramVector->push_back(valueArray);
    }

    delete iss;
}



/*!
 *  Returns the file type enum 
 */
svkDcmHeader::DcmPixelDataFormat svkBrukerRawMRSReader::GetFileType()
{
    svkDcmHeader::DcmPixelDataFormat format = svkDcmHeader::SIGNED_FLOAT_4;

    return format;
}


/*!
 *
 */
int svkBrukerRawMRSReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}


/*!
 * 
 */
void svkBrukerRawMRSReader::SetProgressText( string progressText )
{
    this->progressText = progressText;
}


/*!
 * 
 */
void svkBrukerRawMRSReader::UpdateProgress(double amount)
{
    this->InvokeEvent(vtkCommand::ProgressEvent,static_cast<void *>(&amount));
}


/*!
 * 
 */
void svkBrukerRawMRSReader::UpdateProgressCallback(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<svkBrukerRawMRSReader*>(thisObject)->UpdateProgress(*(double*)(callData));
}

