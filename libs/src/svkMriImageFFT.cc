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



#include <svkMriImageFFT.h>


using namespace svk;


//vtkCxxRevisionMacro(svkMriImageFFT, "$Rev$");
vtkStandardNewMacro(svkMriImageFFT);


svkMriImageFFT::svkMriImageFFT()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    //  Start index for update extent
    this->mode   = FORWARD;

    // By default we will not operate in place
    this->operateInPlace = false;
}


svkMriImageFFT::~svkMriImageFFT()
{
}


/*! 
 *  This is the primary execution method. If the mode is set to FORWARD it will do an fft of the
 *  data. If it is set to REVERSE then it will run an rfft and then extract the real component.
 *  Input can be any data type but output is always complex double.
 */
int svkMriImageFFT::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    vtkImageFourierFilter* fourierFilter = NULL;

    // Determine if we want a forward or reverse FFT
    if( this->mode == REVERSE ) {
        fourierFilter = vtkImageRFFT::New();
    } else {
        fourierFilter = vtkImageFFT::New();
    }

    // We want to run the FT in 3D
    fourierFilter->SetDimensionality(3);

    // Get our input image and cast to svkImageData
    svkMriImageData* data = svkMriImageData::SafeDownCast(this->GetImageDataInput(0));

    // If this is not operating in place thin we have to copy the basic header information here
    if( !this->operateInPlace ) {
        this->GetOutput()->DeepCopy( data );
    }

    //  First check to see if the transform is required. If not just return: 

    // If the tags are missing we assume it is in the spatial domain
    string domainCol = string("SPACE");
    string domainRow = string("SPACE");
    string domainSlice = string("SPACE");

    // If the tag is set lets get it
    if( data->GetDcmHeader()->ElementExists( "SVK_ColumnsDomain" ) ){
        domainCol = data->GetDcmHeader()->GetStringValue( "SVK_ColumnsDomain");
    }
    if( data->GetDcmHeader()->ElementExists( "SVK_RowsDomain" ) ){
        domainRow = data->GetDcmHeader()->GetStringValue( "SVK_RowsDomain");
    }
    if( data->GetDcmHeader()->ElementExists( "SVK_SliceDomain" ) ){
        domainSlice = data->GetDcmHeader()->GetStringValue( "SVK_SliceDomain"); 
    }
    if( this->mode == REVERSE ) {
        // REVERSE should take to space domain, so if the data is already in space then skip the FT 
        if (  ( domainCol.compare("SPACE")   == 0 ) 
           || ( domainRow.compare("SPACE")   == 0 ) 
           || ( domainSlice.compare("SPACE") == 0 ) 
        ){
            cout << "svkMriImageFFT: Already in target domain, not transforming " << endl; 
            return 1; 
        }; 
    } else {
        // FORWARD should take to kspace domain, so if the data is already in kspace then skip the FT 
        if (  ( domainCol.compare("KSPACE")   == 0 ) 
           || ( domainRow.compare("KSPACE")   == 0 ) 
           || ( domainSlice.compare("KSPACE") == 0 ) 
        ){
            cout << "svkMriImageFFT: Already in target domain, not transforming " << endl; 
            return 1; 
        }; 
    }

    // Do the Fourier Transform
    fourierFilter->SetInputData(data);
    fourierFilter->Update();
    if( this->operateInPlace ) {
        data->ShallowCopy(fourierFilter->GetOutput());
    } else {
        this->GetOutput()->ShallowCopy(fourierFilter->GetOutput());
    }
       
    // Lets update the header to reflect the domain change. 
    if( this->operateInPlace ) {
        this->UpdateHeader( data->GetDcmHeader() );
        data->SyncVTKImageDataToDcmHeader();
        data->Modified();
    } else {
        this->UpdateHeader( this->GetOutput()->GetDcmHeader() );
        this->GetOutput()->SyncVTKImageDataToDcmHeader();
    }

    fourierFilter->Delete();
    return 1; 
}


/*!
 *  Updates the header information to make sure the domain is correct.
 *  We are using private tags to store the domain, but only if it is
 *  in k-space. If the data is in the spatial domain the tags are
 *  not set.
 */
void svkMriImageFFT::UpdateHeader( svkDcmHeader* targetHeader )
{
    //  Update the DICOM header to reflect the domain changes:
    if( this->mode == REVERSE ) {
        if( targetHeader->ElementExists( "SVK_ColumnsDomain" ) ){
            targetHeader->ClearElement( "SVK_ColumnsDomain" );
        }
        if( targetHeader->ElementExists( "SVK_RowsDomain" ) ){
            targetHeader->ClearElement( "SVK_RowsDomain" );
        }
        if( targetHeader->ElementExists( "SVK_SliceDomain" ) ){
            targetHeader->ClearElement( "SVK_SliceDomain" );
        }
    } else {
        targetHeader->SetPixelDataType( svkDcmHeader::SIGNED_FLOAT_8 );
        targetHeader->SetValue( "SVK_PRIVATE_TAG", "SVK_PRIVATE_CREATOR" );
        targetHeader->SetValue( "SVK_ColumnsDomain", "KSPACE" );
        targetHeader->SetValue( "SVK_RowsDomain",    "KSPACE" );
        targetHeader->SetValue( "SVK_SliceDomain",   "KSPACE" );
    }

}


/*!
 * This determines if you want a forward fft are a reverse.
 */
void svkMriImageFFT::SetFFTMode( FFTMode mode )
{
    this->mode = mode;
}


/*!
 * This determines if the input data is being operated on, or if a new dataset should be
 * created.
 */
void svkMriImageFFT::SetOperateInPlace( bool operateInPlace )
{
    this->operateInPlace = operateInPlace;
}


/*!
 * Defines the input type: svkMriImageData.
 */
int svkMriImageFFT::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData"); 
    return 1;
}


/*!
 * We want to override this method so that we can return a pointer to the input
 * in the case of an in place operation.
 */
svkImageData* svkMriImageFFT::GetOutput(int port)
{
    if( this->operateInPlace ) {
        return svkMriImageData::SafeDownCast(this->GetImageDataInput(0));
    } else {
        return svkImageData::SafeDownCast(this->GetOutputDataObject(port));
    }
}


/*!
 *   This is only overriden to prevent it from being hidden. When you override
 *   a method you need to override every overloaded version.
 */
svkImageData* svkMriImageFFT::GetOutput()
{   
    return this->GetOutput(0); 
}
