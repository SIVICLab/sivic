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
    this->sliceSlider = NULL;
    this->overlayOpacitySlider = NULL;
    this->overlayThresholdSlider = NULL;
    this->imageSlider = NULL;
    this->orthoXSlider = NULL;
    this->orthoYSlider = NULL;
    this->volSelButton = NULL;
    this->plotGridButton = NULL;
    this->satBandButton = NULL;
    this->satBandOutlineButton = NULL;
    this->overlayButton = NULL;
    this->colorBarButton = NULL;
    this->orthImagesButton = NULL;
    this->interpolationBox = NULL;
    this->lutBox = NULL;
    this->imageViewFrame = NULL;
    this->orthoViewFrame = NULL;
    this->overlayViewFrame = NULL;

}


/*! 
 *  Destructor
 */
sivicImageViewWidget::~sivicImageViewWidget()
{

    if( this->sliceSlider != NULL ) {
        this->sliceSlider->Delete();
        this->sliceSlider = NULL;
    }

    if( this->imageSlider != NULL ) {
        this->imageSlider->Delete();
        this->imageSlider= NULL;
    }

    if( this->orthoXSlider != NULL ) {
        this->orthoXSlider->Delete();
        this->orthoXSlider= NULL;
    }

    if( this->orthoYSlider != NULL ) {
        this->orthoYSlider->Delete();
        this->orthoYSlider= NULL;
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
  
    //The Master slice slider 
    this->sliceSlider = vtkKWScaleWithEntry::New();
    this->sliceSlider->SetParent(this);
    this->sliceSlider->Create();
    this->sliceSlider->SetEntryWidth( 4 );
    this->sliceSlider->SetOrientationToHorizontal();
    this->sliceSlider->SetLabelText("Slice");
    this->sliceSlider->SetValue(this->plotController->GetSlice()+1);
    this->sliceSlider->SetBalloonHelpString("Changes the spectroscopic slice.");
    this->sliceSlider->SetRange( 1, 1 );
    this->sliceSlider->EnabledOff();
    this->sliceSlider->SetEntryPositionToTop();
    this->sliceSlider->SetLabelPositionToTop();

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
    this->interpolationBox->SetPadY(10);
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
    this->lutBox->SetPadY(10);
    this->lutBox->EnabledOff();
    vtkKWMenu* lutMenu = this->lutBox->GetWidget()->GetMenu();
    lutMenu->AddRadioButton("color", this->sivicController, "SetLUTCallback 0");
    lutMenu->AddRadioButton("hurd", this->sivicController, "SetLUTCallback 1");
    lutMenu->AddRadioButton("cyan", this->sivicController, "SetLUTCallback 2");
    this->lutBox->GetWidget()->SetValue( "color" );

    this->imageSlider = vtkKWScaleWithEntry::New();
    this->imageSlider->SetParent(this);
    this->imageSlider->Create();
    this->imageSlider->SetEntryWidth( 3 );
    this->imageSlider->SetOrientationToHorizontal();
    this->imageSlider->SetLabelText("Image Slice  ");
    this->imageSlider->SetValue(1);
    this->imageSlider->SetRange( 1, 1);
    this->imageSlider->SetBalloonHelpString("Adjusts image slice.");
    this->imageSlider->EnabledOff();
    this->imageSlider->SetLabelPositionToLeft();
    this->imageSlider->SetEntryPositionToRight();

    this->orthoXSlider = vtkKWScaleWithEntry::New();
    this->orthoXSlider->SetParent(this);
    this->orthoXSlider->Create();
    this->orthoXSlider->SetEntryWidth( 3 );
    this->orthoXSlider->SetOrientationToHorizontal();
    this->orthoXSlider->SetLabelText("Ortho X Slice");
    this->orthoXSlider->SetValue(1);
    this->orthoXSlider->SetRange( 1, 1);
    this->orthoXSlider->SetBalloonHelpString("Adjusts the ortho view x slice.");
    this->orthoXSlider->EnabledOff();
    this->orthoXSlider->SetLabelPositionToLeft();
    this->orthoXSlider->SetEntryPositionToRight();

    this->orthoYSlider = vtkKWScaleWithEntry::New();
    this->orthoYSlider->SetParent(this);
    this->orthoYSlider->Create();
    this->orthoYSlider->SetEntryWidth( 3 );
    this->orthoYSlider->SetOrientationToHorizontal();
    this->orthoYSlider->SetLabelText("Ortho Y Slice");
    this->orthoYSlider->SetValue(1);
    this->orthoYSlider->SetRange( 1, 1);
    this->orthoYSlider->SetBalloonHelpString("Adjusts the ortho view y slice.");
    this->orthoYSlider->EnabledOff();
    this->orthoYSlider->SetLabelPositionToLeft();
    this->orthoYSlider->SetEntryPositionToRight();

    this->overlayOpacitySlider = vtkKWScaleWithEntry::New();
    this->overlayOpacitySlider->SetParent(this);
    this->overlayOpacitySlider->Create();
    this->overlayOpacitySlider->SetEntryWidth( 3 );
    this->overlayOpacitySlider->SetOrientationToHorizontal();
    this->overlayOpacitySlider->SetLabelText("Opacity   ");
    this->overlayOpacitySlider->SetValue(50);
    this->overlayOpacitySlider->SetRange( 0, 100 );
    this->overlayOpacitySlider->SetBalloonHelpString("Adjusts the opacity of image overlay.");
    this->overlayOpacitySlider->EnabledOff();
    this->overlayOpacitySlider->SetLabelPositionToLeft();
    this->overlayOpacitySlider->SetEntryPositionToRight();

    this->overlayThresholdSlider = vtkKWScaleWithEntry::New();
    this->overlayThresholdSlider->SetParent(this);
    this->overlayThresholdSlider->Create();
    this->overlayThresholdSlider->SetEntryWidth( 3 );
    this->overlayThresholdSlider->SetOrientationToHorizontal();
    this->overlayThresholdSlider->SetLabelText("Threshold");
    this->overlayThresholdSlider->SetValue(0);
    this->overlayThresholdSlider->SetRange( 0, 100 );
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
    this->orthImagesButton->SetText("Orthogonal Images");
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
    this->Script("grid %s -row %d -column 0 -rowspan 1 -sticky nsew -padx 10 -pady 10", this->orthoViewFrame->GetWidgetName(), row);
        this->Script("grid %s -in %s -row 0 -column 0 -sticky w", 
                this->orthImagesButton->GetWidgetName(), this->orthoViewFrame->GetWidgetName() );
        //this->Script("grid %s -in %s -row 1 -column 0  -columnspan 2 -sticky ew ", 
        //        this->imageSlider->GetWidgetName(), this->orthoViewFrame->GetWidgetName() );
        this->Script("grid %s -in %s -row 1 -column 0  -columnspan 2 -sticky ew ", 
                this->orthoXSlider->GetWidgetName(), this->orthoViewFrame->GetWidgetName() );
        this->Script("grid %s -in %s -row 2 -column 0 -columnspan 2 -sticky ew ", 
                this->orthoYSlider->GetWidgetName(), this->orthoViewFrame->GetWidgetName() );

        this->Script("grid columnconfigure %s 0 -weight 10 ", this->orthoViewFrame->GetWidgetName() );
        this->Script("grid columnconfigure %s 1 -weight 90 ",  this->orthoViewFrame->GetWidgetName() );
        this->Script("grid columnconfigure %s 2 -weight 20 ",  this->orthoViewFrame->GetWidgetName() );
        this->Script("grid columnconfigure %s 3 -weight 20 ",  this->orthoViewFrame->GetWidgetName() );


    //==================================================================
    //  Overlay View Widgets Frame
    //==================================================================
    row++; 
    this->Script("grid %s -row %d -column 0 -sticky ew", overlaySeparator->GetWidgetName(), row); 

    row++; 
    this->Script("grid %s -row %d -column 0 -rowspan 4 -sticky nsew -padx 10 -pady 10 ", this->overlayViewFrame->GetWidgetName(), row);

        this->Script("grid %s -in %s -row 0 -column 2 -sticky w", 
                this->overlayButton->GetWidgetName(), this->overlayViewFrame->GetWidgetName() );
        this->Script("grid %s -in %s -row 1 -column 2 -sticky w", 
                this->colorBarButton->GetWidgetName(), this->overlayViewFrame->GetWidgetName() );
        this->Script("grid %s -in %s -row 0 -column 0 -sticky w", 
                this->lutBox->GetWidgetName(), this->overlayViewFrame->GetWidgetName() ); 
        this->Script("grid %s -in %s -row 0 -column 1 -sticky w", 
                this->interpolationBox->GetWidgetName(), this->overlayViewFrame->GetWidgetName() );
        this->Script("grid %s -in %s -row 1 -column 0  -columnspan 2 -sticky ew ", 
                this->overlayOpacitySlider->GetWidgetName(), this->overlayViewFrame->GetWidgetName() ); 
        this->Script("grid %s -in %s -row 2 -column 0 -columnspan 2  -sticky ew ", 
                this->overlayThresholdSlider->GetWidgetName(), this->overlayViewFrame->GetWidgetName() );

        this->Script("grid columnconfigure %s 0 -weight 10 ", this->overlayViewFrame->GetWidgetName() );
        this->Script("grid columnconfigure %s 1 -weight 90 ", this->overlayViewFrame->GetWidgetName() );
        this->Script("grid columnconfigure %s 2 -weight 20 ", this->overlayViewFrame->GetWidgetName() );
    row++; 
    this->Script("grid %s -row 8 -column 0  -sticky nsew -padx 10 -pady 10 ", this->sliceSlider->GetWidgetName(), row);


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
        this->sliceSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->imageSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->orthoXSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->orthoYSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->overlayOpacitySlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->overlayThresholdSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

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

    // We can delete our references to all widgets that we do not have callbacks for.
    overlaySeparator->Delete();
    orthoSeparator->Delete();
    titleText->Delete();
    infoText->Delete(); 
    
}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicImageViewWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    // Respond to a selection change in the overlay view
    if( caller == this->sliceSlider->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent) {   
        this->sivicController->SetSlice( static_cast<int>(this->sliceSlider->GetValue()) - 1);
        stringstream increment;
        increment << "SetValue " << this->overlayController->GetSlice() + 2;
        stringstream decrement;
        decrement << "SetValue " << this->overlayController->GetSlice();
        this->sliceSlider->RemoveBinding( "<Left>");
        this->sliceSlider->AddBinding( "<Left>", this->sliceSlider, decrement.str().c_str() );
        this->sliceSlider->RemoveBinding( "<Right>");
        this->sliceSlider->AddBinding( "<Right>", this->sliceSlider, increment.str().c_str() );
        this->sliceSlider->Focus(); 
        //this->SetPhaseUpdateExtent();
        this->plotController->GetView()->Refresh();
        // viewer widget renders automatically when you set its slice
    } else if( caller == this->imageSlider->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent) {
        this->overlayController->SetSlice( static_cast<int>(this->imageSlider->GetValue()) - 1, 0); 
        this->overlayController->GetView()->Refresh();

    } else if( caller == this->orthoXSlider->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent) {
        this->overlayController->SetSlice( static_cast<int>(this->orthoXSlider->GetValue()) - 1, 1); 
        this->overlayController->GetView()->Refresh();

    } else if( caller == this->orthoYSlider->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent) {
        this->overlayController->SetSlice( static_cast<int>(this->orthoYSlider->GetValue()) - 1, 2); 
        this->overlayController->GetView()->Refresh();

    } else if( caller == this->overlayOpacitySlider->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent) {

        this->overlayController->SetOverlayOpacity( this->overlayOpacitySlider->GetValue()/100.0 );
        this->plotController->SetOverlayOpacity( this->overlayOpacitySlider->GetValue()/100.0 );
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

    } else if( caller == this->overlayThresholdSlider->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent) {

        this->overlayController->SetOverlayThreshold( this->overlayThresholdSlider->GetValue()/100.0 );
        this->plotController->SetOverlayThreshold( this->overlayThresholdSlider->GetValue()/100.0 );
        stringstream increment;
        increment << "SetValue " << this->overlayThresholdSlider->GetValue() + 1;
        stringstream decrement;
        decrement << "SetValue " << this->overlayThresholdSlider->GetValue()-1;
        this->overlayThresholdSlider->RemoveBinding( "<Left>");
        this->overlayThresholdSlider->AddBinding( "<Left>", this->overlayThresholdSlider, decrement.str().c_str() );
        this->overlayThresholdSlider->RemoveBinding( "<Right>");
        this->overlayThresholdSlider->AddBinding( "<Right>", this->overlayThresholdSlider, increment.str().c_str() );
        this->overlayThresholdSlider->Focus(); 
        this->overlayController->GetView()->Refresh();
        this->plotController->GetView()->Refresh();

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
            this->overlayController->TurnPropOn( svkOverlayView::OVERLAY_IMAGE);
            this->overlayController->TurnPropOn( svkOverlayView::OVERLAY_IMAGE_BACK);
        } else {
            this->overlayController->TurnPropOff( svkOverlayView::OVERLAY_IMAGE);
            this->overlayController->TurnPropOff( svkOverlayView::OVERLAY_IMAGE_BACK);
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
            this->overlayController->TurnPropOn( svkOverlayView::SAT_BANDS);
            if( this->overlayController->GetCurrentStyle() == svkOverlayViewController::ROTATION ) {
                this->overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_PERP1);
                this->overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_PERP2);
            }
        } else {
            this->overlayController->TurnPropOff( svkOverlayView::SAT_BANDS);
            this->overlayController->TurnPropOff( svkOverlayView::SAT_BANDS_PERP1);
            this->overlayController->TurnPropOff( svkOverlayView::SAT_BANDS_PERP2);
        }
        this->overlayController->GetView()->Refresh();
    } else if( caller == this->satBandOutlineButton && event == vtkKWCheckButton::SelectedStateChangedEvent) {
        if ( this->satBandOutlineButton->GetSelectedState() ) {
            this->overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_OUTLINE);
            if( this->overlayController->GetCurrentStyle() == svkOverlayViewController::ROTATION ) {
                this->overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_PERP1_OUTLINE);
                this->overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_PERP2_OUTLINE);
            }
        } else {
            this->overlayController->TurnPropOff( svkOverlayView::SAT_BANDS_OUTLINE);
            this->overlayController->TurnPropOff( svkOverlayView::SAT_BANDS_PERP1_OUTLINE);
            this->overlayController->TurnPropOff( svkOverlayView::SAT_BANDS_PERP2_OUTLINE);
        }
        this->overlayController->GetView()->Refresh();
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
