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


#ifndef SVK_MRS_IMAGE_FFT_H
#define SVK_MRS_IMAGE_FFT_H


#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>
#include </usr/include/vtk/vtkImageFourierFilter.h>
#include </usr/include/vtk/vtkImageFFT.h>
#include </usr/include/vtk/vtkImageRFFT.h>
#include <svkImageLinearPhase.h>
#include <svkImageFourierCenter.h>

#include <svkImageInPlaceFilter.h>


namespace svk {


using namespace std;



/*! 
 *  Class to apply spectral or spatial FFT. If both spectral and spatial transforms are required, 
 *  then should be called multiple timw with SetFFTDomain set appropriately.  
 *  
 *  Based on reconstruction methods developed by Sarah J. Nelson, Ph.D (UCSF).  
 *  1. Nelson S.J, "Analysis of volume MRI and MR spectroscopic imaging data for the evaluation 
 *  of patients with brain tumors",  Magnetic Resonance in Medicine, 46(2), p228-239 (2001). 
 * 
 *  In development!
 */
class svkMrsImageFFT : public svkImageInPlaceFilter
{

    public:

        static svkMrsImageFFT* New();
        vtkTypeMacro( svkMrsImageFFT, svkImageInPlaceFilter);

        typedef enum {
            SPECTRAL = 0, 
            SPATIAL 
        } FFTDomain;

        typedef enum {
            FORWARD = 0, 
            REVERSE 
        } FFTMode;

        void             SetUpdateExtent(int* start, int* end);
        static void      ConvertArrayToImageComplex( vtkDataArray* array, vtkImageComplex* imageComplexArray);
        void             SetFFTDomain( FFTDomain domain );
        void             SetFFTMode( FFTMode mode );
        void             SetPreCorrectCenter( bool preCorrectCenter );
        void             SetPostCorrectCenter( bool postCorrectCenter );
        void             SetVoxelShift( double voxelShift[3] );
        static void      FFTShift( vtkImageComplex* dataIn, int numPoints ); 
        static void      IFFTShift( vtkImageComplex* dataIn, int numPoints ); 
        void             OnlyUseSelectionBox();
        void             MaximizeVoxelsInSelectionBox();
        void             SetVolumeCenter( double centerLPS[3] );
        void             NormalizeTransform(); 


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

        bool            preCorrectCenter;
        bool            postCorrectCenter;
        bool            onlyUseSelectionBox;
        short*          selectionBoxMask;

        double          voxelShift[3];
        int             updateExtent[6]; 
        FFTDomain       domain; 
        FFTMode         mode; 

        void            UpdateOrigin(); 
        void            PrintSpectrum( vtkImageComplex* data, int numPoints, std::string msg ); 
        void            NormalizePhaseShift( double shift[3] );
        void            ValidateRequest(); 
        bool            normalizeTransform;



};


}   //svk


#endif //SVK_MRS_IMAGE_FFT_H

