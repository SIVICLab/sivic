/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */



#include <sivicImageViewWidget.h>
#include <vtkSivicController.h>
#include <vtkJPEGReader.h>

#define MINIMUM_RANGE_FACTOR 100

vtkStandardNewMacro( sivicImageViewWidget );
vtkCxxRevisionMacro( sivicImageViewWidget, "$Revision$");


/*! 
 *  Constructor
 */
sivicImageViewWidget::sivicImageViewWidget()
{
    this->overlayOpacitySlider = NULL;
    this->overlayThresholdSlider = NULL;
    this->axialSlider = NULL;
    this->coronalSlider = NULL;
    this->sagittalSlider = NULL;
    this->volSelButton = NULL;
    this->plotGridButton = NULL;
    this->satBandButton = NULL;
    this->satBandOutlineButton = NULL;
    this->overlayButton = NULL;
    this->colorBarButton = NULL;
    this->orthImagesButton = NULL;
    this->interpolationBox = NULL;
    this->lutBox = NULL;
    this->thresholdType = NULL;
    this->imageViewFrame = NULL;
    this->orthoViewFrame = NULL;
    this->overlayViewFrame = NULL;

}


/*! 
 *  Destructor
 */
sivicImageViewWidget::~sivicImageViewWidget()
{
    if( this->axialSlider != NULL ) {
        this->axialSlider->Delete();
        this->axialSlider= NULL;
    }

    if( this->coronalSlider != NULL ) {
        this->coronalSlider->Delete();
        this->coronalSlider= NULL;
    }

    if( this->sagittalSlider != NULL ) {
        this->sagittalSlider->Delete();
        this->sagittalSlider= NULL;
    }

    if( this->overlayOpacitySlider != NULL ) {
        this->overlayOpacitySlider->Delete();
        this->overlayOpacitySlider = NULL;
    }

    if( this->overlayThresholdSlider != NULL ) {
        this->overlayThresholdSlider->Delete();
        this->overlayThresholdSlider = NULL;
    }

    if( this->volSelButton != NULL ) {
        this->volSelButton ->Delete();
        this->volSelButton = NULL;
    }

    if( this->plotGridButton != NULL ) {
        this->plotGridButton ->Delete();
        this->plotGridButton = NULL;
    }

    if( this->satBandButton != NULL ) {
        this->satBandButton ->Delete();
        this->satBandButton = NULL;
    }

    if( this->satBandOutlineButton != NULL ) {
        this->satBandOutlineButton ->Delete();
        this->satBandOutlineButton = NULL;
    }

    if( this->overlayButton != NULL ) {
        this->overlayButton ->Delete();
        this->overlayButton = NULL;
    }

    if( this->colorBarButton != NULL ) {
        this->colorBarButton ->Delete();
        this->colorBarButton = NULL;
    }

    if( this->orthImagesButton != NULL ) {
        this->orthImagesButton ->Delete();
        this->orthImagesButton = NULL;
    }

    if( this->interpolationBox != NULL ) {
        this->interpolationBox ->Delete();
        this->interpolationBox = NULL;
    }

    if( this->lutBox!= NULL ) {
        this->lutBox->Delete();
        this->lutBox= NULL;
    }

    if( this->thresholdType!= NULL ) {
        this->thresholdType->Delete();
        this->thresholdType= NULL;
    }

    if( this->imageViewFrame != NULL ) {
        this->imageViewFrame->Delete();
        this->imageViewFrame = NULL;
    }

    if( this->orthoViewFrame != NULL ) {
        this->orthoViewFrame->Delete();
        this->orthoViewFrame = NULL;
    }

    if( this->overlayViewFrame != NULL ) {
        this->overlayViewFrame->Delete();
        this->overlayViewFrame = NULL;
    }

    if( this->loadingLabel != NULL ) {
        this->loadingLabel->Delete();
        this->loadingLabel = NULL;
    }
    
}


/*! 
 *  Method in superclass to be overriden to add our custom widgets.
 */
void sivicImageViewWidget::CreateWidget()
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

    //  =======================================================
    //  Spec View Widgets
    //  =======================================================
  
    //  =======================================================
    //  Image View Widgets
    //  =======================================================
    this->imageViewFrame = vtkKWFrame::New();   
    this->imageViewFrame->SetParent(this);
    this->imageViewFrame->Create();

    // Here is a radio button to toggle the vol selection box on/off
    this->volSelButton = vtkKWCheckButton::New();
    this->volSelButton->SetParent(this);
    this->volSelButton->Create();
    this->volSelButton->EnabledOff();
    this->volSelButton->SetText("Sel Box");
    this->volSelButton->SelectedStateOn();

    // Here is a radio button to toggle the vol selection box on/off
    this->plotGridButton = vtkKWCheckButton::New();
    this->plotGridButton->SetParent(this);
    this->plotGridButton->Create();
    this->plotGridButton->EnabledOff();
    this->plotGridButton->SetText("Grid");
    this->plotGridButton->SelectedStateOn();

    // Here is a radio button to toggle the sat bands on/off
    this->satBandButton = vtkKWCheckButton::New();
    this->satBandButton->SetParent(this);
    this->satBandButton->Create();
    this->satBandButton->EnabledOff();
    this->satBandButton->SetText("Sat Bands");
    this->satBandButton->SelectedStateOff();

    // Here is a radio button to toggle the sat bands on/off
    this->satBandOutlineButton = vtkKWCheckButton::New();
    this->satBandOutlineButton->SetParent(this);
    this->satBandOutlineButton->Create();
    this->satBandOutlineButton->EnabledOff();
    this->satBandOutlineButton->SetText("Sat Outline");
    this->satBandOutlineButton->SelectedStateOff();
    //  ======================================================


    //  =======================================================
    //  Overlay Widgets
    //  =======================================================
    this->orthoViewFrame = vtkKWFrame::New();   
    this->orthoViewFrame->SetParent(this);
    this->orthoViewFrame->Create();

    this->overlayViewFrame = vtkKWFrame::New();   
    this->overlayViewFrame->SetParent(this);
    this->overlayViewFrame->Create();

    vtkKWSeparator* orthoSeparator = vtkKWSeparator::New();   
    orthoSeparator->SetParent(this);
    orthoSeparator->Create();
    orthoSeparator->SetThickness(5);

    vtkKWSeparator* overlaySeparator = vtkKWSeparator::New();   
    overlaySeparator->SetParent(this);
    overlaySeparator->Create();
    overlaySeparator->SetThickness(5);

    this->interpolationBox = vtkKWMenuButtonWithLabel::New();   
    this->interpolationBox->SetParent(this);
    this->interpolationBox->Create();
    this->interpolationBox->SetLabelText("Interpolation Method");
    this->interpolationBox->SetLabelPositionToTop();
    this->interpolationBox->SetPadY(5);
    this->interpolationBox->EnabledOff();
    vtkKWMenu* interpMenu = this->interpolationBox->GetWidget()->GetMenu();
    interpMenu->AddRadioButton("nearest neighbor", this->sivicController, "SetInterpolationCallback 0");
    interpMenu->AddRadioButton("linear", this->sivicController, "SetInterpolationCallback 1");
    interpMenu->AddRadioButton("sinc", this->sivicController, "SetInterpolationCallback 2");
    this->interpolationBox->GetWidget()->SetValue( "nearest neighbor" );

    this->lutBox = vtkKWMenuButtonWithLabel::New();   
    this->lutBox->SetParent(this);
    this->lutBox->Create();
    this->lutBox->SetLabelText("Color Map");
    this->lutBox->SetLabelPositionToTop();
    this->lutBox->SetPadY(5);
    this->lutBox->EnabledOff();
    vtkKWMenu* lutMenu = this->lutBox->GetWidget()->GetMenu();

    stringstream invocation;
    invocation << "SetLUTCallback " << svkLookupTable::COLOR << endl;
    lutMenu->AddRadioButton("color", this->sivicController, invocation.str().c_str());

    invocation.str("");
    invocation << "SetLUTCallback " << svkLookupTable::GREY_SCALE << endl;
    lutMenu->AddRadioButton("grey ", this->sivicController, invocation.str().c_str());

    invocation.str("");
    invocation << "SetLUTCallback " << svkLookupTable::HURD << endl;
    lutMenu->AddRadioButton("hurd ", this->sivicController, invocation.str().c_str());

    invocation.str("");
    invocation << "SetLUTCallback " << svkLookupTable::CYAN_HOT << endl;
    lutMenu->AddRadioButton("cyan ", this->sivicController, invocation.str().c_str());

    this->lutBox->GetWidget()->SetValue( "color" );

    this->thresholdType = vtkKWMenuButtonWithLabel::New();   
    this->thresholdType->SetParent(this);
    this->thresholdType->Create();
    this->thresholdType->SetLabelText("Threshold Type");
    this->thresholdType->SetLabelPositionToTop();
    this->thresholdType->SetPadY(5);
    this->thresholdType->EnabledOff();
    vtkKWMenu* thresholdTypeMenu = this->thresholdType->GetWidget()->GetMenu();

    invocation.str("");
    invocation << "SetThresholdTypeToQuantity " << endl;
    thresholdTypeMenu->AddRadioButton("Quantity", this->sivicController, invocation.str().c_str());

    invocation.str("");
    invocation << "SetThresholdTypeToPercent " << endl;
    thresholdTypeMenu->AddRadioButton("Percent", this->sivicController, invocation.str().c_str());

    this->thresholdType->GetWidget()->SetValue( "Quantity" );

    this->axialSlider = vtkKWScaleWithEntry::New();
    this->axialSlider->SetParent(this);
    this->axialSlider->Create();
    this->axialSlider->SetEntryWidth( 3 );
    this->axialSlider->SetOrientationToHorizontal();
    this->axialSlider->SetLabelText("Axial Slice    ");
    this->axialSlider->SetValue(1);
    this->axialSlider->SetRange( 1, 1);
    this->axialSlider->SetBalloonHelpString("Adjusts image slice.");
    this->axialSlider->EnabledOff();
    this->axialSlider->SetLabelPositionToLeft();
    this->axialSlider->SetEntryPositionToRight();

    this->coronalSlider = vtkKWScaleWithEntry::New();
    this->coronalSlider->SetParent(this);
    this->coronalSlider->Create();
    this->coronalSlider->SetEntryWidth( 3 );
    this->coronalSlider->SetOrientationToHorizontal();
    this->coronalSlider->SetLabelText("Coronal Slice");
    this->coronalSlider->SetValue(1);
    this->coronalSlider->SetRange( 1, 1);
    this->coronalSlider->SetBalloonHelpString("Adjusts the ortho view x slice.");
    this->coronalSlider->EnabledOff();
    this->coronalSlider->SetLabelPositionToLeft();
    this->coronalSlider->SetEntryPositionToRight();

    this->sagittalSlider = vtkKWScaleWithEntry::New();
    this->sagittalSlider->SetParent(this);
    this->sagittalSlider->Create();
    this->sagittalSlider->SetEntryWidth( 3 );
    this->sagittalSlider->SetOrientationToHorizontal();
    this->sagittalSlider->SetLabelText("Sagittal Slice");
    this->sagittalSlider->SetValue(1);
    this->sagittalSlider->SetRange( 1, 1);
    this->sagittalSlider->SetBalloonHelpString("Adjusts the ortho view y slice.");
    this->sagittalSlider->EnabledOff();
    this->sagittalSlider->SetLabelPositionToLeft();
    this->sagittalSlider->SetEntryPositionToRight();

    this->overlayOpacitySlider = vtkKWScaleWithEntry::New();
    this->overlayOpacitySlider->SetParent(this);
    this->overlayOpacitySlider->Create();
    this->overlayOpacitySlider->SetEntryWidth( 3 );
    this->overlayOpacitySlider->SetOrientationToHorizontal();
    this->overlayOpacitySlider->SetLabelText("Opacity   ");
    this->overlayOpacitySlider->SetValue(35);
    this->overlayOpacitySlider->SetRange( 0, 100 );
    this->overlayOpacitySlider->SetBalloonHelpString("Adjusts the opacity of image overlay.");
    this->overlayOpacitySlider->EnabledOff();
    this->overlayOpacitySlider->SetLabelPositionToLeft();
    this->overlayOpacitySlider->SetEntryPositionToRight();

    this->overlayThresholdSlider = vtkKWScaleWithEntry::New();
    this->overlayThresholdSlider->SetParent(this);
    this->overlayThresholdSlider->Create();
    this->overlayThresholdSlider->SetEntryWidth( 10 );
    this->overlayThresholdSlider->SetOrientationToHorizontal();
    this->overlayThresholdSlider->SetLabelText("Threshold");
    this->overlayThresholdSlider->SetValue(0);
    this->overlayThresholdSlider->SetRange( 0, 0 );
    this->overlayThresholdSlider->SetBalloonHelpString("Adjusts the threshold of image overlay.");
    this->overlayThresholdSlider->EnabledOff();
    this->overlayThresholdSlider->SetLabelPositionToLeft();
    this->overlayThresholdSlider->SetEntryPositionToRight();

    // Here is a radio button to toggle the overlay on/off
    this->overlayButton = vtkKWCheckButton::New();
    this->overlayButton->SetParent(this);
    this->overlayButton->Create();
    this->overlayButton->EnabledOff();
    this->overlayButton->SetText("Overlay");
    this->overlayButton->SelectedStateOn();

    // Here is a radio button to toggle the overlay on/off
    this->colorBarButton = vtkKWCheckButton::New();
    this->colorBarButton->SetParent(this);
    this->colorBarButton->Create();
    this->colorBarButton->EnabledOff();
    this->colorBarButton->SetText("Color Bar");
    this->colorBarButton->SelectedStateOn();

    // Here is a radio button to toggle the overlay on/off
    this->orthImagesButton = vtkKWCheckButton::New();
    this->orthImagesButton->SetParent(this);
    this->orthImagesButton->Create();
    this->orthImagesButton->EnabledOff();
    this->orthImagesButton->SetText("Orthogonal\n Images");
    this->orthImagesButton->SelectedStateOn();

    // Here is a progress gauge
    this->progressGauge = vtkKWProgressGauge::New();
    this->progressGauge->SetParent(this);
    this->progressGauge->Create();
    this->progressGauge->EnabledOn();
    this->progressGauge->SetValue(0);
    this->progressGauge->SetBarColor(0, 0, 0.5);

    // Here is a progress gauge
    this->loadingLabel = vtkKWLabel::New();
    this->loadingLabel->SetParent(this);
    this->loadingLabel->Create();
    this->loadingLabel->EnabledOn();
    this->loadingLabel->SetText("");
    //  ======================================================
     
    // Create a little information Box
    vtkKWLabel* titleText = vtkKWLabel::New();   
    titleText->SetParent(this);
    titleText->SetBackgroundColor(1.0, 1.0, 1.0 );
    titleText->SetText( "SIVIC");
    titleText->Create();
    titleText->SetFont( "courier 20");

    // Create a little information Box
    vtkKWLabel* infoText = vtkKWLabel::New();   
    infoText->SetParent(this);
    infoText->SetBackgroundColor(1.0, 1.0, 1.0 );

    infoText->SetText( "A prototype DICOM MR Spectroscopy viewer.\n Version: 0.3");
    infoText->Create();
    infoText->SetBorderWidth(1);
    infoText->SetRelief(1);

    // ========================================================================================
    //  View/Controller Creation
    // ========================================================================================


    //  =======================================================
    //  Now we pack the application together
    //  =======================================================
    int row = 0; 

    //==================================================================
    //  Image View Widgets Frame
    //==================================================================
    this->Script("grid %s -row %d -column 0 -sticky nsew -padx 10", this->imageViewFrame->GetWidgetName(), row); 
        this->Script("grid %s -in %s -row 0 -column 0 -sticky nw ", 
            this->volSelButton->GetWidgetName(), this->imageViewFrame->GetWidgetName(), row ); 
        this->Script( "grid %s -in %s -row 0 -column 1 -sticky nw ", 
            this->satBandButton->GetWidgetName(), this->imageViewFrame->GetWidgetName(), row ); 
        this->Script( "grid %s -in %s -row 0 -column 2 -sticky nw ", 
            this->satBandOutlineButton->GetWidgetName(), this->imageViewFrame->GetWidgetName(), row); 
        this->Script("grid %s -in %s -row 0 -column 3 -sticky nw ", 
            this->plotGridButton->GetWidgetName(), this->imageViewFrame->GetWidgetName(), row); 

        this->Script("grid columnconfigure %s 0 -weight 80 ", this->imageViewFrame->GetWidgetName() );
        this->Script("grid columnconfigure %s 1 -weight 80 ", this->imageViewFrame->GetWidgetName() );
        this->Script("grid columnconfigure %s 2 -weight 80 ", this->imageViewFrame->GetWidgetName() );
        this->Script("grid columnconfigure %s 3 -weight 80 ", this->imageViewFrame->GetWidgetName() );
    //==================================================================
    //  Ortho View Widgets Frame
    //==================================================================
    row++; 
    this->Script("grid %s -row %d -column 0 -sticky ew", orthoSeparator->GetWidgetName(), row); 
    row++; 
    this->Script("grid %s -row %d -column 0 -sticky nsew -padx 10 -pady 5 ", this->overlayViewFrame->GetWidgetName(), row);


    this->Script("grid %s -in %s -row 0 -column 2 -sticky w", 
                this->overlayButton->GetWidgetName(), this->overlayViewFrame->GetWidgetName() );
    this->Script("grid %s -in %s -row 1 -column 2 -sticky w", 
                this->colorBarButton->GetWidgetName(), this->overlayViewFrame->GetWidgetName() );
    this->Script("grid %s -in %s -row 0 -column 0 -rowspan 2 -sticky w", 
                this->lutBox->GetWidgetName(), this->overlayViewFrame->GetWidgetName() ); 
    this->Script("grid %s -in %s -row 0 -column 1 -rowspan 2 -sticky w", 
                this->interpolationBox->GetWidgetName(), this->overlayViewFrame->GetWidgetName() );
    this->Script("grid %s -in %s -row 2 -column 0  -columnspan 2 -sticky ew ", 
                this->overlayOpacitySlider->GetWidgetName(), this->overlayViewFrame->GetWidgetName() ); 
    this->Script("grid %s -in %s -row 3 -column 0  -columnspan 2 -sticky ew ", 
                this->overlayThresholdSlider->GetWidgetName(), this->overlayViewFrame->GetWidgetName() );
    this->Script("grid %s -in %s -row 2 -column 2 -rowspan 2 -sticky e", 
                this->thresholdType->GetWidgetName(), this->overlayViewFrame->GetWidgetName() ); 
    this->Script("grid columnconfigure %s 0 -weight 10 ", this->overlayViewFrame->GetWidgetName() );
    this->Script("grid columnconfigure %s 1 -weight 90 ", this->overlayViewFrame->GetWidgetName() );
    this->Script("grid columnconfigure %s 2 -weight 20 ", this->overlayViewFrame->GetWidgetName() );

    //==================================================================
    //  Overlay View Widgets Frame
    //==================================================================
    row++; 
    this->Script("grid %s -row %d -column 0 -sticky ew", overlaySeparator->GetWidgetName(), row); 
    row++; 
    this->Script("grid %s -row %d -column 0 -rowspan 1 -sticky nsew -padx 10 -pady 5", this->orthoViewFrame->GetWidgetName(), row);
        this->Script("grid %s -in %s -row 0 -column 0 -sticky ew ", 
                this->axialSlider->GetWidgetName(), this->orthoViewFrame->GetWidgetName() );
        this->Script("grid %s -in %s -row 0 -column 1 -rowspan 2 -sticky w", 
                this->orthImagesButton->GetWidgetName(), this->orthoViewFrame->GetWidgetName() );
        this->Script("grid %s -in %s -row 1 -column 0 -sticky ew ", 
                this->coronalSlider->GetWidgetName(), this->orthoViewFrame->GetWidgetName() );
        this->Script("grid %s -in %s -row 2 -column 0 -sticky ew ", 
                this->sagittalSlider->GetWidgetName(), this->orthoViewFrame->GetWidgetName() );
        this->Script("grid columnconfigure %s 0 -weight 90 ", this->orthoViewFrame->GetWidgetName() );
        this->Script("grid columnconfigure %s 1 -weight 10 ",  this->orthoViewFrame->GetWidgetName() );


    this->Script("grid rowconfigure %s 0  -weight 1", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 1  -weight 1", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 2  -weight 1", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 3  -weight 1", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 4  -weight 1", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 5  -weight 1", this->GetWidgetName() );

    this->Script("grid columnconfigure %s 0 -weight 100 -uniform 1 -minsize 200", this->GetWidgetName() );

    // Here we will add callbacks 
    this->AddCallbackCommandObserver(
        this->overlayController->GetRWInteractor(), vtkCommand::SelectionChangedEvent );

    this->AddCallbackCommandObserver(
        this->plotController->GetRWInteractor(), vtkCommand::SelectionChangedEvent );

    this->AddCallbackCommandObserver(
        this->axialSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->axialSlider->GetWidget(), vtkKWScale::ScaleValueStartChangingEvent );

    this->AddCallbackCommandObserver(
        this->coronalSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->coronalSlider->GetWidget(), vtkKWScale::ScaleValueStartChangingEvent );

    this->AddCallbackCommandObserver(
        this->sagittalSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->sagittalSlider->GetWidget(), vtkKWScale::ScaleValueStartChangingEvent );

    this->AddCallbackCommandObserver(
        this->overlayOpacitySlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->overlayOpacitySlider->GetWidget(), vtkKWScale::ScaleValueStartChangingEvent );

    this->AddCallbackCommandObserver(
        this->overlayThresholdSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->overlayThresholdSlider->GetWidget(), vtkKWScale::ScaleValueStartChangingEvent );

    this->AddCallbackCommandObserver(
        this->volSelButton, vtkKWCheckButton::SelectedStateChangedEvent );

    this->AddCallbackCommandObserver(
        this->plotGridButton, vtkKWCheckButton::SelectedStateChangedEvent );

    this->AddCallbackCommandObserver(
        this->overlayButton, vtkKWCheckButton::SelectedStateChangedEvent );

    this->AddCallbackCommandObserver(
        this->colorBarButton, vtkKWCheckButton::SelectedStateChangedEvent );

    this->AddCallbackCommandObserver(
        this->satBandButton, vtkKWCheckButton::SelectedStateChangedEvent );

    this->AddCallbackCommandObserver(
        this->satBandOutlineButton, vtkKWCheckButton::SelectedStateChangedEvent );

    this->AddCallbackCommandObserver(
        this->orthImagesButton, vtkKWCheckButton::SelectedStateChangedEvent );

    this->AddCallbackCommandObserver(
        this->interpolationBox->GetWidget(), vtkKWMenu::MenuItemInvokedEvent);

    // this is to catch overlay events to reset threshold slider
    this->AddCallbackCommandObserver(
        this->overlayController->GetRWInteractor(), vtkCommand::EndWindowLevelEvent );


    // We can delete our references to all widgets that we do not have callbacks for.
    overlaySeparator->Delete();
    orthoSeparator->Delete();
    titleText->Delete();
    infoText->Delete(); 
    
}

void sivicImageViewWidget::UpdateThreshold( )
{
    if( this->sivicController->GetThresholdType() == "Quantity" ) {

        double* dataRange = svkOverlayView::SafeDownCast(overlayController->GetView())->GetLookupTable()->GetRange();

        int numTableVals = svkOverlayView::SafeDownCast(
                                  overlayController->GetView())->GetLookupTable()->GetNumberOfTableValues();

        double currentThreshold =  svkLookupTable::SafeDownCast( 
                                      svkOverlayView::SafeDownCast(
                                         overlayController->GetView())->GetLookupTable() )->GetAlphaThreshold();
        double newThreshold = (this->overlayThresholdSlider->GetValue()-dataRange[0])/(dataRange[1]-dataRange[0]);

        int firstVisibleIndex = (int)ceil( newThreshold * numTableVals );
        double trueThreshold = ((double)firstVisibleIndex) / numTableVals;
        double thresholdValue = dataRange[0] + (trueThreshold)*(dataRange[1] - dataRange[0]);
        if( thresholdValue != this->overlayThresholdSlider->GetValue() ) {
            this->overlayThresholdSlider->SetValue( thresholdValue );
        } else {

            this->sivicController->SetOverlayThreshold( this->overlayThresholdSlider->GetValue() );
            stringstream increment;
            increment << "SetValue " 
                      << this->overlayThresholdSlider->GetValue() + this->overlayThresholdSlider->GetResolution();
            stringstream decrement;
            decrement << "SetValue " 
                      << this->overlayThresholdSlider->GetValue() - this->overlayThresholdSlider->GetResolution();
            this->overlayThresholdSlider->RemoveBinding( "<Left>");
            this->overlayThresholdSlider->AddBinding( "<Left>", this->overlayThresholdSlider, decrement.str().c_str() );
            this->overlayThresholdSlider->RemoveBinding( "<Right>");
            this->overlayThresholdSlider->AddBinding( "<Right>", this->overlayThresholdSlider, increment.str().c_str() );
            this->overlayThresholdSlider->Focus(); 
        }
    } else {

        this->sivicController->SetOverlayThreshold( this->overlayThresholdSlider->GetValue() );
        stringstream increment;
        increment << "SetValue " 
                  << this->overlayThresholdSlider->GetValue() + this->overlayThresholdSlider->GetResolution();
        stringstream decrement;
        decrement << "SetValue " 
                  << this->overlayThresholdSlider->GetValue() - this->overlayThresholdSlider->GetResolution();
        this->overlayThresholdSlider->RemoveBinding( "<Left>");
        this->overlayThresholdSlider->AddBinding( "<Left>", this->overlayThresholdSlider, decrement.str().c_str() );
        this->overlayThresholdSlider->RemoveBinding( "<Right>");
        this->overlayThresholdSlider->AddBinding( "<Right>", this->overlayThresholdSlider, increment.str().c_str() );
        this->overlayThresholdSlider->Focus(); 
    }

}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicImageViewWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    // Respond to a selection change in the overlay view
   if( caller == this->axialSlider->GetWidget()) {
        if( event != vtkKWScale::ScaleValueStartChangingEvent ) {
            this->sivicController->SetImageSlice( static_cast<int>(this->axialSlider->GetValue()) - 1, string("AXIAL")); 
        }
        int imageSlice = static_cast<int>(this->axialSlider->GetValue()) - 1; 
        stringstream increment;
        increment << "SetValue " << imageSlice + 2;
        stringstream decrement;
        decrement << "SetValue " << imageSlice;
        this->axialSlider->RemoveBinding( "<Left>");
        this->axialSlider->AddBinding( "<Left>", this->axialSlider, decrement.str().c_str() );
        this->axialSlider->RemoveBinding( "<Right>");
        this->axialSlider->AddBinding( "<Right>", this->axialSlider, increment.str().c_str() );
        this->axialSlider->Focus();
    } else if( caller == this->coronalSlider->GetWidget() ) {
        if( event != vtkKWScale::ScaleValueStartChangingEvent ) {
            this->sivicController->SetImageSlice( static_cast<int>(this->coronalSlider->GetValue()) - 1, string("CORONAL")); 
        }
        int imageSlice = static_cast<int>(this->coronalSlider->GetValue()) - 1; 
        stringstream increment;
        increment << "SetValue " << imageSlice + 2;
        stringstream decrement;
        decrement << "SetValue " << imageSlice;
        this->coronalSlider->RemoveBinding( "<Left>");
        this->coronalSlider->AddBinding( "<Left>", this->coronalSlider, decrement.str().c_str() );
        this->coronalSlider->RemoveBinding( "<Right>");
        this->coronalSlider->AddBinding( "<Right>", this->coronalSlider, increment.str().c_str() );
        this->coronalSlider->Focus();
    } else if( caller == this->sagittalSlider->GetWidget() ) {
        if( event != vtkKWScale::ScaleValueStartChangingEvent ) {
            this->sivicController->SetImageSlice( static_cast<int>(this->sagittalSlider->GetValue()) - 1, string("SAGITTAL")); 
        }
        int imageSlice = static_cast<int>(this->sagittalSlider->GetValue()) - 1; 
        stringstream increment;
        increment << "SetValue " << imageSlice + 2;
        stringstream decrement;
        decrement << "SetValue " << imageSlice;
        this->sagittalSlider->RemoveBinding( "<Left>");
        this->sagittalSlider->AddBinding( "<Left>", this->sagittalSlider, decrement.str().c_str() );
        this->sagittalSlider->RemoveBinding( "<Right>");
        this->sagittalSlider->AddBinding( "<Right>", this->sagittalSlider, increment.str().c_str() );
        this->sagittalSlider->Focus();
    } else if( caller == this->overlayOpacitySlider->GetWidget() ) {
        if( event != vtkKWScale::ScaleValueStartChangingEvent ) {
            this->overlayController->SetOverlayOpacity( this->overlayOpacitySlider->GetValue()/100.0 );
            this->plotController->SetOverlayOpacity( this->overlayOpacitySlider->GetValue()/100.0 );
        }
        stringstream increment;
        increment << "SetValue " << this->overlayOpacitySlider->GetValue() + 1;
        stringstream decrement;
        decrement << "SetValue " << this->overlayOpacitySlider->GetValue()-1;
        this->overlayOpacitySlider->RemoveBinding( "<Left>");
        this->overlayOpacitySlider->AddBinding( "<Left>", this->overlayOpacitySlider, decrement.str().c_str() );
        this->overlayOpacitySlider->RemoveBinding( "<Right>");
        this->overlayOpacitySlider->AddBinding( "<Right>", this->overlayOpacitySlider, increment.str().c_str() );
        this->overlayOpacitySlider->Focus(); 
        this->overlayController->GetView()->Refresh();
        this->plotController->GetView()->Refresh();

    } else if( caller == this->overlayThresholdSlider->GetWidget() ) {
        this->UpdateThreshold();
    } else if( caller == this->volSelButton && event == vtkKWCheckButton::SelectedStateChangedEvent) {
        if ( this->volSelButton->GetSelectedState() ) {
            this->overlayController->TurnPropOn( svkOverlayView::VOL_SELECTION );
        } else {
            this->overlayController->TurnPropOff( svkOverlayView::VOL_SELECTION );
        }
        this->overlayController->GetView()->Refresh();

    } else if( caller == this->plotGridButton && event == vtkKWCheckButton::SelectedStateChangedEvent) {
        string acquisitionType = this->model->GetDataObject("SpectroscopicData")
                                            ->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
        if ( this->plotGridButton->GetSelectedState() && acquisitionType != "SINGLE VOXEL") {
            this->overlayController->TurnPropOn( svkOverlayView::PLOT_GRID );
        } else {
            this->overlayController->TurnPropOff( svkOverlayView::PLOT_GRID );
        }
        this->overlayController->GetView()->Refresh();

    } else if( caller == this->overlayButton && event == vtkKWCheckButton::SelectedStateChangedEvent) {

        if ( this->overlayButton->GetSelectedState() ) {
            this->overlayController->TurnPropOn( svkOverlayView::AXIAL_OVERLAY_FRONT);
            this->overlayController->TurnPropOn( svkOverlayView::AXIAL_OVERLAY_BACK);
            this->overlayController->TurnPropOn( svkOverlayView::CORONAL_OVERLAY_FRONT);
            this->overlayController->TurnPropOn( svkOverlayView::CORONAL_OVERLAY_BACK);
            this->overlayController->TurnPropOn( svkOverlayView::SAGITTAL_OVERLAY_FRONT);
            this->overlayController->TurnPropOn( svkOverlayView::SAGITTAL_OVERLAY_BACK);
        } else {
            this->overlayController->TurnPropOff( svkOverlayView::AXIAL_OVERLAY_FRONT);
            this->overlayController->TurnPropOff( svkOverlayView::AXIAL_OVERLAY_BACK);
            this->overlayController->TurnPropOff( svkOverlayView::CORONAL_OVERLAY_FRONT);
            this->overlayController->TurnPropOff( svkOverlayView::CORONAL_OVERLAY_BACK);
            this->overlayController->TurnPropOff( svkOverlayView::SAGITTAL_OVERLAY_FRONT);
            this->overlayController->TurnPropOff( svkOverlayView::SAGITTAL_OVERLAY_BACK);
        }
        this->overlayController->GetView()->Refresh();

    } else if( caller == this->colorBarButton && event == vtkKWCheckButton::SelectedStateChangedEvent) {

        if ( this->colorBarButton->GetSelectedState() ) {
            this->overlayController->TurnPropOn( svkOverlayView::COLOR_BAR);
        } else {
            this->overlayController->TurnPropOff( svkOverlayView::COLOR_BAR);
        }
        this->overlayController->GetView()->Refresh();
    } else if( caller == this->satBandButton && event == vtkKWCheckButton::SelectedStateChangedEvent) {
        if ( this->satBandButton->GetSelectedState() ) {
            switch( this->overlayController->GetView()->GetOrientation() ) {
                case svkDcmHeader::AXIAL:
                    this->overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_AXIAL);
                    break;
                case svkDcmHeader::CORONAL:
                    this->overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_CORONAL);
                    break;
                case svkDcmHeader::SAGITTAL:
                    this->overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_SAGITTAL);
                    break;
            }
            this->plotController->TurnPropOn( svkPlotGridView::SAT_BANDS);
            if( this->overlayController->GetCurrentStyle() == svkOverlayViewController::ROTATION ) {
                this->overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_AXIAL);
                this->overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_SAGITTAL);
                this->overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_CORONAL);
            }
        } else {
            this->overlayController->TurnPropOff( svkOverlayView::SAT_BANDS_AXIAL);
            this->overlayController->TurnPropOff( svkOverlayView::SAT_BANDS_SAGITTAL);
            this->overlayController->TurnPropOff( svkOverlayView::SAT_BANDS_CORONAL);
            this->plotController->TurnPropOff( svkPlotGridView::SAT_BANDS);
        }
        this->overlayController->GetView()->Refresh();
    } else if( caller == this->satBandOutlineButton && event == vtkKWCheckButton::SelectedStateChangedEvent) {
        if ( this->satBandOutlineButton->GetSelectedState() ) {
            switch( this->overlayController->GetView()->GetOrientation() ) {
                case svkDcmHeader::AXIAL:
                    this->overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_AXIAL_OUTLINE);
                    break;
                case svkDcmHeader::CORONAL:
                    this->overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_CORONAL_OUTLINE);
                    break;
                case svkDcmHeader::SAGITTAL:
                    this->overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE);
                    break;
            }
            this->plotController->TurnPropOn( svkPlotGridView::SAT_BANDS_OUTLINE);
            if( this->overlayController->GetCurrentStyle() == svkOverlayViewController::ROTATION ) {
                this->overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_AXIAL_OUTLINE);
                this->overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE);
                this->overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_CORONAL_OUTLINE);
            }
        } else {
            this->overlayController->TurnPropOff( svkOverlayView::SAT_BANDS_AXIAL_OUTLINE);
            this->overlayController->TurnPropOff( svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE);
            this->overlayController->TurnPropOff( svkOverlayView::SAT_BANDS_CORONAL_OUTLINE);
            this->plotController->TurnPropOff( svkPlotGridView::SAT_BANDS_OUTLINE);
        }
        this->overlayController->GetView()->Refresh();
    // Respond to an overlay window level 
    }else if (  caller == this->overlayController->GetRWInteractor() && event == vtkCommand::EndWindowLevelEvent ) {
        if( this->sivicController->GetThresholdType() == "Quantity" ) {
            this->sivicController->SetThresholdTypeToQuantity();
        }

    } else if( caller == this->orthImagesButton && event == vtkKWCheckButton::SelectedStateChangedEvent) {

        if ( this->orthImagesButton->GetSelectedState() ) {
            this->overlayController->TurnOrthogonalImagesOn();
        } else {
            this->overlayController->TurnOrthogonalImagesOff();
        }
        this->overlayController->GetView()->Refresh();
    } 

    // Make sure the superclass gets called for render requests
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);

}
