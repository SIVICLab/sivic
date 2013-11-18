/*
 *  Copyright © 2009-2013 The Regents of the University of California.
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
    this->dataRange[0] = 0;
    this->dataRange[1] = 1;
    this->point = svkSpecPoint::New();

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

}


/*
 *
 */
void sivicSpectraRangeWidget::SetSpecUnitsCallback(svkSpecPoint::UnitType targetUnits)
{
    svk4DImageData* data = this->sivicController->GetActive4DImageData();
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
    float lowestPoint = this->ConvertPosUnits(
        minValue,
        this->specUnits, 
        targetUnits 
    );

    float highestPoint = this->ConvertPosUnits(
        maxValue,
        this->specUnits, 
        targetUnits 
    );

    //  convert the Whole Range to the target unit scale:
    float lowestPointRange = this->ConvertPosUnits(
        0,
        svkSpecPoint::PTS, 
        targetUnits 
    );

    float highestPointRange = this->ConvertPosUnits(
        data->GetCellData()->GetArray(0)->GetNumberOfTuples()-1, 
        svkSpecPoint::PTS, 
        targetUnits 
    );

    this->specUnits = targetUnits; 

    //  Adjust the slider resolution for the target units:
    if ( targetUnits == svkSpecPoint::PPM ) {
        this->xSpecRange->SetResolution( .001 );
        this->unitSelectBox->SetValue( "PPM" );
		this->xSpecRange->SetLabelText( "Frequency" );
    } else if ( targetUnits == svkSpecPoint::Hz ) {
        this->xSpecRange->SetResolution( .1 );
        this->unitSelectBox->SetValue( "Hz" );
		this->xSpecRange->SetLabelText( "Frequency" );
    } else if ( targetUnits == svkSpecPoint::PTS ) {
        this->xSpecRange->SetResolution( 1 );
        this->unitSelectBox->SetValue( "PTS" );
		this->xSpecRange->SetLabelText( "Points" );
        // We add one to all parameters to so that the user sees a range of 1 to numPoints
        // This will be deducted before being set into the views 
        lowestPoint = (float)(svkUtils::NearestInt(lowestPoint))+1;
        highestPoint = (float)(svkUtils::NearestInt(highestPoint))+1;
        lowestPointRange = (float)(svkUtils::NearestInt(lowestPointRange))+1;
        highestPointRange = (float)(svkUtils::NearestInt(highestPointRange))+1;
    }

    svkPlotGridView::SafeDownCast(this->plotController->GetView())->SetPlotUnits( this->specUnits );
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
    svk4DImageData* data = this->sivicController->GetActive4DImageData();
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
    svk4DImageData* data = this->sivicController->GetActive4DImageData();
    if( data != NULL ) {
        int component = this->plotController->GetComponent();
        float lowestPoint = this->ConvertPosUnits(
            this->xSpecRange->GetEntry1()->GetValueAsDouble(),
            this->specUnits,
            svkSpecPoint::PTS
        );

        float highestPoint = this->ConvertPosUnits(
            this->xSpecRange->GetEntry2()->GetValueAsDouble(),
            this->specUnits,
            svkSpecPoint::PTS
        );

        string domain = "TIME";
        if( data->IsA("svkMrsImageData")) {
            domain = data->GetDcmHeader()->GetStringValue("SignalDomainColumns");
        }
        double range[2];

        if ( useFullRange || domain == "TIME" ) {
            data->GetDataRange( range, this->plotController->GetComponent());
        } else {
            int lowInt = static_cast<int>(lowestPoint);
            int highInt =  static_cast<int>(highestPoint);
            int component = this->plotController->GetComponent();
            int* volumeIndexArray = this->plotController->GetVolumeIndexArray();
            int* tlcBrc = this->plotController->GetTlcBrc();
            data->EstimateDataRange( range, lowInt, highInt, component, tlcBrc , volumeIndexArray );
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
    svk4DImageData* data = this->sivicController->GetActive4DImageData();
    if( data != NULL ) {
        string domain = "TIME";
        if( data->IsA("svkMrsImageData")) {
            domain = data->GetDcmHeader()->GetStringValue("SignalDomainColumns");
        }
        float min = 1;
        float max = data->GetCellData()->GetArray(0)->GetNumberOfTuples(); 
        if( domain == "FREQUENCY" ) {
            min = this->ConvertPosUnits(
                            0,
                            svkSpecPoint::PTS,
                            this->specUnits 
                                );
            max = this->ConvertPosUnits(
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
    svk4DImageData* data = this->sivicController->GetActive4DImageData();
    if( data != NULL ) {
        string domain = "TIME";
        if( data->IsA("svkMrsImageData")) {
            domain = data->GetDcmHeader()->GetStringValue("SignalDomainColumns");
        }
        float min = 1;
        float max = data->GetCellData()->GetArray(0)->GetNumberOfTuples(); 
        if( domain == "FREQUENCY" ) {
            this->SetSpecUnitsCallback(svkSpecPoint::PPM);
            min = this->ConvertPosUnits(
                            0,
                            svkSpecPoint::PTS,
                            this->specUnits 
                                );
            max = this->ConvertPosUnits(
                            data->GetCellData()->GetArray(0)->GetNumberOfTuples(),
                            svkSpecPoint::PTS,
                            this->specUnits 
                                );

            if ( useFullRange ) {
                this->xSpecRange->SetRange( min, max );
            } else {

                float ppmMin;  
                float ppmMax;  
                //  The default range must be within the actual range of the data. If not, use full range: 
                this->sivicController->GetMRSDefaultPPMRange( data, ppmMin, ppmMax ); 
                if ( ppmMin > min || ppmMax < max ) { 
                    ppmMin = min; 
                    ppmMax = max; 
                } 
                        
                this->xSpecRange->SetRange( ppmMin, ppmMax);
            }
        } else {
            this->SetSpecUnitsCallback(svkSpecPoint::PTS);
            this->xSpecRange->SetRange( min, max );
        }
        //We now need to reset the range of the plotController
        double currentRange[2] = {this->xSpecRange->GetEntry1()->GetValueAsDouble(),
                                  this->xSpecRange->GetEntry2()->GetValueAsDouble()};
        // Adjust for 0 to 1 indexing if we are using points
        if( this->specUnits == svkSpecPoint::PTS ) {
            currentRange[0]--;
            currentRange[1]--;
        }
        float lowestPoint = this->ConvertPosUnits(
            currentRange[0],
            this->specUnits,
            svkSpecPoint::PTS
        );

        float highestPoint = this->ConvertPosUnits(
            currentRange[1],
            this->specUnits,
            svkSpecPoint::PTS
        );

        this->plotController->SetWindowLevelRange( lowestPoint, highestPoint, svkPlotGridView::FREQUENCY);
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

    int rangeEntryWidth = 10;
    int rangeLabelWidth = 9;
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
    this->xSpecRange->SetEntriesWidth(10);
    this->xSpecRange->SetEntry1PositionToLeft();
    this->xSpecRange->SetEntry2PositionToRight();
    this->xSpecRange->SetLabelPositionToLeft();
    this->xSpecRange->SetLabelWidth( rangeLabelWidth );
    this->xSpecRange->SetResolution( .001 );

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
    this->ySpecRange->SetEntriesWidth(10);
    this->ySpecRange->SetEntry1PositionToLeft();
    this->ySpecRange->SetEntry2PositionToRight();
    this->ySpecRange->SetLabelPositionToLeft();

    this->ySpecRange->SetWholeRange(0, 1);
    this->ySpecRange->SetRange(0, 1);
    this->ySpecRange->ClampRangeOff();
    //  Set default resolution for PPM:
    this->ySpecRange->SetLabelWidth( rangeLabelWidth );

    int menuLabelWidth = 0;
    int menuWidth = 3;
    int menuHeight = 2;
    int menuPadY = 3;

    //  X Spec Unit Selector
    this->unitSelectBox = vtkKWMenuButton::New();   
    this->unitSelectBox->SetParent(this);
    this->unitSelectBox->Create();
    this->unitSelectBox->EnabledOff();
    this->unitSelectBox->SetFont("system 8");

    vtkKWMenu* unitMenu = this->unitSelectBox->GetMenu();
    unitMenu->AddRadioButton("PPM", this->sivicController, "SetSpecUnitsCallback 0");
    unitMenu->AddRadioButton("Hz", this->sivicController, "SetSpecUnitsCallback 1");
    unitMenu->AddRadioButton("PTS", this->sivicController, "SetSpecUnitsCallback 2");
    unitSelectBox->SetValue( "PPM" );
    this->specUnits = svkSpecPoint::PPM; 

    //  Component Selector (RE, IM, MAG) 
    this->componentSelectBox = vtkKWMenuButton::New();   
    this->componentSelectBox->SetParent(this);
    this->componentSelectBox->Create();
    this->componentSelectBox->EnabledOff();
    this->componentSelectBox->SetFont("system 8");

    vtkKWMenu* componentMenu = this->componentSelectBox->GetMenu();
    componentMenu->AddRadioButton("real", this->sivicController, "SetComponentCallback 0");
    componentMenu->AddRadioButton("imag", this->sivicController, "SetComponentCallback 1");
    componentMenu->AddRadioButton("mag", this->sivicController, "SetComponentCallback 2");
    componentSelectBox->SetValue( "real" );

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

    this->Script("grid %s -in %s -row 1 -column 0 -sticky wnse -padx 2 -pady 2", 
                    this->ySpecRange->GetWidgetName(), this->specRangeFrame->GetWidgetName()); 

    this->Script("grid columnconfigure %s 0 -weight 1 ", this->specRangeFrame->GetWidgetName() );
    this->Script("grid columnconfigure %s 1 -weight 1 ",  this->specRangeFrame->GetWidgetName() );
    this->Script("grid rowconfigure    %s 0 -weight 1 ", this->specRangeFrame->GetWidgetName() );
    this->Script("grid rowconfigure    %s 1 -weight 1 ", this->specRangeFrame->GetWidgetName() );

    this->Script("grid %s -in %s -row 0 -column 0 -sticky wnse", this->specRangeFrame->GetWidgetName(), this->GetWidgetName()); 


    this->Script("grid %s -in %s -row 0 -column 1 -sticky wnse -pady 1", 
                this->unitSelectBox->GetWidgetName(), this->specViewFrame->GetWidgetName()); 

    this->Script("grid %s -in %s -row 2 -column 1 -sticky wnse -pady 1", 
                this->componentSelectBox->GetWidgetName(), this->specViewFrame->GetWidgetName()); 

    this->Script("grid rowconfigure %s 0 -weight 1 ", this->specViewFrame->GetWidgetName() );
    this->Script("grid rowconfigure %s 1 -weight 1 ", this->specViewFrame->GetWidgetName() );
    this->Script("grid rowconfigure %s 2 -weight 1 ", this->specViewFrame->GetWidgetName() );
    this->Script("grid columnconfigure %s 0 -weight 1 ", this->specViewFrame->GetWidgetName() );
    this->Script("grid columnconfigure %s 1 -weight 1 -minsize 73", this->specViewFrame->GetWidgetName() );

    this->Script("grid %s -in %s -row 0 -column 1 -sticky wnse -rowspan 2 ", this->specViewFrame->GetWidgetName(), this->GetWidgetName()); 

    this->Script("grid rowconfigure %s 0 -weight 1 -minsize 30 ", this->GetWidgetName() );
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
        this->unitSelectBox, vtkKWMenu::MenuItemInvokedEvent);

    this->AddCallbackCommandObserver(
        this->componentSelectBox, vtkKWMenu::MenuItemInvokedEvent);

    // We can delete our references to all widgets that we do not have callbacks for.
    separator->Delete();
    separatorVert->Delete();
    
}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicSpectraRangeWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{

    // Respond to a change in the x range (frequency)
    if( caller == this->xSpecRange ) {
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
        float lowestPoint = minValue;
        float highestPoint = maxValue;
        lowestPoint = this->ConvertPosUnits(
            minValue,
            this->specUnits,
            svkSpecPoint::PTS
        );
    
        highestPoint = this->ConvertPosUnits(
            maxValue,
            this->specUnits,
            svkSpecPoint::PTS
        );

        this->plotController->SetWindowLevelRange( lowestPoint, highestPoint, svkPlotGridView::FREQUENCY);


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
    } 


    // Make sure the superclass gets called for render requests
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);

}


/*!
 *
 * @param position
 * @param inType
 * @param targetType
 */
float sivicSpectraRangeWidget::ConvertPosUnits(float position, int inType, int targetType)
{

    svk4DImageData* data = this->sivicController->GetActive4DImageData();
    if( data != NULL && data->IsA("svkMrsImageData") ){
        this->point->SetDcmHeader( data->GetDcmHeader() );
        float value = this->point->ConvertPosUnits( position, inType, targetType );
        return value;
    } else {
        return position;
    }

}
