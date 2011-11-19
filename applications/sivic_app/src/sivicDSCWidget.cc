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
    this->generateMapsButton = NULL;
    this->mapViewSelector = NULL;

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

    if( this->generateMapsButton != NULL ) {
        this->generateMapsButton->Delete();
        this->generateMapsButton= NULL;
    }

    if( this->dscRep != NULL ) {
        this->dscRep->Delete();
        this->dscRep = NULL;
    }

    if( this->mapViewSelector != NULL ) {
        this->mapViewSelector ->Delete();
        this->mapViewSelector = NULL;
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
    this->dscRepresentationSelector = vtkKWMenuButtonWithLabel::New();
    this->dscRepresentationSelector->SetParent(this);
    this->dscRepresentationSelector->Create();
    this->dscRepresentationSelector->SetLabelText("Select DSC Representation");
    this->dscRepresentationSelector->SetLabelPositionToTop();
    this->dscRepresentationSelector->EnabledOn();

    vtkKWMenu* representationMenu = this->dscRepresentationSelector->GetWidget()->GetMenu();
    //representationMenu->SetFont("times 24 bold");
    

    string zfOption1 = "T2* - susceptibility";
    string zfOption2 = "Delta R2* - relative Gd concentration";
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
    this->dscRepresentationSelector->GetWidget()->SetValue( zfOption1.c_str() );


    this->generateMapsButton = vtkKWPushButton::New();
    this->generateMapsButton->SetParent( this );
    this->generateMapsButton->Create( );
    this->generateMapsButton->SetText( "Generate DSC Maps");
    this->generateMapsButton->SetBalloonHelpString("Apply.");
    this->generateMapsButton->EnabledOn();

    //  Set DSC map names:
    this->dscNames.push_back( "DSC_Peak_Ht" );


    //  Map View Selector
    this->mapViewSelector = vtkKWMenuButtonWithLabel::New();
    this->mapViewSelector->SetParent(this);
    this->mapViewSelector->Create();
    this->mapViewSelector->SetLabelText("Select Map");
    this->mapViewSelector->SetLabelPositionToTop();
    this->mapViewSelector->SetPadY(2);
    this->mapViewSelector->GetWidget()->SetWidth(7);
    this->mapViewSelector->EnabledOff();
    this->mapViewSelector->GetLabel()->SetFont("system 8");
    this->mapViewSelector->GetWidget()->SetFont("system 8");
    vtkKWMenu* mapViewMenu = this->mapViewSelector->GetWidget()->GetMenu();
    mapViewMenu->SetFont("system 8");


    //  Set default value
    vtkstd::string mapSelectLabel = this->dscNames[0];
    this->mapViewSelector->GetWidget()->SetValue( mapSelectLabel.c_str() );
    this->mapViewSelector->GetWidget()->IndicatorVisibilityOn();



    //  Layout 

    this->Script("grid %s -row %d -column 0 -sticky ew -padx 10 -pady 1", this->dscRepresentationSelector->GetWidgetName(), 0);

    this->Script("grid %s -row %d -column 1 -sticky ew -padx 10 -pady 1", this->mapViewSelector->GetWidgetName(), 0);
    this->Script("grid %s -row %d -column 1 -sticky ew -padx 10 -pady 1", this->generateMapsButton->GetWidgetName(), 1);

    this->Script("grid rowconfigure %s 0 -weight 1", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 1 -weight 1", this->GetWidgetName() );

    this->Script("grid columnconfigure %s 0 -weight 1 ", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 1 -weight 1 -uniform 1 -minsize 84", this->GetWidgetName() );

    this->AddCallbackCommandObserver( this->generateMapsButton, vtkKWPushButton::InvokedEvent );
    this->AddCallbackCommandObserver(
        this->dscRepresentationSelector, vtkKWMenu::MenuItemInvokedEvent);


}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicDSCWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    // Respond to a selection change in the overlay view
    if( caller == this->dscRepresentationSelector && event == vtkKWPushButton::InvokedEvent ) {
        this->ExecuteDSC();
    }
    if( caller == this->generateMapsButton && event == vtkKWPushButton::InvokedEvent ) {
        this->ExecuteQuantification();
    }

    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}


void sivicDSCWidget::SetDSCRepresentationCallback( svkDSCDeltaR2::representation representation)
{
    svkImageData* data = this->model->GetDataObject("AnatomicalData");

    if( data != NULL ) {

        // We'll turn the renderer off to avoid rendering intermediate steps
        this->plotController->GetView()->TurnRendererOff(svkPlotGridView::PRIMARY);

        if (dscRep == NULL) {
            dscRep = svkDSCDeltaR2::New();
        }
        dscRep->SetInput( data );
        dscRep->SetRepresentation(representation);
        data->Modified();
        dscRep->Update();
        

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


/*!
 *  Executes the generation of DSC maps. 
 */
void sivicDSCWidget::ExecuteQuantification()
{

    svkImageData* data = this->model->GetDataObject("AnatomicalData");
    vtkKWMenu* mapViewMenu = this->mapViewSelector->GetWidget()->GetMenu();

    if( data != NULL ) {

        this->dscQuant = svkQuantifyDSC::New();
        this->dscQuant->SetInput( data );
        this->dscQuant->Update();
        vtkstd::vector< svkMriImageData* >* dscMaps = this->dscQuant->GetDSCMaps();

        for (int i = 0; i < dscMaps->size(); i ++ ) {

            //
            //  This is a vector of DSC map names used in the svkDataModel:
            //  The names should appear in the same order in the vector as they do
            //  in the view selector
            //
            vtkstd::string modelDataName = (*dscMaps)[i]->GetDcmHeader()->GetStringValue("SeriesDescription");
            this->modelDSCNames.push_back( modelDataName );
 
            if( this->model->DataExists( this->modelDSCNames[i] ) ) {
                this->model->ChangeDataObject( this->modelDSCNames[ i ], (*dscMaps)[i]);  
            } else {
                this->model->AddDataObject( this->modelDSCNames[ i ], (*dscMaps)[i]);

                //  Add label to menu:
                ostringstream mapNum;
                mapNum <<  i;
                stringstream invocation;
                invocation.str("");
                invocation << "DSCMapViewCallback " << mapNum.str() << endl;

                mapViewMenu->AddRadioButton(modelDataName.c_str(), this->sivicController, invocation.str().c_str());
            }
 
        }

        //  if overlay has not been initialized, the overlay with the first met map
        //  otherwise grab the current menu value and use that to init the overlay
        if( this->model->DataExists( "MetaboliteData" ) == false ) {
            vtkKWMenu* mapViewMenu = this->mapViewSelector->GetWidget()->GetMenu();
            this->mapViewSelector->GetWidget()->SetValue( this->modelDSCNames[0].c_str() );
            this->SetOverlay( this->modelDSCNames[0] );
        }  else {
            vtkKWMenu* mapViewMenu = this->mapViewSelector->GetWidget()->GetMenu();
            vtkstd::string menuValue = this->mapViewSelector->GetWidget()->GetValue();
            if( this->model->DataExists( menuValue )) {
                this->SetOverlay(  this->mapViewSelector->GetWidget()->GetValue()  );
            } else {
                this->mapViewSelector->GetWidget()->SetValue( this->modelDSCNames[0].c_str() );
                this->SetOverlay( this->modelDSCNames[0] );
            }
        }

        this->plotController->GetView()->Refresh();

        this->sivicController->EnableWidgets( );

    }

    this->mapViewSelector->EnabledOn();

}


/*!
 *  Called by parent controller to enable this panel and initialize values
 *  modelObjectName is the series description of the map
 */
void sivicDSCWidget::SetOverlay( vtkstd::string modelObjectName)
{
    //  Initialize the overlay with the NAA met map
    if( this->model->DataExists( "MetaboliteData" ) ) {
        this->model->ChangeDataObject( "MetaboliteData", this->model->GetDataObject( modelObjectName ) );
    } else {
        this->model->AddDataObject( "MetaboliteData", this->model->GetDataObject(modelObjectName ));
    }
    if( this->model->DataExists( "OverlayData" ) ) {
        this->model->ChangeDataObject( "OverlayData", this->model->GetDataObject( modelObjectName ) );
    } else {
        this->model->AddDataObject( "OverlayData", this->model->GetDataObject(modelObjectName ));
    }
    string tmpFilename = "temporaryFile";
    if( this->model->DataExists( "AnatomicalData" ) ) {
        this->sivicController->OpenOverlay(this->model->GetDataObject(modelObjectName), tmpFilename );
    } else {
        this->plotController->SetInput( this->model->GetDataObject( modelObjectName ), svkPlotGridView::MET );
        this->overlayController->SetInput( this->model->GetDataObject( modelObjectName ), svkOverlayView::OVERLAY );
   }
}



void sivicDSCWidget::UpdateProgress(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 100.0*(*(double*)(callData)) );
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->SetStatusText(
                  static_cast<vtkAlgorithm*>(subject)->GetProgressText() );

}

