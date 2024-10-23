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


#include <svkImageViewer2.h>
#include "/usr/include/vtk/vtkStreamingDemandDrivenPipeline.h"



using namespace svk;


//vtkCxxRevisionMacro(svkImageViewer2, "$Rev$");
vtkStandardNewMacro(svkImageViewer2);


//! Constructor
svkImageViewer2::svkImageViewer2()
{
    this->axialWinLevel = svkImageMapToWindowLevelColors::New();
    this->coronalWinLevel = svkImageMapToWindowLevelColors::New();
    this->sagittalWinLevel = svkImageMapToWindowLevelColors::New();
    
    // We are going to tie the default WL object to one of our WL objects so it updates correctly
    if( this->WindowLevel != NULL ) {
        this->WindowLevel->Delete();
    }
    this->WindowLevel = axialWinLevel;
    
    // The parent class will free this object, but we want to free it too (just to be clear we kill everything we create) 
    axialWinLevel->Register( this );

    this->axialSlice    = 0;
    this->coronalSlice  = 0;
    this->sagittalSlice = 0;

    this->axialImageActor = svkOrientedImageActor::New();
    this->coronalImageActor = svkOrientedImageActor::New();
    this->sagittalImageActor = svkOrientedImageActor::New();

    this->data = NULL;

    this->orientation = svkDcmHeader::AXIAL;

    this->cameraZoom = 1.2;     

}


//! Destructor
svkImageViewer2::~svkImageViewer2()
{
    if( this->axialImageActor != NULL ) {
        this->axialImageActor->Delete();
        this->axialImageActor = NULL;
    }

    if( this->coronalImageActor != NULL ) {
        this->coronalImageActor->Delete();
        this->coronalImageActor = NULL;
    }

    if( this->sagittalImageActor != NULL ) {
        this->sagittalImageActor->Delete();
        this->sagittalImageActor = NULL;
    }

    if( this->axialWinLevel != NULL ) {
        this->axialWinLevel->Delete();
        this->axialWinLevel = NULL;
    }

    if( this->coronalWinLevel != NULL ) {
        this->coronalWinLevel->Delete();
        this->coronalWinLevel = NULL;
    }

    if( this->sagittalWinLevel != NULL ) {
        this->sagittalWinLevel->Delete();
        this->sagittalWinLevel = NULL;
    }

    if( this->data != NULL ) {
        this->data->Delete();
        this->data = NULL;
    }

}


// Callback for our interactor. This has only been modified to reduce sensitivity.
class vtkImageViewer2Callback : public vtkCommand
{
public:
  static vtkImageViewer2Callback *New() { return new vtkImageViewer2Callback; }
  
  void Execute(vtkObject *caller, 
               unsigned long event, 
               void *vtkNotUsed(callData))
    {
      if (this->IV->GetInput() == NULL)
        {
        return;
        }

      // Reset

      if (event == vtkCommand::ResetWindowLevelEvent)
        {
        this->IV->GetInputAlgorithm()->UpdateInformation();
        vtkStreamingDemandDrivenPipeline::SetExtent(
          this->IV->GetInputInformation(),
          vtkStreamingDemandDrivenPipeline::GetWholeExtent(
            this->IV->GetInputInformation()));
        this->IV->GetInputAlgorithm()->Update();
        double *range = this->IV->GetInput()->GetScalarRange();
        this->IV->SetColorWindow(range[1] - range[0]);
        this->IV->SetColorLevel(0.5 * (range[1] + range[0]));
        this->IV->Render();
        return;
        }

      // Start

      if (event == vtkCommand::StartWindowLevelEvent)
        {
        this->InitialWindow = this->IV->GetColorWindow();
        this->InitialLevel = this->IV->GetColorLevel();
        this->HasWindowLevelStarted = true;
        return;
        }
      
      // Adjust the window level here

      vtkInteractorStyleImage *isi = 
        static_cast<vtkInteractorStyleImage *>(caller);

      int *size = this->IV->GetRenderWindow()->GetSize();
      double window = this->InitialWindow;
      double level = this->InitialLevel;
      
      // Compute normalized delta

      double dx = 2.0 * 
        (isi->GetWindowLevelCurrentPosition()[0] - 
         isi->GetWindowLevelStartPosition()[0]) / size[0];
      double dy = 2.0 * 
        (isi->GetWindowLevelStartPosition()[1] - 
         isi->GetWindowLevelCurrentPosition()[1]) / size[1];
      
      // Scale by current values
      double dMin = 1.0/VTK_DOUBLE_MAX;

      if (fabs(window) > dMin )
        {
        dx = dx * window;
        }
      else
        {
        dx = dx * (window < 0 ? -dMin : dMin);
        }
      if (fabs(level) > dMin)
        {
        dy = dy * level;
        }
      else
        {
        dy = dy * (level < 0 ? -dMin : dMin);
        }
      
      // Abs so that direction does not flip

      if (window < 0.0) 
        {
        dx = -1*dx;
        }
      if (level < 0.0) 
        {
        dy = -1*dy;
        }
      
      // Compute new window level

      double newWindow = dx + window;
      double newLevel;
      newLevel = level - dy;
      
      // Stay away from zero and really

      if (fabs(newWindow) < dMin)
        {
        newWindow = dMin*(newWindow < 0 ? -1 : 1);
        }
      if (fabs(newLevel) < dMin)
        {
        newLevel = dMin*(newLevel < 0 ? -1 : 1);
        }
      if( this->HasWindowLevelStarted ) {
          this->IV->SetColorWindow(newWindow);
          this->IV->SetColorLevel(newLevel);
      }
      this->IV->Render();
      this->IV->GetRenderer()->GetRenderWindow()->GetInteractor()->InvokeEvent(vtkCommand::WindowLevelEvent);
      if (event == vtkCommand::EndWindowLevelEvent) {
          this->IV->GetRenderer()->GetRenderWindow()->GetInteractor()->InvokeEvent(vtkCommand::EndWindowLevelEvent);
          this->HasWindowLevelStarted = false;
      }
    }
  
  vtkImageViewer2 *IV;
  double InitialWindow;
  double InitialLevel;
  bool   HasWindowLevelStarted;
};


/*!
 *  We are overridding this so we can cast the WindowLevel object to
 *  svkImageMapToWindowLevelColors, ensuring its GetOutput gives us svkImageData.
 */
void svkImageViewer2::InstallPipeline()
{
  if (this->RenderWindow && this->Renderer)
    {
    this->RenderWindow->AddRenderer(this->Renderer);
    }

  if (this->Interactor)
    {
    if (!this->InteractorStyle)
      {
		  vtkInteractorStyleImage* style = vtkInteractorStyleImage::New();
		  this->SetInteractorStyle(style);
      }

    this->Interactor->SetInteractorStyle(this->InteractorStyle);
    this->Interactor->SetRenderWindow(this->RenderWindow);
    }

  if (this->Renderer && this->ImageActor)
    {
    this->Renderer->AddViewProp(this->axialImageActor);
    this->Renderer->AddViewProp(this->coronalImageActor);
    this->Renderer->AddViewProp(this->sagittalImageActor);
    }

  if ( this->data != NULL )
    {
    /*
     * Since the WindowLevel object is declared in the parent class it is actually 
     * of type vtkImageMapToWindowLevelColors. vtkImageMapToWindowLevelColors::GetOutput
     * is not virtual, so we need to cast it to get our GetOutput method which copies
     * the correct dcos on output.
     */
    this->axialImageActor->GetMapper()->SetInputConnection(this->axialWinLevel->GetOutputPort());
    this->coronalImageActor->GetMapper()->SetInputConnection(this->coronalWinLevel->GetOutputPort());
    this->sagittalImageActor->GetMapper()->SetInputConnection(this->sagittalWinLevel->GetOutputPort());

    }
}


/*!
 * Sets the input. Was overriden to allow the use of svkImageData object.
 */
void svkImageViewer2::SetInputData(svkImageData *in)
{
    if( this->data != NULL ) {
        this->data->Delete();
    }
    this->data = in;
    this->data->Register(this);
    this->axialWinLevel->SetInputData(in);
    this->axialWinLevel->UpdateWholeExtent();
    this->axialWinLevel->SetNumberOfThreads(1);
    this->coronalWinLevel->SetInputData(in);
    this->coronalWinLevel->UpdateWholeExtent();
    this->coronalWinLevel->SetNumberOfThreads(1);
    this->sagittalWinLevel->SetInputData(in);
    this->sagittalWinLevel->UpdateWholeExtent();
    this->sagittalWinLevel->SetNumberOfThreads(1);

    //this->UpdateDisplayExtent();

    int* extent = in->GetExtent();
    int axialIndex    = this->GetInput()->GetOrientationIndex( svkDcmHeader::AXIAL );
    int coronalIndex  = this->GetInput()->GetOrientationIndex( svkDcmHeader::CORONAL );
    int sagittalIndex = this->GetInput()->GetOrientationIndex( svkDcmHeader::SAGITTAL );
    this->axialSlice    = (extent[2*axialIndex+1]- extent[2*axialIndex])/2;
    this->coronalSlice  = (extent[2*coronalIndex+1]- extent[2*coronalIndex])/2;
    this->sagittalSlice = (extent[2*sagittalIndex+1]- extent[2*sagittalIndex])/2;

    this->InitializeOrthogonalActors();

}


/*!
 *  Renders the actor. The only reason for the overload is to
 *  update the image actor. This appears to be necessary for the image
 *  to update as you window level, and is likely caused because we had 
 *  to override the GetOutput method of the WindowLevel class.
 */
void svkImageViewer2::Render()
{
  if (this->FirstRender)
    {
    // Initialize the size if not set yet
    
    vtkAlgorithm *input = this->GetInputAlgorithm();
    if (input)
      {
      input->UpdateInformation();
      int *w_ext = this->GetInputInformation()->Get(
        vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT());
      int xs = 0, ys = 0;

      switch (this->SliceOrientation)
        {
        case vtkImageViewer2::SLICE_ORIENTATION_XY:
        default:
          xs = w_ext[1] - w_ext[0] + 1;
          ys = w_ext[3] - w_ext[2] + 1;
          break;

        case vtkImageViewer2::SLICE_ORIENTATION_XZ:
          xs = w_ext[1] - w_ext[0] + 1;
          ys = w_ext[5] - w_ext[4] + 1;
          break;

        case vtkImageViewer2::SLICE_ORIENTATION_YZ:
          xs = w_ext[3] - w_ext[2] + 1;
          ys = w_ext[5] - w_ext[4] + 1;
          break;
        }

      // if it would be smaller than 150 by 100 then limit to 150 by 100  
      if (this->RenderWindow->GetSize()[0] == 0)
        {
        this->RenderWindow->SetSize(
          xs < 150 ? 150 : xs, ys < 100 ? 100 : ys);
        }

      if (this->Renderer)
        {
        this->Renderer->ResetCamera();
        this->Renderer->GetActiveCamera()->SetParallelScale(
          xs < 150 ? 75 : (xs - 1 ) / 2.0);
        }
      this->FirstRender = 0;
      }
    }
  
  if (this->GetInput())
    {
    this->axialImageActor->Modified();
    this->coronalImageActor->Modified();
    this->sagittalImageActor->Modified();
    this->RenderWindow->Render();
    }
}


/*
 *   Gets the slice of the main image.
 */
int svkImageViewer2::GetSlice()
{
    return this->GetSlice( this->orientation ); 
}


/*
 *   Gets the slice of the main image.
 */
int svkImageViewer2::GetSlice(svkDcmHeader::Orientation orientation )
{
    int currentSlice; 
    switch( orientation ) {
        case svkDcmHeader::AXIAL:
            currentSlice = axialSlice;
            break;
        case svkDcmHeader::CORONAL:
            currentSlice = coronalSlice;
            break;
        case svkDcmHeader::SAGITTAL:
            currentSlice = sagittalSlice;
            break;
    }
    return currentSlice; 
}


void svkImageViewer2::SetSlice( int slice ) 
{
    //Superclass::SetSlice(slice);
    this->SetSlice( slice, this->orientation );
}
 

/*
 *  Sets the slice for a given image. 0 is the primary, 1 and 2 are the orthogonal.
 */
void svkImageViewer2::SetSlice( int slice, svkDcmHeader::Orientation sliceOrientation ) 
{
    if( this->GetInput() != NULL ) {
        int* extent = this->GetInput()->GetExtent();
        int sliceIndex = this->GetInput()->GetOrientationIndex( sliceOrientation );

        int sliceExtent[6] = { extent[ 0 ], extent[ 1 ],
                               extent[ 2 ], extent[ 3 ],
                               extent[ 4 ], extent[ 5 ] };
        int sliceRange[2] = { this->GetInput()->GetFirstSlice( sliceOrientation ), 
                              this->GetInput()->GetLastSlice( sliceOrientation ) };

        if( slice < sliceRange[0] ) {
            slice = sliceRange[0];
        } else if ( slice > sliceRange[1] ) { 
            slice = sliceRange[1];
        }
        sliceExtent[ sliceIndex *2 ] = slice;
        sliceExtent[ sliceIndex *2 + 1 ] = slice;
        this->GetImageActor( sliceOrientation )->SetDisplayExtent( sliceExtent );
        this->GetImageActor( sliceOrientation )->Modified( );
        
        switch( sliceOrientation ) {
            case svkDcmHeader::AXIAL:
                this->axialSlice = slice;
                break;
            case svkDcmHeader::CORONAL:
                this->coronalSlice = slice;
                break;
            case svkDcmHeader::SAGITTAL:
                this->sagittalSlice = slice;
                break;
        }
    }
}


/*!
 *
 *  Sets the active scalars for the image data set.
 *
 * @param volume
 */
void svkImageViewer2::SetActiveVolume( int volume )
{
    svkImageData* data = svkImageData::SafeDownCast(this->GetInput());
    if( data != NULL && volume < data->GetPointData()->GetNumberOfArrays()) {
        data->GetPointData()->SetActiveScalars( data->GetPointData()->GetArray(volume)->GetName() );
        /*
         * TODO: The above line modifies the input to the WindowLevel algorithms. The actor is not
         *       re-rendering afterward. We need to figure out why this is and if manually calling
         *       modified is the best solution or not. For now we manually call modified to get
         *       the actors updated.
         */
        this->axialWinLevel->Modified();
        this->coronalWinLevel->Modified();
        this->sagittalWinLevel->Modified();
        this->Render( );
    }
}


/*!
 *  Resets the camera to be exactly perpendicular to the image.
 */
void svkImageViewer2::ResetCamera()
{
    if ( this->GetInput() != NULL) {

        // We need to reset and render to get the distance to the object correct
        this->Render();
        this->GetRenderer()->ResetCamera();
        int* extent = this->GetInput()->GetExtent();
        double* spacing = this->GetInput()->GetSpacing();
        double* origin = this->GetInput()->GetOrigin();
        double distance = this->GetRenderer()->GetActiveCamera()->GetDistance();
        double dcos[3][3];
        static_cast<svkImageData*>(this->GetInput())->GetDcos( dcos );
        double x[3] = {0,0,0};
        double tmpCenter[3] = {0,0,0};
        this->data->GetImageCenter( tmpCenter );
        double imageCenter[3] = {0, 0, 0};
        imageCenter[0] = tmpCenter[0];
        imageCenter[1] = tmpCenter[1];
        imageCenter[2] = tmpCenter[2];
        // If it is an axial data set...
        this->GetRenderer()->GetActiveCamera()->SetFocalPoint( imageCenter  );
        double axialNormal[3] = {0,0,0};
        this->data->GetSliceNormal( axialNormal, svkDcmHeader::AXIAL );
        double coronalNormal[3] = {0,0,0};
        this->data->GetSliceNormal( coronalNormal, svkDcmHeader::CORONAL );
        double sagittalNormal[3] = {0,0,0};
        this->data->GetSliceNormal( sagittalNormal, svkDcmHeader::SAGITTAL );

        int inverter = -1;
        switch( this->orientation ) {
            case svkDcmHeader::AXIAL:
                if( axialNormal[2] > 0 ) {
                    distance *=-1;
                }
                x[0] = imageCenter[0] + distance * axialNormal[0];
                x[1] = imageCenter[1] + distance * axialNormal[1];
                x[2] = imageCenter[2] + distance * axialNormal[2];
                GetRenderer()->GetActiveCamera()->SetPosition( x );
                if( coronalNormal[1] < 0 ) {
                    inverter *=-1;
                }
                GetRenderer()->GetActiveCamera()->SetViewUp( inverter*coronalNormal[0], 
                                                             inverter*coronalNormal[1], 
                                                             inverter*coronalNormal[2] );
                break;
            case svkDcmHeader::CORONAL:
                if( coronalNormal[1] > 0 ) {
                    distance *=-1;
                }
                x[0] = imageCenter[0] + distance * coronalNormal[0];
                x[1] = imageCenter[1] + distance * coronalNormal[1];
                x[2] = imageCenter[2] + distance * coronalNormal[2];
                GetRenderer()->GetActiveCamera()->SetPosition( x );
                if( axialNormal[2] > 0 ) {
                    inverter*=-1;
                }
                GetRenderer()->GetActiveCamera()->SetViewUp( inverter*axialNormal[0], 
                                                             inverter*axialNormal[1], 
                                                             inverter*axialNormal[2] );
                break;
            case svkDcmHeader::SAGITTAL:
                if( sagittalNormal[0] < 0 ) {
                    distance *=-1;
                }
                x[0] = imageCenter[0] + distance * sagittalNormal[0];
                x[1] = imageCenter[1] + distance * sagittalNormal[1];
                x[2] = imageCenter[2] + distance * sagittalNormal[2];
                GetRenderer()->GetActiveCamera()->SetPosition( x );
                if( axialNormal[2] > 0 ) {
                    inverter*=-1;
                }
                GetRenderer()->GetActiveCamera()->SetViewUp( inverter*axialNormal[0],
                                                             inverter*axialNormal[1],
                                                             inverter*axialNormal[2] );
                break;
        }
        GetRenderer()->ResetCamera();
        GetRenderer()->GetActiveCamera()->Zoom( this->cameraZoom );
    } 
}

/*!
 *  Set the camera zoom factor
 */
void svkImageViewer2::SetCameraZoom( double zoom )
{
    this->cameraZoom = zoom; 
}


/*
 *  Sets the window of the window/level.
 */
void svkImageViewer2::SetColorWindow(double s)
{
  this->axialWinLevel->SetWindow(s);
  this->coronalWinLevel->SetWindow(s);
  this->sagittalWinLevel->SetWindow(s);
}


/*
 *  Sets the level of the window/level.
 */
void svkImageViewer2::SetColorLevel(double s)
{
  this->axialWinLevel->SetLevel(s);
  this->coronalWinLevel->SetLevel(s);
  this->sagittalWinLevel->SetLevel(s);
}


/*
 *  Creates the orthogonal images.
 */
void svkImageViewer2::InitializeOrthogonalActors() 
{
    this->SetSlice( this->axialSlice, svkDcmHeader::AXIAL ); 
    this->SetSlice( this->coronalSlice, svkDcmHeader::CORONAL ); 
    this->SetSlice( this->sagittalSlice, svkDcmHeader::SAGITTAL ); 

    this->axialImageActor->InterpolateOff();
    this->coronalImageActor->InterpolateOff();
    this->sagittalImageActor->InterpolateOff();

}


/*!
 *
 */
void svkImageViewer2::TurnOrthogonalImagesOn()
{
    switch ( this->orientation ) {
        case svkDcmHeader::AXIAL:
            this->coronalImageActor->VisibilityOn(); 
            this->coronalImageActor->Modified(); 
            this->sagittalImageActor->VisibilityOn(); 
            this->sagittalImageActor->Modified(); 
            break;
        case svkDcmHeader::CORONAL:
            this->axialImageActor->VisibilityOn(); 
            this->axialImageActor->Modified(); 
            this->sagittalImageActor->VisibilityOn(); 
            this->sagittalImageActor->Modified(); 
            break;
        case svkDcmHeader::SAGITTAL:
            this->axialImageActor->VisibilityOn(); 
            this->axialImageActor->Modified(); 
            this->coronalImageActor->VisibilityOn(); 
            this->coronalImageActor->Modified(); 
            break;
    }
}


/*!
 *
 */
void svkImageViewer2::TurnOrthogonalImagesOff()
{
    switch ( this->orientation ) {
        case svkDcmHeader::AXIAL:
            this->coronalImageActor->VisibilityOff(); 
            this->coronalImageActor->Modified(); 
            this->sagittalImageActor->VisibilityOff(); 
            this->sagittalImageActor->Modified(); 
            break;
        case svkDcmHeader::CORONAL:
            this->axialImageActor->VisibilityOff(); 
            this->axialImageActor->Modified(); 
            this->sagittalImageActor->VisibilityOff(); 
            this->sagittalImageActor->Modified(); 
            break;
        case svkDcmHeader::SAGITTAL:
            this->axialImageActor->VisibilityOff(); 
            this->axialImageActor->Modified(); 
            this->coronalImageActor->VisibilityOff(); 
            this->coronalImageActor->Modified(); 
            break;
    }
}


/*!
 *
 */
bool svkImageViewer2::AreOrthogonalImagesOn()
{
    if( this->axialImageActor->GetVisibility() 
     && this->sagittalImageActor->GetVisibility() 
     && this->coronalImageActor->GetVisibility() ) {
        return true;
    } else {
        return false;
    }
        
}


void svkImageViewer2::SetInteractorStyle( vtkInteractorStyleImage* style )
{
	this->InteractorStyle = style;
      vtkImageViewer2Callback *cbk = vtkImageViewer2Callback::New();
      cbk->IV = this;
      cbk->HasWindowLevelStarted = false;
      this->InteractorStyle->AddObserver(
        vtkCommand::WindowLevelEvent, cbk);
      this->InteractorStyle->AddObserver(
        vtkCommand::StartWindowLevelEvent, cbk);
      this->InteractorStyle->AddObserver(
        vtkCommand::ResetWindowLevelEvent, cbk);
      this->InteractorStyle->AddObserver(
        vtkCommand::EndWindowLevelEvent, cbk);
      this->InteractorStyle->AddObserver(
        vtkCommand::InteractionEvent, cbk);
      cbk->Delete();
}


/*!
 *
 */
void svkImageViewer2::SetOrientation( svkDcmHeader::Orientation orientation )
{
    this->orientation = orientation;
}


/*!
 *
 */
svkDcmHeader::Orientation svkImageViewer2::GetOrientation( )
{
    return this->orientation;
}


/*!
 *
 */
svkOrientedImageActor* svkImageViewer2::GetImageActor( svkDcmHeader::Orientation actorOrientation )
{
    svkOrientedImageActor* actor = NULL;
    actorOrientation = (actorOrientation == svkDcmHeader::UNKNOWN_ORIENTATION ) ?
                                this->GetInput()->GetDcmHeader()->GetOrientationType() : actorOrientation;
    switch ( actorOrientation ) {
        case svkDcmHeader::AXIAL:
            actor = this->axialImageActor;
            break;
        case svkDcmHeader::CORONAL:
            actor = this->coronalImageActor;
            break;
        case svkDcmHeader::SAGITTAL:
            actor = this->sagittalImageActor;
            break;
    }
    return actor;
}

svkImageData* svkImageViewer2::GetInput( ) 
{
    return this->data;
}


/*
 *  Updates the input algorithm info .
 */
void svkImageViewer2::UpdateInputInformation()
{
    vtkAlgorithm* input = this->GetInputAlgorithm();
    input->UpdateInformation();

    vtkStreamingDemandDrivenPipeline::SetExtent(
        this->GetInputInformation(),
        vtkStreamingDemandDrivenPipeline::GetWholeExtent(
            this->GetInputInformation()
        )
    );
    input->Update(); 

}

