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
 *  $URL: svn+ssh://jccrane@svn.code.sf.net/p/sivic/code/trunk/libs/src/svkF2C.h $
 *  $Rev: 2108 $
 *  $Author: jccrane $
 *  $Date: 2014-12-10 14:33:37 -0800 (Wed, 10 Dec 2014) $
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#ifndef SVK_F2C_H
#define SVK_F2C_H


#ifdef __cplusplus

#include <vtkObject.h>
#include <vtkObjectFactory.h>


namespace svk {


using namespace std;


/*! 
 *  Class to support using SIVIC readers/writers from FORTRAN.
 */
class svkF2C: public vtkObject
{

    public:

        static svkF2C* New();
        vtkTypeRevisionMacro( svkF2C, vtkObject);

        static int  GetIDFHeader( char* idfFileName ); 


    protected:

        svkF2C();
        ~svkF2C();


    private:



};


}   //svk
#endif

#endif //SVK_F2C_H

#ifdef __cplusplus
extern "C" {
#endif


void  svkf2c_getidfheader_(char* idfFileName); 


#ifdef __cplusplus
}
#endif
