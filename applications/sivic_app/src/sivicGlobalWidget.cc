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
}


/*! 
 *  Destructor
 */
sivicGlobalWidget::~sivicGlobalWidget()
{
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
}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicGlobalWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
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
