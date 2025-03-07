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


#include <svkDataViewController.h>


using namespace svk;


//vtkCxxRevisionMacro(svkDataViewController, "$Rev$");

//! Constructor
svkDataViewController::svkDataViewController()
{
}

//! Destructor
svkDataViewController::~svkDataViewController()
{

    // We must ensure that the data are deleted using vtk::Delete()
    for( vector<svkImageData*>::iterator iter = dataVector.begin();
        iter != dataVector.end(); ++iter) {
        if( (*iter) != NULL ) {
            (*iter)->Delete();
        }
    }

}

//! Set input data and initialize default range values.
void svkDataViewController::SetView( svkDataView* view )
{
    if( this->view != NULL ) {
        this->view->Delete();
    }
    this->view = view;
    this->view->Register(this);
}

//! 
svkDataView* svkDataViewController::GetView()
{
    return this->view;
}


/*!  
 *  
 */
void svkDataViewController::SetSlice(int slice)
{
    this->view->SetSlice(slice);
}


/*!
 *
 */
int svkDataViewController::GetSlice()
{
    return this->view->GetSlice();
}

/*!
 *
 */
void svkDataViewController::SetRWInteractor(vtkRenderWindowInteractor* rwi)
{
    this->rwi = rwi;
    this->view->SetRWInteractor(rwi);
}


/*!
 *
 */
vtkRenderWindowInteractor* svkDataViewController::GetRWInteractor()
{
    return this->rwi;
}


/*!
 *
 */
svkImageData* svkDataViewController::GetData( int index )
{
    return dataVector[ index ];
}


/*!
 *  Method for setting viewer's window level range on a particular coordinate, e.g.
 *  intensity for images, or freq (index0) and intensity (index1) for spectra, etc.
 */
void  svkDataViewController::SetWindowLevelRange( double lower, double upper, int index)
{
}
