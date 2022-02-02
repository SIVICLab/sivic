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


#ifndef  SVK_TYPES_H           
#define  SVK_TYPES_H          


typedef enum {
    UNDEFINED_EPSI_TYPE = 0,
    FLYBACK,
    SYMMETRIC,
    INTERLEAVED
} EPSIType;

#if defined(__cplusplus) && !defined(SVK_EPIC)
namespace svk {


using namespace std;


class svkTypes : public vtkObject
{

    public:

        vtkTypeMacro( svkTypes, vtkObject);

        typedef enum {
            ANATOMY_UNDEFINED = -1,
            ANATOMY_BRAIN = 0,
            ANATOMY_PROSTATE
        } AnatomyType;



        static string  GetAnatomyTypeString( AnatomyType anatomyType ) 
        {
            if ( anatomyType == ANATOMY_BRAIN ) {
                return string("brain"); 
            } else if ( anatomyType == ANATOMY_PROSTATE) {
                return string("prostate"); 
            }  else {
                return "UNDEFINED"; 
            }
        }

        static svkTypes::AnatomyType GetAnatomyType( string anatomyType) 
        {
            if ( anatomyType.compare("brain") == 0 ) {
                return svkTypes::ANATOMY_BRAIN; 
            } else if ( anatomyType.compare("prostate") == 0 ) {
                return svkTypes::ANATOMY_PROSTATE; 
            } else {
                return svkTypes::ANATOMY_UNDEFINED; 
            }
        }
};

}
#endif
#endif
