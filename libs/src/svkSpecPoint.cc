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


#include <svkSpecPoint.h>


using namespace svk;


//vtkCxxRevisionMacro(svkSpecPoint, "$Rev$");
vtkStandardNewMacro(svkSpecPoint);


/*!
 *
 */
svkSpecPoint::svkSpecPoint()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

}


/*!
 *
 */
svkSpecPoint::~svkSpecPoint()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}


/*!
 *
 */
void svkSpecPoint::SetDcmHeader(svkDcmHeader* hdr)
{
    this->sweepWidth   = hdr->GetFloatValue( "SpectralWidth" );
    this->numPts       = hdr->GetIntValue( "DataPointColumns" );
    this->freqOffset   = 0;
    this->ppmRef       = hdr->GetFloatValue( "ChemicalShiftReference" );
    this->transmitFreq = hdr->GetFloatValue( "TransmitterFrequency" );
}


/*! Frequency scale and ppm scale decrease from left to right.  
 *  frequency_offset is how far to the left in HZ (positive HZ), the water reference 
 *  was shifted.
 */
float svkSpecPoint::ConvertPosUnits(float position, int inType, int targetType) 
{

    if (inType == targetType) {
        return position;
    }
        
    float positionPoints;
    float positionHz;
    float positionPPM;
        
    if (inType == PTS) {

        positionPoints = position;
        positionHz = ConvertPtsToHz(positionPoints, numPts, sweepWidth, freqOffset); 

        if (targetType == Hz) {
            return positionHz;
        }
        if (targetType == PPM) {
            return ConvertHzToPPM(positionHz, ppmRef, freqOffset, transmitFreq); 
        }
    }

    if (inType == Hz) {

        positionHz = position;

        if (targetType == PTS) {
            return ConvertHzToPts(positionHz, numPts, sweepWidth, freqOffset); 
        }
        if (targetType == PPM) {
            return ConvertHzToPPM(positionHz, ppmRef, freqOffset, transmitFreq); 
        }
    }

    if (inType == PPM) {

        positionPPM = position;
        positionHz = ConvertPPMToHz(positionPPM, ppmRef, freqOffset, transmitFreq); 

        if (targetType == PTS) {
            return ConvertHzToPts(positionHz, numPts, sweepWidth, freqOffset); 
        }
        if (targetType == Hz) {
            return positionHz;
        }
    }

}


/*
 *
 */
float svkSpecPoint::ConvertHzToPts(float positionHz, int numPts, float sweepWidth, float freqOffset) 
{
    return (numPts/sweepWidth)*(sweepWidth/2 - positionHz - freqOffset); 
}


/*
 *
 */
float svkSpecPoint::ConvertHzToPPM(float positionHz, float ppmRef, float freqOffset, float transmitFreq) 
{
    return  ppmRef + (positionHz+freqOffset)/transmitFreq;   
}


/*
 *
 */
float svkSpecPoint::ConvertPPMToHz(float positionPPM, float ppmRef, float freqOffset, float transmitFreq) 
{
    return transmitFreq * (positionPPM - ppmRef) - freqOffset;
}


/*
 *
 */
float svkSpecPoint::ConvertPtsToHz(float positionPoints, int numPts, float sweepWidth, float freqOffset) 
{
    return sweepWidth/2 - (positionPoints * (sweepWidth/numPts)) - freqOffset;
}
