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


#include <svkDcmMriVolumeReader.h>
#include <svkMriImageData.h>
#include </usr/include/vtk/vtkType.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkStringArray.h>
#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkMath.h>
#include <svkIOD.h>
#include <svkEnhancedMRIIOD.h>
#include <vector>
#include <utility>
#include <sstream>


using namespace svk;


//vtkCxxRevisionMacro(svkDcmMriVolumeReader, "$Rev$");
vtkStandardNewMacro(svkDcmMriVolumeReader);


/*!
 *
 */
svkDcmMriVolumeReader::svkDcmMriVolumeReader()
{
#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkDcmMriVolumeReader");
#endif
    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");
}


/*!
 *
 */
svkDcmMriVolumeReader::~svkDcmMriVolumeReader()
{
    vtkDebugMacro(<<this->GetClassName() << "::~" << this->GetClassName() << "()");
}


/*!
 *  Mandator, must be overridden to get the factory to check for proper
 *  type of vtkImageReader2 to return. 
 */
int svkDcmMriVolumeReader::CanReadFile(const char* fname)
{

    string fileToCheck(fname);

    bool isDcmMri = false; 

    if ( svkDcmHeader::IsFileDICOM( fname ) ) {

        svkImageData* tmp = svkMriImageData::New(); 

        tmp->GetDcmHeader()->ReadDcmFileHeaderOnly( fname );
        string SOPClassUID = tmp->GetDcmHeader()->GetStringValue( "SOPClassUID" ) ;

        //verify that this isn't a proprietary use of DICOM MR ImageStorage: 
        if ( this->ContainsProprietaryContent( tmp ) == svkDcmVolumeReader::DICOM_STD_SOP ) {
                    
            // Check for MR Image Storage (and for now CTImageStorage and PETImageStorage too )
            if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.4" 
                || SOPClassUID == "1.2.840.10008.5.1.4.1.1.2" 
            ) {
                this->SetFileName(fname);
                isDcmMri = true; 
            }
        
        }

        tmp->Delete(); 
    }

    if ( isDcmMri ) {
        vtkDebugMacro(<<this->GetClassName() << "::CanReadFile(): It's a DICOM MRI File: " <<  fileToCheck );
        return 1;
    }

    vtkDebugMacro(<<this->GetClassName() << "::CanReadFile() It's Not a DICOM MRI file " << fileToCheck );

    return 0;
}


/*!
 *
 */
void svkDcmMriVolumeReader::InitDcmHeader()
{

    this->InitFileNames(); 

    // Read the first file and load the header as the starting point
    this->GetOutput()->GetDcmHeader()->ReadDcmFile( this->GetFileNames()->GetValue(0) );

    string studyInstanceUID( this->GetOutput()->GetDcmHeader()->GetStringValue("StudyInstanceUID"));
    int rows    = this->GetOutput()->GetDcmHeader()->GetIntValue( "Rows" ); // Y
    int columns = this->GetOutput()->GetDcmHeader()->GetIntValue( "Columns" ); // X
     
    //  Now override elements with Multi-Frame sequences and default details:
    svkIOD* iod = svkEnhancedMRIIOD::New();
    iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    iod->SetReplaceOldElements(false); 
    iod->InitDcmHeader();
    iod->Delete();

    this->GetOutput()->GetDcmHeader()->SetValue( "StudyInstanceUID", studyInstanceUID.c_str() );


    //  Now move info from original MRImageStorage header elements to flesh out enhanced
    //  SOP class elements (often this is just a matter of copying elements from the top 
    //  level to a sequence item. 
    this->InitMultiFrameFunctionalGroupsModule(); 

    /*
     *  odds and ends: 
     */
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "Rows", 
        rows 
    );
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "Columns", 
        columns 
    );

    if (this->GetDebug()) {
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader(); 
    }
    
}


/*!
 *  Initializes any private DICOM attributes that are needed internally
 */
void svkDcmMriVolumeReader::InitPrivateHeader()
{
}


/*! 
 *  Read the data for each volume into a separate point array in the vtkImageData object: 
 *  file names are in the following order: 
 *      fileNum slice   vol
 *      0       0       0   
 *      1       0       1   
 *      2       0       2   
 *      3       1       0   
 *      4       1       1   
 *      5       1       2   
 *          
 */
void svkDcmMriVolumeReader::LoadData( svkImageData* data )
{

    int rows               = this->GetOutput()->GetDcmHeader()->GetIntValue( "Rows" );
    int columns            = this->GetOutput()->GetDcmHeader()->GetIntValue( "Columns" );
    int numSlices          = this->GetFileNames()->GetNumberOfValues() / this->numVolumes;
    int numPixelsInSlice   = rows * columns;
    int dataWordSize       = 2;

    int numBytesPerVol     = numPixelsInSlice * numSlices * dataWordSize;
    string voiLUTFunction  = this->GetOutput()->GetDcmHeader()->GetStringValue("VOILUTFunction" );

    bool isVOILUTDefined = false;
    if( voiLUTFunction.compare("LINEAR") == 0 ) {
    	isVOILUTDefined = true;
    }


    /*
     *  Iterate over slices (frames) and copy ImagePositions
     */
    for (int vol = 0; vol < this->numVolumes; vol++) { 

        void* imageData = (void* ) malloc( numBytesPerVol );
        vtkDataArray* array = NULL;
        if( isVOILUTDefined ) {
        	array = vtkFloatArray::New();
        } else {
        	array = vtkUnsignedShortArray::New();
        }

        ostringstream volNumber;
        volNumber << vol;
        string arrayNameString("pixels");
        arrayNameString.append(volNumber.str());
        array->SetName( arrayNameString.c_str() );

        for (int slice = 0; slice < numSlices; slice++ ) {
            svkImageData* tmpImage = svkMriImageData::New(); 
            tmpImage->GetDcmHeader()->ReadDcmFile( this->GetFileNames()->GetValue( vol + slice * this->numVolumes ) ); 
            tmpImage->GetDcmHeader()->GetShortValue( "PixelData", ((short *)imageData) + (slice * numPixelsInSlice), numPixelsInSlice );
            tmpImage->Delete(); 
        }

        if( isVOILUTDefined ) {
        	int floatWordSize = 4;
			void* imageFloatData = (void* ) malloc( numPixelsInSlice * numSlices * floatWordSize );
            svkImageData* tmpImage = svkMriImageData::New(); 
            tmpImage->GetDcmHeader()->ReadDcmFileHeaderOnly( this->GetFileNames()->GetValue( vol ) );
			double voiWindowCenter = tmpImage->GetDcmHeader()->GetDoubleValue("WindowCenter" );
			double voiWindowWidth  = tmpImage->GetDcmHeader()->GetDoubleValue("WindowWidth" );
			this->GetVOILUTScaledPixels((float*)imageFloatData,
					                    (unsigned short*)imageData ,
					                    voiWindowCenter, voiWindowWidth, numSlices * numPixelsInSlice);
			delete (unsigned short*)imageData;
			imageData = imageFloatData;
			// The image has now been scaled so we can remove the scaling tags
			this->GetOutput()->GetDcmHeader()->RemoveElement("VOILUTFunction" );
			this->GetOutput()->GetDcmHeader()->RemoveElement("WindowCenter" );
			this->GetOutput()->GetDcmHeader()->RemoveElement("WindowWidth" );
            tmpImage->Delete();
        }

        array->SetVoidArray( (void*)(imageData), numPixelsInSlice * numSlices, 0);
        if (vol == 0 ) {
            data->GetPointData()->SetScalars(array);
        } else {
            data->GetPointData()->AddArray(array);
        }

        // This array has been attached to the point data so lets release our local reference.
        array->Delete();

        if( vol % 2 == 0 ) { // update progress every other volume
			ostringstream progressStream;
			progressStream <<"Reading Volume " << vol << " of " << this->numVolumes;
			this->SetProgressText( progressStream.str().c_str() );
			this->UpdateProgress( vol/((double)this->numVolumes) );
        }

    }

}


/*!
 *
 */
void svkDcmMriVolumeReader::InitMultiFrameFunctionalGroupsModule()
{

    this->numFrames =  this->GetFileNames()->GetNumberOfValues(); 

    this->InitSharedFunctionalGroupMacros();
    this->InitPerFrameFunctionalGroupMacros();

}


/*!
 *
 */
void svkDcmMriVolumeReader::InitSharedFunctionalGroupMacros()
{
    this->InitPixelMeasuresMacro();
    this->InitPlaneOrientationMacro();
    this->InitMRReceiveCoilMacro();
}


/*!
 *  Pixel Spacing:
 */
void svkDcmMriVolumeReader::InitPixelMeasuresMacro()
{

    svkImageData* tmp = svkMriImageData::New(); 
    tmp->GetDcmHeader()->ReadDcmFileHeaderOnly(  this->GetFileNames()->GetValue( 0 ) );

    this->GetOutput()->GetDcmHeader()->InitPixelMeasuresMacro(
        tmp->GetDcmHeader()->GetStringValue( "PixelSpacing" ), 
        tmp->GetDcmHeader()->GetStringValue( "SliceThickness" ) 
    );

    tmp->Delete(); 
}


/*!
 *
 */
void svkDcmMriVolumeReader::InitPlaneOrientationMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PlaneOrientationSequence"
    );

    svkImageData* tmp = svkMriImageData::New(); 
    tmp->GetDcmHeader()->ReadDcmFileHeaderOnly(  this->GetFileNames()->GetValue( 0 ) );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PlaneOrientationSequence",
        0,
        "ImageOrientationPatient",
        tmp->GetDcmHeader()->GetStringValue( "ImageOrientationPatient" ), 
        "SharedFunctionalGroupsSequence",
        0
    );

    tmp->Delete(); 

}


/*!
 *  Receive Coil:
 */
void svkDcmMriVolumeReader::InitMRReceiveCoilMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRReceiveCoilSequence"
    );

    svkImageData* tmp = svkMriImageData::New(); 
    tmp->GetDcmHeader()->ReadDcmFileHeaderOnly(  this->GetFileNames()->GetValue( 0 ) );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "ReceiveCoilName",
        tmp->GetDcmHeader()->GetStringValue( "ReceiveCoilName" ), 
        "SharedFunctionalGroupsSequence",
        0
    );

    tmp->Delete(); 
}


/*!
 *
 */
void svkDcmMriVolumeReader::InitPerFrameFunctionalGroupMacros()
{

    int numSlices = this->numFrames / this->numVolumes;  

    svkImageData* tmpImage = svkMriImageData::New(); 
    tmpImage->GetDcmHeader()->ReadDcmFileHeaderOnly( this->GetFileNames()->GetValue( 0 ) );

    double toplc[3];
    for (int i = 0; i < 3; i++ ) {
        string pos( tmpImage->GetDcmHeader()->GetStringValue("ImagePositionPatient", i));
        std::istringstream positionInString(pos);
        positionInString >> toplc[i];
    }

    double pixelSize[3];
    for (int i = 0; i < 2; i++ ) {
        string pos( tmpImage->GetDcmHeader()->GetStringValue("PixelSpacing", i));
        std::istringstream positionInString(pos);
        positionInString >> pixelSize[i];
    }
    pixelSize[2] = tmpImage->GetDcmHeader()->GetFloatValue( "SliceThickness" ); 

    //  Get pixelSize from input series:
    pixelSize[2] = this->GetSliceSpacing();

    string fileStart = this->GetFileNames()->GetValue( 0 ); 
    string fileEnd = this->GetFileNames()->GetValue( this->numFrames - 1 ); 

    this->InitSliceOrder(fileStart, fileEnd) ; 
    tmpImage->GetDcmHeader()->SetSliceOrder( 
        this->dataSliceOrder
    );

    double dcos[3][3];
    tmpImage->GetDcmHeader()->GetDataDcos( dcos ); 

    svkDcmHeader::DimensionVector dimensionVector = this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector(); 
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, numSlices-1);
    this->GetOutput()->GetDcmHeader()->AddDimensionIndex(
            &dimensionVector, svkDcmHeader::TIME_INDEX, this->numVolumes-1);

    this->GetOutput()->GetDcmHeader()->InitPerFrameFunctionalGroupSequence(
                toplc,        
                pixelSize,  
                dcos,  
                &dimensionVector
    );
    tmpImage->Delete();

}


/*
 *  If true, then make sure only one data volume
 *  is present in file list
 */
bool svkDcmMriVolumeReader::CheckForMultiVolume() {
    return false;
}



/*!
 *  Returns the pixel type 
 */
svkDcmHeader::DcmPixelDataFormat svkDcmMriVolumeReader::GetFileType()
{
    string voiLUTFunction  = this->GetOutput()->GetDcmHeader()->GetStringValue("VOILUTFunction" );
    if( voiLUTFunction.compare("LINEAR") == 0 ) {
		return svkDcmHeader::SIGNED_FLOAT_4;
    } else {
		return svkDcmHeader::UNSIGNED_INT_2;
	}
}


/*!
 *
 */
int svkDcmMriVolumeReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData");
    return 1;
}

