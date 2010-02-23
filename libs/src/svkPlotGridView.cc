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
    this->orientation = svkDcmHeader::AXIAL;

    // Setting layers is key for solaris rendering

    this->slice = 0;
    this->channel = 0;
    this->timePoint = 0;
    this->rwi = NULL;
    this->dataVector.push_back(NULL);
    this->dataVector.push_back(NULL);

    this->isPropOn.assign(svkPlotGridView::LAST_PROP+1, FALSE);
    this->isRendererOn.assign(svkPlotGridView::LAST_RENDERER+1, FALSE);
    this->isPropVisible.assign(svkPlotGridView::LAST_PROP+1, FALSE);     //Is the actor in the views FOV?

    vtkRenderer* nullRenderer = NULL;
    this->renCollection.assign(svkPlotGridView::LAST_RENDERER+1, nullRenderer);     //Is the actor in the views FOV?


    vtkProp* nullProp = NULL;
    this->propCollection.assign(svkPlotGridView::LAST_PROP+1, nullProp);     //Is the actor in the views FOV?n
   
    vtkActor* entirePlotGrid = vtkActor::New();
    this->SetProp( svkPlotGridView::PLOT_GRID, entirePlotGrid );
    entirePlotGrid->Delete();

    svkOpenGLOrientedImageActor* overlayActor = svkOpenGLOrientedImageActor::New();
    this->SetProp( svkPlotGridView::OVERLAY_IMAGE, overlayActor );
    this->SetProp( svkPlotGridView::OVERLAY_TEXT, nullProp );
    overlayActor->InterpolateOff();

    overlayActor->Delete();
    this->colorTransfer = NULL;
    this->tlcBrc[0] = -1; 
    this->tlcBrc[1] = -1; 

    this->satBands = svkSatBandSet::New();
    this->SetProp( svkPlotGridView::SAT_BANDS, this->satBands->GetSatBandsActor() );
    this->TurnPropOff( svkPlotGridView::SAT_BANDS );
    this->SetProp( svkPlotGridView::SAT_BANDS_OUTLINE, this->satBands->GetSatBandsOutlineActor() );
    this->TurnPropOff( svkPlotGridView::SAT_BANDS_OUTLINE );
    this->satBands->SetOrientation( this->orientation );

    
    
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
    if( this->satBands != NULL ) {
        this->satBands->Delete();
        this->satBands = NULL;
    }

   
    // NOTE: The data is destroyed in the superclass 
    for( vector<svkImageClip*>::iterator iter = metClippers.begin(); iter!= metClippers.end(); iter++ ) {
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
            this->satBands->SetInput( static_cast<svkMrsImageData*>(this->dataVector[MRS]) );
            this->satBands->SetClipSlice( this->slice );
            if( !this->GetRenderer( svkPlotGridView::PRIMARY)->HasViewProp( this->GetProp( svkPlotGridView::SAT_BANDS))) {
                this->GetRenderer( svkPlotGridView::PRIMARY)->AddActor(  this->GetProp( svkPlotGridView::SAT_BANDS));
            }
            if( !this->GetRenderer( svkPlotGridView::PRIMARY)->HasViewProp( this->GetProp( svkPlotGridView::SAT_BANDS_OUTLINE))) {
                this->GetRenderer( svkPlotGridView::PRIMARY)->AddActor(  this->GetProp( svkPlotGridView::SAT_BANDS_OUTLINE));
            }
            this->SetProp( svkPlotGridView::SAT_BANDS, this->satBands->GetSatBandsActor() );
            string acquisitionType = data->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
            if( acquisitionType == "SINGLE VOXEL" ) {
                this->TurnPropOff( svkPlotGridView::PLOT_GRID );
            } else {
                this->TurnPropOn( svkPlotGridView::PLOT_GRID );
            }

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
    this->slice = slice;
    if( this->dataVector[MRS] != NULL ) {
        int tlcIndex[3];
        int brcIndex[3];
        this->dataVector[MRS]->GetIndexFromID( tlcBrc[0], tlcIndex );
        this->dataVector[MRS]->GetIndexFromID( tlcBrc[1], brcIndex );
        int lastSlice  = dataVector[MRS]->GetLastSlice( this->orientation );
        int firstSlice = dataVector[MRS]->GetFirstSlice( this->orientation );
        slice = (slice > lastSlice) ? lastSlice:slice;
        slice = (slice < firstSlice) ? firstSlice:slice;
        this->slice = slice;
        tlcIndex[ this->dataVector[MRS]->GetOrientationIndex( this->orientation ) ] = slice;
        brcIndex[ this->dataVector[MRS]->GetOrientationIndex( this->orientation ) ] = slice;
        tlcBrc[0] = this->dataVector[MRS]->GetIDFromIndex( tlcIndex[0], tlcIndex[1], tlcIndex[2] );
        tlcBrc[1] = this->dataVector[MRS]->GetIDFromIndex( brcIndex[0], brcIndex[1], brcIndex[2] );
        this->satBands->SetClipSlice( this->slice );

    }
    this->plotGrid->SetSlice(slice);
    this->plotGrid->SetTlcBrc(tlcBrc);
    this->plotGrid->Update();
    this->plotGrid->AlignCamera();
    if( dataVector[MET] != NULL ) {
        this->UpdateMetaboliteText(tlcBrc);
        this->UpdateMetaboliteImage(tlcBrc);
    }
    this->GenerateClippingPlanes();
    if( toggleDraw ) {
        this->GetRenderer( svkPlotGridView::PRIMARY )->DrawOn( );
    }
    this->Refresh();
}


/*!
 *
 */
void svkPlotGridView::SetTlcBrc(int tlcBrc[2])
{
    this->SetTlcBrc( tlcBrc[0], tlcBrc[1]);
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
    if( svkDataView::IsTlcBrcWithinData(this->dataVector[MRS],tlcID, brcID ) ) {
        this->tlcBrc[0] = tlcID;
        this->tlcBrc[1] = brcID;
        int toggleDraw = this->GetRenderer( svkPlotGridView::PRIMARY )->GetDraw();
        if( toggleDraw ) {
            this->GetRenderer( svkPlotGridView::PRIMARY )->DrawOff( );
        }
        plotGrid->SetTlcBrc(this->tlcBrc);
        plotGrid->Update();
        this->UpdateMetaboliteText(tlcBrc);
        this->UpdateMetaboliteImage(tlcBrc);
        this->GenerateClippingPlanes();
        plotGrid->AlignCamera(); 
        if( toggleDraw ) {
            this->GetRenderer( svkPlotGridView::PRIMARY )->DrawOn( );
        }
        this->Refresh();
    }
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

    this->rwi->GetRenderWindow()->AddRenderer( this->GetRenderer( svkPlotGridView::PRIMARY ) );
    this->plotGrid->SetRenderer( this->GetRenderer( svkPlotGridView::PRIMARY) );

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

    } 
    this->rwi->Render();
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
        int tlcBrcImageData[2];
        svkMrsImageData::SafeDownCast(this->dataVector[MRS])->GetTlcBrcInSelectionBox( tlcBrcImageData, tolerance, this->orientation, this->slice );
        this->SetTlcBrc( tlcBrcImageData );
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
        double* spacing = data->GetSpacing();

        if( this->GetProp( svkPlotGridView::OVERLAY_TEXT ) == NULL ) {
            vtkActor2D* metActor = vtkActor2D::New();
            this->SetProp( svkPlotGridView::OVERLAY_TEXT, metActor );
            metActor->Delete();
        }
        vtkLabeledDataMapper* metMapper = vtkLabeledDataMapper::New();
        svkImageClip* metTextClipper = svkImageClip::New();
        metTextClipper->SetInput( this->dataVector[MET] );
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
        vtkActor2D::SafeDownCast(this->GetProp( svkPlotGridView::OVERLAY_TEXT ))->SetMapper( metMapper );  
        metMapper->Delete();

        // If it has not been added, add it
        if( !this->GetRenderer( svkPlotGridView::PRIMARY )->HasViewProp( this->GetProp( svkPlotGridView::OVERLAY_TEXT )) ) {
            this->GetRenderer( svkPlotGridView::PRIMARY )->AddViewProp( this->GetProp( svkPlotGridView::OVERLAY_TEXT ));
        }
        this->UpdateMetaboliteTextDisplacement();
        this->UpdateMetaboliteText( tlcBrc );

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
        windowLevel->Delete();
        if( this->GetRenderer( svkPlotGridView::PRIMARY)->HasViewProp( this->GetProp( svkPlotGridView::OVERLAY_IMAGE) ) ) {
            this->GetRenderer( svkPlotGridView::PRIMARY)->RemoveActor( this->GetProp( svkPlotGridView::OVERLAY_IMAGE) );
        }
        this->GetRenderer( svkPlotGridView::PRIMARY)->AddActor( this->GetProp( svkPlotGridView::OVERLAY_IMAGE) );
        this->TurnPropOff( svkPlotGridView::OVERLAY_IMAGE );
        this->GetProp( svkPlotGridView::OVERLAY_IMAGE )->Modified();
        this->GetRenderer( svkPlotGridView::PRIMARY)->Render();
        this->UpdateMetaboliteText(this->tlcBrc);
        this->plotGrid->AlignCamera();
        this->SetOverlayOpacity(0.5);
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
        int tlcIndex[3];
        int brcIndex[3];
        this->dataVector[MRS]->GetIndexFromID( tlcBrc[0], tlcIndex );
        this->dataVector[MRS]->GetIndexFromID( tlcBrc[1], brcIndex );
        for( vector<svkImageClip*>::iterator iter = metClippers.begin(); iter!= metClippers.end(); iter++ ) {
            (*iter)->SetOutputWholeExtent(tlcIndex[0], brcIndex[0], tlcIndex[1], brcIndex[1], tlcIndex[2], brcIndex[2]); 
            (*iter)->ClipDataOn();
        }
        svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkPlotGridView::OVERLAY_IMAGE ))->SetDisplayExtent(tlcIndex[0], brcIndex[0], tlcIndex[1], brcIndex[1], tlcIndex[2], brcIndex[2]);
    }
}


/*!
 *  Updates the visible metabolite values depending on the selection
 *
 *  \param tlcBrc points to a legth two int vector, the first index being the top left corner
 *         and the second being the bottom right.
 */
void svkPlotGridView::UpdateMetaboliteImage(int* tlcBrc) 
{
    int tlcIndex[3];
    int brcIndex[3];
    if( dataVector[MRS] != NULL ) {
        int minIndex[3] = {0,0,0};
        int maxIndex[3] = {this->dataVector[MRS]->GetDimensions()[0]-2,this->dataVector[MRS]->GetDimensions()[1]-2,this->dataVector[MRS]->GetDimensions()[2]-2};
        int orientationIndex = this->dataVector[MRS]->GetOrientationIndex( this->orientation );

        minIndex[ orientationIndex ] = this->slice;
        maxIndex[ orientationIndex ] = this->slice;
        int minID = this->dataVector[MRS]->GetIDFromIndex( minIndex[0], minIndex[1], minIndex[2] );
        int maxID = this->dataVector[MRS]->GetIDFromIndex( maxIndex[0], maxIndex[1], maxIndex[2] );

        this->dataVector[MRS]->GetIndexFromID( tlcBrc[0], tlcIndex );
        this->dataVector[MRS]->GetIndexFromID( tlcBrc[1], brcIndex );
        for( int i = 0; i < 3; i++ ) {
            if( i != orientationIndex ) {
                tlcIndex[i]--;
                brcIndex[i]++;
            }
            if( tlcIndex[i] < minIndex[i] ) {
                tlcIndex[i] = minIndex[i];
            }
            if( brcIndex[i] > maxIndex[i] ) {
                brcIndex[i] = maxIndex[i];
            }
        }
    }
    svkOpenGLOrientedImageActor::SafeDownCast(this->GetProp( svkPlotGridView::OVERLAY_IMAGE ))->SetDisplayExtent(tlcIndex[0], brcIndex[0], tlcIndex[1], brcIndex[1], tlcIndex[2], brcIndex[2]);
    this->GetProp( svkPlotGridView::OVERLAY_IMAGE )->Modified();
}


/*!
 *  
 */
void svkPlotGridView::UpdateMetaboliteTextDisplacement() 
{
    if( this->dataVector[MET] != NULL ) {
        vtkTransform* optimus = vtkTransform::New();
        double displacement[3] = {0,0,0};
        double dL;
        double dP;
        double dS;
        double* spacing = this->dataVector[MET]->GetSpacing();
        switch( this->orientation ) {
            case svkDcmHeader::AXIAL:
                dL = -spacing[0]/2.1;
                dP = -spacing[1]/3.2;
                dS = spacing[2]/2.0;
                break;
            case svkDcmHeader::CORONAL:
                dL = -spacing[0]/2.1;
                dP = -spacing[1]/2.0;
                dS = -spacing[2]/3.2;
                break;
            case svkDcmHeader::SAGITTAL:
                dL = -spacing[0]/2.0;
                dP = -spacing[1]/2.1;
                dS = -spacing[2]/3.2;
                break;
        }
        double dcos[3][3];
        this->dataVector[MET]->GetDcos( dcos );
        for ( int i = 0; i < 3; i++ ) {
            displacement[i] =  (dL) * dcos[0][i] + (dP) * dcos[1][i] + (dS) * dcos[2][i];
        }
        optimus->Translate( displacement );
        vtkLabeledDataMapper::SafeDownCast( 
                vtkActor2D::SafeDownCast( 
                    this->GetProp( svkPlotGridView::OVERLAY_TEXT ) )->GetMapper())->SetTransform(optimus);
        optimus->Delete();

    }
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
    this->plotGrid->plotGridActor->GetProperty()->SetColor( foregroundColor );

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
void svkPlotGridView::SetTimePoint( int timePoint )
{
    this->timePoint = timePoint;
    this->plotGrid->SetTimePoint( timePoint );
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
    if( this->dataVector[MRS] != NULL ) {
        string acquisitionType = this->dataVector[MRS]->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
        if( acquisitionType != "SINGLE VOXEL" ) {
            this->ClipMapperToTlcBrc( dataVector[MRS],
                                 vtkActor::SafeDownCast( this->GetProp( svkPlotGridView::PLOT_GRID ))->GetMapper(), tlcBrc, CLIP_TOLERANCE, CLIP_TOLERANCE, CLIP_TOLERANCE );
            this->ClipMapperToTlcBrc( this->dataVector[MRS], this->plotGrid->plotGridActor->GetMapper(),
                                 tlcBrc, CLIP_TOLERANCE, CLIP_TOLERANCE, CLIP_TOLERANCE );
        }
    }
}


/*!     
 *
 */
void svkPlotGridView::SetOrientation( svkDcmHeader::Orientation orientation )
{
    this->orientation = orientation;
    if( this->dataVector[MRS] != NULL ) {
        int toggleDraw = this->GetRenderer( svkPlotGridView::PRIMARY )->GetDraw();
        if( toggleDraw ) {
            this->GetRenderer( svkPlotGridView::PRIMARY)->DrawOff();
        }
        this->UpdateMetaboliteTextDisplacement();
        this->satBands->SetOrientation( this->orientation );
        this->plotGrid->SetOrientation( this->orientation );
        this->SetTlcBrc( this->plotGrid->GetTlcBrc()[0], this->plotGrid->GetTlcBrc()[1] );
        if( toggleDraw ) {
            this->GetRenderer( svkPlotGridView::PRIMARY)->DrawOn();
        }
        this->SetSlice( this->plotGrid->GetSlice() );
        this->Refresh();
    }
}

