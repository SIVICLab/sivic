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



#include <svkEnhancedMRIIOD.h>


using namespace svk;


//vtkCxxRevisionMacro(svkEnhancedMRIIOD, "$Rev$");
vtkStandardNewMacro(svkEnhancedMRIIOD);


/*!
 *
 */
svkEnhancedMRIIOD::svkEnhancedMRIIOD()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif
    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

}


/*!
 *
 */
svkEnhancedMRIIOD::~svkEnhancedMRIIOD()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}


/*!
 *
 */
void svkEnhancedMRIIOD::InitDcmHeader()
{

    //  Patient IE
    this->InitPatientModule();

    //  Study IE
    this->InitGeneralStudyModule();

    //  Series IE
    this->InitGeneralSeriesModule();
    this->InitMRSeriesModule();

    //  FrameOfReference IE
    this->InitFrameOfReferenceModule();

    //  Equipment IE
    this->InitGeneralEquipmentModule();
    this->InitEnhancedGeneralEquipmentModule();

    //  Image IE
    this->InitImagePixelModule();
    this->InitMultiFrameFunctionalGroupsModule();
    this->InitFrameAnatomyMacro(); 
    this->dcmHeader->InitMRAveragesMacro(); 
    this->InitMRImageFrameTypeMacro(); 
    this->dcmHeader->InitMRTimingAndRelatedParametersMacro(); 
    this->dcmHeader->InitMREchoMacro(); 
    this->dcmHeader->InitMRModifierMacro(); 
    this->dcmHeader->InitMRTransmitCoilMacro(); 
    this->dcmHeader->InitPixelValueTransformationMacro();
    this->InitMultiFrameDimensionModule();
    this->InitAcquisitionContextModule();
    this->InitEnhancedMRImageModule();
    this->InitSOPCommonModule();

}


/*!
 *
 */
void svkEnhancedMRIIOD::InitEnhancedMRImageModule()
{
}


/*!
 *
 */
void svkEnhancedMRIIOD::InitMRImageFrameTypeMacro()
{
    this->dcmHeader->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRImageFrameTypeSequence"
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRImageFrameTypeSequence",
        0,
        "FrameType",
        string("ORIGINAL\\PRIMARY\\VOLUME\\NONE"),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRImageFrameTypeSequence",
        0,
        "ComplexImageComponent",
        string("MAGNITUDE"),
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *  Initializes the SOP Class UID
 */
void svkEnhancedMRIIOD::InitSOPCommonModule()
{
    this->dcmHeader->SetSOPClassUID( svkDcmHeader::ENHANCED_MR_IMAGE );
    this->dcmHeader->InsertUniqueUID( "SOPInstanceUID" );
}

