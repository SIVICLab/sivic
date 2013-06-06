/*
 *  Copyright © 2009-2012 The Regents of the University of California.
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


#ifndef SVK_HSVD_H
#define SVK_HSVD_H


#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <svkImageInPlaceFilter.h>

#include <complex>


extern "C" { // C interface
    #undef max
    #undef min
    #undef small
    #undef large
    //#include "clapack.h"
    #include "f2c.h"
}

//  For some reason including the clapack.h inside the above extern"C" block
//  doesn't work as expected. 
extern "C" {
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
}




namespace svk {


using namespace std;



/*! 
 *  Class to use HSVD algorithm to model time domain spectrum
 *  primarily for removal of unwanted spectral components such as
 *  water or fat.  
 * 
 *  References/Attributions to be inserted.
 * 
 *  This class was developed: 
 *      Bjoern Menze, Ph.D. 
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 *  
 */
class svkHSVD : public svkImageInPlaceFilter
{


    public:

        static svkHSVD* New();
        vtkTypeRevisionMacro( svkHSVD, svkImageInPlaceFilter);

        
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

        void                ExportFilterImage(); 
        svkMrsImageData*    GetFilterImage(); 


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


    private:

        typedef std::complex<float>         complexf;
        typedef std::complex<double>        complexd;
        typedef std::complex<long double>   complexld;

        void    HSVD( int cellID, vector< vector <double> >* hsvdModel ); 
        void    GenerateHSVDFilterModel( int cellID, vector< vector<double> >* hsvdModel); 
        void    SubtractFilter(); 
        void    CheckInputSpectralDomain(); 
        void    CheckOutputSpectralDomain(); 


        void    MatMat( 
                    const doublecomplex* dc, 
                    const doublecomplex* dc, 
                    int m, 
                    int n, 
                    doublecomplex* dc
        );

        void    MatSq( 
                    const doublecomplex* dc, 
                    int m, 
                    int n, 
                    doublecomplex* dc
        );

        void    MatVec( 
                    const doublecomplex* dc, 
                    const doublecomplex* dc, 
                    int m, 
                    int n, 
                    doublecomplex* dc
        );

        vector< vector< float > >   filterRules; 
        svkMrsImageData*            filterImage; 
        bool                        isInputInTimeDomain; 
        bool                        exportFilterImage; 

};


}   //svk


#endif //SVK_HSVD_H

