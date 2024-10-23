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


#include <svkTransform.h>

#include </usr/include/vtk/vtkImageChangeInformation.h>
#include </usr/include/vtk/vtkInformationVector.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>


using namespace svk;


//vtkCxxRevisionMacro(svkTransform, "$Rev$");
vtkStandardNewMacro(svkTransform);


/*!
 *
 */
svkTransform::svkTransform()
{
#if VTK_DEBUG_ON
    //this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");
    this->translation[0]= 0.; 
    this->translation[1]= 0.; 
    this->translation[2]= 0.; 
    this->isTranslationColRowSlice = false;
}


/*!
 *
 */
svkTransform::~svkTransform()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~svkTransform()");
    
}


/*!
 *  Set the translation along the LP and S directions. 
 *  direction. 
 */
void svkTransform::SetTranslationLPS( float dL, float dP, float dS)
{
    this->translation[0] = dL; 
    this->translation[1] = dP; 
    this->translation[2] = dS; 
    this->isTranslationColRowSlice = false;
    if ( this->GetDebug() ) {
        cout << "Translation: " << this->translation[0] << " " << this->translation[1] << " " <<  this->translation[2] << endl;
    }
}

/*!
 *  Set the translation along the Column (dx), Row (dy) and Slice (dz) 
 *  direction. 
 */
void svkTransform::SetTranslationCRS( float dC, float dR, float dS)
{
    this->translation[0] = dC; 
    this->translation[1] = dR; 
    this->translation[2] = dS; 
    this->isTranslationColRowSlice = true;
    if ( this->GetDebug() ) {
        cout << "Translation: " << this->translation[0] << " " << this->translation[1] << " " <<  this->translation[2] << endl;
    }
}


/*!
 *
 */
int svkTransform::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    return 1;
}


/*!
 *  Execute the transformation. 
 */
int svkTransform::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) 
{
    this->UpdateHeader(); 
    return 1; 
}


/*!
 *
 */
void svkTransform::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
}


/*!
 *  Set the header info to match the resliced data set: 
 */
void svkTransform::UpdateHeader()
{
    string seriesDescription("RESLICED ");
	seriesDescription.append(this->GetImageDataInput(0)->GetDcmHeader()->GetStringValue("SeriesDescription"));

    this->TransformHeaderPerFrameFunctionalGroups();

    
}



/*!
 *  Calculate the new ImagePositionPatient values in the PerFrameFunctinalGroup. 
 */
void svkTransform::TransformHeaderPerFrameFunctionalGroups()
{
    
    //  Get Center of original image volume: 
    //  first get the top left corner (TLC) of the stack:
    double* tlcIn = new double[3]; 
    this->GetImageDataInput(0)->GetDcmHeader()->GetOrigin(tlcIn, 0);

    double* inputSpacing = new double[3]; 
    this->GetImageDataInput(0)->GetDcmHeader()->GetPixelSpacing(inputSpacing); 

    int numVoxels[3]; 
    this->GetImageDataInput(0)->GetNumberOfVoxels(numVoxels);

    double dcosIn[3][3];
    this->GetImageDataInput(0)->GetDcmHeader()->GetDataDcos(dcosIn); 

    //  Now calculate the new TLC by applying translation
    double tlcOut[3]; 
    if ( this->isTranslationColRowSlice == true ) { 
        //  Translation specified in col, row, slice coordinates (i.e. image frame): 
        for (int i = 0; i < 3; i++) {
            tlcOut[i] = tlcIn[i];  
            for (int j = 0; j < 3; j++) {
                tlcOut[i] += dcosIn[j][i] * ( this->translation[j] );
            }
        }
    } else {
        //  Translation specified in LPS coordinates: 
        for (int i = 0; i < 3; i++) {
            tlcOut[i] = tlcIn[i] + this->translation[i];  
        }
    }

    svkDcmHeader::DimensionVector dimensionVector = this->GetImageDataInput(0)->GetDcmHeader()->GetDimensionIndexVector();
    this->GetOutput()->GetDcmHeader()->InitPerFrameFunctionalGroupSequence(
        tlcOut, 
        inputSpacing, 
        dcosIn, 
        &dimensionVector
    ); 

    delete[] tlcIn;
    delete[] inputSpacing;
}



/*!
 *  Oblique Reslice only works with image data.  Output will be of the same type as input 
 *  which is the default behavior for an svkImageAlgorithm. 
 */
int svkTransform::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
    return 1;
}

