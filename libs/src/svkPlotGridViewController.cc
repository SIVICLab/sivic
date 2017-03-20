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


#include <svkPlotGridViewController.h>


using namespace svk;


#define DEBUG 0

//vtkCxxRevisionMacro(svkPlotGridViewController, "$Rev$");
vtkStandardNewMacro(svkPlotGridViewController);


//! Constructor 
svkPlotGridViewController::svkPlotGridViewController()
{
    this->dragSelectionCB = vtkCallbackCommand::New();
    this->dragSelectionCB->SetCallback( UpdateSelection );
    this->dragSelectionCB->SetClientData( (void*)this );
    this->dragSelect = svkSpecGridSelector::New();
    dataVector.push_back(NULL);
    dataVector.push_back(NULL);
    this->view = svkPlotGridView::New();
    this->view->SetController(this);

    vtkRenderer* primaryRenderer = vtkRenderer::New();
    this->view->SetRenderer(svkPlotGridView::PRIMARY, primaryRenderer );
    primaryRenderer->Delete();

    this->rwi = NULL;
}


//! Destructor
svkPlotGridViewController::~svkPlotGridViewController()
{
    if (rwi != NULL) {
        rwi->Delete();
        rwi = NULL;
    }

    if (dragSelect != NULL) {
        dragSelect->Delete();
        dragSelect = NULL;
    }

    if (dragSelectionCB != NULL) {
        dragSelectionCB->Delete();
        dragSelectionCB = NULL;
    }


    if( view != NULL ) {
        view->Delete();
        view = NULL;
    }

    // NOTE: Data is destroyed in the superclass
}

void svkPlotGridViewController::Reset()
{
    // lets remove the mose move callback in case it was in the detailed view when we reset
    svkPlotGridView::SafeDownCast(this->view)->detailedPlotDirector->RemoveOnMouseMoveObserver( this->rwi );
    this->view->Delete();
    this->view = NULL;

    if( this->dataVector[svkPlotGridView::MR4D] != NULL ) {
        this->dataVector[svkPlotGridView::MR4D]->Delete();
        this->dataVector[svkPlotGridView::MR4D] = NULL;
    }
    if( this->dataVector[svkPlotGridView::MET] != NULL ) {
        this->dataVector[svkPlotGridView::MET]->Delete();
        this->dataVector[svkPlotGridView::MET] = NULL;
    }
    if (dragSelect != NULL) {
        dragSelect->Delete();
        dragSelect = NULL;
    }

    this->rwi->RemoveObserver( dragSelectionCB );

    if (dragSelectionCB != NULL) {
        dragSelectionCB->Delete();
        dragSelectionCB = NULL;
    }
    this->dragSelectionCB = vtkCallbackCommand::New();
    this->dragSelectionCB->SetCallback( UpdateSelection );
    this->dragSelectionCB->SetClientData( (void*)this );
    this->dragSelect = svkSpecGridSelector::New();

    // Create View
    this->view = svkPlotGridView::New();
    this->view->SetController(this);
    vtkRenderer* primaryRenderer = vtkRenderer::New();
    this->view->SetRenderer(svkPlotGridView::PRIMARY, primaryRenderer );
    primaryRenderer->Delete();

    this->SetRWInteractor( this->rwi );
    this->view->Refresh();

}


//! Set input data and initialize default range values.
void svkPlotGridViewController::SetInput(svkImageData* data, int index)
{
    if( index > svkPlotGridView::MET ) {
        while( dataVector.size() < index + 1 ) {
            dataVector.push_back(NULL);
        }
    }
    if( dataVector[index] != NULL ) {
        (dataVector[index])->Delete();
    }
    data->Register(this);
    dataVector[index] = data;
    this->view->SetInput(data, index);
}


/*!  cast base class var type to specific sub-class in the implementation.  Or... declare
 *   view member variable to be of type svkPlotGridView*, overriding the base class type.
 */ 
void svkPlotGridViewController::SetSlice(int slice)
{
    this->view->SetSlice(slice);
}


//! Returns the current top left/ bottom right corners of the plotGrid
int* svkPlotGridViewController::GetTlcBrc()
{
    return static_cast<svkPlotGridView*>(this->view)->tlcBrc;
}


//! Sets the current top left/bottm right corners of the plotGrid
void svkPlotGridViewController::SetTlcBrc( int* tlcBrc )
{
    int* currentTlcBrc = GetTlcBrc();
    if( tlcBrc != NULL ) {
        if( currentTlcBrc != NULL ) {
            if( tlcBrc[0] != currentTlcBrc[0] || tlcBrc[1] != currentTlcBrc[1] ) {
               static_cast<svkPlotGridView*>(this->view)->SetTlcBrc( tlcBrc[0], tlcBrc[1] );
            }
        } else {
           static_cast<svkPlotGridView*>(this->view)->SetTlcBrc( tlcBrc[0], tlcBrc[1] );
        }
    }

}


/*!
 *  SetWindowLevel for spectral view;  index 0 is frequency, index 1 is intensity
 */
void svkPlotGridViewController::SetWindowLevelRange( double lower, double upper, int index)
{
   this->view->SetWindowLevelRange(lower, upper, index);
}


/*!
 *  GetWindowLevel for spectral view;  index 0 is frequency, index 1 is intensity
 */
void svkPlotGridViewController::GetWindowLevelRange( double &lower, double &upper, int index)
{
   svkPlotGridView::SafeDownCast(this->view)->GetWindowLevelRange(lower, upper, index);
}


/*!
 *  SetWindowLevel for spectral view;  index 0 is frequency, index 1 is intensity
 */
void svkPlotGridViewController::SetComponent( svkPlotLine::PlotComponent component)
{
    (static_cast<svkPlotGridView*>(this->view))->SetComponent( component );
}


/*!
 *  Get the current component, REAL, IMAGE, etc...
 */
int svkPlotGridViewController::GetComponent( )
{
    return (static_cast<svkPlotGridView*>(this->view))->plotGrids[0]->GetComponent();
}



//! Set the RenderWindowInteractor and setup the drag selector.
void svkPlotGridViewController::SetRWInteractor( vtkRenderWindowInteractor* rwi )
{
    if( rwi != NULL ) {
        if( this->rwi != NULL ) {
            this->rwi->Delete();
        } 
        this->rwi = rwi;
        this->rwi->Register( this );
        // Now we must remove all renderers, in case the window already had renderers init
        vtkRendererCollection* renderers = this->rwi->GetRenderWindow()->GetRenderers();
		for( int i = 0; i < renderers->GetNumberOfItems(); i++ ) {
			vtkObject* obj = renderers->GetItemAsObject(i);
			if( obj != NULL ) {
				vtkRenderer* renderer = vtkRenderer::SafeDownCast( obj );
				if( renderer != NULL ) {
					this->rwi->GetRenderWindow()->RemoveRenderer( renderer );
				}
			}
		}

        // If we wanted another type of interactor...
        //rwi->SetInteractorStyle( vtkInteractorStyleTrackballCamera::New ());
        rwi->SetInteractorStyle( dragSelect );
        rwi->GetRenderWindow()->SetNumberOfLayers(1);
        rwi->AddObserver(vtkCommand::LeftButtonReleaseEvent, dragSelectionCB);
        this->svkDataViewController::SetRWInteractor( rwi );
    }
}


/*!
 * Callback for setting the selected voxels.
 */
void svkPlotGridViewController::UpdateSelection(vtkObject* subject, unsigned long eid, void* thisObject, void *callData)
{
    svkPlotGridViewController* dvController = static_cast<svkPlotGridViewController*>(thisObject);
    svkSpecGridSelector* myRubberband = dvController->dragSelect;

    vtkRenderWindowInteractor *rwi =
            vtkRenderWindowInteractor::SafeDownCast( subject );
    double* selectionArea = new double[4];
    if( DEBUG ) {
        cout<<"start position ="<<myRubberband->GetStartX()<<","<<myRubberband->GetStartY()<<endl;
        cout<<"end position ="<<myRubberband->GetEndX()<<","<<myRubberband->GetEndY()<<endl;
    }    
    selectionArea[0] = static_cast<double>(myRubberband->GetStartX());
    selectionArea[1] = static_cast<double>(myRubberband->GetStartY());
    selectionArea[2] = static_cast<double>(myRubberband->GetEndX());
    selectionArea[3] = static_cast<double>(myRubberband->GetEndY()),
    static_cast<svkPlotGridView*>(dvController->GetView())->SetSelection( selectionArea);
    rwi->InvokeEvent(vtkCommand::SelectionChangedEvent);
    delete[] selectionArea;
}


void svkPlotGridViewController::TurnPropOn(int propIndex)
{
    view->TurnPropOn( propIndex );
    view->Refresh();
}


void svkPlotGridViewController::TurnPropOff(int propIndex)
{
    view->TurnPropOff( propIndex);
    view->Refresh();
}


void svkPlotGridViewController::HighlightSelectionVoxels() 
{
    static_cast<svkPlotGridView*>(view)->HighlightSelectionVoxels(); 
}


void svkPlotGridViewController::SetColorSchema( int colorSchema )
{
    static_cast<svkPlotGridView*>(view)->SetColorSchema( colorSchema ); 
}


string svkPlotGridViewController::GetDataCompatibility( svkImageData* data, int targetIndex ) 
{
    return static_cast<svkPlotGridView*>(view)->GetDataCompatibility( data, targetIndex ); 
}

void svkPlotGridViewController::SetVolumeIndex( int index, int volumeIndex )
{
    static_cast<svkPlotGridView*>(view)->SetVolumeIndex( index, volumeIndex );
}

int svkPlotGridViewController::GetVolumeIndex( int volumeIndex )
{
    return static_cast<svkPlotGridView*>(view)->GetVolumeIndex( volumeIndex );
}

int* svkPlotGridViewController::GetVolumeIndexArray(  )
{
    return static_cast<svkPlotGridView*>(view)->GetVolumeIndexArray( );
}

/*!
 *
 */
void svkPlotGridViewController::SetOverlayOpacity(double opacity)
{
    static_cast<svkPlotGridView*>( view )->SetOverlayOpacity( opacity );
}


/*!
 *
 */
void svkPlotGridViewController::SetOverlayThreshold(double threshold)
{
    static_cast<svkPlotGridView*>( view )->SetOverlayThreshold( threshold );
}

/*!
 *
 */
void svkPlotGridViewController::SetLUT( svkLookupTable::svkLookupTableType type )
{
    static_cast<svkPlotGridView*>(this->view)->SetLUT( type );
}
