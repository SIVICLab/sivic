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



#include <svkDcmtkUtils.h>


using namespace svk;


/*! 
 *  integer and string VRs.
 *      @throws svkDicomRunTimeError
 *      @throws overflow_error
 *      @throws svkTagNotFound
 *      @throws svkIncompatibleVR
 */
void svkDcmtkUtils::setValue(DcmItem* item, const DcmTag &tag, const int value, const int pos)
    throw (overflow_error, svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR)
{

    stringstream ss;
    switch(tag.getEVR()) {

        case EVR_US:
            if(value > USHRT_MAX || value < 0)
                throw overflow_error("setValue(int)");
            handleError(item->putAndInsertUint16(tag, value, pos), "setValue(int)", &tag);
            break;

        case EVR_SS:
            if(value > SHRT_MAX || value < SHRT_MIN)
                throw overflow_error("setValue(int)");
            handleError(item->putAndInsertSint16(tag, value, pos), "setValue(int)", &tag);
            break;

        case EVR_UL:
            if(value < 0 || (unsigned long)value > ULONG_MAX)
                throw overflow_error("setValue(int)");
            handleError(item->putAndInsertUint32(tag, value, pos), "setValue(int)", &tag);
            break;

        case EVR_SL:
            handleError(item->putAndInsertSint32(tag, value, pos), "setValue(int)", &tag);
            break;
    
        case EVR_FL:
        case EVR_OF:
            handleError(item->putAndInsertFloat32(tag, value, pos), "setValue(int)", &tag);
            break;

        case EVR_FD:
            handleError(item->putAndInsertFloat64(tag, (Float64)value, pos), "setValue(int)", &tag);
            break;

        case EVR_AS:
        case EVR_CS:
        case EVR_DS:
        case EVR_IS:
        case EVR_LO:
        case EVR_LT:
        case EVR_UT:
        case EVR_SH:
        case EVR_ST:
            ss<<value;
            handleError(item->putAndInsertString(tag, ss.str().c_str()), "setValue(int)", &tag);
            break;

        default:
            throw svkIncompatibleVR(string("setValue(int): ") +
                            tag.getVR().getVRName());
    }
}


/*! applies to float32, float64 and string VRs.
 *  @throws svkDicomRunTimeError
 *  @throws svkTagNotFound
 *  @throws IncomPatibleVR
 */
void svkDcmtkUtils::setValue(DcmItem* item, const DcmTag &tag, const long int value)
    throw (svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR)
{

    stringstream ss;
    switch(tag.getEVR()) {
        case EVR_FL:
            if( value > LONG_MAX || value < LONG_MIN )
                throw overflow_error("setValue(long int)");
            handleError(item->putAndInsertUint16(tag, value), "setValue(long)", &tag);
            break;

        case EVR_OF:
            if( value > LONG_MAX || value < LONG_MIN )
                throw overflow_error("setValue(long int)");
            handleError(item->putAndInsertFloat32(tag,value), "setValue(long)", &tag);
            break;

        case EVR_FD:
            handleError(item->putAndInsertFloat64(tag,(Float64)value), "setValue(long)", &tag);
            break;

        case EVR_AS:
        case EVR_CS:
        case EVR_DS:
        case EVR_IS:
        case EVR_LO:
        case EVR_LT:
        case EVR_UT:
        case EVR_SH:
        case EVR_ST:
            ss<<fixed<<setprecision(15)<<value;
            handleError(item->putAndInsertString(tag,ss.str().c_str()), "setValue(long)", &tag);
            break;
    
        default:
            throw svkIncompatibleVR(string("setValue(long): ") +
                            tag.getVR().getVRName());
    }
}


/*! applies to float32, float64 and string VRs.
 *  @throws svkDicomRunTimeError
 *  @throws svkTagNotFound
 *  @throws IncomPatibleVR
 */
void svkDcmtkUtils::setValue(DcmItem* item, const DcmTag &tag, const float value)
    throw (svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR)
{

    stringstream ss;
    switch(tag.getEVR()) {
        case EVR_FL:
        case EVR_OF:
            handleError(item->putAndInsertFloat32(tag,value), "setValue(float)", &tag);
            break;

        case EVR_FD:
            handleError(item->putAndInsertFloat64(tag,(Float64)value), "setValue(float)", &tag);
            break;

        case EVR_AS:
        case EVR_CS:
        case EVR_DS:
        case EVR_IS:
        case EVR_LO:
        case EVR_LT:
        case EVR_UT:
        case EVR_SH:
        case EVR_ST:
            ss<<fixed<<setprecision(6)<<value;
            handleError(item->putAndInsertString(tag,ss.str().c_str()), "setValue(float)", &tag);
            break;
    
        default:
            throw svkIncompatibleVR(string("setValue(float): ") +
                            tag.getVR().getVRName());
    }
}


/*! applies to float32, float64 and string VRs.
 *  for float32 overflow is checked.
 *  @throws svkDicomRunTimeError
 *  @throws overflow_error
 *  @throws svkTagNotFound
 *  @throws IncomPatibleVR
 */
void svkDcmtkUtils::setValue(DcmItem* item, const DcmTag &tag, const double value)
    throw (overflow_error, svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR)
{

    //  When setting values, it's a good idea to enforce that VR 
    //  should be defined in the dictionary or in the tag - not
    //  to look it up in the file itself.
    //  by the way, in the geprivtag.h, the tags should probably be defined as
    //  DcmTag with VRs.
    stringstream ss;
    switch(tag.getEVR()) {
        case EVR_FL:
        case EVR_OF:
            if(value>FLT_MAX) throw overflow_error("setValue(double)");
            handleError(item->putAndInsertFloat32(tag,(Float32)value), "setValue(double)", &tag);
            break;

        case EVR_FD:
            handleError(item->putAndInsertFloat64(tag,value), "setValue(double)", &tag);
            break;

        case EVR_AS:
        case EVR_CS:
        case EVR_DS:
        case EVR_IS:
        case EVR_LO:
        case EVR_LT:
        case EVR_UT:
        case EVR_SH:
        case EVR_ST:
        	/*
        	 * Decimal Strings Limit us to 16 characters. A double in exponential
        	 * notation can take up to 6 extra characters leaving only 10 for
        	 * significant digits.
        	 */
        	ss<<setprecision(10);
            ss<<value;
            handleError(item->putAndInsertString(tag,ss.str().c_str()), "setValue(double)", &tag);
            break;

        default:
            throw svkIncompatibleVR(string("setValue(double): ") +
                            tag.getVR().getVRName());
    }
}


/*! applies to practically all non-numerical VRs.
 *  @throws svkDicomRunTimeError
 *  @throws svkTagNotFound
 *  @throws IncomPatibleVR
 */
void svkDcmtkUtils::setValue(DcmItem* item, const DcmTag &tag, const string value)
    throw (svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR)
{

    stringstream ss;
    switch(tag.getEVR()) {
        case EVR_AE:
        case EVR_AT:
        case EVR_AS:
        case EVR_CS:
        case EVR_DA:
        case EVR_DS:
        case EVR_DT:
        case EVR_FL:
        case EVR_FD:
        case EVR_IS:
        case EVR_LO:
        case EVR_LT:
        case EVR_OB:
        case EVR_OF:
        case EVR_OW:
        case EVR_PN:
        case EVR_SH:
        case EVR_ST:
        case EVR_TM:
        case EVR_UI:
        case EVR_UT:
            ss<<value;
            handleError(item->putAndInsertString(tag, value.c_str()), "setValue(string)", &tag);
            break;
    
        default:
            throw svkIncompatibleVR(string("setValue(string): ") +
                            tag.getVR().getVRName());
    }
}


/*! applies to values, represented as short, int, integer string.
 *  @throws svkDicomRunTimeError
 *  @throws svkTagNotFound
 *  @throws svkIncompatibleVR
 */
int svkDcmtkUtils::getIntValue(DcmItem* item, const DcmTagKey &tag, const int pos) 
    throw (svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR)
{

    //  Tag not const, since it may be resolved
    long int value = 0;

    //  Get VR
    DcmStack stack;
    handleError(item->search(tag, stack), "getIntValue");
    DcmElement *elem = (DcmElement *)stack.top();
    if (elem == NULL) throw svkTagNotFound(tag, "getIntValue: ");                                                                       
    switch (elem->ident()){
        case EVR_UL:
        case EVR_up:
        case EVR_SL:
        case EVR_IS:
        case EVR_US:
        case EVR_xs:
        case EVR_SS:
            handleError(item->findAndGetLongInt(tag, value, pos),"getIntValue", &tag);
            break;
        default:
            throw svkIncompatibleVR(string("getIntValue: ") +
                DcmVR(elem->ident()).getVRName());
    }
    return value;

}


/*! applies to values, represented as short, int, integer string.
 *  @throws svkDicomRunTimeError
 *  @throws svkTagNotFound
 *  @throws svkIncompatibleVR
 */
long int svkDcmtkUtils::getLongIntValue(DcmItem* item, const DcmTagKey &tag, const int pos) 
    throw (svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR)
{

    //  Tag not const, since it may be resolved
    long int value = 0;

    //  Get VR
    DcmStack stack;
    handleError(item->search(tag, stack), "getIntValue");
    DcmElement *elem = (DcmElement *)stack.top();
    if (elem == NULL) throw svkTagNotFound(tag, "getIntValue: ");                                                                       
    switch (elem->ident()){
        case EVR_UL:
        case EVR_up:
        case EVR_SL:
        case EVR_IS:
        case EVR_US:
        case EVR_xs:
        case EVR_SS:
            handleError(item->findAndGetLongInt(tag, value, pos),"getLongIntValue", &tag);
            break;
        default:
            throw svkIncompatibleVR(string("getLongIntValue: ") +
                DcmVR(elem->ident()).getVRName());
    }
    return value;

}

/*! applies to values, represented as short, int, integer string,
 *  float, double, decimal string.
 *  @throws svkDicomRunTimeError
 *  @throws overflow_error
 *  @throws svkTagNotFound
 *  @throws svkIncompatibleVR
 */
float svkDcmtkUtils::getFloatValue(DcmItem* item, const DcmTagKey &tag, int pos)
    throw (overflow_error, svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR)
{

    OFCondition status;

    double double_value;
    float float_value;
    long int int_value;

    //  Get VR
    DcmStack stack;
    handleError(item->search(tag, stack), "getFloatValue", &tag);
    DcmElement *elem = (DcmElement *)stack.top();
    if (elem == NULL) throw svkTagNotFound(tag, "getFloatValue: ");                                                                     
    switch (elem->ident()){
        case EVR_FL:
        case EVR_OF:
            handleError(item->findAndGetFloat32(tag, float_value, pos),"getFloatValue", &tag);
            return float_value;
            break;

        case EVR_DS:
        case EVR_FD:
            handleError(item->findAndGetFloat64(tag, double_value, pos),"getFloatValue", &tag);
            if(double_value>FLT_MAX)
                throw overflow_error("getFloatValue");
            return (float)double_value;
            break;

        case EVR_UL:
        case EVR_up:
        case EVR_SL:
        case EVR_IS:
        case EVR_US:
        case EVR_xs:
        case EVR_SS:
            handleError(item->findAndGetLongInt(tag, int_value, pos),"getFloatValue", &tag);
            return (float) int_value;
            break;
        default:
            throw svkIncompatibleVR(string("getFloatValue: ") +
                DcmVR(elem->ident()).getVRName());
    }
}


/*! applies to values, represented as short, int, integer string,
 *  float, double, decimal string.
 *  @throws svkDicomRunTimeError
 *  @throws svkTagNotFound
 *  @throws svkIncompatibleVR
 */
double svkDcmtkUtils::getDoubleValue(DcmItem* item, const DcmTagKey &tag, bool searchInto) 
    throw (svkDicomRunTimeError, svkTagNotFound, svkIncompatibleVR)
{

    OFCondition status;

    double double_value;
    float float_value;
    long int int_value;

    //  Get VR
    DcmStack stack;
    handleError(item->search(tag, stack), "getDoubleValue", &tag);
    DcmElement *elem = (DcmElement *)stack.top();
    if (elem == NULL) throw svkTagNotFound(tag, "getDoubleValue: ");                                                                    
    switch (elem->ident()){
        case EVR_FL:
        case EVR_OF:
            if ( searchInto == true ) {
                handleError(item->findAndGetFloat32(tag, float_value, 0, OFTrue),"getDoubleValue", &tag);
            } else {
                handleError(item->findAndGetFloat32(tag, float_value),"getDoubleValue", &tag);
            }
            return (double) float_value;
            break;

        case EVR_DS:
        case EVR_FD:
            if ( searchInto == true ) {
                handleError(item->findAndGetFloat64(tag, double_value, 0, OFTrue),"getDoubleValue", &tag);
            } else {
                handleError(item->findAndGetFloat64(tag, double_value),"getDoubleValue", &tag);
            }
            return double_value;
            break;

        case EVR_UL:
        case EVR_up:
        case EVR_SL:
        case EVR_IS:
        case EVR_US:
        case EVR_xs:
        case EVR_SS:
            if ( searchInto == true ) {
                handleError(item->findAndGetLongInt(tag, int_value, 0, OFTrue),"getDoubleValue", &tag);
            } else {
                handleError(item->findAndGetLongInt(tag, int_value),"getDoubleValue", &tag);
            }
            return (double) int_value;
            break;
        default:
            throw svkIncompatibleVR(string("getDoubleValue: ") +
                DcmVR(elem->ident()).getVRName());
    }

}


/*! applies to practically every dicom vr.
 *  @throws svkDicomRunTimeError
 *  @throws svkTagNotFound
 */
string svkDcmtkUtils::getStringValue(DcmItem* item, const DcmTagKey &tag, int pos)
    throw (svkDicomRunTimeError, svkTagNotFound)
{
    OFString s;
    //  Virtually everything can be returned as string.
    //  however, a missing value should not lead to an error - just to an empty string
    OFCondition cond = item->findAndGetOFString(tag,s,pos);
    if(cond==EC_IllegalParameter)
        return string("");
    handleError(cond, "getStringValue", &tag);
    return string(s.c_str());
}


/*! applies to practically every dicom vr.
 *  if vm>1 returns all values, separated by backslashes
 *  @throws svkDicomRunTimeError
 *  @throws svkTagNotFound
 */
string svkDcmtkUtils::getStringValue(DcmItem* item, const DcmTagKey &tag)
    throw (svkDicomRunTimeError, svkTagNotFound)
{
    OFString s;
    //  Virtually everything can be returned as string.
    //  However, a missing value should not lead to an error - just to an empty string
    OFCondition cond = item->findAndGetOFStringArray(tag,s);
    if(cond==EC_IllegalParameter)
        return string("");
    handleError(cond, "getStringValue", &tag);
    return string(s.c_str());
}


