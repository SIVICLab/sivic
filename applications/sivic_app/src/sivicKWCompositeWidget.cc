/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */



#include <sivicKWCompositeWidget.h>
#include <vtkSivicController.h>

vtkStandardNewMacro( sivicKWCompositeWidget );
vtkCxxRevisionMacro( sivicKWCompositeWidget, "$Revision$");


/*! 
 *  Constructor
 */
sivicKWCompositeWidget::sivicKWCompositeWidget()
{
    this->plotController = NULL;
    this->overlayController = NULL;
    this->sivicController = NULL;
    this->model = NULL;

}


/*! 
 *  Destructor
 */
sivicKWCompositeWidget::~sivicKWCompositeWidget()
{

}

/*! 
 *  Pure setter method (this->x = x) 
 */     
void sivicKWCompositeWidget::SetSivicController( vtkSivicController* sivicController )
{       
    this->sivicController = sivicController;
}       

/*! 
 *  Pure setter method (this->x = x) 
 */     
void sivicKWCompositeWidget::SetPlotController( svkPlotGridViewController* plotController )
{       
    this->plotController = plotController;
}       
        
/*! 
 *  Pure setter method (this->x = x) 
 */     
void sivicKWCompositeWidget::SetOverlayController( svkOverlayViewController* overlayController )
{       
    this->overlayController = overlayController;
}  


/*!
 *  Sets the model to be used which contains the data.
 */
void sivicKWCompositeWidget::SetModel( svkDataModel* model )
{
    this->model = model;
}
