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
#include "vtkKWRange.h"
#include "vtkKWLoadSaveButtonWithLabel.h"

#include "vtkKWPushButton.h"

#include "vtkCornerAnnotation.h"

#include "svkDataModel.h"
#include "svkImageData.h"
#include "svkImageTopologyGenerator.h"
#include "svkPlotGridViewController.h"
#include "svkSpecPoint.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLsvkImageDataNode.h"
#include "vtkMRMLsvkImageDataStorageNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLLayoutNode.h"
#include "vtkExtractUnstructuredGrid.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLUnstructuredGridNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLUnstructuredGridDisplayNode.h"
#include "vtkStructuredPointsToUnstructuredGridFilter.h" 

#include <svkExtractMRIFromMRS.h> 

#include "cmath"
#include "set"

#define PPM_DEFAULT_MIN 3.844 
#define PPM_DEFAULT_MAX 0.602 
#define NEG_RANGE_SCALE .17
#define POS_RANGE_SCALE .70
#define SLIDER_RELATIVE_RESOLUTION 0.002



//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRSpectroscopyGUI );
vtkCxxRevisionMacro ( vtkMRSpectroscopyGUI, "$Revision$");
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

    this->model = NULL;

    //----------------------------------------------------------------
    // GUI widgets
    //----------------------------------------------------------------
    this->VolumeSelector = NULL;
    this->LoadSpectraButton = NULL;
    this->SpectraSlider  = NULL;
    this->xSpecRange     = NULL;
    this->ySpecRange     = NULL;
    
    //----------------------------------------------------------------
    // Locator  (MRML)
    //----------------------------------------------------------------
    this->TimerFlag = 0;
    this->TimerFlag = 1;

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
   
    if (this->LoadSpectraButton) {
        this->LoadSpectraButton->SetParent(NULL);
        this->LoadSpectraButton->Delete();
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

    if (this->Grids[0]) {
        this->Grids[0]->Delete();
    }

    if (this->Grids[1]) {
        this->Grids[1]->Delete();
    }

    if (this->PlotView) {
        this->PlotView->Delete();
    }

    if (this->CurrentSpectra) {
        this->CurrentSpectra->Delete();
    }

    if (this->SpectraSlider) {
        this->SpectraSlider->Delete();
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

    if (this->DataCallbackCommand) {
        this->DataCallbackCommand->Delete();
    }

    if (this->SlicerStyle) {
        this->SlicerStyle->Delete();
    }

    if (this->Interactor) {
        this->Interactor->Delete();
    }

    if (this->Style) {
        this->Style->Delete();
    }

    if ( this->model != NULL ) {
        this->model->Delete();
        this->model = NULL;
    }
}


/*!
 *  Register the svkImageDataNode and svkImageDataStorageNode into the 
 *  vtkMRMLScene. These MRML Nodes are for the derived metabolite maps 
 *  in the Slicer framework. 
 */
void vtkMRSpectroscopyGUI::Init()
{
    vtkMRMLScene* scene = this->GetMRMLScene();

    vtkMRMLsvkImageDataNode* imageDataNode = vtkMRMLsvkImageDataNode::New();
    scene->RegisterNodeClass(imageDataNode);
    imageDataNode->Delete();

    vtkMRMLsvkImageDataStorageNode* imageDataStorageNode = vtkMRMLsvkImageDataStorageNode::New();
    scene->RegisterNodeClass(imageDataStorageNode);
    imageDataStorageNode->Delete();
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::Enter()
{
    vtkRenderer* ren = this->GetApplicationGUI()->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()->GetRenderer();
    ren->Render();

    if (this->TimerFlag == 0) 
    {
        this->TimerFlag = 1;
        this->TimerInterval = 100;  // 100 ms
    }
}



//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::Exit ( )
{
    // Fill in
    std::cout << "exit" << std::endl;
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
    //std::cout << "RemoveGUIObservers" << std::endl;
    //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

    if (this->VolumeSelector) {
        this->VolumeSelector->
            RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

    if (this->LoadSpectraButton)
    {
        this->LoadSpectraButton->GetWidget()->GetLoadSaveDialog()->RemoveObservers(
            vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand 
        );
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
    this->RemoveMRMLObservers();

    //std::cout << "finish RemoveGUIObservers" << std::endl;
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::AddGUIObservers ( )
{

    //std::cout << "AddGUIObservers" << std::endl;
    this->RemoveGUIObservers();

    vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

    //----------------------------------------------------------------
    // MRML
    //----------------------------------------------------------------
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
    this->LoadSpectraButton->GetWidget()->GetLoadSaveDialog()->AddObserver(
        vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand 
    );
    this->DisplayButton
        ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->DisplayButton2
        ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->SpectraSlider
        ->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);

    this->AddLogicObservers();

    //  API is changing between slicer versions:
    this->RedScale    = appGUI->GetMainSliceGUI("Red")->GetSliceController()->GetOffsetScale();
    this->YellowScale = appGUI->GetMainSliceGUI("Yellow")->GetSliceController()->GetOffsetScale();
    this->GreenScale  = appGUI->GetMainSliceGUI("Green")->GetSliceController()->GetOffsetScale();

    this->RedScale->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);
    this->RedScale->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->YellowScale->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);
    this->YellowScale->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->GreenScale->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);
    this->GreenScale->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);

    this->xSpecRange->AddObserver( vtkKWRange::RangeValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ySpecRange->AddObserver( vtkKWRange::RangeValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);

    this->AddMRMLObservers();

    //std::cout << "dataNode = " << this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLsvkImageDataNode") << std::endl;
}


/*
 *
 */
void vtkMRSpectroscopyGUI::AddGridObserver( )
{
    vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
    //  API is changing between slicer versions:
    //this->Interactor = appGUI->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor();
    this->Interactor = appGUI->GetActiveViewerWidget()->GetMainViewer()->GetRenderWindowInteractor();
    this->Interactor->AddObserver(
        vtkCommand::LeftButtonReleaseEvent, (vtkCommand *)this->GUICallbackCommand
    );
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::RemoveMRMLObservers(void)
{
    //std::cout << "RemoveMRMLObservers" << std::endl;
    //Remove the MRML observer
    if ( this->GetApplicationGUI() )
    {
        this->GetApplicationGUI()->GetMRMLScene()->RemoveObservers(
            vtkMRMLScene::SceneLoadEndEvent, this->MRMLCallbackCommand
        );
    }
}

//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::AddMRMLObservers(void)
{
    //std::cout << "AddMRMLObservers" << std::endl;
    //Remove the MRML observer
    if ( this->GetApplicationGUI() )
    {
        this->GetApplicationGUI()->GetMRMLScene()->AddObserver(
            vtkMRMLScene::SceneLoadEndEvent, this->MRMLCallbackCommand
        );
    }
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::RemoveLogicObservers ( )
{
    //std::cout << "RemoveLogicObservers" << std::endl;
    if (this->GetLogic())
    {
        this->GetLogic()->RemoveObservers(
            vtkCommand::ModifiedEvent, (vtkCommand *)this->LogicCallbackCommand
        );
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
    //std::cout << "finish AddLogicObservers" << std::endl;
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::HandleMouseEvent(vtkSlicerInteractorStyle *style)
{  
    std::cout << "HandleMouseEvent" << std::endl;
    std::cout << "dataNode = " << this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLsvkImageDataNode") << std::endl;
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::ProcessGUIEvents(vtkObject *caller,
                                         unsigned long event, void *callData)
{ 

    //std::cout << "ProcessGUIEvents" << std::endl;

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
    else   if (this->LoadSpectraButton->GetWidget()->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller) && event == vtkKWTopLevel::WithdrawEvent )
    {
	    std::cerr << "LoadSpectraButton is pressed." << std::endl;
        const char * filename = this->LoadSpectraButton->GetWidget()->GetFileName();
        if (filename) {
            LoadSpectraFromFile(filename);
	    }
    }
    else if (this->DisplayButton == vtkKWPushButton::SafeDownCast(caller) 
      && event == vtkKWPushButton::InvokedEvent)
    {
        this->GenerateMetaboliteMap(1.99, 0.4, "NAA Integraged Area"); 
    }
    else if (this->DisplayButton2 == vtkKWPushButton::SafeDownCast(caller) 
      && event == vtkKWPushButton::InvokedEvent)
    {
        this->GenerateMetaboliteMap(3.0, 0.4, "CHO Integrated Area"); 
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
	            std::cerr << "Closest MRS Slice " << axialSlice << std::endl;
	            this->PlotView->SetSlice(axialSlice);
	            std::cerr << "New Axial Slice " << axialSlice << std::endl;
            }
        }
    }
    else if (this->GreenScale == vtkKWScale::SafeDownCast(caller)
	   && (event == vtkKWScale::ScaleValueChangedEvent || event == vtkKWScale::ScaleValueChangingEvent)) 
    {
        std::cerr << "Green moved to " << this->GreenScale->GetValue() << std::endl;
        if(this->CurrentSpectra != NULL) 
        {
            int axialSlice = GetMRSSlice("Green");
            if(axialSlice != -1) 
            {
	            std::cerr << "Closest MRS Slice " << axialSlice << std::endl;
	            //this->PlotView->SetSlice(axialSlice);
	            std::cerr << "New Slice " << axialSlice << std::endl;
            }
        }
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
        point->SetDcmHeader( this->CurrentSpectra->GetDcmHeader() ); 
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


/*!
 *
 */
void vtkMRSpectroscopyGUI::GenerateMetaboliteMap(float peak, float width, string mapName) 
{

    svkExtractMRIFromMRS* quant = svkExtractMRIFromMRS::New();
    //if( this->model->GetDataObject( "SpectroscopicData" ) == NULL ) {
        //cout << "ERROR no data object" << endl; 
        //return;
    //}

    quant->SetInput( this->ddfData ); 
    //quant->SetInput( this->model->GetDataObject("SpectroscopicData") ); 
    quant->SetSeriesDescription( " Metabolite Map" );
    quant->SetPeakPosPPM( peak );
    quant->SetPeakWidthPPM( width );
    quant->GetOutput()->GetIncrements();
    quant->Update(); 
    
    svkMriImageData* outputData = svkMriImageData::New();
    outputData->DeepCopy(quant->GetOutput());
    outputData->SetDcmHeader(quant->GetOutput()->GetDcmHeader());
    
    //svkPlotGridViewController::SafeDownCast(dataViewer)->SetInput(
        //outputData, svkPlotGridView::MET );
    //svkPlotGridViewController::SafeDownCast(dataViewer)->TurnPropOn(
        //svkPlotGridView::OVERLAY_IMAGE );
    //svkPlotGridViewController::SafeDownCast(dataViewer)->TurnPropOn(
        //svkPlotGridView::OVERLAY_TEXT );
    
    //this->PlotView->SetInput( quant->GetOutput(), svkPlotGridView::MET );
    this->PlotView->SetInput( outputData, svkPlotGridView::MET );
    this->PlotView->TurnPropOn( svkPlotGridView::OVERLAY_IMAGE ) ; 
    this->PlotView->TurnPropOn( svkPlotGridView::OVERLAY_TEXT ) ; 
    this->PlotView->SetOverlayOpacity( .5 );
    this->PlotView->GetView()->Refresh();
    
    //quant->Delete(); 

    //  Register new svkMriImageData with scene: 
    //  This pipeline is borrowed from vtkSlicerVolumesLogic::CloneVolume()
    //  clone the volume node to represent the new Metabolite Map

    //  Register the new class type with the scene:
    vtkMRMLScalarVolumeNode *metMapNode = vtkMRMLScalarVolumeNode::New();
    this->GetMRMLScene()->RegisterNodeClass(metMapNode);
    metMapNode->Delete();

    vtkMRMLScalarVolumeNode *clonedVolumeNode = vtkMRMLScalarVolumeNode::New();
    //clonedVolumeNode->CopyWithScene(volumeNode);
    clonedVolumeNode->SetAndObserveStorageNodeID(NULL);
    //std::string uname = this->MRMLScene->GetUniqueNameByString(name);
    clonedVolumeNode->SetName( mapName.c_str()); 
    //clonedVolumeNode->SetAndObserveDisplayNodeID(clonedDisplayNode->GetID());
  
    // copy over the volume's data
    vtkImageData* clonedVolumeData = vtkImageData::New();
    clonedVolumeData->DeepCopy( outputData ); 

    //  sivic dcos is i,j,k in LPS we want this in RAS, so mulitply first two
    //  cols of dcos by -1:
    double dcos[3][3]; 
    this->ddfData->GetDcos( dcos ); 

    for(int j = 0; j < 3; j++) {
        for (int k = 0; k < 3; k++) {
            if ( ( j < 2 ) ) {
                dcos[j][k] = -1 * dcos[j][k]; 
            } else {
                dcos[j][k] = dcos[j][k]; 
            }
        }
    }
    cout << dcos[0][0] << " " << dcos[0][1] << " " << dcos[0][2] << endl;
    cout << dcos[1][0] << " " << dcos[1][1] << " " << dcos[1][2] << endl;
    cout << dcos[2][0] << " " << dcos[2][1] << " " << dcos[2][2] << endl;

    clonedVolumeNode->SetIJKToRASDirections (dcos); 

    //  Get the LPS origin values from the input image and convert them to RAS for 
    //  use in Slicer MRML Node(clonedVolumeNode). The vtkImageData is generic based 
    //  on unit spacing and 0 origin.  MRML manages the scaling and translation to real
    //  RAS values: 
    cout << "MET MAP ORIGIN" << endl;
    double origin[3]; 
    this->ddfData->GetDcmHeader()->GetOrigin(origin); 
    origin[0] = -1 * origin[0]; 
    origin[1] = -1 * origin[1]; 
    clonedVolumeNode->SetOrigin(origin); 
    clonedVolumeData->SetOrigin(0,0,0); 

    //  Here too, set the vtkImageData spacing to 1,1,1, (in this case, but should be
    //  generic for anisotropic aspect ratio
    double spacing[3]; 
    this->ddfData->GetDcmHeader()->GetPixelSpacing(spacing); 
    clonedVolumeNode->SetSpacing(spacing); 
    clonedVolumeData->SetSpacing(1,1,1); 


    clonedVolumeNode->SetAndObserveImageData( clonedVolumeData );
    clonedVolumeNode->SetModifiedSinceRead(1);
  
    // add the cloned volume to the scene
    this->GetMRMLScene()->AddNode( clonedVolumeNode );
  
    // remove references
    clonedVolumeNode->Delete(); 
    clonedVolumeData->Delete();

}


/*!
 *  Loads MRS dat file and puts it into MRML scene
 */
void vtkMRSpectroscopyGUI::LoadSpectraFromFile(const char *filename)
{  

    this->model = svkDataModel::New();
    this->model->SetDataFileName( "SpectroscopicData", filename );
    this->ddfData = svkMrsImageData::SafeDownCast(this->model->LoadFile(filename));

    vtkMRMLsvkImageDataNode* imageDataNode = vtkMRMLsvkImageDataNode::New();
    imageDataNode->SetData( this->ddfData );

    //TODO: how to add?
    //this->VolumeSelector->AddNodeClass(
        //"vtkMRMLsvkImageDataNode", "vtkMRMLsvkImageDataNode1", "vtkMRMLsvkImageDataNode1", "vtkMRMLsvkImageDataNode1"
    //);
    //this->VolumeSelector->SetSelected(ImageDataNode);

    if (this->GetMRMLScene() != NULL) {
        this->GetMRMLScene()->AddNodeNoNotify(imageDataNode);
    }

    //  Set new default layout for this module: 
    this->GetApplicationGUI()->GetGUILayoutNode()->SetViewArrangement( vtkMRMLLayoutNode::SlicerLayoutDual3DView ) ; 

    SetSpectraData(this->ddfData);
}


/*!
 *  Should create a voxel grid for spatial referencing on anatomical data.  Should be a
 *  MRML object in scene that could be turned on/off (data module visibility? )
 */
void vtkMRSpectroscopyGUI::SetSpectraData(svkImageData* ddfData)
{   

    this->CurrentSpectra = ddfData;
    ddfData->Register(NULL);

/*
    cout << "DDF DATA Loaded: " << *(ddfData) << endl;

    svkImageTopologyGenerator* gen = svkMrsTopoGenerator::New();

    //vtkActor *grid = vtkActor::New();
    //gen->GenerateVoxelGridActor(ddfData, grid);
    //grid->GetMapper()->Update();

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
        cout << "cell 0, POINT x: " << (pdata->GetCell(0)->GetPoints()->GetPoint(0))[0] << endl; 
        cout << "cell 0, POINT y: " << (pdata->GetCell(0)->GetPoints()->GetPoint(0))[1] << endl; 
        cout << "cell 0, POINT z: " << (pdata->GetCell(0)->GetPoints()->GetPoint(0))[2] << endl; 

        filter->SetInput(pdata);

        vtkPolyData* ndata = filter->GetOutput();
        ndata->Update();

        //==============================================================
        // Set Cell topology in unstructured grid
        vtkExtractUnstructuredGrid* extractGrid = vtkExtractUnstructuredGrid::New();
        extractGrid->SetInput( this->ddfData );
        
        //  Look at the cells in this unstructured grid:
        vtkUnstructuredGrid* ug = extractGrid->GetOutput();
        cout << "NUMBER OF CELLS: " << ug->GetNumberOfCells() << endl;
        
        int numCells = this->ddfData->GetNumberOfCells();
        cout << "NUMBER OF vtkID CELLS: " << numCells << endl;
        
        //  Get Cells from vtkImageData (this->ddfData) and use the cell definitions to to set cells in UG using VTK_VOXEL
        vtkCellArray * cellArray = vtkCellArray::New(); 
        for (int cellID = 0; cellID < numCells; cellID++ ) {
            cellArray->InsertNextCell( this->ddfData->GetCell( cellID ) );
        }
        ug->SetCells( VTK_VOXEL, cellArray );
        cout << "NUMBER OF CELLS: " << ug->GetNumberOfCells() << endl;
        
        vtkMRMLUnstructuredGridNode* gridNode = vtkMRMLUnstructuredGridNode::New(); 
        this->GetMRMLScene()->RegisterNodeClass(gridNode);
        gridNode->Delete();
        
        //  Register the new class type with the scene:
        gridNode = vtkMRMLUnstructuredGridNode::New();
        gridNode->SetAndObserveUnstructuredGrid( ug ); 
        
        vtkMRMLUnstructuredGridDisplayNode* gridDisplayNode = vtkMRMLUnstructuredGridDisplayNode::New(); 
        gridDisplayNode->SetUnstructuredGrid( ug ); 
        
        this->GetMRMLScene()->AddNode( gridNode );
        this->GetMRMLScene()->AddNode( gridDisplayNode );
        
        //==============================================================
        
        vtkMRMLModelNode* modelNode = vtkMRMLModelNode::New(); 
        this->GetMRMLScene()->RegisterNodeClass(modelNode);
        modelNode->Delete();

        //  Register the new class type with the scene:
        modelNode = vtkMRMLModelNode::New();
        modelNode->SetAndObservePolyData( ndata ); 
        //gridNode->SetAndObserveStorageNodeID(NULL);

        vtkMRMLModelDisplayNode* mdn = vtkMRMLModelDisplayNode::New();
        mdn->SetPolyData( ndata ); 
        mdn->SetScene( this->GetMRMLScene() ); 
        modelNode->SetScene( this->GetMRMLScene() ); 

        // add the model volume to the scene
        this->GetMRMLScene()->AddNode( modelNode );
        this->GetMRMLScene()->AddNode( mdn );

        vtkDataSetAttributes* cd = ndata->GetCellData();
        cd->Initialize();
        //ndata->Update();
        int cellcount = ndata->GetNumberOfCells();
        cout << "num cells (ndata): " << cellcount << endl;

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

        //guis[i]->GetSliceViewer()->GetRenderWidget()->GetRenderer()->AddActor(actor2D);
    }

    redgui->GetSliceViewer()->Render();
    yellowgui->GetSliceViewer()->Render();
    greengui->GetSliceViewer()->Render();
*/

    //------------------------
    //  Initialize Widgets:
    //------------------------
    svkSpecPoint* point = svkSpecPoint::New();
    point->SetDcmHeader( this->CurrentSpectra->GetDcmHeader() ); 
    float lowestPoint = point->ConvertPosUnits(
                                0,
                                svkSpecPoint::PTS,
                                svkSpecPoint::PPM
                            );

    float highestPoint = point->ConvertPosUnits(
                                //ddfData->GetImage()->GetCellData()->GetArray(0)->GetNumberOfTuples(),
                                ddfData->GetCellData()->GetArray(0)->GetNumberOfTuples(),
                                svkSpecPoint::PTS,
                                svkSpecPoint::PPM
                            );
    this->xSpecRange->SetWholeRange( lowestPoint, highestPoint );
    //this->xSpecRange->SetRange( PPM_DEFAULT_MIN, PPM_DEFAULT_MAX );
    this->xSpecRange->EnabledOn();
    point->Delete();

    double range[2];
    this->CurrentSpectra->GetDataRange( range, svkImageData::REAL  );
    this->ySpecRange->SetWholeRange( range[0], range[1] );
    //this->ySpecRange->SetRange( range[0]*NEG_RANGE_SCALE, range[1]*POS_RANGE_SCALE );
    this->ySpecRange->SetResolution( (range[1] - range[0])*SLIDER_RELATIVE_RESOLUTION );
    this->ySpecRange->EnabledOn();

    DisplaySpectra();

}


/*!
 *
 */
void vtkMRSpectroscopyGUI::DisplaySpectra( )
{
    svkImageData* ddfData = this->CurrentSpectra;
    //ddfData->Update();

    vtkSlicerApplicationGUI* appgui = this->GetApplicationGUI();

    //  Set which Slicer 3D Window to put spectra into:
    //vtkSlicerViewerWidget* viewer = appgui->GetNthViewerWidget(0);
    vtkSlicerViewerWidget* viewer = appgui->GetNthViewerWidget(1);
    
    vtkKWRenderWidget* rwidget = viewer->GetMainViewer();
    vtkRenderWindowInteractor* rwi = rwidget->GetRenderWindowInteractor();

    this->PlotView = svkPlotGridViewController::New();
    this->PlotView->SetRWInteractor(rwi);
    this->PlotView->SetInput( ddfData );

    // Set Cell topology in unstructured grid
    vtkExtractUnstructuredGrid* extractGrid = vtkExtractUnstructuredGrid::New();
    extractGrid->SetInput( this->ddfData ); 

    //  Look at the cells in this unstructured grid:
    vtkUnstructuredGrid* ug = extractGrid->GetOutput(); 
    cout << "NUMBER OF CELLS: " << ug->GetNumberOfCells() << endl;

    int numCells = this->ddfData->GetNumberOfCells(); 
    cout << "NUMBER OF vtkID CELLS: " << numCells << endl; 

    //  Get Cells from vtkImageData (this->ddfData) and use the cell definitions to to set cells in UG using VTK_VOXEL
    vtkCellArray * cellArray = vtkCellArray::New(); 
    for (int i = 0; i < numCells; i++ ) {    
        cellArray->InsertNextCell( this->ddfData->GetCell( i ) ); 
    }
    ug->SetCells( VTK_VOXEL, cellArray ); 
    cout << "NUMBER OF CELLS: " << ug->GetNumberOfCells() << endl;

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


int vtkMRSpectroscopyGUI::GetMRSSlice( vtkstd::string  windowColor) 
{
    
    vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
    cout << "SLICE ORIENTATION: " << appGUI->GetMainSliceGUI(windowColor.c_str())->GetSliceController()->GetSliceNode()->GetOrientationString() << endl; 
    cout << "slicetoRAS: " << *(appGUI->GetMainSliceGUI(windowColor.c_str())->GetSliceController()->GetSliceNode()->GetSliceToRAS()) << endl; 

    //  see which spectral slice intersects with the last column 
    //  of SliceToRAS matrix.
    vtkMatrix4x4* sliceCenterRASMatrix = appGUI->GetMainSliceGUI(windowColor.c_str())->GetSliceController()->GetSliceNode()->GetSliceToRAS(); 
    double imageSliceCenterRAS[3]; 
    imageSliceCenterRAS[0] = sliceCenterRASMatrix->GetElement(0,3); 
    imageSliceCenterRAS[1] = sliceCenterRASMatrix->GetElement(1,3); 
    imageSliceCenterRAS[2] = sliceCenterRASMatrix->GetElement(2,3); 

    //  Transform RAS to LPS (Slicer to SIVIC convention transformation):
    double imageSliceCenterLPS[3]; 
    imageSliceCenterLPS[0] = -1 * imageSliceCenterRAS[0];
    imageSliceCenterLPS[1] = -1 * imageSliceCenterRAS[1];
    imageSliceCenterLPS[2] = imageSliceCenterRAS[2];

    cout << "slice center LPS" << imageSliceCenterLPS[0] << " " << imageSliceCenterLPS[1] << " " << imageSliceCenterLPS[2] << endl;

    //  Determine which MRS slice contains these coordinates:
    int mrsSlice = this->CurrentSpectra->GetClosestSlice(imageSliceCenterLPS, svkDcmHeader::AXIAL);
    cout << "Get MRS Slice: " << mrsSlice << endl;

    return mrsSlice;

}


int vtkMRSpectroscopyGUI::GetAxialSlice() 
{

    vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();


    //  Get slice orientation: axial, sag, cor
    cout << "RED SLICE ORIENTATION: " << appGUI->GetMainSliceGUI("Red")->GetSliceController()->GetSliceNode()->GetOrientationString() << endl;; 

    //  vtkIndent ident; 
    //  appGUI->GetMainSliceGUI("Red")->GetSliceController()->GetSliceNode()->PrintSelf(cout, ident); 
    //  cout << "XYRAS: " << *(appGUI->GetMainSliceGUI("Red")->GetSliceController()->GetSliceNode()->GetXYToRAS()) << endl; 

    cout << "slicetoRAS: " << *(appGUI->GetMainSliceGUI("Red")->GetSliceController()->GetSliceNode()->GetSliceToRAS()) << endl; 

    //  see which spectral slice intersects with the last column 
    //  of SliceToRAS matrix.
    vtkMatrix4x4* sliceCenterRASMatrix = appGUI->GetMainSliceGUI("Red")->GetSliceController()->GetSliceNode()->GetSliceToRAS(); 
    double redImageSliceCenterRAS[3]; 
    redImageSliceCenterRAS[0] = sliceCenterRASMatrix->GetElement(0,3); 
    redImageSliceCenterRAS[1] = sliceCenterRASMatrix->GetElement(1,3); 
    redImageSliceCenterRAS[2] = sliceCenterRASMatrix->GetElement(2,3); 

    //  Transform RAS to LPS (Slicer to SIVIC convention transformation):
    double redImageSliceCenterLPS[3]; 
    redImageSliceCenterLPS[0] = -1 * redImageSliceCenterRAS[0];
    redImageSliceCenterLPS[1] = -1 * redImageSliceCenterRAS[1];
    redImageSliceCenterLPS[2] = redImageSliceCenterRAS[2];

    cout << "slice center LPS" << redImageSliceCenterLPS[0] << " " << redImageSliceCenterLPS[1] << " " << redImageSliceCenterLPS[2] << endl;

    //  Determine which MRS slice contains these coordinates:
    int mrsSlice = this->CurrentSpectra->GetClosestSlice(redImageSliceCenterLPS, svkDcmHeader::AXIAL);
    cout << "Get MRS Slice: " << mrsSlice << endl;

    return mrsSlice;
}


/*!  
 *  This refreshes the PlotGrid on the Red, Green, Yellow windows.
 */
void vtkMRSpectroscopyGUI::UpdateGridScalars( )
{

/*
    vtkExtractUnstructuredGrid* extractGrid = vtkExtractUnstructuredGrid::New();
    extractGrid->SetInput( this->ddfData ); 
    
    //  Look at the cells in this unstructured grid:
    vtkUnstructuredGrid* ug = extractGrid->GetOutput(); 
    cout << "NUMBER OF CELLS: " << ug->GetNumberOfCells() << endl;

    int* corners = this->PlotView->GetTlcBrc();
    int minPoints[3];
    int maxPoints[3];
    int curPoints[3];

    const unsigned char OPAQUE[4] = {0, 255, 0, 255};   //Green, Opaque
    const unsigned char TRANSPARENT[4] = {255, 255, 0, 0};

    //  This displays the i,j,k index of the tlc and brc cells selected (voxel indices)
    cout << "corners: " << corners[0] << " " << corners[1] << endl;
    this->CurrentSpectra->GetIndexFromID(corners[0], minPoints); 
    this->CurrentSpectra->GetIndexFromID(corners[1], maxPoints); 
    cout << "min: " << minPoints[0] << " " << minPoints[1] <<  " " << minPoints[2] << endl;
    cout << "max: " << maxPoints[0] << " " << maxPoints[1] <<  " " << maxPoints[2] << endl;

    set<int> pointSet;
    vtkGenericCell* cellBuffer = vtkGenericCell::New();

    //for (int i = 0; i < this->CurrentSpectra->GetImage()->GetNumberOfCells(); ++i) {
    cout << "NUMBER OF CELLS in svkMRSIMageData: " <<  this->CurrentSpectra->GetNumberOfCells() << endl;
    for (int i = 0; i < this->CurrentSpectra->GetNumberOfCells(); ++i) {

        this->CurrentSpectra->GetIndexFromID(i, curPoints);

        if(minPoints[0] <= curPoints[0]  && curPoints[0] <= maxPoints[0] &&
            minPoints[1] <= curPoints[1]  && curPoints[1] <= maxPoints[1] &&
            minPoints[2] <= curPoints[2]  && curPoints[2] <= maxPoints[2]) 
        {
            //this->CurrentSpectra->GetImage()->GetCell(i, cellBuffer);
            this->CurrentSpectra->GetCell(i, cellBuffer);
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
        cout << "CELL COUNT IN OPACITY LOOP: " << cellcount << endl;
        newScalars->SetNumberOfTuples(cellcount); 
        newScalars->SetNumberOfComponents(4);
        for (int j = 0; j < cellcount; ++j) 
        {
            bool shouldPaint = true;
            vtkIdList* pointIDs = this->Grids[i]->GetCell(j)->GetPointIds();
            //cout << "NUM POINTIDs to check for cell: "<<   pointIDs->GetNumberOfIds() << endl;
            //  evalute this for both endpoints of line cell: 
            for (int k = 0; k < pointIDs->GetNumberOfIds(); ++k) 
            {
                //  If the id is not in the point set then set to false
	            if(!pointSet.count(pointIDs->GetId(k))) 
                {
	                shouldPaint = false;
	                break;
	            }
            }
            if(shouldPaint) 
            {
	            cout << "should paint? " << j << endl; 
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
*/
   
    
}


void vtkMRSpectroscopyGUI::RefreshSliceWindows()
{
    this->GetApplicationGUI()->GetMainSliceGUI("Red")->GetSliceViewer()->Render();
    this->GetApplicationGUI()->GetMainSliceGUI("Yellow")->GetSliceViewer()->Render();
    this->GetApplicationGUI()->GetMainSliceGUI("Green")->GetSliceViewer()->Render();
}


void vtkMRSpectroscopyGUI::ResetStyle()
{
    //  API is changing between slicer versions:
    //this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->SetInteractorStyle(this->SlicerStyle);
    this->GetApplicationGUI()->GetActiveViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->SetInteractorStyle(this->SlicerStyle);
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
    std::cout << "ProcessLogicEvents" << std::endl;
    std::cout << "dataNode = " << this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLsvkImageDataNode") << std::endl;

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
    if (event == vtkMRMLScene::SceneLoadStartEvent)
    {
        return;
    }
    if (event == vtkMRMLScene::SceneLoadEndEvent)
    {
        if (this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLsvkImageDataNode") > 0)
        {
	        // TODO: should GetNodesByClass, but don't know how to do so.
	        vtkCollection* collection = this->GetMRMLScene()->GetNodesByName("vtkMRMLsvkImageDataNode1");
	        vtkObject* object = collection->GetItemAsObject(0);
	        vtkMRMLsvkImageDataNode* dataNode = vtkMRMLsvkImageDataNode::SafeDownCast(object);
	        SetSpectraData(dataNode->GetData());
        }
        return;
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
    BuildGUIForSpectraFrame();
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
void vtkMRSpectroscopyGUI::BuildGUIForSpectraFrame()
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

    // File browser
    this->LoadSpectraButton = vtkKWLoadSaveButtonWithLabel::New();
    this->LoadSpectraButton->SetParent ( frame->GetFrame() );
    this->LoadSpectraButton->GetWidget()->GetLoadSaveDialog()->SetMasterWindow ( 
        this->GetApplicationGUI()->GetMainSlicerWindow() 
    );
    this->LoadSpectraButton->Create ( );
    this->LoadSpectraButton->SetWidth(20);
    this->LoadSpectraButton->GetWidget()->SetText ("Select MRSI File");
    this->LoadSpectraButton->GetWidget()->GetLoadSaveDialog()->SetTitle("Open Volume File");
    this->LoadSpectraButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes("{ {Spectra} {*} }");
    this->LoadSpectraButton->GetWidget()->GetLoadSaveDialog()->RetrieveLastPathFromRegistry("OpenPath");

    // Display button
    this->DisplayButton = vtkKWPushButton::New();
    this->DisplayButton->SetParent(frame->GetFrame());
    this->DisplayButton->Create();
    this->DisplayButton->SetText("Generate NAA Integrated Area Map");
    this->DisplayButton->SetWidth(10);

    this->DisplayButton2 = vtkKWPushButton::New();
    this->DisplayButton2->SetParent(frame->GetFrame());
    this->DisplayButton2->Create();
    this->DisplayButton2->SetText("Generate CHO Integrated Area Map");
    this->DisplayButton2->SetWidth(10);
    
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
    this->ySpecRange->SetWholeRange(-150000000, 150000000);
    this->ySpecRange->SetResolution(1.0);
    this->ySpecRange->Create();
    this->ySpecRange->SetRange(-150000000, 150000000);
    this->ySpecRange->EnabledOff();
    this->ySpecRange->SetSliderSize(3);
    this->ySpecRange->SetEntry1PositionToTop();
    this->ySpecRange->SetEntry2PositionToTop();
    this->ySpecRange->SetLabelPositionToTop();

    //this->ySpecRange->SetWholeRange(0, 1);
    //this->ySpecRange->SetRange(0, 1);
    this->ySpecRange->ClampRangeOff();

    // display spectra button
    this->checkBoxOrginal = vtkKWCheckButton::New();
    this->checkBoxOrginal->SetParent(frame->GetFrame());
    this->checkBoxOrginal->Create();
    this->checkBoxOrginal->SelectedStateOff();
    this->checkBoxOrginal->SetText("Original Spectra");

    // display channel2 button
    this->checkBoxChannel2 = vtkKWCheckButton::New();
    this->checkBoxChannel2->SetParent(frame->GetFrame());
    this->checkBoxChannel2->Create();
    this->checkBoxChannel2->SelectedStateOff();
    this->checkBoxChannel2->SetText("Channel 2");

    // display channel3 button
    this->checkBoxChannel3 = vtkKWCheckButton::New();
    this->checkBoxChannel3->SetParent(frame->GetFrame());
    this->checkBoxChannel3->Create();
    this->checkBoxChannel3->SelectedStateOff();
    this->checkBoxChannel3->SetText("Channel 3");

    // display channel4 button
    this->checkBoxChannel4 = vtkKWCheckButton::New();
    this->checkBoxChannel4->SetParent(frame->GetFrame());
    this->checkBoxChannel4->Create();
    this->checkBoxChannel4->SelectedStateOff();
    this->checkBoxChannel4->SetText("Channel 4");

    // display metabolite map
    this->checkBoxMetMap = vtkKWCheckButton::New();
    this->checkBoxMetMap->SetParent(frame->GetFrame());
    this->checkBoxMetMap->Create();
    this->checkBoxMetMap->SelectedStateOff();
    this->checkBoxMetMap->SetText("Metabolite Map");

    int row = 0;
    this->Script("grid %s -row %d -column 0 -columnspan 2 -sticky ewns", this->LoadSpectraButton->GetWidgetName(), row);

    row++;
    this->Script("grid %s -row %d -column 0 -sticky ewns -pady 4", this->DisplayButton->GetWidgetName(), row);

    row++;
    this->Script("grid %s -row %d -column 0 -sticky ewns -pady 4", this->DisplayButton2->GetWidgetName(), row);

    row++;
    this->Script("grid %s -row %d -column 0 -columnspan 2 -sticky nsew", this->xSpecRange->GetWidgetName(), row );

    row++;
    this->Script("grid %s -row %d -column 0 -columnspan 2 -sticky nsew", this->ySpecRange->GetWidgetName(), row );

    row++;
    this->Script("grid %s -row %d -column 0 -sticky nsew", this->checkBoxOrginal->GetWidgetName(), row );

    row++;
    this->Script("grid %s -row %d -column 0 -sticky nsew", this->checkBoxChannel2->GetWidgetName(), row );

    row++;
    this->Script("grid %s -row %d -column 0 -sticky nsew", this->checkBoxChannel3->GetWidgetName(), row );

    row++;
    this->Script("grid %s -row %d -column 0 -sticky nsew", this->checkBoxChannel4->GetWidgetName(), row );

    row++;
    this->Script("grid %s -row %d -column 0 -sticky nsew", this->checkBoxMetMap->GetWidgetName(), row );


    this->SpectraSlider = vtkKWScale::New ( );


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

  
    conBrowsFrame->Delete();
    frame->Delete();

}

//----------------------------------------------------------------------------
void vtkMRSpectroscopyGUI::UpdateAll()
{
}
