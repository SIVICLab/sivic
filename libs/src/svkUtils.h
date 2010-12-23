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
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/libs/src/svkUCSFUtils.h $
 *  $Rev: 248 $
 *  $Author: beckn8tor $
 *  $Date: 2010-03-30 14:12:39 -0700 (Tue, 30 Mar 2010) $
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
#include <vtkObjectFactory.h>
#include <vtkObject.h>
#include <vtkGlobFileNames.h>
#include <vtkStringArray.h>
#include <vtkDirectory.h>

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
        vtkTypeRevisionMacro( svkUtils, vtkObject );
  
        // vtk initialization 
        static svkUtils* New();  

        //! Does the file or path exist:
		static bool           FilePathExists( const char* path );
		static string		  GetCurrentWorkingDirectory();
		static string		  GetUserName();
		static bool			  CanWriteToPath( const char* path );
		static bool           CopyFile( const char* input, const char* output );
		static bool           PrintFile( const char* fileName, const char* printerName );
	protected:

       svkUtils();
       ~svkUtils();
        
};


}   //svk



#endif //SVK_UTILS
