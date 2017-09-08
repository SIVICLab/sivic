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



#include <svkMriZeroFill.h>


using namespace svk;


//vtkCxxRevisionMacro(svkMriZeroFill, "$Rev$");
vtkStandardNewMacro(svkMriZeroFill);


svkMriZeroFill::svkMriZeroFill()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    // By default we will not operate in place
    this->operateInPlace = false;

    // Initialize output image extent to INVALID:
    //  0 -> -1 in each dimension
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
 */
int svkMriZeroFill::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    // Get our input
    svkMriImageData* inputData = svkMriImageData::SafeDownCast(this->GetImageDataInput(0));
    svkMriImageData* targetData = svkMriImageData::New();
    targetData->DeepCopy( inputData );
    
    /*
     * First we need to translate the extent if the input image data to be in the center of the target volume
     * This does not modify the input data, just re-defines its extent for the padding.
     */
    vtkImageChangeInformation* translateExtent = vtkImageChangeInformation::New();
    int* inExtent = inputData->GetExtent();

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
    translateExtent->SetInputData( inputData );
    translateExtent->Update(); 
    vtkImageData::SetScalarType( 
        vtkImageData::GetScalarType( inputData->GetInformation() ), 
        translateExtent->GetOutput()->GetInformation() 
    ); 
   
    // Now we can use the vtk pad algorithm 
    vtkImageConstantPad* pad = NULL;
    pad = vtkImageConstantPad::New();
    pad->SetOutputWholeExtent(this->outputWholeExtent);
    pad->SetConstant(0.0);
    pad->SetInputData(translateExtent->GetOutput());
    pad->Update();
    vtkImageData::SetScalarType( 
        vtkImageData::GetScalarType( inputData->GetInformation() ), 
        pad->GetOutput()->GetInformation() 
    ); 

    // Now let's move the origin to the appropriate location
    double* spacing = inputData->GetSpacing();
    double newSpacing[3] = {0,0,0};
    newSpacing[0] = spacing[0]*((double)(inExtent[1]-inExtent[0] + 1))/(this->outputWholeExtent[1] - this->outputWholeExtent[0] + 1);
    newSpacing[1] = spacing[1]*((double)(inExtent[3]-inExtent[2] + 1))/(this->outputWholeExtent[3] - this->outputWholeExtent[2] + 1);
    newSpacing[2] = spacing[2]*((double)(inExtent[5]-inExtent[4] + 1))/(this->outputWholeExtent[5] - this->outputWholeExtent[4] + 1);
    if (this->GetDebug()) {
        cout << "NEW SPACING: " << newSpacing[0] 
            << " old " << inExtent[1] - inExtent[0]  
            << " new " << this->outputWholeExtent[1] - this->outputWholeExtent[0] << endl;
    }

    ostringstream* oss = new ostringstream();
    *oss << newSpacing[0];
    string pixelSpacingString( oss->str() + "\\" );
    delete oss;

    oss = new ostringstream();
    *oss << newSpacing[1];
    pixelSpacingString.append( oss->str() );

    delete oss;
    oss = new ostringstream();
    *oss << newSpacing[2];
    string sliceThicknessString( oss->str() );
    delete oss;

    double dcos[3][3];
    inputData->GetDcos( dcos );
    double* origin = inputData->GetOrigin();
    double newOrigin[3] = { origin[0], origin[1], origin[2] };

    double originShift[3] = { 0, 0, 0 };
    originShift[0] = newSpacing[0]/2 - spacing[0]/2;
    originShift[1] = newSpacing[1]/2 - spacing[1]/2;
    originShift[2] = newSpacing[2]/2 - spacing[2]/2;
    if (this->GetDebug()) {
        cout << "ORIG SPC: " << spacing[0] << endl;
        cout << "NEW  SPC: " << newSpacing[0] << endl;
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            newOrigin[i] += (originShift[j]) * dcos[j][i];
        }
    }

    int numSlices = this->outputWholeExtent[5] - this->outputWholeExtent[4] + 1;

    // Update the header
    targetData->GetDcmHeader()->SetValue("Columns", newNumVoxels[0]);
    targetData->GetDcmHeader()->SetValue("Rows", newNumVoxels[1]);
    targetData->GetDcmHeader()->SetValue("NumberOfFrames", newNumVoxels[2]);
    targetData->GetDcmHeader()->InitPixelMeasuresMacro( pixelSpacingString, sliceThicknessString );

    svkDcmHeader::DimensionVector dimensionVector = inputData->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, numSlices-1);
    targetData->GetDcmHeader()->InitPerFrameFunctionalGroupSequence( newOrigin, newSpacing, dcos, &dimensionVector); 

    targetData->DeepCopy(pad->GetOutput() );
    targetData->SyncVTKImageDataToDcmHeader(); 

    double shiftWindow[3] = { 0, 0, 0 };
    shiftWindow[0] = -originShift[0]/newSpacing[0];
    shiftWindow[1] = -originShift[1]/newSpacing[1];
    shiftWindow[2] = -originShift[2]/newSpacing[2];
    //shiftWindow[0] = 0; 
    //shiftWindow[1] = 0; 
    //shiftWindow[2] = 0; 

    // Apply a half voxel phase shift. This is because the sampled points of the data has changed.
    if (this->GetDebug()) {
        cout << "SHIFT kspace  MRI ZF: " << shiftWindow[0] << " " << shiftWindow[1] << " "<<  shiftWindow[2] << endl;
    }

    svkImageLinearPhase* linearShift = svkImageLinearPhase::New();
    linearShift->SetShiftWindow( shiftWindow );
    linearShift->SetInputData( targetData );
    linearShift->Update();

    // We need to scale the image by the updated size
    // This is because the FFT scalse up by the number of points
    // and the IFFT divides by it.
    double scale = ((double)(newNumVoxels[0]*newNumVoxels[1]*newNumVoxels[2]))
                   / ((double)(oldNumVoxels[0]*oldNumVoxels[1]*oldNumVoxels[2]));


    vtkImageMathematics* multiply = vtkImageMathematics::New();
    multiply->SetConstantK( scale );
    multiply->SetOperationToMultiplyByK();
    multiply->SetInputData( linearShift->GetOutput() );
    multiply->Update();
    vtkImageData::SetScalarType( 
        vtkImageData::GetScalarType( linearShift->GetOutput()->GetInformation() ), 
        multiply->GetOutput()->GetInformation() 
    ); 

    targetData->DeepCopy(multiply->GetOutput());

    // After the Linear Phase the BitsAllocated and PixelRepresentation must be reset
    // TODO: Make svkImageLinearPhase output svkImageData with the correct header info
    targetData->GetDcmHeader()->SetValue("BitsAllocated", 64);
    targetData->GetDcmHeader()->SetValue("PixelRepresentation", 1);
    targetData->GetDcmHeader()->ClearSequence("MRImageFrameTypeSequence");
    targetData->GetDcmHeader()->AddSequenceItemElement(
        "MRImageFrameTypeSequence",
        0,
        "ComplexImageComponent",
        string("MAGNITUDE"),
        "SharedFunctionalGroupsSequence",
        0
    );

    //  Trigger observer update via modified event:
    if( this->operateInPlace ) {
        inputData->DeepCopy( targetData );
        inputData->SyncVTKImageDataToDcmHeader();
        inputData->Modified();
    } else {
        this->GetOutput()->DeepCopy( targetData );
        this->GetOutput()->SyncVTKImageDataToDcmHeader();
    }

    linearShift->Delete();
    targetData->Delete();
    multiply->Delete();
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
