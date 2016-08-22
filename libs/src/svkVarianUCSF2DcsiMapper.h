/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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

#ifndef SVK_VARIAN_UCSF_2D_CSI_MAPPER_H
#define SVK_VARIAN_UCSF_2D_CSI_MAPPER_H


#include <vtkImageData.h>

#include <svkDcmHeader.h>
#include <svkMRSIOD.h>
#include <svkVarianFidMapper.h>

#include <map>
#include <string>
#include <vector>


namespace svk {


/*! 
 *  Concrete mapper for C13 UCSF 2D CSI Varian sequence. 
 *  
 *  contributors to the development of the 2D C13 CSI sequence and software :
 *      Sukumar Subramaniam, PhD (UCSF Surbeck Lab) 
 *      Jason C. Crane, PhD (UCSF Surbeck Lab) 
 *      Sarah J. Nelson, PhD (UCSF Surbeck Lab) 
 *      Dan B. Vigneron, PhD (UCSF Surbeck Lab) 
 *      John Kurhanewicz, PhD (UCSF Surbeck Lab) 
 *  
 *  Supported by: NIH P41EB013598
 *
 */
class svkVarianUCSF2DcsiMapper : public svkVarianFidMapper
{

    public:

        vtkTypeMacro( svkVarianUCSF2DcsiMapper, svkVarianFidMapper);
        static          svkVarianUCSF2DcsiMapper* New();

        
    protected:

        svkVarianUCSF2DcsiMapper();
        ~svkVarianUCSF2DcsiMapper();

        virtual void    InitMRSpectroscopyPulseSequenceModule(); 
        virtual void    InitPixelMeasuresMacro(); 
        virtual void    InitPerFrameFunctionalGroupMacros(); 

};


}   //svk

#endif //SVK_VARIAN_UCSF_2D_CSI_MAPPER_H

