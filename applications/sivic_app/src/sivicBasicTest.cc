/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */
#include <sivicBasicTest.h>
#include <vtkSivicController.h>


/*! 
 *  Constructor
 */
sivicBasicTest::sivicBasicTest( )
{

}


/*! 
 *  Destructor
 */
sivicBasicTest::~sivicBasicTest()
{
    
}

void sivicBasicTest::Run() 
{
    cout << "Basic test running " << endl;
    this->sivicController->PopupMessage( "TESTING IMAGE OPEN...." );
    this->sivicController->OpenImage( "/home/bolson/Projects/visualization/4svn/visualization/trunk/libs/testing/data/t3148_fla.idf" );
    this->sivicController->PopupMessage( "TESTING SPECTRA OPEN...." );
    this->sivicController->OpenSpectra( "/home/bolson/Projects/visualization/4svn/visualization/trunk/libs/testing/data/t3148_1_cor.ddf" );
    this->sivicController->PopupMessage( "TESTING METABOLTE OPEN...." );
    this->sivicController->OpenOverlay( "/home/bolson/Projects/visualization/4svn/visualization/trunk/libs/testing/data/t3148_1r01.idf" );
    this->sivicController->PopupMessage( "TESTING IMAGE OVERLAY OPEN...." );
    this->sivicController->OpenOverlay( "/home/bolson/Projects/visualization/4svn/visualization/trunk/libs/testing/data/t3148_adca.idf" );
    this->sivicController->PopupMessage( "TESTING SLICING...." );
    this->sivicController->SetSlice(0);
    this->sivicController->SetSlice(1);
    this->sivicController->SetSlice(2);
    this->sivicController->SetSlice(3);
    this->sivicController->SetSlice(4);
    this->sivicController->SetSlice(5);
    this->sivicController->SetSlice(6);
    this->sivicController->SetSlice(7);
    this->sivicController->PopupMessage( "TESTING SAVE AND RESTORE...." );
    this->sivicController->SaveSession();
    this->sivicController->ResetApplication();
    this->sivicController->RestoreSession();
    this->sivicController->PopupMessage( "TESTING RESET...." );
    this->sivicController->ResetApplication();

}
