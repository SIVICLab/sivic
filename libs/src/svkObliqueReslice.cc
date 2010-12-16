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


#include <svkObliqueReslice.h>

#include <vtkImageChangeInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>


using namespace svk;


vtkCxxRevisionMacro(svkObliqueReslice, "$Rev$");
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

    this->reslicer      = vtkImageReslice::New(); 
    this->reslicedImage = NULL;  
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
 * 
 */
void svkObliqueReslice::SetTargetDcosFromImage(svkImageData* image)
{
    image->GetDcmHeader()->GetDataDcos(this->targetDcos); 
    cout << "reslicer input in dcos setter: " << *(this->GetImageDataInput(0))<<endl;
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
 *
 */
int svkObliqueReslice::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    cout << "reslicer input: " << *( this->GetImageDataInput(0) ) << endl;
    this->reslicer->SetInput( this->GetImageDataInput(0) ); 

    double rotation[3][3];   
    this->SetRotationMatrix(); 

    this->reslicer->SetResliceAxesDirectionCosines(
        this->rotation[0][0],
        this->rotation[0][1],
        this->rotation[0][2],
        this->rotation[1][0],
        this->rotation[1][1],
        this->rotation[1][2],
        this->rotation[2][0],
        this->rotation[2][1],
        this->rotation[2][2]
    );
    
    reslicer->Update();

    vtkInformation* reslicedInfo = this->reslicer->GetOutput()->GetInformation(); 
    vtkInformation* outInfo = outputVector->GetInformationObject(0);

    int outWholeExt[6];
    reslicedInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), outWholeExt);

    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), outWholeExt, 6);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), outWholeExt, 6);

    // These values are not quite right... we only know the correct origin after request data is run
    outInfo->Set(vtkDataObject::SPACING(), this->reslicer->GetOutput()->GetSpacing(), 3);

    return 1;

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

    //  Copy the vtkImageAlgo output to the allocated svkImageData output image
    this->GetOutput()->DeepCopy( this->reslicer->GetOutput() ); 

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

    double dcosIn[3][3];   
    this->GetImageDataInput(0)->GetDcmHeader()->GetDataDcos(dcosIn); 

    double dcosInInverse[3][3];   
    vtkMath::Invert3x3(dcosIn, dcosInInverse);

    vtkMath::Multiply3x3(targetDcos, dcosInInverse, this->rotation);

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
    string seriesDescription("resliced"); 

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
    this->reslicedImage->GetDcmHeader()->SetValue( 
        "NumberOfFrames", 
        this->newNumVoxels[2]
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
 *  Calculate the oblique spacing and reset header values: 
 *  This needs to be calculated since the input spacing may not have been correct for 
 *  obliquely oriented data sets. 
 */
void svkObliqueReslice::SetReslicedHeaderSpacing()
{
    double* inputSpacing = new double[3]; 
    this->GetImageDataInput(0)->GetDcmHeader()->GetPixelSpacing(inputSpacing); 

    //  Get the coordinate system with length representing the pixel spacing in each
    //  dimension Just multiply each vector or input dcos, but the pixel spacing
    //  in that direction: 
    double dcosIn[3][3];   
    this->GetImageDataInput(0)->GetDcmHeader()->GetDataDcos(dcosIn); 

    //  Normalize each of the input coordinate system axes to the length of the pixel spacing along
    //  that axis: 
    double spacingAxesIn[3][3];   
    for (int i = 0; i < 3; i++) {
        vtkMath::Normalize(dcosIn[i]);
        for (int j = 0; j < 3; j++) {
            spacingAxesIn[i][j] = inputSpacing[i] * dcosIn[i][j];
        }
    }

    this->Print3x3(dcosIn, "dcosIn");
    this->Print3x3(spacingAxesIn, "spacingAxesIn");

    double calculatedTargetDcos[3][3]; 
    this->RotateAxes(dcosIn, calculatedTargetDcos); 
    this->Print3x3(calculatedTargetDcos, "caluclated target dcos");
    this->Print3x3(targetDcos, "intended targetDcos");

    double newSpacingAxes[3][3]; 
    vtkMath::Multiply3x3(this->rotation, inputSpacing, this->newSpacing); 
    cout << "new spacing: " << this->newSpacing[0] << " " << this->newSpacing[1] << " " << this->newSpacing[2] << endl;

    this->RotateAxes(spacingAxesIn, newSpacingAxes); 

    for (int i = 0; i < 3; i++) {
        double* axis = new double[3]; 
        axis[0] = newSpacingAxes[i][0]; 
        axis[1] = newSpacingAxes[i][1]; 
        axis[2] = newSpacingAxes[i][2]; 
        this->newSpacing[i] = vtkMath::Norm(axis); 
        cout << "new spacing axes norm : " << this->newSpacing[i] << endl;
        delete[] axis; 
    }

    cout << endl;
    ostringstream* oss = new ostringstream();
    *oss << this->newSpacing[0];
    string pixelSpacingString( oss->str() + "\\" );
    delete oss;

    oss = new ostringstream();
    *oss << this->newSpacing[1];
    pixelSpacingString.append( oss->str() );
    
    this->reslicedImage->GetDcmHeader()->AddSequenceItemElement(
        "PixelMeasuresSequence",
        0,
        "PixelSpacing",
        pixelSpacingString,  
        "SharedFunctionalGroupsSequence",
        0
    );

    delete oss;
    oss = new ostringstream();
    *oss << this->newSpacing[2];
    string sliceThicknessString( oss->str() );
    this->reslicedImage->GetDcmHeader()->AddSequenceItemElement(
        "PixelMeasuresSequence",
        0,
        "SliceThickness",
        sliceThicknessString,
        "SharedFunctionalGroupsSequence",
        0
    );

    delete oss;
    delete[] inputSpacing; 
}


/*!
 *  Calculate the new slice positins. 
 */
void svkObliqueReslice::SetReslicedHeaderPerFrameFunctionalGroups()
{
    
    //  Get Center of original image volume: 
    //  first get the tlc of the stack:
    double* tlc0 = new double[3]; 
    this->GetImageDataInput(0)->GetDcmHeader()->GetOrigin(tlc0, 0);

    double* inputSpacing = new double[3]; 
    this->GetImageDataInput(0)->GetDcmHeader()->GetPixelSpacing(inputSpacing); 

    int numVoxels[3]; 
    numVoxels[0] = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "Columns" ); 
    numVoxels[1] = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "Rows" ); 
    numVoxels[2] = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "NumberOfFrames" ); 

    double dcosIn[3][3];
    this->GetImageDataInput(0)->GetDcmHeader()->GetDataDcos(dcosIn); 

    //  Now calculate the volumetric center by displacing by 1/2 fov - 1/2 voxel from tlc position: 
    double origin[3]; 
    for (int i = 0; i < 3; i++) {
        origin[i] = tlc0[i];  
        for (int j = 0; j < 3; j++) {
            origin[i] += dcosIn[j][i] * (inputSpacing[j] * ((numVoxels[j]-1)/2.) );
        }
    }

    cout << "original tlc: " << tlc0[0] << " " << tlc0[1] << " " << tlc0[2] << endl;
    cout << "original spacing: " << inputSpacing[0] << " " << inputSpacing[1] << " " << inputSpacing[2] << endl;
    cout << "original voxels: " << numVoxels[0] << " " << numVoxels[1] << " " << numVoxels[2] << endl;
    cout << "original ORIGIN: " << origin[0] << " " << origin[1] << " " << origin[2] << endl;

    //  Now calculate the NEW tlc: 
    //  this->targetDcos 
    //  this->newSpacing
    //  this->newNumVoxels
    double newTlc[3]; 
    for (int i = 0; i < 3; i++) {
        newTlc[i] = origin[i];  
        for (int j = 0; j < 3; j++) {
            newTlc[i] -= targetDcos[j][i] * (this->newSpacing[j] * ((this->newNumVoxels[j] - 1)/2.) );
        }
    }
    cout << "new tlc: " << newTlc[0] << " " << newTlc[1] << " " << newTlc[2] << endl;
    this->reslicedImage->SetOrigin( newTlc );

    float displacement[3];
    float frameLPSPosition[3];

    int numberOfFrames = this->reslicedImage->GetDcmHeader()->GetIntValue( "NumberOfFrames" ); 

    this->reslicedImage->GetDcmHeader()->ClearSequence( "PerFrameFunctionalGroupsSequence" ); 

    if ( this->GetDebug() ) {
        //this->reslicedImage->GetDcmHeader()->PrintDcmHeader();
    }

    for (int i = 0; i < numberOfFrames; i++) {

        this->reslicedImage->GetDcmHeader()->AddSequenceItemElement(
            "PerFrameFunctionalGroupsSequence",
            i,
            "PlanePositionSequence"
        );

        //add displacement along normal vector:
        for (int j = 0; j < 3; j++) {
            displacement[j] = this->targetDcos[2][j] * this->newSpacing[2] * i;
        }
        for(int j = 0; j < 3; j++) { //L, P, S
            frameLPSPosition[j] = newTlc[j] +  displacement[j] ;
        }

        string imagePositionPatient;
        for (int j = 0; j < 3; j++) {
            ostringstream oss;
            oss.precision(8);
            oss << frameLPSPosition[j];
            imagePositionPatient += oss.str();
            if (j < 2) {
               imagePositionPatient += '\\';
            }
        }

        this->reslicedImage->GetDcmHeader()->AddSequenceItemElement(
            "PlanePositionSequence",
            0,
            "ImagePositionPatient",
            imagePositionPatient,
            "PerFrameFunctionalGroupsSequence",
            i
        );


        this->reslicedImage->GetDcmHeader()->AddSequenceItemElement(
            "PerFrameFunctionalGroupsSequence",
            i,
            "FrameContentSequence"
        );

        this->reslicedImage->GetDcmHeader()->AddSequenceItemElement(
            "FrameContentSequence",
            0,
            "FrameAcquisitionDateTime",
            "EMPTY_ELEMENT",
            "PerFrameFunctionalGroupsSequence",
            i
        );

        this->reslicedImage->GetDcmHeader()->AddSequenceItemElement(
            "FrameContentSequence",
            0,
            "FrameReferenceDateTime",
            "EMPTY_ELEMENT",
            "PerFrameFunctionalGroupsSequence",
            i
        );

        this->reslicedImage->GetDcmHeader()->AddSequenceItemElement(
            "FrameContentSequence",
            0,
            "FrameAcquisitionDuration",
            "-1",
            "PerFrameFunctionalGroupsSequence",
            i
        );

    }

    delete[] tlc0;
    delete[] inputSpacing;
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
}


/*
 *  Applies calculated rotation matrix to rotate coordinate system 
 *  vectors (dcos, with matrix with rows representing axes), needs
 *  to be transposed to apply rotation
 */
void svkObliqueReslice::RotateAxes(double axesIn[3][3], double rotatedAxes[3][3])
{
    //double axesInTsp[3][3]; 
    //vtkMath::Transpose3x3(axesIn, axesInTsp); 

    //double rotatedAxes[3][3]; 
    vtkMath::Multiply3x3(this->rotation, axesIn, rotatedAxes); 

    //vtkMath::Transpose3x3(rotatedAxesTsp, rotatedAxes); 
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


/*!
 *  Oblique Reslice only works with image data.  Output will be of the same type as input 
 *  which is the default behavior for an svkImageAlgorithm. 
 */
int svkObliqueReslice::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
    return 1;
}

