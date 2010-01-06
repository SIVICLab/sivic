/*
 *  Copyright © 2009 The Regents of the University of California.
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
    this->exitStatus = 0;
    this->processingWidget = sivicProcessingWidget::New();
    this->imageViewWidget = sivicImageViewWidget::New();
    this->spectraViewWidget = sivicSpectraViewWidget::New();
    this->viewRenderingWidget = sivicViewRenderingWidget::New();
    this->tabbedPanel = vtkKWNotebook::New();
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

    if( this->imageViewWidget != NULL ) {
        this->imageViewWidget->Delete();
        this->imageViewWidget = NULL;
    }

    if( this->spectraViewWidget != NULL ) {
        this->spectraViewWidget->Delete();
        this->spectraViewWidget = NULL;
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
    this->sivicKWApp->SetMajorVersion( MAJOR_VERSION );
    this->sivicKWApp->SetMinorVersion( MINOR_VERSION );

    // Add a Window to the application
    this->sivicWindow = vtkKWWindowBase::New();
    this->sivicKWApp->AddWindow(this->sivicWindow);
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

    this->processingWidget->SetParent(tabbedPanel );
    this->processingWidget->SetPlotController(this->sivicController->GetPlotController());
    this->processingWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->processingWidget->SetSivicController(this->sivicController);
    this->processingWidget->Create();

    this->imageViewWidget->SetParent(this->sivicWindow->GetViewFrame());
    this->imageViewWidget->SetPlotController(this->sivicController->GetPlotController());
    this->imageViewWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->imageViewWidget->SetSivicController(this->sivicController);
    this->imageViewWidget->Create();

    this->spectraViewWidget->SetParent(tabbedPanel);
    this->spectraViewWidget->SetPlotController(this->sivicController->GetPlotController());
    this->spectraViewWidget->SetOverlayController(this->sivicController->GetOverlayController());
    this->spectraViewWidget->SetSivicController(this->sivicController);
    this->spectraViewWidget->Create();


    this->sivicController->SetViewRenderingWidget( viewRenderingWidget );
    this->sivicController->SetProcessingWidget( processingWidget );
    this->sivicController->SetImageViewWidget( imageViewWidget );
    this->sivicController->SetSpectraViewWidget( spectraViewWidget );

    this->tabbedPanel->AddPage("Inspecting", "Interact with the view of the loaded data.", NULL);
    vtkKWWidget* interactorPanel = tabbedPanel->GetFrame("Inspecting");

    this->tabbedPanel->AddPage("Processing", "Process the data.", NULL);
    vtkKWWidget* processingPanel = tabbedPanel->GetFrame("Processing");

    vtkKWSeparator* separator = vtkKWSeparator::New();
    separator->SetParent(this->sivicWindow->GetViewFrame());
    separator->Create();
    separator->SetThickness(5);

    vtkKWSeparator* separatorVert = vtkKWSeparator::New();
    separatorVert->SetParent(this->sivicWindow->GetViewFrame());
    separatorVert->Create();
    separatorVert->SetThickness(3);

    this->sivicKWApp->Script("grid %s -row 0 -column 0 -columnspan 3 -sticky nsew", viewRenderingWidget->GetWidgetName());
    this->sivicKWApp->Script("grid %s -row 1 -column 0 -columnspan 3 -sticky nsew", separator->GetWidgetName());
    this->sivicKWApp->Script("grid %s -row 2 -column 0 -sticky wnse", imageViewWidget->GetWidgetName());
    this->sivicKWApp->Script("grid %s -row 2 -column 1 -sticky nsew", separatorVert->GetWidgetName());
    this->sivicKWApp->Script("grid %s -row 2 -column 2 -sticky ensw -padx 2 -pady 2", tabbedPanel->GetWidgetName());

    this->sivicKWApp->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 2 -in %s", 
              this->spectraViewWidget->GetWidgetName(), interactorPanel->GetWidgetName());
    this->sivicKWApp->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 2 -in %s", 
              this->processingWidget->GetWidgetName(), processingPanel->GetWidgetName());

    this->sivicKWApp->Script("grid rowconfigure    %s 0 -weight 60 -minsize 300 ", this->sivicWindow->GetViewFrame()->GetWidgetName() );
    this->sivicKWApp->Script("grid rowconfigure    %s 1 -weight 0 ", this->sivicWindow->GetViewFrame()->GetWidgetName() );
    this->sivicKWApp->Script("grid rowconfigure    %s 2 -weight 40 -minsize 300 ", this->sivicWindow->GetViewFrame()->GetWidgetName() );
    this->sivicKWApp->Script("grid columnconfigure %s 0 -weight 40 -uniform 1 -minsize 350 ", this->sivicWindow->GetViewFrame()->GetWidgetName() );
    this->sivicKWApp->Script("grid columnconfigure %s 1 -weight 0 -minsize 5", this->sivicWindow->GetViewFrame()->GetWidgetName() );
    this->sivicKWApp->Script("grid columnconfigure %s 2 -weight 60 -uniform 1", this->sivicWindow->GetViewFrame()->GetWidgetName() );
    separator->Delete();
    separatorVert->Delete();


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

    // Now we add the "open" callback
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            0, "&Open", this->sivicController, "OpenFile .* NULL");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            1, "&Save Data", this->sivicController, "SaveData");
   /*
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            2, "&Save Spectra Secondary Capture", this->sivicController, "SaveSecondaryCapture SPECTRA_CAPTURE");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            3, "&Save Image Secondary Capture", this->sivicController, "SaveSecondaryCapture IMAGE_CAPTURE");
    */
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            2, "&Save Secondary Capture", this->sivicController, "SaveSecondaryCapture SPECTRA_WITH_OVERVIEW_CAPTURE");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            3, "&Print Current Slice", this->sivicController, "Print 1");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            4, "&Print All Slices", this->sivicController, "Print 0");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            5, "&Save Session", this->sivicController, "SaveSession");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            6, "&Restore Session", this->sivicController, "RestoreSession");
    this->sivicKWApp->GetNthWindow(0)->GetFileMenu()->InsertCommand(
            7, "&Close All", this->sivicController, "ResetApplication");
    this->sivicKWApp->GetNthWindow(0)->GetHelpMenu()->InsertCommand(
            0, "&Sivic Resources", this->sivicController, "DisplayInfo");
#if defined(DEBUG_BUILD)
    this->sivicKWApp->GetNthWindow(0)->GetHelpMenu()->InsertCommand(
            1, "&Run Tests", this->sivicController, "RunTestingSuite");
#endif
    this->sivicKWApp->SetHelpDialogStartingPage("https://sivic.sourceforge.com");
    this->sivicWindow->Display();
}


//! Populates the main toolbar with buttons
void sivicApp::PopulateMainToolbar(vtkKWToolbar* toolbar)
{

    // Create Open image Button
    vtkKWPushButton* openExamButton = vtkKWPushButton::New();
    openExamButton->SetParent( toolbar->GetFrame() );
    openExamButton->Create();
    openExamButton->SetText("exam");
    openExamButton->SetCompoundModeToLeft();
    openExamButton->SetImageToPredefinedIcon( vtkKWIcon::IconFileOpen );
    openExamButton->SetCommand( this->sivicController, "OpenExam");
    openExamButton->SetBalloonHelpString( "Open an exam." );
    toolbar->AddWidget( openExamButton );

    // Create Open image Button
    vtkKWPushButton* openIdfButton = vtkKWPushButton::New();
    openIdfButton->SetParent( toolbar->GetFrame() );
    openIdfButton->Create();
    openIdfButton->SetText("image");
    openIdfButton->SetCompoundModeToLeft();
    openIdfButton->SetImageToPredefinedIcon( vtkKWIcon::IconFileOpen );
    openIdfButton->SetCommand( this->sivicController, "OpenFile image NULL");
    openIdfButton->SetBalloonHelpString( "Open a image file." );
    toolbar->AddWidget( openIdfButton );

    // Create Open spectra Button
    vtkKWPushButton* openDdfButton = vtkKWPushButton::New();
    openDdfButton->SetParent( toolbar->GetFrame() );
    openDdfButton->Create();
    openDdfButton->SetReliefToGroove();
    openDdfButton->SetText("spectra");
    openDdfButton->SetCompoundModeToLeft();
    openDdfButton->SetImageToPredefinedIcon( vtkKWIcon::IconFileOpen ); 
    openDdfButton->SetCommand( this->sivicController, "OpenFile spectra NULL");
    openDdfButton->SetBalloonHelpString( "Open a spectra file." );
    toolbar->AddWidget( openDdfButton );

    // Create Open metabolite/overlay Button
    vtkKWPushButton* openMetButton = vtkKWPushButton::New();
    openMetButton->SetParent( toolbar->GetFrame() );
    openMetButton->Create();
    openMetButton->SetReliefToGroove();
    openMetButton->SetText("overlay");
    openMetButton->SetCompoundModeToLeft();
    openMetButton->SetImageToPredefinedIcon( vtkKWIcon::IconFileOpen );
    openMetButton->SetCommand( this->sivicController, "OpenFile overlay NULL");
    openMetButton->SetBalloonHelpString( "Open an image to overlay or a metabolite file." );
    toolbar->AddWidget( openMetButton );

    // Create secondary capture Button
    vtkKWPushButton* scButton = vtkKWPushButton::New();
    scButton->SetParent( toolbar->GetFrame() );
    scButton->Create();
    scButton->SetImageToPredefinedIcon( vtkKWIcon::IconCamera ); 
    scButton->SetCommand( this->sivicController, "SaveSecondaryCapture SPECTRA_CAPTURE");
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
    vtkKWPushButton* vsResetButton = vtkKWPushButton::New();
    vsResetButton->SetParent( toolbar->GetFrame() );
    vsResetButton->Create();
    vsResetButton->SetImageToPredefinedIcon( vtkKWIcon::IconResetCamera ); 
    vsResetButton->SetCommand( this->sivicController, "HighlightSelectionBoxVoxels");
    vsResetButton->SetBalloonHelpString( "Highlight the voxels within the selection box of the current slice." );
    toolbar->AddWidget( vsResetButton );

    // Create Rotation Style 
    vtkKWPushButton* rotateButton = vtkKWPushButton::New();
    rotateButton->SetParent( toolbar->GetFrame() );
    rotateButton->Create();
    rotateButton->SetImageToPredefinedIcon( vtkKWIcon::IconCrystalProject16x16ActionsRotate ); 
    rotateButton->SetCommand( this->sivicController, "UseRotationStyle");
    rotateButton->SetBalloonHelpString( "Switch to 3D rotation interactor." );
    toolbar->AddWidget( rotateButton );

#if defined(Darwin)
    //  Create OsiriX buttons
    vtkKWPushButton* osirixSCButton = vtkKWPushButton::New();
    osirixSCButton->SetParent( toolbar->GetFrame() );
    osirixSCButton->Create();
    osirixSCButton->SetText( "OsiriX SC");
    osirixSCButton->SetCommand( this->sivicController, "SaveSecondaryCaptureOsiriX"); 
    osirixSCButton->SetBalloonHelpString( "Save DICOM Secondary Capture data to OsiriX" );
    toolbar->AddWidget( osirixSCButton );

    vtkKWPushButton* osirixMRSButton = vtkKWPushButton::New();
    osirixMRSButton->SetParent( toolbar->GetFrame() );
    osirixMRSButton->Create();
    osirixMRSButton->SetText( "OsiriX MRS");
    osirixMRSButton->SetCommand( this->sivicController, "SaveDataOsiriX"); 
    osirixMRSButton->SetBalloonHelpString( "Save DICOM MRS data to OsiriX" );
    toolbar->AddWidget( osirixMRSButton );
#endif


}


/*! 
 *  Start the application.
 */
int sivicApp::Start( int argc, char* argv[] )
{

    //  Check each argv and set the load file type 
    for (int i=1 ; i < argc; i++) {

        svkImageData* tmp = svkMriImageData::New();
        tmp->GetDcmHeader()->ReadDcmFile( argv[i] );
        //tmp->GetDcmHeader()->PrintDcmHeader(); 
        string SOPClassUID = tmp->GetDcmHeader()->GetStringValue( "SOPClassUID" ) ;
        tmp->Delete();

        if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.4" ) {
            this->GetView()->OpenFile("command_line_image", argv[i]); 
        } else if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.4.2" ) {
            this->GetView()->OpenFile("command_line_spectra", argv[i]); 
        }
    }
    
    // model is used to manage data that has been loaded 
    this->sivicKWApp->Start(argc, argv);
    return this->exitStatus;
}
