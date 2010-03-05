/*
 *  $URL: $
 *  $Rev: $
 *  $Author: $
 *  $Date: $
 */



#include <sivicGlobalWidget.h>
#include <vtkSivicController.h>


vtkStandardNewMacro( sivicGlobalWidget );
vtkCxxRevisionMacro( sivicGlobalWidget, "$Revision: 38 $");


/*! 
 *  Constructor
 */
sivicGlobalWidget::sivicGlobalWidget()
{
    this->orientationSelect = NULL;
    this->sliceSlider = NULL;
    this->centerImage = true;

}


/*! 
 *  Destructor
 */
sivicGlobalWidget::~sivicGlobalWidget()
{
    if( this->sliceSlider != NULL ) {
        this->sliceSlider->Delete();
        this->sliceSlider = NULL;
    }

    if( this->orientationSelect != NULL ) {
        this->orientationSelect ->Delete();
        this->orientationSelect = NULL;
    }
    if( this->metaboliteSelect != NULL ) {
        this->metaboliteSelect ->Delete();
        this->metaboliteSelect = NULL;
    }
}


/*!
 *   Set to true if callback for slicing should center the image inside the spectra
 */
void sivicGlobalWidget::SetCenterImage( bool centerImage )
{
    this->centerImage = centerImage;
}


/*! 
 *  Method in superclass to be overriden to add our custom widgets.
 */
void sivicGlobalWidget::CreateWidget()
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
    this->sliceSlider->SetEntryWidth( 3 );
    this->sliceSlider->SetOrientationToHorizontal();
    //this->sliceSlider->SetOrientationToVertical();
    this->sliceSlider->SetLabelText("Spectroscopic Slice");
    this->sliceSlider->SetValue(this->plotController->GetSlice()+1);
    this->sliceSlider->SetBalloonHelpString("Changes the spectroscopic slice.");
    this->sliceSlider->SetEntryPositionToBottom();
    this->sliceSlider->SetLabelPositionToBottom();
    this->sliceSlider->SetRange( 1, 1 );
    this->sliceSlider->EnabledOff();

    this->orientationSelect = vtkKWMenuButtonWithLabel::New();   
    this->orientationSelect->SetParent(this);
    this->orientationSelect->Create();
    this->orientationSelect->SetLabelText("Orientation");
    this->orientationSelect->SetLabelPositionToTop();
    //this->orientationSelect->SetPadY(10);
    //this->orientationSelect->SetPadX(8);
    //this->orientationSelect->SetHeight(2);
    //this->orientationSelect->GetWidget()->SetWidth(4);
    //this->orientationSelect->EnabledOff();
    vtkKWMenu* unitMenu = this->orientationSelect->GetWidget()->GetMenu();
    unitMenu->AddRadioButton("AXIAL", this->sivicController, "SetOrientation AXIAL");
    unitMenu->AddRadioButton("CORONAL", this->sivicController, "SetOrientation CORONAL");
    unitMenu->AddRadioButton("SAGITTAL", this->sivicController, "SetOrientation SAGITTAL");

    this->metaboliteSelect = vtkKWMenuButtonWithLabel::New();
    this->metaboliteSelect->SetParent( this->specViewFrame );
    this->metaboliteSelect->Create();
    this->metaboliteSelect->SetLabelText("Detected Metabolites");
    this->metaboliteSelect->SetLabelPositionToTop();
    this->metaboliteSelect->EnabledOff();
    this->PopulateMetaboliteMenu( );

    this->Script("grid %s -row 0 -column 0 -sticky wnse -pady 5 ", this->specViewFrame->GetWidgetName());

    this->Script("grid %s -in %s -row 0 -column 0 -sticky sw", 
                this->orientationSelect->GetWidgetName(), this->specViewFrame->GetWidgetName()); 

#if defined( UCSF_INTERNAL )
    // Add a drop down that lets you select overlays by name. Based on UCSF naming conventions.
    this->Script("grid %s -in %s -row 1 -column 0 -sticky sw", 
                this->metaboliteSelect->GetWidgetName(), this->specViewFrame->GetWidgetName()); 
#endif

    this->Script("grid %s -in %s -row 2 -column 0 -sticky s -padx 5 -pady 10 ", this->sliceSlider->GetWidgetName(), this->specViewFrame->GetWidgetName());
    this->Script("grid rowconfigure  %s 0 -weight 1 ", this->specViewFrame->GetWidgetName() );
    this->Script("grid rowconfigure  %s 1 -weight 1 ", this->specViewFrame->GetWidgetName() );
    this->Script("grid rowconfigure  %s 2 -weight 100 -minsize 150", this->specViewFrame->GetWidgetName() );



    this->AddCallbackCommandObserver(
        this->sliceSlider->GetWidget(), vtkKWEntry::EntryValueChangedEvent );

}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicGlobalWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    if( caller == this->sliceSlider->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent) {
        this->sivicController->SetSlice( static_cast<int>(this->sliceSlider->GetValue()) - 1, centerImage);
        stringstream increment;
        increment << "SetValue " << this->overlayController->GetSlice() + 2;
        stringstream decrement;
        decrement << "SetValue " << this->overlayController->GetSlice();
        this->sliceSlider->RemoveBinding( "<Left>");
        this->sliceSlider->AddBinding( "<Left>", this->sliceSlider, decrement.str().c_str() );
        this->sliceSlider->RemoveBinding( "<Right>");
        this->sliceSlider->AddBinding( "<Right>", this->sliceSlider, increment.str().c_str() );
        this->sliceSlider->Focus();
    }
}


/*!
 *
 */
void sivicGlobalWidget::PopulateMetaboliteMenu( ) 
{
    vector<string> names = svkUCSFUtils::GetAllMetaboliteNames(); 
    vector<string>::iterator it = names.begin();
    string commandString;
    vtkKWMenu* metaboliteNames = this->metaboliteSelect->GetWidget()->GetMenu();
    metaboliteNames->DeleteAllItems();
    if( this->model != NULL ) {
        string spectraFile = this->model->GetDataFileName("SpectroscopicData");
        while(it != names.end()) {
            if( spectraFile != "" ) {
                bool includePath = true;
                string metaboliteFileName;
                metaboliteFileName = svkUCSFUtils::GetMetaboliteFileName(
                                        this->model->GetDataFileName("SpectroscopicData"), it->c_str(), includePath );

                struct stat st;
                // Lets check to see if the file exists 
                if(stat(metaboliteFileName.c_str(),&st) == 0) {
                    commandString = "OpenMetabolites";
                    commandString += " \""; 
                    commandString += *it;
                    commandString += "\""; 
                    cout << "Command string is: " << commandString << endl;
                    metaboliteNames->AddRadioButton(it->c_str(), this->sivicController, commandString.c_str());
                }

            }
            ++it;
        }
    }
}


/*! 
 *  Make sure no metabolites are selected.
 */
void sivicGlobalWidget::DeselectMetabolites( )
{
    int numItems = this->metaboliteSelect->GetWidget()->GetMenu()->GetNumberOfItems(); 
    for( int i = 0; i < numItems; i++ ) {
        this->metaboliteSelect->GetWidget()->GetMenu()->DeselectItem(i); 
    }
 
}
