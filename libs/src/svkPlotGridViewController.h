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


#ifndef SVK_PLOT_GRID_VIEW_CONTROLLER_H
#define SVK_PLOT_GRID_VIEW_CONTROLLER_H


#include </usr/include/vtk/vtkInteractorStyleRubberBand2D.h>
#include </usr/include/vtk/vtkInteractorStyleTrackballCamera.h>
#include </usr/include/vtk/vtkCallbackCommand.h>
#include </usr/include/vtk/vtkCommand.h>
#include </usr/include/vtk/vtkRenderWindowInteractor.h>
#include </usr/include/vtk/vtkCoordinate.h>
#include </usr/include/vtk/vtkAreaPicker.h>
#include </usr/include/vtk/vtkObjectFactory.h>

#include <svkSpecGridSelector.h>
#include <svkPlotGridView.h>
#include <svkPlotLine.h>
#include <svkLookupTable.h>


namespace svk {


using namespace std;


// Note forward declaration to avoid self refererncing includes. 
class svkPlotGridView; 

/*!
 * This is the controller class that represents the plots of the 4D data.
 */
class svkPlotGridViewController : public svkDataViewController 
{

    public:
        // vtk type revision macro
        vtkTypeMacro( svkPlotGridViewController, svkDataViewController );

        static svkPlotGridViewController*       New();

        svkPlotGridViewController();
        ~svkPlotGridViewController();

        //  Methods:
        virtual void                    SetInput(svkImageData* data, int index = 0);
        virtual void                    SetSlice(int slice);
        virtual int*                    GetTlcBrc();
        virtual void                    SetTlcBrc( int* tlcBrc );
        virtual void                    SetWindowLevelRange( double lower, double upper, int index);
        virtual void                    GetWindowLevelRange( double &lower, double &upper, int index);
                void                    SetComponent( svkPlotLine::PlotComponent component);
                int                     GetComponent( );
        virtual void                    SetRWInteractor( vtkRenderWindowInteractor* rwi );
        virtual void                    TurnPropOn(int propIndex);
        virtual void                    TurnPropOff(int propIndex);
                void                    HighlightSelectionVoxels();
                void                    Reset();
                void                    SetColorSchema( int colorSchema );
                string                  GetDataCompatibility( svkImageData* data, int targetIndex );
        virtual void                    SetVolumeIndex( int index, int volumeIndex = 0 );
        virtual int                     GetVolumeIndex( int volumeIndex = 0 );
        virtual int*                    GetVolumeIndexArray( );
                void                    SetOverlayOpacity( double opacity );
                void                    SetOverlayThreshold( double threshold );
                void                    SetLUT( svkLookupTable::svkLookupTableType type );


    protected:

        svkSpecGridSelector*            dragSelect;
        vtkCallbackCommand*             dragSelectionCB;
        static void UpdateSelection(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);

    private:


};


}   //svk

#endif //SVK_PLOT_GRID_VIEW_CONTROLLER_H

