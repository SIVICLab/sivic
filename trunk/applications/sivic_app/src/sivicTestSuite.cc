/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */
#include <sivicTestSuite.h>


/*! 
 *  Constructor
 */
sivicTestSuite::sivicTestSuite( vtkSivicController* sivicController)
{
    this->sivicController = sivicController;

}


/*! 
 *  Destructor
 */
sivicTestSuite::~sivicTestSuite()
{
    
}


/*!
 * Run the tests.
 */
void sivicTestSuite::RunTests()
{
    cout << "Running Tests..." << endl;
    sivicTestCase* testCase = new sivicBasicTest();
    testCase->SetSivicController( this->sivicController );
    testCase->Run();
    
}
