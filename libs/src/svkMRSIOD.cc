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


#include <svkMRSIOD.h>


using namespace svk;


vtkCxxRevisionMacro(svkMRSIOD, "$Rev$");
vtkStandardNewMacro(svkMRSIOD);


/*!
 *
 */
svkMRSIOD::svkMRSIOD()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif
    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );
}


/*!
 *
 */
svkMRSIOD::~svkMRSIOD()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}


/*!
 *
 */
void svkMRSIOD::InitDcmHeader()
{
    this->InitPatientModule();

    this->InitGeneralStudyModule();

    this->InitGeneralSeriesModule();
    this->InitMRSeriesModule();

    this->InitFrameOfReferenceModule();

    this->InitGeneralEquipmentModule();
    this->InitEnhancedGeneralEquipmentModule();

    this->InitMultiFrameFunctionalGroupsModule();
    this->InitMultiFrameDimensionModule();
    this->InitAcquisitionContextModule();
    this->InitMRSpectroscopyModule();
    this->InitMRSpectroscopyPulseSequenceModule();
    this->InitMRSpectroscopyDataModule();
    this->InitSOPCommonModule();

}


/*!
 *
 */
void svkMRSIOD::InitMRSpectroscopyModule()
{
    this->dcmHeader->SetValue(
        "ContentQualification",
        "RESEARCH" 
    );
    this->dcmHeader->InsertEmptyElement( "ResonantNucleus" ); 
    this->dcmHeader->InsertEmptyElement( "MagneticFieldStrength" ); 

    this->dcmHeader->SetValue( "ImageType", "ORIGINAL\\PRIMARY\\SPECTROSCOPY\\NONE" ); 
}


/*!
 *
 */
void svkMRSIOD::InitMRSpectroscopyPulseSequenceModule()
{
}


/*!
 *
 */
void svkMRSIOD::InitMRSpectroscopyDataModule()
{
    this->dcmHeader->SetValue( "Rows", 0 );
    this->dcmHeader->SetValue( "Columns", 0 ); 
    this->dcmHeader->SetValue( "DataPointRows", 0 ); 
    this->dcmHeader->SetValue( "DataPointColumns", 0 ); 
    this->dcmHeader->InsertEmptyElement( "DataRepresentation" ); 
    this->dcmHeader->InsertEmptyElement( "SignalDomainColumns" ); 
    //this->dcmHeader->InsertEmptyElement( "SignalDomainRows" ); 
    this->dcmHeader->InsertEmptyElement( "SpectroscopyData" ); 
}


/*!
 *  Initializes the SOP Class UID
 */
void svkMRSIOD::InitSOPCommonModule()
{
    this->dcmHeader->SetSOPClassUID( svkDcmHeader::MR_SPECTROSCOPY);
    this->dcmHeader->InsertUniqueUID( "SOPInstanceUID" );
}


/*!
 *  Initializes the MREchoMacro: 
 */
void svkMRSIOD::InitMREchoMacro(float TE) 
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
void svkMRSIOD::InitMRAveragesMacro(int numAverages)
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


/*!
 *
 */
void svkMRSIOD::InitMRSpectroscopyFrameTypeMacro()
{
    this->dcmHeader->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRSpectroscopyFrameTypeSequence"
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFrameTypeSequence",
        0,
        "FrameType",
        string("ORIGINAL\\PRIMARY\\SPECTROSCOPY\\NONE"),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFrameTypeSequence",
        0,
        "VolumetricProperties",
        string("VOLUME"),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFrameTypeSequence",
        0,
        "VolumeBasedCalculationTechnique",
        string("NONE"),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFrameTypeSequence",
        0,
        "ComplexImageComponent",
        string("COMPLEX"), 
        "SharedFunctionalGroupsSequence",  
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFrameTypeSequence",
        0,
        "AcquisitionContrast",
        "UNKNOWN",
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *
 */
void svkMRSIOD::InitFrameAnatomyMacro()
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
void svkMRSIOD::InitMRTimingAndRelatedParametersMacro(float tr, float flipAngle)
{
    this->dcmHeader->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRTimingAndRelatedParametersSequence"
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "RepetitionTime",    
        tr, 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "FlipAngle",
        flipAngle, 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "EchoTrainLength",   
        1,
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
 *
 */
void svkMRSIOD::InitMRModifierMacro(float inversionTime)
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
void svkMRSIOD::InitMRTransmitCoilMacro(string coilName, string coilMfg, string coilType)
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
 *  Initializes the VolumeLocalizationSequence in the MRSpectroscopy
 *  DICOM object for PRESS excitation.
 */
void svkMRSIOD::InitVolumeLocalizationSeq(float size[3], float center[3], float dcos[3][3])
{

    this->dcmHeader->InsertEmptyElement( "VolumeLocalizationSequence" );

    string midSlabPosition;
    for (int i = 0; i < 3; i++) {
        ostringstream oss;
        oss << center[i];
        midSlabPosition += oss.str();
        if (i < 2) {
            midSlabPosition += '\\';
        }
    }

    //  Volume Localization (PRESS BOX)
    for (int i = 0; i < 3; i++) {

        this->dcmHeader->AddSequenceItemElement(
            "VolumeLocalizationSequence",
            i,
            "SlabThickness",
            size[i]
        );

        this->dcmHeader->AddSequenceItemElement(
            "VolumeLocalizationSequence",
            i,
            "MidSlabPosition",
            midSlabPosition
        );

        string slabOrientation;
        for (int j = 0; j < 3; j++) {
            ostringstream oss;
            oss << dcos[i][j];
            slabOrientation += oss.str();
            if (j < 2) {
                slabOrientation += '\\';
            }
        }
        this->dcmHeader->AddSequenceItemElement(
            "VolumeLocalizationSequence",
            i,
            "SlabOrientation",
            slabOrientation
        );

    }
}
