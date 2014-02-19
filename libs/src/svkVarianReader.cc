/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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


#include <svkVarianReader.h>
#include <vtkGlobFileNames.h>
#include <vtkSortFileNames.h>
#include <vtkStringArray.h>
#include <vtkDebugLeaks.h>

#include <sys/stat.h>


using namespace svk;


vtkCxxRevisionMacro(svkVarianReader, "$Rev$");


/*!
 *
 */
svkVarianReader::svkVarianReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkVarianReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->procparFile = NULL;
}


/*!
 *
 */
svkVarianReader::~svkVarianReader()
{

    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if ( this->procparFile != NULL )  {
        delete procparFile; 
        this->procparFile = NULL; 
    }
}



/*!
 *
 */
int svkVarianReader::GetNumPixelsInVol()
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
int svkVarianReader::GetNumSlices()
{
    return (this->GetDataExtent())[5] + 1;
}


/*! 
 *  Parses the procpar file if present. 
 *  The format appears to be:
 *      keyName
 *          numElements1 space_separated_values
 *          numElements2 space_separated_values
 */
void svkVarianReader::ParseProcpar( vtkstd::string path )    
{

    //  If ONE procpar file is present, parse it as well:
    vtkGlobFileNames* procparGlob = vtkGlobFileNames::New();
    procparGlob->AddFileNames( vtkstd::string( path + "/procpar" ).c_str() );

    if ( procparGlob->GetFileNames()->GetNumberOfValues() == 1 ) {

        try { 

            this->procparFile = new ifstream();
            this->procparFile->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

            vtkstd::string procparFileName( path + "/procpar" ); 

            this->procparFile->open( procparFileName.c_str(), ifstream::in );
            if ( ! this->procparFile->is_open() ) {
                throw runtime_error( "Could not open procpar file: " + procparFileName);
            } 

            this->procparFileSize = this->GetFileSize( this->procparFile );
            while (! this->procparFile->eof() ) {
                if (this->GetProcparKeyValuePair() != 0 ) {
                    break;
                }
            }

            this->procparFile->close();

        } catch (const exception& e) {
            cerr << "ERROR opening or reading Varian procpar file: " << e.what() << endl;
        }


    } else {
        cout << "WARNING: " << this->GetClassName() << " Can't parse procpar file" << endl; 
        cout << "         found " << procparGlob->GetFileNames()->GetNumberOfValues();
        cout << " procpar files in " << path << endl;
    }
    procparGlob->Delete();
}


/*! 
 *  Utility function to read key/values from procpar file 
 *  and set the delimited key/value pair into the stl map.  
 *  Returns -1 if can't parse line. 
 */
int svkVarianReader::GetProcparKeyValuePair( )    
{

    int status = 0; 

    istringstream* iss = new istringstream();

    vtkstd::string keyString;
    vtkstd::string* valueString1 = new vtkstd::string("");
    vtkstd::string* valueString2 = new vtkstd::string("");

    try {

        this->ReadLine(this->procparFile, iss); 

        size_t  position; 
        vtkstd::string  tmp; 

        if ( this->procparFile->tellg() < this->procparFileSize - 1 ) {

            //  find first white space position before "key" string: 
            position = iss->str().find_first_of(' ');
            if (position != vtkstd::string::npos) {
                keyString.assign( iss->str().substr(0, position) );
            } 
            //cout << "PROCPAR KEY: " << keyString << endl;

            //  Now determine how many elements array 1 contains:

            this->GetProcparValueArray( valueString1 );
            this->GetProcparValueArray( valueString2 );

            //  Assign elements to map vectors: 
            this->ParseAndSetProcparStringElements(keyString, *valueString1, *valueString2);

        } else { 
            this->procparFile->seekg(0, ios::end);     
        }

    } catch (const exception& e) {
        if (this->GetDebug()) {
            cout <<  "ERROR reading line: " << e.what() << endl;
        }
        status = -1;  
    }

    delete valueString1; 
    delete valueString2; 
    delete iss; 
    return status; 
}


/*! 
 *  Utility function to read a single line from the fdf file stream.
 */
void svkVarianReader::ReadLine(ifstream* fs, istringstream* iss)    
{
    char line[2000];
    iss->clear();    
    fs->getline(line, 2000);
    iss->str(vtkstd::string(line));
}


/*!
 *  Push key value pairs into the map's value vector: 
 *  mapFor values that are space delimited lists, put each element into the value 
 *  vector. 
 */
void svkVarianReader::ParseAndSetProcparStringElements(vtkstd::string key, vtkstd::string valueArray1, vtkstd::string valueArray2) 
{
    vtkstd::vector <vtkstd::string> vector1;
    vtkstd::vector <vtkstd::string> vector2;

    this->AssignProcparVectorElements(&vector1, valueArray1);
    this->AssignProcparVectorElements(&vector2, valueArray2);

    procparMap[key].push_back(vector1);
    procparMap[key].push_back(vector2);

}


/*!
 *  Utility method to determine how many space delimited elements 
 *  are in current propcar string. 
 */
int svkVarianReader::GetNumberOfProcparElements( vtkstd::string* valueString )
{
    int numElementsParsed = 0; 
    size_t  position; 
    vtkstd::string tmpString = vtkstd::string(*valueString);


    //  If it's a quoted string, ignore spaces:
    int endQuote = 1;
    if ( ( position = tmpString.find('"') ) != vtkstd::string::npos) {
        while ( ( position = tmpString.find('"') ) != vtkstd::string::npos) {
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
        if ( tmpString.find(' ') == vtkstd::string::npos) {
            if (tmpString.length() >= 1) {
                numElementsParsed++;  
            }
        } else {
            //  if a space was found increment by 1, then see how many more spaces can be found:
            numElementsParsed++;  
            while ( ( position = tmpString.find(' ') ) != vtkstd::string::npos) {
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
void svkVarianReader::GetProcparValueArray( vtkstd::string* valueString )
{
    istringstream* iss = new istringstream();
    this->ReadLine(this->procparFile, iss); 

    size_t  position; 
    position = iss->str().find_first_of(' ');
    if (position != vtkstd::string::npos) {

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
}


/*!
 *  Remove any quotation marks from the string
 */
void svkVarianReader::RemoveStringQuotes(vtkstd::string* input) 
{
    size_t  position; 
    while ( ( position = input->find('"') ) != vtkstd::string::npos) {
        input->erase( position, 1 );
    }
}
    

/*!
 * 
 */
void svkVarianReader::AssignProcparVectorElements(vtkstd::vector<vtkstd::string>* procparVector, vtkstd::string valueArray)
{        
    size_t pos = 0;
    istringstream* iss = new istringstream();
    vtkstd::string tmpString;     

    int endQuote = 1;
    //  Parse quoted strings separately from unquoted values:    

    if ( ( pos = valueArray.find('"', pos) ) != vtkstd::string::npos) {
        endQuote *= -1; 
        while ( ( pos = valueArray.find('"', pos + 1) ) != vtkstd::string::npos) {
            endQuote *= -1; 
            if (endQuote == 1) {
                tmpString.assign( valueArray.substr(0, pos) );
                this->RemoveStringQuotes(&tmpString);
                procparVector->push_back(tmpString); 
                valueArray.assign( valueArray.substr(pos + 1) ); 
                pos = vtkstd::string::npos; 
            }
        }

    } else {

        while ( (pos = valueArray.find_first_of(' ')) != vtkstd::string::npos) {  
    
            iss->str( valueArray.substr(0, pos) );
            *iss >> tmpString;
            this->RemoveStringQuotes(&tmpString);
            procparVector->push_back(tmpString); 
            iss->clear();
    
            valueArray.assign( valueArray.substr(pos + 1) ); 
        }
        iss->str( valueArray);
        *iss >> tmpString;
        this->RemoveStringQuotes(&tmpString);
        procparVector->push_back(tmpString); 
    }

    delete iss; 
}


/*!
 *  Prints the key value pairs parsed from the header. 
 */
void svkVarianReader::PrintProcparKeyValuePairs()
{

    //  Print out key value pairs parsed from header:
    vtkstd::map< vtkstd::string, vtkstd::vector< vtkstd::vector<vtkstd::string> > >::iterator mapIter;
    for ( mapIter = procparMap.begin(); mapIter != procparMap.end(); ++mapIter ) {
     
        cout << this->GetClassName() << " " << mapIter->first << " = " << endl;

        vtkstd::vector < vtkstd::vector<vtkstd::string> >::iterator it;
        for ( it = procparMap[mapIter->first].begin() ; it < procparMap[mapIter->first].end(); it++ ) {

            vtkstd::vector<vtkstd::string> tmpVector = *it;  
            vtkstd::vector<vtkstd::string>::iterator vecIter;

            for ( vecIter = tmpVector.begin(); vecIter != tmpVector.end(); ++vecIter ) {
                cout << "       ->" << *vecIter << endl ;
            }
            cout << endl;
        }
        cout << endl;
    }
}


/*
 *  Utility method to convert from the Varian user frame to the magnet XYZ frame: 
 */
void svkVarianReader::UserToMagnet(double* user, double* magnet, double dcos[3][3])
{  
    for (int i = 0; i < 3; i++) {
        magnet[i] = 0.;     
        for (int j = 0; j < 3; j++) {
            magnet[i] += dcos[j][i] * user[j];
        }
    }
}
