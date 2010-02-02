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


#include <svkPlotGridView.h>


using namespace svk;


#define DEBUG 0

vtkCxxRevisionMacro(svkPlotGridView, "$Rev$");
vtkStandardNewMacro(svkPlotGridView);


//! Constructor 
svkPlotGridView::svkPlotGridView()
{
    this->plotGrid = svkPlotLineGrid::New();

    // Setting layers is key for solaris rendering

    this->slice = 0;
    this->channel = 0;
    this->rwi = NULL;
    this->dataVector.push_back(NULL);
    this->dataVector.push_back(NULL);

    this->isPropOn.assign(svkPlotGridView::LAST_PROP+1, FALSE);
    this->isRendererOn.assign(svkPlotGridView::LAST_RENDERER+1, FALSE);
    this->isPropVisible.assign(svkPlotGridView::LAST_PROP+1, FALSE);     //Is the actor in the views FOV?

    vtkRenderer* nullRenderer = NULL;
    this->renCollection.assign(svkPlotGridView::LAST_RENDERER+1, nullRenderer);     //Is the actor in the views FOV?


    vtkProp* nullProp = NULL;
    this->propCollection.assign(svkPlotGridView::LAST_PROP+1, nullProp);     //Is the actor in the views FOV?
   
    vtkActor* entirePlotGrid = vtkActor::New();
    this->SetProp( svkPlotGridView::PLOT_GRID, entirePlotGrid );
    entirePlotGrid->Delete();

    vtkRenderer* ren = vtkRenderer::New();
    this->SetRenderer( svkPlotGridView::PRIMARY, ren ); 
    ren->Delete();

    this->GetRenderer( svkPlotGridView::PRIMARY )->SetLayer(0);
    this->plotGrid->SetRenderer( this->GetRenderer( svkPlotGridView::PRIMARY) );

    svkOpenGLOrientedImageActor* overlayActor = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkPlotGridView::OVERLAY_IMAGE, overlayActor );
    this->SetProp( svkPlotGridView::OVERLAY_TEXT, nullProp );
    overlayActor->Delete();
    this->colorTransfer = NULL;
    this->tlcBrc[0] = -1; 
    this->tlcBrc[1] = -1; 
    
    
}


//! Destructor
svkPlotGridView::~svkPlotGridView()
{
    if( this->rwi != NULL ) {
        this->rwi->Delete();
        this->rwi = NULL;
    }
    if( this->plotGrid != NULL ) {
        this->plotGrid->Delete();
        this->plotGrid = NULL;
    }
    if( this->colorTransfer != NULL ) {
        this->colorTransfer->Delete();
        this->colorTransfer = NULL;
    }
   
    // NOTE: The data is destroyed in the superclass 
    for( vector<vtkImageClip*>::iterator iter = metClippers.begin(); iter!= metClippers.end(); iter++ ) {
        if( *iter != NULL ) {
            (*iter)->Delete();
            (*iter) = NULL;
        }
    }

}


/*!
 *  Set input data and initialize default range values. 
 *  Also, should call ObserveData. Does some compatibility checking based
 *  on what is currently loaded.
 *
 *  \param data the candidate data for input
 *  \param index the index in which to set the data
 *
 */
void svkPlotGridView::SetInput(svkImageData* data, int index)
{
    if( strcmp( GetDataCompatibility( data, index).c_str(),"") == 0 ) { 
        if( index == MRS ) {
            if( dataVector[MRS] != NULL  ) {
                (dataVector[MRS])->Delete();
            }
            ObserveData( data );
            data->Register( this );
            dataVector[MRS] = data;
            int* extent = data->GetExtent();
            bool toggleDraw = this->GetRenderer( svkPlotGridView::PRIMARY )->GetDraw();
            if( toggleDraw ) {
                this->GetRenderer( svkPlotGridView::PRIMARY )->DrawOff();
            }
            if( slice > data->GetLastSlice() ) {
                slice = data->GetLastSlice();
            } else if ( slice < data->GetFirstSlice() ) {
                slice = data->GetFirstSlice(); 
            }
            plotGrid->SetInput(svkMrsImageData::SafeDownCast(data));
            plotGrid->AlignCamera(); 
            this->GeneratePlotGridActor();
            this->HighlightSelectionVoxels();
            this->SetSlice( slice );
            if( toggleDraw ) {
                this->GetRenderer( svkPlotGridView::PRIMARY )->DrawOn();
            }

        } else if( index == MET ) {
            if( dataVector[MET] != NULL  ) {
                (dataVector[MET])->Delete();
            }
            ObserveData( data );
            dataVector[MET] = data;
            data->Register( this );
            CreateMetaboliteOverlay( data );
        } 
    }
}


/*!
 *   Sets the slice.
 *  
 *  \param slice the new slice
 */
void svkPlotGridView::SetSlice(int slice)
{
    int toggleDraw = this->GetRenderer( svkPlotGridView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->GetRenderer( svkPlotGridView::PRIMARY )->DrawOff( );
    }
    if( this->dataVector[MRS] != NULL ) {
        if( slice > this->dataVector[MRS]->GetLastSlice() ) {
            slice = this->dataVector[MRS]->GetLastSlice();
        } else if ( slice < this->dataVector[MRS]->GetFirstSlice() ) {
            slice = this->dataVector[MRS]->GetFirstSlice(); 
        }
    }
    if( tlcBrc[0] >= 0 && tlcBrc[1] >= 0 ) {
        int* extent = dataVector[MRS]->GetExtent();
        tlcBrc[0] += (slice-this->slice)*extent[1]*extent[3];
        tlcBrc[1] += (slice-this->slice)*extent[1]*extent[3];
    }
    this->slice = slice;
    this->plotGrid->SetSlice(slice);
    this->plotGrid->SetPlotVoxels(tlcBrc[0], tlcBrc[1]);
    this->plotGrid->Update();
    this->plotGrid->AlignCamera();
    if( dataVector.size() > MET && dataVector[MET] != NULL ) {
        UpdateMetaboliteText(tlcBrc);
        int* extent = dataVector[MET]->GetExtent();
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkPlotGridView::OVERLAY_IMAGE ))->SetDisplayExtent(extent[0], extent[1], extent[2], extent[3], slice, slice);
        this->GetProp( svkPlotGridView::OVERLAY_IMAGE )->Modified();
    }
    this->GenerateClippingPlanes();
    if( toggleDraw ) {
        this->GetRenderer( svkPlotGridView::PRIMARY )->DrawOn( );
    }
    this->Refresh();
}


/*!
 *  Currently this method is being used to set the selection range. This
 *  should be changed to have a more meaningful name, or be implemented
 *  differently.
 *
 *  TODO: Re-implement once svkDataImage is being used to pass Actor settings.
 *
 *  \param tlcID the id of the top left corner voxel
 *  \param tlcID the id of the bottom right corner voxel
 */
void svkPlotGridView::SetTlcBrc(int tlcID, int brcID)
{
    int toggleDraw = this->GetRenderer( svkPlotGridView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->GetRenderer( svkPlotGridView::PRIMARY )->DrawOff( );
    }
    this->tlcBrc[0] = tlcID;
    this->tlcBrc[1] = brcID;
    plotGrid->SetPlotVoxels(tlcID, brcID);
    plotGrid->Update();
    UpdateMetaboliteText(tlcBrc);
    this->GenerateClippingPlanes();
    plotGrid->AlignCamera(); 
    if( toggleDraw ) {
        this->GetRenderer( svkPlotGridView::PRIMARY )->DrawOn( );
    }
    this->Refresh();
}


/*!
 *  Sets the interactor, and attach a renderer it its window.
 *  NOTE: This will remove any/all renderers that may have been
 *  in the renderWindow.
 *  
 *  TODO: AddRenderer can throw an error if the RenderWindow has not been
 *        initialized, either add a check or re-implement.
 *
 *  \param rwi the vtkRendererWindowInteractor you wish to associate with the view
 */
void svkPlotGridView::SetRWInteractor( vtkRenderWindowInteractor* rwi )
{
    if( this->rwi != NULL ) {
        this->rwi->Delete();
    }
    this->rwi = rwi;
    this->rwi->Register( this );

    // Lets make sure there are no renderers in the new render window, if there are remove them
    vtkRendererCollection* myRenderers = this->rwi->GetRenderWindow()->GetRenderers();
    vtkCollectionIterator* myIterator = vtkCollectionIterator::New();
    myIterator->SetCollection( myRenderers );
    while( !myIterator->IsDoneWithTraversal() ) {
        this->rwi->GetRenderWindow()->RemoveRenderer( static_cast<vtkRenderer*>(myIterator->GetCurrentObject()) );
        myIterator->GoToNextItem();
    }
    if( !this->rwi->GetRenderWindow()->HasRenderer( this->GetRenderer( svkPlotGridView::PRIMARY) ) ) {
        this->rwi->GetRenderWindow()->AddRenderer( this->GetRenderer( svkPlotGridView::PRIMARY ) );
    }
    myIterator->Delete();

}


/*!
 *  SetWindowLevel for spectral view;  index 0 is frequency, index 1 is intensity.
 *
 *  TODO: Currently the range is cast to ints, so that it can set the index
 *        in the arrays of the plotGrid object. This should be changed to
 *        to handle units other than points, OR set this to always take ints
 *        and handle unit conversion on the widget layer.
 *
 *  \param lower the lower limit
 *  \param upper the upper limit
 *  \param index which dimension you wish to chhange, frequency or magnitude 
 *
 */
void svkPlotGridView::SetWindowLevelRange( double lower, double upper, int index)
{
    if (index == FREQUENCY) {
        this->plotGrid->SetFrequencyWLRange((int)lower, (int)upper);
    } else if (index == AMPLITUDE) {
        this->plotGrid->SetIntensityWLRange(lower, upper);
    }
    this->Refresh();
}


/*
 *  Set the component to display: 
 *
 *  \param component the compent you wish to display, REAL, IMAGINARY, MAGNITUDE
 */
void svkPlotGridView::SetComponent( svkPlotLine::PlotComponent component)
{
    this->plotGrid->SetComponent( component ); 
    this->Refresh();
}


/*!
 *  Sets desired the current selection in Display (pixels) coordinates
 *  and highlights the intersected voxels.
 *
 *  \param selectionArea the area you wish to select voxels within [xmin, xmax, ymin, ymax]
 */ 
void svkPlotGridView::SetSelection( double* selectionArea, bool isWorldCords )
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
            worldStart[0] = *coordStart->GetComputedWorldValue(this->GetRenderer( svkPlotGridView::PRIMARY)); 
            worldStart[1] = *(coordStart->GetComputedWorldValue(this->GetRenderer( svkPlotGridView::PRIMARY)) + 1); 
            worldStart[2] = *(coordStart->GetComputedWorldValue(this->GetRenderer( svkPlotGridView::PRIMARY)) + 2); 
            worldEnd[0] = *coordEnd->GetComputedWorldValue(this->GetRenderer( svkPlotGridView::PRIMARY)); 
            worldEnd[1] = *(coordEnd->GetComputedWorldValue(this->GetRenderer( svkPlotGridView::PRIMARY)) + 1); 
            worldEnd[2] = *(coordEnd->GetComputedWorldValue(this->GetRenderer( svkPlotGridView::PRIMARY)) + 2); 
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
            this->plotGrid->SetPlotVoxels(tlcBrc[0], tlcBrc[1]);
            GenerateClippingPlanes(); 
        } 
        if( dataVector[MET] != NULL ) {
            UpdateMetaboliteText(tlcBrc);
        }


    } else if( dataVector[MRS] != NULL ) {

        //What should we do when the mri data is null, but the mrs is not....
    } 
    rwi->Render();
}


//! Method is called when data object is Modified. 
void svkPlotGridView::Refresh()
{
    if (DEBUG) {
        cout << "svkPlotGridView::Refresh calls plotGrid->Update() first " << endl; 
    }

    this->svkDataView::Refresh(); 
}


/*! 
 *  Method highlights voxels within the selection box
 *
 *  \return tlcBrc the cell id's of the desired top left, bottom right corners
 */
int* svkPlotGridView::HighlightSelectionVoxels()
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
        vtkPoints* cellBoxPoints = vtkPointSet::SafeDownCast(
                                   plotGrid->selectionBoxActor->GetMapper()->GetInput())->GetPoints();

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

        this->SetSelection( thresholdBounds, 1 );
        return tlcBrc;
    } else {
        return NULL; 
    }
}


/*!
 *  Creates the svkOrientedImageActor for rendering an overlay.
 *
 *  \param data the data to be rendered as an overlay
 *
 *  TODO: Implement changes for multiple overlays
 */
void svkPlotGridView::CreateMetaboliteOverlay( svkImageData* data )
{
    if( dataVector[MRS] != NULL && data != NULL ) {
        int* extent = data->GetExtent();
        double* spacing = data->GetSpacing();

        if( this->GetProp( svkPlotGridView::OVERLAY_TEXT ) == NULL ) {
            vtkActor2D* metActor = vtkActor2D::New();
            this->SetProp( svkPlotGridView::OVERLAY_TEXT, metActor );
            metActor->Delete();
        }
        vtkLabeledDataMapper* metMapper = vtkLabeledDataMapper::New();
        vtkImageClip* metTextClipper = vtkImageClip::New();
        metTextClipper->SetInput( dataVector[MET] );
        metMapper->SetInput( metTextClipper->GetOutput() );
        metMapper->SetLabelModeToLabelScalars();
        metMapper->SetLabeledComponent(0);
        double *range = dataVector[MET]->GetScalarRange();
        if( range[1] <= 1 ) {
            metMapper->SetLabelFormat("%1.0f");
        } else if( range[1] < 100000 ) {
            metMapper->SetLabelFormat("%1.2f");
        } else {
           metMapper->SetLabelFormat("%0.1e");
        }
        metMapper->GetLabelTextProperty()->ShadowOff();
        metMapper->GetLabelTextProperty()->ItalicOff();
        metMapper->GetLabelTextProperty()->BoldOff();
        metMapper->GetLabelTextProperty()->SetFontSize(10);
        metMapper->GetLabelTextProperty()->SetColor(0,1,1);

        // Met clippers is an array so that we could potentially add multiple metabolites
        if( metClippers.size() == 0 ) {
            metClippers.push_back( metTextClipper ); 
        } else {
            if( metClippers[0] != NULL ) {
                metClippers[0]->Delete(); 
            }
            metClippers[0] = metTextClipper; 
        }

        // Sets up to transform text relative position to the corner of the voxel
        vtkTransform* optimus = vtkTransform::New();
        double displacement[3] = {0,0,0};
        double dU = -spacing[0]/2.1;
        double dV = -spacing[1]/3.2;
        double dcos[3][3];
        this->dataVector[MET]->GetDcos( dcos );
        for ( int i = 0; i < 3; i++ ) {
            displacement[i] =  (dU) * dcos[0][i] + (dV) * dcos[1][i] + (spacing[2]/2.0) * dcos[2][i];
        }
        optimus->Translate( displacement );

        metMapper->SetTransform(optimus);
        optimus->Delete();
        vtkActor2D::SafeDownCast(this->GetProp( svkPlotGridView::OVERLAY_TEXT ))->SetMapper( metMapper );  
        metMapper->Delete();

        // If it has not been added, add it
        if( !this->GetRenderer( svkPlotGridView::PRIMARY )->HasViewProp( this->GetProp( svkPlotGridView::OVERLAY_TEXT )) ) {
            this->GetRenderer( svkPlotGridView::PRIMARY )->AddViewProp( this->GetProp( svkPlotGridView::OVERLAY_TEXT ));
        }
        UpdateMetaboliteText( tlcBrc );

        svkImageMapToColors* windowLevel = svkImageMapToColors::New();
        metTextClipper->Update();
        windowLevel->SetInput( dataVector[MET] );
        if( this->colorTransfer == NULL ) {
           this->colorTransfer = svkLookupTable::New();
        }

        double window = range[1] - range[0];
        double level = 0.1*(range[1] + range[0]);
        this->colorTransfer->SetRange( level - window/2.0, level + window/2.0);

        this->colorTransfer->SetLUTType( svkLookupTable::GREY_SCALE );
        this->colorTransfer->SetAlphaThreshold( 0.9 );

        windowLevel->SetLookupTable( this->colorTransfer );
        windowLevel->SetOutputFormatToRGBA( );
        windowLevel->Update( );


        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkPlotGridView::OVERLAY_IMAGE ))->SetInput(windowLevel->GetOutput());
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkPlotGridView::OVERLAY_IMAGE ))->SetDisplayExtent(extent[0], extent[1], extent[2], extent[3], slice, slice);
        vtkImageActor::SafeDownCast(this->GetProp( svkPlotGridView::OVERLAY_IMAGE ))->SetOpacity( 0.5 );
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkPlotGridView::OVERLAY_IMAGE ))->InterpolateOff();
        if( this->GetRenderer( svkPlotGridView::PRIMARY)->HasViewProp( this->GetProp( svkPlotGridView::OVERLAY_IMAGE) ) ) {
            this->GetRenderer( svkPlotGridView::PRIMARY)->RemoveActor( this->GetProp( svkPlotGridView::OVERLAY_IMAGE) );
        }
        this->GetRenderer( svkPlotGridView::PRIMARY)->AddActor( this->GetProp( svkPlotGridView::OVERLAY_IMAGE) );
        this->TurnPropOff( svkPlotGridView::OVERLAY_IMAGE );
        this->GetProp( svkPlotGridView::OVERLAY_IMAGE )->Modified();
        this->GetRenderer( svkPlotGridView::PRIMARY)->Render();
        this->plotGrid->AlignCamera();
        this->Refresh();

   } 
}


/*!
 *  Updates the visible metabolite values depending on the selection
 *
 *  \param tlcBrc points to a legth two int vector, the first index being the top left corner
 *         and the second being the bottom right.
 */
void svkPlotGridView::UpdateMetaboliteText(int* tlcBrc) 
{
    if( dataVector[MRS] != NULL ) {
        int* extent = dataVector[MRS]->GetExtent();
    
        int tlcIndexY = ((tlcBrc[0] - (slice * extent[1] * extent[3]))/extent[1]);
        int tlcIndexX = (tlcBrc[0]%extent[1]);
        int brcIndexY = ((tlcBrc[1] - (slice * extent[1] * extent[3]))/extent[1]);
        int brcIndexX = (tlcBrc[1]%extent[1]);
        for( vector<vtkImageClip*>::iterator iter = metClippers.begin(); iter!= metClippers.end(); iter++ ) {
            (*iter)->SetOutputWholeExtent(tlcIndexX, brcIndexX, tlcIndexY, brcIndexY, slice,slice); 
            (*iter)->ClipDataOn();
        }
        this->Refresh();
    }
}


/*!
 *  Get the current slice.
 *
 *  \return slice the current slice
 *
 */
int  svkPlotGridView::GetSlice() 
{
    return this->slice;
}


/*!
 *  Sets the color schema. Currently we only support one light-on-dark 
 *  and one dark-on-light. Used for making a printable version.
 *
 *  \param colorSchema the color scheme you want, options are
 *                     svkPlotGriView::LIGHT_ON_DARK or svkPlotGridView::DARK_ON_LIGHT
 */
void svkPlotGridView::SetColorSchema( int colorSchema )                
{
    double backgroundColor[3];
    double foregroundColor[3];
    if( colorSchema == svkPlotGridView::LIGHT_ON_DARK ) {
        backgroundColor[0] = 0;
        backgroundColor[1] = 0;
        backgroundColor[2] = 0;
        foregroundColor[0] = 1;
        foregroundColor[1] = 1;
        foregroundColor[2] = 1;
    } else if ( colorSchema == svkPlotGridView::DARK_ON_LIGHT ) {
        backgroundColor[0] = 1;
        backgroundColor[1] = 1;
        backgroundColor[2] = 1;
        foregroundColor[0] = 0;
        foregroundColor[1] = 0;
        foregroundColor[2] = 0;

    } 
    this->GetRenderer( svkPlotGridView::PRIMARY )->SetBackground( backgroundColor );
    vtkActorCollection* plotGridActors = this->GetRenderer( svkPlotGridView::PRIMARY )->GetActors();
    vtkCollectionIterator* iterator = vtkCollectionIterator::New();
    iterator->SetCollection( plotGridActors );
    iterator->InitTraversal();
    vtkActor* currentActor;
    while( !iterator->IsDoneWithTraversal() ) {
        if( iterator->GetCurrentObject() != NULL && iterator->GetCurrentObject()->IsA("vtkActor") ) {
            currentActor = vtkActor::SafeDownCast( iterator->GetCurrentObject() );
            if( currentActor->IsA("vtkActor") ) {
                currentActor->GetProperty()->SetAmbientColor( foregroundColor );
                currentActor->Modified();
            }
        }
        iterator->GoToNextItem();
    }
    iterator->Delete();

}


/*!
 *  Check to make sure a given dataset is comptabile with the currently loaded data sets.
 *
 *  \param data the prospective data.
 *  \param targetIndex the index in which the data is trying to be placed.
 *
 *  \return resultInfo returns an empty string if the dataset is compatible,
 *          otherwise an explaination of why the dataset is not compatible is returned.
 */
string svkPlotGridView::GetDataCompatibility( svkImageData* data, int targetIndex )
{
    svkDataValidator* validator = svkDataValidator::New();
    string resultInfo = "";
    
    // Check for null datasets and out of bound data sets...
    if ( data == NULL || targetIndex > MET || targetIndex < 0 ) {
        resultInfo += "Data incompatible-- NULL or outside of input range!\n";

    } else if( data->IsA("svkMriImageData") ) {
        if( dataVector[MRS] != NULL ) {
            //cout << "PLOT GRID VIEW VALIDATOR 1" << endl;
            svkDataValidator::ValidationErrorStatus status
                = validator->AreDataIncompatible( data, dataVector[MRS] ); 
            if( status == svkDataValidator::INVALID_DATA_ORIENTATION ) {
                cout << "WARNING, reformatting images to spectroscopic orientation" << endl; 
                resultInfo = "";
                this->ResliceImage( data, dataVector[MRS] );
            } else if( status ) {
                resultInfo += validator->resultInfo.c_str(); 
                resultInfo += "\n"; 
            } 
            int* overlayExtent = data->GetExtent();
            int* spectraExtent = dataVector[MRS]->GetExtent();

            // If its on overlay it must have the same extent as our spectra
            //  SHOULDN'T THIS BE IN VALIDATOR CLASS?
            if( overlayExtent[0] != spectraExtent[0] || overlayExtent[1] != spectraExtent[1]-1 || 
                overlayExtent[2] != spectraExtent[2] || overlayExtent[3] != spectraExtent[3]-1 ||
                overlayExtent[4] != spectraExtent[4] || overlayExtent[5] != spectraExtent[5]-1 ) { 

                    resultInfo += "Mismatched extents.\n";
                 
            }
        } else {
            resultInfo += "Spectra must be loaded before overlays!\n";
        } 
    } else if( data->IsA("svkMrsImageData") ) {
        if( dataVector[MET] != NULL ) {
            //cout << "PLOT GRID VIEW VALIDATOR 2" << endl;
            svkDataValidator::ValidationErrorStatus status
                = validator->AreDataIncompatible( data, dataVector[MET] );  
            if( status == svkDataValidator::INVALID_DATA_ORIENTATION ) {
                cout << "WARNING, reformatting images to spectroscopic orientation" << endl; 
                resultInfo = "";
                this->ResliceImage( dataVector[MET], data );

            } else if( status ) {
                resultInfo += validator->resultInfo.c_str(); 
                resultInfo += "\n"; 
            }
        }
    } else {
        resultInfo += "Unrecognized data type!\n";
    }
   
    cout << resultInfo.c_str() << endl;
    validator->Delete();
    return resultInfo; 
}


/*!
 *  Reslice images to MRS orientation
 */
void svkPlotGridView::ResliceImage(svkImageData* input, svkImageData* target)
{
    //  if (orthogonal orientations) {
    svkObliqueReslice* reslicer = svkObliqueReslice::New();
    reslicer->SetInput( input );
    reslicer->SetTargetDcosFromImage( target );
    reslicer->Update();
    this->SetInput( reslicer->GetOutput(), MET );
    //}
}


/*!
 *
 */
void svkPlotGridView::SetChannel( int channel )
{
    this->channel = channel;
    this->plotGrid->SetChannel( channel );
    this->rwi->InvokeEvent(vtkCommand::SelectionChangedEvent);
    this->Refresh();

}


/*!
 *
 */
int svkPlotGridView::GetChannel( )
{
    return this->channel;

}


/*!
 *  Sets the opacity of the image overlay.
 *
 *   \param opacity the new opacity you wish the image overlay to have. 
 */
void svkPlotGridView::SetOverlayOpacity( double opacity )
{
    if( this->GetProp( svkPlotGridView::OVERLAY_IMAGE ) != NULL ) {
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkPlotGridView::OVERLAY_IMAGE ))->SetOpacity( opacity );
    }
}


/*!
 *  Sets the threshold of the image overlay.
 *
 *   \param threshold the new threshold you wish the image overlay to have. 
 */
void svkPlotGridView::SetOverlayThreshold( double threshold )
{
    if( this->GetProp( svkPlotGridView::OVERLAY_IMAGE ) != NULL ) {
        this->colorTransfer->SetAlphaThreshold(threshold);
    }
}


/*!
 *  Creates the actor that outlines the voxels
 */
void svkPlotGridView::GeneratePlotGridActor( ) 
{
    int toggleDraw = this->GetRenderer( svkPlotGridView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->GetRenderer( svkPlotGridView::PRIMARY)->DrawOff();
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
    vtkActor::SafeDownCast( this->GetProp( svkPlotGridView::PLOT_GRID))->SetMapper( entireGridMapper );
    entireGridMapper->Delete();
    vtkActor::SafeDownCast( GetProp( svkPlotGridView::PLOT_GRID) )->GetProperty()->SetDiffuseColor( 0, 1, 0 );
    this->GetRenderer( svkPlotGridView::PRIMARY)->AddActor( this->GetProp( svkPlotGridView::PLOT_GRID ) );
    if( toggleDraw ) {
        this->GetRenderer( svkPlotGridView::PRIMARY)->DrawOn();
    }

}

/*!
 * Generates the clipping planes for the mMMapper. This is how the boundries
 * set are enforced, after the data is scaled, it is clipped so that data
 * outside the plot range is simply not shown.
 */
void svkPlotGridView::GenerateClippingPlanes()
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
        vtkActor::SafeDownCast( this->GetProp( svkPlotGridView::PLOT_GRID )
                                 )->GetMapper()->RemoveAllClippingPlanes();
        vtkActor::SafeDownCast( this->GetProp( svkPlotGridView::PLOT_GRID )
                                 )->GetMapper()->AddClippingPlane( clipperPlane0 );
        vtkActor::SafeDownCast( this->GetProp( svkPlotGridView::PLOT_GRID )
                                 )->GetMapper()->AddClippingPlane( clipperPlane1 );
        vtkActor::SafeDownCast( this->GetProp( svkPlotGridView::PLOT_GRID )
                                 )->GetMapper()->AddClippingPlane( clipperPlane2 );
        vtkActor::SafeDownCast( this->GetProp( svkPlotGridView::PLOT_GRID )
                                 )->GetMapper()->AddClippingPlane( clipperPlane3 );
        vtkActor::SafeDownCast( this->GetProp( svkPlotGridView::PLOT_GRID )
                                 )->GetMapper()->AddClippingPlane( clipperPlane4 );
        vtkActor::SafeDownCast( this->GetProp( svkPlotGridView::PLOT_GRID )
                                 )->GetMapper()->AddClippingPlane( clipperPlane5 );
        plotGrid->plotGridActor->GetMapper()->RemoveAllClippingPlanes();
        
        plotGrid->plotGridActor->GetMapper()->AddClippingPlane( clipperPlane0 );
        plotGrid->plotGridActor->GetMapper()->AddClippingPlane( clipperPlane1 );
        plotGrid->plotGridActor->GetMapper()->AddClippingPlane( clipperPlane2 );
        plotGrid->plotGridActor->GetMapper()->AddClippingPlane( clipperPlane3 );
        plotGrid->plotGridActor->GetMapper()->AddClippingPlane( clipperPlane4 );
        plotGrid->plotGridActor->GetMapper()->AddClippingPlane( clipperPlane5 );

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
 *
 */
void svkPlotGridView::SetOrientation( svkDcmHeader::Orientation orientation ) 
{
    this->orientation = orientation;
    this->GenerateClippingPlanes();
}

