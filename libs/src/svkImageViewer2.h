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
 *  NOTE:: SOME SOURCE WAS DIRECTLY TAKEN FROM VTKIMAGEVIEWER2
 */


#ifndef SVK_IMAGE_VIEWER_2_H
#define SVK_IMAGE_VIEWER_2_H

#include </usr/include/vtk/vtkImageViewer2.h>
#include </usr/include/vtk/vtkImageMapper3D.h>
#include </usr/include/vtk/vtkTransform.h>
#include </usr/include/vtk/vtkImageActor.h>
#include </usr/include/vtk/vtkRenderWindow.h>
#include </usr/include/vtk/vtkRenderWindowInteractor.h>
#include </usr/include/vtk/vtkInteractorStyleImage.h>
#include </usr/include/vtk/vtkCommand.h>
#include </usr/include/vtk/vtkCamera.h>
#include </usr/include/vtk/vtkImageMapToWindowLevelColors.h>

#include <svkImageMapToWindowLevelColors.h>
#include <svkImageData.h>
#include <svkOrientedImageActor.h>
#include </usr/include/vtk/vtkRenderer.h>
#include </usr/include/vtk/vtkCollectionIterator.h>


namespace svk {


class svkImageViewer2 : public vtkImageViewer2
{

    public:

        // vtk type revision macro
        vtkTypeMacro( svkImageViewer2, vtkImageViewer2 );
   
        static svkImageViewer2*             New();  
        
        void                                SetInputData( svkImageData *in );
        virtual void                        SetSlice( int slice );
        virtual void                        SetSlice( int slice, svkDcmHeader::Orientation sliceOrientation );
        virtual void                        SetActiveVolume( int volume );
        void                                Render();
        void                                ResetCamera();
        void                                TurnOrthogonalImagesOn();
        void                                TurnOrthogonalImagesOff();
        int                                 GetSlice();
        int                                 GetSlice( svkDcmHeader::Orientation orientation );
        virtual void                        SetColorLevel(double s);
        virtual void                        SetColorWindow(double s);
        virtual void                        SetOrientation( svkDcmHeader::Orientation orientation );
        virtual svkDcmHeader::Orientation   GetOrientation( );
        virtual svkOrientedImageActor*      GetImageActor( 
                                                svkDcmHeader::Orientation actorOrientation = svkDcmHeader::UNKNOWN_ORIENTATION );
        virtual svkImageData*               GetInput();
        virtual bool                        AreOrthogonalImagesOn();
        void                                SetInteractorStyle( vtkInteractorStyleImage* style );
        void                                SetCameraZoom( double zoom ); 

        int                                 axialSlice;
        int                                 coronalSlice;
        int                                 sagittalSlice;
        void                                UpdateInputInformation(); 

    protected:

        svkImageViewer2();
        ~svkImageViewer2();

        svkImageData* data;

        int                                 orthSlice1;
        int                                 orthSlice2;

        svkDcmHeader::Orientation           orientation;

        void                                InitializeOrthogonalActors();
        void                                InstallPipeline();
        svkOrientedImageActor*              axialImageActor;
        svkOrientedImageActor*              coronalImageActor;
        svkOrientedImageActor*              sagittalImageActor;

        //svkOpenGLOrientedImageActor*      orthImageActor1;
        //svkOpenGLOrientedImageActor*      orthImageActor2;
        //svkImageMapToWindowLevelColors*   orthWinLevel1;
        //svkImageMapToWindowLevelColors*   orthWinLevel2;
        svkImageMapToWindowLevelColors*     axialWinLevel;
        svkImageMapToWindowLevelColors*     coronalWinLevel;
        svkImageMapToWindowLevelColors*     sagittalWinLevel;

    private: 
        double                              cameraZoom;  
	

};


}   //svk


#endif //SVK_IMAGE_VIEWER_2_H
