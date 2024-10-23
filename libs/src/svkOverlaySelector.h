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

#ifndef SELECT_BOX_INTERACTOR_H
#define SELECT_BOX_INTERACTOR_H

#include </usr/include/vtk/vtkInteractorStyleRubberBand2D.h>
#include </usr/include/vtk/vtkInteractorStyleRubberBand2D.h>
#include </usr/include/vtk/vtkCommand.h>
#include </usr/include/vtk/vtkRenderWindowInteractor.h>
#include </usr/include/vtk/vtkRenderWindow.h>
#include </usr/include/vtk/vtkUnsignedCharArray.h>
#include </usr/include/vtk/vtkRenderer.h>
#include </usr/include/vtk/vtkCamera.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkRendererCollection.h>
#include </usr/include/vtk/vtkCornerAnnotation.h>


namespace svk {


/*!
 *  This class extends vtkInteractorStyleRubberBand2D to create accesor
 *  methods, and to modify some interactions. These accesor methods allow us to 
 *  grab the starting and ending points of the drag selection. Also the some
 *  methods have been reimplementd due to control rendering. NOTE: Currently
 *  the OnMouseMove method inverts the y-input to compensate for the camera
 *  positien. This should be changed once the coordinate system issues are
 *  taken care off.
 */
class svkOverlaySelector : public vtkInteractorStyleRubberBand2D
{

    public:
    
        //vtkTypeMacro( svkOverlaySelector, vtkInteractorStyleRubberBand2D );

        static       svkOverlaySelector* New(); 
        virtual void OnLeftButtonUp();
        virtual void OnMouseMove();
        virtual void RedrawRubberBand(); 
        virtual void OnLeftButtonDown();
        int          GetStartX(); 
        int          GetStartY();
        int          GetEndX();
        int          GetEndY();

    private:

        void         Pan();
    
};


}   //svk


#endif //SELECT_BOX_INTERACTAR_H
