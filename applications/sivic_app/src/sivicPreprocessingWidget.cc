/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */



#include <sivicPreprocessingWidget.h>
#include <vtkSivicController.h>

vtkStandardNewMacro( sivicPreprocessingWidget );
//vtkCxxRevisionMacro( sivicPreprocessingWidget, "$Revision$");


/*! 
 *  Constructor
 */
sivicPreprocessingWidget::sivicPreprocessingWidget()
{

    this->zeroFillSelectorSpec = NULL;
    this->zeroFillSelectorCols = NULL;
    this->zeroFillSelectorRows = NULL;
    this->zeroFillSelectorSlices = NULL;
    this->applyButton = NULL;
    this->apodizationSelectorSpec = NULL;
    this->apodizationSelectorCols = NULL;
    this->apodizationSelectorRows = NULL;
    this->apodizationSelectorSlices = NULL;

    this->specLabel = NULL;
    this->colsLabel = NULL;
    this->rowsLabel = NULL;
    this->slicesLabel = NULL;
    this->apodFreqEntry = NULL;
    this->customValueEntry = NULL;

    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgress );
    this->progressCallback->SetClientData( (void*)this );

}


/*! 
 *  Destructor
 */
sivicPreprocessingWidget::~sivicPreprocessingWidget()
{

    if( this->zeroFillSelectorSpec != NULL ) {
        this->zeroFillSelectorSpec->Delete();
        this->zeroFillSelectorSpec = NULL;
    }

    if( this->zeroFillSelectorCols != NULL ) {
        this->zeroFillSelectorCols->Delete();
        this->zeroFillSelectorCols = NULL;
    }

    if( this->zeroFillSelectorRows != NULL ) {
        this->zeroFillSelectorRows->Delete();
        this->zeroFillSelectorRows = NULL;
    }

    if( this->zeroFillSelectorSlices != NULL ) {
        this->zeroFillSelectorSlices->Delete();
        this->zeroFillSelectorSlices = NULL;
    }

    if( this->applyButton != NULL ) {
        this->applyButton->Delete();
        this->applyButton= NULL;
    }

    if( this->apodizationSelectorSpec != NULL ) {
        this->apodizationSelectorSpec->Delete();
        this->apodizationSelectorSpec = NULL;
    }

    if( this->apodizationSelectorCols != NULL ) {
        this->apodizationSelectorCols->Delete();
        this->apodizationSelectorCols = NULL;
    }

    if( this->apodizationSelectorRows != NULL ) {
        this->apodizationSelectorRows->Delete();
        this->apodizationSelectorRows = NULL;
    }

    if( this->apodizationSelectorSlices != NULL ) {
        this->apodizationSelectorSlices->Delete();
        this->apodizationSelectorSlices = NULL;
    }

    if( this->specLabel != NULL ) {
        this->specLabel->Delete();
        this->specLabel = NULL;
    }

    if( this->colsLabel != NULL ) {
        this->colsLabel->Delete();
        this->colsLabel = NULL;
    }

    if( this->rowsLabel != NULL ) {
        this->rowsLabel->Delete();
        this->rowsLabel = NULL;
    }

    if( this->slicesLabel != NULL ) {
        this->slicesLabel->Delete();
        this->slicesLabel = NULL;
    }

    if( this->apodFreqEntry != NULL ) {
        this->apodFreqEntry->Delete();
        this->apodFreqEntry = NULL;
    }

    if( this->customValueEntry != NULL ) {
        this->customValueEntry->Delete();
        this->customValueEntry = NULL;
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

    //  =================================== 
    //  Zero Filling Selector
    //  =================================== 

    int labelWidth = 0;
    this->zeroFillSelectorSpec = vtkKWMenuButton::New();
    this->zeroFillSelectorSpec->SetParent(this);
    this->zeroFillSelectorSpec->Create();
    this->zeroFillSelectorSpec->EnabledOff();

    vtkKWMenu* zfSpecMenu = this->zeroFillSelectorSpec->GetMenu();

    this->zeroFillSelectorCols = vtkKWMenuButton::New();
    this->zeroFillSelectorCols->SetParent(this);
    this->zeroFillSelectorCols->Create();
    this->zeroFillSelectorCols->EnabledOff();

    vtkKWMenu* zfColsMenu = this->zeroFillSelectorCols->GetMenu();

    this->zeroFillSelectorRows = vtkKWMenuButton::New();
    this->zeroFillSelectorRows->SetParent(this);
    this->zeroFillSelectorRows->Create();
    this->zeroFillSelectorRows->EnabledOff();

    vtkKWMenu* zfRowsMenu = this->zeroFillSelectorRows->GetMenu();

    this->zeroFillSelectorSlices = vtkKWMenuButton::New();
    this->zeroFillSelectorSlices->SetParent(this);
    this->zeroFillSelectorSlices->Create();
    this->zeroFillSelectorSlices->EnabledOff();

    vtkKWMenu* zfSlicesMenu = this->zeroFillSelectorSlices->GetMenu();

    string zfOption1 = "none";
    string zfOption2 = "double";
    string zfOption3 = "next y^2";
    string zfOption4 = "custom";
    string invocationString;

    invocationString = "ZeroFill SPECTRAL none"; 
    zfSpecMenu->AddRadioButton(zfOption1.c_str(), this->sivicController, invocationString.c_str());
    invocationString = "ZeroFill SPECTRAL double"; 
    zfSpecMenu->AddRadioButton(zfOption2.c_str(), this->sivicController, invocationString.c_str());
    invocationString = "ZeroFill SPECTRAL nextPower2"; 
    zfSpecMenu->AddRadioButton(zfOption3.c_str(), this->sivicController, invocationString.c_str());
    invocationString = "ZeroFill SPECTRAL custom";
    zfSpecMenu->AddRadioButton(zfOption4.c_str(), this->sivicController, invocationString.c_str());

    invocationString = "ZeroFill SPATIAL_COLS none"; 
    zfColsMenu->AddRadioButton(zfOption1.c_str(), this->sivicController, invocationString.c_str());
    invocationString = "ZeroFill SPATIAL_COLS double"; 
    zfColsMenu->AddRadioButton(zfOption2.c_str(), this->sivicController, invocationString.c_str());
    invocationString = "ZeroFill SPATIAL_COLS nextPower2"; 
    zfColsMenu->AddRadioButton(zfOption3.c_str(), this->sivicController, invocationString.c_str());

    invocationString = "ZeroFill SPATIAL_ROWS none"; 
    zfRowsMenu->AddRadioButton(zfOption1.c_str(), this->sivicController, invocationString.c_str());
    invocationString = "ZeroFill SPATIAL_ROWS double"; 
    zfRowsMenu->AddRadioButton(zfOption2.c_str(), this->sivicController, invocationString.c_str());
    invocationString = "ZeroFill SPATIAL_ROWS nextPower2"; 
    zfRowsMenu->AddRadioButton(zfOption3.c_str(), this->sivicController, invocationString.c_str());

    invocationString = "ZeroFill SPATIAL_SLICES none"; 
    zfSlicesMenu->AddRadioButton(zfOption1.c_str(), this->sivicController, invocationString.c_str());
    invocationString = "ZeroFill SPATIAL_SLICES double"; 
    zfSlicesMenu->AddRadioButton(zfOption2.c_str(), this->sivicController, invocationString.c_str());
    invocationString = "ZeroFill SPATIAL_SLICES nextPower2"; 
    zfSlicesMenu->AddRadioButton(zfOption3.c_str(), this->sivicController, invocationString.c_str());

    //  Set default values
    this->zeroFillSelectorSpec->SetValue( zfOption1.c_str() );
    this->zeroFillSelectorCols->SetValue( zfOption1.c_str() );
    this->zeroFillSelectorRows->SetValue( zfOption1.c_str() );
    this->zeroFillSelectorSlices->SetValue( zfOption1.c_str() );

    this->applyButton = vtkKWPushButton::New();
    this->applyButton->SetParent( this );
    this->applyButton->Create( );
    this->applyButton->SetText( "Apply");
    this->applyButton->SetBalloonHelpString("Apply Preprocessing.");
    this->applyButton->EnabledOff();

    //  =================================== 
    //  Apodization Selectors
    //  =================================== 

    //this->apodizationSelectorSpec = vtkKWMenuButton::New();
    this->apodizationSelectorSpec = vtkKWMenuButton::New();
    this->apodizationSelectorSpec->SetParent(this);
    this->apodizationSelectorSpec->Create();
    this->apodizationSelectorSpec->EnabledOff();

    vtkKWMenu* apSpecMenu = this->apodizationSelectorSpec->GetMenu();

    this->apodizationSelectorCols = vtkKWMenuButton::New();
    this->apodizationSelectorCols->SetParent(this);
    this->apodizationSelectorCols->Create();
    this->apodizationSelectorCols->EnabledOff();

    vtkKWMenu* apColsMenu = this->apodizationSelectorCols->GetMenu();

    this->apodizationSelectorRows = vtkKWMenuButton::New();
    this->apodizationSelectorRows->SetParent(this);
    this->apodizationSelectorRows->Create();
    this->apodizationSelectorRows->EnabledOff();

    vtkKWMenu* apRowsMenu = this->apodizationSelectorRows->GetMenu();

    this->apodizationSelectorSlices = vtkKWMenuButton::New();
    this->apodizationSelectorSlices->SetParent(this);
    this->apodizationSelectorSlices->Create();
    this->apodizationSelectorSlices->EnabledOff();

    vtkKWMenu* apSlicesMenu = this->apodizationSelectorSlices->GetMenu();

    string apOption1 = "none";
    string apOption2 = "Lorentz";
    string apOption3 = "Gauss";
    string apOption4 = "Hamming";

    apSpecMenu->AddRadioButton(apOption1.c_str(), this->sivicController, "");
    apSpecMenu->AddRadioButton(apOption2.c_str(), this->sivicController, "");
    apSpecMenu->AddRadioButton(apOption3.c_str(), this->sivicController, "");

    apColsMenu->AddRadioButton(apOption1.c_str(), this->sivicController, "");
    apColsMenu->AddRadioButton(apOption4.c_str(), this->sivicController, "");

    apRowsMenu->AddRadioButton(apOption1.c_str(), this->sivicController, "");
    apRowsMenu->AddRadioButton(apOption4.c_str(), this->sivicController, "");

    apSlicesMenu->AddRadioButton(apOption1.c_str(), this->sivicController, "");
    apSlicesMenu->AddRadioButton(apOption4.c_str(), this->sivicController, "");
    
    //  Set default values
    this->apodizationSelectorSpec->SetValue( apOption1.c_str() );
    this->apodizationSelectorCols->SetValue( apOption1.c_str() );
    this->apodizationSelectorRows->SetValue( apOption1.c_str() );
    this->apodizationSelectorSlices->SetValue( apOption1.c_str() );

    // Titles

    vtkKWLabel* zeroFillTitle = vtkKWLabel::New(); 
    zeroFillTitle->SetText( string("Zero Fill").c_str() );
    zeroFillTitle->SetParent(this);
    zeroFillTitle->SetJustificationToLeft();
    zeroFillTitle->Create();

    vtkKWLabel* apodizationTitle = vtkKWLabel::New(); 
    apodizationTitle->SetText( string("Apodize").c_str() );
    apodizationTitle->SetParent(this);
    apodizationTitle->SetJustificationToLeft();
    apodizationTitle->Create();

    vtkKWLabel* hzTitle = vtkKWLabel::New(); 
    hzTitle->SetText( string("Hz").c_str() );
    hzTitle->SetParent(this);
    hzTitle->SetJustificationToLeft();
    hzTitle->Create();

    vtkKWLabel* specTitle = vtkKWLabel::New(); 
    specTitle->SetText( string("Spec").c_str() );
    specTitle->SetParent(this);
    specTitle->SetJustificationToLeft();
    specTitle->SetPadY(0);
    specTitle->Create();

    vtkKWLabel* colsTitle = vtkKWLabel::New(); 
    colsTitle->SetText( string("Cols").c_str() );
    colsTitle->SetParent(this);
    colsTitle->SetJustificationToLeft();
    colsTitle->SetPadY(0);
    colsTitle->Create();

    vtkKWLabel* rowsTitle = vtkKWLabel::New(); 
    rowsTitle->SetText( string("Rows").c_str() );
    rowsTitle->SetParent(this);
    rowsTitle->SetJustificationToLeft();
    rowsTitle->SetPadY(0);
    rowsTitle->Create();

    vtkKWLabel* sliceTitle = vtkKWLabel::New(); 
    sliceTitle->SetText( string("Slice").c_str() );
    sliceTitle->SetParent(this);
    sliceTitle->SetJustificationToLeft();
    sliceTitle->SetPadY(0);
    sliceTitle->Create();

    vtkKWLabel* customTitle = vtkKWLabel::New();
    customTitle->SetText( string("Custom").c_str() );
    customTitle->SetParent(this);
    customTitle->SetJustificationToLeft();
    customTitle->Create();

    this->customValueEntry = vtkKWEntry::New();
    this->customValueEntry->SetParent(this);
    this->customValueEntry->Create();
    this->customValueEntry->EnabledOff();

    this->apodFreqEntry = vtkKWEntry::New();
    this->apodFreqEntry->SetParent(this);
    this->apodFreqEntry->Create();
    this->apodFreqEntry->EnabledOff();
    this->Script("grid %s -row 0 -column 1 -sticky wnse", hzTitle->GetWidgetName(), 4);
    this->Script("grid %s -row 0 -column 2 -sticky wnse", specTitle->GetWidgetName(), 4);
    this->Script("grid %s -row 0 -column 3 -sticky wnse", colsTitle->GetWidgetName(), 4);
    this->Script("grid %s -row 0 -column 4 -sticky wnse", rowsTitle->GetWidgetName(), 4);
    this->Script("grid %s -row 0 -column 5 -sticky wnse", sliceTitle->GetWidgetName(), 4);


    this->Script("grid %s -row 1 -column %d -sticky nwse", apodizationTitle->GetWidgetName(),   0);
    this->Script("grid %s -row 1 -column %d -sticky nwse -padx 2 -pady 0", this->apodFreqEntry->GetWidgetName(), 1);
    this->Script("grid %s -row 1 -column %d -sticky nwse -padx 2 -pady 0", this->apodizationSelectorSpec->GetWidgetName(),   2);
    this->Script("grid %s -row 1 -column %d -sticky nwse -padx 2 -pady 0", this->apodizationSelectorCols->GetWidgetName(),   3);
    this->Script("grid %s -row 1 -column %d -sticky nwse -padx 2 -pady 0", this->apodizationSelectorRows->GetWidgetName(),   4);
    this->Script("grid %s -row 1 -column %d -sticky nwse -padx 2 -pady 0", this->apodizationSelectorSlices->GetWidgetName(), 5);

    this->Script("grid %s -row 2 -column %d -sticky nwse", zeroFillTitle->GetWidgetName(),   0);
    this->Script("grid %s -row 2 -column %d -sticky nwse -padx 2 -pady 0", this->zeroFillSelectorSpec->GetWidgetName(),   2);
    this->Script("grid %s -row 2 -column %d -sticky nwse -padx 2 -pady 0", this->zeroFillSelectorCols->GetWidgetName(),   3);
    this->Script("grid %s -row 2 -column %d -sticky nwse -padx 2 -pady 0", this->zeroFillSelectorRows->GetWidgetName(),   4);
    this->Script("grid %s -row 2 -column %d -sticky nwse -padx 2 -pady 0", this->zeroFillSelectorSlices->GetWidgetName(), 5);


    this->Script("grid %s -row %d -column 1 -sticky nwse -padx 2 -pady 0", customTitle->GetWidgetName(), 3);
    this->Script("grid %s -row %d -column 2 -sticky nwse -padx 2 -pady 0", this->customValueEntry->GetWidgetName(), 3);
    this->Script("grid %s -row %d -column 5 -sticky nwse -padx 2 -pady 0", this->applyButton->GetWidgetName(), 3);

    this->Script("grid rowconfigure %s 0 -weight 0", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 1 -weight 0", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 2 -weight 0", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 3 -weight 0", this->GetWidgetName() );

    this->Script("grid columnconfigure %s 0 -weight 0 ", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 1 -weight 1 -uniform 1 -minsize 42", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 2 -weight 2 -uniform 1 -minsize 84", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 3 -weight 2 -uniform 1 -minsize 84", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 4 -weight 2 -uniform 1 -minsize 84", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 5 -weight 2 -uniform 1 -minsize 84", this->GetWidgetName() );

    this->AddCallbackCommandObserver( this->applyButton, vtkKWPushButton::InvokedEvent );

}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicPreprocessingWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    // Respond to a selection change in the overlay view
    if( caller == this->applyButton && event == vtkKWPushButton::InvokedEvent ) {
        this->ExecutePreprocessing();
    }
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}


/*!
 *  Executes the proprocessing filters.
 */
void sivicPreprocessingWidget::ExecutePreprocessing() 
{
    svkImageData* data = this->model->GetDataObject("SpectroscopicData");
    string domain = data->GetDcmHeader()->GetStringValue("SignalDomainColumns");
    if( domain != "TIME") {
        cout << "ERROR: Data must be in the time domain for pre-processing." << endl;
        return;
    }
    if( data != NULL ) {
        int toggleDraw = this->sivicController->GetDraw();
        if( toggleDraw ) {
        	this->sivicController->DrawOff();
        }
        string apodizeSpec(this->apodizationSelectorSpec->GetValue());
        string apodizeCols(this->apodizationSelectorCols->GetValue());
        string apodizeRows(this->apodizationSelectorRows->GetValue());
        string apodizeSlices(this->apodizationSelectorSlices->GetValue());
        string zeroFillSpec(this->zeroFillSelectorSpec->GetValue());
        string zeroFillCols(this->zeroFillSelectorCols->GetValue());
        string zeroFillRows(this->zeroFillSelectorRows->GetValue());
        string zeroFillSlices(this->zeroFillSelectorSlices->GetValue());

        bool executeZeroFill = false;
        svkMrsZeroFill* zeroFill = svkMrsZeroFill::New();
        zeroFill->SetInputData(data);

        if( zeroFillSpec.compare("double") == 0 ) {
            zeroFill->SetNumberOfSpecPointsToDouble();
            executeZeroFill = true;
        } else if( zeroFillSpec.compare("next y^2") == 0 ) {
            zeroFill->SetNumberOfSpecPointsToNextPower2();
            executeZeroFill = true;
        } else if( zeroFillSpec.compare("custom") == 0 ) {
            zeroFill->SetNumberOfSpecPoints(this->customValueEntry->GetValueAsInt());
            executeZeroFill = true;
        }
        if( zeroFillRows.compare("double") == 0 ) {
            zeroFill->SetNumberOfRowsToDouble();
            executeZeroFill = true;
        } else if( zeroFillRows.compare("next y^2") == 0 ) {
            zeroFill->SetNumberOfRowsToNextPower2();
            executeZeroFill = true;
        }

        if( zeroFillCols.compare("double") == 0 ) {
            zeroFill->SetNumberOfColumnsToDouble();
            executeZeroFill = true;
        } else if( zeroFillCols.compare("next y^2") == 0 ) {
            zeroFill->SetNumberOfColumnsToNextPower2();
            executeZeroFill = true;
        }

        if( zeroFillSlices.compare("double") == 0 ) {
            zeroFill->SetNumberOfSlicesToDouble();
            executeZeroFill = true;
        } else if( zeroFillSlices.compare("next y^2") == 0 ) {
            zeroFill->SetNumberOfSlicesToNextPower2();
            executeZeroFill = true;
        }
        
        if( executeZeroFill ) {
            zeroFill->AddObserver(vtkCommand::ProgressEvent, progressCallback);
            this->GetApplication()->GetNthWindow(0)->SetStatusText("Executing Zero Fill...");
            zeroFill->Update();
            bool useFullFrequencyRange = 1;
            bool useFullAmplitudeRange = 1;
            bool resetAmplitude = 1;
            bool resetFrequency = 1;
            zeroFill->RemoveObserver( progressCallback);
            string stringFilename = "ZF";
            this->sivicController->Open4DImage( data, stringFilename, data);
            this->sivicController->ResetRange( useFullFrequencyRange, useFullAmplitudeRange,
                                          resetAmplitude, resetFrequency );
        }

        float fwhh = this->GetApodizationFWHH(); 
        if( apodizeSpec.compare("Lorentz") == 0 ) {
            svkMrsApodizationFilter* af = svkMrsApodizationFilter::New();
            vector< vtkFloatArray* >* window = new vector< vtkFloatArray* >();
            svkApodizationWindow::GetLorentzianWindow( window, data, fwhh );
            af->SetInputData( data );
            af->SetWindow( window );
            af->Update();
            af->Delete();
            data->Modified();
        } else if ( apodizeSpec.compare("Gauss") == 0 ) {
            svkMrsApodizationFilter* af = svkMrsApodizationFilter::New();
            vector< vtkFloatArray* >* window = new vector< vtkFloatArray* >();
            float center = 0.0;
            //  default, set center to center time point (assume full symmetric echo)
            float bandwidth = data->GetDcmHeader()->GetFloatValue("SpectralWidth");
            float dwellTime = 1./bandwidth; 
            int numFreqPts = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );     
            
            float echoCenter = 0; 
            if ( data->GetDcmHeader()->ElementExists( "SVK_ECHO_CENTER_PT" ) ) {
                echoCenter = data->GetDcmHeader()->GetFloatValue( "SVK_ECHO_CENTER_PT" );     
            }

            //cout << "CENTER ECHO POINT: " << echoCenter << endl;
            center = (echoCenter) * dwellTime; 
            //cout << "APOD: " << center << " numpts: " << numFreqPts << endl;
            char centerDefault[50]="";
            this->GetApplication()->GetRegistryValue( 0, "apodization", "center", centerDefault  );
            if( string(centerDefault) != "" ) {
                center = atof( centerDefault );
            }
            svkApodizationWindow::GetGaussianWindow( window, data, fwhh, center );
            af->SetInputData( data );
            af->SetWindow( window );
            af->Update();
            af->Delete();
            data->Modified();
        } 

        if ( apodizeCols.compare("Hamming") == 0 && apodizeRows.compare("Hamming") == 0 && apodizeSlices.compare("Hamming") == 0 ) {
            svkMrsApodizationFilter* af = svkMrsApodizationFilter::New();
            vector< vtkFloatArray* >* window = new vector< vtkFloatArray* >();
            svkApodizationWindow::GetHammingWindow( window, data, svkApodizationWindow::THREE_D );
            af->SetInputData( data );
            af->SetWindow( window );
            af->Update();
            af->Delete();
            data->Modified();
        } else {
            if ( apodizeCols.compare("Hamming") == 0 ) {
                svkMrsApodizationFilter* af = svkMrsApodizationFilter::New();
                vector< vtkFloatArray* >* window = new vector< vtkFloatArray* >();
                svkApodizationWindow::GetHammingWindow( window, data, svkApodizationWindow::COL );
                af->SetInputData( data );
                af->SetWindow( window );
                af->Update();
                af->Delete();
                data->Modified();
            }
            if ( apodizeRows.compare("Hamming") == 0 ) {
                svkMrsApodizationFilter* af = svkMrsApodizationFilter::New();
                vector< vtkFloatArray* >* window = new vector< vtkFloatArray* >();
                svkApodizationWindow::GetHammingWindow( window, data, svkApodizationWindow::ROW );
                af->SetInputData( data );
                af->SetWindow( window );
                af->Update();
                af->Delete();
                data->Modified();
            }
            if ( apodizeSlices.compare("Hamming") == 0 ) {
                svkMrsApodizationFilter* af = svkMrsApodizationFilter::New();
                vector< vtkFloatArray* >* window = new vector< vtkFloatArray* >();
                svkApodizationWindow::GetHammingWindow( window, data, svkApodizationWindow::SLICE );
                af->SetInputData( data );
                af->SetWindow( window );
                af->Update();
                af->Delete();
                data->Modified();
            }
        }

	zeroFill->Delete();
        if( toggleDraw ) {
           this->sivicController->DrawOn();
	}

    }
}


/*!
 *  Executes the proprocessing filters.
 */
float sivicPreprocessingWidget::GetApodizationFWHH() 
{
    float fwhh;

    svkImageData* data = this->model->GetDataObject("SpectroscopicData");


    //  Initiall try to use a default with based on the nucleus (field strength): 
    fwhh = 4.0f;
    std::string nucleus = data->GetDcmHeader()->GetStringValue( "ResonantNucleus" );
    if( nucleus.compare("13C") == 0 ) {
        fwhh = 9.0f;
    }

    //  If there is a custom setting in the preferences, then use that
    char fwhhDefault[50]="";
    this->GetApplication()->GetRegistryValue( 0, "apodization", "fwhh", fwhhDefault  );
    if( string(fwhhDefault) != "" ) {
        fwhh = atof( fwhhDefault );
    }

    //  Finally, if user specified a value in entry box use, override fwhh with that that: 
    float entryValue = this->apodFreqEntry->GetValueAsDouble(); 
    if ( entryValue != 0 ) {
        fwhh = entryValue; 
    }
    return fwhh; 
}


void sivicPreprocessingWidget::UpdateProgress(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 100.0*(*(double*)(callData)) );
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->SetStatusText(
                  static_cast<vtkAlgorithm*>(subject)->GetProgressText() );

}
