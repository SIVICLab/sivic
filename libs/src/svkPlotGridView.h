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


#ifndef SVK_PLOT_GRID_VIEW_H
#define SVK_PLOT_GRID_VIEW_H


#include <vtkObjectFactory.h>
#include <vtkProp3DCollection.h>
#include <vtkLabeledDataMapper.h>
#include <vtkSelectVisiblePoints.h>
#include <vtkImageClip.h>
#include <vtkTextProperty.h>
#include <vtkDoubleArray.h>

#include <svkDataView.h>
#include <svkPlotGridViewController.h>
#include <svkPlotGrid.h>
#include <svkDataValidator.h>
#include <svkObliqueReslice.h>

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
        virtual void                SetSlice( int slice );
        virtual void                SetTlcBrc( int tlcID, int brcID );
        virtual void                SetWindowLevelRange( double lower, double upper, int index );
        void                        SetComponent( svkBoxPlot::PlotComponent component );
        virtual void                SetRWInteractor( vtkRenderWindowInteractor* rwi );
        virtual void                Refresh();

        //! Enum represents objects in the scene
        typedef enum {
            VOL_SELECTION = 0, 
            SAT_BANDS,
            LAST_PROP = SAT_BANDS 
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
        enum DataInputs { MRS,MET };

        //! Enum represents different color schemes. Used for printing.
        typedef enum {
            FREQUENCY = 0,
            AMPLITUDE
        } WindowLevelRanges;

    protected:


        //  Members:
        int                    slice;
        svkPlotGrid*           plotGrid; 
        vector<vtkImageClip*>  metClippers;
        void                   CreateMetaboliteOverlay( svkImageData* data );
        void                   UpdateMetaboliteText( int* tlcBrc );
        void                   SetSelection( int* selectionArea );
        void                   HighlightSelectionVoxels();
        void                   SetColorSchema( int colorSchema );                
        int                    GetSlice();
        string                 GetDataCompatibility( svkImageData* data, int targetIndex );
        void                   SetChannel( int channel );
        int                    GetChannel( );


    private: 
        void                ResliceImage(svkImageData* input, svkImageData* target); 
        int                 channel;


};


}   //svk


#endif //SVK_PLOT_GRID_VIEW_H

