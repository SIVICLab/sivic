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


#include <svkVarianCSFidMapper.h>
//#include <svkVarianReader.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkTransform.h>
#include </usr/include/vtk/vtkDataArray.h>
#include </usr/include/vtk/vtkCellData.h>
#include </usr/include/vtk/vtkMatrix4x4.h>
#include <svkDcmHeader.h>
#include </usr/include/vtk/vtkByteSwap.h>


using namespace svk;


//vtkCxxRevisionMacro(svkVarianCSFidMapper, "$Rev$");
vtkStandardNewMacro(svkVarianCSFidMapper);


/*!
 *
 */
svkVarianCSFidMapper::svkVarianCSFidMapper()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkVarianCSFidMapper");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->paddedData = NULL;

    this->numXReordered = 16;
    this->numYReordered = 16;
    this->numZReordered = 16;
    this->numTReordered = 59;
    this->rectilinearData = NULL;
    this->csReorder = NULL;
    this->dadFile = NULL;

}


/*!
 *
 */
svkVarianCSFidMapper::~svkVarianCSFidMapper()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if ( this->paddedData != NULL ) {
            int lengthX  = 0;
            int lengthY  = 0;
        if( this->GetDadFile() == NULL ) {
            std::vector<int> blipVector = this->GetBlips();
            lengthX  = blipVector[0];
            lengthY  = blipVector[1];
        } else {
            lengthX = this->GetDadFile()->GetEncodedMatrixSizeDimensionValue(0);
            lengthY = this->GetDadFile()->GetEncodedMatrixSizeDimensionValue(1);
        }

        for (int y = 0; y < lengthY; y++ ) {
            for (int x = 0; x < lengthX; x++ ) {
                delete [] this->paddedData[y][x];
            }
            delete [] this->paddedData[y];
        }

        delete [] this->paddedData; 
        this->paddedData = NULL; 
    }

    if ( this->specData != NULL ) {
        delete [] this->specData; 
        this->specData = NULL; 
    }


    if ( this->rectilinearData != NULL ) {
        for (int z = 0; z < this->numZReordered; z++ ) {
            for (int y = 0; y < this->numYReordered; y++ ) {
                for (int x = 0; x < this->numXReordered; x++ ) {
                    delete [] this->rectilinearData[z][y][x];
                }
                delete [] this->rectilinearData[z][y];
            }
            delete [] this->rectilinearData[z];
        }
        delete [] this->rectilinearData;

        this->rectilinearData = NULL; 
    }
}


/*!
 *
 */
void svkVarianCSFidMapper::InitMultiFrameFunctionalGroupsModule()
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

    this->InitPerFrameFunctionalGroupMacros();

}


/*!
 *
 */
void svkVarianCSFidMapper::InitSharedFunctionalGroupMacros()
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
 *  The DICOM PlaneOrientationSequence is set from orientational params defined in the 
 *  Varian procpar file.  According to the VNMR User Programming documentation available at
 *  http://www.varianinc.com/cgi-bin/nav?varinc/docs/products/nmr/apps/pubs/sys_vn&cid=975JIOILOKPQNGMKIINKNGK&zsb=1060363007.usergroup 
 *  (VNMR 6.1C, Pub No. 01-999165-00, Rev B0802, page 155), the Euler angles are 
 *  defined in  the "User Guide Imaging.  The Varian User Guide: Imaging 
 *  (Pub. No. 01-999163-00, Rev. A0201, page 272) provides the following definition
 *  of the procpar euler angles: 
 * 
 *  For this psd, the flyback encoding represents the outer data loop (essentially the slice).
 *  This complicates the interpretation of the Varian conventions which define coronal
 *  acquisitions as those with readout along the z direction and slice selection along
 *  the y direction in the magnet frame (not relevant to mrs and flyback).  Therefore the
 *  euler angles definitions need to be redefined here for this application context.  
 *
 *  Currently only support "coronal" defined by varian to be readout along z, which for us
 *  is the most slowly varying outer loop (slice loop) or rather an axial data set.   
 *
 */
void svkVarianCSFidMapper::InitPlaneOrientationMacro()
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
    cout << *dcos << endl;

    double dcosArrayTmp[3][3]; 
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            dcosArrayTmp[i][j] = dcos->GetElement(i,j); 
        }
    }
    dcos->Delete(); 

    //Reorder since the readout represents the slice: 
    double dcosArray[3][3]; 
    dcosArray[0][0] = -1 * dcosArrayTmp[0][0]; 
    dcosArray[0][1] = -1 * dcosArrayTmp[0][1]; 
    dcosArray[0][2] = -1 * dcosArrayTmp[0][2]; 
    dcosArray[1][0] = dcosArrayTmp[2][0]; 
    dcosArray[1][1] = dcosArrayTmp[2][1]; 
    dcosArray[1][2] = dcosArrayTmp[2][2]; 
    dcosArray[2][0] = -1 * dcosArrayTmp[1][0]; 
    dcosArray[2][1] = -1 * dcosArrayTmp[1][1]; 
    dcosArray[2][2] = -1 * dcosArrayTmp[1][2]; 

    //  If feet first, swap LR, SI
    string position1 = this->GetHeaderValueAsString("position1", 0);
    if( position1.find("feet first") != string::npos ) {
        dcosArray[0][0] *=-1;
        dcosArray[1][0] *=-1;
        dcosArray[2][0] *=-1;

        dcosArray[0][2] *=-1;
        dcosArray[1][2] *=-1;
        dcosArray[2][2] *=-1;
    }


    string orientationString;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            ostringstream dcosOss;
            dcosOss.setf(ios::fixed);
            dcosOss << dcosArray[i][j];
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
        dcosSliceOrder[j] = dcosArray[2][j];
    }

    //  Use the scalar product to determine whether the data in the .cmplx
    //  file is ordered along the slice normal or antiparalle to it.
    vtkMath* math = vtkMath::New();
    if (math->Dot(normal, dcosSliceOrder) > 0 ) {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
    } else {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_NEG_NORMAL;
    }

    math->Delete(); 
    eulerTransform->Delete(); 

}



/*!
 *  This is initialized to represent the target reconstructed params.  FID toplc 
 *  is the center of the first voxel.
 */
void svkVarianCSFidMapper::InitPerFrameFunctionalGroupMacros()
{

    double dcos[3][3];
    this->dcmHeader->SetSliceOrder( this->dataSliceOrder );
    this->dcmHeader->GetDataDcos( dcos );
    double pixelSpacing[3];
    this->dcmHeader->GetPixelSize(pixelSpacing);

    //  Get center coordinate float array from fidMap and use that to generate
    //  Displace from that coordinate by 1/2 fov - 1/2voxel to get to the center of the
    //  toplc from which the individual frame locations are calculated

    //  If volumetric 3D (not 2D), get the center of the TLC voxel in Magnet coords:
    double* volumeTlcUserFrame = new double[3];
    double* volumeTlcMagnetFrame = new double[3];
    if ( this->GetHeaderValueAsInt("np", 0) / 2 > 1 ) {

        //  Get the volumetric center in acquisition frame coords.  pro, ppe, ppe2 give the offset from isocenter
        //  along acq frame (not magnet frame?). 
        double volumeCenterUserFrame[3];
        volumeCenterUserFrame[0] = this->GetHeaderValueAsFloat("ppe", 0);
        volumeCenterUserFrame[1] = this->GetHeaderValueAsFloat("ppe2", 0);
        volumeCenterUserFrame[2] = this->GetHeaderValueAsFloat("pro", 0);

        //  Regridded fov, doesn't work until we regrid, so for now use bogus FOV.  Rather it should be 
        //  spacing times num pts:
        cout << "WARNING FOV Spacing not stable until after regridding "  << endl;
        double fov[3];
        fov[0] = this->GetHeaderValueAsFloat("lpe", 0);
        fov[1] = this->GetHeaderValueAsFloat("lpe2", 0);
        fov[2] = this->GetHeaderValueAsFloat("lro", 0);

        // center to Toplc
        for (int i = 0; i < 3; i++) {
            volumeTlcUserFrame[i] = volumeCenterUserFrame[i]; 
            for (int j = 0; j < 3; j++) {
                volumeTlcUserFrame[i] -= dcos[j][i] * ( fov[j] - pixelSpacing[j] )/2;
            }
        }
    }

    this->numSlices = this->GetHeaderValueAsInt("np") / 2;
    int numEchoes = this->GetHeaderValueAsInt("ne");

    svkDcmHeader::DimensionVector dimensionVector = this->dcmHeader->GetDimensionIndexVector(); 
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, this->numSlices - 1);

    this->dcmHeader->InitPerFrameFunctionalGroupSequence(
        volumeTlcUserFrame, 
        pixelSpacing, 
        dcos, 
        &dimensionVector        
    ); 
    delete[] volumeTlcUserFrame; 
    delete[] volumeTlcMagnetFrame; 
}


/*!
 *  Pixel Spacing:
 */
void svkVarianCSFidMapper::InitPixelMeasuresMacro()
{

    //  Regrided dimensionality:
    float numPixels[3];
    numPixels[0] = this->GetHeaderValueAsInt("fullnv", 0);
    numPixels[1] = this->GetHeaderValueAsInt("fullnv2", 0);
    numPixels[2] = this->GetHeaderValueAsInt("np", 0) / 2;

    //  FOV/numVoxels afer regridding 
    float pixelSize[3];
    pixelSize[0] = this->GetHeaderValueAsFloat("lpe", 0) / numPixels[0];
    pixelSize[1] = this->GetHeaderValueAsFloat("lpe2", 0) / numPixels[1];
    pixelSize[2] = this->GetHeaderValueAsFloat("lro", 0) / numPixels[2];

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


/*!
 *
 */
void svkVarianCSFidMapper::InitMRSpectroscopyFOVGeometryMacro()
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
        this->GetHeaderValueAsInt("nv", 0),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionPhaseColumns",
        this->GetHeaderValueAsInt("nv2", 0),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionPhaseRows",
        this->GetHeaderValueAsInt("ns", 0),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionOutOfPlanePhaseSteps",
        this->GetHeaderValueAsInt("np", 0)/2,
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

    float numPixels[3];
    numPixels[0] = this->GetHeaderValueAsInt("fullnv", 0);
    numPixels[1] = this->GetHeaderValueAsInt("fullnv2", 0);
    numPixels[2] = this->GetHeaderValueAsInt("np", 0) / 2;
    float pixelSize[3];
    pixelSize[0] = this->GetHeaderValueAsFloat("lpe", 0) / numPixels[0];
    pixelSize[1] = this->GetHeaderValueAsFloat("lpe2", 0) / numPixels[1];
    pixelSize[2] = this->GetHeaderValueAsFloat("lro", 0) / numPixels[2];

    string spacingString; 
    for (int i = 0; i < 2; i++) {
        ostringstream spacingOss;
        spacingOss.setf(ios::fixed);
        spacingOss << pixelSize[i];
        spacingString.append( spacingOss.str() );
        if (i != 1) {
            spacingString.append( "\\" );
        }
    }

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionPixelSpacing",
        spacingString, 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionSliceThickness",
        this->GetHeaderValueAsFloat("lro", 0),
        "SharedFunctionalGroupsSequence",
        0
    );

    double dcos[3][3];
    this->dcmHeader->GetDataDcos( dcos );

    string orientationString; 
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            ostringstream dcosOss;
            dcosOss.setf(ios::fixed);
            dcosOss << dcos[i][j];
            orientationString.append( dcosOss.str() );
            if (i != 2 || j != 2  ) {
                orientationString.append( "\\");
            }
        }
    }

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionOrientation",
        orientationString, 
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
        this->GetHeaderValueAsInt("fullnv", 0),
        "SharedFunctionalGroupsSequence",
        0
    );
    
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                               
        "SVK_SpectroscopyAcqReorderedPhaseRows",
        this->GetHeaderValueAsInt("fullnv2", 0),
        "SharedFunctionalGroupsSequence",
        0
    );
    
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                               
        "SVK_SpectroscopyAcqReorderedOutOfPlanePhaseSteps",
        this->GetHeaderValueAsInt("np", 0)/2, 
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
        spacingString, 
        "SharedFunctionalGroupsSequence",
        0
    );
    
    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,                                      
        "SVK_SpectroscopyAcqReorderedSliceThickness",
        pixelSize[2], 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->dcmHeader->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedOrientation",
        orientationString, 
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
 *  Receive Coil:
 */
void svkVarianCSFidMapper::InitMRReceiveCoilMacro()
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
void svkVarianCSFidMapper::InitAcquisitionContextModule()
{
}


/*!
 *
 */
void svkVarianCSFidMapper::InitMRSpectroscopyModule()
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

    this->dcmHeader->SetValue(
        "ResonantNucleus",
        "13C"
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
        this->GetHeaderValueAsFloat( "swf" )
    );

    this->dcmHeader->SetValue(
        "SVK_FrequencyOffset",
        0
    );
    this->dcmHeader->SetValue(
        "ChemicalShiftReference",
        0
    );

    this->dcmHeader->SetValue(
        "VolumeLocalizationTechnique",
        ""
    );

    //if ( strcmp(ddfMap["localizationType"].c_str(), "PRESS") == 0)  {
        //this->InitVolumeLocalizationSeq();
    //}

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
void svkVarianCSFidMapper::InitMRSpectroscopyPulseSequenceModule()
{
    this->dcmHeader->SetValue(
        "PulseSequenceName",
        this->GetHeaderValueAsString( "seqfil" )
    );

    int numVoxels[3];
    numVoxels[0] = this->GetHeaderValueAsInt("nv", 0);
    numVoxels[1] = this->GetHeaderValueAsInt("nv2", 0);
    numVoxels[2] = this->GetHeaderValueAsInt("ns", 0);

    string acqType = "VOLUME";
    if (numVoxels[0] == 1 && numVoxels[1] == 1 &&  numVoxels[2] == 1) {
        acqType = "SINGLE VOXEL";
    }

    this->dcmHeader->SetValue(
        "MRSpectroscopyAcquisitionType",
        acqType
    );

    this->dcmHeader->SetValue(
        "EchoPulseSequence",
        "SPIN"
    );

    this->dcmHeader->SetValue(
        "MultipleSpinEcho",
        "NO"
    );

    this->dcmHeader->SetValue(
        "MultiPlanarExcitation",
        "NO"
    );

    this->dcmHeader->SetValue(
        "SteadyStatePulseSequence",
        "NONE"
    );

    this->dcmHeader->SetValue(
        "EchoPlanarPulseSequence",
        "NO"
    );

    this->dcmHeader->SetValue(
        "SpectrallySelectedSuppression",
        ""
    );

    this->dcmHeader->SetValue(
        "GeometryOfKSpaceTraversal",
        "RECTILINEAR"
    );

    this->dcmHeader->SetValue(
        "RectilinearPhaseEncodeReordering",
        "LINEAR"
    );

    this->dcmHeader->SetValue(
        "SegmentedKSpaceTraversal",
        "SINGLE"
    );

    this->dcmHeader->SetValue(
        "CoverageOfKSpace",
        "FULL"
    );

    this->dcmHeader->SetValue( "NumberOfKSpaceTrajectories", 1 );

    //  Assume EVEN sampling about k=0 (k0 not sampled): 
    string k0Sampled = "NO";
    this->dcmHeader->SetValue( "SVK_K0Sampled", k0Sampled);

}


/*!
 *  Current PixelData ordering:
 */
void svkVarianCSFidMapper::InitMRSpectroscopyDataModule()
{
    cout << "Warning: num points not regridded, so tlc will be incorrect, as will ImagePositionPatientValues." << endl;
    this->dcmHeader->SetValue( "Columns", this->GetHeaderValueAsInt("nv2", 0) );
    this->dcmHeader->SetValue( "Rows", this->GetHeaderValueAsInt("ns", 0) );
        //  This is the target regridded dimensionality: 
        //this->dcmHeader->SetValue( "Columns", this->GetHeaderValueAsInt("fullnv", 0) );
        //this->dcmHeader->SetValue( "Rows", this->GetHeaderValueAsInt("fullnv2", 0) );
    this->dcmHeader->SetValue( "DataPointColumns", this->GetHeaderValueAsInt("nv", 0) );
    this->dcmHeader->SetValue( "DataPointRows", 0 );
    this->dcmHeader->SetValue( "DataRepresentation", "COMPLEX" );
    this->dcmHeader->SetValue( "SignalDomainColumns", "TIME" );
    this->dcmHeader->SetValue( "SVK_ColumnsDomain", "KSPACE" );
    this->dcmHeader->SetValue( "SVK_RowsDomain", "KSPACE" );
    this->dcmHeader->SetValue( "SVK_SliceDomain", "KSPACE" );
}


/*!
 *  Reads spec data from compressed sensing fid.
 */
void svkVarianCSFidMapper::ReadFidFile( string fidFileName, svkImageData* data )
{
    this->fidFileName = fidFileName;
    vtkDebugMacro( << this->GetClassName() << "::ReadFidFile()" );

    ifstream* fidDataIn = new ifstream();
    fidDataIn->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

    int pixelWordSize = 4;
    int numComponents = 2;
    int numSpecPoints = this->dcmHeader->GetIntValue( "DataPointColumns" );

    //  These should represent the original data dimensionality (SVK_ACQ) fields: 
    int numVoxels[3]; 
    numVoxels[0] =  this->GetHeaderValueAsInt("nv2", 0);
    numVoxels[1] =  this->GetHeaderValueAsInt("ns", 0);
    numVoxels[2] =  this->GetHeaderValueAsInt("np", 0) / 2;

    int numVoxelsInVolume = numVoxels[0] * numVoxels[1] * numVoxels[2]; 

    int numBytesInVol = ( numVoxelsInVolume * pixelWordSize * numComponents * numSpecPoints );

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
        vtkByteSwap::SwapVoidRange((void *)specData, numBytesInVol/pixelWordSize, pixelWordSize);
    }

    this->ZeroPadCompressedSensingData( numBytesInVol/pixelWordSize ); 

    this->ReOrderFlyback(); 

    // Loop over reordered rectlinear dimensionality
    svkDcmHeader* hdr = this->dcmHeader;
    numVoxels[0] =  hdr->GetIntValue( "Columns" );
    numVoxels[1] =  hdr->GetIntValue( "Rows" );
    numVoxels[2] =  16; //hdr->GetNumberOfSlices(); 

    for (int coilNum = 0; coilNum < hdr->GetNumberOfCoils(); coilNum++) {
        for (int timePt = 0; timePt < hdr->GetNumberOfTimePoints(); timePt++) {
            for (int z = 0; z < numVoxels[2] ; z++) {
                for (int y = 0; y < numVoxels[1]; y++) {
                    for (int x = 0; x < numVoxels[0]; x++) {
                        SetCellSpectrum(data, x, y, z, timePt, coilNum);
                    }
                }
            }
        }
    }

    fidDataIn->close();
    delete fidDataIn;

}


/*!
 *  Reorder the input data from the fid according to the hardcoded sampling strategy and then
 *  zero pad the undersampled compressed sensing data out to a rectalinear data array. 
 *  For now the flyback phase encodes are still interleaved with the frequency doman data points.
 *  This gets separated out in sample_epsi (temporary). 
 *  This method also resets the data set dimensionality in the DICOM header: (Columns, Rows, etc.) 
 */
void svkVarianCSFidMapper::ZeroPadCompressedSensingData( int numberDataPointsInFIDFile )
{


    if( this->GetDadFile() != NULL ) {
        int ptsPerLobe = this->GetHeaderValueAsInt("np", 0)/2;
        int lengthY = this->GetDadFile()->GetEncodedMatrixSizeDimensionValue(1);
        this->paddedData       = new float**[lengthY];
        this->csReorder->ReOrderAndPadData(this->specData, numberDataPointsInFIDFile, this->paddedData);
    } else {

        float *specDataReordered = new float[numberDataPointsInFIDFile];
        this->ReOrderSamples(specDataReordered, numberDataPointsInFIDFile);

        //  I think everything is ok up to here.  Not positive about the padding yet.

        std::vector<int> blipVector = this->GetBlips();

        int lengthX = blipVector[0];
        int lengthY = blipVector[1];
        int lengthF = blipVector[2];
        int numTRs = blipVector[3];

        int **encodeMatrix = new int *[lengthX];
        for (int i = 0; i < lengthX; i++) {
            encodeMatrix[i] = new int[lengthY];
        }

        //  init to 0
        for (int x = 0; x < lengthX; x++) {
            for (int y = 0; y < lengthY; y++) {
                encodeMatrix[x][y] = 0;
            }
        }

        //  Offset 4 to skip past meta fields (lengthX, Y, Z, TR):
        int typeIndex = 4 + (2 * lengthX * lengthY * lengthF);

        int index;
        for (int y = 0; y < lengthY; y++) {
            for (int x = 0; x < lengthX; x++) {
                encodeMatrix[x][y] = blipVector[typeIndex];
                typeIndex++;
            }
        }

        //==================================
        //  Zero pad the data
        //==================================
        //  X = X(16:length(X));
        //  Allocate space for a complex array of  lengthY * lengthX * specPts:
        //  59 samples, 16 flyback phase encodes:
        int specPts = 59 * 16;

        this->paddedData = new float **[lengthY];
        float ***paddedDataTmp = new float **[lengthY];
        int ***xBlips = new int **[lengthY];
        int ***yBlips = new int **[lengthY];
        for (int y = 0; y < lengthY; y++) {
            this->paddedData[y] = new float *[lengthX];
            paddedDataTmp[y] = new float *[lengthX];
            xBlips[y] = new int *[lengthX];
            yBlips[y] = new int *[lengthX];
            for (int x = 0; x < lengthX; x++) {
                this->paddedData[y][x] = new float[specPts * 2];
                paddedDataTmp[y][x] = new float[specPts * 2];
                xBlips[y][x] = new int[lengthF];
                yBlips[y][x] = new int[lengthF];
            }
        }

        for (int y = 0; y < lengthY; y++) {
            for (int x = 0; x < lengthX; x++) {
                for (int s = 0; s < specPts * 2; s++) {
                    this->paddedData[y][x][s] = 0.;
                    paddedDataTmp[y][x][s] = 0.;
                }
                for (int s = 0; s < lengthF; s++) {
                    xBlips[y][x][s] = 0;
                    yBlips[y][x][s] = 0;
                }
            }
        }

        //  Initialze xBlips and yBlips:
        int xBlipIndex = 4;
        int yBlipIndex = 4 + lengthX * lengthY * lengthF;

        for (int y = 0; y < lengthY; y++) {
            for (int x = 0; x < lengthX; x++) {

                int counter = 0;
                for (int sx = xBlipIndex; sx < xBlipIndex + lengthF; sx++) {
                    xBlips[y][x][counter] = blipVector[sx];
                    counter++;
                }
                xBlipIndex = xBlipIndex + lengthF;

                counter = 0;
                for (int sb = yBlipIndex; sb < yBlipIndex + lengthF; sb++) {
                    yBlips[y][x][counter] = blipVector[sb];
                    counter++;
                }
                yBlipIndex = yBlipIndex + lengthF;

            }
        }


        //  Check, can I write out zero data array
        //  next, fill in with real values:
        int startIndex = 0;
        int lengthZ = 16;
        int numSkip = 0;
        int counter = 0;

        for (int y = 0; y < lengthY; y++) {
            for (int x = 0; x < lengthX; x++) {

                if (encodeMatrix[x][y] > 0) {

                    //  loop over 59 lobes:
                    for (int f = 0; f < lengthF; f++) {

                        //  each of the 59 lobe cycles has a length of (lengthZ)
                        int padIndStart = f * (lengthZ * 2);             // target index of zero padded matrix
                        int dataIndStart = (counter * 2 * lengthZ * lengthF) + (f * 2 * lengthZ);

                        int counter2 = dataIndStart;

                        for (int s = padIndStart; s < padIndStart + lengthZ * 2; s += 2) {
                            //  real and imaginary values:
                            paddedDataTmp[y][x][s] = specDataReordered[counter2];
                            paddedDataTmp[y][x][s + 1] = specDataReordered[counter2 + 1];
                            counter2 = counter2 + 2;
                        }
                    }
                    counter++;
                }
            }
        }


        /*!
         *  The next block of code is reimplemented from Simon Hu's func_reorder_blipped_data.m
         *  Takes blipped data, presumably something like (16*59)x16x16 flyback data,
         *  and based on the blips used, puts the data into the correct view
         *  locations.
         *  Note that unlike the matlab implementation, this version does not artificially zero
         *  zero fill the flyback plateau to 43 points.
         *
         *  This just reorders where the x and y phase encodes locations, but leaves the
         *  flyback ordering as is.
         *  matlab function: ordered_data =
         *      orderData(data, specpts, xlen, ylen, flen, pts_lobe,
         *                encodeMatrix, xblips, yblips, blip_phase_correct, phi_x, phi_y);
         */
        int ptsPerLobe = this->GetHeaderValueAsInt("np", 0) / 2;

        for (int y = 0; y < lengthY; y++) {
            for (int x = 0; x < lengthX; x++) {
                if (encodeMatrix[x][y] > 0) {

                    int prevStateX = 0;
                    int prevStateY = 0;

                    int numComponents = 2;

                    for (int f = 0; f < lengthF; f++) {

                        int currStateX = prevStateX - xBlips[y][x][f];
                        int currStateY = prevStateY - yBlips[y][x][f];

                        index = f * (ptsPerLobe * numComponents);

                        for (int s = index; s < (index + (ptsPerLobe * numComponents)); s += 2) {

                            this->paddedData[y + currStateY][x + currStateX][s] =
                                    paddedDataTmp[y][x][s];
                            this->paddedData[y + currStateY][x + currStateX][s + 1] =
                                    paddedDataTmp[y][x][s + 1];

                        }

                        prevStateX = currStateX;
                        prevStateY = currStateY;

                    }
                }
            }
        }

        //++++++++++++++++++++++++++++++++++++

        delete[] specDataReordered;
        for (int i = 0; i < lengthX; i++) {
            delete[] encodeMatrix[i];
        }
        delete[] encodeMatrix;


        for (int y = 0; y < lengthY; y++) {
            for (int x = 0; x < lengthX; x++) {
                delete[] paddedDataTmp[y][x];
                delete[] xBlips[y][x];
                delete[] yBlips[y][x];
            }
            delete[] paddedDataTmp[y];
            delete[] xBlips[y];
            delete[] yBlips[y];
        }

        delete[] paddedDataTmp;
        delete[] xBlips;
        delete[] yBlips;

    }
    //  Now, reset the DICOM header to refect the data reorganization:
    //  This is the target regridded dimensionality:
    this->dcmHeader->SetValue("Columns", this->GetHeaderValueAsInt("fullnv", 0));
    this->dcmHeader->SetValue("Rows", this->GetHeaderValueAsInt("fullnv2", 0));
    this->dcmHeader->SetValue("DataPointColumns",
                              this->GetHeaderValueAsInt("nv", 0)
                              * this->GetHeaderValueAsInt("np", 0) / 2
    );
}


/*!
 *  Reorder FID data encoding
 */
void svkVarianCSFidMapper::ReOrderSamples( float* specDataReordered, int numberDataPointsInFIDFile )
{

    //  Phase encoding order for kx, ky:
    int numKxEncodes = 16;
    int numKyEncodes = 16;

    int kx[16][16] = {
        { 8, 7, 7, 8, 9, 9, 9, 8, 7, 6, 6, 6, 6, 7, 8, 9 },
        { 10, 10, 10, 10, 10, 9, 8, 7, 6, 5, 5, 5, 5, 5, 5, 6 },
        { 7, 8, 9, 10, 11, 11, 11, 11, 11, 11, 11, 10, 9, 8, 7, 6 },
        { 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 6, 7, 8, 9, 10, 11 },
        { 12, 12, 12, 12, 12, 12, 12, 12, 12, 11, 10, 9, 8, 7, 6, 5 },
        { 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 5, 6, 7, 8 },
        { 9, 10, 11, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12 },
        { 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2 },
        { 2, 2, 2, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 },
        { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 13, 12, 11 },
        { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 },
        { 11, 12, 13, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 },
        { 15, 15, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }
    };

    int ky[16][16] = {
        { 7, 7, 8, 8, 8, 7, 6, 6, 6, 6, 7, 8, 9, 9, 9, 9 },
        { 9, 8, 7, 6, 5, 5, 5, 5, 5, 5, 6, 7, 8, 9, 10, 10 },
        { 10, 10, 10, 10, 10, 9, 8, 7, 6, 5, 4, 4, 4, 4, 4, 4 },
        { 4, 4, 5, 6, 7, 8, 9, 10, 11, 11, 11, 11, 11, 11, 11, 11 },
        { 11, 10, 9, 8, 7, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3 },
        { 3, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 12, 12, 12, 12, 12 },
        { 12, 12, 12, 12, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 2 },
        { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 5, 6, 7, 8 },
        { 9, 10, 11, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13 },
        { 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 4, 5, 6, 7 },
        { 8, 9, 10, 11, 12, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
        { 14, 14, 14, 14, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3 },
        { 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 },
        { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 }
    };

    //  See Figure Ref:  Represents Sampling blocks:

    int A2D[16][16] = {
        { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0 },
        { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
        { 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0 },
        { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
        { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    };


    int acqOrder2D[16][16];  //((Y*nx) + X) +1 % L=order of acquisition; matrix operation
    for (int x = 0; x < numKxEncodes; x++) {
        for (int y = 0; y < numKyEncodes; y++) {
            acqOrder2D[x][y] = ky[x][y] * numKxEncodes;
        }
    }
    for (int x = 0; x < numKxEncodes; x++) {
        for (int y = 0; y < numKyEncodes; y++) {
            acqOrder2D[x][y] += kx[x][y] + 1;
            //cout << acqOrder2D[x][y] << " ";
        }
        //cout << endl;
    }

    int A1D[16 * 16];
    int acqOrder1D[16 * 16];
    int i = 0;
    for (int s = 0; s < 16; s++ ) {
        for (int f = 0; f < 16; f++ ) {
            A1D[i] = A2D[s][f] * (i + 1);             //  flatten 2D=>1D array
            acqOrder1D[i] = acqOrder2D[s][f];   //  2D=>1D array  121 120 136 37 38 122 ...
            i++;
        }
    }

    int L3[16 * 16];
    int j = 0;
    for ( int i = 0; i < 256; i++ ) {   //  note only 76 echotrains acquired, not 256
        int index = acqOrder1D[i];      //  full centric acqn order
        if ( A1D[index -1] > 0) {          //  check if data acquired
            //  acqn order; no zeros; 76 entries;
            //  L3: 121   120   136   137   138   122   106   105...
            L3[j] = index;
            j++;
        }
    }


    int A3[16 * 16];
    j = 0;
    for ( int i = 0; i < 256; i++ ) {   //  note only 76 echotrains acquired, not 256
        if (A1D[i] > 0) {               //  check if data acquired
            //  linear acqn order; no zeros; 76 entries;
            //  A3: 1     3     5     7     9    11    13    15    33...
            A3[j] = A1D[i];
            j++;
        }
    }

    int lobes = 59;             //  echoes
    int np = 16 * 2;            //  re+im
    int fidSize = lobes * np;   //  float words re+im; z+f points; etl*np*4 bytes

    int out; 
    int offsetIn; 
    int offsetOut;
    for (int i = 0; i < 76; i++) {
        out = A3[i];                            //  output index
        for (int k = 0; k < 76; k++) {
            if ( L3[k] == out) {
                offsetIn  = k * fidSize;     //  offset into input data
                offsetOut = i * fidSize;              //  offset to output data
                for (int j = 0; j < fidSize; j++ ) {
                    specDataReordered[offsetOut] = this->specData[offsetIn];
                    offsetOut++;
                    offsetIn++;
                } 
            } 
        } 
    } 
}


/*!
 *  Separate flyback encodeing into spatial k space and spectral loop 
 */
void svkVarianCSFidMapper::ReOrderFlyback( ) 
{

    int numX = this->numXReordered; 
    int numY = this->numYReordered; 
    int numZ = this->numZReordered; 
    int numT = this->numTReordered; 

    this->rectilinearData = new float***[numZ];  
    for (int z = 0; z < numZ; z++ ) {
        this->rectilinearData[z] = new float**[ numY ]; 
        for (int y = 0; y < numY; y++ ) {
            this->rectilinearData[z][y] = new float*[ numX ]; 
            for (int x = 0; x < numX; x++ ) {
                this->rectilinearData[z][y][x] = new float[ numT * 2 ]; 
            }
        }
    }

    //  Now remap padded data to new struct:
    for (int y = 0; y < numY; y++ ) {
        for (int x = 0; x < numX; x++ ) {
            for (int t = 0; t < numT*2; t+=2 ) {
                for (int z = 0; z < numZ; z++ ) {

                    this->rectilinearData[z][y][x][t]   = this->paddedData[y][x][ (t * numZ) + (z * 2) ]; 
                    this->rectilinearData[z][y][x][t+1] = this->paddedData[y][x][ (t * numZ) + (z * 2) + 1 ]; 

                }
            }
        }
    }

    //  Now, reset the DICOM header to refect the data reorganization:
    //  This is the target regridded dimensionality: 
    this->dcmHeader->SetValue( "Columns", this->GetHeaderValueAsInt("fullnv", 0) );
    this->dcmHeader->SetValue( "Rows", this->GetHeaderValueAsInt("fullnv2", 0) );
    this->dcmHeader->SetValue( "DataPointColumns",  this->GetHeaderValueAsInt("nv", 0) ); 
    
}


/*!
 *
 */
std::vector<int> svkVarianCSFidMapper::GetBlips()
{

        string blipString;
        this->GetBlipString( &blipString );

    std::vector<int> blipVector;
        istringstream* iss = new istringstream();
        int intVal;

        size_t pos = 0;
        size_t endPos = 0;

        while ( (endPos = blipString.find_first_of(" ", pos) ) != string::npos ) {
            string tmp = blipString.substr(pos, endPos - pos);
            //cout << " blip: " << pos << " " << endPos << " " << tmp << endl;
            iss->str( tmp );
            *iss >> intVal;
        blipVector.push_back( intVal );
            iss->clear();
            pos = endPos + 1;
        }
        if ( pos != blipString.length() ) {
            string tmp = blipString.substr(pos, blipString.length() - pos);
            //cout << " blip: " << tmp << endl;
            iss->str( tmp );
            *iss >> intVal;
        blipVector.push_back( intVal );
        }

        delete iss;

    return blipVector;
}


/*!
 *
 */
void svkVarianCSFidMapper::SetCellSpectrum(vtkImageData* data, int x, int y, int z, int timePt, int coilNum)
{

    int numComponents = 1;
    string representation = this->dcmHeader->GetStringValue( "DataRepresentation" );
    if ( representation.compare( "COMPLEX" ) == 0) {
        numComponents = 2;
    }
    vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
    dataArray->SetNumberOfComponents( numComponents );

    //  This is the flyback encoding dimensionality: 
    int numPts = this->dcmHeader->GetIntValue( "DataPointColumns" );
    dataArray->SetNumberOfTuples(numPts);

    char arrayName[30];
    sprintf(arrayName, "%d %d %d %d %d", x, y, z, timePt, coilNum);
    dataArray->SetName(arrayName);

    int counter = 0; 
    for ( int i = 0; i < numPts; i++ ) {
        counter = i * 2;  
        dataArray->SetTuple(i, &( this->rectilinearData[z][y][x][ counter ] ));
        //cout << "PD: " << y << " " << x << " " << i << " " << this->paddedData[y][x][counter] << " " << this->paddedData[y][x][counter + 1] << endl;
        //dataArray->SetTuple(i, &( this->paddedData[y][x][ counter ] ));
    }

    //  Add the spectrum's dataArray to the CellData:
    //  vtkCellData is a subclass of vtkFieldData
    data->GetCellData()->AddArray(dataArray);

    dataArray->Delete();
    
    return;
}


svkDataAcquisitionDescriptionXML* svkVarianCSFidMapper::GetDadFile() {
    if( this->dadFile == NULL ) {
        string stringFileName = svkUtils::GetPathFromFilename(this->fidFileName);
        vtkGlobFileNames* glob = vtkGlobFileNames::New();
        stringFileName.append("*.xml");
        glob->AddFileNames(stringFileName.c_str());
        if( glob->GetNumberOfFileNames() == 1 ) {
            this->csReorder = svkCSReorder::New();
            this->csReorder->SetDADFilename(glob->GetNthFileName(0));
            this->dadFile = svkDataAcquisitionDescriptionXML::New();
            this->dadFile->SetXMLFileName( glob->GetNthFileName(0));
        }
    }
    return this->dadFile;
}

/*!
 *  Return the blip string.  Should be read from file most likely 
 */
void svkVarianCSFidMapper::GetBlipString( string* blipString )
{

    blipString->assign("16 16 59 76 0 0 -1 0 0 1 -1 0 1 0 -1 0 1 0 0 0 -1 0 0 0 0 1 -1 1 -1 0 0 1 0 -1 1 -1 0 0 1 0 0 -1 0 0 1 -1 0 1 0 -1 1 0 0 0 0 0 -1 0 1 -1 1 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 0 1 0 0 -1 0 1 0 0 -1 1 -1 1 -1 1 -1 1 -1 0 0 0 0 0 1 -1 0 1 -1 0 1 0 0 0 -1 1 0 0 -1 0 0 0 1 -1 1 0 0 -1 1 -1 0 1 0 -1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 -1 1 -1 0 1 0 -1 1 -1 0 0 1 0 -1 0 0 0 0 1 -1 0 1 0 0 -1 1 0 -1 0 1 -1 1 0 0 0 -1 0 0 0 1 0 0 0 0 -1 1 -1 0 1 0 -1 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 -1 0 0 1 0 0 -1 1 0 -1 0 0 1 -1 0 0 1 -1 1 0 0 0 -1 0 0 0 0 1 -1 1 0 -1 0 1 0 -1 1 -1 1 0 -1 1 -1 1 0 0 -1 0 1 -1 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 0 0 0 0 -1 0 1 0 -1 1 -1 0 1 -1 1 0 -1 1 -1 1 0 -1 0 1 -1 0 0 0 1 0 -1 1 -1 1 -1 1 0 -1 0 1 0 -1 1 -1 0 1 -1 1 -1 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 1 0 0 -1 1 -1 0 0 0 1 0 0 0 0 -1 0 1 0 0 -1 0 0 0 0 0 1 -1 1 0 0 0 0 0 -1 0 1 0 -1 0 0 0 0 1 0 0 -1 1 -1 0 0 0 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 0 -1 0 1 -1 1 -1 0 0 0 0 1 -1 0 1 0 -1 1 0 0 -1 1 -1 1 0 0 0 0 -1 0 1 -1 0 0 1 -1 1 -1 1 0 -1 0 0 0 0 0 0 1 0 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 1 0 -1 1 -1 0 1 -1 0 0 0 0 1 -1 0 0 0 1 0 0 -1 0 1 -1 1 0 0 -1 0 1 -1 1 0 0 0 -1 0 1 0 0 -1 0 1 0 0 0 -1 1 0 -1 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 -1 0 1 0 0 -1 0 0 1 0 -1 0 0 0 0 0 0 1 0 0 -1 0 0 1 -1 1 0 -1 1 0 0 0 -1 0 1 -1 1 -1 0 0 0 0 0 1 0 -1 1 0 0 0 0 0 -1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 1 -1 0 1 0 0 -1 1 -1 0 1 0 0 -1 0 0 1 -1 1 0 0 0 0 -1 0 1 0 -1 1 -1 0 0 1 0 -1 1 0 0 -1 0 0 1 0 0 -1 1 -1 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 1 -1 1 0 -1 0 1 0 0 0 0 -1 0 0 0 1 -1 1 0 -1 0 1 -1 1 -1 0 1 -1 1 0 0 -1 0 0 0 0 0 1 0 -1 1 0 0 -1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 0 0 -1 1 -1 0 1 -1 1 -1 1 -1 1 -1 0 1 -1 1 -1 1 -1 1 -1 0 0 0 0 1 0 0 -1 0 0 1 0 0 0 -1 1 0 -1 0 0 1 -1 1 0 -1 1 0 -1 1 -1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 0 -1 0 0 1 -1 1 -1 0 0 1 0 0 0 -1 1 -1 0 1 -1 0 1 -1 0 0 0 1 0 0 0 0 0 0 0 -1 0 1 -1 0 1 0 -1 1 0 0 0 -1 1 -1 0 1 0 -1 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 1 -1 1 -1 0 1 0 0 0 -1 0 0 1 -1 1 -1 0 1 0 0 0 -1 0 0 0 0 1 -1 0 0 1 -1 1 -1 1 -1 1 0 -1 0 0 1 -1 1 -1 1 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 0 0 -1 1 0 -1 1 -1 1 -1 0 1 -1 0 0 0 1 0 -1 1 0 -1 0 0 1 0 -1 1 0 -1 1 -1 0 0 0 0 0 1 0 -1 1 0 -1 0 1 -1 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 0 0 -1 1 0 0 -1 0 1 0 -1 0 1 -1 0 0 1 0 -1 1 -1 0 0 0 0 1 -1 0 1 -1 1 0 -1 1 0 -1 0 0 0 1 -1 0 1 0 0 0 -1 1 0 -1 1 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 0 0 0 0 1 0 0 0 -1 1 -1 0 0 1 0 0 -1 1 -1 0 0 1 -1 1 0 0 -1 1 0 -1 1 0 -1 1 0 0 0 0 0 -1 0 1 0 -1 0 0 1 -1 0 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 0 0 1 0 -1 1 0 0 -1 1 0 -1 1 0 -1 0 0 1 -1 1 0 0 0 -1 0 0 0 0 1 0 -1 0 1 -1 0 0 1 -1 0 0 1 -1 0 1 0 0 -1 1 0 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 1 0 0 0 0 0 -1 1 -1 0 1 0 -1 1 -1 1 0 -1 0 1 0 -1 1 -1 1 0 -1 0 1 -1 0 0 1 0 0 -1 0 0 1 -1 0 1 0 0 -1 0 0 1 -1 1 -1 0 0 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 1 0 0 0 0 0 0 -1 1 -1 0 0 1 0 -1 0 0 0 1 -1 1 0 -1 0 0 1 0 -1 0 1 0 -1 1 -1 0 0 1 -1 0 1 0 -1 0 0 1 -1 1 -1 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 0 1 0 0 0 0 0 -1 1 -1 0 1 0 0 -1 0 1 0 -1 1 -1 0 0 0 1 -1 1 -1 0 1 0 -1 0 0 1 -1 0 0 1 0 -1 1 0 0 -1 1 -1 0 1 0 0 -1 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 1 -1 0 0 0 1 -1 1 -1 1 -1 1 -1 1 -1 1 0 0 -1 0 1 -1 1 0 -1 1 0 -1 1 0 -1 0 0 1 0 0 0 0 -1 1 0 -1 0 0 1 -1 1 0 0 -1 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 1 0 -1 0 0 1 -1 0 1 0 -1 1 -1 1 0 -1 1 0 0 0 -1 1 0 -1 0 0 1 -1 0 1 0 0 0 -1 1 -1 0 1 0 0 0 0 -1 1 -1 0 0 0 0 1 -1 1 -1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 1 0 -1 0 1 -1 1 -1 0 1 0 -1 1 -1 1 -1 1 0 -1 0 0 1 0 0 0 0 0 0 -1 0 1 -1 1 -1 1 0 -1 0 1 -1 0 1 -1 1 -1 1 -1 1 -1 0 0 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 1 0 0 0 0 -1 0 1 -1 0 0 0 0 0 1 -1 1 0 0 -1 1 -1 0 1 -1 0 0 0 1 0 -1 1 0 0 0 -1 1 0 -1 1 0 0 0 -1 0 1 0 -1 0 1 0 -1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 1 -1 0 1 -1 1 -1 0 1 -1 1 -1 0 0 1 0 -1 0 0 0 0 0 1 0 -1 1 -1 1 -1 1 -1 1 0 -1 1 0 -1 1 0 -1 1 0 0 0 -1 1 0 0 -1 1 0 0 0 -1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 0 0 0 -1 0 1 -1 1 0 0 0 -1 0 0 1 -1 0 0 1 -1 0 0 1 0 0 0 0 0 -1 1 -1 1 0 0 -1 1 0 -1 1 0 0 -1 1 -1 1 -1 0 0 0 0 1 -1 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 0 0 0 0 0 -1 1 0 0 -1 0 0 0 1 0 0 0 0 0 0 -1 0 0 0 1 -1 1 0 -1 0 0 0 1 0 0 -1 0 1 0 -1 0 0 1 -1 0 0 1 0 -1 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 0 -1 0 1 -1 1 -1 0 1 0 0 0 0 0 -1 0 1 0 -1 0 0 0 1 -1 0 0 1 0 0 0 -1 0 0 1 0 0 -1 1 -1 0 1 0 -1 0 0 0 1 -1 0 0 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 0 0 -1 1 -1 1 -1 1 0 0 -1 0 0 1 0 -1 1 0 -1 0 1 -1 0 0 1 -1 1 -1 0 1 -1 1 0 0 -1 0 1 0 0 -1 0 0 1 0 -1 1 0 -1 1 0 -1 0 0 0 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 "
					   "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 0 0 0 -1 1 0 -1 1 -1 0 0 1 0 0 0 0 0 -1 1 0 -1 1 -1 1 -1 1 0 0 0 -1 0 0 0 0 0 0 1 -1 1 0 -1 1 -1 0 0 0 1 -1 1 0 -1 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 -1 1 0 -1 0 1 -1 0 1 -1 1 -1 1 -1 1 -1 0 1 -1 1 -1 1 -1 0 0 1 0 -1 0 0 1 -1 0 1 0 0 0 0 -1 0 0 0 0 0 0 0 1 0 0 0 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 0 0 0 1 -1 1 -1 0 1 0 -1 1 -1 0 1 -1 0 1 0 -1 1 -1 1 -1 1 -1 0 1 0 -1 0 1 -1 0 1 -1 0 0 1 0 -1 1 0 0 -1 1 -1 0 1 0 0 0 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 1 0 0 -1 1 0 -1 1 -1 1 -1 0 0 0 1 0 -1 1 0 -1 0 1 -1 1 0 -1 0 1 0 0 0 0 -1 0 1 0 -1 1 -1 0 1 -1 0 1 -1 0 1 -1 0 1 0 -1 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 1 -1 0 1 -1 1 -1 0 0 0 1 0 -1 1 0 0 -1 0 0 0 0 1 0 0 0 -1 0 0 1 0 0 -1 0 1 -1 0 1 0 -1 1 0 -1 1 0 -1 1 0 -1 1 -1 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 -1 1 0 -1 1 0 0 -1 1 -1 0 1 0 -1 1 0 -1 0 0 0 0 1 -1 0 0 1 -1 1 0 -1 0 0 1 0 0 -1 1 0 -1 0 0 1 0 -1 0 1 -1 1 0 0 -1 0 1 -1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 0 0 0 -1 0 0 0 0 0 1 -1 1 0 -1 0 1 -1 0 1 -1 1 -1 0 0 1 0 -1 0 0 1 -1 1 -1 0 1 -1 1 0 -1 0 1 -1 0 1 -1 1 -1 1 0 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 0 1 0 -1 0 0 0 0 0 1 0 0 0 -1 1 0 -1 0 1 0 0 0 -1 0 0 0 1 0 -1 1 0 -1 0 0 1 -1 0 0 1 0 -1 0 0 0 0 1 0 0 0 -1 1 0 -1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 -1 0 1 -1 1 0 -1 1 -1 0 0 1 -1 1 -1 0 0 1 0 0 0 -1 0 0 1 -1 0 1 0 0 0 -1 1 0 -1 0 0 0 0 1 0 -1 0 1 0 0 0 0 -1 0 0 1 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 0 -1 1 0 -1 1 0 0 0 -1 0 1 0 -1 0 0 0 0 1 -1 1 0 0 0 -1 0 0 0 0 0 0 1 -1 1 0 -1 0 1 -1 0 1 -1 1 -1 0 1 0 0 0 -1 1 -1 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 0 1 0 -1 1 0 -1 0 0 1 -1 1 0 -1 0 0 0 1 -1 1 0 0 -1 0 0 0 0 0 1 0 0 -1 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 1 -1 0 0 0 1 -1 1 -1 0 0 1 -1 1 0 -1 0 1 -1 0 1 0 0 0 0 0 0 -1 0 1 0 -1 1 -1 0 0 1 0 -1 1 0 0 -1 1 -1 0 0 0 1 0 0 0 0 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 1 0 0 0 -1 1 0 0 -1 0 0 0 0 0 0 0 1 0 0 -1 0 1 0 -1 0 1 0 0 0 0 0 -1 0 0 1 -1 1 0 0 0 -1 0 1 0 0 0 -1 0 0 0 0 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 0 -1 0 0 1 0 -1 0 1 0 -1 1 -1 1 -1 0 0 1 -1 1 -1 1 0 0 -1 1 0 -1 0 1 0 -1 1 0 0 -1 0 0 1 -1 0 0 1 -1 0 0 1 -1 0 1 0 0 -1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 1 0 0 -1 0 0 0 1 -1 1 -1 1 0 0 -1 0 1 0 0 0 -1 1 0 -1 0 0 1 0 0 -1 0 0 0 1 0 0 -1 1 -1 1 0 -1 0 0 0 0 1 0 -1 1 0 -1 0 1 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 1 -1 1 0 -1 0 1 -1 0 1 0 0 -1 0 0 1 -1 1 0 0 0 -1 0 1 -1 1 0 0 -1 0 1 -1 0 0 1 0 0 -1 0 0 1 0 -1 0 0 0 0 1 -1 1 -1 0 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 0 0 0 0 0 -1 1 0 0 0 -1 0 0 0 1 -1 0 0 0 0 0 0 1 0 -1 1 -1 0 1 0 0 -1 0 1 -1 0 1 -1 1 -1 0 1 0 -1 1 0 0 -1 0 0 1 0 0 -1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 0 0 1 0 0 0 -1 1 -1 1 -1 0 0 0 0 0 0 0 0 0 1 -1 1 0 0 -1 0 0 1 -1 1 0 0 0 -1 0 1 0 -1 1 -1 1 -1 1 0 -1 1 -1 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 0 -1 0 1 -1 1 0 -1 1 0 -1 1 -1 0 0 1 0 -1 0 1 0 0 0 -1 1 -1 0 0 1 0 -1 0 1 0 0 -1 0 1 -1 1 -1 0 1 -1 0 0 0 1 0 -1 1 -1 0 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 0 -1 0 1 0 0 -1 0 1 -1 0 1 -1 1 0 0 -1 1 -1 1 0 0 0 -1 0 0 1 -1 1 -1 1 -1 0 0 0 0 1 0 0 -1 1 -1 1 0 0 -1 0 1 -1 0 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 0 1 -1 1 -1 1 -1 1 0 -1 0 0 1 -1 0 1 0 -1 1 0 0 0 -1 0 0 0 1 -1 0 1 0 -1 0 1 0 -1 0 1 0 0 0 0 -1 1 -1 1 0 -1 1 -1 1 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 1 -1 0 0 0 1 0 0 0 0 0 0 -1 1 0 0 -1 1 -1 0 1 -1 1 -1 1 0 -1 1 -1 1 0 0 -1 0 1 -1 0 1 0 -1 0 1 -1 0 1 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 0 1 0 -1 1 0 0 -1 1 0 -1 1 0 -1 1 -1 1 0 0 0 -1 0 1 0 0 -1 0 0 0 0 0 1 0 0 -1 0 0 1 0 0 -1 1 -1 1 0 -1 1 -1 1 -1 0 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 1 -1 1 -1 1 0 -1 1 0 0 -1 0 0 0 1 -1 0 1 0 -1 1 0 -1 0 0 0 0 1 -1 0 1 0 -1 0 1 -1 1 0 -1 1 -1 0 1 -1 1 0 0 0 -1 1 -1 1 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 -1 0 0 1 0 0 -1 0 1 0 -1 0 1 0 -1 1 0 -1 1 0 -1 1 0 -1 0 1 0 0 -1 1 0 -1 0 0 0 0 0 0 0 0 1 0 0 -1 1 -1 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 0 1 0 -1 1 -1 0 0 0 0 1 -1 1 0 0 -1 0 1 -1 1 -1 1 0 -1 0 0 0 0 1 -1 0 0 1 -1 1 -1 1 -1 0 1 0 -1 1 0 0 -1 1 -1 1 0 0 0 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 0 0 0 1 0 -1 0 1 0 0 0 0 0 -1 1 0 0 0 0 -1 0 1 -1 1 -1 0 0 1 0 0 -1 1 0 -1 1 -1 1 0 0 -1 0 0 1 0 -1 0 0 0 0 0 0 1 -1 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 1 0 0 -1 0 1 -1 1 -1 1 -1 0 1 -1 0 1 -1 0 0 1 -1 1 0 -1 1 -1 1 0 -1 0 1 0 0 0 -1 0 0 0 1 -1 1 0 -1 0 1 -1 1 0 -1 0 1 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 0 1 0 -1 0 0 0 1 0 0 0 0 0 0 -1 0 0 1 -1 0 1 -1 0 0 0 0 1 -1 0 1 0 -1 1 0 0 0 -1 0 0 0 0 0 1 -1 1 0 0 -1 0 1 -1 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 0 0 -1 0 0 0 1 0 -1 0 0 0 1 0 0 -1 0 0 1 0 -1 1 -1 0 0 1 0 -1 1 -1 1 -1 0 0 1 0 -1 1 0 0 0 0 -1 1 0 0 -1 1 0 0 -1 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 "
					   "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 1 0 0 0 0 -1 1 -1 1 0 -1 0 1 0 0 -1 1 -1 0 1 0 -1 0 1 -1 1 -1 1 0 -1 0 1 0 0 -1 0 0 0 0 1 -1 0 1 -1 0 0 1 -1 0 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 1 -1 1 0 -1 1 -1 0 1 0 0 0 0 0 -1 0 1 -1 0 1 -1 0 0 0 0 0 1 -1 1 0 -1 1 0 -1 1 -1 1 0 -1 1 -1 0 0 0 0 0 1 0 0 -1 1 0 -1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 1 0 0 -1 0 1 0 -1 1 0 0 0 0 0 -1 1 0 0 0 0 -1 1 -1 0 0 0 0 1 -1 1 0 -1 1 0 0 -1 0 0 1 0 -1 1 -1 1 -1 1 -1 0 1 -1 0 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 0 -1 1 -1 1 -1 0 0 1 0 0 -1 1 0 -1 1 0 0 -1 1 -1 0 0 1 0 -1 1 -1 1 -1 0 0 0 1 -1 1 -1 1 0 -1 1 -1 1 0 -1 0 0 1 0 0 -1 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 0 0 0 -1 0 0 0 0 0 1 -1 1 0 0 -1 0 1 -1 0 0 0 1 -1 1 0 0 -1 0 0 0 0 1 0 0 0 -1 0 1 0 0 -1 1 -1 0 1 0 0 0 -1 1 -1 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 -1 0 0 0 0 1 0 0 0 0 0 0 -1 0 0 0 0 1 -1 1 0 0 -1 0 0 0 1 -1 1 0 -1 1 0 -1 0 1 -1 0 1 -1 1 -1 1 -1 0 0 1 0 -1 1 0 -1 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 1 -1 1 0 0 0 -1 0 1 0 0 -1 0 0 1 0 -1 1 -1 0 0 1 0 -1 0 1 0 0 -1 1 -1 0 1 -1 1 -1 1 -1 1 -1 1 0 0 0 -1 1 -1 1 -1 1 0 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 0 -1 1 -1 1 -1 1 -1 1 0 -1 1 0 0 0 -1 1 0 0 0 0 -1 1 -1 0 0 0 0 0 1 -1 1 -1 0 1 -1 1 0 0 -1 0 1 -1 0 1 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 1 -1 0 1 -1 0 0 1 0 0 -1 1 -1 1 0 -1 1 -1 0 1 0 -1 0 0 0 0 1 -1 0 1 0 0 -1 1 0 -1 1 -1 1 -1 1 -1 1 0 0 0 0 -1 1 -1 1 0 -1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 0 0 0 0 1 0 0 0 -1 1 0 -1 1 0 0 0 0 0 -1 1 -1 1 -1 0 0 0 0 0 1 0 0 -1 1 -1 0 1 0 -1 0 1 -1 0 0 0 1 0 0 -1 0 1 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 0 0 0 -1 1 -1 0 1 -1 0 1 -1 0 1 0 -1 0 1 0 -1 1 0 -1 1 -1 1 -1 1 -1 0 1 -1 0 1 0 -1 0 1 -1 1 -1 0 1 0 -1 0 1 -1 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 0 0 0 0 0 0 1 0 0 0 0 -1 1 -1 0 1 0 0 0 -1 1 -1 1 0 -1 0 0 1 -1 0 0 1 0 -1 1 0 -1 0 0 1 0 -1 1 0 0 -1 1 0 0 -1 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 1 -1 1 0 0 -1 0 1 0 -1 1 0 -1 1 0 0 0 -1 1 -1 1 -1 1 -1 0 1 -1 1 0 -1 0 1 0 0 -1 1 -1 0 0 0 0 1 -1 0 1 0 0 0 -1 0 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 0 -1 1 0 -1 1 -1 0 0 0 0 1 -1 0 0 1 0 -1 0 0 0 1 -1 0 1 0 -1 1 0 -1 1 -1 1 -1 1 0 0 -1 0 1 0 0 -1 1 -1 1 -1 0 0 1 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 0 1 -1 0 1 0 -1 1 -1 0 1 0 -1 0 0 0 0 1 0 0 -1 0 1 -1 1 0 0 -1 1 0 0 0 -1 0 1 -1 0 0 1 0 0 0 0 0 -1 1 -1 0 1 0 -1 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 1 0 -1 0 1 0 0 -1 1 -1 1 -1 1 0 0 -1 0 0 1 0 -1 1 0 0 -1 0 1 0 0 -1 0 0 0 0 1 0 0 -1 0 0 0 0 0 1 -1 1 -1 1 -1 1 0 0 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 0 -1 1 0 0 0 -1 0 1 0 -1 1 0 -1 1 0 -1 1 0 -1 1 -1 0 1 -1 1 0 -1 0 1 -1 0 1 -1 1 0 0 -1 0 1 -1 1 -1 0 0 0 0 0 0 0 0 0 1 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 -1 0 0 0 0 1 -1 0 0 1 0 -1 0 0 1 0 0 -1 0 0 1 0 0 -1 0 0 0 0 1 -1 1 -1 1 -1 1 -1 1 -1 1 0 0 0 0 -1 1 0 -1 1 -1 0 1 0 0 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 0 -1 1 -1 1 -1 0 1 0 -1 0 0 1 0 -1 0 1 -1 0 1 0 -1 1 -1 0 0 1 0 0 0 -1 1 0 0 0 -1 1 -1 0 0 1 -1 0 1 0 0 -1 1 0 -1 0 0 1 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 0 -1 0 0 1 -1 0 0 0 0 0 0 1 -1 1 0 -1 0 0 1 -1 0 1 0 -1 1 -1 0 1 0 0 0 -1 1 0 0 -1 0 0 0 1 0 -1 1 0 -1 0 1 0 0 0 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 -1 0 0 0 1 0 -1 1 -1 1 -1 0 0 0 0 1 -1 1 -1 1 -1 1 0 0 0 -1 1 0 0 0 -1 0 1 0 0 -1 1 -1 0 1 0 -1 1 0 -1 0 0 1 0 -1 1 -1 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 -1 0 1 -1 0 0 1 0 -1 0 1 0 -1 1 0 -1 1 0 -1 1 -1 0 0 1 -1 0 0 0 1 0 -1 0 0 0 0 1 0 0 0 0 0 -1 0 0 1 0 0 -1 1 -1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 1 0 0 0 0 -1 1 0 -1 1 0 -1 1 0 -1 0 1 -1 0 0 0 0 1 0 -1 1 0 -1 0 1 -1 1 0 -1 0 0 0 0 1 0 0 -1 0 0 1 0 0 -1 0 1 -1 1 0 -1 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 1 0 0 -1 0 1 -1 1 0 -1 0 0 0 0 0 1 -1 1 -1 0 1 0 -1 1 0 -1 1 -1 0 1 -1 0 1 0 0 0 0 -1 0 1 0 0 -1 1 -1 0 1 -1 1 0 0 0 -1 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 1 -1 0 1 -1 1 -1 0 0 0 1 -1 1 -1 1 -1 0 0 1 0 -1 1 0 0 -1 0 0 1 0 -1 1 0 -1 1 -1 0 0 0 0 0 1 -1 1 0 0 0 0 -1 1 0 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 1 0 -1 1 -1 0 1 0 0 -1 0 1 0 0 -1 1 -1 0 1 -1 1 0 0 -1 1 0 0 -1 1 -1 1 0 0 0 -1 0 0 1 -1 1 -1 0 0 0 0 0 1 0 -1 1 0 0 -1 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 1 0 0 -1 0 1 -1 1 0 -1 0 0 1 -1 0 0 0 1 0 -1 0 1 -1 0 0 0 1 0 -1 0 1 -1 1 0 -1 0 1 0 0 -1 1 0 -1 0 0 1 0 -1 1 0 -1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 0 1 -1 1 -1 1 -1 0 1 0 -1 0 0 0 1 0 -1 0 1 0 0 0 -1 0 1 -1 0 0 0 1 0 -1 0 0 0 1 -1 1 0 -1 1 -1 1 0 -1 1 0 0 0 0 0 0 0 -1 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 1 -1 1 -1 1 -1 0 0 1 0 -1 1 -1 0 0 1 -1 0 1 -1 1 0 0 -1 1 -1 1 0 -1 1 -1 1 -1 0 1 0 0 -1 0 1 0 -1 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 0 0 0 0 0 1 0 -1 0 1 -1 1 -1 1 0 -1 1 0 -1 0 0 1 0 0 -1 1 -1 1 -1 1 0 -1 1 0 -1 1 -1 1 -1 1 -1 0 1 0 -1 1 0 0 -1 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0"
					   " 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 1 -1 0 0 1 -1 1 0 0 -1 0 0 1 -1 1 0 -1 1 0 -1 0 1 -1 1 0 -1 0 0 1 0 0 0 -1 0 0 1 -1 1 0 0 -1 0 1 0 0 -1 0 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 1 0 0 -1 0 0 1 0 0 0 0 -1 0 0 1 -1 1 -1 0 0 0 0 1 0 -1 0 1 0 0 -1 1 0 -1 0 1 0 -1 1 0 0 -1 0 1 0 -1 0 0 1 -1 0 1 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 1 0 0 0 0 0 -1 0 1 0 -1 1 0 0 0 0 0 -1 0 1 -1 0 0 1 -1 0 0 1 0 -1 1 -1 1 0 0 -1 0 1 0 0 -1 1 -1 0 0 0 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 -1 1 -1 0 1 -1 1 -1 0 1 0 0 0 -1 0 1 0 -1 0 1 -1 0 1 0 -1 0 0 0 0 0 1 0 -1 0 0 1 0 0 -1 1 0 -1 1 0 0 -1 1 -1 0 1 -1 0 1 0 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 1 0 -1 1 0 0 -1 0 1 0 -1 1 -1 0 1 -1 1 -1 1 -1 1 0 -1 1 -1 0 1 0 0 -1 1 0 -1 1 -1 0 0 0 1 0 -1 1 0 0 0 0 -1 1 -1 1 -1 0 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 1 0 0 0 0 -1 0 0 1 -1 1 -1 0 1 -1 1 0 0 0 -1 0 0 0 1 0 0 -1 1 -1 1 0 -1 1 0 -1 1 -1 0 1 -1 0 1 -1 0 1 0 -1 1 -1 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 1 -1 0 1 -1 1 0 0 0 -1 0 0 0 0 0 0 1 0 -1 1 0 0 0 -1 1 0 -1 1 0 0 0 -1 1 -1 1 -1 0 0 0 1 0 -1 0 0 1 -1 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 1 0 0 0 0 -1 0 1 0 0 0 0 -1 1 0 -1 1 0 0 -1 1 0 0 0 -1 0 1 -1 1 -1 1 -1 0 0 0 0 0 1 0 0 0 0 -1 0 0 0 0 0 1 0 0 -1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 0 0 0 -1 1 0 0 -1 0 0 0 0 1 0 -1 1 0 0 -1 0 1 0 -1 0 1 -1 1 -1 0 0 0 0 0 0 0 0 0 0 1 -1 1 -1 1 0 -1 1 0 0 0 -1 1 0 0 -1 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 -1 1 0 -1 0 0 0 0 1 -1 0 1 0 0 -1 1 -1 0 0 1 -1 1 -1 0 0 1 0 0 0 -1 1 0 -1 0 1 0 0 -1 1 -1 1 0 -1 0 0 0 0 1 -1 1 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 1 0 0 -1 1 -1 1 -1 0 1 0 -1 0 1 -1 0 1 0 0 -1 0 1 -1 0 1 0 0 0 -1 1 -1 1 -1 1 -1 1 0 -1 1 -1 1 -1 1 -1 0 0 1 -1 1 0 -1 0 0 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 1 -1 1 -1 0 0 0 0 1 0 0 0 0 -1 0 1 0 0 0 0 0 -1 1 -1 0 0 1 -1 0 0 1 -1 1 -1 1 -1 0 1 -1 1 0 0 0 -1 1 -1 1 0 0 0 -1 0 0 1 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 0 -1 0 1 -1 1 0 0 -1 0 1 -1 0 1 0 -1 1 0 -1 1 -1 1 -1 1 -1 0 1 -1 1 0 -1 1 -1 0 1 0 -1 1 -1 0 1 -1 0 0 0 1 0 -1 0 0 1 0 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 0 0 -1 0 0 0 0 1 -1 0 1 -1 0 1 -1 0 0 1 0 -1 1 0 0 0 -1 0 1 0 -1 0 0 1 0 -1 1 -1 0 1 -1 0 1 0 0 0 0 0 -1 1 0 -1 0 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 1 -1 0 0 0 0 1 0 0 -1 0 1 0 0 -1 0 1 0 0 0 -1 1 -1 0 0 1 -1 0 1 0 0 0 -1 1 -1 1 0 -1 0 1 0 0 0 -1 0 0 1 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 1 0 0 0 -1 1 -1 0 1 -1 1 -1 0 0 1 -1 0 1 0 -1 1 0 0 0 -1 0 0 0 1 -1 0 1 -1 1 0 -1 0 1 -1 1 -1 0 1 0 0 -1 1 0 -1 1 -1 1 -1 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 1 0 0 0 -1 1 0 -1 1 0 -1 0 0 1 0 0 -1 1 0 0 0 -1 0 1 0 -1 0 0 1 -1 0 0 0 0 0 0 0 0 0 1 0 0 -1 0 0 0 1 -1 1 0 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 1 0 0 -1 1 0 -1 0 1 -1 0 1 0 0 -1 1 0 0 -1 1 -1 0 0 1 -1 1 0 -1 1 0 -1 0 0 0 1 -1 1 0 0 -1 0 0 1 0 0 -1 0 0 1 -1 1 0 -1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 0 -1 0 1 0 0 -1 1 -1 0 0 0 1 0 -1 0 0 1 0 0 -1 0 0 1 -1 0 0 1 0 0 0 0 -1 0 1 -1 1 0 0 0 0 -1 0 1 -1 1 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 0 1 -1 1 0 -1 1 0 -1 1 0 -1 0 0 1 0 -1 1 0 0 -1 0 1 0 0 0 0 -1 1 -1 1 0 -1 1 -1 0 0 0 0 1 -1 1 0 -1 0 1 -1 1 -1 1 -1 0 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 -1 1 0 0 0 -1 0 0 0 1 0 -1 1 -1 0 1 -1 1 0 0 0 -1 1 0 0 -1 1 -1 1 -1 0 0 0 0 0 1 0 0 -1 0 1 -1 0 1 0 0 0 -1 0 0 1 -1 0 1 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 0 0 0 1 -1 1 -1 1 -1 1 -1 0 1 0 -1 1 -1 1 0 -1 1 0 -1 0 1 -1 1 0 -1 1 -1 0 0 1 0 0 0 0 0 0 -1 0 0 0 1 0 -1 0 0 1 -1 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 1 0 -1 1 0 0 0 0 -1 1 -1 0 0 1 0 0 0 0 -1 1 -1 0 0 1 -1 1 -1 0 1 -1 0 1 0 0 -1 0 0 1 0 -1 1 0 0 0 -1 0 1 -1 1 0 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 0 0 0 1 -1 1 -1 0 0 0 0 1 -1 0 1 0 -1 1 0 0 -1 1 0 -1 1 0 -1 1 -1 0 1 0 0 0 0 -1 1 0 -1 0 0 1 0 0 -1 0 0 1 0 -1 1 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 0 -1 1 0 0 0 -1 1 0 0 -1 0 1 -1 0 1 -1 1 -1 0 0 1 -1 1 0 -1 0 1 0 -1 1 -1 0 1 -1 0 0 1 -1 0 0 0 1 -1 0 1 -1 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 0 0 0 0 -1 1 -1 1 -1 0 0 0 1 -1 0 1 0 0 -1 0 1 -1 0 1 -1 1 -1 1 0 -1 1 0 0 0 -1 0 0 1 0 -1 0 1 0 -1 1 0 -1 1 -1 1 -1 0 0 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 1 -1 1 0 -1 1 -1 1 0 0 0 0 -1 0 0 1 -1 0 0 0 0 1 -1 0 1 0 -1 0 1 -1 1 -1 1 -1 1 0 -1 1 0 -1 0 1 -1 1 -1 1 -1 0 0 0 0 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 1 0 0 -1 0 0 0 0 0 1 -1 1 0 0 0 -1 1 -1 0 0 0 0 1 0 0 -1 1 0 0 0 0 -1 1 -1 1 0 -1 0 1 -1 0 1 -1 1 0 0 -1 0 0 0 1 -1 1 -1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 1 -1 1 -1 0 1 -1 1 -1 1 -1 1 0 0 0 -1 1 -1 1 -"
					   "1 1 0 -1 1 0 0 0 -1 1 0 -1 1 -1 0 0 1 -1 1 0 0 -1 0 0 0 1 0 -1 1 0 0 -1 1 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 1 -1 0 0 0 1 -1 1 -1 1 0 0 -1 1 0 0 -1 0 0 1 0 -1 0 1 -1 1 -1 1 -1 1 0 -1 0 1 -1 0 1 0 -1 1 0 -1 0 1 -1 0 0 1 0 0 0 -1 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 4 0 4 0 4 0 4 0 4 0 4 0 4 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 4 0 4 0 4 0 4 0 4 0 4 0 4 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 4 0 4 0 4 0 4 0 4 0 4 0 4 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 4 0 4 0 4 0 1 1 1 1 4 0 4 0 4 0 0 0 0 0 0 0 1 1 1 1 0 0 0 0 0 0 4 0 4 0 4 0 1 1 1 1 4 0 4 0 4 0 0 0 0 0 0 0 1 1 1 1 0 0 0 0 0 0 4 0 4 0 4 0 4 0 4 0 4 0 4 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 4 0 4 0 4 0 4 0 4 0 4 0 4 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 4 0 4 0 4 0 4 0 4 0 4 0 4 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0"); 

}


