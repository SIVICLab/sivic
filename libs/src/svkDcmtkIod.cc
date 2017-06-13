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


#include <svkDcmtkIod.h>


using namespace svk;


//! Constructor.  Initializes all DICOM modules comprising the MRImageStorage svkDcmtkIod.
svkDcmtkIod::svkDcmtkIod() 
{
}


/*!
 *
 */
svkDcmtkIod::~svkDcmtkIod() {   
}


/*! integer and string VRs.
 *  @throws svkDicomRunTimeError
 *  @throws overflow_error
 *  @throws svkTagNotFound
 *  @throws svkIncompatibleVR
 */
void svkDcmtkIod::setValue(const DcmTag &tag, const int value)
    throw (overflow_error, svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR)
{
    svkDcmtkUtils::setValue(this->getItem(), tag, value);
}


/*! applies to float32, float64 and string VRs.
 *  @throws svkDicomRunTimeError
 *  @throws svkTagNotFound
 *  @throws IncomPatibleVR
 */
void svkDcmtkIod::setValue(const DcmTag &tag, const float value)
    throw (svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR)
{
    svkDcmtkUtils::setValue(this->getItem(), tag, value);
}


/*! applies to float32, float64 and string VRs.
 *  for float32 overflow is checked.
 *  @throws svkDicomRunTimeError
 *  @throws overflow_error
 *  @throws svkTagNotFound
 *  @throws IncomPatibleVR
 */
void svkDcmtkIod::setValue(const DcmTag &tag, const double value)
    throw (overflow_error, svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR)
{
    svkDcmtkUtils::setValue(this->getItem(), tag, value);
}


/*! applies to practically all non-numerical VRs.
 *  @throws svkDicomRunTimeError
 *  @throws svkTagNotFound
 *  @throws IncomPatibleVR
 */
void svkDcmtkIod::setValue(const DcmTag &tag, const string value, bool meta_header_value)
    throw (svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR)
{
    svkDcmtkUtils::setValue(this->getItem(meta_header_value), tag, value);
}


/*! applies to values, represented as short, int, integer string.
 *  @throws svkDicomRunTimeError
 *  @throws svkTagNotFound
 *  @throws svkIncompatibleVR
 */
int svkDcmtkIod::getIntValue(const DcmTagKey &tag) 
    throw (svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR)
{
    return svkDcmtkUtils::getIntValue(this->getItem(), tag);
}


/*! applies to values, represented as short, int, integer string,
 *  float, double, decimal string.
 *  @throws svkDicomRunTimeError
 *  @throws overflow_error
 *  @throws svkTagNotFound
 *  @throws svkIncompatibleVR
 */
float svkDcmtkIod::getFloatValue(const DcmTagKey &tag, int pos)
    throw (overflow_error, svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR)
{
    return svkDcmtkUtils::getFloatValue(this->getItem(), tag, pos);
}


/*! applies to values, represented as short, int, integer string,
 *  float, double, decimal string.
 *  @throws svkDicomRunTimeError
 *  @throws svkTagNotFound
 *  @throws svkIncompatibleVR
 */
double svkDcmtkIod::getDoubleValue(const DcmTagKey &tag, bool searchInto) 
    throw (svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR)
{
    return svkDcmtkUtils::getDoubleValue(this->getItem(), tag, searchInto);
}


/*! applies to practically every dicom vr.
 *  @throws svkDicomRunTimeError
 *  @throws svkTagNotFound
 */
string svkDcmtkIod::getStringValue(const DcmTagKey &tag, int pos)
    throw (svkDicomRunTimeError, svkTagNotFound)
{
    return svkDcmtkUtils::getStringValue(this->getItem(), tag, pos);
}


/*! applies to practically every dicom vr.
 *  if vm>1 returns all values, separated by backslashes
 *  @throws svkDicomRunTimeError
 *  @throws svkTagNotFound
 */
string svkDcmtkIod::getStringValue(const DcmTagKey &tag)
    throw (svkDicomRunTimeError, svkTagNotFound)
{
    return svkDcmtkUtils::getStringValue(this->getItem(), tag);
}


/*!
 *  Returns the base clase type for svkDcmtkUtils to operate on (element getters/setters).
 */
DcmItem* svkDcmtkIod::getItem(bool meta_header_value)
{
    if (meta_header_value) {
        return dynamic_cast<DcmItem*>( 
             this->getMetaInfo()  
        );
    } else {
        return dynamic_cast<DcmItem*>( 
             this->getDataset()  
        );
    }
}


