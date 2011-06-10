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
    this->zeroFillButton = NULL;
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

    if( this->zeroFillButton != NULL ) {
        this->zeroFillButton->Delete();
        this->zeroFillButton= NULL;
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
    this->zeroFillSelectorSpec = vtkKWMenuButtonWithLabel::New();
    this->zeroFillSelectorSpec->SetParent(this);
    this->zeroFillSelectorSpec->Create();
    this->zeroFillSelectorSpec->SetLabelPositionToLeft();
    this->zeroFillSelectorSpec->SetPadY(2);
    this->zeroFillSelectorSpec->EnabledOff();
    this->zeroFillSelectorSpec->EnabledOn();
    this->zeroFillSelectorSpec->SetHeight(.8);
    vtkKWMenu* zfSpecMenu = this->zeroFillSelectorSpec->GetWidget()->GetMenu();

    this->zeroFillSelectorCols = vtkKWMenuButtonWithLabel::New();
    this->zeroFillSelectorCols->SetParent(this);
    this->zeroFillSelectorCols->Create();
    this->zeroFillSelectorSpec->SetLabelPositionToTop();
    this->zeroFillSelectorCols->LabelVisibilityOff();
    this->zeroFillSelectorCols->SetPadY(2);
    this->zeroFillSelectorCols->EnabledOn();
    vtkKWMenu* zfColsMenu = this->zeroFillSelectorCols->GetWidget()->GetMenu();

    this->zeroFillSelectorRows = vtkKWMenuButtonWithLabel::New();
    this->zeroFillSelectorRows->SetParent(this);
    this->zeroFillSelectorRows->Create();
    this->zeroFillSelectorCols->LabelVisibilityOff();
    this->zeroFillSelectorRows->SetPadY(2);
    this->zeroFillSelectorRows->EnabledOn();
    vtkKWMenu* zfRowsMenu = this->zeroFillSelectorRows->GetWidget()->GetMenu();

    this->zeroFillSelectorSlices = vtkKWMenuButtonWithLabel::New();
    this->zeroFillSelectorSlices->SetParent(this);
    this->zeroFillSelectorSlices->Create();
    this->zeroFillSelectorCols->LabelVisibilityOff();
    this->zeroFillSelectorSlices->SetPadY(2);
    this->zeroFillSelectorSlices->EnabledOn();
    vtkKWMenu* zfSlicesMenu = this->zeroFillSelectorSlices->GetWidget()->GetMenu();

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
    this->zeroFillSelectorSpec->GetWidget()->SetValue( zfOption1.c_str() );
    this->zeroFillSelectorCols->GetWidget()->SetValue( zfOption1.c_str() );
    this->zeroFillSelectorRows->GetWidget()->SetValue( zfOption1.c_str() );
    this->zeroFillSelectorSlices->GetWidget()->SetValue( zfOption1.c_str() );

    this->zeroFillButton = vtkKWPushButton::New();
    this->zeroFillButton->SetParent( this );
    this->zeroFillButton->Create( );
    this->zeroFillButton->EnabledOff();
    this->zeroFillButton->SetText( "Zero Fill");
    this->zeroFillButton->SetBalloonHelpString("Zero fill data.");

    //  =================================== 
    //  Zero Fill Labels
    //  =================================== 
    vtkKWLabel* zfSpecLabel = vtkKWLabel::New(); 
    zfSpecLabel->SetText( string("Zero Fill Spec").c_str() );
    zfSpecLabel->SetParent(this);
    zfSpecLabel->SetHeight(1);
    zfSpecLabel->SetPadX(0);
    zfSpecLabel->SetPadY(0);
    zfSpecLabel->SetJustificationToLeft();
    zfSpecLabel->Create();

    vtkKWLabel* zfColLabel = vtkKWLabel::New(); 
    zfColLabel->SetText( string("Zero Fill #Cols").c_str() );
    zfColLabel->SetParent(this);
    zfColLabel->SetHeight(1);
    zfColLabel->SetPadX(0);
    zfColLabel->SetPadY(0);
    zfColLabel->SetJustificationToLeft();
    zfColLabel->Create();

    vtkKWLabel* zfRowLabel = vtkKWLabel::New(); 
    zfRowLabel->SetText( string("Zero Fill #Rows").c_str() );
    zfRowLabel->SetParent(this);
    zfRowLabel->SetHeight(1);
    zfRowLabel->SetPadX(0);
    zfRowLabel->SetPadY(0);
    zfRowLabel->SetJustificationToLeft();
    zfRowLabel->Create();

    vtkKWLabel* zfSliceLabel = vtkKWLabel::New(); 
    zfSliceLabel->SetText( string("Zero Fill #Slices").c_str() );
    zfSliceLabel->SetParent(this);
    zfSliceLabel->SetHeight(1);
    zfSliceLabel->SetPadX(0);
    zfSliceLabel->SetPadY(0);
    zfSliceLabel->SetJustificationToLeft();
    zfSliceLabel->Create();

    this->Script("grid %s -row %d -column 1 -rowspan 1 -padx 2", zfSpecLabel->GetWidgetName(),  0);
    this->Script("grid %s -row %d -column 1 -rowspan 1 -padx 2", zfColLabel->GetWidgetName(),   1);
    this->Script("grid %s -row %d -column 1 -rowspan 1 -padx 2", zfRowLabel->GetWidgetName(),   2);
    this->Script("grid %s -row %d -column 1 -rowspan 1 -padx 2", zfSliceLabel->GetWidgetName(), 3);

    this->Script("grid %s -row %d -column 2 -rowspan 1 -padx 2", this->zeroFillSelectorSpec->GetWidgetName(),   0);
    this->Script("grid %s -row %d -column 2 -rowspan 1 -padx 2", this->zeroFillSelectorCols->GetWidgetName(),   1);
    this->Script("grid %s -row %d -column 2 -rowspan 1 -padx 2", this->zeroFillSelectorRows->GetWidgetName(),   2);
    this->Script("grid %s -row %d -column 2 -rowspan 1 -padx 2", this->zeroFillSelectorSlices->GetWidgetName(), 3);

    this->Script("grid rowconfigure %s 0  -weight 16", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 1  -weight 16", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 2  -weight 16", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 3  -weight 16", this->GetWidgetName() );

    this->Script("grid columnconfigure %s 0 -weight 50 -uniform 1 -minsize 50", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 1 -weight 50 -uniform 1 -minsize 50", this->GetWidgetName() );



    this->AddCallbackCommandObserver(
        this->zeroFillButton, vtkKWPushButton::InvokedEvent );

}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicPreprocessingWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    // Respond to a selection change in the overlay view
    if( caller == this->zeroFillButton && event == vtkKWPushButton::InvokedEvent ) {
        this->ExecuteZeroFill();
    }
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}


/*!
 *  Executes the combining of the channels.
 */
void sivicPreprocessingWidget::ExecuteZeroFill() 
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

