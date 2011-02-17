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

#ifndef SVK_PLOT_LINE_H
#define SVK_PLOT_LINE_H

#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkFloatArray.h>
#include <vtkPolyLine.h>


namespace svk {


/*!
 *
 */ 
class svkPlotLine : public vtkPolyLine
{
    friend class svkPlotLineGrid;

    public:

        enum PlotComponent {
            REAL = 0,
            IMAGINARY,
            MAGNITUDE 
        };

        typedef enum {
            ROW_COLUMN = 0,
            COLUMN_ROW,
            SLICE_ROW,
            ROW_SLICE,
            SLICE_COLUMN,
            COLUMN_SLICE
        } PlotDirection;


        vtkTypeRevisionMacro( svkPlotLine, vtkPolyLine );
        
        static svkPlotLine*  New();

        svkPlotLine();
        ~svkPlotLine();

        void                GeneratePolyData();
        void                SetData( vtkFloatArray* plotData );
        //void                SetPlotAreaBounds( double* bounds );
        void                SetPointRange( int startPt, int endPt );
        void                SetValueRange( double minValue, double maxValue );
        double*             GetBounds();
        void                SetComponent( PlotComponent component );
        vtkPoints*          GetDataPoints();
        void                SetDataPoints(vtkPoints* polyLinePoints);
        double*             GetOrigin();
        void                SetOrigin( double* origin );
        double*             GetSpacing();
        void                SetSpacing( double* spacing );
        void                GetDcos( double dcos[3][3] );
        void                SetDcos( double dcos[3][3] );
        void                SetInvertPlots( bool invertPlots );
        void                SetMirrorPlots( bool mirrorPlots );
        void                SetPlotDirection( int amplitudeDirection, int plotDirection );


    protected:
    
        //Members:
   
        //! The bounds in which the plot is to be placed 
        double              plotAreaBounds[6];

        //! The index of the first point in the plotData to be plotted
        int                 startPt;

        //! The index of the last point in the plotData to be plotted
        int                 endPt; 

        //! The number of points in the plotData
        int                 numPoints;

        //int                 numComponents;

        int                 componentOffset;

        //! The minimum value to be plotted
        double              minValue;

        //! The maximum value to be plotted
        double              maxValue;

        //! The data to be plotted
        vtkFloatArray*      plotData;

        float*              dataPtr;

        //! The magnitude of the data to be plotted
        //vtkFloatArray*      plotDataMagnitude;

        //! The data point values used to make up the line 
        vtkFloatArray*      pointData;

        //! The points that make up the line
        vtkPoints*          polyLinePoints;

        //! Should the plots be inverted?
        bool                invertPlots;

        //! Should the plots be flipped Left-Right?
        bool                mirrorPlots;

        PlotDirection       plotDirection;

    private:

        //! Which index should map to amplitude
        int amplitudeIndex;

        //! Which index should map to frequency/points
        int pointIndex;

        //! the scale 
        float scale[2]; 

        //! Which component should we plot? 
        PlotComponent   plotComponent; 

        //! Origin from which to start drawing the line
        double          origin[3];
        
        //! Size of the box to fill
        double          spacing[3];

        //! The dcos of the dataset
        double          dcos[3][3];

        void RecalculateScale();
        void RecalculatePlotAreaBounds();

};


}   //svk


#endif //SVK_PLOT_LINE_H
