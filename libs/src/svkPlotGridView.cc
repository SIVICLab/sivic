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


#include <svkPlotGridView.h>
#include <svkTypeUtils.h>


using namespace svk;


//vtkCxxRevisionMacro(svkPlotGridView, "$Rev$");
vtkStandardNewMacro(svkPlotGridView);


const double svkPlotGridView::CLIP_TOLERANCE = 0.001;


//! Constructor 
svkPlotGridView::svkPlotGridView()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    this->plotGrids.push_back(svkPlotLineGrid::New());
    this->orientation = svkDcmHeader::AXIAL;
    this->windowLevel = svkImageMapToColors::New();
    this->overlayTextDigits = 0;

    // Setting layers is key for solaris rendering

    this->slice = 0;
    this->rwi = NULL;
    this->plotUnitType = svkSpecPoint::PTS;
    this->dataVector.push_back(NULL);
    this->dataVector.push_back(NULL);
    this->dataVector.push_back(NULL);

    this->isPropOn.assign(svkPlotGridView::LAST_PROP+1, false);
    this->isRendererOn.assign(svkPlotGridView::LAST_RENDERER+1, false);
    this->isPropVisible.assign(svkPlotGridView::LAST_PROP+1, false);     //Is the actor in the views FOV?

    vtkRenderer* nullRenderer = NULL;
    this->renCollection.assign(svkPlotGridView::LAST_RENDERER+1, nullRenderer);

    vtkProp* nullProp = NULL;
    this->propCollection.assign(svkPlotGridView::LAST_PROP+1, nullProp);     //Is the actor in the views FOV?n
   
    vtkActor* entirePlotGrid = vtkActor::New();
    this->SetProp( svkPlotGridView::PLOT_GRID, entirePlotGrid );
    entirePlotGrid->Delete();

    this->SetProp( svkPlotGridView::VOL_SELECTION, NULL );
    this->SetProp( svkPlotGridView::PLOT_LINES, this->plotGrids[0]->GetPlotGridActor()  );

    this->detailedPlotDirector = svkDetailedPlotDirector::New();
    this->SetProp( svkPlotGridView::DETAILED_PLOT, this->detailedPlotDirector->GetPlotActor());
    this->TurnPropOn( svkPlotGridView::DETAILED_PLOT );
    this->SetProp( svkPlotGridView::RULER, this->detailedPlotDirector->GetRuler());
    this->TurnPropOn( svkPlotGridView::RULER );

    svkOrientedImageActor* overlayActor = svkOrientedImageActor::New();
    this->SetProp( svkPlotGridView::OVERLAY_IMAGE, overlayActor );
    this->TurnPropOff( svkPlotGridView::OVERLAY_IMAGE );
	vtkActor2D* metActor = vtkActor2D::New();
	this->SetProp( svkPlotGridView::OVERLAY_TEXT, metActor );
	metActor->Delete();
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
    //white
    this->referencePlotColors[0][0] = 1;
    this->referencePlotColors[0][1] = 1;
    this->referencePlotColors[0][2] = 1;
    //magenta
    this->referencePlotColors[1][0] = 1;
    this->referencePlotColors[1][1] = 0;
    this->referencePlotColors[1][2] = 1;
    //cyan
    this->referencePlotColors[2][0] = 0;
    this->referencePlotColors[2][1] = 1;
    this->referencePlotColors[2][2] = 1;
    //orange
    this->referencePlotColors[3][0] = 1;
    this->referencePlotColors[3][1] = 0.85;
    this->referencePlotColors[3][2] = 0;
    //green
    this->referencePlotColors[4][0] = 0.15;
    this->referencePlotColors[4][1] = 1;
    this->referencePlotColors[4][2] = 0.15;
    //grey
    this->referencePlotColors[5][0] = 0.75;
    this->referencePlotColors[5][1] = 0.75;
    this->referencePlotColors[5][2] = 0.75;
    this->numColors = 6;
    
    this->activePlot = 0;

    this->alignCamera = true; 
    
}


//! Destructor
svkPlotGridView::~svkPlotGridView()
{
    if( this->rwi != NULL ) {
        this->rwi->Delete();
        this->rwi = NULL;
    }
    for( vector<svkPlotLineGrid*>::iterator iter = this->plotGrids.begin();
        iter != this->plotGrids.end(); ++iter) {
        if( (*iter) != NULL ) {
            (*iter)->Delete();
            (*iter) = NULL;
        }
    }

    if( this->colorTransfer != NULL ) {
        this->colorTransfer->Delete();
        this->colorTransfer = NULL;
    }
    if( this->satBands != NULL ) {
        this->satBands->Delete();
        this->satBands = NULL;
    }

    if( this->detailedPlotDirector != NULL ) {
        this->detailedPlotDirector->Delete();
        this->detailedPlotDirector = NULL;
    }

    if( this->windowLevel != NULL ) {
        this->windowLevel->Delete();
        this->windowLevel = NULL;
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
        if( index == MR4D ) {
        	// If the user specifies just MR4D as input we'll assume they
        	// want to changet the active plot.
            if( this->activePlot != 0 ) {
                this->SetInput( data, this->activePlot + 1);
                return;
            }
            while( this->volumeIndexVector.size() < svk4DImageData::SafeDownCast(data)->GetNumberOfVolumeDimensions()) {
                this->volumeIndexVector.push_back(0);
            }
            if( dataVector[MR4D] != NULL  ) {
                (dataVector[MR4D])->Delete();
            }
            ObserveData( data );
            data->Register( this );
            dataVector[MR4D] = data;
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
            if( this->GetProp( PLOT_LINES ) != NULL ) {
                if( this->GetRenderer(svkPlotGridView::PRIMARY)->HasViewProp( this->GetProp( PLOT_LINES ) ) ) {
                    this->GetRenderer(svkPlotGridView::PRIMARY)->RemoveViewProp( this->GetProp( PLOT_LINES ) );
                }
            }
            this->plotGrids[0]->SetInput(svk4DImageData::SafeDownCast(data));
            this->GeneratePlotGridActor();
            this->TurnPropOn( PLOT_LINES );
            this->TurnPropOn( svkPlotGridView::DETAILED_PLOT );
            this->SetProp( svkPlotGridView::PLOT_LINES, this->plotGrids[0]->GetPlotGridActor()  );
            this->GetRenderer( svkPlotGridView::PRIMARY)->AddActor( this->GetProp( svkPlotGridView::PLOT_LINES ) );
            if( data->IsA("svkMrsImageData")) {
                if( this->dataVector[MR4D]->GetNumberOfCells() == 1 && svkMrsImageData::SafeDownCast(data)->HasSelectionBox()) {
                    this->TurnPropOff( svkPlotGridView::PLOT_GRID );
                } else {
                    this->TurnPropOn( svkPlotGridView::PLOT_GRID );
                }
            }
            if( this->GetProp( VOL_SELECTION ) != NULL ) {
                if( this->GetRenderer(svkPlotGridView::PRIMARY)->HasViewProp( this->GetProp( VOL_SELECTION ) ) ) {
                    this->GetRenderer(svkPlotGridView::PRIMARY)->RemoveViewProp( this->GetProp( VOL_SELECTION ) );
                }
            }
            int minIndex[3] = {0,0,0};
            int maxIndex[3] = {this->dataVector[MR4D]->GetDimensions()[0]-2,this->dataVector[MR4D]->GetDimensions()[1]-2, this->slice};
            this->tlcBrc[0] = 0;
            this->tlcBrc[1] = svk4DImageData::SafeDownCast(this->dataVector[MR4D])->GetIDFromIndex( maxIndex[0], maxIndex[1], maxIndex[2] );
            if( this->dataVector[MR4D]->IsA("svkMrsImageData")) {
                svkMrsTopoGenerator* topoGenerator = svkMrsTopoGenerator::New(); 
                vtkActorCollection* selectionBoxTopology = topoGenerator->GetTopoActorCollection( dataVector[MR4D], svk4DImageData::VOL_SELECTION);
                topoGenerator->Delete();
                // Case for no selection Box
                if( selectionBoxTopology != NULL ) {
                    selectionBoxTopology->InitTraversal();
                    this->SetProp( VOL_SELECTION , selectionBoxTopology->GetNextActor());
                    if( this->dataVector[MR4D]->GetNumberOfCells() != 1 ) {
                        this->GetRenderer(svkPlotGridView::PRIMARY)->AddActor( this->GetProp( VOL_SELECTION ) );
                    }
                    selectionBoxTopology->Delete();
                }
                this->HighlightSelectionVoxels();
                this->satBands->SetInput( static_cast<svkMrsImageData*>(this->dataVector[MR4D]) );
                this->satBands->SetClipSlice( this->slice );
                this->plotUnitType = svkSpecPoint::PPM;
            }
            this->SetSlice( slice );
            if( !this->GetRenderer( svkPlotGridView::PRIMARY)->HasViewProp( this->GetProp( svkPlotGridView::SAT_BANDS))) {
                this->GetRenderer( svkPlotGridView::PRIMARY)->AddActor(  this->GetProp( svkPlotGridView::SAT_BANDS));
            }
            if( !this->GetRenderer( svkPlotGridView::PRIMARY)->HasViewProp( this->GetProp( svkPlotGridView::SAT_BANDS_OUTLINE))) {
                this->GetRenderer( svkPlotGridView::PRIMARY)->AddActor(  this->GetProp( svkPlotGridView::SAT_BANDS_OUTLINE));
            }
            this->SetProp( svkPlotGridView::SAT_BANDS, this->satBands->GetSatBandsActor() );
            this->SetProp( svkPlotGridView::PLOT_LINES, this->plotGrids[0]->GetPlotGridActor()  );
            this->TurnPropOn( svkPlotGridView::PLOT_LINES );
            this->GetRenderer( svkPlotGridView::PRIMARY)->AddActor(  this->GetProp( svkPlotGridView::DETAILED_PLOT));
            this->GetRenderer( svkPlotGridView::PRIMARY)->AddActor(  this->GetProp( svkPlotGridView::RULER));
            this->TurnPropOn( svkPlotGridView::VOL_SELECTION );
            this->SetOrientation( this->orientation );
            this->AlignCamera(); 
            this->UpdateDetailedPlot( this->tlcBrc );

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
            this->SetTlcBrc(this->tlcBrc);
        } else if( index >= ADDITIONAL_MR4D ) {
            while( dataVector.size() <= index ) {
                dataVector.push_back(NULL);
            }
            if( dataVector[index] != NULL  ) {
                (dataVector[index])->Delete();
            }
            bool toggleDraw = this->GetRenderer( svkPlotGridView::PRIMARY )->GetDraw();
            if( toggleDraw ) {
                this->GetRenderer( svkPlotGridView::PRIMARY )->DrawOff();
            }
            ObserveData( data );
            data->Register( this );
            dataVector[index] = data;
            while( plotGrids.size() < index ) {
                plotGrids.push_back( svkPlotLineGrid::New() );
            }
            plotGrids[index-1]->SetInput(svk4DImageData::SafeDownCast(data));
            int minFreq;
            int maxFreq;
            plotGrids[0]->GetFrequencyWLRange(minFreq, maxFreq);
            plotGrids[index-1]->SetFrequencyWLRange(minFreq, maxFreq, this->tlcBrc);
            double minInt;
            double maxInt;
            plotGrids[0]->GetIntensityWLRange(minInt, maxInt);
            plotGrids[index-1]->SetIntensityWLRange(minInt, maxInt, this->tlcBrc);
            plotGrids[index-1]->SetComponent( plotGrids[0]->GetComponent() );
            plotGrids[index-1]->SetVolumeIndexVector( plotGrids[0]->GetVolumeIndexVector() );
            int colorIndex = index-1;
            while( colorIndex >= this->numColors ) {
                colorIndex -= this->numColors;
            }

            plotGrids[index-1]->SetColor( this->referencePlotColors[colorIndex] );
            if( !this->GetRenderer(svkPlotGridView::PRIMARY)->HasViewProp( this->plotGrids[index-1]->GetPlotGridActor() ) ) {
                this->GetRenderer(svkPlotGridView::PRIMARY)->AddActor( this->plotGrids[index-1]->GetPlotGridActor() );
            }

            this->SetSlice( slice );
            this->SetOrientation( this->orientation );
            if( toggleDraw ) {
                this->GetRenderer( svkPlotGridView::PRIMARY )->DrawOn();
            }

            this->AlignCamera(); 
            this->Refresh();
            // We need to update after the refresh because some initialization occurs during the rendering in vtkXYPlotActor
            this->UpdateDetailedPlot( this->tlcBrc );

        }

    }
}

void svkPlotGridView::AddReferenceInput( svkImageData* data )
{
    this->SetInput( data, ADDITIONAL_MR4D + (plotGrids.size()-1));
}

/*!
 * Removes a data input and the associated actors. Currently only implemented for overlay.
 */
void svkPlotGridView::RemoveInput(int index)
{
    if( index == MET ) {
        Superclass::RemoveInput(index);
        if( this->GetProp( svkPlotGridView::OVERLAY_TEXT) != NULL ) {
            vtkRenderer* ren = this->GetRenderer( svkPlotGridView::PRIMARY );
            if( ren->HasViewProp( this->GetProp( svkPlotGridView::OVERLAY_TEXT )) ) {
                ren->RemoveActor(this->GetProp( svkPlotGridView::OVERLAY_TEXT) );
            }
			vtkActor2D* metActor = vtkActor2D::New();
			this->SetProp( svkPlotGridView::OVERLAY_TEXT, metActor );
			metActor->Delete();
        }

        if( this->GetProp( svkPlotGridView::OVERLAY_IMAGE) != NULL ) {
            vtkRenderer* ren = this->GetRenderer( svkPlotGridView::PRIMARY );
            if( ren->HasViewProp( this->GetProp( svkPlotGridView::OVERLAY_IMAGE )) ) {
                ren->RemoveActor(this->GetProp( svkPlotGridView::OVERLAY_IMAGE) );
            }
            svkOrientedImageActor* overlayActor = svkOrientedImageActor::New();
            this->SetProp( svkPlotGridView::OVERLAY_IMAGE, overlayActor );
            overlayActor->InterpolateOff();
            overlayActor->Delete();
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
    if( this->dataVector[MR4D] != NULL ) {
        int tlcIndex[3];
        int brcIndex[3];
        this->dataVector[MR4D]->GetIndexFromID( tlcBrc[0], tlcIndex );
        this->dataVector[MR4D]->GetIndexFromID( tlcBrc[1], brcIndex );
        int lastSlice  = dataVector[MR4D]->GetLastSlice( this->orientation );
        int firstSlice = dataVector[MR4D]->GetFirstSlice( this->orientation );
        slice = (slice > lastSlice) ? lastSlice:slice;
        slice = (slice < firstSlice) ? firstSlice:slice;
        this->slice = slice;
        tlcIndex[ this->dataVector[MR4D]->GetOrientationIndex( this->orientation ) ] = slice;
        brcIndex[ this->dataVector[MR4D]->GetOrientationIndex( this->orientation ) ] = slice;
        tlcBrc[0] = this->dataVector[MR4D]->GetIDFromIndex( tlcIndex[0], tlcIndex[1], tlcIndex[2] );
        tlcBrc[1] = this->dataVector[MR4D]->GetIDFromIndex( brcIndex[0], brcIndex[1], brcIndex[2] );
        if( this->dataVector[MR4D]->IsA("svkMrsImageData")){
            this->satBands->SetClipSlice( this->slice );
                // Case for no selection box
            if( this->GetProp( VOL_SELECTION ) != NULL ) {
                if( svkMrsImageData::SafeDownCast( this->dataVector[MR4D])
                          ->IsSliceInSelectionBox( this->slice, this->orientation ) ) {
                    if( !this->GetRenderer( svkPlotGridView::PRIMARY)->HasViewProp( this->GetProp( svkPlotGridView::VOL_SELECTION) )
                         &&  this->dataVector[MR4D]->GetNumberOfCells() != 1) {
                        this->GetRenderer( svkPlotGridView::PRIMARY)->AddActor(
                                       this->GetProp( svkPlotGridView::VOL_SELECTION) );
                    }
                } else {
                    if( this->GetRenderer( svkPlotGridView::PRIMARY)
                                ->HasViewProp( this->GetProp( svkPlotGridView::VOL_SELECTION) ) ) {
                        this->GetRenderer( svkPlotGridView::PRIMARY)
                                ->RemoveActor(this->GetProp( svkPlotGridView::VOL_SELECTION) );
                    }
                }
            }
        }

    }
    
    for( vector<svkPlotLineGrid*>::iterator iter = this->plotGrids.begin();
        iter != this->plotGrids.end(); ++iter) {
        if( (*iter) != NULL ) {
            (*iter)->SetSlice(slice);
        }
    }
    this->UpdateDetailedPlot( this->tlcBrc );
    this->AlignCamera();
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
    if( svkDataView::IsTlcBrcWithinData(this->dataVector[MR4D],tlcID, brcID ) ) {
        this->tlcBrc[0] = tlcID;
        this->tlcBrc[1] = brcID;
        int toggleDraw = this->GetRenderer( svkPlotGridView::PRIMARY )->GetDraw();
        if( toggleDraw ) {
            this->GetRenderer( svkPlotGridView::PRIMARY )->DrawOff( );
        }
        for( vector<svkPlotLineGrid*>::iterator iter = this->plotGrids.begin();
            iter != this->plotGrids.end(); ++iter) {
            if( (*iter) != NULL ) {
                (*iter)->SetTlcBrc(this->tlcBrc);
                (*iter)->Update(tlcBrc);
            }
        }
        this->UpdateMetaboliteText(tlcBrc);
        this->UpdateMetaboliteImage(tlcBrc);
        this->GenerateClippingPlanes();
        if( tlcID == brcID ) {
            this->UpdateDetailedPlot( this->tlcBrc );
            for( vector<svkPlotLineGrid*>::iterator iter = this->plotGrids.begin();
                iter != this->plotGrids.end(); ++iter) {
                if( (*iter) != NULL ) {
                    if(this->GetRenderer(svkPlotGridView::PRIMARY)->HasViewProp( (*iter)->GetPlotGridActor() ) ) {
                        this->GetRenderer(svkPlotGridView::PRIMARY)->RemoveViewProp( (*iter)->GetPlotGridActor() );
                    }
                }
            }
            this->TurnPropOff( svkPlotGridView::PLOT_GRID);
            this->TurnPropOn( svkPlotGridView::DETAILED_PLOT);
            this->TurnPropOn( svkPlotGridView::RULER);
            if( this->GetRenderer(svkPlotGridView::PRIMARY)->HasViewProp( this->GetProp( VOL_SELECTION ) ) ) {
                this->GetRenderer(svkPlotGridView::PRIMARY)->RemoveViewProp( this->GetProp( VOL_SELECTION ) );
            }
            if( this->GetRenderer(svkPlotGridView::PRIMARY)->HasViewProp( this->GetProp( OVERLAY_IMAGE ) ) ) {
                this->GetRenderer(svkPlotGridView::PRIMARY)->RemoveViewProp( this->GetProp( OVERLAY_IMAGE ) );
            }
            if( this->GetRenderer(svkPlotGridView::PRIMARY)->HasViewProp( this->GetProp( OVERLAY_TEXT ) ) ) {
                this->GetRenderer(svkPlotGridView::PRIMARY)->RemoveViewProp( this->GetProp( OVERLAY_TEXT ) );
            }
            this->detailedPlotDirector->AddOnMouseMoveObserver( this->rwi );
        } else {
            for( vector<svkPlotLineGrid*>::iterator iter = this->plotGrids.begin();
                iter != this->plotGrids.end(); ++iter) {
                if(!this->GetRenderer(svkPlotGridView::PRIMARY)->HasViewProp( (*iter)->GetPlotGridActor() ) ) {
                    this->GetRenderer(svkPlotGridView::PRIMARY)->AddViewProp( (*iter)->GetPlotGridActor() );
                }
            }
            this->detailedPlotDirector->RemoveOnMouseMoveObserver( this->rwi );
            if( !this->GetRenderer(svkPlotGridView::PRIMARY)->HasViewProp( this->GetProp( VOL_SELECTION ))
                 && this->dataVector[MR4D]->IsA("svkMrsImageData") ) {
                if( this->dataVector[MR4D]->GetNumberOfCells() != 1 && svkMrsImageData::SafeDownCast(this->dataVector[MR4D])->IsSliceInSelectionBox( this->slice, this->orientation ) ) {
                    this->GetRenderer(svkPlotGridView::PRIMARY)->AddViewProp( this->GetProp( VOL_SELECTION ) );
                }
            }
            if( !this->GetRenderer(svkPlotGridView::PRIMARY)->HasViewProp( this->GetProp( OVERLAY_IMAGE ) ) && this->dataVector[MET]!= NULL) {
                this->GetRenderer(svkPlotGridView::PRIMARY)->AddViewProp( this->GetProp( OVERLAY_IMAGE ) );
            }
            if( !this->GetRenderer(svkPlotGridView::PRIMARY)->HasViewProp( this->GetProp( OVERLAY_TEXT ) )
            		&& vtkActor2D::SafeDownCast(this->GetProp( OVERLAY_TEXT ))->GetMapper() != NULL ) {
                this->GetRenderer(svkPlotGridView::PRIMARY)->AddViewProp( this->GetProp( OVERLAY_TEXT ) );
            }
            this->TurnPropOn( svkPlotGridView::PLOT_GRID);
            this->TurnPropOff( svkPlotGridView::DETAILED_PLOT);
            this->TurnPropOff( svkPlotGridView::RULER);
        }
        this->AlignCamera(); 
        if( toggleDraw ) {
            this->GetRenderer( svkPlotGridView::PRIMARY )->DrawOn( );
        }
        
        this->Refresh();
    }
}

int svkPlotGridView::GetNumberOfReferencePlots( )
{
    return this->plotGrids.size() - 1;
}

/*!
 * Sets the color for the given plot index.
 */
void svkPlotGridView::SetPlotColor(int plotIndex, double* rgb )
{
    if( this->plotGrids[plotIndex] != NULL ) {
        this->plotGrids[plotIndex]->GetPlotGridActor()->GetProperty()->SetColor(rgb);
    }
    this->UpdateDetailedPlot(this->tlcBrc);
    this->Refresh();
}


/*!
 * Sets the color for the given plot index.
 */
double* svkPlotGridView::GetPlotColor(int plotIndex )
{
    if( this->plotGrids[plotIndex] != NULL ) {
        return this->plotGrids[plotIndex]->GetPlotGridActor()->GetProperty()->GetColor();
    } else {
        return NULL;
    }
}

void svkPlotGridView::SetPlotLineWidth( float width )
{
	for( vector<svkPlotLineGrid*>::iterator iter = this->plotGrids.begin();
		iter != this->plotGrids.end(); ++iter) {
		if( (*iter) != NULL ) {
			(*iter)->GetPlotGridActor()->GetProperty()->SetLineWidth( width);
			(*iter)->GetPlotGridActor()->Modified();
		}
	}
	this->detailedPlotDirector->SetLineWidth( width );

}

/*!
 *  Show or hide the given plot by index.
 * @param index
 * @param visible
 */
void svkPlotGridView::SetPlotVisibility( int plotIndex, bool visible )
{
    if( this->plotGrids[plotIndex] != NULL ) {
        this->plotGrids[plotIndex]->GetPlotGridActor()->SetVisibility(visible);
    }
    this->UpdateDetailedPlot(this->tlcBrc);
    this->Refresh();
}


/*!
 * Returns if the requested plot is visible, otherwise returns false.
 *
 * @param plotIndex
 * @return
 */
bool svkPlotGridView::GetPlotVisibility( int plotIndex )
{
    if( this->plotGrids[plotIndex] != NULL ) {
        return this->plotGrids[plotIndex]->GetPlotGridActor()->GetVisibility();
    } else {
       return false;
    }
}

/*!
 *  Sets the active plot.
 *
 * @param index
 */
void svkPlotGridView::SetActivePlotIndex( int index )
{
    this->activePlot = index;
    for( int i = 0; i < this->volumeIndexVector.size(); i++ ) {
        this->volumeIndexVector[i] = this->plotGrids[ index ]->GetVolumeIndex( i );
    }
    // Lets put this actor on top
    this->GetRenderer(svkPlotGridView::PRIMARY)->RemoveViewProp( this->plotGrids[index]->GetPlotGridActor());
    this->GetRenderer(svkPlotGridView::PRIMARY)->AddViewProp( this->plotGrids[index]->GetPlotGridActor());
    if( this->dataVector[MR4D]->IsA("svkMrsImageData")) {
        this->satBands->SetInput( svkMrsImageData::SafeDownCast(this->GetActivePlot()) );
    }
}


/*!
 *  Returns the svkImageData object of the active plot.
 *
 * @return
 */
svkImageData* svkPlotGridView::GetActivePlot( )
{
    if( this->activePlot == 0 ) {
        return this->dataVector[0];
    } else {
        return this->dataVector[ this->activePlot + 1];
    }
}


/*!
 * Gets the index of the plot that is currently active.
 * @return
 */
int svkPlotGridView::GetActivePlotIndex( )
{
    return this->activePlot;
}


/*!
 *
 * @param type
 */
void svkPlotGridView::SetPlotUnits( svkSpecPoint::UnitType plotUnitType )
{
    if( this->dataVector[MR4D] != NULL ) {
        this->detailedPlotDirector->GenerateAbscissa( this->dataVector[MR4D]->GetDcmHeader(), plotUnitType );
        this->plotUnitType = plotUnitType;
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
    this->GetRenderer( svkPlotGridView::PRIMARY)->GetActiveCamera()->ParallelProjectionOn();
    this->TurnRendererOn( svkPlotGridView::PRIMARY );
}


/*!
 *  SetWindowLevel for plot view;  index 0 is frequency, index 1 is intensity.
 *  NOTE: Method assumes that frequency ranges are in integers (points).
 *
 *  \param lower the lower limit
 *  \param upper the upper limit
 *  \param index which dimension you wish to change, frequency or magnitude 
 *
 */
void svkPlotGridView::SetWindowLevelRange( double lower, double upper, int index)
{
    if (index == FREQUENCY) {
        // Round doubles to int.
        int lowerInt = static_cast<int>(floor( lower + 0.5 ));
        int upperInt = static_cast<int>(floor( upper + 0.5 ));
        for( vector<svkPlotLineGrid*>::iterator iter = this->plotGrids.begin();
            iter != this->plotGrids.end(); ++iter) {
            if( (*iter) != NULL ) {
                (*iter)->SetFrequencyWLRange(lowerInt, upperInt, this->tlcBrc);
            }
        }
        this->detailedPlotDirector->SetIndexRange( lowerInt, upperInt );
    } else if (index == AMPLITUDE) {
        for( vector<svkPlotLineGrid*>::iterator iter = this->plotGrids.begin();
            iter != this->plotGrids.end(); ++iter) {
            if( (*iter) != NULL ) {
                (*iter)->SetIntensityWLRange(lower, upper, this->tlcBrc);
            }
        }
        this->detailedPlotDirector->SetYRange( lower, upper );
    }
    this->Refresh();
}


/*!
 *
 */
void svkPlotGridView::GetWindowLevelRange( double &lower, double &upper, int index)
{
    if (index == FREQUENCY) {
        int integerLower;
        int integerUpper;
        this->plotGrids[0]->GetFrequencyWLRange( integerLower, integerUpper );
        lower = integerLower;
        upper = integerUpper;
    } else if (index == AMPLITUDE) {
        this->plotGrids[0]->GetIntensityWLRange(lower, upper);
    }
}


/*!
 *  Method sets the window level range for the overlay.
 */
void svkPlotGridView::SetOverlayWLRange( double* range )
{
    if( this->colorTransfer != NULL ) {
        this->colorTransfer->SetRange(range);
        this->GetProp( svkPlotGridView::OVERLAY_IMAGE )->Modified();
        if( this->tlcBrc[0] == this->tlcBrc[1] && this->tlcBrc[0] != -1 ) {
            this->UpdateDetailedPlotOverlay( this->tlcBrc[0] );
        }
        this->Refresh();
    }
}


/*!
 *  Method gets the window level range for the overlay.
 */
double* svkPlotGridView::GetOverlayWLRange( )
{
    if( this->colorTransfer != NULL ) {
        return this->colorTransfer->GetRange(); 
    } else {
        return NULL;
    }
}


/*
 *  Set the component to display: 
 *
 *  \param component the compent you wish to display, REAL, IMAGINARY, MAGNITUDE
 */
void svkPlotGridView::SetComponent( svkPlotLine::PlotComponent component, int plotIndex)
{
    if( plotIndex != -1 ) {
        if( this->plotGrids[plotIndex] != NULL ) {
            this->plotGrids[plotIndex]->SetComponent( component );
        }
    } else {
        for( vector<svkPlotLineGrid*>::iterator iter = this->plotGrids.begin();
            iter != this->plotGrids.end(); ++iter) {
            if( (*iter) != NULL ) {
                (*iter)->SetComponent( component );
            }
        }
    }
    this->UpdateDetailedPlot( this->tlcBrc );
    this->Refresh();
}

/*!
 * Sets the component of the active plot.
 */
void svkPlotGridView::SetActiveComponent( svkPlotLine::PlotComponent component )
{
	this->SetComponent( component, this->activePlot );
}


/*!
 * Gets the component of the active plot.
 */
svkPlotLine::PlotComponent  svkPlotGridView::GetActiveComponent( )
{
	return this->plotGrids[ this->activePlot]->GetComponent();
}


/*!
 *  Sets desired the current selection in Display (pixels) coordinates
 *  and highlights the intersected voxels.
 *
 *  \param selectionArea the area you wish to select voxels within [xmin, xmax, ymin, ymax]
 */ 
void svkPlotGridView::SetSelection( double* selectionArea, bool isWorldCords )
{
    if( selectionArea != NULL && dataVector[MR4D] != NULL) {
        double worldStart[3]; 
        double worldEnd[3]; 
        bool tagVoxel = false;
        if( !isWorldCords ) {
        	if( svkVoxelTaggingUtils::IsImageVoxelTagData( this->dataVector[MET])
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
        svk4DImageData::SafeDownCast(this->dataVector[MR4D])->GetTlcBrcInUserSelection( tlcBrcImageData, selection, this->orientation, this->slice );
        if( tagVoxel && tlcBrcImageData[0] == tlcBrcImageData[1] ) {
        	svkVoxelTaggingUtils::ToggleVoxelTag(this->dataVector[MET], tlcBrcImageData[0]);
        } else {
			this->SetTlcBrc( tlcBrcImageData );
        }

    } 
    this->rwi->Render();
}


//! Method is called when data object is Modified. 
void svkPlotGridView::Refresh()
{
    if (this->GetDebug()) {
        cout << "svkPlotGridView::Refresh calls plotGrid->Update() first " << endl; 
    }
    if( tlcBrc[0] == this->tlcBrc[1] ) {
        this->detailedPlotDirector->Refresh();
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
    if( this->dataVector[MR4D] != NULL && this->dataVector[MR4D]->IsA("svkMrsImageData") ) {
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
 *  Creates the svkOrientedImageActor for rendering an overlay.
 *
 *  \param data the data to be rendered as an overlay
 *
 *  TODO: Implement changes for multiple overlays
 */
void svkPlotGridView::CreateMetaboliteOverlay( svkImageData* data )
{
	if( dataVector[MR4D] != NULL && data != NULL ) {
        double* spacing = data->GetSpacing();
        svkLabeledDataMapper* metMapper = svkLabeledDataMapper::New();
        // Check for voxel tag information
  	    if( svkVoxelTaggingUtils::IsImageVoxelTagData( data )) {
			metMapper->SetDisplayTags(true);
			metMapper->GetLabelTextProperty()->SetVerticalJustificationToTop();
			metMapper->GetLabelTextProperty()->SetJustificationToLeft();
  	    }

        svkImageClip* metTextClipper = svkImageClip::New();
        metTextClipper->SetInputData( this->dataVector[MET] );
        metMapper->SetInputConnection( metTextClipper->GetOutputPort());
        metMapper->SetLabelModeToLabelScalars();
        metMapper->SetLabeledComponent(0);
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
        this->SetOverlayTextDigits( this->overlayTextDigits );

        // If it has not been added, add it
        if( !this->GetRenderer( svkPlotGridView::PRIMARY )->HasViewProp( this->GetProp( svkPlotGridView::OVERLAY_TEXT )) ) {
            this->GetRenderer( svkPlotGridView::PRIMARY )->AddViewProp( this->GetProp( svkPlotGridView::OVERLAY_TEXT ));
        }
        this->UpdateMetaboliteTextDisplacement();
        this->UpdateMetaboliteText( tlcBrc );
        if( this->windowLevel != NULL ) {
            this->windowLevel->Delete();
        }
        this->windowLevel = svkImageMapToColors::New();
        metTextClipper->Update();
        this->windowLevel->SetInputData( this->dataVector[MET] );
        if( this->colorTransfer == NULL ) {
            this->colorTransfer = svkLookupTable::New();
        }

        double window;
        double level;
        svkMriImageData::SafeDownCast(data)->GetAutoWindowLevel(window, level);
        this->colorTransfer->SetRange( level - window/2.0, level + window/2.0);

        //this->colorTransfer->SetLUTType( svkLookupTable::GREY_SCALE );
        this->colorTransfer->SetLUTType( svkLookupTable::COLOR );
        this->colorTransfer->SetAlphaThreshold( 0.9 );

        this->windowLevel->SetLookupTable( this->colorTransfer );
        this->windowLevel->SetOutputFormatToRGBA( );
        this->windowLevel->Update( );


        svkOrientedImageActor::SafeDownCast(this->GetProp( svkPlotGridView::OVERLAY_IMAGE ))->GetMapper()->SetInputConnection(this->windowLevel->GetOutputPort());
        bool isOverlayImageOn = this->IsPropOn(svkPlotGridView::OVERLAY_IMAGE); 
        if( this->GetRenderer( svkPlotGridView::PRIMARY)->HasViewProp( this->GetProp( svkPlotGridView::OVERLAY_IMAGE) ) ) {
            this->GetRenderer( svkPlotGridView::PRIMARY)->RemoveActor( this->GetProp( svkPlotGridView::OVERLAY_IMAGE) );
        }
        this->GetRenderer( svkPlotGridView::PRIMARY)->AddActor( this->GetProp( svkPlotGridView::OVERLAY_IMAGE) );
        if( isOverlayImageOn ) {
            this->TurnPropOn( svkPlotGridView::OVERLAY_IMAGE );
        } else {
            this->TurnPropOff( svkPlotGridView::OVERLAY_IMAGE );
        }
        this->GetProp( svkPlotGridView::OVERLAY_IMAGE )->Modified();
        this->GetRenderer( svkPlotGridView::PRIMARY)->Render();
        this->UpdateMetaboliteText(this->tlcBrc);
        this->AlignCamera();
        this->SetOverlayOpacity(0.35);
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
    if( dataVector[MR4D] != NULL ) {
        int* extent = dataVector[MR4D]->GetExtent();
        int tlcIndex[3];
        int brcIndex[3];
        this->dataVector[MR4D]->GetIndexFromID( tlcBrc[0], tlcIndex );
        this->dataVector[MR4D]->GetIndexFromID( tlcBrc[1], brcIndex );
        for( vector<svkImageClip*>::iterator iter = metClippers.begin(); iter!= metClippers.end(); iter++ ) {
            (*iter)->SetOutputWholeExtent(tlcIndex[0], brcIndex[0], tlcIndex[1], brcIndex[1], tlcIndex[2], brcIndex[2]); 
            (*iter)->ClipDataOn();
        }
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
    if( dataVector[MR4D] != NULL ) {
        int minIndex[3] = {0,0,0};
        int maxIndex[3] = {this->dataVector[MR4D]->GetDimensions()[0]-2,this->dataVector[MR4D]->GetDimensions()[1]-2,this->dataVector[MR4D]->GetDimensions()[2]-2};
        int orientationIndex = this->dataVector[MR4D]->GetOrientationIndex( this->orientation );

        minIndex[ orientationIndex ] = this->slice;
        maxIndex[ orientationIndex ] = this->slice;
        int minID = this->dataVector[MR4D]->GetIDFromIndex( minIndex[0], minIndex[1], minIndex[2] );
        int maxID = this->dataVector[MR4D]->GetIDFromIndex( maxIndex[0], maxIndex[1], maxIndex[2] );

        this->dataVector[MR4D]->GetIndexFromID( tlcBrc[0], tlcIndex );
        this->dataVector[MR4D]->GetIndexFromID( tlcBrc[1], brcIndex );
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
    svkOrientedImageActor::SafeDownCast(this->GetProp( svkPlotGridView::OVERLAY_IMAGE ))->SetDisplayExtent(tlcIndex[0], brcIndex[0], tlcIndex[1], brcIndex[1], tlcIndex[2], brcIndex[2]);
    this->GetProp( svkPlotGridView::OVERLAY_IMAGE )->Modified();
}


/*!
 *  The actor that represents the text values of the metabolite overlays will,
 *  by default, render the text at the center of each voxel. We want the text
 *  to show up in the top left corner of each voxel. To do this we will
 *  apply a transform to the actor to translate it. 
 *   
 */
void svkPlotGridView::UpdateMetaboliteTextDisplacement() 
{
    if( this->dataVector[MET] != NULL ) {
        vtkTransform* optimus = vtkTransform::New();
        double lpsDisplacement[3] = {0,0,0};
        double sagittalDisplacementMagnitude = 0;
        double coronalDisplacementMagnitude = 0;
        double axialDisplacementMagnitude = 0;
        double horizontalVoxelFractionDisplacement = 1/2.1;
        double verticalVoxelFractionDisplacement   = 1/3.2;
        if( svkVoxelTaggingUtils::IsImageVoxelTagData( this->dataVector[MET])){ 
        	verticalVoxelFractionDisplacement = 1/2.4;
        }
        double inScreenVoxelFractionDisplacement      = 1/2.0;
        double* spacing = this->dataVector[MET]->GetSpacing();
       
        // Let's get the index of each orientation 
        int sagIndex = this->dataVector[MET]->GetOrientationIndex(svkDcmHeader::SAGITTAL);
        int corIndex = this->dataVector[MET]->GetOrientationIndex(svkDcmHeader::CORONAL);
        int axiIndex = this->dataVector[MET]->GetOrientationIndex(svkDcmHeader::AXIAL);

        /* 
         * Depending on how the data is being viewed the displacement will change
         * because the vertical and horizontal dimension may change. The reason
         * vertical and horizontal have different displacements is that the bottom
         * left corner of the text is centered within each voxel. As such we will
         * have to displace it more horizontally than vertically to account for the
         * height of the text.
         */
        
        switch( this->orientation ) {
            /*
             *  In the axial view the sagittal plane is perpendicular to 
             *  horizontal, and coronal is perpendicular to vertical
             */
            case svkDcmHeader::AXIAL:
                sagittalDisplacementMagnitude = spacing[sagIndex]*horizontalVoxelFractionDisplacement;
                coronalDisplacementMagnitude  = spacing[corIndex]*verticalVoxelFractionDisplacement;
                axialDisplacementMagnitude    = spacing[axiIndex]*inScreenVoxelFractionDisplacement;
                break;
            /*
             *  In the coronal view the sagittal plane is perpendicular to 
             *  horizontal, and axial is perpendicular to vertical
             */
            case svkDcmHeader::CORONAL:
                sagittalDisplacementMagnitude = spacing[sagIndex]*horizontalVoxelFractionDisplacement;
                coronalDisplacementMagnitude  = spacing[corIndex]*inScreenVoxelFractionDisplacement;
                axialDisplacementMagnitude    = spacing[axiIndex]*verticalVoxelFractionDisplacement;
                break;
            /*
             *  In the sagittal view the coronal plane is perpendicular to 
             *  horizontal, and axial is perpendicular to vertical
             */
            case svkDcmHeader::SAGITTAL:
                sagittalDisplacementMagnitude = spacing[sagIndex]*inScreenVoxelFractionDisplacement;
                coronalDisplacementMagnitude  = spacing[corIndex]*horizontalVoxelFractionDisplacement;
                axialDisplacementMagnitude    = spacing[axiIndex]*verticalVoxelFractionDisplacement;
                break;
        }

        /*
         * The sign of the displacement does not depend on the dcos, it
         * only depends on the absolute direction in LPS. 
         *
         * For example if we have two dataset with:
         *
         * dcos = 1 0 0
         *        0 1 0
         *        0 0 1
         * 
         * OR
         *
         * dcos = -1 0 0
         *        0 -1 0
         *        0 0 -1
         *
         * text should be displaced in exactly the same way. To account
         * for this we take magnitude of the displacement in each
         * orientation and set the direction.
         * 
         */
        double dcos[3][3];
        this->dataVector[MET]->GetDcos( dcos );
        if ( dcos[sagIndex][0] > 0 ) {
            sagittalDisplacementMagnitude *= -1;
        }
        if ( dcos[corIndex][1] > 0 ) {
            coronalDisplacementMagnitude *= -1;
        }
        if ( dcos[axiIndex][2] < 0 ) {
            axialDisplacementMagnitude *= -1;
        }

        lpsDisplacement[0] =  sagittalDisplacementMagnitude * dcos[sagIndex][0] 
                         + coronalDisplacementMagnitude * dcos[corIndex][0] 
                         + axialDisplacementMagnitude * dcos[axiIndex][0];

        lpsDisplacement[1] =  sagittalDisplacementMagnitude * dcos[sagIndex][1] 
                         + coronalDisplacementMagnitude * dcos[corIndex][1] 
                         + axialDisplacementMagnitude * dcos[axiIndex][1];

        lpsDisplacement[2] =  sagittalDisplacementMagnitude * dcos[sagIndex][2] 
                         + coronalDisplacementMagnitude * dcos[corIndex][2] 
                         + axialDisplacementMagnitude * dcos[axiIndex][2];
        
        // Now lets apply the transform...
        optimus->Translate( lpsDisplacement );
        svkLabeledDataMapper::SafeDownCast( 
                vtkActor2D::SafeDownCast( 
                    this->GetProp( svkPlotGridView::OVERLAY_TEXT ) )->GetMapper())->SetTransform(optimus);
        optimus->Delete();    

    }
}


/*!
 *
 * @param tlcBrc
 */
void svkPlotGridView::UpdateDetailedPlot( int* tlcBrc )
{
    if( this->dataVector[MR4D] != NULL ) {
        int counter = 0;
        this->detailedPlotDirector->RemoveAllInputs();
        int voxelIndex[3] = {0, 0, 0};
        this->dataVector[MR4D]->GetIndexFromID( tlcBrc[0], voxelIndex );
        for( vector<svkPlotLineGrid*>::iterator iter = this->plotGrids.begin();
            iter != this->plotGrids.end(); ++iter) {
            if( (*iter) != NULL && (*iter)->GetPlotGridActor()->GetVisibility() ) {
                vtkDataArray* spectrum = (*iter)->GetInput()->GetArray( voxelIndex[0], voxelIndex[1], voxelIndex[2], &this->volumeIndexVector[0]);
                this->detailedPlotDirector->AddInput( spectrum , (*iter)->GetComponent(), (*iter)->GetInput());
                this->detailedPlotDirector->SetPlotColor(counter, (*iter)->GetColor());
                counter++;
            }
        }
        this->detailedPlotDirector->GenerateAbscissa( this->dataVector[MR4D]->GetDcmHeader() , this->plotUnitType );
        int lower;
        int upper;
        this->plotGrids[0]->GetFrequencyWLRange(lower, upper);
        this->detailedPlotDirector->SetIndexRange(lower, upper);
        this->UpdateDetailedPlotOverlay(tlcBrc[0]);
    }

}


/*!
 * Updates the overlay color and text in the detailed plot director
 */
void svkPlotGridView::UpdateDetailedPlotOverlay( int tlcID )
{
    svkOrientedImageActor* overlayActor = svkOrientedImageActor::SafeDownCast(this->GetProp( svkPlotGridView::OVERLAY_IMAGE ));
    if( this->dataVector[MET] != NULL && overlayActor != NULL && this->windowLevel->GetInput() != NULL ) {
        this->detailedPlotDirector->SetBackgroundOpacity( overlayActor->GetOpacity());
        double value = this->dataVector[MET]->GetPointData()->GetScalars()->GetTuple1(tlcID);
        unsigned char* voxelColor = this->colorTransfer->MapValue(value);
        double color[3] = {voxelColor[0]/255.0, voxelColor[1]/255.0, voxelColor[2]/255.0};
        this->detailedPlotDirector->SetBackgroundColor(color);
        // Check to see if the voxel was thresholded
        if( voxelColor[3] == 0 ) {
            this->detailedPlotDirector->SetBackgroundOpacity(0);
        } else {
            this->detailedPlotDirector->SetBackgroundOpacity(overlayActor->GetOpacity());
        }
        this->detailedPlotDirector->SetAnnotationText(svkTypeUtils::DoubleToString(value));
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
    double textColor[3];
    if( colorSchema == svkPlotGridView::LIGHT_ON_DARK ) {
        backgroundColor[0] = 0;
        backgroundColor[1] = 0;
        backgroundColor[2] = 0;
        foregroundColor[0] = 1;
        foregroundColor[1] = 1;
        foregroundColor[2] = 1;
        textColor[0]       = 0;
        textColor[1]       = 1;
        textColor[2]       = 1;
    } else if ( colorSchema == svkPlotGridView::DARK_ON_LIGHT ) {
        backgroundColor[0] = 1;
        backgroundColor[1] = 1;
        backgroundColor[2] = 1;
        foregroundColor[0] = 0;
        foregroundColor[1] = 0;
        foregroundColor[2] = 0;
        textColor[0]       = 0;
        textColor[1]       = 0;
        textColor[2]       = 0;

    } 
    this->GetRenderer( svkPlotGridView::PRIMARY )->SetBackground( backgroundColor );
    this->plotGrids[0]->GetPlotGridActor()->GetProperty()->SetColor( foregroundColor );
    this->detailedPlotDirector->SetBackgroundColor(backgroundColor);
    this->detailedPlotDirector->SetPlotColor( 0, foregroundColor );
    if( vtkActor2D::SafeDownCast( this->GetProp( svkPlotGridView::OVERLAY_TEXT ))->GetMapper() != NULL ) {
        svkLabeledDataMapper::SafeDownCast(
                vtkActor2D::SafeDownCast( this->GetProp( svkPlotGridView::OVERLAY_TEXT ))->GetMapper())
            ->GetLabelTextProperty()->SetColor(textColor);
    }
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
    if ( data == NULL || targetIndex < 0 ) {
        resultInfo += "Data incompatible-- NULL or outside of input range!\n";

    } else if( targetIndex == MET ) {
        if( dataVector[MR4D] != NULL ) {
            //cout << "PLOT GRID VIEW VALIDATOR 1" << endl;
            bool valid = validator->AreDataCompatible( data, dataVector[MR4D] );
            if( validator->IsInvalid( svkDataValidator::INVALID_DATA_ORIENTATION ) ) {
                cout << "WARNING, reformatting images to spectroscopic orientation" << endl; 
                resultInfo = "";
                this->ResliceImage( data, dataVector[MR4D] );
            } else if( !valid && this->isValidationOn ) {
                resultInfo += validator->resultInfo.c_str(); 
                resultInfo += "\n"; 
            } 

            bool geometriesMatch = validator->AreDataGeometriesSame( data, dataVector[MR4D] );
            if( !geometriesMatch ) {
                resultInfo += validator->resultInfo.c_str();
                resultInfo += "\n";
            }
        } else {
            resultInfo += "4D data must be loaded before overlays!\n";
        } 
    } else if( targetIndex == MR4D ) {
        if( dataVector[MET] != NULL ) {
            //cout << "PLOT GRID VIEW VALIDATOR 2" << endl;
            bool valid = validator->AreDataCompatible( data, dataVector[MET] );  
            if( validator->IsInvalid( svkDataValidator::INVALID_DATA_ORIENTATION ) ) {
                cout << "WARNING, reformatting images to spectroscopic orientation" << endl; 
                resultInfo = "";
                this->ResliceImage( dataVector[MET], data );
            } else if( !valid ) {
                resultInfo += validator->resultInfo.c_str(); 
                resultInfo += "\n"; 
            }
        }
    } else if( targetIndex >= ADDITIONAL_MR4D ) {
        if( dataVector[MR4D] != NULL ) {
            bool valid = validator->AreDataCompatible( data, dataVector[MR4D] );
            if( !valid ) {
                resultInfo += validator->resultInfo.c_str();
            }
            valid = validator->AreDataGeometriesSame( data, dataVector[MR4D] );
            if( !valid ) {
                resultInfo += validator->resultInfo.c_str();
            }
            bool cellDataArraysMatch = validator->AreCellDataArrayStructureSame( data, dataVector[MR4D] );
            if( !cellDataArraysMatch ) {
                resultInfo += validator->resultInfo.c_str();
                resultInfo += "\n";
            }

        } else {
            resultInfo += "4D data must be loaded before overlays!\n";
        }
    } else {
        resultInfo += "Unrecognized data type!\n";
    }

    cout << resultInfo.c_str() << endl;
    validator->Delete();
    return resultInfo; 
}


/*!
 *  Reslice images to MR4D orientation
 */
void svkPlotGridView::ResliceImage(svkImageData* input, svkImageData* target)
{
    //  if (orthogonal orientations) {
    svkObliqueReslice* reslicer = svkObliqueReslice::New();
    reslicer->SetInputData( input );
    reslicer->SetTarget( target );
    reslicer->Update();
    this->SetInput( reslicer->GetOutput(), MET );
    //}
}


/*!
 * Set the index for the given volume dimension index.
 *
 * @param index the index of the volume
 * @param volumeIndex the index of the volume dimension
 * @param plotIndex the index of the plot line you wish to modify
 */
void svkPlotGridView::SetVolumeIndex( int index, int volumeIndex, int plotIndex )
{
    if( plotIndex == 0 ) {
        this->volumeIndexVector[ volumeIndex ] = index;
    }
    if( plotIndex != -1 ) {
        if( this->plotGrids[plotIndex] != NULL ) {
            this->plotGrids[plotIndex]->SetVolumeIndex( index, volumeIndex );
        }
    } else {
        this->volumeIndexVector[ volumeIndex ] = index;
        for( vector<svkPlotLineGrid*>::iterator iter = this->plotGrids.begin();
            iter != this->plotGrids.end(); ++iter) {
            if( (*iter) != NULL ) {
                (*iter)->SetVolumeIndex( index, volumeIndex);
            }
        }
    }
    if( this->tlcBrc[0] == this->tlcBrc[1] && this->tlcBrc[0] != -1 ) {
        this->UpdateDetailedPlot(this->tlcBrc);
    }
    this->Refresh();

    this->rwi->InvokeEvent(vtkCommand::SelectionChangedEvent);
    this->Refresh();

}


/*!
 * Getter for the index of current volume.
 *
 * @param volumeIndex
 * @return
 */
int  svkPlotGridView::GetVolumeIndex( int volumeIndex  )
{
    return this->volumeIndexVector[ volumeIndex ];
}

/*!
 * Getter for the index of current volume.
 *
 * @param volumeIndex
 * @return
 */
int*  svkPlotGridView::GetVolumeIndexArray( )
{
    return &this->volumeIndexVector[0];
}


/*!
 *  Sets the opacity of the image overlay.
 *
 *   \param opacity the new opacity you wish the image overlay to have. 
 */
void svkPlotGridView::SetOverlayOpacity( double opacity )
{
    if( this->dataVector[MET] != NULL ) {
        svkOrientedImageActor::SafeDownCast(this->GetProp( svkPlotGridView::OVERLAY_IMAGE ))->SetOpacity( opacity );
        this->detailedPlotDirector->SetBackgroundOpacity( opacity );
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
        if( this->tlcBrc[0] == this->tlcBrc[1] && this->tlcBrc[0] != -1) {
            this->UpdateDetailedPlotOverlay(this->tlcBrc[0]);
        }
    }
}


void svkPlotGridView::SetLUT( svkLookupTable::svkLookupTableType type )
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
	this->colorTransfer->SetLUTType( type );
	this->colorTransfer->SetAlphaThreshold( threshold );
    if(this->dataVector[MET] != NULL ) {
		double window;
		double level;
		svkMriImageData::SafeDownCast(this->dataVector[MET])->GetAutoWindowLevel(window, level);
		this->colorTransfer->SetRange( level - window/2.0, level + window/2.0);
		this->windowLevel->SetLookupTable( this->colorTransfer );
    }
	this->Refresh();

}


/*!
 * Hides all the actors.
 */
void svkPlotGridView::HideView()
{
    this->GetRenderer(svkPlotGridView::PRIMARY)->RemoveViewProp( this->GetProp(svkPlotGridView::PLOT_LINES) );
    this->GetRenderer(svkPlotGridView::PRIMARY)->RemoveViewProp( this->GetProp(svkPlotGridView::VOL_SELECTION) );
    this->GetRenderer(svkPlotGridView::PRIMARY)->RemoveViewProp( this->GetProp(svkPlotGridView::PLOT_GRID) );
    this->GetRenderer(svkPlotGridView::PRIMARY)->RemoveViewProp( this->GetProp(svkPlotGridView::OVERLAY_IMAGE));
    this->GetRenderer(svkPlotGridView::PRIMARY)->RemoveViewProp( this->GetProp(svkPlotGridView::OVERLAY_TEXT));
    this->GetRenderer(svkPlotGridView::PRIMARY)->RemoveViewProp( this->GetProp(svkPlotGridView::DETAILED_PLOT));

}


/*!
 * Reveals all the appropriate actors.
 */
void svkPlotGridView::ShowView()
{
    string acquisitionType = "UNKNOWN";
    svk4DImageData* activeData = this->GetActiveInput();
    if( activeData != NULL && activeData->IsA("svkMrsImageData") ) {
        acquisitionType = activeData->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
    }
    // We don't want to turn on the press box if its single voxel
    if( this->dataVector[MR4D]->GetNumberOfCells() != 1) {
        this->GetRenderer(svkPlotGridView::PRIMARY)->AddActor(
                                       this->GetProp(svkPlotGridView::VOL_SELECTION));
        this->GetRenderer(svkPlotGridView::PRIMARY)->AddActor( this->GetProp(svkPlotGridView::PLOT_LINES));
    }

        this->GetRenderer(svkPlotGridView::PRIMARY)->AddActor(
                                           this->GetProp(svkPlotGridView::PLOT_GRID));
        this->GetRenderer(svkPlotGridView::PRIMARY)->AddActor(
                                           this->GetProp(svkPlotGridView::OVERLAY_IMAGE));
        this->GetRenderer(svkPlotGridView::PRIMARY)->AddActor(
                                           this->GetProp(svkPlotGridView::OVERLAY_TEXT));
        this->GetRenderer(svkPlotGridView::PRIMARY)->AddActor(
                this->GetProp(svkPlotGridView::DETAILED_PLOT));
    if( this->tlcBrc[0] != this->tlcBrc[1] ) {
        this->TurnPropOff(svkPlotGridView::DETAILED_PLOT);

    }

}


/*!
 * Sets the active overlay volume.
 *
 * @param volume
 */
void svkPlotGridView::SetActiveOverlayVolume( int volume )
{
    if( this->dataVector[MET] != NULL && volume < this->dataVector[MET]->GetPointData()->GetNumberOfArrays()) {
        this->dataVector[MET]->GetPointData()->SetActiveScalars( this->dataVector[MET]->GetPointData()->GetArray(volume)->GetName() );
        this->windowLevel->Modified();
        if( this->tlcBrc[0] == this->tlcBrc[1] && this->tlcBrc[0] != -1 ) {
            this->UpdateDetailedPlotOverlay(this->tlcBrc[0]);
        }
        this->Refresh( );
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

    // We'll need a poly data mapper
    vtkPolyDataMapper* entireGridMapper = vtkPolyDataMapper::New();
    // This will be our grid
    vtkPolyData* grid = vtkPolyData::New();
    svk4DImageData::SafeDownCast( this->dataVector[MR4D] )->GetPolyDataGrid( grid );
    vtkCleanPolyData* cleaner = vtkCleanPolyData::New();
    cleaner->SetInputData( grid );
    cleaner->Update();
    grid->Delete();

    entireGridMapper->SetInputData( cleaner->GetOutput() );
    entireGridMapper->Update();
    cleaner->Delete();
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
    if( this->dataVector[MR4D] != NULL ) {
        string acquisitionType;
        if( this->dataVector[MR4D]->IsA("svkMrsImageData")) {
            acquisitionType = this->dataVector[MR4D]->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
        }
        if( this->dataVector[MR4D]->GetNumberOfCells() != 1) {
            this->ClipMapperToTlcBrc( dataVector[MR4D],
                                 vtkActor::SafeDownCast( this->GetProp( svkPlotGridView::PLOT_GRID ))->GetMapper(), tlcBrc, CLIP_TOLERANCE, CLIP_TOLERANCE, CLIP_TOLERANCE );
            for( vector<svkPlotLineGrid*>::iterator iter = this->plotGrids.begin();
                iter != this->plotGrids.end(); ++iter) {
                if( (*iter) != NULL ) {
                    this->ClipMapperToTlcBrc( this->dataVector[MR4D], (*iter)->GetPlotGridActor()->GetMapper(), tlcBrc, 0, 0, 0 );
                }
            }
        }
    }
}


/*!     
 *
 */
void svkPlotGridView::SetOrientation( svkDcmHeader::Orientation orientation )
{
    this->orientation = orientation;
    if( this->dataVector[MR4D] != NULL ) {
        int toggleDraw = this->GetRenderer( svkPlotGridView::PRIMARY )->GetDraw();
        if( toggleDraw ) {
            this->GetRenderer( svkPlotGridView::PRIMARY)->DrawOff();
        }
        this->UpdateMetaboliteTextDisplacement();
        this->satBands->SetOrientation( this->orientation );

        for( vector<svkPlotLineGrid*>::iterator iter = this->plotGrids.begin();
            iter != this->plotGrids.end(); ++iter) {
            if( (*iter) != NULL ) {
                (*iter)->SetOrientation( this->orientation );
            }
        }

        this->SetTlcBrc( this->plotGrids[0]->GetTlcBrc()[0], this->plotGrids[0]->GetTlcBrc()[1] );
        if( toggleDraw ) {
            this->GetRenderer( svkPlotGridView::PRIMARY)->DrawOn();
        }
        this->SetSlice( this->plotGrids[0]->GetSlice() );
        this->AlignCamera();
        this->Refresh();

    }
}


//! Resets the camera to look at the new selection
void svkPlotGridView::AlignCamera( ) 
{  

    double currentScale = this->GetRenderer( svkPlotGridView::PRIMARY)->GetActiveCamera()->GetParallelScale( );
    double currentViewAngle= this->GetRenderer( svkPlotGridView::PRIMARY)->GetActiveCamera()->GetViewAngle( );
    if (this->GetDebug()) {
        cout << endl;
        cout << "Slice " << this->slice << endl;
        cout << "CurScale: " << currentScale << endl;
        cout << "CurAngle: " << currentViewAngle<< endl;
    }

    if( this->GetRenderer( svkPlotGridView::PRIMARY) != NULL && this->dataVector[MR4D] != NULL ) {
        double bounds[6];
        double normal[3];
        this->dataVector[MR4D]->GetSliceNormal( normal, this->orientation );
        double zoom;
        string acquisitionType;
        if( this->dataVector[MR4D]->GetNumberOfCells() == 1
            && this->dataVector[MR4D]->IsA("svkMrsImageData")
            && svkMrsImageData::SafeDownCast(this->dataVector[MR4D])->HasSelectionBox()) {
            memcpy( bounds, this->GetProp( VOL_SELECTION )->GetBounds(), sizeof(double)*6 );
        } else {
            this->plotGrids[0]->CalculateTlcBrcBounds( bounds, this->tlcBrc );
        }
        double viewWidth =  bounds[1] - bounds[0];
        double viewHeight = bounds[3] - bounds[2];
        double viewDepth =  bounds[5] - bounds[4];

        double diagonal = sqrt( pow(viewWidth,2) 
                              + pow(viewHeight,2) 
                              + pow(viewDepth,2) );
        double focalPoint[3] = { bounds[0] + (viewWidth)/2.0 ,
                                 bounds[2] + (viewHeight)/2.0 ,
                                 bounds[4] + (viewDepth)/2.0 };

        this->GetRenderer( svkPlotGridView::PRIMARY)->ResetCamera( bounds );
        if( this->dataVector[MR4D]->IsA("svkMrsImageData") 
            && svkMrsImageData::SafeDownCast(this->dataVector[MR4D])->IsSliceInSelectionBox( this->slice, this->orientation ) ) 
        {
            double* selectionBoxBounds = this->GetProp( VOL_SELECTION )->GetBounds();
            double tmpViewBounds[6];
            memcpy( tmpViewBounds, bounds, sizeof(double)*6 );
            int orientationIndex = this->dataVector[MR4D]->GetOrientationIndex( this->orientation );
            tmpViewBounds[2*orientationIndex] = selectionBoxBounds[2*orientationIndex];
            tmpViewBounds[2*orientationIndex+1] = selectionBoxBounds[2*orientationIndex+1];
            diagonal = sqrt( pow(tmpViewBounds[1] - tmpViewBounds[0],2) 
                           + pow(tmpViewBounds[3] - tmpViewBounds[2],2) 
                           + pow(tmpViewBounds[5] - tmpViewBounds[4],2) );
            this->GetRenderer( svkPlotGridView::PRIMARY)->ResetCamera( tmpViewBounds );
        }

        int toggleDraw = this->GetRenderer( svkPlotGridView::PRIMARY)->GetDraw();
        if( toggleDraw ) {
            this->GetRenderer( svkPlotGridView::PRIMARY)->DrawOff();
        }

        // if the data set is not axial, move the camera
        this->GetRenderer( svkPlotGridView::PRIMARY)->GetActiveCamera()->SetFocalPoint( focalPoint );
        double* cameraPosition = this->GetRenderer( svkPlotGridView::PRIMARY)->GetActiveCamera()->GetPosition();
        double distance = sqrt( pow( focalPoint[0] - cameraPosition[0], 2 ) +
                                pow( focalPoint[1] - cameraPosition[1], 2 ) +
                                pow( focalPoint[2] - cameraPosition[2], 2 ) );
        
        double newCameraPosition[3] = {0,0,0};

        // Lets calculate the distance from the focal point to the selection box
        if( this->orientation == svkDcmHeader::AXIAL && normal[2] > 0 ) {
            distance *=-1;
        } else if( this->orientation == svkDcmHeader::CORONAL && normal[1] > 0 ) { 
            distance *=-1;
        } else if( this->orientation == svkDcmHeader::SAGITTAL && normal[0] < 0 ) { 
            distance *=-1;
        }
         
        newCameraPosition[0] = focalPoint[0] + distance*normal[0]; 
        newCameraPosition[1] = focalPoint[1] + distance*normal[1];
        newCameraPosition[2] = focalPoint[2] + distance*normal[2];
        this->GetRenderer( svkPlotGridView::PRIMARY)->GetActiveCamera()->SetPosition( newCameraPosition );

        double* visibleBounds  = this->GetRenderer( svkPlotGridView::PRIMARY)->ComputeVisiblePropBounds();
        double thickness = sqrt( pow( visibleBounds[1] - visibleBounds[0], 2 ) +
                                 pow( visibleBounds[3] - visibleBounds[2], 2 ) +
                                 pow( visibleBounds[5] - visibleBounds[4], 2 ) ) + fabs(distance);
        this->GetRenderer( svkPlotGridView::PRIMARY)->GetActiveCamera()->SetThickness( thickness );
        double columnNormal[3];
        double viewUp[3];
        int inverter = -1;

        switch ( this->orientation ) {
            case svkDcmHeader::AXIAL:
                this->dataVector[MR4D]->GetSliceNormal( viewUp, svkDcmHeader::CORONAL );
                if( viewUp[1] < 0 ) {
                    inverter *=-1;
                }
                this->GetRenderer( svkPlotGridView::PRIMARY)->GetActiveCamera()->SetViewUp( inverter*viewUp[0], 
                                                              inverter*viewUp[1], 
                                                              inverter*viewUp[2] );
                break;
            case svkDcmHeader::CORONAL:
                this->dataVector[MR4D]->GetSliceNormal( viewUp, svkDcmHeader::AXIAL );
                if( viewUp[2] > 0 ) {
                    inverter *=-1;
                }
                this->GetRenderer( svkPlotGridView::PRIMARY)->GetActiveCamera()->SetViewUp( inverter*viewUp[0], 
                                                              inverter*viewUp[1], 
                                                              inverter*viewUp[2] );
                break;
            case svkDcmHeader::SAGITTAL:
                this->dataVector[MR4D]->GetSliceNormal( viewUp, svkDcmHeader::AXIAL );
                if( viewUp[2] > 0 ) {
                    inverter *=-1;
                }
                this->GetRenderer( svkPlotGridView::PRIMARY)->GetActiveCamera()->SetViewUp( inverter*viewUp[0], 
                                                              inverter*viewUp[1], 
                                                              inverter*viewUp[2] );
                break;
        }


        if( viewWidth >= viewHeight && viewWidth >= viewDepth ) {
            zoom = diagonal/viewWidth;        
        } else if( viewHeight >= viewWidth && viewHeight >= viewDepth ) {
            zoom = diagonal/viewHeight;        
        } else {
            zoom = diagonal/viewDepth;        
        }
        // We'll back off the zoom to 95% to leave some edges
        zoom = 0.95 * zoom; 
        this->GetRenderer( svkPlotGridView::PRIMARY)->GetActiveCamera()->Zoom( zoom );

        //  If not aligning camera, then retain the current rwi "zoom" or scale factor
        if ( this->alignCamera == false ) {
            this->GetRenderer( svkPlotGridView::PRIMARY)->GetActiveCamera()->SetParallelScale( currentScale);
        }

        if( toggleDraw ) {
            this->GetRenderer( svkPlotGridView::PRIMARY)->DrawOn();
        }

        if (this->GetDebug()) {
            currentScale = this->GetRenderer( svkPlotGridView::PRIMARY)->GetActiveCamera()->GetParallelScale( );
            cout << "reset CurScale: " << currentScale << endl;
            currentViewAngle= this->GetRenderer( svkPlotGridView::PRIMARY)->GetActiveCamera()->GetViewAngle( );
            cout << "reset CurAngle: " << currentViewAngle<< endl;
        }
    }

}


/*!
 *  Returns the active input dataset.
 *
 * @return
*/
svk4DImageData* svkPlotGridView::GetActiveInput()
{
    return this->plotGrids[ this->activePlot ]->GetInput();
}


/*!
 * Sets the minimum number of digits after the decimal point for the overlay text.
 */
void svkPlotGridView::SetOverlayTextDigits( int digits )
{
    this->overlayTextDigits = digits;
    vtkActor2D* textActor = vtkActor2D::SafeDownCast(this->GetProp( svkPlotGridView::OVERLAY_TEXT ));
    if( textActor != NULL ) {
        svkLabeledDataMapper* metMapper = svkLabeledDataMapper::SafeDownCast( textActor->GetMapper( ));
        if( metMapper != NULL ) {
            double *range = dataVector[MET]->GetScalarRange();
            if( range[1] < 1000 ) {
                metMapper->SetLabelFormat(this->GetDecimalFormat( this->overlayTextDigits ).c_str());
            } else {
                metMapper->SetLabelFormat(this->GetScientificFormat( this->overlayTextDigits ).c_str());
            }
        }
    }
}


/*!
 *  Creates a printf style string for formatting
 *  the overlay text. Takes the number of significant
 *  digits as input.
 */
string svkPlotGridView::GetScientificFormat( int digits )
{
    if( digits <= 0 ){
        digits = 1;
    }
    string format = "%0.";
    format.append(svkTypeUtils::IntToString( digits ));
    format.append("e");
    return format;
}


/*!
 *  Creates a printf style string for formatting
 *  the overlay text. Takes the number of digits
 *  after the decimal as input.
 */
string svkPlotGridView::GetDecimalFormat( int digits )
{
    if( digits <= 0 ){
        digits = 2;
    }
    string format = "%1.";
    format.append(svkTypeUtils::IntToString( digits ));
    format.append("f");
    return format;
}


/*!
 * Turns on the given prop, in both the view and in the detailed plot directory
 */
void svkPlotGridView::TurnPropOn(int propIndex)
{
    bool turnPropOn = true;
    if( propIndex == OVERLAY_IMAGE ) {
        this->detailedPlotDirector->SetBackgroundVisibility(true);
    } else if (propIndex == OVERLAY_TEXT ) {
        this->detailedPlotDirector->SetAnnotationTextVisibility(true);
    } else if (propIndex == VOL_SELECTION && this->dataVector[MR4D] != NULL
            && this->dataVector[MR4D]->GetNumberOfCells() == 1) {
        turnPropOn = false;
    }
    if( turnPropOn ) {
        Superclass::TurnPropOn(propIndex);
    }

}


/*!
 * Turns off the given prop, in both the view and in the detailed plot directory
 */
void svkPlotGridView::TurnPropOff(int propIndex)
{
    Superclass::TurnPropOff(propIndex);
    if( propIndex == OVERLAY_IMAGE ) {
        this->detailedPlotDirector->SetBackgroundVisibility(false);
    } else if (propIndex == OVERLAY_TEXT ) {
        this->detailedPlotDirector->SetAnnotationTextVisibility(false);
    }
}


/*! 
 * Turn off this behavior to prevent auto 
 *  alignment of camera during slice changes, etc. 
 */ 
void svkPlotGridView::AlignCameraOff() 
{
    this->alignCamera = false; 
}


/*! 
 * Turn on (on by default) this behavior to prevent auto 
 *  alignment of camera during slice changes, etc. 
 */ 
void svkPlotGridView::AlignCameraOn() 
{
    this->alignCamera = true; 
}
