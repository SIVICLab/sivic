/*
 *  $URL$ 
 *  $Rev$
 *  $Author$
 *  $Date$
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 *
 *  License: TBD
 *
 *
 *
 *  Test driver for the svkPlotGridView/Controller pair.
 *
 *  The following classes are utilized in this driver.
 *      svkPlotGridView
 *      svkPlotGridViewController
 *
 */

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <svkPlotGridViewController.h>
#include <svkPlotGridView.h>
#include <svkPlotGrid.h>
#include <svkImageReader2.h>
#include <svkImageReaderFactory.h>
#include <vtkCamera.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <svkDataModel.h>

using namespace svk;
    
void DefaultTest( );
void MemoryTest( );
void DisplayUsage( );

struct globalArgs_t {
    char *firstSpectraName;    /* -s  option */
    char *secondSpectraName;   /* -S option */
    char *firstOverlayName;     /* -o  option */
    char *secondOverlayName;    /* -O option */
} globalArgs;

static const struct option longOpts[] = {
    { "test_name",       required_argument, NULL, 't' },
    { "spectra",       required_argument, NULL, 's' },
    { "second_spectra", required_argument, NULL, 'S' },
    { "overlay",        required_argument, NULL, 'o' },
    { "second_overlay",  required_argument, NULL, 'O' },
    { NULL,             no_argument,       NULL,  0  }
};
static const char *optString = "t:s:S:o:O:";


int main ( int argc, char** argv )
{
    void (*testFunction)() = NULL;
    int opt = 0;
    int longIndex;
    /* Initialize globalArgs before we get to work. */
    globalArgs.firstSpectraName = NULL;    /* -s  option */
    globalArgs.secondSpectraName = NULL;   /* -S option */
    globalArgs.firstOverlayName = NULL;     /* -o  option */
    globalArgs.secondOverlayName = NULL;    /* -O option */
    testFunction = DefaultTest;

    opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
    if( opt == -1 ) {
        DisplayUsage();
    }
        while( opt != -1 ) {
            switch( opt ) {
                case 't':
                    if( strcmp( optarg, "MemoryTest" ) == 0 ) {
                        testFunction = MemoryTest;
                        cout<<" Executing Memory Check... "<<endl;
                    }
                case 's':
                    globalArgs.firstSpectraName = optarg;
                    break;
                case 'S': 
                    globalArgs.secondSpectraName = optarg;
                    break;
                case 'o':
                    globalArgs.firstOverlayName = optarg;
                    break;
                case 'O': 
                    globalArgs.secondOverlayName = optarg;
                    break;
                default:
                    cout<< endl <<" ERROR: Unrecognized option... " << endl << endl;
                    DisplayUsage();
                break;
            }
            opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
        }

    testFunction();

    return 0;
  
}

// Checks common operations
void MemoryTest()
{   
    if( globalArgs.firstSpectraName  == NULL ||
        globalArgs.secondSpectraName == NULL ||
        globalArgs.firstOverlayName  == NULL ||
        globalArgs.secondOverlayName == NULL ) {
        cout << endl << " ERROR: ";
        cout <<" Two spectra and two metabolite files must be specified to run the memory check! " << endl << endl; 
        DisplayUsage();
    }

    svkDataModel* model = svkDataModel::New();

    svkImageData* firstSpectra = model->LoadFile( globalArgs.firstSpectraName );
    firstSpectra->Register(NULL);
    svkImageData* secondSpectra = model->LoadFile( globalArgs.secondSpectraName );
    secondSpectra->Register(NULL);
    svkImageData* firstOverlay = model->LoadFile( globalArgs.firstOverlayName );
    firstOverlay->Register(NULL);
    svkImageData* secondOverlay = model->LoadFile( globalArgs.secondOverlayName );
    secondOverlay->Register(NULL);

    firstSpectra->Update();
    secondSpectra->Update();
    firstOverlay->Update();
    secondOverlay->Update();

    vtkRenderWindow* window = vtkRenderWindow::New(); 
    vtkRenderWindowInteractor* rwi = window->MakeRenderWindowInteractor();
    svkPlotGridViewController* plotController = svkPlotGridViewController::New();

    plotController->SetRWInteractor( rwi );

    plotController->SetInput( firstSpectra, 0  );
    plotController->SetSlice(1);
    plotController->HighlightSelectionVoxels();
    plotController->SetInput( firstOverlay, 1 );
    window->Render();
    
    
    int tlcbrc[2] = {1,14}; 
    window->Render();
    plotController->SetSlice( 0 );
    window->Render();
    plotController->SetWindowLevelRange( 300, 500, 0);
    window->Render();
    plotController->SetWindowLevelRange( -50000000, 500000000, 1);
    window->Render();
    plotController->SetTlcBrc( tlcbrc );
    // Check to see if the tlc brc was retained...
    int* newTlcBrc = plotController->GetTlcBrc();
    cout<<" new TlcBrc: "<<newTlcBrc[0]<<" "<<newTlcBrc[1]<<endl;
    assert( newTlcBrc[0] == tlcbrc[0] );
    assert( newTlcBrc[1] == tlcbrc[1] );
    window->Render();
    plotController->SetRWInteractor( rwi );
    window->Render();
    plotController->SetInput( secondSpectra  );
    plotController->SetInput( secondOverlay, 1 );
    window->Render();
    plotController->SetSlice( 1 );
    window->Render();
    

    // Check a selection highlight
    plotController->HighlightSelectionVoxels();
    newTlcBrc = plotController->GetTlcBrc();
    cout<<" new TlcBrc: "<<newTlcBrc[0]<<" "<<newTlcBrc[1]<<endl;
    assert( newTlcBrc[0] != tlcbrc[0] );
    assert( newTlcBrc[1] != tlcbrc[1] );
    window->Render();

    
    plotController->Delete();
    firstSpectra->Delete();
    secondSpectra->Delete();
    firstOverlay->Delete();
    secondOverlay->Delete();
    model->Delete();
    window->Delete();
     
}


void DefaultTest()
{
    if( globalArgs.firstSpectraName  == NULL ) {
        DisplayUsage();
        cout << endl << " ERROR: ";
        cout << "The first spectra and metabolite files must be specified to run the current test! " << endl; 
    }

    svkDataModel* model = svkDataModel::New();

    svkImageData* spectra = model->LoadFile( globalArgs.firstSpectraName );
    spectra->Register( NULL );
    spectra->Update();

    svkImageData* overlay = NULL; 
    if( globalArgs.firstOverlayName  != NULL ) { 
        overlay = model->LoadFile( globalArgs.firstOverlayName );
        overlay->Register( NULL );
        overlay->Update();
    }

    vtkRenderWindow* window = vtkRenderWindow::New(); 
    vtkRenderWindowInteractor* rwi = window->MakeRenderWindowInteractor();
    svkPlotGridViewController* plotController = svkPlotGridViewController::New();

    plotController->SetRWInteractor( rwi );

    window->SetSize(600,600);

    plotController->SetInput( spectra, 0  );
    plotController->SetSlice(0);
    plotController->HighlightSelectionVoxels();
    //plotController->SetChannel(1);
    plotController->SetWindowLevelRange(-100000000,100000000,1);
    if( overlay != NULL ) {
        plotController->SetInput( overlay, 1 );
    }
    window->Render();
    rwi->Start();

    spectra->Delete();    
    if( overlay != NULL ) {
        overlay->Delete();    
    }
    plotController->Delete();
    model->Delete();
    window->Delete();
}

void DisplayUsage( void )
{
    cout << endl << "############  USAGE  ############ " << endl << endl;
    cout << "NAME" << endl;
    cout << "    test_svkPlotGridView" << endl << endl;
    cout << "SYNOPSIS" << endl;
    cout << "    test_svkPlotGridView [-t testName] [-s filename] [-o filename] [-S filename] [-O filename]" << endl << endl;
    cout << "        -t --test_name          Names the test you wish to run. See TESTS below for vaild names. " << endl;
    cout << "        -s --spectra           A spectroscopy file. " << endl;
    cout << "        -S --second_spectra    A second spectroscopy file. " << endl;
    cout << "        -o --overlay            An overlay (metabolite) file. " << endl;
    cout << "        -O --second_overlay     A second overlay (metabolite) file. " << endl << endl;
    cout << "DESCRIPTION" << endl;
    cout << "    Testing harness for svkPlotGridView. " << endl << endl;
    cout << "TESTS" << endl;
    cout << "    MemoryTest "<<endl;
    cout << "        Tests many methods of svkPlotGridView to discovery memory leaks. " << endl;
    cout << "        Also will load multiple datasets to ensure no memory leaks when  " << endl;
    cout << "        switching between datasets. As such two spectra and two overlays " << endl;
    cout << "        (metabolites) are required to run the memory check. " << endl;
    cout << endl << "############  USAGE  ############ " << endl << endl;
    /* ... */
    exit( EXIT_FAILURE );
}
