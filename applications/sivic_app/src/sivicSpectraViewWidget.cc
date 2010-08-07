/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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

    if( this->overlayImageCheck != NULL ) {
        this->overlayImageCheck->Delete();
        this->overlayImageCheck = NULL;
    }

    if( this->overlayTextCheck != NULL ) {
        this->overlayTextCheck->Delete();
        this->overlayTextCheck = NULL;
    }

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

    this->sliceSlider = vtkKWScaleWithEntry::New();
    this->sliceSlider->SetParent(this);
    this->sliceSlider->Create();
    this->sliceSlider->SetEntryWidth( 2 );
    this->sliceSlider->SetOrientationToHorizontal();
    this->sliceSlider->SetLabelText("Spectra Slice");
    this->sliceSlider->SetValue(this->plotController->GetSlice()+1);
    this->sliceSlider->SetBalloonHelpString("Changes the spectroscopic slice.");
    this->sliceSlider->SetEntryPositionToRight();
    this->sliceSlider->SetLabelPositionToLeft();
    this->sliceSlider->SetRange( 1, 1 );
    this->sliceSlider->ClampValueOff();
    this->sliceSlider->EnabledOff();

    //channel slider 
    this->channelSlider = vtkKWScaleWithEntry::New();
    this->channelSlider->SetParent(this);
    this->channelSlider->Create();
    this->channelSlider->SetEntryWidth( 2 );
    this->channelSlider->SetLength( 200 );
    this->channelSlider->SetOrientationToHorizontal();
    this->channelSlider->SetLabelText("Channel       ");
    this->channelSlider->SetValue(1);
    this->channelSlider->SetBalloonHelpString("Changes the spectroscopic channel.");
    this->channelSlider->SetRange( 1, 1 );
    this->channelSlider->EnabledOff();
    this->channelSlider->SetEntryPositionToRight();
    this->channelSlider->SetLabelPositionToLeft();
    this->channelSlider->ClampValueOff();

    //channel slider 
    this->timePointSlider = vtkKWScaleWithEntry::New();
    this->timePointSlider->SetParent(this);
    this->timePointSlider->Create();
    this->timePointSlider->SetEntryWidth( 2 );
    this->timePointSlider->SetLength( 200 );
    this->timePointSlider->SetOrientationToHorizontal();
    this->timePointSlider->SetLabelText("Time Point    ");
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
    this->Script("grid %s -row %d -column 0 -rowspan 3 -columnspan 2 -sticky wnse -pady 2 ", this->specViewFrame->GetWidgetName(), row); 

        this->Script("grid %s -in %s -row 0 -column 0 -sticky wnse -padx 5 -pady 2 ", 
                    this->timePointSlider->GetWidgetName(), this->specViewFrame->GetWidgetName());

        this->Script("grid %s -in %s -row 0 -column 1 -sticky wnse -padx 2 -pady 2", 
                    this->overlayImageCheck->GetWidgetName(), this->specViewFrame->GetWidgetName()); 

        this->Script("grid %s -in %s -row 1 -column 0 -sticky wnse -padx 5 -pady 2 ", 
                    this->channelSlider->GetWidgetName(), this->specViewFrame->GetWidgetName());

        this->Script("grid %s -in %s -row 1 -column 1 -sticky wnse -padx 2 -pady 2", 
                    this->overlayTextCheck->GetWidgetName(), this->specViewFrame->GetWidgetName()); 

        this->Script("grid %s -in %s -row 2 -column 0 -sticky wnse -padx 5 -pady 2 ", 
                    this->sliceSlider->GetWidgetName(), this->specViewFrame->GetWidgetName());


    this->Script("grid columnconfigure %s 0 -weight 1 ", this->specViewFrame->GetWidgetName() );
    this->Script("grid columnconfigure %s 1 -weight 100 -minsize 110",  this->specViewFrame->GetWidgetName() );
    this->Script("grid rowconfigure    %s 0 -weight 1 ", this->specViewFrame->GetWidgetName() );
    this->Script("grid rowconfigure    %s 1 -weight 1 ", this->specViewFrame->GetWidgetName() );
    this->Script("grid rowconfigure    %s 2 -weight 1 ", this->specViewFrame->GetWidgetName() );

    this->Script("grid rowconfigure %s 0 -weight 1 -minsize 10 ", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 0 -weight 1 -minsize 100 ", this->GetWidgetName() );



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
            this->plotController->SetChannel( channel );
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
            this->plotController->SetTimePoint( timePoint );
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
