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


#ifndef SVK_UCSFPACS_INTERFACE_H
#define SVK_UCSFPACS_INTERFACE_H

#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include <svkPACSInterface.h>
#include <svkUtils.h>
#include <svkUCSFUtils.h>
#include <svkTypes.h>

namespace svk {


using namespace std;

/*!
 *   This is current for UCSF only. This method will do the following...
 *   
 *   1. Create a temporary directary in the pacsDirectory below. \n
 *   2. Verify that it can write to this path. \n
 *   3. Copy all images to the PACS temporary directory. \n
 *   4. Reidentify these images in the temporary directory. \n
 *   5. Move the images to the PACS directory. \n
 *   6. Delete the temporary PACS directory. \n
 */
class svkUCSFPACSInterface : public svkPACSInterface 
{

    public:

        vtkTypeMacro( svkUCSFPACSInterface, svkPACSInterface);
    
        static svkUCSFPACSInterface* New();

  
        bool Connect();
        bool SendImagesToPACS( string sourceDirectory, svkTypes::AnatomyType anatomyType );
        bool Disconnect();

    protected:

        svkUCSFPACSInterface();
        ~svkUCSFPACSInterface();

};


}   //svk


#endif //SVK_UCSFPACS_INTERFACE_H

