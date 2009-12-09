/*
 *  Copyright © 2009 The Regents of the University of California.
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

#define DEBUG 0 

#include <svkOverlayView.h>


using namespace svk;


vtkCxxRevisionMacro(svkOverlayView, "$Rev$");
vtkStandardNewMacro(svkOverlayView);


/*!
 *  Constructor creates the vtkImageView2, and sets its size.
 */
svkOverlayView::svkOverlayView()
{
    this->imageViewer = svkImageViewer2::New();
    this->satBands = svkSatBandSet::New();
    this->satBandsPerp1 = svkSatBandSet::New();
    this->satBandsPerp2 = svkSatBandSet::New();
    this->slice = 0;
    this->dataVector.push_back( NULL );
    this->dataVector.push_back( NULL );
    this->dataVector.push_back( NULL );
    this->rwi = NULL;
    this->myRenderWindow = NULL;
    this->tlcBrc = new int[2];
    this->tlcBrc[0] = -1;
    this->tlcBrc[1] = -1;

    this->windowLeveler = NULL;
    this->colorTransfer = NULL ;
    // Create a state vector for our images

    this->isPropOn.assign(LAST_PROP+1, FALSE);
    this->isRendererOn.assign(LAST_RENDERER+1, FALSE);
    this->isPropVisible.assign(LAST_PROP+1, FALSE);     //Is the actor in the views FOV?
    
    // This fixes a compiler error in OS X, not sure why....
    vtkRenderer* nullRenderer = NULL;
    vtkProp* nullProp = NULL;
    this->renCollection.assign(svkOverlayView::LAST_RENDERER+1, nullRenderer);     //Is the actor in the views FOV?
    this->propCollection.assign(svkOverlayView::LAST_PROP+1, nullProp);     //Is the actor in the views FOV?

    vtkActor* entirePlotGrid = vtkActor::New();
    this->SetProp( svkOverlayView::PLOT_GRID, entirePlotGrid );
    entirePlotGrid->Delete();

    this->SetProp( svkOverlayView::VOL_SELECTION, NULL );

    svkOpenGLOrientedImageActor* overlayActor = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkOverlayView::OVERLAY_IMAGE, overlayActor );
    overlayActor->Delete();

    svkOpenGLOrientedImageActor* overlayActorBack = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkOverlayView::OVERLAY_IMAGE_BACK, overlayActorBack );
    overlayActorBack->Delete();

    vtkScalarBarActor* bar = vtkScalarBarActor::New();
    this->SetProp( svkOverlayView::COLOR_BAR, bar );
    bar->Delete();

    this->SetProp( svkOverlayView::SAT_BANDS, this->satBands->GetSatBandsActor() );
    this->TurnPropOff( svkOverlayView::SAT_BANDS );

    this->SetProp( svkOverlayView::SAT_BANDS_OUTLINE, this->satBands->GetSatBandsOutlineActor() );
    this->TurnPropOff( svkOverlayView::SAT_BANDS_OUTLINE );

    this->SetProp( svkOverlayView::SAT_BANDS_PERP1_OUTLINE, this->satBandsPerp1->GetSatBandsOutlineActor() );
    this->TurnPropOff( svkOverlayView::SAT_BANDS_PERP1_OUTLINE );

    this->SetProp( svkOverlayView::SAT_BANDS_PERP2_OUTLINE, this->satBandsPerp2->GetSatBandsOutlineActor() );
    this->TurnPropOff( svkOverlayView::SAT_BANDS_PERP2_OUTLINE );

    this->satBandsPerp1->SetOrientation( svkSatBandSet::YZ );
    this->satBandsPerp2->SetOrientation( svkSatBandSet::XZ );

    this->SetProp( svkOverlayView::SAT_BANDS_PERP1, this->satBandsPerp1->GetSatBandsActor() );
    this->TurnPropOff( svkOverlayView::SAT_BANDS_PERP1 );

    this->SetProp( svkOverlayView::SAT_BANDS_PERP2, this->satBandsPerp2->GetSatBandsActor() );
    this->TurnPropOff( svkOverlayView::SAT_BANDS_PERP2 );

    
    this->interpolationType = NEAREST; 
}


/*!
 *  Destructor deletes the vtkImageViewer2.
 */
svkOverlayView::~svkOverlayView()
{

    if( this->imageViewer != NULL ) {   
        this->imageViewer->Delete();
        this->imageViewer = NULL;
    }

    if( this->rwi != NULL ) {
        this->rwi->Delete();
        this->rwi = NULL;
    }

    if( this->windowLeveler != NULL ) {
        this->windowLeveler->Delete();
        this->windowLeveler = NULL;     
    }

    if( this->colorTransfer != NULL ) {
        this->colorTransfer->Delete();
        this->colorTransfer = NULL;     
    }
    if( this->satBands != NULL ) {
        this->satBands->Delete();
        this->satBands = NULL;     
    }
    if( this->satBandsPerp1 != NULL ) {
        this->satBandsPerp1->Delete();
        this->satBandsPerp1 = NULL;     
    }
    if( this->satBandsPerp2 != NULL ) {
        this->satBandsPerp2->Delete();
        this->satBandsPerp2 = NULL;     
    }

    delete[] tlcBrc;

}


/*!
 *   Sets the input of the vtkImageViewer2. It also resets the camera view and 
 *   the slice.
 *
 *   \param resetViewState boolean identifies of this is the first dataset input
 */        
void svkOverlayView::SetupMsInput( bool resetViewState ) 
{

    this->GetRenderer( svkOverlayView::PRIMARY)->DrawOff();
    vtkPolyDataMapper* entireGridMapper = vtkPolyDataMapper::New();
    entireGridMapper->ScalarVisibilityOff( );
    entireGridMapper->InterpolateScalarsBeforeMappingOff();
    entireGridMapper->ReleaseDataFlagOn();
    entireGridMapper->ImmediateModeRenderingOn();

    // We need a filter to pull out the edges of the data cells (voxels)
    vtkExtractEdges* edgeExtractor = vtkExtractEdges::New();
    double dcos[3][3];
    
    // Here we are making a copy of the image for the grid.
    // For some reason valgrind reports massive read errors
    // if the data arrays are present when passed to
    // vtkExtractEdges. 

    svkImageData* geometryData = svkMrsImageData::New();
    geometryData->SetOrigin( dataVector[MRS]->GetOrigin() );
    geometryData->SetSpacing( dataVector[MRS]->GetSpacing() );
    geometryData->SetExtent( dataVector[MRS]->GetExtent() );
    dataVector[MRS]->GetDcos(dcos);
    geometryData->SetDcos(dcos);
    //edgeExtractor->SetInput( dataVector[MRS] );
    edgeExtractor->SetInput( geometryData );
    geometryData->Delete(); 

    // Pipe the edges into the mapper 
    entireGridMapper->SetInput( edgeExtractor->GetOutput() );
    edgeExtractor->Delete();
    vtkActor::SafeDownCast( this->GetProp( svkOverlayView::PLOT_GRID))->SetMapper( entireGridMapper );
    entireGridMapper->Delete();
    vtkActor::SafeDownCast( GetProp( svkOverlayView::PLOT_GRID) )->GetProperty()->SetDiffuseColor( 0, 1, 0 );

    // Now we need to grab the selection box
    vtkActorCollection* selectionTopo = dataVector[MRS]->GetTopoActorCollection( 1 );
    selectionTopo->InitTraversal();
    this->SetProp( svkOverlayView::VOL_SELECTION, selectionTopo->GetNextActor());     
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::PLOT_GRID ) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::VOL_SELECTION) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_OUTLINE) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_PERP1) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_PERP1_OUTLINE) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_PERP2) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_PERP2_OUTLINE) );

    this->TurnPropOn( svkOverlayView::VOL_SELECTION );
    
    this->SetProp( svkOverlayView::PLOT_GRID, this->GetProp( svkOverlayView::PLOT_GRID ) );
    string acquisitionType = dataVector[MRS]->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
    if( acquisitionType != "SINGLE VOXEL" ) {
        this->TurnPropOn( svkOverlayView::PLOT_GRID );
    } else {
        this->TurnPropOff( svkOverlayView::PLOT_GRID );
    }

    this->SetSlice( slice );
    selectionTopo->Delete();
    if( resetViewState ) {
        this->imageViewer->ResetCamera( ); 
        this->HighlightSelectionVoxels();
    }

    this->satBands->SetInput( static_cast<svkMrsImageData*>(this->dataVector[MRS]) );
    this->satBandsPerp1->SetInput( static_cast<svkMrsImageData*>(this->dataVector[MRS]) );
    this->satBandsPerp2->SetInput( static_cast<svkMrsImageData*>(this->dataVector[MRS]) );
    if( this->dataVector[MRI] != NULL ) {
        int* extent = this->dataVector[MRI]->GetExtent();
        this->SetSlice((extent[3]-extent[2])/2,2);
        this->SetSlice((extent[1]-extent[0])/2,1);
    }

    this->GetRenderer( svkOverlayView::PRIMARY)->DrawOn();

}


/*!
 *  Sets the input of the vtkImageViewer2. It also resets the camera view and 
 *  the slice. This should be modified once the new image loader is written.
 *
 *   \param resetViewState boolean identifies of this is the first dataset input
 *
 */        
void svkOverlayView::SetupMrInput( bool resetViewState )
{
    // Rendering is set off to avoid uncontrolled rendering
    double cameraPosition[3];
    double cameraViewUp[3];
    double cameraFocus[3];
    
    this->GetRenderer(svkOverlayView::PRIMARY)->DrawOff();

    // Do we want to reset the camera?
    if( !resetViewState ) {

        memcpy( cameraPosition, 
                this->GetRenderer(svkOverlayView::PRIMARY)->GetActiveCamera()->GetPosition(), 
                sizeof(double)*3);
        memcpy(cameraViewUp, 
                this->GetRenderer(svkOverlayView::PRIMARY)->GetActiveCamera()->GetViewUp(), 
                sizeof(double)*3);
        memcpy(cameraFocus, 
                this->GetRenderer(svkOverlayView::PRIMARY)->GetActiveCamera()->GetFocalPoint(), 
                sizeof(double)*3);
    }
    
    // We need to execute these before resolving the state
    imageViewer->SetInput( dataVector[MRI] );
    imageViewer->SetSlice( slice );
    
    if( resetViewState ) {
        imageViewer->SetupInteractor( rwi );
        imageViewer->SetRenderWindow( myRenderWindow ); 
        imageViewer->SetRenderer( this->GetRenderer( svkOverlayView::PRIMARY ) ); 
        imageViewer->GetRenderer()->SetBackground(0.0,0.0,0.0);
        imageViewer->ResetCamera();
        imageViewer->GetRenderer()->GetActiveCamera()->SetParallelProjection(1);
        imageViewer->GetImageActor()->PickableOff();
        this->ResetWindowLevel();
    }
 
    int* extent = this->dataVector[MRI]->GetExtent();
    if( dataVector[MRS] == NULL ) {
        this->SetSlice( (extent[5]-extent[4])/2 );
    } 
    this->SetSlice((extent[1]-extent[0])/2,1);
    this->SetSlice((extent[3]-extent[2])/2,2);

    // And here we return the camera to its original state
    if( !resetViewState ) {
        this->GetRenderer( svkOverlayView::PRIMARY )->GetActiveCamera()->SetPosition( cameraPosition ); 
        this->GetRenderer( svkOverlayView::PRIMARY )->GetActiveCamera()->SetViewUp( cameraViewUp ); 
        this->GetRenderer( svkOverlayView::PRIMARY )->GetActiveCamera()->SetFocalPoint( cameraFocus ); 
    }
    
    
    rwi->Render();
    this->GetRenderer(svkOverlayView::PRIMARY)->DrawOn();

    // We need to reset the camera once Draw is on or the pipeline will not run
    imageViewer->ResetCamera();
    resetViewState = 0;

}


/*!
 *   Sets the data object in the object, also sets the input to the
 *   vtkImageViewer2 and renders it.
 *
 *   \param data the data you want to view
 *   \param index the index of the data you want to set
 *
 */        
void svkOverlayView::SetInput(svkImageData* data, int index)
{
    bool resetViewState = 1;
    // Check data compatiblity
    string resultInfo = this->GetDataCompatibility( data, index );
    if( strcmp( resultInfo.c_str(), "" ) == 0 ) { 
    
        if( dataVector[index] != NULL ) {
            if( dataVector[MRI] != NULL && dataVector[MRS] != NULL ) {
                resetViewState = 0; 
            }
            dataVector[index]->Delete();
            dataVector[index] = NULL;
        }
        data->Register( this );
        // We must register the dataset so it can be deleted
        ObserveData( data );
        dataVector[index] = data;
        if( index == OVERLAY ) {
            SetupOverlay();
        } else if( data->IsA("svkMriImageData") ) {
            SetupMrInput( resetViewState );
        } else if( data->IsA("svkMrsImageData") ) {
            SetupMsInput( resetViewState );
        } 
    } else {
        string message = "ERROR: Dataset NOT loaded!!! \n";
        message += resultInfo;
        cout << message.c_str() << endl;
    }
}


/*!
 *   Sets the current slice.
 *
 *   \param slice the slice you want to view
 */
void svkOverlayView::SetSlice(int slice)
{
    if( dataVector[MRI] != NULL ) {
        if( dataVector[MRS] != NULL ) { 
    
            if( tlcBrc[0] >= 0 && tlcBrc[1] >= 0 ) {
                int* extent = dataVector[MRS]->GetExtent();
                tlcBrc[0] += (slice-this->slice)*extent[1]*extent[3];
                tlcBrc[1] += (slice-this->slice)*extent[1]*extent[3];
            }
            this->slice = slice;
            GenerateClippingPlanes();
            // If it is make it visible, otherwise hide it
            if( static_cast<svkMrsImageData*>(this->dataVector[MRS])->SliceInSelectionBox( this->slice ) && isPropOn[VOL_SELECTION] ) {
                this->GetProp( svkOverlayView::VOL_SELECTION )->SetVisibility(1);
            } else {
                this->GetProp( svkOverlayView::VOL_SELECTION )->SetVisibility(0);
            }
            this->GetRenderer( svkOverlayView::PRIMARY)->DrawOff();
            this->SetCenterImageSlice( );
            this->SetSliceOverlay();
            this->satBands->SetClipSlice( slice );
            this->GetRenderer( svkOverlayView::PRIMARY)->DrawOn();
            this->Refresh();
        } else {
            this->imageViewer->SetSlice( slice );    
        } 
    } else {
        this->slice = slice;
    }
}


/*
 *  Sets the slices for the images orthogonal to the primary.
 */
void svkOverlayView::SetSlice(int slice, int imageNum)
{
    this->imageViewer->SetSlice( slice, imageNum );    
    if( imageNum == 1 ) {
        this->satBandsPerp1->SetClipSlice( this->FindSpectraSlice( slice, imageNum) );
    } else if ( imageNum == 2 ) {
        this->satBandsPerp2->SetClipSlice( this->FindSpectraSlice( slice, imageNum) );
    }
}


/*
 *  Finds the image slice that most closely corresponds to the input spectra slice.
 */
int svkOverlayView::FindCenterImageSlice( int spectraSlice, int orientation ) 
{
    int imageSlice;
    vtkImageData* tmpData;
    double* anatOrigin;
    double* anatSpacing;
    double* specOrigin;
    double* specSpacing;
    double sliceVoxelCenter;
    double dcos[3][3];
    int* imageExtent = dataVector[MRI]->GetExtent();
    int wIndex = 2-orientation;

    // If the spectral data has been set, find the center of the slice, 
    // and then the closest image slice to it
    if( dataVector[MRS] != NULL ) {
        tmpData = dataVector[MRI];
        anatOrigin = tmpData->GetOrigin();
        anatSpacing = tmpData->GetSpacing() ;
        tmpData = dataVector[MRS];
        specOrigin = tmpData->GetOrigin();
        specSpacing= tmpData->GetSpacing();
        dataVector[MRS]->GetDcos( dcos );

        double wVecSpec[3];
        wVecSpec[0] = dcos[wIndex][0];  
        wVecSpec[1] = dcos[wIndex][1];  
        wVecSpec[2] = dcos[wIndex][2];  

        dataVector[MRI]->GetDcos( dcos );

        double wVecImage[3];
        wVecImage[0] = dcos[wIndex][0];  
        wVecImage[1] = dcos[wIndex][1];  
        wVecImage[2] = dcos[wIndex][2];  
        // We project the origin of the spectroscopic data onto the wVec (slicing vector)
        double spectroCenter = vtkMath::Dot( specOrigin, wVecSpec ) + specSpacing[wIndex]*(spectraSlice+0.5); 
        double imageCenter = vtkMath::Dot( wVecImage, wVecSpec ) * spectroCenter; 
        double idealCenter = ( imageCenter-vtkMath::Dot( anatOrigin, wVecImage) )/anatSpacing[wIndex];
        imageSlice =(int) floor( idealCenter + 0.5); 
    } else {
        imageSlice = spectraSlice;
    }
    return imageSlice;

}


/*
 *  Finds the image slice that most closely corresponds to the input spectra slice.
 */
int svkOverlayView::FindSpectraSlice( int imageSlice, int orientation ) 
{
    int spectraSlice;
    vtkImageData* tmpData;
    double* imageOrigin;
    double* imageSpacing;
    double* specOrigin;
    double* specSpacing;
    double sliceVoxelCenter;
    double dcos[3][3];
    int* imageExtent = dataVector[MRI]->GetExtent();
    int wIndex = orientation-1;

    // If the spectral data has been set, find the center of the slice, 
    // and then the closest image slice to it
    if( dataVector[MRS] != NULL ) {
        tmpData = dataVector[MRI];
        imageOrigin = tmpData->GetOrigin();
        imageSpacing = tmpData->GetSpacing() ;
        tmpData = dataVector[MRS];
        specOrigin = tmpData->GetOrigin();
        specSpacing= tmpData->GetSpacing();
        dataVector[MRS]->GetDcos( dcos );

        double wVecSpec[3];
        wVecSpec[0] = dcos[wIndex][0];  
        wVecSpec[1] = dcos[wIndex][1];  
        wVecSpec[2] = dcos[wIndex][2];  

        dataVector[MRI]->GetDcos( dcos );

        double wVecImage[3];
        wVecImage[0] = dcos[wIndex][0];  
        wVecImage[1] = dcos[wIndex][1];  
        wVecImage[2] = dcos[wIndex][2];  
        // We project the origin of the spectroscopic data onto the wVec (slicing vector)
        double imageCenter = vtkMath::Dot( imageOrigin, wVecImage )  + imageSpacing[wIndex] * imageSlice; 
        double spectroCenter = vtkMath::Dot( wVecSpec, wVecImage ) * imageCenter; 
        double idealCenter = ( spectroCenter-vtkMath::Dot( specOrigin, wVecSpec) )/specSpacing[wIndex] -0.5;
        spectraSlice =(int) floor( idealCenter + 0.5); 
    } else {
        spectraSlice = imageSlice;
    }
    
    return spectraSlice;

}


/*!
 *  Sets the slice of the anatamical data, based on the spectroscopic slice.
 *  It calculates the anatomical slice closest to the center of the spectroscopic
 *  slice.
 *
 */
void svkOverlayView::SetCenterImageSlice()
{
    int imageSlice = FindCenterImageSlice(this->slice, 0);
    int* imageExtent = dataVector[MRI]->GetExtent();

    // Case if the the image is outside of the extent
    if( imageSlice >= imageExtent[4] && imageSlice <= imageExtent[5] ) {
        if( this->imageViewer->GetImageActor()->GetVisibility() == 0 ) {
            this->imageViewer->GetImageActor()->SetVisibility(1);
        }
        this->imageViewer->SetSlice( imageSlice );
    } else if( imageSlice > imageExtent[5] ) {
        this->imageViewer->SetSlice( imageExtent[5] );
        this->imageViewer->GetImageActor()->SetVisibility(0);
        this->imageViewer->GetImageActor()->Modified();
    } else {
        this->imageViewer->SetSlice( imageExtent[4] );
        this->imageViewer->GetImageActor()->SetVisibility(0);
        this->imageViewer->GetImageActor()->Modified();
    }
}


/*!
 *  Sets the vtkRenderWindowInteractor to be associated with this view.
 *
 *  \param rwi the vtkRenderWindowInteractor you wish this view to use
 */  
void svkOverlayView::SetRWInteractor( vtkRenderWindowInteractor* rwi )
{
    if( rwi == NULL ) {
        return;
    }
    if( this->rwi != NULL ) {
        this->rwi->Delete();
    }
    this->rwi = rwi;
    this->rwi->Register( this );
    this->myRenderWindow = this->rwi->GetRenderWindow();
    this->myRenderWindow->AddRenderer( this->GetRenderer(svkOverlayView::PRIMARY) );
    this->TurnRendererOn( svkOverlayView::PRIMARY );
}  


/*!
 *  Rerenders the vtkRenderWindow associated with the vtkRenderWindowInteractor
 *  used by this DataView.
 */
void svkOverlayView::Refresh()
{
    if( dataVector[MRI] != NULL ) {
        this->myRenderWindow->Render();
    }
}  

/*!
 *  Sets desired the current selection in Display (pixels) coordinates
 *  and highlights the intersected voxels.
 *
 *  \param selectionArea the area you wish to select voxels within [xmin, xmax, ymin, ymax]
 */ 
void svkOverlayView::SetSelection( double* selectionArea, bool isWorldCords )
{
    if( selectionArea != NULL && dataVector[MRS] != NULL) {
        double worldStart[3]; 
        double worldEnd[3]; 
        if( !isWorldCords ) {
            vtkCoordinate* coordStart = vtkCoordinate::New();
            vtkCoordinate* coordEnd = vtkCoordinate::New();
            coordStart->SetCoordinateSystemToDisplay();
            coordEnd->SetCoordinateSystemToDisplay();
            coordStart->SetValue(selectionArea[0], selectionArea[1], 0);
            coordEnd->SetValue(selectionArea[2], selectionArea[3], 0);
            worldStart[0] = *coordStart->GetComputedWorldValue(this->GetRenderer( svkOverlayView::PRIMARY)); 
            worldStart[1] = *(coordStart->GetComputedWorldValue(this->GetRenderer( svkOverlayView::PRIMARY)) + 1); 
            worldStart[2] = *(coordStart->GetComputedWorldValue(this->GetRenderer( svkOverlayView::PRIMARY)) + 2); 
            worldEnd[0] = *coordEnd->GetComputedWorldValue(this->GetRenderer( svkOverlayView::PRIMARY)); 
            worldEnd[1] = *(coordEnd->GetComputedWorldValue(this->GetRenderer( svkOverlayView::PRIMARY)) + 1); 
            worldEnd[2] = *(coordEnd->GetComputedWorldValue(this->GetRenderer( svkOverlayView::PRIMARY)) + 2); 
            coordStart->Delete();
            coordEnd->Delete();
        } else {
            worldStart[0] = selectionArea[0]; 
            worldStart[1] = selectionArea[2]; 
            worldStart[2] = selectionArea[4]; 
            worldEnd[0] = selectionArea[1]; 
            worldEnd[1] = selectionArea[3]; 
            worldEnd[2] = selectionArea[5]; 
        }
        double* origin  = dataVector[MRS]->GetOrigin();
        double* spacing = dataVector[MRS]->GetSpacing();
        double dcos[3][3];
        dataVector[MRS]->GetDcos( dcos );
        double xVec[3];
        xVec[0] = dcos[0][0];
        xVec[1] = dcos[1][0];
        xVec[2] = dcos[2][0];
        double yVec[3];
        yVec[0] = dcos[0][1];
        yVec[1] = dcos[1][1];
        yVec[2] = dcos[2][1];
        double uVec[3];
        uVec[0] = dcos[0][0];
        uVec[1] = dcos[0][1];
        uVec[2] = dcos[0][2];
        double vVec[3];
        vVec[0] = dcos[1][0];
        vVec[1] = dcos[1][1];
        vVec[2] = dcos[1][2];
        int* extent = dataVector[MRS]->GetExtent();
        int indexRangeX[2];
        int indexRangeY[2];
        double tmp;
        // Make Sure worldStart is less than worldEnd
        double originProjection[2];
        originProjection[0] = vtkMath::Dot( origin, uVec );
        originProjection[1] = vtkMath::Dot( origin, vVec );
        double worldStartProjection[2];
        worldStartProjection[0] = vtkMath::Dot( worldStart, uVec );
        worldStartProjection[1] = vtkMath::Dot( worldStart, vVec );
        double worldEndProjection[2];
        worldEndProjection[0] = vtkMath::Dot( worldEnd, uVec );
        worldEndProjection[1] = vtkMath::Dot( worldEnd, vVec );
        if( worldStartProjection[0] > worldEndProjection[0] ) {
            tmp = worldStartProjection[0]; 
            worldStartProjection[0] = worldEndProjection[0]; 
            worldEndProjection[0] = tmp; 
        }
        if( worldStartProjection[1] > worldEndProjection[1] ) {
            tmp = worldStartProjection[1]; 
            worldStartProjection[1] = worldEndProjection[1]; 
            worldEndProjection[1] = tmp; 
        }

        indexRangeX[0] = (int)( floor( ((worldStartProjection[0] - originProjection[0])/spacing[0] ) ));
        indexRangeX[1] = (int)( ceil( ((worldEndProjection[0] - originProjection[0] )/spacing[0] ) ));
        if( indexRangeX[0] < extent[0] ) {
            indexRangeX[0] = extent[0];
        }
        if( indexRangeX[0] >= extent[1] ) {
            indexRangeX[0] = -1;
        }
        if( indexRangeX[1] >= extent[1] ) {
            indexRangeX[1] = extent[1];
        }
        if( indexRangeX[1] <= extent[0] ) {
            indexRangeX[1] = -1;
        }

        indexRangeY[0] = (int)( floor( ((worldStartProjection[1] - originProjection[1])/spacing[1] ) ));
        indexRangeY[1] = (int)( ceil( ((worldEndProjection[1] - originProjection[1] )/spacing[1] ) ));

        if( indexRangeY[0] < extent[2] ) {
            indexRangeY[0] = extent[2];
        }
        if( indexRangeY[0] >= extent[3] ) {
            indexRangeY[0] = -1;
        }
        if( indexRangeY[1] >= extent[3] ) {
            indexRangeY[1] = extent[3];
        }
        if( indexRangeY[1] <= extent[2] ) {
            indexRangeY[1] = -1;
        }
        
        if( indexRangeX[0] >= 0 && indexRangeX[1] >= 0 && indexRangeY[0] >= 0 && indexRangeY[1] >= 0 ) {
            tlcBrc[0] = slice*extent[3] * extent[1] + indexRangeY[0]*extent[1] + indexRangeX[0]; 
            tlcBrc[1] = slice*extent[3] * extent[1] + (indexRangeY[1]-1)*extent[1] + (indexRangeX[1]-1); 
            GenerateClippingPlanes(); 
        } 


    } else if( dataVector[MRS] != NULL ) {

        //What should we do when the mri data is null, but the mrs is not....
    } 
    rwi->Render();
}


/*! 
 *  Used to select Actors based on their Top Left and Bottom Right Corners
 *
 *  \param tlcBrc the cell id's of the desired top left, bottom right corners
 */
void svkOverlayView::SelectActors( int* tlcBrc ) 
{
    if( tlcBrc != NULL && dataVector[MRS] != NULL) {
        this->tlcBrc[0] = tlcBrc[0];
        this->tlcBrc[1] = tlcBrc[1];
        this->GenerateClippingPlanes();
    }
}


/*! 
 *  Method highlights voxels within the selection box
 *
 *  \return tlcBrc the cell id's of the desired top left, bottom right corners
 */
int* svkOverlayView::HighlightSelectionVoxels()
{
    if( dataVector[MRS] != NULL ) { 
        double tolerance = 0.99;
        double* spacing = dataVector[MRS]->GetSpacing();
        double* corner;
        double projectedCorner[6];
        double thresholdBounds[6]; 

        double dcos[3][3];
        dataVector[MRS]->GetDcos( dcos );
        double xVec[3];
        xVec[0] = dcos[0][0];
        xVec[1] = dcos[1][0];
        xVec[2] = dcos[2][0];
        double yVec[3];
        yVec[0] = dcos[0][1];
        yVec[1] = dcos[1][1];
        yVec[2] = dcos[2][1];
        double zVec[3];
        zVec[0] = dcos[0][2];
        zVec[1] = dcos[1][2];
        zVec[2] = dcos[2][2];
        double uVec[3];
        uVec[0] = dcos[0][0];
        uVec[1] = dcos[0][1];
        uVec[2] = dcos[0][2];
        double vVec[3];
        vVec[0] = dcos[1][0];
        vVec[1] = dcos[1][1];
        vVec[2] = dcos[1][2];
        double wVec[3];
        wVec[0] = dcos[2][0];
        wVec[1] = dcos[2][1];
        wVec[2] = dcos[2][2];
        vtkPoints* cellBoxPoints = vtkPointSet::SafeDownCast( vtkActor::SafeDownCast( 
                                   this->GetProp( svkOverlayView::VOL_SELECTION )
                                   )->GetMapper()->GetInput())->GetPoints();

        projectedCorner[0] = VTK_DOUBLE_MAX; 
        projectedCorner[1] = -VTK_DOUBLE_MAX; 
        projectedCorner[2] = VTK_DOUBLE_MAX;
        projectedCorner[3] = -VTK_DOUBLE_MAX;
        projectedCorner[4] = VTK_DOUBLE_MAX;
        projectedCorner[5] = -VTK_DOUBLE_MAX;
        int minCornerIndex;
        int maxCornerIndex;

        for( int i = 0; i < cellBoxPoints->GetNumberOfPoints(); i++ ) {
            corner = cellBoxPoints->GetPoint(i); 
            if( i == 0 ) {
                projectedCorner[4] = vtkMath::Dot( corner, wVec);
                minCornerIndex = i;
                maxCornerIndex = i;
            }
            if( vtkMath::Dot( corner, uVec ) <= projectedCorner[0]   && 
                  vtkMath::Dot( corner, vVec ) <= projectedCorner[2] && 
                  (int)(vtkMath::Dot( corner, wVec )*10) == (int)(projectedCorner[4]*10) ) {

                projectedCorner[0] = vtkMath::Dot( corner, uVec);
                projectedCorner[2] = vtkMath::Dot( corner, vVec);
                minCornerIndex = i;
            } 
            if( vtkMath::Dot( corner, uVec ) >= projectedCorner[1] && 
                  vtkMath::Dot( corner, vVec ) >= projectedCorner[3] && 
                  (int)(vtkMath::Dot( corner, wVec )*10) == (int)(projectedCorner[4]*10) ) {

                projectedCorner[1] = vtkMath::Dot( corner, uVec);
                projectedCorner[3] = vtkMath::Dot( corner, vVec);
                maxCornerIndex = i;
            } 
        }
        thresholdBounds[0] =(cellBoxPoints->GetPoint(minCornerIndex))[0] 
                                             + vtkMath::Dot(spacing, xVec)*tolerance;
        thresholdBounds[1] =(cellBoxPoints->GetPoint(maxCornerIndex))[0] 
                                             - vtkMath::Dot(spacing, xVec)*tolerance;
        thresholdBounds[2] =(cellBoxPoints->GetPoint(minCornerIndex))[1] 
                                             + vtkMath::Dot(spacing, yVec)*tolerance;
        thresholdBounds[3] =(cellBoxPoints->GetPoint(maxCornerIndex))[1] 
                                             - vtkMath::Dot(spacing, yVec)*tolerance;
        thresholdBounds[4] =(cellBoxPoints->GetPoint(minCornerIndex))[2] 
                                             + vtkMath::Dot(spacing, zVec)*tolerance;
        thresholdBounds[5] =(cellBoxPoints->GetPoint(maxCornerIndex))[2] 
                                             - vtkMath::Dot(spacing, zVec)*tolerance;

        SetSelection( thresholdBounds, 1 );

        return tlcBrc;
    } else {
        return NULL; 
    }
}


/*!
 *  Sets the opacity of the image overlay.
 *
 *   \param opacity the new opacity you wish the image overlay to have. 
 */ 
void svkOverlayView::SetOverlayOpacity( double opacity ) 
{
    if( this->GetProp( svkOverlayView::OVERLAY_IMAGE ) != NULL ) {
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::OVERLAY_IMAGE ))->SetOpacity( opacity );    
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::OVERLAY_IMAGE_BACK ))->SetOpacity( opacity );    
    } 
}


/*!
 *  Sets the threshold of the image overlay.
 *
 *   \param threshold the new threshold you wish the image overlay to have. 
 */ 
void svkOverlayView::SetOverlayThreshold( double threshold ) 
{
    if( this->GetProp( svkOverlayView::OVERLAY_IMAGE ) != NULL ) {
        this->colorTransfer->SetAlphaThreshold(threshold); 
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::OVERLAY_IMAGE_BACK ))->Modified( );    
    } 
}


/*!
 * Generates the clipping planes for the mMMapper. This is how the boundries
 * set are enforced, after the data is scaled, it is clipped so that data
 * outside the plot range is simply not shown.
 */
void svkOverlayView::GenerateClippingPlanes()
{
    // We need to leave a little room around the edges, so the border does not get cut off
    if( dataVector[MRS] != NULL ) {
        vtkPlane* clipperPlane0 = vtkPlane::New();
        vtkPlane* clipperPlane1 = vtkPlane::New();
        vtkPlane* clipperPlane2 = vtkPlane::New();
        vtkPlane* clipperPlane3 = vtkPlane::New();
        vtkPlane* clipperPlane4 = vtkPlane::New();
        vtkPlane* clipperPlane5 = vtkPlane::New();

        double dcos[3][3];
        dataVector[MRS]->GetDcos( dcos );
        double* spacing = dataVector[MRS]->GetSpacing();
        double* origin = dataVector[MRS]->GetOrigin();
        int* extent = dataVector[MRS]->GetExtent();
        int uIndexRange[2];
        int vIndexRange[2];
        if( tlcBrc [0] >= 0 && tlcBrc[1] >= 0 ) {
            uIndexRange[0] = (tlcBrc[0] - slice*extent[1]*extent[3] ) % extent[1];
            uIndexRange[1] = (tlcBrc[1] - slice*extent[1]*extent[3] ) % extent[1];
            vIndexRange[0] = (tlcBrc[0] - slice*extent[1]*extent[3] ) / extent[1];
            vIndexRange[1] = (tlcBrc[1] - slice*extent[1]*extent[3] ) / extent[1];
        } else {
            uIndexRange[0] = extent[0];
            uIndexRange[1] = extent[1];
            vIndexRange[0] = extent[2];
            vIndexRange[1] = extent[3];
        }
        double uVec[3];
        uVec[0] = dcos[0][0];  
        uVec[1] = dcos[0][1];  
        uVec[2] = dcos[0][2];  
        double vVec[3];
        vVec[0] = dcos[1][0];  
        vVec[1] = dcos[1][1];  
        vVec[2] = dcos[1][2];  
        double wVec[3];
        wVec[0] = dcos[2][0];  
        wVec[1] = dcos[2][1];  
        wVec[2] = dcos[2][2];  
        double xVec[3];
        xVec[0] = dcos[0][0];  
        xVec[1] = dcos[1][0];  
        xVec[2] = dcos[2][0];  
        double yVec[3];
        yVec[0] = dcos[0][1];  
        yVec[1] = dcos[1][1];  
        yVec[2] = dcos[2][1];  
        double zVec[3];
        zVec[0] = dcos[0][2];  
        zVec[1] = dcos[1][2];  
        zVec[2] = dcos[2][2];  
        double deltaX = vtkMath::Dot( spacing, xVec );
        double deltaY = vtkMath::Dot( spacing, yVec );
        double deltaZ = vtkMath::Dot( spacing, zVec );

        clipperPlane0->SetNormal( uVec[0], uVec[1], uVec[2] );
        clipperPlane0->SetOrigin( origin[0] + deltaX*(uIndexRange[0] - CLIP_TOLERANCE),
                                  origin[1] + deltaY*(uIndexRange[0] - CLIP_TOLERANCE), 
                                  origin[2] + deltaZ*(uIndexRange[0] - CLIP_TOLERANCE) );

        clipperPlane1->SetNormal( -uVec[0], -uVec[1], -uVec[2] );
        clipperPlane1->SetOrigin( origin[0] + deltaX * (uIndexRange[1] + 1 + CLIP_TOLERANCE),
                                  origin[1] + deltaY * (uIndexRange[1] + 1 + CLIP_TOLERANCE), 
                                  origin[2] + deltaZ * (uIndexRange[1] + 1 + CLIP_TOLERANCE) );


        clipperPlane2->SetNormal( vVec[0], vVec[1], vVec[2] );
        clipperPlane2->SetOrigin( origin[0] + deltaX*(vIndexRange[0] - CLIP_TOLERANCE),
                                  origin[1] + deltaY*(vIndexRange[0] - CLIP_TOLERANCE), 
                                  origin[2] + deltaZ*(vIndexRange[0] - CLIP_TOLERANCE) );

        clipperPlane3->SetNormal( -vVec[0], -vVec[1], -vVec[2] );
        clipperPlane3->SetOrigin( origin[0] + deltaX*(vIndexRange[1] + 1 + CLIP_TOLERANCE),
                                  origin[1] + deltaY*(vIndexRange[1] + 1 + CLIP_TOLERANCE), 
                                  origin[2] + deltaZ*(vIndexRange[1] + 1 + CLIP_TOLERANCE) );

        clipperPlane4->SetNormal( wVec[0], wVec[1], wVec[2] );
        clipperPlane4->SetOrigin( origin[0] + deltaX * ( slice - CLIP_TOLERANCE), 
                                  origin[1] + deltaY * ( slice - CLIP_TOLERANCE), 
                                  origin[2] + deltaZ * ( slice - CLIP_TOLERANCE) );

        clipperPlane5->SetNormal( -wVec[0], -wVec[1], -wVec[2] );
        clipperPlane5->SetOrigin( origin[0] + deltaX * ( slice + 1 + CLIP_TOLERANCE ), 
                                  origin[1] + deltaY * ( slice + 1 + CLIP_TOLERANCE ), 
                                  origin[2] + deltaZ * ( slice + 1 + CLIP_TOLERANCE ) );

        vtkActor::SafeDownCast( this->GetProp( svkOverlayView::PLOT_GRID )
                                 )->GetMapper()->RemoveAllClippingPlanes();
        vtkActor::SafeDownCast( this->GetProp( svkOverlayView::PLOT_GRID )
                                 )->GetMapper()->AddClippingPlane( clipperPlane0 );
        vtkActor::SafeDownCast( this->GetProp( svkOverlayView::PLOT_GRID )
                                 )->GetMapper()->AddClippingPlane( clipperPlane1 );
        vtkActor::SafeDownCast( this->GetProp( svkOverlayView::PLOT_GRID )
                                 )->GetMapper()->AddClippingPlane( clipperPlane2 );
        vtkActor::SafeDownCast( this->GetProp( svkOverlayView::PLOT_GRID )
                                 )->GetMapper()->AddClippingPlane( clipperPlane3 );
        vtkActor::SafeDownCast( this->GetProp( svkOverlayView::PLOT_GRID )
                                 )->GetMapper()->AddClippingPlane( clipperPlane4 );
        vtkActor::SafeDownCast( this->GetProp( svkOverlayView::PLOT_GRID )
                                 )->GetMapper()->AddClippingPlane( clipperPlane5 );
        clipperPlane0->Delete();
        clipperPlane1->Delete();
        clipperPlane2->Delete();
        clipperPlane3->Delete();
        clipperPlane4->Delete();
        clipperPlane5->Delete();
    } else {
        cerr<<"INPUT HAS NOT BEEN SET!!"<<endl;
    }
}


/*!
 *  Method sets the slice of the overlay to the closests slice of the spectra.
 */
void svkOverlayView::SetSliceOverlay() {

    if( this->GetRenderer( svkOverlayView::PRIMARY )->HasViewProp(this->GetProp( svkOverlayView::OVERLAY_IMAGE ))) {
        int overlaySlice;
        vtkImageData* tmpData;
        double* overlayOrigin;
        double* overlaySpacing;
        double* specOrigin;
        double* specSpacing;
        double* imageOrigin;
        double* imageSpacing;
        int imageSlice;
        double sliceVoxelCenter;
        double dcos[3][3];
        int* overlayExtent = dataVector[OVERLAY]->GetExtent();

        // If the spectral data has been set, find the center of the slice, 
        // and then the closest image slice to it
        tmpData = dataVector[OVERLAY];
        overlayOrigin = tmpData->GetOrigin();
        overlaySpacing = tmpData->GetSpacing() ;
        tmpData = dataVector[MRS];
        specOrigin = tmpData->GetOrigin();
        specSpacing= tmpData->GetSpacing();
        tmpData = dataVector[MRI];
        imageOrigin = tmpData->GetOrigin();
        imageSpacing= tmpData->GetSpacing();
        imageSlice = imageViewer->GetSlice();
        dataVector[MRS]->GetDcos( dcos );

        double wVec[3];
        wVec[0] = dcos[2][0];  
        wVec[1] = dcos[2][1];  
        wVec[2] = dcos[2][2];  
        overlaySlice =(int) floor( (  vtkMath::Dot( specOrigin, wVec ) - 
                                 vtkMath::Dot( overlayOrigin, wVec ) + 
                                 specSpacing[2] * (slice + 0.5) )
                                /overlaySpacing[2] +0.5); 

        double distance;
        double distanceBack;
        // We need to guarantee that the overlay is between the image and the camera
        // This is why 0.01 is added, just some small fraction so they don't overlap. 

        distance  = (vtkMath::Dot(imageOrigin, wVec )+(imageSlice)*imageSpacing[2] - 
                             (vtkMath::Dot( overlayOrigin, wVec ) +
                             overlaySpacing[2] * overlaySlice)) + 0.01;

        distanceBack  = (vtkMath::Dot(imageOrigin, wVec )+(imageSlice)*imageSpacing[2] - 
                             (vtkMath::Dot( overlayOrigin, wVec ) +
                             overlaySpacing[2] * overlaySlice)) - 0.01;

        vtkTransform* transform = vtkTransform::New();
        vtkTransform* transformBack = vtkTransform::New();
        transform->Translate( distance*wVec[0], distance*wVec[1], distance*wVec[2] );
        transformBack->Translate( distanceBack*wVec[0], distanceBack*wVec[1], distanceBack*wVec[2] );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::OVERLAY_IMAGE )
                             )->SetUserTransform( transform );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::OVERLAY_IMAGE_BACK )
                             )->SetUserTransform( transformBack );
        transform->Delete();
        transformBack->Delete();
        if( overlaySlice >= overlayExtent[4] && overlaySlice <= overlayExtent[5]) {
            svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::OVERLAY_IMAGE )
                                   )->SetDisplayExtent( overlayExtent[0], overlayExtent[1], 
                                                        overlayExtent[2], overlayExtent[3], 
                                                        overlaySlice, overlaySlice );
            svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::OVERLAY_IMAGE_BACK )
                                   )->SetDisplayExtent( overlayExtent[0], overlayExtent[1], 
                                                        overlayExtent[2], overlayExtent[3], 
                                                        overlaySlice, overlaySlice );
        } else {
            this->TurnPropOff( svkOverlayView::OVERLAY_IMAGE );
            this->TurnPropOff( svkOverlayView::OVERLAY_IMAGE_BACK );
        }
    }  
}


/*!
 *  Sets up the overlay actor.
 */
void svkOverlayView::SetupOverlay()
{
    if( this->windowLeveler != NULL ) {
        this->windowLeveler->Delete();
        this->windowLeveler = NULL;     
    }
    if( this->colorTransfer != NULL ) {
        this->colorTransfer->Delete();
        this->colorTransfer = NULL;     
    }
    this->windowLeveler = svkImageMapToColors::New();
    int* extent = dataVector[OVERLAY]->GetExtent();
    double* overlayOrigin = dataVector[OVERLAY]->GetOrigin();

    // Need a modification if we need to render the overlay w/out an image
    double* imageOrigin = dataVector[MRI]->GetOrigin();
    double dcos[3][3];
    dataVector[OVERLAY]->GetDcos( dcos );
    double wVec[3];
    wVec[0] = dcos[2][0];  
    wVec[1] = dcos[2][1];  
    wVec[2] = dcos[2][2];  
    double wDistance = vtkMath::Dot( overlayOrigin, wVec ) - vtkMath::Dot( imageOrigin, wVec ); 

    this->SetLUT( svkLookupTable::COLOR); 
   
    this->windowLeveler->SetInput( dataVector[OVERLAY] );
    this->windowLeveler->SetOutputFormatToRGBA();

    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::OVERLAY_IMAGE )
                                   )->SetInput( this->windowLeveler->GetOutput() );

    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::OVERLAY_IMAGE_BACK )
                                   )->SetInput( this->windowLeveler->GetOutput() );

    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::OVERLAY_IMAGE )
                                   )->SetDisplayExtent( extent[0], extent[1], 
                                                        extent[2], extent[3], slice, slice );
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::OVERLAY_IMAGE_BACK )
                                   )->SetDisplayExtent( extent[0], extent[1], 
                                                        extent[2], extent[3], slice, slice );

    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( 
                                    svkOverlayView::OVERLAY_IMAGE ))->InterpolateOff();
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( 
                                    svkOverlayView::OVERLAY_IMAGE_BACK ))->InterpolateOff();

    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( 
                                    svkOverlayView::OVERLAY_IMAGE ))->SetOpacity(0.5);
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( 
                                    svkOverlayView::OVERLAY_IMAGE_BACK ))->SetOpacity(0.5);

    if( !this->GetRenderer( svkOverlayView::PRIMARY
                   )->HasViewProp( this->GetProp( svkOverlayView::OVERLAY_IMAGE ) ) ) {

        this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( 
                   this->GetProp( svkOverlayView::OVERLAY_IMAGE ) );
    }

    if( !this->GetRenderer( svkOverlayView::PRIMARY
                   )->HasViewProp( this->GetProp( svkOverlayView::OVERLAY_IMAGE_BACK ) ) ) {

        this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( 
                   this->GetProp( svkOverlayView::OVERLAY_IMAGE_BACK ) );
    }

    this->TurnPropOn( svkOverlayView::OVERLAY_IMAGE );
    this->TurnPropOn( svkOverlayView::OVERLAY_IMAGE_BACK );


    vtkScalarBarActor::SafeDownCast(this->GetProp( svkOverlayView::COLOR_BAR )
                                    )->SetTextPositionToPrecedeScalarBar();

    if( !this->GetRenderer( svkOverlayView::PRIMARY
                           )->HasViewProp( this->GetProp( svkOverlayView::COLOR_BAR) ) ) {

        this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( 
                                this->GetProp( svkOverlayView::COLOR_BAR) );
    }

    this->SetProp( svkOverlayView::COLOR_BAR, this->GetProp( svkOverlayView::COLOR_BAR) );
    this->TurnPropOn( svkOverlayView::COLOR_BAR);
    this->GetProp( svkOverlayView::COLOR_BAR)->Modified();
    this->GetRenderer( svkOverlayView::PRIMARY)->Render();

    // Now lets make sure we are looking at the appropriate slice.
    this->SetSliceOverlay();
    this->Refresh();
}


/*!
 *  Sets the type of interpolation for the overlayed image.
 *
 *  \param interpolationType options are NEAREST, LINEAR, or SINC
 */
void svkOverlayView::SetInterpolationType( int interpolationType )
{
    if (interpolationType == NEAREST ) {
        this->interpolationType = NEAREST; 
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::OVERLAY_IMAGE ))->InterpolateOff();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::OVERLAY_IMAGE_BACK ))->InterpolateOff();
    } else if (interpolationType == LINEAR) {
        this->interpolationType = LINEAR; 
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::OVERLAY_IMAGE ))->InterpolateOn();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::OVERLAY_IMAGE_BACK ))->InterpolateOn();
    } else if (interpolationType == SINC) {
        this->interpolationType = SINC; 
        cout << "SINC NOT SUPPORTED YET" << endl;
    }
    this->Refresh();
}


/*!
 *  Sets the LUT type 
 *
 *  \param LUT type 
 */
void svkOverlayView::SetLUT( svkLookupTable::svkLookupTableType type )
{

    //  Set default threshold
    double threshold;

    if (this->colorTransfer != NULL) {
        threshold = this->colorTransfer->GetAlphaThreshold(); 
        this->colorTransfer->Delete(); 
        this->colorTransfer = NULL; 
    } else {
        threshold = 0.;
    }

    this->colorTransfer = svkLookupTable::New();

    double *range = dataVector[OVERLAY]->GetScalarRange();
    double window = range[1] - range[0];
    double level = 0.1*(range[1] + range[0]);
    this->colorTransfer->SetRange( level - window/2.0, level + window/2.0);

    this->colorTransfer->SetLUTType( type ); 
    this->colorTransfer->SetAlphaThreshold( threshold ); 

    this->windowLeveler->SetLookupTable( this->colorTransfer );

    vtkScalarBarActor::SafeDownCast(this->GetProp( svkOverlayView::COLOR_BAR )
                                    )->SetLookupTable( this->colorTransfer );

    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::OVERLAY_IMAGE_BACK ))->Modified( );    

    this->Refresh();
}


/*!
 *  Check to make sure a given dataset is comptabile with the currently loaded data sets.
 *
 *  \param data the prospective data.
 *  \param targetIndex the index in which the data is trying to be placed.
 *
 *  \return resultInfo returns an empty string if the dataset is compatibly,
 *          otherwise an explaination of why the dataset is not compatible is returned.
 */
string svkOverlayView::GetDataCompatibility( svkImageData* data, int targetIndex )
{
    svkDataValidator* validator = svkDataValidator::New();
    string resultInfo = "";
    
    // Check for null datasets and out of bound data sets...
    if ( data == NULL || targetIndex > OVERLAY || targetIndex < 0 ) {
        resultInfo = "Data incompatible-- NULL or outside of input range.\n";

    } else if( data->IsA("svkMriImageData") ) {
        
        if( targetIndex == OVERLAY && dataVector[MRS] == NULL ) {
            resultInfo = "ERROR: Spectra must be loaded before overlays!\n";
        } 

        if( dataVector[MRS] != NULL ) {
            //cout << "OVERLAY VIEW VALIDATION 1: " << endl;
            svkDataValidator::ValidationErrorStatus status 
                = validator->AreDataIncompatible( data, dataVector[MRS] );
            if( status == svkDataValidator::INVALID_DATA_ORIENTATION ) { 
                resultInfo = ""; 
                this->ResliceImage(data, dataVector[MRS], targetIndex); 
            } else if (status) {
                resultInfo += validator->resultInfo.c_str();
                resultInfo += "\n";
            }
        } 

    } else if( data->IsA("svkMrsImageData") ) {
        if( dataVector[MRI] != NULL ) {
            //cout << "OVERLAY VIEW VALIDATION 2: " << endl;
            svkDataValidator::ValidationErrorStatus status 
                = validator->AreDataIncompatible( data, dataVector[MRI] ); 
            if( status  == svkDataValidator::INVALID_DATA_ORIENTATION ) {
                resultInfo = ""; 
                //resultInfo = "Orientation mismatch: reslicing images to MRS data orientation."; 
                this->ResliceImage(dataVector[MRI], data, targetIndex); 
            } else if (status) {
                resultInfo += validator->resultInfo.c_str();
                resultInfo += "\n";
            }
        } 
    } else {
        resultInfo = "ERROR: Unrecognized data type!";
    } 
    cout<<resultInfo.c_str()<<endl;
  
    validator->Delete(); 
    return resultInfo; 
}


/*!
 *  Reslice images to MRS orientation
 */
void svkOverlayView::ResliceImage(svkImageData* input, svkImageData* target, int targetIndex)    
{
    //  if (orthogonal orientations) {
    svkObliqueReslice* reslicer = svkObliqueReslice::New();
    reslicer->SetInput( input );
    reslicer->SetTargetDcosFromImage( target );
    reslicer->Update();
    this->SetInput( reslicer->GetOutput(), targetIndex );
    //}
}


//! Resets the window level, source taken from vtkImageViewer2
void svkOverlayView::ResetWindowLevel()
{
    if( dataVector[MRI] != NULL ) {
        this->imageViewer->GetInput()->UpdateInformation();
        this->imageViewer->GetInput()->SetUpdateExtent
           (this->imageViewer->GetInput()->GetWholeExtent());
        this->imageViewer->GetInput()->Update();
        double *range = this->imageViewer->GetInput()->GetScalarRange();
        this->imageViewer->SetColorWindow(range[1] - range[0]);
        this->imageViewer->SetColorLevel(0.5 * (range[1] + range[0]));
        this->imageViewer->GetImageActor()->Modified();
        this->imageViewer->Render();
   }
}


/*
 *  Turns the orthogonal images on.
 */
void svkOverlayView::TurnOrthogonalImagesOn()
{
    this->imageViewer->TurnOrthogonalImagesOn();
}


/*
 *  Turns the orthogonal images off.
 */
void svkOverlayView::TurnOrthogonalImagesOff()
{
    this->imageViewer->TurnOrthogonalImagesOff();
}

