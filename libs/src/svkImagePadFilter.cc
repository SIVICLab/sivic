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



#include <svkMriZeroFill.h>


using namespace svk;


vtkCxxRevisionMacro(svkMriZeroFill, "$Rev$");
vtkStandardNewMacro(svkMriZeroFill);


svkMriZeroFill::svkMriZeroFill()
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


svkMriZeroFill::~svkMriZeroFill()
{
}


/*!
 *
 */
void svkMriZeroFill::SetOutputWholeExtent(int extent[6])
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
void svkMriZeroFill::SetOutputWholeExtent(int minX, int maxX, int minY, int maxY, int minZ, int maxZ) 
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
void svkMriZeroFill::GetOutputWholeExtent(int extent[6])
{

    for (int i = 0; i < 6; i++) {
        extent[i] = this->outputWholeExtent[i];
    }
}


/*!
 *
 */
int svkMriZeroFill::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    // get the info objects
    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);

    if (this->outputWholeExtent[0] > this->outputWholeExtent[1]) {
        // invalid setting, it has not been set, so default to whole Extent
        inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), this->outputWholeExtent);
    }
    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), this->outputWholeExtent, 6);

    return 1;

}

/*!
 *
 */
void svkMriZeroFill::ComputeInputUpdateExtent (int inExt[6], int outExt[6], int wholeExtent[6])
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
int svkMriZeroFill::RequestUpdateExtent (vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVector, vtkInformationVector* outputVector)
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
 *  This is the primary execution method. If the mode is set to FORWARD it will do an fft of the
 *  data. If it is set to REVERSE then it will run an rfft and then extract the real component.
 *  Input can be any data type but output is always complex double.
 */
int svkMriZeroFill::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    // Get our input
    svkMriImageData* data = svkMriImageData::SafeDownCast(this->GetImageDataInput(0));

    // If this is not operating in place thin we have to copy the basic header information here
    if( !this->operateInPlace ) {
        this->GetOutput()->DeepCopy( data );
    }
    
    /*
     * First we need to translate the extent if the input image data to be in the center of the target volume
     * This does not modify the input data, just re-defines its extent for the padding.
     */
    vtkImageChangeInformation* translateExtent = vtkImageChangeInformation::New();
    int* inExtent = data->GetExtent();

    int oldNumVoxels[3];
    oldNumVoxels[0] =  inExtent[1] - inExtent[0] + 1;
    oldNumVoxels[1] =  inExtent[3] - inExtent[2] + 1;
    oldNumVoxels[2] =  inExtent[5] - inExtent[4] + 1;

    // This will be used to correct the DICOM headers
    int newNumVoxels[3];
    newNumVoxels[0] =  this->outputWholeExtent[1] - this->outputWholeExtent[0] + 1;
    newNumVoxels[1] =  this->outputWholeExtent[3] - this->outputWholeExtent[2] + 1;
    newNumVoxels[2] =  this->outputWholeExtent[5] - this->outputWholeExtent[4] + 1;
    int extentTranslation[3] = {0,0,0};
    for( int i = 0; i <3; i++ ) {
        if( newNumVoxels[i] % 2 == 1 && oldNumVoxels[i] % 2 == 0 ) {
            extentTranslation[i] = (newNumVoxels[i] - oldNumVoxels[i])/2;
        } else {
            extentTranslation[i] = vtkMath::Round((newNumVoxels[i] - oldNumVoxels[i])/2.0);
        }
    }
    translateExtent->SetExtentTranslation( extentTranslation ); 
    translateExtent->SetInput( data );
   
    // Now we can use the vtk pad algorithm 
    vtkImageConstantPad* pad = NULL;
    pad = vtkImageConstantPad::New();
    pad->SetOutputWholeExtent(this->outputWholeExtent);
    pad->SetConstant(0.0);
    pad->SetInput(translateExtent->GetOutput());
    pad->Update();


    // Now let's move the origin to the appropriate location
    double* origin = data->GetOrigin(); 
    double* spacing = data->GetSpacing();
    double dcos[3][3];
    double newOrigin[3];
    data->GetDcos( dcos );
    for (int i = 0; i < 3; i++) {
        newOrigin[i] = origin[i];
        for( int j = 0; j < 3; j++) {
            newOrigin[i] -= (extentTranslation[j]) * spacing[j] * dcos[j][i];
        }
    }
    int numSlices = this->outputWholeExtent[5] - this->outputWholeExtent[4] + 1;

    if( this->operateInPlace ) {
        data->ShallowCopy(pad->GetOutput());
        data->GetDcmHeader()->SetValue("Columns", newNumVoxels[0]);
        data->GetDcmHeader()->SetValue("Rows", newNumVoxels[1]);
        data->GetDcmHeader()->SetValue("NumberOfFrames", newNumVoxels[2]);
        data->GetDcmHeader()->InitPerFrameFunctionalGroupSequence( newOrigin, spacing, dcos, numSlices, 1, 1);
    } else {
        this->GetOutput()->ShallowCopy(pad->GetOutput());
        this->GetOutput()->GetDcmHeader()->SetValue("Columns", newNumVoxels[0]);
        this->GetOutput()->GetDcmHeader()->SetValue("Rows", newNumVoxels[1]);
        this->GetOutput()->GetDcmHeader()->SetValue("NumberOfFrames", newNumVoxels[2]);
        this->GetOutput()->GetDcmHeader()->InitPerFrameFunctionalGroupSequence( newOrigin, spacing, dcos, numSlices, 1, 1);
    }

    //  Trigger observer update via modified event:
    if( this->operateInPlace ) {
        this->GetInput()->Modified();
        this->GetInput()->Update();
    }
    translateExtent->Delete();
    pad->Delete();
    return 1; 
}


/*!
 * This determines if the input data is being operated on, or if a new dataset should be
 * created.
 */
void svkMriZeroFill::SetOperateInPlace( bool operateInPlace )
{
    this->operateInPlace = operateInPlace;
}


/*!
 * Defines the input type: svkMriImageData.
 */
int svkMriZeroFill::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData"); 
    return 1;
}


/*!
 * We want to override this method so that we can return a pointer to the input
 * in the case of an in place operation.
 */
svkImageData* svkMriZeroFill::GetOutput(int port)
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
svkImageData* svkMriZeroFill::GetOutput()
{   
    return this->GetOutput(0); 
}
