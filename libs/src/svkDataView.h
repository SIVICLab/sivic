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


#ifndef SVK_DATA_VIEW_H
#define SVK_DATA_VIEW_H


#include </usr/include/vtk/vtkRenderWindowInteractor.h>
#include </usr/include/vtk/vtkRendererCollection.h>
#include </usr/include/vtk/vtkCallbackCommand.h>
#include </usr/include/vtk/vtkObject.h>

#include <svkImageData.h>
#include </usr/include/vtk/vtkPlane.h>
#include </usr/include/vtk/vtkPlaneCollection.h>
#include </usr/include/vtk/vtkCollectionIterator.h>
#include </usr/include/vtk/vtkAbstractMapper.h>
#include <svkDataViewController.h>

#include <vector>
#ifdef WIN32
    #include <windows.h>
#endif
namespace svk {


using namespace std;


// Note forward declaration to avoid self refererncing includes. 
class svkDataViewController; 


class svkDataView : public vtkObject 
{
    friend class svkDataViewController;

    // if these are accessed only via the corresponding controller, then these don't need to be public
    public:

        vtkTypeMacro( svkDataView, vtkObject);

        svkDataView();
        ~svkDataView();

        //  Methods
        // for DataViews with multiple data objects (e.g. reference view has image and spec data) 
        virtual void                    SetInput( svkImageData* data, int index = 0 ) = 0;

        virtual svkImageData*           GetInput( int index = 0 );
        //! Sets input data set to NULL for index and call Delete
        virtual void                    RemoveInput(  int index = 0 );

        //  probably set this in an "Init" method (no constructor for abstract classes).  Also, 
        //  doesn't need to be virtual or abstract, just
        //  implement this setting in the base class. 
        virtual void                    SetController( svkDataViewController* controller );

        // implement in base class - not virtual
        virtual svkDataViewController*  GetController();

        virtual void                    SetSlice( int slice ) = 0;
        virtual int                     GetSlice( );
        virtual void                    SetWindowLevelRange( double lower, double upper, int index );

        // intended to represent setting of real world FOV of data in renderer
        virtual void                    Refresh();

        /* 
         * Note (promote to main class docs after cleaning up)  
         * that each view is placed within it's own vtkRenderWindow.  The renderWindow may be 
         * obtained from the rwi so the getRendererCollection isn't necessary.  So, it then 
         * becomes the responsibility of the DataView to place it's renderers within the 
         * renderWindow obtained from the rwi. 
         *      Q:  how to prevent a renderwindow (or rwi) from being passed to multiple DataViews?
         */
        virtual void                    SetRWInteractor( vtkRenderWindowInteractor* rwi );

        /*! View may have more than one svkImageData it's rendering, so may 
         *  specify specific ImageIndex
         */

        // Are they toggled on or off
        virtual void            SetPropState(int propIndex, bool visible);
        virtual void            TurnPropOn(int propIndex);
        virtual void            TurnPropOff(int propIndex);
        virtual bool            IsPropOn(int propIndex);

        virtual void            SetRendererState(int rendererIndex, bool visible);
        virtual void            TurnRendererOn(int rendererIndex);
        virtual void            TurnRendererOff(int rendererIndex);
        virtual bool            IsRendererOn(int rendererIndex);

        // Is it in the views current displayed FOV? 
        virtual void            SetVisibility(int actorIndex, bool visible);

        virtual vtkRenderer*    GetRenderer(int index);
        virtual void            SetRenderer(int index, vtkRenderer* ren);
        virtual vtkProp*        GetProp(int index);
        virtual void            SetProp(int index, vtkProp* prop);

        virtual void            SetOrientation( svkDcmHeader::Orientation orientation );
        virtual svkDcmHeader::Orientation  GetOrientation( );
        static void             ClipMapperToTlcBrc( svkImageData* data, vtkAbstractMapper* mapper, int tlcBrc[2], double clip_tolerance_row , double clip_tolerance_column, double clip_tolerance_slice);
        static void             GetClippingIndexFromTlcBrc( svkImageData* data, int indexRange[2][3], int tlcBrc[2] );
        static void             GetClippingPlanes( vtkPlaneCollection* planes, svkImageData* data, int* tlcBrc, double clip_tolerance_row, double clip_tolerance_column, double clip_tolerance_slice );
        static bool             IsTlcBrcWithinData(svkImageData* data, int tlcBrc[2]);
        static bool             IsTlcBrcWithinData( svkImageData* data, int tlcID, int brcID);
        static void             ResetTlcBrcForNewOrientation( svkImageData* data, svkDcmHeader::Orientation orientation, int tlcBrc[2], int &slice);

        void                    ValidationOff();


    protected:

        //  Members:
        vector < vtkRenderer* >         renCollection;
        vector < vtkProp* >             propCollection;
        vtkRenderWindowInteractor*      rwi;
        vector <svkImageData*>          dataVector;
        svkDataViewController*          controller;  
        vtkCallbackCommand*             dataModifiedCallback;
        svkDcmHeader::Orientation       orientation;

        //! the top left, bottom right corners of the current view 
        int                             tlcBrc[2];
        int                             slice;

        vector < bool >                 isPropOn;          //Is the actor turned on or off?
        vector < bool >                 isRendererOn;       //Is the renderer turned on or off?
        vector < bool >                 isPropVisible;     //Is the actor in the views FOV?
        bool                            isValidationOn; 

        /*!
         *  Method to to get all actors from each svkImageData
         *  and update the view with it's current' properties/state, etc.
         *  via svkImageData::GetAllActorCollectins();  Probably called by 
         *  the dataModifiedCallback whenevera data modified event is caught. 
         *  Stub to be implemented later.
         */
        virtual void                    UpdateProps() {;};
              
        //  Methods:
        void                            ObserveData( svkImageData* data );
        static void                     UpdateView(
                                            vtkObject* subject, 
                                            unsigned long eid, 
                                            void* thisObject, 
                                            void *callData
                                        );

};


}   //svk


#endif //SVK_DATA_VIEW_H

