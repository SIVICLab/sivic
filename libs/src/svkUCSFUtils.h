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


#ifndef SVK_UCSF_UTILS_H
#define SVK_UCSF_UTILS_H


#include <string>
#include <map>
#include <vector>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkObject.h>
#include <svkDcmHeader.h>
#include <svkMriImageData.h>
#include <svkMrsImageData.h>



namespace svk {


using namespace std;
/*! 
 *  UCSF specific utilities.
 */
class svkUCSFUtils : public vtkObject
{

    public:

        static bool mapCreated;

        // vtk type revision macro
        vtkTypeMacro( svkUCSFUtils,vtkObject );
  
        // vtk initialization 
        static svkUCSFUtils* New();  

        //! Create the map from metabolite proper names to filenames
        static void           CreateMap();

        //! Gets the name of a metabolite fore a given filename
        static string         GetMetaboliteName( string fileName );

        //! Gets the directory in which a metabolite file should reside
        static string         GetMetaboliteDirectoryName( string spectraFileName );

        //! Gets the root name for a metabolite file
        static string         GetMetaboliteRoot( string spectraFileName );

        //! Get the filename for a given metabolite
        static string         GetMetaboliteFileName( string spectraFileName, string metaboliteName, bool includePath=false );

        //! Get the postfix for a given metabolite
        static string         GetMetabolitePostfix( string metaboliteName );

        //! Get the metabolite name for a given postfix
        static string         GetMetaboliteFromPostfix( string postfixName );

        //! Returns a liste of all metabolites in our hash
        static vector<string> GetAllMetaboliteNames();

        static string         GetDICOMFileName( string fileName, svkDcmHeader* header );

        //! Reidentify images in a given directory
        static int            ReidentifyImages( string directory );

        //! Maps metabolite names to file extentions
        static map<string, string> metaboliteMap;



    protected:

       svkUCSFUtils();
       ~svkUCSFUtils();
        
};


}   //svk



#endif //SVK_UCSF_UTILS
