/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */



#include <sivicGlobalWidget.h>
#include <vtkSivicController.h>


vtkStandardNewMacro( sivicGlobalWidget );
vtkCxxRevisionMacro( sivicGlobalWidget, "$Revision$");


/*! 
 *  Constructor
 */
sivicGlobalWidget::sivicGlobalWidget()
{
    this->orientationSelect = NULL;

}


/*! 
 *  Destructor
 */
sivicGlobalWidget::~sivicGlobalWidget()
{
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

    this->orientationSelect = vtkKWMenuButtonWithLabel::New();   
    this->orientationSelect->SetParent(this);
    this->orientationSelect->Create();
    this->orientationSelect->SetLabelText("Orientation");
    this->orientationSelect->SetLabelPositionToTop();

    vtkKWMenu* unitMenu = this->orientationSelect->GetWidget()->GetMenu();
    unitMenu->AddRadioButton("AXIAL", this->sivicController, "SetOrientation AXIAL 1");
    unitMenu->AddRadioButton("CORONAL", this->sivicController, "SetOrientation CORONAL 1");
    unitMenu->AddRadioButton("SAGITTAL", this->sivicController, "SetOrientation SAGITTAL 1");

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

    this->Script("grid rowconfigure  %s 0 -weight 1 ", this->specViewFrame->GetWidgetName() );
    this->Script("grid rowconfigure  %s 1 -weight 1 ", this->specViewFrame->GetWidgetName() );
    this->Script("grid rowconfigure  %s 2 -weight 100 -minsize 150", this->specViewFrame->GetWidgetName() );

}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicGlobalWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
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
				if( svkUtils::FilePathExists(metaboliteFileName.c_str()) ) {
                    commandString = "OpenMetabolites";
                    commandString += " \""; 
                    commandString += *it;
                    commandString += "\""; 
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
