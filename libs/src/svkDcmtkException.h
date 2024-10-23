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


#ifndef  SVK_DCMTK_EXCEPTION_H
#define  SVK_DCMTK_EXCEPTION_H


#include <stdexcept>
#include <string>
#include </mnt/nfs/rad/apps/netopt/versions/dcmtk/latest/include/dcmtk/config/osconfig.h>
#include </mnt/nfs/rad/apps/netopt/versions/dcmtk/latest/include/dcmtk/dcmdata/dctk.h>


namespace svk {


using namespace std; 


///                                                                                                                           
class svkDicomRunTimeError : public runtime_error
{  
    public:
        svkDicomRunTimeError (const string& msg) : runtime_error("WARNING: DICOM Runtime Error: " + msg) {};                                                                     
        svkDicomRunTimeError (const DcmTagKey& tag, const string& msg) : 
            runtime_error("WARNING: DICOM Runtime Error: " + msg + " tag: " + DcmTag(tag).toString().c_str() +" "+
                        ((DcmTag)tag).getTagName()) {
                        };                                                                     
};                                                                                                                            


///                                                                                                                           
class svkTagNotFound : public svkDicomRunTimeError
{  
    public:
        svkTagNotFound (const string& msg) : svkDicomRunTimeError("Tag Not Found: " + msg) {};                                                                     
        svkTagNotFound (const DcmTagKey& tag, const string& msg) : svkDicomRunTimeError(tag,"Tag Not Found: " + msg) {};                                                                     
};                                                                                                                            


///                                                                                                                           
class svkIncompatibleVR : public logic_error
{  
    public:
        svkIncompatibleVR (const string& msg) : logic_error("Incompatible VR: "+msg) {};                                                                     
};                                                                                                                            
#ifdef WIN32
#pragma warning( disable : 4290 )
#endif
/** handle return value of DcmTk functions.
  * if error is not ok, throws various exceptions 
  * @throws svkTagNotFound
  * @throws svkDicomRunTimeError
  */
extern void handleError(OFCondition error, string message = "", const DcmTagKey *ptag = NULL) throw (svkTagNotFound, svkDicomRunTimeError);



} // svk

#endif

