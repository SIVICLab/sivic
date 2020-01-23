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


#ifndef SVK_MRS_AUTO_FREQ_SHIFT_H
#define SVK_MRS_AUTO_FREQ_SHIFT_H

#define SWARM

#include <itkPowellOptimizer.h>
#include <itkParticleSwarmOptimizer.h>

#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkInformation.h>

#include <svkImageData.h>
#include <svkMrsImageData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <svkThreadedImageAlgorithm.h>

#include <complex>
#include <svkDcmHeader.h>

#include <math.h>
#include <stdio.h>
#include <string.h>


namespace svk {


using namespace std;


/*! 
 *  Algorithm for automatic phase correction of MR spectra. 
 */
class svkMRSAutoFreqCorrect : public svkThreadedImageAlgorithm
{

    public:

        vtkTypeMacro( svkMRSAutoFreqCorrect, svkThreadedImageAlgorithm);
        //static          svkMRSAutoFreqCorrect* New();
        

        //  _0 are zero order models
        //      MAX_PEAK_HTS_0 maximizes the peak height of a specified peak
        //  _1 are first order models
        typedef enum {
            UNDEFINED_PHASE_MODEL   = 0, 
            PEAK_LOC                = 1, 
            LAST_MODEL 
        } PhasingModel;

        //void   SetPhasingModel(svkMRSAutoFreqCorrect::phasingModel model); 
        void                    OnlyUseSelectionBox(); 
        
        virtual svkImageData*   GetOutput(int port); 



    protected:

        svkMRSAutoFreqCorrect();
        ~svkMRSAutoFreqCorrect();

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

        int             SVKRequestDataPreExec( 
                            vtkInformation* request, 
                            vtkInformationVector** inputVector, 
                            vtkInformationVector* outputVector 
                        );
        int             SVKRequestDataPostExec( 
                            vtkInformation* request, 
                            vtkInformationVector** inputVector, 
                            vtkInformationVector* outputVector 
                        );

        virtual void    ThreadedRequestData(
                            vtkInformation* request,
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector,
                            vtkImageData*** inData,
                            vtkImageData** outData,
                            int extent[6],
                            int threadId
                        );

        virtual void    ValidateInput(); 


        virtual int     FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info );
        virtual int     FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info ); 

        void            ZeroData(); 
        virtual void    UpdateProvenance();

        void            AutoFreqCorrectExecute(int* outExt, int id); 
        virtual void    AutoFreqCorrectSpectrum( int cellID );
        virtual void    FitPhase( int cellID ) = 0; 
        virtual void    PrePhaseSetup() = 0;
        virtual void    PostPhaseCleanup() = 0; 
        void            SyncPointsFromCells(); 
        virtual void    SetMapSeriesDescription( ); 


        static int*     progress;


#ifdef SWARM
        virtual void    InitOptimizer( int cellID, itk::ParticleSwarmOptimizer::Pointer itkOptimizer ) = 0; 
#else
        virtual void    InitOptimizer( int cellID, itk::PowellOptimizer::Pointer itkOptimizer ) = 0; 
#endif

        int                             numTimePoints;
        svkMRSAutoFreqCorrect::PhasingModel   phaseModelType; 
        bool                            onlyUseSelectionBox; 
        short*                          selectionBoxMask;
        bool                            isSpectralFFTRequired; 
        string                          seriesDescription; 
        vtkDataArray*                   mapArrayZeroOrderPhase; 

};


}   //svk


#endif //SVK_MRS_AUTO_FREQ_SHIFT_H

