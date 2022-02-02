/*
 *  Copyright © 2009-2017 The Regents of the University of California.
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


#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#include <unistd.h>
#endif

#include <sivicApp.h>

#include <vtkJPEGReader.h>

/*! 
 *  Constructor
 */
sivicApp::sivicApp()
{
    this->model = NULL;

    // Creating our widgetView/Controller
    this->sivicController = NULL;
    this->sivicKWApp = NULL; 

    // For returnting the application status
    this->exitStatus            = 0;
    this->processingWidget      = sivicProcessingWidget::New();
    this->phaseWidget           = sivicPhaseWidget::New();
    this->preprocessingWidget   = sivicPreprocessingWidget::New();
    this->postprocessingWidget  = sivicPostprocessingWidget::New();
    this->dataWidget            = sivicDataWidget::New();
    this->imageDataWidget       = sivicImageDataWidget::New();
    this->quantificationWidget  = sivicQuantificationWidget::New();
    this->combineWidget         = sivicCombineWidget::New();
    this->dscWidget             = sivicDSCWidget::New();
    this->imageViewWidget       = sivicImageViewWidget::New();
    this->spectraViewWidget     = sivicSpectraViewWidget::New();
    this->windowLevelWidget     = sivicWindowLevelWidget::New();
    this->windowLevelWidget->SetSliderLabel("Image Window Level");
    this->overlayWindowLevelWidget = sivicWindowLevelWidget::New();
    this->overlayWindowLevelWidget->SetSliderLabel("Overlay Window Level");
    this->preferencesWidget     = sivicPreferencesWidget::New();
    this->voxelTaggingWidget    = sivicVoxelTaggingWidget::New();
    this->spectraRangeWidget    = sivicSpectraRangeWidget::New();
    this->viewRenderingWidget   = sivicViewRenderingWidget::New();
    this->tabbedPanel           = vtkKWNotebook::New();


}


/*! 
 *  Destructor
 */
sivicApp::~sivicApp()
{
    // Deallocate and exit
    if( this->sivicKWApp != NULL ) {
        this->exitStatus = this->sivicKWApp->GetExitStatus();
    }

    if( this->sivicWindow != NULL ) {
        this->sivicWindow->Close();
    }

    if( this->sivicController != NULL ) {
        this->sivicController->Delete();
        this->sivicController = NULL;
    }


    if( this->viewRenderingWidget != NULL ) {
        this->viewRenderingWidget->Delete();
        this->viewRenderingWidget = NULL;
    }

    if( this->processingWidget != NULL ) {
        this->processingWidget->Delete();
        this->processingWidget = NULL;
    }

    if( this->phaseWidget != NULL ) {
        this->phaseWidget->Delete();
        this->phaseWidget = NULL;
    }

    if( this->preprocessingWidget != NULL ) {
        this->preprocessingWidget->Delete();
        this->preprocessingWidget = NULL;
    }

    if( this->postprocessingWidget != NULL ) {
        this->postprocessingWidget->Delete();
        this->postprocessingWidget = NULL;
    }

    if( this->dataWidget != NULL ) {
        this->dataWidget->Delete();
        this->dataWidget = NULL;
    }

    if( this->imageDataWidget != NULL ) {
        this->imageDataWidget->Delete();
        this->imageDataWidget = NULL;
    }

    if( this->quantificationWidget != NULL ) {
        this->quantificationWidget->Delete();
        this->quantificationWidget = NULL;
    }

    if( this->combineWidget != NULL ) {
        this->combineWidget->Delete();
        this->combineWidget = NULL;
    }

    if( this->dscWidget != NULL ) {
        this->dscWidget->Delete();
        this->dscWidget = NULL;
    }

    if( this->imageViewWidget != NULL ) {
        this->imageViewWidget->Delete();
        this->imageViewWidget = NULL;
    }


    if( this->spectraViewWidget != NULL ) {
        this->spectraViewWidget->Delete();
        this->spectraViewWidget = NULL;
    }

    if( this->windowLevelWidget != NULL ) {
        this->windowLevelWidget->Delete();
        this->windowLevelWidget = NULL;
    }

    if( this->preferencesWidget != NULL ) {
        this->preferencesWidget->Delete();
        this->preferencesWidget = NULL;
    }

    if( this->voxelTaggingWidget != NULL ) {
        this->voxelTaggingWidget->Delete();
        this->voxelTaggingWidget = NULL;
    }

    if( this->overlayWindowLevelWidget != NULL ) {
        this->overlayWindowLevelWidget->Delete();
        this->overlayWindowLevelWidget = NULL;
    }

    if( this->spectraRangeWidget != NULL ) {
        this->spectraRangeWidget->Delete();
        this->spectraRangeWidget = NULL;
    }

    if( this->model != NULL ) {
        this->model->Delete(); 
        this->model = NULL; 
    }

    if( this->sivicWindow != NULL ) {
        this->sivicWindow->Delete();
        this->sivicWindow = NULL;
    }

    if( this->sivicKWApp != NULL ) {
        this->sivicKWApp->Delete();
        this->sivicKWApp = NULL;
    }
/*
    if( this->uiPanel != NULL ) {
        this->uiPanel->Delete();
        this->uiPanel = NULL;
    }
*/

    if( this->tabbedPanel != NULL ) {
        this->tabbedPanel->Delete();
        this->tabbedPanel = NULL;
    }

}

/*!
 * Returns the View. The intent is to use this for testing purposes.
 */
vtkSivicController* sivicApp::GetView()
{
    return this->sivicController;
}


/*! 
 *  Build the sivicKWApp.
 */
int sivicApp::Build( int argc, char* argv[] )
{
    // Kludge to avoid Tcl/TK initialization error when requesting help message.
    if( argc == 2) {
        string firstArgument = argv[1];
        if( firstArgument.compare("-h") == 0){
            return 1;
        }
    }
    // First we have to initialize the tcl interpreter...
    Tcl_Interp *interp = vtkKWApplication::InitializeTcl(argc, argv, &cerr);
    if (!interp)
        {
        cerr << "Error: InitializeTcl failed" << endl ;
        return 1;
        }
        
    Sivickwcallbackslib_Init(interp);

    if( this->model != NULL ) {
        this->model->Delete();
        this->model = NULL;
    }
    this->model = svkDataModel::New();

    // Creating our widgetView/Controller
    if( this->sivicController != NULL ) {
        this->sivicController->Delete();
        this->sivicController = NULL;
    }
    this->sivicController = vtkSivicController::New();
    this->sivicController->registryLevel = 0;


    if( this->sivicKWApp != NULL ) {
        this->sivicKWApp->Delete();
        this->sivicKWApp = NULL;
    }
    this->sivicKWApp = vtkKWApplication::New();

    this->sivicKWApp->ReleaseModeOn();
    this->sivicController->SetApplication( this->sivicKWApp );
    this->sivicController->SetModel( this->model );

    // Create Application
    this->sivicKWApp->SetName("SIVIC");

    this->sivicKWApp->SetMajorVersion( SVK_MAJOR_VERSION );
    this->sivicKWApp->SetMinorVersion( SVK_MINOR_VERSION );
    this->sivicKWApp->SetVersionName( SVK_RELEASE_VERSION );

    // Add a Window to the application
    this->sivicWindow = vtkKWWindowBase::New();
    this->sivicKWApp->AddWindow(this->sivicWindow);
    this->sivicController->SetMainWindow( this->sivicWindow );
    this->sivicWindow->Create();
    this->sivicWindow->SetSize( WINDOW_SIZE_X, WINDOW_SIZE_Y);

    this->tabbedPanel->SetParent( this->sivicWindow->GetViewFrame() );
    this->tabbedPanel->SetApplication( this->sivicKWApp );
    this->tabbedPanel->Create( );


    this->viewRenderingWidget->SetParent(this->sivicWindow->GetViewFrame());
    this->viewRenderingWidget->SetPlotController(this->sivicController->GetPlotController());
    this->viewRenderingWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->viewRenderingWidget->SetApplication( this->sivicKWApp );
    this->viewRenderingWidget->Create();

    vtkKWText* welcomeText = vtkKWText::New();
    welcomeText->SetParent( tabbedPanel );
    welcomeText->Create();
    this->GetWelcomeMessage( welcomeText );
    welcomeText->ReadOnlyOn(); 
    welcomeText->SetFont("system 8"); 

    this->preprocessingWidget->SetParent(tabbedPanel );
    this->preprocessingWidget->SetPlotController(this->sivicController->GetPlotController());
    this->preprocessingWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->preprocessingWidget->SetSivicController(this->sivicController);
    this->preprocessingWidget->Create();

    this->postprocessingWidget->SetParent(tabbedPanel );
    this->postprocessingWidget->SetPlotController(this->sivicController->GetPlotController());
    this->postprocessingWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->postprocessingWidget->SetSivicController(this->sivicController);
    this->postprocessingWidget->Create();

    this->processingWidget->SetParent(tabbedPanel );
    this->processingWidget->SetPlotController(this->sivicController->GetPlotController());
    this->processingWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->processingWidget->SetSivicController(this->sivicController);
    this->processingWidget->Create();

    this->phaseWidget->SetParent(tabbedPanel );
    this->phaseWidget->SetPlotController(this->sivicController->GetPlotController());
    this->phaseWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->phaseWidget->SetSivicController(this->sivicController);
    this->phaseWidget->Create();

    this->combineWidget->SetParent(tabbedPanel );
    this->combineWidget->SetPlotController(this->sivicController->GetPlotController());
    this->combineWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->combineWidget->SetSivicController(this->sivicController);
    this->combineWidget->Create();

    this->quantificationWidget->SetParent(tabbedPanel );
    this->quantificationWidget->SetPlotController(this->sivicController->GetPlotController());
    this->quantificationWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->quantificationWidget->SetSivicController(this->sivicController);
    this->quantificationWidget->Create();

    this->dataWidget->SetParent(tabbedPanel );
    this->dataWidget->SetPlotController(this->sivicController->GetPlotController());
    this->dataWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->dataWidget->SetSivicController(this->sivicController);
    this->dataWidget->Create();

    this->imageDataWidget->SetParent(tabbedPanel );
    this->imageDataWidget->SetPlotController(this->sivicController->GetPlotController());
    this->imageDataWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->imageDataWidget->SetSivicController(this->sivicController);
    this->imageDataWidget->Create();

    this->dscWidget->SetParent(tabbedPanel );
    this->dscWidget->SetPlotController(this->sivicController->GetPlotController());
    this->dscWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->dscWidget->SetSivicController(this->sivicController);
    this->dscWidget->Create();

    this->imageViewWidget->SetParent(this->sivicWindow->GetViewFrame());
    this->imageViewWidget->SetPlotController(this->sivicController->GetPlotController());
    this->imageViewWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->imageViewWidget->SetSivicController(this->sivicController);
    this->imageViewWidget->Create();

    this->spectraViewWidget->SetParent(this->sivicWindow->GetViewFrame());
    this->spectraViewWidget->SetPlotController(this->sivicController->GetPlotController());
    this->spectraViewWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->spectraViewWidget->SetSivicController(this->sivicController);
    this->spectraViewWidget->Create();

    this->spectraRangeWidget->SetParent(this->sivicWindow->GetViewFrame());
    this->spectraRangeWidget->SetPlotController(this->sivicController->GetPlotController());
    this->spectraRangeWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->spectraRangeWidget->SetSivicController(this->sivicController);
    this->spectraRangeWidget->Create();

    this->windowLevelWidget->SetPlotController(this->sivicController->GetPlotController());
    this->windowLevelWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->windowLevelWidget->SetSivicController(this->sivicController);
    this->windowLevelWidget->SetWindowLevelTarget( svkOverlayViewController::REFERENCE_IMAGE );

    this->preferencesWidget->SetPlotController(this->sivicController->GetPlotController());
    this->preferencesWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->preferencesWidget->SetSivicController(this->sivicController);

    this->voxelTaggingWidget->SetPlotController(this->sivicController->GetPlotController());
    this->voxelTaggingWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->voxelTaggingWidget->SetSivicController(this->sivicController);

    this->overlayWindowLevelWidget->SetPlotController(this->sivicController->GetPlotController());
    this->overlayWindowLevelWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->overlayWindowLevelWidget->SetSivicController(this->sivicController);
    this->overlayWindowLevelWidget->SetWindowLevelTarget( svkOverlayViewController::IMAGE_OVERLAY );


    this->sivicController->SetViewRenderingWidget( viewRenderingWidget );
    this->sivicController->SetProcessingWidget( processingWidget );
    this->sivicController->SetPhaseWidget( phaseWidget );
    this->sivicController->SetPreprocessingWidget( preprocessingWidget );
    this->sivicController->SetPostprocessingWidget( postprocessingWidget );
    this->sivicController->SetDataWidget( dataWidget );
    this->sivicController->SetImageDataWidget( imageDataWidget );
    this->sivicController->SetCombineWidget( combineWidget );
    this->sivicController->SetQuantificationWidget( quantificationWidget );
    this->sivicController->SetImageViewWidget( imageViewWidget );
    this->sivicController->SetSpectraViewWidget( spectraViewWidget );
    this->sivicController->SetWindowLevelWidget( windowLevelWidget );
    this->sivicController->SetOverlayWindowLevelWidget( overlayWindowLevelWidget );
    this->sivicController->SetPreferencesWidget( preferencesWidget );
    this->sivicController->SetVoxelTaggingWidget( voxelTaggingWidget );
    this->sivicController->SetSpectraRangeWidget( spectraRangeWidget );
    this->sivicController->SetDSCWidget( dscWidget );

    //this->tabbedPanel->AddPage("Home", "Home", NULL);
    //vtkKWWidget* welcomePanel = tabbedPanel->GetFrame("Home");

    this->tabbedPanel->AddPage("Image Data", "Image Data.", NULL);
    vtkKWWidget* imageDataPanel = tabbedPanel->GetFrame("Image Data");

    this->tabbedPanel->AddPage("4D Data", "4D Data.", NULL);
    vtkKWWidget* dataPanel = tabbedPanel->GetFrame("4D Data");

    this->tabbedPanel->AddPage("Preproc", "Preprocessing.", NULL);
    vtkKWWidget* preprocessingPanel = tabbedPanel->GetFrame("Preproc");

    this->tabbedPanel->AddPage("Recon", "MRS recon.", NULL);
    vtkKWWidget* processingPanel = tabbedPanel->GetFrame("Recon");

    this->tabbedPanel->AddPage("Phase", "Phasing", NULL);
    vtkKWWidget* phasePanel = tabbedPanel->GetFrame("Phase");

    this->tabbedPanel->AddPage("Comb", "Coil Combination", NULL);
    vtkKWWidget* combinePanel = tabbedPanel->GetFrame("Comb");

    this->tabbedPanel->AddPage("PostProc", "post process.", NULL);
    vtkKWWidget* postprocessingPanel = tabbedPanel->GetFrame("PostProc");

    this->tabbedPanel->AddPage("Quant", "MRS Quant.", NULL);
    vtkKWWidget* quantificationPanel = tabbedPanel->GetFrame("Quant");

    this->tabbedPanel->AddPage("DSC", "DSC", NULL);
    vtkKWWidget* dscPanel = tabbedPanel->GetFrame("DSC");

    vtkKWSeparator* separator = vtkKWSeparator::New();
    separator->SetParent(this->sivicWindow->GetViewFrame());
    separator->Create();
    separator->SetThickness(5);

    vtkKWSeparator* separator2 = vtkKWSeparator::New();
    separator2->SetParent(this->sivicWindow->GetViewFrame());
    separator2->Create();
    separator2->SetThickness(5);

    vtkKWSeparator* separatorVert = vtkKWSeparator::New();
    separatorVert->SetParent(this->sivicWindow->GetViewFrame());
    separatorVert->Create();
    separatorVert->SetThickness(3);

    vtkKWSeparator* separatorVert2 = vtkKWSeparator::New();
    separatorVert2->SetParent(this->sivicWindow->GetViewFrame());
    separatorVert2->Create();
    separatorVert2->SetThickness(3);

    this->sivicKWApp->Script("grid %s -row 0 -column 0 -columnspan 5 -sticky nsew", viewRenderingWidget->GetWidgetName());
    this->sivicKWApp->Script("grid %s -row 1 -column 0 -columnspan 5 -sticky nsew", separator->GetWidgetName());
    this->sivicKWApp->Script("grid %s -row 2 -column 0 -rowspan 3 -sticky wnse", imageViewWidget->GetWidgetName());
    this->sivicKWApp->Script("grid %s -row 2 -column 1 -sticky nsew -rowspan 3", separatorVert->GetWidgetName());

    this->sivicKWApp->Script("grid %s -row 2 -column 2 -sticky ensw -padx 2", tabbedPanel->GetWidgetName());
    this->sivicKWApp->Script("grid %s -row 3 -column 2 -sticky nsew -padx 2 -pady 2", spectraRangeWidget->GetWidgetName());
    this->sivicKWApp->Script("grid %s -row 4 -column 2 -sticky nsew -padx 2", spectraViewWidget->GetWidgetName());

    this->sivicKWApp->Script("grid %s -row 2 -column 3 -sticky nsew -rowspan 3", separatorVert2->GetWidgetName());

    //this->sivicKWApp->Script("pack %s -side top -anchor nw -expand y -fill both -padx 2 -pady 2 -in %s",
    //          welcomeText->GetWidgetName(), welcomePanel->GetWidgetName());
    this->sivicKWApp->Script("pack %s -side top -anchor nw -expand y -fill both -padx 4 -pady 2 -in %s", 
              this->preprocessingWidget->GetWidgetName(), preprocessingPanel->GetWidgetName());
    this->sivicKWApp->Script("pack %s -side top -anchor nw -expand y -fill both -padx 4 -pady 2 -in %s", 
              this->processingWidget->GetWidgetName(), processingPanel->GetWidgetName());
    this->sivicKWApp->Script("pack %s -side top -anchor nw -expand y -fill both -padx 4 -pady 2 -in %s", 
              this->phaseWidget->GetWidgetName(), phasePanel->GetWidgetName());
    this->sivicKWApp->Script("pack %s -side top -anchor nw -expand y -fill both -padx 4 -pady 2 -in %s", 
              this->combineWidget->GetWidgetName(), combinePanel->GetWidgetName());
    this->sivicKWApp->Script("pack %s -side top -anchor nw -expand y -fill both -padx 4 -pady 2 -in %s", 
              this->postprocessingWidget->GetWidgetName(), postprocessingPanel->GetWidgetName());
    this->sivicKWApp->Script("pack %s -side top -anchor nw -expand y -fill both -padx 4 -pady 2 -in %s", 
              this->quantificationWidget->GetWidgetName(), quantificationPanel->GetWidgetName());
    this->sivicKWApp->Script("pack %s -side top -anchor nw -expand y -fill both -padx 4 -pady 2 -in %s",
              this->imageDataWidget->GetWidgetName(), imageDataPanel->GetWidgetName());
    this->sivicKWApp->Script("pack %s -side top -anchor nw -expand y -fill both -padx 4 -pady 2 -in %s",
              this->dataWidget->GetWidgetName(), dataPanel->GetWidgetName());
    this->sivicKWApp->Script("pack %s -side top -anchor nw -expand y -fill both -padx 4 -pady 2 -in %s", 
              this->dscWidget->GetWidgetName(), dscPanel->GetWidgetName());

    //  row 0 -> render view
    //  row 1 -> seperator
    //  row 2 -> tab processing    
    //  row 3 -> spec range    
    //  row 4 -> spec view
    this->sivicKWApp->Script("grid rowconfigure    %s 0 -weight 100  -minsize 300 ", this->sivicWindow->GetViewFrame()->GetWidgetName() );
    this->sivicKWApp->Script("grid rowconfigure    %s 1 -weight 0    -minsize 3   ", this->sivicWindow->GetViewFrame()->GetWidgetName() );
    this->sivicKWApp->Script("grid rowconfigure    %s 2 -weight 0    -minsize 137 -maxsize 137", this->sivicWindow->GetViewFrame()->GetWidgetName() );
    this->sivicKWApp->Script("grid rowconfigure    %s 3 -weight 0    -minsize 20  -maxsize 20", this->sivicWindow->GetViewFrame()->GetWidgetName() );
    this->sivicKWApp->Script("grid rowconfigure    %s 4 -weight 0    -minsize 20  -maxsize 20", this->sivicWindow->GetViewFrame()->GetWidgetName() );

    this->sivicKWApp->Script("grid columnconfigure %s 0 -weight 50 -uniform 1 -minsize 350 ", this->sivicWindow->GetViewFrame()->GetWidgetName() );
    this->sivicKWApp->Script("grid columnconfigure %s 1 -weight 0 -minsize 5", this->sivicWindow->GetViewFrame()->GetWidgetName() );
    this->sivicKWApp->Script("grid columnconfigure %s 2 -weight 50 -uniform 1 -minsize 300", this->sivicWindow->GetViewFrame()->GetWidgetName() );
    separator->Delete();
    separatorVert->Delete();
    separatorVert2->Delete();
    welcomeText->Delete();


    // Create Toolbar
    vtkKWToolbar* toolbar = vtkKWToolbar::New();
    toolbar->SetName("Main Toolbar"); 
    toolbar->SetParent(this->sivicWindow->GetMainToolbarSet()->GetToolbarsFrame()); 
    toolbar->Create();
    PopulateMainToolbar( toolbar );

    // Add toolbar to window
    this->sivicWindow->GetMainToolbarSet()->AddToolbar( toolbar );
    this->sivicWindow->GetMainToolbarSet()->ShowToolbar( toolbar );

    toolbar->Delete();

    this->sivicWindow->Display();
#if defined( UCSF_INTERNAL )
    vtkKWMenu* ucsfMenu = vtkKWMenu::New();

    ucsfMenu->SetParent(this->sivicKWApp->GetNthWindow(0)->GetMenu());
    ucsfMenu->Create();
    this->sivicKWApp->GetNthWindow(0)->GetMenu()->AddCascade("UCSF", ucsfMenu);

    //  Add menu of UCSF metabolite maps:
    vtkKWMenu* ucsfMetaboliteMenu = vtkKWMenu::New();
    ucsfMetaboliteMenu->SetParent(ucsfMenu);
    ucsfMetaboliteMenu->Create();
    ucsfMenu->AddCascade("Load Metabolite Map", ucsfMetaboliteMenu);
    ucsfMenu->InsertCommand( 0, "&Send To PACS", this->sivicController, "PushToPACS" );

#endif

#if defined( BUILD_GE_CONSOLE )
    vtkKWMenu* ucsfMenu = vtkKWMenu::New();

    ucsfMenu->SetParent(this->sivicKWApp->GetNthWindow(0)->GetMenu());
    ucsfMenu->Create();
    this->sivicKWApp->GetNthWindow(0)->GetMenu()->AddCascade("GE Console", ucsfMenu);

    //  Add menu of UCSF metabolite maps:
    vtkKWMenu* ucsfMetaboliteMenu = vtkKWMenu::New();
    ucsfMetaboliteMenu->SetParent(ucsfMenu);
    ucsfMetaboliteMenu->Create();
    ucsfMenu->InsertCommand( 0, "&Save Secondary Capture Report", this->sivicController, "PushToPACS" );
#endif


    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            0, "&Save Active Spectroscopic Data", this->sivicController, "SaveData");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            1, "&Save Spectra Secondary Capture", this->sivicController, "SaveSecondaryCapture SPECTRA_CAPTURE 0");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            2, "&Save Image Secondary Capture", this->sivicController, "SaveSecondaryCapture IMAGE_CAPTURE 0");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            3, "&Save Secondary Capture Report", this->sivicController, "SaveSecondaryCapture SPECTRA_WITH_OVERVIEW_CAPTURE 0");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            4, "&Save One Slice Combined Capture", this->sivicController, "SaveSecondaryCapture COMBINED_CAPTURE 1");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            5, "&Save Metabolite Maps", this->sivicController, "SaveMetaboliteMaps");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            6, "&Save Voxel Tags", this->sivicController, "SaveImageFromModel VoxelTagData");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            7, "&Print Current Slice", this->sivicController, "Print COMBINED_CAPTURE 1");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            8, "&Print All Slices", this->sivicController, "Print COMBINED_CAPTURE 0");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            9, "&Print Images Only", this->sivicController, "Print IMAGE_CAPTURE 0");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            10, "&Save Session", this->sivicController, "SaveSession");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            11, "&Restore Session", this->sivicController, "RestoreSession");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            12, "&Close All", this->sivicController, "ResetApplication");
    this->sivicKWApp->GetNthWindow(0)->GetHelpMenu()->InsertCommand(
            13, "&Sivic Help Resources", this->sivicController, "DisplayInfo");

    // Tools menu
    this->sivicKWApp->GetNthWindow(0)->GetWindowMenu()->InsertCommand(
            0, "&Show Window Level", this->sivicController, "DisplayWindowLevelWindow");
    this->sivicKWApp->GetNthWindow(0)->GetWindowMenu()->InsertCommand(
            1, "&Preferences", this->sivicController, "DisplayPreferencesWindow");
    this->sivicKWApp->GetNthWindow(0)->GetWindowMenu()->InsertCommand(
            2, "&Voxel Tagging", this->sivicController, "DisplayVoxelTaggingWindow");
#if defined(DEBUG_BUILD)
    this->sivicKWApp->GetNthWindow(0)->GetHelpMenu()->InsertCommand(
            3, "&Run Tests", this->sivicController, "RunTestingSuite");
#endif
    this->sivicKWApp->SetHelpDialogStartingPage("http://sivic.sourceforge.net");
    this->sivicWindow->Display();

	string pathName = svkUtils::GetCurrentWorkingDirectory();
    this->sivicKWApp->SetRegistryValue( this->sivicController->registryLevel, "RunTime", "lastPath", pathName.c_str() );


    //this->sivicKWApp->SetName("SIVIC_prostate");
    //cout << "REG NAME : " << this->sivicKWApp->GetRegistryVersionName() << endl;
    //cout << "REG LEVEL: " << this->sivicKWApp->GetRegistryLevel() << endl;

    // create default xml config file
    bool updateXML = svkQuantifyMetabolites::ShouldUpgradeXML();
    struct stat buf;
    if ( updateXML && stat(svkQuantifyMetabolites::GetDefaultXMLFileName( ).c_str(), &buf) == 0 ) {
        string message = "Your quantification file is out of date.  Updating to latest default. \nOld version saved as "; 
        message.append( svkQuantifyMetabolites::GetOldVersionName()); 
        vtkKWMessageDialog *messageDialog = vtkKWMessageDialog::New();
        messageDialog->SetApplication( this->sivicKWApp );
        messageDialog->SetStyle( 0 );
        messageDialog->Create();
        messageDialog->SetOptions( vtkKWMessageDialog::ErrorIcon );
        messageDialog->SetText(message.c_str());
        messageDialog->Invoke();
        int status = messageDialog->GetStatus();
        messageDialog->Delete();

        svkQuantifyMetabolites::SaveOldVersion(); 

    }
    svkQuantifyMetabolites::WriteDefaultXMLTemplate( "", updateXML ); 


}

//! Populates the welcome message. 
void sivicApp::GetWelcomeMessage(vtkKWText* text)
{
    ostringstream* oss = new ostringstream();
    *oss << "**Welcome to ";
    *oss << this->sivicKWApp->GetName() << " " << this->sivicKWApp->GetVersionName() << "!**" << endl; 
    *oss << endl << "Home Page:" << endl << "  http://sivic.sourceforge.net" << endl;; 
    *oss << endl << "User's Guide:" << endl << "  http://sourceforge.net/apps/trac/sivic/wiki/users_guide" << endl;; 
    text->QuickFormattingOn(); 
    text->SetText( oss->str().c_str()); 
    delete oss;
}


//! Populates the main toolbar with buttons
void sivicApp::PopulateMainToolbar(vtkKWToolbar* toolbar)
{

    // Create Open image Button
    vtkKWPushButton* openExamButton = vtkKWPushButton::New();
    openExamButton->SetParent( toolbar->GetFrame() );
    openExamButton->Create();
    openExamButton->SetPadY( 2 );
    openExamButton->SetAnchorToNorth();
    openExamButton->SetText("exam");
    openExamButton->SetCompoundModeToLeft();
    openExamButton->SetImageToPredefinedIcon( vtkKWIcon::IconFileOpen );
    openExamButton->SetCommand( this->sivicController, "OpenExam");
    openExamButton->SetBalloonHelpString( "Open an exam." );
    toolbar->AddWidget( openExamButton );

    // Create Open image Button
    vtkKWMenuButtonWithLabel* openImageButton = vtkKWMenuButtonWithLabel::New();
    openImageButton->SetParent( toolbar->GetFrame() );
    openImageButton->GetWidget()->SetReliefToFlat();
    openImageButton->GetLabel()->SetPadY( 3 );
    openImageButton->GetLabel()->SetAnchorToSouth();
    openImageButton->GetLabel()->SetText("image");
    openImageButton->Create();
    openImageButton->GetLabel()->SetPadY( 3 );
    openImageButton->GetLabel()->SetAnchorToSouth();
    openImageButton->GetWidget()->SetReliefToFlat();
    openImageButton->GetWidget()->SetImageToPredefinedIcon( vtkKWIcon::IconFileOpen );
    openImageButton->SetLabelPositionToRight();
    openImageButton->SetBalloonHelpString( "Open an image dataset.");
    vtkKWMenu* openImageMenu = openImageButton->GetWidget()->GetMenu();
    openImageMenu->AddRadioButton("Load Data", this->sivicController, "OpenFile image NULL 0 0");
    openImageMenu->AddRadioButton("Load Single File", this->sivicController, "OpenFile image NULL 0 1");
    //openImageMenu->AddRadioButton("Open Image As Traces", this->sivicController, "OpenFile image_dynamic NULL 0 0");
    //openImageMenu->AddRadioButton("Add Image As Traces", this->sivicController, "OpenFile add_image_dynamic NULL 0 0");
    openImageMenu->AddRadioButton("Load Data As Traces", this->sivicController, "OpenFile load_images_dynamic NULL 0 0");
    toolbar->AddWidget( openImageButton );

    // Create Open Spectra Selector Menu
    vtkKWMenuButtonWithLabel* openSpectraButton = vtkKWMenuButtonWithLabel::New();
    openSpectraButton->SetParent( toolbar->GetFrame() );
    openSpectraButton->GetWidget()->SetReliefToFlat();
    openSpectraButton->GetLabel()->SetPadY( 3 );
    openSpectraButton->GetLabel()->SetAnchorToSouth();
    openSpectraButton->GetLabel()->SetText("spectra");
    openSpectraButton->Create();
    openSpectraButton->GetLabel()->SetPadY( 3 );
    openSpectraButton->GetLabel()->SetAnchorToSouth();
    openSpectraButton->GetWidget()->SetReliefToFlat();
    openSpectraButton->GetWidget()->SetImageToPredefinedIcon( vtkKWIcon::IconFileOpen );
    //openSpectraButton->GetLabel()->SetHeight( 1 );
    openSpectraButton->SetLabelPositionToRight();
    openSpectraButton->SetBalloonHelpString( "Open a spectroscopic dataset.");
    vtkKWMenu* openSpectraMenu = openSpectraButton->GetWidget()->GetMenu();
    openSpectraMenu->AddRadioButton("Load Data", this->sivicController, "OpenFile spectra NULL 0 0");
    openSpectraMenu->AddRadioButton("Load Single File", this->sivicController, "OpenFile spectra NULL 0 1");
    //openSpectraMenu->AddRadioButton("Add Spectra", this->sivicController, "OpenFile add_spectra NULL 0 0");
    //openSpectraMenu->AddRadioButton("Add Spectra Single Channel", this->sivicController, "OpenFile add_spectra NULL 0 1");
    toolbar->AddWidget( openSpectraButton );

    // Create Open metabolite/overlay Button
    vtkKWMenuButtonWithLabel* openOverlayButton = vtkKWMenuButtonWithLabel::New();
    openOverlayButton->SetParent( toolbar->GetFrame() );
    openOverlayButton->GetWidget()->SetReliefToFlat();
    openOverlayButton->GetLabel()->SetPadY( 3 );
    openOverlayButton->GetLabel()->SetAnchorToSouth();
    openOverlayButton->GetLabel()->SetText("overlay");
    openOverlayButton->Create();
    openOverlayButton->GetLabel()->SetPadY( 3 );
    openOverlayButton->GetLabel()->SetAnchorToSouth();
    openOverlayButton->GetWidget()->SetReliefToFlat();
    openOverlayButton->GetWidget()->SetImageToPredefinedIcon( vtkKWIcon::IconFileOpen );
    openOverlayButton->SetLabelPositionToRight();
    openOverlayButton->SetBalloonHelpString( "Open an image to overlay or a metabolite file." );
    vtkKWMenu* openOverlayMenu = openOverlayButton->GetWidget()->GetMenu();
    openOverlayMenu->AddRadioButton("Load Data", this->sivicController, "OpenFile overlay NULL 0 0");
    openOverlayMenu->AddRadioButton("Load Data Single File", this->sivicController, "OpenFile overlay NULL 0 1");
    openOverlayMenu->AddRadioButton("Load Data As Contours", this->sivicController, "OpenFile contour NULL 0 1");
    toolbar->AddWidget( openOverlayButton );


    // Create secondary capture Button
    vtkKWPushButton* scButton = vtkKWPushButton::New();
    scButton->SetParent( toolbar->GetFrame() );
    scButton->Create();
    scButton->SetImageToPredefinedIcon( vtkKWIcon::IconCamera ); 
    scButton->SetCommand( this->sivicController, "SaveSecondaryCapture COMBINED_CAPTURE 1");
    scButton->SetBalloonHelpString( "Take a secondary capture." );
    toolbar->AddWidget( scButton );

    // Create Selection Style 
    vtkKWPushButton* selectionButton = vtkKWPushButton::New();
    selectionButton->SetParent( toolbar->GetFrame() );
    selectionButton->Create();
    selectionButton->SetImageToPredefinedIcon( vtkKWIcon::IconGridLinear ); 
    selectionButton->SetCommand( this->sivicController, "UseSelectionStyle");
    selectionButton->SetBalloonHelpString( "Switch to voxel selection interactor." );
    toolbar->AddWidget( selectionButton );

    // Create Window Level Style 
    vtkKWPushButton* wlButton = vtkKWPushButton::New();
    wlButton->SetParent( toolbar->GetFrame() );
    wlButton->Create();
    wlButton->SetImageToPredefinedIcon( vtkKWIcon::IconWindowLevel ); 
    wlButton->SetCommand( this->sivicController, "UseWindowLevelStyle");
    wlButton->SetBalloonHelpString( "Switch to window level interactor." );
    toolbar->AddWidget( wlButton );

    // Create color overlay Style 
    vtkKWPushButton* colorOverlayButton = vtkKWPushButton::New();
    colorOverlayButton->SetParent( toolbar->GetFrame() );
    colorOverlayButton->Create();
    colorOverlayButton->SetImageToPredefinedIcon( vtkKWIcon::IconColorSquares ); 
    colorOverlayButton->SetCommand( this->sivicController, "UseColorOverlayStyle");
    colorOverlayButton->SetBalloonHelpString( "Switch to color overlay window level interactor." );
    toolbar->AddWidget( colorOverlayButton );

    // Create Window Level Reset 
    vtkKWPushButton* wlResetButton = vtkKWPushButton::New();
    wlResetButton->SetParent( toolbar->GetFrame() );
    wlResetButton->Create();
    wlResetButton->SetImageToPredefinedIcon( vtkKWIcon::IconDocumentWindowLevel ); 
    wlResetButton->SetCommand( this->sivicController, "ResetWindowLevel");
    wlResetButton->SetBalloonHelpString( "Reset window level." );
    toolbar->AddWidget( wlResetButton );

    // Create Window Voxel Selection Reset 
    vtkKWPushButtonWithMenu* vsResetButton = vtkKWPushButtonWithMenu::New();
    vsResetButton->SetParent( toolbar->GetFrame() );
    vsResetButton->Create();
    //vsResetButton->SetImageToPredefinedIcon( vtkKWIcon::IconCameraMini ); 
    vsResetButton->GetMenuButton()->SetImageToPredefinedIcon( vtkKWIcon::IconResetCamera );
    vsResetButton->SetBalloonHelpString( "Highlight the voxels within the selection box of the current slice or reset data range." );
    vtkKWMenu* vsResetMenu = vsResetButton->GetMenu();
    vsResetMenu->AddRadioButton("Reset View to Voxels Within Selection Box", this->sivicController, "HighlightSelectionBoxVoxels");
    vsResetMenu->AddRadioButton("Reset 4D Amplitude Range To Current Voxels", this->sivicController, "Reset4DAmplitudeRange 0");
    vsResetMenu->AddRadioButton("Reset 4D Amplitude Range To Full", this->sivicController, "Reset4DAmplitudeRange 1");
    toolbar->AddWidget( vsResetButton );

    // Create Rotation Style 
    vtkKWPushButton* rotateButton = vtkKWPushButton::New();
    rotateButton->SetParent( toolbar->GetFrame() );
    rotateButton->Create();
    //rotateButton->SetImageToPredefinedIcon( vtkKWIcon::IconRotate ); 
    rotateButton->SetImageToPredefinedIcon( vtkKWIcon::IconCrystalProject16x16ActionsRotate );
    rotateButton->SetCommand( this->sivicController, "UseRotationStyle");
    rotateButton->SetBalloonHelpString( "Switch to 3D rotation interactor." );
    toolbar->AddWidget( rotateButton );

    // Create Open image Button
    vtkKWPushButton* reloadPreferences = vtkKWPushButton::New();
    reloadPreferences->SetParent( toolbar->GetFrame() );
    reloadPreferences->Create();
    reloadPreferences->SetCompoundModeToLeft();
    reloadPreferences->SetImageToPredefinedIcon( vtkKWIcon::IconHSVDiagram );
    reloadPreferences->SetCommand( this->sivicController, "SetPreferencesFromRegistry");
    reloadPreferences->SetBalloonHelpString( "Reload Preferences." );
    //toolbar->AddWidget( reloadPreferences );

    // Create Orientation Selector Menu
    vtkKWPushButtonWithMenu* orientationButton = vtkKWPushButtonWithMenu::New();
    orientationButton->SetParent( toolbar->GetFrame() );
    orientationButton->Create();
    orientationButton->GetMenuButton()->SetImageToPredefinedIcon( vtkKWIcon::IconAngleTool);
    orientationButton->SetBalloonHelpString( "Select the orientation to view."); 
    vtkKWMenu* orientationMenu = orientationButton->GetMenu();
    orientationMenu->AddRadioButton("Axial", this->sivicController, "SetOrientation AXIAL 1"); 
    orientationMenu->AddRadioButton("Coronal", this->sivicController, "SetOrientation CORONAL 1"); 
    orientationMenu->AddRadioButton("Sagittal", this->sivicController, "SetOrientation SAGITTAL 1"); 
    toolbar->AddWidget( orientationButton );



#if defined(Darwin)
    //  Create OsiriX buttons
    vtkKWPushButton* osirixSCButton = vtkKWPushButton::New();
    osirixSCButton->SetParent( toolbar->GetFrame() );
    osirixSCButton->Create();
#if defined (OSX_PLUGIN_OSIRIX)
    osirixSCButton->SetText( "OsiriX SC");
#elif defined (OSX_PLUGIN_HOROS)
    osirixSCButton->SetText( "Horos SC");
#endif
    osirixSCButton->SetCommand( this->sivicController, "SaveSecondaryCaptureOsiriX"); 
    osirixSCButton->SetBalloonHelpString( "Save DICOM Secondary Capture data to OsiriX" );
    toolbar->AddWidget( osirixSCButton );

    vtkKWPushButton* osirixMRSButton = vtkKWPushButton::New();
    osirixMRSButton->SetParent( toolbar->GetFrame() );
    osirixMRSButton->Create();
#if defined (OSX_PLUGIN_OSIRIX)
    osirixMRSButton->SetText( "OsiriX MRS");
#elif defined (OSX_PLUGIN_HOROS)
    osirixMRSButton->SetText( "Horos MRS");
#endif
    osirixMRSButton->SetCommand( this->sivicController, "SaveDataOsiriX"); 
    osirixMRSButton->SetBalloonHelpString( "Save DICOM MRS data to OsiriX" );
    toolbar->AddWidget( osirixMRSButton );

    vtkKWPushButton* osirixMetMapButton = vtkKWPushButton::New();
    osirixMetMapButton->SetParent( toolbar->GetFrame() );
    osirixMetMapButton->Create();
#if defined (OSX_PLUGIN_OSIRIX)
    osirixMetMapButton->SetText( "OsiriX Met Maps");
#elif defined (OSX_PLUGIN_HOROS)
    osirixMetMapButton->SetText( "Horos Met Maps");
#endif
    osirixMetMapButton->SetCommand( this->sivicController, "SaveMetMapDataOsiriX"); 
    osirixMetMapButton->SetBalloonHelpString( "Save DICOM metabolite map data to OsiriX" );
    toolbar->AddWidget( osirixMetMapButton );
#endif

    // Create Open Spectra Selector Menu
    /*
    vtkKWMenuButtonWithLabel* generateTraces = vtkKWMenuButtonWithLabel::New();
    generateTraces->SetParent( toolbar->GetFrame() );
    generateTraces->GetWidget()->SetReliefToFlat();
    generateTraces->SetLabelPositionToLeft();
    generateTraces->GetLabel()->SetPadY( 3 );
    generateTraces->GetLabel()->SetAnchorToSouth();
    generateTraces->GetLabel()->SetText("Generate Traces");
    generateTraces->Create();
    generateTraces->GetLabel()->SetPadY( 3 );
    generateTraces->GetLabel()->SetAnchorToSouth();
    generateTraces->GetWidget()->SetReliefToFlat();
    generateTraces->GetWidget()->SetImageToPredefinedIcon( vtkKWIcon::IconBrowserForward );
    //openSpectraButton->GetLabel()->SetHeight( 1 );
    generateTraces->SetBalloonHelpString( "Generate traces from a multi-volumetric image dataset.");
    vtkKWMenu* generateTracesMenu = generateTraces->GetWidget()->GetMenu();
    generateTracesMenu->AddRadioButton("From Reference Image", this->sivicController, "GenerateTraces reference_image");
    generateTracesMenu->AddRadioButton("From Overlay Image", this->sivicController, "GenerateTraces overlay_image");
    toolbar->AddWidget( generateTraces );
    */

}


/*! 
 *  Start the application and parse command line arguments. 
 */
char** sivicApp::ParseCommandLineArgs( int* argc, char* argv[] )
{


    string usemsg("\n") ;
    usemsg += "Version " + string(SVK_RELEASE_VERSION) +                   "\n";
    usemsg += "                                                             \n";
    usemsg += "sivic [-a anatomy] [-h]                                      \n";
    usemsg += "                                                             \n";
    usemsg += "     -a anatomy      Anatomy preferences                     \n";
    usemsg += "                         brain (default)                     \n";
    usemsg += "                         prostate                            \n";
    usemsg += "     -i fileName     Input file name, may specify multiple,  \n";
    usemsg += "                     instances of -i, e.g MRS, MRI (ref and  \n";
    usemsg += "                     overlay). For series, only specify one  \n"; 
    usemsg += "                     image from the series.                  \n"; 
    usemsg += "     --is fileName    Only load the single explicit file name,\n";  
    usemsg += "                     no globbing of associated files names.  \n";
    usemsg += "     --id fileName    Load as dynamic traces                  \n";  
    usemsg += "     -h              Print help mesage.                      \n";
    usemsg += "                                                             \n";
    usemsg += "SIVIC GUI.                                                   \n";
    usemsg += "                                                             \n";
    usemsg += "Example:                                                     \n";
    usemsg += "sivic -i P1234 -i E1234S1I1.DCM -i E1234S2I1.DCM             \n";
    usemsg += "                                                             \n";

    string anatomyTypeString = "brain";  
    int anatomyType = svkTypes::ANATOMY_BRAIN;

    enum FLAG_NAME {
        FLAG_SINGLE_FILE_LOAD = 0, 
        FLAG_LOAD_DYNAMIC
    };


    static struct option long_options[] =
    {
        {"is",                required_argument, NULL,  FLAG_SINGLE_FILE_LOAD},
        {"id",                required_argument, NULL,  FLAG_LOAD_DYNAMIC},
        {0, 0, 0, 0}
    };


    /*
     *  Process flags and arguments
     */
    int i;
    int option_index = 0;
    while ((i = getopt_long(*argc, argv, "a:i:h", long_options, &option_index)) != EOF) {
        switch (i) {
            case 'a':
                anatomyTypeString.assign( optarg );
                anatomyType = svkTypes::GetAnatomyType(anatomyTypeString); 
                if ( anatomyType != -1 ) {
                    this->sivicController->SetAnatomyType(anatomyType); 
                }
                break;
            case 'i':
                this->inputFiles.push_back( optarg );
                this->inputFilesOnlyLoadOne.push_back( false);
                this->inputFilesDynamic.push_back( false);
                break;
            case FLAG_SINGLE_FILE_LOAD:
                this->inputFiles.push_back( optarg );
                this->inputFilesOnlyLoadOne.push_back( true );
                this->inputFilesDynamic.push_back( false);
                break;
            case FLAG_LOAD_DYNAMIC:
                this->inputFiles.push_back( optarg );
                this->inputFilesOnlyLoadOne.push_back( false );
                this->inputFilesDynamic.push_back( true );
                break;
            case 'h':
                cout << usemsg << endl;
                exit(1);
                break;
            default:
                ;
        }
    }

    *argc -= optind;
    argv += optind;

    if ( *argc != 0 ) {
        cout << "ERROR: invalid arguments" << *argc << endl;
        cout << usemsg << endl;
        exit(1);
    }

    return argv; 

}


/*! 
 *  Start the application and parse command line arguments. 
 */
int sivicApp::Start( int argc, char* argv[] )
{


    argv = this->ParseCommandLineArgs(&argc, argv); 

    //  argc is reduced by 1 after ParseCommandLineArgs, so it only reflects the 
    //  remaining args after the program name, if any


    //  Preparse the MR Image objects to try and detect which is an overlay:
    //  Assume that the reference image is higher res than the overlay
    int refImageIndex = -1; 
    int overlayImageIndex = -1; 
    int refRows = 0; 
    int refColumns = 0; 
    int overlayRows = 10000000; 
    int overlayColumns = 10000000; 

    for (int i = 0 ; i < this->inputFiles.size(); i++) {

        svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
        readerFactory->QuickParse();
        svkImageReader2* reader = readerFactory->CreateImageReader2( this->inputFiles[i].c_str() );
        readerFactory->Delete();
        if (reader == NULL ) {
            cout << "ERROR: Can not read specified input file (not DICOM): " << this->inputFiles[i] << endl;
            return 0; 
        } 
        reader->SetFileName( this->inputFiles[i].c_str() );
        reader->OnlyReadOneInputFile();
        reader->Update();
        svkImageData* tmp =  reader->GetOutput();

        string SOPClassUID = tmp->GetDcmHeader()->GetStringValue( "SOPClassUID" ) ;

        //  Check MRImage Storage and Enhanced MRImage Storage
        if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.4" || SOPClassUID == "1.2.840.10008.5.1.4.1.1.4.1" ) {

            if ( this->inputFilesDynamic[i] == false ) {
                int rows = tmp->GetDcmHeader()->GetIntValue( "Rows" ) ;
                int columns = tmp->GetDcmHeader()->GetIntValue( "Columns" ) ;
                //cout << "check : " << rows << " vs " << refRows << " " << columns << " vs " << refColumns << endl;
                if ( rows > refRows && columns > refColumns) {
                    refRows = rows; 
                    refColumns = columns; 
                    refImageIndex = i; 
                } 

                if ( rows < refRows && columns < refColumns) {
                    overlayRows = rows; 
                    overlayColumns = columns; 
                    overlayImageIndex = i; 
                }
            }
        }
        reader->Delete();
    }

    //  Overlay is always the last to load: 
    vtkstd::vector< int >   loadOrder; 
    for (int i = 0 ; i < this->inputFiles.size(); i++) {
        if ( i != overlayImageIndex ) {
            loadOrder.push_back(i);
        }
    }
    //  Load overlay last:
    loadOrder.push_back(overlayImageIndex);

    for (int i = 0 ; i < this->inputFiles.size(); i++) {
        //cout << "loading order: " << loadOrder[i] << endl;
    }

    //  Check each argv and set the load file type 
    for (int i = 0 ; i < this->inputFiles.size(); i++) {

        //cout << " load order: " << i << " " << loadOrder[i] <<  argv[ loadOrder[i-1] ] << endl;
        //cout << "INPUT FILES loadOrder: " << this->inputFiles[ loadOrder[i] ] << endl;
        svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
        readerFactory->QuickParse();
        svkImageReader2* reader = readerFactory->CreateImageReader2( this->inputFiles[ loadOrder[i] ].c_str() );
        readerFactory->Delete();
        if (reader == NULL ) {
            cout << "ERROR: Can not read specified input file (not DICOM): " << this->inputFiles[i] << endl;
            return 0; 
        } 

        reader->SetFileName( this->inputFiles[ loadOrder[i] ].c_str() );
        reader->OnlyReadOneInputFile();
        reader->Update();

        svkImageData* tmp =  reader->GetOutput();
        string SOPClassUID = tmp->GetDcmHeader()->GetStringValue( "SOPClassUID" ) ;
        if ( reader->IsA("svkGEPostageStampReader") ) {
            SOPClassUID = "1.2.840.10008.5.1.4.1.1.4.2";
        }
        reader->Delete();
        //cout << " LOAD ONE? " << this->inputFilesOnlyLoadOne[i] << endl;
        //  Check MRImage Storage and Enhanced MRImage Storage
        if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.4" || SOPClassUID == "1.2.840.10008.5.1.4.1.1.4.1" ) {

            if ( this->inputFilesDynamic[i] == true ) {

                //  When loading dynamic data turn off text overlay since this is typically 
                //  perf data with too much text (voxels)
                this->GetView()->OverlayTextOff();
                this->GetView()->OpenFile(
                            "command_line_load_images_dynamic", 
                            this->inputFiles[ loadOrder[i] ].c_str(), 
                            0, 
                            this->inputFilesOnlyLoadOne[i]
                        ); 

            }
            if ( loadOrder[i] == refImageIndex ) {
                this->GetView()->OpenFile(
                        "command_line_image", 
                        this->inputFiles[ loadOrder[i] ].c_str(), 
                        0, 
                        this->inputFilesOnlyLoadOne[i]
                ); 
            } else {
                this->GetView()->OpenFile(
                            "command_line_overlay", 
                            this->inputFiles[ loadOrder[i] ].c_str(), 
                            0, 
                            this->inputFilesOnlyLoadOne[i] 
                        ); 
            }

        } else if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.4.2" ) {
            this->GetView()->OpenFile(
                    "command_line_spectra", 
                    this->inputFiles[ loadOrder[i] ].c_str(), 
                    0, 
                    this->inputFilesOnlyLoadOne[i] 
                    ); 
        }
    }

    if ( this->sivicController->anatomyType == vtkSivicController::ANATOMY_PROSTATE ) {
        this->GetView()->SetAnatomyPrefs();
    }
    
    // model is used to manage data that has been loaded 
    this->sivicKWApp->Start(argc, argv);
    return this->exitStatus;
}
