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


#include <svkDICOMImageWriter.h>
#include <vtkErrorCode.h>
#include <vtkCellData.h>
#include <vtkExecutive.h>
#include <vtkImageAccumulate.h>


using namespace svk;


vtkCxxRevisionMacro(svkDICOMImageWriter, "$Rev$");


/*!
 *
 */
svkDICOMImageWriter::svkDICOMImageWriter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );
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
 *
 */
void svkDICOMImageWriter::SetInput( vtkDataObject* input )
{
    this->SetInput(0, input);
}


/*!
 *
 */
void svkDICOMImageWriter::SetInput(int index, vtkDataObject* input)
{
    if(input) {
        this->SetInputConnection(index, input->GetProducerPort());
    } else {
        // Setting a NULL input removes the connection.
        this->SetInputConnection(index, 0);
    }
}


/*!
 *
 */
vtkDataObject* svkDICOMImageWriter::GetInput(int port)
{
    return this->GetExecutive()->GetInputData(port, 0);
}


/*!
 *
 */
svkImageData* svkDICOMImageWriter::GetImageDataInput(int port)
{
    return svkImageData::SafeDownCast( this->GetInput(port) );
}


/*!
 *  Write the pixel data to the PixelData DICOM element.       
 *  if a slice number (starting at 0 index) is specified, will 
 *  init only that block of PixelData in the DCM file.  
 * 
 */
void svkDICOMImageWriter::InitPixelData( svkDcmHeader* dcmHeader, int sliceNumber )
{

    vtkDebugMacro( << this->GetClassName() << "::InitPixelData()" );

    int dataLength = this->GetDataLength();
    int offset = 0; 
    if (sliceNumber >= 0 ) {
        offset = dataLength * sliceNumber; 
    }

    switch ( this->GetImageDataInput(0)->GetDcmHeader()->GetPixelDataType( this->GetImageDataInput(0)->GetScalarType() ) ) {

        case svkDcmHeader::UNSIGNED_INT_1:
        {
            // Dicom does not support the writing of 8 bit words so we will convert to 16
            unsigned char *pixelData = (unsigned char *)this->GetImageDataInput(0)->GetScalarPointer();
            unsigned short* shortPixelData = new unsigned short[dataLength];
            for (int i = 0; i < dataLength; i++) {
                shortPixelData[i] = pixelData[offset + i];
            }
            dcmHeader->SetPixelDataType( svkDcmHeader::UNSIGNED_INT_2 );
            dcmHeader->SetValue(
                  "PixelData",
                  shortPixelData,
                  dataLength 
            );
            delete[] shortPixelData;
        }
        break;

        case svkDcmHeader::UNSIGNED_INT_2:
        {
            unsigned short *pixelData = static_cast<unsigned short *>( this->GetImageDataInput(0)->GetScalarPointer() );
            dcmHeader->SetValue(
                  "PixelData",
                  &(pixelData[offset]), 
                  dataLength 
            );
        }
        break; 

        case svkDcmHeader::SIGNED_FLOAT_4:
        case svkDcmHeader::SIGNED_FLOAT_8:
        {
            //  Fix BitsAllocated, etc to be represent 
            //  signed short data
            dcmHeader->SetPixelDataType(svkDcmHeader::UNSIGNED_INT_2);

            unsigned short* pixelData = new unsigned short[dataLength];  
            float slope; 
            float intercept; 
            this->GetShortScaledPixels( pixelData, slope, intercept, sliceNumber ); 
            
            dcmHeader->SetValue(
                  "PixelData",
                  pixelData, 
                  dataLength 
            );

            //  Init Rescale Attributes:    
            //  For Enhanced MR Image Storage use the PixelValueTransformation Macro
            //  For MR Image Stroage use VOI LUT Module. 
            double inputRangeMin;
            double inputRangeMax;
            this->GetPixelRange(inputRangeMin, inputRangeMax);
            vtkstd::string SOPClassUID = dcmHeader->GetStringValue( "SOPClassUID" ) ;
            if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.4" ) {

                //  MR Image Storage: 
                //      convert slope and intercept to center and width:     
                //      Get the pixel value range (defines the WL of VOI LUT):
                double width = inputRangeMax - inputRangeMin;
                double center = (inputRangeMax + inputRangeMin)/2;
                dcmHeader->InitVOILUTModule( center, width ); 

            } else {

                //  Enhanced MR Image Storage: 
                //      slope and intercept are for real -> short, we need the 
                //      inverse transformation here that a downstream application:
                //      can use to regenerate the original values:
                float slopeReverse = 1/slope;  
                int shortMin = VTK_UNSIGNED_SHORT_MIN; 
                float interceptReverse = inputRangeMin - shortMin/slope;
                dcmHeader->InitPixelValueTransformationMacro( slopeReverse, interceptReverse ); 

            }

            delete[] pixelData; 
        }
        break;

        case svkDcmHeader::SIGNED_INT_2: 
        {
            short *pixelData = static_cast<short *>( this->GetImageDataInput(0)->GetScalarPointer() );
            dcmHeader->SetValue(
                  "PixelData",
                  &(pixelData[offset]), 
                  dataLength 
            );
        }
        default:
          vtkErrorMacro("Undefined or unsupported pixel data type");
    }
}


/*!
 *  vtkImageAccumulate doesn't seem to work for doubles, so need custom method for 
 *  calculating pixel value ranges. 
 */
void svkDICOMImageWriter::GetPixelRange(double& min, double& max)
{
    int dataType = 
            this->GetImageDataInput(0)->GetDcmHeader()->GetPixelDataType( this->GetImageDataInput(0)->GetScalarType() ); 

    if (dataType == svkDcmHeader::SIGNED_FLOAT_4) {

        vtkImageAccumulate* histo = vtkImageAccumulate::New();
        histo->SetInput( this->GetImageDataInput(0) );
        histo->Update();

        //  Get the input range for scaling:
        min = *( histo->GetMin() ); 
        max = *( histo->GetMax() ); 

        histo->Delete();

    } else if (dataType == svkDcmHeader::SIGNED_FLOAT_8) {

        int cols = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "Columns" ); 
        int rows = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "Rows" ); 
        int slices = (this->GetImageDataInput(0)->GetExtent() ) [5] - (this->GetImageDataInput(0)->GetExtent() ) [4] + 1;
        int numPixels = cols * rows * slices; 

        min = 0.;
        max = 0.;
        double* doublePixels = static_cast<double *>( this->GetImageDataInput(0)->GetScalarPointer() );
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
void svkDICOMImageWriter::GetShortScaledPixels( unsigned short* shortPixels, float& slope, float& intercept, int sliceNumber )
{

    //  Get the input range for scaling:
    double inputRangeMin;
    double inputRangeMax;

    this->GetPixelRange(inputRangeMin, inputRangeMax);
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
    if (this->GetDebug()) {
        cout << "DICOM MRI Writer float to int scaling (slope, intercept): " << slope << " " << intercept << endl;     
    }
    
    int dataLength = this->GetDataLength();
    int offset = 0; 
    if (sliceNumber >= 0 ) {
        offset = dataLength * sliceNumber; 
    }

    int dataType = 
            this->GetImageDataInput(0)->GetDcmHeader()->GetPixelDataType( this->GetImageDataInput(0)->GetScalarType() ); 

    if (dataType == svkDcmHeader::SIGNED_FLOAT_4) {
        float* floatPixels = static_cast<float *>( this->GetImageDataInput(0)->GetScalarPointer() );
        for (int i = 0; i < dataLength; i++) {
            shortPixels[i] = static_cast<unsigned short> ( slope * floatPixels[offset + i] + intercept ); 
        }
    } else if (dataType == svkDcmHeader::SIGNED_FLOAT_8) {
        double* doublePixels = static_cast<double *>( this->GetImageDataInput(0)->GetScalarPointer() );
        for (int i = 0; i < dataLength; i++) {
            shortPixels[i] = static_cast<unsigned short> ( slope * doublePixels[offset + i] + intercept ); 
        }
    }

}

