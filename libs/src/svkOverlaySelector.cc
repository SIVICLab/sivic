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

/*
 *  NOTE: Some of this source was taken from vtkInteractorStyle2D DIRECTLY. Check about liscensing issues.
 */

#include <svkOverlaySelector.h>


using namespace svk;


////vtkCxxRevisionMacro(svkOverlaySelector, "$Rev$");
vtkStandardNewMacro(svkOverlaySelector);

/*!
    Callback for a left button release. Clears the rubberband from the screen
    and throws a SelectionChangedEvent.
*/
void svkOverlaySelector::OnLeftButtonUp()
{
    this->Superclass::OnLeftButtonUp();
   
  if(this->Interaction == SELECTING)
    {
    this->Interaction = NONE;

    // Clear the rubber band
    int* size = this->Interactor->GetRenderWindow()->GetSize();
    unsigned char* pixels = this->PixelArray->GetPointer(0);
    this->Interactor->GetRenderWindow()->Frame();

    unsigned int rect[5];
    rect[0] = this->StartPosition[0];
    rect[1] = this->StartPosition[1];
    rect[2] = this->EndPosition[0];
    rect[3] = this->EndPosition[1];
    if (this->Interactor->GetShiftKey())
      {
      rect[4] = SELECT_UNION;
      }
    else
      {
      rect[4] = SELECT_NORMAL;
      }
    this->InvokeEvent(vtkCommand::SelectionChangedEvent, StartPosition);
    this->InvokeEvent(vtkCommand::EndInteractionEvent);
    }

}


/*!
    Callback for mouse motion. It grabs the current pixel array of the 
    vtkRenderWindow and draws the rubberband on top. Camera motion is inverted
    for the y-axis when panning.
*/
void svkOverlaySelector::OnMouseMove()
{
  vtkRenderWindowInteractor* rwi = this->GetInteractor();
  if (this->Interaction == PANNING || this->Interaction == ZOOMING)
    {
    vtkRenderWindowInteractor* rwi = this->GetInteractor();
    int lastPt[] = {0, 0};
    rwi->GetLastEventPosition(lastPt);
    int curPt[] = {0, 0};
    rwi->GetEventPosition(curPt);
    
    vtkCamera* camera = this->CurrentRenderer->GetActiveCamera();
    double lastScale = 2.0 * camera->GetParallelScale() / this->CurrentRenderer->GetSize()[1];
    double lastFocalPt[] = {0, 0, 0};
    camera->GetFocalPoint(lastFocalPt);
    double lastPos[] = {0, 0, 0};
    camera->GetPosition(lastPos);
  
    if (this->Interaction == PANNING)
      {
      this->Pan();
      /*
      double delta[] = {0, 0, 0};
      delta[0] = -lastScale*(curPt[0] - lastPt[0]);
      delta[1] = lastScale*(curPt[1] - lastPt[1]);
      delta[2] = 0;
      camera->SetFocalPoint(lastFocalPt[0] + delta[0], lastFocalPt[1] + delta[1], lastFocalPt[2] + delta[2]);
      camera->SetPosition(lastPos[0] + delta[0], lastPos[1] + delta[1], lastPos[2] + delta[2]);
      */
      this->InvokeEvent(vtkCommand::InteractionEvent);
      rwi->Render();
      }
    else if (this->Interaction == ZOOMING)
      {
      double motion = 10.0;
      double dyf = motion*(curPt[1] - lastPt[1])/this->CurrentRenderer->GetCenter()[1];
      double factor = pow(1.1, dyf);
      camera->SetParallelScale(camera->GetParallelScale() / factor);
      this->InvokeEvent(vtkCommand::InteractionEvent);
      rwi->Render();
      }
    }
  else if (this->Interaction == SELECTING)
    {
    this->EndPosition[0] = this->Interactor->GetEventPosition()[0];
    this->EndPosition[1] = this->Interactor->GetEventPosition()[1];  
    int *size = this->Interactor->GetRenderWindow()->GetSize();  
    if (this->EndPosition[0] > (size[0]-1))
      {
      this->EndPosition[0] = size[0]-1;
      }
    if (this->EndPosition[0] < 0)
      {
      this->EndPosition[0] = 0;
      }
    if (this->EndPosition[1] > (size[1]-1))
      {
      this->EndPosition[1] = size[1]-1;
      }
    if (this->EndPosition[1] < 0)
      {
      this->EndPosition[1] = 0;
      }
    this->RedrawRubberBand();
    }
  else
    {
       //this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->DrawOff();
       //this->Interactor->Render();
        //this->GetInteractor()->Render();
        //this->Interactor->GetRenderWindow()->CopyResultFrame();
        //this->Interactor->GetRenderWindow()->Frame();
        //this->Interactor->GetRenderWindow()->MakeCurrent(); 
        rwi->Render();
       //this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->DrawOn();
    }
}

/*!
    Redraws the rubberband during the interaction. It also turns off drawing of
    the main renderer to improve performance.
*/
void svkOverlaySelector::RedrawRubberBand()
{
  // Update the rubber band on the screen
  int *size = this->Interactor->GetRenderWindow()->GetSize();  

  vtkUnsignedCharArray *tmpPixelArray = vtkUnsignedCharArray::New();
  tmpPixelArray->DeepCopy(this->PixelArray);
  unsigned char *pixels = tmpPixelArray->GetPointer(0);

  int min[2], max[2];

  min[0] = this->StartPosition[0] <= this->EndPosition[0] ?
    this->StartPosition[0] : this->EndPosition[0];
  if (min[0] < 0) { min[0] = 0; }
  if (min[0] >= size[0]) { min[0] = size[0] - 1; }

  min[1] = this->StartPosition[1] <= this->EndPosition[1] ?
    this->StartPosition[1] : this->EndPosition[1];
  if (min[1] < 0) { min[1] = 0; }
  if (min[1] >= size[1]) { min[1] = size[1] - 1; }

  max[0] = this->EndPosition[0] > this->StartPosition[0] ?
    this->EndPosition[0] : this->StartPosition[0];
  if (max[0] < 0) { max[0] = 0; }
  if (max[0] >= size[0]) { max[0] = size[0] - 1; }

  max[1] = this->EndPosition[1] > this->StartPosition[1] ?
    this->EndPosition[1] : this->StartPosition[1];
  if (max[1] < 0) { max[1] = 0; }
  if (max[1] >= size[1]) { max[1] = size[1] - 1; }

  int i;
  for (i = min[0]; i <= max[0]; i++)
    {
    pixels[4*(min[1]*size[0]+i)] = 255 ^ pixels[4*(min[1]*size[0]+i)];
    pixels[4*(min[1]*size[0]+i)+1] = 255 ^ pixels[4*(min[1]*size[0]+i)+1];
    pixels[4*(min[1]*size[0]+i)+2] = 255 ^ pixels[4*(min[1]*size[0]+i)+2];
    pixels[4*(max[1]*size[0]+i)] = 255 ^ pixels[4*(max[1]*size[0]+i)];
    pixels[4*(max[1]*size[0]+i)+1] = 255 ^ pixels[4*(max[1]*size[0]+i)+1];
    pixels[4*(max[1]*size[0]+i)+2] = 255 ^ pixels[4*(max[1]*size[0]+i)+2];
    }
  for (i = min[1]+1; i < max[1]; i++)
    {
    pixels[4*(i*size[0]+min[0])] = 255 ^ pixels[4*(i*size[0]+min[0])];
    pixels[4*(i*size[0]+min[0])+1] = 255 ^ pixels[4*(i*size[0]+min[0])+1];
    pixels[4*(i*size[0]+min[0])+2] = 255 ^ pixels[4*(i*size[0]+min[0])+2];
    pixels[4*(i*size[0]+max[0])] = 255 ^ pixels[4*(i*size[0]+max[0])];
    pixels[4*(i*size[0]+max[0])+1] = 255 ^ pixels[4*(i*size[0]+max[0])+1];
    pixels[4*(i*size[0]+max[0])+2] = 255 ^ pixels[4*(i*size[0]+max[0])+2];
    }

  this->Interactor->GetRenderWindow()->SetRGBACharPixelData(0, 0, size[0]-1, size[1]-1, pixels, 0);
  //this->Interactor->GetRenderWindow()->SetRGBACharPixelData(0, 0, size[0]-1, size[1]-1, tmpPixelArray, 0);
  this->Interactor->GetRenderWindow()->Frame();
  
  tmpPixelArray->Delete();
}


/*!
    Callback for pressing the left button. This starts the rubberband
    interactor, and grabs a current image of the RenderWindow.
*/
void svkOverlaySelector::OnLeftButtonDown()
{
    vtkRenderWindowInteractor* rwi = this->GetInteractor();
    //rwi->Render();
  if(this->Interaction == NONE)
    {
    this->Interaction = SELECTING;
    vtkRenderWindow *renWin = this->Interactor->GetRenderWindow();
    this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->DrawOn();
    //this->Interactor->Render();
    //renWin->Render();
    //this->Interactor->Render();
    rwi->Render();
    renWin->SwapBuffersOff();
    renWin->DoubleBufferOff();
    
    renWin->Frame();
    this->Interactor->GetRenderWindow()->CopyResultFrame();
    this->Interactor->GetRenderWindow()->Frame();
    this->Interactor->GetRenderWindow()->MakeCurrent(); 
    
    this->StartPosition[0] = this->Interactor->GetEventPosition()[0];
    this->StartPosition[1] = this->Interactor->GetEventPosition()[1];
    this->EndPosition[0] = this->StartPosition[0];
    this->EndPosition[1] = this->StartPosition[1];

    this->PixelArray->Initialize();
    this->PixelArray->SetNumberOfComponents(4);
    int *size = renWin->GetSize();
    this->PixelArray->SetNumberOfTuples(size[0]*size[1]);

    renWin->GetRGBACharPixelData(0, 0, size[0]-1, size[1]-1, 0, this->PixelArray);
    renWin->SwapBuffersOn();
    renWin->DoubleBufferOn();
    this->FindPokedRenderer(this->StartPosition[0], this->StartPosition[1]);
    this->InvokeEvent(vtkCommand::StartInteractionEvent);
    }
}


//----------------------------------------------------------------------------
void svkOverlaySelector::Pan()
{
  if (this->CurrentRenderer == NULL)
    {
    return;
    }

  vtkRenderWindowInteractor *rwi = this->Interactor;

  double viewFocus[4], focalDepth, viewPoint[3];
  double newPickPoint[4], oldPickPoint[4], motionVector[3];
  
  // Calculate the focal depth since we'll be using it a lot

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  camera->GetFocalPoint(viewFocus);
  this->ComputeWorldToDisplay(viewFocus[0], viewFocus[1], viewFocus[2], 
                              viewFocus);
  focalDepth = viewFocus[2];

  this->ComputeDisplayToWorld(rwi->GetEventPosition()[0], 
                              rwi->GetEventPosition()[1],
                              focalDepth, 
                              newPickPoint);
    
  // Has to recalc old mouse point since the viewport has moved,
  // so can't move it outside the loop

  this->ComputeDisplayToWorld(rwi->GetLastEventPosition()[0],
                              rwi->GetLastEventPosition()[1],
                              focalDepth, 
                              oldPickPoint);
  
  // Camera motion is reversed

  motionVector[0] = oldPickPoint[0] - newPickPoint[0];
  motionVector[1] = oldPickPoint[1] - newPickPoint[1];
  motionVector[2] = oldPickPoint[2] - newPickPoint[2];
  
  camera->GetFocalPoint(viewFocus);
  camera->GetPosition(viewPoint);
  camera->SetFocalPoint(motionVector[0] + viewFocus[0],
                        motionVector[1] + viewFocus[1],
                        motionVector[2] + viewFocus[2]);

  camera->SetPosition(motionVector[0] + viewPoint[0],
                      motionVector[1] + viewPoint[1],
                      motionVector[2] + viewPoint[2]);
      
  if (rwi->GetLightFollowCamera()) 
    {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }
    
  rwi->Render();
}

//! Pure getter, returns the x value where the selection started.
int svkOverlaySelector::GetStartX() { return StartPosition[0]; }
//! Pure getter, returns the y value where the selection started.
int svkOverlaySelector::GetStartY() { return StartPosition[1]; }
//! Pure getter, returns the x value where the selection ended.
int svkOverlaySelector::GetEndX() { return EndPosition[0]; }
//! Pure getter, returns the y value where the selection ended.
int svkOverlaySelector::GetEndY() { return EndPosition[1]; }
