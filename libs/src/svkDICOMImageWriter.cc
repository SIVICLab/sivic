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


#include <svkDICOMImageWriter.h>
#include </usr/include/vtk/vtkErrorCode.h>
#include </usr/include/vtk/vtkCellData.h>
#include </usr/include/vtk/vtkExecutive.h>
#include </usr/include/vtk/vtkImageAccumulate.h>
#include </usr/include/vtk/vtkDoubleArray.h>


using namespace svk;


//vtkCxxRevisionMacro(svkDICOMImageWriter, "$Rev$");


/*!
 *
 */
svkDICOMImageWriter::svkDICOMImageWriter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->useLosslessCompression = false; 
    this->SetFilePrefix("dcm"); 
}


/*!
 *
 */
svkDICOMImageWriter::~svkDICOMImageWriter()
{
}


/*!
 *
 */
int svkDICOMImageWriter::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(svkDICOMImageWriter::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
    return 1;
}


/*!
 *  vtkImageAccumulate doesn't seem to work for doubles, so need custom method for 
 *  calculating pixel value ranges. 
 */
void svkDICOMImageWriter::GetPixelRange(double& min, double& max, int volNumber)
{

    int vtkDataType = vtkImageData::GetScalarType( this->GetImageDataInput(0)->GetInformation() ); 
    int dataType = 
            this->GetImageDataInput(0)->GetDcmHeader()->GetPixelDataType( vtkDataType ); 
    int cols = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "Columns" );
    int rows = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "Rows" );
    int slices = (this->GetImageDataInput(0)->GetExtent() ) [5] - (this->GetImageDataInput(0)->GetExtent() ) [4] + 1;
    int numPixels = cols * rows * slices;

    if (dataType == svkDcmHeader::SIGNED_FLOAT_4) {

        min = VTK_FLOAT_MAX;
        max = VTK_FLOAT_MIN;
        float* floatPixels = static_cast<float *>(
                vtkFloatArray::SafeDownCast(this->GetImageDataInput(0)->GetPointData()->GetArray(volNumber))->GetPointer(0)
        );

        for (int i = 0; i < numPixels; i++ ) {
            if ( floatPixels[i] > max ) {
                max = floatPixels[i];
            }
            if ( floatPixels[i] < min ) {
                min = floatPixels[i];
            }
        }


    } else if (dataType == svkDcmHeader::SIGNED_FLOAT_8) {

        min = VTK_DOUBLE_MAX;
        max = VTK_DOUBLE_MIN;
        double* doublePixels = static_cast<double *>( 
                vtkDoubleArray::SafeDownCast(this->GetImageDataInput(0)->GetPointData()->GetArray(volNumber))->GetPointer(0)
        );

        for (int i = 0; i < numPixels; i++ ) {
            if ( doublePixels[i] > max ) {
                max = doublePixels[i];
            }
            if ( doublePixels[i] < min ) {
                min = doublePixels[i];
            }
        }
    }
}


/*!
 *  Performs a linear mapping of floating point image values to 16 bit integer dynamic range. 
 *  Returns a signed short array, together with the intercept and slope of the linear scaling 
 *  transformation ( shortVal = floatVal * slope + intercept).    
 */
void svkDICOMImageWriter::GetShortScaledPixels( unsigned short* shortPixels, double& slope, double& intercept, int sliceNumber, int volNumber)
{

    //  Get the input range for scaling:
    double inputRangeMin;
    double inputRangeMax;

    this->GetPixelRange(inputRangeMin, inputRangeMax, volNumber);
    double deltaRangeIn = inputRangeMax - inputRangeMin;

    //  Get the output range for scaling:
    int shortMin = VTK_UNSIGNED_SHORT_MIN; 
    int shortMax = VTK_UNSIGNED_SHORT_MAX; 
    double deltaRangeOut = shortMax - shortMin;

    //  apply linear mapping from float range to signed short range; 
    //  shortMax = inputRangeMax * m + b;
    //  shortMin = inputRangeMin * m + b; 
    slope = deltaRangeOut/deltaRangeIn; 
    intercept = shortMin - inputRangeMin * ( deltaRangeOut/deltaRangeIn ); 
    this->GetScaledPixels( shortPixels, slope, intercept, sliceNumber, volNumber);
}


/*!
 *  Performs a linear mapping of floating point image values to 16 bit integer dynamic range.
 *  Returns a signed short array, together with the intercept and slope of the linear scaling
 *  transformation ( shortVal = floatVal * slope + intercept).
 */
void svkDICOMImageWriter::GetScaledPixels( unsigned short* shortPixels, double slope, double intercept, int sliceNumber, int volNumber)
{
    if (this->GetDebug()) {
        cout << "DICOM MRI Writer float to int scaling (slope, intercept): " << slope << " " << intercept << endl;     
    }
    
    int dataLength = this->GetDataLength();
    int offset = 0; 
    if (sliceNumber >= 0 ) {
        offset = dataLength * sliceNumber; 
    }

    int vtkDataType = vtkImageData::GetScalarType( this->GetImageDataInput(0)->GetInformation() ); 
    int dataType = 
            this->GetImageDataInput(0)->GetDcmHeader()->GetPixelDataType( vtkDataType ); 

    if (dataType == svkDcmHeader::SIGNED_FLOAT_4) {

        float* floatPixels = static_cast<float *>( 
            static_cast<vtkFloatArray*>(this->GetImageDataInput(0)->GetPointData()->GetArray(volNumber))->GetPointer(0)
        );

        for (int i = 0; i < dataLength; i++) {
            shortPixels[i] = static_cast<unsigned short> (svkUtils::NearestInt( slope * floatPixels[offset + i] + intercept ) );
        }
    } else if (dataType == svkDcmHeader::SIGNED_FLOAT_8) {
        double* doublePixels = static_cast<double *>( 
            vtkDoubleArray::SafeDownCast(this->GetImageDataInput(0)->GetPointData()->GetArray(volNumber))->GetPointer(0)
        );
        for (int i = 0; i < dataLength; i++) {
            shortPixels[i] = static_cast<unsigned short> (svkUtils::NearestInt( slope * doublePixels[offset + i] + intercept ) );
        }
    }

}


/*!
 *  Use lossless compression transfer syntax. 
 */
void svkDICOMImageWriter::UseLosslessCompression() 
{
    this->useLosslessCompression = true; 
}

