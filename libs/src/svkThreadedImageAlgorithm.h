/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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


#ifndef SVK_THREADED_IMAGE_ALGORITHM_H
#define SVK_THREADED_IMAGE_ALGORITHM_H


#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkImageData.h>
#include <vtkCellData.h>
#include <vtkThreadedImageAlgorithm.h>
#include <vtkInstantiator.h>
#include <vtkInformation.h>
#include <vtkExecutive.h>

#include <svkMriImageData.h>
#include <svkMrsImageData.h>
#include <svk4DImageData.h>


namespace svk {


using namespace std;


/*! 
 *  Base class for svkThreadedImageAlgorithm. Implicitly in place with 
 *  output port wired to input port 0.  
 */
class svkThreadedImageAlgorithm : public vtkThreadedImageAlgorithm
{

    public:

        vtkTypeMacro( svkThreadedImageAlgorithm, vtkThreadedImageAlgorithm);
        svkImageData*               GetOutput(); 
        virtual svkImageData*       GetOutput(int port); 
        svkImageData*               GetImageDataInput(int port);


    protected:

        svkThreadedImageAlgorithm();
        ~svkThreadedImageAlgorithm();

        virtual int         FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info );
        virtual int         FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info );
        int                 SplitExtent(int splitExt[6],int startExt[6],int num, int total); 

        //virtual int     RequestData(
        //                    vtkInformation* request,
        //                    vtkInformationVector** inputVector,
        //                    vtkInformationVector* outputVector
        //                );


};


}   //svk


#endif //SVK_THREADED_IMAGE_ALGORITHM_H

