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


#ifndef SVK_DATA_VIEW_CONTROLLER_H
#define SVK_DATA_VIEW_CONTROLLER_H


#include <svkImageData.h>
#include <svkDataView.h>
#include </usr/include/vtk/vtkRenderWindow.h>
#include </usr/include/vtk/vtkRenderWindowInteractor.h>
#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkCallbackCommand.h>
#include <vector>


namespace svk {


using namespace std;


// forward declaration:
class svkDataView;


// Note forward declaration to avoid self refererncing includes.


/*!  
 *  Set slice, etc operations affecting view are implemented in the controller
 */
class svkDataViewController: public vtkObject 
{


    //  Check access specifiers and whether some of these can be concrete 
    //  implementations in base class.  See if DataView setup can be automated 
    //  via controller methods rather than requiring application layer to deal with it. 
    //  validate inputs and initialization order... simplify life for application layer. 
    public:

        vtkTypeMacro( svkDataViewController, vtkObject);
        
        svkDataViewController();
        ~svkDataViewController();

        //  Methods
        virtual void                       SetInput( svkImageData* data, int index = 0 ) =  0;
        virtual svkImageData*              GetData( int index = 0);
        virtual void                       SetView( svkDataView* view );
        virtual svkDataView*               GetView();
        virtual void                       SetSlice(int slice);
        virtual int                        GetSlice();
        // Method for setting visible data ranges, e.g. spectral view intensity, freq range:
        virtual void                       SetWindowLevelRange( double lower, double upper, int index );
        virtual void                       SetRWInteractor( vtkRenderWindowInteractor* rwi );
        virtual vtkRenderWindowInteractor* GetRWInteractor( );

    protected:

        //  Members:
        vector <svkImageData*>          dataVector;
        svkDataView*                    view;
        vtkRenderWindowInteractor*      rwi;


};


}   //svk   


#endif //SVK_DATA_VIEW_CONTROLLER_H

