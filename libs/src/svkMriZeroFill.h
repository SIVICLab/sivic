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


#ifndef SVK_MRI_ZERO_FILL
#define SVK_MRI_ZERO_FILL


#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkImageConstantPad.h>
#include </usr/include/vtk/vtkInformationVector.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>
#include </usr/include/vtk/vtkImageChangeInformation.h>
#include <svkImageAlgorithm.h>
#include <svkImageLinearPhase.h>
#include </usr/include/vtk/vtkImageMathematics.h>


namespace svk {


using namespace std;


/*! 
 *  This filter pads the input dataset to the size of the OutputWholeExtent. The output
 *  dataset will have the same content as the input dataset but will be padded. The
 *  input volume will be in the center of the output volume.
 *
 *  NOTE:
 *     * The spacing and the origin will be updated to maintain the same field of
 *       view.
 *
 */
class svkMriZeroFill : public svkImageAlgorithm
{

    public:

        static svkMriZeroFill* New();
        vtkTypeMacro( svkMriZeroFill, svkImageAlgorithm);

        void             SetOperateInPlace( bool operateInPlace );
        svkImageData*    GetOutput();
        svkImageData*    GetOutput(int port);
        void             SetOutputWholeExtent( int extent[6] );
        void             SetOutputWholeExtent(int minX, int maxX, int minY, int maxY, int minZ, int maxZ);
        void             GetOutputWholeExtent( int extent[6] );

    protected:

        svkMriZeroFill();
        ~svkMriZeroFill();

        virtual int     FillInputPortInformation(int port, vtkInformation* info);


        //  Methods:
        virtual int     RequestInformation(
                            vtkInformation* request,
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector
                        );

        virtual int     RequestData(
                            vtkInformation* request, 
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector
                        );

        virtual int     RequestUpdateExtent(vtkInformation*,
                             vtkInformationVector**,
                             vtkInformationVector*);

        virtual void    ComputeInputUpdateExtent (int inExt[6], int outExt[6], int wExt[6]);

    private:

        bool            operateInPlace;
        int             outputWholeExtent[6];

};


}   //svk


#endif //SVK_MRI_ZERO_FILL

