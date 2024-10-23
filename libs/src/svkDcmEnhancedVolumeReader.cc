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

#include <svkDcmEnhancedVolumeReader.h>
#include <svkMriImageData.h>
#include <svkTypeUtils.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkInformation.h>



using namespace svk;


//vtkCxxRevisionMacro(svkDcmEnhancedVolumeReader, "$Rev$");
vtkStandardNewMacro(svkDcmEnhancedVolumeReader);


/*!
 *
 */
svkDcmEnhancedVolumeReader::svkDcmEnhancedVolumeReader()
{
#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkDcmEnhancedVolumeReader");
#endif
    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");
    this->useDoublePrecision = false;
    this->rescalePixels = true;
}


/*!
 *
 */
svkDcmEnhancedVolumeReader::~svkDcmEnhancedVolumeReader()
{
    vtkDebugMacro(<<this->GetClassName() << "::~" << this->GetClassName() << "()");
}


/*! 
 * Set to true if the data should be interpreted as double precision.
 * This is unlikely BUT hypothetically the slope and intercept fields
 * allow for double precision. This precision is most likely artificial.
 */
void svkDcmEnhancedVolumeReader::UseDoublePrecision( bool useDoublePrecision )
{
    this->useDoublePrecision = useDoublePrecision;
}


/*! 
 * If set to true then rescale data to floating point when appropriate.
 */
void svkDcmEnhancedVolumeReader::SetRescalePixels( bool rescalePixels )
{
    this->rescalePixels = rescalePixels;
}


/*!
 *  Mandatory, must be overridden to get the factory to check for proper
 *  type of vtkImageReader2 to return. 
 */
int svkDcmEnhancedVolumeReader::CanReadFile(const char* fname)
{

    string fileToCheck(fname);

    if ( svkDcmHeader::IsFileDICOM( fname ) ) {
 
        svkImageData* tmp = svkMriImageData::New(); 
        tmp->GetDcmHeader()->ReadDcmFileHeaderOnly( fname );
        string SOPClassUID = tmp->GetDcmHeader()->GetStringValue( "SOPClassUID" ) ; 
        tmp->Delete(); 

        if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.4.1" ) {

            vtkDebugMacro(<<this->GetClassName() << "::CanReadFile(): It's a DICOM Enhanced file " << fileToCheck );

            this->SetFileName(fname);

            return 1;
        }
    } 

    vtkDebugMacro(<<this->GetClassName() << "::CanReadFile() It's Not a DICOM Enhanced file " << fileToCheck );

    return 0;
}


/*! 
 *  Initializes any private DICOM attributes that are needed internally
 */
void svkDcmEnhancedVolumeReader::InitPrivateHeader()
{
	// Required by parent class, not sure if we need anything here.
}


/*! 
 *  Loads the data. If the data header contains a RescaleSlope and Rescale Intercept
 *  That are not 1 and 0 respectively then the data will be loaded into doubles and
 *  rescaled.
 */
void svkDcmEnhancedVolumeReader::LoadData( svkImageData* data )
{
    svkDcmHeader::DimensionVector dimensionVector = this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector();
    int rows = svkDcmHeader::GetDimensionVectorValue( &dimensionVector, svkDcmHeader::ROW_INDEX) + 1;
    int columns = svkDcmHeader::GetDimensionVectorValue( &dimensionVector, svkDcmHeader::COL_INDEX) + 1;
    int numSlices = svkDcmHeader::GetDimensionVectorValue( &dimensionVector, svkDcmHeader::SLICE_INDEX) + 1;
    int numFrames = this->GetOutput()->GetDcmHeader()->GetIntValue( "NumberOfFrames" );
    this->numVolumes = numFrames / numSlices;

    int numPixelsInSlice   = rows * columns;
    int dataWordSize       = 2; // Size of data stored in Enhanced Object, always short
    int numPixelsInVolume  = numPixelsInSlice * numSlices;
    int numBytesPerVol     = numPixelsInVolume * dataWordSize;

    // Get the Pixel Data from the DICOM header.
	void* imageData = NULL;
    if( this->numVolumes == 1 && !this->ArePixelsScaled( ) ) {
        imageData = this->GetOutput()->GetPointData()->GetScalars()->GetVoidPointer(0);
    } else {
	    imageData = (void*) malloc( numBytesPerVol*this->numVolumes );
    }
	this->GetOutput()->GetDcmHeader()->GetShortValue( "PixelData", ((short *)imageData), numPixelsInVolume*this->numVolumes );

    // Lets go ahead and free the short data now that we have a copy of it.
    this->GetOutput()->GetDcmHeader()->ClearElement("PixelData");

	// If necessary convert the pixel data to floating point...
    if( this->ArePixelsScaled( )) {
        int wordSize = 4;
        if( this->useDoublePrecision ) {
            wordSize = 8;
        }
       
        void* imageFloatData = NULL; 
        if( this->numVolumes == 1 ) {
            imageFloatData = this->GetOutput()->GetPointData()->GetScalars()->GetVoidPointer(0);
        } else {
            imageFloatData = (void* ) malloc( numPixelsInVolume * this->numVolumes * wordSize );
        }
		double intercept = 0;
		double slope = 1;
		this->GetPixelTransform( intercept, slope, this->GetOutput()->GetDcmHeader());
        if( this->useDoublePrecision ) {
            this->GetRescaledPixels((double*)imageFloatData,
					                    (unsigned short*)imageData ,
					                    intercept, slope, numPixelsInVolume * this->numVolumes);
        } else {
            this->GetRescaledPixels((float*)imageFloatData,
					                    (unsigned short*)imageData ,
					                    intercept, slope, numPixelsInVolume * this->numVolumes);
        }
        free (imageData);
		imageData = imageFloatData;
	}

	// Create vtk Point Data arrays for each volume.
    for (int vol = 0; vol < this->numVolumes; vol++) {

        vtkDataArray* array = NULL;
        if( this->numVolumes > 1 ) {
            if( this->ArePixelsScaled( )  ) {
                if( this->useDoublePrecision ) {
                    array = vtkDoubleArray::New();
                } else {
                    array = vtkFloatArray::New();
                }
            } else {
                array = vtkUnsignedShortArray::New();
            }

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
            if( this->ArePixelsScaled( ) ) {
                if( this->useDoublePrecision ) {
                    array->SetVoidArray( (double*)(imageData) + vol*numPixelsInVolume , numPixelsInVolume, 0);
                } else {
                    array->SetVoidArray( (float*)(imageData) + vol*numPixelsInVolume , numPixelsInVolume, 0);
                }
            } else {
                array->SetVoidArray( (short*)(imageData) + vol*numPixelsInVolume , numPixelsInVolume, 0);
            }
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
 * Get the slope and intercept from the PixelValueTransformationSequence in the DICOM Header.
 */
void svkDcmEnhancedVolumeReader::GetPixelTransform(double& intercept, double& slope, svkDcmHeader* header)
{
    intercept = 0;
    slope = 1;

    string parentSequence;
    if( this->GetOutput()->GetDcmHeader()->ElementExists( 
            "PixelValueTransformationSequence", "PerFrameFunctionalGroupsSequence" ) ) {
        parentSequence = "PerFrameFunctionalGroupsSequence";
    } else if( this->GetOutput()->GetDcmHeader()->ElementExists( 
            "PixelValueTransformationSequence", "SharedFunctionalGroupsSequence" ) ) {
        parentSequence = "SharedFunctionalGroupsSequence";
    }



    string interceptString; 
    if ( this->GetOutput()->GetDcmHeader()->ElementExists( "RescaleIntercept", "PixelValueTransformationSequence") == true ) {
        interceptString = this->GetOutput()->GetDcmHeader()->GetStringSequenceItemElement ( 
            "PixelValueTransformationSequence", 0, "RescaleIntercept", parentSequence.c_str() ); 
	    intercept = svkTypeUtils::StringToDouble( interceptString );
    }

    string slopeString;
    if ( this->GetOutput()->GetDcmHeader()->ElementExists( "RescaleSlope", "PixelValueTransformationSequence") == true ) {
        slopeString = this->GetOutput()->GetDcmHeader()->GetStringSequenceItemElement ( 
            "PixelValueTransformationSequence", 0, "RescaleSlope", parentSequence.c_str() ); 
	    slope = svkTypeUtils::StringToDouble( slopeString );
    }
}


/*!
 *  Check if data is actually floating point data. The data is floating point unless the
 *  RescaleIntercept is 0 and the RescaleSlope is 1.
 */
bool svkDcmEnhancedVolumeReader::IsDataFloatingPoint( )
{
	bool isDataFloats = true;
	double intercept = 0;
	double slope = 1;
	this->GetPixelTransform( intercept, slope, this->GetOutput()->GetDcmHeader());
	if( intercept == 0 && slope == 1 ) {
		isDataFloats = false;
	}
	return isDataFloats;
}


/*!
 *  Check if the data needs to be rescaled.
 */
bool svkDcmEnhancedVolumeReader::ArePixelsScaled( )
{
    return (this->IsDataFloatingPoint() && this->rescalePixels);
}


/*!
 * Rescales the short data to floating point data using an intercept and slope.
 */
void svkDcmEnhancedVolumeReader::GetRescaledPixels(double* doublePixels, unsigned short* shortPixels, double intercept, double slope, int numberOfValues )
{
	for( int i = 0; i < numberOfValues; i++ ) {
		doublePixels[i] = slope*shortPixels[i] + intercept;
	}
}


/*!
 * Rescales the short data to floating point data using an intercept and slope.
 */
void svkDcmEnhancedVolumeReader::GetRescaledPixels(float* floatPixels, unsigned short* shortPixels, double intercept, double slope, int numberOfValues )
{
	for( int i = 0; i < numberOfValues; i++ ) {
		floatPixels[i] = slope*shortPixels[i] + intercept;
	}
}


/*!
 *  Returns the pixel type. Either unsinged int or float.
 */
svkDcmHeader::DcmPixelDataFormat svkDcmEnhancedVolumeReader::GetFileType()
{
	if( this->ArePixelsScaled( ) ) {
        if ( this->useDoublePrecision ) {
            return svkDcmHeader::SIGNED_FLOAT_8;
        } else {
            return svkDcmHeader::SIGNED_FLOAT_4;
        }
    } else {
		return svkDcmHeader::UNSIGNED_INT_2;
	}
}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called before ExecuteData()
 */
void svkDcmEnhancedVolumeReader::ExecuteInformation()
{
    Superclass::ExecuteInformation();
    if( this->ArePixelsScaled( )  ) {
        int vtkDataType; 
        if( this->useDoublePrecision ) {
            vtkDataType = VTK_DOUBLE; 
        } else {
            vtkDataType = VTK_FLOAT; 
        }
        vtkDataObject::SetPointDataActiveScalarInfo(
            this->GetOutput()->GetInformation(),
            vtkDataType,
            this->GetOutput()->GetNumberOfScalarComponents()
        );

    } 
}


/*!
 *  Define the output data type. This is used to initialize the output object.
 */
int svkDcmEnhancedVolumeReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData");
    return 1;
}
