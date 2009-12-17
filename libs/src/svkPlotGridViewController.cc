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


#include <svkPlotGridViewController.h>


using namespace svk;


#define DEBUG 1

vtkCxxRevisionMacro(svkPlotGridViewController, "$Rev$");
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
    this->slice = 0;
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
    this->view->Delete();
    this->view = NULL;

    if( this->dataVector[svkPlotGridView::MRS] != NULL ) {
        this->dataVector[svkPlotGridView::MRS]->Delete();
        this->dataVector[svkPlotGridView::MRS] = NULL;
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
    this->SetRWInteractor( this->rwi );
    this->view->Refresh();

}


//! Set input data and initialize default range values.
void svkPlotGridViewController::SetInput(svkImageData* data, int index)
{
    if( index == 0 || index == 1 ) {
        if( dataVector[index] != NULL ) {
            (dataVector[index])->Delete();
        }
        data->Register(this);
        dataVector[index] = data;
        this->view->SetInput(data, index);
    } else {
        cout<<"WARNING: svkPlotGridViewController only takes one image input!"<<endl;
    }
}


/*!  cast base class var type to specific sub-class in the implementation.  Or... declare
 *   view member variable to be of type svkPlotGridView*, overriding the base class type.
 */ 
void svkPlotGridViewController::SetSlice(int slice)
{
    this->slice = slice;
    this->view->SetSlice(slice);
}


//! Returns the current top left/ bottom right corners of the plotGrid
int* svkPlotGridViewController::GetTlcBrc()
{
    return static_cast<svkPlotGridView*>(this->view)->plotGrid->GetCurrentTlcBrc();
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
 *  SetWindowLevel for spectral view;  index 0 is frequency, index 1 is intensity
 */
void svkPlotGridViewController::SetComponent( svkBoxPlot::PlotComponent component)
{
    (static_cast<svkPlotGridView*>(this->view))->SetComponent( component );
}


/*!
 *  Get the current component, REAL, IMAGE, etc...
 */
int svkPlotGridViewController::GetComponent( )
{
    return (static_cast<svkPlotGridView*>(this->view))->plotGrid->GetComponent();
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
        // Now we must remove all renderers, in case the window already had renderers in it
        vtkRendererCollection* renderers = this->rwi->GetRenderWindow()->GetRenderers();
        vtkCollectionIterator* myIterator = vtkCollectionIterator::New();
        myIterator->SetCollection( renderers );
        myIterator->InitTraversal();
        while( !myIterator->IsDoneWithTraversal() ) {
            this->rwi->GetRenderWindow()->RemoveRenderer( static_cast<vtkRenderer*>(myIterator->GetCurrentObject()) );
            myIterator->GoToNextItem();
        }
        myIterator->Delete();

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
    int* selectionArea = new int[4];

    svkPlotGridViewController* thisController = static_cast<svkPlotGridViewController*>(thisObject);
    svkSpecGridSelector* thisDragSelect = thisController->dragSelect; 
    svkPlotGridView* view = static_cast<svkPlotGridView*>(thisController->GetView());
     
    selectionArea[0] = thisDragSelect->GetStartX();
    selectionArea[1] = thisDragSelect->GetStartY();
    selectionArea[2] = thisDragSelect->GetEndX();
    selectionArea[3] = thisDragSelect->GetEndY();
    view->SetSelection( selectionArea );
    delete[] selectionArea;
    
    // Now lets notify anyone watching the rwi that we have changed its selection
    thisController->rwi->InvokeEvent(vtkCommand::SelectionChangedEvent);
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


/*!
 *
 */
void svkPlotGridViewController::SetChannel( int channel ) 
{
    svkPlotGridView::SafeDownCast( this->view )->SetChannel( channel );
}


/*!
 *
 */
int svkPlotGridViewController::GetChannel( ) 
{
    return svkPlotGridView::SafeDownCast( this->view )->GetChannel();
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

