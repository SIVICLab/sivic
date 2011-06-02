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

#ifndef SVK_GE_PFILE_MAPPER_UCSF_FIDCSI_DEV0_H
#define SVK_GE_PFILE_MAPPER_UCSF_FIDCSI_DEV0_H


#include <svkGEPFileMapperUCSF.h>


namespace svk {


/*! 
 *  Mapper from a GE P-file header to a DICOM MRS SOP Class. This 
 *  concrete svkGEPFileMapper overrides default product sequence logic 
 *  with specifics required for reading symmetric EPSI data acquired with  
 *  the fidcsi_ucsf_dev0 research psd developed by Peder Larson, PhD,  UCSF. 
 *  This mapper outputs a regular cartesian grid of k-space data suitable
 *  for Fourier transform reconstruction. 
 *  
 *  Non-uniformly sampled Symmetric EPSI ramp data is sampled to a cartesian 
 *  grid using a "Gridding"  algorithm using a Kaiser-Bessel convolving function 
 *  as described here:
 *
 *      IEEE TRANSACTIONS ON MEDICAL IMAGING. VOL. IO. NO. 3 , SEPTEMBER 1991
 *      Selection of a Convolution Function for Fourier Inversion Using Gridding
 *      John I. Jackson, Craig H. Meyer, Dwight G. Nishimura and Albert Macovski
 *
 *  The following people contributed to the development, implementation and 
 *  validation of this class: 
 *  
 *      Peder EZ Larson, PhD (UCSF Department of Radiology and Biomedical Imaging)
 *      Jason C. Crane, PhD (UCSF Department of Radiology and Biomedical Imaging) 
 *      Sarah J. Nelson, PhD (UCSF Department of Radiology and Biomedical Imaging)
 *      Daniel Vigneron, PhD (UCSF Department of Radiology and Biomedical Imaging)
 *      Brian Hargreaves, PhD (Standford University ) ?????
 *  
 */
class svkGEPFileMapperUCSFfidcsiDev0 : public svkGEPFileMapperUCSF 
{

    public:

        vtkTypeRevisionMacro( svkGEPFileMapperUCSFfidcsiDev0, svkGEPFileMapperUCSF );
        static svkGEPFileMapperUCSFfidcsiDev0* New();

        virtual void    ReadData(vtkstd::string pFileName, svkImageData* data); 


    protected:

        svkGEPFileMapperUCSFfidcsiDev0();
        ~svkGEPFileMapperUCSFfidcsiDev0();
  
        virtual vtkstd::string  GetVolumeLocalizationTechnique(); 
        virtual void            GetSelBoxCenter( float selBoxCenter[3] );
        virtual void            GetSelBoxSize( float selBoxSize[3] );
        virtual void            GetCenterFromRawFile( double* center ); 
        virtual bool            IsChopOn(); 
        void                    ModifyForPatientEntry( svkImageData* data ); 


    private: 

        void                    ReorderEPSIData( svkImageData* data ); 
        void                    RemoveArrays( svkImageData* data ); 
        void                    RedimensionData( svkImageData* data, int* numVoxelsOriginal, int* numVoxelsReordered, int numFreqPts ); 
        void                    EPSIPhaseCorrection( 
                                    svkImageData* data, 
                                    int* numVoxels, 
                                    int numRead, 
                                    int epsiAxis
                                ); 
        void                    ReverseOddEPSILobe( svkImageData* data, int epsiAxis ); 
        void                    ZeroFill( svkImageData* data ); 
        void                    FlipAxis( svkImageData* data, int axis ); 
        void                    FFTShift( svkImageData* data ); 
        void                    ResampleRamps( svkImageData* data, int deltaT, int plateauTime, int rampTime, int epsiAxis ); 
        virtual void            GetWaveFormIntegral( float* waveFormIntegral, int deltaT, int plateauTime, int rampTime ); 
        void                    GetKaiserBesselValues( 
                                    vtkstd::vector<float>* u, 
                                    float width, 
                                    float beta,
                                    vtkstd::vector<float>* kbVals 
                                    ); 
        double                  GetBessel0Term( float arg, int index);
        double                  GetBessel0( float arg);
        double                  GetModifiedBessel0( float arg ); 
        void                    GetRolloffCorrection( int gridSize, float width, float beta, float* apodCor); 


}; 

}   //svk

#endif //SVK_GE_PFILE_MAPPER_UCSF_FIDCSI_DEV0_H

