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
 *
 */

#include <svkSpecPoint.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <svkDetailedPlotViewController.h>
#include <svkDetailedPlotView.h>
#include <svkImageReader2.h>
#include <svkImageReaderFactory.h>
#include <vtkCamera.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <svkDataModel.h>
#include <svkPhaseSpec.h>

using namespace svk;
    
void DefaultTest( );
void MemoryTest( );
void DisplayUsage( );

struct globalArgs_t {
    char *firstSpectraName;    /* -s  option */
    char *secondSpectraName;   /* -S option */
} globalArgs;

static const struct option longOpts[] = {
    { "test_name",       required_argument, NULL, 't' },
    { "spectra",       required_argument, NULL, 's' },
    { "second_spectra", required_argument, NULL, 'S' },
    { NULL,             no_argument,       NULL,  0  }
};
static const char *optString = "t:s:S:o:O:";

int main ( int argc, char** argv )
{
    void (*testFunction)() = DefaultTest;
    int opt = 0;
    int longIndex;
    /* Initialize globalArgs before we get to work. */
    globalArgs.firstSpectraName = NULL;    /* -s  option */
    globalArgs.secondSpectraName = NULL;   /* -S option */

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
                    } else {
                        testFunction = DefaultTest;
                        cout<<" Executing Default Test... "<<endl;
                    }
                case 's':
                    globalArgs.firstSpectraName = optarg;
                    break;
                case 'S': 
                    globalArgs.secondSpectraName = optarg;
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
        globalArgs.secondSpectraName == NULL ) {
        cout << endl << " ERROR: ";
        cout <<" Two spectra files must be specified to run the memory check! " << endl << endl; 
        DisplayUsage();
    }

    svkDataModel* model = svkDataModel::New();

    svkImageData* firstSpectra = model->LoadFile( globalArgs.firstSpectraName );
    firstSpectra->Register(NULL);
    svkImageData* secondSpectra = model->LoadFile( globalArgs.secondSpectraName );
    secondSpectra->Register(NULL);

    firstSpectra->Update();
    secondSpectra->Update();

    vtkRenderWindow* window = vtkRenderWindow::New(); 
    vtkRenderWindowInteractor* rwi = window->MakeRenderWindowInteractor();
    svkDetailedPlotViewController* plotController = svkDetailedPlotViewController::New();
    window->Render();

    
    plotController->Delete();
    firstSpectra->Delete();
    secondSpectra->Delete();
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
    svkPhaseSpec* phaser = svkPhaseSpec::New();
    

    svkImageData* spectra = model->LoadFile( globalArgs.firstSpectraName );
    spectra->Register( NULL );

    spectra->Update();
    vtkRenderWindow* window = vtkRenderWindow::New(); 
    vtkRenderWindowInteractor* rwi = window->MakeRenderWindowInteractor();
    svkDetailedPlotViewController* plotController = svkDetailedPlotViewController::New();

    plotController->SetRWInteractor( rwi );

    svkSpecPoint* point = svkSpecPoint::New();
    //point->SetDataHdr( spectra->GetDcmHeader() );

    window->SetSize(600,600);
    //plotController->SetUnits( svkSpecPoint::PPM);
    plotController->SetInput( spectra, 0  );
    //plotController->SetSlice(0);
    plotController->AddPlot( 655,0 );
    //plotController->AddPlot( 655,1 );
    //plotController->AddPlot( 656,0 );
    //plotController->AddPlot( 656,1 );
    //plotController->AddPlot( 657,0 );
    //plotController->AddPlot( 657,1 );
    plotController->SetWindowLevelRange( -60000000, 0, 1 );
    window->Render();
    rwi->Start();
    plotController->SetWindowLevelRange( -60000000, 90000000, 1 );
    window->Render();
    rwi->Start();

    plotController->SetWindowLevelRange( 0, 90000000, 1 );
    window->Render();
    rwi->Start();

    plotController->SetUnits( svkSpecPoint::PPM);
    plotController->SetWindowLevelRange( 0, 512, 0 );
    window->Render();
    rwi->Start();

    plotController->SetWindowLevelRange( 0, 200, 0 );
    window->Render();
    rwi->Start();

    plotController->SetWindowLevelRange( 0, 100, 0 );
    window->Render();
    rwi->Start();
    plotController->SetWindowLevelRange( 0, 512, 0 );

    plotController->SetUnits( svkSpecPoint::Hz );
    window->Render();
    rwi->Start();

    plotController->SetWindowLevelRange( 0, 200, 0 );
    window->Render();
    rwi->Start();
    plotController->SetWindowLevelRange( 0, 512, 0 );

    plotController->SetUnits( svkSpecPoint::PTS);
    window->Render();
    rwi->Start();

    plotController->SetWindowLevelRange( 0, 200, 0 );
    window->Render();
    rwi->Start();
/*
    static_cast<svkDetailedPlotView*>(plotController->GetView())->plotActor->GetXAxisActor2D()->SetRange( 0, 200 );
    double* range = static_cast<svkDetailedPlotView*>(plotController->GetView())->plotActor->GetXAxisActor2D()->GetRange( );
    cout << " New Range is : " << range[0] << " " << range[1] << endl;
    static_cast<svkDetailedPlotView*>(plotController->GetView())->plotActor->GetXAxisActor2D()->Modified();
    static_cast<svkDetailedPlotView*>(plotController->GetView())->plotActor->GetXAxisActor2D()->TitleVisibilityOff();
    static_cast<svkDetailedPlotView*>(plotController->GetView())->plotActor->Modified();
    static_cast<svkDetailedPlotView*>(plotController->GetView())->Refresh();
    window->Render();
    rwi->Start();
    */

    /*
    plotController->SetUnits( svkSpecPoint::PPM );
    plotController->Update( );
    float min = point->ConvertPosUnits(200, svkSpecPoint::PTS, svkSpecPoint::PPM );
    float max = point->ConvertPosUnits(0, svkSpecPoint::PTS, svkSpecPoint::PPM );
    cout<< " min is : " << min << " max is : " << max << endl;
   // plotController->SetWindowLevelRange( min, max, 0 );
    window->Render();
    rwi->Start();

    plotController->SetUnits( svkSpecPoint::Hz );
    min = point->ConvertPosUnits(200, svkSpecPoint::PTS, svkSpecPoint::Hz );
    max = point->ConvertPosUnits(0, svkSpecPoint::PTS, svkSpecPoint::Hz );
    //plotController->SetWindowLevelRange( min, max, 0 );
    cout<< " min is : " << min << " max is : " << max << endl;
    window->Render();
    rwi->Start();

    phaser->SetInput( spectra );
    for( int i = -180; i < 180; i++ ) {
        phaser->SetPhase0( i );
        phaser->Update();
        plotController->Update();
        window->Render();
        i+=6;
        
    }
    */

    spectra->Delete();    
    plotController->Delete();
    model->Delete();
    window->Delete();
}

void DisplayUsage( void )
{
    cout << endl << "############  USAGE  ############ " << endl << endl;
    cout << "NAME" << endl;
    cout << "    test_svkDetailedPlotView" << endl << endl;
    cout << "SYNOPSIS" << endl;
    cout << "    test_svkDetailedPlotView [-t testName] [-s filename] [-S filename] " << endl << endl;
    cout << "        -t --test_name          Names the test you wish to run. See TESTS below for vaild names. " << endl;
    cout << "        -s --spectra           A spectroscopy file. " << endl;
    cout << "        -S --second_spectra    A second spectroscopy file. " << endl;
    cout << "DESCRIPTION" << endl;
    cout << "    Testing harness for svkDetailedPlotView. " << endl << endl;
    cout << "TESTS" << endl;
    cout << "    MemoryTest "<<endl;
    cout << "        Tests many methods of svkDetailedPlotView to discovery memory leaks. " << endl;
    cout << "        Also will load multiple datasets to ensure no memory leaks when  " << endl;
    cout << "        switching between datasets. As such two spectra and two overlays " << endl;
    cout << "        (metabolites) are required to run the memory check. " << endl;
    cout << endl << "############  USAGE  ############ " << endl << endl;
    /* ... */
    exit( EXIT_FAILURE );
}
