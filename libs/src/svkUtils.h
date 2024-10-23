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


#ifndef SVK_UTILS_H
#define SVK_UTILS_H


#include <string>
#include <map>
#include <vector>
#include <stdio.h>
#include <sstream>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkGlobFileNames.h>
#include </usr/include/vtk/vtkStringArray.h>
#include </usr/include/vtk/vtkDirectory.h>
#include <svkMriImageData.h>
#include <svkMrsImageData.h>

#ifdef WIN32
#include <windows.h>
#define MAXPATHLEN 260
#else
#include <sys/param.h>
#include <pwd.h>
#endif
namespace svk {


using namespace std;
/*! 
 *  UCSF specific utilities.
 */
class svkUtils : public vtkObject
{

    public:


        // vtk type revision macro
        vtkTypeMacro( svkUtils, vtkObject );
  
        // vtk initialization 
        static svkUtils* New();  

        //! Does the file or path exist:
		static bool               FilePathExists( const char* path );
		static string             GetCurrentWorkingDirectory();
        static string             GetHomeDirectory();
		static string             GetUserName();
		static bool               CanWriteToPath( const char* path );
		static int                CopyFile( const char* input, const char* output );
		static int                MoveFile( const char* input, const char* output );
		static bool               PrintFile( const char* fileName, const char* printerName );
		static vector<string>     GetFileNamesFromPattern( 
                                    string imageBaseName, 
                                    int startSlice, 
                                    int endSlice 
                                  );
        static string             GetDefaultSecondaryCaptureDirectory( 
                                    svkMriImageData* image, 
                                    svk4DImageData* spectra
                                  );
        static string             GetDefaultSecondaryCaptureFilePattern( 
                                    svkMriImageData* image, 
                                    svk4DImageData* spectra
                                  );
        static int                GetNextPower2( int x );
        static void               StringToColorArray( double color[3], string colorString );
        static string             Double3x3ToString( double doubleMatrix[][3] );
        static string             ColorArrayToString( double color[3] );
        static vector<string>     SplitString( string str, string token );
        static string             GetFilenameFromFullPath( string fullPath );
        static string             GetPathFromFilename( string filename );
        static int                NearestInt(double x);
        static bool               UncompressFiles( vtkStringArray *filenames );
        static bool               UncompressFile( std::string filename );
        static bool               IsFileCompressed( std::string filename );
        static string             SpacesTo_( string inputString );
        static bool               AreValuesClose(double x, double y, double maxRatio = 0.01 );
        static bool               AreValuesClose(double x[3], double y[3], double maxRatio = 0.01 );
        static int                GetNumberOfDigits( int value, bool isMinusDigit = false );
        static void               GetRealpath( const char * path, int size, char* realpath );
        static void               ReadLine(ifstream* fs, istringstream* iss);   


	protected:

       svkUtils();
       ~svkUtils();
        
};


}   //svk



#endif //SVK_UTILS
