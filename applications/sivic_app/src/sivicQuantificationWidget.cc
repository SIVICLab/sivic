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
    this->quant = NULL;
    this->mapViewSelector = NULL;   
    this->numMets = 0; 
    this->isEnabled = false; 
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

    if( this->quant != NULL ) {
        this->quant->Delete();
        this->quant= NULL;
    }

    if( this->mapViewSelector != NULL ) {
        this->mapViewSelector ->Delete();
        this->mapViewSelector = NULL;
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

    this->numMets = 4; 
    this->metNames.push_back( "choline" ); 
    this->metNames.push_back( "creatine" ); 
    this->metNames.push_back( "NAA" ); 
    this->metNames.push_back( "lipid" ); 

    // this is a map of default metabolite names to ppm ranges: 
    this->metQuantMap["choline"].push_back(3.358); 
    this->metQuantMap["choline"].push_back(3.169); 
    this->metQuantMap["creatine"].push_back(3.154); 
    this->metQuantMap["creatine"].push_back(2.973); 
    this->metQuantMap["NAA"].push_back(2.164); 
    this->metQuantMap["NAA"].push_back(1.93); 
    this->metQuantMap["lipid"].push_back(1.439); 
    this->metQuantMap["lipid"].push_back(1.273); 


    //  Map View Selector
    this->mapViewSelector = vtkKWMenuButtonWithLabel::New();
    this->mapViewSelector->SetParent(this);
    this->mapViewSelector->Create();
    this->mapViewSelector->SetLabelText("Met Map View");
    this->mapViewSelector->SetLabelPositionToTop();
    this->mapViewSelector->SetPadY(5);
    this->mapViewSelector->EnabledOff();
    vtkKWMenu* mapViewMenu = this->mapViewSelector->GetWidget()->GetMenu();

    
    double rangeMin; 
    double rangeMax; 
    this->GetMRSFrequencyRange( rangeMin, rangeMax, svkSpecPoint::PPM); 

    stringstream invocation; 
    vtkstd::string mapSelectLabel;

    for ( int i = 0; i < this->numMets; i++ ) {
        this->metRangeVector.push_back( vtkKWRange::New() ); 
        this->metRangeVector[i]->SetParent(this);
        this->metRangeVector[i]->SetLabelPositionToLeft();
        this->metRangeVector[i]->SetBalloonHelpString("Adjusts freq range of metabolite.");
        this->metRangeVector[i]->SetWholeRange(rangeMin, rangeMax);
        this->metRangeVector[i]->Create();
        this->metRangeVector[i]->SetRange(rangeMin, rangeMax);
        this->metRangeVector[i]->EnabledOff();
        this->metRangeVector[i]->SetSliderSize(2);
        this->metRangeVector[i]->SetPadY(4);
        this->metRangeVector[i]->SetEntry1PositionToLeft();
        this->metRangeVector[i]->SetEntry2PositionToRight();
        this->metRangeVector[i]->SetEntriesWidth(4);
        this->metRangeVector[i]->SetResolution(.01);

        //  These are text labels for the range sliders
        this->metLabelVector.push_back( vtkKWLabel::New() );  
        this->metLabelVector[i]->SetText( (this->metNames[i]).c_str() );
        this->metLabelVector[i]->SetParent(this);
        this->metLabelVector[i]->SetHeight(1);
        this->metLabelVector[i]->SetPadX(0);
        this->metLabelVector[i]->SetPadY(0);
        this->metLabelVector[i]->SetJustificationToLeft();
        this->metLabelVector[i]->Create();

        ostringstream mapNumArea;
        mapNumArea <<  i * 2;
        mapSelectLabel = this->metNames[i] + "_area";
        invocation.str("");
        invocation << "MetMapViewCallback " << mapNumArea.str() << endl;
        mapViewMenu->AddRadioButton(mapSelectLabel.c_str(), this->sivicController, invocation.str().c_str());

        ostringstream mapNumHt;
        mapNumHt <<  (i * 2) + 1;
        mapSelectLabel = this->metNames[i] + "_ht";
        invocation.str("");
        invocation << "MetMapViewCallback " << mapNumHt.str() << endl;
        mapViewMenu->AddRadioButton(mapSelectLabel.c_str(), this->sivicController, invocation.str().c_str());

/*
        mapSelectLabel = this->metNames[i] + "_area"; 
        invocation.str("");
        invocation << "MetMapViewCallback " << mapSelectLabel << endl;
        mapViewMenu->AddRadioButton(mapSelectLabel.c_str(), this->sivicControlle, invocation.str().c_str());

        mapSelectLabel = this->metNames[i] + "_ht"; 
        invocation.str("");
        invocation << "MetMapViewCallback" << mapSelectLabel << endl;
        mapViewMenu->AddRadioButton(mapSelectLabel.c_str(), this->sivicController, invocation.str().c_str());
*/

    }
    mapSelectLabel = this->metNames[2] + "area"; 
    this->mapViewSelector->GetWidget()->SetValue( mapSelectLabel.c_str() );


    //  Generate button
    this->quantButton = vtkKWPushButton::New();
    this->quantButton->SetParent( this );
    this->quantButton->Create( );
    this->quantButton->EnabledOff();
    this->quantButton->SetText( "Generate Met. Maps");
    this->quantButton->SetBalloonHelpString("Prototype Metabolite Quantification ( peak ht and area ).");


    //  Format the GUI grid in this panel:
    for ( int i = 0; i < this->numMets; i++ ) {
        this->Script("grid %s -row %d -column 0 -sticky w", 
                        this->metLabelVector[i]->GetWidgetName(), i);
        this->Script("grid %s -row %d -column 1 -sticky wnse -padx 2", 
                        this->metRangeVector[i]->GetWidgetName(), i);
    };
    this->Script("grid %s -row %d -column 2 -rowspan 2 -padx 2", this->mapViewSelector->GetWidgetName(), 0);
    this->Script("grid %s -row %d -column 2 -padx 2", this->quantButton->GetWidgetName(), 2);


    for ( int i = 0; i < this->numMets; i++ ) {
        this->Script("grid rowconfigure %s %d  -weight 10", this->GetWidgetName(), i );
    }

    this->Script("grid columnconfigure %s 0 -weight 20 -uniform 1 -minsize 20", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 1 -weight 65 -uniform 1 -minsize 65", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 2 -weight 45 -uniform 1 -minsize 45", this->GetWidgetName() );

    //  Callbacks
    this->AddCallbackCommandObserver(
        this->quantButton, vtkKWPushButton::InvokedEvent 
    );

    this->AddCallbackCommandObserver(
        this->mapViewSelector->GetWidget(), vtkKWMenu::MenuItemInvokedEvent
    );

}


/*
 *  Gets the frequency range limits for range sliders in the specified units. 
 */
void sivicQuantificationWidget::GetMRSFrequencyRange( double& min, double& max, svkSpecPoint::UnitType units)
{
    min = 0.; 
    max = 0.; 
    if ( this->model != NULL ) {
        svkImageData* data = this->model->GetDataObject( "SpectroscopicData" );

        if( data != NULL ) {
            min = 1;
            max = data->GetCellData()->GetArray(0)->GetNumberOfTuples();
    
            //  If the domain is frequency, then convert from point space to target units.
            string domain = model->GetDataObject( "SpectroscopicData" )
                                    ->GetDcmHeader()->GetStringValue("SignalDomainColumns");
            if( domain == "FREQUENCY" ) {
                svkSpecPoint* point = svkSpecPoint::New();
                point->SetDcmHeader( data->GetDcmHeader() );
                min = point->ConvertPosUnits( min, svkSpecPoint::PTS, units ); 
                max = point->ConvertPosUnits( max, svkSpecPoint::PTS, units ); 
                point->Delete();
            }
        } 
    }
}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicQuantificationWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    if( caller == this->quantButton && event == vtkKWPushButton::InvokedEvent ) {
        this->ExecuteQuantification();
    } 
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);

}


/*!
 *  Executes the combining of the channels.
 *  Generates peak area and peak ht maps for each metabolite defined in GUI.    
 */
void sivicQuantificationWidget::ExecuteQuantification() 
{
    svkImageData* data = this->model->GetDataObject("SpectroscopicData");

    if( data != NULL ) {

        this->quant = svkExtractMRIFromMRS::New();
        this->quant->SetInput( data );

//generate pk ht, peak area and magnitude area met maps for the specified intervals
//save each in the data model and through a drop down select which one to view (load as overlay)

        double minValue;
        double maxValue;
        float peak;
        float width;
        svkMriImageData* tmp;

        //
        //  This is a vector of metabolite map names used in the svkDataModel: 
        //  The names should appear in the same order in the vector as they do 
        //  in the view selector 
        //
        int objectNumber= 0; 
        for (int i = 0; i < this->metRangeVector.size(); i++ ) {

            minValue = this->metRangeVector[i]->GetEntry1()->GetValueAsDouble();
            maxValue = this->metRangeVector[i]->GetEntry2()->GetValueAsDouble();
            peak  = static_cast< float > ( (maxValue + minValue)/2 );
            width = static_cast< float > ( fabs( ( maxValue - minValue ) ) ); 

            cout << "QUANT THIS ONE: " << this->metNames[i] << " " << peak << " " << width << endl;

            this->quant->SetPeakPosPPM( peak );
            this->quant->SetPeakWidthPPM( width );

            for (int quantMethod = 0; quantMethod < 2; quantMethod++) {

                //  Add met map to model 
                vtkstd::string modelDataName = this->metNames[i]; 
                if (quantMethod == 0) {
                    this->quant->SetSeriesDescription( this->metNames[i] + " area Metabolite Map" );
                    this->quant->SetAlgorithmToIntegrate(); 
                    modelDataName += "_area";
                } else if (quantMethod == 1) {
                    this->quant->SetSeriesDescription( this->metNames[i] + " peak ht Metabolite Map" );
                    this->quant->SetAlgorithmToPeakHeight(); 
                    modelDataName += "_ht";
                }
                this->modelMetNames.push_back( modelDataName ); 

                this->quant->Update();

                tmp = svkMriImageData::New();
                tmp->DeepCopy(this->quant->GetOutput());
                if( this->model->DataExists( this->modelMetNames[ objectNumber ] ) ) {
                    this->model->ChangeDataObject( this->modelMetNames[ objectNumber ], tmp); 
                } else {
                    this->model->AddDataObject( this->modelMetNames[ objectNumber ], tmp );
                }
                objectNumber++; 
            }
        }

        //  Initialize the overlay with the NAA met map
        this->SetOverlay( this->modelMetNames[2] ); 

        this->plotController->TurnPropOn( svkPlotGridView::OVERLAY_IMAGE );
        this->plotController->TurnPropOn( svkPlotGridView::OVERLAY_TEXT );
        this->plotController->SetOverlayOpacity( .5 );
        this->plotController->GetView()->Refresh();

        this->sivicController->EnableWidgets( );

    }

    this->mapViewSelector->EnabledOn();
}


/*!
 *  Called by parent controller to enable this panel and initialize values
 */
void sivicQuantificationWidget::SetOverlay( vtkstd::string modelObjectName)
{
    //  Initialize the overlay with the NAA met map
    if( this->model->DataExists( "MetaboliteData" ) ) {
        this->model->ChangeDataObject( "MetaboliteData", this->model->GetDataObject( modelObjectName ) );
    } else {
        this->model->AddDataObject( "MetaboliteData", this->model->GetDataObject(modelObjectName ));
    }

    this->plotController->SetInput( this->model->GetDataObject( modelObjectName ), svkPlotGridView::MET ); 
    this->overlayController->SetInput( this->model->GetDataObject( modelObjectName ), svkOverlayView::OVERLAY );
}


/*!
 *  Called by parent controller to enable this panel and initialize values
 */
void sivicQuantificationWidget::EnableWidgets()
{

    //  If this is the first time through, initialize the ranges to the default 
    //  values, otherwise leave them where the user set them. 
    if ( this->isEnabled == false ) {

        vtkstd::string metName;
        float metMin;
        float metMax;
        double rangeMin; 
        double rangeMax; 
        this->GetMRSFrequencyRange( rangeMin, rangeMax, svkSpecPoint::PPM); 

        for ( int i = 0; i < this->numMets; i++ ) {
            this->metRangeVector[i]->SetWholeRange(rangeMin, rangeMax);
            string metName = this->metNames[i]; 
            metMin = this->metQuantMap[metName][0]; 
            metMax = this->metQuantMap[metName][1]; 
            this->metRangeVector[i]->SetRange(metMin, metMax);
            this->metRangeVector[i]->EnabledOn();
        }

        this->quantButton->EnabledOn();

        this->isEnabled = true; 
    }
}


/*!
 *
 */
void sivicQuantificationWidget::UpdateProgress(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 100.0*(*(double*)(callData)) );
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->SetStatusText(
                  static_cast<vtkAlgorithm*>(subject)->GetProgressText() );

}

