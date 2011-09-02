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
    string invocationString;

    invocationString = "ZeroFill SPECTRAL none"; 
    zfSpecMenu->AddRadioButton(zfOption1.c_str(), this->sivicController, invocationString.c_str());
    invocationString = "ZeroFill SPECTRAL double"; 
    zfSpecMenu->AddRadioButton(zfOption2.c_str(), this->sivicController, invocationString.c_str());
    invocationString = "ZeroFill SPECTRAL nextPower2"; 
    zfSpecMenu->AddRadioButton(zfOption3.c_str(), this->sivicController, invocationString.c_str());

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

    invocationString = "Apodize SPECTRAL none"; 
    apSpecMenu->AddRadioButton(apOption1.c_str(), this->sivicController, invocationString.c_str());
    invocationString = "Apodize SPECTRAL lorentzian"; 
    apSpecMenu->AddRadioButton(apOption2.c_str(), this->sivicController, invocationString.c_str());

    invocationString = "Apodize SPATIAL_COLS none"; 
    apColsMenu->AddRadioButton(apOption1.c_str(), this->sivicController, invocationString.c_str());
    invocationString = "Apodize SPATIAL_COLS lorentzian"; 
    apColsMenu->AddRadioButton(apOption2.c_str(), this->sivicController, invocationString.c_str());

    invocationString = "Apodize SPATIAL_ROWS none"; 
    apRowsMenu->AddRadioButton(apOption1.c_str(), this->sivicController, invocationString.c_str());
    invocationString = "Apodize SPATIAL_ROWS lorentzian"; 
    apRowsMenu->AddRadioButton(apOption2.c_str(), this->sivicController, invocationString.c_str());

    invocationString = "Apodize SPATIAL_SLICES none"; 
    apSlicesMenu->AddRadioButton(apOption1.c_str(), this->sivicController, invocationString.c_str());
    invocationString = "Apodize SPATIAL_SLICES lorentzian"; 
    apSlicesMenu->AddRadioButton(apOption2.c_str(), this->sivicController, invocationString.c_str());

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

    vtkKWLabel* specTitle = vtkKWLabel::New(); 
    specTitle->SetText( string("Spec").c_str() );
    specTitle->SetParent(this);
    specTitle->SetJustificationToLeft();
    specTitle->Create();

    vtkKWLabel* colsTitle = vtkKWLabel::New(); 
    colsTitle->SetText( string("Cols").c_str() );
    colsTitle->SetParent(this);
    colsTitle->SetJustificationToLeft();
    colsTitle->Create();

    vtkKWLabel* rowsTitle = vtkKWLabel::New(); 
    rowsTitle->SetText( string("Rows").c_str() );
    rowsTitle->SetParent(this);
    rowsTitle->SetJustificationToLeft();
    rowsTitle->Create();

    vtkKWLabel* sliceTitle = vtkKWLabel::New(); 
    sliceTitle->SetText( string("Slice").c_str() );
    sliceTitle->SetParent(this);
    sliceTitle->SetJustificationToLeft();
    sliceTitle->Create();

    this->Script("grid %s -row 0 -column 1 -sticky wnse", specTitle->GetWidgetName(), 4);
    this->Script("grid %s -row 0 -column 2 -sticky wnse", colsTitle->GetWidgetName(), 4);
    this->Script("grid %s -row 0 -column 3 -sticky wnse", rowsTitle->GetWidgetName(), 4);
    this->Script("grid %s -row 0 -column 4 -sticky wnse", sliceTitle->GetWidgetName(), 4);

    this->Script("grid %s -row 1 -column %d -sticky nwse", zeroFillTitle->GetWidgetName(),   0);
    this->Script("grid %s -row 1 -column %d -sticky nwse -padx 2 -pady 1", this->zeroFillSelectorSpec->GetWidgetName(),   1);
    this->Script("grid %s -row 1 -column %d -sticky nwse -padx 2 -pady 1", this->zeroFillSelectorCols->GetWidgetName(),   2);
    this->Script("grid %s -row 1 -column %d -sticky nwse -padx 2 -pady 1", this->zeroFillSelectorRows->GetWidgetName(),   3);
    this->Script("grid %s -row 1 -column %d -sticky nwse -padx 2 -pady 1", this->zeroFillSelectorSlices->GetWidgetName(), 4);

    this->Script("grid %s -row 2 -column %d -sticky nwse", apodizationTitle->GetWidgetName(),   0);
    this->Script("grid %s -row 2 -column %d -sticky nwse -padx 2 -pady 1", this->apodizationSelectorSpec->GetWidgetName(),   1);
    this->Script("grid %s -row 2 -column %d -sticky nwse -padx 2 -pady 1", this->apodizationSelectorCols->GetWidgetName(),   2);
    this->Script("grid %s -row 2 -column %d -sticky nwse -padx 2 -pady 1", this->apodizationSelectorRows->GetWidgetName(),   3);
    this->Script("grid %s -row 2 -column %d -sticky nwse -padx 2 -pady 1", this->apodizationSelectorSlices->GetWidgetName(), 4);


    this->Script("grid %s -row %d -column 4 -sticky nwse -padx 2 -pady 1", this->applyButton->GetWidgetName(), 3);

    this->Script("grid rowconfigure %s 0 -weight 0", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 1 -weight 0", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 2 -weight 0", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 3 -weight 0", this->GetWidgetName() );

    this->Script("grid columnconfigure %s 0 -weight 0 ", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 1 -weight 1 -uniform 1 -minsize 84", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 2 -weight 1 -uniform 1 -minsize 84", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 3 -weight 1 -uniform 1 -minsize 84", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 4 -weight 1 -uniform 1 -minsize 84", this->GetWidgetName() );

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
 *  Executes the combining of the channels.
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
        zeroFill->SetInput(data);

        if( zeroFillSpec.compare("double") == 0 ) {
            zeroFill->SetNumberOfSpecPointsToDouble();
            executeZeroFill = true;
        } else if( zeroFillSpec.compare("next y^2") == 0 ) {
            zeroFill->SetNumberOfSpecPointsToNextPower2();
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
            bool useFullFrequencyRange = 0;
            bool useFullAmplitudeRange = 1;
            bool resetAmplitude = 1;
            bool resetFrequency = 0;
            data->Modified();
            zeroFill->RemoveObserver( progressCallback);
            string stringFilename = "ZF";
            this->sivicController->ResetRange( useFullFrequencyRange, useFullAmplitudeRange,
                                          resetAmplitude, resetFrequency );
            this->sivicController->OpenSpectra( data, stringFilename);
        }
        if( apodizeSpec.compare("Lorentz") == 0 ) {
            svkMrsApodizationFilter* af = svkMrsApodizationFilter::New();
            vtkFloatArray* window = vtkFloatArray::New();
            float fwhh = 4.0;
            svkApodizationWindow::GetLorentzianWindow( window, data, fwhh );
            af->SetInput( data );
            af->SetWindow( window );
            af->Update();
            af->Delete();
            data->Modified();
        }
    }
    return; 

}


void sivicPreprocessingWidget::UpdateProgress(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 100.0*(*(double*)(callData)) );
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->SetStatusText(
                  static_cast<vtkAlgorithm*>(subject)->GetProgressText() );

}

