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


#include <svkNIFTIVolumeReader.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include <svkTypeUtils.h>


using namespace svk;


vtkStandardNewMacro(svkNIFTIVolumeReader);


/*!
 *  
 */
svkNIFTIVolumeReader::svkNIFTIVolumeReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkNIFTIVolumeReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->vtkNIFTIReader = vtkNIFTIImageReader::New();
}


/*!
 *
 */
svkNIFTIVolumeReader::~svkNIFTIVolumeReader()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
    this->vtkNIFTIReader->Delete();
}



/*!
 *  Call VTK's native CanReadFile() 
 */
int svkNIFTIVolumeReader::CanReadFile(const char* fname)
{
    return this->vtkNIFTIReader->CanReadFile(fname);
}


/*!
 *  Loads the nifti file (.nii) into a VTK nifti reader
 */
void svkNIFTIVolumeReader::LoadNifti()
{

    vtkDebugMacro( << this->GetClassName() << "::LoadNifti()" );

    this->vtkNIFTIReader->SetFileName(this->GetFileName());
    this->vtkNIFTIReader->Update();

    this->vtkNIFTIHeader = this->vtkNIFTIReader->GetNIFTIHeader();

}


/*!
 *  Utility method returns the total number of Pixels in 3D.
 */
int svkNIFTIVolumeReader::GetNumPixelsInVol()
{
    return (
        ( (this->GetDataExtent())[1] + 1 ) *
        ( (this->GetDataExtent())[3] + 1 ) *
        ( (this->GetDataExtent())[5] + 1 )
    );
}


/*!
 *  Not sure if this is always extent 5 if the data is coronal and sagital for example
 */
int svkNIFTIVolumeReader::GetNumSlices()
{
    vtkWarningWithObjectMacro(this, "GetNumSlices: May not be correct for non axial data.");
    return (this->GetDataExtent())[5] + 1;
}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called after ExecuteInformation()
 */
void svkNIFTIVolumeReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
{
    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );

    svkImageData* data = this->GetOutput();
    vtkImageData* niiData = this->vtkNIFTIReader->GetOutput();

    data->DeepCopy(niiData);

    string arrayNameString("pixels");
    data->GetPointData()->GetArray(0)->SetName(arrayNameString.c_str());

    /* 
     * We need to make a shallow copy of the output, otherwise we would have it
     * registered twice to the same reader which would cause the reader to never delete.
     */
    double dcos[3][3];
    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
    this->GetOutput()->SetDcos(dcos); 

    //  SetNumberOfIncrements is supposed to call this, but only works if the data has already
    //  been allocated. but that requires the number of components to be specified. 
    this->GetOutput()->GetIncrements(); 
    this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector();

    cout << "NIFTI Header:\n";
    cout << *this->vtkNIFTIHeader;

    cout << "Image Data Output: \n";
    cout << *this->GetOutput();

}


/*!
 *  Side effect of Update() method.  Used to initialize the svkDcmHeader member of 
 *  the target svkImageData object and uses the header to set up the Output Informatin.
 *  Called before ExecuteData()
 */
void svkNIFTIVolumeReader::ExecuteInformation()
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

        this->LoadNifti();
        this->InitDcmHeader();
        this->SetupOutputInformation();
    }

}


/*!
 *  Initializes the svkDcmHeader adapter to a specific IOD type
 *  and initizlizes the svkDcmHeader member of the svkImageData
 *  object.
 */
void svkNIFTIVolumeReader::InitDcmHeader()
{

    vtkDebugMacro( << this->GetClassName() << "::InitDcmHeader()" );

    this->iod = svkEnhancedMRIIOD::New();
    this->iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    this->iod->InitDcmHeader();

    this->InitPatientModule();
    this->InitGeneralStudyModule();
    this->InitGeneralSeriesModule();
    this->InitGeneralEquipmentModule();
    this->InitImagePixelModule();
    this->InitMultiFrameFunctionalGroupsModule();
    this->InitAcquisitionContextModule();

    if (this->GetDebug()) {
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }

    this->iod->Delete();
}


/*!
 *  Returns the file root without extension
 */
svkDcmHeader::DcmPixelDataFormat svkNIFTIVolumeReader::GetFileType()
{

    int fileType = this->vtkNIFTIReader->GetDataScalarType();

    switch (fileType) {
        case 0: return svkDcmHeader::UNDEFINED;
        case 7: return svkDcmHeader::UNSIGNED_INT_1;
        case 9: return svkDcmHeader::UNSIGNED_INT_2;
        case 8: return svkDcmHeader::SIGNED_INT_2;
        case 10: return svkDcmHeader::SIGNED_FLOAT_4;
        case 11: return svkDcmHeader::SIGNED_FLOAT_8;
        default: return svkDcmHeader::UNSIGNED_INT_2;
    }

}


/*!
 *  If the IDF studyID field contains an accession number/t-number than
 *  we don't know the PatientID
 */
void svkNIFTIVolumeReader::InitPatientModule()
{

    string patientName = this->GetFileName();
    string patientID = this->GetFileName();

    this->GetOutput()->GetDcmHeader()->InitPatientModule(
        patientName,
        patientID, 
        "",
        "" 
    );

}


/*!
 *
 */
void svkNIFTIVolumeReader::InitGeneralStudyModule()
{
    string accessionNumber = this->GetFileName();
    string studyID = this->GetFileName();

    this->GetOutput()->GetDcmHeader()->InitGeneralStudyModule(
        "",
        "",
        "",
        studyID,
        accessionNumber, 
        "" 
    );

}


/*!
 *
 */
void svkNIFTIVolumeReader::InitGeneralSeriesModule()
{
    string patientPosition;

    this->GetOutput()->GetDcmHeader()->InitGeneralSeriesModule(
        "0",
        "",
        "HFS"
    );

}


/*!
 *  DDF is historically the UCSF representation of a GE raw file so
 *  initialize to svkNIFTIVolumeReader::MFG_STRING.
 */
void svkNIFTIVolumeReader::InitGeneralEquipmentModule()
{
    // No way to know what type of scanner the images were acquired on. 
}


/*!
 *
 */
void svkNIFTIVolumeReader::InitImagePixelModule()
{        
    int rows = this->vtkNIFTIHeader->GetDim(1);
    this->GetOutput()->GetDcmHeader()->SetValue( "Rows", rows );

    int cols = this->vtkNIFTIHeader->GetDim(2);
    this->GetOutput()->GetDcmHeader()->SetValue( "Columns", cols );
}


/*! 
 *  
 */
void svkNIFTIVolumeReader::InitMultiFrameFunctionalGroupsModule()
{
    this->numSlices = this->vtkNIFTIHeader->GetDim(3);
    this->numVolumes = this->vtkNIFTIHeader->GetDim(4);
    this->numFrames = this->numSlices * this->numVolumes;

    this->InitSharedFunctionalGroupMacros();
    this->InitPerFrameFunctionalGroupMacros();
}


/*! 
 *  
 */
void svkNIFTIVolumeReader::InitSharedFunctionalGroupMacros()
{
    this->InitPixelMeasuresMacro();
    this->InitPlaneOrientationMacro();
    this->InitMRImagingModifierMacro();
    this->InitMRTransmitCoilMacro();
    this->InitMRReceiveCoilMacro();
}


/*!
 *  Pixel Spacing:
 */
void svkNIFTIVolumeReader::InitPixelMeasuresMacro()
{
    double rowDim = this->vtkNIFTIHeader->GetPixDim(1);
    double colDim = this->vtkNIFTIHeader->GetPixDim(2);
    double sliceDim = this->vtkNIFTIHeader->GetPixDim(3);

    string rowString = svkTypeUtils::DoubleToString(rowDim, 5);
    string colString = svkTypeUtils::DoubleToString(colDim, 5);
    string pixSpacing = rowString + "\\" + colString;
    string sliceThickness = svkTypeUtils::DoubleToString(sliceDim, 5);

    this->GetOutput()->GetDcmHeader()->InitPixelMeasuresMacro(
        pixSpacing,
        sliceThickness
    );
}


/*!
 *  
 */
void svkNIFTIVolumeReader::InitPlaneOrientationMacro()
{

    double dcos[3][3] = {
            {1.00000, 0.00000, 0.00000},
            {0.00000, 1.00000, 0.00000},
            {0.00000, 0.00000, -1.00000}
    };

    ostringstream ossDcos;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            ossDcos << dcos[i][j];
            if (i * j != 2) {
                ossDcos<< "\\";
            }
        }
    }

    string orientationString = ossDcos.str();

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PlaneOrientationSequence"
    );

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

    double dcosSliceOrder[3] = {0, 1, 0};

    //  Use the scalar product to determine whether the data in the .cmplx
    //  file is ordered along the slice normal or antiparalle to it.
    vtkMath* math = vtkMath::New();
    if (math->Dot(normal, dcosSliceOrder) > 0 ) {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
    } else {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_NEG_NORMAL;
    }
    this->GetOutput()->GetDcmHeader()->SetSliceOrder( this->dataSliceOrder );
    math->Delete();

}


/*!
 *  Transmit Coil: (Info not present => use dummy default)
 */
void svkNIFTIVolumeReader::InitMRTransmitCoilMacro()
{
    this->GetOutput()->GetDcmHeader()->InitMRTransmitCoilMacro("UNKNOWN", "UNKNOWN", "UNKNOWN");
}


/*!
 *  Receive Coil: (Info not present => use dummy default)
 */
void svkNIFTIVolumeReader::InitMRReceiveCoilMacro()
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
            "UNKNOWN",
            "SharedFunctionalGroupsSequence",
            0
    );

}


/*!
 *
 */
void svkNIFTIVolumeReader::InitPerFrameFunctionalGroupMacros()
{

    //  Get toplc float array from idfMap and use that to generate
    //  frame locations:
    double toplc[3] = {-127.50000, -156.49001, 96.99010};

    double dcos[3][3] = {
            {1.00000, 0.00000, 0.00000},
            {0.00000, 1.00000, 0.00000},
            {0.00000, 0.00000, -1.00000}
    };

    double pixelSize[3] = {1.00000, 1.00000, 1.50000};

    svkDcmHeader::DimensionVector dimensionVector =
            this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(
            &dimensionVector, svkDcmHeader::SLICE_INDEX, this->numSlices-1);

    this->GetOutput()->GetDcmHeader()->AddDimensionIndex(
            &dimensionVector, svkDcmHeader::TIME_INDEX, this->numVolumes-1);

    this->GetOutput()->GetDcmHeader()->InitPerFrameFunctionalGroupSequence(
            toplc,
            pixelSize,
            dcos,
            &dimensionVector
    );
}


/*!
 *
 */
void svkNIFTIVolumeReader::InitMRImagingModifierMacro()
{

    float transmitFreq = -1;
    float pixelBandwidth = -1; 

    this->GetOutput()->GetDcmHeader()->InitMRImagingModifierMacro( transmitFreq, pixelBandwidth ); 
}

/*!
 *
 */
void svkNIFTIVolumeReader::InitAcquisitionContextModule()
{
}

/*!
 *
 */
int svkNIFTIVolumeReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData");
    return 1;
}


