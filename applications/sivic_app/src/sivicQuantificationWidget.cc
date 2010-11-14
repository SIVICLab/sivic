/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */



#include <sivicQuantificationWidget.h>
#include <vtkSivicController.h>


vtkStandardNewMacro( sivicQuantificationWidget );
vtkCxxRevisionMacro( sivicQuantificationWidget, "$Revision$");


/*! 
 *  Constructor
 */
sivicQuantificationWidget::sivicQuantificationWidget()
{
    this->quantButton = NULL;
    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgress );
    this->progressCallback->SetClientData( (void*)this );
}


/*! 
 *  Destructor
 */
sivicQuantificationWidget::~sivicQuantificationWidget()
{

    if( this->quantButton != NULL ) {
        this->quantButton->Delete();
        this->quantButton= NULL;
    }

}


/*! 
 *  Method in superclass to be overriden to add our custom widgets.
 */
void sivicQuantificationWidget::CreateWidget()
{
/*  This method will create our main window. The main window is a 
 *  vtkKWCompositeWidget with a vtkKWRendWidget. 
*/

    // Check if already created
    if ( this->IsCreated() )
    {
        vtkErrorMacro(<< this->GetClassName() << " already created");
        return;
    }

    // Call the superclass to create the composite widget container
    this->Superclass::CreateWidget();

    this->quantButton = vtkKWPushButton::New();
    this->quantButton->SetParent( this );
    this->quantButton->Create( );
    this->quantButton->EnabledOn();
    this->quantButton->SetText( "QUANT");
    this->quantButton->SetBalloonHelpString("Prototype Metabolite Quantification.");

    this->Script("grid %s -row 0 -column 0 -columnspan 2 -sticky nsew", this->quantButton->GetWidgetName() );
    this->Script("grid %s -row 1 -column 0 -sticky nsew", this->quantButton->GetWidgetName() );
    this->Script("grid %s -row 1 -column 1 -sticky nsew", this->quantButton->GetWidgetName() );
    this->Script("grid %s -row 2 -column 0 -columnspan 2 -sticky nsew", this->quantButton->GetWidgetName() );
    this->Script("grid %s -row 3 -column 0 -columnspan 2 -sticky nsew", this->quantButton->GetWidgetName() );
    this->Script("grid %s -row 4 -column 0 -columnspan 2 -sticky nsew", this->quantButton->GetWidgetName() );

    this->Script("grid rowconfigure %s 0  -weight 16", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 1  -weight 16", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 2  -weight 16", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 3  -weight 16", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 4  -weight 16", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 0 -weight 200 -uniform 1 -minsize 100", this->GetWidgetName() );

    this->AddCallbackCommandObserver(
        this->quantButton, vtkKWPushButton::InvokedEvent );

}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicQuantificationWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    // Respond to a selection change in the overlay view
    if (  caller == this->plotController->GetRWInteractor() && event == vtkCommand::SelectionChangedEvent ) {

        this->SetPhaseUpdateExtent();

    // Respond to a selection change in the plot grid view 
    } else if (  caller == this->overlayController->GetRWInteractor() && event == vtkCommand::SelectionChangedEvent ) {

        this->SetPhaseUpdateExtent();
    } else if( caller == this->quantButton && event == vtkKWPushButton::InvokedEvent ) {
        this->ExecuteCombine();
    }
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}


/*!
 *  Sets the correct update extent for phasing
 */
void sivicQuantificationWidget::SetPhaseUpdateExtent()
{
}


/*!
 *  Updates/Adds keyboard bindings to the phase slider when it is in focus.
 */
void sivicQuantificationWidget::UpdatePhaseSliderBindings()
{
}


/*!
 *  Executes the FFT in place.
 */
void sivicQuantificationWidget::ExecuteFFT() 
{
    svkImageData* data = this->model->GetDataObject("SpectroscopicData");
    if( data != NULL ) {
        // We'll turn the renderer off to avoid rendering intermediate steps
        this->plotController->GetView()->TurnRendererOff(svkPlotGridView::PRIMARY);
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

}


/*!
 *  Executes the Recon.
 */
void sivicQuantificationWidget::ExecuteRecon() 
{
    svkImageData* data = this->model->GetDataObject("SpectroscopicData");
    if( data != NULL ) {

    }

}


/*!
 *  Executes the Phasing.
 */
void sivicQuantificationWidget::ExecutePhase() 
{
    svkImageData* data = this->model->GetDataObject("SpectroscopicData");
    if( data != NULL ) {
    }
}



/*!
 *  Executes the combining of the channels.
 */
void sivicQuantificationWidget::ExecuteCombine() 
{
    svkImageData* data = this->model->GetDataObject("SpectroscopicData");
    if( data != NULL ) {
    }
}


void sivicQuantificationWidget::UpdateProgress(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 100.0*(*(double*)(callData)) );
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->SetStatusText(
                  static_cast<vtkAlgorithm*>(subject)->GetProgressText() );

}

