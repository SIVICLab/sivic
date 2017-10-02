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


#include <svkOverlayView.h>


using namespace svk;


//vtkCxxRevisionMacro(svkOverlayView, "$Rev$");
vtkStandardNewMacro(svkOverlayView);


const double svkOverlayView::CLIP_TOLERANCE = 0.001; 


/*!
 *  Constructor creates the vtkImageView2, and sets its size.
 */
svkOverlayView::svkOverlayView()
{
    this->contourDirector = svkOverlayContourDirector::New();
    this->imageViewer = svkImageViewer2::New();
    this->satBandsAxial = svkSatBandSet::New();
    this->satBandsCoronal = svkSatBandSet::New();
    this->satBandsSagittal = svkSatBandSet::New();
    this->slice = 0;
    this->dataVector.push_back( NULL );
    this->dataVector.push_back( NULL );
    this->dataVector.push_back( NULL );
    this->rwi = NULL;
    this->myRenderWindow = NULL;
    this->tlcBrc[0] = -1;
    this->tlcBrc[1] = -1;

    this->windowLevelerAxial = NULL;
    this->windowLevelerCoronal = NULL;
    this->windowLevelerSagittal = NULL;
    this->colorTransfer = NULL ;
    this->imageInsideSpectra = false;
    this->overlayOpacity = 0.35;
    this->overlayThreshold = 0.0;

    // This will hold thie sinc interpolation of the overlay
    this->sincInterpolation = svkSincInterpolationFilter::New();
    this->interpOverlay = svkMriImageData::New();

    // Create a state vector for our images

    this->isPropOn.assign(LAST_PROP+1, false);
    this->isRendererOn.assign(LAST_RENDERER+1, false);
    this->isPropVisible.assign(LAST_PROP+1, false);     //Is the actor in the views FOV?
    
    // This fixes a compiler error in OS X, not sure why....
    vtkRenderer* nullRenderer = NULL;
    vtkProp* nullProp = NULL;
    this->renCollection.assign(svkOverlayView::LAST_RENDERER+1, nullRenderer);     //Is the actor in the views FOV?
    this->propCollection.assign(svkOverlayView::LAST_PROP+1, nullProp);     //Is the actor in the views FOV?

    vtkActor* entirePlotGrid = vtkActor::New();
    this->SetProp( svkOverlayView::PLOT_GRID, entirePlotGrid );
    entirePlotGrid->Delete();

    this->SetProp( svkOverlayView::VOL_SELECTION, NULL );

    svkOrientedImageActor* overlayActor = svkOrientedImageActor::New();
    this->SetProp( svkOverlayView::AXIAL_OVERLAY_FRONT, overlayActor );
    overlayActor->Delete();

    svkOrientedImageActor* overlayActorBack = svkOrientedImageActor::New();
    this->SetProp( svkOverlayView::AXIAL_OVERLAY_BACK, overlayActorBack );
    overlayActorBack->Delete();

    overlayActor = svkOrientedImageActor::New();
    this->SetProp( svkOverlayView::CORONAL_OVERLAY_FRONT, overlayActor );
    overlayActor->Delete();

    overlayActorBack = svkOrientedImageActor::New();
    this->SetProp( svkOverlayView::CORONAL_OVERLAY_BACK, overlayActorBack );
    overlayActorBack->Delete();

    overlayActor = svkOrientedImageActor::New();
    this->SetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT, overlayActor );
    overlayActor->Delete();

    overlayActorBack = svkOrientedImageActor::New();
    this->SetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK, overlayActorBack );
    overlayActorBack->Delete();

    vtkScalarBarActor* bar = vtkScalarBarActor::New();
    this->SetProp( svkOverlayView::COLOR_BAR, bar );
    bar->Delete();

    this->SetProp( svkOverlayView::SAT_BANDS_AXIAL, this->satBandsAxial->GetSatBandsActor() );
    this->TurnPropOff( svkOverlayView::SAT_BANDS_AXIAL );

    this->SetProp( svkOverlayView::SAT_BANDS_AXIAL_OUTLINE, this->satBandsAxial->GetSatBandsOutlineActor() );
    this->TurnPropOff( svkOverlayView::SAT_BANDS_AXIAL_OUTLINE );

    this->SetProp( svkOverlayView::SAT_BANDS_CORONAL_OUTLINE, this->satBandsCoronal->GetSatBandsOutlineActor() );
    this->TurnPropOff( svkOverlayView::SAT_BANDS_CORONAL_OUTLINE );

    this->SetProp( svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE, this->satBandsSagittal->GetSatBandsOutlineActor() );
    this->TurnPropOff( svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE );

    this->satBandsAxial->SetOrientation( svkDcmHeader::AXIAL );
    this->satBandsCoronal->SetOrientation( svkDcmHeader::CORONAL );
    this->satBandsSagittal->SetOrientation( svkDcmHeader::SAGITTAL );

    this->SetProp( svkOverlayView::SAT_BANDS_CORONAL, this->satBandsCoronal->GetSatBandsActor() );
    this->TurnPropOff( svkOverlayView::SAT_BANDS_CORONAL );

    this->SetProp( svkOverlayView::SAT_BANDS_SAGITTAL, this->satBandsSagittal->GetSatBandsActor() );
    this->TurnPropOff( svkOverlayView::SAT_BANDS_SAGITTAL );
    this->selBoxVisibility = VISIBLE_WHEN_CONTAINS_CURRENT_SLICE;
    
    this->interpolationType = NEAREST; 

    //  if true, then the actor's interpolation will be turned on in the view.  This is in 
    //  addition to the defined "interpolationType".
    this->interpolateView = true; 
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

    if( this->windowLevelerAxial != NULL ) {
        this->windowLevelerAxial->Delete();
        this->windowLevelerAxial = NULL;     
    }

    if( this->windowLevelerCoronal != NULL ) {
        this->windowLevelerCoronal->Delete();
        this->windowLevelerCoronal = NULL;     
    }

    if( this->windowLevelerSagittal != NULL ) {
        this->windowLevelerSagittal->Delete();
        this->windowLevelerSagittal = NULL;     
    }

    if( this->colorTransfer != NULL ) {
        this->colorTransfer->Delete();
        this->colorTransfer = NULL;     
    }
    if( this->satBandsAxial != NULL ) {
        this->satBandsAxial->Delete();
        this->satBandsAxial = NULL;     
    }
    if( this->satBandsCoronal != NULL ) {
        this->satBandsCoronal->Delete();
        this->satBandsCoronal = NULL;     
    }
    if( this->satBandsSagittal != NULL ) {
        this->satBandsSagittal->Delete();
        this->satBandsSagittal = NULL;     
    }
    if( this->sincInterpolation != NULL ) {
        this->sincInterpolation->Delete();
        this->sincInterpolation = NULL;     
    }
    if( this->interpOverlay != NULL ) {
        this->interpOverlay->Delete();
        this->interpOverlay = NULL;     
    }

}


/*!
 *   Sets the input of the vtkImageViewer2. It also resets the camera view and 
 *   the slice.
 *
 *   \param resetViewState boolean identifies of this is the first dataset input
 */        
void svkOverlayView::SetupMsInput( bool resetViewState ) 
{

    int toggleDraw = this->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->GetRenderer( svkOverlayView::PRIMARY)->DrawOff();
    }
    vtkPolyDataMapper* entireGridMapper = vtkPolyDataMapper::New();
    // This will be our grid
    vtkPolyData* grid = vtkPolyData::New();
    svk4DImageData::SafeDownCast( this->dataVector[MR4D] )->GetPolyDataGrid( grid );
    vtkCleanPolyData* cleaner = vtkCleanPolyData::New();
    cleaner->SetInputData( grid );
    grid->Delete();
    
    // Pipe the edges into the mapper 
    entireGridMapper->SetInputConnection( cleaner->GetOutputPort() );
    cleaner->Delete();
    vtkActor::SafeDownCast( this->GetProp( svkOverlayView::PLOT_GRID))->SetMapper( entireGridMapper );
    entireGridMapper->Delete();
    //vtkActor::SafeDownCast( GetProp( svkOverlayView::PLOT_GRID) )->GetProperty()->SetDiffuseColor( 0, 1, 0 );
    vtkActor::SafeDownCast( GetProp( svkOverlayView::PLOT_GRID) )->GetProperty()->SetDiffuseColor( 0, 0, 0 );

    if ( svkMrsImageData::SafeDownCast(this->dataVector[MR4D]) != NULL ) {

        // Now we need to grab the selection box
        svkMrsTopoGenerator* topoGenerator = svkMrsTopoGenerator::New();

        vtkActorCollection* selectionTopo = topoGenerator->GetTopoActorCollection( 
                dataVector[MR4D], svk4DImageData::VOL_SELECTION);
        topoGenerator->Delete();

       // Case for no selection box
        if( selectionTopo != NULL ) {
            selectionTopo->InitTraversal();
            if( this->GetRenderer( svkOverlayView::PRIMARY)->HasViewProp( this->GetProp( svkOverlayView::VOL_SELECTION) ) ) {
                this->GetRenderer( svkOverlayView::PRIMARY)->RemoveActor( this->GetProp( svkOverlayView::VOL_SELECTION) );
            }
            this->SetProp( svkOverlayView::VOL_SELECTION, selectionTopo->GetNextActor());     
            this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::VOL_SELECTION) );
            this->TurnPropOn( svkOverlayView::VOL_SELECTION );
            selectionTopo->Delete();
        }
    }
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::PLOT_GRID ) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_AXIAL) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_AXIAL_OUTLINE) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_CORONAL) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_CORONAL_OUTLINE) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_SAGITTAL) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE) );

   
    this->SetProp( svkOverlayView::PLOT_GRID, this->GetProp( svkOverlayView::PLOT_GRID ) );
    if( this->dataVector[MR4D]->GetNumberOfCells() == 1 && svkMrsImageData::SafeDownCast(this->dataVector[MR4D])->HasSelectionBox() ) {
        this->TurnPropOff( svkOverlayView::PLOT_GRID );
    } else {
        this->TurnPropOn( svkOverlayView::PLOT_GRID );
    }

    this->SetSlice( slice );
    if( resetViewState ) {
        //this->AlignCamera();
        this->HighlightSelectionVoxels();
    }

    if( this->dataVector[MR4D]->IsA("svkMrsImageData")) {
        this->satBandsAxial->SetInput( static_cast<svkMrsImageData*>(this->dataVector[MR4D]) );
        this->satBandsCoronal->SetInput( static_cast<svkMrsImageData*>(this->dataVector[MR4D]) );
        this->satBandsSagittal->SetInput( static_cast<svkMrsImageData*>(this->dataVector[MR4D]) );
    }
    if( this->dataVector[MRI] != NULL ) {
        this->SetSlice(this->imageViewer->GetSlice(svkDcmHeader::AXIAL),    svkDcmHeader::AXIAL);
        this->SetSlice(this->imageViewer->GetSlice(svkDcmHeader::SAGITTAL), svkDcmHeader::SAGITTAL);
        this->SetSlice(this->imageViewer->GetSlice(svkDcmHeader::CORONAL),  svkDcmHeader::CORONAL);
    }

    if( toggleDraw ) {
        this->GetRenderer( svkOverlayView::PRIMARY)->DrawOn();
    }

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
    
    int toggleDraw = this->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->GetRenderer(svkOverlayView::PRIMARY)->DrawOff();
    }

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
    imageViewer->SetInputData( dataVector[MRI] );
    this->contourDirector->SetReferenceImage(svkMriImageData::SafeDownCast(this->dataVector[MRI]));
    imageViewer->SetSlice( slice );
    
    if( resetViewState ) {
        imageViewer->SetupInteractor( rwi );
        imageViewer->SetRenderWindow( myRenderWindow ); 
        imageViewer->SetRenderer( this->GetRenderer( svkOverlayView::PRIMARY ) ); 
        imageViewer->GetRenderer()->SetBackground(0.0,0.0,0.0);
        this->AlignCamera( ); 
        imageViewer->GetRenderer()->GetActiveCamera()->SetParallelProjection(1);
        imageViewer->GetImageActor()->PickableOff();
        this->ResetWindowLevel();
    }
 
    int* extent = this->dataVector[MRI]->GetExtent();
    if( dataVector[MR4D] == NULL ) {
        this->SetSlice( (extent[5]-extent[4])/2 );
    } 
    //this->SetSlice((extent[2]-extent[3])/2,svkDcmHeader::CORONAL);
    //this->SetSlice((extent[1]-extent[0])/2,svkDcmHeader::SAGITTAL);

    this->satBandsAxial->SetReferenceImage( static_cast<svkMriImageData*>(this->dataVector[MRI]) );
    this->satBandsCoronal->SetReferenceImage( static_cast<svkMriImageData*>(this->dataVector[MRI]) );
    this->satBandsSagittal->SetReferenceImage( static_cast<svkMriImageData*>(this->dataVector[MRI]) );

    // And here we return the camera to its original state
    if( !resetViewState ) {
        this->GetRenderer( svkOverlayView::PRIMARY )->GetActiveCamera()->SetPosition( cameraPosition ); 
        this->GetRenderer( svkOverlayView::PRIMARY )->GetActiveCamera()->SetViewUp( cameraViewUp ); 
        this->GetRenderer( svkOverlayView::PRIMARY )->GetActiveCamera()->SetFocalPoint( cameraFocus ); 
    } else {
        this->AlignCamera();
    }
    
    
    rwi->Render();
    if( toggleDraw ) {
        this->GetRenderer(svkOverlayView::PRIMARY)->DrawOn();
    }

    // We need to reset the camera once Draw is on or the pipeline will not run

}


/*!
 *   Sets the data object in the object, also sets the input to the
 *   vtkImageViewer2 and renders it. Reslices any images necesary
 *
 *   \param data the data you want to view
 *   \param index the index of the data you want to set
 *
 */        
void svkOverlayView::SetInput(svkImageData* data, int index)
{

    // Check for out of bounds
    if( data != NULL && index >= 0 ) {
        // Check for null datasets and out of bound data sets...
        vector<svkImageData*> allImages( this->dataVector );
        allImages.push_back( data );
        svkImageData* resliceTargetData = NULL;
        //First we determine which image to to reslice to
        if( index == MR4D ) {
            resliceTargetData = data;
        } else {
            float minPixelVolume = VTK_FLOAT_MAX;
            for( int i = 0; i < allImages.size(); i++ ) {
                if( allImages[i] != NULL ) {
                    double* spacing = allImages[i]->GetSpacing();
                    float volumeAtIndex = spacing[0] * spacing[1] * spacing[2];
                    if( allImages[i]->IsA("svk4DImageData")) {
                        // Always pick 4D data as reference if possible
                        resliceTargetData = allImages[i];
                        break;
                    } else if( volumeAtIndex < minPixelVolume ) {
                        resliceTargetData = allImages[i];
                        minPixelVolume = volumeAtIndex;
                    }
                }
            }
        }
        this->ResliceImage(this->dataVector[MRI], resliceTargetData , MRI);
        this->ResliceImage(this->dataVector[OVERLAY], resliceTargetData , OVERLAY);
        for( int i = OVERLAY_CONTOUR; i < this->dataVector.size(); i++ ) {
            this->ResliceImage(this->dataVector[i], resliceTargetData , i);
        }
        bool wasDataResliced = false;
        if( index != MR4D ) {
            wasDataResliced = this->ResliceImage(data, resliceTargetData , index);
        }
        if( !wasDataResliced ) {
            this->SetInputPostReslice(data, index);
        }
    }
}


/*!
 *  Sets the input without reslicing.
 */
void svkOverlayView::SetInputPostReslice(svkImageData* data, int index)
{
    bool resetViewState = 1;
    string resultInfo = this->GetDataCompatibility( data, index );
    if( strcmp( resultInfo.c_str(), "" ) == 0 ) { 

        // Contours will always add new, will not replace existing
        if( index == OVERLAY_CONTOUR ) {
            index = dataVector.size();
            dataVector.push_back(NULL);
        }
        if( dataVector[index] != NULL ) {
            dataVector[index]->Delete();
            dataVector[index] = NULL;
        }
        data->Register( this );
        // We must register the dataset so it can be deleted
        ObserveData( data );
        dataVector[index] = data;
        if( index == OVERLAY ) {
            SetupOverlay();
        } else if( index >= OVERLAY_CONTOUR){
            SetupOverlayContour( index );
        } else if( data->IsA("svkMriImageData") ) {
            SetupMrInput( resetViewState );
        } else if( data->IsA("svk4DImageData") ) {
            SetupMsInput( resetViewState );
        } 
        this->Refresh();

    } else {

        string message = "ERROR: Dataset NOT loaded!!! \n";
        message += resultInfo;
        cout << message.c_str() << endl;

    }

}

/*!
 * Removes a data input and the associated actors. Currently only implemented for overlay.
 */
void svkOverlayView::RemoveInput(int index)
{
    if( index == OVERLAY ) {
        Superclass::RemoveInput(index);
        this->SetupOverlay();
    }
}


/*!
 *   Sets the current slice.
 *
 *   \param slice the slice you want to view
 */
void svkOverlayView::SetSlice(int slice)
{
    bool centerImage = true;
    this->SetSlice( slice, centerImage );
}


/*!
 *   Sets the current slice and centers the image to the voxel.
 *
 *   \param slice the slice you want to view
 */
void svkOverlayView::SetSlice(int slice, bool centerImage)
{
    if( dataVector[MRI] != NULL ) {
        if( dataVector[MR4D] != NULL ) {
    
            if( tlcBrc[0] >= 0 && tlcBrc[1] >= 0 ) {
                int* extent = dataVector[MR4D]->GetExtent();
                svkDcmHeader::Orientation dataOrientation = dataVector[MR4D]->GetDcmHeader()->GetOrientationType();
                int tlcIndex[3];
                int brcIndex[3];
                this->dataVector[MR4D]->GetIndexFromID( tlcBrc[0], tlcIndex );
                this->dataVector[MR4D]->GetIndexFromID( tlcBrc[1], brcIndex );
                int lastSlice  = dataVector[MR4D]->GetLastSlice( this->orientation );
                int firstSlice = dataVector[MR4D]->GetFirstSlice( this->orientation );
                slice = (slice > lastSlice) ? lastSlice:slice;
                slice = (slice < firstSlice) ? firstSlice:slice;
                tlcIndex[ this->dataVector[MR4D]->GetOrientationIndex( this->orientation ) ] = slice;
                brcIndex[ this->dataVector[MR4D]->GetOrientationIndex( this->orientation ) ] = slice;
                tlcBrc[0] = this->dataVector[MR4D]->GetIDFromIndex( tlcIndex[0], tlcIndex[1], tlcIndex[2] );
                tlcBrc[1] = this->dataVector[MR4D]->GetIDFromIndex( brcIndex[0], brcIndex[1], brcIndex[2] );
            }
            this->slice = slice;
            this->GenerateClippingPlanes();


            int toggleDraw = this->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
            if( toggleDraw ) {
                this->GetRenderer( svkOverlayView::PRIMARY)->DrawOff();
            }
            this->UpdateImageSlice( centerImage );
            this->SetSliceOverlay();
            this->UpdateSelectionBoxVisibility();

            if( toggleDraw ) {
                this->GetRenderer( svkOverlayView::PRIMARY)->DrawOn();
            }
            this->Refresh();
        } else {
            this->imageViewer->SetSlice( slice );    
            this->SetSliceOverlay();
        } 
    } else {
        this->slice = slice;
    }
}


/*
 * Check the state of the selection box visibility and update if necessary
 */
void svkOverlayView::UpdateSelectionBoxVisibility() {
    if( this->GetProp( svkOverlayView::VOL_SELECTION ) ) {
        bool isSliceInBox = static_cast<svkMrsImageData *>(dataVector[MR4D])->IsSliceInSelectionBox(slice, orientation);
        if ( this->selBoxVisibility == VISIBLE ||
            (isSliceInBox && this->selBoxVisibility == VISIBLE_WHEN_CONTAINS_CURRENT_SLICE)) {
            this->TurnPropOn(VOL_SELECTION);
            this->GetRenderer(PRIMARY)->ResetCameraClippingRange();
        } else {
            this->TurnPropOff(VOL_SELECTION);
        }
    }
}


/*
 *  Sets the slices for the images orthogonal to the primary.
 */
void svkOverlayView::SetSlice(int slice, svkDcmHeader::Orientation orientation)
{
    int toggleDraw = this->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->GetRenderer( svkOverlayView::PRIMARY)->DrawOff();
    }
    if( this->dataVector[MR4D] != NULL && orientation == this->orientation ) {

        // We may have removed the plot grid, so lets make sure its present
        if( !this->GetRenderer(svkOverlayView::PRIMARY)->HasViewProp( this->GetProp( svkOverlayView::PLOT_GRID ) )) {
            this->GetRenderer(svkOverlayView::PRIMARY)->AddViewProp(this->GetProp( svkOverlayView::PLOT_GRID ));
        }
        int newSpectraSlice = this->FindSpectraSlice( slice, orientation );
        this->UpdateSelectionBoxVisibility();
        if(  newSpectraSlice >= this->dataVector[MR4D]->GetFirstSlice( this->orientation ) &&
             newSpectraSlice <=  this->dataVector[MR4D]->GetLastSlice( this->orientation ) ) {

            if( newSpectraSlice != this->slice ) {
                this->SetSlice( newSpectraSlice );    
            }
            this->imageInsideSpectra = true;
        } else {
            if( this->GetRenderer(svkOverlayView::PRIMARY)->HasViewProp( this->GetProp( svkOverlayView::PLOT_GRID ) )) {
                this->GetRenderer(svkOverlayView::PRIMARY)->RemoveViewProp(this->GetProp( svkOverlayView::PLOT_GRID ));
            }
            this->imageInsideSpectra = false;
        }
    }
    this->imageViewer->SetSlice( slice, orientation );    
    this->SetSliceOverlay();

    if( dataVector[MR4D] != NULL ) {
        switch ( orientation ) {
            case svkDcmHeader::AXIAL:
                this->satBandsAxial->SetClipSlice( slice );
                break;
            case svkDcmHeader::CORONAL:
                this->satBandsCoronal->SetClipSlice( slice );
                break;
            case svkDcmHeader::SAGITTAL:
                this->satBandsSagittal->SetClipSlice(  slice );
                break;
        }
    }

    if( toggleDraw ) {
        this->GetRenderer( svkOverlayView::PRIMARY)->DrawOn();
    }
    this->Refresh();

}



/*!
 *  Sets the active volume. The active volume will be the rendered
 *  volume.
 *
 * @param volume
 */
void svkOverlayView::SetActiveImageVolume( int volume )
{
    this->imageViewer->SetActiveVolume( volume );
}


/*!
 * Sets the active overlay volume.
 *
 * @param volume
 */
void svkOverlayView::SetActiveOverlayVolume( int volume )
{
    if( this->dataVector[OVERLAY] != NULL && volume < this->dataVector[OVERLAY]->GetPointData()->GetNumberOfArrays()) {
        this->dataVector[OVERLAY]->GetPointData()->SetActiveScalars( this->dataVector[OVERLAY]->GetPointData()->GetArray(volume)->GetName() );
        this->dataVector[OVERLAY]->Modified();
        this->windowLevelerAxial->Modified();
        this->windowLevelerCoronal->Modified();
        this->windowLevelerSagittal->Modified();
        this->GetRenderer( svkOverlayView::PRIMARY)->Modified();
        this->Refresh( );
    }

}

/*!
 *  Finds the image slice that most closely corresponds to the input spectra slice.
 */
int svkOverlayView::FindCenterImageSlice( int spectraSlice, svkDcmHeader::Orientation orientation ) 
{
    int imageSlice;
    double spectraSliceCenter[3];
    this->dataVector[MR4D]->GetSliceCenter( spectraSlice, spectraSliceCenter, orientation );
    double normal[3];
    this->dataVector[MR4D]->GetSliceNormal( normal, orientation );
    int index = this->dataVector[MR4D]->GetOrientationIndex( orientation );
    double tolerance = this->dataVector[MR4D]->GetSpacing()[index]/2.0;

    imageSlice = this->dataVector[MRI]->GetClosestSlice( spectraSliceCenter, orientation, tolerance );
    return imageSlice;
}


/*
 *  Finds the spectra slice that most closely corresponds to the input image slice.
 */
int svkOverlayView::FindSpectraSlice( int imageSlice, svkDcmHeader::Orientation orientation ) 
{
    int spectraSlice;
    double imageSliceCenter[3];
    this->dataVector[MRI]->GetSliceOrigin( imageSlice, imageSliceCenter, orientation );
    int index = this->dataVector[MR4D]->GetOrientationIndex( orientation );
	double spacing[3] = {0,0,0};
    this->dataVector[MR4D]->GetSpacing(spacing);
    if( svkMrsImageData::SafeDownCast( this->dataVector[MR4D]) ) {
    	if( this->dataVector[MR4D]->GetNumberOfCells() == 1 && svkMrsImageData::SafeDownCast(this->dataVector[MR4D])->HasSelectionBox() ) {
    		svkMrsImageData::SafeDownCast(this->dataVector[MR4D])->GetSelectionBoxSpacing( spacing );
    	}
    }
    double tolerance = spacing[index]/2.0;
    spectraSlice = this->dataVector[MR4D]->GetClosestSlice( imageSliceCenter, orientation, tolerance );
    return spectraSlice;
}


/*
 *  Finds the spectra slice that most closely corresponds to the input image slice.
 */
int svkOverlayView::FindOverlaySlice( int imageSlice, svkDcmHeader::Orientation orientation )
{
    int overlaySlice = -1;
    double sliceCenter[3];
    double tolerance = 0;
    this->dataVector[MRI]->GetSliceOrigin( imageSlice, sliceCenter, orientation );
    if( this->dataVector[MR4D] != NULL ) {
        int index = this->dataVector[MR4D]->GetOrientationIndex( orientation );
        tolerance = this->dataVector[MR4D]->GetSpacing()[index]/2.0;
    } else {
        int index = this->dataVector[MRI]->GetOrientationIndex( orientation );
        tolerance = this->dataVector[OVERLAY]->GetSpacing()[index]/2.0;
    }
    overlaySlice = this->dataVector[OVERLAY]->GetClosestSlice( sliceCenter, orientation, tolerance );
    return overlaySlice;
}


/*!
 *  Sets the slice of the anatomical data, based on the spectroscopic slice.
 *  It calculates the anatomical slice closest to the center of the spectroscopic
 *  slice.
 *
 */
void svkOverlayView::UpdateImageSlice( bool centerImage )
{
    int imageSlice = this->imageViewer->GetSlice( this->orientation );
    if( centerImage ) {
        imageSlice = FindCenterImageSlice(this->slice, this->orientation);
    }
    int* imageExtent = dataVector[MRI]->GetExtent();
    int satBandClipSlice = this->slice;
    if ( imageSlice >  this->dataVector[MRI]->GetLastSlice( this->orientation )) {
        this->imageViewer->GetImageActor( this->orientation )->SetVisibility(0);
        this->imageViewer->SetSlice( this->dataVector[MRI]->GetLastSlice(), this->orientation );
        this->imageInsideSpectra = false;
    } else if ( imageSlice <  this->dataVector[MRI]->GetFirstSlice( this->orientation )) {
        this->imageViewer->GetImageActor( this->orientation )->SetVisibility(0);
        this->imageViewer->SetSlice( this->dataVector[MRI]->GetFirstSlice(), this->orientation );
        this->imageInsideSpectra = false;
    } else {
        this->imageViewer->GetImageActor( this->orientation )->SetVisibility(1);
        this->imageViewer->SetSlice( imageSlice, this->orientation );
        this->imageInsideSpectra = true;
        satBandClipSlice = imageSlice;
    }
    switch ( this->orientation ) {
        case svkDcmHeader::AXIAL:
            this->satBandsAxial->SetClipToReferenceImage( this->imageInsideSpectra );
            this->satBandsAxial->SetClipSlice( satBandClipSlice );
            break;
        case svkDcmHeader::CORONAL:
            this->satBandsCoronal->SetClipToReferenceImage( this->imageInsideSpectra );
            this->satBandsCoronal->SetClipSlice( satBandClipSlice );
            break;
        case svkDcmHeader::SAGITTAL:
            this->satBandsSagittal->SetClipToReferenceImage( this->imageInsideSpectra );
            this->satBandsSagittal->SetClipSlice( satBandClipSlice );
            break;
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
 *  Sets desired the current selection in Display (pixels) coordinates
 *  and highlights the intersected voxels.
 *
 *  \param selectionArea the area you wish to select voxels within [xmin, xmax, ymin, ymax]
 */ 
void svkOverlayView::SetSelection( double* selectionArea, bool isWorldCords )
{
    if( selectionArea != NULL && dataVector[MR4D] != NULL) {
        double worldStart[3]; 
        double worldEnd[3]; 
        bool tagVoxel = false;
        if( !isWorldCords ) {
        	if( svkVoxelTaggingUtils::IsImageVoxelTagData( this->dataVector[OVERLAY])
        	  && fabs(selectionArea[0] - selectionArea[2]) < 5
        	  &&  fabs(selectionArea[1] - selectionArea[3]) < 5 ) {
        		tagVoxel = true;
        	}
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
        double selection[6];

        selection[0] = worldStart[0];
        selection[1] = worldEnd[0];
        selection[2] = worldStart[1];
        selection[3] = worldEnd[1];
        selection[4] = worldStart[2];
        selection[5] = worldEnd[2];

        int tlcBrcImageData[2];
        svk4DImageData::SafeDownCast(this->dataVector[MR4D])->GetTlcBrcInUserSelection( tlcBrcImageData, selection, this->orientation, this->slice );
        if( tagVoxel && tlcBrcImageData[0] == tlcBrcImageData[1] ) {
        	svkVoxelTaggingUtils::ToggleVoxelTag(this->dataVector[OVERLAY], tlcBrcImageData[0]);
        } else {
			this->SetTlcBrc( tlcBrcImageData );
        }
    } else if( dataVector[MR4D] != NULL ) {

        //What should we do when the mri data is null, but the mrs is not....
    } 
    rwi->Render();
}


/*! 
 *  Used to select Actors based on their Top Left and Bottom Right Corners
 *
 *  \param tlcBrc the cell id's of the desired top left, bottom right corners
 */
void svkOverlayView::SetTlcBrc( int* tlcBrc ) 
{
    if( svkDataView::IsTlcBrcWithinData( this->dataVector[MR4D], tlcBrc ) ) {
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
    if( dataVector[MR4D] != NULL && this->dataVector[MR4D]->IsA("svkMrsImageData") ) {
        int tlcBrcImageData[2];
        svkMrsImageData::SafeDownCast(this->dataVector[MR4D])->Get2DProjectedTlcBrcInSelectionBox( tlcBrcImageData, this->orientation, this->slice );
        this->SetTlcBrc( tlcBrcImageData );
        return tlcBrc;
    } else if(this->dataVector[MR4D] != NULL) {
        // If there is no selection, select everything in the slice
        double selection[6];
        int tlcBrcImageData[2];
        selection[0] = VTK_INT_MIN/2;
        selection[1] = VTK_INT_MAX/2;
        selection[2] = VTK_INT_MIN/2;
        selection[3] = VTK_INT_MAX/2;
        selection[4] = VTK_INT_MIN/2;
        selection[5] = VTK_INT_MAX/2;
        svk4DImageData::SafeDownCast(this->dataVector[MR4D])->GetTlcBrcInUserSelection( tlcBrcImageData, selection, this->orientation, this->slice );
        this->SetTlcBrc( tlcBrcImageData );
        return this->tlcBrc;
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
    this->overlayOpacity = opacity;
    svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT ))->SetOpacity(opacity);
    svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK ))->SetOpacity(opacity);
    svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT ))->SetOpacity(opacity) ;
    svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK ))->SetOpacity(opacity) ;
    svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT ))->SetOpacity(opacity) ;
    svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK ))->SetOpacity(opacity) ;
}


/*!
 *  Gets the opacity of the image overlay.
 *
 */ 
double svkOverlayView::GetOverlayOpacity( ) 
{
    return this->overlayOpacity;
}


/*!
 *  Sets the threshold of the image overlay.
 *
 *   \param threshold the new threshold you wish the image overlay to have. 
 */ 
void svkOverlayView::SetOverlayThreshold( double threshold ) 
{
    this->overlayThreshold = threshold;
    if( this->colorTransfer != NULL ) {
        this->colorTransfer->SetAlphaThreshold(threshold);
        this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT )->Modified();
    } 
}


/*!
 *  Gets the opacity of the image overlay.
 *
 */ 
double svkOverlayView::GetOverlayThreshold( ) 
{
    return this->overlayThreshold;
}


//!
void svkOverlayView::SetLevel(double level){
    if( this->imageViewer != NULL ) {
        this->imageViewer->SetColorLevel( level );
        this->imageViewer->GetImageActor()->Modified();
        this->imageViewer->Render();
    }
}


//!
void svkOverlayView::SetWindow(double window){
    if( this->imageViewer != NULL ) {
        this->imageViewer->SetColorWindow( window );
        this->imageViewer->GetImageActor()->Modified();
        this->imageViewer->Render();
    }
}


//!
double svkOverlayView::GetLevel( )
{
    return this->imageViewer->GetColorLevel( );
}


//!
double svkOverlayView::GetWindow( )
{
    return this->imageViewer->GetColorWindow( );
}


//!
void svkOverlayView::SetColorOverlayLevel( double level )
{
    if( this->colorTransfer != NULL ) {
        double* range = this->colorTransfer->GetRange();
        double window = range[1] - range [0];
        this->colorTransfer->SetRange( level - window/2.0, level + window/2.0 );
        this->Refresh();
    }
}


//!
void svkOverlayView::SetColorOverlayWindow( double window )
{
    if( this->colorTransfer != NULL ) {
        double* range = this->colorTransfer->GetRange();
        double oldWindow = range[1] - range [0];
        double level = oldWindow/2.0;
        this->colorTransfer->SetRange( level - window/2.0, level + window/2.0 );
        this->Refresh();
    }
}


//!
double svkOverlayView::GetColorOverlayLevel()
{
    double level = 0;
    if( this->colorTransfer != NULL ) {
        double* range =  this->colorTransfer->GetRange( );
        level = range[0] + (range[1]-range[0])/2.0;
    } 
    return level;
}


//!
double svkOverlayView::GetColorOverlayWindow()
{
    double window = 0;
    if( this->colorTransfer != NULL ) {
        double* range =  this->colorTransfer->GetRange( );
        window = range[1]-range[0];
    }
    return window;
}

/*!
 * Generates the clipping planes for the mMMapper. This is how the boundries
 * set are enforced, after the data is scaled, it is clipped so that data
 * outside the plot range is simply not shown.
 */
void svkOverlayView::GenerateClippingPlanes( )
{
    // We need to leave a little room around the edges, so the border does not get cut off
    if( dataVector[MR4D] != NULL ) {
        this->ClipMapperToTlcBrc( dataVector[MR4D],
                                 vtkActor::SafeDownCast( this->GetProp( svkOverlayView::PLOT_GRID ))->GetMapper(), 
                                 tlcBrc, svkOverlayView::CLIP_TOLERANCE, 
                                 svkOverlayView::CLIP_TOLERANCE, svkOverlayView::CLIP_TOLERANCE );
    }
}


/*!
 *  Method sets the slice of the overlay to the closests slice of the spectra.
 */
void svkOverlayView::SetSliceOverlay() {

    if( this->dataVector[OVERLAY] != NULL && dataVector[MRI] != NULL) {
        int overlaySliceAxial;
        int overlaySliceCoronal;
        int overlaySliceSagittal;
        double axialDelta;
        double coronalDelta;
        double sagittalDelta;

        vtkImageData* tmpData;
        double* overlayOrigin;
        double* overlaySpacing;
        double* imageOrigin;
        double* imageSpacing;
        int imageSlice;
        double sliceVoxelCenter;
        int* overlayExtent = dataVector[OVERLAY]->GetExtent();
        vtkTransform* transformFrontAxial = vtkTransform::New();
        vtkTransform* transformBackAxial = vtkTransform::New();
        vtkTransform* transformFrontCoronal = vtkTransform::New();
        vtkTransform* transformBackCoronal = vtkTransform::New();
        vtkTransform* transformFrontSagittal = vtkTransform::New();
        vtkTransform* transformBackSagittal = vtkTransform::New();

        // If the spectral data has been set, find the center of the slice, 
        // and then the closest image slice to it
        tmpData = dataVector[OVERLAY];
        overlayOrigin = tmpData->GetOrigin();
        overlaySpacing = tmpData->GetSpacing() ;
        tmpData = dataVector[MRI];
        imageOrigin = tmpData->GetOrigin();
        imageSpacing= tmpData->GetSpacing();
        imageSlice = imageViewer->GetSlice();
        int* imageExtent = this->dataVector[MRI]->GetExtent();

        double normal[3];
        this->dataVector[MRI]->GetSliceNormal( normal, svkDcmHeader::AXIAL );
        double axialNormal[3] = { normal[0], normal[1], normal[2] };
        this->dataVector[MRI]->GetSliceNormal( normal, svkDcmHeader::CORONAL );
        double coronalNormal[3] = { normal[0], normal[1], normal[2] };
        this->dataVector[MRI]->GetSliceNormal( normal, svkDcmHeader::SAGITTAL );
        double sagittalNormal[3] = { normal[0], normal[1], normal[2] };

        int axialRange[2] = {this->dataVector[OVERLAY]->GetFirstSlice( svkDcmHeader::AXIAL ),
                             this->dataVector[OVERLAY]->GetLastSlice( svkDcmHeader::AXIAL ) };
        int coronalRange[2] = {this->dataVector[OVERLAY]->GetFirstSlice( svkDcmHeader::CORONAL ),
                             this->dataVector[OVERLAY]->GetLastSlice( svkDcmHeader::CORONAL ) };
        int sagittalRange[2] = {this->dataVector[OVERLAY]->GetFirstSlice( svkDcmHeader::SAGITTAL ),
                             this->dataVector[OVERLAY]->GetLastSlice( svkDcmHeader::SAGITTAL ) };
        overlaySliceAxial =   this->FindOverlaySlice( imageViewer->axialSlice, svkDcmHeader::AXIAL); 
        overlaySliceCoronal = this->FindOverlaySlice( imageViewer->coronalSlice, svkDcmHeader::CORONAL); 
        overlaySliceSagittal =this->FindOverlaySlice( imageViewer->sagittalSlice, svkDcmHeader::SAGITTAL); 

        if( this->IsPropOn( svkOverlayView::AXIAL_OVERLAY_FRONT ) ) {
            if( overlaySliceAxial > axialRange[1] ) {
                overlaySliceAxial = axialRange[1];
                this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT )->VisibilityOff();
                this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK )->VisibilityOff();
            } else if ( overlaySliceAxial < axialRange[0] ) {
                overlaySliceAxial = axialRange[0];
                this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT )->VisibilityOff();
                this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK )->VisibilityOff();
            } else {
                this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT )->VisibilityOn();
                this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK )->VisibilityOn();
            }
        }

        if( this->IsPropOn( svkOverlayView::CORONAL_OVERLAY_FRONT ) ) {
            if( overlaySliceCoronal > coronalRange[1] ) {
                overlaySliceCoronal = coronalRange[1];
                this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT )->VisibilityOff();
                this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK )->VisibilityOff();
            } else if ( overlaySliceCoronal < coronalRange[0] ) {
                overlaySliceCoronal = coronalRange[0];
                this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT )->VisibilityOff();
                this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK )->VisibilityOff();
            } else {
                this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT )->VisibilityOn();
                this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK )->VisibilityOn();
            }
        }

        if( this->IsPropOn( svkOverlayView::SAGITTAL_OVERLAY_FRONT ) ) {
            if( overlaySliceSagittal > sagittalRange[1] ) {
                overlaySliceSagittal = sagittalRange[1];
                this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT )->VisibilityOff();
                this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK )->VisibilityOff();
            } else if ( overlaySliceSagittal < sagittalRange[0] ) {
                overlaySliceSagittal = sagittalRange[0];
                this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT )->VisibilityOff();
                this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK )->VisibilityOff();
            } else {
                this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT )->VisibilityOn();
                this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK )->VisibilityOn();
            }
        }

        int axialIndex = this->dataVector[MRI]->GetOrientationIndex( svkDcmHeader::AXIAL);
        int coronalIndex = this->dataVector[MRI]->GetOrientationIndex( svkDcmHeader::CORONAL);
        int sagittalIndex = this->dataVector[MRI]->GetOrientationIndex( svkDcmHeader::SAGITTAL);

        axialDelta  = (vtkMath::Dot(imageOrigin, axialNormal )+(imageViewer->axialSlice)*imageSpacing[axialIndex] - 
                                 (vtkMath::Dot( overlayOrigin, axialNormal ) +
                                 overlaySpacing[axialIndex] * overlaySliceAxial));

        coronalDelta  = (vtkMath::Dot(imageOrigin, coronalNormal )+(imageViewer->coronalSlice)*imageSpacing[coronalIndex] - 
                                 (vtkMath::Dot( overlayOrigin, coronalNormal ) +
                                 overlaySpacing[coronalIndex] * overlaySliceCoronal));
        sagittalDelta  = (vtkMath::Dot(imageOrigin, sagittalNormal )+(imageViewer->sagittalSlice)*imageSpacing[sagittalIndex] -                                 (vtkMath::Dot( overlayOrigin, sagittalNormal ) +
                                 overlaySpacing[sagittalIndex] * overlaySliceSagittal));
        // We need to guarantee that the overlay is between the image and the camera
        // This is why 0.05 is added, just some small fraction so they don't overlap. 
        transformFrontAxial->Translate(
                                   (axialDelta+0.05)*axialNormal[0], 
                                   (axialDelta+0.05)*axialNormal[1], 
                                   (axialDelta+0.05)*axialNormal[2]);
        transformBackAxial->Translate(
                                   (axialDelta-0.05)*axialNormal[0], 
                                   (axialDelta-0.05)*axialNormal[1], 
                                   (axialDelta-0.05)*axialNormal[2]);

        transformFrontCoronal->Translate(
                                   (coronalDelta+0.05)*coronalNormal[0], 
                                   (coronalDelta+0.05)*coronalNormal[1], 
                                   (coronalDelta+0.05)*coronalNormal[2]);
        transformBackCoronal->Translate(
                                   (coronalDelta-0.05)*coronalNormal[0], 
                                   (coronalDelta-0.05)*coronalNormal[1], 
                                   (coronalDelta-0.05)*coronalNormal[2]);
        transformFrontSagittal->Translate(
                                   (sagittalDelta+0.05)*sagittalNormal[0], 
                                   (sagittalDelta+0.05)*sagittalNormal[1], 
                                   (sagittalDelta+0.05)*sagittalNormal[2]);
        transformBackSagittal->Translate(
                                   (sagittalDelta-0.05)*sagittalNormal[0], 
                                   (sagittalDelta-0.05)*sagittalNormal[1], 
                                   (sagittalDelta-0.05)*sagittalNormal[2]);

        int axialExtent[6] = { overlayExtent[ 0 ], overlayExtent[ 1 ],
                               overlayExtent[ 2 ], overlayExtent[ 3 ],
                               overlayExtent[ 4 ], overlayExtent[ 5 ] };
        axialExtent[ axialIndex *2 ] = overlaySliceAxial; 
        axialExtent[ axialIndex *2 + 1 ] = overlaySliceAxial; 
        int coronalExtent[6] = { overlayExtent[ 0 ], overlayExtent[ 1 ],
                                 overlayExtent[ 2 ], overlayExtent[ 3 ],
                                 overlayExtent[ 4 ], overlayExtent[ 5 ] };
        coronalExtent[ coronalIndex *2 ] = overlaySliceCoronal; 
        coronalExtent[ coronalIndex *2 + 1 ] = overlaySliceCoronal; 

        int sagittalExtent[6] = { overlayExtent[ 0 ], overlayExtent[ 1 ],
                                 overlayExtent[ 2 ], overlayExtent[ 3 ],
                                 overlayExtent[ 4 ], overlayExtent[ 5 ] };
        sagittalExtent[ sagittalIndex *2 ] = overlaySliceSagittal; 
        sagittalExtent[ sagittalIndex *2 + 1 ] = overlaySliceSagittal; 

        svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT )
                               )->SetDisplayExtent( axialExtent );
        svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK )
                               )->SetDisplayExtent( axialExtent );
        svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT )
                               )->SetDisplayExtent( coronalExtent );
        svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK )
                               )->SetDisplayExtent( coronalExtent );
        svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT )
                               )->SetDisplayExtent( sagittalExtent );
        svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK )
                               )->SetDisplayExtent( sagittalExtent );

        svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT )
                             )->SetUserTransform(transformFrontAxial );
        svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK )
                             )->SetUserTransform( transformBackAxial );
        svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT )
                             )->SetUserTransform( transformFrontCoronal );
        svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK )
                             )->SetUserTransform( transformBackCoronal );
        svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT )
                             )->SetUserTransform( transformFrontSagittal );
        svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK )
                             )->SetUserTransform( transformBackSagittal );

        transformFrontAxial->Delete();
        transformBackAxial->Delete();
        transformFrontCoronal->Delete();
        transformBackCoronal->Delete();
        transformFrontSagittal->Delete();
        transformBackSagittal->Delete();



    }
    this->contourDirector->SetSlice(this->imageViewer->GetSlice(), this->GetOrientation());
}


/*!
 *  Sets up the overlay actor.
 */
void svkOverlayView::SetupOverlay()
{
	svkLookupTable::svkLookupTableType lutType = svkLookupTable::COLOR;
    if( this->windowLevelerAxial != NULL ) {
        this->windowLevelerAxial->Delete();
        this->windowLevelerAxial = NULL;     
    }
    if( this->windowLevelerCoronal != NULL ) {
        this->windowLevelerCoronal->Delete();
        this->windowLevelerCoronal = NULL;     
    }
    if( this->windowLevelerSagittal != NULL ) {
        this->windowLevelerSagittal->Delete();
        this->windowLevelerSagittal = NULL;     
    }
    if( this->colorTransfer != NULL ) {
    	if( this->colorTransfer->GetLUTType() != svkLookupTable::NONE ) {
    		lutType = this->colorTransfer->GetLUTType();
    	}
        this->colorTransfer->Delete();
        this->colorTransfer = NULL;     
    }

    // We need to kill the old actors, window levelers, and the color transfer otherwise update cause problems
    // (seg faults in vtkLookupTable) when switching from a larger dataset to a smaller dataset
    this->GetRenderer( svkOverlayView::PRIMARY )->RemoveViewProp( this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT ) );
    svkOrientedImageActor* overlayActor = svkOrientedImageActor::New();
    this->SetProp( svkOverlayView::AXIAL_OVERLAY_FRONT, overlayActor );
    overlayActor->Delete();

    this->GetRenderer( svkOverlayView::PRIMARY )->RemoveViewProp( this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK ) );
    svkOrientedImageActor* overlayActorBack = svkOrientedImageActor::New();
    this->SetProp( svkOverlayView::AXIAL_OVERLAY_BACK, overlayActorBack );
    overlayActorBack->Delete();

    this->GetRenderer( svkOverlayView::PRIMARY )->RemoveViewProp( this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT ) );
    overlayActor = svkOrientedImageActor::New();
    this->SetProp( svkOverlayView::CORONAL_OVERLAY_FRONT, overlayActor );
    overlayActor->Delete();

    this->GetRenderer( svkOverlayView::PRIMARY )->RemoveViewProp( this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK ) );
    overlayActorBack = svkOrientedImageActor::New();
    this->SetProp( svkOverlayView::CORONAL_OVERLAY_BACK, overlayActorBack );
    overlayActorBack->Delete();

    this->GetRenderer( svkOverlayView::PRIMARY )->RemoveViewProp( this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT ) );
    overlayActor = svkOrientedImageActor::New();
    this->SetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT, overlayActor );
    overlayActor->Delete();

    this->GetRenderer( svkOverlayView::PRIMARY )->RemoveViewProp( this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK ) );
    overlayActorBack = svkOrientedImageActor::New();
    this->SetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK, overlayActorBack );
    overlayActorBack->Delete();
    
    if( this->windowLevelerAxial != NULL ) {
        this->windowLevelerAxial->Delete();
        this->windowLevelerAxial= NULL;
    }
    if( this->windowLevelerCoronal != NULL ) {
        this->windowLevelerCoronal->Delete();
        this->windowLevelerCoronal= NULL;
    }
    if( this->windowLevelerSagittal != NULL ) {
        this->windowLevelerSagittal->Delete();
        this->windowLevelerSagittal= NULL;
    }
    if( this->GetProp( svkOverlayView::COLOR_BAR ) != NULL ) {
        this->GetRenderer( svkOverlayView::PRIMARY )->RemoveViewProp( this->GetProp( svkOverlayView::COLOR_BAR ) );
        this->TurnPropOff( svkOverlayView::COLOR_BAR );
        vtkScalarBarActor* bar = vtkScalarBarActor::New();
        this->SetProp( svkOverlayView::COLOR_BAR, bar );
        bar->Delete();
    }

    // IF the data is NULL we can now return
    if( this->dataVector[OVERLAY] == NULL ) {
        return; 
    }
    
    this->windowLevelerAxial = svkImageMapToColors::New();
    this->windowLevelerCoronal = svkImageMapToColors::New();
    this->windowLevelerSagittal = svkImageMapToColors::New();
    int* extent = dataVector[OVERLAY]->GetExtent();
    double* overlayOrigin = dataVector[OVERLAY]->GetOrigin();

    // Need a modification if we need to render the overlay w/out an image
    double* imageOrigin = dataVector[MRI]->GetOrigin();
    double sliceNormal[3];
    dataVector[OVERLAY]->GetDataBasis( sliceNormal, svkImageData::SLICE);
    double wDistance = vtkMath::Dot( overlayOrigin, sliceNormal ) - vtkMath::Dot( imageOrigin, sliceNormal ); 

    this->SetLUT( lutType );
   
    this->windowLevelerAxial->SetInputData( dataVector[OVERLAY] );
    this->windowLevelerAxial->SetOutputFormatToRGBA();
    this->windowLevelerAxial->Update();
    this->windowLevelerCoronal->SetInputData( dataVector[OVERLAY] );
    this->windowLevelerCoronal->SetOutputFormatToRGBA();
    this->windowLevelerCoronal->Update();
    this->windowLevelerSagittal->SetInputData( dataVector[OVERLAY] );
    this->windowLevelerSagittal->SetOutputFormatToRGBA();
    this->windowLevelerSagittal->Update();

    //this->axialImageActor->GetMapper()->SetInputConnection(this->axialWinLevel->GetOutputPort());

    //svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT )
                                   //)->SetInputData( this->windowLevelerAxial->GetOutput() );
    //svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK )
                                   //)->SetInputData( this->windowLevelerAxial->GetOutput() );
    //svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT )
                                   //)->SetInputData( this->windowLevelerCoronal->GetOutput() );
    //svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK )
                                   //)->SetInputData( this->windowLevelerCoronal->GetOutput() );
    //svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT )
                                   //)->SetInputData( this->windowLevelerSagittal->GetOutput() );
    //svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK )
                                   //)->SetInputData( this->windowLevelerSagittal->GetOutput() );

    svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT )
                                   )->GetMapper()->SetInputConnection( this->windowLevelerAxial->GetOutputPort() );
    svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK )
                                   )->GetMapper()->SetInputConnection( this->windowLevelerAxial->GetOutputPort() );
    svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT )
                                   )->GetMapper()->SetInputConnection( this->windowLevelerCoronal->GetOutputPort() );
    svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK )
                                   )->GetMapper()->SetInputConnection( this->windowLevelerCoronal->GetOutputPort() );
    svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT )
                                   )->GetMapper()->SetInputConnection( this->windowLevelerSagittal->GetOutputPort() );
    svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK )
                                   )->GetMapper()->SetInputConnection( this->windowLevelerSagittal->GetOutputPort() );

    this->SetInterpolationType( this->interpolationType );
    this->SetOverlayOpacity( this->overlayOpacity );

    if( !this->GetRenderer( svkOverlayView::PRIMARY
                   )->HasViewProp( this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT ) ) ) {

        this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( 
                   this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT ) );
    }

    if( !this->GetRenderer( svkOverlayView::PRIMARY
                   )->HasViewProp( this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK ) ) ) {

        this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( 
                   this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK ) );
    }

    if( !this->GetRenderer( svkOverlayView::PRIMARY
                   )->HasViewProp( this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT ) ) ) {

        this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( 
                   this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT ) );
    }

    if( !this->GetRenderer( svkOverlayView::PRIMARY
                   )->HasViewProp( this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK ) ) ) {

        this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( 
                  this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK ) );
    }

    if( !this->GetRenderer( svkOverlayView::PRIMARY
                   )->HasViewProp( this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT ) ) ) {

        this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( 
                  this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT ) );
    }

    if( !this->GetRenderer( svkOverlayView::PRIMARY
                   )->HasViewProp( this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK ) ) ) {

        this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( 
                  this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK ) );
    }
    this->TurnPropOn( svkOverlayView::AXIAL_OVERLAY_FRONT );
    this->TurnPropOn( svkOverlayView::AXIAL_OVERLAY_BACK );
    this->TurnPropOn( svkOverlayView::CORONAL_OVERLAY_FRONT );
    this->TurnPropOn( svkOverlayView::CORONAL_OVERLAY_BACK );
    this->TurnPropOn( svkOverlayView::SAGITTAL_OVERLAY_FRONT );
    this->TurnPropOn( svkOverlayView::SAGITTAL_OVERLAY_BACK );


    vtkScalarBarActor::SafeDownCast(this->GetProp( svkOverlayView::COLOR_BAR )
                                    )->SetTextPositionToPrecedeScalarBar();

    vtkScalarBarActor::SafeDownCast(this->GetProp( svkOverlayView::COLOR_BAR )
                                    )->UseOpacityOn();

    vtkScalarBarActor::SafeDownCast(this->GetProp( svkOverlayView::COLOR_BAR )
                                    )->SetNumberOfLabels(3);

    vtkScalarBarActor::SafeDownCast(this->GetProp( svkOverlayView::COLOR_BAR )
                                    )->GetLabelTextProperty()->ShadowOff();

    vtkScalarBarActor::SafeDownCast(this->GetProp( svkOverlayView::COLOR_BAR )
                                    )->GetLabelTextProperty()->ItalicOff();

    vtkScalarBarActor::SafeDownCast(this->GetProp( svkOverlayView::COLOR_BAR )
                                    )->SetPosition(0.03,0.23725);
    vtkScalarBarActor::SafeDownCast(this->GetProp( svkOverlayView::COLOR_BAR )
                                    )->SetPosition2(0.145,0.73);

    if( !this->GetRenderer( svkOverlayView::PRIMARY
                           )->HasViewProp( this->GetProp( svkOverlayView::COLOR_BAR) ) ) {

        this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( 
                                this->GetProp( svkOverlayView::COLOR_BAR) );
    }
    

    this->SetProp( svkOverlayView::COLOR_BAR, this->GetProp( svkOverlayView::COLOR_BAR) );
    this->TurnPropOn( svkOverlayView::COLOR_BAR );
    this->GetProp( svkOverlayView::COLOR_BAR )->Modified();
    this->GetRenderer( svkOverlayView::PRIMARY)->Render();

    // Now lets make sure we are looking at the appropriate slice.
    this->SetSliceOverlay();
    this->Refresh();
}


/*!
 *  Sets up the contour overlay actor.
 */
void svkOverlayView::SetupOverlayContour( int contourIndex )
{
    vtkActor* contourActor = this->contourDirector->AddInput(svkMriImageData::SafeDownCast(this->dataVector[contourIndex]));
    this->GetRenderer( svkOverlayView::PRIMARY )->AddActor(contourActor);
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
        // Check to see if the current overlay is interpolated already
        if( this->interpOverlay == dataVector[OVERLAY] ) {
            this->SetInput( svkMriImageData::SafeDownCast(sincInterpolation->GetInput()), OVERLAY );
        }
        svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT ))->InterpolateOff();
        svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK ))->InterpolateOff();
        svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT ))->InterpolateOff();
        svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK ))->InterpolateOff();
        svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT ))->InterpolateOff();
        svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK ))->InterpolateOff();
    } else if (interpolationType == LINEAR) {
        this->interpolationType = LINEAR; 
        // Check to see if the current overlay is interpolated already
        if( this->interpOverlay == dataVector[OVERLAY] ) {
            this->SetInput( svkMriImageData::SafeDownCast(sincInterpolation->GetInput()), OVERLAY );
        }
        if (this->interpolateView)
        if ( this->interpolateView == true ) {
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT ))->InterpolateOn();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK ))->InterpolateOn();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT ))->InterpolateOn();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK ))->InterpolateOn();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT ))->InterpolateOn();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK ))->InterpolateOn();
        } else {
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT ))->InterpolateOff();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK ))->InterpolateOff();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT ))->InterpolateOff();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK ))->InterpolateOff();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT ))->InterpolateOff();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK ))->InterpolateOff();
        }
    } else if (interpolationType == SINC) {
        this->interpolationType = SINC; 
        if( this->interpOverlay != dataVector[OVERLAY] ) {
        	this->UpdateSincInterpolation();
        }
        if ( this->interpolateView == true ) {
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT ))->InterpolateOn();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK ))->InterpolateOn();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT ))->InterpolateOn();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK ))->InterpolateOn();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT ))->InterpolateOn();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK ))->InterpolateOn();
        } else {
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT ))->InterpolateOff();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK ))->InterpolateOff();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT ))->InterpolateOff();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK ))->InterpolateOff();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT ))->InterpolateOff();
            svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK ))->InterpolateOff();
        }
    }
    this->Refresh();
}


/*!
 *
 */
void svkOverlayView::UpdateSincInterpolation()
{
    //  get the extent of the overlay image that is being interpolated. 
	int* extent = this->dataVector[OVERLAY]->GetExtent();
	int xLength = extent[1]-extent[0] + 1;
	int yLength = extent[3]-extent[2] + 1;
	int zLength = extent[5]-extent[4] + 1;

    //  get the extent of the reference "MRI" image that the overlay is being interpolated to.
	int* targetExtent = this->dataVector[MRI]->GetExtent();
    int xLengthTarget = targetExtent[1]-targetExtent[0] + 1;
    int yLengthTarget = targetExtent[3]-targetExtent[2] + 1;
    int zLengthTarget = targetExtent[5]-targetExtent[4] + 1;

	double* imageSpacing = this->dataVector[MRI]->GetSpacing();
	double* overlaySpacing = this->dataVector[OVERLAY]->GetSpacing();

	//  Define the target resolution of the sinc interpolation.  If the target array size is still fairly small 
    //  use it as defined, oterhwise try to interpolate to a size that is a power of 2 and close to the target resolution
    //  so that the FT is "fast". 
	int xSize;
	int ySize;
	int zSize;
    if ( ( xLengthTarget > 64 )  || ( yLengthTarget > 64 ) || ( zLengthTarget > 64 ) )  {
	    xSize = (int)pow( 2., vtkMath::Round( log( static_cast<double>(xLength * (overlaySpacing[0]/imageSpacing[0]) ))/log(2.) ) );
	    ySize = (int)pow( 2., vtkMath::Round( log( static_cast<double>(yLength * (overlaySpacing[1]/imageSpacing[1]) ))/log(2.) ) );
	    zSize = (int)pow( 2., vtkMath::Round( log( static_cast<double>(zLength * (overlaySpacing[2]/imageSpacing[2]) ))/log(2.) ) );
    } else {
        xSize = xLengthTarget; 
        ySize = yLengthTarget; 
        zSize = zLengthTarget; 
    }

	if( xSize > SINC_MAX_EXTENT ) {
		xSize = SINC_MAX_EXTENT;
	}
	if( ySize > SINC_MAX_EXTENT ) {
		ySize = SINC_MAX_EXTENT;
	}
	if( zSize > SINC_MAX_EXTENT ) {
		zSize = SINC_MAX_EXTENT;
	}

	// We need to save the source data
	svkMriImageData* overlaySource = svkMriImageData::SafeDownCast(this->dataVector[OVERLAY]);

	int numArrays = overlaySource->GetPointData()->GetNumberOfArrays();
	this->interpOverlay->Delete();
	this->interpOverlay = svkMriImageData::New();
	vtkDataArray* oldScalars = this->dataVector[OVERLAY]->GetPointData()->GetScalars();

	// This is a temporary Kludge until we have the sinc interpolation algorithm working with multiple volumes
	// Also there is a problem with the sinc interpolate filter that causes it to not update when the input is modified
	// So we recreate the sinc interpolation filter for each volume.
	for( int i = 0; i < numArrays; i++ ) {
		overlaySource->GetPointData()->SetActiveScalars( overlaySource->GetPointData()->GetArray( i )->GetName() );
		this->sincInterpolation->Delete();
		this->sincInterpolation = svkSincInterpolationFilter::New();
		this->sincInterpolation->SetOutputWholeExtent( 0, xSize-1, 0, ySize-1, 0, zSize-1 );
		this->sincInterpolation->SetInputData( overlaySource );
		this->sincInterpolation->Update();
		if( i == 0 ) {
			this->interpOverlay->DeepCopy( this->sincInterpolation->GetOutput() );
            this->interpOverlay->GetPointData()->GetArray( i )->DeepCopy(sincInterpolation->GetOutput()->GetPointData()->GetScalars() );
		} else {
            this->interpOverlay->GetPointData()->AddArray(sincInterpolation->GetOutput()->GetPointData()->GetScalars() );
		}
		// Array name order is not maintained so we need to rename the arrays
		this->interpOverlay->GetPointData()->GetArray( i )->SetName(overlaySource->GetPointData()->GetArray( i )->GetName() );
    }

    this->interpOverlay->GetPointData()->SetActiveScalars( oldScalars->GetName() );

	this->interpOverlay->SyncVTKImageDataToDcmHeader();
	this->SetInput( this->interpOverlay, OVERLAY );
}


/*!
 *  Sets the LUT type 
 *
 *  \param LUT type 
 */
svkLookupTable* svkOverlayView::GetLookupTable( )
{
    return this->colorTransfer; 
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
    double window;
    double level;
    svkMriImageData::SafeDownCast(dataVector[OVERLAY])->GetAutoWindowLevel(window, level);
    this->colorTransfer->SetRange( level - window/2.0, level + window/2.0);

    this->colorTransfer->SetLUTType( type ); 
    this->colorTransfer->SetAlphaThreshold( threshold ); 

    this->windowLevelerAxial->SetLookupTable( this->colorTransfer );
    this->windowLevelerCoronal->SetLookupTable( this->colorTransfer );
    this->windowLevelerSagittal->SetLookupTable( this->colorTransfer );

    vtkScalarBarActor::SafeDownCast(this->GetProp( svkOverlayView::COLOR_BAR )
                                    )->SetLookupTable( this->colorTransfer );

    svkOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT ))->Modified( );

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

    string resultInfo = "";
    if ( !isValidationOn ) {
        return resultInfo; 
    } 

    svkDataValidator* validator = svkDataValidator::New();
    
    // Check for null datasets and out of bound data sets...
    if ( data == NULL ||  targetIndex < 0 ) {

        resultInfo = "Data incompatible-- NULL or outside of input range.\n";

    } else if( data->IsA("svkMriImageData") ) {
        
        if( dataVector[MR4D] != NULL ) {
            bool valid = validator->AreDataCompatible( data, dataVector[MR4D] );
            if( validator->IsInvalid( svkDataValidator::INVALID_DATA_ORIENTATION ) ) {
                resultInfo = "Orientation mismatch: reslicing images to MR4D data orientation.";
                resultInfo = ""; 
            } else if ( !valid ) {
                resultInfo += validator->resultInfo.c_str();
                resultInfo += "\n";
            }
        } else {
            svkImageData* loadedImage = NULL;
            if( targetIndex == MRI && this->dataVector[OVERLAY] != NULL ) {
                loadedImage = this->dataVector[OVERLAY];
            } else if( targetIndex == MRI && this->dataVector.size() > OVERLAY_CONTOUR && this->dataVector[OVERLAY_CONTOUR] != NULL ) {
                loadedImage = this->dataVector[OVERLAY_CONTOUR];
            } else if ( targetIndex >= OVERLAY && this->dataVector[MRI] != NULL ) {
                loadedImage = this->dataVector[MRI];
            }
            if( loadedImage != NULL ) {
                bool valid = validator->AreDataCompatible( data, loadedImage );
                if( validator->IsOnlyError( svkDataValidator::INVALID_DATA_ORIENTATION ) ) {
                    resultInfo = "Orientation mismatch: reslicing image data orientation.";
                    resultInfo = "";
                } else if ( !valid ) {
                    resultInfo += validator->resultInfo.c_str();
                    resultInfo += "\n";
                }
            }
        }

    } else if( data->IsA("svk4DImageData") ) {

        if( dataVector[MRI] != NULL ) {
            bool valid = validator->AreDataCompatible( data, dataVector[MRI] ); 
            if( validator->IsInvalid( svkDataValidator::INVALID_DATA_ORIENTATION ) ) {
                resultInfo = "Orientation mismatch: reslicing images to MR4D data orientation.";
                resultInfo = ""; 
            } else if ( !valid ) {
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
 *  Reslice images to MR4D orientation
 */
bool svkOverlayView::ResliceImage(svkImageData* input, svkImageData* target, int targetIndex)
{
    bool didReslice = false;
    if( input != NULL && target != NULL && input->IsA("svkMriImageData") ) {
        svkDataValidator* validator = svkDataValidator::New();
        bool valid = validator->AreDataCompatible( input, target );
        if( validator->IsInvalid( svkDataValidator::INVALID_DATA_ORIENTATION ) ) {
            cout << "Reslice image to MRS orientation  " << endl;
            svkObliqueReslice* reslicer = svkObliqueReslice::New();
            reslicer->SetInputData( input );
            reslicer->SetTarget( target );
            reslicer->Update();

            string resultInfo = this->GetDataCompatibility( reslicer->GetOutput(), targetIndex );
            if( strcmp( resultInfo.c_str(), "" ) == 0 ) {
                this->SetInputPostReslice( reslicer->GetOutput(), targetIndex );
                input = reslicer->GetOutput();
                didReslice = true;
            }
            reslicer->Delete();
        }

        //================================================================================
       
        if( input != NULL && input->IsA("svkMriImageData") && target->IsA("svk4DImageData") ) {
            double targetOrigin[3]; 
            float imageToSpecSliceThickness;
            if ( this->OriginShiftRequired(input, target, targetOrigin) == true ) {
                cout << "SHIFT ORIGIN!!! spacings identical and 1/2 voxel center diff" << endl;
                //  the origin shift doesn't work right now, so just upsample image data slightly
                //
                
                //svkObliqueReslice* reslicer3 = svkObliqueReslice::New();
                //reslicer3->SetInput( input );
                //reslicer3->SetTargetDcosFromImage( target );
                //reslicer3->SetOutputOrigin( targetOrigin );
                //reslicer3->Update();
                ////double oo[3]; 
                ////reslicer3->GetOutputOrigin(oo); 
                ////cout << "INPUT " << *input << endl;
                ////cout << "OO : " << oo[0]  << " " << oo[1] << " " << oo[2] << endl;
                //this->SetInputPostReslice( reslicer3->GetOutput(), targetIndex );
                //didReslice = true;
                //reslicer3->Delete(); 

                float magX = 1; 
                float magY = 1; 
                float magZ = 1.01; //this decreases the imag slice spacing.  
                cout << "reduce image slice spacing by factor of: " << magZ << endl;
                svkObliqueReslice* reslicer3 = svkObliqueReslice::New();
                reslicer3->SetInputData( input );
                reslicer3->SetInterpolationMode( VTK_RESLICE_NEAREST );
                reslicer3->SetTarget( target );
                reslicer3->SetMagnificationFactors( magX, magY, 1./magZ);
                reslicer3->Update();
                this->SetInputPostReslice( reslicer3->GetOutput(), targetIndex );
                didReslice = true;
                reslicer3->Delete(); 
            }  else if ( ( imageToSpecSliceThickness = this->GetImageToSpecSliceRatio(input, target) ) > 1. ) {
                //  check to see if image needs to be upsampled to a higher resolution: 
                float magX = 1; 
                float magY = 1; 
                float magZ = imageToSpecSliceThickness; 
                cout << "MRS slice thicknes < MRI slice thickness.  Upsample MRI slice thickness " << endl;
                cout << "upsample factor: = " << magZ << endl;
                svkObliqueReslice* reslicer2 = svkObliqueReslice::New();
                reslicer2->SetInputData( input );
                reslicer2->SetInterpolationMode( VTK_RESLICE_NEAREST );
                reslicer2->SetTarget( target );
                reslicer2->SetMagnificationFactors( magX, magY, 1./magZ);
                reslicer2->Update();
                this->SetInputPostReslice( reslicer2->GetOutput(), targetIndex );
                didReslice = true;
                reslicer2->Delete(); 
            }
        }
        validator->Delete();
    }
    if( didReslice ){
        cout << "RESLICED INDEX: " << targetIndex << endl;
    }
    return didReslice;
}


/*!  
 *  If the slice thickness fo the image and spec are the same but offset by 1/2 voxel,
 *  then reslice to have the same origins: 
 *  Check the delta by projeting both onto the normal    
 */
bool svkOverlayView::OriginShiftRequired(svkImageData* input, svkImageData* target, double* targetOrigin)
{

    bool originShiftRequired = false; 

    double specPixelSpacing[3];
    target->GetDcmHeader()->GetPixelSpacing( specPixelSpacing );
    double imagePixelSpacing[3];
    input->GetDcmHeader()->GetPixelSpacing( imagePixelSpacing );
    double tol = .001; 

    //cout << "FABS: " <<  fabs( specPixelSpacing[2] - imagePixelSpacing[2] ) << endl; 
    //  spacing of both images is the same within tolerance
    if ( fabs( specPixelSpacing[2] - imagePixelSpacing[2] ) < tol ) {

        double normal[3];
        input->GetSliceNormal( normal, this->orientation);

        int slice = 0;
        double specCenter[3];
        double imagCenter[3];
        target->GetSliceCenter( slice, specCenter, this->orientation );
        input->GetSliceCenter(  slice, imagCenter, this->orientation );
            
        //cout << "NML: " << normal[0] << " " << normal[1] << " " << normal[2] << endl;
        //cout << "SSC: " << specCenter[0] << " " << specCenter[1] << " " << specCenter[2] << endl;
        //cout << "ISC: " << imagCenter[0] << " " << imagCenter[1] << " " << imagCenter[2] << endl;
        
        double projectedSpecCenter = vtkMath::Dot( specCenter, normal );
        double projectedImagCenter = vtkMath::Dot( imagCenter, normal );
        //cout << "PSPEC: " << projectedSpecCenter << endl;
        //cout << "PIMAG: " << projectedImagCenter << endl;
    
        //  number of slice thickness between the 2 origins: 
        //  If the number is within tol of 1/2 slice thickness then shift origin:  
        double normalizedSliceCenterDiff = fabs( projectedSpecCenter - projectedImagCenter )  /  specPixelSpacing[2] ; 
        int    intDiff = (int) normalizedSliceCenterDiff; 
            
        //  centers are off by 1/2 a voxel within tolerance
        if (  fabs(  fabs( normalizedSliceCenterDiff - intDiff ) - .5 )  < tol ) {
            cout << "NEED TOSHIFT ORIGIN: " <<  normalizedSliceCenterDiff << endl;
            originShiftRequired = true; 
            targetOrigin[0] = specCenter[0];     
            targetOrigin[1] = specCenter[1];     
            targetOrigin[2] = specCenter[2];     
        }
    }
    return originShiftRequired; 
}


/*
 *
 */
float svkOverlayView::GetImageToSpecSliceRatio(svkImageData* input, svkImageData* target) 
{
    float imageToSpecSliceThickness; 
    double pixelSpacing[3];
    input->GetDcmHeader()->GetPixelSpacing( pixelSpacing );
    float imageSliceThickness = pixelSpacing[2]; 
    target->GetDcmHeader()->GetPixelSpacing( pixelSpacing );
    float specSliceThickness  = pixelSpacing[2]; 
    imageToSpecSliceThickness = imageSliceThickness / specSliceThickness; 
    return imageToSpecSliceThickness; 
}


/*
 *
 */
bool svkOverlayView::CheckDataOrientations()
{
    bool orientationsOkay = true;
    vector<svkImageData*> allImages( this->dataVector );
    svkImageData* firstDataset = allImages[0];
    double dcos[3][3];
    firstDataset->GetDcos(dcos);
    for( int i = 1; i < allImages.size(); i++ ) {
        if( allImages[i] != NULL ) {
            svkDataValidator* validator = svkDataValidator::New();
            bool valid = validator->AreDataCompatible( allImages[i], firstDataset );
            allImages[i]->GetDcos(dcos);
            if( validator->IsInvalid( svkDataValidator::INVALID_DATA_ORIENTATION ) ) {
                orientationsOkay = false;
            }
            validator->Delete();
        }
    }
    if( !orientationsOkay  ) {
        cerr << "FATAL ERROR: Orientations were not correctly re-sliced. Exiting sivic..." << endl;
        exit(1);
    }
    return orientationsOkay;

}

//! Resets the window level, source taken from vtkImageViewer2
void svkOverlayView::ResetWindowLevel()
{
    if( dataVector[MRI] != NULL ) {

        //this->imageViewer->GetInput()->UpdateInformation();
        //this->imageViewer->GetInput()->SetUpdateExtent
            //(this->imageViewer->GetInput()->GetWholeExtent());
        //this->imageViewer->GetInput()->Update();
        this->imageViewer->UpdateInputInformation(); 

        double window;
        double level;
        svkMriImageData::SafeDownCast(dataVector[MRI])->GetAutoWindowLevel(window, level);
        int toggleDraw = this->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
        if( toggleDraw ) {
            this->GetRenderer( svkOverlayView::PRIMARY)->DrawOff();
        }
        this->imageViewer->SetColorWindow(window);
        this->imageViewer->SetColorLevel(level);
        this->imageViewer->GetImageActor()->Modified();
        if( toggleDraw ) {
            this->GetRenderer( svkOverlayView::PRIMARY)->DrawOn();
        }
        this->imageViewer->Render();
   }
}


/*
 *  Turns the orthogonal images on.
 */
void svkOverlayView::TurnOrthogonalImagesOn()
{
    this->imageViewer->TurnOrthogonalImagesOn();
    bool areSatBandsOn;
    bool areSatBandOutlinesOn;

    switch( this->GetOrientation() ) {
        case svkDcmHeader::AXIAL:
             areSatBandsOn = this->IsPropOn(svkOverlayView::SAT_BANDS_AXIAL);
             areSatBandOutlinesOn = this->IsPropOn(svkOverlayView::SAT_BANDS_AXIAL_OUTLINE);
            break;
        case svkDcmHeader::CORONAL:
             areSatBandsOn = this->IsPropOn(svkOverlayView::SAT_BANDS_CORONAL);
             areSatBandOutlinesOn = this->IsPropOn(svkOverlayView::SAT_BANDS_CORONAL_OUTLINE);
            break;
        case svkDcmHeader::SAGITTAL:
             areSatBandsOn = this->IsPropOn(svkOverlayView::SAT_BANDS_SAGITTAL);
             areSatBandOutlinesOn = this->IsPropOn(svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE);
            break;
    }
    if( areSatBandsOn ) {
        this->TurnPropOn(svkOverlayView::SAT_BANDS_AXIAL); 
        this->TurnPropOn(svkOverlayView::SAT_BANDS_CORONAL); 
        this->TurnPropOn(svkOverlayView::SAT_BANDS_SAGITTAL); 
    }
    if( areSatBandOutlinesOn ) {
        this->TurnPropOn(svkOverlayView::SAT_BANDS_AXIAL_OUTLINE); 
        this->TurnPropOn(svkOverlayView::SAT_BANDS_CORONAL_OUTLINE); 
        this->TurnPropOn(svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE); 
    }
}


/*
 *  Turns the orthogonal images off.
 */
void svkOverlayView::TurnOrthogonalImagesOff()
{
    this->imageViewer->TurnOrthogonalImagesOff();
    bool satBandsOn = 0; 
    bool satBandsOutlineOn = 0; 
    if( this->IsPropOn(svkOverlayView::SAT_BANDS_AXIAL) ||
        this->IsPropOn(svkOverlayView::SAT_BANDS_CORONAL) || 
        this->IsPropOn(svkOverlayView::SAT_BANDS_SAGITTAL) ) { 
        satBandsOn = 1;
    }
    if( this->IsPropOn(svkOverlayView::SAT_BANDS_AXIAL_OUTLINE) ||
        this->IsPropOn(svkOverlayView::SAT_BANDS_CORONAL_OUTLINE) || 
        this->IsPropOn(svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE) ) { 
        satBandsOutlineOn = 1;
    }
        
    // Lets make sure orthogonal sat bands are off...
    this->TurnPropOff(svkOverlayView::SAT_BANDS_AXIAL); 
    this->TurnPropOff(svkOverlayView::SAT_BANDS_AXIAL_OUTLINE); 
    this->TurnPropOff(svkOverlayView::SAT_BANDS_CORONAL); 
    this->TurnPropOff(svkOverlayView::SAT_BANDS_CORONAL_OUTLINE); 
    this->TurnPropOff(svkOverlayView::SAT_BANDS_SAGITTAL); 
    this->TurnPropOff(svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE); 

    if( satBandsOn ) { 
        switch( this->GetOrientation()) {
            case svkDcmHeader::AXIAL:
                this->TurnPropOn(svkOverlayView::SAT_BANDS_AXIAL);
                break;
            case svkDcmHeader::CORONAL:
                this->TurnPropOn(svkOverlayView::SAT_BANDS_CORONAL);
                break;
            case svkDcmHeader::SAGITTAL:
                this->TurnPropOn(svkOverlayView::SAT_BANDS_SAGITTAL);
                break;
        }
    }
    if( satBandsOutlineOn ) { 
        switch( this->GetOrientation() ) {
            case svkDcmHeader::AXIAL:
                this->TurnPropOn(svkOverlayView::SAT_BANDS_AXIAL_OUTLINE);
                break;
            case svkDcmHeader::CORONAL:
                this->TurnPropOn(svkOverlayView::SAT_BANDS_CORONAL_OUTLINE);
                break;
            case svkDcmHeader::SAGITTAL:
                this->TurnPropOn(svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE);
                break;
        }
    }
}

/*
 *  Turns the orthogonal images off.
 */
bool svkOverlayView::AreOrthogonalImagesOn()
{
    return this->imageViewer->AreOrthogonalImagesOn();
}

/*!     
 *
 */     
void svkOverlayView::SetOrientation( svkDcmHeader::Orientation orientation )
{
    svkDcmHeader::Orientation oldOrientation;
    oldOrientation = this->orientation;
    this->orientation = orientation;
    if( this->dataVector[MRI] != NULL ) {
        int toggleDraw = this->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
        if( toggleDraw ) {
            this->GetRenderer( svkOverlayView::PRIMARY)->DrawOff();
        }
        this->imageViewer->SetOrientation( orientation );
        //this->AlignCamera();
        if( this->dataVector[MR4D] != NULL ) {
            svkDataView::ResetTlcBrcForNewOrientation( this->dataVector[MR4D], this->orientation, this->tlcBrc, this->slice );
            bool satBandsAllOn = false;
            bool satBandsOutlineAllOn = false;
            bool satBandsSliceOn = false;
            bool satBandsOutlineSliceOn = false;
/*
            int imageSlice = this->imageViewer->GetSlice( orientation );
            int spectraSlice = this->FindSpectraSlice( imageSlice, orientation);
*/
            this->imageViewer->GetImageActor( svkDcmHeader::AXIAL )->SetVisibility(1);
            this->imageViewer->GetImageActor( svkDcmHeader::CORONAL )->SetVisibility(1);
            this->imageViewer->GetImageActor( svkDcmHeader::SAGITTAL )->SetVisibility(1);
            //this->HighlightSelectionVoxels();
            if( !this->AreAllSatBandsOn( oldOrientation ) ) {
                if( this->IsSatBandForSliceOn( oldOrientation ) ) {
                    switch( oldOrientation ) {
                        case svkDcmHeader::AXIAL:
                            this->TurnPropOff( svkOverlayView::SAT_BANDS_AXIAL );
                            break;
                        case svkDcmHeader::CORONAL:
                            this->TurnPropOff( svkOverlayView::SAT_BANDS_CORONAL );
                            break;
                        case svkDcmHeader::SAGITTAL:
                            this->TurnPropOff( svkOverlayView::SAT_BANDS_SAGITTAL );
                            break;
                    }
                    switch( this->orientation ) {
                        case svkDcmHeader::AXIAL:
                            this->TurnPropOn( svkOverlayView::SAT_BANDS_AXIAL );
                            break;
                        case svkDcmHeader::CORONAL:
                            this->TurnPropOn( svkOverlayView::SAT_BANDS_CORONAL );
                            break;
                        case svkDcmHeader::SAGITTAL:
                            this->TurnPropOn( svkOverlayView::SAT_BANDS_SAGITTAL );
                            break;
                    }
                }
            }
            if( !this->AreAllSatBandOutlinesOn( oldOrientation ) ) {
                if( this->IsSatBandOutlineForSliceOn( oldOrientation ) ) {
                    switch( oldOrientation ) {
                        case svkDcmHeader::AXIAL:
                            this->TurnPropOff( svkOverlayView::SAT_BANDS_AXIAL_OUTLINE);
                            break;
                        case svkDcmHeader::CORONAL:
                            this->TurnPropOff( svkOverlayView::SAT_BANDS_CORONAL_OUTLINE);
                            break;
                        case svkDcmHeader::SAGITTAL:
                            this->TurnPropOff( svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE);
                            break;
                    }
                    switch( this->orientation ) {
                        case svkDcmHeader::AXIAL:
                            this->TurnPropOn( svkOverlayView::SAT_BANDS_AXIAL_OUTLINE);
                            break;
                        case svkDcmHeader::CORONAL:
                            this->TurnPropOn( svkOverlayView::SAT_BANDS_CORONAL_OUTLINE);
                            break;
                        case svkDcmHeader::SAGITTAL:
                            this->TurnPropOn( svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE);
                            break;
                    }
                }
            } 

            this->SetSlice( this->slice );

        } else {
            this->SetSlice( this->imageViewer->GetSlice( orientation ), orientation );
        }
        if( toggleDraw ) {
            this->GetRenderer( svkOverlayView::PRIMARY)->DrawOn();
        }
        this->UpdateSelectionBoxVisibility();
        this->Refresh();
    }
}


/*!     
 *
 */     
bool svkOverlayView::IsSatBandForSliceOn( svkDcmHeader::Orientation orientation )
{
    bool satBandsSliceOn = false;
    switch( orientation ) {
        case svkDcmHeader::AXIAL:
            if( this->IsPropOn( svkOverlayView::SAT_BANDS_AXIAL ) ) {
                satBandsSliceOn = true;
            }
            break;
        case svkDcmHeader::CORONAL:
            if( this->IsPropOn( svkOverlayView::SAT_BANDS_CORONAL ) ) {
                satBandsSliceOn = true;
            }
            break;
        case svkDcmHeader::SAGITTAL:
            if( this->IsPropOn( svkOverlayView::SAT_BANDS_SAGITTAL ) ) {
                satBandsSliceOn = true;
            }
            break;
    }
    return satBandsSliceOn;
}


/*!     
 *
 */     
bool svkOverlayView::IsSatBandOutlineForSliceOn( svkDcmHeader::Orientation orientation )
{
    bool satBandsOutlineSliceOn = false;
    switch( orientation ) {
        case svkDcmHeader::AXIAL:
            if( this->IsPropOn( svkOverlayView::SAT_BANDS_AXIAL_OUTLINE ) ) {
                satBandsOutlineSliceOn = true;
            }
            break;
        case svkDcmHeader::CORONAL:
            if( this->IsPropOn( svkOverlayView::SAT_BANDS_CORONAL_OUTLINE ) ) {
                satBandsOutlineSliceOn = true;
            }
            break;
        case svkDcmHeader::SAGITTAL:
            if( this->IsPropOn( svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE ) ) {
                satBandsOutlineSliceOn = true;
            }
            break;
    }
    return satBandsOutlineSliceOn;
}


/*!     
 * Are all the sat bands on? If any bands other than the slice is on, we'll say yes
 */     
bool svkOverlayView::AreAllSatBandsOn( svkDcmHeader::Orientation orientation )
{
    bool satBandsAllOn = false;
    switch( orientation ) {
        case svkDcmHeader::AXIAL:
            if( this->IsPropOn( svkOverlayView::SAT_BANDS_CORONAL ) ) {
                satBandsAllOn = true;
            }
            if( this->IsPropOn( svkOverlayView::SAT_BANDS_SAGITTAL ) ) {
                satBandsAllOn = true;
            }
            break;
        case svkDcmHeader::CORONAL:
            if( this->IsPropOn( svkOverlayView::SAT_BANDS_AXIAL ) ) {
                satBandsAllOn = true;
            }
            if( this->IsPropOn( svkOverlayView::SAT_BANDS_SAGITTAL ) ) {
                satBandsAllOn = true;
            }
            break;
        case svkDcmHeader::SAGITTAL:
            if( this->IsPropOn( svkOverlayView::SAT_BANDS_AXIAL ) ) {
                satBandsAllOn = true;
            }
            if( this->IsPropOn( svkOverlayView::SAT_BANDS_CORONAL ) ) {
                satBandsAllOn = true;
            }
            break;
    }
    return satBandsAllOn;
}


/*!     
 *
 */     
bool svkOverlayView::AreAllSatBandOutlinesOn( svkDcmHeader::Orientation orientation )
{
    bool satBandsOutlineAllOn = false;
    switch( orientation ) {
        case svkDcmHeader::AXIAL:
            if( this->IsPropOn( svkOverlayView::SAT_BANDS_CORONAL_OUTLINE ) ) {
                satBandsOutlineAllOn = true;
            }
            if( this->IsPropOn( svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE ) ) {
                satBandsOutlineAllOn = true;
            }
            break;
        case svkDcmHeader::CORONAL:
            if( this->IsPropOn( svkOverlayView::SAT_BANDS_AXIAL_OUTLINE ) ) {
                satBandsOutlineAllOn = true;
            }
            if( this->IsPropOn( svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE ) ) {
                satBandsOutlineAllOn = true;
            }
            break;
        case svkDcmHeader::SAGITTAL:
            if( this->IsPropOn( svkOverlayView::SAT_BANDS_AXIAL_OUTLINE ) ) {
                satBandsOutlineAllOn = true;
            }
            if( this->IsPropOn( svkOverlayView::SAT_BANDS_CORONAL_OUTLINE ) ) {
                satBandsOutlineAllOn = true;
            }
            break;
    }
    return satBandsOutlineAllOn;

}


/*!
 *
 */
void svkOverlayView::AlignCamera() 
{
    int toggleDraw = this->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->GetRenderer( svkOverlayView::PRIMARY)->DrawOff();
    }

    // We don't want the presence of the sat bands to influence the alignment, so we will temporarily remove them.
    this->GetRenderer( svkOverlayView::PRIMARY)->RemoveViewProp( this->GetProp( svkOverlayView::SAT_BANDS_AXIAL) );
    this->GetRenderer( svkOverlayView::PRIMARY)->RemoveViewProp( this->GetProp( svkOverlayView::SAT_BANDS_AXIAL_OUTLINE) );
    this->GetRenderer( svkOverlayView::PRIMARY)->RemoveViewProp( this->GetProp( svkOverlayView::SAT_BANDS_CORONAL) );
    this->GetRenderer( svkOverlayView::PRIMARY)->RemoveViewProp( this->GetProp( svkOverlayView::SAT_BANDS_CORONAL_OUTLINE) );
    this->GetRenderer( svkOverlayView::PRIMARY)->RemoveViewProp( this->GetProp( svkOverlayView::SAT_BANDS_SAGITTAL) );
    this->GetRenderer( svkOverlayView::PRIMARY)->RemoveViewProp( this->GetProp( svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE) );
    
    // For consistency with VTK6 we remove the clipping planes, reset the camera, then put them back.
    if( vtkActor::SafeDownCast( this->GetProp( svkOverlayView::PLOT_GRID )) && vtkActor::SafeDownCast( this->GetProp( svkOverlayView::PLOT_GRID ))->GetMapper()) {
		vtkActor::SafeDownCast( this->GetProp( svkOverlayView::PLOT_GRID ))->GetMapper()->RemoveAllClippingPlanes();
    }
    this->imageViewer->ResetCamera();
    this->GenerateClippingPlanes();
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_AXIAL) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_AXIAL_OUTLINE) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_CORONAL) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_CORONAL_OUTLINE) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_SAGITTAL) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE) );

    if( toggleDraw ) {
        this->GetRenderer( svkOverlayView::PRIMARY)->DrawOn();
    }
}


/*!
 *  Set the camera zoom factor
 */
void svkOverlayView::SetCameraZoom( double zoom )
{
    this->imageViewer->SetCameraZoom( zoom ); 
    this->imageViewer->ResetCamera(); 
}


/*!
 *  Returns true if the current image is within the spectrscopy data set
 */
bool svkOverlayView::IsImageInsideSpectra() 
{
    return imageInsideSpectra;
}

void svkOverlayView::SetSelectionBoxVisibility(svkOverlayView::SelectionBoxVisibilityState visibility)
{
    this->selBoxVisibility = visibility;
    this->UpdateSelectionBoxVisibility();
}

svkOverlayView::SelectionBoxVisibilityState svkOverlayView::GetSelectionBoxVisibility( ) {
    return  this->selBoxVisibility;
}

void svkOverlayView::SetContourColor(int index, svkOverlayContourDirector::ContourColor color)
{
    this->contourDirector->SetContourColor(index, color);
}
