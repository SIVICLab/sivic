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


#ifndef SVK_IMAGE_ACTOR
#define SVK_IMAGE_ACTOR


#include <svkImageData.h>
#include <map>
#include </usr/include/vtk/vtkObject.h>
#include <string>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkObjectBase.h>
#include </usr/include/vtk/vtkTransform.h>
#include </usr/include/vtk/vtkImageActor.h>
#include </usr/include/vtk/vtkCallbackCommand.h>
#include </usr/include/vtk/vtkImageProperty.h>

#include <svkImageData.h>

namespace svk {


using std::string;
using std::map;


/*! 
 * vtkImageActor assumes axis alignment so to support oblique data this class
 * was created.
 *
 * This class uses the input image's dcos to apply additional transform to the
 * vtkImageActor. This transform makes the actor render in the correct
 * real-world (LPS) coordinate system.
 */
class svkImageActor : public vtkImageActor
{

    public:

        // vtk type revision macro
        vtkTypeMacro( svkImageActor,vtkImageActor );
   
        static svkImageActor*  New();
        void ComputeMatrix();

    protected: 

        svkImageActor();
        ~svkImageActor();
        
};


}   //svk


#endif //SVK_IMAGE_ACTOR
