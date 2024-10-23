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


#include <svkOverlayContourDirector.h>
#include </usr/include/vtk/vtkObjectFactory.h>

using namespace svk;


vtkStandardNewMacro(svkOverlayContourDirector);


//! Constructor 
svkOverlayContourDirector::svkOverlayContourDirector()
{
    this->referenceImage = NULL;
}


//! Destructor
svkOverlayContourDirector::~svkOverlayContourDirector()
{
}


/*!
 * Sets the reference image by which the contours are sliced.
 * @param referenceImage
 */
void svkOverlayContourDirector::SetReferenceImage(svkMriImageData *referenceImage)
{
    this->referenceImage = referenceImage;
}


/*!
 * Add an input. Will return the vtkActor for the input
 * @param image
 * @return
 */
vtkActor* svkOverlayContourDirector::AddInput(svkMriImageData *image)
{
    vtkExtractVOI* extractVOI = vtkExtractVOI::New();
    extractVOI->SetInputData(image);
    vtkContourFilter* contourFilter =  vtkContourFilter::New();
    contourFilter->SetInputConnection(extractVOI->GetOutputPort());
    this->contourFilters.push_back(contourFilter);
    vtkPolyDataMapper* contourMapper = vtkPolyDataMapper::New();

    // Create an actor for the contours
    vtkActor* contourActor = vtkActor::New();
    contourActor->SetMapper(contourMapper);
    contourMapper->Delete();


    contourActor->GetProperty()->SetLineWidth(3);
    contourActor->GetProperty()->SetEdgeVisibility(1);
    this->contourActors.push_back(contourActor);
    ContourColor color = ContourColor(((this->contourFilters.size())-1)%(LAST_COLOR+1));
    this->SetContourColor(this->contourFilters.size()-1, color );
    this->dataVector.push_back(image);
    return contourActor;
}


/*!
 * Set the slice for all contour actors.
 *
 * @param referenceSlice
 * @param orientation
 */
void svkOverlayContourDirector::SetSlice( int referenceSlice, svkDcmHeader::Orientation orientation )
{
    for( int i = 0; i < this->contourFilters.size(); i++) {
        svkMriImageData* contourData = this->dataVector[i];
        int* extent = contourData->GetExtent();
        int contourExtent[6];
        memcpy( contourExtent, extent, sizeof(int)*6);
        int sliceIndex = contourData->GetOrientationIndex( orientation );
        int contourSlice = this->FindOverlayContourSlice(referenceSlice, orientation, i);
        if( contourSlice >= contourExtent[4]
                && contourSlice <= contourExtent[5] ) {
            contourExtent[2 * sliceIndex] = this->FindOverlayContourSlice(referenceSlice, orientation, i);
            contourExtent[2 * sliceIndex + 1] = this->FindOverlayContourSlice(referenceSlice, orientation, i);
            this->contourActors[i]->SetVisibility(1);
        } else {
            contourSlice = 0;
            this->contourActors[i]->SetVisibility(0);
        }

        vtkContourFilter *contourFilter = vtkContourFilter::New();
        vtkExtractVOI *contourExtractor = vtkExtractVOI::SafeDownCast(
                this->contourFilters[i]->GetInputAlgorithm());
        contourExtractor->SetVOI(contourExtent);
        contourFilter->SetInputConnection(contourExtractor->GetOutputPort());
        this->contourFilters[i] = contourFilter;
        this->contourFilters[i]->GenerateValues(1, 0.5, 1.5);

        vtkTubeFilter* tubeFilter = vtkTubeFilter::New();
        tubeFilter->SetInputConnection(this->contourFilters[i]->GetOutputPort());
        tubeFilter->SetRadius(0.025);
        tubeFilter->SetNumberOfSides(8);
        tubeFilter->Update();

        this->contourActors[i]->GetMapper()->SetInputConnection(tubeFilter->GetOutputPort());
        tubeFilter->Delete();
        double *origin = contourData->GetOrigin();
        double dcos[3][3];
        contourData->GetDcos(dcos);
        vtkTransform *dcosTransform = this->GetContourTransform( contourData, contourSlice, referenceSlice, orientation);

        this->contourActors[i]->SetUserTransform(dcosTransform);
        this->contourActors[i]->Modified();
    }

}


/*!
 *  Generates a vtkTransform to apply dcos and shift slice for contour actors.
 */
vtkTransform* svkOverlayContourDirector::GetContourTransform(svkImageData* contourData, int contourSlice,
                                                             int referenceSlice,
                                                             svkDcmHeader::Orientation orientation) {

    double dcos[3][3];
    contourData->GetDcos(dcos);
    double* origin = contourData->GetOrigin();
    double* spacing = contourData->GetSpacing();
    vtkTransform* dcosTransform = vtkTransform::New();
    dcosTransform->Identity();
    // Translations occur around 0,0,0 so to get our matrix we untranslate, apply dcos, and retranslate
    vtkTransform* untranslate = vtkTransform::New();
    untranslate->Identity();
    untranslate->Translate(-origin[0], -origin[1], -origin[2] );

    vtkTransform* rotationTransform = vtkTransform::New();
    rotationTransform->Identity();
    vtkMatrix4x4* rotationMatrix = vtkMatrix4x4::New();
    for( int i = 0; i < 3; i++) {
        for( int j = 0; j < 3; j++) {
            rotationMatrix->SetElement(i,j,dcos[j][i]);
        }
    }
    rotationTransform->SetMatrix(rotationMatrix);
    rotationMatrix->Delete();

    vtkTransform* retranslate = vtkTransform::New();
    retranslate->Identity();
    retranslate->Translate(origin[0], origin[1], origin[2] );

    dcosTransform->PostMultiply();
    dcosTransform->Concatenate( untranslate );
    dcosTransform->Concatenate( rotationTransform );
    dcosTransform->Concatenate( retranslate );


    untranslate->Delete();
    rotationTransform->Delete();
    retranslate->Delete();
    vtkTransform* referenceImageSliceOffsetTransform = vtkTransform::New();
    double* imageOrigin = this->referenceImage->GetOrigin();
    referenceImageSliceOffsetTransform->Identity();
    double normal[3];

    contourData->GetSliceNormal( normal, orientation );
    double sliceNormal[3] = { normal[0], normal[1], normal[2] };
    int sliceIndex = referenceImage->GetOrientationIndex( orientation );
    double* imageSpacing = referenceImage->GetSpacing();

    double delta  = (vtkMath::Dot(imageOrigin, sliceNormal )+(referenceSlice)*imageSpacing[sliceIndex] -
                     (vtkMath::Dot( origin, sliceNormal ) + spacing[sliceIndex] * contourSlice));
    referenceImageSliceOffsetTransform->Translate(delta*sliceNormal[0], delta*sliceNormal[1], delta*sliceNormal[2]);
    dcosTransform->Concatenate(referenceImageSliceOffsetTransform);
    referenceImageSliceOffsetTransform->Delete();

    return dcosTransform;
}


/*!
 *  Finds the reference slice that most closely corresponds to the contour slice.
 */
int svkOverlayContourDirector::FindOverlayContourSlice( int referenceSlice, svkDcmHeader::Orientation orientation, int overlayIndex )
{
    int overlaySlice = -1;
    double sliceCenter[3];
    double tolerance = 0;
    referenceImage->GetSliceOrigin( referenceSlice, sliceCenter, orientation );
    int index = referenceImage->GetOrientationIndex( orientation );
    tolerance = this->dataVector[overlayIndex]->GetSpacing()[index]/2.0;
    overlaySlice = this->dataVector[overlayIndex]->GetClosestSlice( sliceCenter, orientation, tolerance );
    return overlaySlice;
}

void svkOverlayContourDirector::SetContourColor(int index, ContourColor color)
{
        switch( color ) {
            case GREEN:
                this->contourActors[index]->GetProperty()->SetEdgeColor(0, 1, 0);
                break;
            case RED:
                this->contourActors[index]->GetProperty()->SetEdgeColor(1, 0, 0);
                break;
            case BLUE:
                this->contourActors[index]->GetProperty()->SetEdgeColor(0, 0, 1);
                break;
            case PINK:
                this->contourActors[index]->GetProperty()->SetEdgeColor(242.0/256.0,135.0/256.0, 233.0/246.0);
                break;
            case YELLOW:
                this->contourActors[index]->GetProperty()->SetEdgeColor(1, 1, 0.5);
                break;
            case CYAN:
                this->contourActors[index]->GetProperty()->SetEdgeColor(0.5, 1, 1);
                break;
            case ORANGE:
                this->contourActors[index]->GetProperty()->SetEdgeColor(1, 0.5, 0);
                break;
            case GRAY:
                this->contourActors[index]->GetProperty()->SetEdgeColor(0.5, 0.5, 0.5);
                break;
            default :
                this->contourActors[index]->GetProperty()->SetEdgeColor(1, 1, 1);
        }
}