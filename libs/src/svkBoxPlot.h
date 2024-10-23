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

#ifndef SVK_BOX_PLOT_H
#define SVK_BOX_PLOT_H

#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkActor.h>
#include </usr/include/vtk/vtkPoints.h>
#include </usr/include/vtk/vtkProperty.h>
#include </usr/include/vtk/vtkFloatArray.h>
#include </usr/include/vtk/vtkPolyLine.h>
#include </usr/include/vtk/vtkPlane.h>
#include </usr/include/vtk/vtkTransform.h>
#include </usr/include/vtk/vtkPolyData.h>
#include </usr/include/vtk/vtkPolyDataMapper.h>
#include </usr/include/vtk/vtkTransformPolyDataFilter.h>
#include </usr/include/vtk/vtkRenderedAreaPicker.h>
#include </usr/include/vtk/vtkViewport.h>
#include </usr/include/vtk/vtkTexture.h>
#include </usr/include/vtk/vtkRenderer.h>
#include </usr/include/vtk/vtkOpenGLActor.h>


namespace svk {


/*!
 *  svkBoxPlot generates and handles an actor to be used in a plot grid.\
 *  This actor is a poly line, and its orginal coordinate system is inherently
 *  maintained. This mean that its x range is 0-npoints and y range is the min
 *  and max magnitudes of the data array that is input. The actor is then scaled
 *  to match the geometry of the current range, and location of the voxel in the
 *  given scan. It has a method called "initialize" which will create a legitimate
 *  actor, with all values zero-- to be used for testing purposes.
 *
 *  TODO: This class extends vtkActor, and overrides its RenderOpaqueGeometry. 
 *        this allows the object to be added to collections, and renders (where 
 *        it renders properly, but the actor still does not behave exactly as it
 *        should. It has an internal actor that is rendered for the 
 *        RenderOpaqueGemoetry, but when you use Get/SetProperty it has to be
 *        manually passed to this internal actor. This may be the only way to do
 *        it, but it may be worth checking out to see if there is a better way.
 *
 *  NOTE: This class is deprecated and no longer in use.
 */ 
class svkBoxPlot : public vtkOpenGLActor
{

    public:

        enum PlotComponent {
            REAL = 0,
            IMAGINARY,
            MAGNITUDE 
        };

        vtkTypeMacro( svkBoxPlot, vtkOpenGLActor );
        
        static svkBoxPlot*  New();

        svkBoxPlot();
        ~svkBoxPlot();

        void                Initialize();
        void                GeneratePolyData();
        vtkTransform*       GetTransform();
        void                SetData( vtkFloatArray* plotData );
        void                SetPlotAreaBounds( double* bounds );
        void                SetPointRange( int startPt, int endPt );
        void                SetValueRange( double minValue, double maxValue );
        double*             GetBounds();
        void                SetComponent( PlotComponent component );


    protected:
    
        //Members:
   
        //! The bounds in which the plot is to be placed 
        double*             plotAreaBounds;

        //! The index of the first point in the plotData to be plotted
        int                 startPt;

        //! The index of the last point in the plotData to be plotted
        int                 endPt; 

        //! The number of points in the plotData
        int                 numPoints;

        //! The minimum value to be plotted
        double              minValue;

        //! The maximum value to be plotted
        double              maxValue;

        //! The data to be plotted
        vtkFloatArray*      plotData;

        //! The data point values used to make up the line 
        vtkFloatArray*      pointData;

        //! The points that make up the line
        vtkPoints*          polyLinePoints;

    private:

        void            GenerateClippingPlanes();
        void            TransformActor();
        void            GenerateActor();

        //! Which component should we plot? 
        PlotComponent   plotComponent; 

};


}   //svk


#endif //SVK_BOX_PLOT_H
