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



#include <sivicSpectraRangeWidget.h>
#include <vtkSivicController.h>
#include <vtkJPEGReader.h>

#define MAXIMUM_RANGE_FACTOR 20 

vtkStandardNewMacro( sivicSpectraRangeWidget );
vtkCxxRevisionMacro( sivicSpectraRangeWidget, "$Revision$");

static int nearestInt(float x);   


/*! 
 *  Constructor
 */
sivicSpectraRangeWidget::sivicSpectraRangeWidget()
{
    this->unitSelectBox = NULL;
    this->componentSelectBox = NULL;
    this->xSpecRange = NULL;
    this->ySpecRange = NULL;
    this->specViewFrame = NULL;
    this->specRangeFrame = NULL;
    this->detailedPlotWindow = NULL;
    this->dataRange[0] = 0;
    this->dataRange[1] = 1;

}


/*! 
 *  Destructor
 */
sivicSpectraRangeWidget::~sivicSpectraRangeWidget()
{
    vtkKWApplication *app = this->GetApplication();

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
    if( this->specRangeFrame != NULL ) {
        this->specRangeFrame->Delete();
        this->specRangeFrame = NULL;
    }
    if( this->detailedPlotWindow != NULL ) {
        this->detailedPlotWindow->Delete();
        this->detailedPlotWindow = NULL;
    }

}


/*
 *
 */
void sivicSpectraRangeWidget::SetSpecUnitsCallback(int targetUnits)
{
    svkImageData* data = this->model->GetDataObject( "SpectroscopicData" );
    if( data == NULL ) {
        return;
    }
    double minValue = this->xSpecRange->GetEntry1()->GetValueAsDouble();
    double maxValue = this->xSpecRange->GetEntry2()->GetValueAsDouble();

    if( this->specUnits == svkSpecPoint::PTS ) {
        minValue--;
        maxValue--;
    }

    //  Convert the current values to the target unit scale:
    float lowestPoint = this->point->ConvertPosUnits(
        minValue,
        this->specUnits, 
        targetUnits 
    );

    float highestPoint = this->point->ConvertPosUnits(
        maxValue,
        this->specUnits, 
        targetUnits 
    );

    //  convert the Whole Range to the target unit scale:
    float lowestPointRange = this->point->ConvertPosUnits(
        0,
        svkSpecPoint::PTS, 
        targetUnits 
    );

    float highestPointRange = this->point->ConvertPosUnits(
        data->GetCellData()->GetArray(0)->GetNumberOfTuples()-1, 
        svkSpecPoint::PTS, 
        targetUnits 
    );

    this->specUnits = targetUnits; 

    //  Adjust the slider resolution for the target units:
    if ( targetUnits == svkSpecPoint::PPM ) {
        this->xSpecRange->SetResolution( .001 );
        this->unitSelectBox->GetWidget()->SetValue( "PPM" );
    } else if ( targetUnits == svkSpecPoint::Hz ) {
        this->xSpecRange->SetResolution( .1 );
        this->unitSelectBox->GetWidget()->SetValue( "Hz" );
    } else if ( targetUnits == svkSpecPoint::PTS ) {
        this->xSpecRange->SetResolution( 1 );
        this->unitSelectBox->GetWidget()->SetValue( "PTS" );
        // We add one to all parameters to so that the user sees a range of 1 to numPoints
        // This will be deducted before being set into the views 
        lowestPoint = (float)(nearestInt(lowestPoint))+1; 
        highestPoint = (float)(nearestInt(highestPoint))+1; 
        lowestPointRange = (float)(nearestInt(lowestPointRange))+1; 
        highestPointRange = (float)(nearestInt(highestPointRange))+1; 
    }

    this->detailedPlotController->SetUnits( this->specUnits );
    this->detailedPlotController->GetView()->Refresh( );
    this->xSpecRange->SetWholeRange( lowestPointRange, highestPointRange );
    this->xSpecRange->SetRange( lowestPoint, highestPoint ); 

}


/*!
 * Resets the x and y ranges
 */
void sivicSpectraRangeWidget::ResetRange( bool useFullFrequencyRange, bool useFullAmplitudeRange,
                                          bool resetAmplitude, bool resetFrequency)
{
    this->ResetFrequencyWholeRange( );
    if( resetFrequency ) {
        this->ResetFrequencyRange( useFullFrequencyRange );
    }

    this->ResetAmplitudeWholeRange( );
    if( resetAmplitude ) {
        this->ResetAmplitudeRange( useFullAmplitudeRange );
    }

}


/*!
 * Resets the x and y ranges
 */
void sivicSpectraRangeWidget::ResetAmplitudeWholeRange( )
{
    svkImageData* data = this->model->GetDataObject( "SpectroscopicData" ); 
    if( data != NULL ) {
        data->GetDataRange( this->dataRange, this->plotController->GetComponent()  );
        this->ySpecRange->SetWholeRange( this->dataRange[0], this->dataRange[1] );
        this->ySpecRange->SetResolution( (this->dataRange[1] - this->dataRange[0])*SLIDER_RELATIVE_RESOLUTION );
    }
}


/*!
 * Resets the x and y ranges
 */
void sivicSpectraRangeWidget::ResetAmplitudeRange( bool useFullRange )
{
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->model->GetDataObject( "SpectroscopicData" ));
    if( data != NULL ) {
        int component = this->plotController->GetComponent();
        int channel = this->plotController->GetChannel();
        int timePoint = this->plotController->GetTimePoint();
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

        string domain = model->GetDataObject( "SpectroscopicData" )
                                 ->GetDcmHeader()->GetStringValue("SignalDomainColumns");
        double range[2];

        if ( useFullRange || domain == "TIME" ) {
            data->GetDataRange( range, this->plotController->GetComponent());
        } else {
            data->EstimateDataRange( range, static_cast<int>(lowestPoint), static_cast<int>(highestPoint)
                                          , this->plotController->GetComponent(), timePoint, channel  );
            double rangeWidth = range[1]-range[0];
            range[0] -= 0.05 * rangeWidth;
            range[1] += 0.05 * rangeWidth;
        }
        this->ySpecRange->SetRange( range[0], range[1] );
        this->ySpecRange->SetResolution( (range[1] - range[0])*SLIDER_RELATIVE_RESOLUTION );
    }
}


/*!
 * Resets the x and y ranges
 */
void sivicSpectraRangeWidget::ResetFrequencyWholeRange( )
{
    svkImageData* data = this->model->GetDataObject( "SpectroscopicData" ); 
    if( data != NULL ) {
        string domain = model->GetDataObject( "SpectroscopicData" )
                                 ->GetDcmHeader()->GetStringValue("SignalDomainColumns");
        float min = 1;
        float max = data->GetCellData()->GetArray(0)->GetNumberOfTuples(); 
        if( domain == "FREQUENCY" ) {
            min = this->point->ConvertPosUnits(
                            0,
                            svkSpecPoint::PTS,
                            this->specUnits 
                                );
            max = this->point->ConvertPosUnits(
                            data->GetCellData()->GetArray(0)->GetNumberOfTuples(),
                            svkSpecPoint::PTS,
                            this->specUnits 
                                );

            this->xSpecRange->SetWholeRange( min, max );
        } else {
            this->SetSpecUnitsCallback(svkSpecPoint::PTS);
            this->xSpecRange->SetWholeRange( min, max );
        }
    }
}


/*!
 * Resets the x and y ranges
 */
void sivicSpectraRangeWidget::ResetFrequencyRange( bool useFullRange )
{
    svkImageData* data = this->model->GetDataObject( "SpectroscopicData" ); 
    if( data != NULL ) {
        string domain = model->GetDataObject( "SpectroscopicData" )
                                 ->GetDcmHeader()->GetStringValue("SignalDomainColumns");
        float min = 1;
        float max = data->GetCellData()->GetArray(0)->GetNumberOfTuples(); 
        if( domain == "FREQUENCY" ) {
            this->SetSpecUnitsCallback(svkSpecPoint::PPM);
            min = this->point->ConvertPosUnits(
                            0,
                            svkSpecPoint::PTS,
                            this->specUnits 
                                );
            max = this->point->ConvertPosUnits(
                            data->GetCellData()->GetArray(0)->GetNumberOfTuples(),
                            svkSpecPoint::PTS,
                            this->specUnits 
                                );

            if ( useFullRange ) {
                this->xSpecRange->SetRange( min, max );
            } else {
                this->xSpecRange->SetRange( PPM_DEFAULT_MIN, PPM_DEFAULT_MAX );
            }
        } else {
            this->SetSpecUnitsCallback(svkSpecPoint::PTS);
            this->xSpecRange->SetRange( min, max );
        }
        //We now need to reset the range of the plotController
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
    }
}


/*! 
 *  Method in superclass to be overriden to add our custom widgets.
 */
void sivicSpectraRangeWidget::CreateWidget()
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

    this->specRangeFrame = vtkKWFrame::New();   
    this->specRangeFrame->SetParent(this);
    this->specRangeFrame->Create();

    //  ======================================================

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
    this->unitSelectBox->SetLabelPositionToLeft();
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
    this->componentSelectBox->SetLabelText("Comp.");
    this->componentSelectBox->SetLabelPositionToLeft();
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
 
    //==================================================================
    //  Spec View Widgets Frame
    //==================================================================

        this->Script("grid %s -in %s -row 0 -column 0 -sticky wnse -padx 2 -pady 2", 
                    this->xSpecRange->GetWidgetName(), this->specRangeFrame->GetWidgetName()); 

        this->Script("grid %s -in %s -row 1 -column 0 -sticky wnse -padx 2  -pady 2", 
                    this->ySpecRange->GetWidgetName(), this->specRangeFrame->GetWidgetName()); 

        this->Script("grid columnconfigure %s 0 -weight 1 ", this->specRangeFrame->GetWidgetName() );
        this->Script("grid columnconfigure %s 1 -weight 1 ",  this->specRangeFrame->GetWidgetName() );
        this->Script("grid rowconfigure    %s 0 -weight 1 ", this->specRangeFrame->GetWidgetName() );
        this->Script("grid rowconfigure    %s 1 -weight 1 ", this->specRangeFrame->GetWidgetName() );


    this->Script("grid %s -in %s -row 0 -column 0 -sticky wnse -pady 2 ", this->specRangeFrame->GetWidgetName(), this->GetWidgetName()); 


        this->Script("grid %s -in %s -row 0 -column 0 -sticky nwse -pady 2", 
                this->unitSelectBox->GetWidgetName(), this->specViewFrame->GetWidgetName()); 

        this->Script("grid %s -in %s -row 1 -column 0 -sticky nwse -pady 2", 
                this->componentSelectBox->GetWidgetName(), this->specViewFrame->GetWidgetName()); 

        this->Script("grid %s -in %s -row 2 -column 0 -sticky nwsse -pady 2 -padx 10 ", 
                this->detailedPlotButton->GetWidgetName(), this->specViewFrame->GetWidgetName()); 

    this->Script("grid %s -in %s -row 0 -column 1 -sticky wnse -pady 2 ", this->specViewFrame->GetWidgetName(), this->GetWidgetName()); 

    this->Script("grid rowconfigure %s 0 -weight 1 -minsize 80 ", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 0 -weight 1  ", this->GetWidgetName() );



    // Here we will add callbacks 

    this->AddCallbackCommandObserver(
        this->GetApplication()->GetNthWindow(0), vtkKWWindowBase::WindowClosingEvent );

    this->AddCallbackCommandObserver(
        this->overlayController->GetRWInteractor(), vtkCommand::SelectionChangedEvent );

    this->AddCallbackCommandObserver(
        this->plotController->GetRWInteractor(), vtkCommand::SelectionChangedEvent );

    this->AddCallbackCommandObserver(
        this->xSpecRange, vtkKWRange::RangeValueChangingEvent );

    this->AddCallbackCommandObserver(
        this->xSpecRange, vtkKWRange::RangeValueStartChangingEvent );

    this->AddCallbackCommandObserver(
        this->ySpecRange, vtkKWRange::RangeValueChangingEvent );

    this->AddCallbackCommandObserver(
        this->ySpecRange, vtkKWRange::RangeValueStartChangingEvent );

    this->AddCallbackCommandObserver(
        this->unitSelectBox->GetWidget(), vtkKWMenu::MenuItemInvokedEvent);

    this->AddCallbackCommandObserver(
        this->componentSelectBox->GetWidget(), vtkKWMenu::MenuItemInvokedEvent);

    this->AddCallbackCommandObserver(
        this->detailedPlotButton, vtkKWPushButton::InvokedEvent);

    // We can delete our references to all widgets that we do not have callbacks for.
    separator->Delete();
    separatorVert->Delete();
    
}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicSpectraRangeWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
 if (  caller == this->GetApplication()->GetNthWindow(0) && event == vtkKWWindowBase::WindowClosingEvent ) {
        if( this->detailedPlotWindow != NULL ) {
            this->GetApplication()->RemoveWindow( this->detailedPlotWindow ); 
        }
 } else if (  caller == this->plotController->GetRWInteractor() && event == vtkCommand::SelectionChangedEvent ) {
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
    // Respond to a change in the x range (frequency)
    } else if( caller == this->xSpecRange ) {
        double minValue;
        double maxValue;
        xSpecRange->GetRange( minValue, maxValue ); 
        if( this->specUnits == svkSpecPoint::PTS ) {
            minValue--;
            maxValue--;
        }
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

        //  Get the display unit type and convert to points:
        //  Convert Values to points before setting the plot controller's range

        float lowestPoint = this->point->ConvertPosUnits(
            minValue,
            this->specUnits,
            svkSpecPoint::PTS 
        );
    
        float highestPoint = this->point->ConvertPosUnits(
            maxValue,
            this->specUnits,
            svkSpecPoint::PTS 
        );

        this->plotController->SetWindowLevelRange( lowestPoint, highestPoint, svkPlotGridView::FREQUENCY);
        this->detailedPlotController->SetWindowLevelRange( lowestPoint, highestPoint, svkDetailedPlotView::FREQUENCY);


    // Respond to a change in the y range (amplitude) 
    } else if( caller == this->ySpecRange ) {
        double min = this->ySpecRange->GetEntry1()->GetValueAsDouble();
        double max = this->ySpecRange->GetEntry2()->GetValueAsDouble(); 
        double* sliderRange = ySpecRange->GetWholeRange( ); 
        double viewWidth = max-min;
        double fullWidth = sliderRange[1]-sliderRange[0];
        double dataRangeMag = this->dataRange[1]-this->dataRange[0];
        
        double viewCenter = min + (max-min)/2.0;

        if( viewWidth > 0 && (fullWidth/viewWidth > MAXIMUM_RANGE_FACTOR || fullWidth < dataRangeMag) ) {
            double viewRatio[2] = { (viewCenter - this->dataRange[0])/dataRangeMag, (this->dataRange[1]-viewCenter)/dataRangeMag };
            double newWholeRangeMag = viewWidth * MAXIMUM_RANGE_FACTOR;
            double newWholeRange[2] = { viewCenter - newWholeRangeMag*viewRatio[0]
                                      , viewCenter + newWholeRangeMag*viewRatio[1] }; 
            if( newWholeRange[0] < this->dataRange[0] ) {
                newWholeRange[0] = this->dataRange[0];
            }
            if( newWholeRange[1] > this->dataRange[1] ) {
                newWholeRange[1] = this->dataRange[1];
            }
            this->ySpecRange->SetWholeRange( newWholeRange ); 
        } 

        this->plotController->SetWindowLevelRange( min, max, 1 );

        double minValue;
        double maxValue;
        this->ySpecRange->GetRange( minValue, maxValue ); 
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
        //}
    } else if( caller == this->detailedPlotButton && event == vtkKWPushButton::InvokedEvent) {
        vtkKWApplication *app = this->GetApplication();
        if( this->detailedPlotWindow == NULL ) {
            this->detailedPlotWindow = vtkKWWindowBase::New(); 
            app->AddWindow( this->detailedPlotWindow );
            this->detailedPlotWindow->Create(); 
            this->detailedPlotWindow->SetSize(500,250);
            this->detailedPlotWidget = vtkKWRenderWidget::New();
            this->detailedPlotWidget->SetParent( this->detailedPlotWindow->GetViewFrame() );
            this->detailedPlotWidget->Create();
            detailedPlotController->SetRWInteractor(this->detailedPlotWidget->GetRenderWindowInteractor());
            app->Script("pack %s -expand y -fill both -anchor c",
                    this->detailedPlotWidget->GetWidgetName());
        }
        bool foundDetailedWindow = false;
        for( int i = 0; i < app->GetNumberOfWindows(); i++ ) {
            if( app->GetNthWindow(i) == this->detailedPlotWindow ) {
                foundDetailedWindow = true;
            }
        }

        if( !foundDetailedWindow ) {
            app->AddWindow( this->detailedPlotWindow );
        }
        int* tlcBrc = this->plotController->GetTlcBrc();
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
            this->detailedPlotController->AddPlot( 0, this->plotController->GetComponent()
                                                    , this->plotController->GetChannel()
                                                    , this->plotController->GetTimePoint());
        } else {
            this->detailedPlotController->AddPlot( tlcBrc[0], this->plotController->GetComponent() 
                                                    , this->plotController->GetChannel()
                                                    , this->plotController->GetTimePoint());
        }
        this->detailedPlotController->GetView()->Refresh( );
        this->detailedPlotController->SetUnits( this->specUnits );
        this->detailedPlotController->GetView()->Refresh( );
    } 


    // Make sure the superclass gets called for render requests
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);

}


/*
 *   Returns the nearest int.  For values at the mid-point,
 *   the value is rounded to the larger int.
 */
int nearestInt(float x)
{
    int x_to_int;
    x_to_int = (int) x;

    /*
     *   First do positive numbers, then negative ones.
     */
    if (x>=0) {
        if ((x - x_to_int) >= 0.5) {
            x_to_int += 1;
        }
    } else {
        if ((x_to_int - x) > 0.5) {
            x_to_int -= 1;
        }
    }

    return (int) x_to_int;
}
