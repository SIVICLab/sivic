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


#include <svkVarianFidReader.h>


using namespace svk;


vtkCxxRevisionMacro(svkVarianFidReader, "$Rev$");
vtkStandardNewMacro(svkVarianFidReader);


/*!
 *
 */
svkVarianFidReader::svkVarianFidReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkVarianFidReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->specData = NULL;
    this->dataArray = NULL; 
    this->fidFile = NULL;
    this->procparFile = NULL;
    this->fileSize = 0;
}


/*!
 *
 */
svkVarianFidReader::~svkVarianFidReader()
{

    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if (this->dataArray != NULL) {
        this->dataArray->Delete();
        this->dataArray = NULL; 
    }

    if ( this->fidFile != NULL )  {
        delete fidFile; 
        this->fidFile = NULL; 
    }

    if ( this->procparFile != NULL )  {
        delete procparFile; 
        this->procparFile = NULL; 
    }
}



/*!
 *  Check to see if the extension indicates a Varian FDF file.  If so, try 
 *  to open the file for reading.  If that works, then return a success code. 
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkVarianFidReader::CanReadFile(const char* fname)
{

    string fileToCheck(fname);

    if( fileToCheck.size() >= 3 ) {

        // Also see "vtkImageReader2::GetFileExtensions method" 
        if (  fileToCheck.substr( fileToCheck.size() - 3 ) == "fid" ) {
            FILE *fp = fopen(fname, "rb");
            if (fp) {
                fclose(fp);
                vtkDebugMacro(
                    << this->GetClassName() << "::CanReadFile(): It's a Varian FID File: " << fileToCheck
                );
                return 1;
            }
        } else {
            vtkDebugMacro(
                << this->GetClassName() << "::CanReadFile(): It's NOT a Varian FID File: " << fileToCheck
            );
            return 0;
        }
    } else {
        vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): is NOT a valid file: " << fileToCheck);
        return 0;
    }
}


/*!
 *  Reads spec data from fid file. 
 */
void svkVarianFidReader::ReadFidFiles( vtkImageData* data )
{

    vtkDebugMacro( << this->GetClassName() << "::ReadFidFiles()" );

    ifstream* fidDataIn = new ifstream();
    fidDataIn->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

    int pixelWordSize = 4; 
    int numComponents = 2; 
    int numSpecPoints = this->GetOutput()->GetDcmHeader()->GetIntValue( "DataPointColumns" );

    int numBytesInVol = ( this->GetNumPixelsInVol() * pixelWordSize * numComponents * numSpecPoints );

    fidDataIn->open( this->GetFileName(), ios::binary );

    /*
     *   Flatten the data volume into one dimension
     */
    if (this->specData == NULL) {
        this->specData = new float[ numBytesInVol/pixelWordSize ]; 
    }
/*
    fidDataIn->seekg(0, ios::end);     
    fidDataIn->seekg(-1 * numBytesInFile, ios::end);
    int offset = (fileIndex * numBytesInFile);
    fidDataIn->read( (char *)(this->specData) + offset, numBytesInFile);
    fidDataIn->close();
    delete fidDataIn;
*/

    /*  
     *  If this is running on linux, and the input is bigendian, then swap bytes:
     *  Otherwise, if this is runnin on Solaris/Sparc and the input is NOT bigendian
     *  also swap bytes: 
     */
#if !defined (linux) && !defined(Darwin)
    svkByteSwap::SwapBufferEndianness( (float*)specData, this->GetNumPixelsInVol() );
#endif

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();

    for (int coilNum = 0; coilNum < hdr->GetNumberOfCoils(); coilNum++) {
        for (int timePt = 0; timePt < hdr->GetNumberOfTimePoints(); timePt++) {
            for (int z = 0; z < (this->GetDataExtent())[5] ; z++) {
                for (int y = 0; y < (this->GetDataExtent())[3]; y++) {
                    for (int x = 0; x < (this->GetDataExtent())[1]; x++) {
                        SetCellSpectrum(data, x, y, z, timePt, coilNum);
                    }
                }
            }
        }
    }

    fidDataIn->close();
    delete fidDataIn;
    delete [] this->specData; 
  
}


/*!
 *
 */
void svkVarianFidReader::SetCellSpectrum(vtkImageData* data, int x, int y, int z, int timePt, int coilNum)
{

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();

    int numComponents = 1; 
    string representation =  hdr->GetStringValue( "DataRepresentation" );
    if (representation.compare( "COMPLEX" ) ) { 
        numComponents = 2;
    } 
    vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
    dataArray->SetNumberOfComponents( numComponents );

    int numPts = hdr->GetIntValue( "DataPointColumns" ); 
    dataArray->SetNumberOfTuples(numPts);

    char arrayName[30];
    sprintf(arrayName, "%d %d %d %d %d", x, y, z, timePt, coilNum);
    dataArray->SetName(arrayName);

    int numVoxels[3];
    numVoxels[0] = hdr->GetIntValue( "Columns" );
    numVoxels[1] = hdr->GetIntValue( "Rows" );
    numVoxels[2] = hdr->GetNumberOfSlices();

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
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called after ExecuteInformation()
 */
void svkVarianFidReader::ExecuteData(vtkDataObject* output)
{

    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );
    svkImageData* data = svkImageData::SafeDownCast( this->AllocateOutputData(output) );

    if ( this->FileName ) {
        this->ReadFidFiles( data );
    }

    double dcos[3][3];
    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
    this->GetOutput()->SetDcos(dcos);

    //  SetNumberOfIncrements is supposed to call this, but only works if the data has already
    //  been allocated. but that requires the number of components to be specified.
    this->GetOutput()->GetIncrements();

}


/*!
 *  Side effect of Update() method.  Used to initialize the svkDcmHeader member of 
 *  the target svkImageData object and uses the header to set up the Output Informatin.
 *  Called before ExecuteData()
 */
void svkVarianFidReader::ExecuteInformation()
{
    vtkDebugMacro( << this->GetClassName() << "::ExecuteInformation()" );

    if (this->FileName == NULL) {
        return;
    }

    if ( this->FileName ) {
        vtkDebugMacro( << this->GetClassName() << " FileName: " << FileName );

        struct stat fs;
        if ( stat(this->FileName, &fs) ) {
            vtkErrorMacro("Unable to open file " << this->FileName );
            return;
        }

        this->InitDcmHeader();
        this->SetupOutputInformation();
    }

}


/*!
 *  Initializes the svkDcmHeader adapter to a specific IOD type
 *  and initizlizes the svkDcmHeader member of the svkImageData
 *  object.
 */
void svkVarianFidReader::InitDcmHeader()
{

    vtkDebugMacro( << this->GetClassName() << "::InitDcmHeader()" );

    svkIOD* iod = svkMRSIOD::New();
    iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    iod->InitDcmHeader();
    iod->Delete();

    //  Read the fid header into a map of values used to initialize the
    //  DICOM header. 
    this->ParseFid(); 

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

    if (this->GetDebug()) {
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }
}


/*!
 *  Returns the file type enum 
 */
svkDcmHeader::DcmPixelDataFormat svkVarianFidReader::GetFileType()
{
    svkDcmHeader::DcmPixelDataFormat format = svkDcmHeader::SIGNED_FLOAT_4;

    return format; 
}


/*!
 *
 */
void svkVarianFidReader::InitPatientModule()
{
    this->GetOutput()->GetDcmHeader()->SetDcmPatientsName( this->GetHeaderValueAsString("samplename") );
    this->GetOutput()->GetDcmHeader()->SetValue( "PatientID", this->GetHeaderValueAsString("dataid"));
    this->GetOutput()->GetDcmHeader()->SetValue( "PatientsBirthDate", this->GetHeaderValueAsString("birthday") );
    this->GetOutput()->GetDcmHeader()->SetValue( "PatientsSex", this->GetHeaderValueAsString("gender") );
}


/*!
 *
 */
void svkVarianFidReader::InitGeneralStudyModule()
{
    this->GetOutput()->GetDcmHeader()->SetValue(
        "StudyDate",
        this->GetHeaderValueAsString("date") 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "StudyID",
        this->GetHeaderValueAsString("studyid_") 
    );
}


/*!
 *
 */
void svkVarianFidReader::InitGeneralSeriesModule()
{

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SeriesNumber",
        0 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SeriesDescription",
        "Varian FID Data"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "PatientPosition",
        "UNKNOWN" 
    );


}


/*!
 *  DDF is historically the UCSF representation of a GE raw file so
 *  initialize to svkVarianFidReader::MFG_STRING.
 */
void svkVarianFidReader::InitGeneralEquipmentModule()
{
    this->GetOutput()->GetDcmHeader()->SetValue(
        "Manufacturer",
        "Varian"
    );
}


/*!
 *
 */
void svkVarianFidReader::InitMRSpectroscopyModule()
{

    /*  =======================================
     *  MR Image and Spectroscopy Instance Macro
     *  ======================================= */

    this->GetOutput()->GetDcmHeader()->SetValue(
        "AcquisitionDatetime",
        this->GetHeaderValueAsString("date") 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "AcquisitionDuration",
        0
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ResonantNucleus",
        "H1" 
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "KSpaceFiltering",
        "NONE"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ApplicableSafetyStandardAgency",
        "FDA"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "MagneticFieldStrength",
        -1 
    );
    /*  =======================================
     *  END: MR Image and Spectroscopy Instance Macro
     *  ======================================= */

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ImageType",
        string("ORIGINAL\\PRIMARY\\SPECTROSCOPY\\NONE")
    );


    /*  =======================================
     *  Spectroscopy Description Macro
     *  ======================================= */
    this->GetOutput()->GetDcmHeader()->SetValue(
        "VolumetricProperties",
        string("VOLUME")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "VolumeBasedCalculationTechnique",
        string("NONE")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ComplexImageComponent",
        string("COMPLEX")
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
        "0"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SpectralWidth",
        this->GetHeaderValueAsFloat( "sw" )
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SVK_FrequencyOffset",
        0
    );
    this->GetOutput()->GetDcmHeader()->SetValue(
        "ChemicalShiftReference",
        0
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "VolumeLocalizationTechnique", 
        ""
    );  
    
    //if ( strcmp(ddfMap["localizationType"].c_str(), "PRESS") == 0)  {
        //this->InitVolumeLocalizationSeq();
    //}

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
        string("NONE")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "FrequencyCorrection", 
        "NO"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "FirstOrderPhaseCorrection", 
        string("NO")
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "WaterReferencedPhaseCorrection", 
        string("NO")
    );
}




/*!
 *
 */
void svkVarianFidReader::InitMRSpectroscopyPulseSequenceModule()
{
    this->GetOutput()->GetDcmHeader()->SetValue(
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

    this->GetOutput()->GetDcmHeader()->SetValue(
        "MRSpectroscopyAcquisitionType",
        acqType
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "EchoPulseSequence",
        "SPIN"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "MultipleSpinEcho",
        "NO"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "MultiPlanarExcitation",
        "NO"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SteadyStatePulseSequence",
        "NONE"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "EchoPlanarPulseSequence",
        "NO"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SpectrallySelectedSuppression",
        ""
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "GeometryOfKSpaceTraversal",
        "RECTILINEAR"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "RectilinearPhaseEncodeReordering",
        "LINEAR"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "SegmentedKSpaceTraversal",
        "SINGLE"
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "CoverageOfKSpace",
        "FULL"
    );

    this->GetOutput()->GetDcmHeader()->SetValue( "NumberOfKSpaceTrajectories", 1 );
}


/*!
 *
 */
void svkVarianFidReader::InitMRSpectroscopyDataModule()
{
    this->GetOutput()->GetDcmHeader()->SetValue( "Columns", this->GetHeaderValueAsInt("nv", 0) );
    this->GetOutput()->GetDcmHeader()->SetValue( "Rows", this->GetHeaderValueAsInt("nv2", 0) );
    this->GetOutput()->GetDcmHeader()->SetValue( "DataPointRows", 0 );
    this->GetOutput()->GetDcmHeader()->SetValue( "DataPointColumns", this->GetHeaderValueAsInt("np", 0)/2 );
    this->GetOutput()->GetDcmHeader()->SetValue( "DataRepresentation", "COMPLEX" );
    this->GetOutput()->GetDcmHeader()->SetValue( "SignalDomainColumns", "TIME" );
    this->GetOutput()->GetDcmHeader()->SetValue( "SVK_ColumnsDomain", "KSPACE" );
    this->GetOutput()->GetDcmHeader()->SetValue( "SVK_RowsDomain", "KSPACE" );
    this->GetOutput()->GetDcmHeader()->SetValue( "SVK_SliceDomain", "KSPACE" );
}



/*! 
 *  
 */
void svkVarianFidReader::InitMultiFrameFunctionalGroupsModule()
{

    this->InitSharedFunctionalGroupMacros();

    this->GetOutput()->GetDcmHeader()->SetValue(
        "InstanceNumber",
        1
    );

    this->GetOutput()->GetDcmHeader()->SetValue(
        "ContentDate",
        this->GetHeaderValueAsString( "time_svfdate" )
    );

    this->numSlices = this->GetHeaderValueAsInt("ns");
    int numEchoes = this->GetHeaderValueAsInt("ne");

    this->GetOutput()->GetDcmHeader()->SetValue( 
        "NumberOfFrames", 
        this->numSlices * numEchoes 
    ); 

    this->InitPerFrameFunctionalGroupMacros();

}


/*! 
 *  
 */
void svkVarianFidReader::InitMultiFrameDimensionModule()
{
}


/*! 
 *  
 */
void svkVarianFidReader::InitAcquisitionContextModule()
{
}


/*! 
 *  
 */
void svkVarianFidReader::InitSharedFunctionalGroupMacros()
{

    this->InitPixelMeasuresMacro();
    this->InitPlaneOrientationMacro();

    //this->InitFrameAnatomyMacro();
    //this->InitMRSpectroscopyFrameTypeMacro();

    this->InitMRTimingAndRelatedParametersMacro();
    this->InitMRSpectroscopyFOVGeometryMacro();
    this->InitMREchoMacro();

    //this->InitMRModifierMacro();

    this->InitMRReceiveCoilMacro();

    //this->InitMRTransmitCoilMacro();
    //this->InitMRAveragesMacro();
    //this->InitMRSpatialSaturationMacro();
    //this->InitMRSpatialVelocityEncodingMacro();

}


/*! 
 *  
 */
void svkVarianFidReader::InitPerFrameFunctionalGroupMacros()
{

    this->InitFrameContentMacro();
    this->InitPlanePositionMacro();

}



/*!
 *  Mandatory, Must be a per-frame functional group
 */
void svkVarianFidReader::InitFrameContentMacro()
{

    int numCoils = 1; 
    int numTimePts = 1; 
    int numFrameIndices = svkDcmHeader::GetNumberOfDimensionIndices( numTimePts, numCoils ) ;

    unsigned int* indexValues = new unsigned int[numFrameIndices];

    int frame = 0;

    for (int coilNum = 0; coilNum < numCoils; coilNum++) {

        for (int timePt = 0; timePt < numTimePts; timePt++) {

            for (int sliceNum = 0; sliceNum < this->numSlices; sliceNum++) {

                svkDcmHeader::SetDimensionIndices(
                    indexValues, numFrameIndices, sliceNum, timePt, coilNum, numTimePts, numCoils
                );

                this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
                    "PerFrameFunctionalGroupsSequence",
                    frame,
                    "FrameContentSequence"
                );

                this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
                    "FrameContentSequence",
                    0,
                    "DimensionIndexValues",
                    indexValues,        //  array of vals
                    numFrameIndices,    // num values in array
                    "PerFrameFunctionalGroupsSequence",
                    frame
                );

                this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
                    "FrameContentSequence",
                    0,
                    "FrameAcquisitionDateTime",
                    "EMPTY_ELEMENT",
                    "PerFrameFunctionalGroupsSequence",
                    frame 
                );
    
                this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
                    "FrameContentSequence",
                    0,
                    "FrameReferenceDateTime",
                    "EMPTY_ELEMENT",
                    "PerFrameFunctionalGroupsSequence",
                    frame 
                );
    
                this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
                    "FrameContentSequence",
                    0,
                    "FrameAcquisitionDuration",
                    "-1",
                    "PerFrameFunctionalGroupsSequence",
                    frame 
                );
    
                frame++; 
            }
        }
    }

    delete[] indexValues;

}


/*!
 *  The FID toplc is the center of the first voxel. 
 */
void svkVarianFidReader::InitPlanePositionMacro()
{

    double dcos[3][3];
    this->GetOutput()->GetDcmHeader()->SetSliceOrder( this->dataSliceOrder );
    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
    double pixelSpacing[3];
    this->GetOutput()->GetDcmHeader()->GetPixelSize(pixelSpacing); 

    float numPixels[3]; 
    numPixels[0] = this->GetHeaderValueAsInt("nv", 0);
    numPixels[1] = this->GetHeaderValueAsInt("nv2", 0);
    numPixels[2] = this->GetHeaderValueAsInt("ns", 0);

    //  Get center coordinate float array from fidMap and use that to generate 
    //  Displace from that coordinate by 1/2 fov - 1/2voxel to get to the center of the
    //  toplc from which the individual frame locations are calculated

    //  If volumetric 3D (not 2D), get the center of the TLC voxel in LPS coords: 
    float* volumeTlcLPSFrame = new float[3];  
    if ( this->GetHeaderValueAsInt("ns", 0) > 1 ) {

        //  Get the volumetric center in acquisition frame coords: 
        float volumeCenterAcqFrame[3];  
        for (int i = 0; i < 3; i++) {
            volumeCenterAcqFrame[i] = this->GetHeaderValueAsFloat("location[]", i); 
        }

        float* volumeTlcAcqFrame = new float[3];  
        for (int i = 0; i < 3; i++) {
            volumeTlcAcqFrame[i] = volumeCenterAcqFrame[i] 
                                 + ( this->GetHeaderValueAsFloat("span[]", i) - pixelSpacing[i] )/2; 
        }
        this->UserToMagnet(volumeTlcAcqFrame, volumeTlcLPSFrame, dcos);  
        delete [] volumeTlcAcqFrame;
        
    }

    float displacement[3];
    //  Center of toplc (LPS) pixel in frame:  
    float frameLPSPosition[3];

    /*  
     *  Iterate over slices (frames)
     *  If 3D vol, calculate slice position, otherwise use value encoded 
     *  into slice header
     */
    for (int i = 0; i < this->GetOutput()->GetDcmHeader()->GetNumberOfSlices(); i++) {

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "PerFrameFunctionalGroupsSequence",
            i,
            "PlanePositionSequence"
        );

        //  Need to displace along normal from tlc of slice: 
        //  add displacement along normal vector to get toplc for each frame:
        for (int j = 0; j < 3; j++) {
            displacement[j] = dcos[2][j] * pixelSpacing[2] * i;
        }

        string imagePositionPatient;

        //  If 2D (single slice)
        if ( this->GetHeaderValueAsInt("ns", 0) == 1 ) {

            //  Location is the center of the image frame in user (acquisition frame). 
            float centerAcqFrame[3];  
            for ( int j = 0; j < 3; j++) {
                centerAcqFrame[j] = 0.0;  
            }

            //  Now get the center of the tlc voxel in the acq frame: 
            float* tlcAcqFrame = new float[3];  
            for (int j = 0; j < 2; j++) {
                tlcAcqFrame[j] = centerAcqFrame[j] 
                                 - ( ( numPixels[j] * pixelSpacing[j] ) - pixelSpacing[j] )/2; 
            }
            tlcAcqFrame[2] = centerAcqFrame[2]; 

            //  and convert to LPS (magnet) frame: 
            this->UserToMagnet(tlcAcqFrame, frameLPSPosition, dcos);  
                
            delete [] tlcAcqFrame; 

        } else {

            for(int j = 0; j < 3; j++) { //L, P, S
                frameLPSPosition[j] = volumeTlcLPSFrame[j] +  displacement[j] ;
            }

        }

        for (int j = 0; j < 3; j++) {
            ostringstream oss;
            oss.precision(8);
            oss << frameLPSPosition[j];
            imagePositionPatient += oss.str();
            if (j < 2) {
                imagePositionPatient += '\\';
            }
        }

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "PlanePositionSequence",
            0,
            "ImagePositionPatient",
            imagePositionPatient,
            "PerFrameFunctionalGroupsSequence",
            i
        );
    }
}


/*!
 *  Pixel Spacing:
 */
void svkVarianFidReader::InitPixelMeasuresMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PixelMeasuresSequence"
    );

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

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PixelMeasuresSequence",
        0,
        "PixelSpacing",
        pixelSizeString[0] + "\\" + pixelSizeString[1], 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PixelMeasuresSequence",
        0,
        "SliceThickness",
        pixelSize[2],
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *  
 */
void svkVarianFidReader::InitPlaneOrientationMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PlaneOrientationSequence"
    );

    //  Get the euler angles for the acquisitin coordinate system: 
    float psi = this->GetHeaderValueAsFloat("psi", 0);
    float phi = this->GetHeaderValueAsFloat("phi", 0);
    float theta = this->GetHeaderValueAsFloat("theta", 0);

    vtkTransform* eulerTransform = vtkTransform::New(); 
    eulerTransform->RotateX( psi ); 
    eulerTransform->RotateY( theta ); 
    eulerTransform->RotateY( phi ); 
    vtkMatrix4x4* dcos = vtkMatrix4x4::New();  
    eulerTransform->GetMatrix(dcos); 
    cout << *dcos << endl; 

    string orientationString;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            ostringstream dcosOss;
            dcosOss << dcos->GetElement(i, j); 
            orientationString.append( dcosOss.str() );
            if (i != 1 || j != 2  ) {
                orientationString.append( "\\");
            }
        }
    }

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PlaneOrientationSequence",
        0,
        "ImageOrientationPatient",
        orientationString,
        "SharedFunctionalGroupsSequence",
        0
    );

    //  Determine whether the data is ordered with or against the slice normal direction.
    double normal[3];
    this->GetOutput()->GetDcmHeader()->GetNormalVector(normal);

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
void svkVarianFidReader::InitMRTimingAndRelatedParametersMacro()
{
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRTimingAndRelatedParametersSequence"
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "RepetitionTime",
        this->GetHeaderValueAsFloat( "tr" ),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "FlipAngle",
        this->GetHeaderValueAsFloat("fliplist", 0),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "EchoTrainLength",
        1,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "RFEchoTrainLength",
        1,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "GradientEchoTrainLength",
        1,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
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
void svkVarianFidReader::InitMRSpectroscopyFOVGeometryMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRSpectroscopyFOVGeometrySequence"
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionDataColumns",
        this->GetHeaderValueAsInt("np", 0)/2, 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionPhaseColumns",
        this->GetHeaderValueAsInt("nv", 0), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionPhaseRows",
        this->GetHeaderValueAsInt("nv2", 0), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SpectroscopyAcquisitionOutOfPlanePhaseSteps",
        this->GetHeaderValueAsInt("ns", 0), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionTLC",
        "0\\0\\0",
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionPixelSpacing",
        this->GetHeaderValueAsString("vox1", 0) + '\\' + this->GetHeaderValueAsString("vox2", 0), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcquisitionSliceThickness",
        this->GetHeaderValueAsFloat("vox3", 0), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
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
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,                               
        "SVK_SpectroscopyAcqReorderedPhaseColumns",
        this->GetHeaderValueAsInt("nv", 0),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                               
        "SVK_SpectroscopyAcqReorderedPhaseRows",
        this->GetHeaderValueAsInt("nv2", 0), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                               
        "SVK_SpectroscopyAcqReorderedOutOfPlanePhaseSteps",
        this->GetHeaderValueAsInt("ns", 0), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,                               
        "SVK_SpectroscopyAcqReorderedTLC",
        "0\\0\\0",
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",   
        0,                                      
        "SVK_SpectroscopyAcqReorderedPixelSpacing",
        this->GetHeaderValueAsString("vox1", 0) + '\\' + this->GetHeaderValueAsString("vox2", 0), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,                                      
        "SVK_SpectroscopyAcqReorderedSliceThickness",
        this->GetHeaderValueAsFloat("vox3", 0), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence",
        0,
        "SVK_SpectroscopyAcqReorderedOrientation",
        "0\\0\\0\\0\\0\\0\\0\\0\\0", 
        "SharedFunctionalGroupsSequence",
        0
    );

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
void svkVarianFidReader::InitMREchoMacro()
{
    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MREchoSequence"
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MREchoSequence",
        0,
        "EffectiveEchoTime",
        this->GetHeaderValueAsFloat( "te" ) * 1000,
        "SharedFunctionalGroupsSequence",
        0
    );
}



/*!
 *  Receive Coil:
 */
void svkVarianFidReader::InitMRReceiveCoilMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRReceiveCoilSequence"
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "ReceiveCoilName",
        "Varian Coil",
        "SharedFunctionalGroupsSequence",
        0
    );

}



/*! 
 *  Use the FDF patient position string to set the DCM_PatientPosition data element.
 */
string svkVarianFidReader::GetDcmPatientPositionString(string patientPosition)
{
    size_t delim = patientPosition.find_first_of(',');
    string headFeetFirst( patientPosition.substr(0, delim) );

    for(int i = 0; i < headFeetFirst.size(); i++){
        headFeetFirst[i] = tolower( headFeetFirst[i] );
    }

    string dcmPatientPosition;
    if( headFeetFirst.find("head first") != string::npos ) {
        dcmPatientPosition.assign("HF");
    } else if( headFeetFirst.find("feet first") != string::npos ) {
        dcmPatientPosition.assign("FF");
    } else {
        dcmPatientPosition.assign("UNKNOWN");
    }

    //  skip ", ":
    string spd( patientPosition.substr(delim + 2) );
    for(int i = 0; i < spd.size(); i++){
        spd[i] = tolower( spd[i] );
    }

    if( spd.find("supine") != string::npos ) {
        dcmPatientPosition += "S";
    } else if( spd.find("prone") != string::npos ) {
        dcmPatientPosition += "P";
    } else if( spd.find("decubitus left") != string::npos ) {
        dcmPatientPosition += "DL";
    } else if( spd.find("decubitus right") != string::npos ) {
        dcmPatientPosition += "DR";
    } else {
        dcmPatientPosition += "UNKNOWN";
    }

    return dcmPatientPosition; 
}


/*
 *  Read FID header blocks into a string STL map for use during initialization 
 *  of DICOM header by Init*Module methods. 
 *  If a procpar file is present in the directory, parse that as well. 
 */
void svkVarianFidReader::ParseFid()
{

    string fidFileName( this->GetFileName() );  
    string fidFilePath( this->GetFilePath( this->GetFileName() ) );  

    try { 

        this->ParseProcpar(fidFilePath);
        this->PrintProcparKeyValuePairs();

    } catch (const exception& e) {
        cerr << "ERROR opening or reading Varian fid file (" << fidFileName << "): " << e.what() << endl;
    }

}


/*! 
 *  Utility function to read a single line from the fid file and return 
 *  set the delimited key/value pair into the stl map.  
 */
void svkVarianFidReader::GetFidKeyValuePair( vtkStringArray* keySet )    
{

    istringstream* iss = new istringstream();

    string keyString;
    string valueString;

    try {

        this->ReadLine(this->fidFile, iss); 

        size_t  position; 
        string  tmp; 
        string  dataType; 
        long    headerSize;

        int dataBufferSize = this->GetDataBufferSize();

        //  Read only to the start of the pixel buffer, 
        //  i.e. no more than the header size:     
        headerSize = this->fileSize - dataBufferSize; 
        if ( this->fidFile->tellg() < headerSize - 1 ) {

            //  find first white space position before "key" string: 
            position = iss->str().find_first_of(' ');
            if (position != string::npos) {
                tmp.assign( iss->str().substr(position) );
                dataType.assign( iss->str().substr(0, position) ) ; 
            } 
    
            //  If necessary, remove pointer indicator: 
            position = tmp.find_first_of('*');
            if (position != string::npos) {
                tmp.assign( tmp.substr(position + 1) );
            } 
    
            //  Extract key and value strings:
            position = tmp.find_first_of('=');
            if (position != string::npos) {
                keyString.assign( tmp.substr(0, position - 1) );
                keyString = StripWhite(keyString); 
                // Check for key match if doing a limited search: 
                int parseValue = 1;
                if (keySet != NULL) { 
                    if (keySet->GetNumberOfValues() > 0) { 
                        parseValue = 0; 
                        for (int i = 0; i < keySet->GetNumberOfValues(); i++) { 
                            if ( keySet->GetValue(i) == keyString ) {
                                parseValue = 1; 
                            } 
                        } 
                    } 
                }
                if ( !parseValue )  {
                    return; 
                }   

                valueString.assign( tmp.substr(position + 2) );
                // Remove terminating ; 
                position = valueString.find_first_of(';');
                valueString.assign( valueString.substr(0, position) );

                // Remove string quotes
                this->RemoveStringQuotes( &valueString );
                while ( ( position = valueString.find('"') ) != string::npos) {
                    valueString.erase( position, 1 );
                }
    
                //  Parse elements into vector: remove matrix brackets 
                //  and assign elements to vector: 
                position = valueString.find_first_of('{');
                if (position != string::npos) {

                    valueString.assign( valueString.substr(position + 1) );
                    position = valueString.find_first_of('}');

                    if (position != string::npos) {
                        valueString.assign( valueString.substr(0, position) );
                    } 
                } 

                this->ParseAndSetStringElements(keyString, valueString);
            } 

        } else { 
            this->fidFile->seekg(0, ios::end);     
        }
    } catch (const exception& e) {
        cout <<  "ERROR reading line: " << e.what() << endl;
    }

    delete iss; 
}


/*!
 *  Attempts to determine the pixel data buffer size from the currently 
 *  available header information.  If the size can be determined return that, 
 *  otherwise return 0. 
 */
int svkVarianFidReader::GetDataBufferSize()
{
    int bufferSize = 0; 
    map<string, string>::iterator it;

    if (fidMap.find("bits") != fidMap.end() && 
        fidMap.find("rank") != fidMap.end() && 
        fidMap.find("matrix[]") != fidMap.end() ) {

        int numDims = this->GetHeaderValueAsInt("rank");

        int numBitsPerByte = 8;
        int pixelWordSize = this->GetHeaderValueAsInt("bits")/numBitsPerByte;

        bufferSize = pixelWordSize; 
        for (int i = 0; i < numDims; i++) { 
            bufferSize *= this->GetHeaderValueAsInt("matrix[]", i);
        }
    }

    return bufferSize;
}


/*!
 *  Push key value pairs into the map's value vector: 
 *  mapFor values that are comma separated lists, put each element into the value 
 *  vector. 
 */
void svkVarianFidReader::ParseAndSetStringElements(string key, string valueArrayString) 
{
    size_t pos;
    istringstream* iss = new istringstream();
    string tmpString;     

    while ( (pos = valueArrayString.find_first_of(',')) != string::npos) {  

        iss->str( valueArrayString.substr(0, pos) );
        *iss >> tmpString;
        fidMap[key].push_back(tmpString); 
        iss->clear();

        valueArrayString.assign( valueArrayString.substr(pos + 1) ); 
    }
    iss->str( valueArrayString );
    *iss >> tmpString;
    fidMap[key].push_back(tmpString); 
    delete iss; 
}


/*!
 *
 */
int svkVarianFidReader::GetHeaderValueAsInt(string keyString, int valueIndex, int procparRow) 
{
    
    istringstream* iss = new istringstream();
    int value;

    iss->str( (procparMap[keyString])[procparRow][valueIndex]);
    *iss >> value;
    return value; 
}


/*!
 *
 */
float svkVarianFidReader::GetHeaderValueAsFloat(string keyString, int valueIndex, int procparRow) 
{
    
    istringstream* iss = new istringstream();
    float value;

    iss->str( (procparMap[keyString])[procparRow][valueIndex]);
    *iss >> value;
    return value; 
}


/*!
 *
 */
string svkVarianFidReader::GetHeaderValueAsString(string keyString, int valueIndex, int procparRow) 
{
    return (procparMap[keyString])[procparRow][valueIndex];
}


/*!
 *
 */
int svkVarianFidReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}

