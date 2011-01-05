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



#include <sivicAppGUI.h>


using namespace svk; 


vtkStandardNewMacro( sivicAppGUI );
vtkCxxRevisionMacro( sivicAppGUI, "$Revision$");


/*! 
 *  Constructor
 */
sivicAppGUI::sivicAppGUI()
{
    this->xSpecRange    = NULL;
    this->ySpecRange    = NULL;
    this->point         = NULL;
    this->mrsImageData  = NULL; 
}



/*! 
 *  Destructor
 */
sivicAppGUI::~sivicAppGUI()
{
    if( this->xSpecRange != NULL ) 
    {
        this->xSpecRange->Delete();
        this->xSpecRange = NULL;
    }

    if( this->ySpecRange != NULL) 
    {
        this->ySpecRange->Delete();
        this->ySpecRange = NULL;
    }

    if( this->point != NULL) 
    {
        this->point->Delete();
        this->point = NULL;
    }

    if( this->mrsImageData != NULL) 
    {
        this->mrsImageData->Delete();
        this->mrsImageData = NULL;
    }
}


/*!
 *  Create a default MRS Widgets for SIVIC MRS GUI. 
 *  range controller in a sivic MRS display.  
 */
void sivicAppGUI::CreateMRSWidgets( vtkKWWidget* parent )
{
    this->CreateXSpecRangeWidget( parent ); 
    this->CreateYSpecRangeWidget( parent ); 
    
    this->EnableMRSWidgetCallbacks( parent );
}


/*!
 *  Create a default vtkKWRange Widget for the frequency 
 *  range controller in a sivic MRS display.  
 */
void sivicAppGUI::CreateXSpecRangeWidget( vtkKWWidget* parent )
{
    // Create the x range widget 
    this->xSpecRange = vtkKWRange::New(); 
    this->xSpecRange->SetParent( parent );
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

    //  Set default resolution for PPM:
    this->xSpecRange->SetResolution( .001 );
}


/*!
 *  Create a default vtkKWRange Widget for the intensity 
 *  range controller in a sivic MRS display.  
 */
void sivicAppGUI::CreateYSpecRangeWidget( vtkKWWidget* parent )
{
    // Create the y range widget 
    this->ySpecRange = vtkKWRange::New(); 
    this->ySpecRange->SetParent( parent );
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
}


/*!
 *  Set callback events for newly created widgets.  
 */
void sivicAppGUI::EnableMRSWidgetCallbacks( vtkKWWidget* parent )
{
    parent->AddCallbackCommandObserver( this->xSpecRange, vtkKWRange::RangeValueChangingEvent );
    parent->AddCallbackCommandObserver( this->ySpecRange, vtkKWRange::RangeValueChangingEvent );
}


/*!
 *  Initializes a vtkKWRange Widget for the frequency 
 *  range controller in a sivic MRS display, using the loaded data to 
 *  set ranges.  
 */
void sivicAppGUI::InitXSpecRangeWidget()
{

    float lowestPoint = this->point->ConvertPosUnits(
                                0,
                                svkSpecPoint::PTS,
                                svkSpecPoint::PPM
                            );
    float highestPoint = this->point->ConvertPosUnits(
                                this->mrsImageData->GetCellData()->GetArray(0)->GetNumberOfTuples(),
                                svkSpecPoint::PTS,
                                svkSpecPoint::PPM
                            );

    this->xSpecRange->SetWholeRange( lowestPoint, highestPoint );
    this->xSpecRange->SetRange( PPM_DEFAULT_MIN, PPM_DEFAULT_MAX );
}


/*!
 *  Initializes a vtkKWRange Widget for the intensity 
 *  range controller in a sivic MRS display, using the loaded data to 
 *  set ranges.  
 */
void sivicAppGUI::InitYSpecRangeWidget( svkImageData::RangeComponent component)
{
    double range[2];
    if( component == svkImageData::REAL ) 
    {
        this->mrsImageData->GetDataRange( range, svkImageData::REAL  );
    } 
    else if( component == svkImageData::IMAGINARY ) 
    {
        this->mrsImageData->GetDataRange( range, svkImageData::IMAGINARY  );
    } 
    else if( component == svkImageData::MAGNITUDE ) 
    {
        this->mrsImageData->GetDataRange( range, svkImageData::MAGNITUDE  );
    }
    this->ySpecRange->SetWholeRange( range[0], range[1] );
    this->ySpecRange->SetRange( range[0] * NEG_RANGE_SCALE, range[1] * POS_RANGE_SCALE );
    this->ySpecRange->SetResolution( (range[1] - range[0]) * SLIDER_RELATIVE_RESOLUTION );
}


/*!
 *  Returns the new lowest and min and max intensity values in the modified range. 
 *  This is the response to a RangeValueChangingEvent. 
 */
void sivicAppGUI::YSpecRangeChanged(double& minValue, double& maxValue)
{
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
}


/*!
 *  Returns the new lowest and highest point values in the modified range. 
 *  This is the response to a RangeValueChangingEvent. 
 */
void sivicAppGUI::XSpecRangeChanged(float& lowestPoint, float& highestPoint)
{
    double minValue;
    double maxValue;
    this->xSpecRange->GetRange( minValue, maxValue ); 

    stringstream widenRange;
    widenRange << "SetRange " << minValue - this->xSpecRange->GetResolution() 
                                  << " " << maxValue + this->xSpecRange->GetResolution();
    stringstream narrowRange;
    narrowRange << "SetRange " << minValue + this->xSpecRange->GetResolution() 
                                   << " " << maxValue - this->xSpecRange->GetResolution();
    stringstream incrementRange;
    incrementRange << "SetRange " << minValue + this->xSpecRange->GetResolution() 
                                      << " " << maxValue + this->xSpecRange->GetResolution();
    stringstream decrementRange;
    decrementRange << "SetRange " << minValue - this->xSpecRange->GetResolution()
                                      << " " << maxValue - this->xSpecRange->GetResolution();

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

    lowestPoint = this->point->ConvertPosUnits(
        this->xSpecRange->GetEntry1()->GetValueAsDouble(),
        this->specUnits,
        svkSpecPoint::PTS 
    );
    
    highestPoint = this->point->ConvertPosUnits(
        this->xSpecRange->GetEntry2()->GetValueAsDouble(),
        this->specUnits,
        svkSpecPoint::PTS 
    );
}


/*!
 *  Set representative MRS data set for the purpose of setting data ranges in interactive
 *  widgets (range sliders) and also for converting between units (PPM, Hz, Pts). 
 */
void sivicAppGUI::SetMRSData( svkImageData* mrsData )
{
    if (! mrsData->IsA("svkMrsImageData") ) 
    {
        vtkErrorWithObjectMacro(this, "Must initialize svkMRSData with svkMrsImageData" ); 
    }
    else 
    {
        this->mrsImageData = mrsData; 
        this->point = svkSpecPoint::New();
        this->point->SetDcmHeader( mrsData->GetDcmHeader() ); 
        this->EnableMRSWidgets();
    }
}


/*!
 *  After loading data, the widgets should be enabled. 
 */
void sivicAppGUI::EnableMRSWidgets()
{
    this->xSpecRange->EnabledOn();
    this->ySpecRange->EnabledOn();
}


/*!
 *  After loading data, the widgets should be enabled. 
 */
void sivicAppGUI::DisableMRSWidgets()
{
    this->xSpecRange->EnabledOff();
    this->ySpecRange->EnabledOff();
}


/*!
 *  Gets the svkSpecPoint. 
 */
svkSpecPoint* sivicAppGUI::GetSpecPoint()
{
    return this->point; 
}
