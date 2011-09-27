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
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/libs/src/svkDetailedPlotView.h $
 *  $Rev: 913 $
 *  $Author: beckn8tor $
 *  $Date: 2011-04-13 13:19:09 -0700 (Wed, 13 Apr 2011) $
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#ifndef SVK_DETAILED_PLOT_DIRECTOR_H
#define SVK_DETAILED_PLOT_DIRECTOR_H


#include <vtkObjectFactory.h>
#include <vtkDataArray.h>
#include <vtkAxisActor2D.h>
#include <svkXYPlotActor.h>
#include <vtkTextProperty.h>
#include <svkXYPlotActor.h>
#include <vtkCollectionIterator.h>
#include <vtkAxisActor2D.h>
#include <vtkProperty2D.h>
#include <vtkObject.h>
#include <vtkDataSet.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCursor2D.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>

#include <svkSpecPoint.h>
#include <svkPlotLine.h>
#include <svkDcmHeader.h>
#include <vector>


namespace svk {


using namespace std;
using namespace svk;



/*!
 *  The purpose if this class is to provide a simplified interface to
 *  the svkXYPlotActor that will work with simple vtkDataArray objects
 *  instead of requiring full fledged vtkDataObjects.
 */
class svkDetailedPlotDirector : public vtkObject
{

    public:

        // vtk type revision macro
        vtkTypeRevisionMacro( svkDetailedPlotDirector, vtkObject );

        static svkDetailedPlotDirector*       New();
    

        //  Methods:
        virtual void                AddInput( vtkDataArray* array, int component, vtkDataObject* sourceToObserve );
        virtual void                RemoveAllInputs( );
        virtual void                RemoveInput( vtkDataArray* array );
        virtual void                SetPlotColor( int plotIndex, double* rgb);

        virtual void                SetIndexRange( int lower, int upper );
        virtual void                GenerateAbscissa( double firstPointValue, double lastPointValue );
        virtual void                GenerateAbscissa( svkDcmHeader* header, svkSpecPoint::UnitType type );
        virtual void                SetYRange( double lower, double upper );
        virtual svkXYPlotActor*     GetPlotActor();
        virtual vtkAxisActor2D*     GetRuler();
        virtual void                AddOnMouseMoveObserver( vtkRenderWindowInteractor* rwi);
        virtual void                RemoveOnMouseMoveObserver( vtkRenderWindowInteractor* rwi);
        virtual int                 GetPointIndexFromXValue( double xValue );
        virtual double              GetYValueFromIndex( int plotIndex, int pointIndex );
        virtual double              GetYValueFromXValue( int plotIndex, double xValue );
        virtual void                Refresh( );

    protected:

        svkDetailedPlotDirector();
        ~svkDetailedPlotDirector();

    
    private:

        void                           GenerateMagnitudeArray( vtkDataArray* complexArray, vtkDataArray* magnitudeArray);
        void                           RegenerateMagnitudeArrays();

        vtkstd::vector<vtkDataObject*> dataVector;
        svkXYPlotActor*                xyPlotActor;
        double                         xMin;
        double                         yMin;
        vtkDataArray*                  abscissa;
        int                            numPoints;
        vtkCallbackCommand*            cursorLocationCB;
        vtkCursor2D*                   cursor2D;
        vtkAxisActor2D*                ruler;
        //! A callback to update the plot lines when the data changes
        vtkCallbackCommand*            dataModifiedCB;

        static void UpdateData(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);
        static void UpdateCursorLocation( vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);

};


}   //svk


#endif //SVK_DETAILED_PLOT_DIRECTOR_H

