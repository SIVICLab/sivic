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


#ifndef SVK_IMAGE_CLIP_H
#define SVK_IMAGE_CLIP_H


#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkDataObject.h>
#include </usr/include/vtk/vtkImageClip.h>
#include <svkMriImageData.h>
//include <vtkInstantiator.h>


namespace svk {



/*! 
 *  This class is designed to be a an svk replacement for vtkImageClip.
 *  The only differences are that this class will instantiate an svkImageData
 *  object for output, and it will copy the dcos of the input on GetOutput().
 */
class svkImageClip : public vtkImageClip
{

    public:

        // vtk type revision macro
        vtkTypeMacro( svkImageClip, vtkImageClip );
   
        static svkImageClip*  New();  
        
        svkImageData*                   GetImageDataInput(int port);



    protected:        
        svkImageClip();
        ~svkImageClip();
        int RequestData(
          vtkInformation *request,
          vtkInformationVector **inputVector,
          vtkInformationVector *outputVector);

        //virtual int FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info);
        virtual int FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info);


};


}   //svk


#endif //SVK_IMAGE_CLIP_H
