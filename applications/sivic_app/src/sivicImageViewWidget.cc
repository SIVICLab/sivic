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
//vtkCxxRevisionMacro( sivicImageViewWidget, "$Revision$");


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
    this->volumeSlider = NULL;
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

    vtkKWLabel* imageToolsLabel = vtkKWLabel::New();
    imageToolsLabel->SetParent(this->orthoViewFrame);
    imageToolsLabel->Create();
    imageToolsLabel->SetText( string("Reference Image Tools").c_str() );
    imageToolsLabel->SetJustificationToLeft();
    imageToolsLabel->SetAnchorToWest();
    imageToolsLabel->SetBorderWidth(0);
    imageToolsLabel->SetFont("arial 8 {bold}");

    this->overlayViewFrame = vtkKWFrame::New();   
    this->overlayViewFrame->SetParent(this);
    this->overlayViewFrame->Create();

    vtkKWLabel* overlayToolsLabel = vtkKWLabel::New();
    overlayToolsLabel->SetParent(this->overlayViewFrame);
    overlayToolsLabel->Create();
    overlayToolsLabel->SetText( string("Overlay Tools").c_str() );
    overlayToolsLabel->SetJustificationToLeft();
    overlayToolsLabel->SetAnchorToWest();
    overlayToolsLabel->SetBorderWidth(0);
    overlayToolsLabel->SetFont("arial 8 {bold}");

    vtkKWSeparator* orthoSeparator = vtkKWSeparator::New();   
    orthoSeparator->SetParent(this);
    orthoSeparator->Create();
    orthoSeparator->SetThickness(5);

    vtkKWSeparator* overlaySeparator = vtkKWSeparator::New();   
    overlaySeparator->SetParent(this);
    overlaySeparator->Create();
    overlaySeparator->SetThickness(5);

    int boxWidth = 14;
    this->interpolationBox = vtkKWMenuButtonWithLabel::New();   
    this->interpolationBox->SetParent(this);
    this->interpolationBox->Create();
    this->interpolationBox->SetLabelText("Interpolation");
    this->interpolationBox->EnabledOff();
    this->interpolationBox->SetLabelPositionToTop();
    this->interpolationBox->LabelVisibilityOff();
    this->interpolationBox->GetWidget()->SetFont("system 8");
    this->interpolationBox->GetWidget()->SetWidth(boxWidth);
    vtkKWMenu* interpMenu = this->interpolationBox->GetWidget()->GetMenu();
    interpMenu->AddRadioButton("Nearest Neighbor", this->sivicController, "SetInterpolationCallback 0");
    interpMenu->AddRadioButton("Linear", this->sivicController, "SetInterpolationCallback 1");
    interpMenu->AddRadioButton("Sinc", this->sivicController, "SetInterpolationCallback 2");
    interpMenu->SetFont("system 8");
    this->interpolationBox->GetWidget()->SetValue( "Nearest Neighbor" );
    int labelWidth = 8;

    this->lutBox = vtkKWMenuButtonWithLabel::New();   
    this->lutBox->SetParent(this);
    this->lutBox->Create();
    this->lutBox->SetLabelText("Color Map");
    this->lutBox->EnabledOff();
    this->lutBox->SetLabelPositionToTop();
    this->lutBox->LabelVisibilityOff();
    this->lutBox->GetWidget()->SetFont("system 8");
    this->lutBox->GetWidget()->SetWidth(boxWidth);
    vtkKWMenu* lutMenu = this->lutBox->GetWidget()->GetMenu();
    lutMenu->SetFont("system 8");

    stringstream invocation;
    invocation << "SetLUTCallback " << svkLookupTable::COLOR << endl;
    lutMenu->AddRadioButton("Color LUT", this->sivicController, invocation.str().c_str());

    invocation.str("");
    invocation << "SetLUTCallback " << svkLookupTable::GREY_SCALE << endl;
    lutMenu->AddRadioButton("Grey LUT", this->sivicController, invocation.str().c_str());

    invocation.str("");
    invocation << "SetLUTCallback " << svkLookupTable::HURD << endl;
    lutMenu->AddRadioButton("Hurd LUT", this->sivicController, invocation.str().c_str());

    invocation.str("");
    invocation << "SetLUTCallback " << svkLookupTable::CYAN_HOT << endl;
    lutMenu->AddRadioButton("Cyan LUT", this->sivicController, invocation.str().c_str());

    invocation.str("");
    invocation << "SetLUTCallback " << svkLookupTable::FIRE << endl;
    lutMenu->AddRadioButton("Fire LUT", this->sivicController, invocation.str().c_str());

    invocation.str("");
    invocation << "SetLUTCallback " << svkLookupTable::REVERSE_COLOR << endl;
    lutMenu->AddRadioButton("Reverse Color LUT", this->sivicController, invocation.str().c_str());

    invocation.str("");
    invocation << "SetLUTCallback " << svkLookupTable::CNI_FIXED << endl;
    lutMenu->AddRadioButton("Fixed CNI LUT", this->sivicController, invocation.str().c_str());

    invocation.str("");
    invocation << "SetLUTCallback " << svkLookupTable::CBF_FIXED << endl;
    lutMenu->AddRadioButton("Fixed CBF LUT", this->sivicController, invocation.str().c_str());

    this->lutBox->GetWidget()->SetValue( "Color LUT" );

    this->thresholdType = vtkKWMenuButtonWithLabel::New();   
    this->thresholdType->SetParent(this);
    this->thresholdType->Create();
    this->thresholdType->SetLabelText("Threshold Type");
    this->thresholdType->SetLabelPositionToTop();
    this->thresholdType->LabelVisibilityOff();
    this->thresholdType->GetWidget()->SetAnchorToSouth();
    this->thresholdType->EnabledOff();
    this->thresholdType->EnabledOff();
    this->thresholdType->GetWidget()->SetFont("system 8");
    this->thresholdType->GetWidget()->SetWidth(boxWidth);
    vtkKWMenu* thresholdTypeMenu = this->thresholdType->GetWidget()->GetMenu();
    thresholdTypeMenu->SetFont("system 8");

    invocation.str("");
    invocation << "SetThresholdTypeToQuantity " << endl;
    thresholdTypeMenu->AddRadioButton("Quantity", this->sivicController, invocation.str().c_str());

    invocation.str("");
    invocation << "SetThresholdTypeToPercent " << endl;
    thresholdTypeMenu->AddRadioButton("Percent", this->sivicController, invocation.str().c_str());

    this->thresholdType->GetWidget()->SetValue( "Quantity" );

    vtkKWScaleWithEntrySet* sliceSliders = vtkKWScaleWithEntrySet::New();
    sliceSliders->SetParent( this );
    sliceSliders->Create();
    sliceSliders->ExpandWidgetsOn();

    this->axialSlider = sliceSliders->AddWidget(0);
    this->axialSlider->SetParent(this);
    this->axialSlider->Create();
    this->axialSlider->SetEntryWidth( 3 );
    this->axialSlider->SetOrientationToHorizontal();
    this->axialSlider->SetLabelText("Axial Slice");
    this->axialSlider->SetValue(1);
    this->axialSlider->SetRange( 1, 1);
    this->axialSlider->SetBalloonHelpString("Adjusts axial image slice.");
    this->axialSlider->EnabledOff();
    this->axialSlider->SetPadY(1);
    this->axialSlider->SetLabelPositionToLeft();
    this->axialSlider->SetEntryPositionToRight();

    this->coronalSlider = sliceSliders->AddWidget(1);
    this->coronalSlider->SetParent(this);
    this->coronalSlider->Create();
    this->coronalSlider->SetEntryWidth( 3 );
    this->coronalSlider->SetOrientationToHorizontal();
    this->coronalSlider->SetLabelText("Coronal Slice");
    this->coronalSlider->SetValue(1);
    this->coronalSlider->SetRange( 1, 1);
    this->coronalSlider->SetPadY(1);
    this->coronalSlider->SetBalloonHelpString("Adjusts the coronal image slice.");
    this->coronalSlider->EnabledOff();
    this->coronalSlider->SetLabelPositionToLeft();
    this->coronalSlider->SetEntryPositionToRight();

    this->sagittalSlider = sliceSliders->AddWidget(2);
    this->sagittalSlider->SetParent(this);
    this->sagittalSlider->Create();
    this->sagittalSlider->SetEntryWidth( 3 );
    this->sagittalSlider->SetOrientationToHorizontal();
    this->sagittalSlider->SetLabelText("Sagittal Slice");
    this->sagittalSlider->SetValue(1);
    this->sagittalSlider->SetRange( 1, 1);
    this->sagittalSlider->SetPadY(1);
    this->sagittalSlider->SetBalloonHelpString("Adjusts the sagittal image slice.");
    this->sagittalSlider->EnabledOff();
    this->sagittalSlider->SetLabelPositionToLeft();
    this->sagittalSlider->SetEntryPositionToRight();

    this->volumeSlider = sliceSliders->AddWidget(3);
    this->volumeSlider->SetParent(this);
    this->volumeSlider->Create();
    this->volumeSlider->SetEntryWidth( 3 );
    this->volumeSlider->SetOrientationToHorizontal();
    this->volumeSlider->SetLabelText("Volume");
    this->volumeSlider->SetValue(1);
    this->volumeSlider->SetRange( 1, 1);
    this->volumeSlider->SetBalloonHelpString("Adjusts image volume.");
    this->volumeSlider->EnabledOff();
    this->volumeSlider->SetPadY(1);
    this->volumeSlider->SetLabelPositionToLeft();
    this->volumeSlider->SetEntryPositionToRight();

    // Let's setup the slice sliders in the set to be the same geometry 
    int entryWidth = 3;
    labelWidth = 11;
    vtkKWScaleWithEntry* slider = NULL;
    for (int i = 0; i < sliceSliders->GetNumberOfWidgets(); i++) {
        slider = sliceSliders->GetWidget(sliceSliders->GetIdOfNthWidget(i));
        if (sliceSliders) {
            slider->SetEntryWidth(entryWidth);
            slider->SetLabelWidth(labelWidth);
        }
    }

    int overlaySliderEntryWidth = 10;
    int overlayLabelWidth = 8;
    this->overlayOpacitySlider = vtkKWScaleWithEntry::New();
    this->overlayOpacitySlider->SetParent(this);
    this->overlayOpacitySlider->Create();
    this->overlayOpacitySlider->SetEntryWidth( overlaySliderEntryWidth );
    this->overlayOpacitySlider->SetOrientationToHorizontal();
    this->overlayOpacitySlider->SetLabelText("Opacity");
    this->overlayOpacitySlider->SetLabelWidth( overlayLabelWidth );
    this->overlayOpacitySlider->SetValue(35);
    this->overlayOpacitySlider->SetRange( 0, 100 );
    this->overlayOpacitySlider->SetBalloonHelpString("Adjusts the opacity of image overlay.");
    this->overlayOpacitySlider->EnabledOff();
    this->overlayOpacitySlider->SetEntryPositionToRight();

    this->overlayThresholdSlider = vtkKWScaleWithEntry::New();
    this->overlayThresholdSlider->SetParent(this);
    this->overlayThresholdSlider->Create();
    this->overlayThresholdSlider->SetEntryWidth( overlaySliderEntryWidth );
    this->overlayThresholdSlider->SetOrientationToHorizontal();
    this->overlayThresholdSlider->SetLabelText("Threshold");
    this->overlayThresholdSlider->SetLabelWidth( overlayLabelWidth );
    this->overlayThresholdSlider->SetValue(0);
    this->overlayThresholdSlider->SetRange( 0, 0 );
    this->overlayThresholdSlider->SetBalloonHelpString("Adjusts the threshold of image overlay.");
    this->overlayThresholdSlider->EnabledOff();
    this->overlayThresholdSlider->SetEntryPositionToRight();

    this->overlayVolumeSlider = vtkKWScaleWithEntry::New();
    this->overlayVolumeSlider->SetParent(this);
    this->overlayVolumeSlider->Create();
    this->overlayVolumeSlider->SetEntryWidth( overlaySliderEntryWidth );
    this->overlayVolumeSlider->SetOrientationToHorizontal();
    this->overlayVolumeSlider->SetLabelText("Volume");
    this->overlayVolumeSlider->SetLabelWidth( overlayLabelWidth );
    this->overlayVolumeSlider->SetValue(1);
    this->overlayVolumeSlider->SetRange( 1, 1 );
    this->overlayVolumeSlider->SetBalloonHelpString("Adjusts the volume of image overlay.");
    this->overlayVolumeSlider->EnabledOff();
    this->overlayVolumeSlider->SetEntryPositionToRight();

    vtkKWCheckButtonSet* checkButtons = vtkKWCheckButtonSet::New();
    checkButtons->SetParent( this );
    checkButtons->PackHorizontallyOn();
    checkButtons->ExpandWidgetsOn();
    checkButtons->Create();

    // Here is a radio button to toggle the overlay on/off
    this->overlayButton = checkButtons->AddWidget(0);
    this->overlayButton->SetParent(this);
    this->overlayButton->Create();
    this->overlayButton->EnabledOff();
    this->overlayButton->SetText("Overlay");
    this->overlayButton->SelectedStateOn();

    // Here is a radio button to toggle the color bar on/off
    this->colorBarButton = checkButtons->AddWidget(1);
    this->colorBarButton->SetParent(this);
    this->colorBarButton->Create();
    this->colorBarButton->EnabledOff();
    this->colorBarButton->SetText("Color Bar");
    this->colorBarButton->SelectedStateOn();

    // Here is a radio button to toggle the orthogonal images on/off
    this->orthImagesButton = vtkKWCheckButton::New();
    this->orthImagesButton->SetParent(this);
    this->orthImagesButton->Create();
    this->orthImagesButton->EnabledOff();
    this->orthImagesButton->SetText("Orthogonal\n Images");
    this->orthImagesButton->SelectedStateOn();
    this->orthImagesButton->SelectedStateOn();
    this->orthImagesButton->SetAnchorToSouth();

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
    this->Script("grid %s -row %d -column 0 -sticky nsew -padx 8", this->imageViewFrame->GetWidgetName(), row); 
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
    //  Overlay View Widgets Frame
    //==================================================================
    row++; 
    this->Script("grid %s -row %d -column 0 -sticky ew", orthoSeparator->GetWidgetName(), row); 
    row++; 
    this->Script("grid %s -row %d -column 0 -sticky nsew -padx 8 -pady 1 ", this->overlayViewFrame->GetWidgetName(), row);

    this->Script("grid %s -in %s -row 0 -column 0 -sticky we -columnspan 1 -padx 2 -pady 0",
                overlayToolsLabel->GetWidgetName(), this->overlayViewFrame->GetWidgetName() ); 
    this->Script("grid %s -in %s -row 0 -column 1 -sticky we -columnspan 2",
                checkButtons->GetWidgetName(), this->overlayViewFrame->GetWidgetName() );
    this->Script("grid %s -in %s -row 2 -column 0  -columnspan 2 -sticky ew -pady 1",
                this->overlayThresholdSlider->GetWidgetName(), this->overlayViewFrame->GetWidgetName() );
    this->Script("grid %s -in %s -row 2 -column 2 -sticky we -padx 2 -pady 1" ,
                this->thresholdType->GetWidgetName(), this->overlayViewFrame->GetWidgetName() );
    this->Script("grid %s -in %s -row 3 -column 0  -columnspan 2 -sticky ew -pady 1",
                this->overlayOpacitySlider->GetWidgetName(), this->overlayViewFrame->GetWidgetName() );
    this->Script("grid %s -in %s -row 3 -column 2  -sticky we -padx 2 -pady 1",
                this->lutBox->GetWidgetName(), this->overlayViewFrame->GetWidgetName() );
    this->Script("grid %s -in %s -row 4 -column 0  -columnspan 2 -sticky ew -pady 1",
                this->overlayVolumeSlider->GetWidgetName(), this->overlayViewFrame->GetWidgetName() );
    this->Script("grid %s -in %s -row 4 -column 2 -sticky e -columnspan 1 -padx 2 -pady 1",
                this->interpolationBox->GetWidgetName(), this->overlayViewFrame->GetWidgetName() );
    this->Script("grid columnconfigure %s 0 -weight 0 ", this->overlayViewFrame->GetWidgetName() );
    this->Script("grid columnconfigure %s 1 -weight 1 ", this->overlayViewFrame->GetWidgetName() );
    this->Script("grid columnconfigure %s 2 -weight 0 ", this->overlayViewFrame->GetWidgetName() );

    //==================================================================
    //  Ortho View Widgets Frame
    //==================================================================
    row++; 
    this->Script("grid %s -row %d -column 0 -sticky ew", overlaySeparator->GetWidgetName(), row); 
    row++; 
    this->Script("grid %s -row %d -column 0 -rowspan 1 -sticky sew -padx 8 -pady 1", this->orthoViewFrame->GetWidgetName(), row);

    this->Script("grid %s -in %s -row 0 -column 0 -sticky w -pady 0", imageToolsLabel->GetWidgetName(), this->orthoViewFrame->GetWidgetName() );
    this->Script("grid %s -in %s -row 1 -column 0 -sticky swe", sliceSliders->GetWidgetName(), this->orthoViewFrame->GetWidgetName() );
    this->Script("grid %s -in %s -row 1 -column 1 -sticky e", this->orthImagesButton->GetWidgetName(), this->orthoViewFrame->GetWidgetName() );
    
    this->Script("grid columnconfigure %s 0 -weight 1", this->orthoViewFrame->GetWidgetName() );
    this->Script("grid columnconfigure %s 1 -weight 0", this->orthoViewFrame->GetWidgetName() );
    this->Script("grid rowconfigure %s 0 -weight 1", this->orthoViewFrame->GetWidgetName() );
    this->Script("grid rowconfigure %s 0 -weight 0", this->orthoViewFrame->GetWidgetName() );

    this->Script("grid rowconfigure %s 0  -weight 0", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 1  -weight 0", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 2  -weight 1", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 3  -weight 0", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 4  -weight 0", this->GetWidgetName() );

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
        this->volumeSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->volumeSlider->GetWidget(), vtkKWScale::ScaleValueStartChangingEvent );

    this->AddCallbackCommandObserver(
        this->overlayVolumeSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->overlayVolumeSlider->GetWidget(), vtkKWScale::ScaleValueStartChangingEvent );

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
    checkButtons->Delete();
    sliceSliders->Delete();
    overlaySeparator->Delete();
    orthoSeparator->Delete();
    titleText->Delete();
    infoText->Delete(); 
    
}


/*!
 * Update the threshold value and correct for the discrete bin size of the
 * thresholding algorithm.
 */
void sivicImageViewWidget::UpdateThreshold( )
{
    if( svkOverlayView::SafeDownCast(overlayController->GetView())->GetLookupTable() == NULL ) {
        return;
    }
    if( this->sivicController->GetThresholdType() == "Quantity" ) {

        double* dataRange = svkOverlayView::SafeDownCast(overlayController->GetView())->GetLookupTable()->GetRange();

        int numTableVals = svkOverlayView::SafeDownCast(
                                  overlayController->GetView())->GetLookupTable()->GetNumberOfTableValues();
        double currentValue = this->overlayThresholdSlider->GetValue();
        double newThreshold = ((currentValue - dataRange[0])/(dataRange[1]-dataRange[0]));
        // Negative threshold makes no sense since it is a percentage.
        if( newThreshold < 0 ) {
            newThreshold = 0;
        }

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
        }
        this->overlayThresholdSlider->SetRange( dataRange[0], dataRange[1] );
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
    } else if( caller == this->volumeSlider->GetWidget() ) {
        int volume = static_cast<int>(this->volumeSlider->GetValue()) - 1;
        if( event != vtkKWScale::ScaleValueStartChangingEvent ) {
        	this->sivicController->SyncDisplayVolumes( this->model->GetDataObject("AnatomicalData"), volume );
            svkOverlayView::SafeDownCast( this->overlayController->GetView() )->SetActiveImageVolume( volume );
        }
        stringstream increment;
        increment << "SetValue " << volume + 2;
        stringstream decrement;
        decrement << "SetValue " << volume;
        this->volumeSlider->RemoveBinding( "<Left>");
        this->volumeSlider->AddBinding( "<Left>", this->volumeSlider, decrement.str().c_str() );
        this->volumeSlider->RemoveBinding( "<Right>");
        this->volumeSlider->AddBinding( "<Right>", this->volumeSlider, increment.str().c_str() );
        this->volumeSlider->Focus();
    } else if( caller == this->overlayVolumeSlider->GetWidget() ) {
        int volume = static_cast<int>(this->overlayVolumeSlider->GetValue()) - 1;
        if( event != vtkKWScale::ScaleValueStartChangingEvent ) {
        	if( this->model->DataExists("OverlayData")) {
        		this->sivicController->SyncDisplayVolumes( this->model->GetDataObject("OverlayData"), volume );
				svkOverlayView::SafeDownCast( this->overlayController->GetView() )->SetActiveOverlayVolume( volume );
        	} else if ( this->model->DataExists("MetaboliteData")){
        		this->sivicController->SyncDisplayVolumes( this->model->GetDataObject("MetaboliteData"), volume );
				svkOverlayView::SafeDownCast( this->overlayController->GetView() )->SetActiveOverlayVolume( volume );
				svkPlotGridView::SafeDownCast( this->plotController->GetView() )->SetActiveOverlayVolume( volume );
        	}
        }
        stringstream increment;
        increment << "SetValue " << volume + 2;
        stringstream decrement;
        decrement << "SetValue " << volume;
        this->overlayVolumeSlider->RemoveBinding( "<Left>");
        this->overlayVolumeSlider->AddBinding( "<Left>", this->overlayVolumeSlider, decrement.str().c_str() );
        this->overlayVolumeSlider->RemoveBinding( "<Right>");
        this->overlayVolumeSlider->AddBinding( "<Right>", this->overlayVolumeSlider, increment.str().c_str() );
        this->overlayVolumeSlider->Focus();
        this->overlayController->GetView()->Refresh();
        this->plotController->GetView()->Refresh();
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
        this->overlayThresholdSlider->Focus(); 
    } else if( caller == this->volSelButton && event == vtkKWCheckButton::SelectedStateChangedEvent) {
        if ( this->volSelButton->GetSelectedState() ) {
            svkOverlayView::SafeDownCast(this->overlayController->GetView())->ToggleSelBoxVisibilityOn();
            this->plotController->TurnPropOn( svkPlotGridView::VOL_SELECTION );
        } else {
            svkOverlayView::SafeDownCast(this->overlayController->GetView())->ToggleSelBoxVisibilityOn();
            this->plotController->TurnPropOff( svkPlotGridView::VOL_SELECTION );
        }
        this->overlayController->GetView()->Refresh();

    } else if( caller == this->plotGridButton && event == vtkKWCheckButton::SelectedStateChangedEvent) {
        string acquisitionType = "UNKNOWN";
        svk4DImageData* activeData = this->sivicController->GetActive4DImageData();
        bool isSingleVoxel = false;
        bool hasSelectionBox = false;
        if( activeData != NULL && activeData->IsA("svkMrsImageData")) {
            acquisitionType = activeData->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
            if( acquisitionType == "SINGLE VOXEL"){
            	isSingleVoxel = true;
            }
            hasSelectionBox = svkMrsImageData::SafeDownCast(activeData)->HasSelectionBox();
        }
        bool plotGridOn = this->plotGridButton->GetSelectedState();

        if ( plotGridOn && !(isSingleVoxel && hasSelectionBox) ) {
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
