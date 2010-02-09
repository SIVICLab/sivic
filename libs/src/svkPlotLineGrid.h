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


#ifndef SVK_PLOT_LINE_GRID_H_INCL
#define SVK_PLOT_LINE_GRID_H_INCL


#include <vector>
#include <vtkRenderer.h>

#include <vtkImageData.h>
#include <vtkCellData.h>
#include <vtkCamera.h>
#include <vtkProp3DCollection.h>
#include <vtkDataSetCollection.h>
#include <vtkCallbackCommand.h>
#include <vtkAppendPolyData.h>
#include <vtkAreaPicker.h>
#include <vtkCollectionIterator.h>
#include <vtkTransform.h>
#include <svkPlotLine.h>
#include <svkAreaPicker.h>
#include <svkImageData.h>
#include <svkMrsImageData.h>


namespace svk {


using namespace std;

/*
 * Representation for spectroscopic data. It creates a vtkPolyData to represent
 * the spatial location of the data aquired, and uses svkPlotLien objects
 * to represent the plot lines. There are methods for controlling the range,
 * slice, etc.
 *
 */
class svkPlotLineGrid : public vtkObject
{
    friend class svkPlotGridView;

    public:
        // vtk type revision macro
        vtkTypeRevisionMacro( svkPlotLineGrid, vtkObject );

        static svkPlotLineGrid*       New();

        svkPlotLineGrid();
        ~svkPlotLineGrid();


        //  Methods
        void                    SetInput( svkMrsImageData* data );
        void                    Update();
        void                    SetTlcBrc(int tlcBrc[2]);
        void                    SetSlice(int slice);
        void                    SetRenderer(vtkRenderer* renderer);
        //int*                    GetCurrentTlcBrc();
        void                    SetFrequencyWLRange(int lower, int range); 
        void                    SetIntensityWLRange(double lower, double range); 
        //void                    SetSelection(int* selectionArea);
        void                    AlignCamera( bool invertView = 1 );
        void                    SetComponent( svkPlotLine::PlotComponent component );
        int                     GetComponent( );
        void                    SetChannel( int channel );
        void                    SetTimePoint( int timePoint );
        void                    UpdateDataArrays(int tlc, int brc);
        void                    SetOrientation( svkDcmHeader::Orientation orientation );

    private:

        //  Members:

        int                         tlcBrc[2];

        //! The current slice
        int                         slice;

        //! The current channel
        int                         channel;

        //! The current timePoint
        int                         timePoint;

        //! The ID's of the top left corner, and bottom right corner cells 
        //int                         tlcBrc[2];

        //! Represents the current slice of plot lines
        vtkCollection*              xyPlots;

        //! The actor that holds the plot grid
        vtkActor*                   plotGridActor;

        //! The index of the first point in the plots to be plotted-
        int                         plotRangeX1;     

        //! The index of the last point in the plots to be plotted-
        int                         plotRangeX2;     

        //! The minimum value to be plotted
        double                      plotRangeY1;     

        //! The maximum value to be plotted
        double                      plotRangeY2;     

        //! Array of booleans keeps track of frequency changes
        bool*                       freqUpToDate;

        //! Array of booleans keeps track of frequency changes
        bool*                       ampUpToDate;

        //! Has the channel been modified since the last slice update 
        bool                        channelChanged;

        //! The bounds of the camera that this will be rendered in
        double*                     viewBounds;

        //! The data
        svkMrsImageData*               data;

        //! The context in which to render
        vtkRenderer*                renderer; 

        //! The selection box actor in LPS coordinates
        vtkActor*                   selectionBoxActor;

        //! A callback to update the plot lines when the data changes
        vtkCallbackCommand*         dataModifiedCB;

        //! Component to display (e.g. REAL, IMAG..)
        svkPlotLine::PlotComponent   plotComponent;

        svkDcmHeader::Orientation orientation;

        vtkPoints*                  points;

        //  Methods:
        void                        RegeneratePlots();
        static void                 UpdateData(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);
        void                        AllocateXYPlots();        
        void                        RemoveActors();
        void                        UpdatePlotRange();
        void                        UpdateComponent();
        void                        UpdateOrientation();
        void                        GenerateActor();
        void                        HighlightSelectionVoxels();

};


}   //svk


#endif //SVK_PLOT_LINE_GRID_H_INCL
