/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: https://intrarad.ucsf.edu/svn/rad_software/surbeck/brain/sivic/trunk/applications/slicer_plugin/MRSpectroscopy/vtkMRSpectroscopyGUI.h $
  Date:      $Date: 2009-09-28 22:20:25 -0700 (Mon, 28 Sep 2009) $
  Version:   $Revision: 15111 $

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
