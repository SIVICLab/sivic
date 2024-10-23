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


#ifndef SVK_METABOLITE_RATIO_Z_SCORES_H
#define SVK_METABOLITE_RATIO_Z_SCORES_H


#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkInformation.h>

#include <svkImageData.h>
#include <svkMriImageData.h>
#include <svkMrsImageData.h>
#include <svkImageAlgorithm.h>
#include <svkDcmHeader.h>
#include <svkEnhancedMRIIOD.h>


namespace svk {


using namespace std;


/*! 
 *  Class to generate z-scores (aka metabolite indices) from metabolite ratios. Z-scores are analyzed
 *  over the entire MRS volume (or sub-set of voxels, e.g. within the selected volume) and represent the 
 *  number of standard deviations (perpendicular distance) a given metabolite ratio is from the linear 
 *  regresion.  This computation is repeated with z-score outliers thrown out in subsequent iterations until 
 *  the regresion fit converges. This class takes metabolite map inputs for already quantified values.  These 
 *  may be integrated area maps, peak_ht maps, or maps derived from other quantification methods.  This class 
 *  only computes the z-score map for the given input. The z-scores are computed as defined in reference 1.  
 *  The reported map is the ratio of the perpendicular distance of a point from the regression line, normalized
 *  by the standard deviation of these distance values.  
 *
 *  References:
 *  
 *  1. Tracy R. McKnight PhD, Susan M. Noworolski PhD, Daniel B. Vigneron PhD, Sarah J. Nelson PhD, 
 *     "An Automated Technique for the Quantitative Assessment of 3D-MRSI Data from Patients with
 *     Glioma", Journal of Magnetic Resonance Imaging, 13:167-177 (2001).  
 */
class svkMetaboliteRatioZScores: public svkImageAlgorithm
{

    public:

        static svkMetaboliteRatioZScores* New();
        vtkTypeMacro( svkMetaboliteRatioZScores, svkImageAlgorithm);

        void                    SetSeriesDescription(std::string newSeriesDescription);
        void                    SetVerbose( bool isVerbose );     
        void                    LimitToSelectedVolume(float fraction = 0.5001);
        void                    LimitToSelectedVolume( short* selectedVolumeMask);
        void                    SetInputNumerator(vtkDataObject *in) { this->SetInputDataObject(0, in); }
        void                    SetInputDenominator(vtkDataObject *in) { this->SetInputDataObject(1, in); }
        void                    SetInputMrsData(vtkDataObject *in) { this->SetInputDataObject(2, in); }
        void                    SetZScoreThresholds( double lowerThreshold, double upperThreshold ); 


    protected:

        svkMetaboliteRatioZScores();
        ~svkMetaboliteRatioZScores();

        virtual int             RequestData( 
                                    vtkInformation* request, 
                                    vtkInformationVector** inputVector, 
                                    vtkInformationVector* outputVector 
                                );

        virtual int             FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info );
        virtual int             FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info ); 


    private:

        //  Methods:
        void                    ZeroData(); 
        void                    InitQuantificationMask(); 
        int                     GetTotalVoxels(); 
        void                    ComputeZScore(); 
        virtual void            UpdateProvenance();
        double                  GetMean( double* pixels, int numVoxels ); 
        double                  GetDistanceSD( double* pixels, int numVoxels); 
        double                  GetRegressionSlope( 
                                    double* numeratorPixels, 
                                    double* denominatorPixels, 
                                    double numeratorMean, 
                                    double denominatorMean, 
                                    int numVoxels 
                                ); 
        double                  GetRegressionSlopeZeroIntercept(
                                    double* numeratorPixels, 
                                    double* denominatorPixels, 
                                    int totalVoxels
                                ); 
        double                  GetRegressionIntercept(double slope, double numeratorMean, double denominatorMean); 
        void                    GetRegression(double& slope, double& intercept); 
        void                    GetDistanceFromRegression( double slope, double intercept ); 
        void                    GetZScores(double slope, double intercept); 
        int                     GetOutliers(); 


        //  Members:
        std::string          newSeriesDescription; 
        bool                    isVerbose; 
        float                   useSelectedVolumeFraction;
        short*                  quantificationMask;
        short*                  iterationMask;
        double                  zscoreLowerThreshold; 
        double                  zscoreUpperThreshold; 
        bool                    yInterceptZero;

};


}   //svk


#endif //SVK_METABOLITE_RATIO_Z_SCORES_H

