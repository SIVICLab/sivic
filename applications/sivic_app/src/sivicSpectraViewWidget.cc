/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */



#include <sivicSpectraViewWidget.h>
#include <vtkSivicController.h>
#include <vtkJPEGReader.h>

#define MINIMUM_RANGE_FACTOR 100

vtkStandardNewMacro( sivicSpectraViewWidget );
vtkCxxRevisionMacro( sivicSpectraViewWidget, "$Revision$");


/*! 
 *  Constructor
 */
sivicSpectraViewWidget::sivicSpectraViewWidget()
{
    this->unitSelectBox = NULL;
    this->componentSelectBox = NULL;
    this->channelSlider = NULL;
    this->timePointSlider = NULL;
    this->xSpecRange = NULL;
    this->ySpecRange = NULL;
    this->specViewFrame = NULL;
    this->overlayImageCheck = NULL;
    this->overlayTextCheck = NULL;
    this->sliceSlider = NULL;
    this->detailedPlotController = svkDetailedPlotViewController::New();
    this->detailedPlotWindow = NULL;

}


/*! 
 *  Destructor
 */
sivicSpectraViewWidget::~sivicSpectraViewWidget()
{
    if( this->sliceSlider != NULL ) {
        this->sliceSlider->Delete();
        this->sliceSlider = NULL;
    }

    if( this->channelSlider != NULL ) {
        this->channelSlider->Delete();
        this->channelSlider = NULL;
    }

    if( this->timePointSlider != NULL ) {
        this->timePointSlider->Delete();
        this->timePointSlider = NULL;
    }

    if( this->detailedPlotController != NULL ) {
        this->detailedPlotController->Delete();
        this->detailedPlotController = NULL;
    }

    if( this->overlayImageCheck != NULL ) {
        this->overlayImageCheck->Delete();
        this->overlayImageCheck = NULL;
    }

    if( this->overlayTextCheck != NULL ) {
        this->overlayTextCheck->Delete();
        this->overlayTextCheck = NULL;
    }

    if( this->detailedPlotButton != NULL ) {
        this->detailedPlotButton->Delete();
        this->detailedPlotButton = NULL;
    }


    if( this->xSpecRange != NULL ) {
        this->xSpecRange->Delete();
        this->xSpecRange = NULL;
    }

    if( this->unitSelectBox != NULL ) {
        this->unitSelectBox ->Delete();
        this->unitSelectBox = NULL;
    }

    if( this->componentSelectBox != NULL ) {
        this->componentSelectBox ->Delete();
        this->componentSelectBox = NULL;
    }


    if( this->ySpecRange != NULL) {
        this->ySpecRange->Delete();
        this->ySpecRange = NULL;
    }


    if( this->point != NULL ) {
        this->point->Delete();
        this->point = NULL;
    }


    if( this->specViewFrame != NULL ) {
        this->specViewFrame->Delete();
        this->specViewFrame = NULL;
    }

}


/*! 
 *  Method in superclass to be overriden to add our custom widgets.
 */
void sivicSpectraViewWidget::CreateWidget()
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
    this->specViewFrame = vtkKWFrame::New();   
    this->specViewFrame->SetParent(this);
    this->specViewFrame->Create();

     //The Master slice slider 
    this->sliceSlider = vtkKWScaleWithEntry::New();
    this->sliceSlider->SetParent(this);
    this->sliceSlider->Create();
    this->sliceSlider->SetEntryWidth( 4 );
    this->sliceSlider->SetOrientationToHorizontal();
    this->sliceSlider->SetLabelText("Slice        ");
    this->sliceSlider->SetValue(this->plotController->GetSlice()+1);
    this->sliceSlider->SetBalloonHelpString("Changes the spectroscopic slice.");
    this->sliceSlider->SetRange( 1, 1 );
    this->sliceSlider->EnabledOff();

    //channel slider 
    this->channelSlider = vtkKWScaleWithEntry::New();
    this->channelSlider->SetParent(this);
    this->channelSlider->Create();
    this->channelSlider->SetEntryWidth( 4 );
    this->channelSlider->SetLength( 200 );
    this->channelSlider->SetOrientationToHorizontal();
    this->channelSlider->SetLabelText("Channel   ");
    this->channelSlider->SetValue(1);
    this->channelSlider->SetBalloonHelpString("Changes the spectroscopic channel.");
    this->channelSlider->SetRange( 1, 1 );
    this->channelSlider->EnabledOff();
    this->channelSlider->SetEntryPositionToRight();
    this->channelSlider->SetLabelPositionToLeft();

    //channel slider 
    this->timePointSlider = vtkKWScaleWithEntry::New();
    this->timePointSlider->SetParent(this);
    this->timePointSlider->Create();
    this->timePointSlider->SetEntryWidth( 4 );
    this->timePointSlider->SetLength( 200 );
    this->timePointSlider->SetOrientationToHorizontal();
    this->timePointSlider->SetLabelText("TimePoint");
    this->timePointSlider->SetValue(1);
    this->timePointSlider->SetBalloonHelpString("Changes the spectroscopic timepoint.");
    this->timePointSlider->SetRange( 1, 1 );
    this->timePointSlider->EnabledOff();
    this->timePointSlider->SetEntryPositionToRight();
    this->timePointSlider->SetLabelPositionToLeft();
 
    //  ======================================================

    this->overlayImageCheck = vtkKWCheckButton::New();
    this->overlayImageCheck->SetParent(this);
    this->overlayImageCheck->Create();
    this->overlayImageCheck->EnabledOff();
    this->overlayImageCheck->SetPadX(2);
    this->overlayImageCheck->SetText("Overlay Image");
    this->overlayImageCheck->SelectedStateOff();

    this->overlayTextCheck = vtkKWCheckButton::New();
    this->overlayTextCheck->SetParent(this);
    this->overlayTextCheck->Create();
    this->overlayTextCheck->EnabledOff();
    this->overlayTextCheck->SetPadX(2);
    this->overlayTextCheck->SetText("Overlay Text");
    this->overlayTextCheck->SelectedStateOn();

    // Create the x range widget 
    this->xSpecRange = vtkKWRange::New(); 
    this->xSpecRange->SetParent(this);
    this->xSpecRange->SetLabelText( "Frequency" );
    this->xSpecRange->SetBalloonHelpString("Adjusts x range of the spectroscopic data.");
    this->xSpecRange->SetWholeRange(0, 1);
    this->xSpecRange->Create();
    this->xSpecRange->SetRange(0, 1);
    this->xSpecRange->EnabledOff();
    this->xSpecRange->SetSliderSize(3);
    this->xSpecRange->SetEntry1PositionToTop();
    this->xSpecRange->SetEntry2PositionToTop();
    this->xSpecRange->SetLabelPositionToTop();
    this->point = svkSpecPoint::New();
    //  Set default resolution for PPM:
    this->xSpecRange->SetResolution( .001 );

    //  X Spec Unit Selector
    this->unitSelectBox = vtkKWMenuButtonWithLabel::New();   
    this->unitSelectBox->SetParent(this);
    this->unitSelectBox->Create();
    this->unitSelectBox->SetLabelText("Units");
    this->unitSelectBox->SetLabelPositionToTop();
    this->unitSelectBox->SetPadY(10);
    this->unitSelectBox->SetPadX(8);
    this->unitSelectBox->SetHeight(2);
    this->unitSelectBox->GetWidget()->SetWidth(4);
    this->unitSelectBox->EnabledOff();
    vtkKWMenu* unitMenu = this->unitSelectBox->GetWidget()->GetMenu();
    unitMenu->AddRadioButton("PPM", this->sivicController, "SetSpecUnitsCallback 0");
    unitMenu->AddRadioButton("Hz", this->sivicController, "SetSpecUnitsCallback 1");
    unitMenu->AddRadioButton("PTS", this->sivicController, "SetSpecUnitsCallback 2");
    unitSelectBox->GetWidget()->SetValue( "PPM" );
    this->specUnits = svkSpecPoint::PPM; 

    //  Component Selector (RE, IM, MAG) 
    this->componentSelectBox = vtkKWMenuButtonWithLabel::New();   
    this->componentSelectBox->SetParent(this);
    this->componentSelectBox->Create();
    this->componentSelectBox->SetLabelText("Component");
    this->componentSelectBox->SetLabelPositionToTop();
    this->componentSelectBox->SetPadY(10);
    this->componentSelectBox->SetPadX(8);
    this->componentSelectBox->SetHeight(2);
    this->componentSelectBox->GetWidget()->SetWidth(4);
    this->componentSelectBox->EnabledOff();
    vtkKWMenu* componentMenu = this->componentSelectBox->GetWidget()->GetMenu();
    componentMenu->AddRadioButton("real", this->sivicController, "SetComponentCallback 0");
    componentMenu->AddRadioButton("imag", this->sivicController, "SetComponentCallback 1");
    componentMenu->AddRadioButton("mag", this->sivicController, "SetComponentCallback 2");
    componentSelectBox->GetWidget()->SetValue( "real" );


    // Create the y range widget 
    this->ySpecRange = vtkKWRange::New(); 
    this->ySpecRange->SetParent(this);
    this->ySpecRange->SetLabelText( "Amplitude" );
    this->ySpecRange->SetBalloonHelpString("Adjusts y range of the spectroscopic data.");
    this->ySpecRange->SetWholeRange(0, 1);
    this->ySpecRange->SetResolution(1.0);
    this->ySpecRange->Create();
    this->ySpecRange->SetRange(0, 1);
    this->ySpecRange->EnabledOff();
    this->ySpecRange->SetSliderSize(3);
    this->ySpecRange->SetEntry1PositionToTop();
    this->ySpecRange->SetEntry2PositionToTop();
    this->ySpecRange->SetLabelPositionToTop();

    this->ySpecRange->SetWholeRange(0, 1);
    this->ySpecRange->SetRange(0, 1);
    this->ySpecRange->ClampRangeOff();

    this->detailedPlotButton = vtkKWPushButton::New();
    this->detailedPlotButton->SetParent(this);
    this->detailedPlotButton->Create();
    this->detailedPlotButton->SetText("Plot");
    this->detailedPlotButton->SetBalloonHelpString("Gives you a detailed plot of the selected voxel. You must select only one voxel to activate");
    this->detailedPlotButton->EnabledOff();

    // Create separator 
    vtkKWSeparator* separator = vtkKWSeparator::New();   
    separator->SetParent(this);
    separator->Create();
    separator->SetThickness(5);
    separator->SetWidth(300);

    // Create separator 
    vtkKWSeparator* separatorVert = vtkKWSeparator::New();   
    separatorVert->SetParent(this);
    separatorVert->Create();
    separatorVert->SetThickness(5);
    separatorVert->SetOrientationToVertical();
 
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
    //  Spec View Widgets Frame
    //==================================================================
    this->Script("grid %s -row %d -column 0 -rowspan 5 -columnspan 2 -sticky wnse -pady 2 ", this->specViewFrame->GetWidgetName(), row); 

        this->Script("grid %s -in %s -row 0 -column 0 -sticky wse -padx 2 -pady 2", 
                    this->xSpecRange->GetWidgetName(), this->specViewFrame->GetWidgetName(), row); 

        this->Script("grid %s -in %s -row 0 -column 1 -sticky wse", 
                this->unitSelectBox->GetWidgetName(), this->specViewFrame->GetWidgetName()); 

        this->Script("grid %s -in %s -row 1 -column 0 -sticky wse -padx 2  -pady 2", 
                    this->ySpecRange->GetWidgetName(), this->specViewFrame->GetWidgetName(), row); 

        this->Script("grid %s -in %s -row 1 -column 1 -sticky wse", 
                this->componentSelectBox->GetWidgetName(), this->specViewFrame->GetWidgetName()); 

        this->Script("grid %s -in %s -row 2 -column 0 -sticky wse -padx 5 -pady 2 ", this->timePointSlider->GetWidgetName(), this->specViewFrame->GetWidgetName());

        this->Script("grid %s -in %s -row 2 -column 1 -sticky wse -padx 2 -pady 2", 
                    this->overlayImageCheck->GetWidgetName(), this->specViewFrame->GetWidgetName(), row); 


        this->Script("grid %s -in %s -row 3 -column 0 -sticky wnse -padx 5 -pady 2 ", this->channelSlider->GetWidgetName(), this->specViewFrame->GetWidgetName());

        this->Script("grid %s -in %s -row 3 -column 1 -sticky wse -padx 2 -pady 2", 
                    this->overlayTextCheck->GetWidgetName(), this->specViewFrame->GetWidgetName(), row); 

        this->Script("grid %s -in %s -row 4 -column 0 -sticky wse -padx 5 -pady 2 ", this->sliceSlider->GetWidgetName(), this->specViewFrame->GetWidgetName());


        this->Script("grid %s -in %s -row 4 -column 1 -sticky wse -padx 5 ", 
                this->detailedPlotButton->GetWidgetName(), this->specViewFrame->GetWidgetName()); 

    this->Script("grid columnconfigure %s 0 -weight 1 ", this->specViewFrame->GetWidgetName() );
    this->Script("grid columnconfigure %s 1 -weight 100 -minsize 110",  this->specViewFrame->GetWidgetName() );
    this->Script("grid rowconfigure    %s 0 -weight 1 ", this->specViewFrame->GetWidgetName() );
    this->Script("grid rowconfigure    %s 1 -weight 1 ", this->specViewFrame->GetWidgetName() );
    this->Script("grid rowconfigure    %s 2 -weight 1 ", this->specViewFrame->GetWidgetName() );
    this->Script("grid rowconfigure    %s 3 -weight 1 ", this->specViewFrame->GetWidgetName() );
    this->Script("grid rowconfigure    %s 4 -weight 1 ", this->specViewFrame->GetWidgetName() );

    this->Script("grid rowconfigure %s 0 -weight 1 -minsize 100 ", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 0 -weight 1 -minsize 100 ", this->GetWidgetName() );



    // Here we will add callbacks 
    this->AddCallbackCommandObserver(
        this->overlayImageCheck, vtkKWCheckButton::SelectedStateChangedEvent );

    this->AddCallbackCommandObserver(
        this->overlayTextCheck, vtkKWCheckButton::SelectedStateChangedEvent );

    this->AddCallbackCommandObserver(
        this->sliceSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->channelSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->timePointSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->overlayController->GetRWInteractor(), vtkCommand::SelectionChangedEvent );

    this->AddCallbackCommandObserver(
        this->plotController->GetRWInteractor(), vtkCommand::SelectionChangedEvent );

    this->AddCallbackCommandObserver(
        this->xSpecRange, vtkKWRange::RangeValueChangingEvent );

    this->AddCallbackCommandObserver(
        this->ySpecRange, vtkKWRange::RangeValueChangingEvent );

    this->AddCallbackCommandObserver(
        this->unitSelectBox->GetWidget(), vtkKWMenu::MenuItemInvokedEvent);

    this->AddCallbackCommandObserver(
        this->componentSelectBox->GetWidget(), vtkKWMenu::MenuItemInvokedEvent);

    this->AddCallbackCommandObserver(
        this->detailedPlotButton, vtkKWPushButton::InvokedEvent);

    // We can delete our references to all widgets that we do not have callbacks for.
    separator->Delete();
    separatorVert->Delete();
    titleText->Delete();
    infoText->Delete(); 
    
}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicSpectraViewWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
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
    } else if( caller == this->channelSlider->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent) {   
        int channel = static_cast<int>(this->channelSlider->GetValue()) - 1;
        this->plotController->SetChannel( channel );
        stringstream increment;
        increment << "SetValue " << channel + 1;
        stringstream decrement;
        decrement << "SetValue " << channel - 1;
        this->channelSlider->RemoveBinding( "<Left>");
        this->channelSlider->AddBinding( "<Left>", this->channelSlider, decrement.str().c_str() );
        this->channelSlider->RemoveBinding( "<Right>");
        this->channelSlider->AddBinding( "<Right>", this->channelSlider, increment.str().c_str() );
        this->channelSlider->Focus(); 
    } else if( caller == this->timePointSlider->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent) {   
        int timePoint = static_cast<int>(this->timePointSlider->GetValue()) - 1;
        this->plotController->SetTimePoint( timePoint );
        stringstream increment;
        increment << "SetValue " << timePoint + 1;
        stringstream decrement;
        decrement << "SetValue " << timePoint - 1;
        this->timePointSlider->RemoveBinding( "<Left>");
        this->timePointSlider->AddBinding( "<Left>", this->timePointSlider, decrement.str().c_str() );
        this->timePointSlider->RemoveBinding( "<Right>");
        this->timePointSlider->AddBinding( "<Right>", this->timePointSlider, increment.str().c_str() );
        this->timePointSlider->Focus(); 

    }else if (  caller == this->plotController->GetRWInteractor() && event == vtkCommand::SelectionChangedEvent ) {
        int * tlcBrc = overlayController->GetTlcBrc();
        string acquisitionType; 
        if( this->model->DataExists( "SpectroscopicData" ) ) {
            acquisitionType = this->model->GetDataObject( "SpectroscopicData" )->
                                GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
        }
        if( (tlcBrc[0] == tlcBrc[1] && tlcBrc[0] != -1)|| acquisitionType == "SINGLE VOXEL" ) {
            this->detailedPlotButton->EnabledOn();
        } else {
            this->detailedPlotButton->EnabledOff();
        }
    // Respond to a selection change in the plot grid view 
    } else if (  caller == this->overlayController->GetRWInteractor() && event == vtkCommand::SelectionChangedEvent ) {
        int * tlcBrc = overlayController->GetTlcBrc();
        string acquisitionType; 
        if( this->model->DataExists( "SpectroscopicData" ) ) {
            acquisitionType = this->model->GetDataObject( "SpectroscopicData" )->
                                GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
        }
        if( tlcBrc[0] == tlcBrc[1] && tlcBrc[0] != -1 || acquisitionType == "SINGLE VOXEL" ) {
            this->detailedPlotButton->EnabledOn();
        } else {
            this->detailedPlotButton->EnabledOff();
        }
    } else if( caller == this->overlayImageCheck && event == vtkKWCheckButton::SelectedStateChangedEvent) {
        if ( this->overlayImageCheck->GetSelectedState() ) {
            this->plotController->TurnPropOn( svkPlotGridView::OVERLAY_IMAGE );
        } else {
            this->plotController->TurnPropOff( svkPlotGridView::OVERLAY_IMAGE );
        }
        this->plotController->GetView()->Refresh();
    } else if( caller == this->overlayTextCheck && event == vtkKWCheckButton::SelectedStateChangedEvent) {
        if ( this->overlayTextCheck->GetSelectedState() ) {
            this->plotController->TurnPropOn( svkPlotGridView::OVERLAY_TEXT );
        } else {
            this->plotController->TurnPropOff( svkPlotGridView::OVERLAY_TEXT );
        }
        this->plotController->GetView()->Refresh();
    // Respond to a change in the x range (frequency)
    } else if( caller == this->xSpecRange && event == vtkKWRange::RangeValueChangingEvent) {
        double minValue;
        double maxValue;
        xSpecRange->GetRange( minValue, maxValue ); 
        stringstream widenRange;
        widenRange << "SetRange " << minValue - xSpecRange->GetResolution() 
                                  << " " << maxValue + xSpecRange->GetResolution();
        stringstream narrowRange;
        narrowRange << "SetRange " << minValue + xSpecRange->GetResolution() 
                                   << " " << maxValue - xSpecRange->GetResolution();
        stringstream incrementRange;
        incrementRange << "SetRange " << minValue + xSpecRange->GetResolution() 
                                      << " " << maxValue + xSpecRange->GetResolution();
        stringstream decrementRange;
        decrementRange << "SetRange " << minValue - xSpecRange->GetResolution()
                                      << " " << maxValue - xSpecRange->GetResolution();
        this->xSpecRange->RemoveBinding( "<Left>");
        this->xSpecRange->AddBinding( "<Left>", this->xSpecRange, decrementRange.str().c_str() );
        this->xSpecRange->RemoveBinding( "<Right>");
        this->xSpecRange->AddBinding( "<Right>", this->xSpecRange, incrementRange.str().c_str() );
        this->xSpecRange->RemoveBinding( "<Up>");
        this->xSpecRange->AddBinding( "<Up>", this->xSpecRange, narrowRange.str().c_str() );
        this->xSpecRange->RemoveBinding( "<Down>");
        this->xSpecRange->AddBinding( "<Down>", this->xSpecRange, widenRange.str().c_str() );
        this->xSpecRange->Focus(); 
        this->plotController->GetView()->Refresh();

        //  Get the display unit type and convert to points:
        //  Convert Values to points before setting the plot controller's range

        float lowestPoint = this->point->ConvertPosUnits(
            this->xSpecRange->GetEntry1()->GetValueAsDouble(),
            this->specUnits,
            svkSpecPoint::PTS 
        );
    
        float highestPoint = this->point->ConvertPosUnits(
            this->xSpecRange->GetEntry2()->GetValueAsDouble(),
            this->specUnits,
            svkSpecPoint::PTS 
        );

        this->plotController->SetWindowLevelRange( lowestPoint, highestPoint, svkPlotGridView::FREQUENCY);
        this->detailedPlotController->SetWindowLevelRange( lowestPoint, highestPoint, svkDetailedPlotView::FREQUENCY);


    // Respond to a change in the y range (amplitude) 
    } else if( caller == this->ySpecRange && event == vtkKWRange::RangeValueChangingEvent) {
        double newMin = this->ySpecRange->GetEntry1()->GetValueAsDouble();
        double newMax = this->ySpecRange->GetEntry2()->GetValueAsDouble(); 
        double* wholeRange = ySpecRange->GetWholeRange( ); 
        double newRangeMag = newMax-newMin;
        double wholeRangeMag = wholeRange[1]-wholeRange[0];
        if( wholeRangeMag/newRangeMag > MINIMUM_RANGE_FACTOR ) {
            
            double newCenter = (newMax-newMin)/2.0 + newMin;
            newMin = newCenter-wholeRangeMag/(MINIMUM_RANGE_FACTOR*2); 
            newMax = newCenter+wholeRangeMag/(MINIMUM_RANGE_FACTOR*2); 
            if( newMin < wholeRange[0] ) {
                newMin = wholeRange[0];
                newMax = wholeRange[0] + wholeRangeMag/MINIMUM_RANGE_FACTOR; 
            }
            if( newMin > wholeRange[1] ) {
                newMin = wholeRange[1];
                newMax = wholeRange[1] - wholeRangeMag/MINIMUM_RANGE_FACTOR; 
            }
            this->ySpecRange->SetRange( newMin, newMax );
            
        } else { 

            this->plotController->SetWindowLevelRange( newMin, newMax, 1 );

            double minValue;
            double maxValue;
            ySpecRange->GetRange( minValue, maxValue ); 
            double delta = (maxValue-minValue)/500;
            stringstream widenRange;
            widenRange << "SetRange " << minValue - delta << " " << maxValue + delta;
            stringstream narrowRange;
            narrowRange << "SetRange " << minValue + delta << " " << maxValue - delta;
            stringstream incrementRange;
            incrementRange << "SetRange " << minValue + delta << " " << maxValue + delta;
            stringstream decrementRange;
            decrementRange << "SetRange " << minValue - delta << " " << maxValue - delta;
            this->ySpecRange->RemoveBinding( "<Left>");
            this->ySpecRange->AddBinding( "<Left>", this->ySpecRange, decrementRange.str().c_str() );
            this->ySpecRange->RemoveBinding( "<Right>");
            this->ySpecRange->AddBinding( "<Right>", this->ySpecRange, incrementRange.str().c_str() );
            this->ySpecRange->RemoveBinding( "<Up>");
            this->ySpecRange->AddBinding( "<Up>", this->ySpecRange, narrowRange.str().c_str() );
            this->ySpecRange->RemoveBinding( "<Down>");
            this->ySpecRange->AddBinding( "<Down>", this->ySpecRange, widenRange.str().c_str() );
            this->ySpecRange->Focus(); 
            this->detailedPlotController->SetWindowLevelRange(minValue, maxValue, svkDetailedPlotView::AMPLITUDE);
        }
    } else if( caller == this->detailedPlotButton && event == vtkKWPushButton::InvokedEvent) {
        if( this->detailedPlotWindow == NULL ) {
            this->detailedPlotWindow = vtkKWWindowBase::New(); 
            vtkKWApplication *app = this->GetApplication();
            app->AddWindow( this->detailedPlotWindow );
            this->detailedPlotWindow->Create(); 
            this->detailedPlotWindow->SetSize(500,250);
            this->detailedPlotWidget = vtkKWRenderWidget::New();
            this->detailedPlotWidget->SetParent( this->detailedPlotWindow->GetViewFrame() );
            this->detailedPlotWidget->Create();
            detailedPlotController->SetRWInteractor(this->detailedPlotWidget->GetRenderWindowInteractor());
            app->Script("pack %s -expand y -fill both -anchor c -expand y",
            this->detailedPlotWidget->GetWidgetName());
        }
        int* tlcBrc = this->plotController->GetTlcBrc();
        cout << "Tlc is: " << tlcBrc[0] << " Brc is: " << tlcBrc[1] << endl; 
        this->detailedPlotController->SetUnits( this->specUnits );
        double minValue;
        double maxValue;
        this->xSpecRange->GetRange( minValue, maxValue ); 
        float lowestPoint = this->point->ConvertPosUnits(
            this->xSpecRange->GetEntry1()->GetValueAsDouble(),
            this->specUnits,
            svkSpecPoint::PTS 
        );
        float highestPoint = this->point->ConvertPosUnits(
            this->xSpecRange->GetEntry2()->GetValueAsDouble(),
            this->specUnits,
            svkSpecPoint::PTS 
        );
        this->detailedPlotController->SetWindowLevelRange(lowestPoint, highestPoint, svkDetailedPlotView::FREQUENCY);
        this->detailedPlotController->GetView()->Refresh( );
        this->detailedPlotWindow->Display();
        this->detailedPlotController->GetView()->Refresh( );
        this->ySpecRange->GetRange( minValue, maxValue ); 
        this->detailedPlotController->SetWindowLevelRange(minValue, maxValue, svkDetailedPlotView::AMPLITUDE);
        string acquisitionType;
        if( this->model->DataExists( "SpectroscopicData" ) ) {
            acquisitionType = this->model->GetDataObject( "SpectroscopicData" )->
                                GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
        }
        if( acquisitionType == "SINGLE VOXEL" ) {
            this->detailedPlotController->AddPlot( 0, this->plotController->GetComponent(), this->plotController->GetChannel());
        } else {
            this->detailedPlotController->AddPlot( tlcBrc[0], this->plotController->GetComponent(), this->plotController->GetChannel());
        }
        this->detailedPlotController->GetView()->Refresh( );
        this->detailedPlotController->SetUnits( this->specUnits );
        this->detailedPlotController->GetView()->Refresh( );
    } 


    // Make sure the superclass gets called for render requests
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);

}
