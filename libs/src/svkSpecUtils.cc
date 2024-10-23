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


#include <svkSpecUtils.h>


using namespace svk;


//vtkCxxRevisionMacro(svkSpecUtils, "$Rev$");


const float svkSpecUtils::ZERO_KELVIN = 273.0;       
const float svkSpecUtils::H2O_Y_INTERCEPT = 7.83;  
const float svkSpecUtils::H2O_SLOPE = 96.9;
const float svkSpecUtils::BODY_TEMPERATURE = 36.6; 

//  Values of GAMMA in MHz / T
const float svkSpecUtils::GAMMA_1H  = 42.576; 
const float svkSpecUtils::GAMMA_13C = 10.705;  
const float svkSpecUtils::GAMMA_31P = 17.235;  
const float svkSpecUtils::GAMMA_19F = 40.053;  
const float svkSpecUtils::GAMMA_15N = -4.316;  


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

    spectrum->GetTypedTuple(point, phasedPoint);
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
 *      temp is in degrees Celcius. 
 */
float svkSpecUtils::GetPPMRef(float transmitFreq, float freqOffset, float temp )
{

    float ZERO_KELVIN       = svkSpecUtils::ZERO_KELVIN;    // 0C in kelvin scale 
    float H2O_Y_INTERCEPT   = svkSpecUtils::H2O_Y_INTERCEPT;
    float H2O_SLOPE         = svkSpecUtils::H2O_SLOPE;
    float BODY_TEMPERATURE  = svkSpecUtils::BODY_TEMPERATURE;

    float ppmRef = H2O_Y_INTERCEPT - ( (temp + ZERO_KELVIN)/H2O_SLOPE ) - (freqOffset / transmitFreq );

    return ppmRef;
}


/*!
 *  This method takes an array of N, vtkImageComplex values, where  vtkImageComplex
 *  is a struct with a double real and double imaginary component representing
 *  a single complex value.  The function applies a linear phase correction to the
 *  values with pivot given by the origin (middle index of array) and linear factor
 *  that increments by 2*pi/N.  An additional phase shift may be applied (e.g.
 *  for voxel shifting origin.
 *  On output the vtkImageComplex variable phaseArray contains the phase factors
 *  to be applied to each point.
 */
void svkSpecUtils::CreateLinearPhaseShiftArray(int N, vtkImageComplex* phaseArray, double shift, int origin)
{
    double phaseIncrement;
    double mult;
    for( int i = 0; i <  N; i++ ) {
        phaseIncrement = (i - origin)/((double)(N));
        mult = -2 * vtkMath::Pi()* phaseIncrement * shift;
        phaseArray[i].Real = cos(mult);
        phaseArray[i].Imag = sin(mult);
    }

}


/*!
 *  Default uses N/2 as origin.
 */
void svkSpecUtils::CreateLinearPhaseShiftArray(int N, vtkImageComplex* phaseArray, double shift)
{
	int origin = N/2;
	svkSpecUtils::CreateLinearPhaseShiftArray(N, phaseArray, shift, origin);
}


/*!
 *  Determines the nucleus based on the computed value of gamma determined from the input
 *  parameters:  
 *      transmitter frequency (MHz)
 *      field strength (Gauss)       
 *  
 *  R C Weast, M J Astle, ed. (1982). 
 *  Handbook of Chemistry and Physics. 
 *  Boca Raton: CRC Press. p. E66. ISBN 0-8493-0463-6.
 */
string svkSpecUtils::GetNucleus( float transmitFreq, float fieldStrength )
{

    float transmitFreqMhz = transmitFreq; 
    float fieldStrengthTesla = fieldStrength / 10000. ; 
    float gamma =  transmitFreqMhz / fieldStrengthTesla;  

    //  Values of GAMMA in MHz / T
    //svkSpecUtils::GAMMA_1H  = 42.576; 
    //svkSpecUtils::GAMMA_13C = 10.705;  
    //svkSpecUtils::GAMMA_31P = 17.235;  
    //svkSpecUtils::GAMMA_19F = 40.053;  
    //svkSpecUtils::GAMMA_15N = -4.316;  

    string nucleus;
    if ( fabs( gamma - svkSpecUtils::GAMMA_1H ) < 0.1 ) {
        nucleus.assign("1H");
    } else if ( fabs( gamma - svkSpecUtils::GAMMA_13C ) < 0.1 ) {
        nucleus.assign("13C");
    } else if ( fabs( gamma - svkSpecUtils::GAMMA_31P ) < 0.1 ) {
        nucleus.assign("31P");
    } else if ( fabs( gamma - svkSpecUtils::GAMMA_19F ) < 0.1 ) {
        nucleus.assign("19F");
    } else if ( fabs( gamma - svkSpecUtils::GAMMA_15N ) < 0.1 ) {
        nucleus.assign("15N");
    }

    return nucleus;  

}


/*!
 *  GetFieldStrength
 *  parameters:  
 *      transmitter frequency (MHz)
 *      nucleus        
 */
float svkSpecUtils::GetFieldStrength( string nucleus, float transmitFreq)
{

    float gamma; 
    if ( nucleus.compare("1H") == 0 ) {
        gamma = svkSpecUtils::GAMMA_1H; 
    } else if ( nucleus.compare("13C") == 0 ) {
        gamma = svkSpecUtils::GAMMA_13C; 
    } else if ( nucleus.compare("31P") == 0 ) {
        gamma = svkSpecUtils::GAMMA_31P; 
    } else if ( nucleus.compare("19F") == 0 ) {
        gamma = svkSpecUtils::GAMMA_19F; 
    } else if ( nucleus.compare("15N") == 0 ) {
        gamma = svkSpecUtils::GAMMA_15N; 
    } else {
        cout << "ERROR: Undefined nucleus: " << nucleus << endl;
    }

    float fieldStrengthTesla = transmitFreq/gamma;
    return fieldStrengthTesla; 
}
