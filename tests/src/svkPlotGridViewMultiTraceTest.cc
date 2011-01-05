/*
 *  Copyright © 2009-2011 The Regents of the University of California.
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
void DisplayUsage( );

struct globalArgs_t {
    char *firstSpectraName;    /* -f option */
    char *secondSpectraName;   /* -s option */
    char *thirdSpectraName;   /* -t option */
    char *overlayName;    /* -o option */
} globalArgs;

static const struct option longOpts[] = {
    { "spectra",       required_argument, NULL, 'f' },
    { "second_spectra", required_argument, NULL, 's' },
    { "third_spectra", required_argument, NULL, 't' },
    { "overlay",        required_argument, NULL, 'o' },
    { NULL,             no_argument,       NULL,  0  }
};
static const char *optString = "f:s:t:o:";


int main ( int argc, char** argv )
{
    void (*testFunction)() = NULL;
    int opt = 0;
    int longIndex;
    /* Initialize globalArgs before we get to work. */
    globalArgs.firstSpectraName = NULL;    /* -s  option */
    globalArgs.secondSpectraName = NULL;   /* -S option */
    globalArgs.thirdSpectraName = NULL;   /* -p option */
    globalArgs.overlayName = NULL;     /* -o  option */
    testFunction = DefaultTest;

    opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
    if( opt == -1 ) {
        DisplayUsage();
    }
        while( opt != -1 ) {
            switch( opt ) {
                case 'f':
                    globalArgs.firstSpectraName = optarg;
                    break;
                case 's': 
                    globalArgs.secondSpectraName = optarg;
                    break;
                case 't': 
                    globalArgs.thirdSpectraName = optarg;
                    break;
                case 'o':
                    globalArgs.overlayName = optarg;
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

void DefaultTest()
{
    if( globalArgs.firstSpectraName  == NULL ) {
        DisplayUsage();
        cout << endl << " ERROR: ";
        cout << "Specify at least one spectra data set! " << endl; 
    }

    svkDataModel* model = svkDataModel::New();

    svkImageData* firstSpectra = NULL;
    if( globalArgs.firstSpectraName  != NULL ) {
        firstSpectra = model->LoadFile( globalArgs.firstSpectraName );
        firstSpectra->Register( NULL );
        firstSpectra->Update();
    } 
    svkImageData* secondSpectra = NULL;
    if( globalArgs.secondSpectraName  != NULL ) {
        secondSpectra = model->LoadFile( globalArgs.secondSpectraName );
        secondSpectra->Register( NULL );
        secondSpectra->Update();
    }
    svkImageData* thirdSpectra = NULL;
    if( globalArgs.thirdSpectraName  != NULL ) {
        thirdSpectra = model->LoadFile( globalArgs.thirdSpectraName );
        thirdSpectra->Register( NULL );
        thirdSpectra->Update();
    }

    svkImageData* overlay = NULL;
    if( globalArgs.overlayName  != NULL ) {
        overlay = model->LoadFile( globalArgs.overlayName );
        overlay->Register( NULL );
        overlay->Update();
    }


    vtkRenderWindow* window = vtkRenderWindow::New(); 
    vtkRenderWindowInteractor* rwi = window->MakeRenderWindowInteractor();
    svkPlotGridViewController* plotController = svkPlotGridViewController::New();

    plotController->SetRWInteractor( rwi );

    window->SetSize(600,600);

    plotController->SetInput( firstSpectra  );

    if( secondSpectra != NULL ) {
        plotController->SetInput( secondSpectra, 2  );
    }
    if( thirdSpectra != NULL ) {
        plotController->SetInput( thirdSpectra, 3  );
    }
    if( overlay != NULL ) {
        plotController->SetInput( overlay, 1 );
    }
    plotController->HighlightSelectionVoxels();
    plotController->GetView()->Refresh();
    window->GetRenderers()->GetFirstRenderer()->DrawOn();
    window->GetRenderers()->GetFirstRenderer()->Render();
    //plotController->SetWindowLevelRange( 100, 400, svkPlotGridView::FREQUENCY);
    //plotController->SetWindowLevelRange( -50000000, 500000000, svkPlotGridView::AMPLITUDE);
    int* extent = firstSpectra->GetExtent();
    plotController->SetSlice((extent[5]-extent[4])/2);
    plotController->GetView()->SetOrientation( svkDcmHeader::AXIAL);
    rwi->Start();
    firstSpectra->Delete();    
    if( secondSpectra != NULL ) {
        secondSpectra->Delete();    
    }
    if( thirdSpectra != NULL ) {
        thirdSpectra->Delete();    
    }
    plotController->Delete();
    if( overlay != NULL ) {
        overlay->Delete();    
    }
    model->Delete();
    window->Delete();
}


void DisplayUsage( void )
{
    cout << endl << "############  USAGE  ############ " << endl << endl;
    cout << "NAME" << endl;
    cout << "   svkPlotGridViewMultiTraceTest" << endl << endl;
    cout << "SYNOPSIS" << endl;
    cout << "    test_svkPlotGridView [-s filename] [-S filename] [-c filename] [-o filename] [-O filename]" << endl << endl;
    cout << "        -f --first_spectra     The first spectroscopy file. " << endl;
    cout << "        -s --second_spectra    The second spectroscopy file. " << endl;
    cout << "        -t --third_spectra     The third spectroscopy file. " << endl;
    cout << "        -o --overlay           An overlay (metabolite) file. " << endl;
    cout << "DESCRIPTION" << endl;
    cout << "    Testing harness for svkPlotGridView with multiple plots. " << endl << endl;
    cout << endl << "############  USAGE  ############ " << endl << endl;
    /* ... */
    exit( EXIT_FAILURE );
}
