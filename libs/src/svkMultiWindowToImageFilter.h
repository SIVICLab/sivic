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


#ifndef SVK_MULTI_WINDOW_TO_IMAGE_FILTER_H
#define SVK_MULTI_WINDOW_TO_IMAGE_FILTER_H


#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkAlgorithm.h>
#include </usr/include/vtk/vtkImageData.h>
#include </usr/include/vtk/vtkWindowToImageFilter.h>
#include </usr/include/vtk/vtkRenderWindow.h>
#include </usr/include/vtk/vtkInformationVector.h>
#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>
#include </usr/include/vtk/vtkImageConstantPad.h>
#include </usr/include/vtk/vtkImageAppend.h>
#include <vector>
#include </usr/include/vtk/vtkImageFlip.h>
#include </usr/include/vtk/vtkRenderLargeImage.h>
#include </usr/include/vtk/vtkRendererCollection.h>


namespace svk {


using namespace std;


/*!
 *  Class that creates a composite vtkImageData object from multiple vtkRenderWindows. 
 *  Window images are obtained from vtkWindowToImageFilter and packed into a 
 *  rectangle according to their specified indices starting at the toplc (0,0) restering
 *  first in x, then downward in y.   
 */


class svkMultiWindowToImageFilter: public vtkAlgorithm
{

    public:

        static svkMultiWindowToImageFilter* New();
        vtkTypeMacro( svkMultiWindowToImageFilter, vtkAlgorithm);

        //  Methods
        virtual void            SetInput(vtkRenderWindow* data, int indX = 0, int indY = 0, int magnification = 1 );
        virtual vtkImageData*   GetOutput();
        void                    SetPadConstant( int );



    protected:

        svkMultiWindowToImageFilter();
        ~svkMultiWindowToImageFilter();

        virtual int         FillOutputPortInformation(int vtkNotUsed(port), vtkInformation* info);
        virtual int         ProcessRequest(
                                vtkInformation* request, 
                                vtkInformationVector** inputVector,  
                                vtkInformationVector* outputVector
                            );
        virtual int         RequestData(
                                vtkInformation* request, 
                                vtkInformationVector** inputVector,  
                                vtkInformationVector* outputVector
                            );
        virtual int         RequestInformation(
                                vtkInformation* request, 
                                vtkInformationVector** inputVector,  
                                vtkInformationVector* outputVector
                            );


    private:

        //  Members:
        vector < vector <vtkRenderWindow*> >    vtkRenderWindowArray; 
        vector <int>                            colWidthVector; 
        vector <int>                            rowHeightVector; 
        vector < vector <int> >                 magnificationVector;
        double                                  padConstant;
        

};


}   //svk


#endif //SVK_MULTI_WINDOW_TO_IMAGE_FILTER_H

