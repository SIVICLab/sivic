/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */



#include <sivicQuantificationWidget.h>
#include <svkImageCopy.h>
#include <vtkSivicController.h>
#include <vtkImageMathematics.h>


vtkStandardNewMacro( sivicQuantificationWidget );
vtkCxxRevisionMacro( sivicQuantificationWidget, "$Revision$");


/*! 
 *  Constructor
 */
sivicQuantificationWidget::sivicQuantificationWidget()
{

    this->quantButton = NULL;
    this->mrsQuant = NULL;
    this->mapViewSelector = NULL;   
    this->numMets = 0; 
    this->isEnabled = false; 
    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgress );
    this->progressCallback->SetClientData( (void*)this );
    this->units = svkSpecPoint::PPM;
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

    if( this->mrsQuant != NULL ) {
        this->mrsQuant->Delete();
        this->mrsQuant= NULL;
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

    this->mrsQuant = svkQuantifyMetabolites::New();

}


/*
 *  Gets the frequency range limits for range sliders in the specified units. If peakStart and peakEnd
 *  are provided, the range is relative to those parameters.  Otherwise (peakStart, peakEnd < 0 ) the 
 *  full range is returned. 
 */
void sivicQuantificationWidget::GetMRSFrequencyRange( double peakStart, double peakEnd, double& min, double& max, svkSpecPoint::UnitType units)
{
    min = 0.; 
    max = 0.; 

    if ( this->model != NULL ) {

        svkImageData* data = this->model->GetDataObject( "SpectroscopicData" );

        if( data != NULL ) {

            //  Max allowable range: 
            min = 1;
            max = data->GetCellData()->GetArray(0)->GetNumberOfTuples();

            svkSpecPoint* point = svkSpecPoint::New();
            point->SetDcmHeader( data->GetDcmHeader() );
            double peakStartPt = point->ConvertPosUnits( peakStart, units, svkSpecPoint::PTS ); 
            double peakEndPt   = point->ConvertPosUnits( peakEnd, units, svkSpecPoint::PTS ); 

            double deltaPts    = peakEndPt - peakStartPt;  
            peakStartPt -= 3 * deltaPts;  
            if ( peakStartPt < min ) {
                peakStartPt = min;
            }
            peakEndPt += 3 * deltaPts;  
            if ( peakEndPt > max ) {
                peakEndPt = max;
            }
    
            //  If the domain is frequency, then convert from point space to target units.
            string domain = model->GetDataObject( "SpectroscopicData" )
                                    ->GetDcmHeader()->GetStringValue("SignalDomainColumns");
            if( domain == "FREQUENCY" ) {
                min = point->ConvertPosUnits( peakStartPt, svkSpecPoint::PTS, units ); 
                max = point->ConvertPosUnits( peakEndPt, svkSpecPoint::PTS, units ); 
                point->Delete();
            }
        } 
    }
}

void sivicQuantificationWidget::SetSpecUnits( svkSpecPoint::UnitType units )
{
	svkImageData* data = this->model->GetDataObject( "SpectroscopicData" );
	if( this->units != units && data != NULL ) {
		svkSpecPoint* point = svkSpecPoint::New();
		point->SetDcmHeader( data->GetDcmHeader() );
		for (int i = 0; i < this->numMets; i++ ) {
			double* wholeRange = this->metRangeVector[i]->GetWholeRange();

			double oldWholeRangeStart = wholeRange[0];
			double oldWholeRangeEnd   = wholeRange[1];

            double newWholeRangeStart = point->ConvertPosUnits( oldWholeRangeStart, this->units, units );
            double newWholeRangeEnd   = point->ConvertPosUnits( oldWholeRangeEnd, this->units, units );

			double* range      = this->metRangeVector[i]->GetRange();

			double oldRangeStart = range[0];
			double oldRangeEnd   = range[1];

            double newRangeStart = point->ConvertPosUnits( oldRangeStart, this->units, units );
            double newRangeEnd   = point->ConvertPosUnits( oldRangeEnd, this->units, units );

			this->metRangeVector[i]->SetWholeRange(newWholeRangeStart, newWholeRangeEnd);
			this->metRangeVector[i]->SetRange(newRangeStart, newRangeEnd);
		}
		this->units = units;
		point->Delete();
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
    vtkKWMenu* mapViewMenu = this->mapViewSelector->GetWidget()->GetMenu();

    if( data != NULL ) {

        this->mrsQuant->SetInput( data );
        this->mrsQuant->LimitToSelectedVolume();
		svkSpecPoint* point = svkSpecPoint::New();
		point->SetDcmHeader( data->GetDcmHeader() );

        //  update XML from current slider positions:
        for (int i = 0; i < this->numMets; i++ ) {
            float minPPM   = point->ConvertPosUnits( this->metRangeVector[i]->GetEntry1()->GetValueAsDouble(), this->units, svkSpecPoint::PPM);
            float maxPPM   = point->ConvertPosUnits( this->metRangeVector[i]->GetEntry2()->GetValueAsDouble(), this->units, svkSpecPoint::PPM);
            float peakPPM  = static_cast< float > ( (maxPPM + minPPM)/2 );
            float widthPPM = static_cast< float > ( fabs( ( maxPPM - minPPM) ) ); 
            this->mrsQuant->ModifyRegion( i, peakPPM, widthPPM ); 
        }
        point->Delete();

        this->mrsQuant->Update();

        vtkstd::vector< svkMriImageData* >* metMaps = this->mrsQuant->GetMetMaps(); 

        for (int i = 0; i < metMaps->size(); i ++ ) {

            //
            //  This is a vector of metabolite map names used in the svkDataModel: 
            //  The names should appear in the same order in the vector as they do 
            //  in the view selector 
            //
            vtkstd::string modelDataName = (*metMaps)[i]->GetDcmHeader()->GetStringValue("SeriesDescription"); 
            this->modelMetNames.push_back( modelDataName ); 

            if( this->model->DataExists( this->modelMetNames[i] ) ) {
                this->model->ChangeDataObject( this->modelMetNames[ i ], (*metMaps)[i]); 
            } else {
                this->model->AddDataObject( this->modelMetNames[ i ], (*metMaps)[i]);

                //  Add label to menu:
                ostringstream mapNum;
                mapNum <<  i; 
                stringstream invocation; 
                invocation.str("");
                invocation << "MetMapViewCallback " << mapNum.str() << endl;
                mapViewMenu->AddRadioButton(modelDataName.c_str(), this->sivicController, invocation.str().c_str());
            }

        }

        //  if overlay has not been initialized, the overlay with the first met map 
        //  otherwise grab the current menu value and use that to init the overlay
        if( this->model->DataExists( "MetaboliteData" ) == false ) {
            vtkKWMenu* mapViewMenu = this->mapViewSelector->GetWidget()->GetMenu();
            this->mapViewSelector->GetWidget()->SetValue( this->modelMetNames[0].c_str() );
            this->SetOverlay( this->modelMetNames[0] ); 
        }  else {
            vtkKWMenu* mapViewMenu = this->mapViewSelector->GetWidget()->GetMenu();
            vtkstd::string menuValue = this->mapViewSelector->GetWidget()->GetValue();
            if( this->model->DataExists( menuValue )) {
                this->SetOverlay(  this->mapViewSelector->GetWidget()->GetValue()  );
            } else {
                this->mapViewSelector->GetWidget()->SetValue( this->modelMetNames[0].c_str() );
                this->SetOverlay( this->modelMetNames[0] );
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
void sivicQuantificationWidget::SetOverlay( vtkstd::string modelObjectName)
{
	this->sivicController->OpenOverlayFromModel( modelObjectName.c_str() );
}


/*!
 *  Called by parent controller to enable this panel and initialize values
 */
void sivicQuantificationWidget::EnableWidgets()
{

    //  If this is the first time through, initialize the ranges to the default 
    //  values, otherwise leave them where the user set them. 
    if ( this->isEnabled == false ) {

        svkImageData* data = this->model->GetDataObject("SpectroscopicData");
        this->mrsQuant->SetInput( data );

        vtkstd::vector< vtkstd::vector< vtkstd::string > > regionNameVector = this->mrsQuant->GetRegionNameVector(); 
        this->numMets = regionNameVector.size(); 
        for (int i = 0; i < regionNameVector.size() ; i ++ ) {
            string regionName = regionNameVector[i][0]; 
            float peakPPM  = this->mrsQuant->GetFloatFromString( regionNameVector[i][1] ); 
            float widthPPM = this->mrsQuant->GetFloatFromString( regionNameVector[i][2] ); 
            this->metNames.push_back( regionName ); 
            this->metQuantMap[regionName].push_back(peakPPM + (widthPPM/2.)); 
            this->metQuantMap[regionName].push_back(peakPPM - (widthPPM/2.)); 
        }


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

    
        double rangeMin; 
        double rangeMax; 
        double peakStart; 
        double peakEnd; 
    
        int i; 
        for ( i = 0; i < this->numMets; i++ ) {

            string metName = this->metNames[i]; 
            float peakPPM  = this->mrsQuant->GetFloatFromString( regionNameVector[i][1] ); 
            float widthPPM = this->mrsQuant->GetFloatFromString( regionNameVector[i][2] ); 
            peakStart = peakPPM + (widthPPM/2.);
            peakEnd   = peakPPM - (widthPPM/2.);
            this->GetMRSFrequencyRange( peakStart, peakEnd, rangeMin, rangeMax, svkSpecPoint::PPM); 
    
            this->metRangeVector.push_back( vtkKWRange::New() ); 
            this->metRangeVector[i]->SetParent(this);
            this->metRangeVector[i]->SetLabelPositionToLeft();
            this->metRangeVector[i]->SetBalloonHelpString("Adjusts freq range of metabolite.");
            this->metRangeVector[i]->SetWholeRange(rangeMin, rangeMax);
            this->metRangeVector[i]->Create();
            this->metRangeVector[i]->SetRange(rangeMin, rangeMax);
            this->metRangeVector[i]->EnabledOff();
            this->metRangeVector[i]->SetSliderSize(1);
            this->metRangeVector[i]->SetPadY(1);
            this->metRangeVector[i]->SetEntry1PositionToLeft();
            this->metRangeVector[i]->SetEntry2PositionToRight();
            this->metRangeVector[i]->SetEntriesWidth(4);
            this->metRangeVector[i]->SetResolution(.01);
            this->metRangeVector[i]->SetLabelText( (this->metNames[i]).c_str() );  
            this->metRangeVector[i]->GetLabel()->SetWidth(10); 
            this->metRangeVector[i]->GetLabel()->SetFont("system 8"); 
        }

        //  Set default value
        vtkstd::string mapSelectLabel = this->metNames[0]; 
        this->mapViewSelector->GetWidget()->SetValue( mapSelectLabel.c_str() );
        this->mapViewSelector->GetWidget()->IndicatorVisibilityOn();


        //  Generate button
        this->quantButton = vtkKWPushButton::New();
        this->quantButton->SetParent( this );
        this->quantButton->Create( );
        this->quantButton->EnabledOff();
        this->quantButton->SetText( "Generate Maps");
        this->quantButton->SetBalloonHelpString("Prototype Metabolite Quantification ( peak ht and area ).");


        //  Format the GUI grid in this panel:
        for ( int i = 0; i < this->numMets; i++ ) {
            this->Script("grid %s -row %d -column 0 -sticky we -padx 1", this->metRangeVector[i]->GetWidgetName(), i);
        };
        //this->Script("grid %s -row %d -column 1 -rowspan 2 -sticky ew -padx 3", this->mapViewSelector->GetWidgetName(), 0);
        this->Script("grid %s -row %d -column 1 -rowspan 2 -sticky ew -padx 3", this->quantButton->GetWidgetName(), 2);


        for ( int i = 0; i < this->numMets; i++ ) {
            this->Script("grid rowconfigure %s %d  -weight 10", this->GetWidgetName(), i );
        }

        this->Script("grid columnconfigure %s 0 -weight 1 -minsize 140", this->GetWidgetName() );
        this->Script("grid columnconfigure %s 1 -weight 1 -minsize 100", this->GetWidgetName() );

        //  Callbacks
        this->AddCallbackCommandObserver(
            this->quantButton, vtkKWPushButton::InvokedEvent 
        );
    
        this->AddCallbackCommandObserver(
            this->mapViewSelector->GetWidget(), vtkKWMenu::MenuItemInvokedEvent
        );
    
        vtkstd::string metName;
        float metMin;
        float metMax;


        for ( i = 0; i < this->numMets; i++ ) {

            string metName = this->metNames[i]; 
            float peakPPM  = this->mrsQuant->GetFloatFromString( regionNameVector[i][1] ); 
            float widthPPM = this->mrsQuant->GetFloatFromString( regionNameVector[i][2] ); 
            peakStart = peakPPM + (widthPPM/2.);
            peakEnd   = peakPPM - (widthPPM/2.);
            this->GetMRSFrequencyRange( peakStart, peakEnd, rangeMin, rangeMax, svkSpecPoint::PPM); 
    
            this->metRangeVector.push_back( vtkKWRange::New() ); 
            this->metRangeVector[i]->SetParent(this);
            this->metRangeVector[i]->SetLabelPositionToLeft();
            this->metRangeVector[i]->SetBalloonHelpString("Adjusts freq range of metabolite.");
            this->metRangeVector[i]->SetWholeRange(rangeMin, rangeMax);
            this->metRangeVector[i]->Create();
            this->metRangeVector[i]->SetRange(rangeMin, rangeMax);
            this->metRangeVector[i]->EnabledOff();
            this->metRangeVector[i]->SetSliderSize(1);
            this->metRangeVector[i]->SetPadY(1);
            this->metRangeVector[i]->SetEntry1PositionToLeft();
            this->metRangeVector[i]->SetEntry2PositionToRight();
            this->metRangeVector[i]->SetEntriesWidth(4);
            this->metRangeVector[i]->SetResolution(.01);
            this->metRangeVector[i]->SetLabelText( (this->metNames[i]).c_str() );  
            this->metRangeVector[i]->GetLabel()->SetWidth(10); 
            this->metRangeVector[i]->GetLabel()->SetFont("system 8"); 

            this->metRangeVector[i]->SetWholeRange(rangeMin, rangeMax);
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

