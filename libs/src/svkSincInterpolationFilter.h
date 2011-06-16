/*
 *  Copyright © 2009-2011 The Regents of the University of California.
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


#ifndef SVK_SINC_INTERPOLATION_FILTER_H
#define SVK_SINC_INTERPOLATION_FILTER_H


#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkImageConstantPad.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkImageChangeInformation.h>
#include <svkImageAlgorithm.h>
#include <svkMriImageFFT.h>
#include <svkImageFourierCenter.h>
#include <vtkImageFourierCenter.h>
#include <vtkImageMathematics.h>
#include <svkMriZeroFill.h>
#include <svkIdfVolumeWriter.h>
#include <svkImageLinearPhase.h>
#include <vtkImageMagnitude.h>


namespace svk {


using namespace std;



/*! 
 *  Applies a sinc interpolation to a given image. You must specify an output extent greater
 *  than the original image to get the interpolated result. The algorithm follows the 
 *  following steps:
 *
 *  Run svkMriImageFFT on the input volume.
 *  Move k=0 to the center of the volume.
 *  Use svkMriZeroFill to pad the kspace volume.
 *  Move k=0 back to the origin.
 *  Run svkMriImageFFT to reverse the FFT.
 *  Update the header to decrease the pixel size.
 *  Reset the origin back to its original position position (padding changes the origin).
 */
class svkSincInterpolationFilter : public svkImageAlgorithm
{

    public:

        static svkSincInterpolationFilter* New();
        vtkTypeRevisionMacro( svkSincInterpolationFilter, svkImageAlgorithm);

        void             SetOperateInPlace( bool operateInPlace );
        svkImageData*    GetOutput();
        svkImageData*    GetOutput(int port);
        void             SetOutputWholeExtent( int extent[6] );
        void             SetOutputWholeExtent(int minX, int maxX, int minY, int maxY, int minZ, int maxZ);
        void             GetOutputWholeExtent( int extent[6] );

    protected:

        svkSincInterpolationFilter();
        ~svkSincInterpolationFilter();

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

        virtual void ComputeInputUpdateExtent (int inExt[6], int outExt[6], int wExt[6]);

    private:

        bool            operateInPlace;
        int             outputWholeExtent[6];

};


}   //svk


#endif //SVK_SINC_INTERPOLATION_FILTER_H

