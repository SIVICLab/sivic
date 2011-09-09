/*
 *  Copyright © 2009-2011 The Regents of the University of California.
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

#include <svkGEPostageStampReader.h>
#include <vtkObjectFactory.h>
#include <vtkDebugLeaks.h>
#include <vtkInformation.h>
#include <vtkStringArray.h>

#include <svkMrsImageData.h>
#include <svkIOD.h>
#include <svkMRSIOD.h>
#include <svkUtils.h>


using namespace svk;


vtkCxxRevisionMacro(svkGEPostageStampReader, "$Rev$");
vtkStandardNewMacro(svkGEPostageStampReader);


/*!
 *
 */
svkGEPostageStampReader::svkGEPostageStampReader()
{
#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkGEPostageStampReader");
#endif
    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");
}


/*!
 *
 */
svkGEPostageStampReader::~svkGEPostageStampReader()
{
    vtkDebugMacro(<<this->GetClassName() << "::~" << this->GetClassName() << "()");
}


/*!
 *  Mandatory, must be overridden to get the factory to check for proper
 *  type of vtkImageReader2 to return. 
 */
int svkGEPostageStampReader::CanReadFile(const char* fname)
{

    vtkstd::string fileToCheck(fname);
    bool isGEPostage = false; 

    if ( svkDcmHeader::IsFileDICOM( fname ) ) {
 
        svkImageData* tmp = svkMrsImageData::New(); 
        tmp->GetDcmHeader()->ReadDcmFile( fname ); 
        vtkstd::string SOPClassUID = tmp->GetDcmHeader()->GetStringValue( "SOPClassUID" ) ; 

        // Check for MR Image Storage
        if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.4" ) {

            //  Check for proprietary use of DICOM MR ImageStorage:
            if ( this->ContainsProprietaryContent( tmp ) == true ) {
                this->SetFileName(fname);
                isGEPostage = true;
            }

        }

        tmp->Delete(); 
    } 

    if ( isGEPostage ) { 
            cout << this->GetClassName() << "::CanReadFile(): It's a GE Postage Stamp File: " <<  fileToCheck << endl;
            this->SetFileName(fname);
            return 1;

    } else {

        vtkDebugMacro(<<this->GetClassName() << "::CanReadFile() It's Not a GE Postage Stamp file " << fileToCheck );

    }

    return 0;

}


/*
 *  returns false, meaning it's ok to have a multi-volumetric data set
 *  since real and imaginary components are stored in separate PS images 
 *  with the same imagePositionPatient values. 
 */
bool svkGEPostageStampReader::CheckForMultiVolume() {
    return false;
}


/*!
 *
 */
void svkGEPostageStampReader::InitDcmHeader()
{

    this->InitFileNames();

    // Read the first file and load the header as the starting point
    this->GetOutput()->GetDcmHeader()->ReadDcmFile( this->GetFileNames()->GetValue(0) );

    //  Now override elements with Multi-Frame sequences and default details:
    svkIOD* iod = svkMRSIOD::New();
    iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    iod->SetReplaceOldElements(false);
    iod->InitDcmHeader();
    iod->Delete();

    vtkstd::string studyInstanceUID( this->GetOutput()->GetDcmHeader()->GetStringValue("StudyInstanceUID"));
    this->GetOutput()->GetDcmHeader()->SetValue( "StudyInstanceUID", studyInstanceUID.c_str() );


    //  Now move info from original MRImageStorage header elements to flesh out enhanced
    //  SOP class elements (often this is just a matter of copying elements from the top
    //  level to a sequence item.
    this->InitMultiFrameFunctionalGroupsModule();
    this->InitMRSpectroscopyModule();
    this->InitMRSpectroscopyDataModule();

    /*
     *  odds and ends:
     */
    this->GetOutput()->GetDcmHeader()->SetValue(
        "Rows",
       16 
    );
    this->GetOutput()->GetDcmHeader()->SetValue(
        "Columns",
       16 
    );


    if (this->GetDebug()) {
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }

}


/*!
 *  Initializes any private DICOM attributes that are needed internally
 */
void svkGEPostageStampReader::InitPrivateHeader()
{
}



/*!
 *
 */
void svkGEPostageStampReader::InitMultiFrameFunctionalGroupsModule()
{

    //  num frames is 1/2 num files for postage stamp data, since real and
    //  imaginary components of each slice are in separate files. 
    this->numFrames =  this->GetFileNames()->GetNumberOfValues() / 2;
    this->GetOutput()->GetDcmHeader()->SetValue(
        "NumberOfFrames",
        this->numFrames
    );

    this->InitSharedFunctionalGroupMacros();
    this->InitPerFrameFunctionalGroupMacros();

}


/*!
 *
 */
void svkGEPostageStampReader::InitPerFrameFunctionalGroupMacros()
{

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();

    //  Get toplc float array from rdaMap and use that to generate
    //  frame locations:
    double toplc[3];
    for (int i = 0; i < 3; i++) {
        toplc[i] = svkUtils::StringToDouble( hdr->GetStringValue( "ImagePositionPatient", i ) );
    }
    
    double pixelSpacing[3];
    hdr->GetPixelSpacing(pixelSpacing);

    if ( this->numFrames > 2 ) {

        double origin0[3];
        hdr->GetOrigin(origin0, 0); 

        svkImageData* tmpImage = svkMriImageData::New();
        tmpImage->GetDcmHeader()->ReadDcmFile( this->GetFileNames()->GetValue( 2 ) );

        double origin1[3];
        tmpImage->GetDcmHeader()->GetOrigin(origin1, 0);
        tmpImage->Delete();
        

        double sliceSpacing = 0;
        for (int i = 0; i < 3; i++ ) {
            sliceSpacing += pow(origin1[i] - origin0[i], 2);
        }
        sliceSpacing = pow(sliceSpacing, .5);

        pixelSpacing[2] = sliceSpacing;
    }

    hdr->SetSliceOrder( svkDcmHeader::INCREMENT_ALONG_NEG_NORMAL );

    double dcos[3][3];
    hdr->GetDataDcos(dcos);

    hdr->InitPerFrameFunctionalGroupSequence(
        toplc, pixelSpacing, dcos, this->numFrames, 1, 1
    );

}




/*!
 *
 */
void svkGEPostageStampReader::InitSharedFunctionalGroupMacros()
{

    //this->InitPixelMeasuresMacro();
    //this->InitPlaneOrientationMacro();
    this->InitMRTimingAndRelatedParametersMacro();
    this->InitMRSpectroscopyFOVGeometryMacro();
    this->InitMREchoMacro();
    //this->InitMRModifierMacro();
    this->InitMRReceiveCoilMacro();
    //this->InitMRTransmitCoilMacro();
    this->InitMRAveragesMacro();
    //this->InitMRSpatialSaturationMacro();
    //this->InitMRSpatialVelocityEncodingMacro();
}


/*!
 *
 */
void svkGEPostageStampReader::InitMRTimingAndRelatedParametersMacro()
{
    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    float repTime = hdr->GetFloatValue("RepetitionTime"); 
    hdr->ClearElement("RepetitionTime");
    float flipAngle = hdr->GetFloatValue("FlipAngle"); 
    hdr->ClearElement("FlipAngle");
    hdr->InitMRTimingAndRelatedParametersMacro(
        repTime, 
        flipAngle
    );
}


/*!
 *
 */
void svkGEPostageStampReader::InitMREchoMacro()
{

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    float echoTime = hdr->GetFloatValue("EchoTime"); 
    hdr->ClearElement("EchoTime");
    hdr->InitMREchoMacro( echoTime ); 
}


/*!
 *
 */
void svkGEPostageStampReader::InitMRReceiveCoilMacro()
{

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();

    hdr->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRReceiveCoilSequence"
    );

    string rcvCoilName = hdr->GetStringValue("ReceiveCoilName"); 
    hdr->ClearElement("ReceiveCoilName");
    hdr->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "ReceiveCoilName",
        rcvCoilName, 
        "SharedFunctionalGroupsSequence",
        0
    );

    hdr->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "ReceiveCoilManufacturerName",
        vtkstd::string("GE"),
        "SharedFunctionalGroupsSequence",
        0
    );

    vtkstd::string coilType("VOLUME");
    if ( rcvCoilName.compare("8HRBRAIN") == 0 ) {
        coilType.assign("MULTICOIL");
    }

    hdr->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "ReceiveCoilType",
        coilType,
        "SharedFunctionalGroupsSequence",
        0
    );

    hdr->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "QuadratureReceiveCoil",
        vtkstd::string("YES"),
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *
 */
void svkGEPostageStampReader::InitMRSpectroscopyModule()
{


    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();


    /*  =======================================
     *  MR Image and Spectroscopy Instance Macro
     *  ======================================= */
    string studyDate = hdr->GetStringValue("StudyDate");
    string acqTime = hdr->GetStringValue("AcquisitionTime");
    hdr->SetValue(
        "AcquisitionDatetime",
        studyDate + acqTime
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "AcquisitionDuration",
        0
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ResonantNucleus",
        "1H" 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "KSpaceFiltering",
        "NONE"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ApplicableSafetyStandardAgency",
        "Research"
    );

    //this->GetOutput()->GetDcmHeader()->SetValue(
        //"MagneticFieldStrength",
        //hdr->GetStringValue("MagneticFieldStrength");
    //);
    /*  =======================================
     *  END: MR Image and Spectroscopy Instance Macro
     *  ======================================= */

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ImageType",
        vtkstd::string("ORIGINAL\\PRIMARY\\SPECTROSCOPY\\NONE")
    );


    /*  =======================================
     *  Spectroscopy Description Macro
     *  ======================================= */
    this->GetOutput()->GetDcmHeader()->SetValue(
        "VolumetricProperties",
        vtkstd::string("VOLUME")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "VolumeBasedCalculationTechnique",
        vtkstd::string("NONE")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ComplexImageComponent",
        vtkstd::string("COMPLEX")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "AcquisitionContrast",
        "UNKNOWN"
    );
    /*  =======================================
     *  END: Spectroscopy Description Macro
     *  ======================================= */


    this->GetOutput()->GetDcmHeader()->SetValue(
        "TransmitterFrequency",
        hdr->GetStringValue("ImagingFrequency")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SpectralWidth",
        "1000"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_FrequencyOffset",
        "0"
    );  

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ChemicalShiftReference",
        "4.7"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "VolumeLocalizationTechnique", 
        "PRESS"
    );  
    
//    if ( strcmp(ddfMap["localizationType"].c_str(), "PRESS") == 0)  {
//       this->InitVolumeLocalizationSeq();
//  }

    this->GetOutput()->GetDcmHeader()->SetValue(
        "Decoupling", 
        "NO" 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "TimeDomainFiltering", 
        "NONE" 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "NumberOfZeroFills", 
        0 
    );
    
    this->GetOutput()->GetDcmHeader()->SetValue(
        "BaselineCorrection", 
        vtkstd::string("NONE")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "FrequencyCorrection",
        "NO"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "FirstOrderPhaseCorrection",
        vtkstd::string("NO")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "WaterReferencedPhaseCorrection",
        vtkstd::string("NO")
    );
}


/*!
 *
 */
void svkGEPostageStampReader::InitMRAveragesMacro()
{
    int numAverages = 1;
    this->GetOutput()->GetDcmHeader()->InitMRAveragesMacro(numAverages);
}


/*!
 *
 */
void svkGEPostageStampReader::InitMRSpectroscopyFOVGeometryMacro()
{

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();

    hdr->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRSpectroscopyFOVGeometrySequence"
    );

    hdr->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionDataColumns",
        256, 
        "SharedFunctionalGroupsSequence",
        0
    );

    hdr->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionPhaseColumns",
        hdr->GetIntValue( "Columns" ),
        "SharedFunctionalGroupsSequence",
        0
    );

    hdr->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionPhaseRows",
        hdr->GetIntValue( "Rows" ),
        "SharedFunctionalGroupsSequence",
        0
    );

    hdr->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionOutOfPlanePhaseSteps",
        hdr->GetIntValue( "NumberOfFrames" ),
        "SharedFunctionalGroupsSequence",
        0
    );
/*
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionTLC",
        ddfMap["acqToplcLPS0"] + '\\' + ddfMap["acqToplcLPS1"] + '\\' + ddfMap["acqToplcLPS2"],
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionPixelSpacing",
        ddfMap["acqSpacing0"] + '\\' + ddfMap["acqSpacing1"],
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionSliceThickness",
        ddfMap["acqSpacing2"],
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionOrientation",
        ddfMap["acqDcos00"] + '\\' + ddfMap["acqDcos01"] + '\\' + ddfMap["acqDcos02"] + '\\' +
        ddfMap["acqDcos10"] + '\\' + ddfMap["acqDcos11"] + '\\' + ddfMap["acqDcos12"] + '\\' +
        ddfMap["acqDcos20"] + '\\' + ddfMap["acqDcos21"] + '\\' + ddfMap["acqDcos22"],
        "SharedFunctionalGroupsSequence",    
        0
    );
    

    // ==================================================
    //  Reordered Params
    // ==================================================
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                               
        "SVK_SpectroscopyAcqReorderedPhaseColumns",
        this->GetHeaderValueAsInt( ddfMap, "reorderedNumberOfPoints0" ),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                               
        "SVK_SpectroscopyAcqReorderedPhaseRows",
        this->GetHeaderValueAsInt( ddfMap, "reorderedNumberOfPoints1" ),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                               
        "SVK_SpectroscopyAcqReorderedOutOfPlanePhaseSteps",
        this->GetHeaderValueAsInt( ddfMap, "reorderedNumberOfPoints2"),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,                               
        "SVK_SpectroscopyAcqReorderedTLC",
        ddfMap["reorderedToplcLPS0"] + '\\' + ddfMap["reorderedToplcLPS1"] + '\\' + ddfMap["reorderedToplcLPS2"],
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedPixelSpacing",
        ddfMap["reorderedSpacing0"] + '\\' + ddfMap["reorderedSpacing1"],
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedSliceThickness",
        ddfMap["reorderedSpacing2"],
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedOrientation",
        ddfMap["reorderedDcos00"] + '\\' + ddfMap["reorderedDcos01"] + '\\' + ddfMap["reorderedDcos02"] + '\\' +
        ddfMap["reorderedDcos10"] + '\\' + ddfMap["reorderedDcos11"] + '\\' + ddfMap["reorderedDcos12"] + '\\' +
        ddfMap["reorderedDcos20"] + '\\' + ddfMap["reorderedDcos21"] + '\\' + ddfMap["reorderedDcos22"],
        "SharedFunctionalGroupsSequence",
        0
    );
*/
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "PercentSampling",
        1,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
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
void svkGEPostageStampReader::InitMRSpectroscopyDataModule()
{


    int numCols = static_cast<int>(this->GetOutput()->GetDcmHeader()->GetFloatValue( "GE_PS_MATRIX_X" ));
    int numRows = static_cast<int>(this->GetOutput()->GetDcmHeader()->GetFloatValue( "GE_PS_MATRIX_Y" ));


    this->GetOutput()->GetDcmHeader()->SetValue(
        "Rows", 
        numRows 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "Columns",
        numCols 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "DataPointRows",
        1
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "DataPointColumns",
        256
    );

    int numComponents =  2; 
    vtkstd::string representation;
    if (numComponents == 1) {
        representation = "REAL";
    } else if (numComponents == 2) {
        representation = "COMPLEX";
    }

    this->GetOutput()->GetDcmHeader()->SetValue(
        "DataRepresentation",
        representation
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SignalDomainColumns",
        "frequency"
    );


    //  Private Attributes for spatial domain encoding:
    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_ColumnsDomain",
        "space"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_RowsDomain",
        "space"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_SliceDomain",
        "space"
    );
}




/*! 
 *  
 */
void svkGEPostageStampReader::LoadData( svkImageData* data )
{

    vtkstd::string dataRepresentation = this->GetOutput()->GetDcmHeader()->GetStringValue( "DataRepresentation" ); 
    int numComponents; 
    if ( dataRepresentation == "COMPLEX" ) {
        numComponents = 2; 
    } else {
        numComponents = 1; 
    }

    this->numFreqPts = this->GetOutput()->GetDcmHeader()->GetIntValue( "DataPointColumns" ); 
    this->numTimePts = this->GetOutput()->GetDcmHeader()->GetNumberOfTimePoints();
    int numCoils = this->GetOutput()->GetDcmHeader()->GetNumberOfCoils();

    int numVoxels[3]; 
    data->GetNumberOfVoxels(numVoxels); 

    //  the num pixels in plane
    long unsigned int dataLength = 
        numVoxels[0] * numVoxels[1] * this->numFreqPts * numComponents * numCoils * this->numTimePts;

    short* specData = new short[ dataLength ];

    for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {
        
        int realImag = fileIndex % 2; 
        svkImageData* tmpImage = svkMriImageData::New();
        tmpImage->GetDcmHeader()->ReadDcmFile( this->GetFileNames()->GetValue( fileIndex ) );
        //  if this is the 2nd component of complex pair, adjust the offset of 
        //  the specData by 1/2 data length:
        tmpImage->GetDcmHeader()->GetShortValue( "PixelData", specData + realImag * (dataLength/2) , dataLength/2);  

        //  once both components of complex pair have been read in, then set the spectrum
        if ( realImag == 1 ) {
            for (int coilNum = 0; coilNum < numCoils; coilNum ++) {
                for (int timePt = 0; timePt < this->numTimePts; timePt ++) {
                    for (int y = 0; y < (this->GetDataExtent())[3]; y++) {
                        for (int x = 0; x < (this->GetDataExtent())[1]; x++) {
                            int z = fileIndex / 2; 
                            SetCellSpectrum( data, x, y, z, timePt, coilNum, numComponents, specData);
                        }
                    }
                }
            }
        }

        tmpImage->Delete();
    }

    delete [] specData; 

    //this->GetOutput()->GetDcmHeader()->PrintDcmHeader(); 
}


/*!
 *
 */
void svkGEPostageStampReader::SetCellSpectrum(svkImageData* data, int x, int y, int z, int timePt, int coilNum, int numComponents, short* specData)
{

    //  Set XY points to plot - 2 components per tuble for complex data sets:
    vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
    dataArray->SetNumberOfComponents( numComponents );

    dataArray->SetNumberOfTuples(this->numFreqPts);
    char arrayName[30];
    sprintf(arrayName, "%d %d %d %d %d", x, y, z, timePt, coilNum);
    dataArray->SetName(arrayName);

    int numVoxels[3]; 
    data->GetNumberOfVoxels(numVoxels); 
    int cols = numVoxels[0];
    int rows = numVoxels[1];

    int dx = 16;        //  16 stride along columns
    int dy = 256*16;    //  16*256 stride along rows
    int componentOffset = 16 * 16 * 16 * 16;    // hardcoded size convention of postage stamp data:
    int startIndex = x + y * 16 * 16; 
    float specVal[2];
    int freqPt = 0; 
    for ( int rows = 0; rows < 16; rows++) {
        for ( int cols = 0; cols < 16; cols++) {
            int offset = startIndex + cols * dx + rows * dy; 
            specVal[0] =  static_cast<float>(specData[ offset ]);
            specVal[1] =  static_cast<float>(specData[ offset + componentOffset ]);
            dataArray->SetTuple( freqPt, specVal );
            freqPt++; 
        }
    }

    //  Add the spectrum's dataArray to the CellData:
    //  vtkCellData is a subclass of vtkFieldData
    data->GetCellData()->AddArray(dataArray);

    //  Should these be a member var, deleted in destructor?
    dataArray->Delete();

    return;
}


/*!
 *  Returns the file root without extension
 */
svkDcmHeader::DcmPixelDataFormat svkGEPostageStampReader::GetFileType()
{
    return svkDcmHeader::SIGNED_FLOAT_4;
}


/*!
 *
 */
int svkGEPostageStampReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}

