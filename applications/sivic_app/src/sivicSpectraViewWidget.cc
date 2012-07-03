/*
 *  Copyright © 2009-2011 The Regents of the University of California.
 *  All Rights Reserved.
 *
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions are met:
 *  •   Redistributions of source code must retain the above copyright notice, 
 *      this list of conditions and the following disclaimer.
 *  •   Redistributions in binary form must reproduce the above copyright notice, 
 *      this list of conditions and the following disclaimer in the documentation 
 *      and/or other materials provided with the distribution.
 *  •   None of the names of any campus of the University of California, the name 
 *      "The Regents of the University of California," or the names of any of its 
 *      contributors may be used to endorse or promote products derived from this 
 *      software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 *  OF SUCH DAMAGE.
 */

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
    this->channelSlider = NULL;
    this->timePointSlider = NULL;
    this->specViewFrame = NULL;
    this->overlayImageCheck = NULL;
    this->overlayTextCheck = NULL;
    this->syncOverlayWL = false;
    this->centerImage = true;

}


/*! 
 *  Destructor
 */
sivicSpectraViewWidget::~sivicSpectraViewWidget()
{

    if( this->specViewFrame != NULL ) {
        this->specViewFrame->Delete();
        this->specViewFrame = NULL;
    }

}

/*!     
 *   Set to true if callback for slicing should center the image inside the spectra
 */     
void sivicSpectraViewWidget::SetCenterImage( bool centerImage )
{       
    this->centerImage = centerImage;
}  


/*! 
 *  Set to true if you want it to sync to window level events from the overlay
 *  This should be done when both views are displaying the same data.
 */
void sivicSpectraViewWidget::SetSyncOverlayWL( bool syncOverlayWL )
{
    this->syncOverlayWL = syncOverlayWL;
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

    vtkKWScaleWithEntrySet* sliders = vtkKWScaleWithEntrySet::New();
    sliders->SetParent( this );
    sliders->Create();
    sliders->ExpandWidgetsOn();

    //channel slider 
    this->channelSlider = sliders->AddWidget(0);
    this->channelSlider->SetParent(this);
    this->channelSlider->Create();
    this->channelSlider->SetLabelText("Channel");
    this->channelSlider->SetValue(1);
    this->channelSlider->SetBalloonHelpString("Changes the spectroscopic channel.");
    this->channelSlider->SetRange( 1, 1 );
    this->channelSlider->EnabledOff();
    this->channelSlider->ClampValueOff();

    //channel slider 
    this->timePointSlider = sliders->AddWidget(1);
    this->timePointSlider->SetParent(this);
    this->timePointSlider->Create();
    this->timePointSlider->SetLabelText("Time Point");
    this->timePointSlider->SetValue(1);
    this->timePointSlider->SetBalloonHelpString("Changes the spectroscopic timepoint.");
    this->timePointSlider->SetRange( 1, 1 );
    this->timePointSlider->EnabledOff();
    this->timePointSlider->ClampValueOff();

    // slice slider
    this->sliceSlider = sliders->AddWidget(2);
    this->sliceSlider->SetParent(this);
    this->sliceSlider->Create();
    this->sliceSlider->SetLabelText("Slice");
    this->sliceSlider->SetValue(this->plotController->GetSlice()+1);
    this->sliceSlider->SetBalloonHelpString("Changes the spectroscopic slice.");
    this->sliceSlider->SetRange( 1, 1 );
    this->sliceSlider->ClampValueOff();
    this->sliceSlider->EnabledOff();

 
    //  ======================================================
  
    // Let's setup the sliders in the set to be the same geometry 
    int entryWidth = 2;
    int labelWidth = 9; 
    vtkKWScaleWithEntry* slider = NULL; 
    for (int i = 0; i < sliders->GetNumberOfWidgets(); i++) {
        slider = sliders->GetWidget(sliders->GetIdOfNthWidget(i));
        if (slider) {
            slider->SetEntryWidth(entryWidth);
            slider->SetLabelWidth(labelWidth);
        }
    }

    vtkKWCheckButtonSet* checkButtons = vtkKWCheckButtonSet::New();
    checkButtons->SetParent( this );
    checkButtons->Create();
    checkButtons->ExpandWidgetsOn();

    this->overlayImageCheck = checkButtons->AddWidget(0);
    this->overlayImageCheck->SetParent(this);
    this->overlayImageCheck->Create();
    this->overlayImageCheck->EnabledOff();
    this->overlayImageCheck->SetText("Overlay\n Image");
    this->overlayImageCheck->SelectedStateOn();
    this->overlayImageCheck->SetAnchorToWest();

    this->overlayTextCheck = checkButtons->AddWidget(1);
    this->overlayTextCheck->SetParent(this);
    this->overlayTextCheck->Create();
    this->overlayTextCheck->EnabledOff();
    this->overlayTextCheck->SetText("Overlay\n Text");
    this->overlayTextCheck->SelectedStateOn();
    this->overlayTextCheck->SetAnchorToWest();

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
    this->Script("grid %s -row %d -column 0 -sticky wnse ", this->specViewFrame->GetWidgetName(), row); 

        // Lets script everything into the frame
        this->Script("grid %s -in %s -row 0 -column 0 -sticky wnse", 
                    sliders->GetWidgetName(), this->specViewFrame->GetWidgetName());

        this->Script("grid %s -in %s -row 0 -column 1 -sticky wnse -padx 2 ", 
                    checkButtons->GetWidgetName(), this->specViewFrame->GetWidgetName());


        // Lets weight the sliders to take up the maximum space
        this->Script("grid columnconfigure %s 0 -weight 9 ", this->specViewFrame->GetWidgetName() );
        this->Script("grid columnconfigure %s 1 -weight 0",  this->specViewFrame->GetWidgetName() );
        this->Script("grid rowconfigure    %s 0 -weight 1 ", this->specViewFrame->GetWidgetName() );

    this->Script("grid rowconfigure %s 0 -weight 1 -minsize 10 -maxsize 30 ", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 0 -weight 1 ", this->GetWidgetName() );



    // Here we will add callbacks 
    this->AddCallbackCommandObserver(
        this->overlayImageCheck, vtkKWCheckButton::SelectedStateChangedEvent );

    this->AddCallbackCommandObserver(
        this->overlayTextCheck, vtkKWCheckButton::SelectedStateChangedEvent );

    this->AddCallbackCommandObserver(
        this->sliceSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->sliceSlider->GetWidget(), vtkKWScale::ScaleValueStartChangingEvent );

    this->AddCallbackCommandObserver(
        this->channelSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->channelSlider->GetWidget(), vtkKWScale::ScaleValueStartChangingEvent );

    this->AddCallbackCommandObserver(
        this->timePointSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->timePointSlider->GetWidget(), vtkKWScale::ScaleValueStartChangingEvent );

    this->AddCallbackCommandObserver(
        this->overlayController->GetRWInteractor(), vtkCommand::SelectionChangedEvent );

    // this is to catch overlay events
    this->AddCallbackCommandObserver(
        this->overlayController->GetRWInteractor(), vtkCommand::WindowLevelEvent );

    this->AddCallbackCommandObserver(
        this->plotController->GetRWInteractor(), vtkCommand::SelectionChangedEvent );

    // We can delete our references to all widgets that we do not have callbacks for.
    sliders->Delete();
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
    if( caller == this->sliceSlider->GetWidget()) {

        // Correct for out of bounds... not sure why KW doesn't correctly handle this.
        if( this->sliceSlider->GetValue() < this->sliceSlider->GetRangeMin() ) {
            this->sliceSlider->SetValue( this->sliceSlider->GetRangeMin());
        } else if( this->sliceSlider->GetValue() > this->sliceSlider->GetRangeMax() ) {
            this->sliceSlider->SetValue( this->sliceSlider->GetRangeMax());
        }
        if( event != vtkKWScale::ScaleValueStartChangingEvent ) {
            this->sivicController->SetSlice( static_cast<int>(this->sliceSlider->GetValue()) - 1, centerImage);
        }
        int decrementValue = this->plotController->GetSlice();
        int incrementValue = this->plotController->GetSlice() + 2;
        stringstream increment;
        increment << "SetValue " << incrementValue;
        stringstream decrement;
        decrement << "SetValue " << decrementValue;
        this->sliceSlider->RemoveBinding( "<Left>");
        this->sliceSlider->AddBinding( "<Left>", this->sliceSlider, decrement.str().c_str() );
        this->sliceSlider->RemoveBinding( "<Right>");
        this->sliceSlider->AddBinding( "<Right>", this->sliceSlider, increment.str().c_str() );
        this->sliceSlider->Focus();
    } else if( caller == this->channelSlider->GetWidget() ) {   
        // Correct for out of bounds... not sure why KW doesn't correctly handle this.
        if( this->channelSlider->GetValue() < this->channelSlider->GetRangeMin() ) {
            this->channelSlider->SetValue( this->channelSlider->GetRangeMin());
        } else if( this->channelSlider->GetValue() > this->channelSlider->GetRangeMax() ) {
            this->channelSlider->SetValue( this->channelSlider->GetRangeMax());
        }
        int channel = static_cast<int>(this->channelSlider->GetValue()) - 1;
        if( event != vtkKWScale::ScaleValueStartChangingEvent ) {
            this->plotController->SetVolumeIndex( channel, svkMrsImageData::CHANNEL );
        }
        stringstream increment;
        increment << "SetValue " << channel + 2;
        stringstream decrement;
        decrement << "SetValue " << channel;
        this->channelSlider->RemoveBinding( "<Left>");
        this->channelSlider->AddBinding( "<Left>", this->channelSlider, decrement.str().c_str() );
        this->channelSlider->RemoveBinding( "<Right>");
        this->channelSlider->AddBinding( "<Right>", this->channelSlider, increment.str().c_str() );
        this->channelSlider->Focus(); 
    } else if( caller == this->timePointSlider->GetWidget()) {   
        // Correct for out of bounds... not sure why KW doesn't correctly handle this.
        if( this->timePointSlider->GetValue() < this->timePointSlider->GetRangeMin() ) {
            this->timePointSlider->SetValue( this->timePointSlider->GetRangeMin());
        } else if( this->timePointSlider->GetValue() > this->timePointSlider->GetRangeMax() ) {
            this->timePointSlider->SetValue( this->timePointSlider->GetRangeMax());
        }
        int timePoint = static_cast<int>(this->timePointSlider->GetValue()) - 1;
        if( event != vtkKWScale::ScaleValueStartChangingEvent ) {
            this->plotController->SetVolumeIndex( timePoint, svkMrsImageData::TIMEPOINT );
        }
        stringstream increment;
        increment << "SetValue " << timePoint + 2;
        stringstream decrement;
        decrement << "SetValue " << timePoint;
        this->timePointSlider->RemoveBinding( "<Left>");
        this->timePointSlider->AddBinding( "<Left>", this->timePointSlider, decrement.str().c_str() );
        this->timePointSlider->RemoveBinding( "<Right>");
        this->timePointSlider->AddBinding( "<Right>", this->timePointSlider, increment.str().c_str() );
        this->timePointSlider->Focus(); 
    // Respond to an overlay window level 
    }else if (  caller == this->overlayController->GetRWInteractor() && event == vtkCommand::WindowLevelEvent ) {
        if( this->overlayController->GetCurrentStyle() == svkOverlayViewController::COLOR_OVERLAY && this->syncOverlayWL ) {
            double* range = svkOverlayView::SafeDownCast( this->overlayController->GetView())->GetLookupTable()->GetRange(); 
            svkPlotGridView::SafeDownCast(this->plotController->GetView())->SetOverlayWLRange(range); 
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
    } 


    // Make sure the superclass gets called for render requests
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);

}
