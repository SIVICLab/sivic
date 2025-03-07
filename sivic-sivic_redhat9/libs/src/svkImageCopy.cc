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



#include <svkImageCopy.h>


using namespace svk;


//vtkCxxRevisionMacro(svkImageCopy, "$Rev$");
vtkStandardNewMacro(svkImageCopy);


/*!
 *
 */
svkImageCopy::svkImageCopy()
{
#ifdef VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");


    this->SetNumberOfInputPorts(4);
    bool required = true;
    this->GetPortMapper()->InitializeInputPort( INPUT_IMAGE, "INPUT_IMAGE", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
    this->GetPortMapper()->InitializeInputPort( OUTPUT_SERIES_DESCRIPTION, "OUTPUT_SERIES_DESCRIPTION", svkAlgorithmPortMapper::SVK_STRING, !required);
    this->GetPortMapper()->InitializeInputPort( OUTPUT_SCALAR_TYPE, "OUTPUT_SCALAR_TYPE", svkAlgorithmPortMapper::SVK_INT, !required);
    this->GetPortMapper()->InitializeInputPort( ZERO_COPY, "ZERO_COPY", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->SetNumberOfOutputPorts(1);
    this->GetPortMapper()->InitializeOutputPort( 0, "COPY_OUTPUT", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);

}


/*!
 *
 */
svkImageCopy::~svkImageCopy()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~svkImageCopy()");
}


/*!
 *
 */
//void svkImageCopy::SetInput(svkImageData* image, int index)
//{
//    this->imageCopy = svkImageDataFactory::CreateInstance( image->GetClassName() );  
//}


/*!
 *  This is an in-place filter, so just pass original 
 *  svkImageData input back out:
 */
//svkImageData* svkImageCopy::GetOutput(int index)
//{
    //return this->imageCopy;
//}


/*!
 *  Set the series description for the DICOM header of the copy.  
 */
void svkImageCopy::SetSeriesDescription( string newSeriesDescription )
{
    this->GetPortMapper()->SetStringInputPortValue( OUTPUT_SERIES_DESCRIPTION, newSeriesDescription );
}


/*!
 *  Optionally sets the output data type to cast the copy to. 
 */
void svkImageCopy::SetOutputDataType(int dataType)
{
    this->GetPortMapper()->SetIntInputPortValue( OUTPUT_SCALAR_TYPE, dataType );
}


/*!
 *  If set, the copy will have pixels set to 0.  This is also useful for downcasting
 *  a copy. 
 */
void svkImageCopy::SetZeroCopy(bool zeroCopy)
{
    this->GetPortMapper()->SetBoolInputPortValue( ZERO_COPY, zeroCopy );
}


/*!
 *  Copy the Dcm Header and Provenance from the input to the output. 
 */
int svkImageCopy::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    this->UpdateHeader(); 
    int dataType = svkDcmHeader::UNDEFINED;
    if( this->GetPortMapper()->GetIntInputPortValue(OUTPUT_SCALAR_TYPE) != NULL ) {
        dataType = this->GetPortMapper()->GetIntInputPortValue(OUTPUT_SCALAR_TYPE)->GetValue();
    }
    if ( this->GetPortMapper()->GetBoolInputPortValue(ZERO_COPY) != NULL
            &&   this->GetPortMapper()->GetBoolInputPortValue(ZERO_COPY)->GetValue() == true) {
        this->GetOutput()->ZeroCopy( this->GetImageDataInput(0), (svkDcmHeader::DcmPixelDataFormat)dataType );
    } else {
        this->GetOutput()->DeepCopy( this->GetImageDataInput(0), (svkDcmHeader::DcmPixelDataFormat)dataType );
    }

    return 1; 
}


/*!
 *
 */
void svkImageCopy::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
}


/*!
 *
 */
void svkImageCopy::UpdateHeader()
{

    int dataType = svkDcmHeader::UNDEFINED;
    if( this->GetPortMapper()->GetIntInputPortValue(OUTPUT_SCALAR_TYPE) != NULL ) {
        dataType = this->GetPortMapper()->GetIntInputPortValue(OUTPUT_SCALAR_TYPE)->GetValue();
    }
    string newSeriesDescription = "";
    if( this->GetPortMapper()->GetStringInputPortValue(OUTPUT_SERIES_DESCRIPTION)) {
        newSeriesDescription = this->GetPortMapper()->GetStringInputPortValue(OUTPUT_SERIES_DESCRIPTION)->GetValue();
    }
    if (newSeriesDescription == "") {
        cout << "ERROR:  must set target image series description" << endl;
        exit(1); 
    }
    //  Copy the DICOM header:     
    this->GetImageDataInput(0)->GetDcmHeader()->MakeDerivedDcmHeader( 
        this->GetOutput()->GetDcmHeader(), 
        newSeriesDescription
    );

    if ( dataType != svkDcmHeader::UNDEFINED ) {
        this->GetOutput()->GetDcmHeader()->SetPixelDataType((svkDcmHeader::DcmPixelDataFormat)dataType);
    }
}

