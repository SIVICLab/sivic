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



#include <svkSincInterpolationFilter.h>


using namespace svk;


//vtkCxxRevisionMacro(svkSincInterpolationFilter, "$Rev$");
vtkStandardNewMacro(svkSincInterpolationFilter);


svkSincInterpolationFilter::svkSincInterpolationFilter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    // By default we will not operate in place
    this->operateInPlace = false;

    // Initialize output image extent to INVALID
    for (int i = 0; i < 3; i++) {
        this->outputWholeExtent[i * 2] = 0;
        this->outputWholeExtent[i * 2 + 1] = -1;
    }

}


svkSincInterpolationFilter::~svkSincInterpolationFilter()
{
}


/*!
 *
 */
void svkSincInterpolationFilter::SetOutputWholeExtent(int extent[6])
{
    int modified = 0;

    for ( int i = 0; i < 6; i++ ) {
        if (this->outputWholeExtent[i] != extent[i]) {
            this->outputWholeExtent[i] = extent[i];
            modified = 1;
        }
    }

    if ( modified ) {
        this->Modified();
    }
}


/*!
 *
 */
void svkSincInterpolationFilter::SetOutputWholeExtent(int minX, int maxX, int minY, int maxY, int minZ, int maxZ) 
{
    int extent[6];

    extent[0] = minX;  extent[1] = maxX;
    extent[2] = minY;  extent[3] = maxY;
    extent[4] = minZ;  extent[5] = maxZ;
    this->SetOutputWholeExtent(extent);
}


/*!
 *
 */
void svkSincInterpolationFilter::GetOutputWholeExtent(int extent[6])
{

    for (int i = 0; i < 6; i++) {
        extent[i] = this->outputWholeExtent[i];
    }
}


/*!
 *
 */
int svkSincInterpolationFilter::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    // get the info objects
    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);

    if (this->outputWholeExtent[0] > this->outputWholeExtent[1]) {
        // invalid setting, it has not been set, so default to whole Extent
        inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), this->outputWholeExtent);
    }
    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), this->outputWholeExtent, 6);
    if( this->GetImageDataInput(0) != NULL) {
		svkMriImageData* inputData = svkMriImageData::SafeDownCast(this->GetImageDataInput(0));
		double* spacing = inputData->GetSpacing();
		int* inExtent = inputData->GetExtent();
		double newSpacing[3] = {0,0,0};
		newSpacing[0] = spacing[0]*((double)(inExtent[1]-inExtent[0] + 1))/(this->outputWholeExtent[1] - this->outputWholeExtent[0] + 1);
		newSpacing[1] = spacing[1]*((double)(inExtent[3]-inExtent[2] + 1))/(this->outputWholeExtent[3] - this->outputWholeExtent[2] + 1);
		newSpacing[2] = spacing[2]*((double)(inExtent[5]-inExtent[4] + 1))/(this->outputWholeExtent[5] - this->outputWholeExtent[4] + 1);
		outInfo->Set(vtkDataObject::SPACING(), newSpacing, 3);

	    double dcos[3][3];
	    inputData->GetDcos( dcos );
	    double* origin = inputData->GetOrigin();
	    double newOrigin[3] = { origin[0], origin[1], origin[2] };

	    double originShift[3] = { 0, 0, 0 };
	    originShift[0] = newSpacing[0]/2 - spacing[0]/2;
	    originShift[1] = newSpacing[1]/2 - spacing[1]/2;
	    originShift[2] = newSpacing[2]/2 - spacing[2]/2;

	    for (int i = 0; i < 3; i++) {
	        for (int j = 0; j < 3; j++) {
	            newOrigin[i] += (originShift[j]) * dcos[j][i];
	        }
	    }
		outInfo->Set(vtkDataObject::ORIGIN(), newOrigin, 3);
    }


    return 1;

}

/*!
 *
 */
void svkSincInterpolationFilter::ComputeInputUpdateExtent (int inExt[6], int outExt[6], int wholeExtent[6])
{
    // Clip
    for (int i = 0; i < 3; i++) {
        inExt[i*2] = outExt[i*2];
        inExt[i*2+1] = outExt[i*2+1];
        if (inExt[i*2] < wholeExtent[i*2]) {
          inExt[i*2] = wholeExtent[i*2];
        }
        if (inExt[i*2] > wholeExtent[i*2 + 1]) {
            inExt[i*2] = wholeExtent[i*2 + 1];
        }
        if (inExt[i*2+1] < wholeExtent[i*2]) {
            inExt[i*2+1] = wholeExtent[i*2];
        }
        if (inExt[i*2 + 1] > wholeExtent[i*2 + 1]) {
            inExt[i*2 + 1] = wholeExtent[i*2 + 1];
        }
    }
}


/*!
 *
 */
int svkSincInterpolationFilter::RequestUpdateExtent (vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
    // get the info objects
    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);

    int wholeExtent[6];
    int inExt[6];

    // handle XYZ
    inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),wholeExtent);
    outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),inExt);

    this->ComputeInputUpdateExtent(inExt, inExt, wholeExtent);

    inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),inExt,6);

    return 1;
}


/*! 
 *  Primary execution method, runs through each step of the algorithm.
 */
int svkSincInterpolationFilter::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    // Get our input
    svkMriImageData* data = svkMriImageData::SafeDownCast(this->GetImageDataInput(0));
    svkMriImageData* target = svkMriImageData::New();
    target->DeepCopy( data );
    double *range;
    std::string representation = data->GetDcmHeader()->GetStringSequenceItemElement(
                                        "MRImageFrameTypeSequence",
                                        0,
                                        "ComplexImageComponent",
                                        "SharedFunctionalGroupsSequence",
                                        0
                                        );


    /********************************************************
     *   STEP 1: FFT the input data...
     ********************************************************/

    //Reverse FFT spatial data: kspace to spatial domain
    svkMriImageFFT* fft = svkMriImageFFT::New();
    fft->SetInputData( target );
    fft->SetFFTMode( svkMriImageFFT::FORWARD );
    fft->SetOperateInPlace( true );
    fft->Update();

    /********************************************************
     *   STEP 2: Move K = 0 to the center of the image
     ********************************************************/

    svkImageFourierCenter* center = svkImageFourierCenter::New();
    center->SetInputData( fft->GetOutput() );
    center->Update();
    target->DeepCopy( center->GetOutput() );

    // New we are done with fft algorithm
    fft->Delete();
    center->Delete();

    /********************************************************
     *   STEP 3: Now pad the data about the center...
     ********************************************************/
    // We want to store the origin and extent before we pad so use them later 
    double origin[3];
    data->GetOrigin( origin );


    int inExtent[6];
    data->GetExtent( inExtent );

    svkMriZeroFill* pad = svkMriZeroFill::New();
    pad->SetInputData( target );
    pad->SetOutputWholeExtent(this->outputWholeExtent);
    pad->SetOperateInPlace( true );
    pad->Update();

    /********************************************************
     *   STEP 4: Now we reverse the center...
     ********************************************************/
    svkImageFourierCenter* reverseCenter = svkImageFourierCenter::New();
    reverseCenter->SetReverseCenter( true );
    reverseCenter->SetInputData( target );
    reverseCenter->Update();

    // We need to deep copy here because it is a non-svk output
    target->ShallowCopy( reverseCenter->GetOutput() );

    // And we are done with the padding algorithm and the centering algo
    pad->Delete();
    reverseCenter->Delete();
    
    target->SyncVTKImageDataToDcmHeader(); 

    /********************************************************
     *   STEP 5: Now we return to the spatial domain and
     ********************************************************/

    svkMriImageFFT* rfft = svkMriImageFFT::New();
    rfft->SetInputData( target );
    rfft->SetFFTMode( svkMriImageFFT::REVERSE );
    rfft->SetOperateInPlace( true );
    rfft->Update();
    
    if( representation.compare("MAGNITUDE") == 0 ) {
        vtkImageExtractComponents* real = vtkImageExtractComponents::New();
        real->SetComponents( 0 );
        real->SetInputData( rfft->GetOutput() );
        real->Update();
        target->ShallowCopy( real->GetOutput() );
        real->Delete();
        target->SyncVTKImageDataToDcmHeader();
    }

    //  Trigger observer update via modified event:
    if( this->operateInPlace ) {
        data->DeepCopy( target );
        data->SyncVTKImageDataToDcmHeader();
        data->Modified();
    } else {
        this->GetOutput()->DeepCopy( target );
        this->GetOutput()->SyncVTKImageDataToDcmHeader();
    }

    target->Delete();
    rfft->Delete();
    return 1; 
}


/*!
 * This determines if the input data is being operated on, or if a new dataset should be
 * created.
 */
void svkSincInterpolationFilter::SetOperateInPlace( bool operateInPlace )
{
    this->operateInPlace = operateInPlace;
}


/*!
 * Defines the input type: svkMriImageData.
 */
int svkSincInterpolationFilter::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData"); 
    return 1;
}


/*!
 * We want to override this method so that we can return a pointer to the input
 * in the case of an in place operation.
 */
svkImageData* svkSincInterpolationFilter::GetOutput(int port)
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
svkImageData* svkSincInterpolationFilter::GetOutput()
{   
    return this->GetOutput(0); 
}
