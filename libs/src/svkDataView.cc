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



#include <svkDataView.h>


using namespace svk;


#define DEBUG 0


vtkCxxRevisionMacro(svkDataView, "$Rev$");


//! Constructor
svkDataView::svkDataView()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    dataModifiedCallback = vtkCallbackCommand::New();
}


//! Destructor
svkDataView::~svkDataView()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
    for( vector<vtkRenderer*>::iterator iter = this->renCollection.begin();
        iter != this->renCollection.end(); ++iter) {
        if( (*iter) != NULL ) {
            (*iter)->Delete();
            (*iter) = NULL;
        }
    }

    for( vector<vtkProp*>::iterator iter = this->propCollection.begin();
        iter != this->propCollection.end(); ++iter) {
        if( (*iter) != NULL ) {
            (*iter)->Delete();
            (*iter) = NULL;
        }
    }
    if( dataModifiedCallback != NULL ) {
        dataModifiedCallback->Delete();
        dataModifiedCallback = NULL;
    }
    
    // We must ensure that the data are deleted using vtk::Delete()
    for( vector<svkImageData*>::iterator iter = dataVector.begin();
        iter != dataVector.end(); ++iter) {
        if( (*iter) != NULL ) {
            (*iter)->Delete();
            (*iter) = NULL;
        }
    }

}

/*
 *
 */
void svkDataView::SetController( svkDataViewController* controller )
{
    this->controller = controller; 
}


/*!
 *
 */
void svkDataView::SetRWInteractor(vtkRenderWindowInteractor* rwi)
{
    this->rwi = rwi;
}


/*!
 *       
 */
void svkDataView::SetWindowLevelRange( double lower, double upper, int index)
{
}


/*!
 *
 */
void svkDataView::ObserveData( svkImageData* data )
{
    dataModifiedCallback->SetCallback( UpdateView );
    dataModifiedCallback->SetClientData( (void*)this );
    data->AddObserver(vtkCommand::ModifiedEvent, dataModifiedCallback);
}


/*! 
 *  Observer callback must be a static member:
 *  Note that vtkObject* is a pointer to the subject being observed.
 *  Here the clientData contains a pointer to the observer object
 */
void svkDataView::UpdateView(vtkObject* subject, unsigned long, void* thisObject, void*)
{
    
    if (DEBUG) {
        cout <<"svkDataView::UpdateView()" << endl;
        cout <<"svkDataView OBSERVED Data Modified Event" << endl;
    }

    static_cast<svkDataView*>(thisObject)->Refresh();
    // Also forward the modified event along 
    static_cast<svkDataView*>(thisObject)->InvokeEvent(vtkCommand::ModifiedEvent);
}


/*!
 *
 */
svkDataViewController* svkDataView::GetController()
{
    return controller;
}


/*!
 *
 */
void svkDataView::Refresh()
{
    vtkDebugMacro(<<"svkDataView::Refresh()()");
    vtkDebugMacro( << this->GetClassName() << "::Refresh()" );

    this->InvokeEvent(vtkCommand::ModifiedEvent);
    if ( this->rwi != NULL ) {
        this->rwi->GetRenderWindow()->Render();
        this->rwi->InvokeEvent(vtkCommand::RenderEvent);
    }
}


//!
void svkDataView::SetPropState(int propIndex, bool visible)
{
    //first Check to see if it exists!
    this->isPropOn[propIndex] = visible; 
}


//!
void svkDataView::TurnPropOn(int propIndex)
{
    this->isPropOn[propIndex] = TRUE; 
    if ( propIndex < this->propCollection.size() ) {
        if (this->propCollection[propIndex] != NULL) { 
            this->propCollection[propIndex]->VisibilityOn();
            // We need to force a render after the change-- not sure why
            this->propCollection[propIndex]->Modified();
            for( vector<vtkRenderer*>::iterator iter = this->renCollection.begin();
                iter != this->renCollection.end(); ++iter) {
                if( (*iter) != NULL ) {
                    (*iter)->Render();
                    (*iter)->Modified();
                }
            }
 
        }
    }
}


//!
void svkDataView::TurnPropOff(int propIndex)
{
    this->isPropOn[propIndex] = FALSE; 
    if ( propIndex < this->propCollection.size() ) {
        if (this->propCollection[propIndex] != NULL) { 
            this->propCollection[propIndex]->VisibilityOff();
            // We need to force a render after the change-- not sure why
            this->propCollection[propIndex]->Modified();
            for( vector<vtkRenderer*>::iterator iter = this->renCollection.begin();
                iter != this->renCollection.end(); ++iter) {
                if( (*iter) != NULL ) {
                    (*iter)->Render();
                    (*iter)->Modified();
                }
            }
        }
    }
}


//!
bool svkDataView::IsPropOn(int propIndex)
{
    return this->isPropOn[propIndex];
}


//!
void svkDataView::SetRendererState(int rendererIndex, bool visible)
{
    this->isRendererOn[rendererIndex] = visible; 
}


//!
void svkDataView::TurnRendererOn(int rendererIndex)
{
    this->isRendererOn[rendererIndex] = TRUE; 
    if( this->rwi != NULL && !(this->rwi->GetRenderWindow()->HasRenderer(this->renCollection[rendererIndex])) ) {
        if( this->rwi->GetRenderWindow()->GetNumberOfLayers()  < this->renCollection[rendererIndex]->GetLayer()+1 ) {
            this->rwi->GetRenderWindow()->SetNumberOfLayers( this->renCollection[rendererIndex]->GetLayer()+1);
        }
        this->rwi->GetRenderWindow()->AddRenderer( this->renCollection[rendererIndex]);
        
    } 
}


//!
void svkDataView::TurnRendererOff(int rendererIndex)
{
    this->isRendererOn[rendererIndex] = FALSE; 
    if( this->rwi != NULL && this->rwi->GetRenderWindow()->HasRenderer(this->renCollection[rendererIndex]) ) {
        this->rwi->GetRenderWindow()->RemoveRenderer( this->renCollection[rendererIndex]);
    } 
}


//!
bool svkDataView::IsRendererOn(int rendererIndex)
{
    return this->isRendererOn[rendererIndex];
}

//! Is it in the views current displayed FOV?
void svkDataView::SetVisibility(int propIndex, bool visible)
{
    this->isPropVisible[propIndex] = visible; 
}


//!
vtkRenderer* svkDataView::GetRenderer(int index)
{
    return this->renCollection[index]; 
}


//!
void svkDataView::SetRenderer(int index, vtkRenderer* ren)
{
    if( this->renCollection[index] != NULL ) {
        this->renCollection[index]->Delete();
        this->renCollection[index] = NULL;
    }
    if( ren != NULL ) {
        ren->Register(this);
    }
    this->renCollection[index] = ren; 
}


//! 
vtkProp* svkDataView::GetProp(int index)
{
    return this->propCollection[index]; 
}


//!
void svkDataView::SetProp(int index, vtkProp* prop)
{
    if( this->propCollection[index] != NULL ) {
        this->propCollection[index]->Delete();
        this->propCollection[index] = NULL;
    }
    if( prop != NULL ) {
        prop->Register(this);
    }
    this->propCollection[index] = prop; 
}
