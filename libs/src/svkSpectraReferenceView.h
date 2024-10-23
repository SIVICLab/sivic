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

#ifndef SVK_OVERLAY_VIEW_H
#define SVK_OVERLAY_VIEW_H


#include </usr/include/vtk/vtkImageViewer2.h>
#include </usr/include/vtk/vtkRenderWindow.h>
#include </usr/include/vtk/vtkRenderWindowInteractor.h>
#include </usr/include/vtk/vtkRenderer.h>
#include </usr/include/vtk/vtkCamera.h>
#include </usr/include/vtk/vtkImageMapToWindowLevelColors.h>
#include </usr/include/vtk/vtkProp3DCollection.h>
#include </usr/include/vtk/vtkAreaPicker.h>
#include </usr/include/vtk/vtkImageActor.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkExtractEdges.h>
#include </usr/include/vtk/vtkCellData.h>
#include </usr/include/vtk/vtkPointData.h>
#include </usr/include/vtk/vtkPlane.h>
#include </usr/include/vtk/vtkColorTransferFunction.h>
#include </usr/include/vtk/vtkLookupTable.h>
#include </usr/include/vtk/vtkScalarBarActor.h>

#include <svkSpectraReferenceViewController.h>
#include <svkImageViewer2.h>
#include <svkDataView.h>
#include <svkMrsImageData.h>
#include <svkDataValidator.h>
#include <svkObliqueReslice.h>
#include <svkPlotLineGrid.h>
#include <svkImageMapToColors.h>
#include <svkLookupTable.h>
#include <svkSatBandSet.h>


namespace svk {


// Determines how close to the voxels things are clipped
#define CLIP_TOLERANCE 0.001

class svkDataViewController;
class svkSpectraReferenceViewController;

/*!
 *   An implementation of DataView, this class is designed to create a
 *   visualization of the spectroscopy spatial information overlayed on the
 *   anatomical image. It uses a vtkImageViewer2 to present the anatamical data
 *   then the spectroscopy data is added.
 *
 *   TODO: Find a better (more condensed) way to deal with satband and overlay actors
 */
class svkSpectraReferenceView : public svkDataView
{
    friend class svkSpectraReferenceViewController;
    
    public:
        // vtk type revision macro
        vtkTypeMacro( svkSpectraReferenceView, svkDataView );

        static svkSpectraReferenceView*       New();
    
        svkSpectraReferenceView();
        ~svkSpectraReferenceView();

        // Methods:
        virtual void        SetInput( svkImageData* data, int index = 0);
        virtual void        SetSlice(int slice);
        virtual void        SetSlice(int slice, bool centerImage );
        virtual void        SetSlice(int slice, svkDcmHeader::Orientation orientation);
        virtual void        SetRWInteractor( vtkRenderWindowInteractor* );    
        virtual void        SetWindowLevelRange( double lower, double upper, int index );
        virtual void        GetWindowLevelRange( double &lower, double &upper, int index );
        void                SetComponent( svkPlotLine::PlotComponent component );
        string              GetDataCompatibility( svkImageData* data, int targetIndex );
        void                TurnOrthogonalImagesOn();
        void                TurnOrthogonalImagesOff();
        void                SetOrientation( svkDcmHeader::Orientation orientation );
        void                ToggleSelBoxVisibilityOn();
        void                ToggleSelBoxVisibilityOff();
        void                AlignCamera();
        bool                IsImageInsideSpectra();
        svkLookupTable*     GetLookupTable( );

        //! Enum represents different color schemes. Used for printing.
        typedef enum {
            FREQUENCY = 0,
            AMPLITUDE
        } WindowLevelRanges;


        //! Enum represents input indecies
        enum DataInputs { 
            MRI = 0, 
            MRS = 1, 
            OVERLAY = 2
        };

        //! Enum represents objects in the scene
        enum PropType {
            VOL_SELECTION = 0, 
            SAT_BANDS_AXIAL,
            SAT_BANDS_AXIAL_OUTLINE,
            SAT_BANDS_CORONAL,
            SAT_BANDS_SAGITTAL,
            SAT_BANDS_CORONAL_OUTLINE,
            SAT_BANDS_SAGITTAL_OUTLINE,
            PLOT_GRID,
            PLOT_LINES,
            AXIAL_OVERLAY_FRONT,
            AXIAL_OVERLAY_BACK,
            CORONAL_OVERLAY_FRONT,
            CORONAL_OVERLAY_BACK,
            SAGITTAL_OVERLAY_FRONT,
            SAGITTAL_OVERLAY_BACK,
            COORDINATES,
            COLOR_BAR,
            LAST_PROP = COLOR_BAR
        };

        //! Enum represents interpolation methods for the overlay 
        enum InterpolationType {
            NEAREST = 0,
            LINEAR,
            SINC 
        };

        //! Enum represents renderers in the window
        enum RendererType {
            PRIMARY = 0, 
            MOUSE_LOCATION,
            LAST_RENDERER = MOUSE_LOCATION
        };

        //! Enum represents color schema, used for printing
        enum {
            LIGHT_ON_DARK = 0,
            DARK_ON_LIGHT
        } ColorSchema;



    protected: 

        //! Represents the plotted lines of the grid
        svkPlotLineGrid*                plotGrid; 
        
        //! the vtkImageViewer2 object used to display the image 
        svkImageViewer2*                imageViewer;   

        //! the render window in which the view is to be displayed 
        vtkRenderWindow*                myRenderWindow;

        bool                            toggleSelBoxVisibility;
        bool                            imageInsideSpectra;

        //! Object used to window livel the overlay 
        svkImageMapToColors*            windowLevelerAxial;
        svkImageMapToColors*            windowLevelerCoronal;
        svkImageMapToColors*            windowLevelerSagittal;

        // Transfer function for rendering overlays
        svkLookupTable*                 colorTransfer;


        // Methods:
        void                            SetupMrInput( bool firstInput );
        void                            SetupMsInput( bool firstInput );
        void                            UpdateImageSlice( bool centerImage );
        void                            SetSelection( double* selectionArea, bool isWorldCords = 0 );
        void                            SetOverlayOpacity( double opacity );
        double                          GetOverlayOpacity( );
        void                            SetOverlayThreshold( double threshold );
        double                          GetOverlayThreshold( );
        void                            SetTlcBrc( int* tlcBrc );
        int*                            HighlightSelectionVoxels();
        void                            GenerateClippingPlanes( );
        void                            SetupOverlay();
        void                            SetInterpolationType( int interpolationType );
        void                            SetLUT( svkLookupTable::svkLookupTableType type );
        void                            ResetWindowLevel();
        int                             FindCenterImageSlice( int spectraSlice, svkDcmHeader::Orientation orientation );
        int                             FindSpectraSlice( int imageSlice, svkDcmHeader::Orientation orientation );
        bool                            IsSatBandForSliceOn( svkDcmHeader::Orientation orientation );
        bool                            IsSatBandOutlineForSliceOn( svkDcmHeader::Orientation orientation );
        bool                            AreAllSatBandsOn( svkDcmHeader::Orientation orientation );
        bool                            AreAllSatBandOutlinesOn( svkDcmHeader::Orientation orientation );

    private:

        void                            SetSliceOverlay();
        void                            ResliceImage(svkImageData* input, svkImageData* target, int targetIndex);

        double                          overlayOpacity;
        double                          overlayThreshold;

        
        //! Stores the interpolation method of the overlay
        InterpolationType               interpolationType; 

        //! Manipulates the actor that represents the sat bands
        svkSatBandSet*                 satBandsAxial;
        svkSatBandSet*                 satBandsCoronal;
        svkSatBandSet*                 satBandsSagittal;


        
};


}   //svk


#endif //SVK_OVERLAY_VIEW_H
