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
#include <svkOverlayViewController.h>
#include <svkImageReader2.h>
#include <svkImageReaderFactory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <svkDataModel.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkDataObjectTypes.h>

using namespace svk;

void DefaultTest( );
void MemoryTest( );
void DisplayUsage( );

struct globalArgs_t {
    char *firstImageName;       /* -i  option */
    char *secondImageName;      /* -I option */
    char *firstSpectraName;    /* -s  option */
    char *secondSpectraName;   /* -S option */
    char *firstOverlayName;     /* -o  option */
    char *secondOverlayName;    /* -O option */
} globalArgs;

static const struct option longOpts[] = {
    { "test_name",       required_argument, NULL, 't' },
    { "image",           required_argument, NULL, 'i' },
    { "second_image",    required_argument, NULL, 'I' },
    { "spectra",         required_argument, NULL, 's' },
    { "second_spectra",  required_argument, NULL, 'S' },
    { "overlay",         required_argument, NULL, 'o' },
    { "second_overlay",  required_argument, NULL, 'O' },
    { NULL,              no_argument,       NULL,  0  }
};
static const char *optString = "t:i:I:s:S:o:O:";


int main ( int argc, char** argv )
{
    void (*testFunction)() = NULL;
    int opt = 0;
    int longIndex;
    /* Initialize globalArgs before we get to work. */
    globalArgs.firstImageName = NULL;       /* -i  option */
    globalArgs.secondImageName = NULL;      /* -I option */
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
                    break;
                case 'i':
                    globalArgs.firstImageName = optarg;
                    break;
                case 'I': 
                    globalArgs.secondImageName = optarg;
                    break;
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
        globalArgs.firstOverlayName   == NULL ||
        globalArgs.secondOverlayName  == NULL || 
        globalArgs.firstImageName     == NULL ||
        globalArgs.secondImageName    == NULL ) {
        cout << endl << " ERROR: ";
        cout <<" Two images, two spectra, and  two metabolite files must be specified to run the memory check! " << endl << endl; 
        DisplayUsage();
    }

    svkDataModel* model = svkDataModel::New();
    
    svkImageData* firstImage = model->LoadFile( globalArgs.firstImageName );
    firstImage->Register(NULL);
    svkImageData* secondImage = model->LoadFile( globalArgs.secondImageName );
    secondImage->Register(NULL);
    svkImageData* firstSpectra = model->LoadFile( globalArgs.firstSpectraName );
    firstSpectra->Register(NULL);
    svkImageData* secondSpectra = model->LoadFile( globalArgs.secondSpectraName );
    secondSpectra->Register(NULL);
    svkImageData* firstOverlay = model->LoadFile( globalArgs.firstOverlayName );
    firstOverlay->Register(NULL);
    svkImageData* secondOverlay = model->LoadFile( globalArgs.secondOverlayName );
    secondOverlay->Register(NULL);

    firstImage->Update();
    secondImage->Update();
    firstSpectra->Update();
    secondSpectra->Update();
    firstOverlay->Update();
    secondOverlay->Update();

    vtkRenderWindow* window = vtkRenderWindow::New(); 
    vtkRenderWindowInteractor* rwi = window->MakeRenderWindowInteractor();
    svkOverlayViewController* overlayController = svkOverlayViewController::New();

    overlayController->SetRWInteractor( rwi );

    overlayController->SetInput( firstImage, 0  );
    overlayController->SetInput( firstSpectra, 1 );
    overlayController->SetInput( firstOverlay, 2 );
    overlayController->SetSlice(1);
    overlayController->HighlightSelectionVoxels();
    window->Render();
    int tlcbrc[2] = {1,14};
    overlayController->SetSlice( 0 );
    window->Render();
    overlayController->SetInput( secondImage, 0 );
    window->Render();
    overlayController->SetInput( secondSpectra, 1 );
    window->Render();
    overlayController->SetInput( secondOverlay, 2 );
    overlayController->SetTlcBrc( tlcbrc );
    window->Render();
    int* newTlcBrc = overlayController->GetTlcBrc();
    cout<<" new TlcBrc: "<<newTlcBrc[0]<<" "<<newTlcBrc[1]<<endl;
    overlayController->SetSlice( 4 );
    window->Render();
    overlayController->SetRWInteractor(rwi);
    window->Render();
    overlayController->UseWindowLevelStyle();
    window->Render();
    overlayController->UseSelectionStyle();
    window->Render();
    overlayController->UseRotationStyle();
    window->Render();
    overlayController->ResetWindowLevel();
    window->Render();
    overlayController->SetSlice( 0 );
    window->Render();
    tlcbrc[0] = 0;
    tlcbrc[1] = 143;
    overlayController->SetTlcBrc( tlcbrc );
    window->Render();

    overlayController->Delete();
    firstImage->Delete();
    secondImage->Delete();
    firstSpectra->Delete();
    secondSpectra->Delete();
    firstOverlay->Delete();
    secondOverlay->Delete();
    model->Delete();
    window->Delete();
     
}


void DefaultTest()
{
    int slice =4 ;
    if( globalArgs.firstImageName     == NULL  ) {
        DisplayUsage();
        cout << endl << " ERROR: ";
        cout << "At least an image must be specified to run this test! " << endl; 
    }

    svkDataModel* model = svkDataModel::New();
    

    svkImageData* spectra = NULL; 
    if( globalArgs.firstSpectraName != NULL ) {
        spectra = model->LoadFile( globalArgs.firstSpectraName );
        spectra->Register(NULL);
        spectra->Update();
    }

    svkImageData* image = NULL; 
    if( globalArgs.firstImageName != NULL ) {
        image = model->LoadFile( globalArgs.firstImageName );
        image->Register(NULL);
        image->Update();
    }

    svkImageData* overlay = NULL; 
    if( globalArgs.firstOverlayName != NULL ) {
        overlay = model->LoadFile( globalArgs.firstOverlayName );
        overlay->Register(NULL);
        overlay->Update();
    }

    
    vtkRenderWindow* window = vtkRenderWindow::New(); 
    vtkRenderWindowInteractor* rwi = window->MakeRenderWindowInteractor();
    svkOverlayViewController* overlayController = svkOverlayViewController::New();

    overlayController->SetRWInteractor( rwi );


    window->SetSize(600,600);

    if( spectra != NULL ) {
        overlayController->SetInput( spectra, 1  );
    }
    overlayController->SetInput( image, 0  );
    overlayController->SetSlice(slice);
    overlayController->HighlightSelectionVoxels();
    overlayController->ResetWindowLevel();
    if( overlay != NULL ) {
        overlayController->SetInput( overlay, 2 );
    }
    //static_cast<svkOverlayView*>(overlayController->GetView())->SetSlice( 128, 2 );
    //static_cast<svkOverlayView*>(overlayController->GetView())->SetSlice( 96, 1 );
    //overlayController->TurnPropOn( svkOverlayView::SAT_BANDS );
    //overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_OUTLINE );
    //overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_PERP1 );
    //overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_PERP1_OUTLINE );
    //overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_PERP2);
    //overlayController->TurnPropOn( svkOverlayView::SAT_BANDS_PERP2_OUTLINE );
    overlayController->UseWindowLevelStyle( );
    overlayController->UseRotationStyle();

    overlayController->GetView()->Refresh();
    window->Render();
    rwi->Start();

    image->Delete();    
    if( spectra != NULL ) {
        spectra->Delete();    
    }
    if( overlay != NULL ) {
        overlay->Delete();    
    }
    model->Delete();
    overlayController->Delete();
    window->Delete();
}

void DisplayUsage( void )
{
    cout << endl << "############  USAGE  ############ " << endl << endl;
    cout << "NAME" << endl;
    cout << "    test_svkOverlayView" << endl << endl;
    cout << "SYNOPSIS" << endl;
    cout << "    test_svkOverlayView -t testName [-i fileName] [-s fileName] [-o fileName] [-I fileName] [-S fileName] [-O fileName]" << endl << endl;
    cout << "        -t --test_name          Names the test you wish to run. See TESTS below for valid names. " << endl;
    cout << "        -i --image              An image file. " << endl;
    cout << "        -s --spectra           A spectroscopy file. " << endl;
    cout << "        -o --overlay            An overlay (metabolite) file. " << endl;
    cout << "        -I --second_image       A second image file.          [Used for MemoryTest.] " << endl;
    cout << "        -S --second_spectra    A second spectroscopy file.          [Used for MemoryTest.]" << endl;
    cout << "        -O --second_overlay     A second overlay (metabolite) file.  [Used for MemoryTest.]" << endl << endl;
    cout << "DESCRIPTION" << endl;
    cout << "    Testing harness for svkOverlayView. Test name is passed in as argument." << endl << endl;
    cout << "TESTS" << endl;
    cout << "    MemoryTest "<< endl;
    cout << "        Tests many methods of svkOverlayView to discovery memory leaks. " << endl;
    cout << "        Also will load multiple datasets to ensure no memory leaks when  " << endl;
    cout << "        switching between datasets. As such two images, two spectra, and" << endl;
    cout << "        two overlays (metabolites) are required to run the memory check. " << endl << endl;
    cout << "    DefaultTest "<< endl;
    cout << "        Loads each type of data and creates an interactive window. " << endl;

    cout << endl << "############  USAGE  ############ " << endl << endl;
    /* ... */
    exit( EXIT_FAILURE );
}
