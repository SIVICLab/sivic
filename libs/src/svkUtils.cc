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
	bool fileExists = false;
	vtkGlobFileNames* files = vtkGlobFileNames::New();
	files->AddFileNames( path );
	vtkStringArray* fileFound = files->GetFileNames();
	if( fileFound != NULL && fileFound->GetNumberOfValues() == 1 ) {
		fileExists = true;
	}
	files->Delete();
	return fileExists;
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
bool svkUtils::CopyFile( const char* input, const char* output )
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

