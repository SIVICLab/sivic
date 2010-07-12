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
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/libs/src/svkTestUtils.h $
 *  $Rev: 248 $
 *  $Author: beckn8tor $
 *  $Date: 2010-03-30 14:12:39 -0700 (Tue, 30 Mar 2010) $
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#ifndef SVK_TEST_UTILS_H
#define SVK_TEST_UTILS_H


#include <string>
#include <vtkWindow.h>
#include <svkImageWriter.h>
#include <svkImageWriterFactory.h>
#include <vtkWindowToImageFilter.h>
#include <vtkObjectFactory.h>
#include <vtkObject.h>


namespace svk {


using namespace std;
/*! 
 *  Test specific utilities.
 */
class svkTestUtils : public vtkObject
{

    public:

        // vtk type revision macro
        vtkTypeRevisionMacro( svkTestUtils,vtkObject );
  
        // vtk initialization 
        static svkTestUtils* New();  

        static void SaveWindow( vtkWindow* window, string filename );

    protected:

       svkTestUtils();
       ~svkTestUtils();
        
};


}   //svk



#endif //SVK_TEST_UTILS
