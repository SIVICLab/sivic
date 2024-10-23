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


#ifndef SVK_DCMTK_UTILS_H
#define	SVK_DCMTK_UTILS_H

#include </mnt/nfs/rad/apps/netopt/versions/dcmtk/latest/include/dcmtk/config/osconfig.h>
#include </mnt/nfs/rad/apps/netopt/versions/dcmtk/latest/include/dcmtk/dcmdata/dctk.h>

#include <stdexcept>
#include <float.h>
#include <sstream>
#include <limits.h>

#include <svkDcmtkException.h>


namespace svk {


using namespace std;


/*! 
 */
class svkDcmtkUtils 
{

    public:

        //  Methods:
        static void setValue(DcmItem* item, const DcmTag &tag, const int value, const int pos = 0)
                throw (overflow_error, svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR);

        static void setValue(DcmItem* item, const DcmTag &tag, const long int value)
                throw (svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR); 

        static void setValue(DcmItem* item, const DcmTag &tag, const float value)
                throw (svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR);

        static void setValue(DcmItem* item, const DcmTag &tag, const double value)
                throw (overflow_error, svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR);

        static void setValue(DcmItem* item, const DcmTag &tag, const string value)
                throw (svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR);



        static int getIntValue(DcmItem* item, const DcmTagKey &tag, const int pos = 0) 
            throw (svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR);

        static long int getLongIntValue(DcmItem* item, const DcmTagKey &tag, const int pos = 0) 
            throw (svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR);

        static float getFloatValue(DcmItem* item, const DcmTagKey &tag, int pos = 0)
            throw (overflow_error, svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR);

        static double getDoubleValue(DcmItem* item, const DcmTagKey &tag, bool searchInto = false ) 
            throw (svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR);

        static string getStringValue(DcmItem* item, const DcmTagKey &tag, int pos)
            throw (svkDicomRunTimeError, svkTagNotFound);

        static string getStringValue(DcmItem* item, const DcmTagKey &tag)
            throw (svkDicomRunTimeError, svkTagNotFound);

};


}   //svk


#endif  /* SVK_DCMTK_UTILS_H */

