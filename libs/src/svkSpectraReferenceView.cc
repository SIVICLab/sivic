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

#define DEBUG 0 

#include <svkSpectraReferenceView.h>


using namespace svk;


//vtkCxxRevisionMacro(svkSpectraReferenceView, "$Rev$");
vtkStandardNewMacro(svkSpectraReferenceView);


/*!
 *  Constructor creates the vtkImageView2, and sets its size.
 */
svkSpectraReferenceView::svkSpectraReferenceView()
{

    this->plotGrid = svkPlotLineGrid::New();
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


    // Create a state vector for our images

    this->isPropOn.assign(LAST_PROP+1, false);
    this->isRendererOn.assign(LAST_RENDERER+1, false);
    this->isPropVisible.assign(LAST_PROP+1, false);     //Is the actor in the views FOV?
    
    // This fixes a compiler error in OS X, not sure why....
    vtkRenderer* nullRenderer = NULL;
    vtkProp* nullProp = NULL;
    this->renCollection.assign(svkSpectraReferenceView::LAST_RENDERER+1, nullRenderer);     //Is the actor in the views FOV?
    this->propCollection.assign(svkSpectraReferenceView::LAST_PROP+1, nullProp);     //Is the actor in the views FOV?

    vtkActor* entirePlotGrid = vtkActor::New();
    this->SetProp( svkSpectraReferenceView::PLOT_GRID, entirePlotGrid );
    entirePlotGrid->Delete();

    this->SetProp( svkSpectraReferenceView::VOL_SELECTION, NULL );
    this->SetProp( svkSpectraReferenceView::PLOT_LINES, this->plotGrid->GetPlotGridActor()  );

    svkOpenGLOrientedImageActor* overlayActor = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkSpectraReferenceView::AXIAL_OVERLAY_FRONT, overlayActor );
    overlayActor->Delete();

    svkOpenGLOrientedImageActor* overlayActorBack = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkSpectraReferenceView::AXIAL_OVERLAY_BACK, overlayActorBack );
    overlayActorBack->Delete();

    overlayActor = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkSpectraReferenceView::CORONAL_OVERLAY_FRONT, overlayActor );
    overlayActor->Delete();

    overlayActorBack = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkSpectraReferenceView::CORONAL_OVERLAY_BACK, overlayActorBack );
    overlayActorBack->Delete();

    overlayActor = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_FRONT, overlayActor );
    overlayActor->Delete();

    overlayActorBack = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_BACK, overlayActorBack );
    overlayActorBack->Delete();

    vtkScalarBarActor* bar = vtkScalarBarActor::New();
    this->SetProp( svkSpectraReferenceView::COLOR_BAR, bar );
    bar->Delete();

    this->SetProp( svkSpectraReferenceView::SAT_BANDS_AXIAL, this->satBandsAxial->GetSatBandsActor() );
    this->TurnPropOff( svkSpectraReferenceView::SAT_BANDS_AXIAL );

    this->SetProp( svkSpectraReferenceView::SAT_BANDS_AXIAL_OUTLINE, this->satBandsAxial->GetSatBandsOutlineActor() );
    this->TurnPropOff( svkSpectraReferenceView::SAT_BANDS_AXIAL_OUTLINE );

    this->SetProp( svkSpectraReferenceView::SAT_BANDS_CORONAL_OUTLINE, this->satBandsCoronal->GetSatBandsOutlineActor() );
    this->TurnPropOff( svkSpectraReferenceView::SAT_BANDS_CORONAL_OUTLINE );

    this->SetProp( svkSpectraReferenceView::SAT_BANDS_SAGITTAL_OUTLINE, this->satBandsSagittal->GetSatBandsOutlineActor() );
    this->TurnPropOff( svkSpectraReferenceView::SAT_BANDS_SAGITTAL_OUTLINE );

    this->satBandsAxial->SetOrientation( svkDcmHeader::AXIAL );
    this->satBandsCoronal->SetOrientation( svkDcmHeader::CORONAL );
    this->satBandsSagittal->SetOrientation( svkDcmHeader::SAGITTAL );

    this->SetProp( svkSpectraReferenceView::SAT_BANDS_CORONAL, this->satBandsCoronal->GetSatBandsActor() );
    this->TurnPropOff( svkSpectraReferenceView::SAT_BANDS_CORONAL );

    this->SetProp( svkSpectraReferenceView::SAT_BANDS_SAGITTAL, this->satBandsSagittal->GetSatBandsActor() );
    this->TurnPropOff( svkSpectraReferenceView::SAT_BANDS_SAGITTAL );

    
    this->interpolationType = NEAREST; 
}


/*!
 *  Destructor deletes the vtkImageViewer2.
 */
svkSpectraReferenceView::~svkSpectraReferenceView()
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
    if( this->plotGrid != NULL ) {
        this->plotGrid->Delete();
        this->plotGrid = NULL;
    }


}


/*!
 *   Sets the input of the vtkImageViewer2. It also resets the camera view and 
 *   the slice.
 *
 *   \param resetViewState boolean identifies of this is the first dataset input
 */        
void svkSpectraReferenceView::SetupMsInput( bool resetViewState ) 
{

    int toggleDraw = this->GetRenderer( svkSpectraReferenceView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->GetRenderer( svkSpectraReferenceView::PRIMARY)->DrawOff();
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
    vtkActor::SafeDownCast( this->GetProp( svkSpectraReferenceView::PLOT_GRID))->SetMapper( entireGridMapper );
    entireGridMapper->Delete();
    //vtkActor::SafeDownCast( GetProp( svkSpectraReferenceView::PLOT_GRID) )->GetProperty()->SetDiffuseColor( 0, 1, 0 );
    vtkActor::SafeDownCast( GetProp( svkSpectraReferenceView::PLOT_GRID) )->GetProperty()->SetDiffuseColor( 0, 0, 0 );

    // Now we need to grab the selection box
    vtkActorCollection* selectionTopo = dataVector[MRS]->GetTopoActorCollection( 1 );

    // Case for no selection box
    if( selectionTopo != NULL ) {
        selectionTopo->InitTraversal();
        if( this->GetRenderer( svkSpectraReferenceView::PRIMARY)->HasViewProp( this->GetProp( svkSpectraReferenceView::VOL_SELECTION) ) ) {
            this->GetRenderer( svkSpectraReferenceView::PRIMARY)->RemoveActor( this->GetProp( svkSpectraReferenceView::VOL_SELECTION) );
        }
        this->SetProp( svkSpectraReferenceView::VOL_SELECTION, selectionTopo->GetNextActor());     
        this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( this->GetProp( svkSpectraReferenceView::VOL_SELECTION) );
        this->TurnPropOn( svkSpectraReferenceView::VOL_SELECTION );
        selectionTopo->Delete();
    }
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( this->GetProp( svkSpectraReferenceView::PLOT_GRID ) );
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( this->GetProp( svkSpectraReferenceView::SAT_BANDS_AXIAL) );
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( this->GetProp( svkSpectraReferenceView::SAT_BANDS_AXIAL_OUTLINE) );
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( this->GetProp( svkSpectraReferenceView::SAT_BANDS_CORONAL) );
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( this->GetProp( svkSpectraReferenceView::SAT_BANDS_CORONAL_OUTLINE) );
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( this->GetProp( svkSpectraReferenceView::SAT_BANDS_SAGITTAL) );
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( this->GetProp( svkSpectraReferenceView::SAT_BANDS_SAGITTAL_OUTLINE) );

   
    this->plotGrid->SetInput(svkMrsImageData::SafeDownCast(dataVector[MRS]));
    this->SetProp( svkSpectraReferenceView::PLOT_LINES, this->plotGrid->GetPlotGridActor()  );
    this->TurnPropOn( PLOT_LINES );
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( this->GetProp( svkSpectraReferenceView::PLOT_LINES ) );



    this->SetProp( svkSpectraReferenceView::PLOT_GRID, this->GetProp( svkSpectraReferenceView::PLOT_GRID ) );
    string acquisitionType = dataVector[MRS]->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
    if( acquisitionType != "SINGLE VOXEL" ) {
        this->TurnPropOn( svkSpectraReferenceView::PLOT_GRID );
    } else {
        this->TurnPropOff( svkSpectraReferenceView::PLOT_GRID );
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
        this->GetRenderer( svkSpectraReferenceView::PRIMARY)->DrawOn();
    }

}


/*!
 *  Sets the input of the vtkImageViewer2. It also resets the camera view and 
 *  the slice. This should be modified once the new image loader is written.
 *
 *   \param resetViewState boolean identifies of this is the first dataset input
 *
 */        
void svkSpectraReferenceView::SetupMrInput( bool resetViewState )
{
    // Rendering is set off to avoid uncontrolled rendering
    double cameraPosition[3];
    double cameraViewUp[3];
    double cameraFocus[3];
    
    int toggleDraw = this->GetRenderer( svkSpectraReferenceView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->GetRenderer(svkSpectraReferenceView::PRIMARY)->DrawOff();
    }

    // Do we want to reset the camera?
    if( !resetViewState ) {
        memcpy( cameraPosition, 
                this->GetRenderer(svkSpectraReferenceView::PRIMARY)->GetActiveCamera()->GetPosition(), 
                sizeof(double)*3);
        memcpy(cameraViewUp, 
                this->GetRenderer(svkSpectraReferenceView::PRIMARY)->GetActiveCamera()->GetViewUp(), 
                sizeof(double)*3);
        memcpy(cameraFocus, 
                this->GetRenderer(svkSpectraReferenceView::PRIMARY)->GetActiveCamera()->GetFocalPoint(), 
                sizeof(double)*3);
    }
    
    // We need to execute these before resolving the state
    imageViewer->SetInput( dataVector[MRI] );
    imageViewer->SetSlice( slice );
    
    if( resetViewState ) {
        cout << * this->GetRenderer( svkSpectraReferenceView::PRIMARY ) << endl;
        this->imageViewer->SetRenderer( this->GetRenderer( svkSpectraReferenceView::PRIMARY ) ); 
        this->imageViewer->GetRenderer()->SetBackground(0.0,0.0,0.0);
        this->AlignCamera( ); 
        imageViewer->GetRenderer()->GetActiveCamera()->SetParallelProjection(1);
        imageViewer->GetImageActor()->PickableOff();
        this->ResetWindowLevel();
    }
 
    int* extent = this->dataVector[MRI]->GetExtent();
    if( dataVector[MRS] == NULL ) {
        this->SetSlice( (extent[5]-extent[4])/2 );
    } 

    this->satBandsAxial->SetReferenceImage( static_cast<svkMriImageData*>(this->dataVector[MRI]) );
    this->satBandsCoronal->SetReferenceImage( static_cast<svkMriImageData*>(this->dataVector[MRI]) );
    this->satBandsSagittal->SetReferenceImage( static_cast<svkMriImageData*>(this->dataVector[MRI]) );

    // And here we return the camera to its original state
    if( !resetViewState ) {
        this->GetRenderer( svkSpectraReferenceView::PRIMARY )->GetActiveCamera()->SetPosition( cameraPosition ); 
        this->GetRenderer( svkSpectraReferenceView::PRIMARY )->GetActiveCamera()->SetViewUp( cameraViewUp ); 
        this->GetRenderer( svkSpectraReferenceView::PRIMARY )->GetActiveCamera()->SetFocalPoint( cameraFocus ); 
    } else {
        this->AlignCamera();
    }
    
    
    rwi->Render();
    if( toggleDraw ) {
        this->GetRenderer(svkSpectraReferenceView::PRIMARY)->DrawOn();
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
void svkSpectraReferenceView::SetInput(svkImageData* data, int index)
{
    bool resetViewState = 1;
    // Check data compatiblity
    string resultInfo = this->GetDataCompatibility( data, index );
    if( strcmp( resultInfo.c_str(), "" ) == 0 ) { 
    
        if( dataVector[index] != NULL ) {
            //if( dataVector[MRI] != NULL && dataVector[MRS] != NULL ) {
                resetViewState = 0; 
            //}
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
 *   Sets the current slice.
 *
 *   \param slice the slice you want to view
 */
void svkSpectraReferenceView::SetSlice(int slice)
{
    bool centerImage = true;
    this->SetSlice( slice, centerImage );
}

/*!
 *   Sets the current slice and centers the image to the voxel.
 *
 *   \param slice the slice you want to view
 */
void svkSpectraReferenceView::SetSlice(int slice, bool centerImage)
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
                this->plotGrid->SetSlice(slice);
                this->plotGrid->SetTlcBrc(tlcBrc);
                this->plotGrid->Update(tlcBrc);
            }
            this->slice = slice;
            this->GenerateClippingPlanes();

            // Case for no selection box
            if( this->GetProp( svkSpectraReferenceView::VOL_SELECTION ) != NULL ) {

                // If it is make it visible, otherwise hide it
                if( static_cast<svkMrsImageData*>(this->dataVector[MRS])->IsSliceInSelectionBox( this->slice, this->orientation ) && isPropOn[VOL_SELECTION] && this->toggleSelBoxVisibility) {
                    this->GetProp( svkSpectraReferenceView::VOL_SELECTION )->SetVisibility(1);
                } else if( this->toggleSelBoxVisibility ) {
                    this->GetProp( svkSpectraReferenceView::VOL_SELECTION )->SetVisibility(0);
                }

            }
            int toggleDraw = this->GetRenderer( svkSpectraReferenceView::PRIMARY )->GetDraw();
            if( toggleDraw ) {
                this->GetRenderer( svkSpectraReferenceView::PRIMARY)->DrawOff();
            }
            this->UpdateImageSlice( centerImage );
            this->SetSliceOverlay();

                
            if( toggleDraw ) {
                this->GetRenderer( svkSpectraReferenceView::PRIMARY)->DrawOn();
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
void svkSpectraReferenceView::SetSlice(int slice, svkDcmHeader::Orientation orientation)
{
    int toggleDraw = this->GetRenderer( svkSpectraReferenceView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->GetRenderer( svkSpectraReferenceView::PRIMARY)->DrawOff();
    }
    if( this->dataVector[MRS] != NULL && orientation == this->orientation ) {

        // We may have removed the plot grid, so lets make sure its present
        if( !this->GetRenderer(svkSpectraReferenceView::PRIMARY)->HasViewProp( this->GetProp( svkSpectraReferenceView::PLOT_GRID ) )) {
            this->GetRenderer(svkSpectraReferenceView::PRIMARY)->AddViewProp(this->GetProp( svkSpectraReferenceView::PLOT_GRID ));
        }
        int newSpectraSlice = this->FindSpectraSlice( slice, orientation );
        if( static_cast<svkMrsImageData*>(this->dataVector[MRS])->IsSliceInSelectionBox( newSpectraSlice, orientation )                       && isPropOn[VOL_SELECTION] 
                 && this->toggleSelBoxVisibility) {
            this->GetProp( svkSpectraReferenceView::VOL_SELECTION )->SetVisibility(1);
        } else if( this->toggleSelBoxVisibility ) {
            this->GetProp( svkSpectraReferenceView::VOL_SELECTION )->SetVisibility(0);
        }
        if(  newSpectraSlice >= this->dataVector[MRS]->GetFirstSlice( this->orientation ) &&
             newSpectraSlice <=  this->dataVector[MRS]->GetLastSlice( this->orientation ) ) {

            if( newSpectraSlice != this->slice ) {
                this->SetSlice( newSpectraSlice );    
            }
            this->imageInsideSpectra = true;
        } else {
            if( this->GetRenderer(svkSpectraReferenceView::PRIMARY)->HasViewProp( this->GetProp( svkSpectraReferenceView::PLOT_GRID ) )) {
                this->GetRenderer(svkSpectraReferenceView::PRIMARY)->RemoveViewProp(this->GetProp( svkSpectraReferenceView::PLOT_GRID ));
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
        this->GetRenderer( svkSpectraReferenceView::PRIMARY)->DrawOn();
    }
    this->Refresh();

}


/*
 *  Finds the image slice that most closely corresponds to the input spectra slice.
 */
int svkSpectraReferenceView::FindCenterImageSlice( int spectraSlice, svkDcmHeader::Orientation orientation ) 
{
    int imageSlice;
    double spectraSliceCenter[3];
    this->dataVector[MRS]->GetSliceCenter( spectraSlice, spectraSliceCenter, orientation );
    double normal[3];
    this->dataVector[MRS]->GetSliceNormal( normal, orientation );

    imageSlice = this->dataVector[MRI]->GetClosestSlice( spectraSliceCenter, orientation );
    return imageSlice;
}


/*
 *  Finds the spectra slice that most closely corresponds to the input image slice.
 */
int svkSpectraReferenceView::FindSpectraSlice( int imageSlice, svkDcmHeader::Orientation orientation ) 
{
    int spectraSlice;
    double imageSliceCenter[3];
    this->dataVector[MRI]->GetSliceOrigin( imageSlice, imageSliceCenter, orientation );
    spectraSlice = this->dataVector[MRS]->GetClosestSlice( imageSliceCenter, orientation );
    return spectraSlice;
}


/*!
 *  Sets the slice of the anatamical data, based on the spectroscopic slice.
 *  It calculates the anatomical slice closest to the center of the spectroscopic
 *  slice.
 *
 */
void svkSpectraReferenceView::UpdateImageSlice( bool centerImage )
{
    int imageSlice = this->imageViewer->GetSlice( this->orientation );
    if( centerImage ) {
        imageSlice = FindCenterImageSlice(this->slice, this->orientation);
    }
    int* imageExtent = dataVector[MRI]->GetExtent();
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
    }
    switch ( this->orientation ) {
        case svkDcmHeader::AXIAL:
            this->satBandsAxial->SetClipSlice( imageSlice );
            break;
        case svkDcmHeader::CORONAL:
            this->satBandsCoronal->SetClipSlice( imageSlice );
            break;
        case svkDcmHeader::SAGITTAL:
            this->satBandsSagittal->SetClipSlice( imageSlice );
            break;
    }
}


/*!
 *  Sets the vtkRenderWindowInteractor to be associated with this view.
 *
 *  \param rwi the vtkRenderWindowInteractor you wish this view to use
 */  
void svkSpectraReferenceView::SetRWInteractor( vtkRenderWindowInteractor* rwi )
{
    if( rwi != NULL ) {
        if( this->rwi != NULL ) {
            this->rwi->Delete();
        }
        this->rwi = rwi;
        this->rwi->Register( this );
        this->myRenderWindow = this->rwi->GetRenderWindow();
        this->myRenderWindow->AddRenderer( this->GetRenderer(svkSpectraReferenceView::PRIMARY) );
        this->TurnRendererOn( svkSpectraReferenceView::PRIMARY );
        this->imageViewer->SetupInteractor( rwi );
        this->imageViewer->SetRenderWindow( myRenderWindow ); 
        cout << * this->GetRenderer( svkSpectraReferenceView::PRIMARY ) << endl;
    }
}  


/*!
 *  Sets desired the current selection in Display (pixels) coordinates
 *  and highlights the intersected voxels.
 *
 *  \param selectionArea the area you wish to select voxels within [xmin, xmax, ymin, ymax]
 */ 
void svkSpectraReferenceView::SetSelection( double* selectionArea, bool isWorldCords )
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
            worldStart[0] = *coordStart->GetComputedWorldValue(this->GetRenderer( svkSpectraReferenceView::PRIMARY)); 
            worldStart[1] = *(coordStart->GetComputedWorldValue(this->GetRenderer( svkSpectraReferenceView::PRIMARY)) + 1); 
            worldStart[2] = *(coordStart->GetComputedWorldValue(this->GetRenderer( svkSpectraReferenceView::PRIMARY)) + 2); 
            worldEnd[0] = *coordEnd->GetComputedWorldValue(this->GetRenderer( svkSpectraReferenceView::PRIMARY)); 
            worldEnd[1] = *(coordEnd->GetComputedWorldValue(this->GetRenderer( svkSpectraReferenceView::PRIMARY)) + 1); 
            worldEnd[2] = *(coordEnd->GetComputedWorldValue(this->GetRenderer( svkSpectraReferenceView::PRIMARY)) + 2); 
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
void svkSpectraReferenceView::SetTlcBrc( int* tlcBrc ) 
{
    if( svkDataView::IsTlcBrcWithinData( this->dataVector[MRS], tlcBrc ) ) {
        this->tlcBrc[0] = tlcBrc[0];
        this->tlcBrc[1] = tlcBrc[1];
        this->GenerateClippingPlanes();
        this->plotGrid->SetTlcBrc(this->tlcBrc);
        this->plotGrid->Update(tlcBrc);

    }
}

/*!
 *  SetWindowLevel for spectral view;  index 0 is frequency, index 1 is intensity.
 *  NOTE: Method assumes that frequency ranges are in integers (points).
 *
 *  \param lower the lower limit
 *  \param upper the upper limit
 *  \param index which dimension you wish to change, frequency or magnitude 
 *
 */
void svkSpectraReferenceView::SetWindowLevelRange( double lower, double upper, int index)
{
    if (index == FREQUENCY) {
        this->plotGrid->SetFrequencyWLRange(static_cast<int>(lower), static_cast<int>(upper), this->tlcBrc);
    } else if (index == AMPLITUDE) {
        this->plotGrid->SetIntensityWLRange(lower, upper, this->tlcBrc);
    }
    this->Refresh();
}


/*!
 *
 */
void svkSpectraReferenceView::GetWindowLevelRange( double &lower, double &upper, int index)
{
    if (index == FREQUENCY) {
        int integerLower;
        int integerUpper;
        this->plotGrid->GetFrequencyWLRange( integerLower, integerUpper );
        lower = integerLower;
        upper = integerUpper;
    } else if (index == AMPLITUDE) {
        this->plotGrid->GetIntensityWLRange(lower, upper);
    }
}


/*
 *  Set the component to display: 
 *
 *  \param component the compent you wish to display, REAL, IMAGINARY, MAGNITUDE
 */
void svkSpectraReferenceView::SetComponent( svkPlotLine::PlotComponent component)
{
    this->plotGrid->SetComponent( component );
    this->Refresh();
}


/*! 
 *  Method highlights voxels within the selection box
 *
 *  \return tlcBrc the cell id's of the desired top left, bottom right corners
 */
int* svkSpectraReferenceView::HighlightSelectionVoxels()
{
    if( dataVector[MRS] != NULL ) { 
        int tlcBrcImageData[2];
        svkMrsImageData::SafeDownCast(this->dataVector[MRS])->Get2DProjectedTlcBrcInSelectionBox( tlcBrcImageData, this->orientation, this->slice );
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
void svkSpectraReferenceView::SetOverlayOpacity( double opacity ) 
{
    this->overlayOpacity = opacity;
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_FRONT ))->SetOpacity(opacity);  
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_BACK ))->SetOpacity(opacity);   
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_FRONT ))->SetOpacity(opacity) ;
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_BACK ))->SetOpacity(opacity) ;
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_FRONT ))->SetOpacity(opacity) ;
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_BACK ))->SetOpacity(opacity) ;
}


/*!
 *  Gets the opacity of the image overlay.
 *
 */ 
double svkSpectraReferenceView::GetOverlayOpacity( ) 
{
    return this->overlayOpacity;
}


/*!
 *  Sets the threshold of the image overlay.
 *
 *   \param threshold the new threshold you wish the image overlay to have. 
 */ 
void svkSpectraReferenceView::SetOverlayThreshold( double threshold ) 
{
    this->overlayThreshold = threshold;
    if( this->colorTransfer != NULL ) {
        this->colorTransfer->SetAlphaThreshold(threshold); 
        this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_FRONT )->Modified();
    } 
}


/*!
 *  Gets the opacity of the image overlay.
 *
 */ 
double svkSpectraReferenceView::GetOverlayThreshold( ) 
{
    return this->overlayThreshold;
}


/*!
 * Generates the clipping planes for the mMMapper. This is how the boundries
 * set are enforced, after the data is scaled, it is clipped so that data
 * outside the plot range is simply not shown.
 */
void svkSpectraReferenceView::GenerateClippingPlanes( )
{
    // We need to leave a little room around the edges, so the border does not get cut off
    if( dataVector[MRS] != NULL ) {
        this->ClipMapperToTlcBrc( dataVector[MRS], 
                                 vtkActor::SafeDownCast( this->GetProp( svkSpectraReferenceView::PLOT_GRID ))->GetMapper(), 
                                 tlcBrc, CLIP_TOLERANCE, CLIP_TOLERANCE, CLIP_TOLERANCE );
        this->ClipMapperToTlcBrc( this->dataVector[MRS], this->plotGrid->GetPlotGridActor()->GetMapper(),
                                 tlcBrc, 0, 0, 0 );

    }
}


/*!
 *  Method sets the slice of the overlay to the closests slice of the spectra.
 */
void svkSpectraReferenceView::SetSliceOverlay() {

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
        if(    imageExtent[0] == overlayExtent[0] && imageExtent[1] == overlayExtent[1]  
            && imageExtent[2] == overlayExtent[2] && imageExtent[3] == overlayExtent[3]  
            && imageExtent[4] == overlayExtent[4] && imageExtent[5] == overlayExtent[5] ) {
            overlaySliceAxial =   imageViewer->axialSlice; 
            overlaySliceCoronal = imageViewer->coronalSlice; 
            overlaySliceSagittal =imageViewer->sagittalSlice; 
              
        } else {
            overlaySliceAxial =   this->FindSpectraSlice( imageViewer->axialSlice, svkDcmHeader::AXIAL); 
            overlaySliceCoronal = this->FindSpectraSlice( imageViewer->coronalSlice, svkDcmHeader::CORONAL); 
            overlaySliceSagittal =this->FindSpectraSlice( imageViewer->sagittalSlice, svkDcmHeader::SAGITTAL); 
        }

        if( this->IsPropOn( svkSpectraReferenceView::AXIAL_OVERLAY_FRONT ) ) {
            if( overlaySliceAxial > axialRange[1] ) {
                overlaySliceAxial = axialRange[1];
                this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_FRONT )->VisibilityOff();
                this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_BACK )->VisibilityOff();
            } else if ( overlaySliceAxial < axialRange[0] ) {
                overlaySliceAxial = axialRange[0];
                this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_FRONT )->VisibilityOff();
                this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_BACK )->VisibilityOff();
            } else {
                this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_FRONT )->VisibilityOn();
                this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_BACK )->VisibilityOn();
            }
        }

        if( this->IsPropOn( svkSpectraReferenceView::CORONAL_OVERLAY_FRONT ) ) {
            if( overlaySliceCoronal > coronalRange[1] ) {
                overlaySliceCoronal = coronalRange[1];
                this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_FRONT )->VisibilityOff();
                this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_BACK )->VisibilityOff();
            } else if ( overlaySliceCoronal < coronalRange[0] ) {
                overlaySliceCoronal = coronalRange[0];
                this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_FRONT )->VisibilityOff();
                this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_BACK )->VisibilityOff();
            } else {
                this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_FRONT )->VisibilityOn();
                this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_BACK )->VisibilityOn();
            }
        }

        if( this->IsPropOn( svkSpectraReferenceView::SAGITTAL_OVERLAY_FRONT ) ) {
            if( overlaySliceSagittal > sagittalRange[1] ) {
                overlaySliceSagittal = sagittalRange[1];
                this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_FRONT )->VisibilityOff();
                this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_BACK )->VisibilityOff();
            } else if ( overlaySliceSagittal < sagittalRange[0] ) {
                overlaySliceSagittal = sagittalRange[0];
                this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_FRONT )->VisibilityOff();
                this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_BACK )->VisibilityOff();
            } else {
                this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_FRONT )->VisibilityOn();
                this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_BACK )->VisibilityOn();
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
        // This is why 0.01 is added, just some small fraction so they don't overlap. 
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

        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_FRONT )
                               )->SetDisplayExtent( axialExtent );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_BACK )
                               )->SetDisplayExtent( axialExtent );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_FRONT )
                               )->SetDisplayExtent( coronalExtent );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_BACK )
                               )->SetDisplayExtent( coronalExtent );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_FRONT )
                               )->SetDisplayExtent( sagittalExtent );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_BACK )
                               )->SetDisplayExtent( sagittalExtent );

        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_FRONT )
                             )->SetUserTransform( transformFrontAxial );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_BACK )
                             )->SetUserTransform( transformBackAxial );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_FRONT )
                             )->SetUserTransform( transformFrontCoronal );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_BACK )
                             )->SetUserTransform( transformBackCoronal );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_FRONT )
                             )->SetUserTransform( transformFrontSagittal );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_BACK )
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
void svkSpectraReferenceView::SetupOverlay()
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
    this->GetRenderer( svkSpectraReferenceView::PRIMARY )->RemoveViewProp( this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_FRONT ) );
    svkOpenGLOrientedImageActor* overlayActor = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkSpectraReferenceView::AXIAL_OVERLAY_FRONT, overlayActor );
    overlayActor->Delete();

    this->GetRenderer( svkSpectraReferenceView::PRIMARY )->RemoveViewProp( this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_BACK ) );
    svkOpenGLOrientedImageActor* overlayActorBack = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkSpectraReferenceView::AXIAL_OVERLAY_BACK, overlayActorBack );
    overlayActorBack->Delete();

    this->GetRenderer( svkSpectraReferenceView::PRIMARY )->RemoveViewProp( this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_FRONT ) );
    overlayActor = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkSpectraReferenceView::CORONAL_OVERLAY_FRONT, overlayActor );
    overlayActor->Delete();

    this->GetRenderer( svkSpectraReferenceView::PRIMARY )->RemoveViewProp( this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_BACK ) );
    overlayActorBack = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkSpectraReferenceView::CORONAL_OVERLAY_BACK, overlayActorBack );
    overlayActorBack->Delete();

    this->GetRenderer( svkSpectraReferenceView::PRIMARY )->RemoveViewProp( this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_FRONT ) );
    overlayActor = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_FRONT, overlayActor );
    overlayActor->Delete();

    this->GetRenderer( svkSpectraReferenceView::PRIMARY )->RemoveViewProp( this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_BACK ) );
    overlayActorBack = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_BACK, overlayActorBack );
    overlayActorBack->Delete();
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

    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_FRONT )
                                   )->SetInput( this->windowLevelerAxial->GetOutput() );
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_BACK )
                                   )->SetInput( this->windowLevelerAxial->GetOutput() );
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_FRONT )
                                   )->SetInput( this->windowLevelerCoronal->GetOutput() );
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_BACK )
                                   )->SetInput( this->windowLevelerCoronal->GetOutput() );
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_FRONT )
                                   )->SetInput( this->windowLevelerSagittal->GetOutput() );
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_BACK )
                                   )->SetInput( this->windowLevelerSagittal->GetOutput() );

    this->SetInterpolationType( NEAREST );
    this->SetOverlayOpacity( 0.35 );

    if( !this->GetRenderer( svkSpectraReferenceView::PRIMARY
                   )->HasViewProp( this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_FRONT ) ) ) {

        this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( 
                   this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_FRONT ) );
    }

    if( !this->GetRenderer( svkSpectraReferenceView::PRIMARY
                   )->HasViewProp( this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_BACK ) ) ) {

        this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( 
                   this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_BACK ) );
    }

    if( !this->GetRenderer( svkSpectraReferenceView::PRIMARY
                   )->HasViewProp( this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_FRONT ) ) ) {

        this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( 
                   this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_FRONT ) );
    }

    if( !this->GetRenderer( svkSpectraReferenceView::PRIMARY
                   )->HasViewProp( this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_BACK ) ) ) {

        this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( 
                  this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_BACK ) );
    }

    if( !this->GetRenderer( svkSpectraReferenceView::PRIMARY
                   )->HasViewProp( this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_FRONT ) ) ) {

        this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( 
                  this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_FRONT ) );
    }

    if( !this->GetRenderer( svkSpectraReferenceView::PRIMARY
                   )->HasViewProp( this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_BACK ) ) ) {

        this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( 
                  this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_BACK ) );
    }
    this->TurnPropOn( svkSpectraReferenceView::AXIAL_OVERLAY_FRONT );
    this->TurnPropOn( svkSpectraReferenceView::AXIAL_OVERLAY_BACK );
    this->TurnPropOn( svkSpectraReferenceView::CORONAL_OVERLAY_FRONT );
    this->TurnPropOn( svkSpectraReferenceView::CORONAL_OVERLAY_BACK );
    this->TurnPropOn( svkSpectraReferenceView::SAGITTAL_OVERLAY_FRONT );
    this->TurnPropOn( svkSpectraReferenceView::SAGITTAL_OVERLAY_BACK );


    vtkScalarBarActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::COLOR_BAR )
                                    )->SetTextPositionToPrecedeScalarBar();

    vtkScalarBarActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::COLOR_BAR )
                                    )->UseOpacityOn();

    vtkScalarBarActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::COLOR_BAR )
                                    )->SetNumberOfLabels(3);

    vtkScalarBarActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::COLOR_BAR )
                                    )->GetLabelTextProperty()->ShadowOff();

    vtkScalarBarActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::COLOR_BAR )
                                    )->GetLabelTextProperty()->ItalicOff();

    vtkScalarBarActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::COLOR_BAR )
                                    )->SetPosition(0.02,0.27);
    vtkScalarBarActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::COLOR_BAR )
                                    )->SetPosition2(0.15,0.8);

    if( !this->GetRenderer( svkSpectraReferenceView::PRIMARY
                           )->HasViewProp( this->GetProp( svkSpectraReferenceView::COLOR_BAR) ) ) {

        this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( 
                                this->GetProp( svkSpectraReferenceView::COLOR_BAR) );
    }
    

    this->SetProp( svkSpectraReferenceView::COLOR_BAR, this->GetProp( svkSpectraReferenceView::COLOR_BAR) );
    this->TurnPropOn( svkSpectraReferenceView::COLOR_BAR );
    this->GetProp( svkSpectraReferenceView::COLOR_BAR )->Modified();
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->Render();

    // Now lets make sure we are looking at the appropriate slice.
    this->SetSliceOverlay();
    this->Refresh();
}


/*!
 *  Sets the type of interpolation for the overlayed image.
 *
 *  \param interpolationType options are NEAREST, LINEAR, or SINC
 */
void svkSpectraReferenceView::SetInterpolationType( int interpolationType )
{
    if (interpolationType == NEAREST ) {
        this->interpolationType = NEAREST; 
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_FRONT ))->InterpolateOff();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_BACK ))->InterpolateOff();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_FRONT ))->InterpolateOff();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_BACK ))->InterpolateOff();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_FRONT ))->InterpolateOff();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_BACK ))->InterpolateOff();
    } else if (interpolationType == LINEAR) {
        this->interpolationType = LINEAR; 
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_FRONT ))->InterpolateOn();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_BACK ))->InterpolateOn();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_FRONT ))->InterpolateOn();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::CORONAL_OVERLAY_BACK ))->InterpolateOn();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_FRONT ))->InterpolateOn();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::SAGITTAL_OVERLAY_BACK ))->InterpolateOn();
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
svkLookupTable* svkSpectraReferenceView::GetLookupTable( )
{
    return this->colorTransfer; 
}


/*!
 *  Sets the LUT type 
 *
 *  \param LUT type 
 */
void svkSpectraReferenceView::SetLUT( svkLookupTable::svkLookupTableType type )
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

    this->windowLevelerAxial->SetLookupTable( this->colorTransfer );
    this->windowLevelerCoronal->SetLookupTable( this->colorTransfer );
    this->windowLevelerSagittal->SetLookupTable( this->colorTransfer );

    vtkScalarBarActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::COLOR_BAR )
                                    )->SetLookupTable( this->colorTransfer );

    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkSpectraReferenceView::AXIAL_OVERLAY_FRONT ))->Modified( );    

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
string svkSpectraReferenceView::GetDataCompatibility( svkImageData* data, int targetIndex )
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
                resultInfo = ""; 
                this->ResliceImage(data, dataVector[MRS], targetIndex); 
            } else if ( !valid ) {
                resultInfo += validator->resultInfo.c_str();
                resultInfo += "\n";
            }
        } 

    } else if( data->IsA("svkMrsImageData") ) {
        if( dataVector[MRI] != NULL ) {
            bool valid = validator->AreDataCompatible( data, dataVector[MRI] ); 
            if( validator->IsInvalid( svkDataValidator::INVALID_DATA_ORIENTATION ) ) {
                resultInfo = ""; 
                //resultInfo = "Orientation mismatch: reslicing images to MRS data orientation."; 
                this->ResliceImage(dataVector[MRI], data, targetIndex); 
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
 *  Reslice images to MRS orientation
 */
void svkSpectraReferenceView::ResliceImage(svkImageData* input, svkImageData* target, int targetIndex)    
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
void svkSpectraReferenceView::ResetWindowLevel()
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
void svkSpectraReferenceView::TurnOrthogonalImagesOn()
{
    this->imageViewer->TurnOrthogonalImagesOn();
}


/*
 *  Turns the orthogonal images off.
 */
void svkSpectraReferenceView::TurnOrthogonalImagesOff()
{
    this->imageViewer->TurnOrthogonalImagesOff();
}


/*!     
 *
 */     
void svkSpectraReferenceView::SetOrientation( svkDcmHeader::Orientation orientation )
{
    svkDcmHeader::Orientation oldOrientation;
    oldOrientation = this->orientation;
    this->orientation = orientation;
    if( this->dataVector[MRI] != NULL ) {
        int toggleDraw = this->GetRenderer( svkSpectraReferenceView::PRIMARY )->GetDraw();
        if( toggleDraw ) {
            this->GetRenderer( svkSpectraReferenceView::PRIMARY)->DrawOff();
        }
        this->imageViewer->SetOrientation( orientation );
        //this->AlignCamera();
        if( this->dataVector[MRS] != NULL ) {
            svkDataView::ResetTlcBrcForNewOrientation( this->dataVector[MRS], this->orientation, this->tlcBrc, this->slice );
            this->plotGrid->SetOrientation( this->orientation );

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
                            this->TurnPropOff( svkSpectraReferenceView::SAT_BANDS_AXIAL );
                            break;
                        case svkDcmHeader::CORONAL:
                            this->TurnPropOff( svkSpectraReferenceView::SAT_BANDS_CORONAL );
                            break;
                        case svkDcmHeader::SAGITTAL:
                            this->TurnPropOff( svkSpectraReferenceView::SAT_BANDS_SAGITTAL );
                            break;
                    }
                    switch( this->orientation ) {
                        case svkDcmHeader::AXIAL:
                            this->TurnPropOn( svkSpectraReferenceView::SAT_BANDS_AXIAL );
                            break;
                        case svkDcmHeader::CORONAL:
                            this->TurnPropOn( svkSpectraReferenceView::SAT_BANDS_CORONAL );
                            break;
                        case svkDcmHeader::SAGITTAL:
                            this->TurnPropOn( svkSpectraReferenceView::SAT_BANDS_SAGITTAL );
                            break;
                    }
                }
            }
            if( !this->AreAllSatBandOutlinesOn( oldOrientation ) ) {
                if( this->IsSatBandOutlineForSliceOn( oldOrientation ) ) {
                    switch( oldOrientation ) {
                        case svkDcmHeader::AXIAL:
                            this->TurnPropOff( svkSpectraReferenceView::SAT_BANDS_AXIAL_OUTLINE);
                            break;
                        case svkDcmHeader::CORONAL:
                            this->TurnPropOff( svkSpectraReferenceView::SAT_BANDS_CORONAL_OUTLINE);
                            break;
                        case svkDcmHeader::SAGITTAL:
                            this->TurnPropOff( svkSpectraReferenceView::SAT_BANDS_SAGITTAL_OUTLINE);
                            break;
                    }
                    switch( this->orientation ) {
                        case svkDcmHeader::AXIAL:
                            this->TurnPropOn( svkSpectraReferenceView::SAT_BANDS_AXIAL_OUTLINE);
                            break;
                        case svkDcmHeader::CORONAL:
                            this->TurnPropOn( svkSpectraReferenceView::SAT_BANDS_CORONAL_OUTLINE);
                            break;
                        case svkDcmHeader::SAGITTAL:
                            this->TurnPropOn( svkSpectraReferenceView::SAT_BANDS_SAGITTAL_OUTLINE);
                            break;
                    }
                }
            } 
            this->SetTlcBrc( this->plotGrid->GetTlcBrc() );
            this->SetSlice( this->plotGrid->GetSlice() );

        } else {
            this->SetSlice( this->imageViewer->GetSlice( orientation ), orientation );
        }
        if( toggleDraw ) {
            this->GetRenderer( svkSpectraReferenceView::PRIMARY)->DrawOn();
        }
        this->Refresh();
    }
}


/*!     
 *
 */     
bool svkSpectraReferenceView::IsSatBandForSliceOn( svkDcmHeader::Orientation orientation )
{
    bool satBandsSliceOn = false;
    switch( orientation ) {
        case svkDcmHeader::AXIAL:
            if( this->IsPropOn( svkSpectraReferenceView::SAT_BANDS_AXIAL ) ) {
                satBandsSliceOn = true;
            }
            break;
        case svkDcmHeader::CORONAL:
            if( this->IsPropOn( svkSpectraReferenceView::SAT_BANDS_CORONAL ) ) {
                satBandsSliceOn = true;
            }
            break;
        case svkDcmHeader::SAGITTAL:
            if( this->IsPropOn( svkSpectraReferenceView::SAT_BANDS_SAGITTAL ) ) {
                satBandsSliceOn = true;
            }
            break;
    }
    return satBandsSliceOn;
}


/*!     
 *
 */     
bool svkSpectraReferenceView::IsSatBandOutlineForSliceOn( svkDcmHeader::Orientation orientation )
{
    bool satBandsOutlineSliceOn = false;
    switch( orientation ) {
        case svkDcmHeader::AXIAL:
            if( this->IsPropOn( svkSpectraReferenceView::SAT_BANDS_AXIAL_OUTLINE ) ) {
                satBandsOutlineSliceOn = true;
            }
            break;
        case svkDcmHeader::CORONAL:
            if( this->IsPropOn( svkSpectraReferenceView::SAT_BANDS_CORONAL_OUTLINE ) ) {
                satBandsOutlineSliceOn = true;
            }
            break;
        case svkDcmHeader::SAGITTAL:
            if( this->IsPropOn( svkSpectraReferenceView::SAT_BANDS_SAGITTAL_OUTLINE ) ) {
                satBandsOutlineSliceOn = true;
            }
            break;
    }
    return satBandsOutlineSliceOn;
}


/*!     
 * Are all the sat bands on? If any bands other than the slice is on, we'll say yes
 */     
bool svkSpectraReferenceView::AreAllSatBandsOn( svkDcmHeader::Orientation orientation )
{
    bool satBandsAllOn = false;
    switch( orientation ) {
        case svkDcmHeader::AXIAL:
            if( this->IsPropOn( svkSpectraReferenceView::SAT_BANDS_CORONAL ) ) {
                satBandsAllOn = true;
            }
            if( this->IsPropOn( svkSpectraReferenceView::SAT_BANDS_SAGITTAL ) ) {
                satBandsAllOn = true;
            }
            break;
        case svkDcmHeader::CORONAL:
            if( this->IsPropOn( svkSpectraReferenceView::SAT_BANDS_AXIAL ) ) {
                satBandsAllOn = true;
            }
            if( this->IsPropOn( svkSpectraReferenceView::SAT_BANDS_SAGITTAL ) ) {
                satBandsAllOn = true;
            }
            break;
        case svkDcmHeader::SAGITTAL:
            if( this->IsPropOn( svkSpectraReferenceView::SAT_BANDS_AXIAL ) ) {
                satBandsAllOn = true;
            }
            if( this->IsPropOn( svkSpectraReferenceView::SAT_BANDS_CORONAL ) ) {
                satBandsAllOn = true;
            }
            break;
    }
    return satBandsAllOn;
}


/*!     
 *
 */     
bool svkSpectraReferenceView::AreAllSatBandOutlinesOn( svkDcmHeader::Orientation orientation )
{
    bool satBandsOutlineAllOn = false;
    switch( orientation ) {
        case svkDcmHeader::AXIAL:
            if( this->IsPropOn( svkSpectraReferenceView::SAT_BANDS_CORONAL_OUTLINE ) ) {
                satBandsOutlineAllOn = true;
            }
            if( this->IsPropOn( svkSpectraReferenceView::SAT_BANDS_SAGITTAL_OUTLINE ) ) {
                satBandsOutlineAllOn = true;
            }
            break;
        case svkDcmHeader::CORONAL:
            if( this->IsPropOn( svkSpectraReferenceView::SAT_BANDS_AXIAL_OUTLINE ) ) {
                satBandsOutlineAllOn = true;
            }
            if( this->IsPropOn( svkSpectraReferenceView::SAT_BANDS_SAGITTAL_OUTLINE ) ) {
                satBandsOutlineAllOn = true;
            }
            break;
        case svkDcmHeader::SAGITTAL:
            if( this->IsPropOn( svkSpectraReferenceView::SAT_BANDS_AXIAL_OUTLINE ) ) {
                satBandsOutlineAllOn = true;
            }
            if( this->IsPropOn( svkSpectraReferenceView::SAT_BANDS_CORONAL_OUTLINE ) ) {
                satBandsOutlineAllOn = true;
            }
            break;
    }
    return satBandsOutlineAllOn;

}


/*!
 *
 */
void svkSpectraReferenceView::ToggleSelBoxVisibilityOn() 
{
    this->toggleSelBoxVisibility = true;
    if( this->GetProp( svkSpectraReferenceView::VOL_SELECTION ) == NULL ) {
        return;
    }
    if( this->dataVector[MRS] != NULL && static_cast<svkMrsImageData*>(this->dataVector[MRS])->IsSliceInSelectionBox( this->slice, this->orientation ) ) {
        this->GetProp( svkSpectraReferenceView::VOL_SELECTION )->SetVisibility(1);
        this->TurnPropOn( svkSpectraReferenceView::VOL_SELECTION );
    } else {
        vtkProp* volSelection = this->GetProp( svkSpectraReferenceView::VOL_SELECTION ); 
        if( volSelection != NULL ) {
            this->GetProp( svkSpectraReferenceView::VOL_SELECTION )->SetVisibility(0);
        }
    }
}


/*!
 *
 */
void svkSpectraReferenceView::ToggleSelBoxVisibilityOff() 
{
    this->toggleSelBoxVisibility = false;
    vtkProp* volSelection = this->GetProp( svkSpectraReferenceView::VOL_SELECTION ); 
    if( volSelection != NULL ) {
        this->TurnPropOn( svkSpectraReferenceView::VOL_SELECTION );
        this->GetProp( svkSpectraReferenceView::VOL_SELECTION )->SetVisibility(1);
    }
}


/*!
 *
 */
void svkSpectraReferenceView::AlignCamera() 
{
    int toggleDraw = this->GetRenderer( svkSpectraReferenceView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->GetRenderer( svkSpectraReferenceView::PRIMARY)->DrawOff();
    }

    // We don't want the presence of the sat bands to influence the alignment, so we will temporarily remove them.
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->RemoveViewProp( this->GetProp( svkSpectraReferenceView::SAT_BANDS_AXIAL) );
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->RemoveViewProp( this->GetProp( svkSpectraReferenceView::SAT_BANDS_AXIAL_OUTLINE) );
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->RemoveViewProp( this->GetProp( svkSpectraReferenceView::SAT_BANDS_CORONAL) );
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->RemoveViewProp( this->GetProp( svkSpectraReferenceView::SAT_BANDS_CORONAL_OUTLINE) );
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->RemoveViewProp( this->GetProp( svkSpectraReferenceView::SAT_BANDS_SAGITTAL) );
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->RemoveViewProp( this->GetProp( svkSpectraReferenceView::SAT_BANDS_SAGITTAL_OUTLINE) );
    this->imageViewer->ResetCamera();
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( this->GetProp( svkSpectraReferenceView::SAT_BANDS_AXIAL) );
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( this->GetProp( svkSpectraReferenceView::SAT_BANDS_AXIAL_OUTLINE) );
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( this->GetProp( svkSpectraReferenceView::SAT_BANDS_CORONAL) );
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( this->GetProp( svkSpectraReferenceView::SAT_BANDS_CORONAL_OUTLINE) );
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( this->GetProp( svkSpectraReferenceView::SAT_BANDS_SAGITTAL) );
    this->GetRenderer( svkSpectraReferenceView::PRIMARY)->AddActor( this->GetProp( svkSpectraReferenceView::SAT_BANDS_SAGITTAL_OUTLINE) );

    if( toggleDraw ) {
        this->GetRenderer( svkSpectraReferenceView::PRIMARY)->DrawOn();
    }
}


/*!
 *  Returns true if the current image is within the spectrscopy data set
 */
bool svkSpectraReferenceView::IsImageInsideSpectra() 
{
    return imageInsideSpectra;
}
