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


#ifndef SVK_DETAILED_PLOT_VIEW_CONTROLLER_H
#define SVK_DETAILED_PLOT_VIEW_CONTROLLER_H


#include <svkDetailedPlotView.h>
#include <vtkObjectFactory.h>


namespace svk {


using namespace std;


// Note forward declaration to avoid self refererncing includes. 
class svkDetailedPlotView; 

/*!
 *  This is the controller class that represents the plots of the 4D data.
 */
class svkDetailedPlotViewController : public svkDataViewController 
{

    public:
        // vtk type revision macro
        vtkTypeRevisionMacro( svkDetailedPlotViewController, svkDataViewController );

        static svkDetailedPlotViewController*       New();

        svkDetailedPlotViewController();
        ~svkDetailedPlotViewController();

        //  Methods:
        virtual void                    SetInput(svkImageData* data, int index = 0);
        virtual void                    SetSlice(int slice);
        virtual int*                    GetTlcBrc();
        virtual void                    SetTlcBrc( int* tlcBrc );
        virtual void                    SetWindowLevelRange( double lower, double upper, int index);
                void                    SetComponent( svkBoxPlot::PlotComponent component);
                int                     GetComponent( );
        virtual void                    SetRWInteractor( vtkRenderWindowInteractor* rwi );
        virtual void                    TurnPropOn(int propIndex);
        virtual void                    TurnPropOff(int propIndex);
                void                    Reset();
                void                    AddPlot( int index, int component, int channel=0, int timePoint=0 );
                void                    SetUnits( int units);
                void                    Update();

    protected:

        vtkRenderWindowInteractor*      rwi;
        int                             slice;

    private:


};


}   //svk


#endif //SVK_DETAILED_PLOT_VIEW_CONTROLLER_H

