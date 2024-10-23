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
 *      
 *     
 */


#ifndef SVK_EPSI_PHASE_CORRECT_H
#define SVK_EPSI_PHASE_CORRECT_H

#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>

#include <svkMrsImageFFT.h>
#include <svkImageInPlaceFilter.h>
#include <svkEPSIReorder.h>


namespace svk {


using namespace std;


/*! 
 *  This class corrects for the time shift of spectra in the 
 *  EPSI direction that results from the EPSI sampling trajectory. 
 *  For each TR, the spectra from neighboring K space values along 
 *  the EPSI k-space axis are displaced in time relative to one
 *  another by Dt (time between EPSI samples).  This time-domain 
 *  shift is corrected here via the Fourier shift theorem, by applying
 *  a linear phase shift to the spectra in the frequency domain, 
 *  where the magnitude of the phase shift is a linear functino of the
 *  distance of the spectrum from the origin of k-space (in EPSI 
 *  direction).  Spectra are then inverse FT'd back to the time domain 
 *  resulting in a rectilinear spectral/spatial data set (see Figure 3 
 *  and equation 3 in the following reference). 
 *  
 *  References:
 *      Charles H. Cunningham, Daniel B. Vigneron, Albert P. Chen, Duan Xu, 
 *      Sarah J. Nelson, Ralph E. Hurd, Douglas A. Kelley, John M. Pauly:
 *      "Design of Flyback Echo-Planar Readout Gradients for Magnetic
 *      Resonance Spectroscopic Imaging", Magnetic Resonance in Medicine
 *      54:1286-1289 (2005).  
 * 
 */
class svkEPSIPhaseCorrect : public svkImageInPlaceFilter
{

    public:

        static svkEPSIPhaseCorrect* New();
        vtkTypeMacro( svkEPSIPhaseCorrect, svkImageInPlaceFilter);

        void            SetNumEPSIkRead( int numKspaceSamples );
        void            SetEPSIAxis( int epsiAxis );
        void            SetEPSIOrigin( float epsiOrigin );
        float           GetEPSIOrigin();
        void            SetEPSIType( EPSIType type );


    protected:

        svkEPSIPhaseCorrect();
        ~svkEPSIPhaseCorrect();

        virtual int     FillInputPortInformation(int port, vtkInformation* info);

        //  Methods:
        virtual int     RequestData(
                            vtkInformation* request, 
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector
                        );


    private: 

        void            CreateEPSIPhaseCorrectionFactors( vtkImageComplex** phaseArray, int numSpecPts ); 
        int             SpectralFFT( svkMrsImageFFT::FFTMode direction ); 
        void            PhaseAlternatingSymmetricEPSILobes( int cellID ); 


        int                      numEPSIkRead;
        int                      epsiAxis;
        float                    epsiOrigin;
        double*                  epsiSpatialPhaseCorrection;
        vtkImageComplex*         symEPSIPhaseArray; 
        svkImageData*            tmpData; 
        EPSIType                 epsiType;
        int                      phaseSlope; 

};


}   //svk


#endif //SVK_EPSI_PHASE_CORRECT_H

