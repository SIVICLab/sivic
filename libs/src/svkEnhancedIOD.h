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


#ifndef SVK_ENHANCED_IOD_H
#define SVK_ENHANCED_IOD_H

#define UNKNOWN_TIME -1

#include <vtkObjectFactory.h>
#include <svkIOD.h>


namespace svk {


using namespace std;


/*! 
 *  Base class of static methods for modules shared between MRI and MRS ENHANCED_IODs. 
 */
class svkEnhancedIOD : public svkIOD 
{

    public:

        vtkTypeRevisionMacro( svkEnhancedIOD, svkIOD);

        //  Methods Shared between Enhanced MRI and MRS IODs:
        //void          InitPixelMeasuresMacro();  
        //void          InitPlaneOrientationMacro(); 
        void          InitFrameAnatomyMacro(); 
        void          InitMRTimingAndRelatedParametersMacro(float tr = UNKNOWN_TIME, float flipAngle= -999, int numEchoes = 1); 
        void          InitMREchoMacro(float TE = UNKNOWN_TIME ); 
        void          InitMRModifierMacro(float inversionTime = UNKNOWN_TIME); 
        //void          InitMRReceiveCoilMacro(); 
        void          InitMRTransmitCoilMacro(string coilMfg = "UNKNOWN", string coilName = "UNKNOWN", string coilType = "UNKNOWN"); 
        void          InitMRAveragesMacro(int numAverages = 1); 


    protected:

        svkEnhancedIOD();
        ~svkEnhancedIOD();

};


}   //svk


#endif //SVK_ENHANCED_IOD_H

