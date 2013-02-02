/*
 *  Copyright © 2009-2012 The Regents of the University of California.
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
#include <vtkObjectFactory.h>
#include <vtkDebugLeaks.h>
#include <vtkInformation.h>

#include <svkMriImageData.h>


using namespace svk;


vtkCxxRevisionMacro(svkDcmEnhancedVolumeReader, "$Rev$");
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
}


/*!
 *
 */
svkDcmEnhancedVolumeReader::~svkDcmEnhancedVolumeReader()
{
    vtkDebugMacro(<<this->GetClassName() << "::~" << this->GetClassName() << "()");
}


/*!
 *  Mandatory, must be overridden to get the factory to check for proper
 *  type of vtkImageReader2 to return. 
 */
int svkDcmEnhancedVolumeReader::CanReadFile(const char* fname)
{

    vtkstd::string fileToCheck(fname);

    if ( svkDcmHeader::IsFileDICOM( fname ) ) {
 
        svkImageData* tmp = svkMriImageData::New(); 
        tmp->GetDcmHeader()->ReadDcmFile( fname ); 
        vtkstd::string SOPClassUID = tmp->GetDcmHeader()->GetStringValue( "SOPClassUID" ) ; 
        tmp->Delete(); 

        if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.4.1" ) {

            cout << this->GetClassName() << "::CanReadFile(): It's a DICOM Enhanced File: " <<  fileToCheck << endl;

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
    int rows = svkDcmHeader::GetDimensionValue( &dimensionVector, svkDcmHeader::ROW_INDEX) + 1;
    int columns = svkDcmHeader::GetDimensionValue( &dimensionVector, svkDcmHeader::COL_INDEX) + 1;
    int numSlices = svkDcmHeader::GetDimensionValue( &dimensionVector, svkDcmHeader::SLICE_INDEX) + 1;
    int numFrames = this->GetOutput()->GetDcmHeader()->GetIntValue( "NumberOfFrames" );
    this->numVolumes = numFrames / numSlices;

    int numPixelsInSlice   = rows * columns;
    int dataWordSize       = 2; // Size of data stored in Enhanced Object, always short
    int numPixelsInVolume  = numPixelsInSlice * numSlices;
    int numBytesPerVol     = numPixelsInVolume * dataWordSize;

    // Get the Pixel Data from the DICOM header.
	void* imageData = (void* ) malloc( numBytesPerVol*this->numVolumes );
	this->GetOutput()->GetDcmHeader()->GetShortValue( "PixelData", ((short *)imageData), numPixelsInVolume*this->numVolumes );

	// If necessary convert the pixel data to doubles...
	if( this->IsDataFloatingPoint( this->GetOutput())  ) {
		int doubleWordSize = 8;
		void* imageDoubleData = (void* ) malloc( numPixelsInVolume * this->numVolumes * doubleWordSize );
		double intercept = 0;
		double slope = 1;
		this->GetPixelTransform( intercept, slope, this->GetOutput()->GetDcmHeader());
		this->GetRescaledPixels((double*)imageDoubleData,
					                    (unsigned short*)imageData ,
					                    intercept, slope, numPixelsInVolume * this->numVolumes);
		delete (unsigned short*)imageData;
		imageData = imageDoubleData;
	}

	// Create vtk Point Data arrays for each volume.
    for (int vol = 0; vol < this->numVolumes; vol++) {

        vtkDataArray* array = NULL;
        if( this->IsDataFloatingPoint( this->GetOutput())  ) {
        	array = vtkDoubleArray::New();
        } else {
			array = vtkUnsignedShortArray::New();
        }

		array->SetNumberOfComponents(1);
		array->SetNumberOfTuples(rows*columns*numSlices);

        ostringstream volNumber;
        volNumber << vol;
        vtkstd::string arrayNameString("pixels");
        arrayNameString.append(volNumber.str());
        array->SetName( arrayNameString.c_str() );

        if( this->IsDataFloatingPoint( this->GetOutput()) ) {
			array->SetVoidArray( (double*)(imageData) + vol*numPixelsInVolume , numPixelsInVolume, 0);
        } else {
			array->SetVoidArray( (short*)(imageData) + vol*numPixelsInVolume , numPixelsInVolume, 0);
        }
        if (vol == 0 ) {
            data->GetPointData()->SetScalars(array);
        } else {
            data->GetPointData()->AddArray(array);
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
    string interceptString = this->GetOutput()->GetDcmHeader()->GetStringSequenceItemElement ( "PixelValueTransformationSequence", 0, "RescaleIntercept", "SharedFunctionalGroupsSequence" );
    string slopeString = this->GetOutput()->GetDcmHeader()->GetStringSequenceItemElement ( "PixelValueTransformationSequence", 0, "RescaleSlope", "SharedFunctionalGroupsSequence" );
	intercept = svkUtils::StringToDouble( interceptString );
	slope = svkUtils::StringToDouble( slopeString );
}


/*!
 *  Check if data is actually floating point data. The data is floating point unless the
 *  RescaleIntercept is 0 and the RescaleSlope is 1.
 */
bool svkDcmEnhancedVolumeReader::IsDataFloatingPoint(svkImageData* image)
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
 * Rescales the short data to floating point data using an intercept and slope.
 */
void svkDcmEnhancedVolumeReader::GetRescaledPixels(double* doublePixels, unsigned short* shortPixels, double intercept, double slope, int numberOfValues )
{
	for( int i = 0; i < numberOfValues; i++ ) {
		doublePixels[i] = slope*shortPixels[i] + intercept;
	}
}


/*!
 *  Returns the pixel type. Either unsinged int or float.
 */
svkDcmHeader::DcmPixelDataFormat svkDcmEnhancedVolumeReader::GetFileType()
{
	if( this->IsDataFloatingPoint( this->GetOutput()) ) {
		return svkDcmHeader::SIGNED_FLOAT_8;
    } else {
		return svkDcmHeader::UNSIGNED_INT_2;
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
