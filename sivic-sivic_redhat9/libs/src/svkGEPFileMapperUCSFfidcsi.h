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

#ifndef SVK_GE_PFILE_MAPPER_UCSF_FIDCSI_H
#define SVK_GE_PFILE_MAPPER_UCSF_FIDCSI_H


#include <svkGEPFileMapperUCSF.h>


namespace svk {


/*! 
 *  Mapper from pfile header to DICOM IOD/SOP Class instance, overrides
 *  specific product logic with UCSF fidcsi psd research sequence logic. 
 */
class svkGEPFileMapperUCSFfidcsi : public svkGEPFileMapperUCSF 
{

    public:

        vtkTypeMacro( svkGEPFileMapperUCSFfidcsi, svkGEPFileMapperUCSF );
        static svkGEPFileMapperUCSFfidcsi* New();

    protected:

        svkGEPFileMapperUCSFfidcsi();
        ~svkGEPFileMapperUCSFfidcsi();
  
        virtual void            GetSelBoxCenter( float selBoxCenter[3] );
        virtual void            GetCenterFromRawFile( double* center );
        virtual string          GetVolumeLocalizationTechnique(); 

        virtual void            GetFOV( float fov[3] ); 
        virtual void            GetVoxelSpacing( double voxelSpacing[3] ); 

    private: 
        string                  GetAcqDADFileName( ); 
        int                     InitAcqDad( ); 




};


}   //svk

#endif //SVK_GE_PFILE_MAPPER_UCSF_FIDCSI_H

