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


#include <svkImageViewer2.h>


using namespace svk;


vtkCxxRevisionMacro(svkImageViewer2, "$Rev$");
vtkStandardNewMacro(svkImageViewer2);


//! Constructor
svkImageViewer2::svkImageViewer2()
{
    if( this->ImageActor != NULL ) {
        this->ImageActor->Delete();
        this->ImageActor = NULL;
    }
    this->ImageActor = svkOpenGLOrientedImageActor::New();
    this->ImageActor->InterpolateOff();
    if( this->WindowLevel != NULL ) {
        this->WindowLevel->Delete();
    }
    this->WindowLevel = svkImageMapToWindowLevelColors::New();
    this->orthImageActor1 = svkOpenGLOrientedImageActor::New();
    this->orthImageActor2 = svkOpenGLOrientedImageActor::New();
    this->orthWinLevel1 = svkImageMapToWindowLevelColors::New();
    this->orthWinLevel2 = svkImageMapToWindowLevelColors::New();
    this->orthSlice1 = -1;
    this->orthSlice2 = -1;

}


//! Destructor
svkImageViewer2::~svkImageViewer2()
{
    if( orthImageActor1 != NULL ) {
        orthImageActor1->Delete();
        orthImageActor1 = NULL;
    }
    if( orthImageActor2 != NULL ) {
        orthImageActor2->Delete();
        orthImageActor2 = NULL;
    }
    if( orthWinLevel1 != NULL ) {
        orthWinLevel1->Delete();
        orthWinLevel1 = NULL;
    }
    if( orthWinLevel2 != NULL ) {
        orthWinLevel2->Delete();
        orthWinLevel2 = NULL;
    }
}


// Callback for our interactor. This has not been modified, but needs to be present.
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
        this->IV->GetInput()->UpdateInformation();
        this->IV->GetInput()->SetUpdateExtent
          (this->IV->GetInput()->GetWholeExtent());
        this->IV->GetInput()->Update();
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
        return;
        }
      
      // Adjust the window level here

      vtkInteractorStyleImage *isi = 
        static_cast<vtkInteractorStyleImage *>(caller);

      int *size = this->IV->GetRenderWindow()->GetSize();
      double window = this->InitialWindow;
      double level = this->InitialLevel;
      
      // Compute normalized delta

      double dx = 4.0 * 
        (isi->GetWindowLevelCurrentPosition()[0] - 
         isi->GetWindowLevelStartPosition()[0]) / size[0];
      double dy = 4.0 * 
        (isi->GetWindowLevelStartPosition()[1] - 
         isi->GetWindowLevelCurrentPosition()[1]) / size[1];
      
      // Scale by current values

      if (fabs(window) > 0.01)
        {
        dx = dx * window;
        }
      else
        {
        dx = dx * (window < 0 ? -0.01 : 0.01);
        }
      if (fabs(level) > 0.01)
        {
        dy = dy * level;
        }
      else
        {
        dy = dy * (level < 0 ? -0.01 : 0.01);
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

      if (fabs(newWindow) < 0.01)
        {
        newWindow = 0.01*(newWindow < 0 ? -1 : 1);
        }
      if (fabs(newLevel) < 0.01)
        {
        newLevel = 0.01*(newLevel < 0 ? -1 : 1);
        }
      
      this->IV->SetColorWindow(newWindow);
      this->IV->SetColorLevel(newLevel);
      this->IV->Render();
    }
  
  vtkImageViewer2 *IV;
  double InitialWindow;
  double InitialLevel;
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
      this->InteractorStyle = vtkInteractorStyleImage::New();
      vtkImageViewer2Callback *cbk = vtkImageViewer2Callback::New();
      cbk->IV = this;
      this->InteractorStyle->AddObserver(
        vtkCommand::WindowLevelEvent, cbk);
      this->InteractorStyle->AddObserver(
        vtkCommand::StartWindowLevelEvent, cbk);
      this->InteractorStyle->AddObserver(
        vtkCommand::ResetWindowLevelEvent, cbk);
      cbk->Delete();
      }

    this->Interactor->SetInteractorStyle(this->InteractorStyle);
    this->Interactor->SetRenderWindow(this->RenderWindow);
    }

  if (this->Renderer && this->ImageActor)
    {
    this->Renderer->AddViewProp(this->ImageActor);
    this->Renderer->AddViewProp(this->orthImageActor1);
    this->Renderer->AddViewProp(this->orthImageActor2);
    }

  if (this->ImageActor && this->WindowLevel)
    {
    /*
     * Since the WindowLevel object is declared in the parent class it is actually 
     * of type vtkImageMapToWindowLevelColors. vtkImageMapToWindowLevelColors::GetOutput
     * is not virtual, so we need to cast it to get our GetOutput method which copies
     * the correct dcos on output.
     */
    this->ImageActor->SetInput(this->WindowLevel->GetOutput());
    this->orthImageActor1->SetInput(this->orthWinLevel1->GetOutput());
    this->orthImageActor2->SetInput(this->orthWinLevel2->GetOutput());
    }
}


/*!
 * Sets the input. Was overriden to allow the use of svkImageData object.
 */
void svkImageViewer2::SetInput(svkImageData *in)
{
    this->WindowLevel->SetInput(in);
    this->WindowLevel->SetNumberOfThreads(1);
    this->orthWinLevel1->SetInput(in);
    this->orthWinLevel1->SetNumberOfThreads(1);
    this->orthWinLevel2->SetInput(in);
    this->orthWinLevel2->SetNumberOfThreads(1);
    this->UpdateDisplayExtent();
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

    vtkImageData *input = this->GetInput();
    if (this->RenderWindow->GetSize()[0] == 0 && input)
      {
      input->UpdateInformation();
      int *w_ext = input->GetWholeExtent();
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
      this->RenderWindow->SetSize(
        xs < 150 ? 150 : xs, ys < 100 ? 100 : ys);

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
    this->ImageActor->Modified();
    this->orthImageActor1->Modified();
    this->orthImageActor2->Modified();
    this->RenderWindow->Render();
    }
}


/*
 *   Gets the slice of the main image.
 */
int svkImageViewer2::GetSlice()
{
    return Slice; 
}


/*
 *  Sets the slice for a given image. 0 is the primary, 1 and 2 are the orthogonal.
 */
void svkImageViewer2::SetSlice( int slice, int imageNum ) 
{

    int* extent =  NULL; 
    if( this->GetInput() != NULL ) {
        extent = this->GetInput()->GetExtent();
    }
    switch( imageNum ) {
        case 1: 
            this->orthSlice1 = slice;
            if( extent != NULL ) {
                this->orthImageActor1->SetDisplayExtent( slice, slice, extent[2], extent[3], extent[4], extent[5] );
            }
            break;
        case 2: 
            this->orthSlice2 = slice;
            if( extent != NULL ) {
                this->orthImageActor2->SetDisplayExtent( extent[0], extent[1], slice, slice, extent[4], extent[5] );
            }
            break;
        default:
            Superclass::SetSlice( slice ); 
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
        GetRenderer()->ResetCamera();
        int* extent = this->GetInput()->GetExtent();
        double* spacing = this->GetInput()->GetSpacing();
        double* origin = this->GetInput()->GetOrigin();
        double distance = this->GetRenderer()->GetActiveCamera()->GetDistance();
        double dcos[3][3];
        static_cast<svkImageData*>(this->GetInput())->GetDcos( dcos );
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
        double imageCenter[3];
        double x[3];
        for( int i = 0; i < 3; i++ ) {
                imageCenter[i] = origin[i];
            for( int j = 0; j < 3; j++ ) {
                if( j == this->SliceOrientation ) {
                    imageCenter[i] += Slice * spacing[j] * dcos[j][i];
                } else {
                    imageCenter[i] += (((extent[2*j+1] - extent[2*j]) * spacing[j])/2) * dcos[j][i];
                }
            }
        }
        // If it is an axial data set...
        GetRenderer()->GetActiveCamera()->SetFocalPoint( imageCenter  );
        if( pow( wVec[2], 2) >= pow( wVec[1], 2 ) && pow( wVec[2], 2) >= pow( wVec[0], 2 ) ) {
            x[0] = imageCenter[0] + distance * dcos[this->SliceOrientation][0];
            x[1] = imageCenter[1] + distance * dcos[this->SliceOrientation][1];
            x[2] = imageCenter[2] + distance * dcos[this->SliceOrientation][2];
            GetRenderer()->GetActiveCamera()->SetPosition( x );
            GetRenderer()->GetActiveCamera()->SetViewUp( -vVec[0], -vVec[1], -vVec[2] );
        } else if( pow( wVec[1], 2) >= pow( wVec[0], 2 ) && pow( wVec[1], 2) >= pow( wVec[2], 2 ) ) {
            x[0] = imageCenter[0] - distance * dcos[this->SliceOrientation][0];
            x[1] = imageCenter[1] - distance * dcos[this->SliceOrientation][1];
            x[2] = imageCenter[2] - distance * dcos[this->SliceOrientation][2];
            GetRenderer()->GetActiveCamera()->SetPosition( x );
            GetRenderer()->GetActiveCamera()->SetViewUp( -vVec[0], -vVec[1], -vVec[2] );
        } else {
            x[0] = imageCenter[0] + distance * dcos[this->SliceOrientation][0];
            x[1] = imageCenter[1] + distance * dcos[this->SliceOrientation][1];
            x[2] = imageCenter[2] + distance * dcos[this->SliceOrientation][2];
            GetRenderer()->GetActiveCamera()->SetPosition( x );
            GetRenderer()->GetActiveCamera()->SetViewUp( -vVec[0], -vVec[1], -vVec[2] );
        }
        GetRenderer()->ResetCamera();
        GetRenderer()->GetActiveCamera()->Zoom( 1.2 );
    } 
}


/*
 *  Sets the window of the window/level.
 */
void svkImageViewer2::SetColorWindow(double s)
{
  this->WindowLevel->SetWindow(s);
  this->orthWinLevel1->SetWindow(s);
  this->orthWinLevel2->SetWindow(s);
}


/*
 *  Sets the level of the window/level.
 */
void svkImageViewer2::SetColorLevel(double s)
{
  this->WindowLevel->SetLevel(s);
  this->orthWinLevel1->SetLevel(s);
  this->orthWinLevel2->SetLevel(s);
}


/*
 *  Creates the orthogonal images.
 */
void svkImageViewer2::InitializeOrthogonalActors() 
{
    int* extent = this->GetInput()->GetExtent();

    // Setting the DisplayExtent is how we select a slice.
    if( orthSlice1 >=0 ) {
        this->orthImageActor1->SetDisplayExtent( orthSlice1, orthSlice1, extent[2], extent[3], extent[4], extent[5] );
    } else {
        this->orthImageActor1->SetDisplayExtent( extent[1]/2, extent[1]/2, extent[2], extent[3], extent[4], extent[5] );
    }
    if( orthSlice2 >=0 ) {
        this->orthImageActor2->SetDisplayExtent( extent[0], extent[1], orthSlice2, orthSlice2, extent[4], extent[5] );
    } else {
        this->orthImageActor2->SetDisplayExtent( extent[0], extent[1], extent[3]/2, extent[3]/2, extent[4], extent[5] );
    }
    this->orthImageActor1->InterpolateOff();
    this->orthImageActor2->InterpolateOff();

}



void svkImageViewer2::TurnOrthogonalImagesOn()
{
    this->orthImageActor1->VisibilityOn();
    this->orthImageActor2->VisibilityOn();
}

void svkImageViewer2::TurnOrthogonalImagesOff()
{
    this->orthImageActor1->VisibilityOff();
    this->orthImageActor1->Modified();
    this->orthImageActor2->VisibilityOff();
    this->orthImageActor2->Modified();
}
