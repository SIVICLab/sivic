/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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



#include <sivicWindowLevelWidget.h>
#include <vtkSivicController.h>
#include <vtkJPEGReader.h>

#define MINIMUM_RANGE_FACTOR 100

vtkStandardNewMacro( sivicWindowLevelWidget );
vtkCxxRevisionMacro( sivicWindowLevelWidget, "$Revision$");


/*! 
 *  Constructor
 */
sivicWindowLevelWidget::sivicWindowLevelWidget()
{
    this->windowSlider = NULL;
    this->levelSlider = NULL;
    this->maxSlider = NULL;
    this->minSlider = NULL;
    this->imageSliders = NULL;
    this->window = 1;
    this->level = 1;
    this->windowRange[0] = 1;
    this->windowRange[1] = 1;
    this->levelRange[0] = 1;
    this->levelRange[1] = 1;
    this->target = svkOverlayViewController::REFERENCE_IMAGE;
    this->overlayDataName = "";
    this->updateEnabled = true;
    this->sliderType = NULL;
    this->syncPlotGrid = false;
    this->sliderLabel = "Window Level";


}


/*! 
 *  Destructor
 */
sivicWindowLevelWidget::~sivicWindowLevelWidget()
{
    if( this->sliderType != NULL ) {
        this->sliderType ->Delete();
        this->sliderType = NULL;
    }

    if( this->imageSliders != NULL ) {
        this->imageSliders->Delete();
        this->imageSliders = NULL;
    }


    
}


//! Should sliders update
void sivicWindowLevelWidget::EnableUpdate( )
{
    this->updateEnabled = true;
    this->UpdateSliders();
}


//! Should sliders update
void sivicWindowLevelWidget::DisableUpdate( )
{
    this->updateEnabled = false;
}


//! Set the Window
void sivicWindowLevelWidget::SetWindow( double window )
{
    this->window = window;
    if( this->IsCreated() && (this->window != window || this->window != this->windowSlider->GetValue() 
                               || this->window != this->windowSlider->GetWidget()->GetValue() )) {
        this->UpdateSliders();
    }
}


//! Set the Window range for slider
void sivicWindowLevelWidget::SetWindowRange( double windowMin, double windowMax )
{
    // Window cannot ever be exactly or less than zero.
    if( windowMin <= 0 ) {
        windowMin = (windowMax-windowMin)/1000.0;
    }
    this->windowRange[0] = windowMin;
    this->windowRange[1] = windowMax;
    this->UpdateSliders();
}


//! Set the Window range for slider
void sivicWindowLevelWidget::SetWindowRange( double* range )
{
    this->SetWindowRange( range[0], range[1] );
    this->UpdateSliders();
}


//! Set the Level
void sivicWindowLevelWidget::SetLevel(  double level ) 
{
    this->level = level;
    if( this->IsCreated() && (this->level != level || this->level != this->levelSlider->GetValue() 
                             || this->level != this->levelSlider->GetWidget()->GetValue() )) {
        this->UpdateSliders();
    }
}


//! Set the Window range for slider
void sivicWindowLevelWidget::SetLevelRange( double levelMin, double levelMax )
{
    this->levelRange[0] = levelMin;
    this->levelRange[1] = levelMax;
    this->UpdateSliders();
}

//! Set the Window range for slider
void sivicWindowLevelWidget::SetLevelRange( double* range )
{
    this->SetLevelRange( range[0], range[1] );
}


//! Set the slider label 
void sivicWindowLevelWidget::SetSliderLabel( string sliderLabel )
{
    this->sliderLabel = sliderLabel;
    if( this->imageSliders != NULL ) {
        this->imageSliders->SetLabelText(sliderLabel.c_str());
    }
}


//! Set the data name 
void sivicWindowLevelWidget::SetOverlayDataName( string overlayDataName)
{
    this->overlayDataName = overlayDataName;
}


//! Get the data name
string sivicWindowLevelWidget::GetOverlayDataName( )
{
    return this->overlayDataName;
}


//! Set the Window range for slider
void sivicWindowLevelWidget::SetSyncPlotGrid( bool syncPlotGrid )
{
    this->syncPlotGrid = syncPlotGrid;
}


//! Makes sure the sliders match the internal variables
void sivicWindowLevelWidget::UpdateSliders() 
{
    if( this->IsCreated() && this->updateEnabled) {
        this->levelSlider->SetResolution( this->windowRange[1]/500.0 );
        this->levelSlider->SetRange( this->levelRange );
        this->levelSlider->SetValue( this->level );
        this->levelSlider->GetWidget()->SetValue( this->level );
        this->windowSlider->SetResolution( this->windowRange[1]/500.0 );
        this->windowSlider->SetRange( this->windowRange );
        this->windowSlider->SetValue( this->window );
        this->windowSlider->GetWidget()->SetValue( this->window );

        this->maxSlider->SetResolution( this->windowRange[1]/500.0 );
        this->maxSlider->SetRange( this->levelRange[0]
                                 , this->levelRange[1]);
        this->maxSlider->SetValue( this->level + this->window/2.0);

        this->minSlider->SetResolution( this->windowRange[1]/500.0 );
        this->minSlider->SetRange( this->levelRange[0]
                                 , this->levelRange[1]);
        this->minSlider->SetValue( this->level - this->window/2.0);
    }
}


//! Makes sure the Views are showing the same values as the sliders.
void sivicWindowLevelWidget::UpdateView() 
{
    if( (this->target == svkOverlayViewController::REFERENCE_IMAGE && !this->model->DataExists("AnatomicalData") ) 
           || (this->target == svkOverlayViewController::IMAGE_OVERLAY && !this->model->DataExists(overlayDataName) ) ) {
        return;
    }
    
    // Lets turn of drawing while we update
    int toggleDraw = this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOff();
    }
    
    double* range = NULL;
    // Here we are using the resolution to account for potential rounding errors thout could cause excesive refreshes
    if( fabs(this->overlayController->GetWindow(this->target) - this->window) > this->windowSlider->GetResolution()/100.0 ) {
        this->overlayController->SetWindow( this->window, this->target );
        if( svkOverlayView::SafeDownCast( this->overlayController->GetView())->GetLookupTable() != NULL ) {
            range = svkOverlayView::SafeDownCast( this->overlayController->GetView())->GetLookupTable()->GetRange();
        }
    }
    if( fabs(this->overlayController->GetLevel( this->target ) - this->level) > this->levelSlider->GetResolution()/100.0 ) {
        this->overlayController->SetLevel( this->level, this->target );
        if( svkOverlayView::SafeDownCast( this->overlayController->GetView())->GetLookupTable() != NULL ) {
            range = svkOverlayView::SafeDownCast( this->overlayController->GetView())->GetLookupTable()->GetRange();
        }
    }

    if( this->syncPlotGrid && range != NULL ) {
        double* plotGridRange = svkPlotGridView::SafeDownCast(this->plotController->GetView())->GetOverlayWLRange();
        if( range[0] != plotGridRange[0] || range[1] != plotGridRange[1] ) {
            svkPlotGridView::SafeDownCast(this->plotController->GetView())->SetOverlayWLRange(range);
        }
    }

    if( this->sivicController->GetThresholdType() == "Quantity" ) {
        // This updates the threshold. This does not update the value the slider represents so it can
        this->sivicController->UpdateThreshold();
    }


    if( toggleDraw ) {
        this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
        this->overlayController->GetView()->Refresh();
    }
}


//! Set the target for the sliders
void sivicWindowLevelWidget::SetWindowLevelTarget(svkOverlayViewController::WindowLevelTarget target)
{
    this->target = target;
    if( this->imageSliders != NULL ) {
        this->imageSliders->GetLabel()->SetImageToPredefinedIcon(vtkKWIcon::IconWindowLevel);
    }
}

/*! 
 *  Method in superclass to be overriden to add our custom widgets.
 */
void sivicWindowLevelWidget::CreateWidget()
{

    // Check if already created
    if ( this->IsCreated() )
    {
        vtkErrorMacro(<< this->GetClassName() << " already created");
        return;
    }

    // Call the superclass to create the composite widget container
    this->Superclass::CreateWidget();

    this->imageSliders = vtkKWScaleWithEntrySetWithLabel::New();
    this->imageSliders->SetParent(this);
    this->imageSliders->Create();
    this->imageSliders->SetLabelText(sliderLabel.c_str());
    if( this->target == svkOverlayViewController::REFERENCE_IMAGE ) {
        this->imageSliders->GetLabel()->SetImageToPredefinedIcon(vtkKWIcon::IconWindowLevel);
    } else if( this->target == svkOverlayViewController::IMAGE_OVERLAY ) {
        this->imageSliders->GetLabel()->SetImageToPredefinedIcon(vtkKWIcon::IconColorSquares);
    }
    this->imageSliders->GetLabel()->SetFont("times 12 {bold}");
    this->imageSliders->GetLabel()->SetCompoundModeToLeft();
    this->imageSliders->GetLabel()->SetPadX(4);
    this->imageSliders->SetBorderWidth(3);
    this->imageSliders->SetLabelPositionToTop();
    this->imageSliders->GetWidget()->ExpandWidgetsOn();
    this->imageSliders->SetReliefToRaised();

    this->levelSlider = this->imageSliders->GetWidget()->AddWidget(0);
    this->levelSlider->SetParent(this);
    this->levelSlider->ExpandWidgetOn();
    this->levelSlider->Create();
    this->levelSlider->SetEntryWidth( 10 );
    this->levelSlider->SetLabelWidth( 10 );
    this->levelSlider->SetOrientationToHorizontal();
    this->levelSlider->SetLabelText("Level");
    this->levelSlider->SetBalloonHelpString("Adjusts Level.");
    this->levelSlider->SetLabelPositionToLeft();
    this->levelSlider->SetEntryPositionToRight();
    this->levelSlider->SetResolution( this->windowRange[1]/500.0 );
    this->levelSlider->SetRange( this->levelRange );
    this->levelSlider->SetValue( this->level );
    this->levelSlider->ClampValueOff();

    this->windowSlider = this->imageSliders->GetWidget()->AddWidget(1);
    this->windowSlider->SetParent(this);
    this->windowSlider->ExpandWidgetOn();
    this->windowSlider->Create();
    this->windowSlider->SetEntryWidth( 10 );
    this->windowSlider->SetLabelWidth( 10 );
    this->windowSlider->SetOrientationToHorizontal();
    this->windowSlider->SetLabelText("Window");
    this->windowSlider->SetBalloonHelpString("Adjusts Level.");
    this->windowSlider->SetLabelPositionToLeft();
    this->windowSlider->SetEntryPositionToRight();
    this->windowSlider->SetResolution( this->windowRange[1]/500.0 );
    this->windowSlider->SetRange( this->windowRange );
    this->windowSlider->SetValue( this->window );
    this->windowSlider->ClampValueOff();


    this->minSlider = this->imageSliders->GetWidget()->AddWidget(3);
    this->minSlider->SetParent(this);
    this->minSlider->ExpandWidgetOn();
    this->minSlider->Create();
    this->minSlider->SetEntryWidth( 10 );
    this->minSlider->SetLabelWidth( 10 );
    this->minSlider->SetOrientationToHorizontal();
    this->minSlider->SetLabelText("Min");

    this->minSlider->SetBalloonHelpString("Adjusts Min.");
    this->minSlider->SetLabelPositionToLeft();
    this->minSlider->SetEntryPositionToRight();
    this->minSlider->SetResolution( this->windowRange[1]/500.0 );
    this->minSlider->SetRange( this->levelRange[0]
                             , this->levelRange[1]);
    this->minSlider->SetValue( this->level - this->window/2.0);
    this->minSlider->ClampValueOff();

    this->maxSlider = this->imageSliders->GetWidget()->AddWidget(2);
    this->maxSlider->SetParent(this);
    this->maxSlider->ExpandWidgetOn();
    this->maxSlider->Create();
    this->maxSlider->SetEntryWidth( 10 );
    this->maxSlider->SetLabelWidth( 10 );
    this->maxSlider->SetOrientationToHorizontal();
    this->maxSlider->SetLabelText("Max");

    this->maxSlider->SetBalloonHelpString("Adjusts Max.");
    this->maxSlider->SetLabelPositionToLeft();
    this->maxSlider->SetEntryPositionToRight();
    this->maxSlider->SetResolution( this->windowRange[1]/500.0 );
    this->maxSlider->SetRange( this->levelRange[0]
                             , this->levelRange[1]);
    this->maxSlider->SetValue( this->level + this->window/2.0);
    this->maxSlider->ClampValueOff();


    this->sliderType = vtkKWMenuButtonWithLabel::New();
    this->sliderType->SetParent(this);
    this->sliderType->Create();
    this->sliderType->SetLabelText("Slider Type");
    this->sliderType->SetLabelPositionToLeft();
    this->sliderType->SetPadY(5);
    vtkKWMenu* interpMenu = this->sliderType->GetWidget()->GetMenu();
    interpMenu->AddRadioButton("Window/Level");
    interpMenu->AddRadioButton("Min/Max");
    interpMenu->AddRadioButton("All");
    this->sliderType->GetWidget()->SetValue( "Window/Level" );
    this->imageSliders->GetWidget()->HideWidget(2);
    this->imageSliders->GetWidget()->HideWidget(3);

    //  =======================================================
    //  Now we pack the application together
    //  =======================================================
    int row = 0; 

    this->Script("grid %s -in %s -row 0 -column 0 -sticky ew ",
             this->imageSliders->GetWidgetName(), this->GetWidgetName() );
    this->Script("grid %s -in %s -row 1 -column 0 -sticky w",
                this->sliderType->GetWidgetName(), this->GetWidgetName() );



    this->Script("grid rowconfigure %s 0  -weight 1", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 1  -weight 1", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 0  -weight 1", this->GetWidgetName() );

    this->AddCallbackCommandObserver(
        this->overlayController->GetRWInteractor(), vtkCommand::EndWindowLevelEvent );

    this->AddCallbackCommandObserver(
        this->windowSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->windowSlider->GetWidget(), vtkKWScale::ScaleValueStartChangingEvent );

    this->AddCallbackCommandObserver(
        this->levelSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->levelSlider->GetWidget(), vtkKWScale::ScaleValueStartChangingEvent );

    this->AddCallbackCommandObserver(
        this->maxSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->maxSlider->GetWidget(), vtkKWScale::ScaleValueStartChangingEvent );

    this->AddCallbackCommandObserver(
        this->minSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

    this->AddCallbackCommandObserver(
        this->minSlider->GetWidget(), vtkKWScale::ScaleValueStartChangingEvent );

    this->AddCallbackCommandObserver(
        this->sliderType->GetWidget()->GetMenu(), vtkKWMenu::MenuItemInvokedEvent);

}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicWindowLevelWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    if( caller == this->levelSlider->GetWidget() ) {
        double level = this->levelSlider->GetValue(); 
        this->SetLevel( level );
        this->UpdateView();
        stringstream increment;
        increment << "SetValue " << level + this->levelSlider->GetResolution();
        stringstream decrement;
        decrement << "SetValue " << level - this->levelSlider->GetResolution();
        this->UpdateSliders();
        this->levelSlider->RemoveBinding( "<Left>");
        this->levelSlider->AddBinding( "<Left>", this->levelSlider, decrement.str().c_str() );
        this->levelSlider->RemoveBinding( "<Right>");
        this->levelSlider->AddBinding( "<Right>", this->levelSlider, increment.str().c_str() );
        this->levelSlider->Focus();
    } else if( caller == this->windowSlider->GetWidget() ) {
        double window = this->windowSlider->GetValue(); 
        this->SetWindow( window );
        this->UpdateView();
        stringstream increment;
        increment << "SetValue " << window + this->windowSlider->GetResolution();
        stringstream decrement;
        decrement << "SetValue " << window - this->windowSlider->GetResolution();
        this->UpdateSliders();
        this->windowSlider->RemoveBinding( "<Left>");
        this->windowSlider->AddBinding( "<Left>", this->windowSlider, decrement.str().c_str() );
        this->windowSlider->RemoveBinding( "<Right>");
        this->windowSlider->AddBinding( "<Right>", this->windowSlider, increment.str().c_str() );
        this->windowSlider->Focus();
    } else if( caller == this->minSlider->GetWidget() ) {
        double min = this->minSlider->GetValue(); 
        double max = this->level + this->window/2.0;
        double window = max - min;
        double level = min + window/2.0;
        // We are not going to use the setters to avoid excessive refreshes, we set them then update
        this->window = window;
        this->level = level;
        this->UpdateSliders();
        stringstream increment;
        increment << "SetValue " << min + this->minSlider->GetResolution();
        stringstream decrement;
        decrement << "SetValue " << min - this->minSlider->GetResolution();
        this->UpdateSliders();
        this->minSlider->RemoveBinding( "<Left>");
        this->minSlider->AddBinding( "<Left>", this->minSlider, decrement.str().c_str() );
        this->minSlider->RemoveBinding( "<Right>");
        this->minSlider->AddBinding( "<Right>", this->minSlider, increment.str().c_str() );
        this->minSlider->Focus();
    } else if( caller == this->maxSlider->GetWidget() ) {
        double max = this->maxSlider->GetValue(); 
        double min = this->level - this->window/2.0;
        double window = max - min;
        double level = min + window/2.0;
        // We are not going to use the setters to avoid excessive refreshes, we set them then update
        this->window = window;
        this->level = level;
        this->UpdateSliders();
        stringstream increment;
        increment << "SetValue " << max + this->maxSlider->GetResolution();
        stringstream decrement;
        decrement << "SetValue " << max - this->maxSlider->GetResolution();
        this->UpdateSliders();
        this->maxSlider->RemoveBinding( "<Left>");
        this->maxSlider->AddBinding( "<Left>", this->maxSlider, decrement.str().c_str() );
        this->maxSlider->RemoveBinding( "<Right>");
        this->maxSlider->AddBinding( "<Right>", this->maxSlider, increment.str().c_str() );
        this->maxSlider->Focus();
    } else if (  caller == this->overlayController->GetRWInteractor() && event == vtkCommand::EndWindowLevelEvent ) {
        if(   ( this->overlayController->GetCurrentStyle() == svkOverlayViewController::WINDOW_LEVEL  
              && this->target == svkOverlayViewController::REFERENCE_IMAGE)
           || ( this->overlayController->GetCurrentStyle() == svkOverlayViewController::COLOR_OVERLAY  
              && this->target == svkOverlayViewController::IMAGE_OVERLAY) ) {
            double* pixelRange = NULL;
            if( this->target == svkOverlayViewController::REFERENCE_IMAGE ) {
                pixelRange = this->model->GetDataObject("AnatomicalData")->GetPointData()->GetArray(0)->GetRange();
            } else if ( this->target == svkOverlayViewController::IMAGE_OVERLAY ) {
                pixelRange = this->model->GetDataObject(overlayDataName)->GetPointData()->GetArray(0)->GetRange();
            }
            // We are not going to use the setters to avoid excessive refreshes, we set them then update
            this->levelRange[0] = pixelRange[0];
            this->levelRange[1] = pixelRange[1];
            this->windowRange[0] = 0;
            this->windowRange[1] = pixelRange[1]-pixelRange[0];
            this->window = this->overlayController->GetWindow( this->target );
            this->level = this->overlayController->GetLevel( this->target );
            this->UpdateSliders();
        }
    } else if( caller == this->sliderType->GetWidget()->GetMenu() ) {
        if( string(this->sliderType->GetWidget()->GetValue()).compare("Min/Max") == 0 ) {
            this->imageSliders->GetWidget()->HideWidget(0);
            this->imageSliders->GetWidget()->HideWidget(1);
            this->imageSliders->GetWidget()->ShowWidget(2);
            this->imageSliders->GetWidget()->ShowWidget(3);
        } else if( string(this->sliderType->GetWidget()->GetValue()).compare("Window/Level") == 0 ) {
            this->imageSliders->GetWidget()->ShowWidget(0);
            this->imageSliders->GetWidget()->ShowWidget(1);
            this->imageSliders->GetWidget()->HideWidget(2);
            this->imageSliders->GetWidget()->HideWidget(3);
        } else if( string(this->sliderType->GetWidget()->GetValue()).compare("All") == 0 ) {
            this->imageSliders->GetWidget()->ShowWidget(0);
            this->imageSliders->GetWidget()->ShowWidget(1);
            this->imageSliders->GetWidget()->ShowWidget(2);
            this->imageSliders->GetWidget()->ShowWidget(3);
        }
    }


    // Make sure the superclass gets called for render requests
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);

}
