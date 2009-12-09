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


#define DEBUG 0

#include <svkOverlayViewController.h>


using namespace svk;


vtkCxxRevisionMacro(svkOverlayViewController, "$Rev$");
vtkStandardNewMacro(svkOverlayViewController);


/*!
 *  Constructor initializes the associated DataView and initializes callbacks.
 */
svkOverlayViewController::svkOverlayViewController()
{
    slice = 0;
    visualizationCreated = 0;

    // Create View 
    view = svkOverlayView::New();
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
    this->view->SetRenderer(svkOverlayView::PRIMARY, primaryRenderer );
    primaryRenderer->Delete();

    vtkRenderer* mouseLocationRenderer = vtkRenderer::New();
    this->view->SetRenderer( svkOverlayView::MOUSE_LOCATION, mouseLocationRenderer );
    mouseLocationRenderer->Delete();

    // Data Vector initialization
    dataVector.push_back( NULL );
    dataVector.push_back( NULL );
    dataVector.push_back( NULL );
    
    // Null initializations
    rwi = NULL;
    vtkTextActor* coordinatesActor = vtkTextActor::New();
    this->view->SetProp(svkOverlayView::COORDINATES, coordinatesActor );
    coordinatesActor->Delete();
    static_cast<vtkTextActor*>(this->view->GetProp(svkOverlayView::COORDINATES))->SetTextScaleModeToProp();
    windowLevelStyle = NULL;
    dragSelectStyle = svkOverlaySelector::New();
    rotationStyle = vtkInteractorStyleTrackballCamera::New();
    colorOverlayStyle = vtkInteractorStyleImage::New();
    windowLevelStyle = NULL;
    scanBoundryActor = NULL;
    // DON'T FORGET TO FREE RESOURCES!!
}


/*! 
 *  Destructor.
 */
svkOverlayViewController::~svkOverlayViewController()
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


/*! 
 *  Creates the patientInfo text actor, and intitializes the mouse
 *  position text.
 *
 */
void svkOverlayViewController::InitDisplayText( )
{
    vtkCornerAnnotation* patientInfo = vtkCornerAnnotation::New();
    double* pixelSpacing;
    float* imageOrigin;
    float defaultOrigin[] = {0,0,0};
    double defaultSpacing[] = {0,0,0};
    
    stringstream out;
    //Create Text Actors to show mouse position and patient info
    static_cast<vtkTextActor*>(this->view->GetProp(svkOverlayView::COORDINATES))->SetInput("x: null \ny: null \n z: null");
    imageOrigin = defaultOrigin; 
    this->view->GetRenderer(svkOverlayView::MOUSE_LOCATION)->AddActor( 
        this->view->GetProp(svkOverlayView::COORDINATES)
    );
    out.setf(ios::fixed,ios::floatfield);
    out.precision(2);
    out<<"Image Info--"<<endl;
    out<<" Study ID: "<<"uk"<<endl;
    out<<" Size: "<<0<<"x"<<0<<endl;
    out<<" Origin: ( "<<*(imageOrigin);
    out<<", "<<*(imageOrigin + 1);
    out<<", "<<*(imageOrigin + 2)<<" )"<<endl;
    pixelSpacing = defaultSpacing;
    out<<" Spacing: ( "<<*(pixelSpacing);
    out<<", "<<*(pixelSpacing + 1);
    out<<", "<<*(pixelSpacing + 2)<<" )";
    patientInfo->SetText(2,(out.str()).c_str());
    patientInfo->SetLayerNumber(1);
    static_cast<vtkTextActor*>(this->view->GetProp(svkOverlayView::COORDINATES))->SetLayerNumber(1);
    //myRenderWindow->GetRenderers()->GetFirstRenderer()->AddActor( patientInfo ); 
    patientInfo->Delete();
}

void svkOverlayViewController::Reset() 
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
    this->view = svkOverlayView::New();
    this->view->SetController(this);
    // New objects
    vtkRenderer* primaryRenderer = vtkRenderer::New();
    this->view->SetRenderer(svkOverlayView::PRIMARY, primaryRenderer );
    primaryRenderer->Delete();

    vtkRenderer* mouseLocationRenderer = vtkRenderer::New();
    this->view->SetRenderer( svkOverlayView::MOUSE_LOCATION, mouseLocationRenderer );
    mouseLocationRenderer->Delete();

    vtkTextActor* coordinatesActor = vtkTextActor::New();
    this->view->SetProp(svkOverlayView::COORDINATES, coordinatesActor );
    coordinatesActor->Delete();
    static_cast<vtkTextActor*>(this->view->GetProp(svkOverlayView::COORDINATES))->SetTextScaleModeToProp();

    this->rwi->RemoveObserver( dragSelectionCB);
    this->rwi->RemoveObserver( colorOverlayCB);
    this->rwi->RemoveObserver( cursorLocationCB);
    this->SetRWInteractor( this->rwi );
    visualizationCreated = 0;
}


/*!
 *  Setter method. Also calls the SetInput method of its view.
 *  svkOverlayView has two inputs, an image and a spectroscopy
 *  data set.
 *
 *  \param data the data you wish to view
 *
 *  \param index the index of the input you wish to set.
 */
void svkOverlayViewController::SetInput( svkImageData* data, int index)
{
    if( index == MRI && data != NULL ) { // 0 for anatomical data
        if( dataVector[MRI] != NULL ) {
            dataVector[MRI]->Delete();
        }
        data->Register( this );
        dataVector[MRI] = data;
        this->view->SetInput(data);
        if( !visualizationCreated ) { 
            CreateDataVisualization();
            if( dataVector[MRS] != NULL ) {
                this->view->SetInput(dataVector[MRS], 1); 
            }
        }
        this->SetSlice( this->slice );
    } else if( index == MRS && data != NULL ) {
        if( dataVector[MRS] != NULL ) {
            dataVector[MRS]->Delete();
        }
        data->Register( this );
        dataVector[MRS] = data;
        if( dataVector[MRI] != NULL ) {
            this->view->SetInput(data, 1);
            this->SetSlice( this->slice );
        }
    } else if( index == MET && data != NULL && dataVector[MRI] != NULL && dataVector[MRS]!=NULL ) {
        if( dataVector[MET] != NULL ) {
            dataVector[MET]->Delete();
        }
        data->Register( this );
        dataVector[MET] = data;
        this->view->SetInput( data, MET );
    } else if( data != NULL ) {
        cout<<"WARNING: svkOverlayViewController only takes two image inputs!"<<endl;
    } else {
        cout<<"WARNING: data is NULL !"<<endl;
    
    }

}


/*!
 *  Primary method, this consructs the actual workspace. It generates the
 *  selection style interactor, creates the voxel actors, adds them to the
 *  renderer, and sets up callbacks.
 */
void svkOverlayViewController::CreateDataVisualization( )
{
    vtkCubeSource* textBacking = vtkCubeSource::New();
    textBacking->SetBounds(0,1,0,1,0,0);
    vtkPolyDataMapper2D* myPD2DMapper = vtkPolyDataMapper2D::New();
    myPD2DMapper->SetInput( textBacking->GetOutput() );
   
    vtkActor2D* backingActor = vtkActor2D::New(); 
    backingActor->SetMapper( myPD2DMapper);
    backingActor->GetProperty()->SetColor( 0, 0, 0 );

    vtkCoordinate* viewTranslator = vtkCoordinate::New();
    viewTranslator->SetCoordinateSystemToNormalizedDisplay(); 
    
    myPD2DMapper->SetTransformCoordinate( viewTranslator );
    vtkTextActor* text = static_cast<vtkTextActor*>(this->view->GetProp(svkOverlayView::COORDINATES)); 
    text->SetPosition(0.1,0.1); 
    text->SetPosition2(0.9,0.9); 
    text->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    text->GetPosition2Coordinate()->SetCoordinateSystemToNormalizedViewport();
    this->view->GetRenderer(svkOverlayView::MOUSE_LOCATION)->SetBackground( 0.1,0.2,0.4);
    this->view->GetRenderer(svkOverlayView::MOUSE_LOCATION)->AddActor( backingActor );
    textBacking->Delete();
    myPD2DMapper->Delete();
    backingActor->Delete();
    viewTranslator->Delete();
    // Setup view
    // Initialize the viewer, which is globally accesible 

    // Load anatamical and spectroscopic data
    InitDisplayText( );

    // Get pointers to the two interactor observers to allow switching
    if( windowLevelStyle != NULL && windowLevelStyle != rwi->GetInteractorStyle() ) {
        windowLevelStyle->Delete();
    }
    windowLevelStyle = rwi->GetInteractorStyle();
    windowLevelStyle->Register( this );

    this->view->GetRenderer(svkOverlayView::MOUSE_LOCATION)->SetViewport( 0.0, 0.0, 0.2, 0.15 );
    this->view->GetRenderer(svkOverlayView::MOUSE_LOCATION)->InteractiveOff();
    this->myRenderWindow->SetNumberOfLayers(2);
    this->view->GetRenderer(svkOverlayView::MOUSE_LOCATION)->SetLayer(1);
    this->view->GetRenderer(svkOverlayView::PRIMARY)->SetLayer(0);
    this->view->GetRenderer(svkOverlayView::PRIMARY)->BackingStoreOn();
    this->view->TurnRendererOn( svkOverlayView::MOUSE_LOCATION );
     
    this->rwi->AddObserver(vtkCommand::MouseMoveEvent, cursorLocationCB);
    rwi->AddObserver(vtkCommand::KeyPressEvent, interactorSwitchCB);
    
 
    rwi->AddObserver(vtkCommand::LeftButtonReleaseEvent, dragSelectionCB);
    colorOverlayStyle->AddObserver(vtkCommand::StartWindowLevelEvent, colorOverlayCB);
    colorOverlayStyle->AddObserver(vtkCommand::WindowLevelEvent, colorOverlayCB);
    colorOverlayStyle->AddObserver(vtkCommand::ResetWindowLevelEvent, colorOverlayCB);
    colorOverlayStyle->AddObserver(vtkCommand::PickEvent, colorOverlayCB);
    colorOverlayStyle->AddObserver(vtkCommand::StartPickEvent, colorOverlayCB);
    
    // Set default style to drag selection
    rwi->SetInteractorStyle( dragSelectStyle );
    visualizationCreated = 1;
}


/*!
 *  Setter. Should be modified to take action once the slice is set,
 *  and to modify its View to do the same. 
 *
 *  \param slice the slice you want to view
 *  \param the image you want to change the slice of, 0 is primary 1 and 2 are orthogonal 
 */
void svkOverlayViewController::SetSlice( int slice )
{
    this->slice = slice;
    if( visualizationCreated ) {
        this->view->SetSlice( slice );
    } else if (DEBUG) {
        cout<<"Visualization has not yet been created!!"<<endl;
    }
}

/*!
 *  Setter. Should be modified to take action once the slice is set,
 *  and to modify its View to do the same. 
 *
 *  \param slice the slice you want to view
 *  \param the image you want to change the slice of, 0 is primary 1 and 2 are orthogonal 
 */
void svkOverlayViewController::SetSlice(int slice, int imageNum)
{
    if( visualizationCreated ) {
        static_cast<svkOverlayView*>(this->view)->SetSlice( slice, imageNum );
    } else if (DEBUG) {
        cout<<"Visualization has not yet been created!!"<<endl;
    }
}


/*!
 *  Getter for the current image slice, as opposed to the spectroscopic slice.
 *
 */
int svkOverlayViewController::GetImageSlice()
{
    return static_cast<svkOverlayView*>(this->GetView())->imageViewer->GetSlice(); 
}

/*!
 *  Setter method. Also gets the render window from the render window actor.
 *  Once these have been set, CreateDataVisualization() is called and starts the actual
 *  create of the visualization.
 *
 *  \param rwi the vtkRenderWindowInteractor you want the View to use
 *
 */
void svkOverlayViewController::SetRWInteractor(vtkRenderWindowInteractor* rwi)
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

        
        // Now if the data has already been set, go ahead and generate the viz
        if( dataVector[MRI] != NULL ) {
            CreateDataVisualization();
        }

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
void svkOverlayViewController::SetTlcBrc( int* tlcBrc) 
{
    int* currentTlcBrc = GetTlcBrc();
    if( tlcBrc != NULL ) {
        if( currentTlcBrc != NULL ) {
            if( tlcBrc[0] != currentTlcBrc[0] || tlcBrc[1] != currentTlcBrc[1] ) {
                static_cast<svkOverlayView*>( view )->SelectActors( tlcBrc ); 
            }
        } else {
            static_cast<svkOverlayView*>( view )->SelectActors( tlcBrc ); 
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
int* svkOverlayViewController::GetTlcBrc() 
{
    return static_cast<svkOverlayView*>(view)->tlcBrc;
}


int svkOverlayViewController::GetCurrentStyle() 
{
    return this->currentInteractorStyle;
}

//! Switches to the window level style interactor
void svkOverlayViewController::UseColorOverlayStyle() 
{
    if( visualizationCreated ) {
        this->myRenderWindow->SetNumberOfLayers(1);
        //this->myRenderWindow->RemoveRenderer(  this->view->GetRenderer(svkOverlayView::MOUSE_LOCATION) );
        this->view->TurnRendererOff( svkOverlayView::MOUSE_LOCATION );
        this->rwi->SetInteractorStyle( colorOverlayStyle ); 
        this->currentInteractorStyle = COLOR_OVERLAY;
    }

}

//! Switches to the window level style interactor
void svkOverlayViewController::UseWindowLevelStyle() 
{
    if( visualizationCreated ) {
        view->TurnPropOff(svkOverlayView::SAT_BANDS_PERP1); 
        view->TurnPropOff(svkOverlayView::SAT_BANDS_PERP2); 
        view->TurnPropOff(svkOverlayView::SAT_BANDS_PERP1_OUTLINE); 
        view->TurnPropOff(svkOverlayView::SAT_BANDS_PERP2_OUTLINE); 
        this->myRenderWindow->SetNumberOfLayers(1);
        //this->myRenderWindow->RemoveRenderer( this->view->GetRenderer(svkOverlayView::MOUSE_LOCATION) );
        this->view->TurnRendererOff( svkOverlayView::MOUSE_LOCATION );
        this->rwi->SetInteractorStyle( windowLevelStyle ); 
        this->currentInteractorStyle = WINDOW_LEVEL;
    }

}


//! Switches to the selection style interactor
void svkOverlayViewController::UseSelectionStyle() 
{
    if( visualizationCreated ) {
        if( !this->myRenderWindow->HasRenderer( this->view->GetRenderer(svkOverlayView::MOUSE_LOCATION) ) ) {
            this->myRenderWindow->SetNumberOfLayers(2);
            this->view->TurnRendererOn( svkOverlayView::MOUSE_LOCATION );
            this->view->GetRenderer(svkOverlayView::PRIMARY)->BackingStoreOn();
        }
        double dcos[3][3];
        dataVector[MRI]->GetDcos( dcos );
        double wVec[3];
        wVec[0] = dcos[2][0];
        wVec[1] = dcos[2][1];
        wVec[2] = dcos[2][2];
        double* viewPlaneNormal =  this->view->GetRenderer(svkOverlayView::PRIMARY)->GetActiveCamera()->GetViewPlaneNormal();
        
        // Lets make sure orthogonal sat bands are off...
        view->TurnPropOff(svkOverlayView::SAT_BANDS_PERP1); 
        view->TurnPropOff(svkOverlayView::SAT_BANDS_PERP2); 
        view->TurnPropOff(svkOverlayView::SAT_BANDS_PERP1_OUTLINE); 
        view->TurnPropOff(svkOverlayView::SAT_BANDS_PERP2_OUTLINE); 
      
        // We are going to turn the sat bands off if they were on just for resetting the camera
        // If left on the, the range if the sat bands changes the fov. 
        bool satBandsOn = 0; 
        bool satBandsOutlineOn = 0; 
        if( view->IsPropOn(svkOverlayView::SAT_BANDS) ) { 
            satBandsOn = 1;
            view->TurnPropOff(svkOverlayView::SAT_BANDS);
        }
        if( view->IsPropOn(svkOverlayView::SAT_BANDS_OUTLINE) ) { 
            satBandsOutlineOn = 1;
            view->TurnPropOff(svkOverlayView::SAT_BANDS_OUTLINE);
        }
            
        rwi->SetInteractorStyle( dragSelectStyle ); 
        if( (int)(viewPlaneNormal[0] * 100) != (int)(wVec[0]*100) ||
            (int)(viewPlaneNormal[1] * 100) != (int)(wVec[1]*100) ||
            (int)(viewPlaneNormal[2] * 100) != (int)(wVec[2]*100) ) {
            (static_cast<svkOverlayView*>(view))->imageViewer->ResetCamera();
        }
        if( satBandsOn ) { 
            view->TurnPropOn(svkOverlayView::SAT_BANDS);
        }
        if( satBandsOutlineOn ) { 
            view->TurnPropOn(svkOverlayView::SAT_BANDS_OUTLINE);
        }
        this->currentInteractorStyle = SELECTION;
    }

}


//! Switches to the rotation style interactor
void svkOverlayViewController::UseRotationStyle()
{
    if( visualizationCreated ) {
        if( view->IsPropOn(svkOverlayView::SAT_BANDS) ) { 
            view->TurnPropOn(svkOverlayView::SAT_BANDS_PERP1); 
            view->TurnPropOn(svkOverlayView::SAT_BANDS_PERP2); 
        }
        if( view->IsPropOn(svkOverlayView::SAT_BANDS_OUTLINE) ) { 
            view->TurnPropOn(svkOverlayView::SAT_BANDS_PERP1_OUTLINE); 
            view->TurnPropOn(svkOverlayView::SAT_BANDS_PERP2_OUTLINE); 
        }
        svkOverlayView* myView = static_cast<svkOverlayView*>(view);
        //this->myRenderWindow->RemoveRenderer( this->view->GetRenderer(svkOverlayView::MOUSE_LOCATION) );
        this->view->TurnRendererOff( svkOverlayView::MOUSE_LOCATION );
        this->myRenderWindow->SetNumberOfLayers(1);
        this->view->GetRenderer(svkOverlayView::PRIMARY)->BackingStoreOff();
        rwi->SetInteractorStyle( rotationStyle );
        this->currentInteractorStyle = ROTATION;
    }

}


//! Resets the window level, source taken from vtkImageViewer2
void svkOverlayViewController::ResetWindowLevel()
{
    svkOverlayView* myView = static_cast<svkOverlayView*>(view);
    myView->ResetWindowLevel();
}


/*!
 *  Method to update the renderer that displays the mouse location. Since it is not clear where the image
 *  lies in the z-buffer, the location is assumed to be z = 0, then the resulting point is projected onto the
 *  image. 
 */
void svkOverlayViewController::UpdateCursorLocation(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata)
{
    svkOverlayViewController* dvController = static_cast<svkOverlayViewController*>(thisObject);
    int pos[2];
    double* imageCords;
    std::stringstream out;
    vtkCoordinate* mousePosition = vtkCoordinate::New();
    vtkRenderWindowInteractor *rwi = 
            vtkRenderWindowInteractor::SafeDownCast( subject );

    //vtkRenderer* viewerRenderer = rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
    vtkRenderer* viewerRenderer = dvController->GetView()->GetRenderer(svkOverlayView::PRIMARY);
    //rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
    pos[0] = dvController->rwi->GetEventPosition()[0];
    pos[1] = dvController->rwi->GetEventPosition()[1];
    double dcos[3][3];
    double distance = viewerRenderer->GetActiveCamera()->GetDistance(); 
    double* origin;
    double* spacing;
    double wVec[3];
    double xVec[3];
    double yVec[3];
    double zVec[3];
    double planeOrigin[3];
    double projection[3];
    // We need the anatomical slice to calculate a point on the image
    int slice; 
    if( static_cast<svkOverlayView*>(dvController->GetView())->dataVector[MRI] != NULL ) {
        // Calculate plane origin
        if( static_cast<svkOverlayView*>(dvController->GetView())->dataVector[MRS] != NULL ) {
            static_cast<svkOverlayView*>(dvController->GetView())->dataVector[MRS]->GetDcos( dcos );
            slice = static_cast<svkOverlayView*>(dvController->GetView())->slice; 
            static_cast<svkOverlayView*>(dvController->GetView())->dataVector[MRS]->GetDcmHeader()->GetOrigin(
                planeOrigin, slice
            ); 
        } else {
            static_cast<svkOverlayView*>(dvController->GetView())->dataVector[MRI]->GetDcos( dcos );
            slice = static_cast<svkOverlayView*>(dvController->GetView())->imageViewer->GetSlice(); 
            static_cast<svkOverlayView*>(dvController->GetView())->dataVector[MRI]->GetDcmHeader()->GetOrigin(
                planeOrigin, slice
            ); 
        }
        mousePosition->SetCoordinateSystemToDisplay();
        mousePosition->SetValue( pos[0], pos[1], 0); 
        imageCords = mousePosition->GetComputedWorldValue( viewerRenderer );
        // Project selection point onto the image

        // We need to extract the vectors of the dcos for the projection
        wVec[0] = dcos[2][0];
        wVec[1] = dcos[2][1];
        wVec[2] = dcos[2][2];
        xVec[0] = dcos[0][0];
        xVec[1] = dcos[1][0];
        xVec[2] = dcos[2][0];
        yVec[0] = dcos[0][1];
        yVec[1] = dcos[1][1];
        yVec[2] = dcos[2][1];
        zVec[0] = dcos[0][2];
        zVec[1] = dcos[1][2];
        zVec[2] = dcos[2][2];

        vtkPlane::GeneralizedProjectPoint( imageCords, planeOrigin, wVec, projection );

        out.setf(ios::fixed,ios::floatfield); 
        out.precision(1);
        out << "x: " << projection[0] << endl;
        out << "y: " << projection[1] << endl;    
        out << "z: " << projection[2];    
        static_cast<vtkTextActor*>(dvController->view->GetProp(svkOverlayView::COORDINATES))->SetInput((out.str()).c_str());
    } else { 
        out<<"x: "<<endl;
        out<<"y: "<<endl;    
        out<<"z: ";    
        static_cast<vtkTextActor*>(dvController->view->GetProp(svkOverlayView::COORDINATES))->SetInput((out.str()).c_str());
    }
    mousePosition->Delete();
}


/* 
 *  Method switches the current interactor, between the selection interactor
 *  and the window level interactor.
 */
void svkOverlayViewController::UpdateInteractor(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata)
{
    svkOverlayViewController* dvController = static_cast<svkOverlayViewController*>(thisObject);

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
void svkOverlayViewController::TurnPropOn(int propIndex)
{
    view->TurnPropOn( propIndex );
    if( visualizationCreated ) {
        // Used as an update, to catch collection change
        static_cast<svkOverlayView*>( view )->SelectActors( GetTlcBrc() ); 
        view->SetSlice( slice ); 
    }   
}


/*!
 *  Turn off the vtkActorCollection representing a given topology for a given image.
 *
 *  \param actorIndex the index of the topology you want to turn off
 *
 *  \param imageIndex the index of the image who's topology you want turned off
 */
void  svkOverlayViewController::TurnPropOff(int propIndex)
{
    view->TurnPropOff( propIndex );
    if( visualizationCreated ) {
        // Used as an update, to catch collection change
        static_cast<svkOverlayView*>( view )->SelectActors( GetTlcBrc() ); 
        view->SetSlice( slice ); 
    }   
}

void svkOverlayViewController::SetOverlayOpacity(double opacity){
    static_cast<svkOverlayView*>( view )->SetOverlayOpacity( opacity );
}

void svkOverlayViewController::SetOverlayThreshold(double threshold){
    static_cast<svkOverlayView*>( view )->SetOverlayThreshold( threshold );
}

/*!
 *  Updates the currently selected voxels. 
 */
void svkOverlayViewController::UpdateSelection(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata)
{
    svkOverlayViewController* dvController = static_cast<svkOverlayViewController*>(thisObject);
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
    static_cast<svkOverlayView*>(dvController->GetView())->SetSelection( selectionArea);
    rwi->InvokeEvent(vtkCommand::SelectionChangedEvent);
    delete[] selectionArea;
}


void svkOverlayViewController::ColorWindowLevel( vtkObject* subject, unsigned long eid, void* thisObject, void *calldata) 
{
    svkOverlayViewController* dvController = static_cast<svkOverlayViewController*>(thisObject);
    svkOverlayView* myView = static_cast<svkOverlayView*>(dvController->GetView());
    if( myView->windowLeveler == NULL ) {
        return;
    } 
    if (eid == vtkCommand::ResetWindowLevelEvent) {
        dvController->dataVector[MET]->UpdateInformation();
        dvController->dataVector[MET]->SetUpdateExtent( 
        dvController->dataVector[MET]->GetWholeExtent() );
        dvController->dataVector[MET]->Update();
        double *range = dvController->dataVector[MET]->GetScalarRange();
        myView->windowLeveler->GetLookupTable()->SetRange(range[0], range[1]);
        myView->GetProp( svkOverlayView::OVERLAY_IMAGE )->Modified();
        myView->Refresh();
        return;
    }
      // Start
    // Adjust the window level here

    vtkInteractorStyleImage *isi = 
    static_cast<vtkInteractorStyleImage *>(subject);
    int *size = dvController->myRenderWindow->GetSize();

    if (eid == vtkCommand::StartWindowLevelEvent) {
        double* range = myView->windowLeveler->GetLookupTable()->GetRange();
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

    myView->GetProp( svkOverlayView::OVERLAY_IMAGE )->Modified();
    myView->Refresh();
}


void svkOverlayViewController::HighlightSelectionVoxels() 
{
    static_cast<svkOverlayView*>(view)->HighlightSelectionVoxels();
}


string svkOverlayViewController::GetDataCompatibility( svkImageData* data, int targetIndex )
{
    return static_cast<svkOverlayView*>(view)->GetDataCompatibility( data, targetIndex );
}


/*!
 *
 */
void svkOverlayViewController::SetInterpolationType(int interpolationType)
{
    static_cast<svkOverlayView*>(this->view)->SetInterpolationType( interpolationType );
}


/*!
 *
 */
void svkOverlayViewController::SetLUT( svkLookupTable::svkLookupTableType type )
{
    static_cast<svkOverlayView*>(this->view)->SetLUT( type );
}


/*
 *  Turns the orthogonal images on.
 */
void svkOverlayViewController::TurnOrthogonalImagesOn()
{
    static_cast<svkOverlayView*>(this->view)->TurnOrthogonalImagesOn();
}


/*
 *  Turns the orthogonal images off.
 */
void svkOverlayViewController::TurnOrthogonalImagesOff()
{
    static_cast<svkOverlayView*>(this->view)->TurnOrthogonalImagesOff();
}

