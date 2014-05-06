/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */



#include <sivicCombineWidget.h>
#include <vtkSivicController.h>

vtkStandardNewMacro( sivicCombineWidget );
vtkCxxRevisionMacro( sivicCombineWidget, "$Revision$");


/*! 
 *  Constructor
 */
sivicCombineWidget::sivicCombineWidget()
{

    this->magnitudeCombinationButton = NULL;
    this->additionCombinationButton = NULL;

    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgress );
    this->progressCallback->SetClientData( (void*)this );

}


/*! 
 *  Destructor
 */
sivicCombineWidget::~sivicCombineWidget()
{

    if( this->magnitudeCombinationButton != NULL ) {
        this->magnitudeCombinationButton->Delete();
        this->magnitudeCombinationButton= NULL;
    }

    if( this->additionCombinationButton != NULL ) {
        this->additionCombinationButton->Delete();
        this->additionCombinationButton= NULL;
    }

}


/*! 
 *  Method in superclass to be overriden to add our custom widgets.
 */
void sivicCombineWidget::CreateWidget()
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

    this->magnitudeCombinationButton = vtkKWPushButton::New();
    this->magnitudeCombinationButton->SetParent( this );
    this->magnitudeCombinationButton->Create( );
    this->magnitudeCombinationButton->SetText( "Combine Magnitude");
    this->magnitudeCombinationButton->SetBalloonHelpString("Combine channels using magnitude.");
    this->magnitudeCombinationButton->EnabledOff();

    this->additionCombinationButton = vtkKWPushButton::New();
    this->additionCombinationButton->SetParent( this );
    this->additionCombinationButton->Create( );
    this->additionCombinationButton->SetText( "Simple Summation");
    this->additionCombinationButton->SetBalloonHelpString("Combine channels by summation.");
    this->additionCombinationButton->EnabledOff();

    this->Script("grid %s -row %d -column %d -pady 3 -sticky we -padx 4 -pady 1", this->additionCombinationButton->GetWidgetName(), 1, 0 );
    this->Script("grid %s -row %d -column %d -pady 3 -sticky we -padx 4 -pady 1", this->magnitudeCombinationButton->GetWidgetName(), 1, 1 );

    this->Script("grid rowconfigure %s 0  -weight 1", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 1  -weight 1", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 2  -weight 1", this->GetWidgetName() );

    this->Script("grid columnconfigure %s 0 -weight 1", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 1 -weight 1", this->GetWidgetName() );

    this->AddCallbackCommandObserver( this->additionCombinationButton, vtkKWPushButton::InvokedEvent );
    this->AddCallbackCommandObserver( this->magnitudeCombinationButton, vtkKWPushButton::InvokedEvent );

}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicCombineWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    // Respond to a selection change in the overlay view
    if( caller == this->magnitudeCombinationButton && event == vtkKWPushButton::InvokedEvent ) {
        this->ExecuteCombine(svkMRSCombine::SUM_OF_SQUARES,svkMRSCombine::COIL);
    } else if( caller == this->additionCombinationButton && event == vtkKWPushButton::InvokedEvent ) {
        this->ExecuteCombine(svkMRSCombine::ADDITION,svkMRSCombine::COIL);
    }
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}


/*!
 *  Executes the combining of the channels.
 */
void sivicCombineWidget::ExecuteCombine(svkMRSCombine::CombinationMethod method, svkMRSCombine::CombinationDimension dimension )
{

    svkImageData* data = this->model->GetDataObject("SpectroscopicData");

    if( data != NULL ) {

        // We'll turn the renderer off to avoid rendering intermediate steps
        this->plotController->GetView()->TurnRendererOff(svkPlotGridView::PRIMARY);

        svkMRSCombine* coilCombine = svkMRSCombine::New();
        coilCombine->SetInput( data );

        coilCombine->SetCombinationDimension( dimension );  //for combining time points
        coilCombine->SetCombinationMethod( method );  //for combining as magnitude data

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
        this->sivicController->Open4DImage( data, stringFilename);
        this->plotController->GetView()->TurnRendererOn(svkPlotGridView::PRIMARY);
        this->plotController->GetView()->Refresh();
    }

    return; 

}


void sivicCombineWidget::UpdateProgress(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 100.0*(*(double*)(callData)) );
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->SetStatusText(
                  static_cast<vtkAlgorithm*>(subject)->GetProgressText() );

}

