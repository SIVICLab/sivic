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

/*
 *  NOTE: Some of this source was taken from vtkInteractorStyle2D DIRECTLY. Check about liscensing issues.
 */

#include <svkSpecGridSelector.h>


using namespace svk;


vtkStandardNewMacro(svkSpecGridSelector);

/*!
    Callback for a left button release. Clears the rubberband from the screen
    and throws a SelectionChangedEvent.
*/

/*
void svkSpecGridSelector::OnRightButtonDown()
{
    StartPosition[0] = -1;
    StartPosition[1] = -1;
    EndPosition[0]   = -1;
    EndPosition[1]   = -1;
}
*/
void svkSpecGridSelector::OnMouseMove()
{
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
      double delta[] = {0, 0, 0};
      delta[0] = -lastScale*(curPt[0] - lastPt[0]);
      delta[1] = lastScale*(curPt[1] - lastPt[1]);
      delta[2] = 0;
      camera->SetFocalPoint(lastFocalPt[0] + delta[0], lastFocalPt[1] + delta[1], lastFocalPt[2] + delta[2]);
      camera->SetPosition(lastPos[0] + delta[0], lastPos[1] + delta[1], lastPos[2] + delta[2]);
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
    unsigned int rect[5];
    rect[0] = this->StartPosition[0];
    rect[1] = this->StartPosition[1];
    rect[2] = this->EndPosition[0];
    rect[3] = this->EndPosition[1];

    this->InvokeEvent(vtkCommand::SelectionChangedEvent, reinterpret_cast<void*>(rect));
    this->InvokeEvent(vtkCommand::InteractionEvent);
    this->RedrawRubberBand();
    }
  else 
    {
    this->GetInteractor()->Render();
    }
}
//! Pure getter, returns the x value where the selection started.
int svkSpecGridSelector::GetStartX() { 
    return StartPosition[0]; 
}


//! Pure getter, returns the y value where the selection started.
int svkSpecGridSelector::GetStartY() { 
    return StartPosition[1]; 
}


//! Pure getter, returns the x value where the selection ended.
int svkSpecGridSelector::GetEndX() { 
    return EndPosition[0]; 
}


//! Pure getter, returns the y value where the selection ended.
int svkSpecGridSelector::GetEndY() { 
    return EndPosition[1]; 
}

//--------------------------------------------------------------------------
void svkSpecGridSelector::RedrawRubberBand()
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
  //this->Interactor->GetRenderWindow()->CopyResultFrame();
  this->Interactor->GetRenderWindow()->Frame();
  
  tmpPixelArray->Delete();
}
