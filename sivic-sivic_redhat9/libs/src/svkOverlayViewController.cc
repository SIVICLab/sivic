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


#include <svkOverlayViewController.h>


using namespace svk;


//vtkCxxRevisionMacro(svkOverlayViewController, "$Rev$");
vtkStandardNewMacro(svkOverlayViewController);


/*!
 *  Constructor initializes the associated DataView and initializes callbacks.
 */
svkOverlayViewController::svkOverlayViewController()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

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
    static_cast<vtkTextActor*>(this->view->GetProp(svkOverlayView::COORDINATES))->SetInput("x: null  y: null  z: null  v: null");
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
    if( dataVector[MR4D] != NULL ) {
        dataVector[MR4D]->Delete();
        dataVector[MR4D] = NULL;
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
            if( dataVector[MR4D] != NULL ) {
                this->view->SetInput(dataVector[MR4D], svkOverlayView::MR4D );
            }
        }
    } else if( index == MR4D && data != NULL ) {
        if( dataVector[MR4D] != NULL ) {
            dataVector[MR4D]->Delete();
            dataVector[MR4D] = NULL;
        }
        data->Register( this );
        dataVector[MR4D] = data;
        if( dataVector[MRI] != NULL ) {
            this->view->SetInput(data, 1);
            this->SetSlice( this->GetView()->GetSlice() );
        }
    } else if( index == svkOverlayView::OVERLAY && dataVector[MRI] != NULL ) {
        if( dataVector[svkOverlayView::OVERLAY] != NULL ) {
            dataVector[svkOverlayView::OVERLAY]->Delete();
        }
        data->Register( this );
        dataVector[svkOverlayView::OVERLAY] = data;
        this->view->SetInput( data, svkOverlayView::OVERLAY );
    } else if( index == svkOverlayView::OVERLAY_CONTOUR && dataVector[MRI] != NULL ) {
        data->Register( this );
        dataVector.push_back(data);
        this->view->SetInput( data, svkOverlayView::OVERLAY_CONTOUR );
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
    myPD2DMapper->SetInputData( textBacking->GetOutput() );
   
    vtkActor2D* backingActor = vtkActor2D::New(); 
    backingActor->SetMapper( myPD2DMapper);
    backingActor->GetProperty()->SetColor( 0, 0, 0 );

    vtkCoordinate* viewTranslator = vtkCoordinate::New();
    viewTranslator->SetCoordinateSystemToNormalizedDisplay(); 
    
    myPD2DMapper->SetTransformCoordinate( viewTranslator );
    vtkTextActor* text = static_cast<vtkTextActor*>(this->view->GetProp(svkOverlayView::COORDINATES)); 
    text->GetTextProperty()->SetFontFamilyToCourier();
    text->SetPosition(0.01,0.01);
    text->SetPosition2(0.95,0.95);
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

    this->view->GetRenderer(svkOverlayView::MOUSE_LOCATION)->SetViewport( 0.0, 0.0, 1.0, 0.05 );
    this->view->GetRenderer(svkOverlayView::MOUSE_LOCATION)->InteractiveOff();
    this->myRenderWindow->SetNumberOfLayers(2);
    this->view->GetRenderer(svkOverlayView::MOUSE_LOCATION)->SetLayer(1);
    this->view->GetRenderer(svkOverlayView::PRIMARY)->SetLayer(0);
    this->view->GetRenderer(svkOverlayView::PRIMARY)->BackingStoreOff();
    this->view->TurnRendererOn( svkOverlayView::MOUSE_LOCATION );
     
    this->rwi->AddObserver(vtkCommand::MouseMoveEvent, cursorLocationCB);
    rwi->AddObserver(vtkCommand::KeyPressEvent, interactorSwitchCB);
    
 
    rwi->AddObserver(vtkCommand::LeftButtonReleaseEvent, dragSelectionCB);
    colorOverlayStyle->AddObserver(vtkCommand::StartWindowLevelEvent, colorOverlayCB);
    colorOverlayStyle->AddObserver(vtkCommand::WindowLevelEvent, colorOverlayCB);
    colorOverlayStyle->AddObserver(vtkCommand::EndWindowLevelEvent, colorOverlayCB);
    colorOverlayStyle->AddObserver(vtkCommand::ResetWindowLevelEvent, colorOverlayCB);
    colorOverlayStyle->AddObserver(vtkCommand::InteractionEvent, colorOverlayCB);
    colorOverlayStyle->AddObserver(vtkCommand::PickEvent, colorOverlayCB);
    colorOverlayStyle->AddObserver(vtkCommand::StartPickEvent, colorOverlayCB);
    
    // Set default style to drag selection
    this->rwi->SetInteractorStyle( dragSelectStyle );
    this->rwi->Enable(); 
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
    if( visualizationCreated ) {
        this->view->SetSlice( slice );
    } else if (this->GetDebug()) {
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
void svkOverlayViewController::SetSlice( int slice, bool centerImage )
{
    if( visualizationCreated ) {
        svkOverlayView::SafeDownCast( this->view )->SetSlice( slice, centerImage );
    } else if (this->GetDebug()) {
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
void svkOverlayViewController::SetSlice(int slice, svkDcmHeader::Orientation orientation)
{
    if( visualizationCreated ) {
        static_cast<svkOverlayView*>(this->view)->SetSlice( slice, orientation );
    } else if (this->GetDebug()) {
        cout<<"Visualization has not yet been created!!"<<endl;
    }
}


/*!
 *  Getter for the current image slice, as opposed to the spectroscopic slice.
 *
 */
int svkOverlayViewController::GetImageSlice( svkDcmHeader::Orientation sliceOrientation )
{
    sliceOrientation = (sliceOrientation == svkDcmHeader::UNKNOWN_ORIENTATION ) ? this->GetView()->GetOrientation() : sliceOrientation;
    return static_cast<svkOverlayView*>(this->GetView())->imageViewer->GetSlice( sliceOrientation ); 
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
        for( int i = 0; i < renderers->GetNumberOfItems(); i++ ) {
			vtkObject* obj = renderers->GetItemAsObject(i);
			if( obj != NULL ) {
				vtkRenderer* renderer = vtkRenderer::SafeDownCast( obj );
				if( renderer != NULL ) {
					this->myRenderWindow->RemoveRenderer( renderer );
				}
			}
		}

        // And add the new renderer
        this->rwi->SetEventPosition(0,0);
        this->rwi->SetLastEventPosition(0,0);
        this->svkDataViewController::SetRWInteractor( this->rwi );

        
        // Now if the data has already been set, go ahead and generate the viz
        if( dataVector[MRI] != NULL ) {
            CreateDataVisualization();
        }

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
                static_cast<svkOverlayView*>( view )->SetTlcBrc( tlcBrc ); 
            }
        } else {
            static_cast<svkOverlayView*>( view )->SetTlcBrc( tlcBrc ); 
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


/*!
 *
 */
void svkOverlayViewController::SetCurrentStyle( int style ) 
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
void svkOverlayViewController::UseColorOverlayStyle() 
{
    if( visualizationCreated ) {
        this->view->GetRenderer(svkOverlayView::PRIMARY)->BackingStoreOff();
        this->myRenderWindow->SetNumberOfLayers(1);
        //this->myRenderWindow->RemoveRenderer(  this->view->GetRenderer(svkOverlayView::MOUSE_LOCATION) );
        this->view->TurnRendererOff( svkOverlayView::MOUSE_LOCATION );
        this->rwi->SetInteractorStyle( colorOverlayStyle ); 
        this->rwi->Enable(); 
        this->currentInteractorStyle = COLOR_OVERLAY;
        this->view->Refresh();
    }

}

//! Switches to the window level style interactor
void svkOverlayViewController::UseWindowLevelStyle() 
{
    if( visualizationCreated ) {
        this->view->TurnRendererOff( svkOverlayView::MOUSE_LOCATION );
        this->myRenderWindow->SetNumberOfLayers(1);
        //this->myRenderWindow->RemoveRenderer( this->view->GetRenderer(svkOverlayView::MOUSE_LOCATION) );
        this->rwi->SetInteractorStyle( windowLevelStyle ); 
        this->rwi->Enable(); 
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
        }
        this->view->GetRenderer(svkOverlayView::PRIMARY)->BackingStoreOff();
        
      
        // We are going to turn the sat bands off if they were on just for resetting the camera
        // If left on the, the range if the sat bands changes the fov. 
        bool satBandsOn = 0; 
        bool satBandsOutlineOn = 0; 
        if( view->IsPropOn(svkOverlayView::SAT_BANDS_AXIAL) ||
            view->IsPropOn(svkOverlayView::SAT_BANDS_CORONAL) || 
            view->IsPropOn(svkOverlayView::SAT_BANDS_SAGITTAL) ) { 
            satBandsOn = 1;
        }
        if( view->IsPropOn(svkOverlayView::SAT_BANDS_AXIAL_OUTLINE) ||
            view->IsPropOn(svkOverlayView::SAT_BANDS_CORONAL_OUTLINE) || 
            view->IsPropOn(svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE) ) { 
            satBandsOutlineOn = 1;
        }
            
        // Lets make sure orthogonal sat bands are off...
        view->TurnPropOff(svkOverlayView::SAT_BANDS_AXIAL); 
        view->TurnPropOff(svkOverlayView::SAT_BANDS_AXIAL_OUTLINE); 
        view->TurnPropOff(svkOverlayView::SAT_BANDS_CORONAL); 
        view->TurnPropOff(svkOverlayView::SAT_BANDS_CORONAL_OUTLINE); 
        view->TurnPropOff(svkOverlayView::SAT_BANDS_SAGITTAL); 
        view->TurnPropOff(svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE); 

        // Set style
        this->rwi->SetInteractorStyle( dragSelectStyle ); 
        this->rwi->Enable(); 

        //(static_cast<svkOverlayView*>(view))->AlignCamera();
        if( satBandsOn ) { 
            switch( this->GetView()->GetOrientation()) {
                case svkDcmHeader::AXIAL:
                    view->TurnPropOn(svkOverlayView::SAT_BANDS_AXIAL);
                    break;
                case svkDcmHeader::CORONAL:
                    view->TurnPropOn(svkOverlayView::SAT_BANDS_CORONAL);
                    break;
                case svkDcmHeader::SAGITTAL:
                    view->TurnPropOn(svkOverlayView::SAT_BANDS_SAGITTAL);
                    break;
            }
        }
        if( satBandsOutlineOn ) { 
            switch( this->GetView()->GetOrientation() ) {
                case svkDcmHeader::AXIAL:
                    view->TurnPropOn(svkOverlayView::SAT_BANDS_AXIAL_OUTLINE);
                    break;
                case svkDcmHeader::CORONAL:
                    view->TurnPropOn(svkOverlayView::SAT_BANDS_CORONAL_OUTLINE);
                    break;
                case svkDcmHeader::SAGITTAL:
                    view->TurnPropOn(svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE);
                    break;
            }
        }
        this->currentInteractorStyle = SELECTION;
        if( svkOverlayView::SafeDownCast(this->view)->GetSelectionBoxVisibility() != svkOverlayView::VISIBLE ) {
            svkOverlayView::SafeDownCast(this->view)->SetSelectionBoxVisibility(svkOverlayView::VISIBLE_WHEN_CONTAINS_CURRENT_SLICE);
        }
    }

}


//! Switches to the rotation style interactor
void svkOverlayViewController::UseRotationStyle()
{
    if( visualizationCreated ) {
        bool areSatBandsOn;
        bool areSatBandOutlinesOn;
        bool areOrthogonalImagesOn = svkOverlayView::SafeDownCast(this->view)->AreOrthogonalImagesOn();

        switch( this->view->GetOrientation() ) {
            case svkDcmHeader::AXIAL:
                 areSatBandsOn = view->IsPropOn(svkOverlayView::SAT_BANDS_AXIAL);
                 areSatBandOutlinesOn = view->IsPropOn(svkOverlayView::SAT_BANDS_AXIAL_OUTLINE);
                break;
            case svkDcmHeader::CORONAL:
                 areSatBandsOn = view->IsPropOn(svkOverlayView::SAT_BANDS_CORONAL);
                 areSatBandOutlinesOn = view->IsPropOn(svkOverlayView::SAT_BANDS_CORONAL_OUTLINE);
                break;
            case svkDcmHeader::SAGITTAL:
                 areSatBandsOn = view->IsPropOn(svkOverlayView::SAT_BANDS_SAGITTAL);
                 areSatBandOutlinesOn = view->IsPropOn(svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE);
                break;
        }
        if( areSatBandsOn && areOrthogonalImagesOn) {
            view->TurnPropOn(svkOverlayView::SAT_BANDS_AXIAL); 
            view->TurnPropOn(svkOverlayView::SAT_BANDS_CORONAL); 
            view->TurnPropOn(svkOverlayView::SAT_BANDS_SAGITTAL); 
        }
        if( areSatBandOutlinesOn && areOrthogonalImagesOn) {
            view->TurnPropOn(svkOverlayView::SAT_BANDS_AXIAL_OUTLINE); 
            view->TurnPropOn(svkOverlayView::SAT_BANDS_CORONAL_OUTLINE); 
            view->TurnPropOn(svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE); 
        }
        if( svkOverlayView::SafeDownCast(this->view)->GetSelectionBoxVisibility() != svkOverlayView::HIDDEN ) {
            svkOverlayView::SafeDownCast(this->view)->SetSelectionBoxVisibility(svkOverlayView::VISIBLE);
        }
        this->view->TurnRendererOff( svkOverlayView::MOUSE_LOCATION );
        this->myRenderWindow->SetNumberOfLayers(1);
        this->view->GetRenderer(svkOverlayView::PRIMARY)->BackingStoreOff();
        this->rwi->SetInteractorStyle( rotationStyle );
        this->rwi->Enable(); 
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

    vtkRenderer* viewerRenderer = dvController->GetView()->GetRenderer(svkOverlayView::PRIMARY);
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
	targetData = static_cast<svkOverlayView*>(dvController->GetView())->dataVector[MRI];
    if( targetData != NULL ) {
    

        origin = targetData->GetOrigin();
        orientation = dvController->GetView()->GetOrientation();
        if( targetData->IsA("svk4DImageData") ) {
            slice = dvController->GetView()->GetSlice(); 
        } else {
            slice = dvController->GetImageSlice();
        }
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

        targetData->GetIndexFromPosition(projection, index);
        double value = targetData->GetScalarComponentAsDouble(index[0], index[1], index[2], 0);
        int writePos = 0;
        out.setf(ios::fixed,ios::floatfield);
        out.precision(1);
        out << "                                        ";
        out.seekp( writePos );
        out << "L:";
        out << projection[0];
        writePos += 9;
        out.seekp( writePos );
        out << "P:";
        out << projection[1];
        writePos += 9;
        out.seekp( writePos );
        out << "S:";
        out << projection[2];
        writePos += 9;
        out.seekp( writePos );

        int vtkDataType =  vtkImageData::GetScalarType( targetData->GetInformation() ); 
		int dataType = targetData->GetDcmHeader()->GetPixelDataType( vtkDataType ); 

        if (dataType == svkDcmHeader::UNSIGNED_INT_1 || dataType == svkDcmHeader::UNSIGNED_INT_2 || dataType == svkDcmHeader::SIGNED_INT_2) {
			out.precision(0);
        } else {
			out.precision(4);
			out.setf(ios::scientific,ios::floatfield);
        }
        out << "V:" << value;
        static_cast<vtkTextActor*>(dvController->view->GetProp(svkOverlayView::COORDINATES))->SetInput((out.str()).c_str());
    } else { 
        out<<"L:       P:       S:       V:";   
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
                svkOverlayView::SafeDownCast(dvController->GetView())->AlignCamera();
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
    if( visualizationCreated ) {
        view->TurnPropOn( propIndex );
        // Used as an update, to catch collection change
        static_cast<svkOverlayView*>( view )->SetTlcBrc( GetTlcBrc() ); 
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
    if( visualizationCreated ) {
        view->TurnPropOff( propIndex );
        // Used as an update, to catch collection change
        static_cast<svkOverlayView*>( view )->SetTlcBrc( GetTlcBrc() ); 
    }   
}

void svkOverlayViewController::SetOverlayOpacity(double opacity){
    static_cast<svkOverlayView*>( view )->SetOverlayOpacity( opacity );
}

double svkOverlayViewController::GetOverlayOpacity(){
    return static_cast<svkOverlayView*>( view )->GetOverlayOpacity( );
}

void svkOverlayViewController::SetOverlayThreshold(double threshold){
    static_cast<svkOverlayView*>( view )->SetOverlayThreshold( threshold );
}

double svkOverlayViewController::GetOverlayThreshold(){
    return static_cast<svkOverlayView*>( view )->GetOverlayThreshold( );
}


double svkOverlayViewController::GetOverlayThresholdValue(){
    return static_cast<svkOverlayView*>( view )->GetLookupTable( )->GetAlphaThresholdValue();
}

void svkOverlayViewController::SetLevel(double level, WindowLevelTarget target){
    if( target == REFERENCE_IMAGE ) { 
        static_cast<svkOverlayView*>( view )->SetLevel( level );
    } else if ( target == IMAGE_OVERLAY ) {
        static_cast<svkOverlayView*>( view )->SetColorOverlayLevel( level );
    }
}

double svkOverlayViewController::GetLevel(WindowLevelTarget target){
    if( target == REFERENCE_IMAGE ) { 
        return static_cast<svkOverlayView*>( view )->GetLevel( );
    } else if ( target == IMAGE_OVERLAY ) {
        return static_cast<svkOverlayView*>( view )->GetColorOverlayLevel( );
    }
}

void svkOverlayViewController::SetWindow(double window, WindowLevelTarget target){
    if( target == REFERENCE_IMAGE ) { 
        static_cast<svkOverlayView*>( view )->SetWindow( window );
    } else if ( target == IMAGE_OVERLAY ) {
        static_cast<svkOverlayView*>( view )->SetColorOverlayWindow( window );
    }
}

double svkOverlayViewController::GetWindow(WindowLevelTarget target ){
    if( target == REFERENCE_IMAGE ) { 
        return static_cast<svkOverlayView*>( view )->GetWindow( );
    } else if ( target == IMAGE_OVERLAY ) {
        return static_cast<svkOverlayView*>( view )->GetColorOverlayWindow( );
    }
}

/*!
 *  Updates the currently selected voxels. 
 */
void svkOverlayViewController::UpdateSelection(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata)
{
    svkOverlayViewController* dvController = static_cast<svkOverlayViewController*>(thisObject);
    svkOverlaySelector* myRubberband = dvController->dragSelectStyle;
    // Make sure we are in selection mode
    if( myRubberband->GetInteraction() == vtkInteractorStyleRubberBand2D::SELECTING ) {

		vtkRenderWindowInteractor *rwi =
				vtkRenderWindowInteractor::SafeDownCast( subject );
		double* selectionArea = new double[4];
		if (dvController->GetDebug()) {
			cout<<"start position ="<<myRubberband->GetStartX()<<","<<myRubberband->GetStartY()<<endl;
			cout<<"end position ="<<myRubberband->GetEndX()<<","<<myRubberband->GetEndY()<<endl;
		}
		selectionArea[0] = static_cast<double>(myRubberband->GetStartX());
		selectionArea[1] = static_cast<double>(myRubberband->GetStartY());
		selectionArea[2] = static_cast<double>(myRubberband->GetEndX());
		selectionArea[3] = static_cast<double>(myRubberband->GetEndY());

		static_cast<svkOverlayView*>(dvController->GetView())->SetSelection( selectionArea);
		rwi->InvokeEvent(vtkCommand::SelectionChangedEvent);
		delete[] selectionArea;
    }
}


void svkOverlayViewController::ColorWindowLevel( vtkObject* subject, unsigned long eid, void* thisObject, void *calldata) 
{
    svkOverlayViewController* dvController = static_cast<svkOverlayViewController*>(thisObject);
    svkOverlayView* myView = static_cast<svkOverlayView*>(dvController->GetView());
    if( myView->windowLevelerAxial == NULL ) {
        return;
    } 
    if (eid == vtkCommand::ResetWindowLevelEvent) {
        //  vtk6 migration: I'm not sure how to get the information from the object's source algo to update this. 
        //  comment out for now
        //dvController->dataVector[MET]->UpdateInformation();
        //dvController->dataVector[MET]->SetUpdateExtent( 
        //dvController->dataVector[MET]->GetWholeExtent() );
        //dvController->dataVector[MET]->Update();
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
      

    double dx = 2.0 * 
        (isi->GetWindowLevelCurrentPosition()[0] - 
         isi->GetWindowLevelStartPosition()[0]) / size[0];
    double dy = 2.0 * 
        (isi->GetWindowLevelStartPosition()[1] - 
         isi->GetWindowLevelCurrentPosition()[1]) / size[1];
      
      // Scale by current values
    double dMin = 1.0/VTK_DOUBLE_MAX;

    if (fabs(window) > dMin) {
        dx = dx * window;
    } else {
        dx = dx * (window < 0 ? -dMin : dMin);
    }

    if (fabs(level) > dMin) {
        dy = dy * level;
    } else {
        dy = dy * (level < 0 ? -dMin : dMin);
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

    if (fabs(newWindow) < dMin) {
        newWindow = dMin*(newWindow < 0 ? -1 : 1);
    }
    if (fabs(newLevel) < dMin) {
        newLevel = dMin*(newLevel < 0 ? -1 : 1);
    }
      
    if( newLevel - newWindow < newLevel + newWindow ) {
        myView->colorTransfer->SetRange( newLevel - newWindow/2.0, newLevel + newWindow/2.0);
    }

    //myView->GetProp( svkOverlayView::OVERLAY_IMAGE )->Modified();
    
    dvController->myRenderWindow->GetInteractor()->InvokeEvent(vtkCommand::WindowLevelEvent);

    // we will need to know when the event ends for other objects To Update themselves
    if (eid == vtkCommand::EndWindowLevelEvent) {
        dvController->myRenderWindow->GetInteractor()->InvokeEvent(vtkCommand::EndWindowLevelEvent);
    }
    myView->Refresh();
}

/*
void svkOverlayViewController::SetColorOverlayWindow( double window ) 
{
    static_cast<svkOverlayView*>(this->GetView())->SetColorOverlayWindow( window );

}


void svkOverlayViewController::SetColorOverlayLevel( double level ) 
{
    static_cast<svkOverlayView*>(this->GetView())->SetColorOverlayLevel( level );

}


double svkOverlayViewController::GetColorOverlayWindow( ) 
{
    return static_cast<svkOverlayView*>(this->GetView())->GetColorOverlayWindow( );
}


double svkOverlayViewController::GetColorOverlayLevel( ) 
{
    return static_cast<svkOverlayView*>(this->GetView())->GetColorOverlayLevel( );
}
*/

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


/*!
 *
 */
bool svkOverlayViewController::IsImageInsideSpectra()
{
    return static_cast<svkOverlayView*>(this->view)->IsImageInsideSpectra();
}
