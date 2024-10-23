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


#ifndef SVK_MRI_IMAGE_FFT_H
#define SVK_MRI_IMAGE_FFT_H


#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkImageFourierFilter.h>
#include </usr/include/vtk/vtkImageFFT.h>
#include </usr/include/vtk/vtkImageRFFT.h>
#include </usr/include/vtk/vtkImageExtractComponents.h>
#include <svkImageAlgorithm.h>


namespace svk {


using namespace std;



/*! 
 *  Applies vtk FFT algorithms to svkMriImageData objects. In contrast to the 
 *  vtkImageFFT and vtkImageRFFT svkMriImageFFT performs both forward and reverse 
 *  FFT's. The domain is defined by the SetFFTMode method. Also this algorithm is
 *  by default not in place, but by using the SetOperateInPlace method you
 *  can force the algorithm to overate in place. Input can be any data type, but
 *  output for the FFT is complex doubles and the output for RFFT is real doubles.
 */
class svkMriImageFFT : public svkImageAlgorithm
{

    public:

        static svkMriImageFFT* New();
        vtkTypeMacro( svkMriImageFFT, svkImageAlgorithm);

        typedef enum {
            FORWARD = 0, 
            REVERSE 
        } FFTMode;

        void             SetFFTMode( FFTMode mode );
        void             SetOperateInPlace( bool operateInPlace );
        svkImageData*    GetOutput();
        svkImageData*    GetOutput(int port);


    protected:

        svkMriImageFFT();
        ~svkMriImageFFT();

        virtual int     FillInputPortInformation(int port, vtkInformation* info);


        //  Methods:
        virtual int     RequestData(
                            vtkInformation* request, 
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector
                        );


    private:

        void            UpdateHeader( svkDcmHeader* targetHeader );

        FFTMode         mode; 
        bool            operateInPlace;
};


}   //svk


#endif //SVK_MRI_IMAGE_FFT_H

