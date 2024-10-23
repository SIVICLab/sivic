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


#ifndef SVK_DCE_BASIC_FIT_H
#define SVK_DCE_BASIC_FIT_H


#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkInformation.h>

#include <svkImageData.h>
#include <svkMriImageData.h>
#include <svkImageAlgorithm.h>
#include <svkDcmHeader.h>
#include <svkDynamicImageMap.h>
#include <svkAlgorithmPortMapper.h>


namespace svk {


using namespace std;


/*! 
 *  Class to derive peak height from a DCE image 
 */
class svkDCEBasicFit: public svkDynamicImageMap
{

    public:

        enum {
            INPUT_IMAGE = 0,
            START_TIME_PT,
            END_TIME_PT
        } svkDCEBasicFitInput;

        enum {
            BASE_HT_MAP = 0,
            PEAK_HT_MAP,
            PEAK_TIME_MAP,
            UP_SLOPE_MAP,
            WASHOUT_SLOPE_MAP,
            WASHOUT_SLOPE_POS_MAP
        } svkDCEBasicFitOutput;

        static svkDCEBasicFit *New();
        vtkTypeMacro(svkDCEBasicFit, svkDynamicImageMap);

        void     SetTimepointStart(int startPt);
        svkInt*  GetTimepointStart();

        void     SetTimepointEnd(int endPt);
        svkInt*  GetTimepointEnd();

        // Get the internal XML interpreter
        svkAlgorithmPortMapper* GetPortMapper();

    protected:

        svkDCEBasicFit();
        ~svkDCEBasicFit();

        virtual int  FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info );
        virtual int  FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info );

        //! The port mapper used to set the input port parameters.
        svkAlgorithmPortMapper* portMapper;

    private:

        //  Methods:
        virtual void GenerateMaps(); 
        void         InitializeOutputVoxelValues( float* dynamicVoxelPtr, int voxelIndex, float imageRate );
        void         InitializeBaseline();
        int          GetInjectionPoint( float* baselineArray );
        double       GetTimePointMean(int timePoint ); 
        double       GetStandardDeviation( vtkDataArray* array, float mean, int endPt); 
        void         InitializeInjectionPoint();
        void         GetBaseHt( float* dynamicVoxelPtr, double* voxelBaseHt );
        void         GetPeakHt( float* dynamicVoxelPtr, double* voxelPeakHt );
        void         GetPeakTm( float* dynamicVoxelPtr, double voxelPeakHt, double* voxelPeakTime);
        void         GetUpSlope( float* dynamicVoxelPtr, double voxelPeakTime, double* voxelUpSlope );
        void         GetWashout( float* dynamicVoxelPtr, int filterWindow, int voxelIndex, double* voxelWashout );
        void         ScaleParams( float imageRate, double voxelBaseHt, double* voxelPeakHt, double* voxelPeakTime, double* voxelUpSlope, double* voxelWashout, double* voxelWashoutPos);

        double       baselineMean;
        double       baselineStdDeviation; 
        int          injectionPoint; 
        
};

}   //svk


#endif //SVK_DCE_BASIC_FIT_H

