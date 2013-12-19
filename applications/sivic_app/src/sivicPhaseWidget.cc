/*
 *  Copyright © 2009-2013 The Regents of the University of California.
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
 */

#include <svkSpecPoint.h>
#include <sivicPhaseWidget.h>
#include <vtkSivicController.h>
#ifdef ITK_BUILD
#include <svkMRSAutoPhase.h>
#include <svkMRSFirstPointPhase.h>
#endif

vtkStandardNewMacro( sivicPhaseWidget );
vtkCxxRevisionMacro( sivicPhaseWidget, "$Revision$");


/*! 
 *  Constructor
 */
sivicPhaseWidget::sivicPhaseWidget()
{
    this->selectionBoxOnlyButton = NULL;
    this->phaseSlider = NULL;
    this->linearPhaseSlider = NULL;
    this->phaser = svkPhaseSpec::New();
    this->phaseAllVoxelsButton = NULL;
    this->phaseAllChannelsButton = NULL;
    this->phaseH20Button = NULL;
    this->phase0Button = NULL;
    this->phasePivotEntry = NULL;
    this->phaseChangeInProgress = 0;
    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgress );
    this->progressCallback->SetClientData( (void*)this );
}


/*! 
 *  Destructor
 */
sivicPhaseWidget::~sivicPhaseWidget()
{

    if( this->phaseSlider != NULL ) {
        this->phaseSlider->Delete();
        this->phaseSlider = NULL;
    }

    if( this->linearPhaseSlider != NULL ) {
        this->linearPhaseSlider->Delete();
        this->linearPhaseSlider = NULL;
    }

    if( this->phaser != NULL ) {
        this->phaser->Delete();
        this->phaser = NULL;
    }

    if( this->phaseH20Button != NULL ) {
        this->phaseH20Button->Delete();
        this->phaseH20Button= NULL;
    }

    if( this->phase0Button != NULL ) {
        this->phase0Button->Delete();
        this->phase0Button= NULL;
    }

    if( this->phasePivotEntry != NULL ) {
        this->phasePivotEntry->Delete();
        this->phasePivotEntry= NULL;
    }

    if( this->selectionBoxOnlyButton != NULL ) {
        this->selectionBoxOnlyButton->Delete();
        this->selectionBoxOnlyButton = NULL;
    }

}


/*! 
 *  Method in superclass to be overriden to add our custom widgets.
 */
void sivicPhaseWidget::CreateWidget()
{
/*  This method will create our main window. The main window is a 
    vtkKWCompositeWidget with a vtkKWRendWidget. */

    // Check if already created
    if ( this->IsCreated() )
    {
        vtkErrorMacro(<< this->GetClassName() << " already created");
        return;
    }

    // Call the superclass to create the composite widget container
    this->Superclass::CreateWidget();

    this->phaseSlider = vtkKWScaleWithEntry::New();
    this->phaseSlider->SetParent(this);
    this->phaseSlider->Create();
    this->phaseSlider->SetEntryWidth( 4 );
    this->phaseSlider->SetOrientationToHorizontal();
    this->phaseSlider->SetLabelText("Zero Phase");
    this->phaseSlider->SetLabelWidth( 10 );
    this->phaseSlider->SetValue(0);
    this->phaseSlider->SetRange( -180, 180 );
    this->phaseSlider->SetBalloonHelpString("Adjusts the phase of the spectroscopic data.");
    this->phaseSlider->EnabledOff();
    this->phaseSlider->SetEntryPositionToRight();
    this->phaseSlider->SetLabelPositionToLeft();

    this->linearPhaseSlider = vtkKWScaleWithEntry::New();
    this->linearPhaseSlider->SetParent(this);
    this->linearPhaseSlider->Create();
    this->linearPhaseSlider->SetEntryWidth( 4 );
    this->linearPhaseSlider->SetOrientationToHorizontal();
    this->linearPhaseSlider->SetLabelText("Linear Phase");
    this->linearPhaseSlider->SetLabelWidth( 10 );
    this->linearPhaseSlider->SetValue(0);
    this->linearPhaseSlider->SetRange( -2048, 2048 );
    this->linearPhaseSlider->SetResolution( 0.1 );
    this->linearPhaseSlider->SetBalloonHelpString("Adjusts the phase of the spectroscopic data.");
    this->linearPhaseSlider->EnabledOff();
    this->linearPhaseSlider->SetEntryPositionToRight();
    this->linearPhaseSlider->SetLabelPositionToLeft();

    vtkKWCheckButtonSet* checkButtons = vtkKWCheckButtonSet::New();
    checkButtons->SetParent( this );
    checkButtons->PackHorizontallyOn( );
    checkButtons->ExpandWidgetsOn( );
    checkButtons->Create();

    this->phaseAllVoxelsButton = checkButtons->AddWidget(0);
    this->phaseAllVoxelsButton->SetParent(this);
    this->phaseAllVoxelsButton->Create();
    this->phaseAllVoxelsButton->EnabledOff();
    this->phaseAllVoxelsButton->SetText("All Voxels");
    this->phaseAllVoxelsButton->SelectedStateOn();

    this->phaseAllChannelsButton = checkButtons->AddWidget(1);
    this->phaseAllChannelsButton->SetParent(this);
    this->phaseAllChannelsButton->Create();
    this->phaseAllChannelsButton->EnabledOff();
    this->phaseAllChannelsButton->SetText("All Channels");
    this->phaseAllChannelsButton->SelectedStateOff();

    this->selectionBoxOnlyButton = checkButtons->AddWidget(2);
    this->selectionBoxOnlyButton->SetParent(this);
    //this->selectionBoxOnlyButton = vtkKWCheckButton::New(); 
    //this->selectionBoxOnlyButton->SetParent(this);
    this->selectionBoxOnlyButton->Create();
    this->selectionBoxOnlyButton->EnabledOff();
    this->selectionBoxOnlyButton->SetText("Only selection box");
    this->selectionBoxOnlyButton->SelectedStateOn();

    this->phaseH20Button = vtkKWPushButton::New();
    this->phaseH20Button->SetParent( this );
    this->phaseH20Button->Create( );
    this->phaseH20Button->EnabledOff();
    this->phaseH20Button->SetText( "Phase On Water");
    this->phaseH20Button->SetBalloonHelpString("Prototype Auto Phasing.");

    this->phase0Button = vtkKWPushButton::New();
    this->phase0Button->SetParent( this );
    this->phase0Button->Create( );
    this->phase0Button->EnabledOff();
    this->phase0Button->SetText( "First Pt Phase");
    this->phase0Button->SetBalloonHelpString("First Pt Phase.");

    this->phasePivotEntry = vtkKWEntryWithLabel::New();
    this->phasePivotEntry->GetLabel()->SetText("Pivot Point ");
    this->phasePivotEntry->GetWidget()->SetWidth(5);
    this->phasePivotEntry->SetParent( this );
    this->phasePivotEntry->Create( );
    this->phasePivotEntry->EnabledOff();
    this->phasePivotEntry->SetLabelWidth(10);
    this->phasePivotEntry->GetWidget()->SetRestrictValueToInteger();
    this->phasePivotEntry->SetLabelPositionToLeft();

    this->Script("grid %s -row 0 -column 0 -columnspan 6 -sticky nwes", this->phaseSlider->GetWidgetName() );
    this->Script("grid %s -row 1 -column 0 -columnspan 6 -sticky nwes", this->linearPhaseSlider->GetWidgetName() );
    this->Script("grid %s -row 2 -column 0 -columnspan 2 -sticky nwes", this->phasePivotEntry->GetWidgetName() );
    this->Script("grid %s -row 2 -column 2 -columnspan 5 -sticky nwes", checkButtons->GetWidgetName() );
    this->Script("grid %s -row 3 -column 0 -columnspan 3 -sticky we -padx 4 -pady 2 ", this->phaseH20Button->GetWidgetName() );
    this->Script("grid %s -row 3 -column 3 -columnspan 3 -sticky we -padx 4 -pady 2 ", this->phase0Button->GetWidgetName() );

    //this->Script("grid %s -row 1 -column 0 -columnspan 1 -sticky w", this->selectionBoxOnlyButton->GetWidgetName() );


    this->Script("grid rowconfigure %s 0  -weight 2", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 1  -weight 2", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 2  -weight 1", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 3  -weight 2", this->GetWidgetName() );
    //this->Script("grid rowconfigure %s 4  -weight 2", this->GetWidgetName() );

    this->Script("grid columnconfigure %s 0 -weight 1", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 1 -weight 1", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 2 -weight 1", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 3 -weight 1", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 4 -weight 1", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 5 -weight 1", this->GetWidgetName() );

    this->AddCallbackCommandObserver(
        this->overlayController->GetRWInteractor(), vtkCommand::SelectionChangedEvent );
    this->AddCallbackCommandObserver(
        this->plotController->GetRWInteractor(), vtkCommand::SelectionChangedEvent );
    this->AddCallbackCommandObserver(
        this->phaseSlider, vtkKWScale::ScaleValueChangedEvent );
    this->AddCallbackCommandObserver(
        this->phaseSlider, vtkKWScale::ScaleValueChangingEvent );
    this->AddCallbackCommandObserver(
        this->phaseSlider, vtkKWScale::ScaleValueStartChangingEvent );
    this->AddCallbackCommandObserver(
        this->linearPhaseSlider, vtkKWScale::ScaleValueChangedEvent );
    this->AddCallbackCommandObserver(
        this->linearPhaseSlider, vtkKWScale::ScaleValueChangingEvent );
    this->AddCallbackCommandObserver(
        this->linearPhaseSlider, vtkKWScale::ScaleValueStartChangingEvent );
    this->AddCallbackCommandObserver(
        this->phaseAllVoxelsButton, vtkKWCheckButton::SelectedStateChangedEvent );
    this->AddCallbackCommandObserver(
        this->phaseAllChannelsButton, vtkKWCheckButton::SelectedStateChangedEvent );
    this->AddCallbackCommandObserver(
        this->phaseH20Button, vtkKWPushButton::InvokedEvent );
    this->AddCallbackCommandObserver(
        this->phase0Button, vtkKWPushButton::InvokedEvent );
    this->AddCallbackCommandObserver(
        this->phasePivotEntry->GetWidget(), vtkKWEntry::EntryValueChangedEvent );


    checkButtons->Delete();    
}



/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicPhaseWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    // Respond to a selection change in the overlay view
    if (  caller == this->plotController->GetRWInteractor() && event == vtkCommand::SelectionChangedEvent ) {

        this->SetPhaseUpdateExtent();

    // Respond to a selection change in the plot grid view 
    } else if (  caller == this->overlayController->GetRWInteractor() && event == vtkCommand::SelectionChangedEvent ) {
        this->SetPhaseUpdateExtent();
    } else if( caller == this->phaseSlider ) {
        switch ( event ) {
            case vtkKWScale::ScaleValueChangedEvent:
                this->phaseChangeInProgress = 0;
                this->UpdatePhaseSliderBindings();
                break;
            case vtkKWScale::ScaleValueChangingEvent:
                this->phaser->SetPhase0( this->phaseSlider->GetValue() );
                this->phaser->Update();
                if( !this->phaseChangeInProgress ) {
                    this->UpdatePhaseSliderBindings();
                }
                break;
            case vtkKWScale::ScaleValueStartChangingEvent:
                this->phaseChangeInProgress = 1;
                break;
            default:
                cout << "Got a unknown event!" << endl;
        }
    } else if( caller == this->linearPhaseSlider ) {
        switch ( event ) {
            case vtkKWScale::ScaleValueChangedEvent:
                this->phaseChangeInProgress = 0;
                this->UpdateLinearPhaseSliderBindings();
                break;
            case vtkKWScale::ScaleValueChangingEvent:
                this->phaser->SetLinearPhase( (-this->linearPhaseSlider->GetValue())/360.0 );
                this->phaser->Update();
                if( !this->phaseChangeInProgress ) {
                    this->UpdateLinearPhaseSliderBindings();
                }
                break;
            case vtkKWScale::ScaleValueStartChangingEvent:
                this->phaseChangeInProgress = 1;
                break;
            default:
                cout << "Got a unknown event!" << endl;
        }
    } else if( caller == this->phaseAllChannelsButton && event == vtkKWCheckButton::SelectedStateChangedEvent) {
        this->SetPhaseUpdateExtent();
    } else if( caller == this->phaseAllVoxelsButton && event == vtkKWCheckButton::SelectedStateChangedEvent) {
        this->SetPhaseUpdateExtent();
    } else if( caller == this->phaseH20Button && event == vtkKWPushButton::InvokedEvent ) {
        this->ExecutePhase();
    } else if( caller == this->phase0Button && event == vtkKWPushButton::InvokedEvent ) {
        this->ExecuteZeroOrderPhase();
    } else if( caller == this->phasePivotEntry->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent) {
        this->phaser->SetLinearPhasePivot( this->phasePivotEntry->GetWidget()->GetValueAsInt() );
        if( this->phaser->GetInput() != NULL ) {
            this->phaser->Update();
        }
    }
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}


/*!
 *  Sets the correct update extent for phasing
 */
void sivicPhaseWidget::SetPhaseUpdateExtent()
{
    int* start = new int[3];
    int* end = new int[3];
    start[0] = -1;
    start[1] = -1;
    start[2] = -1;
    end[0] = -1;
    end[1] = -1;
    end[2] = -1;

    if ( this->phaseAllChannelsButton->GetSelectedState() ) {
        this->phaser->PhaseAllChannels();
    } else if(  this->model->DataExists("SpectroscopicData") ) {
            this->phaser->SetChannel( this->plotController->GetVolumeIndex( svkMrsImageData::CHANNEL ) );
    }

    if ( this->phaseAllVoxelsButton->GetSelectedState() ) {
        this->phaser->SetUpdateExtent(start, end );
    } else {
        int* range = new int[2];
        range = this->plotController->GetTlcBrc();
        this->model->GetDataObject("SpectroscopicData")->GetIndexFromID(range[0], start);
        this->model->GetDataObject("SpectroscopicData")->GetIndexFromID(range[1], end);
        this->phaser->SetUpdateExtent(start, end );
    }
    delete[] start;
    delete[] end;
}


/*!
 *  Updates/Adds keyboard bindings to the phase slider when it is in focus.
 */
void sivicPhaseWidget::UpdatePhaseSliderBindings()
{
    stringstream increment;
    stringstream decrement;
    increment << "SetValue " << this->phaseSlider->GetValue() + this->phaseSlider->GetResolution();
    decrement << "SetValue " << this->phaseSlider->GetValue() - this->phaseSlider->GetResolution();
    this->phaseSlider->RemoveBinding( "<Left>");
    this->phaseSlider->AddBinding( "<Left>", this->phaseSlider, decrement.str().c_str() );
    this->phaseSlider->RemoveBinding( "<Right>");
    this->phaseSlider->AddBinding( "<Right>", this->phaseSlider, increment.str().c_str() );
    this->phaseSlider->Focus();
}


/*!
 *  Updates/Adds keyboard bindings to the phase slider when it is in focus.
 */
void sivicPhaseWidget::UpdateLinearPhaseSliderBindings()
{
    stringstream increment;
    stringstream decrement;
    increment << "SetValue " << this->linearPhaseSlider->GetValue() + this->linearPhaseSlider->GetResolution();
    decrement << "SetValue " << this->linearPhaseSlider->GetValue() - this->linearPhaseSlider->GetResolution();
    this->linearPhaseSlider->RemoveBinding( "<Left>");
    this->linearPhaseSlider->AddBinding( "<Left>", this->linearPhaseSlider, decrement.str().c_str() );
    this->linearPhaseSlider->RemoveBinding( "<Right>");
    this->linearPhaseSlider->AddBinding( "<Right>", this->linearPhaseSlider, increment.str().c_str() );
    this->linearPhaseSlider->Focus();
}


/*!
 *  Executes the Phasing.
 */
void sivicPhaseWidget::ExecutePhase()
{
    svkImageData* data = this->model->GetDataObject("SpectroscopicData");
    if( data != NULL ) {
        // We'll turn the renderer off to avoid rendering intermediate steps
        this->plotController->GetView()->TurnRendererOff(svkPlotGridView::PRIMARY);
        svkMultiCoilPhase* multiCoilPhase = svkMultiCoilPhase::New();
        multiCoilPhase->AddObserver(vtkCommand::ProgressEvent, progressCallback);
        multiCoilPhase->SetInput( data );
        multiCoilPhase->Update();
        data->Modified();
        multiCoilPhase->RemoveObserver( progressCallback);
        multiCoilPhase->Delete();
        bool useFullFrequencyRange = 0;
        bool useFullAmplitudeRange = 1;
        bool resetAmplitude = 1;
        bool resetFrequency = 0;
        string stringFilename = "PhasedData";
        this->sivicController->Open4DImage( data, stringFilename);
        this->sivicController->EnableWidgets( );
        //this->sivicController->ResetRange( useFullFrequencyRange, useFullAmplitudeRange, 
        //                                   resetAmplitude, resetFrequency );
        this->plotController->GetView()->TurnRendererOn(svkPlotGridView::PRIMARY);
        this->plotController->GetView()->Refresh();
        this->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 0.0 );
        this->GetApplication()->GetNthWindow(0)->SetStatusText(" Done ");
    }
}


/*!
 *  Executes the Phasing.
 */
void sivicPhaseWidget::ExecuteZeroOrderPhase()
{
#ifdef ITK_BUILD
    svkImageData* data = this->model->GetDataObject("SpectroscopicData");
    if( data != NULL ) {
        // We'll turn the renderer off to avoid rendering intermediate steps
        this->plotController->GetView()->TurnRendererOff(svkPlotGridView::PRIMARY);
        svkMRSAutoPhase* zeroOrderPhase = svkMRSFirstPointPhase::New();
        zeroOrderPhase->AddObserver(vtkCommand::ProgressEvent, progressCallback);

//phaser->SetInputConnection(0, reader->GetOutputPort(0) );
//model =  svkMRSAutoPhase::MAX_PEAK_HTS_0;
//phaser->SetPhasingModel(svkMRSAutoPhase::MAX_GLOBAL_PEAK_HT_0); 
//phaser->SetPhasingModel(svkMRSAutoPhase::MAX_PEAK_HTS_0); 
//phaser->SetPhasingModel(svkMRSAutoPhase::MAX_PEAK_HTS_1); 
//phaser->SetPhasingModel(svkMRSAutoPhase::MIN_DIFF_FROM_MAG_0); 
//phaser->SetPhasingModel(svkMRSAutoPhase::MIN_DIFF_FROM_MAG_1); 
//phaser->SetPhasingModel(svkMRSAutoPhase::MAX_PEAK_HTS_01); 
//model =  svkMRSAutoPhase::MAX_PEAK_HTS_1;

        zeroOrderPhase->SetInput( data );
        zeroOrderPhase->OnlyUseSelectionBox();

        svkMRSAutoPhase::phasingModel model;
        model = svkMRSAutoPhase::FIRST_POINT_0; 
        //model = svkMRSAutoPhase::MAX_PEAK_HT_0_ONE_PEAK; 
        //zeroOrderPhase->SetPhasingModel( model );

        zeroOrderPhase->Update();
        data->Modified();
        zeroOrderPhase->RemoveObserver( progressCallback);
        zeroOrderPhase->Delete();
        bool useFullFrequencyRange = 0;
        bool useFullAmplitudeRange = 1;
        bool resetAmplitude = 1;
        bool resetFrequency = 0;
        string stringFilename = "ZeoOrderPhasedData";
        this->sivicController->Open4DImage( data, stringFilename);
        this->sivicController->EnableWidgets( );
        //this->sivicController->ResetRange( useFullFrequencyRange, useFullAmplitudeRange, 
        //                                   resetAmplitude, resetFrequency );
        this->plotController->GetView()->TurnRendererOn(svkPlotGridView::PRIMARY);
        this->plotController->GetView()->Refresh();
        this->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 0.0 );
        this->GetApplication()->GetNthWindow(0)->SetStatusText(" Done ");
    }
#endif
}


void sivicPhaseWidget::InitializePhaser()
{
    if( this->phaser != NULL ) {
        this->phaser->Delete();
    }
    this->phaser = svkPhaseSpec::New();
    if( this->model->DataExists("SpectroscopicData") ) {
        svkImageData* data = this->model->GetDataObject("SpectroscopicData");
        this->phaser->SetInput( data );
        int pivotPoint = data->GetCellData()->GetNumberOfTuples() / 2;
        this->phaser->SetLinearPhasePivot( pivotPoint );
        this->phasePivotEntry->GetWidget()->SetValueAsInt( pivotPoint );
        this->SetPhaseUpdateExtent();
    }
    this->phaseSlider->SetValue(0.0);
    this->linearPhaseSlider->SetValue(0.0);

}



void sivicPhaseWidget::UpdateProgress(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)
        ->GetProgressGauge()->SetValue( 100.0*(*(double*)(callData)) );
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->SetStatusText(
        static_cast<vtkAlgorithm*>(subject)->GetProgressText() 
    );

}

