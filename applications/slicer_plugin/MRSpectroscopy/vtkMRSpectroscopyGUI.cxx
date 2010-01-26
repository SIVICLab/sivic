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

#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkMRSpectroscopyGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerSlicesGUI.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkActor2D.h"
#include "vtkActor2DCollection.h"
#include "vtkProperty2D.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkTextActor.h"
#include "vtkMatrix4x4.h"
#include "vtkCubeSource.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkMRMLSliceNode.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkDataSetAttributes.h"
#include "vtkUnsignedCharArray.h"

#include "vtkSlicerColor.h"
#include "vtkSlicerTheme.h"

#include "vtkKWTkUtilities.h"
#include "vtkKWWidget.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWEvent.h"
#include "vtkKWScale.h"
#include <vtkKWRange.h>

#include "vtkKWPushButton.h"

#include "vtkCornerAnnotation.h"

#include "svkDataModel.h"
#include "svkImageData.h"
#include "svkImageTopologyGenerator.h"
#include "svkPlotGridViewController.h"
#include "svkSpecPoint.h"

#include "cmath"
#include "set"

#define PPM_DEFAULT_MIN 3.844 
#define PPM_DEFAULT_MAX 0.602 
#define NEG_RANGE_SCALE .17
#define POS_RANGE_SCALE .70
#define SLIDER_RELATIVE_RESOLUTION 0.002



//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRSpectroscopyGUI );
vtkCxxRevisionMacro ( vtkMRSpectroscopyGUI, "$Revision: 15111 $");
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
vtkMRSpectroscopyGUI::vtkMRSpectroscopyGUI ( )
{

    //----------------------------------------------------------------
    // Logic values
    //----------------------------------------------------------------
    this->Logic = NULL;
    this->DataCallbackCommand = vtkCallbackCommand::New();
    this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
    this->DataCallbackCommand->SetCallback(vtkMRSpectroscopyGUI::DataCallback);
    this->Interactor = NULL;
    this->Style = NULL;
    this->HasStyle = 0;

    //----------------------------------------------------------------
    // GUI widgets
    //----------------------------------------------------------------
    this->VolumeSelector = NULL;
    this->TestButton11   = NULL;
    this->TestButton12   = NULL;
    this->TestButton21   = NULL;
    this->TestButton22   = NULL;
    this->SpectraSlider  = NULL;
    this->xSpecRange     = NULL;
    this->ySpecRange     = NULL;
    
    //----------------------------------------------------------------
    // Locator  (MRML)
    //----------------------------------------------------------------
    this->TimerFlag = 0;

    this->CurrentSpectra = NULL;
}


//---------------------------------------------------------------------------
vtkMRSpectroscopyGUI::~vtkMRSpectroscopyGUI ( )
{

    //----------------------------------------------------------------
    // Remove Callbacks
    
    if (this->DataCallbackCommand)
    {
        this->DataCallbackCommand->Delete();
    }

    //----------------------------------------------------------------
    // Remove Observers

    this->RemoveGUIObservers();

    //----------------------------------------------------------------
    // Remove GUI widgets

    if (this->VolumeSelector) {
        this->VolumeSelector->SetParent(NULL);
        this->VolumeSelector->Delete();
    }
   
    if (this->TestButton11) {
        this->TestButton11->SetParent(NULL);
        this->TestButton11->Delete();
    }

    if (this->TestButton12) {
        this->TestButton12->SetParent(NULL);
        this->TestButton12->Delete();
    }

    if (this->TestButton21)  {
        this->TestButton21->SetParent(NULL);
        this->TestButton21->Delete();
    }
    
    if (this->TestButton22) {
        this->TestButton22->SetParent(NULL);
        this->TestButton22->Delete();
    }

    if (this->SpectraSlider) {
        this->SpectraSlider->SetParent(NULL);
        this->SpectraSlider->Delete();
    }
    
    if (this->RedScale) {
        this->RedScale
        ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

    if (this->GreenScale) {
        this->GreenScale
        ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

    if (this->YellowScale) {
        this->YellowScale
        ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

    if (this->xSpecRange) {
        this->xSpecRange->SetParent(NULL);
        this->xSpecRange->Delete();
    }

    if (this->ySpecRange) {
        this->ySpecRange->SetParent(NULL);
        this->ySpecRange->Delete();
    }

    //----------------------------------------------------------------
    // Unregister Logic class
    
    this->SetModuleLogic ( NULL );

}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::Init()
{
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::Enter()
{
  // Fill in
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  
  //this->SlicerStyle = this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->GetInteractorStyle();
  //this->SplitWindow();
  //this->DoOtherThing();
  vtkRenderer* ren = this->GetApplicationGUI()->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()->GetRenderer();
  ren->Render();

  if (this->TimerFlag == 0)
    {
      this->TimerFlag = 1;
      this->TimerInterval = 100;  // 100 ms
      ProcessTimerEvents();
    }

}


void vtkMRSpectroscopyGUI::SplitWindow()
{
  vtkKWRenderWidget* rw = this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer();
  rw->GetNthRenderer(0)->SetViewport(0, 0, 0.5, 1);
  vtkRenderer* r = vtkRenderer::New();
  double rgb[3] = {1, 0, 0};
  r->SetBackground(rgb);
  r->SetViewport(0.5, 0, 1, 1);
  rw->AddRenderer(r);
}
  

//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::Exit ( )
{
  // Fill in
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "MRSpectroscopyGUI: " << this->GetClassName ( ) << "\n";
  os << indent << "Logic: " << this->GetLogic ( ) << "\n";
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::RemoveGUIObservers ( )
{

    //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

    if (this->VolumeSelector) {
        this->VolumeSelector->
            RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }  

    if (this->TestButton11) {
        this->TestButton11
            ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

    if (this->TestButton11) {
        this->TestButton11
            ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

    if (this->TestButton12) {
        this->TestButton12
            ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

    if (this->TestButton21) {
        this->TestButton21
            ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

    if (this->TestButton22) {
        this->TestButton22
            ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

    if (this->SpectraSlider) {
        this->SpectraSlider
            ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

    if (this->xSpecRange) {
        this->xSpecRange
            ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

    if (this->ySpecRange) {
        this->ySpecRange
            ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

    this->RemoveLogicObservers();
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::AddGUIObservers ( )
{
    this->RemoveGUIObservers();

    vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

    //----------------------------------------------------------------
    // MRML

    vtkIntArray* events = vtkIntArray::New();
    events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
    events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
    events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
    
    if (this->GetMRMLScene() != NULL)
    {
        this->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
    }
    events->Delete();

    //----------------------------------------------------------------
    // GUI Observers
    //----------------------------------------------------------------

    this->VolumeSelector
        ->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);

    this->TestButton11
        ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->TestButton12
        ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->TestButton21
        ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->TestButton22
        ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->SpectraSlider
        ->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);

    this->AddLogicObservers();

    this->RedScale    = appGUI->GetMainSliceGUI("Red")->GetSliceController()->GetOffsetScale()->GetScale();
    this->YellowScale = appGUI->GetMainSliceGUI("Yellow")->GetSliceController()->GetOffsetScale()->GetScale();
    this->GreenScale  = appGUI->GetMainSliceGUI("Green")->GetSliceController()->GetOffsetScale()->GetScale();

    this->RedScale->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);
    this->RedScale->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->YellowScale->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);
    this->YellowScale->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->GreenScale->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);
    this->GreenScale->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);

    this->xSpecRange->AddObserver( vtkKWRange::RangeValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ySpecRange->AddObserver( vtkKWRange::RangeValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);

}


void vtkMRSpectroscopyGUI::AddGridObserver( )
{
    vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
    this->Interactor = appGUI->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor();
    this->Interactor
        ->AddObserver(vtkCommand::LeftButtonReleaseEvent, (vtkCommand *)this->GUICallbackCommand);
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::RemoveLogicObservers ( )
{
    if (this->GetLogic())
    {
        this->GetLogic()->RemoveObservers(vtkCommand::ModifiedEvent,
                                      (vtkCommand *)this->LogicCallbackCommand);
    }
}




//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::AddLogicObservers ( )
{
    this->RemoveLogicObservers();  

    if (this->GetLogic())
    {
        this->GetLogic()->AddObserver(vtkMRSpectroscopyLogic::StatusUpdateEvent,
                                  (vtkCommand *)this->LogicCallbackCommand);
    }
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::HandleMouseEvent(vtkSlicerInteractorStyle *style)
{
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::ProcessGUIEvents(vtkObject *caller,
                                         unsigned long event, void *callData)
{

    const char *eventName = vtkCommand::GetStringFromEventId(event);
    vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

    if (strcmp(eventName, "LeftButtonPressEvent") == 0)
    {
        vtkSlicerInteractorStyle *style = vtkSlicerInteractorStyle::SafeDownCast(caller);
        HandleMouseEvent(style);
        return;
    }

    if (this->HasStyle && strcmp(eventName, "LeftButtonReleaseEvent") == 0) {
        std::cerr << "Left Button Released" << std::endl;
        UpdateGridScalars();
    }

    if (selector == this->VolumeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->VolumeSelector->GetSelected() != NULL) 
    {
        std::cerr << "New Volume Selected!" << std::endl;
        std::cerr << *vtkMRMLScalarVolumeNode::SafeDownCast(this->VolumeSelector->GetSelected())->GetImageData() << std::endl;
        std::cerr << "Meta Data!" << std::endl;
    }
    else if (this->TestButton11 == vtkKWPushButton::SafeDownCast(caller) 
      && event == vtkKWPushButton::InvokedEvent)
    {
        std::cerr << "TestButton11 is pressed." << std::endl;
        LoadSpectra();
    }
    else if (this->TestButton12 == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
        std::cerr << "TestButton12 is pressed." << std::endl;
        DoOtherThing();
    }
    else if (this->TestButton21 == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
        std::cerr << "TestButton21 is pressed." << std::endl;
    }
    else if (this->TestButton22 == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
        std::cerr << "TestButton22 is pressed." << std::endl;
    }
    else if (this->SpectraSlider == vtkKWScale::SafeDownCast(caller)
	   && (event == vtkKWScale::ScaleValueChangedEvent || event == vtkKWScale::ScaleValueChangingEvent)) 
    {
        std::cerr << "Spectra Slider!" << std::endl;
    }
    else if (this->RedScale == vtkKWScale::SafeDownCast(caller)
	   && (event == vtkKWScale::ScaleValueChangedEvent || event == vtkKWScale::ScaleValueChangingEvent)) 
    {
        std::cerr << "Red moved to " << this->RedScale->GetValue() << std::endl;

        if(this->CurrentSpectra != NULL) 
        {
            int axialSlice = GetAxialSlice();
            if(axialSlice != -1) 
            {
	            this->PlotView->SetSlice(axialSlice);
	            std::cerr << "New Axial Slice " << axialSlice << std::endl;
            }
        }
    }
    else if (this->GreenScale == vtkKWScale::SafeDownCast(caller)
	   && (event == vtkKWScale::ScaleValueChangedEvent || event == vtkKWScale::ScaleValueChangingEvent)) 
    {
        std::cerr << "Green moved to " << this->GreenScale->GetValue() << std::endl;
    }
    else if (this->YellowScale == vtkKWScale::SafeDownCast(caller)
	   && (event == vtkKWScale::ScaleValueChangedEvent || event == vtkKWScale::ScaleValueChangingEvent)) 
    {
        std::cerr << "Yellow moved to " << this->YellowScale->GetValue() <<  std::endl;

    } 
    else if( caller == this->xSpecRange && event == vtkKWRange::RangeValueChangingEvent) 
    {

        double minValue;
        double maxValue;
        xSpecRange->GetRange( minValue, maxValue );
        stringstream widenRange;
        widenRange << "SetRange " << minValue - xSpecRange->GetResolution()
                                  << " " << maxValue + xSpecRange->GetResolution();
        stringstream narrowRange;
        narrowRange << "SetRange " << minValue + xSpecRange->GetResolution()
                                   << " " << maxValue - xSpecRange->GetResolution();
        stringstream incrementRange;
        incrementRange << "SetRange " << minValue + xSpecRange->GetResolution()
                                      << " " << maxValue + xSpecRange->GetResolution();
        stringstream decrementRange;
        decrementRange << "SetRange " << minValue - xSpecRange->GetResolution()
                                      << " " << maxValue - xSpecRange->GetResolution();
        this->xSpecRange->RemoveBinding( "<Left>");
        this->xSpecRange->AddBinding( "<Left>", this->xSpecRange, decrementRange.str().c_str() );
        this->xSpecRange->RemoveBinding( "<Right>");
        this->xSpecRange->AddBinding( "<Right>", this->xSpecRange, incrementRange.str().c_str() );
        this->xSpecRange->RemoveBinding( "<Up>");
        this->xSpecRange->AddBinding( "<Up>", this->xSpecRange, narrowRange.str().c_str() );
        this->xSpecRange->RemoveBinding( "<Down>");
        this->xSpecRange->AddBinding( "<Down>", this->xSpecRange, widenRange.str().c_str() );
        this->xSpecRange->Focus();

        //  Get the display unit type and convert to points:
        //  Convert Values to points before setting the plot controller's range

        svkSpecPoint* point = svkSpecPoint::New();
        point->SetDataHdr( this->CurrentSpectra->GetDcmHeader() ); 
        int specUnits = svkSpecPoint::PPM;

        float lowestPoint = point->ConvertPosUnits(
            this->xSpecRange->GetEntry1()->GetValueAsDouble(),
            specUnits,
            svkSpecPoint::PTS
        );

        float highestPoint = point->ConvertPosUnits(
            this->xSpecRange->GetEntry2()->GetValueAsDouble(),
            specUnits,
            svkSpecPoint::PTS
        );

        this->PlotView->SetWindowLevelRange( lowestPoint, highestPoint, svkPlotGridView::FREQUENCY);
    } 
    else if( caller == this->ySpecRange && event == vtkKWRange::RangeValueChangingEvent) 
    {
        this->PlotView->SetWindowLevelRange(
            this->ySpecRange->GetEntry1()->GetValueAsDouble(),
            this->ySpecRange->GetEntry2()->GetValueAsDouble(),
            1
        );

        double minValue;
        double maxValue;
        this->ySpecRange->GetRange( minValue, maxValue );
        double delta = (maxValue-minValue)/500;
        stringstream widenRange;
        widenRange << "SetRange " << minValue - delta << " " << maxValue + delta;
        stringstream narrowRange;
        narrowRange << "SetRange " << minValue + delta << " " << maxValue - delta;
        stringstream incrementRange;
        incrementRange << "SetRange " << minValue + delta << " " << maxValue + delta;
        stringstream decrementRange;
        decrementRange << "SetRange " << minValue - delta << " " << maxValue - delta;
        this->ySpecRange->RemoveBinding( "<Left>");
        this->ySpecRange->AddBinding( "<Left>", this->ySpecRange, decrementRange.str().c_str() );
        this->ySpecRange->RemoveBinding( "<Right>");
        this->ySpecRange->AddBinding( "<Right>", this->ySpecRange, incrementRange.str().c_str() );
        this->ySpecRange->RemoveBinding( "<Up>");
        this->ySpecRange->AddBinding( "<Up>", this->ySpecRange, narrowRange.str().c_str() );
        this->ySpecRange->RemoveBinding( "<Down>");
        this->ySpecRange->AddBinding( "<Down>", this->ySpecRange, widenRange.str().c_str() );
        this->ySpecRange->Focus();
    }

} 

void vtkMRSpectroscopyGUI::RenderActor(vtkProp* actor)
{
  this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderer()->AddActor(actor);
}


void vtkMRSpectroscopyGUI::LoadSpectra( )
{
    svkDataModel* model = svkDataModel::New();
    svkImageData* ddfData = model->LoadFile("/Users/jasonc/data/slicer/t6273/t6273_lac_fbcomb_sum_cp_cor.ddf");
    this->CurrentSpectra = ddfData;
    ddfData->Register(NULL);

    cout << "DDF DATA: " << *(ddfData->GetImage()) << endl;

    svkImageTopologyGenerator* gen = svkMrsTopoGenerator::New();

    vtkActor *grid = vtkActor::New();
    gen->GenerateVoxelGridActor(ddfData, grid);
    grid->GetMapper()->Update();
  
    RenderActor(grid);

    vtkSlicerSliceGUI* redgui = this->GetApplicationGUI()->GetMainSliceGUI("Red");
    vtkSlicerSliceGUI* yellowgui = this->GetApplicationGUI()->GetMainSliceGUI("Yellow");
    vtkSlicerSliceGUI* greengui = this->GetApplicationGUI()->GetMainSliceGUI("Green");
    
    vtkSlicerSliceGUI* guis[3] = {redgui, yellowgui, greengui};
  
    for(int i = 0; i < 3; ++i) {

        vtkMatrix4x4* rasToXYZ = vtkMatrix4x4::New();
        rasToXYZ->DeepCopy(guis[i]->GetSliceNode()->GetXYToRAS());
        cout << "rasToXYZ " << *(rasToXYZ) << endl;

        // Convert RAS to LPS ????
        for (int j = 0; j < 5; j++) {
            rasToXYZ->SetElement(i, j, rasToXYZ->GetElement(i, j) * -1.); 
            if (i < 2) {    
                rasToXYZ->SetElement(i+1, j, rasToXYZ->GetElement(i+1, j) * -1.); 
            } else {
                rasToXYZ->SetElement(i-1, j, rasToXYZ->GetElement(i-1, j) * -1.); 
            }
            // SI
            if (i == 0) {    
                rasToXYZ->SetElement(i+2, j, rasToXYZ->GetElement(i+2, j) * -1.); 
            }
        }
        cout << "rasToXYZ " << *(rasToXYZ) << endl;

        vtkTransform* transform = vtkTransform::New();
        transform->SetMatrix(rasToXYZ);
        transform->Inverse();
        vtkTransformPolyDataFilter* filter = vtkTransformPolyDataFilter::New();
        filter->SetTransform(transform);

        vtkPolyData* pdata = gen->GenerateVoxelGridPolyData(ddfData);
//diagnostics: 
pdata->Update();
cout << "num cells (pdata): " << pdata->GetNumberOfCells() << endl;
cout << "cell 0, POINT 0: " << (pdata->GetCell(0)->GetPoints()->GetPoint(0))[0] << endl; 
cout << "cell 0, POINT 0: " << (pdata->GetCell(0)->GetPoints()->GetPoint(0))[1] << endl; 
cout << "cell 0, POINT 0: " << (pdata->GetCell(0)->GetPoints()->GetPoint(0))[2] << endl; 

        filter->SetInput(pdata);

        vtkPolyData* ndata = filter->GetOutput();
        ndata->Update();

        vtkDataSetAttributes* cd = ndata->GetCellData();
        cd->Initialize();
        //ndata->Update();
        int cellcount = ndata->GetNumberOfCells();
cout << "num cells (ndata): " << cellcount << endl;
cout << "cell 0, POINT 0: " << (ndata->GetCell(0)->GetPoints()->GetPoint(0))[0] << endl; 
cout << "cell 0, POINT 0: " << (ndata->GetCell(0)->GetPoints()->GetPoint(0))[1] << endl; 
cout << "cell 0, POINT 0: " << (ndata->GetCell(0)->GetPoints()->GetPoint(0))[2] << endl; 

        vtkUnsignedCharArray* newScalars = vtkUnsignedCharArray::New(); 
        newScalars->SetNumberOfTuples(cellcount); 
        newScalars->SetNumberOfComponents(4);   
        for (int j = 0; j < cellcount; j++) {
            newScalars->InsertTuple4(j, 0, 255, 0, 255);
        }
        cd->SetScalars(newScalars);
        cd->Update();
        ndata->Modified();

        vtkPolyDataMapper2D* mapper = vtkPolyDataMapper2D::New();
        mapper->SetInput( ndata );
        mapper->SetScalarModeToUseCellData();
        mapper->ScalarVisibilityOn();
        mapper->Update();

        vtkActor2D* actor2D = vtkActor2D::New();
        actor2D->SetMapper(mapper);
        actor2D->Modified();
        actor2D->GetProperty()->SetColor(0, 1, 0);
        this->Grids[i] = ndata;

        guis[i]->GetSliceViewer()->GetRenderWidget()->GetRenderer()->AddActor(actor2D);
    }

    redgui->GetSliceViewer()->Render();
    yellowgui->GetSliceViewer()->Render();
    greengui->GetSliceViewer()->Render();

    //------------------------
    //  Initialize Widgets:
    //------------------------
    svkSpecPoint* point = svkSpecPoint::New();
    point->SetDataHdr( this->CurrentSpectra->GetDcmHeader() ); 
    float lowestPoint = point->ConvertPosUnits(
                                0,
                                svkSpecPoint::PTS,
                                svkSpecPoint::PPM
                            );

    float highestPoint = point->ConvertPosUnits(
                                ddfData->GetImage()->GetCellData()->GetArray(0)->GetNumberOfTuples(),
                                svkSpecPoint::PTS,
                                svkSpecPoint::PPM
                            );
    this->xSpecRange->SetWholeRange( lowestPoint, highestPoint );
    //this->xSpecRange->SetRange( PPM_DEFAULT_MIN, PPM_DEFAULT_MAX );
    this->xSpecRange->EnabledOn();
    point->Delete();

cout << "C1" << endl;
    double range[2];
    this->CurrentSpectra->GetDataRange( range, svkImageData::REAL  );
cout << "C2" << endl;
    this->ySpecRange->SetWholeRange( range[0], range[1] );
cout << "C3" << endl;
    //this->ySpecRange->SetRange( range[0]*NEG_RANGE_SCALE, range[1]*POS_RANGE_SCALE );
cout << "C4" << endl;
    this->ySpecRange->SetResolution( (range[1] - range[0])*SLIDER_RELATIVE_RESOLUTION );
cout << "C5" << endl;
    this->ySpecRange->EnabledOn();
cout << "C6" << endl;

    DoOtherThing();
}


/*!
 *
 */
void vtkMRSpectroscopyGUI::DoOtherThing( )
{
    svkDataModel* model = svkDataModel::New();
    svkImageData* ddfData = this->CurrentSpectra;
    ddfData->GetImage()->Update();

    vtkSlicerApplicationGUI* appgui = this->GetApplicationGUI();
    vtkSlicerViewerWidget* viewer = appgui->GetViewerWidget();
    vtkKWRenderWidget* rwidget = viewer->GetMainViewer();
    vtkRenderWindowInteractor* rwi = rwidget->GetRenderWindowInteractor();
    this->PlotView = svkPlotGridViewController::New();
    this->PlotView->SetRWInteractor(rwi);
    this->PlotView->SetInput( ddfData );                                                                                                                                                                                                              
    this->PlotView->SetSlice( 4 );                                                                                                                                                                                                                 
    this->PlotView->HighlightSelectionVoxels( );                                                                                                                                                                                                   
    this->PlotView->SetWindowLevelRange(150, 350, 0 );                                                                                                                                                                                             
    this->PlotView->SetWindowLevelRange(-98000000, 200000000, 1);
    AddGridObserver();
    int axialSlice = GetAxialSlice();
    if(axialSlice != -1)
        this->PlotView->SetSlice(axialSlice);
    this->HasStyle = 1;
} 


int vtkMRSpectroscopyGUI::GetAxialSlice() 
{
    double sagittal = this->RedScale->GetValue();
    std::cerr << "Sagittal: " << sagittal << std::endl;

    svkImageTopologyGenerator* gen = svkMrsTopoGenerator::New();
    vtkGenericCell* cellBuffer = vtkGenericCell::New();

    int voxelIndex = -1;
    double* bounds;
    double boundArray[6];
    this->CurrentSpectra->GetImage()->GetBounds(boundArray);
    std::cerr << "Bounds: " << boundArray[0] << " " << boundArray[1] << " "
	    << boundArray[2] << " " << boundArray[3] << " " 
	    << boundArray[4] << " " << boundArray[5] << std::endl;

    vtkPoints* pointsBuffer;
    int cellCount = this->CurrentSpectra->GetImage()->GetNumberOfCells();
    this->CurrentSpectra->GetImage()->GetCell(0, cellBuffer);
    pointsBuffer = cellBuffer->GetPoints();
    pointsBuffer->ComputeBounds();
    std::cerr << "Cell 0: " << *pointsBuffer << cellBuffer->GetPointIds()->GetId(0) << std::endl;
    this->CurrentSpectra->GetImage()->GetCell(cellCount / 5, cellBuffer);
    pointsBuffer = cellBuffer->GetPoints();
    pointsBuffer->ComputeBounds();
    std::cerr << "Cell " << cellCount / 5 << ": " << *(pointsBuffer->GetPoint(0)) << " " << cellBuffer->GetPointIds()->GetId(0) << std::endl;
    this->CurrentSpectra->GetImage()->GetCell(2 * cellCount / 5, cellBuffer);
    pointsBuffer = cellBuffer->GetPoints();
    pointsBuffer->ComputeBounds();
    std::cerr << "Cell " << 2 * cellCount / 5 << ": " << *(pointsBuffer->GetPoint(0)) << " " << cellBuffer->GetPointIds()->GetId(0) << std::endl;
    this->CurrentSpectra->GetImage()->GetCell(3 * cellCount / 5, cellBuffer);
    pointsBuffer = cellBuffer->GetPoints();
    pointsBuffer->ComputeBounds();
    std::cerr << "Cell " << 3 * cellCount / 5 << ": " << *(pointsBuffer->GetPoint(0)) << " " << cellBuffer->GetPointIds()->GetId(0) << std::endl;
    this->CurrentSpectra->GetImage()->GetCell(4 * cellCount / 5, cellBuffer);
    pointsBuffer = cellBuffer->GetPoints();
    pointsBuffer->ComputeBounds();
    std::cerr << "Cell " << 4 * cellCount / 5 << ": " << *(pointsBuffer->GetPoint(0)) << " " << cellBuffer->GetPointIds()->GetId(0) << std::endl;
    this->CurrentSpectra->GetImage()->GetCell(5 * cellCount / 5, cellBuffer);
    pointsBuffer = cellBuffer->GetPoints();
    pointsBuffer->ComputeBounds();
    std::cerr << "Cell " << 5 * cellCount / 5 << ": " << *(pointsBuffer->GetPoint(0)) << " " << cellBuffer->GetPointIds()->GetId(0) << std::endl;
    
    for(int i = 0; i < this->CurrentSpectra->GetImage()->GetNumberOfCells(); ++i) {
        this->CurrentSpectra->GetImage()->GetCell(i, cellBuffer);
        bounds = cellBuffer->GetBounds();
        //    std::cerr << "Points " << i << ": " << *(cellBuffer->GetPoints()) << std::endl;
        if(bounds[4] <= sagittal && sagittal <= bounds[5]) {
            voxelIndex = i;
            break;
        }
    }

    std::cerr << std::endl;
    if (voxelIndex == -1) {
        std::cerr << "Voxel Out of Bounds!!!!" << std::endl;
        return -1;
    }
    int coordinates[3];
    this->CurrentSpectra->GetIndexFromID(voxelIndex, coordinates);
    std::cerr << "Axial Slice! " << coordinates[2] << std::endl;
    return coordinates[2];
}


/*!  
 *  This refreshes the PlotGrid on the Red, Green, Yellow windows.
 */
void vtkMRSpectroscopyGUI::UpdateGridScalars( )
{
    int* corners = this->PlotView->GetTlcBrc();
    int minPoints[3];
    int maxPoints[3];
    int curPoints[3];

    const unsigned char OPAQUE[4] = {0, 255, 0, 255};   //Green, Opaque
    const unsigned char TRANSPARENT[4] = {255, 255, 0, 0};

cout << "corners: " << corners[0] << " " << corners[1] << endl;
    this->CurrentSpectra->GetIndexFromID(corners[0], minPoints); 
    this->CurrentSpectra->GetIndexFromID(corners[1], maxPoints); 
cout << "min: " << minPoints[0] << " " << minPoints[1] <<  " " << minPoints[2] << endl;
cout << "max: " << maxPoints[0] << " " << maxPoints[1] <<  " " << maxPoints[2] << endl;

    set<int> pointSet;
    vtkGenericCell* cellBuffer = vtkGenericCell::New();

    for (int i = 0; i < this->CurrentSpectra->GetImage()->GetNumberOfCells(); ++i) {

        this->CurrentSpectra->GetIndexFromID(i, curPoints);

        if(minPoints[0] <= curPoints[0]  && curPoints[0] <= maxPoints[0] &&
            minPoints[1] <= curPoints[1]  && curPoints[1] <= maxPoints[1] &&
            minPoints[2] <= curPoints[2]  && curPoints[2] <= maxPoints[2]) 
        {
            this->CurrentSpectra->GetImage()->GetCell(i, cellBuffer);
            vtkIdList* pointIDs = cellBuffer->GetPointIds();
            for (int j = 0; j < pointIDs->GetNumberOfIds(); ++j) 
            {
	            pointSet.insert(pointIDs->GetId(j));
            }
        }
    }
    for (int i = 0; i < 3; ++i) 
    {
        vtkDataSetAttributes* cd = this->Grids[i]->GetCellData();
        vtkUnsignedCharArray* newScalars = vtkUnsignedCharArray::New(); 
        int cellcount = this->Grids[i]->GetNumberOfCells();
        newScalars->SetNumberOfTuples(cellcount); 
        newScalars->SetNumberOfComponents(4);
        for (int j = 0; j < cellcount; ++j) 
        {
            bool shouldPaint = true;
            vtkIdList* pointIDs = this->Grids[i]->GetCell(j)->GetPointIds();
            for (int k = 0; k < pointIDs->GetNumberOfIds(); ++k) 
            {
	            if(!pointSet.count(pointIDs->GetId(k))) 
                {
	                shouldPaint = false;
	                break;
	            }
            }
            if(shouldPaint) 
            {
	            newScalars->InsertTupleValue(j, OPAQUE);
            } else 
            {
	            newScalars->InsertTupleValue(j, TRANSPARENT);
            }    
            cd->SetScalars(newScalars);
            cd->Update();
            this->Grids[i]->Update();
        }
    }
    this->RefreshSliceWindows();
}


void vtkMRSpectroscopyGUI::RefreshSliceWindows()
{
  this->GetApplicationGUI()->GetMainSliceGUI("Red")->GetSliceViewer()->Render();
  this->GetApplicationGUI()->GetMainSliceGUI("Yellow")->GetSliceViewer()->Render();
  this->GetApplicationGUI()->GetMainSliceGUI("Green")->GetSliceViewer()->Render();
}


void vtkMRSpectroscopyGUI::ResetStyle()
{
  this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->SetInteractorStyle(this->SlicerStyle);
  this->HasStyle = 0;
}


void vtkMRSpectroscopyGUI::DataCallback(vtkObject *caller, 
                                     unsigned long eid, void *clientData, void *callData)
{
  vtkMRSpectroscopyGUI *self = reinterpret_cast<vtkMRSpectroscopyGUI *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkMRSpectroscopyGUI DataCallback");
  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::ProcessLogicEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{

  if (this->GetLogic() == vtkMRSpectroscopyLogic::SafeDownCast(caller))
    {
    if (event == vtkMRSpectroscopyLogic::StatusUpdateEvent)
      {
      //this->UpdateDeviceStatus();
      }
    }
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
  // Fill in

  if (event == vtkMRMLScene::SceneCloseEvent)
    {
    }
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::ProcessTimerEvents()
{
  if (this->TimerFlag)
    {
    // update timer
    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(), 
                                         this->TimerInterval,
                                         this, "ProcessTimerEvents");        
    }
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::BuildGUI ( )
{

  // ---
  // MODULE GUI FRAME 
  // create a page
  this->UIPanel->AddPage ( "SIVIC MR Spectroscopy", "SIVIC MR Spectroscopy", NULL );

  BuildGUIForHelpFrame();
  BuildGUIForTestFrame1();
  BuildGUIForTestFrame2();

}


void vtkMRSpectroscopyGUI::BuildGUIForHelpFrame ()
{
  // Define your help text here.
  const char *help = 
    "See "
    "<a>http://www.slicer.org/slicerWiki/index.php/Modules:SIVIC</a> for details.";
  const char *about =
    "This work is supported by NCIGT, NA-MIC.";

  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "SIVIC MR Spectroscopy" );
  this->BuildHelpAndAboutFrame (page, help, about);
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::BuildGUIForTestFrame1()
{

    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkKWWidget *page = this->UIPanel->GetPageWidget ("SIVIC MR Spectroscopy");
  
    vtkSlicerModuleCollapsibleFrame *conBrowsFrame = vtkSlicerModuleCollapsibleFrame::New();

    conBrowsFrame->SetParent(page);
    conBrowsFrame->Create();
    conBrowsFrame->SetLabelText("SIVIC MRS Module");
    //conBrowsFrame->CollapseFrame();
    app->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               conBrowsFrame->GetWidgetName(), page->GetWidgetName());

    // -----------------------------------------
    // Test child frame
    
    vtkKWFrameWithLabel *frame = vtkKWFrameWithLabel::New();
    frame->SetParent(conBrowsFrame->GetFrame());
    frame->Create();
    //frame->SetLabelText ("");
    this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                    frame->GetWidgetName() );

    // -----------------------------------------
    // Test push button

    this->VolumeSelector = vtkSlicerNodeSelectorWidget::New();
    this->VolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, NULL);
    this->VolumeSelector->SetParent(conBrowsFrame->GetFrame() );
    this->VolumeSelector->Create();
    this->VolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
    this->VolumeSelector->UpdateMenu();
    
    this->VolumeSelector->SetBorderWidth(2);
    this->VolumeSelector->SetLabelText( "Input Volume: ");
    this->VolumeSelector->SetBalloonHelpString("select an input volume from the current mrml scene.");
    app->Script("pack %s -side top -anchor e -padx 20 -pady 4",
	        this->VolumeSelector->GetWidgetName());

    this->TestButton11 = vtkKWPushButton::New ( );
    this->TestButton11->SetParent ( frame->GetFrame() );
    this->TestButton11->Create ( );
    this->TestButton11->SetText ("Load MRS Data");
    this->TestButton11->SetWidth (12);
    
    this->TestButton12 = vtkKWPushButton::New ( );
    this->TestButton12->SetParent ( frame->GetFrame() );
    this->TestButton12->Create ( );
    this->TestButton12->SetText ("Test 12");
    this->TestButton12->SetWidth (12);
    
    // Create the x range widget
    this->xSpecRange = vtkKWRange::New();
    this->xSpecRange->SetParent( frame->GetFrame() );
    this->xSpecRange->SetLabelText( "Frequency" );
    this->xSpecRange->SetBalloonHelpString("Adjusts x range of the spectroscopic data.");
    this->xSpecRange->SetWholeRange(10, -3);
    //this->xSpecRange->SetWholeRange(0, 1);
    this->xSpecRange->Create();
    this->xSpecRange->SetRange(10, -3);
    //this->xSpecRange->SetRange(0, 1);
    this->xSpecRange->EnabledOff();
    this->xSpecRange->SetSliderSize(3);
    this->xSpecRange->SetEntry1PositionToTop();
    this->xSpecRange->SetEntry2PositionToTop();
    this->xSpecRange->SetLabelPositionToTop();
    //  Set default resolution for PPM:
    this->xSpecRange->SetResolution( .001 );

    // Create the y range widget
    this->ySpecRange = vtkKWRange::New();
    this->ySpecRange->SetParent( frame->GetFrame() );
    this->ySpecRange->SetLabelText( "Amplitude" );
    this->ySpecRange->SetBalloonHelpString("Adjusts y range of the spectroscopic data.");
    this->ySpecRange->SetWholeRange(0, 1);
    this->ySpecRange->SetResolution(1.0);
    this->ySpecRange->Create();
    this->ySpecRange->SetRange(0, 1);
    this->ySpecRange->EnabledOff();
    this->ySpecRange->SetSliderSize(3);
    this->ySpecRange->SetEntry1PositionToTop();
    this->ySpecRange->SetEntry2PositionToTop();
    this->ySpecRange->SetLabelPositionToTop();

    this->ySpecRange->SetWholeRange(0, 1);
    this->ySpecRange->SetRange(0, 1);
    this->ySpecRange->ClampRangeOff();


    int row = 0;
    this->Script("grid %s -row %d -column 0 -columnspan 2 -sticky ewns", this->TestButton11->GetWidgetName(), row );

    row++;
    this->Script("grid %s -row %d -column 0 -sticky nsew -columnspan 2", this->xSpecRange->GetWidgetName(), row );

    row++;
    this->Script("grid %s -row %d -column 0 -sticky nsew -columnspan 2", this->ySpecRange->GetWidgetName(), row );


    this->SpectraSlider = vtkKWScale::New ( );
  
    conBrowsFrame->Delete();
    frame->Delete();

}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::BuildGUIForTestFrame2 ()
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("SIVIC MR Spectroscopy");
  
  vtkSlicerModuleCollapsibleFrame *conBrowsFrame = vtkSlicerModuleCollapsibleFrame::New();

  conBrowsFrame->SetParent(page);
  conBrowsFrame->Create();
  conBrowsFrame->SetLabelText("Test Frame 2");
  //conBrowsFrame->CollapseFrame();
  //app->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               //conBrowsFrame->GetWidgetName(), page->GetWidgetName());

  // -----------------------------------------
  // Test child frame

  vtkKWFrameWithLabel *frame = vtkKWFrameWithLabel::New();
  frame->SetParent(conBrowsFrame->GetFrame());
  frame->Create();
  frame->SetLabelText ("SIVIC MRS Module");
  //this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 //frame->GetWidgetName() );
  
  // -----------------------------------------
  // Test push button

  this->TestButton21 = vtkKWPushButton::New ( );
  this->TestButton21->SetParent ( frame->GetFrame() );
  this->TestButton21->Create ( );
  this->TestButton21->SetText ("Test 21");
  this->TestButton21->SetWidth (12);

  this->TestButton22 = vtkKWPushButton::New ( );
  this->TestButton22->SetParent ( frame->GetFrame() );
  this->TestButton22->Create ( );
  this->TestButton22->SetText ("Test 22");
  this->TestButton22->SetWidth (12);

  //this->Script("pack %s %s -side left -padx 2 -pady 2", 
               //this->TestButton21->GetWidgetName(),
               //this->TestButton22->GetWidgetName());


  conBrowsFrame->Delete();
  frame->Delete();
}


//----------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::UpdateAll()
{
}
