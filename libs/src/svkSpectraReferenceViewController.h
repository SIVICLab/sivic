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


#ifndef SVK_OVERLAY_VIEW_CONTROLLER_H
#define SVK_OVERLAY_VIEW_CONTROLLER_H

// standard headers
#include <map>
#include <vector>
#include <sstream>
#include <math.h>

// vtk headers
#include </usr/include/vtk/vtkImageData.h>
#include </usr/include/vtk/vtkRenderWindow.h>
#include </usr/include/vtk/vtkRenderWindowInteractor.h>
#include </usr/include/vtk/vtkActorCollection.h>
#include </usr/include/vtk/vtkImageViewer2.h>
#include </usr/include/vtk/vtkActor.h>
#include </usr/include/vtk/vtkActor2D.h>
#include </usr/include/vtk/vtkCallbackCommand.h>
#include </usr/include/vtk/vtkRenderer.h>
#include </usr/include/vtk/vtkCommand.h>
#include </usr/include/vtk/vtkCamera.h>
#include </usr/include/vtk/vtkCollectionIterator.h>
#include </usr/include/vtk/vtkCoordinate.h>
#include </usr/include/vtk/vtkInteractorStyleImage.h>
#include </usr/include/vtk/vtkInteractorObserver.h>
#include </usr/include/vtk/vtkImageMapToWindowLevelColors.h>
#include </usr/include/vtk/vtkCornerAnnotation.h>
#include </usr/include/vtk/vtkPolyDataMapper2D.h>
#include </usr/include/vtk/vtkPolyData.h>
#include </usr/include/vtk/vtkCubeSource.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkProperty2D.h>
#include </usr/include/vtk/vtkTextActor.h>
#include </usr/include/vtk/vtkTextProperty.h>
#include </usr/include/vtk/vtkAxesActor.h>
#include </usr/include/vtk/vtkPointPicker.h>
#include </usr/include/vtk/vtkPlane.h>

#include <svkSpectraReferenceView.h>
#include <svkOverlaySelector.h>
#include <svkDataView.h>
#include <svkLookupTable.h>


namespace svk {


using namespace std;

class svkSpectraReferenceView;

/*!  
 *  svkSpectraReferenceViewController is a concrete implementation of DataViewController.
 *  It is designed to work with the svkSpectraReferenceView to create a workspace for 
 *  overlaying the spatial location of spectroscopy voxels, with the anatomical
 *  image from the same scan.
 *
 */
class svkSpectraReferenceViewController: public svkDataViewController
{

    public:

 
        vtkTypeMacro( svkSpectraReferenceViewController, svkDataViewController );

        static svkSpectraReferenceViewController*       New();

        svkSpectraReferenceViewController();
        ~svkSpectraReferenceViewController();

        enum InteractorStyle { SELECTION, WINDOW_LEVEL, COLOR_OVERLAY, ROTATION };

        //  Methods
        virtual void              SetInput( svkImageData* data, int index = 0 );
        virtual void              SetSlice( int slice );
        virtual void              SetSlice( int slice, bool centerImage );
        virtual void              SetSlice(int slice, svkDcmHeader::Orientation orientation);
        virtual int               GetImageSlice( svkDcmHeader::Orientation sliceOrientation = svkDcmHeader::UNKNOWN_ORIENTATION );
        virtual void              SetRWInteractor(vtkRenderWindowInteractor*);
        virtual void              TurnPropOn( int propIndex );
        virtual void              TurnPropOff( int propIndex );
        virtual int*              GetTlcBrc();
        virtual void              SetTlcBrc( int* tlcBrc );
        void                      SetOverlayOpacity(double opacity);
        void                      SetOverlayThreshold(double threshold);
        double                    GetOverlayThreshold();
        double                    GetOverlayThresholdValue();
        int                       GetCurrentStyle();
        void                      SetCurrentStyle( int style );
        void                      UseWindowLevelStyle();
        void                      UseColorOverlayStyle();
        void                      UseSelectionStyle();
        void                      UseRotationStyle();
        void                      ResetWindowLevel();
        void                      HighlightSelectionVoxels();    
        void                      Reset();
        string                    GetDataCompatibility( svkImageData* data, int targetIndex );
        void                      SetInterpolationType( int interpolationType);
        void                      SetLUT( svkLookupTable::svkLookupTableType type );
        void                      TurnOrthogonalImagesOn();
        void                      TurnOrthogonalImagesOff();
        bool                      IsImageInsideSpectra();



    protected:
       
        //! enum represents data indicies 
        enum DataInputs { 
            MRI = 0,
            MRS = 1,
            MET = 2 
        };

        //! enum represents data indicies 

        //! determines of the visualization has been initialized
        bool visualizationCreated;
        //! the views render window 
        vtkRenderWindow* myRenderWindow;
        //! callback that responds to mouse movements to update cursor location display
        vtkCallbackCommand* cursorLocationCB;
        //! callback that responds to left click to highlight voxels 
        vtkCallbackCommand* dragSelectionCB;
        //! callback that responds to left click to highlight voxels 
        vtkCallbackCommand* colorOverlayCB;
        vtkCallbackCommand* interactorSwitchCB;
        //! the actor that represents the mouse location
        //vtkCornerAnnotation* mousePositionText;
        vtkTextActor* mousePositionText;
        //! the renderer used to display the mouse location
        vtkRenderer* mouseLocRenderer;
        //! interactor style for window leveling
        vtkInteractorObserver* windowLevelStyle;
        //! interactor style for window leveling
        vtkInteractorObserver* colorOverlayStyle;
        //! interactor style for 3D rotation 
        vtkInteractorObserver* rotationStyle;
        //! interactor style for selecting voxels 
        svkOverlaySelector* dragSelectStyle;
        //! actor that represents the scan boundry
        vtkActor* scanBoundryActor; 
        double initialColorWindowLevel[2];   
        int pickPos[2];   
        double startOpacity;   
        int currentInteractorStyle;
  
    private:

 

        // Callback Command methods
        static void UpdateCursorLocation( vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);
        static void UpdateInteractor( vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);
        static void UpdateSelection(      vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);
        static void ColorWindowLevel(      vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);
};


}   //svk


#endif //SVK_OVERLAY_VIEW_CONTROLLER_H
