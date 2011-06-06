/*
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/applications/sivic_app/src/sivicPreprocessingWidget.cc $
 *  $Rev: 936 $
 *  $Author: jccrane $
 *  $Date: 2011-06-03 11:41:20 -0700 (Fri, 03 Jun 2011) $
 */



#include <sivicPreprocessingWidget.h>
#include <vtkSivicController.h>

vtkStandardNewMacro( sivicPreprocessingWidget );
vtkCxxRevisionMacro( sivicPreprocessingWidget, "$Revision: 936 $");


/*! 
 *  Constructor
 */
sivicPreprocessingWidget::sivicPreprocessingWidget()
{
    this->phaseSlider = NULL;
    this->phaser = NULL;
    this->phaseAllVoxelsButton = NULL;
    this->phaseAllChannelsButton = NULL;
    this->fftButton = NULL;
    this->phaseButton = NULL;
    this->combineButton = NULL;
    this->phaseChangeInProgress = 0;
    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgress );
    this->progressCallback->SetClientData( (void*)this );


}


/*! 
 *  Destructor
 */
sivicPreprocessingWidget::~sivicPreprocessingWidget()
{

    if( this->phaseSlider != NULL ) {
        this->phaseSlider->Delete();
        this->phaseSlider = NULL;
    }

    if( this->phaser != NULL ) {
        this->phaser->Delete();
        this->phaser = NULL;
    }

    if( this->phaseAllVoxelsButton != NULL ) {
        this->phaseAllVoxelsButton->Delete();
        this->phaseAllVoxelsButton= NULL;
    }

    if( this->phaseAllChannelsButton != NULL ) {
        this->phaseAllChannelsButton->Delete();
        this->phaseAllChannelsButton= NULL;
    }

    if( this->fftButton != NULL ) {
        this->fftButton->Delete();
        this->fftButton= NULL;
    }

    if( this->phaseButton != NULL ) {
        this->phaseButton->Delete();
        this->phaseButton= NULL;
    }

    if( this->combineButton != NULL ) {
        this->combineButton->Delete();
        this->combineButton= NULL;
    }


}


/*! 
 *  Method in superclass to be overriden to add our custom widgets.
 */
void sivicPreprocessingWidget::CreateWidget()
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
    this->phaseSlider->SetLength( 200 );
    this->phaseSlider->SetOrientationToHorizontal();
    this->phaseSlider->SetLabelText("TBD");
    this->phaseSlider->SetValue(0);
    this->phaseSlider->SetRange( -180, 180 );
    this->phaseSlider->SetBalloonHelpString("Adjusts the phase of the spectroscopic data.");
    this->phaseSlider->EnabledOff();
    this->phaseSlider->SetEntryPositionToRight();
    this->phaseSlider->SetLabelPositionToLeft();

    this->phaser = svkPhaseSpec::New();
    this->phaser->SetChannel(0);
    this->phaseAllVoxelsButton = vtkKWCheckButton::New();
    this->phaseAllVoxelsButton->SetParent(this);
    this->phaseAllVoxelsButton->Create();
    this->phaseAllVoxelsButton->EnabledOff();
    this->phaseAllVoxelsButton->SetPadX(2);
    this->phaseAllVoxelsButton->SetText("TBD");
    this->phaseAllVoxelsButton->SelectedStateOn();

    this->phaseAllChannelsButton = vtkKWCheckButton::New();
    this->phaseAllChannelsButton->SetParent(this);
    this->phaseAllChannelsButton->Create();
    this->phaseAllChannelsButton->EnabledOff();
    this->phaseAllChannelsButton->SetPadX(2);
    this->phaseAllChannelsButton->SetText("TBD");
    this->phaseAllChannelsButton->SelectedStateOff();
    
    this->fftButton = vtkKWPushButton::New();
    this->fftButton->SetParent( this );
    this->fftButton->Create( );
    this->fftButton->EnabledOff();
    this->fftButton->SetText( "Apodization placeholder");
    this->fftButton->SetBalloonHelpString("Prototype Single Voxel FFT.");

    this->phaseButton = vtkKWPushButton::New();
    this->phaseButton->SetParent( this );
    this->phaseButton->Create( );
    this->phaseButton->EnabledOff();
    this->phaseButton->SetText( "zero-filling placeholder");
    this->phaseButton->SetBalloonHelpString("Prototype Auto Phasing.");

    this->combineButton = vtkKWPushButton::New();
    this->combineButton->SetParent( this );
    this->combineButton->Create( );
    this->combineButton->EnabledOff();
    //this->combineButton->EnabledOn();
    this->combineButton->SetText( "tbd");
    this->combineButton->SetBalloonHelpString("Prototype Multi-Coil Combination.");

    this->Script("grid %s -row 0 -column 0 -columnspan 2 -sticky nsew", this->phaseSlider->GetWidgetName() );
    this->Script("grid %s -row 1 -column 0 -sticky nsew", this->phaseAllVoxelsButton->GetWidgetName() );
    this->Script("grid %s -row 1 -column 1 -sticky nsew", this->phaseAllChannelsButton->GetWidgetName() );
    this->Script("grid %s -row 2 -column 0 -columnspan 2 -sticky nsew", this->fftButton->GetWidgetName() );
    this->Script("grid %s -row 3 -column 0 -columnspan 2 -sticky nsew", this->phaseButton->GetWidgetName() );
    this->Script("grid %s -row 4 -column 0 -columnspan 2 -sticky nsew", this->combineButton->GetWidgetName() );

    this->Script("grid rowconfigure %s 0  -weight 16", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 1  -weight 16", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 2  -weight 16", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 3  -weight 16", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 4  -weight 16", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 0 -weight 200 -uniform 1 -minsize 100", this->GetWidgetName() );

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
        this->phaseAllVoxelsButton, vtkKWCheckButton::SelectedStateChangedEvent );
    this->AddCallbackCommandObserver(
        this->phaseAllChannelsButton, vtkKWCheckButton::SelectedStateChangedEvent );
    this->AddCallbackCommandObserver(
        this->fftButton, vtkKWPushButton::InvokedEvent );
    this->AddCallbackCommandObserver(
        this->phaseButton, vtkKWPushButton::InvokedEvent );
    this->AddCallbackCommandObserver(
        this->combineButton, vtkKWPushButton::InvokedEvent );


}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicPreprocessingWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
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
    } else if( caller == this->phaseAllChannelsButton && event == vtkKWCheckButton::SelectedStateChangedEvent) {
        this->SetPhaseUpdateExtent();
    } else if( caller == this->phaseAllVoxelsButton && event == vtkKWCheckButton::SelectedStateChangedEvent) {
        this->SetPhaseUpdateExtent();
    } else if( caller == this->fftButton && event == vtkKWPushButton::InvokedEvent ) {
        this->ExecuteRecon();
    } else if( caller == this->phaseButton && event == vtkKWPushButton::InvokedEvent ) {
        this->ExecutePhase();
    } else if( caller == this->combineButton && event == vtkKWPushButton::InvokedEvent ) {
        this->ExecuteCombine();
    }
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}


/*!
 *  Sets the correct update extent for phasing
 */
void sivicPreprocessingWidget::SetPhaseUpdateExtent()
{

    return; 
/*
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
    } else {
        this->phaser->SetChannel( this->plotController->GetChannel() );
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
*/
}


/*!
 *  Updates/Adds keyboard bindings to the phase slider when it is in focus.
 */
void sivicPreprocessingWidget::UpdatePhaseSliderBindings()
{

    return; 

/*
    stringstream increment;
    stringstream decrement;
    increment << "SetValue " << this->phaseSlider->GetValue() + this->phaseSlider->GetResolution();
    decrement << "SetValue " << this->phaseSlider->GetValue() - this->phaseSlider->GetResolution();
    this->phaseSlider->RemoveBinding( "<Left>");
    this->phaseSlider->AddBinding( "<Left>", this->phaseSlider, decrement.str().c_str() );
    this->phaseSlider->RemoveBinding( "<Right>");
    this->phaseSlider->AddBinding( "<Right>", this->phaseSlider, increment.str().c_str() );
    this->phaseSlider->Focus(); 
*/
}


/*!
 *  Executes the FFT in place.
 */
void sivicPreprocessingWidget::ExecuteFFT() 
{

    return; 

/*
    svkImageData* data = this->model->GetDataObject("SpectroscopicData");
    if( data != NULL ) {
        // We'll turn the renderer off to avoid rendering intermediate steps
        this->plotController->GetView()->TurnRendererOff(svkPlotGridView::PRIMARY);
        svkMrsImageFFT* imageFFT = svkMrsImageFFT::New();
        imageFFT->SetInput( data );
        imageFFT->Update();
        data->Modified();
        imageFFT->Delete();
        bool useFullFrequencyRange = 1;
        bool useFullAmplitudeRange = 1;
        bool resetAmplitude = 1;
        bool resetFrequency = 1;
        this->sivicController->ResetRange( useFullFrequencyRange, useFullAmplitudeRange, 
                                           resetAmplitude, resetFrequency );
        this->sivicController->EnableWidgets( );
        this->plotController->GetView()->TurnRendererOn(svkPlotGridView::PRIMARY);
        this->plotController->GetView()->Refresh();
    }
*/
}


/*!
 *  Executes the Recon.
 */
void sivicPreprocessingWidget::ExecuteRecon() 
{

    return; 
/*
    svkImageData* data = this->model->GetDataObject("SpectroscopicData");
    if( data != NULL ) {

        svkMrsImageFFT* spatialRFFT = svkMrsImageFFT::New();

        spatialRFFT->SetInput( data );
        spatialRFFT->SetFFTDomain( svkMrsImageFFT::SPATIAL );
        spatialRFFT->SetFFTMode( svkMrsImageFFT::REVERSE );
        spatialRFFT->SetPreCorrectCenter( true );
        double preShiftWindow[3] = {-0.5, -0.5, -0.5 };
        //double preShiftWindow[3] = {-0.5, -1.0, -0.5 };
        double postShiftWindow[3] = {-0.5, -0.5, -0.5 };
        //spatialRFFT->SetPrePhaseShift( -0.5 );
        spatialRFFT->SetPrePhaseShift(preShiftWindow );
        spatialRFFT->SetPostCorrectCenter( true );
        //spatialRFFT->SetPostPhaseShift( -0.5 );
        spatialRFFT->SetPostPhaseShift( postShiftWindow );
        spatialRFFT->AddObserver(vtkCommand::ProgressEvent, progressCallback);
        this->GetApplication()->GetNthWindow(0)->SetStatusText("Executing Spatial Recon...");
        spatialRFFT->Update();
        spatialRFFT->RemoveObserver( progressCallback );

        svkMrsImageFFT* spectralFFT = svkMrsImageFFT::New();
        spectralFFT->AddObserver(vtkCommand::ProgressEvent, progressCallback);
        this->GetApplication()->GetNthWindow(0)->SetStatusText("Executing FFT...");
        spectralFFT->SetInput( spatialRFFT->GetOutput() );
        spectralFFT->SetFFTDomain( svkMrsImageFFT::SPECTRAL );
        spectralFFT->SetFFTMode( svkMrsImageFFT::FORWARD );
        spectralFFT->Update();
        data->Modified();
        data->Update();
        spectralFFT->RemoveObserver( progressCallback);
        

        bool useFullFrequencyRange = 1;
        bool useFullAmplitudeRange = 1;
        bool resetAmplitude = 1;
        bool resetFrequency = 1;
        //this->sivicController->ResetRange( useFullFrequencyRange, useFullAmplitudeRange, 
        //                                   resetAmplitude, resetFrequency );
        string stringFilename = "Result";
        this->sivicController->OpenSpectra( data, stringFilename);
        this->sivicController->EnableWidgets( );

        // We are resetting the input to make sure the actors get updated

        //this->plotController->SetInput(data);
        //this->plotController->GetView()->TurnRendererOn(svkPlotGridView::PRIMARY);
        //this->plotController->GetView()->Refresh();
        spatialRFFT->Delete();
        spectralFFT->Delete();
        this->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 0.0 );
        this->GetApplication()->GetNthWindow(0)->SetStatusText(" Done ");
    }
*/
}


/*!
 *  Executes the Phasing.
 */
void sivicPreprocessingWidget::ExecutePhase() 
{

    return; 

/*
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
        this->sivicController->OpenSpectra( data, stringFilename);
        this->sivicController->EnableWidgets( );
        //this->sivicController->ResetRange( useFullFrequencyRange, useFullAmplitudeRange, 
        //                                   resetAmplitude, resetFrequency );
        this->plotController->GetView()->TurnRendererOn(svkPlotGridView::PRIMARY);
        this->plotController->GetView()->Refresh();
        this->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 0.0 );
        this->GetApplication()->GetNthWindow(0)->SetStatusText(" Done ");
    }
*/
}



/*!
 *  Executes the combining of the channels.
 */
void sivicPreprocessingWidget::ExecuteCombine() 
{

    return; 

/*
    svkImageData* data = this->model->GetDataObject("SpectroscopicData");
    if( data != NULL ) {
        // We'll turn the renderer off to avoid rendering intermediate steps
        this->plotController->GetView()->TurnRendererOff(svkPlotGridView::PRIMARY);
        svkCoilCombine* coilCombine = svkCoilCombine::New();
        coilCombine->SetInput( data );
        //coilCombine->SetCombinationDimension( svkCoilCombine::TIME );  for combining time points
        //coilCombine->SetCombinationMethod( svkCoilCombine::SUM_OF_SQUARES );  for combining as magnitude data 
        coilCombine->Update();
        data->Modified();
        coilCombine->Delete();
        bool useFullFrequencyRange = 0;
        bool useFullAmplitudeRange = 1;
        bool resetAmplitude = 1;
        bool resetFrequency = 0;
        this->sivicController->ResetRange( useFullFrequencyRange, useFullAmplitudeRange, 
                                           resetAmplitude, resetFrequency );
        this->sivicController->ResetChannel( );
        string stringFilename = "CombinedData";
        this->sivicController->OpenSpectra( data, stringFilename);
        this->plotController->GetView()->TurnRendererOn(svkPlotGridView::PRIMARY);
        this->plotController->GetView()->Refresh();
    }
*/
}


void sivicPreprocessingWidget::UpdateProgress(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 100.0*(*(double*)(callData)) );
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->SetStatusText(
                  static_cast<vtkAlgorithm*>(subject)->GetProgressText() );

}

