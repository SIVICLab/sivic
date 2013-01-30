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

}


/*! 
 * DOES THIS NEED TO SUPPORT FLOATING POINT DATA???
 *  
 */
void svkDcmEnhancedVolumeReader::LoadData( svkImageData* data )
{
    svkDcmHeader::DimensionVector dimensionVector = this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector();
    int rows = svkDcmHeader::GetDimensionValue( &dimensionVector, svkDcmHeader::ROW_INDEX) + 1;
    int columns = svkDcmHeader::GetDimensionValue( &dimensionVector, svkDcmHeader::COL_INDEX) + 1;
    int numSlices = svkDcmHeader::GetDimensionValue( &dimensionVector, svkDcmHeader::SLICE_INDEX) + 1;
    int numFrames = this->GetOutput()->GetDcmHeader()->GetIntValue( "NumberOfFrames" );
    this->numVolumes = numFrames / numSlices;
    svkDcmHeader::PrintDimensionIndexVector( &dimensionVector );

    int numPixelsInSlice   = rows * columns;
    int dataWordSize       = 2;
    int numPixelsInVolume  = numPixelsInSlice * numSlices;
    int numBytesPerVol     = numPixelsInVolume * dataWordSize;

    /*
     *  Iterate over slices (frames) and copy ImagePositions
     */
	void* imageData = (void* ) malloc( numBytesPerVol*this->numVolumes );
	this->GetOutput()->GetDcmHeader()->GetShortValue( "PixelData", ((short *)imageData), numPixelsInVolume*this->numVolumes );
    for (int vol = 0; vol < this->numVolumes; vol++) {

        vtkDataArray* array = NULL;
		array = vtkUnsignedShortArray::New();
		array->SetNumberOfComponents(1);
		array->SetNumberOfTuples(rows*columns*numSlices);

        ostringstream volNumber;
        volNumber << vol;
        vtkstd::string arrayNameString("pixels");
        arrayNameString.append(volNumber.str());
        array->SetName( arrayNameString.c_str() );

        array->SetVoidArray( (short*)(imageData) + vol*numPixelsInVolume , numPixelsInVolume, 0);
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
 *  Returns the pixel type
 */
svkDcmHeader::DcmPixelDataFormat svkDcmEnhancedVolumeReader::GetFileType()
{
    //string voiLUTFunction  = this->GetOutput()->GetDcmHeader()->GetStringValue("VOILUTFunction" );
    //if( voiLUTFunction.compare("LINEAR") == 0 ) {
	//	return svkDcmHeader::SIGNED_FLOAT_4;
    //} else {
	return svkDcmHeader::UNSIGNED_INT_2;
	//}
}


/*!
 *
 */
int svkDcmEnhancedVolumeReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData");
    return 1;
}
