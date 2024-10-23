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

#ifndef SVK_VARIAN_CS_FID_MAPPER_H
#define SVK_VARIAN_CS_FID_MAPPER_H


#include </usr/include/vtk/vtkImageData.h>
#include <svkVarianFidMapper.h>
#include <svkCSReorder.h>
#include <vector>
#include "svkDataAcquisitionDescriptionXML.h"
#include <svkUtils.h>
#include <svkTypeUtils.h>


namespace svk {


/*! 
 *  This is a concrete instance of an svkVarianFIDMapper for Varian 3D compressed 
 *  sensing data.  This mapper is used by the svkVarianFidReader to convert Varian 
 *  FID/Procpar data, from the UCSF compressed sensing pulse sequence, into an 
 *  svkImageData object  (includeing a DICOM MR Spectroscopy header). The Varian 
 *  compressed sensing pulse sequence was implemented by Dr. Sukumar Subramaniam 
 *  (UCSF NMR Lab) based on the work of Drs. Hu, Lustig et al.:
 *      Simon Hu, Michael Lustig, Asha Balakrishnan, Peder E. Z. Larson, Robert Bok, 
 *      John Kurhanewicz, Sarah J. Nelson, Andrei Goga, John M. Pauly, Daniel B. Vigneron: 
 *      "3D compressed sensing for highly accelerated hyperpolarized 13c mrsi with in 
 *      vivo applications to transgenic mouse models of cancer", Magn. Reson. Med., 
 *      63(2), 312-321 (2009).
 * 
 *  This mapper takes the sparsely sampled compressed sensing k-space data and reorganizes 
 *  it into a rectalinear data array with kt, kx, ky and kz organized into a regular 4D grid 
 *  with missing points from undersampling filled in with zeros. At the moment, the data 
 *  sampling order is hardcoded into this class for this specific acquisition. 
 *
 *  This missing k-space data from undersampling (encoded in the output data as zeros) needs 
 *  to be filled in using the L1 algorithm described here: 
 *      Lusting M, Donoho D.L., Pauly J.M: "Sparse MRI: the application of compressed sensing for 
 *      rapid MR imaging", Magn. Reson. Med., 58, 1182-1195 (2007). 
 *
 *  Additionally, phase correction for the flyback encoding needs to be accounted for by 
 *  downstream algorithms. 
 *
 *  Thanks to Dr. Simon Hu and Dr. Sukumar Subramaniam from the Vigneron and Kurhanewicz labs 
 *  at the UCSF Department of Radiology and Biomedical Imaging for help implementing this class. 
 * 
 */
class svkVarianCSFidMapper : public svkVarianFidMapper
{

    public:

        vtkTypeMacro( svkVarianCSFidMapper, svkVarianFidMapper);
        static          svkVarianCSFidMapper* New();

        virtual void    ReadFidFile( string fidFileName, svkImageData* data );


    protected:

        svkVarianCSFidMapper();
        ~svkVarianCSFidMapper();
  
        virtual void    InitMultiFrameFunctionalGroupsModule();
        virtual void    InitAcquisitionContextModule();
        virtual void    InitPlaneOrientationMacro(); 
        virtual void    InitSharedFunctionalGroupMacros();
        virtual void    InitPerFrameFunctionalGroupMacros();
        virtual void    InitPixelMeasuresMacro();
        virtual void    InitMRReceiveCoilMacro();
        virtual void    InitMRSpectroscopyPulseSequenceModule();
        virtual void    InitMRSpectroscopyModule();
        virtual void    InitMRSpectroscopyFOVGeometryMacro();
        virtual void    InitMRSpectroscopyDataModule();

        virtual void    SetCellSpectrum(
                            vtkImageData* data, 
                            int x, int y, int z, 
                            int timePt, 
                            int coilNum
                        );
        svkDataAcquisitionDescriptionXML* GetDadFile();

    private:

        float***            paddedData; 
        float****           rectilinearData; 
        void                ReOrderSamples( float* specDataReordered, int numberDataPointsInFIDFile );
        void                ReOrderFlyback();
        void                ZeroPadCompressedSensingData( int numberDataPointsInFIDFile );
        vector<int>         GetBlips();
        void                GetBlipString(string* blipString);

        int                 numXReordered; 
        int                 numYReordered; 
        int                 numZReordered; 
        int                 numTReordered;
        string              fidFileName;
        svkDataAcquisitionDescriptionXML* dadFile;
        svkCSReorder*       csReorder;

};


}   //svk

#endif //SVK_VARIAN_CS_FID_MAPPER_H

