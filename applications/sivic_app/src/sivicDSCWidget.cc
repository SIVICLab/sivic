/*
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/applications/sivic_app/src/sivicDSCWidget.cc $
 *  $Rev: 1112 $
 *  $Author: beckn8tor $
 *  $Date: 2011-10-27 18:00:03 -0700 (Thu, 27 Oct 2011) $
 */



#include <sivicDSCWidget.h>
#include <vtkSivicController.h>

vtkStandardNewMacro( sivicDSCWidget );
vtkCxxRevisionMacro( sivicDSCWidget, "$Revision: 1112 $");


/*! 
 *  Constructor
 */
sivicDSCWidget::sivicDSCWidget()
{

    this->dscRepresentationSelector = NULL;
    this->applyButton = NULL;

    this->dscLabel = NULL;

    this->dscRep = NULL;

    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgress );
    this->progressCallback->SetClientData( (void*)this );

}


/*! 
 *  Destructor
 */
sivicDSCWidget::~sivicDSCWidget()
{

    if( this->dscRepresentationSelector != NULL ) {
        this->dscRepresentationSelector->Delete();
        this->dscRepresentationSelector= NULL;
    }

    if( this->applyButton != NULL ) {
        this->applyButton->Delete();
        this->applyButton= NULL;
    }

    if( this->dscLabel != NULL ) {
        this->dscLabel->Delete();
        this->dscLabel = NULL;
    }

    if( this->dscRep != NULL ) {
        this->dscRep->Delete();
        this->dscRep = NULL;
    }

}


/*! 
 *  Method in superclass to be overriden to add our custom widgets.
 */
void sivicDSCWidget::CreateWidget()
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

    //  =================================== 
    //  DSC Representation Selector
    //  =================================== 

    int labelWidth = 0;
    this->dscRepresentationSelector = vtkKWMenuButton::New();
    this->dscRepresentationSelector->SetParent(this);
    this->dscRepresentationSelector->Create();
    this->dscRepresentationSelector->EnabledOn();

    vtkKWMenu* representationMenu = this->dscRepresentationSelector->GetMenu();
    

    string zfOption1 = "T2*";
    string zfOption2 = "DR2*";
    string invocationString;

    invocationString = "SetDSCRepresentationCallback 0"; 
    representationMenu->AddRadioButton(
            zfOption1.c_str(), 
            this->sivicController, invocationString.c_str());
    invocationString = "SetDSCRepresentationCallback 1"; 
    representationMenu->AddRadioButton(
            zfOption2.c_str(), 
            this->sivicController, invocationString.c_str());


    //  Set default values
    this->dscRepresentationSelector->SetValue( zfOption1.c_str() );

    this->applyButton = vtkKWPushButton::New();
    this->applyButton->SetParent( this );
    this->applyButton->Create( );
    this->applyButton->SetText( "DSC Magnitude");
    this->applyButton->SetBalloonHelpString("Apply.");
    this->applyButton->EnabledOn();


    // Titles

    this->Script("grid %s -row %d -column 0 -sticky nwse -padx 2 -pady 1", this->dscRepresentationSelector->GetWidgetName(), 0);
    this->Script("grid %s -row %d -column 1 -sticky nwse -padx 2 -pady 1", this->applyButton->GetWidgetName(), 1);

    this->Script("grid rowconfigure %s 0 -weight 1", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 1 -weight 1", this->GetWidgetName() );
    //this->Script("grid rowconfigure %s 2 -weight 0", this->GetWidgetName() );
    //this->Script("grid rowconfigure %s 3 -weight 0", this->GetWidgetName() );

    this->Script("grid columnconfigure %s 0 -weight 1 ", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 1 -weight 1 -uniform 1 -minsize 84", this->GetWidgetName() );
    //this->Script("grid columnconfigure %s 2 -weight 1 -uniform 1 -minsize 84", this->GetWidgetName() );
    //this->Script("grid columnconfigure %s 3 -weight 1 -uniform 1 -minsize 84", this->GetWidgetName() );
    //this->Script("grid columnconfigure %s 4 -weight 1 -uniform 1 -minsize 84", this->GetWidgetName() );

    this->AddCallbackCommandObserver( this->applyButton, vtkKWPushButton::InvokedEvent );
    this->AddCallbackCommandObserver(
        this->dscRepresentationSelector, vtkKWMenu::MenuItemInvokedEvent);


}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicDSCWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    // Respond to a selection change in the overlay view
    cout << "TEST" << endl;
    if( caller == this->dscRepresentationSelector && event == vtkKWPushButton::InvokedEvent ) {
        this->ExecuteDSC();
    }
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}


void sivicDSCWidget::SetDSCRepresentationCallback( svkDSCDeltaR2::representation representation)
{
    cout << "Change Representation" << endl;
    svkImageData* data = this->model->GetDataObject("AnatomicalData");

    if( data != NULL ) {

        // We'll turn the renderer off to avoid rendering intermediate steps
        this->plotController->GetView()->TurnRendererOff(svkPlotGridView::PRIMARY);

        if (dscRep == NULL) {
            dscRep = svkDSCDeltaR2::New();
        }
        dscRep->SetInput( data );
        dscRep->SetRepresentation(representation);
        dscRep->Update();
        
        data->Modified();

        this->sivicController->ResetChannel( );
        string stringFilename = "DSCdData";
        this->sivicController->Open4DImage( data, stringFilename);
        this->plotController->GetView()->TurnRendererOn(svkPlotGridView::PRIMARY);
        this->plotController->GetView()->Refresh();
    }
}

/*!
 *  Executes the combining of the channels.
 */
void sivicDSCWidget::ExecuteDSC() 
{

    cout << "EXECUTE" << endl;
    svkImageData* data = this->model->GetDataObject("SpectroscopicData");

    if( data != NULL ) {

        // We'll turn the renderer off to avoid rendering intermediate steps
        this->plotController->GetView()->TurnRendererOff(svkPlotGridView::PRIMARY);

        if (dscRep == NULL) {
            dscRep = svkDSCDeltaR2::New();
        }
        dscRep->SetInput( data );


        dscRep->Update();
        data->Modified();

        bool useFullFrequencyRange = 0;
        bool useFullAmplitudeRange = 1;
        bool resetAmplitude = 1;
        bool resetFrequency = 0;
        this->sivicController->ResetRange( useFullFrequencyRange, useFullAmplitudeRange,
                                           resetAmplitude, resetFrequency );
        this->sivicController->ResetChannel( );
        string stringFilename = "DSCdData";
        this->sivicController->Open4DImage( data, stringFilename);
        this->plotController->GetView()->TurnRendererOn(svkPlotGridView::PRIMARY);
        this->plotController->GetView()->Refresh();
    }

    return; 

}


void sivicDSCWidget::UpdateProgress(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 100.0*(*(double*)(callData)) );
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->SetStatusText(
                  static_cast<vtkAlgorithm*>(subject)->GetProgressText() );

}

