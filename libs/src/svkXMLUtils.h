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


#ifndef SVK_XML_UTILS_H
#define SVK_XML_UTILS_H


#include <string>
#include <map>
#include <vector>
#include <stdio.h>
#include <sstream>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkXMLDataElement.h>
#include </usr/include/vtk/vtkXMLUtilities.h>

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
class svkXMLUtils : public vtkObject
{

    public:


        // vtk type revision macro
        vtkTypeMacro( svkXMLUtils, vtkObject );
  
        // vtk initialization 
        static svkXMLUtils* New();  

        static vtkXMLDataElement* CreateNestedXMLDataElement( 
                                    vtkXMLDataElement* parent, 
                                    string name, 
                                    string value 
                                  );
        static vtkXMLDataElement* ReadXMLAndSubstituteVariables(
                                    string xmlFileName, 
                                    vector<string> xmlVariables 
                                  );
        static vtkXMLDataElement* FindNestedElementWithPath( vtkXMLDataElement* root, string xmlPath);
        static vtkXMLDataElement* FindOrCreateNestedElementWithPath( vtkXMLDataElement* root, string parentPath, string elementName);
        static bool               GetNestedElementCharacterDataWithPath( vtkXMLDataElement* root, string xmlPath, string* data );
        static bool               SetNestedElementWithPath( vtkXMLDataElement* root, string xmlPath, string value );
        static void               ReadLine(ifstream* fs, istringstream* iss);   
        static vector<string>     SplitString( string str, string token );



	protected:

       svkXMLUtils();
       ~svkXMLUtils();
        
};


}   //svk



#endif //SVK_XML_UTILS
