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


#include <svkDetailedPlotViewController.h>


using namespace svk;


vtkCxxRevisionMacro(svkDetailedPlotViewController, "$Rev$");
vtkStandardNewMacro(svkDetailedPlotViewController);


//! Constructor 
svkDetailedPlotViewController::svkDetailedPlotViewController()
{
    this->dataVector.push_back(NULL);
    this->view = svkDetailedPlotView::New();
    this->view->SetController(this);
    this->slice = 0;
    this->rwi = NULL;
}


//! Destructor
svkDetailedPlotViewController::~svkDetailedPlotViewController()
{
    if (this->rwi != NULL) {
        this->rwi->Delete();
        this->rwi = NULL;
    }

    if( this->view != NULL ) {
        this->view->Delete();
        this->view = NULL;
    }

    // NOTE: Data is destroyed in the superclass
}


/*!
 *
 */
void svkDetailedPlotViewController::Reset()
{

}


//! Set input data and initialize default range values.
void svkDetailedPlotViewController::SetInput(svkImageData* data, int index)
{
    if( index == 0 ) {
        if( dataVector[index] != NULL ) {
            (dataVector[index])->Delete();
        }
        data->Register(this);
        dataVector[index] = data;
        this->view->SetInput(data, index);
    } else {
        cout<<"WARNING: svkDetailedPlotViewController only takes one image input!"<<endl;
    }
}


/*!  cast base class var type to specific sub-class in the implementation.  Or... declare
 *   view member variable to be of type svkDetailedPlotView*, overriding the base class type.
 */ 
void svkDetailedPlotViewController::SetSlice(int slice)
{
    this->slice = slice;
    this->view->SetSlice(slice);
}


//! Net yet implemented, may not apply at all 
int* svkDetailedPlotViewController::GetTlcBrc()
{
    return NULL;
}


//! Sets the current top left/bottm right corners of the plotGrid
void svkDetailedPlotViewController::SetTlcBrc( int* tlcBrc )
{
    int* currentTlcBrc = GetTlcBrc();
    if( tlcBrc != NULL ) {
        if( currentTlcBrc != NULL ) {
            if( tlcBrc[0] != currentTlcBrc[0] || tlcBrc[1] != currentTlcBrc[1] ) {
               static_cast<svkDetailedPlotView*>(this->view)->SetTlcBrc( tlcBrc[0], tlcBrc[1] );
            }
        } else {
           static_cast<svkDetailedPlotView*>(this->view)->SetTlcBrc( tlcBrc[0], tlcBrc[1] );
        }
    }

}


/*!
 *  SetWindowLevel for spectral view;  index 0 is frequency, index 1 is intensity
 */
void svkDetailedPlotViewController::SetWindowLevelRange( double lower, double upper, int index)
{
    this->view->SetWindowLevelRange(lower, upper, index);
}


/*!
 *
 */
void svkDetailedPlotViewController::SetComponent( svkBoxPlot::PlotComponent component)
{
    (static_cast<svkDetailedPlotView*>(this->view))->SetComponent( component );
}


//! Set the RenderWindowInteractor and setup the drag selector.
void svkDetailedPlotViewController::SetRWInteractor( vtkRenderWindowInteractor* rwi )
{
    if( rwi != NULL ) {
        if( this->rwi != NULL ) {
            this->rwi->Delete();
        } 
        this->rwi = rwi;
        this->rwi->Register( this );
        // Now we must remove all renderers, in case the window already had renderers in it
        vtkRendererCollection* renderers = this->rwi->GetRenderWindow()->GetRenderers();
        vtkCollectionIterator* myIterator = vtkCollectionIterator::New();
        myIterator->SetCollection( renderers );
        myIterator->InitTraversal();
        while( !myIterator->IsDoneWithTraversal() ) {
            this->rwi->GetRenderWindow()->RemoveRenderer( static_cast<vtkRenderer*>(myIterator->GetCurrentObject()) );
            myIterator->GoToNextItem();
        }
        myIterator->Delete();

        // If we wanted another type of interactor...
        rwi->GetRenderWindow()->SetNumberOfLayers(1);
        this->view->SetRWInteractor( rwi );
    }
}


/*!
 *
 */
void svkDetailedPlotViewController::TurnPropOn(int propIndex)
{
    view->TurnPropOn( propIndex );
    view->Refresh();
}


/*!
 *
 */
void svkDetailedPlotViewController::TurnPropOff(int propIndex)
{
    view->TurnPropOff( propIndex);
    view->Refresh();
}


/*!
 *   This method adds a plot to the detailed view given the index of
 *   the array, and the component you wish to plot.
 */
void svkDetailedPlotViewController::AddPlot( int index, int component, int channel )
{
    static_cast<svkDetailedPlotView*>(this->GetView())->AddPlot( index, component, channel ); 
}


/*!
 *
 */
void svkDetailedPlotViewController::SetUnits( int units)
{
    static_cast<svkDetailedPlotView*>(this->GetView())->SetUnits( units ); 
}


/*!
 *
 */
void svkDetailedPlotViewController::Update()
{
    static_cast<svkDetailedPlotView*>(this->GetView())->Update( ); 
}
