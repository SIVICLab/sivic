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
    this->orientationSelect->SetLabelPositionToLeft();
    //this->orientationSelect->SetPadY(10);
    //this->orientationSelect->SetPadX(8);
    //this->orientationSelect->SetHeight(2);
    //this->orientationSelect->GetWidget()->SetWidth(4);
    //this->orientationSelect->EnabledOff();
    vtkKWMenu* unitMenu = this->orientationSelect->GetWidget()->GetMenu();
    unitMenu->AddRadioButton("AXIAL", this->sivicController, "SetOrientation AXIAL");
    unitMenu->AddRadioButton("CORONAL", this->sivicController, "SetOrientation CORONAL");
    unitMenu->AddRadioButton("SAGITTAL", this->sivicController, "SetOrientation SAGITTAL");
    this->Script("grid %s -row 0 -column 0 -sticky wnse -pady 5 ", this->specViewFrame->GetWidgetName());

    this->Script("grid %s -in %s -row 0 -column 0 -sticky sw", 
                this->orientationSelect->GetWidgetName(), this->specViewFrame->GetWidgetName()); 
}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicGlobalWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{

}
