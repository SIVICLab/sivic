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
 *      Bjoern Menze, Ph.D. 
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */

#pragma once 

#ifndef SVK_HSVD_H
#define SVK_HSVD_H


#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>
#include <svkThreadedImageAlgorithm.h>
#ifdef WIN32
#define _USE_MATH_DEFINES
#include <math.h>
#endif

extern "C" { // C interface
    #include "f2c.h"
    #undef max
    #undef min
    #undef small
    #undef large
    #undef abs
    //#include "clapack.h"
}

#include <complex>

//  For some reason including the clapack.h inside the above extern"C" block
//  doesn't work as expected. 
extern "C" {
// Here too, OsX seems to have header defs for these already which causes a definition
// conflict when building the OsiriX plugin
#ifndef SYS_F2C_OSIRIX
    int zheev_(char *jobz, char *uplo, integer *n, doublecomplex
        *a, integer *lda, doublereal *w, doublecomplex *work, integer *lwork,
        doublereal *rwork, integer *info);
    int zheevd_(char *jobz, char *uplo, integer *n,
        doublecomplex *a, integer *lda, doublereal *w, doublecomplex *work,
        integer *lwork, doublereal *rwork, integer *lrwork, integer *iwork,
        integer *liwork, integer *info);
    int zgeqp3_(integer *m, integer *n, doublecomplex *a,
        integer *lda, integer *jpvt, doublecomplex *tau, doublecomplex *work,
        integer *lwork, doublereal *rwork, integer *info);
    int zungqr_(integer *m, integer *n, integer *k,
        doublecomplex *a, integer *lda, doublecomplex *tau, doublecomplex *
        work, integer *lwork, integer *info); 
    int ztrtrs_(char *uplo, char *trans, char *diag, integer *n,
        integer *nrhs, doublecomplex *a, integer *lda, doublecomplex *b,
        integer *ldb, integer *info);
    int zgeev_(char *jobvl, char *jobvr, integer *n,
        doublecomplex *a, integer *lda, doublecomplex *w, doublecomplex *vl,
        integer *ldvl, doublecomplex *vr, integer *ldvr, doublecomplex *work,
        integer *lwork, doublereal *rwork, integer *info);
#endif
}


namespace svk {


using namespace std;



/*! 
 * .NAME svkHSVD - HSVD and baseline fitting of spectra. 
 * .SECTION Description
 *  Class to apply HSVD algorithm to model time domain spectrum
 *  primarily for removal of unwanted spectral components such as
 *  water or fat.  This underlying algorithm in this class is based 
 *  on code implemented by Bjoern Menze, Ph.D (Zurich) and 
 *  B. Michael Kelm,  Ph.D.(Erlangen).  
 * 
 *  References: 
 *      Pijnappel WWF, van den Boogart A, de Beer R, van Ormondt D. 
 *      SVD-based quantification of magnetic resonance signals. J Magn Reson 1992. 97: 122-134
 * 
 *  This SIVIC class was developed by: 
 *      Bjoern Menze, Ph.D. 
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 *  
 */
    
class svkHSVD : public svkThreadedImageAlgorithm
{

    public:
        
        enum HSVDBehaviorOnError {
            SET_FILTER_TO_ZERO = 0, // leave input signal unchanged      
            SET_SIGNAL_TO_ZERO = 1, 
            IGNORE_ERROR       = 2  // output filter image on error
        };

        static svkHSVD* New();
        vtkTypeMacro( svkHSVD, svkThreadedImageAlgorithm);

        void    RemoveH20On();
        void    RemoveLipidOn();

        void    AddPPMFrequencyFilterRule(
                    float frequencyLimit1PPM,
                    float frequencyLimit2PPM
        );

        void    AddFrequencyAndDampingFilterRule(
                    float frequencyLimit1PPM,
                    float frequencyLimit2PPM,
                    float dampingThreshold
        );

        void    AddDampingFilterRule(
                    float dampingThreshold
        );
        bool                GetFitSuccessStatus();
        void                ExportFilterImage();
        svkMrsImageData*    GetFilterImage();
        svkMriImageData*    GetFitSuccessImage();
        void                OnlyFitSpectraInVolumeLocalization();
        void                SetModelOrder( int modelOrder );

        void                SetErrorHandlingBehavior( HSVDBehaviorOnError errBehavior );
        void                SetErrorHandlingSignalToZeroOn();    
        void                SetErrorHandlingFilterToZeroOn();
        void                SetErrorHandlingIgnoreError();       

        void                SetThresholdModelDifference( float percentDifferenceUp, float percentDifferenceDown );

        void                SetSingleThreaded();
    
    protected:

        svkHSVD();
        ~svkHSVD();

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


        virtual void ThreadedRequestData(
                            vtkInformation* request, 
                            vtkInformationVector** inputVector, 
                            vtkInformationVector* outputVector,
                            vtkImageData*** inData, 
                            vtkImageData** outData,
                            int extent[6], 
                            int threadId
                    );
        static int*  progress;



    private:

        typedef std::complex<float>         complexf;
        typedef std::complex<double>        complexd;
        typedef std::complex<long double>   complexld;

        void    svkHSVDExecute(int ext[6], int id); 
        void    HSVDFitCellSpectrum( int cellID );
        bool    HSVD( int cellID, vector< vector <double> >* hsvdModel );
        void    GenerateHSVDFilterModel( int cellID, vector< vector<double> >* hsvdModel, bool cellFit);
        void    SubtractFilter();
        void    CheckInputSpectralDomain();
        void    CheckOutputSpectralDomain();
        bool    CanFitSignal( const doublecomplex* signal, int numPts ); 
        bool    GetFilterFailStatus(
                    int cellID,
                    vtkFloatArray* filterSpec,
                    float* qfactor
                );

        void    MatMat(
                    const doublecomplex*    matrix1,
                    const doublecomplex*    matrix2,
                    int                     m,
                    int                     n,
                    doublecomplex*          result
                );

        void    MatSq(
                    const doublecomplex*    matrix,
                    int                     m,
                    int                     n,
                    doublecomplex*          result
                );

        void    MatVec(
                    const doublecomplex*    matrix,
                    const doublecomplex*    vector,
                    int                     m,
                    int                     n,
                    doublecomplex*          result
        );


        vector< vector< float > >   filterRules;
        svkMrsImageData*            filterImage;
        bool                        isInputInTimeDomain;
        bool                        exportFilterImage;
        bool                        onlyFitInVolumeLocalization;
        int                         modelOrder;
        short*                      selectionBoxMask;

        int                         numTimePoints;
        double                      spectralWidth; 
        //vtkFloatArray*              apodizationWindow;
        double                      thresholdRMSRatioDown;
        double                      thresholdRMSRatioUp;
        int numberPtsToCheckQuality;
        HSVDBehaviorOnError         errorHandlingFlag;
        svkMriImageData* fitSuccessMap;


};


}   //svk


#endif //SVK_HSVD_H





