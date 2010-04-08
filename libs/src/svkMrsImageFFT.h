/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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


#ifndef SVK_MRS_IMAGE_FFT_H
#define SVK_MRS_IMAGE_FFT_H


#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkImageFourierFilter.h>
#include <vtkImageFFT.h>
#include <vtkImageRFFT.h>
#include <vtkImageViewer2.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageActor.h>
#include <vtkImageFourierCenter.h>

#include <svkImageInPlaceFilter.h>


namespace svk {


using namespace std;



/*! 
 *  Class to apply an FFT to all voxels 
 */
class svkMrsImageFFT : public svkImageInPlaceFilter
{

    public:

        static svkMrsImageFFT* New();
        vtkTypeRevisionMacro( svkMrsImageFFT, svkImageInPlaceFilter);

        typedef enum {
            SPECTRAL = 0, 
            SPATIAL 
        } FFTDomain;

        typedef enum {
            FORWARD = 0, 
            REVERSE 
        } FFTMode;

        void             SetUpdateExtent(int* start, int* end);
        void             ConvertArrayToImageComplex( vtkDataArray* array, vtkImageComplex* imageComplexArray);
        void             SetFFTDomain( FFTDomain domain );
        void             SetFFTMode( FFTMode mode );



    protected:

        svkMrsImageFFT();
        ~svkMrsImageFFT();

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

        virtual int     RequestDataSpatial(
                            vtkInformation* request, 
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector,
                            vtkImageFourierFilter* fourierFilter
                        );

        virtual int     RequestDataSpectral(
                            vtkInformation* request, 
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector,
                            vtkImageFourierFilter* fourierFilter
                        );


    private:
        int             updateExtent[6]; 
        FFTDomain       domain; 
        FFTMode         mode; 

};


}   //svk


#endif //SVK_MRS_IMAGE_FFT_H

