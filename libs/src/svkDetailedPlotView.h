/*
 *  Copyright © 2009-2011 The Regents of the University of California.
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


#ifndef SVK_DETAILED_PLOT_VIEW_H
#define SVK_DETAILED_PLOT_VIEW_H


#include <vtkObjectFactory.h>
#include <vtkProp3DCollection.h>
#include <vtkLabeledDataMapper.h>
#include <vtkSelectVisiblePoints.h>
#include <vtkImageClip.h>
#include <vtkTextProperty.h>
#include <vtkDoubleArray.h>
#include <vtkXYPlotActor.h>
#include <vtkXYPlotWidget.h>
#include <vtkCollectionIterator.h>
#include <vtkAxisActor2D.h>
#include <vtkProperty2D.h>

#include <svkSpecPoint.h>
#include <svkDataView.h>
#include <svkBoxPlot.h>
#include <svkPlotLine.h>
#include <svkMrsImageData.h>

#include <vector>


namespace svk {


using namespace std;
using namespace svk;



// Note forward declaration to avoid self refererncing includes. 
class svkDetailedPlotViewController; 

/*!
 *  A concrete implementation of svkDataView. Its puropose is to visualize 4D
 *  data plots in a rectangular grid.
 */
class svkDetailedPlotView : public svkDataView 
{

    friend class svkDetailedPlotViewController;

    public:

        // vtk type revision macro
        vtkTypeRevisionMacro( svkDetailedPlotView, svkDataView );

        static svkDetailedPlotView*       New();
    
        svkDetailedPlotView();
        ~svkDetailedPlotView();

        //  Methods:
        virtual void                SetInput(svkImageData* data, int index);
        virtual void                SetSlice(int slice);
        virtual void                SetTlcBrc(int tlcID, int brcID);
        virtual void                SetWindowLevelRange( double lower, double upper, int index );
        virtual void                SetRWInteractor( vtkRenderWindowInteractor* rwi);
                void                SetComponent( svkBoxPlot::PlotComponent component);
        virtual void                Refresh();
        void                        AddPlot( int index, int component, int channel = 0, int timepoint = 0 );
        void                        SetUnits( int units);
        void                        Update();

        typedef enum {
            XYPLOT = 0, 
            LAST_PROP = XYPLOT 
        } ActorType;

        //! Enum represents renderers in the window
        typedef enum {
            PRIMARY = 0,
            LAST_RENDERER = PRIMARY 
        } RendererType;

        typedef enum {
            FREQUENCY = 0,
            AMPLITUDE
        } WindowLevelRanges;

        vtkXYPlotActor* plotActor;

    protected:

        int                 slice;
        void                CreateXYPlot();
        void                CalculateXAxisRange();
    
    private:
        
        svkSpecPoint*   point;
        vtkFloatArray*  frequencyPoints;
        int             specUnits;
        int             numPlots;
        vector<int>     inputIndices;
        vector<int>     inputComponents;
        vector<int>     inputChannels;
        // We need to replace the x axis due to some problems with vtkXYPlotActor
        // It will not correctly reverse the axis, so we need to do it ourselves.
        vtkAxisActor2D* xAxis;
        
        void            CreateFrequencyArray();

};


}   //svk


#endif //SVK_DETAILED_PLOT_VIEW_H

