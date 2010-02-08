/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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

#include <vtkImageViewer2.h>
#include <vtkTransform.h>
#include <vtkImageActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkCommand.h>
#include <vtkCamera.h>
#include <vtkImageMapToWindowLevelColors.h>

#include <svkImageMapToWindowLevelColors.h>
#include <svkImageData.h>
#include <svkOpenGLOrientedImageActor.h>


namespace svk {


class svkImageViewer2 : public vtkImageViewer2
{

    public:

        // vtk type revision macro
        vtkTypeRevisionMacro( svkImageViewer2, vtkImageViewer2 );
   
        static svkImageViewer2*       New();  
        
        void SetInput( svkImageData *in );
        virtual void SetSlice( int slice );
        virtual void SetSlice( int slice, svkDcmHeader::Orientation sliceOrientation );
        void Render();
        void ResetCamera();
        void TurnOrthogonalImagesOn();
        void TurnOrthogonalImagesOff();
        int  GetSlice();
        int  GetSlice( svkDcmHeader::Orientation orientation );
        virtual void SetColorLevel(double s);
        virtual void SetColorWindow(double s);
        virtual void                      SetOrientation( svkDcmHeader::Orientation orientation );
        virtual svkDcmHeader::Orientation GetOrientation( );
        virtual svkOpenGLOrientedImageActor* GetImageActor( svkDcmHeader::Orientation actorOrientation = svkDcmHeader::UNKNOWN );
        virtual svkImageData* GetInput();

        int axialSlice;
        int coronalSlice;
        int sagittalSlice;

    protected:

        svkImageViewer2();
        ~svkImageViewer2();

        svkImageData* data;

        int orthSlice1;
        int orthSlice2;

        svkDcmHeader::Orientation orientation;


        void InitializeOrthogonalActors();

        void                  InstallPipeline();
        svkOpenGLOrientedImageActor*             axialImageActor;
        svkOpenGLOrientedImageActor*             coronalImageActor;
        svkOpenGLOrientedImageActor*             sagittalImageActor;

        //svkOpenGLOrientedImageActor*             orthImageActor1;
        //svkOpenGLOrientedImageActor*             orthImageActor2;
        //svkImageMapToWindowLevelColors*          orthWinLevel1;
        //svkImageMapToWindowLevelColors*          orthWinLevel2;
        svkImageMapToWindowLevelColors*          axialWinLevel;
        svkImageMapToWindowLevelColors*          coronalWinLevel;
        svkImageMapToWindowLevelColors*          sagittalWinLevel;

};


}   //svk


#endif //SVK_IMAGE_VIEWER_2_H
