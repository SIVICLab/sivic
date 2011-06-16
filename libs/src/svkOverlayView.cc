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


#include <svkOverlayView.h>


using namespace svk;


vtkCxxRevisionMacro(svkOverlayView, "$Rev$");
vtkStandardNewMacro(svkOverlayView);


const double svkOverlayView::CLIP_TOLERANCE = 0.001; 


/*!
 *  Constructor creates the vtkImageView2, and sets its size.
 */
svkOverlayView::svkOverlayView()
{
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
    this->toggleSelBoxVisibility = true;

    this->windowLevelerAxial = NULL;
    this->windowLevelerCoronal = NULL;
    this->windowLevelerSagittal = NULL;
    this->colorTransfer = NULL ;
    this->imageInsideSpectra = false;
    this->overlayOpacity = 0.5;
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

    svkOpenGLOrientedImageActor* overlayActor = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkOverlayView::AXIAL_OVERLAY_FRONT, overlayActor );
    overlayActor->Delete();

    svkOpenGLOrientedImageActor* overlayActorBack = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkOverlayView::AXIAL_OVERLAY_BACK, overlayActorBack );
    overlayActorBack->Delete();

    overlayActor = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkOverlayView::CORONAL_OVERLAY_FRONT, overlayActor );
    overlayActor->Delete();

    overlayActorBack = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkOverlayView::CORONAL_OVERLAY_BACK, overlayActorBack );
    overlayActorBack->Delete();

    overlayActor = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT, overlayActor );
    overlayActor->Delete();

    overlayActorBack = svkOpenGLOrientedImageActor::New();
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
    //vtkActor::SafeDownCast( GetProp( svkOverlayView::PLOT_GRID) )->GetProperty()->SetDiffuseColor( 0, 1, 0 );
    vtkActor::SafeDownCast( GetProp( svkOverlayView::PLOT_GRID) )->GetProperty()->SetDiffuseColor( 0, 0, 0 );

    // Now we need to grab the selection box
    vtkActorCollection* selectionTopo = dataVector[MRS]->GetTopoActorCollection( 1 );

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
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::PLOT_GRID ) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_AXIAL) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_AXIAL_OUTLINE) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_CORONAL) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_CORONAL_OUTLINE) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_SAGITTAL) );
    this->GetRenderer( svkOverlayView::PRIMARY)->AddActor( this->GetProp( svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE) );

   
    this->SetProp( svkOverlayView::PLOT_GRID, this->GetProp( svkOverlayView::PLOT_GRID ) );
    string acquisitionType = dataVector[MRS]->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
    if( acquisitionType != "SINGLE VOXEL" ) {
        this->TurnPropOn( svkOverlayView::PLOT_GRID );
    } else {
        this->TurnPropOff( svkOverlayView::PLOT_GRID );
    }

    this->SetSlice( slice );
    if( resetViewState ) {
        //this->AlignCamera();
        this->HighlightSelectionVoxels();
    }

    this->satBandsAxial->SetInput( static_cast<svkMrsImageData*>(this->dataVector[MRS]) );
    this->satBandsCoronal->SetInput( static_cast<svkMrsImageData*>(this->dataVector[MRS]) );
    this->satBandsSagittal->SetInput( static_cast<svkMrsImageData*>(this->dataVector[MRS]) );
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
    imageViewer->SetInput( dataVector[MRI] );
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
    if( dataVector[MRS] == NULL ) {
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
    int indexModified = this->InitReslice( data, index ); 
    if( indexModified == index ) {
        return; // Case where the reslice handled the current input target
    }
    if( strcmp( resultInfo.c_str(), "" ) == 0 ) { 
    
        if( dataVector[index] != NULL ) {
            if( indexModified == -1 ) {// if the dataset was not resliced then do not reset view. 
                //resetViewState = 0;
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
        this->Refresh();
        //this->SetOrientation( this->orientation );

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
        if( dataVector[MRS] != NULL ) { 
    
            if( tlcBrc[0] >= 0 && tlcBrc[1] >= 0 ) {
                int* extent = dataVector[MRS]->GetExtent();
                svkDcmHeader::Orientation dataOrientation = dataVector[MRS]->GetDcmHeader()->GetOrientationType();
                int tlcIndex[3];
                int brcIndex[3];
                this->dataVector[MRS]->GetIndexFromID( tlcBrc[0], tlcIndex );
                this->dataVector[MRS]->GetIndexFromID( tlcBrc[1], brcIndex );
                int lastSlice  = dataVector[MRS]->GetLastSlice( this->orientation );
                int firstSlice = dataVector[MRS]->GetFirstSlice( this->orientation );
                slice = (slice > lastSlice) ? lastSlice:slice;
                slice = (slice < firstSlice) ? firstSlice:slice;
                tlcIndex[ this->dataVector[MRS]->GetOrientationIndex( this->orientation ) ] = slice;
                brcIndex[ this->dataVector[MRS]->GetOrientationIndex( this->orientation ) ] = slice;
                tlcBrc[0] = this->dataVector[MRS]->GetIDFromIndex( tlcIndex[0], tlcIndex[1], tlcIndex[2] );
                tlcBrc[1] = this->dataVector[MRS]->GetIDFromIndex( brcIndex[0], brcIndex[1], brcIndex[2] );
            }
            this->slice = slice;
            this->GenerateClippingPlanes();

            // Case for no selection box
            if( this->GetProp( svkOverlayView::VOL_SELECTION ) != NULL ) {

                // If it is make it visible, otherwise hide it
                if( static_cast<svkMrsImageData*>(this->dataVector[MRS])->IsSliceInSelectionBox( this->slice, this->orientation ) && isPropOn[VOL_SELECTION] && this->toggleSelBoxVisibility) {
                    this->GetProp( svkOverlayView::VOL_SELECTION )->SetVisibility(1);
                } else if( this->toggleSelBoxVisibility ) {
                    this->GetProp( svkOverlayView::VOL_SELECTION )->SetVisibility(0);
                }

            }
            int toggleDraw = this->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
            if( toggleDraw ) {
                this->GetRenderer( svkOverlayView::PRIMARY)->DrawOff();
            }
            this->UpdateImageSlice( centerImage );
            this->SetSliceOverlay();
                
            if( toggleDraw ) {
                this->GetRenderer( svkOverlayView::PRIMARY)->DrawOn();
            }
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
void svkOverlayView::SetSlice(int slice, svkDcmHeader::Orientation orientation)
{
    int toggleDraw = this->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->GetRenderer( svkOverlayView::PRIMARY)->DrawOff();
    }
    if( this->dataVector[MRS] != NULL && orientation == this->orientation ) {

        // We may have removed the plot grid, so lets make sure its present
        if( !this->GetRenderer(svkOverlayView::PRIMARY)->HasViewProp( this->GetProp( svkOverlayView::PLOT_GRID ) )) {
            this->GetRenderer(svkOverlayView::PRIMARY)->AddViewProp(this->GetProp( svkOverlayView::PLOT_GRID ));
        }
        int newSpectraSlice = this->FindSpectraSlice( slice, orientation );
        if( static_cast<svkMrsImageData*>(this->dataVector[MRS])->IsSliceInSelectionBox( newSpectraSlice, orientation ) 
                 && isPropOn[VOL_SELECTION] 
                 && this->toggleSelBoxVisibility) {
            this->GetProp( svkOverlayView::VOL_SELECTION )->SetVisibility(1);
        } else if( this->GetProp( svkOverlayView::VOL_SELECTION) && this->toggleSelBoxVisibility ) {
            this->GetProp( svkOverlayView::VOL_SELECTION )->SetVisibility(0);
        }
        if(  newSpectraSlice >= this->dataVector[MRS]->GetFirstSlice( this->orientation ) &&
             newSpectraSlice <=  this->dataVector[MRS]->GetLastSlice( this->orientation ) ) {

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

    if( dataVector[MRS] != NULL ) {
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


/*
 *  Finds the image slice that most closely corresponds to the input spectra slice.
 */
int svkOverlayView::FindCenterImageSlice( int spectraSlice, svkDcmHeader::Orientation orientation ) 
{
    int imageSlice;
    double spectraSliceCenter[3];
    this->dataVector[MRS]->GetSliceCenter( spectraSlice, spectraSliceCenter, orientation );
    double normal[3];
    this->dataVector[MRS]->GetSliceNormal( normal, orientation );
    int index = this->dataVector[MRS]->GetOrientationIndex( orientation );
    double tolerance = this->dataVector[MRS]->GetSpacing()[index]/2.0;

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
    int index = this->dataVector[MRS]->GetOrientationIndex( orientation );
    double tolerance = this->dataVector[MRS]->GetSpacing()[index]/2.0;
    spectraSlice = this->dataVector[MRS]->GetClosestSlice( imageSliceCenter, orientation, tolerance );
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
    if( this->dataVector[MRS] != NULL ) {
        int index = this->dataVector[MRS]->GetOrientationIndex( orientation );
        tolerance = this->dataVector[MRS]->GetSpacing()[index]/2.0;
    } else {
        int index = this->dataVector[MRI]->GetOrientationIndex( orientation );
        tolerance = this->dataVector[MRI]->GetSpacing()[index]/2.0;
    }
    overlaySlice = this->dataVector[OVERLAY]->GetClosestSlice( sliceCenter, orientation, tolerance );
    return overlaySlice;
}


/*!
 *  Sets the slice of the anatamical data, based on the spectroscopic slice.
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
        double selection[6];

        selection[0] = worldStart[0];
        selection[1] = worldEnd[0];
        selection[2] = worldStart[1];
        selection[3] = worldEnd[1];
        selection[4] = worldStart[2];
        selection[5] = worldEnd[2];

        int tlcBrcImageData[2];
        svkMrsImageData::SafeDownCast(this->dataVector[MRS])->GetTlcBrcInUserSelection( tlcBrcImageData, selection, this->orientation, this->slice );
        this->SetTlcBrc( tlcBrcImageData );
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
void svkOverlayView::SetTlcBrc( int* tlcBrc ) 
{
    if( svkDataView::IsTlcBrcWithinData( this->dataVector[MRS], tlcBrc ) ) {
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
        int tlcBrcImageData[2];
        svkMrsImageData::SafeDownCast(this->dataVector[MRS])->GetTlcBrcInSelectionBox( tlcBrcImageData, this->orientation, this->slice );
        this->SetTlcBrc( tlcBrcImageData );
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
    this->overlayOpacity = opacity;
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT ))->SetOpacity(opacity);  
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK ))->SetOpacity(opacity);   
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT ))->SetOpacity(opacity) ;
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK ))->SetOpacity(opacity) ;
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT ))->SetOpacity(opacity) ;
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK ))->SetOpacity(opacity) ;
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
    if( dataVector[MRS] != NULL ) {
        this->ClipMapperToTlcBrc( dataVector[MRS], 
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

        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT )
                               )->SetDisplayExtent( axialExtent );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK )
                               )->SetDisplayExtent( axialExtent );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT )
                               )->SetDisplayExtent( coronalExtent );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK )
                               )->SetDisplayExtent( coronalExtent );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT )
                               )->SetDisplayExtent( sagittalExtent );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK )
                               )->SetDisplayExtent( sagittalExtent );

        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT )
                             )->SetUserTransform( transformFrontAxial );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK )
                             )->SetUserTransform( transformBackAxial );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT )
                             )->SetUserTransform( transformFrontCoronal );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK )
                             )->SetUserTransform( transformBackCoronal );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT )
                             )->SetUserTransform( transformFrontSagittal );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK )
                             )->SetUserTransform( transformBackSagittal );

        transformFrontAxial->Delete();
        transformBackAxial->Delete();
        transformFrontCoronal->Delete();
        transformBackCoronal->Delete();
        transformFrontSagittal->Delete();
        transformBackSagittal->Delete();



    }  
}


/*!
 *  Sets up the overlay actor.
 */
void svkOverlayView::SetupOverlay()
{
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

    // We need to kill the old actors, window levelers, and the color transfer otherwise update cause problems
    // (seg faults in vtkLookupTable) when switching from a larger dataset to a smaller dataset
    this->GetRenderer( svkOverlayView::PRIMARY )->RemoveViewProp( this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT ) );
    svkOpenGLOrientedImageActor* overlayActor = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkOverlayView::AXIAL_OVERLAY_FRONT, overlayActor );
    overlayActor->Delete();

    this->GetRenderer( svkOverlayView::PRIMARY )->RemoveViewProp( this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK ) );
    svkOpenGLOrientedImageActor* overlayActorBack = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkOverlayView::AXIAL_OVERLAY_BACK, overlayActorBack );
    overlayActorBack->Delete();

    this->GetRenderer( svkOverlayView::PRIMARY )->RemoveViewProp( this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT ) );
    overlayActor = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkOverlayView::CORONAL_OVERLAY_FRONT, overlayActor );
    overlayActor->Delete();

    this->GetRenderer( svkOverlayView::PRIMARY )->RemoveViewProp( this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK ) );
    overlayActorBack = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkOverlayView::CORONAL_OVERLAY_BACK, overlayActorBack );
    overlayActorBack->Delete();

    this->GetRenderer( svkOverlayView::PRIMARY )->RemoveViewProp( this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT ) );
    overlayActor = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT, overlayActor );
    overlayActor->Delete();

    this->GetRenderer( svkOverlayView::PRIMARY )->RemoveViewProp( this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK ) );
    overlayActorBack = svkOpenGLOrientedImageActor::New();
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

    this->SetLUT( svkLookupTable::COLOR); 
   
    this->windowLevelerAxial->SetInput( dataVector[OVERLAY] );
    this->windowLevelerAxial->SetOutputFormatToRGBA();
    this->windowLevelerAxial->Update();
    this->windowLevelerCoronal->SetInput( dataVector[OVERLAY] );
    this->windowLevelerCoronal->SetOutputFormatToRGBA();
    this->windowLevelerCoronal->Update();
    this->windowLevelerSagittal->SetInput( dataVector[OVERLAY] );
    this->windowLevelerSagittal->SetOutputFormatToRGBA();
    this->windowLevelerSagittal->Update();

    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT )
                                   )->SetInput( this->windowLevelerAxial->GetOutput() );
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK )
                                   )->SetInput( this->windowLevelerAxial->GetOutput() );
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT )
                                   )->SetInput( this->windowLevelerCoronal->GetOutput() );
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK )
                                   )->SetInput( this->windowLevelerCoronal->GetOutput() );
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT )
                                   )->SetInput( this->windowLevelerSagittal->GetOutput() );
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK )
                                   )->SetInput( this->windowLevelerSagittal->GetOutput() );

    this->SetInterpolationType( this->interpolationType );
    this->SetOverlayOpacity( 0.35 );

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
                                    )->SetPosition(0.02,0.27);
    vtkScalarBarActor::SafeDownCast(this->GetProp( svkOverlayView::COLOR_BAR )
                                    )->SetPosition2(0.15,0.8);

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
 *  Sets the type of interpolation for the overlayed image.
 *
 *  \param interpolationType options are NEAREST, LINEAR, or SINC
 */
void svkOverlayView::SetInterpolationType( int interpolationType )
{
    if (interpolationType == NEAREST ) {
        this->interpolationType = NEAREST; 
        // Check to see if the current overlay is interpolated already
        if( interpOverlay == dataVector[OVERLAY] ) {
            this->SetInput( svkMriImageData::SafeDownCast(sincInterpolation->GetInput()), OVERLAY );
        }
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT ))->InterpolateOff();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK ))->InterpolateOff();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT ))->InterpolateOff();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK ))->InterpolateOff();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT ))->InterpolateOff();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK ))->InterpolateOff();
    } else if (interpolationType == LINEAR) {
        this->interpolationType = LINEAR; 
        // Check to see if the current overlay is interpolated already
        if( interpOverlay == dataVector[OVERLAY] ) {
            this->SetInput( svkMriImageData::SafeDownCast(sincInterpolation->GetInput()), OVERLAY );
        }
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT ))->InterpolateOn();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK ))->InterpolateOn();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT ))->InterpolateOn();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK ))->InterpolateOn();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT ))->InterpolateOn();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK ))->InterpolateOn();
    } else if (interpolationType == SINC) {
        this->interpolationType = SINC; 
        if( interpOverlay != dataVector[OVERLAY] ) {

            int* extent = dataVector[OVERLAY]->GetExtent(); 
            int xLength = extent[1]-extent[0] + 1;
            int yLength = extent[3]-extent[2] + 1;
            int zLength = extent[5]-extent[4] + 1;

            double* imageSpacing = dataVector[MRI]->GetSpacing();
            double* overlaySpacing = dataVector[OVERLAY]->GetSpacing();

            // Lets choose the resolution of the sinc by trying to get to the resolution of the image
            int xSize = (int)pow( 2., vtkMath::Round( log( static_cast<double>(xLength * (overlaySpacing[0]/imageSpacing[0]) ))/log(2.) ) );
            int ySize = (int)pow( 2., vtkMath::Round( log( static_cast<double>(yLength * (overlaySpacing[1]/imageSpacing[1]) ))/log(2.) ) );
            int zSize = (int)pow( 2., vtkMath::Round( log( static_cast<double>(zLength * (overlaySpacing[2]/imageSpacing[2]) ))/log(2.) ) );
            if( xSize > SINC_MAX_EXTENT ) {
                xSize = SINC_MAX_EXTENT;
            }
            if( ySize > SINC_MAX_EXTENT ) {
                ySize = SINC_MAX_EXTENT;
            }
            if( zSize > SINC_MAX_EXTENT ) {
                zSize = SINC_MAX_EXTENT;
            }
            sincInterpolation->SetOutputWholeExtent( 0, xSize-1, 0, ySize-1, 0, zSize-1 );
            sincInterpolation->SetInput( dataVector[OVERLAY] );
            sincInterpolation->Update();
            interpOverlay->Delete();
            interpOverlay = svkMriImageData::New();
            interpOverlay->DeepCopy( sincInterpolation->GetOutput() );
            interpOverlay->Update();
            interpOverlay->SyncVTKImageDataToDcmHeader();

            vtkImageExtractComponents* real = vtkImageExtractComponents::New();
            real->SetComponents( 0 );
            real->SetInput( sincInterpolation->GetOutput() );
            real->Update();
            interpOverlay->ShallowCopy( real->GetOutput() );
            interpOverlay->Update();

            real->Delete();
            interpOverlay->SyncVTKImageDataToDcmHeader();

            this->SetInput( interpOverlay, OVERLAY );
            svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT ))->InterpolateOn();
            svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_BACK ))->InterpolateOn();
            svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_FRONT ))->InterpolateOn();
            svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::CORONAL_OVERLAY_BACK ))->InterpolateOn();
            svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_FRONT ))->InterpolateOn();
            svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::SAGITTAL_OVERLAY_BACK ))->InterpolateOn();

        }
    }
    this->Refresh();
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

    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkOverlayView::AXIAL_OVERLAY_FRONT ))->Modified( );    

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
    if ( data == NULL || targetIndex > OVERLAY || targetIndex < 0 ) {

        resultInfo = "Data incompatible-- NULL or outside of input range.\n";

    } else if( data->IsA("svkMriImageData") ) {
        
        if( targetIndex == OVERLAY && dataVector[MRS] == NULL ) {
            resultInfo = "ERROR: Spectra must be loaded before overlays!\n";
        } 

        if( dataVector[MRS] != NULL ) {
            bool valid = validator->AreDataCompatible( data, dataVector[MRS] );
            if( validator->IsInvalid( svkDataValidator::INVALID_DATA_ORIENTATION ) ) {
                resultInfo = "Orientation mismatch: reslicing images to MRS data orientation."; 
                resultInfo = ""; 
            } else if ( !valid ) {
                resultInfo += validator->resultInfo.c_str();
                resultInfo += "\n";
            }
        } 

    } else if( data->IsA("svkMrsImageData") ) {

        if( dataVector[MRI] != NULL ) {
            bool valid = validator->AreDataCompatible( data, dataVector[MRI] ); 
            if( validator->IsInvalid( svkDataValidator::INVALID_DATA_ORIENTATION ) ) {
                resultInfo = "Orientation mismatch: reslicing images to MRS data orientation."; 
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
 *  Reslice data if necessary: 
 *
 * \return the index that was modified. Returns -1 if no reslice was done.
 */
int svkOverlayView::InitReslice( svkImageData* data, int targetIndex )
{
    int indexModified = -1;
    svkDataValidator* validator = svkDataValidator::New();
    
    // Check for null datasets and out of bound data sets...
    if ( data == NULL || targetIndex > OVERLAY || targetIndex < 0 ) {

        return indexModified; 

    } else if( data->IsA("svkMriImageData") ) {
        
        if( dataVector[MRS] != NULL ) {
            bool valid = validator->AreDataCompatible( data, dataVector[MRS] );
            if( validator->IsInvalid( svkDataValidator::INVALID_DATA_ORIENTATION ) ) {
                this->ResliceImage(data, dataVector[MRS], targetIndex); 
                indexModified = targetIndex;
            } 
        } 

    } else if( data->IsA("svkMrsImageData") ) {

        if( dataVector[MRI] != NULL ) {
            bool valid = validator->AreDataCompatible( data, dataVector[MRI] ); 
            if( validator->IsInvalid( svkDataValidator::INVALID_DATA_ORIENTATION ) ) {
                this->ResliceImage(dataVector[MRI], data, MRI); 
                indexModified = MRI;
            } 
        } 

    } 
    validator->Delete(); 

    return indexModified; 
}


/*!
 *  Reslice images to MRS orientation
 */
void svkOverlayView::ResliceImage(svkImageData* input, svkImageData* target, int targetIndex)    
{
cout << "RESLICE IMAGE" << endl;
cout << "RESLICE IMAGE" << endl;
cout << "RESLICE IMAGE" << endl;
cout << "RESLICE IMAGE" << endl;
cout << "RESLICE IMAGE" << endl;
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
        if( this->dataVector[MRS] != NULL ) {
            svkDataView::ResetTlcBrcForNewOrientation( this->dataVector[MRS], this->orientation, this->tlcBrc, this->slice );
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
void svkOverlayView::ToggleSelBoxVisibilityOn() 
{
    this->toggleSelBoxVisibility = true;
    if( this->GetProp( svkOverlayView::VOL_SELECTION ) == NULL ) {
        return;
    }
    if( this->dataVector[MRS] != NULL && static_cast<svkMrsImageData*>(this->dataVector[MRS])->IsSliceInSelectionBox( this->slice, this->orientation ) ) {
        this->GetProp( svkOverlayView::VOL_SELECTION )->SetVisibility(1);
        this->TurnPropOn( svkOverlayView::VOL_SELECTION );
    } else {
        vtkProp* volSelection = this->GetProp( svkOverlayView::VOL_SELECTION ); 
        if( volSelection != NULL ) {
            this->GetProp( svkOverlayView::VOL_SELECTION )->SetVisibility(0);
        }
    }
}


/*!
 *
 */
void svkOverlayView::ToggleSelBoxVisibilityOff() 
{
    this->toggleSelBoxVisibility = false;
    vtkProp* volSelection = this->GetProp( svkOverlayView::VOL_SELECTION ); 
    if( volSelection != NULL ) {
        this->TurnPropOn( svkOverlayView::VOL_SELECTION );
        this->GetProp( svkOverlayView::VOL_SELECTION )->SetVisibility(1);
    }
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
    this->imageViewer->ResetCamera();
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
 *  Returns true if the current image is within the spectrscopy data set
 */
bool svkOverlayView::IsImageInsideSpectra() 
{
    return imageInsideSpectra;
}
