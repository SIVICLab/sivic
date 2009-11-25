/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */


#ifndef SIVIC_TEST_SUITE_H 
#define SIVIC_TEST_SUITE_H 

#include <svkOverlayViewController.h>
#include <vtkSivicController.h>
#include <sivicTestCase.h>
#include <sivicBasicTest.h>

#include <vtksys/SystemTools.hxx>
#include <vtksys/CommandLineArguments.hxx>

using namespace svk;

class vtkSivicController;

/*! 
 *  The purpose of this class is to test  
 *  many of the components of the application, sivic.
 */ 
class sivicTestSuite
{
    public:
        
        sivicTestSuite( vtkSivicController* sivicController );
        ~sivicTestSuite();
        void RunTests();

    private:
        vtkSivicController* sivicController;
        
};

#endif //SIVIC_TEST_SUITE_H 
