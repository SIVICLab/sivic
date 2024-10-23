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


#ifndef SVK_MRS_PEAK_PICK_H
#define SVK_MRS_PEAK_PICK_H


#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>

#include <svkImageInPlaceFilter.h>
#include <svkSpecUtils.h>


namespace svk {


using namespace std;



/*! 
 *  Class to identify noise region and compute noise SD in spectra.  Uses average magnitude spectrum
 *  to first identify region with smallest SD.  This region is likely to be noise rather than signal.
 *  Once the region is defined, the noise SD is computed as an average value of noise SD computed at
 *  each voxel over the identified frequency range.  
 */
class svkMRSPeakPick : public svkImageInPlaceFilter
{

    public:

        static svkMRSPeakPick* New();
        vtkTypeMacro( svkMRSPeakPick, svkImageInPlaceFilter);

        void            SetNoiseSD(float noise); 
        void            SetBaselineValue(float baseline); 
        void            SetSNLimit(float sn); 
        void            SetResolutionHeightFraction(float resolveHeightFraction); 
        void            OnlyUseSelectionBox();
        void            PrintPeaks(); 
        float           GetPeakHeight( vtkFloatArray* spectrum, int peakNum ); 
        float           GetPeakSymmetry( vtkFloatArray* spectrum, int peakNum ); 
        float           GetPeakHeightAtPeakPos( vtkFloatArray* spectrum, int peakNum ); 
        float           GetPeakArea( vtkFloatArray* spectrum, int peakNum ); 
        float           GetAvRMSPeakHeight( int peakNum ); 
        virtual void    PrintSelf( ostream &os, vtkIndent indent );
        int             GetNumPeaks(); 
        void            GetPeakDefinition( int peakNum, int* startPt, int* peakPt, int* EndPt ); 



    protected:

        svkMRSPeakPick();
        ~svkMRSPeakPick();

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


    private:
        
        void            PickPeaks(); 
        void            InitPeakVector( vector<int>* peak, int startPt, int peakPt, int endPt); 
        void            RefinePeakRanges(); 


        //  Members:
        float           noiseSD;
        float           snLimit;
        float           baselineValue; 
        float           resolveHeightFraction; 
        int 			onlyUseSelectionBox;
        short*          selectionBoxMask;
        vtkFloatArray*  averageSpectrum; 

        vector < vector < int > > peakVector; 
        vector < float >          peakHeightVector; 


};


}   //svk


#endif //SVK_MRS_PEAK_PICK_H

