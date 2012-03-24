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
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#ifndef SVK_SPEC_UTILS_H
#define SVK_SPEC_UTILS_H


#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkFloatArray.h>
#include <vtkMath.h>
#include <vtkImageFourierFilter.h>

namespace svk {



/*! 
 *  Static utility methods for swapping bytes. 
 */
class svkSpecUtils : public vtkObject
{

    public:

        // vtk type revision macro
        vtkTypeRevisionMacro( svkSpecUtils, vtkObject );
        
        static float    GetMagnigutude(vtkFloatArray* spectrum, int point); 
        static void     PhaseSpectrum(vtkFloatArray* spectrum, float phase, int point, float phasedPoint[2]); 
        static float    GetChemicalShiftReference(); 
        static float    GetPPMRef(float transmitFreq, float freqOffset = 0, float temp = svkSpecUtils::BODY_TEMPERATURE ); 
        static void     CreateLinearPhaseShiftArray(int N, vtkImageComplex* phaseArray, double shift);

        static const float ZERO_KELVIN;
        static const float H2O_Y_INTERCEPT; 
        static const float H2O_SLOPE; 
        static const float BODY_TEMPERATURE; 

};


}   //svk


#endif //SVK_SPEC_UTILS_H
