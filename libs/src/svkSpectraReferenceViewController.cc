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

#include <svkSpectraReferenceViewController.h>


using namespace svk;


//vtkCxxRevisionMacro(svkSpectraReferenceViewController, "$Rev$");
vtkStandardNewMacro(svkSpectraReferenceViewController);


/*!
 *  Constructor initializes the associated DataView and initializes callbacks.
 */
svkSpectraReferenceViewController::svkSpectraReferenceViewController()
{

    // Create View 
    view = svkSpectraReferenceView::New();
    view->SetController(this);

    // Setup Callbacks
    cursorLocationCB = vtkCallbackCommand::New();
    cursorLocationCB->SetCallback( UpdateCursorLocation );
    cursorLocationCB->SetClientData( (void*)this );
    
    // Commented code is for keyboard callbacks to change interactors
    this->interactorSwitchCB = vtkCallbackCommand::New();
    this->interactorSwitchCB->SetCallback( UpdateInteractor );
    this->interactorSwitchCB->SetClientData( (void*)this );
    dragSelectionCB = vtkCallbackCommand::New();
    dragSelectionCB->SetCallback( UpdateSelection );
    dragSelectionCB->SetClientData( (void*)this );

    colorOverlayCB = vtkCallbackCommand::New();
    colorOverlayCB->SetCallback( ColorWindowLevel );
    colorOverlayCB->SetClientData( (void*)this );

    // New objects
    //view->GetRendererCollection()->AddItem(index, item);   not possible
    vtkRenderer* primaryRenderer = vtkRenderer::New();
    this->view->SetRenderer(svkSpectraReferenceView::PRIMARY, primaryRenderer );
    primaryRenderer->Delete();

    vtkRenderer* mouseLocationRenderer = vtkRenderer::New();
    this->view->SetRenderer( svkSpectraReferenceView::MOUSE_LOCATION, mouseLocationRenderer );
    mouseLocationRenderer->Delete();

    // Data Vector initialization
    dataVector.push_back( NULL );
    dataVector.push_back( NULL );
    dataVector.push_back( NULL );
    
    // Null initializations
    rwi = NULL;
    vtkTextActor* coordinatesActor = vtkTextActor::New();
    this->view->SetProp(svkSpectraReferenceView::COORDINATES, coordinatesActor );
    coordinatesActor->Delete();
    static_cast<vtkTextActor*>(this->view->GetProp(svkSpectraReferenceView::COORDINATES))->SetTextScaleModeToProp();
    coordinatesActor->SetPosition(0.1,0.1); 
    coordinatesActor->SetPosition2(0.9,0.9); 
    coordinatesActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    coordinatesActor->GetPosition2Coordinate()->SetCoordinateSystemToNormalizedViewport();
    static_cast<vtkTextActor*>(this->view->GetProp(svkSpectraReferenceView::COORDINATES))->SetInput("x: null \ny: null \n z: null");
    // Load anatamical and spectroscopic data
    this->view->GetRenderer(svkSpectraReferenceView::MOUSE_LOCATION)->AddActor( 
        this->view->GetProp(svkSpectraReferenceView::COORDINATES)
    );
    this->view->GetRenderer(svkSpectraReferenceView::MOUSE_LOCATION)->SetViewport( 0.0, 0.0, 0.2, 0.15 );
    this->view->GetRenderer(svkSpectraReferenceView::MOUSE_LOCATION)->InteractiveOff();
    this->view->GetRenderer(svkSpectraReferenceView::PRIMARY)->BackingStoreOn();
    windowLevelStyle = NULL;
    dragSelectStyle = svkOverlaySelector::New();
    rotationStyle = vtkInteractorStyleTrackballCamera::New();
    colorOverlayStyle = vtkInteractorStyleImage::New();
    colorOverlayStyle->AddObserver(vtkCommand::StartWindowLevelEvent, colorOverlayCB);
    colorOverlayStyle->AddObserver(vtkCommand::WindowLevelEvent, colorOverlayCB);
    colorOverlayStyle->AddObserver(vtkCommand::EndWindowLevelEvent, colorOverlayCB);
    colorOverlayStyle->AddObserver(vtkCommand::ResetWindowLevelEvent, colorOverlayCB);
    colorOverlayStyle->AddObserver(vtkCommand::PickEvent, colorOverlayCB);
    colorOverlayStyle->AddObserver(vtkCommand::StartPickEvent, colorOverlayCB);
    windowLevelStyle = NULL;
    scanBoundryActor = NULL;
    // DON'T FORGET TO FREE RESOURCES!!
}


/*! 
 *  Destructor.
 */
svkSpectraReferenceViewController::~svkSpectraReferenceViewController()
{
    if( rwi != NULL ) {
        rwi->Delete();
        rwi = NULL;
    }
    
    if( cursorLocationCB != NULL ) {
        cursorLocationCB->Delete();
        cursorLocationCB = NULL;
    }

    if( colorOverlayCB != NULL ) {
        colorOverlayCB->Delete();
        colorOverlayCB = NULL;
    }

    if( interactorSwitchCB != NULL ) {
        interactorSwitchCB->Delete();
        interactorSwitchCB = NULL;
    }
    if( dragSelectionCB != NULL ) {
        dragSelectionCB->Delete();
        dragSelectionCB = NULL;
    }

    if( dragSelectStyle != NULL ) {
        dragSelectStyle->Delete();
        dragSelectStyle = NULL;
    }

    if( rotationStyle != NULL ) {
        rotationStyle->Delete();
        rotationStyle = NULL;
    }

    if( windowLevelStyle != NULL ) {
        windowLevelStyle->Delete();
        windowLevelStyle = NULL;
    }

    if( colorOverlayStyle != NULL ) {
        colorOverlayStyle->Delete();
        colorOverlayStyle = NULL;
    }

    if( scanBoundryActor != NULL ) {
        scanBoundryActor->Delete();
        scanBoundryActor = NULL;
    }
    
    if( view != NULL ) {
        view->Delete();
        view = NULL;
    }
}


void svkSpectraReferenceViewController::Reset() 
{
    this->view->Delete();
    this->view = NULL;
    if( dataVector[MRI] != NULL ) {
        dataVector[MRI]->Delete();
        dataVector[MRI] = NULL;
    }
    if( dataVector[MRS] != NULL ) {
        dataVector[MRS]->Delete();
        dataVector[MRS] = NULL;
    }
    if( dataVector[MET] != NULL ) {
        dataVector[MET]->Delete();
        dataVector[MET] = NULL;
    }
    // Create View 
    this->view = svkSpectraReferenceView::New();
    this->view->SetController(this);
    // New objects
    vtkRenderer* primaryRenderer = vtkRenderer::New();
    this->view->SetRenderer(svkSpectraReferenceView::PRIMARY, primaryRenderer );
    primaryRenderer->Delete();

    vtkRenderer* mouseLocationRenderer = vtkRenderer::New();
    this->view->SetRenderer( svkSpectraReferenceView::MOUSE_LOCATION, mouseLocationRenderer );
    mouseLocationRenderer->Delete();

    vtkTextActor* coordinatesActor = vtkTextActor::New();
    this->view->SetProp(svkSpectraReferenceView::COORDINATES, coordinatesActor );
    coordinatesActor->Delete();
    static_cast<vtkTextActor*>(this->view->GetProp(svkSpectraReferenceView::COORDINATES))->SetTextScaleModeToProp();

    this->rwi->RemoveObserver( dragSelectionCB);
    this->rwi->RemoveObserver( colorOverlayCB);
    this->rwi->RemoveObserver( cursorLocationCB);
    this->SetRWInteractor( this->rwi );
}


/*!
 *  Setter method. Also calls the SetInput method of its view.
 *  svkSpectraReferenceView has two inputs, an image and a spectroscopy
 *  data set.
 *
 *  \param data the data you wish to view
 *
 *  \param index the index of the input you wish to set.
 */
void svkSpectraReferenceViewController::SetInput( svkImageData* data, int index)
{
    if( index == MRI && data != NULL ) { // 0 for anatomical data
        if( dataVector[MRI] != NULL ) {
            dataVector[MRI]->Delete();
        }
        data->Register( this );
        dataVector[MRI] = data;
        this->view->SetInput(data);
            this->view->TurnRendererOn( svkSpectraReferenceView::MOUSE_LOCATION );
            if( dataVector[MRS] != NULL ) {
                this->view->SetInput(dataVector[MRS], 1); 
            }
    } else if( index == MRS && data != NULL ) {
        if( dataVector[MRS] != NULL ) {
            dataVector[MRS]->Delete();
        }
        data->Register( this );
        dataVector[MRS] = data;
        if( dataVector[MRI] != NULL ) {
            this->view->SetInput(data, 1);
            this->SetSlice( this->GetView()->GetSlice() );
        }
    } else if( index == MET && data != NULL && dataVector[MRI] != NULL && dataVector[MRS]!=NULL ) {
        if( dataVector[MET] != NULL ) {
            dataVector[MET]->Delete();
        }
        data->Register( this );
        dataVector[MET] = data;
        this->view->SetInput( data, MET );
    } else if( data != NULL ) {
        cout<<"WARNING: svkSpectraReferenceViewController only takes two image inputs!"<<endl;
    } else {
        cout<<"WARNING: data is NULL !"<<endl;
    
    }

}


/*!
 *  Setter. Should be modified to take action once the slice is set,
 *  and to modify its View to do the same. 
 *
 *  \param slice the slice you want to view
 *  \param the image you want to change the slice of, 0 is primary 1 and 2 are orthogonal 
 */
void svkSpectraReferenceViewController::SetSlice( int slice )
{
    this->view->SetSlice( slice );
}


/*!
 *  Setter. Should be modified to take action once the slice is set,
 *  and to modify its View to do the same. 
 *
 *  \param slice the slice you want to view
 *  \param the image you want to change the slice of, 0 is primary 1 and 2 are orthogonal 
 */
void svkSpectraReferenceViewController::SetSlice( int slice, bool centerImage )
{
    svkSpectraReferenceView::SafeDownCast( this->view )->SetSlice( slice, centerImage );
}

/*!
 *  Setter. Should be modified to take action once the slice is set,
 *  and to modify its View to do the same. 
 *
 *  \param slice the slice you want to view
 *  \param the image you want to change the slice of, 0 is primary 1 and 2 are orthogonal 
 */
void svkSpectraReferenceViewController::SetSlice(int slice, svkDcmHeader::Orientation orientation)
{
    static_cast<svkSpectraReferenceView*>(this->view)->SetSlice( slice, orientation );
}


/*!
 *  Getter for the current image slice, as opposed to the spectroscopic slice.
 *
 */
int svkSpectraReferenceViewController::GetImageSlice( svkDcmHeader::Orientation sliceOrientation )
{
    sliceOrientation = (sliceOrientation == svkDcmHeader::UNKNOWN_ORIENTATION ) ? this->GetView()->GetOrientation() : sliceOrientation;
    return static_cast<svkSpectraReferenceView*>(this->GetView())->imageViewer->GetSlice( sliceOrientation ); 
}

/*!
 *  Setter method. Also gets the render window from the render window actor.
 *  Once these have been set, CreateDataVisualization() is called and starts the actual
 *  create of the visualization.
 *
 *  \param rwi the vtkRenderWindowInteractor you want the View to use
 *
 */
void svkSpectraReferenceViewController::SetRWInteractor(vtkRenderWindowInteractor* rwi)
{
    if( rwi != NULL ) {

        if( this->rwi != NULL ) {
            this->rwi->Delete();
        }
        
        // Point to new render window interactor
        this->rwi = rwi;
        this->rwi->Register( this );
        this->myRenderWindow = this->rwi->GetRenderWindow();
        

        // Now we must remove all renderers, in case the window already had renderers in it
        vtkRendererCollection* renderers = this->myRenderWindow->GetRenderers();
        vtkCollectionIterator* myIterator = vtkCollectionIterator::New();
        myIterator->SetCollection( renderers );
        myIterator->InitTraversal();
        while( !myIterator->IsDoneWithTraversal() ) {
            this->myRenderWindow->RemoveRenderer( static_cast<vtkRenderer*>(myIterator->GetCurrentObject()) );
            myIterator->GoToNextItem();
        }

        // And add the new renderer
        this->rwi->SetEventPosition(0,0);
        this->rwi->SetLastEventPosition(0,0);
        this->svkDataViewController::SetRWInteractor( this->rwi );

        
        // Get pointers to the two interactor observers to allow switching
        if( this->windowLevelStyle != NULL && this->windowLevelStyle != this->rwi->GetInteractorStyle() ) {
            this->windowLevelStyle->Delete();
        }
        this->windowLevelStyle = rwi->GetInteractorStyle();
        this->windowLevelStyle->Register( this );

     
        this->rwi->AddObserver(vtkCommand::MouseMoveEvent, cursorLocationCB);
        this->rwi->AddObserver(vtkCommand::KeyPressEvent, interactorSwitchCB);
        this->rwi->AddObserver(vtkCommand::LeftButtonReleaseEvent, dragSelectionCB);
    
        // Cleanup
        myIterator->Delete();
    }
}


/*! 
 *  Used to set externally the Top Left and Bottom Right Corners of the display
 *
 *  \param tlcBrc pointer to a length two vector, the first element being the 
 *     l          cell id of the top left corner and the second the bottom right
 *                corner.
 */
void svkSpectraReferenceViewController::SetTlcBrc( int* tlcBrc) 
{
    int* currentTlcBrc = GetTlcBrc();
    if( tlcBrc != NULL ) {
        if( currentTlcBrc != NULL ) {
            if( tlcBrc[0] != currentTlcBrc[0] || tlcBrc[1] != currentTlcBrc[1] ) {
                static_cast<svkSpectraReferenceView*>( view )->SetTlcBrc( tlcBrc ); 
            }
        } else {
            static_cast<svkSpectraReferenceView*>( view )->SetTlcBrc( tlcBrc ); 
        }
    } 
}


/*! 
 *  Used to get the current Top Left and Bottom Right Corners of the display
 *
 *  \return  a pointer to a length two vector, the first element being the
 *           cell id of the top left corner and the second the bottom right
 *           corner.
 */
int* svkSpectraReferenceViewController::GetTlcBrc() 
{
    return static_cast<svkSpectraReferenceView*>(view)->tlcBrc;
}


int svkSpectraReferenceViewController::GetCurrentStyle() 
{
    return this->currentInteractorStyle;
}


/*!
 *
 */
void svkSpectraReferenceViewController::SetCurrentStyle( int style ) 
{
        enum InteractorStyle { SELECTION, WINDOW_LEVEL, COLOR_OVERLAY, ROTATION };
    switch( style ) {
        case SELECTION:
            this->UseSelectionStyle();
            break;
        case WINDOW_LEVEL:
            this->UseWindowLevelStyle();
            break;
        case COLOR_OVERLAY:
            this->UseColorOverlayStyle();
            break;
        case ROTATION:
            this->UseRotationStyle();
            break;
    }
}

//! Switches to the window level style interactor
void svkSpectraReferenceViewController::UseColorOverlayStyle() 
{
    this->view->GetRenderer(svkSpectraReferenceView::PRIMARY)->BackingStoreOff();
    this->view->TurnRendererOff( svkSpectraReferenceView::MOUSE_LOCATION );
    this->rwi->SetInteractorStyle( colorOverlayStyle ); 
    this->rwi->Enable(); 
    this->currentInteractorStyle = COLOR_OVERLAY;
    this->view->Refresh();

}

//! Switches to the window level style interactor
void svkSpectraReferenceViewController::UseWindowLevelStyle() 
{
    this->view->TurnRendererOff( svkSpectraReferenceView::MOUSE_LOCATION );
    this->rwi->SetInteractorStyle( windowLevelStyle ); 
    this->rwi->Enable(); 
    this->currentInteractorStyle = WINDOW_LEVEL;

}


//! Switches to the selection style interactor
void svkSpectraReferenceViewController::UseSelectionStyle() 
{
    if( !this->myRenderWindow->HasRenderer( this->view->GetRenderer(svkSpectraReferenceView::MOUSE_LOCATION) ) ) {
        this->view->TurnRendererOn( svkSpectraReferenceView::MOUSE_LOCATION );
    }
    this->view->GetRenderer(svkSpectraReferenceView::PRIMARY)->BackingStoreOn();
    
      
    // We are going to turn the sat bands off if they were on just for resetting the camera
    // If left on the, the range if the sat bands changes the fov. 
    bool satBandsOn = 0; 
    bool satBandsOutlineOn = 0; 
    if( view->IsPropOn(svkSpectraReferenceView::SAT_BANDS_AXIAL) ||
        view->IsPropOn(svkSpectraReferenceView::SAT_BANDS_CORONAL) || 
        view->IsPropOn(svkSpectraReferenceView::SAT_BANDS_SAGITTAL) ) { 
        satBandsOn = 1;
    }
    if( view->IsPropOn(svkSpectraReferenceView::SAT_BANDS_AXIAL_OUTLINE) ||
        view->IsPropOn(svkSpectraReferenceView::SAT_BANDS_CORONAL_OUTLINE) || 
        view->IsPropOn(svkSpectraReferenceView::SAT_BANDS_SAGITTAL_OUTLINE) ) { 
        satBandsOutlineOn = 1;
    }
        
    // Lets make sure orthogonal sat bands are off...
    view->TurnPropOff(svkSpectraReferenceView::SAT_BANDS_AXIAL); 
    view->TurnPropOff(svkSpectraReferenceView::SAT_BANDS_AXIAL_OUTLINE); 
    view->TurnPropOff(svkSpectraReferenceView::SAT_BANDS_CORONAL); 
    view->TurnPropOff(svkSpectraReferenceView::SAT_BANDS_CORONAL_OUTLINE); 
    view->TurnPropOff(svkSpectraReferenceView::SAT_BANDS_SAGITTAL); 
    view->TurnPropOff(svkSpectraReferenceView::SAT_BANDS_SAGITTAL_OUTLINE); 

    // Set style
    this->rwi->SetInteractorStyle( dragSelectStyle ); 
    this->rwi->Enable(); 

    //(static_cast<svkSpectraReferenceView*>(view))->AlignCamera();
    if( satBandsOn ) { 
        switch( this->GetView()->GetOrientation()) {
            case svkDcmHeader::AXIAL:
                view->TurnPropOn(svkSpectraReferenceView::SAT_BANDS_AXIAL);
                break;
            case svkDcmHeader::CORONAL:
                view->TurnPropOn(svkSpectraReferenceView::SAT_BANDS_CORONAL);
                break;
            case svkDcmHeader::SAGITTAL:
                view->TurnPropOn(svkSpectraReferenceView::SAT_BANDS_SAGITTAL);
                break;
        }
    }
    if( satBandsOutlineOn ) { 
        switch( this->GetView()->GetOrientation() ) {
            case svkDcmHeader::AXIAL:
                view->TurnPropOn(svkSpectraReferenceView::SAT_BANDS_AXIAL_OUTLINE);
                break;
            case svkDcmHeader::CORONAL:
                view->TurnPropOn(svkSpectraReferenceView::SAT_BANDS_CORONAL_OUTLINE);
                break;
            case svkDcmHeader::SAGITTAL:
                view->TurnPropOn(svkSpectraReferenceView::SAT_BANDS_SAGITTAL_OUTLINE);
                    break;
        }
    }
        this->currentInteractorStyle = SELECTION;
        svkSpectraReferenceView::SafeDownCast(this->view)->ToggleSelBoxVisibilityOn();
}


//! Switches to the rotation style interactor
void svkSpectraReferenceViewController::UseRotationStyle()
{
        bool areSatBandsOn;
        bool areSatBandOutlinesOn;

        switch( this->view->GetOrientation() ) {
            case svkDcmHeader::AXIAL:
                 areSatBandsOn = view->IsPropOn(svkSpectraReferenceView::SAT_BANDS_AXIAL);
                 areSatBandOutlinesOn = view->IsPropOn(svkSpectraReferenceView::SAT_BANDS_AXIAL_OUTLINE);
                break;
            case svkDcmHeader::CORONAL:
                 areSatBandsOn = view->IsPropOn(svkSpectraReferenceView::SAT_BANDS_CORONAL);
                 areSatBandOutlinesOn = view->IsPropOn(svkSpectraReferenceView::SAT_BANDS_CORONAL_OUTLINE);
                break;
            case svkDcmHeader::SAGITTAL:
                 areSatBandsOn = view->IsPropOn(svkSpectraReferenceView::SAT_BANDS_SAGITTAL);
                 areSatBandOutlinesOn = view->IsPropOn(svkSpectraReferenceView::SAT_BANDS_SAGITTAL_OUTLINE);
                break;
        }
        if( areSatBandsOn ) {
            view->TurnPropOn(svkSpectraReferenceView::SAT_BANDS_AXIAL); 
            view->TurnPropOn(svkSpectraReferenceView::SAT_BANDS_CORONAL); 
            view->TurnPropOn(svkSpectraReferenceView::SAT_BANDS_SAGITTAL); 
        }
        if( areSatBandOutlinesOn ) {
            view->TurnPropOn(svkSpectraReferenceView::SAT_BANDS_AXIAL_OUTLINE); 
            view->TurnPropOn(svkSpectraReferenceView::SAT_BANDS_CORONAL_OUTLINE); 
            view->TurnPropOn(svkSpectraReferenceView::SAT_BANDS_SAGITTAL_OUTLINE); 
        }
        svkSpectraReferenceView* myView = static_cast<svkSpectraReferenceView*>(view);
        svkSpectraReferenceView::SafeDownCast(this->view)->ToggleSelBoxVisibilityOff();
        this->view->TurnRendererOff( svkSpectraReferenceView::MOUSE_LOCATION );
        this->view->GetRenderer(svkSpectraReferenceView::PRIMARY)->BackingStoreOff();
        this->rwi->SetInteractorStyle( rotationStyle );
        this->rwi->Enable(); 
        this->currentInteractorStyle = ROTATION;

}


//! Resets the window level, source taken from vtkImageViewer2
void svkSpectraReferenceViewController::ResetWindowLevel()
{
    svkSpectraReferenceView* myView = static_cast<svkSpectraReferenceView*>(view);
    myView->ResetWindowLevel();
}


/*!
 *  Method to update the renderer that displays the mouse location. Since it is not clear where the image
 *  lies in the z-buffer, the location is assumed to be z = 0, then the resulting point is projected onto the
 *  image. 
 */
void svkSpectraReferenceViewController::UpdateCursorLocation(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata)
{
    svkSpectraReferenceViewController* dvController = static_cast<svkSpectraReferenceViewController*>(thisObject);
    int pos[2];
    double* imageCords;
    std::stringstream out;
    vtkCoordinate* mousePosition = vtkCoordinate::New();
    vtkRenderWindowInteractor *rwi = 
            vtkRenderWindowInteractor::SafeDownCast( subject );

    vtkRenderer* viewerRenderer = dvController->GetView()->GetRenderer(svkSpectraReferenceView::PRIMARY);
    pos[0] = dvController->rwi->GetEventPosition()[0];
    pos[1] = dvController->rwi->GetEventPosition()[1];
    double* origin;
    double* spacing;
    double planeOrigin[3];
    double projection[3];
    int index[3];
    svkDcmHeader::Orientation orientation;
    // We need the anatomical slice to calculate a point on the image
    int slice; 
    svkImageData* targetData;
    targetData = static_cast<svkSpectraReferenceView*>(dvController->GetView())->dataVector[MRS];
    if( targetData == NULL ) {
        targetData = static_cast<svkSpectraReferenceView*>(dvController->GetView())->dataVector[MRI];
    }
    if( targetData != NULL ) {

        slice = dvController->GetView()->GetSlice(); 
        origin = targetData->GetOrigin();
        orientation = dvController->GetView()->GetOrientation();
        int index[3] = {0,0,0};
        index[ targetData->GetOrientationIndex( orientation ) ] = slice;
        double cellCenter[3];
        targetData->GetPositionFromIndex(index, cellCenter); 
        planeOrigin[0] = cellCenter[0];
        planeOrigin[1] = cellCenter[1];
        planeOrigin[2] = cellCenter[2];

        mousePosition->SetCoordinateSystemToDisplay();
        mousePosition->SetValue( pos[0], pos[1], 0); 
        imageCords = mousePosition->GetComputedWorldValue( viewerRenderer );
        double viewNormal[3];
        targetData->GetSliceNormal( viewNormal, orientation );
        double viewNormalDouble[3] = { viewNormal[0], viewNormal[1], viewNormal[2] };
        // Project selection point onto the image
        vtkPlane::GeneralizedProjectPoint( imageCords, planeOrigin, viewNormalDouble, projection );

        out.setf(ios::fixed,ios::floatfield); 
        out.precision(1);
        out << "L: " << projection[0] << endl;
        out << "P: " << projection[1] << endl;    
        out << "S: " << projection[2];    
        static_cast<vtkTextActor*>(dvController->view->GetProp(svkSpectraReferenceView::COORDINATES))->SetInput((out.str()).c_str());
    } else { 
        out<<"L: "<<endl;
        out<<"P: "<<endl;    
        out<<"S: ";    
        static_cast<vtkTextActor*>(dvController->view->GetProp(svkSpectraReferenceView::COORDINATES))->SetInput((out.str()).c_str());
    }
    mousePosition->Delete();
}


/* 
 *  Method switches the current interactor, between the selection interactor
 *  and the window level interactor.
 */
void svkSpectraReferenceViewController::UpdateInteractor(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata)
{
    svkSpectraReferenceViewController* dvController = static_cast<svkSpectraReferenceViewController*>(thisObject);

    char keyPressed;
    vtkRenderWindowInteractor *rwi =
            vtkRenderWindowInteractor::SafeDownCast( subject );
    keyPressed = rwi->GetKeyCode();
    if ( keyPressed == ' ' ) {  
        switch(dvController->currentInteractorStyle) {
            case WINDOW_LEVEL:
                dvController->UseSelectionStyle(); 
                break;
            case SELECTION:
                dvController->UseRotationStyle(); 
                break;
            case ROTATION:
                svkSpectraReferenceView::SafeDownCast(dvController->GetView())->AlignCamera();
                dvController->UseWindowLevelStyle(); 
                break;
        } 
    }
}


/*!
 *  Turn on the vtkActorCollection representing a given topology for a given image.
 *
 *  \param actorIndex the index of the topology you want to turn on
 *
 *  \param imageIndex the index of the image who's topology you want turned on
 */
void svkSpectraReferenceViewController::TurnPropOn(int propIndex)
{
        view->TurnPropOn( propIndex );
        // Used as an update, to catch collection change
        static_cast<svkSpectraReferenceView*>( view )->SetTlcBrc( GetTlcBrc() ); 
}


/*!
 *  Turn off the vtkActorCollection representing a given topology for a given image.
 *
 *  \param actorIndex the index of the topology you want to turn off
 *
 *  \param imageIndex the index of the image who's topology you want turned off
 */
void  svkSpectraReferenceViewController::TurnPropOff(int propIndex)
{
        view->TurnPropOff( propIndex );
        // Used as an update, to catch collection change
        static_cast<svkSpectraReferenceView*>( view )->SetTlcBrc( GetTlcBrc() ); 
}

void svkSpectraReferenceViewController::SetOverlayOpacity(double opacity){
    static_cast<svkSpectraReferenceView*>( view )->SetOverlayOpacity( opacity );
}

void svkSpectraReferenceViewController::SetOverlayThreshold(double threshold){
    static_cast<svkSpectraReferenceView*>( view )->SetOverlayThreshold( threshold );
}

double svkSpectraReferenceViewController::GetOverlayThreshold(){
    return static_cast<svkSpectraReferenceView*>( view )->GetOverlayThreshold( );
}


double svkSpectraReferenceViewController::GetOverlayThresholdValue(){
    return static_cast<svkSpectraReferenceView*>( view )->GetLookupTable( )->GetAlphaThresholdValue();
}

/*!
 *  Updates the currently selected voxels. 
 */
void svkSpectraReferenceViewController::UpdateSelection(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata)
{
    svkSpectraReferenceViewController* dvController = static_cast<svkSpectraReferenceViewController*>(thisObject);
    svkOverlaySelector* myRubberband = dvController->dragSelectStyle;

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
    static_cast<svkSpectraReferenceView*>(dvController->GetView())->SetSelection( selectionArea);
    rwi->InvokeEvent(vtkCommand::SelectionChangedEvent);
    delete[] selectionArea;
}


void svkSpectraReferenceViewController::ColorWindowLevel( vtkObject* subject, unsigned long eid, void* thisObject, void *calldata) 
{
    svkSpectraReferenceViewController* dvController = static_cast<svkSpectraReferenceViewController*>(thisObject);
    svkSpectraReferenceView* myView = static_cast<svkSpectraReferenceView*>(dvController->GetView());
    if( myView->windowLevelerAxial == NULL ) {
        return;
    } 
    if (eid == vtkCommand::ResetWindowLevelEvent) {
        dvController->dataVector[MET]->UpdateInformation();
        dvController->dataVector[MET]->SetUpdateExtent( 
        dvController->dataVector[MET]->GetWholeExtent() );
        dvController->dataVector[MET]->Update();
        double *range = dvController->dataVector[MET]->GetScalarRange();
        myView->windowLevelerAxial->GetLookupTable()->SetRange(range[0], range[1]);
        myView->windowLevelerCoronal->GetLookupTable()->SetRange(range[0], range[1]);
        myView->windowLevelerSagittal->GetLookupTable()->SetRange(range[0], range[1]);
        myView->Refresh();
        return;
    }
      // Start
    // Adjust the window level here

    vtkInteractorStyleImage *isi = 
    static_cast<vtkInteractorStyleImage *>(subject);
    int *size = dvController->myRenderWindow->GetSize();

    if (eid == vtkCommand::StartWindowLevelEvent) {
        double* range = myView->windowLevelerAxial->GetLookupTable()->GetRange();
        dvController->initialColorWindowLevel[0] = range[1] - range[0];
        dvController->initialColorWindowLevel[1] = range[0] + (range[1] - range[0])/2;
        return;
    }

    double window = dvController->initialColorWindowLevel[0];
    double level = dvController->initialColorWindowLevel[1];
      
    // Compute normalized delta

    double dx = 4.0 * 
        (isi->GetWindowLevelCurrentPosition()[0] - 
         isi->GetWindowLevelStartPosition()[0]) / size[0];
    double dy = 4.0 * 
        (isi->GetWindowLevelStartPosition()[1] - 
         isi->GetWindowLevelCurrentPosition()[1]) / size[1];
      
      // Scale by current values

    if (fabs(window) > 0.01) {
        dx = dx * window;
    } else {
        dx = dx * (window < 0 ? -0.01 : 0.01);
    }

    if (fabs(level) > 0.01) {
        dy = dy * level;
    } else {
        dy = dy * (level < 0 ? -0.01 : 0.01);
    }
      
    // Abs so that direction does not flip

    if (window < 0.0) {
        dx = -1*dx;
    }
    if (level < 0.0) {
        dy = -1*dy;
    }
      
    // Compute new window level

    double newWindow = dx + window;
    double newLevel;
    newLevel = level - dy;
      
    // Stay away from zero and really

    if (fabs(newWindow) < 0.01) {
        newWindow = 0.01*(newWindow < 0 ? -1 : 1);
    }
    if (fabs(newLevel) < 0.01) {
        newLevel = 0.01*(newLevel < 0 ? -1 : 1);
    }
      
    if( newLevel - newWindow < newLevel + newWindow ) {
        myView->colorTransfer->SetRange( newLevel - newWindow/2.0, newLevel + newWindow/2.0);
    }

    //myView->GetProp( svkSpectraReferenceView::OVERLAY_IMAGE )->Modified();
    
    dvController->myRenderWindow->GetInteractor()->InvokeEvent(vtkCommand::WindowLevelEvent);

    // we will need to know when the event ends for other objects To Update themselves
    if (eid == vtkCommand::EndWindowLevelEvent) {
        dvController->myRenderWindow->GetInteractor()->InvokeEvent(vtkCommand::EndWindowLevelEvent);
    }
    myView->Refresh();
}


void svkSpectraReferenceViewController::HighlightSelectionVoxels() 
{
    static_cast<svkSpectraReferenceView*>(view)->HighlightSelectionVoxels();
}


string svkSpectraReferenceViewController::GetDataCompatibility( svkImageData* data, int targetIndex )
{
    return static_cast<svkSpectraReferenceView*>(view)->GetDataCompatibility( data, targetIndex );
}


/*!
 *
 */
void svkSpectraReferenceViewController::SetInterpolationType(int interpolationType)
{
    static_cast<svkSpectraReferenceView*>(this->view)->SetInterpolationType( interpolationType );
}


/*!
 *
 */
void svkSpectraReferenceViewController::SetLUT( svkLookupTable::svkLookupTableType type )
{
    static_cast<svkSpectraReferenceView*>(this->view)->SetLUT( type );
}


/*
 *  Turns the orthogonal images on.
 */
void svkSpectraReferenceViewController::TurnOrthogonalImagesOn()
{
    static_cast<svkSpectraReferenceView*>(this->view)->TurnOrthogonalImagesOn();
}


/*
 *  Turns the orthogonal images off.
 */
void svkSpectraReferenceViewController::TurnOrthogonalImagesOff()
{
    static_cast<svkSpectraReferenceView*>(this->view)->TurnOrthogonalImagesOff();
}


/*!
 *
 */
bool svkSpectraReferenceViewController::IsImageInsideSpectra()
{
    return static_cast<svkSpectraReferenceView*>(this->view)->IsImageInsideSpectra();
}
