/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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


#include <svkEnhancedIOD.h>


using namespace svk;


vtkCxxRevisionMacro(svkEnhancedIOD, "$Rev$");


/*!
 *
 */
svkEnhancedIOD::svkEnhancedIOD()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif
    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );
}


/*!
 *
 */
svkEnhancedIOD::~svkEnhancedIOD()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}


/*!
 *
 */
void svkEnhancedIOD::InitFrameAnatomyMacro()
{
    this->dcmHeader->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "FrameAnatomySequence"
    );

    this->dcmHeader->AddSequenceItemElement(
        "FrameAnatomySequence",
        0,
        "FrameLaterality",
        string("U"),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "FrameAnatomySequence",
        0,
        "AnatomicRegionSequence"
    );


    this->dcmHeader->AddSequenceItemElement(
        "AnatomicRegionSequence",
        0,
        "CodeValue",
        1,
        "FrameAnatomySequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "AnatomicRegionSequence",
        0,
        "CodingSchemeDesignator",
        0,
        "FrameAnatomySequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "AnatomicRegionSequence",
        0,
        "CodeMeaning",
        0,
        "FrameAnatomySequence",
        0
    );
}


/*!
 *
 */
void svkEnhancedIOD::InitMRTimingAndRelatedParametersMacro(float tr, float flipAngle, int numEchoes)
{

    this->dcmHeader->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRTimingAndRelatedParametersSequence"
    );

    if ( tr == -1 ) { 
        this->dcmHeader->AddSequenceItemElement(
            "MRTimingAndRelatedParametersSequence",
            0,
            "RepetitionTime",    
            "UNKNOWN", 
            "SharedFunctionalGroupsSequence",
            0
        );
    } else {
        this->dcmHeader->AddSequenceItemElement(
            "MRTimingAndRelatedParametersSequence",
            0,
            "RepetitionTime",    
            tr, 
            "SharedFunctionalGroupsSequence",
            0
        );
    }

    if ( flipAngle == -999 ) { 
        this->dcmHeader->AddSequenceItemElement(
            "MRTimingAndRelatedParametersSequence",
            0,
            "FlipAngle",
            "UNKNOWN", 
            "SharedFunctionalGroupsSequence",
            0
        );
    } else {
        this->dcmHeader->AddSequenceItemElement(
            "MRTimingAndRelatedParametersSequence",
            0,
            "FlipAngle",
            flipAngle, 
            "SharedFunctionalGroupsSequence",
            0
        );
    }

    this->dcmHeader->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "EchoTrainLength",   
        numEchoes,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "RFEchoTrainLength", 
        1,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "GradientEchoTrainLength",
        1,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "GradientEchoTrainLength",
        1,
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *  Initializes the MREchoMacro:
 */
void svkEnhancedIOD::InitMREchoMacro(float TE)
{
    this->dcmHeader->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MREchoSequence"
    );

    this->dcmHeader->AddSequenceItemElement(
        "MREchoSequence",
        0,
        "EffectiveEchoTime",
        TE,
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *
 */
void svkEnhancedIOD::InitMRModifierMacro(float inversionTime)
{
    this->dcmHeader->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRModifierSequence"
    );

    string invRecov = "NO";
    if ( inversionTime >= 0 ) {
        invRecov.assign("YES");
        this->dcmHeader->AddSequenceItemElement(
            "MRModifierSequence",
            0,
            "InversionTimes",
            inversionTime,
            "SharedFunctionalGroupsSequence",
            0
        );
    }

    this->dcmHeader->AddSequenceItemElement(
        "MRModifierSequence",
        0,
        "InversionRecovery", 
        invRecov,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRModifierSequence",
        0,
        "SpatialPreSaturation",
        string("SLAB"),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRModifierSequence",
        0,
        "ParallelAcquisition",
        string("NO"),
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *
 */
//void svkEnhancedIOD::InitMRReceiveCoilMacro()
//{
//}


/*!
 *
 */
void svkEnhancedIOD::InitMRTransmitCoilMacro(string coilMfg, string coilName, string coilType)
{
    this->dcmHeader->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRTransmitCoilSequence"
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRTransmitCoilSequence",
        0,
        "TransmitCoilName",
        coilName,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRTransmitCoilSequence",
        0,
        "TransmitCoilManufacturerName",
        coilMfg,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRTransmitCoilSequence",
        0,
        "TransmitCoilType",    
        coilType,
        "SharedFunctionalGroupsSequence",
        0
    );

}


/*!
 *
 */
void svkEnhancedIOD::InitMRAveragesMacro(int numAverages)
{
    this->dcmHeader->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRAveragesSequence"
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRAveragesSequence",
        0,
        "NumberOfAverages",
        numAverages,
        "SharedFunctionalGroupsSequence",
        0
    );
}

