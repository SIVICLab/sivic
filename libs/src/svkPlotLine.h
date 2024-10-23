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

#ifndef SVK_PLOT_LINE_H
#define SVK_PLOT_LINE_H

#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkPoints.h>
#include </usr/include/vtk/vtkDataArray.h>
#include </usr/include/vtk/vtkPolyLine.h>
#include <vector>


namespace svk {

using namespace vtkstd;

/*!
 * svkPlotLine is a utility class that manipulates values in an array
 * of x-y-z floating point values. It can be used to manipulate
 * the points in a vtkPolyData object to to represent a vtkDataArray.
 *
 */ 
class svkPlotLine : public vtkObject
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


        vtkTypeMacro( svkPlotLine, vtkObject );
        
        static svkPlotLine*  New();

        svkPlotLine();
        ~svkPlotLine();

        void                SetData( vtkDataArray* plotData );
        //void                SetPlotAreaBounds( double* bounds );
        void                SetPointRange( int startPt, int endPt );
        void                SetValueRange( double minValue, double maxValue );
        double*             GetBounds();
        void                SetComponent( PlotComponent component );
        float*              GetDataPoints();
        void                SetDataPoints(float* polyLinePoints);
        double*             GetOrigin();
        void                SetOrigin( double* origin );
        double*             GetSpacing();
        void                SetSpacing( double* spacing );
        void                GetDcos( double dcos[3][3] );
        void                SetInvertPlots( bool invertPlots );
        void                SetMirrorPlots( bool mirrorPlots );
        void                SetPlotDirection( int amplitudeDirection, int plotDirection );
        void				Modified();

        void	    		SetDcos(vector< vector<double> >* dcos);

        // vtk macros
        void				SetGeneratePolyData( bool generatePolyData );
        bool				GetGeneratePolyData( );

        template <class T>
		void               GeneratePolyDataTemplated( T* castDataPtr );

    protected:
    
        //Members:

        //! when modified should this object regenerate the poly data.
        bool				generatePolyData;
   
        //! The bounds in which the plot is to be placed 
        double              plotAreaBounds[6];

        //! The index of the first point in the plotData to be plotted
        int                 startPt;

        //! The index of the last point in the plotData to be plotted
        int                 endPt; 

        //! The number of points in the plotData
        int                 numPoints;

        //! The number of components
        int                 numComponents;

        int                 componentOffset;

        //! The minimum value to be plotted
        double              minValue;

        //! The maximum value to be plotted
        double              maxValue;

        //! The data to be plotted
        vtkDataArray*      plotData;

        void*               dataPtr;

        int 				dataType;

        //! The points that make up the line
        float*              polyLinePoints;

        //! Should the plots be inverted?
        bool                invertPlots;

        //! Should the plots be flipped Left-Right?
        bool                mirrorPlots;

        PlotDirection       plotDirection;

        void                GeneratePolyData();


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
        vector< vector<double> >*  dcos;

        void RecalculateScale();
        void RecalculatePlotAreaBounds();

};


}   //svk


#endif //SVK_PLOT_LINE_H
