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



#include <svkDataView.h>


using namespace svk;


#define DEBUG 0


//vtkCxxRevisionMacro(svkDataView, "$Rev$");


//! Constructor
svkDataView::svkDataView()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    dataModifiedCallback = vtkCallbackCommand::New();
    this->orientation = svkDcmHeader::AXIAL;
    this->isValidationOn = true;
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


/*!
 * Removes the input.
 */
void svkDataView::RemoveInput(  int index ) 
{
    if( this->dataVector[index] != NULL ) {
        this->dataVector[index]->Delete();
        this->dataVector[index] = NULL;
    }
}


/*!
 * gets the input.
 */
svkImageData* svkDataView::GetInput(  int index ) 
{
    if( this->dataVector.size() > index ) {
        return this->dataVector[index];
    } else {
        return NULL;
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
int svkDataView::GetSlice( ) 
{
    return this->slice;
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
    this->isPropOn[propIndex] = true; 
    if ( propIndex < this->propCollection.size() ) {
        if (this->propCollection[propIndex] != NULL) { 
            this->propCollection[propIndex]->VisibilityOn();
        }
    }
}


//!
void svkDataView::TurnPropOff(int propIndex)
{
    this->isPropOn[propIndex] = false; 
    if ( propIndex < this->propCollection.size() ) {
        if (this->propCollection[propIndex] != NULL) { 
            this->propCollection[propIndex]->VisibilityOff();
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
    this->isRendererOn[rendererIndex] = true; 
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
    this->isRendererOn[rendererIndex] = false; 
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


/*!
 *
 */
void svkDataView::SetOrientation( svkDcmHeader::Orientation orientation ) 
{
    this->orientation = orientation;
}


/*!
 *
 */
svkDcmHeader::Orientation svkDataView::GetOrientation( )
{
    return this->orientation;
}

void svkDataView::GetClippingPlanes( vtkPlaneCollection* planes, svkImageData* data, int* tlcBrc, double clip_tolerance_row,
                                                                               double clip_tolerance_column,
                                                                               double clip_tolerance_slice ) 
{
    if( data != NULL && planes != NULL ) {
        vtkPlane* clipperPlane0 = vtkPlane::New();
        vtkPlane* clipperPlane1 = vtkPlane::New();
        vtkPlane* clipperPlane2 = vtkPlane::New();
        vtkPlane* clipperPlane3 = vtkPlane::New();
        vtkPlane* clipperPlane4 = vtkPlane::New();
        vtkPlane* clipperPlane5 = vtkPlane::New();

        int tlcBrcIndex[2][3]; 

        svkDataView::GetClippingIndexFromTlcBrc( data, tlcBrcIndex, tlcBrc);

        double rowNormal[3];
        data->GetDataBasis( rowNormal, svkImageData::ROW );
        double columnNormal[3];
        data->GetDataBasis( columnNormal, svkImageData::COLUMN );
        double sliceNormal[3];
        data->GetDataBasis( sliceNormal, svkImageData::SLICE );
        double LRNormal[3];
        data->GetDataBasis( LRNormal, svkImageData::LR );
        double PANormal[3];
        data->GetDataBasis( PANormal, svkImageData::PA );
        double SINormal[3];
        data->GetDataBasis( SINormal, svkImageData::SI );

        double* spacing = data->GetSpacing();

        double deltaLR = vtkMath::Dot( spacing, LRNormal );
        double deltaPA = vtkMath::Dot( spacing, PANormal );
        double deltaSI = vtkMath::Dot( spacing, SINormal );

        double* origin = data->GetOrigin();

        clipperPlane0->SetNormal( rowNormal[0], rowNormal[1], rowNormal[2] );
        clipperPlane0->SetOrigin( origin[0] + deltaLR*(tlcBrcIndex[0][0] - clip_tolerance_row),
                                  origin[1] + deltaPA*(tlcBrcIndex[0][0] - clip_tolerance_row), 
                                  origin[2] + deltaSI*(tlcBrcIndex[0][0] - clip_tolerance_row) );

        clipperPlane1->SetNormal( -rowNormal[0], -rowNormal[1], -rowNormal[2] );
        clipperPlane1->SetOrigin( origin[0] + deltaLR * (tlcBrcIndex[1][0] + 1 + clip_tolerance_row),
                                  origin[1] + deltaPA * (tlcBrcIndex[1][0] + 1 + clip_tolerance_row), 
                                  origin[2] + deltaSI * (tlcBrcIndex[1][0] + 1 + clip_tolerance_row) );


        clipperPlane2->SetNormal( columnNormal[0], columnNormal[1], columnNormal[2] );
        clipperPlane2->SetOrigin( origin[0] + deltaLR*(tlcBrcIndex[0][1] - clip_tolerance_column),
                                  origin[1] + deltaPA*(tlcBrcIndex[0][1] - clip_tolerance_column), 
                                  origin[2] + deltaSI*(tlcBrcIndex[0][1] - clip_tolerance_column) );

        clipperPlane3->SetNormal( -columnNormal[0], -columnNormal[1], -columnNormal[2] );
        clipperPlane3->SetOrigin( origin[0] + deltaLR*(tlcBrcIndex[1][1] + 1 + clip_tolerance_column),
                                  origin[1] + deltaPA*(tlcBrcIndex[1][1] + 1 + clip_tolerance_column), 
                                  origin[2] + deltaSI*(tlcBrcIndex[1][1] + 1 + clip_tolerance_column) );

        clipperPlane4->SetNormal( sliceNormal[0], sliceNormal[1], sliceNormal[2] );
        clipperPlane4->SetOrigin( origin[0] + deltaLR * ( tlcBrcIndex[0][2] - clip_tolerance_slice), 
                                  origin[1] + deltaPA * ( tlcBrcIndex[0][2] - clip_tolerance_slice), 
                                  origin[2] + deltaSI * ( tlcBrcIndex[0][2] - clip_tolerance_slice) );

        clipperPlane5->SetNormal( -sliceNormal[0], -sliceNormal[1], -sliceNormal[2] );
        clipperPlane5->SetOrigin( origin[0] + deltaLR * ( tlcBrcIndex[1][2] + 1 + clip_tolerance_slice ), 
                                  origin[1] + deltaPA * ( tlcBrcIndex[1][2] + 1 + clip_tolerance_slice ), 
                                  origin[2] + deltaSI * ( tlcBrcIndex[1][2] + 1 + clip_tolerance_slice ) );
    
        planes->AddItem( clipperPlane0 );
        planes->AddItem( clipperPlane1 );
        planes->AddItem( clipperPlane2 );
        planes->AddItem( clipperPlane3 );
        planes->AddItem( clipperPlane4 );
        planes->AddItem( clipperPlane5 );

        clipperPlane0->Delete();
        clipperPlane1->Delete();
        clipperPlane2->Delete();
        clipperPlane3->Delete();
        clipperPlane4->Delete();
        clipperPlane5->Delete();
        
    }


} 

/*!
 * Generates the clipping planes for the mMMapper. This is how the boundries
 * set are enforced, after the data is scaled, it is clipped so that data
 * outside the plot range is simply not shown.
 */
void svkDataView::ClipMapperToTlcBrc( svkImageData* data, vtkAbstractMapper* mapper, int* tlcBrc, double clip_tolerance_row, 
                                                                                       double clip_tolerance_column,
                                                                                       double clip_tolerance_slice )
{
    // We need to leave a little room around the edges, so the border does not get cut off
    if( data != NULL ) {
        vtkPlaneCollection* planes = vtkPlaneCollection::New();
        svkDataView::GetClippingPlanes( planes, data, tlcBrc, clip_tolerance_row, clip_tolerance_column, clip_tolerance_slice );
        mapper->RemoveAllClippingPlanes();
        vtkCollectionIterator* myIterator = vtkCollectionIterator::New();
        myIterator->SetCollection( planes );
        myIterator->InitTraversal();
        while( !myIterator->IsDoneWithTraversal() ) {
            mapper->AddClippingPlane( vtkPlane::SafeDownCast( myIterator->GetCurrentObject()) );
            myIterator->GoToNextItem();
        }
        myIterator->Delete();
        planes->Delete(); 
    } else {
        if( DEBUG ) {
            cout<<"INPUT HAS NOT BEEN SET!!"<<endl;
        }
    }
}


/*!
 *  Get the  index that should be clipped based on the tlcBrc
 */
void svkDataView::GetClippingIndexFromTlcBrc( svkImageData* data, int indexRange[2][3], int tlcBrc[2] )
{
    double* spacing = data->GetSpacing();
    double* origin = data->GetOrigin();
    int* extent = data->GetExtent();
    int tlcIndex[3];
    int brcIndex[3];
    data->GetIndexFromID( tlcBrc[0], tlcIndex );
    data->GetIndexFromID( tlcBrc[1], brcIndex );
    indexRange[0][0] = tlcIndex[0];
    indexRange[0][1] = tlcIndex[1];
    indexRange[0][2] = tlcIndex[2];
    indexRange[1][0] = brcIndex[0];
    indexRange[1][1] = brcIndex[1];
    indexRange[1][2] = brcIndex[2];
}


/*!
 *
 */
bool svkDataView::IsTlcBrcWithinData( svkImageData* data, int tlcBrc[2])
{
    return svkDataView::IsTlcBrcWithinData( data, tlcBrc[0], tlcBrc[1] );
}


/*!
 *
 */
bool svkDataView::IsTlcBrcWithinData( svkImageData* data, int tlcID, int brcID)
{
    bool isWithinData = false; 
    if( data != NULL ) {
        int* extent = data->GetExtent();
        int maxIndex[3] = {data->GetDimensions()[0]-2,data->GetDimensions()[1]-2,data->GetDimensions()[2]-2};
        int maxID = data->GetIDFromIndex( maxIndex[0], maxIndex[1], maxIndex[2] );
        int rowRange[2]    = {0,0};
        int columnRange[2] = {0,0};
        int sliceRange[2]  = {0,0};

        // Get indecies from the tlcBrc
        data->GetIndexFromID( tlcID, &rowRange[0], &columnRange[0], &sliceRange[0] );
        data->GetIndexFromID( brcID, &rowRange[1], &columnRange[1], &sliceRange[1] );

        // Fist we make sure that the indecies are greater then zero and within the range of a single slice
        if( tlcID >= 0 && brcID >= 0 && tlcID <= maxID && brcID <= maxID && tlcID <= brcID 
            && rowRange[0] <= rowRange[1] && columnRange[0] <= columnRange[1] && sliceRange[0] <= sliceRange[1] ) {
            isWithinData = true; 
        }
    }
    return isWithinData;
}


/*!
 *
 */
void svkDataView::ResetTlcBrcForNewOrientation( svkImageData* data, svkDcmHeader::Orientation orientation, int tlcBrc[2], int &slice)
{
    if( svkDataView::IsTlcBrcWithinData( data, tlcBrc ) ) {
        int tlcIndex[3];
        int brcIndex[3];
        data->GetIndexFromID( tlcBrc[0], tlcIndex );
        data->GetIndexFromID( tlcBrc[1], brcIndex );
        int orientationIndex = data->GetOrientationIndex( orientation );

        slice = tlcIndex[orientationIndex] + (brcIndex[orientationIndex] - tlcIndex[orientationIndex])/2;
        brcIndex[orientationIndex] = slice;
        tlcIndex[orientationIndex] = slice;
        tlcBrc[0] = data->GetIDFromIndex( tlcIndex[0], tlcIndex[1], tlcIndex[2] );
        tlcBrc[1] = data->GetIDFromIndex( brcIndex[0], brcIndex[1], brcIndex[2] );
    } 
}


/*
 *
 */
void svkDataView::ValidationOff()
{
    this->isValidationOn = false;
}

