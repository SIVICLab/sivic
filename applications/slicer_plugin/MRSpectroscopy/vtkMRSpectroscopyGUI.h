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
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/libs/src/svkDdfVolumeReader.cc $
 *  $Rev: 76 $
 *  $Author: jccrane $
 *  $Date: 2010-01-26 11:05:15 -0800 (Tue, 26 Jan 2010) $
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 *      Shawn Krisman
 */


/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
==========================================================================*/


#ifndef __vtkMRSpectroscopyGUI_h
#define __vtkMRSpectroscopyGUI_h

#ifdef WIN32
#include "vtkMRSpectroscopyWin32Header.h"
#endif

#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorObserver.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerInteractorStyle.h"
#include <vtkKWScale.h>
#include <vtkKWRange.h>

#include <svkPlotGridViewController.h>
#include <svkImageData.h>

#include "vtkMRSpectroscopyLogic.h"

using namespace svk; 

class vtkKWPushButton;
class vtkKWRange;
class vtkKWRadioButton;
class vtkSlicerNodeSelectorWidget;
class vtkInteractorObserver;

class VTK_MRSpectroscopy_EXPORT vtkMRSpectroscopyGUI : public vtkSlicerModuleGUI
{

    public:

        vtkTypeRevisionMacro ( vtkMRSpectroscopyGUI, vtkSlicerModuleGUI );

        //----------------------------------------------------------------
        // Set/Get Methods
        //----------------------------------------------------------------
        
        vtkGetObjectMacro ( Logic, vtkMRSpectroscopyLogic );
        void SetModuleLogic ( vtkSlicerLogic *logic )  { 
            this->SetLogic ( vtkObjectPointer (&this->Logic), logic );
        }
    

    protected:

        //----------------------------------------------------------------
        // Constructor / Destructor (proctected/private) 
        //----------------------------------------------------------------
        
        vtkMRSpectroscopyGUI ( );
        virtual ~vtkMRSpectroscopyGUI ( );


    private:

        vtkMRSpectroscopyGUI ( const vtkMRSpectroscopyGUI& ); // Not implemented.
        void operator = ( const vtkMRSpectroscopyGUI& ); //Not implemented.


    public:
        //----------------------------------------------------------------
        // New method, Initialization etc.
        //----------------------------------------------------------------

        static vtkMRSpectroscopyGUI* New ();

        void Init();
        virtual void Enter ( );
        virtual void Exit ( );
        virtual void PrintSelf (ostream& os, vtkIndent indent );

        //----------------------------------------------------------------
        // Observer Management
        //----------------------------------------------------------------
        
        virtual void AddGUIObservers ( );
        virtual void RemoveGUIObservers ( );
        virtual void AddLogicObservers ( );
        virtual void RemoveLogicObservers ( );
        
        //----------------------------------------------------------------
        // Event Handlers
        //----------------------------------------------------------------
        
        virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
        virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
        virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
        
        //----------------------------------------------------------------
        // Build Frames
        //----------------------------------------------------------------

        virtual void BuildGUI ( );

        //----------------------------------------------------------------
        // Update routines
        //----------------------------------------------------------------
        
        void UpdateAll();

        
    protected:
        void BuildGUIForHelpFrame();
        void BuildGUIForTestFrame1();
        void BuildGUIForTestFrame2();

        void ProcessTimerEvents();
        void HandleMouseEvent(vtkSlicerInteractorStyle *style);
        static void DataCallback(vtkObject *caller, 
                           unsigned long eid, void *clientData, void *callData);
  
        //----------------------------------------------------------------
        // Timer
        //----------------------------------------------------------------
        int TimerFlag;
        int TimerInterval;
    
        //----------------------------------------------------------------
        // GUI widgets
        //----------------------------------------------------------------
    
        vtkSlicerNodeSelectorWidget*    VolumeSelector;
        vtkKWPushButton*                TestButton11;
        vtkKWPushButton*                TestButton12;
        vtkKWPushButton*                TestButton21;
        vtkKWPushButton*                TestButton22;
        vtkKWScale*                     RedScale;
        vtkKWScale*                     YellowScale;
        vtkKWScale*                     GreenScale;
        vtkPolyData*                    Grids[3];
        svkPlotGridViewController*      PlotView;
        svkImageData*                   CurrentSpectra;
        vtkKWScale*                     SpectraSlider;
        vtkKWRange*                     xSpecRange;
        vtkKWRange*                     ySpecRange;

    
        //----------------------------------------------------------------
        // Logic Values
        //----------------------------------------------------------------
    
        vtkMRSpectroscopyLogic*         Logic;
        vtkCallbackCommand*             DataCallbackCommand;
        int                             CloseScene;

        vtkInteractorObserver*          SlicerStyle;
        vtkRenderWindowInteractor*      Interactor;
        vtkInteractorObserver*          Style;
        int                             HasStyle;
        void                            SplitWindow();
        void                            LoadSpectra();
        void                            DoOtherThing();
        void                            ResetStyle();
        void                            RenderActor(vtkProp* actor);
        void                            UpdateGridScalars();
        void                            RefreshSliceWindows();
        int                             GetAxialSlice();
        void                            AddGridObserver();
};



#endif
