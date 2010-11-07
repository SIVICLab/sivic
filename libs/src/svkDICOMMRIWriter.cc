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


#include <svkDICOMMRIWriter.h>
#include <vtkErrorCode.h>
#include <vtkCellData.h>
#include <vtkExecutive.h>
#include <vtkImageAccumulate.h>


using namespace svk;


vtkCxxRevisionMacro(svkDICOMMRIWriter, "$Rev$");
vtkStandardNewMacro(svkDICOMMRIWriter);


/*!
 *
 */
svkDICOMMRIWriter::svkDICOMMRIWriter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );


}


/*!
 *
 */
svkDICOMMRIWriter::~svkDICOMMRIWriter()
{
}



/*!
 *  Write the DICOM Enahnced MR Image multi-frame file.   Also initializes the 
 */
void svkDICOMMRIWriter::Write()
{

    this->SetErrorCode(vtkErrorCode::NoError);

    if (! this->FileName ) {
        vtkErrorMacro(<<"Write:Please specify either a FileName or a file prefix and pattern");
        this->SetErrorCode(vtkErrorCode::NoFileNameError);
        return;
    }

    // Make sure the file name is allocated
    this->InternalFileName =
        new char[(this->FileName ? strlen(this->FileName) : 1) +
        (this->FilePrefix ? strlen(this->FilePrefix) : 1) +
        (this->FilePattern ? strlen(this->FilePattern) : 1) + 10];

    this->FileNumber = 0;
    this->MinimumFileNumber = this->FileNumber;
    this->FilesDeleted = 0;
    this->UpdateProgress(0.0);

    this->MaximumFileNumber = this->FileNumber;

    // determine the name
    if (this->FileName) {
        sprintf(this->InternalFileName,"%s",this->FileName);
    } else {
        if (this->FilePrefix) {
            sprintf(this->InternalFileName, this->FilePattern,
                    this->FilePrefix, this->FileNumber);
        } else {
            sprintf(this->InternalFileName, this->FilePattern,this->FileNumber);
        }
    }

    this->iod = svkMRIIOD::New(); 
    this->iod->SetDcmHeader( this->GetImageDataInput(0)->GetDcmHeader() );

    this->InitPixelData();

    this->GetImageDataInput(0)->GetDcmHeader()->WriteDcmFile(this->InternalFileName); 

    if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError) {
        vtkErrorMacro("Ran out of disk space; deleting file(s) already written");
        this->DeleteFiles();
        return;
    }

    delete [] this->InternalFileName;
    this->InternalFileName = NULL;
    this->iod->Delete();

    //  Clear the PixelData element: 
    //this->GetImageDataInput(0)->GetDcmHeader()->ClearElement( "PixelData" ); 
}


/*!
 *
 */
int svkDICOMMRIWriter::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(svkDICOMMRIWriter::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
    return 1;
}


/*!
 *
 */
void svkDICOMMRIWriter::SetInput( vtkDataObject* input )
{
    this->SetInput(0, input);
}


/*!
 *
 */
void svkDICOMMRIWriter::SetInput(int index, vtkDataObject* input)
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
vtkDataObject* svkDICOMMRIWriter::GetInput(int port)
{
    return this->GetExecutive()->GetInputData(port, 0);
}


/*!
 *
 */
svkImageData* svkDICOMMRIWriter::GetImageDataInput(int port)
{
    return svkImageData::SafeDownCast( this->GetInput(port) );
}


/*!
 *  Write the pixel data to the PixelData DICOM element.       
 */
void svkDICOMMRIWriter::InitPixelData()
{

    vtkDebugMacro(<<"svkDICOMMRIWriter::InitPixelData()");
    int cols = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "Columns" ); 
    int rows = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "Rows" ); 
    int slices = (this->GetImageDataInput(0)->GetExtent() ) [5] - (this->GetImageDataInput(0)->GetExtent() ) [4] + 1;
    cout << "slices = " << slices << endl;
    int numComponents = 1;

    int dataLength = cols * rows * slices * numComponents;

    switch ( this->GetImageDataInput(0)->GetDcmHeader()->GetPixelDataType( this->GetImageDataInput(0)->GetScalarType() ) ) {

        case svkDcmHeader::UNSIGNED_INT_1:
        {
            unsigned char *pixelData = (unsigned char *)this->GetImageDataInput(0)->GetScalarPointer();
            this->GetImageDataInput(0)->GetDcmHeader()->SetValue(
                  "PixelData",
                  pixelData, 
                  dataLength 
            );
        }
        break;

        case svkDcmHeader::UNSIGNED_INT_2:
        {
            unsigned short *pixelData = static_cast<unsigned short *>( this->GetImageDataInput(0)->GetScalarPointer() );
            this->GetImageDataInput(0)->GetDcmHeader()->SetValue(
                  "PixelData",
                  pixelData, 
                  dataLength 
            );
        }
        break; 

        case svkDcmHeader::SIGNED_FLOAT_4:
        case svkDcmHeader::SIGNED_FLOAT_8:
        {
            //  Fix BitsAllocated, etc to be represent 
            //  signed short data
            this->GetImageDataInput(0)->GetDcmHeader()->SetPixelDataType(svkDcmHeader::UNSIGNED_INT_2);

            unsigned short* pixelData = new unsigned short[dataLength];  
            float slope; 
            float intercept; 
            this->GetShortScaledPixels( pixelData, slope, intercept ); 
            
            this->GetImageDataInput(0)->GetDcmHeader()->SetValue(
                  "PixelData",
                  pixelData, 
                  dataLength 
            );

            //  Init Rescale Attributes:    
            this->iod->InitPixelValueTransformationMacro( slope, intercept ); 

            delete[] pixelData; 
        }
        break;

        case svkDcmHeader::SIGNED_INT_2: 
        {
            short *pixelData = static_cast<short *>( this->GetImageDataInput(0)->GetScalarPointer() );
            this->GetImageDataInput(0)->GetDcmHeader()->SetValue(
                  "PixelData",
                  pixelData, 
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
void svkDICOMMRIWriter::GetPixelRange(double& min, double& max)
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
void svkDICOMMRIWriter::GetShortScaledPixels( unsigned short* shortPixels, float& slope, float& intercept )
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
    
    int cols = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "Columns" ); 
    int rows = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "Rows" ); 
    int slices = (this->GetImageDataInput(0)->GetExtent() ) [5] - (this->GetImageDataInput(0)->GetExtent() ) [4] + 1;
    int numPixels = cols * rows * slices; 

    int dataType = 
            this->GetImageDataInput(0)->GetDcmHeader()->GetPixelDataType( this->GetImageDataInput(0)->GetScalarType() ); 

    if (dataType == svkDcmHeader::SIGNED_FLOAT_4) {
        float* floatPixels = static_cast<float *>( this->GetImageDataInput(0)->GetScalarPointer() );
        for (int i = 0; i < numPixels; i++) {
            shortPixels[i] = static_cast<unsigned short> ( slope * floatPixels[i] + intercept ); 
        }
    } else if (dataType == svkDcmHeader::SIGNED_FLOAT_8) {
        double* doublePixels = static_cast<double *>( this->GetImageDataInput(0)->GetScalarPointer() );
        for (int i = 0; i < numPixels; i++) {
            shortPixels[i] = static_cast<unsigned short> ( slope * doublePixels[i] + intercept ); 
        }
    }

}

