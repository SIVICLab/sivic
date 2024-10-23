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

#include <svkDcmSegmentationVolumeReader.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkInformation.h>

#include <svkMriImageData.h>


using namespace svk;


//vtkCxxRevisionMacro(svkDcmSegmentationVolumeReader, "$Rev$");
vtkStandardNewMacro(svkDcmSegmentationVolumeReader);


/*!
 *
 */
svkDcmSegmentationVolumeReader::svkDcmSegmentationVolumeReader()
{
#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkDcmSegmentationVolumeReader");
#endif
    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");
    this->rescalePixels = true;
}


/*!
 *
 */
svkDcmSegmentationVolumeReader::~svkDcmSegmentationVolumeReader()
{
    vtkDebugMacro(<<this->GetClassName() << "::~" << this->GetClassName() << "()");
}




/*!
 *  Mandatory, must be overridden to get the factory to check for proper
 *  type of vtkImageReader2 to return. 
 */
int svkDcmSegmentationVolumeReader::CanReadFile(const char* fname)
{

    std::string fileToCheck(fname);

    if ( svkDcmHeader::IsFileDICOM( fname ) ) {
 
        svkImageData* tmp = svkMriImageData::New(); 
        tmp->GetDcmHeader()->ReadDcmFileHeaderOnly( fname );
        std::string SOPClassUID = tmp->GetDcmHeader()->GetStringValue( "SOPClassUID" ) ; 
        tmp->Delete(); 

        if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.66.4" ) {

            vtkDebugMacro(<<this->GetClassName() << "::CanReadFile(): It's a DICOM Segmentation file " << fileToCheck );

            this->SetFileName(fname);

            return 1;
        }
    } 

    vtkDebugMacro(<<this->GetClassName() << "::CanReadFile() It's Not a DICOM Segmentation file " << fileToCheck );

    return 0;
}


/*! 
 *  Initializes any private DICOM attributes that are needed internally
 */
void svkDcmSegmentationVolumeReader::InitPrivateHeader()
{
	// Required by parent class, not sure if we need anything here.
}


/*! 
 *  Loads the data. If the data header contains a RescaleSlope and Rescale Intercept
 *  That are not 1 and 0 respectively then the data will be loaded into doubles and
 *  rescaled.
 */
void svkDcmSegmentationVolumeReader::LoadData( svkImageData* data )
{

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    
    svkDcmHeader::DimensionVector dimensionVector = hdr->GetDimensionIndexVector();
    int rows = svkDcmHeader::GetDimensionVectorValue( &dimensionVector, svkDcmHeader::ROW_INDEX) + 1;
    int columns = svkDcmHeader::GetDimensionVectorValue( &dimensionVector, svkDcmHeader::COL_INDEX) + 1;
    int numSlices = svkDcmHeader::GetDimensionVectorValue( &dimensionVector, svkDcmHeader::SLICE_INDEX) + 1;
    int numFrames = hdr->GetIntValue( "NumberOfFrames" );
    this->numVolumes = numFrames / numSlices;

    int numPixelsInSlice   = rows * columns;
    int dataWordSize       = 2; // Size of data stored in Segmentation Object, always short
    int numPixelsInVolume  = numPixelsInSlice * numSlices;
    int numBytesPerVol     = numPixelsInVolume * dataWordSize;

    // Get the Pixel Data from the DICOM header.
	void* imageData = NULL;
    if( this->numVolumes == 1 ) {
        imageData = this->GetOutput()->GetPointData()->GetScalars()->GetVoidPointer(0);
    } else {
	    imageData = (void*) malloc( numBytesPerVol*this->numVolumes );
    }
	hdr->GetByteValue( "PixelData", ((char*)imageData), numPixelsInVolume*this->numVolumes );

    // Lets go ahead and free the short data now that we have a copy of it.
    hdr->ClearElement("PixelData");

	// Create vtk Point Data arrays for each volume.
    for (int vol = 0; vol < this->numVolumes; vol++) {

        vtkDataArray* array = NULL;
        if( this->numVolumes > 1 ) {
            array = vtkUnsignedCharArray::New();
            array->SetNumberOfComponents(1);
            array->SetNumberOfTuples(rows*columns*numSlices);
        } else {
            array = this->GetOutput()->GetPointData()->GetScalars();
        }
        ostringstream volNumber;
        volNumber << vol;
        std::string arrayNameString("pixels");
        arrayNameString.append(volNumber.str());
        array->SetName( arrayNameString.c_str() );
        if( this->numVolumes > 1 ) {
            array->SetVoidArray( (char*)(imageData) + vol*numPixelsInVolume , numPixelsInVolume, 0);
            if( vol == 0 ) {
                data->GetPointData()->SetScalars(array);
            } else {
                data->GetPointData()->AddArray(array);
            }

        }
        if( vol % 2 == 0 ) { // update progress every other volume
			ostringstream progressStream;
			progressStream <<"Reading Volume " << vol << " of " << this->numVolumes;
			this->SetProgressText( progressStream.str().c_str() );
			this->UpdateProgress( vol/((double)this->numVolumes) );
        }

    }
}


/*!
 *  Returns the pixel type. Either unsinged int or float.
 */
svkDcmHeader::DcmPixelDataFormat svkDcmSegmentationVolumeReader::GetFileType()
{
    return svkDcmHeader::UNSIGNED_INT_1;
}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called before ExecuteData()
 */
void svkDcmSegmentationVolumeReader::ExecuteInformation()
{
    Superclass::ExecuteInformation();

    //  Now set fields require for MRI: 
    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    hdr->SetSOPClassUID( svkDcmHeader::ENHANCED_MR_IMAGE );   
    hdr->SetValue( "PatientPosition", "UNKNOWN" );

    //  kludgy, but since we are converting to MRI internal representation some of these are required: 
    string repTime = "0"; 
    string echoTime = "0"; 
    hdr->InitMREchoMacro( 0 ); 
    hdr->InitMRTimingAndRelatedParametersMacro( 0, 0, 0); 

    if ( hdr->ElementExists("SliceThickness", "PixelMeasuresSequence") == false ) { 
        //cout << "Kludge for non-standard Segmentation Objects from Brainlab. Supposed to get fixed at some point" << endl;
        double pixelSpacing[3];
        hdr->GetPixelSpacing( pixelSpacing );
        //cout << "SliceThickness: " << pixelSpacing[2]  << endl;
        hdr->AddSequenceItemElement(
            "PixelMeasuresSequence",
            0,
            "SliceThickness",
            pixelSpacing[2],
            "SharedFunctionalGroupsSequence",
            0
        );

    }


    hdr->PrintDcmHeader();   
}


/*!
 *  Define the output data type. This is used to initialize the output object.
 */
int svkDcmSegmentationVolumeReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData");
    return 1;
}
