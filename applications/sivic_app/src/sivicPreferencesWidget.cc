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


#include <sivicPreferencesWidget.h>
#include <vtkSivicController.h>

vtkStandardNewMacro( sivicPreferencesWidget );
vtkCxxRevisionMacro( sivicPreferencesWidget, "$Revision$");


/*! 
 *  Constructor
 */
sivicPreferencesWidget::sivicPreferencesWidget()
{
    this->settings.push_back( "defaults/sync_components/Set to 'active' to update only active trace's component, otherwise update all traces. (active or all)");
    this->settings.push_back( "defaults/printer/Name of the printer to use when printing.");
    this->settings.push_back( "defaults/spectra_extension_filtering/Set to ON if the file browser should filter spectra files by extension. (ON | OFF)");
    this->settings.push_back( "plot_grid/red/Sets the red component for the plot overlayed on the image. (0-1)");
    this->settings.push_back( "plot_grid/blue/Sets the blue component for the plot overlayed on the image. (0-1)");
    this->settings.push_back( "plot_grid/green/Sets the green component for the plot overlayed on the image. (0-1)");
    this->settings.push_back( "plot_grid/opacity/Sets the opacity for the grid overlayed on the image. (0-1)");
    this->settings.push_back( "plot_grid/width/Sets the width of the lines in the grid overlayed on the image. (integer greater than 0)");
    this->settings.push_back( "image_background/red/Sets the red component for the background of the image. (0-1)");
    this->settings.push_back( "image_background/blue/Sets the blue component for the background of the image. (0-1)");
    this->settings.push_back( "image_background/green/Sets the green component for the background of the image. (0-1)");
    this->settings.push_back( "trace_background/red/Sets the red component for the background of the traces. (0-1)");
    this->settings.push_back( "trace_lines/width/Sets the width of the trace lines. (integer greater than zero)");
    this->settings.push_back( "trace_plot_grid/red/Sets the red component for the trace grid. (0-1)");
    this->settings.push_back( "trace_plot_grid/blue/Sets the blue component for the trace grid. (0-1)");
    this->settings.push_back( "trace_plot_grid/green/Sets the green component for the trace grid. (0-1)");
    this->settings.push_back( "trace_background/blue/Sets the blue component for the background of the traces. (0-1)");
    this->settings.push_back( "trace_background/green/Sets the green component for the background of the traces. (0-1)");
    this->settings.push_back( "sat_bands/red/Sets the red component for the saturation bands. (0-1)");
    this->settings.push_back( "sat_bands/blue/Sets the blue component for the saturation bands. (0-1)");
    this->settings.push_back( "sat_bands/green/Sets the green component for the saturation bands. (0-1)");
    this->settings.push_back( "sat_bands/opacity/Sets the opacity for the saturation bands (0-1)");
    this->settings.push_back( "sat_bands_outline/red/Sets the red component for the outlines of the saturation bands. (0-1)");
    this->settings.push_back( "sat_bands_outline/blue/Sets the blue component for the outlines of the saturation bands. (0-1)");
    this->settings.push_back( "sat_bands_outline/green/Sets the green component for the outlines of the saturation bands. (0-1)");
    this->settings.push_back( "sat_bands_outline/opacity/Sets the opacity for the outlines of the saturation bands. (0-1)");
    this->settings.push_back( "vol_selection/red/Sets the red component for the selection box. (0-1)");
    this->settings.push_back( "vol_selection/blue/Sets the blue component for the selection box. (0-1)");
    this->settings.push_back( "vol_selection/green/Sets the green component for the selection box. (0-1)");
    this->settings.push_back( "vol_selection/opacity/Sets the opacity for the selection box edges. (0-1)");
    this->settings.push_back( "vol_selection/width/Sets the width for the selection box edges. (integer greater than zero)");
    this->settings.push_back( "apodization/fwhh/Sets the fullwidth half height in Hz for the apodization windows.");
    this->settings.push_back( "apodization/center/Sets the center for the Gaussion apodization window.");
    this->settings.push_back( "data_writing/double_to_float/Conversion method from double to float for writing. (MAP | CAST)");

}


/*! 
 *  Destructor
 */
sivicPreferencesWidget::~sivicPreferencesWidget()
{

	if( this->settingsTable != NULL ) {
		this->settingsTable->Delete();
		this->settingsTable = NULL;
	}
}


/*! 
 *  Get Preference.
 */
bool sivicPreferencesWidget::GetReadOnlyOneInputFile()
{
    return false; 
}

/*! 
 *  Method in superclass to be overriden to add our custom widgets.
 */
void sivicPreferencesWidget::CreateWidget()
{

    // Check if already created
    if ( this->IsCreated() )
    {
        vtkErrorMacro(<< this->GetClassName() << " already created");
        return;
    }

    // Call the superclass to create the composite widget container
    this->Superclass::CreateWidget();

    this->settingsTable = vtkKWMultiColumnListWithScrollbars::New();
    this->settingsTable->SetParent(this);
    this->settingsTable->Create();
    int col_index;
    col_index = this->settingsTable->GetWidget()->AddColumn("Group");
    col_index = this->settingsTable->GetWidget()->AddColumn("Name");
    col_index = this->settingsTable->GetWidget()->AddColumn("Value");
    this->settingsTable->GetWidget()->ColumnEditableOn(col_index);
    col_index = this->settingsTable->GetWidget()->AddColumn("Description");
    this->settingsTable->GetWidget()->StretchableColumnsOn();

    this->Script("pack %s -expand y -fill both", this->settingsTable->GetWidgetName(),   0);

    //  Callbacks
    this->AddCallbackCommandObserver( this->settingsTable->GetWidget(), vtkKWMultiColumnList::CellUpdatedEvent );

}
/*!
 *
 *  Updates the list of settings.
 */
void sivicPreferencesWidget::UpdateSettingsList( )
{

    // We start at one since the first plot index is the active spectra above...
    for( int i = 0; i < this->settings.size(); i ++ ) {
    	vector<string> setting = svkUtils::SplitString( this->settings[i], "/");

        char registryValue[100] = "";
        string registryValueString = "";

        // Lets grab the printer name from the registry
        this->GetApplication()->GetRegistryValue( 0, setting[0].c_str(), setting[1].c_str(), registryValue );
        if( registryValue != NULL && strcmp( registryValue, "" ) != 0 ) {
    		registryValueString = registryValue;
        }

		this->settingsTable->GetWidget()->InsertCellText(i, 0, setting[0].c_str());
		this->settingsTable->GetWidget()->InsertCellText(i, 1, setting[1].c_str());
		this->settingsTable->GetWidget()->InsertCellText(i, 2, registryValueString.c_str());
		this->settingsTable->GetWidget()->InsertCellText(i, 3, setting[2].c_str());

    }

}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicPreferencesWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    for( int i = 0; i < this->settingsTable->GetWidget()->GetNumberOfRows(); i++) {

        string subkey = this->settingsTable->GetWidget()->GetCellText(i,0);
        string key = this->settingsTable->GetWidget()->GetCellText(i,1);
        string value = this->settingsTable->GetWidget()->GetCellText(i,2);

		this->GetApplication()->SetRegistryValue( 0, subkey.c_str(), key.c_str(), value.c_str());

    }

    // Make sure the superclass gets called for render requests
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
    this->sivicController->SetPreferencesFromRegistry();
}


/*!
 *  For a given subkey extract red green and blue components.
 */
void sivicPreferencesWidget::GetColorFromRegistry(vtkKWApplication* app, string subkey, double rgb[3])
{
    char red[50]="";
    app->GetRegistryValue( 0, subkey.c_str(), "red", red );
    char green[50]="";
    app->GetRegistryValue( 0, subkey.c_str(), "green", green );
    char blue[50]="";
    app->GetRegistryValue( 0, subkey.c_str(), "blue", blue );


    // only return the color if all three are present
	if( string(red) != "" && string(green) != "" && string(blue) != "" ) {
		rgb[0] = atof( red );
		rgb[1] = atof( green );
		rgb[2] = atof( blue );
	}
}
