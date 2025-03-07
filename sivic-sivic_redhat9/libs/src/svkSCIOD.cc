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


#include <svkSCIOD.h>


using namespace svk;


//vtkCxxRevisionMacro(svkSCIOD, "$Rev$");
vtkStandardNewMacro(svkSCIOD);


/*!
 *
 */
svkSCIOD::svkSCIOD()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

}


/*!
 *
 */
svkSCIOD::~svkSCIOD()
{
    vtkDebugMacro(<<"svkSCIOD::~svkSCIOD");
}


/*!
 *
 */
void svkSCIOD::InitDcmHeader()
{
    this->InitPatientModule();

    this->InitGeneralStudyModule();

    this->InitGeneralSeriesModule();

    this->InitSCEquipmentModule();

    this->InitGeneralImageModule();
    this->InitImagePixelModule();
    this->InitSCImageModule();
    this->InitSOPCommonModule();
}


/*!
 *
 */
void svkSCIOD::InitSCEquipmentModule()
{
    this->dcmHeader->SetValue( "ConversionType", "WSD" );
    this->dcmHeader->SetValue( "Manufacturer", this->GetManufacturer() );
}


/*!
 *
 */
void svkSCIOD::InitSCImageModule()
{
    this->dcmHeader->InsertEmptyElement( "DateOfSecondaryCapture" );
    this->dcmHeader->InsertEmptyElement( "TimeOfSecondaryCapture" );
}


/*!
 *
 */
void svkSCIOD::InitImagePixelModule()
{

    this->Superclass::InitImagePixelModule();
    this->dcmHeader->SetValue( "BitsAllocated", 16 );
    this->dcmHeader->SetValue( "BitsStored", 16 );
    this->dcmHeader->SetValue( "HighBit", 15 );
    this->dcmHeader->SetValue( "PixelRepresentation", 0 ); //unsigned

}


/*!
 *
 */
void svkSCIOD::InitSOPCommonModule()
{
    this->dcmHeader->SetSOPClassUID( svkDcmHeader::SECONDARY_CAPTURE );
    this->dcmHeader->InsertUniqueUID( "SOPInstanceUID" );
}


/*!
 *
 */
string svkSCIOD::GetModality()
{
    return "MR"; 
} 

