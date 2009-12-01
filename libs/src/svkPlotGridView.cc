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


#include <svkPlotGridView.h>


using namespace svk;


#define DEBUG 0

vtkCxxRevisionMacro(svkPlotGridView, "$Rev$");
vtkStandardNewMacro(svkPlotGridView);


//! Constructor 
svkPlotGridView::svkPlotGridView()
{
    this->plotGrid = svkPlotGrid::New();

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

    vtkRenderer* ren = vtkRenderer::New();
    this->SetRenderer( svkPlotGridView::PRIMARY, ren ); 
    ren->Delete();

    this->GetRenderer( svkPlotGridView::PRIMARY )->SetLayer(0);
    this->plotGrid->SetRenderer( this->GetRenderer( svkPlotGridView::PRIMARY) );

}


//! Destructor
svkPlotGridView::~svkPlotGridView()
{
    if( rwi != NULL ) {
        rwi->Delete();
        rwi = NULL;
    }
    if( plotGrid != NULL ) {
        plotGrid->Delete();
        plotGrid = NULL;
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

            plotGrid->SetInput(svkMrsImageData::SafeDownCast(data));
            plotGrid->AlignCamera(); 

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
    this->slice = slice;
    this->plotGrid->SetSlice(slice);
    if( dataVector.size() > MET && dataVector[MET] != NULL ) {
        UpdateMetaboliteText(plotGrid->GetCurrentTlcBrc());
    }
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
    plotGrid->SetPlotVoxels(tlcID, brcID);
    plotGrid->Update();
    UpdateMetaboliteText(plotGrid->GetCurrentTlcBrc());
    plotGrid->AlignCamera(); 
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
void svkPlotGridView::SetComponent( svkBoxPlot::PlotComponent component)
{
    this->plotGrid->SetComponent( component ); 
    this->Refresh();
}


/*!
 *  Gets the tlc and brc of a set of actors from the plot grid
 *  which has a dictionary of cells to actors.
 *
 *  \param selectionArea the area you wish to select [xmin,xmax,ymin,ymax] 
 */
void svkPlotGridView::SetSelection(int* selectionArea)
{
    this->plotGrid->SetSelection( selectionArea );
    if( dataVector[MET] != NULL ) {
        UpdateMetaboliteText(plotGrid->GetCurrentTlcBrc());
    }
}


//! Method is called when data object is Modified. 
void svkPlotGridView::Refresh()
{
    if (DEBUG) {
        cout << "svkPlotGridView::Refresh calls plotGrid->Update() first " << endl; 
    }

    this->plotGrid->Update();
    this->svkDataView::Refresh(); 
}


//! Selects all voxels within the selection box
void svkPlotGridView::HighlightSelectionVoxels()
{
    this->plotGrid->HighlightSelectionVoxels();
    this->UpdateMetaboliteText( plotGrid->GetCurrentTlcBrc());
}


/*!
 *  Creates the svkOrientedImageActor for rendering an overlay.
 *
 *  \param data the data to be rendered as an overlay
 */
void svkPlotGridView::CreateMetaboliteOverlay( svkImageData* data )
{
    if( dataVector[MRS] != NULL && data != NULL ) {
        int* extent = data->GetExtent();
        double* spacing = data->GetSpacing();

        // We are going to need to make a "projected" svkImageData objects
        // Since the svkPlotGrid maintains the coordinate system of the image
        // it is necessary to drop the dcos. We are simply putting the view 
        // into the reference from of the oblique data.

        vtkImageData* projectedData = vtkImageData::New(); 
        projectedData->SetOrigin(0,0,0);
        projectedData->SetExtent( extent );
        projectedData->SetSpacing( spacing[0], spacing[1], spacing[2] );
        projectedData->GetPointData()->SetScalars( data->GetPointData()->GetScalars() );
        vtkActor2D* metActor = vtkActor2D::New();
        vtkLabeledDataMapper* metMapper = vtkLabeledDataMapper::New();
        vtkImageClip* metTextClipper = vtkImageClip::New();
        metTextClipper->SetInput( projectedData );
        projectedData->Delete();
        metMapper->SetInput( metTextClipper->GetOutput() );
        metMapper->SetLabelModeToLabelScalars();
        metMapper->SetLabeledComponent(0);
        metMapper->SetLabelFormat("%0.1e");
        metMapper->GetLabelTextProperty()->ShadowOff();
        metMapper->GetLabelTextProperty()->ItalicOff();
        metMapper->GetLabelTextProperty()->BoldOff();
        metMapper->GetLabelTextProperty()->SetFontSize(10);
        metMapper->GetLabelTextProperty()->SetColor(0,1,1);
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
        if( metClippers.size() < 5 ) {
            optimus->Translate(spacing[0]/25, spacing[1]/5 + (spacing[1]/4)*(metClippers.size()-1), 0);
        } else {
            optimus->Translate(spacing[0]/2, -spacing[1]/5 + (spacing[1]/4)*(metClippers.size()-5), 0);
        }
        metMapper->SetTransform(optimus);
        optimus->Delete();
        metActor->SetMapper( metMapper );  
        metMapper->Delete();

        // If it has not been added, add it
        if( !this->GetRenderer( svkPlotGridView::PRIMARY )->HasViewProp( metActor ) ) {
            this->GetRenderer( svkPlotGridView::PRIMARY )->AddViewProp( metActor );
        }
        metActor->Delete();
        UpdateMetaboliteText( plotGrid->GetCurrentTlcBrc() );

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
        Refresh();
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
    return slice;
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
            if( currentActor->IsA("svkBoxPlot") ) {
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
