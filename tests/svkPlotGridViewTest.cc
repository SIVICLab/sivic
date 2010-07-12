/*
 *  Copyright © 2009-2010 The Regents of the University of California.
 *  All Rights Reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  •   Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *  •   Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *  •   None of the names of any campus of the University of California, the name
 *      "The Regents of the University of California," or the names of any of its
 *      contributors may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 *  OF SUCH DAMAGE.
 */


/*
 *  $URL$ 
 *  $Rev$
 *  $Author$
 *  $Date$
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkWindowToImageFilter.h>
#include <vtkTIFFWriter.h>
#include <vtkInteractorStyleTrackballCamera.h>
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
#include <svkTestUtils.h>

using namespace svk;
    
void DefaultTest( );
void MemoryTest( );
void RenderingTest( );
void DisplayUsage( );

struct globalArgs_t {
    char *firstSpectraName;    /* -s option */
    char *secondSpectraName;   /* -S option */
    char *firstOverlayName;    /* -o option */
    char *secondOverlayName;   /* -O option */
    char *outputPath;          /* -p option */
} globalArgs;

static const struct option longOpts[] = {
    { "test_name",       required_argument, NULL, 't' },
    { "spectra",       required_argument, NULL, 's' },
    { "second_spectra", required_argument, NULL, 'S' },
    { "overlay",        required_argument, NULL, 'o' },
    { "second_overlay",  required_argument, NULL, 'O' },
    { "output_path",  required_argument, NULL, 'p' },
    { NULL,             no_argument,       NULL,  0  }
};
static const char *optString = "t:s:S:o:O:p:";


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
                    } else if( strcmp( optarg, "RenderingTest" ) == 0 ) {
                        testFunction = RenderingTest;
                        cout<<" Executing Rendering Test... "<<endl;
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
                case 'p': 
                    globalArgs.outputPath = optarg;
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
    plotController->SetSlice(0);
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


// Checks common operations. Saves images to disk.
void RenderingTest()
{   
    if( globalArgs.firstSpectraName  == NULL ||
        globalArgs.secondSpectraName != NULL ||
        globalArgs.secondOverlayName != NULL || 
        globalArgs.outputPath        == NULL ) {
        cout << endl << " ERROR: ";
        cout <<" Only one spectra and one metabolite files and an output path " 
             <<" must be specified to run the rendering test! " << endl << endl; 
        DisplayUsage();
    }

    string rootName = "";
    string spectraRoot = string( globalArgs.firstSpectraName );
    size_t ext = spectraRoot.find_last_of(".");
    size_t path = spectraRoot.find_last_of("/");
    rootName = spectraRoot.substr(path+1,ext-path-1);
   

    svkDataModel* model = svkDataModel::New();

    svkImageData* firstSpectra = model->LoadFile( globalArgs.firstSpectraName );
    firstSpectra->Register(NULL);
    firstSpectra->Update();
    svkImageData* firstOverlay = NULL;
    if( globalArgs.firstOverlayName != NULL ) {
        string overlayRoot = string( globalArgs.firstOverlayName );
        size_t ext = overlayRoot.find_last_of(".");
        size_t path = spectraRoot.find_last_of("/");
        rootName += overlayRoot.substr(path+1,ext-path-1);
        firstOverlay =  model->LoadFile( globalArgs.firstOverlayName );
        firstOverlay->Register(NULL);
        firstOverlay->Update();
    }

    vtkRenderWindow* window = vtkRenderWindow::New(); 
    vtkRenderWindowInteractor* rwi = window->MakeRenderWindowInteractor();
    svkPlotGridViewController* plotController = svkPlotGridViewController::New();

    plotController->SetRWInteractor( rwi );

    plotController->SetInput( firstSpectra, 0  );
    if( firstOverlay != NULL ) {
        plotController->SetInput( firstOverlay, 1 );
    }
    window->SetSize( 640, 640 );
    window->Render();
    plotController->HighlightSelectionVoxels();
    window->Render();

    for( int i = 0; i < firstSpectra->GetNumberOfSlices(); i++ ) {
        stringstream filename;
        filename << globalArgs.outputPath << "/" << rootName.c_str() << "_" << i << ".tiff" ;

        plotController->SetSlice( i );
        window->Render();
        svkTestUtils::SaveWindow( window, (filename.str()).c_str() );
    }

    plotController->Delete();
    firstSpectra->Delete();
    firstOverlay->Delete();
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
    //rwi->SetInteractorStyle( vtkInteractorStyleTrackballCamera::New() );

    window->SetSize(600,600);

    plotController->SetInput( spectra  );
    plotController->HighlightSelectionVoxels();
    plotController->SetSlice(4);
    plotController->GetView()->Refresh();
    window->GetRenderers()->GetFirstRenderer()->DrawOn();
    window->GetRenderers()->GetFirstRenderer()->Render();
    //plotController->SetChannel(1);
    //plotController->SetWindowLevelRange(-1000,1000,1);
    //plotController->TurnPropOff( svkPlotGridView::PLOT_GRID );
    if( overlay != NULL ) {
        plotController->SetInput( overlay, 1 );
    }
    plotController->GetView()->SetOrientation( svkDcmHeader::AXIAL );
    for( int i = 0; i < 8; i++ ) {
        plotController->SetSlice(i);
        window->Render();
        rwi->Start();
    }
    plotController->GetView()->SetOrientation( svkDcmHeader::CORONAL );
    for( int i = 0; i < 12; i++ ) {
        plotController->SetSlice(i);
        window->Render();
        rwi->Start();
    }
    plotController->GetView()->SetOrientation( svkDcmHeader::SAGITTAL );
    for( int i = 0; i < 12; i++ ) {
        plotController->SetSlice(i);
        window->Render();
        rwi->Start();
    }
    

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
