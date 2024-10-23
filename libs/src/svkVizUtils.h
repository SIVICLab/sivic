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


#ifndef SVK_VIZ_UTILS_H
#define SVK_VIZ_UTILS_H


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
#include <svkImageWriterFactory.h>
#include </usr/include/vtk/vtkWindow.h>
#include </usr/include/vtk/vtkImageWriter.h>
#include </usr/include/vtk/vtkTIFFWriter.h>
#include </usr/include/vtk/vtkJPEGWriter.h>
#include </usr/include/vtk/vtkWindowToImageFilter.h>

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
class svkVizUtils : public vtkObject
{

    public:


        // vtk type revision macro
        vtkTypeMacro( svkVizUtils, vtkObject );
  
        // vtk initialization 
        static svkVizUtils* New();

        static void               SaveWindow( vtkWindow* window, string filename );

	protected:

       svkVizUtils();
       ~svkVizUtils();
        
};


}   //svk



#endif //SVK_VIZ_UTILS
