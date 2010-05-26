/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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


#include <svkSpecUtils.h>


using namespace svk;


vtkCxxRevisionMacro(svkSpecUtils, "$Rev$");


/*!
 *  Returns the magnitude of the complex spectrum at the specified point. 
 */
float svkSpecUtils::GetMagnigutude(vtkFloatArray* spectrum, int point)
{
    float magnitudeValue = ( spectrum->GetTuple( point ) )[0] * ( spectrum->GetTuple( point ) )[0] +
                           ( spectrum->GetTuple( point ) )[1] * ( spectrum->GetTuple( point ) )[1] ;
    magnitudeValue = pow( 
            (double)magnitudeValue, 
            .5 
    );
    return magnitudeValue; 
}


/*!
 *  Applies the specified phase (in degrees) to the complex spectrum at the specified point. 
 */
void svkSpecUtils::PhaseSpectrum(vtkFloatArray* spectrum, float phase, int point, float phasedPoint[2])
{

    phase = phase * vtkMath::Pi()/180.0;
    float cosPhase = static_cast<float>( cos( phase ) );
    float sinPhase = static_cast<float>( sin( phase ) );

    spectrum->GetTupleValue(point, phasedPoint);
    float realValue      = phasedPoint[0] * cosPhase - phasedPoint[1] * sinPhase;
    float imaginaryValue = phasedPoint[0] * sinPhase + phasedPoint[1] * cosPhase;
    phasedPoint[0] = realValue;
    phasedPoint[1] = imaginaryValue;

}


/*!
 *  Returns the proton chemical shift of the center of the spectrum at given frequency and 
 *  temperature (default is body temperature).  Based on the linear relationship between 
 *  the chemical shift of water and temperature.
 *      freqOffset is the number of Hz that the center is offset from water 
 *      transmitFreq is in Hz. 
 */
float svkSpecUtils::GetPPMRef(float transmitFreq, float freqOffset, float temp )
{

    float ZERO_KELVIN       = 273.;
    float H20_Y_INTERCEPT   = 7.83;
    float H20_SLOPE         = 96.9;
    float BODY_TEMPERATURE  = 36.6;
    float ppmRef = H20_Y_INTERCEPT - ( (BODY_TEMPERATURE + ZERO_KELVIN)/H20_SLOPE ) - (freqOffset / transmitFreq );

    return ppmRef;
}
