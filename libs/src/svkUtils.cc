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


#include <svkUtils.h>
#include <svkTypeUtils.h>
#include </usr/include/vtk/vtkGlobFileNames.h>

using namespace svk;

//vtkCxxRevisionMacro(svkUtils, "$Rev$");
vtkStandardNewMacro(svkUtils);

//! Constructor
svkUtils::svkUtils()
{
}


//! Destructor
svkUtils::~svkUtils()
{
}


/*!
 *  Checks to see if a file, or path exists.
 */
bool svkUtils::FilePathExists(const char* path)
{
    bool filePathExists = false;
    vtkGlobFileNames* files = vtkGlobFileNames::New();
    files->AddFileNames( path );
    vtkStringArray* fileFound = files->GetFileNames();

    // Check to see if it is a file
    if( (fileFound != NULL && fileFound->GetNumberOfValues() == 1)) {
        filePathExists = true;
    } else { // Check to see if it is a directory
        vtkDirectory* directory = vtkDirectory::New();
        if( directory->FileIsDirectory(path)) {
            filePathExists = true; 
        } 
    }
    files->Delete();
    return filePathExists;
}


/*!
 * Get the current working directory.
 */
string svkUtils::GetCurrentWorkingDirectory() 
{
    char cwd[MAXPATHLEN];
    vtkDirectory::GetCurrentWorkingDirectory(cwd, MAXPATHLEN);
    return string(cwd);
}

/*!
 * Returns the users home diretory.
 */
string svkUtils::GetHomeDirectory()
{
#ifdef WIN32
    return getenv("HOMEPATH");
#else
    return getenv("HOME"); 
#endif
}


string svkUtils::GetUserName()
{
    string userName = "";
#ifdef WIN32
    char acUserName[100];
    DWORD nUserName = sizeof(acUserName);
    if (::GetUserName(acUserName, &nUserName)) {
        userName = string(acUserName);
    }
#else
    register struct passwd *psswd;
    register uid_t uid;
    uid = geteuid ();
    psswd = getpwuid (uid);
    if (psswd) {
        userName.assign(psswd->pw_name);
    }
#endif
    return userName; 

}


/*!
 *  Checks to see if a file, or path exists.
 */
bool svkUtils::CanWriteToPath(const char* path)
{
    bool canWriteToPath = false;
    string testFileName = string( path );
    testFileName.append("/sivic.write.test.txt");
    FILE* testFileHandle = fopen(testFileName.c_str(), "w");
    if( testFileHandle != NULL ) {
        canWriteToPath = true;
        remove( testFileName.c_str() );
        fclose(testFileHandle);
    }
    return canWriteToPath;
}


/*!
 * Copy a file.
 */
int svkUtils::CopyFile( const char* input, const char* output )
{
    int result = 1;
#ifndef WIN32
    stringstream copyCommand;
    copyCommand << "cp " << input <<" "<< output;
    result = system( copyCommand.str().c_str() );
#else
    if (::CopyFile( input, output,false ) ) {
        result = 0;
    }
#endif
    return result;
}


/*!
 * Move a file.
 */
int svkUtils::MoveFile( const char* input, const char* output )
{
    int result = 1;
#ifndef WIN32
    stringstream moveCommand;
    moveCommand << "mv " << input <<" "<< output;
    result = system( moveCommand.str().c_str() );
#else
    if (::CopyFile( input, output,false ) ) {
        result = 0;
    }
#endif
    return result;
}


/*!
 *  Moves all image slices from one location to another. Either * in the 
 *  imageBaseName will be replaced with the slice number
 *  or a number will be inserted before the extention.
 */
vector<string> svkUtils::GetFileNamesFromPattern( string imageBaseName, int startSlice, int endSlice )
{
    vector<string> fileNames;
    for( int i = startSlice; i <= endSlice; i++ ) {
        string sourceImageName= string( imageBaseName );

        ostringstream frameNum;
        frameNum <<  i+1;

        //  Replace asterix with slice number in output file name: 
        size_t pos = sourceImageName.find_last_of( "*" );
        if ( pos != string::npos) {
            sourceImageName.replace(pos, 1, frameNum.str());
        } else {
            size_t pos = sourceImageName.find_last_of(".");
            sourceImageName.replace(pos, 1, frameNum.str() + ".");
        }
        cout << "Source Images: " << sourceImageName << endl;
        fileNames.push_back( sourceImageName );
    }
    return fileNames;
}


/*!
 * Print a file. Not yet supported in Windows.
 */
bool svkUtils::PrintFile( const char* fileName, const char* printerName )
{
#ifndef WIN32
    stringstream printCommand;
    printCommand << "lpr -o fitplot -to-page -o landscape -h -P " << printerName <<" "<<fileName; 
    cout<< printCommand.str().c_str() << endl;
    int result = system( printCommand.str().c_str() ); 
    if( result == 0 ) {
        return true;
    } else {
        return false;
    }

#else
    cout << "Printing not supported in windows..." << endl;
    return false;
#endif

}


/*!
 * Generates a derectory based on the series and series number sof the image and spectra. Used for pushing to PACS.
 */
string svkUtils::GetDefaultSecondaryCaptureDirectory( svkMriImageData* image, svk4DImageData* activeData)
{
    string filePattern = "";
    if( image != NULL && activeData != NULL ) {

        ostringstream ossSN;
        ossSN.str("");

        // Start with the image series number
        ossSN << image->GetDcmHeader()->GetIntValue("SeriesNumber");
        string imageSeriesNumber (ossSN.str());
        filePattern.append(imageSeriesNumber);
        filePattern.append("_");

        // append spectra series number
        ossSN.str("");
        ossSN << activeData->GetDcmHeader()->GetIntValue("SeriesNumber");
        string specSeriesNumber (ossSN.str());
        filePattern.append(specSeriesNumber);

    } 

    return filePattern;
}


/*!
 * Generates a derectory based on the series and series number sof the image and activeData. Used for pushing to PACS.
 */
string svkUtils::GetDefaultSecondaryCaptureFilePattern( svkMriImageData* image, svk4DImageData* activeData)
{
    string filePattern = "";
    if( image != NULL && activeData != NULL ) {
        string studyId = activeData->GetDcmHeader()->GetStringValue("AccessionNumber");
        filePattern.append(studyId);
        filePattern.append("_SIVIC_SC_I*.DCM");

    } 

    return filePattern;
}

        
/*!
 *  Returns the next highest power of two greater than the input x.
 */
int svkUtils::GetNextPower2( int x )
{
    int result = (int)pow( 2., ceil( log( static_cast<double>(x + 1) )/log(2.) ) );
    return result;
}


/*!
 *  @param doubleVal
 *  @return string equivalent
 */
string svkUtils::Double3x3ToString( double doubleMatrix[][3] )
{
    ostringstream intStream;
    intStream << "|" << doubleMatrix[0][0] << "|" << doubleMatrix[0][1] << "|" << doubleMatrix[0][2] << "|\n";
    intStream << "|" << doubleMatrix[1][0] << "|" << doubleMatrix[1][1] << "|" << doubleMatrix[1][2] << "|\n";
    intStream << "|" << doubleMatrix[2][0] << "|" << doubleMatrix[2][1] << "|" << doubleMatrix[2][2] << "|\n";
    return intStream.str();
}


void svkUtils::StringToColorArray( double color[3], string colorString )
{
    vector<string> colors = svkUtils::SplitString( colorString, " ");
    if( colors.size() == 3) {
        color[0] = svkTypeUtils::StringToDouble( colors[0]);
        color[1] = svkTypeUtils::StringToDouble( colors[1]);
        color[2] = svkTypeUtils::StringToDouble( colors[2]);
    }
}


string svkUtils::ColorArrayToString( double color[3] )
{
    stringstream stream;
    stream << color[0] << " " << color[1] << " " << color[2] << endl;
    return stream.str();
}


/*!
 *  Returns the string following final slash.  This could be a filename
 *  or terminal directory.  
 */
string svkUtils::GetFilenameFromFullPath( string fullPath )
{
    vector<string> splitString = svkUtils::SplitString(fullPath, "/");
    return splitString[splitString.size()-1];
}


string svkUtils::GetPathFromFilename( string filename ){
    vector<string> splitString = svkUtils::SplitString(filename, "/");
    if( splitString.size() > 0 ) {
        string path= "";
        if(filename[0] == '/') {
            path.append("/");
        }
        for( int i = 0; i < splitString.size() -1 ; i++ ) {
            path.append(splitString[i]);
            path.append("/");
        }
        return path;
    } else {
        return "./";
    }
}


vector<string> svkUtils::SplitString( string str, string token )
{
    vector<string> result;
    int nPos;
    while( (nPos = str.find_first_of(token)) != str.npos ) {
        if(nPos > 0) {
            result.push_back(str.substr(0,nPos));
        }
        str = str.substr(nPos+1);
    }
    if(str.length() > 0) {
        result.push_back(str);
    }
    return result;
}


/*
 *   Returns the nearest int.  For values at the mid-point,
 *   the value is rounded to the larger int.
 */
int svkUtils::NearestInt(double x)
{
    int x_to_int;
    x_to_int = (int) x;

    /*
     *   First do positive numbers, then negative ones.
     */
    if (x>=0) {
        if ((x - x_to_int) >= 0.5) {
            x_to_int += 1;
        }
    } else {
        if ((x_to_int - x) > 0.5) {
            x_to_int -= 1;
        }
    }

    return (int) x_to_int;
}


/*!
 *  Uncompress all files given.
 */
bool svkUtils::UncompressFiles( vtkStringArray *filenames )
{
    bool success = true;
#ifndef WIN32
    for (int fileIndex = 0; fileIndex < filenames->GetNumberOfValues(); fileIndex++) {
            string filename = filenames->GetValue( fileIndex );
            bool wasUncompressed = svkUtils::UncompressFile( filename );
            if( !wasUncompressed ) {
                success = false;
            }
    }
#else
    // No windows implementation yet.
#endif
    return success;
}


/*!
 * Uncompress a file. Platform dependent, currently not supported on windows.
 */
bool svkUtils::UncompressFile( std::string filename )
{
    bool success = true;
#ifndef WIN32
    if( svkUtils::IsFileCompressed( filename ) ) {
        stringstream uncompressCommand;
#ifdef UCSF_INTERNAL
        uncompressCommand << "group_gunzip " << filename << ".gz";
#else
        uncompressCommand << "gunzip " << filename;
#endif
        cout << "UNCOMPRESSING FILE " << uncompressCommand.str() << endl;
        int result = system( uncompressCommand.str().c_str() );
        if( result != 0 ) {
            success = false;
        }
    }
#else
    // No windows implementation yet.
#endif
    return success;
}


/*!
 *  Check to see if a file is compressed. Platform dependent, currently not supported on windows.
 */
bool svkUtils::IsFileCompressed( std::string filename )
{
    bool isCompressed = false;
#ifndef WIN32
    stringstream testCompressCommand;
    testCompressCommand << "gunzip -t " << filename << " 2> /dev/null";
    int result = system( testCompressCommand.str().c_str() );
    if( result == 0 ) {
        isCompressed = true;
    }
#else
    // No windows implementation yet.
#endif
    return isCompressed;
}


/*!
 *  This method replaces spaces with underscores (_). Spaces are not
 *  permitted in strings used through the tcl layer so if we want to
 *  use a string as a variable in this way then the spaces must be
 *  extracted.
 */
string svkUtils::SpacesTo_( std::string inputString )
{
        size_t pos = inputString.find( " " );
        while( pos!= string::npos ) {
            inputString.replace(pos, 1, "_");
            pos = inputString.find( " " );
        }
        return inputString;
}


/*!
 * Used to test for minor differences in values.
 */
bool svkUtils::AreValuesClose( double x, double y, double maxRatio )
{

    //  If the values are the same then return true, otherwise
    //  carry on to check tolerance
    if (x == y) {
        return true; 
    }

    double ratio;
    if( x > y ) {
        ratio = y/x;
    } else {
        ratio = x/y;
    }
    if( ratio > 0 && 1 - ratio < maxRatio ) {
        return true;
    } else {
        return false;
    }

}


/*!
 * Used to test for minor differences in values.
 */
bool svkUtils::AreValuesClose( double x[3], double y[3], double maxRatio )
{
    if(    svkUtils::AreValuesClose(x[0], y[0], maxRatio)
        && svkUtils::AreValuesClose(x[1], y[1], maxRatio)
        && svkUtils::AreValuesClose(x[2], y[2], maxRatio)) {
        return true;
    } else {
        return false;
    }
}


/*!
 * Get the number of digits in an integer. Minus
 * sign counts as a digit in this case.
 */
int svkUtils::GetNumberOfDigits( int number, bool isMinusDigit )
{
    int numDigits = 1;
    if( isMinusDigit && number < 0){
        numDigits ++;
    }
    number /= 10;
    while( number != 0 ){
        number /= 10;
        numDigits ++;
    }
    return numDigits;
}


/*! 
 * Method expands relative paths.
 */
void svkUtils::GetRealpath( const char * path, int size, char* resultRealpath )
{

#ifndef WIN32
    char* fnameFullPath = realpath(path, NULL);
    memcpy( resultRealpath, fnameFullPath, size * sizeof(char) );
#else
    GetFullPathName(path, size, resultRealpath, NULL);
#endif

}


/*! 
 *  Utility function to read a single line from a file stream.
 */
void svkUtils::ReadLine(ifstream* fs, istringstream* iss)    
{
    char line[2000];
    iss->clear();    
    fs->getline(line, 2000);
    iss->str(string(line));
}
