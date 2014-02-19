/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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


#ifndef SVK_PLOT_GRID_VIEW_H
#define SVK_PLOT_GRID_VIEW_H


#include <vtkObjectFactory.h>
#include <vtkProp3DCollection.h>
#include <svkLabeledDataMapper.h>
#include <vtkSelectVisiblePoints.h>
#include <svkImageClip.h>
#include <vtkImageActor.h>
#include <vtkTextProperty.h>
#include <vtkDoubleArray.h>
#include <vtkExtractEdges.h>
#include <svkImageMapToColors.h>
#include <svkDataView.h>
#include <svkPlotGridViewController.h>
#include <svkPlotLine.h>
#include <svkPlotLineGrid.h>
#include <svkDataValidator.h>
#include <svkObliqueReslice.h>
#include <svkLookupTable.h>
#include <svkImageClip.h>
#include <svkSatBandSet.h>
#include <svkVoxelTaggingUtils.h>
#include <vtkCellCenters.h>
#include <vtkXYPlotActor.h>
#include <vtkCursor2D.h>
#include <vtkCleanPolyData.h>
#include <vtkPolyDataPointSampler.h>
#include <svkDetailedPlotDirector.h>
#include <svkMrsTopoGenerator.h>
#include <svkOrientedImageActor.h>

#include <vector>


namespace svk {


using namespace std;


// Note forward declaration to avoid self refererncing includes. 
class svkPlotGridViewController; 

/*!
 *  A concrete implementation of svkDataView. Its puropose is to visualize 4D
 *  data plots in a rectangular grid.
 */
class svkPlotGridView : public svkDataView 
{

    friend class svkPlotGridViewController;

    public:

        // vtk type revision macro
        vtkTypeRevisionMacro( svkPlotGridView, svkDataView );

        static svkPlotGridView*       New();
    
        svkPlotGridView();
        ~svkPlotGridView();

        //  Methods:
        virtual void                SetInput( svkImageData* data, int index );
        virtual void                AddReferenceInput( svkImageData* data );
        virtual void                RemoveInput( int index );
        virtual void                SetSlice( int slice );
        virtual void                SetTlcBrc( int tlcBrc[2] );
        virtual void                SetTlcBrc( int tlcID, int brcID );
        virtual void                SetWindowLevelRange( double lower, double upper, int index );
        virtual void                GetWindowLevelRange( double &lower, double &upper, int index );
        virtual void                SetOverlayWLRange( double* range );
        virtual double*             GetOverlayWLRange( );
        void                        SetComponent( svkPlotLine::PlotComponent component, int plotIndex = -1 );
        void                        SetActiveComponent( svkPlotLine::PlotComponent component );
        svkPlotLine::PlotComponent  GetActiveComponent( );
        virtual void                SetVolumeIndex( int index, int volumeIndex = 0, int plotIndex = -1 );
        virtual int                 GetVolumeIndex( int volumeIndex = 0 );
        virtual int*                GetVolumeIndexArray(  );
        void                        SetPlotUnits( svkSpecPoint::UnitType plotUnitType );
        virtual void                SetRWInteractor( vtkRenderWindowInteractor* rwi );
        virtual void                SetPlotColor( int plotIndex, double* rgb );
        virtual double*             GetPlotColor( int plotIndex );
        virtual void                SetPlotLineWidth( float width );
        virtual void                SetPlotVisibility( int plotIndex, bool visible );
        virtual bool                GetPlotVisibility( int plotIndex );
        virtual int                 GetNumberOfReferencePlots( );
        virtual void                SetActivePlotIndex( int plotIndex );
        virtual svkImageData*       GetActivePlot( );
        virtual int                 GetActivePlotIndex( );
        virtual void                Refresh();
        void                        GeneratePlotGridActor();  
        void                        GenerateClippingPlanes();
        virtual void                SetOrientation( svkDcmHeader::Orientation orientation );
        virtual void                AlignCamera();
        svk4DImageData*             GetActiveInput();

        //! Enum represents objects in the scene
        typedef enum {
            VOL_SELECTION = 0, 
            OVERLAY_IMAGE,
            OVERLAY_TEXT,
            PLOT_GRID,
            PLOT_LINES,
            DETAILED_PLOT,
            RULER,
            SAT_BANDS,
            SAT_BANDS_OUTLINE,
            LAST_PROP = SAT_BANDS_OUTLINE
        } ActorType;

        //! Enum represents renderers in the window
        typedef enum {
            PRIMARY = 0,
            LAST_RENDERER = PRIMARY 
        } RendererType;

        //! Enum represents different color schemes. Used for printing.
        typedef enum {
            LIGHT_ON_DARK = 0,
            DARK_ON_LIGHT
        } ColorSchema;

        //! Enum represent the data inputs
        enum DataInputs { MR4D, MET, ADDITIONAL_MR4D };

        //! Enum represents different color schemes. Used for printing.
        typedef enum {
            FREQUENCY = 0,
            AMPLITUDE
        } WindowLevelRanges;
        void                   SetActiveOverlayVolume( int volume );

    protected:


        //  Members:
        vector<svkPlotLineGrid*> plotGrids; 
        vector<svkImageClip*>  metClippers;
        vector<vtkActor2D*>    overlayTextActors;
        double                 referencePlotColors[10][3];
        void                   CreateMetaboliteOverlay( svkImageData* data );
        void                   UpdateMetaboliteText( int* tlcBrc );
        void                   UpdateMetaboliteImage( int* tlcBrc );
        void                   UpdateMetaboliteTextDisplacement( );
        void                   UpdateDetailedPlot( int* tlcBrc );
        void                   SetSelection( double* selectionArea, bool isWorldCords = 0 );
        int*                   HighlightSelectionVoxels();
        void                   SetColorSchema( int colorSchema );                
        string                 GetDataCompatibility( svkImageData* data, int targetIndex );
        void                   SetOverlayOpacity( double opacity );
        void                   SetOverlayThreshold( double threshold );
        void                   SetLUT( svkLookupTable::svkLookupTableType type );

    private: 
        void                     ResliceImage(svkImageData* input, svkImageData* target);
        int                      numColors;
        svkLookupTable*          colorTransfer;
        svkSatBandSet*           satBands;
        int                      activePlot;
        svkDetailedPlotDirector* detailedPlotDirector;
        svkSpecPoint::UnitType   plotUnitType;
        svkImageMapToColors*     windowLevel;
        vtkstd::vector<int>      volumeIndexVector;

        static const double CLIP_TOLERANCE;


};


}   //svk


#endif //SVK_PLOT_GRID_VIEW_H

