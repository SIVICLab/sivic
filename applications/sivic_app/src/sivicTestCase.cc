/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */
#include <sivicTestCase.h>
#include <vtkSivicController.h>


/*! 
 *  Constructor
 */
sivicTestCase::sivicTestCase()
{
}

/*! 
 *  Destructor
 */
sivicTestCase::~sivicTestCase()
{
    
}


/*
 *
 */
void sivicTestCase::SetSivicController( vtkSivicController* sivicController ) 
{
    this->sivicController = sivicController;
}
