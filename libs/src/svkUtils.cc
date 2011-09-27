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


#include <svkUtils.h>
#include <vtkGlobFileNames.h>

using namespace svk;

vtkCxxRevisionMacro(svkUtils, "$Rev$");
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
 *  Moves all image slices from one location to another. Either * in the imageBaseName will be replaced with the slice number
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
string svkUtils::GetDefaultSecondaryCaptureDirectory( svkMriImageData* image, svkMrsImageData* spectra)
{
    string filePattern = "";
    if( image != NULL && spectra != NULL ) {

        ostringstream ossSN;
        ossSN.str("");

        // Start with the image series number
        ossSN << image->GetDcmHeader()->GetIntValue("SeriesNumber");
        string imageSeriesNumber (ossSN.str());
        filePattern.append(imageSeriesNumber);
        filePattern.append("_");

        // append spectra series number
        ossSN.str("");
        ossSN << spectra->GetDcmHeader()->GetIntValue("SeriesNumber");
        string specSeriesNumber (ossSN.str());
        filePattern.append(specSeriesNumber);

    } 

    return filePattern;
}


/*!
 * Generates a derectory based on the series and series number sof the image and spectra. Used for pushing to PACS.
 */
string svkUtils::GetDefaultSecondaryCaptureFilePattern( svkMriImageData* image, svkMrsImageData* spectra)
{
    string filePattern = "";
    if( image != NULL && spectra != NULL ) {
        string studyId = spectra->GetDcmHeader()->GetStringValue("AccessionNumber");
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
 *  Converts a string to a double.
 *
 * @param doubleString
 * @return
 */
double svkUtils::StringToDouble( string doubleString )
{
    istringstream* iss = new istringstream();
    double value = 0.0;
    iss->str( doubleString );
    *iss >> value;
    delete iss;
    return value;
}


/*!
 *
 * @param doubleString
 * @return
 */
float svkUtils::StringToFloat( string doubleString )
{
    istringstream* iss = new istringstream();
    float value = 0.0;
    iss->str( doubleString );
    *iss >> value;
    delete iss;
    return value;
}


/*!
 *
 * @param intString
 * @return
 */
int svkUtils::StringToInt( string intString )
{
    istringstream* iss = new istringstream();
    int value;
    iss->str( intString );
    *iss >> value;
    delete iss;
    return value;
}


/*!
 *  @param intVal
 *  @return string equivalent
 */
string svkUtils::IntToString( int intVal )
{
    ostringstream intStream;
    intStream << intVal;
    return intStream.str();
}


void svkUtils::StringToColorArray( double color[3], string colorString )
{
    vector<string> colors = svkUtils::SplitString( colorString, " ");
    if( colors.size() == 3) {
        color[0] = svkUtils::StringToDouble( colors[0]);
        color[1] = svkUtils::StringToDouble( colors[1]);
        color[2] = svkUtils::StringToDouble( colors[2]);
    }
}

string svkUtils::ColorArrayToString( double color[3] )
{
    stringstream stream;
    stream << color[0] << " " << color[1] << " " << color[2] << endl;
    return stream.str();
}

string svkUtils::GetFilenameFromFullPath( string fullPath )
{
    /*
    size_t posSlash = fullPath.find_last_of("/");
    if (  posSlash != string::npos) {
        return fullPath.substr(posSlash, string::npos );
    } else {
        return fullPath;
    }
    */
    vector<string> splitString = svkUtils::SplitString(fullPath, "/");
    return splitString[splitString.size()-1];
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

