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


#include <svkVarianFidMapper.h>
#include <svkVarianReader.h>

#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkTransform.h>
#include </usr/include/vtk/vtkMatrix4x4.h>
#include </usr/include/vtk/vtkByteSwap.h>
#include </usr/include/vtk/vtkCallbackCommand.h>


using namespace svk;


//vtkCxxRevisionMacro(svkVarianFidMapper, "$Rev$");


/*!
 *
 */
svkVarianFidMapper::svkVarianFidMapper()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkVarianFidMapper");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->specData = NULL;
    this->numFrames = 1;

}


/*!
 *
 */
svkVarianFidMapper::~svkVarianFidMapper()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if ( this->specData != NULL )  {
        delete [] specData;
        this->specData = NULL;
    }

}


/*!
 *  Initializes the svkDcmHeader adapter to a specific IOD type      
 *  and initizlizes the svkDcmHeader member of the svkImageData 
 *  object.    
 */
void svkVarianFidMapper::InitializeDcmHeader(map <string, vector < vector<string> > >  procparMap, 
    svkDcmHeader* header, svkMRSIOD* iod, int swapBytes) 
{
    this->procparMap = procparMap; 
    this->dcmHeader = header; 
    this->iod = iod;   
    this->swapBytes = swapBytes; 

    this->ConvertCmToMm(); 

    this->InitPatientModule();
    this->InitGeneralStudyModule();
    this->InitGeneralSeriesModule();
    this->InitGeneralEquipmentModule();

    this->InitMultiFrameFunctionalGroupsModule();
//    this->InitMultiFrameDimensionModule();
//    this->InitAcquisitionContextModule();
    this->InitMRSpectroscopyModule();
    this->InitMRSpectroscopyPulseSequenceModule();

    this->InitMRSpectroscopyDataModule();

    this->dcmHeader->SetValue( "SVK_PRIVATE_TAG",  "SVK_PRIVATE_CREATOR"); 

}


/*!
 *
 */
void svkVarianFidMapper::InitPatientModule()
{

    this->dcmHeader->InitPatientModule(
        this->dcmHeader->GetDcmPatientName( this->GetHeaderValueAsString("samplename") ),
        this->GetHeaderValueAsString("dataid"), 
        this->GetHeaderValueAsString("birthday"), 
        this->GetHeaderValueAsString("gender") 
    );

}


/*!
 *
 */
void svkVarianFidMapper::InitGeneralStudyModule()
{
    string timeDate = this->GetHeaderValueAsString( "time_svfdate" ); 
    size_t delim = timeDate.find("T"); 
    string date = timeDate.substr(0, delim); 

    this->dcmHeader->InitGeneralStudyModule(
        date, 
        "",
        "",
        this->GetHeaderValueAsString("studyid_"), 
        "", 
        ""
    );

}


/*!
 *
 */
void svkVarianFidMapper::InitGeneralSeriesModule()
{
    this->dcmHeader->InitGeneralSeriesModule(
        "0", 
        "Varian FID Data", 
        this->GetDcmPatientPositionString()
    );
}


/*!
 *  DDF is historically the UCSF representation of a GE raw file so
 *  initialize to svkVarianFidMapper::MFG_STRING.
 */
void svkVarianFidMapper::InitGeneralEquipmentModule()
{
    this->dcmHeader->SetValue(
        "Manufacturer",
        "Varian"
    );
}


/*!
 *
 */
void svkVarianFidMapper::InitMultiFrameFunctionalGroupsModule()
{

    this->InitSharedFunctionalGroupMacros();

    this->dcmHeader->SetValue(
        "InstanceNumber",
        1
    );

    this->dcmHeader->SetValue(
        "ContentDate",
        this->GetHeaderValueAsString( "time_svfdate" )
    );

    this->numSlices = this->GetHeaderValueAsInt("ns");
    int numEchoes = this->GetHeaderValueAsInt("ne");

    this->numFrames = this->numSlices * numEchoes;
    this->dcmHeader->SetValue(
        "NumberOfFrames",
        this->numFrames
    );

    this->InitPerFrameFunctionalGroupMacros();

}


/*!
 *
 */
void svkVarianFidMapper::InitSharedFunctionalGroupMacros()
{

    this->InitPixelMeasuresMacro();
    this->InitPlaneOrientationMacro();

    this->InitMRTimingAndRelatedParametersMacro();
    this->InitMRSpectroscopyFOVGeometryMacro();
    this->InitMREchoMacro();

    this->InitMRModifierMacro();

    this->InitMRReceiveCoilMacro();

    //this->InitMRTransmitCoilMacro();
    this->InitMRAveragesMacro();
    //this->InitMRSpatialSaturationMacro();
    //this->InitMRSpatialVelocityEncodingMacro();

}


/*!
 *  The FID toplc is the center of the first voxel.
 */
void svkVarianFidMapper::InitPerFrameFunctionalGroupMacros()
{

    double dcos[3][3];
    this->dcmHeader->SetSliceOrder( this->dataSliceOrder );
    this->dcmHeader->GetDataDcos( dcos );

    double pixelSpacing[3];
    this->dcmHeader->GetPixelSize(pixelSpacing);

    int numPixels[3];
    numPixels[0] = this->GetHeaderValueAsInt("nv", 0);
    numPixels[1] = this->GetHeaderValueAsInt("nv2", 0);
    numPixels[2] = this->GetHeaderValueAsInt("ns", 0);

    //  Get center coordinate float array from fidMap and use that to generate
    //  Displace from that coordinate by 1/2 fov - 1/2voxel to get to the center of the
    //  toplc from which the individual frame locations are calculated

    //  If volumetric 3D (not 2D), get the center of the TLC voxel in LPS coords:
    double* volumeTlcLPSFrame = new double[3];
    //  if more than 1 slice:
    if ( numPixels[2] > 1 ) {

        //  Get the volumetric center in acquisition frame coords:
        double volumeCenterAcqFrame[3];
        for (int i = 0; i < 3; i++) {
            volumeCenterAcqFrame[i] = this->GetHeaderValueAsFloat("location[]", i);
        }

        double* volumeTlcAcqFrame = new double[3];
        for (int i = 0; i < 3; i++) {
            volumeTlcAcqFrame[i] = volumeCenterAcqFrame[i]
                                 + ( this->GetHeaderValueAsFloat("span[]", i) - pixelSpacing[i] )/2;
        }
        svkVarianReader::UserToMagnet(volumeTlcAcqFrame, volumeTlcLPSFrame, dcos);
        delete [] volumeTlcAcqFrame;

    }


    //  Center of toplc (LPS) pixel in frame:
    double toplc[3];

    //
    //  If 3D vol, calculate slice position, otherwise use value encoded
    //  into slice header
    //

     //  If 2D (single slice)
     if ( numPixels[2] == 1 ) {

        //  Location is the center of the image frame in user (acquisition frame).
        double centerAcqFrame[3];
        for ( int j = 0; j < 3; j++) {
            centerAcqFrame[j] = 0.0;
        }

        //  Now get the center of the tlc voxel in the acq frame:
        double* tlcAcqFrame = new double[3];
        for (int j = 0; j < 2; j++) {
            tlcAcqFrame[j] = centerAcqFrame[j]
                - ( ( numPixels[j] * pixelSpacing[j] ) - pixelSpacing[j] )/2;
        }
        tlcAcqFrame[2] = centerAcqFrame[2];

        //  and convert to LPS (magnet) frame:
        svkVarianReader::UserToMagnet(tlcAcqFrame, toplc, dcos);
    
        delete [] tlcAcqFrame;
    
    } else {

        for(int j = 0; j < 3; j++) { //L, P, S
            toplc[j] = volumeTlcLPSFrame[j]; 
        }
    
   }


    svkDcmHeader::DimensionVector dimensionVector = this->dcmHeader->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, this->numFrames-1);

    this->dcmHeader->InitPerFrameFunctionalGroupSequence(
        toplc, pixelSpacing, dcos, &dimensionVector
    );

    delete volumeTlcLPSFrame;
}


/*!
 *  The DICOM PlaneOrientationSequence is set from orientational params defined in the 
 *  Varian procpar file.  According to the VNMR User Programming documentation available at
 *  http://www.varianinc.com/cgi-bin/nav?varinc/docs/products/nmr/apps/pubs/sys_vn&cid=975JIOILOKPQNGMKIINKNGK&zsb=1060363007.usergroup 
 *  (VNMR 6.1C, Pub No. 01-999165-00, Rev B0802, page 155), the Euler angles are 
 *  defined in  the "User Guide Imaging.  The Varian User Guide: Imaging 
 *  (Pub. No. 01-999163-00, Rev. A0201, page 272) provides the following definition
 *  of the procpar euler angles: 
 * 
 *  "Arguments: phi, psi, theta are the coordinates of a point in the logical imaging
 *   reference frame (the coordinate system deﬁned by the readout, phase encode,
 *   and slice select axes) and the Euler angles that deﬁne the orientation of the
 *   logical frame:
 *      • phi is the angular rotation of the image plane about a line normal to the
 *        image plane.
 *      • psi is formed by the projection of a line normal to the imaging plane onto
 *        the magnet XY plane, and the magnet Y axis.
 *      • theta is formed by the line normal to the imaging plane, and the magnet
 *        Z axis."
 *
 *  i.e. 
 *      axial(phi, psi, theta) => 0,  0,  0
 *      cor  (phi, psi, theta) => 0,  0, 90
 *      sag  (phi, psi, theta) => 0, 90, 90
 *
 */
void svkVarianFidMapper::InitPlaneOrientationMacro()
{

    this->dcmHeader->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PlaneOrientationSequence"
    );

    //  Get the euler angles for the acquisition coordinate system:
    float psi = this->GetHeaderValueAsFloat("psi", 0);
    float phi = this->GetHeaderValueAsFloat("phi", 0);
    float theta = this->GetHeaderValueAsFloat("theta", 0);

    vtkTransform* eulerTransform = vtkTransform::New();
    eulerTransform->RotateX( theta);
    eulerTransform->RotateY( phi );
    eulerTransform->RotateZ( psi );
    vtkMatrix4x4* dcos = vtkMatrix4x4::New();
    eulerTransform->GetMatrix(dcos);

    if (this->GetDebug()) {
        cout << *dcos << endl;
    }

    //  and analagous to the fdf reader, convert from LAI to LPS: 
    //dcos->SetElement(0, 0, dcos->GetElement(0, 0)  * 1 );
    //dcos->SetElement(0, 1, dcos->GetElement(0, 1)  * -1);
    //dcos->SetElement(0, 2, dcos->GetElement(0, 2)  * -1);

    //dcos->SetElement(1, 0, dcos->GetElement(1, 0)  * 1 );
    //dcos->SetElement(1, 1, dcos->GetElement(1, 1)  * -1);
    //dcos->SetElement(1, 2, dcos->GetElement(1, 2)  * -1);

    //dcos->SetElement(2, 0, dcos->GetElement(2, 0)  * 1 );
    //dcos->SetElement(2, 1, dcos->GetElement(2, 1)  * -1);
    //dcos->SetElement(2, 2, dcos->GetElement(2, 2)  * -1);
    

    string orientationString;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            ostringstream dcosOss;
            dcosOss.setf(ios::fixed);
            dcosOss << dcos->GetElement(i, j);
            orientationString.append( dcosOss.str() );
            if (i != 1 || j != 2  ) {
                orientationString.append( "\\");
            }
        }
    }

    this->dcmHeader->AddSequenceItemElement(
        "PlaneOrientationSequence",
        0,
        "ImageOrientationPatient",
        orientationString,
        "SharedFunctionalGroupsSequence",
        0
    );

    //  Determine whether the data is ordered with or against the slice normal direction.
    double normal[3];
    this->dcmHeader->GetNormalVector(normal);

    double dcosSliceOrder[3];
    for (int j = 0; j < 3; j++) {
        dcosSliceOrder[j] = dcos->GetElement(2, j);
    }

    //  Use the scalar product to determine whether the data in the .cmplx
    //  file is ordered along the slice normal or antiparalle to it.
    vtkMath* math = vtkMath::New();
    if (math->Dot(normal, dcosSliceOrder) > 0 ) {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
    } else {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_NEG_NORMAL;
    }
}


/*!
 *
 */
void svkVarianFidMapper::InitMRTimingAndRelatedParametersMacro()
{
    this->dcmHeader->InitMRTimingAndRelatedParametersMacro(
        this->GetHeaderValueAsFloat( "tr" ),
        this->GetHeaderValueAsFloat("fliplist", 0)
    ); 
}


/*!
 *
 */
void svkVarianFidMapper::InitMRSpectroscopyFOVGeometryMacro()
{

    this->dcmHeader->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRSpectroscopyFOVGeometrySequence"
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionDataColumns",
        this->GetHeaderValueAsInt("np", 0)/2,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionPhaseColumns",
        this->GetHeaderValueAsInt("nv", 0),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionPhaseRows",
        this->GetHeaderValueAsInt("nv2", 0),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionOutOfPlanePhaseSteps",
        this->GetHeaderValueAsInt("ns", 0),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionTLC",
        "0\\0\\0",
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionPixelSpacing",
        this->GetHeaderValueAsString("vox1", 0) + '\\' + this->GetHeaderValueAsString("vox2", 0),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionSliceThickness",
        this->GetHeaderValueAsFloat("vox3", 0),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionOrientation",
        "0\\0\\0\\0\\0\\0\\0\\0\\0",
        "SharedFunctionalGroupsSequence",
        0
    );


    // ==================================================
    //  Reordered Params
    // ==================================================
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedPhaseColumns",
        this->GetHeaderValueAsInt("nv", 0),
        "SharedFunctionalGroupsSequence",
        0
    );
    
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                               
        "SVK_SpectroscopyAcqReorderedPhaseRows",
        this->GetHeaderValueAsInt("nv2", 0),
        "SharedFunctionalGroupsSequence",
        0
    );
    
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                               
        "SVK_SpectroscopyAcqReorderedOutOfPlanePhaseSteps",
        this->GetHeaderValueAsInt("ns", 0), 
        "SharedFunctionalGroupsSequence",
        0
    );
    
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,                               
        "SVK_SpectroscopyAcqReorderedTLC",
        "0\\0\\0",
        "SharedFunctionalGroupsSequence",
        0
    );
    
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                  
        "SVK_SpectroscopyAcqReorderedPixelSpacing",
        this->GetHeaderValueAsString("vox1", 0) + '\\' + this->GetHeaderValueAsString("vox2", 0),
        "SharedFunctionalGroupsSequence",
        0
    );
    
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,                                      
        "SVK_SpectroscopyAcqReorderedSliceThickness",
        this->GetHeaderValueAsFloat("vox3", 0), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedOrientation",
        "0\\0\\0\\0\\0\\0\\0\\0\\0",
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "PercentSampling",
        1,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "PercentPhaseFieldOfView",
        1,
        "SharedFunctionalGroupsSequence",
        0
    );

}


/*!
 *
 */
void svkVarianFidMapper::InitMREchoMacro()
{
    this->dcmHeader->InitMREchoMacro( this->GetHeaderValueAsFloat( "te" ) * 1000. );
}


/*!
 *  Override in concrete mapper for specific acquisitino
 */
void svkVarianFidMapper::InitMRModifierMacro()
{
    float inversionTime = 0; 
    this->dcmHeader->InitMRModifierMacro( inversionTime );
}


/*!
 *
 */
void svkVarianFidMapper::InitMRTransmitCoilMacro()
{
    this->dcmHeader->InitMRTransmitCoilMacro("Varian", "UNKNOWN", "BODY");
}


/*! 
 *  Receive Coil:
 */
void svkVarianFidMapper::InitMRReceiveCoilMacro()
{

    this->dcmHeader->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRReceiveCoilSequence"
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "ReceiveCoilName",
        "Varian Coil",
        "SharedFunctionalGroupsSequence",
        0
    );

}


/*!
 *
 */
void svkVarianFidMapper::InitMRAveragesMacro()
{
    int numAverages = 1; 
    this->dcmHeader->InitMRAveragesMacro(numAverages);
}


/*!
 *
 */
void svkVarianFidMapper::InitMultiFrameDimensionModule()
{
    int indexCount = 0; 
    this->dcmHeader->AddSequenceItemElement(
        "DimensionIndexSequence",
        indexCount,
        "DimensionDescriptionLabel",
        "Slice"
    );

/*
    if (this->GetNumTimePoints() > 1) {
        indexCount++; 
        this->dcmHeader->AddSequenceItemElement(
            "DimensionIndexSequence",
            indexCount,
            "DimensionDescriptionLabel",
            "Time Point"
        );
    }
*/

//      if (this->GetNumCoils() > 1) {
//          indexCount++; 
//          this->dcmHeader->AddSequenceItemElement(
//          "DimensionIndexSequence",
//          indexCount,
//          "DimensionDescriptionLabel",
//          "Coil Number"
//      );

//
//        this->dmHeader()->AddSequenceItemElement(
//            "DimensionIndexSequence",
//            1,
//            "DimensionIndexPointer",
//            "18H\\00H\\47H\\90"
//        );
//        this->dcmHeader->AddSequenceItemElement(
//            "DimensionIndexSequence",
//            1,
//            "FunctionalGroupPointer",
//            //"MultiCoilDefinitionSequence"
//            "18H\\00H\\47H\\90"
//        );
//  }

}


/*!
 *
 */
void svkVarianFidMapper::InitAcquisitionContextModule()
{
}


/*!
 *
 */
void svkVarianFidMapper::InitMRSpectroscopyModule()
{

    /*  =======================================
     *  MR Image and Spectroscopy Instance Macro
     *  ======================================= */

    this->dcmHeader->SetValue(
        "AcquisitionDateTime",
        this->GetHeaderValueAsString("date")
    );
    
    this->dcmHeader->SetValue(
        "AcquisitionDuration",
        0
    );


    string nucleus = this->GetHeaderValueAsString("tn"); 
    string dicomNucleus = "1H"; 
    if ( nucleus.compare("C13") == 0 ) {
        dicomNucleus = "13C"; 
    } else if ( nucleus.compare("N15") == 0 ) {
        dicomNucleus = "15N"; 
    }


    this->dcmHeader->SetValue(
        "ResonantNucleus",
        dicomNucleus 
    );

    this->dcmHeader->SetValue(
        "KSpaceFiltering",
        "NONE"
    );

    this->dcmHeader->SetValue(
        "ApplicableSafetyStandardAgency",
        "Research"
    );

    //  B0 in Gauss?
    this->dcmHeader->SetValue(
        "MagneticFieldStrength",
        static_cast< int > ( this->GetHeaderValueAsFloat("B0") / 10000 )
    );
    /*  =======================================
     *  END: MR Image and Spectroscopy Instance Macro
     *  ======================================= */

    this->dcmHeader->SetValue(
        "ImageType",
        string("ORIGINAL\\PRIMARY\\SPECTROSCOPY\\NONE")
    );


    /*  =======================================
     *  Spectroscopy Description Macro
     *  ======================================= */
    this->dcmHeader->SetValue(
        "VolumetricProperties",
        string("VOLUME")
    );

    this->dcmHeader->SetValue(
        "VolumeBasedCalculationTechnique",
        string("NONE")
    );

    this->dcmHeader->SetValue(
        "ComplexImageComponent",
        string("COMPLEX")
    );

    this->dcmHeader->SetValue(
        "AcquisitionContrast",
        "UNKNOWN"
    );  
    /*  =======================================
     *  END: Spectroscopy Description Macro
     *  ======================================= */


    this->dcmHeader->SetValue(
        "TransmitterFrequency",
        this->GetHeaderValueAsFloat( "sfrq" )
    );

    this->dcmHeader->SetValue(
        "SpectralWidth",
        this->GetHeaderValueAsFloat( "sw" )
    );

    this->dcmHeader->SetValue(
        "SVK_FrequencyOffset",
        0
    );

    //  sp is the frequency in Hz at left side (downfield/High freq) 
    //  side of spectrum: 
    //
    float ppmRef = this->GetHeaderValueAsFloat( "sp" ) + this->GetHeaderValueAsFloat( "sw" )/2.;
    ppmRef /= this->GetHeaderValueAsFloat( "sfrq" ); 
    this->dcmHeader->SetValue(
        "ChemicalShiftReference",
        ppmRef 
    );

    this->dcmHeader->SetValue(
        "VolumeLocalizationTechnique",
        ""
    );


    this->dcmHeader->SetValue(
        "Decoupling",
        "NO"
    );

    this->dcmHeader->SetValue(
        "TimeDomainFiltering",
        "NONE"
    );

    this->dcmHeader->SetValue(
        "NumberOfZeroFills",
        0
    );

    this->dcmHeader->SetValue(
        "BaselineCorrection",
        string("NONE")
    );

    this->dcmHeader->SetValue(
        "FrequencyCorrection",
        "NO"
    );

    this->dcmHeader->SetValue(
        "FirstOrderPhaseCorrection",
        string("NO")
    );


    this->dcmHeader->SetValue(
        "WaterReferencedPhaseCorrection",
        string("NO")
    );
}


/*!
 *
 */
void svkVarianFidMapper::InitMRSpectroscopyDataModule()
{
    this->dcmHeader->SetValue( "Columns", this->GetHeaderValueAsInt("nv", 0) );
    this->dcmHeader->SetValue( "Rows", this->GetHeaderValueAsInt("nv2", 0) );
    this->dcmHeader->SetValue( "DataPointRows", 0 );
    this->dcmHeader->SetValue( "DataPointColumns", this->GetHeaderValueAsInt("np", 0)/2 );
    this->dcmHeader->SetValue( "DataRepresentation", "COMPLEX" );
    this->dcmHeader->SetValue( "SignalDomainColumns", "TIME" );
    this->dcmHeader->SetValue( "SVK_ColumnsDomain", "KSPACE" );
    this->dcmHeader->SetValue( "SVK_RowsDomain", "KSPACE" );
    this->dcmHeader->SetValue( "SVK_SliceDomain", "KSPACE" );
}


/*!
 *  Reads spec data from fid file.
 */
void svkVarianFidMapper::ReadFidFile( string fidFileName, svkImageData* data )
{
    
    vtkDebugMacro( << this->GetClassName() << "::ReadFidFile()" );

    ifstream* fidDataIn = new ifstream();
    fidDataIn->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

    int pixelWordSize = 4;
    int numComponents = 2;
    int numSpecPoints = this->dcmHeader->GetIntValue( "DataPointColumns" );

    int numVoxels[3]; 
    numVoxels[0] = this->dcmHeader->GetIntValue( "Columns" ); 
    numVoxels[1] = this->dcmHeader->GetIntValue( "Rows" ); 
    numVoxels[2] = this->dcmHeader->GetNumberOfSlices( ); 

    int numPixInVolume = numVoxels[0] * numVoxels[1] * numVoxels[2]; 

    int numBytesInVol = ( numPixInVolume * pixelWordSize * numComponents * numSpecPoints );

    fidDataIn->open( fidFileName.c_str(), ios::binary );

    /*
     *   Flatten the data volume into one dimension
     */
    if (this->specData == NULL) {
        this->specData = new float[ numBytesInVol/pixelWordSize ];
    }

    fidDataIn->seekg(0, ios::beg);
    int fileHeaderSize = 32;
    int blockHeaderSize = 28;
    fidDataIn->seekg(fileHeaderSize + blockHeaderSize, ios::beg);
    fidDataIn->read( (char *)(this->specData), numBytesInVol);

    /*
     *  FID files are bigendian.
     */
    if ( this->swapBytes ) {
        vtkByteSwap::SwapVoidRange((void *)this->specData, numBytesInVol/pixelWordSize, pixelWordSize);
    }

    svkDcmHeader* hdr = this->dcmHeader;

    double progress = 0;

    int numTimePts = hdr->GetNumberOfTimePoints(); 
    int numCoils = hdr->GetNumberOfCoils(); 
    for (int coilNum = 0; coilNum < numCoils; coilNum++) {
        for (int timePt = 0; timePt < numTimePts; timePt++) {

            for (int z = 0; z < numVoxels[2] ; z++) {
                for (int y = 0; y < numVoxels[1]; y++) {
                    for (int x = 0; x < numVoxels[0]; x++) {
                        SetCellSpectrum(data, x, y, z, timePt, coilNum);

                        if( timePt % 2 == 0 ) { // only update every other index
				            progress = (timePt * coilNum)/((double)(numTimePts*numCoils));
				            this->InvokeEvent(vtkCommand::ProgressEvent, static_cast<void *>(&progress));
        	            }
                    }
                }
            }
        }
    }

    progress = 1;
	this->InvokeEvent(vtkCommand::ProgressEvent,static_cast<void *>(&progress));

    fidDataIn->close();
    delete fidDataIn;

}


/*!
 *
 */
void svkVarianFidMapper::SetCellSpectrum(vtkImageData* data, int x, int y, int z, int timePt, int coilNum)
{

    int numComponents = 1;
    string representation =  this->dcmHeader->GetStringValue( "DataRepresentation" );
    if (representation.compare( "COMPLEX" ) == 0 ) {
        numComponents = 2;
    }
    vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
    dataArray->SetNumberOfComponents( numComponents );

    int numPts = this->dcmHeader->GetIntValue( "DataPointColumns" );
    dataArray->SetNumberOfTuples(numPts);

    char arrayName[30];
    sprintf(arrayName, "%d %d %d %d %d", x, y, z, timePt, coilNum);
    dataArray->SetName(arrayName);

    int numVoxels[3];
    numVoxels[0] = this->dcmHeader->GetIntValue( "Columns" );
    numVoxels[1] = this->dcmHeader->GetIntValue( "Rows" );
    numVoxels[2] = this->dcmHeader->GetNumberOfSlices();

    //  if cornoal, swap z and x:
    int xTmp = x; 
    x = y; 
    y = xTmp; 
    x = numVoxels[0] - x - 1; 
    y = numVoxels[1] - y - 1; 

    int offset = (numPts * numComponents) *  (
                     ( numVoxels[0] * numVoxels[1] * numVoxels[2] ) * timePt
                    +( numVoxels[0] * numVoxels[1] ) * z
                    +  numVoxels[0] * y
                    +  x
                 );


    for (int i = 0; i < numPts; i++) {
        dataArray->SetTuple(i, &(this->specData[offset + (i * 2)]));
    }

    //  Add the spectrum's dataArray to the CellData:
    //  vtkCellData is a subclass of vtkFieldData
    data->GetCellData()->AddArray(dataArray);

    dataArray->Delete();
    
    return;
}


/*!
 *  Convert FID (Procpar) spatial values from cm to mm: FOV, Center, etc. 
 */
void svkVarianFidMapper::ConvertCmToMm()
{

    float cmToMm = 10.;
    float tmp;
    ostringstream oss;

    // FOV 
    tmp = cmToMm * this->GetHeaderValueAsFloat("lpe", 0);
    oss << tmp;
    ( this->procparMap["lpe"] )[0][0] = oss.str();

    oss.str("");
    tmp = cmToMm * this->GetHeaderValueAsFloat("lpe2", 0);
    oss << tmp;
    ( this->procparMap["lpe2"] )[0][0] = oss.str();

    oss.str("");
    tmp = cmToMm * this->GetHeaderValueAsFloat("lro", 0);
    oss << tmp;
    ( this->procparMap["lro"] )[0][0] = oss.str(); 


    //  Center 
    oss.str("");
    tmp = cmToMm * this->GetHeaderValueAsFloat("ppe", 0);
    oss << tmp;
    ( this->procparMap["ppe"] )[0][0] = oss.str();

    oss.str("");
    tmp = cmToMm * this->GetHeaderValueAsFloat("ppe2", 0);
    oss << tmp;
    ( this->procparMap["ppe2"] )[0][0] = oss.str();

    oss.str("");
    tmp = cmToMm * this->GetHeaderValueAsFloat("pro", 0);
    oss << tmp;
    ( this->procparMap["pro"] )[0][0] = oss.str();

}


/*!
 *  Use the Procpar patient position string to set the DCM_PatientPosition data element.
 */
string svkVarianFidMapper::GetDcmPatientPositionString()
{
    string dcmPatientPosition;

    string position1 = this->GetHeaderValueAsString("position1", 0);
    if( position1.find("head first") != string::npos ) {
        dcmPatientPosition.assign("HF");
    } else if( position1.find("feet first") != string::npos ) {
        dcmPatientPosition.assign("FF");
    } else {
        dcmPatientPosition.assign("UNKNOWN");
    }

    string position2 = this->GetHeaderValueAsString("position2", 0);
    if( position2.find("supine") != string::npos ) {
        dcmPatientPosition += "S";
    } else if( position2.find("prone") != string::npos ) {
        dcmPatientPosition += "P";
    } else if( position2.find("decubitus left") != string::npos ) {
        dcmPatientPosition += "DL";
    } else if( position2.find("decubitus right") != string::npos ) {
        dcmPatientPosition += "DR";
    } else {
        dcmPatientPosition += "UNKNOWN";
    }

    return dcmPatientPosition;
}


/*!
 *
 */
int svkVarianFidMapper::GetHeaderValueAsInt(string keyString, int valueIndex, int procparRow)
{

    istringstream* iss = new istringstream();
    int value;

    iss->str( (this->procparMap[keyString])[procparRow][valueIndex]);
    *iss >> value;

    delete iss; 

    return value;
}


/*!
 *
 */
float svkVarianFidMapper::GetHeaderValueAsFloat(string keyString, int valueIndex, int procparRow)
{

    istringstream* iss = new istringstream();
    float value;
    iss->str( (this->procparMap[keyString])[procparRow][valueIndex]);
    *iss >> value;

    delete iss; 

    return value;
}


/*!
 *  Check to see if a key exists in the header (procparMap)
 */
bool svkVarianFidMapper::HeaderFieldExists(string keyString)
{
    bool exists = false; 
    if ( this->procparMap.find( keyString ) != this->procparMap.end() ) {
        exists = true; 
    }
    return exists; 
}


/*!
 *
 */
string svkVarianFidMapper::GetHeaderValueAsString(string keyString, int valueIndex, int procparRow)
{
    return (this->procparMap[keyString])[procparRow][valueIndex];
}



/*!
 *  Pixel Spacing:
 */
void svkVarianFidMapper::InitPixelMeasuresMacro()
{
    float numPixels[3];
    numPixels[0] = this->GetHeaderValueAsInt("nv", 0);
    numPixels[1] = this->GetHeaderValueAsInt("nv2", 0);
    numPixels[2] = this->GetHeaderValueAsInt("ns", 0);

    //  Not sure if this is best, also see lpe (phase encode resolution in cm)
    float pixelSize[3];
    pixelSize[0] = this->GetHeaderValueAsFloat("vox1", 0);
    pixelSize[1] = this->GetHeaderValueAsFloat("vox2", 0);
    pixelSize[2] = this->GetHeaderValueAsFloat("vox3", 0);

    string pixelSizeString[3];

    for (int i = 0; i < 3; i++) {
        ostringstream oss;
        oss << pixelSize[i];
        pixelSizeString[i].assign( oss.str() );
    }

    this->dcmHeader->InitPixelMeasuresMacro(
        pixelSizeString[0] + "\\" + pixelSizeString[1],
        pixelSizeString[2]
    );
}

