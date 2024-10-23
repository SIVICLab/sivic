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


#include <svkObliqueReslice.h>

#include </usr/include/vtk/vtkImageChangeInformation.h>
#include </usr/include/vtk/vtkInformationVector.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>
#include <cmath>


using namespace svk;


//vtkCxxRevisionMacro(svkObliqueReslice, "$Rev$");
vtkStandardNewMacro(svkObliqueReslice);


/*!
 *
 */
svkObliqueReslice::svkObliqueReslice()
{
#if VTK_DEBUG_ON
    //this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");
    this->SetNumberOfInputPorts(4);
    bool repeatable = true;
    bool required   = true;
    this->GetPortMapper()->InitializeInputPort(INPUT_IMAGE, "INPUT_IMAGE", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, required, !repeatable);
    this->GetPortMapper()->InitializeInputPort(TARGET_IMAGE, "TARGET_IMAGE", svkAlgorithmPortMapper::SVK_IMAGE_DATA, !required, !repeatable);
    this->GetPortMapper()->InitializeInputPort(MATCH_SPACING_AND_FOV, "MATCH_SPACING_AND_FOV", svkAlgorithmPortMapper::SVK_BOOL, !required, !repeatable);
    this->GetPortMapper()->InitializeInputPort(INTERPOLATION_MODE, "INTERPOLATION_MODE", svkAlgorithmPortMapper::SVK_INT, !required);

    this->SetNumberOfOutputPorts(1);
    this->GetPortMapper()->InitializeOutputPort(RESLICED_IMAGE, "RESLICED_IMAGE", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);

    this->reslicer = vtkImageReslice::New();
    
    this->reslicedImage = NULL;  

    this->targetDcos[0][0] = 0.; 
    this->targetDcos[0][1] = 0.; 
    this->targetDcos[0][2] = 0.; 
    this->targetDcos[1][0] = 0.; 
    this->targetDcos[1][1] = 0.; 
    this->targetDcos[1][2] = 0.; 
    this->targetDcos[2][0] = 0.; 
    this->targetDcos[2][1] = 0.; 
    this->targetDcos[2][2] = 0.; 

    this->magnification[0] = 1; 
    this->magnification[1] = 1; 
    this->magnification[2] = 1; 

}


/*!
 *
 */
svkObliqueReslice::~svkObliqueReslice()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~svkObliqueReslice()");
    
    if (this->reslicer != NULL) {
        this->reslicer->Delete();
        this->reslicer = NULL; 
    }
}

/*!
 *  Utility setter for input port: Interpolation Mode
 */
void svkObliqueReslice::SetInterpolationMode(int interpolationMode)
{
    this->GetPortMapper()->SetIntInputPortValue(INTERPOLATION_MODE, interpolationMode);
}


/*!
 *  Utility getter for input port: Interpolation Mode
 */
svkInt* svkObliqueReslice::GetInterpolationMode()
{
    svkInt* interpolationMode = this->GetPortMapper()->GetIntInputPortValue(INTERPOLATION_MODE);
    if (!interpolationMode)
    {
        this->SetInterpolationMode(1);
        interpolationMode = this->GetPortMapper()->GetIntInputPortValue(INTERPOLATION_MODE);
    }
    return interpolationMode;
}


/*
 *  Check if any of the magnification factors rquire the data to be 
 *  up or down sampled. 
 */
bool svkObliqueReslice::Magnify()
{
    bool magnify = false; 
    for ( int i = 0; i < 3; i++ ) {
        if ( this->magnification[i] != 1 ) { 
            magnify = true; 
        }
    }
    return magnify; 
}


/*!
 * Sets the target image. By default the target dcos will be used to simply
 * reslice the input into the orientation of the target.
 */
void svkObliqueReslice::SetTarget(svkImageData* image)
{
    image->GetDcmHeader()->GetDataDcos(this->targetDcos); 
    this->SetInputData(TARGET_IMAGE, image);
}


/*!
 *  
 * 
 */
void svkObliqueReslice::SetTargetDcos(double dcos[3][3])
{
    this->targetDcos[0][0] = dcos[0][0];
    this->targetDcos[0][1] = dcos[0][1];
    this->targetDcos[0][2] = dcos[0][2];
    this->targetDcos[1][0] = dcos[1][0];
    this->targetDcos[1][1] = dcos[1][1];
    this->targetDcos[1][2] = dcos[1][2];
    this->targetDcos[2][0] = dcos[2][0];
    this->targetDcos[2][1] = dcos[2][1];
    this->targetDcos[2][2] = dcos[2][2];
}


/*!
 *  Check to see if targetDcos has been initialized
 */
bool svkObliqueReslice::IsDcosInitialized()
{

    bool initialized = false; 
    for (int i = 0; i < 3; i++ ) {
        for (int j = 0; j < 3; j++ ) {
            if ( this->targetDcos[i][j] ) {
                initialized = true; 
                break; 
            }
        }
    }
    return initialized; 
}

/*!
 *
 */
int svkObliqueReslice::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    this->reslicer->SetInputData( this->GetImageDataInput(0) ); 

     // Set interpolation mode
    int interpolationMode;
    interpolationMode     = this->GetInterpolationMode()->GetValue();
    switch (interpolationMode) {
        case 1:
            this->reslicer->SetInterpolationModeToCubic();
            break;
        case 2:
            this->reslicer->SetInterpolationModeToLinear();
            break;
        case 3:
            this->reslicer->SetInterpolationModeToNearestNeighbor();
            break;
    }
    double rotation[3][3];   
    this->SetRotationMatrix(); 

    if( this->Magnify() && this->GetMatchSpacingAndFovOn() ) {
        cerr << "ERROR: svkObliqueReslice does not support magnification and matching of FOV." << endl;
    }


    if ( this->Magnify() == true ) { 
        //  get the and modify the input extent 
        if (this->GetDebug() ) {
            cout << this->GetClassName() << "::RequestInformation() Resample factors: " 
                << this->magnification[0] << " " << this->magnification[1] << " " << this->magnification[2] << endl;
        }
        int extent[6]; 
        this->GetImageDataInput(0)->GetExtent( extent ); 
        if ( this->GetDebug() ) {
            cout << "INPUT EXTENT: " << extent[1] << " " << extent[3] << " " << extent[5] << endl;
        }
        for ( int i = 0; i < 3; i++ ) {
            extent[i*2]     /= this->magnification[i]; 
            extent[i*2 + 1] /= this->magnification[i]; 
        }
        if ( this->GetDebug() ) {
            cout << "OUTPUT EXTENT: " << extent[1] << " " << extent[3] << " " << extent[5] << endl;
        }
        this->reslicer->SetOutputExtent( extent ); 

        //  get the and modify the input spacing 
        double spacing[3]; 
        this->GetImageDataInput(0)->GetSpacing( spacing ); 
        if ( this->GetDebug() ) {
            cout << "INPUT SPACING: " << spacing[0] << " " << spacing[1] << " " << spacing[2] << endl;
        }
        for ( int i = 0; i < 3; i++ ) {
            spacing[i] *= this->magnification[i]; 
        }
        if ( this->GetDebug() ) {
            cout << "OUTPUT SPACING: " << spacing[0] << " " << spacing[1] << " " << spacing[2] << endl;
        }
        this->reslicer->SetOutputSpacing( spacing ); 
    } else if (this->GetMatchSpacingAndFovOn() ) {

        // Set Output Extent
        int numVoxels[3];
        this->GetPortMapper()->GetImageInputPortValue(TARGET_IMAGE)->GetNumberOfVoxels(numVoxels);
        int outputExtent[6] = {0,0,0,0,0,0};
        outputExtent[1] = numVoxels[0] -1;
        outputExtent[3] = numVoxels[1] -1;
        outputExtent[5] = numVoxels[2] -1;
        this->reslicer->SetOutputExtent(outputExtent);

        // Set Output Spacing
        double outputSpacing[3];
        this->GetPortMapper()->GetImageInputPortValue(TARGET_IMAGE)->GetSpacing(outputSpacing);
        this->reslicer->SetOutputSpacing(outputSpacing);

        // Set Output Origin
        double inputOrigin[3];
        this->GetPortMapper()->GetMRImageInputPortValue(INPUT_IMAGE)->GetDcmHeader()->GetOrigin(inputOrigin);

        // Get the output origin
        double targetOrigin[3];
        this->GetPortMapper()->GetImageInputPortValue(TARGET_IMAGE)->GetDcmHeader()->GetOrigin(targetOrigin);

        // Get the input spacing
        double inputSpacing[3];
        this->GetPortMapper()->GetMRImageInputPortValue(INPUT_IMAGE)->GetSpacing(inputSpacing);

        /*
         * This next block of code is to *trick* the vtkImageReslice into doing the correct transformation
         * for us. The vtkImageReslice algorithm does not use the dcos in the svkImageData objects so
         * for the transformation to be correct we need to transform the target origin into the "vtk" space,
         * meaning the space assuming our data has an identity dcos. Then we have to pass this target position
         * through the the inverse of the transformation matrix being used. This is because vtkImageReslice
         * will transform the given output origin as part of the process. This is a three stop process:
         *
         * Step 1: Convert the LPS coordinate of the target origin into the ijk index of that location
         *         in the source image.
         *
         * Step 2: Convert the ijk index of the target origin in the input data to an LPS' location assuming
         *         an identity dcos.
         *
         * Step 3: Take the LPS' location and inverse transform it using the current transformation matrix.
         */

        // Step1: Get the index of the target origin in the input image frame.
        double targetOriginInInputIndex[3];
        this->GetPortMapper()->GetMRImageInputPortValue(INPUT_IMAGE)->GetIndexFromPosition(targetOrigin,targetOriginInInputIndex);

        // Step2: Get the position of the given index assuming a unity dcos: LPS'
        double targetUnityDcosOrigin[3];
        targetUnityDcosOrigin[0] = inputOrigin[0] + inputSpacing[0]*(targetOriginInInputIndex[0]-0.5);
        targetUnityDcosOrigin[1] = inputOrigin[1] + inputSpacing[1]*(targetOriginInInputIndex[1]-0.5);
        targetUnityDcosOrigin[2] = inputOrigin[2] + inputSpacing[2]*(targetOriginInInputIndex[2]-0.5);

        // Step3: Use the inverse of the transformation matrix to compute the location PRE-transformation.
        vtkTransform* transform = vtkTransform::New();
        transform->SetMatrix(this->reslicer->GetResliceAxes());
        double outputOrigin[3];
        transform->GetInverse()->TransformPoint(targetUnityDcosOrigin, outputOrigin);
        this->reslicer->SetOutputOrigin(outputOrigin);
        transform->Delete();

    }
    reslicer->Update();

    vtkInformation* outInfo = outputVector->GetInformationObject(0);


    double newTlc[3];
    this->ComputeTopLeftCorner( newTlc );
    outInfo->Set(vtkDataObject::SPACING(), this->reslicer->GetOutput()->GetSpacing(), 3);
    outInfo->Set(vtkDataObject::ORIGIN(), newTlc, 3);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),this->reslicer->GetOutput()->GetExtent() , 6);

    return 1;

}


/*!
 *  This is the ratio of the input and output spacing
 *      Spacing is multiplied by the mag factor
 *      extent is is divided by the mag factor.  
 *
 *      A mag factor > 1 downsamples the data (smaller extent, larger spacing)
 *      A mag factor < 1 upsamples the data (larger extent, smaller spacing)
 *  
 */
void svkObliqueReslice::SetMagnificationFactors( float x, float y, float z)
{
    this->magnification[0] = x; 
    this->magnification[1] = y; 
    this->magnification[2] = z; 
}


/*!
 * Setter to turn on the matching of the spacing and fov to the target image.
 * A target image must be set for this option.
 */
void svkObliqueReslice::SetMatchSpacingAndFovOn( )
{
    this->GetPortMapper()->SetBoolInputPortValue( MATCH_SPACING_AND_FOV, true );
}


/*!
 * Convenience method for checking of spacing and fov matching is on.
 */
bool svkObliqueReslice::GetMatchSpacingAndFovOn( )
{
    bool matchSpacingAndFov =  false;
    svkBool* matchSpacingAndFovObject = this->GetPortMapper()->GetBoolInputPortValue( MATCH_SPACING_AND_FOV );
    if( matchSpacingAndFovObject != NULL ) {
        matchSpacingAndFov = matchSpacingAndFovObject->GetValue();
    }
    return matchSpacingAndFov;

}


/*!
 *
 *  Copy the Dcm Header, svkOrientedImageData and Provenance from the 
 *  input to the output image. 
 */
int svkObliqueReslice::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) 
{

    //  Get the allocated svk output image data (resliced image doesn't have a dcos yet): 
    this->GetOutput()->SetDcos( this->targetDcos ); 
    string activeScalarName = this->GetImageDataInput(0)->GetPointData()->GetScalars()->GetName();

    this->reslicer->SetInputData( this->GetImageDataInput(0) );

    int numberOfVolumes = this->GetImageDataInput(0)->GetPointData()->GetNumberOfArrays();

    for( int i = 0; i < numberOfVolumes; i++ ) {
    	string currentScalarName = this->GetImageDataInput(0)->GetPointData()->GetArray(i)->GetName();
    	this->GetImageDataInput(0)->GetPointData()->SetActiveScalars( currentScalarName.c_str() );

    	/*
    	 *  We are going to switch the input data to NULL and then back to the image data object.
    	 *  This is because the vtk algorithm will not re-execute just because the active scalars
    	 *  have changed (although it really should). To get around this we switch the input to
    	 *  NULL and then back to force an update.
    	 */
		this->reslicer->SetInputData( NULL );
		this->reslicer->SetInputData( this->GetImageDataInput(0) );
    	this->reslicer->Update();

    	if( i == 0 ) {
			//  Copy the vtkImageAlgo output to the allocated svkImageData output image
            vtkImageData::SetScalarType(
                vtkImageData::GetScalarType( this->reslicer->GetImageDataInput(0)->GetInformation()),
                this->reslicer->GetOutput()->GetInformation()
            );
			this->GetOutput()->DeepCopy( this->reslicer->GetOutput() );
    	} else {
    		// We need to copy the data array after each update because it will be reused
    		vtkDataArray* newArray = vtkDataArray::CreateDataArray( this->reslicer->GetOutput()->GetPointData()->GetScalars()->GetDataType());
    		newArray->DeepCopy(this->reslicer->GetOutput()->GetPointData()->GetScalars() );
    		this->GetOutput()->GetPointData()->AddArray(newArray);
    		newArray->Delete();
    	}
		this->GetOutput()->GetPointData()->GetArray(i)->SetName(currentScalarName.c_str());
    }
	this->GetImageDataInput(0)->GetPointData()->SetActiveScalars( activeScalarName.c_str() );

    this->UpdateHeader(); 
    outputVector->GetInformationObject(0)->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), this->GetOutput()->GetExtent(), 6);
    
    // When we updated the header we changed the origin and spacing so we have to update here...
    outputVector->GetInformationObject(0)->Set(vtkDataObject::ORIGIN(), this->GetOutput()->GetOrigin(), 3);
    if ( this->GetDebug() ) {
        //this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }

    return 1; 
}


/*!
 *  Get the rotation matrix that will transform the input orientation (dcos) to the
 *  target orientation (targetDcos):  
 *      Rot * dcosIn = targetDcos
 *      Rot * dcosIn* (dcosIn)-1 = targetDcos* (dcosIn)-1
 *      Rot  = targetDcos* (dcosIn)-1
 */
void svkObliqueReslice::SetRotationMatrix( )
{
    if( this->GetPortMapper()->GetImageInputPortValue(TARGET_IMAGE) != NULL ) {
        this->GetPortMapper()->GetImageInputPortValue(TARGET_IMAGE)->GetDcos(this->targetDcos);
    }

    //  If target dcos isn't initialized, set it from the output image. 
    if ( ! this->IsDcosInitialized() ) {
        this->SetTarget( svkImageData::SafeDownCast(this->GetImageDataInput(0)) );
        svkImageData::SafeDownCast(this->GetImageDataInput(0))->GetDcos(this->targetDcos);
    }


    double dcosIn[3][3];   
    this->GetImageDataInput(0)->GetDcmHeader()->GetDataDcos(dcosIn); 

    double dcosInInverse[3][3];   
    vtkMath::Invert3x3(dcosIn, dcosInInverse);
    double rotation[3][3];
    vtkMath::Multiply3x3(this->targetDcos, dcosInInverse, rotation);
    this->reslicer->SetResliceAxesDirectionCosines(
        rotation[0][0],
        rotation[0][1],
        rotation[0][2],
        rotation[1][0],
        rotation[1][1],
        rotation[1][2],
        rotation[2][0],
        rotation[2][1],
        rotation[2][2]
    );

}


/*!
 *
 */
void svkObliqueReslice::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
}


/*!
 *  Set the header info to match the resliced data set: 
 */
void svkObliqueReslice::UpdateHeader()
{
    string seriesDescription("RESLICED ");
	seriesDescription.append(this->GetImageDataInput(0)->GetDcmHeader()->GetStringValue("SeriesDescription"));

    this->reslicedImage = this->GetOutput(); 

    //  Copy the DICOM header:     
    this->GetImageDataInput(0)->GetDcmHeader()->MakeDerivedDcmHeader( 
        this->reslicedImage->GetDcmHeader(), 
        seriesDescription 
    );

    if ( this->GetDebug() ) {
        //cout << "INPUT IMAGE: " << *( this->GetImageDataInput(0) ) << endl;
        //cout << "OUTPUT IMAGE: " << *( this->reslicedImage ) << endl;
    }

    int* extent = this->reslicedImage->GetExtent(); 
    this->newNumVoxels[0] =  extent[1] + 1; 
    this->newNumVoxels[1] =  extent[3] + 1; 
    this->newNumVoxels[2] =  extent[5] + 1; 

    this->reslicedImage->GetDcmHeader()->SetValue( 
        "Columns", 
        this->newNumVoxels[0]
    );
    this->reslicedImage->GetDcmHeader()->SetValue( 
        "Rows", 
        this->newNumVoxels[1]
    );
    int numberOfVolumes = this->reslicedImage->GetPointData()->GetNumberOfArrays();
    this->reslicedImage->GetDcmHeader()->SetValue( 
        "NumberOfFrames", 
        this->newNumVoxels[2]*numberOfVolumes
    );

    //  Set Origin and Orientation and spacing
    this->SetReslicedHeaderSpacing();
    this->SetReslicedHeaderPerFrameFunctionalGroups();
    this->SetReslicedHeaderOrientation();

    if ( this->GetDebug() ) {
        //this->reslicedImage->GetDcmHeader()->PrintDcmHeader();
        //cout << "OUTPUT IMAGE (updated header): " << *( this->reslicedImage ) << endl;
    }

    
}



/*!
 *  Sets the new spacing into DCM header.
 */
void svkObliqueReslice::SetReslicedHeaderSpacing()
{

    this->reslicedImage->GetSpacing(this->newSpacing);

    cout << endl;
    ostringstream* oss = new ostringstream();
    *oss << this->newSpacing[0];
    string pixelSpacingString( oss->str() + "\\" );
    delete oss;

    oss = new ostringstream();
    *oss << this->newSpacing[1];
    pixelSpacingString.append( oss->str() );
    
    delete oss;
    oss = new ostringstream();
    *oss << this->newSpacing[2];
    string sliceThicknessString( oss->str() );

    this->GetOutput()->GetDcmHeader()->InitPixelMeasuresMacro(
        pixelSpacingString,
        sliceThicknessString
    );

    delete oss;
}


/*!
 *  Calculate the new slice positins. 
 */
void svkObliqueReslice::SetReslicedHeaderPerFrameFunctionalGroups()
{
    
    double newTlc[3];
    this->ComputeTopLeftCorner( newTlc );
    this->reslicedImage->SetOrigin( newTlc );

    int numSlices = this->GetOutput()->GetDimensions()[2];
    int numTimepoints = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfTimePoints();
    int numCoils = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfCoils();

    svkDcmHeader::DimensionVector dimensionVector = this->GetImageDataInput(0)->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, numSlices-1);
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::TIME_INDEX, numTimepoints-1);
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::CHANNEL_INDEX, numCoils-1);

    this->GetOutput()->GetDcmHeader()->InitPerFrameFunctionalGroupSequence(
        newTlc, this->newSpacing, this->targetDcos, &dimensionVector
    );

}


/*!
 *  Reset the DICOM orientation 
 */
void svkObliqueReslice::SetReslicedHeaderOrientation()
{
    string orientationString;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            ostringstream dcosValue;
            dcosValue << this->targetDcos[i][j];
            orientationString.append( dcosValue.str() ) ;
            if (i < 2) {
                orientationString.append( "\\") ;
            }
        }
    }

    this->reslicedImage->GetDcmHeader()->AddSequenceItemElement(
        "PlaneOrientationSequence",
        0,
        "ImageOrientationPatient",
        orientationString,
        "SharedFunctionalGroupsSequence",
        0
    );

        //  Determine whether the data is ordered with or against the slice normal direction.
    double normal[3];
    this->reslicedImage->GetDcmHeader()->GetNormalVector(normal);
    svkDcmHeader::DcmDataOrderingDirection dataSliceOrder = svkDcmHeader::SLICE_ORDER_UNDEFINED;

    double dcosSliceOrder[3];
    for (int i = 0; i < 3; i++) {
        dcosSliceOrder[i] = this->targetDcos[2][i];
    }

    //  Use the scalar product to determine whether the data in the .cmplx 
    //  file is ordered along the slice normal or antiparalle to it. 
    vtkMath* math = vtkMath::New();
    if (math->Dot(normal, dcosSliceOrder) > 0 ) {
        dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
    } else {
        dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_NEG_NORMAL;
    }
    this->reslicedImage->GetDcmHeader()->SetSliceOrder( dataSliceOrder );
    math->Delete();

}


/*!
 * Computes the new origin for the output data object.
 */
void svkObliqueReslice::ComputeTopLeftCorner(double newTlc[3])
{
    //  Get Center of original image volume:
    //  first get the tlc of the stack:
    double* tlc0 = new double[3];
    this->GetImageDataInput(0)->GetDcmHeader()->GetOrigin(tlc0, 0);

    double* inputSpacing = new double[3];
    this->GetImageDataInput(0)->GetDcmHeader()->GetPixelSpacing(inputSpacing);

    int numVoxels[3];
    this->GetImageDataInput(0)->GetNumberOfVoxels(numVoxels);

    double dcosIn[3][3];
    this->GetImageDataInput(0)->GetDcmHeader()->GetDataDcos(dcosIn);

    if( !this->GetMatchSpacingAndFovOn()) {
        //  Now calculate the volumetric center by displacing by 1/2 fov - 1/2 voxel from tlc position:
        double origin[3];
        for (int i = 0; i < 3; i++) {
            origin[i] = tlc0[i];
            for (int j = 0; j < 3; j++) {
                origin[i] += dcosIn[j][i] * (inputSpacing[j] * ((numVoxels[j]-1)/2.) );
            }
        }

        //  Now calculate the NEW tlc:
        //  this->targetDcos
        //  this->newSpacing
        //  this->newNumVoxels
        for (int i = 0; i < 3; i++) {
            newTlc[i] = origin[i];
            for (int j = 0; j < 3; j++) {
                newTlc[i] -= targetDcos[j][i] * (this->newSpacing[j] * ((this->newNumVoxels[j] - 1)/2.) );
            }
        }
    } else {
        this->GetPortMapper()->GetImageInputPortValue(TARGET_IMAGE)->GetDcmHeader()->GetOrigin(newTlc);
    }
    delete[] tlc0;
    delete[] inputSpacing;

}


/*!
 *
 */
void svkObliqueReslice::Print3x3(double matrix[3][3], string name) 
 {
    cout << name << "1:  " << matrix[0][0] << "    " << matrix[0][1]<<"    "<<matrix[0][2] <<endl; 
    cout << name << "2:  " << matrix[1][0] << "    " << matrix[1][1]<<"    "<<matrix[1][2] <<endl; 
    cout << name << "3:  " << matrix[2][0] << "    " << matrix[2][1]<<"    "<<matrix[2][2] <<endl << endl; 
}


