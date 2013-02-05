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

#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#include <unistd.h>
#endif

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <svkOverlayViewController.h>
#include <svkImageReader2.h>
#include <svkImageReaderFactory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <svkDataModel.h>
#include <vtkDataObjectTypes.h>
#include <svkTestUtils.h>

using namespace svk;

void MemoryTest( );
void RenderingTest( );
void OrientationTest( );
void ColorMapTest( );
void DisplayUsage( );

struct globalArgs_t {
    char *firstImageName;     /* -i  option */
    char *secondImageName;    /* -I option */
    char *firstSpectraName;   /* -s  option */
    char *secondSpectraName;  /* -S option */
    char *firstOverlayName;   /* -o  option */
    char *secondOverlayName;  /* -O option */
    char *outputPath;         /* -p option */
    bool disableValidation;  /* -d option */
    bool showSatBands;
} globalArgs;

static const struct option longOpts[] = {
    { "test_name",          required_argument, NULL, 't' },
    { "image",              required_argument, NULL, 'i' },
    { "second_image",       required_argument, NULL, 'I' },
    { "spectra",            required_argument, NULL, 's' },
    { "second_spectra",     required_argument, NULL, 'S' },
    { "overlay",            required_argument, NULL, 'o' },
    { "second_overlay",     required_argument, NULL, 'O' },
    { "output_path",        required_argument, NULL, 'p' },
    { "disable_validation", no_argument,       NULL, 'd' },
    { NULL,                 no_argument,       NULL,  0  }
};
static const char *optString = "t:i:I:s:S:o:O:p:d";


int main ( int argc, char** argv )
{
    void (*testFunction)() = NULL;
    int opt = 0;
    int longIndex;
    /* Initialize globalArgs before we get to work. */
    globalArgs.firstImageName = NULL;       /* -i  option */
    globalArgs.secondImageName = NULL;      /* -I option */
    globalArgs.firstSpectraName = NULL;     /* -s  option */
    globalArgs.secondSpectraName = NULL;    /* -S option */
    globalArgs.firstOverlayName = NULL;     /* -o  option */
    globalArgs.secondOverlayName = NULL;    /* -O option */
    globalArgs.outputPath = NULL;           /* -p option */
    globalArgs.disableValidation = false;
    globalArgs.showSatBands = false;
    testFunction = MemoryTest;

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
                    } else if( strcmp( optarg, "OrientationTest" ) == 0 ) {
                        testFunction = OrientationTest;
                        cout<<" Executing Orientation Test... "<<endl;
                    } else if( strcmp( optarg, "SatBandTest" ) == 0 ) {
                        testFunction = OrientationTest;
                        globalArgs.showSatBands = true;
                        cout<<" Executing Sat Band Test... "<<endl;
                    } else if( strcmp( optarg, "ColorMapTest" ) == 0 ) {
                        testFunction = ColorMapTest;
                        cout<<" Executing Color Map Test... "<<endl;
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
                case 'p': 
                    globalArgs.outputPath = optarg;
                    break;
                case 'd': 
                    globalArgs.disableValidation = true;
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

void OrientationTest()
{
    if( globalArgs.firstImageName == NULL  || 
        globalArgs.firstSpectraName == NULL  ) {
        DisplayUsage();
        cout << endl << " ERROR: ";
        cout << "At least an image and a spectra must be specified to run this test! " << endl; 
    }

    string rootName = "";
    string imageRoot = string( globalArgs.firstImageName );
    size_t ext = imageRoot.find_last_of(".");
    size_t path = imageRoot.find_last_of("/");
    rootName = imageRoot.substr(path+1,ext-path-1);


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


    if( globalArgs.disableValidation ) {
        overlayController->GetView()->ValidationOff();
    }
    window->GetRenderers()->GetFirstRenderer()->DrawOff( );
    overlayController->SetInput( image, 0  );

    if( spectra != NULL ) {
        overlayController->SetInput( spectra, 1  );
    }

    if( overlay != NULL ) {
        overlayController->SetInput( overlay, 2 );
    }
    window->GetRenderers()->GetFirstRenderer()->DrawOn( );
    
    
    overlayController->GetView()->SetOrientation( svkDcmHeader::AXIAL);
    if( globalArgs.showSatBands == true ) {
        overlayController->TurnPropOn(svkOverlayView::SAT_BANDS_AXIAL);
        overlayController->TurnPropOn(svkOverlayView::SAT_BANDS_AXIAL_OUTLINE);
        overlayController->TurnPropOn(svkOverlayView::SAT_BANDS_CORONAL);
        overlayController->TurnPropOn(svkOverlayView::SAT_BANDS_CORONAL_OUTLINE);
        overlayController->TurnPropOn(svkOverlayView::SAT_BANDS_SAGITTAL);
        overlayController->TurnPropOn(svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE);
    }
    svkOverlayView::SafeDownCast(overlayController->GetView())->AlignCamera();
    for( int i = 0; i < spectra->GetNumberOfSlices(svkDcmHeader::AXIAL); i++ ) {
        stringstream filename;
        filename << globalArgs.outputPath << "/" << rootName.c_str() << "_" << i << "AXIAL.tiff" ;
        overlayController->SetSlice( i );
        window->Render();
        svkTestUtils::SaveWindow( window, (filename.str()).c_str() );
    }

    overlayController->GetView()->SetOrientation( svkDcmHeader::CORONAL);
    svkOverlayView::SafeDownCast(overlayController->GetView())->AlignCamera();
    overlayController->GetView()->Refresh();
    overlayController->HighlightSelectionVoxels();
    for( int i = 0; i < spectra->GetNumberOfSlices(svkDcmHeader::CORONAL); i++ ) {
        stringstream filename;
        filename << globalArgs.outputPath << "/" << rootName.c_str() << "_" << i << "CORONAL.tiff" ;
        overlayController->SetSlice( i );
        window->Render();
        svkTestUtils::SaveWindow( window, (filename.str()).c_str() );
    }
    overlayController->GetView()->SetOrientation( svkDcmHeader::SAGITTAL);
    svkOverlayView::SafeDownCast(overlayController->GetView())->AlignCamera();
    overlayController->HighlightSelectionVoxels();
    for( int i = 0; i < spectra->GetNumberOfSlices(svkDcmHeader::SAGITTAL); i++ ) {
        stringstream filename;
        filename << globalArgs.outputPath << "/" << rootName.c_str() << "_" << i << "SAGITTAL.tiff" ;
        overlayController->SetSlice( i );
        window->Render();
        svkTestUtils::SaveWindow( window, (filename.str()).c_str() );
    }
    if( spectra != NULL ) {
        spectra->Delete();
    }
    if( overlay != NULL ) {
        overlay->Delete();
    }

    model->Delete();
    window->Delete();
    overlayController->Delete();
    image->Delete();
    
}


void ColorMapTest()
{
    if( globalArgs.firstImageName == NULL  || 
        globalArgs.firstSpectraName == NULL || 
        globalArgs.firstOverlayName   == NULL ) {
        DisplayUsage();
        cout << endl << " ERROR: ";
        cout << "At least an image, a spectra, and an overlay must be specified to run this test! " << endl; 
    }

    string rootName = "";
    string imageRoot = string( globalArgs.firstImageName );
    size_t ext = imageRoot.find_last_of(".");
    size_t path = imageRoot.find_last_of("/");
    rootName = imageRoot.substr(path+1,ext-path-1);


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


    if( globalArgs.disableValidation ) {
        overlayController->GetView()->ValidationOff();
    }
    window->GetRenderers()->GetFirstRenderer()->DrawOff( );
    overlayController->SetInput( image, 0  );

    if( spectra != NULL ) {
        overlayController->SetInput( spectra, 1  );
    }

    if( overlay != NULL ) {
        overlayController->SetInput( overlay, 2 );
    }
    window->GetRenderers()->GetFirstRenderer()->DrawOn( );
    
    
    overlayController->GetView()->SetOrientation( svkDcmHeader::AXIAL);
    overlayController->SetSlice( spectra->GetNumberOfSlices(svkDcmHeader::AXIAL)/2 );
    svkOverlayView::SafeDownCast(overlayController->GetView())->AlignCamera();
    overlayController->SetOverlayOpacity( 1 );
    overlayController->SetOverlayThreshold( 0 );
    window->Render();
    stringstream filename;
    filename << globalArgs.outputPath << "/" << rootName.c_str() << "_" << "COLOR.tiff" ;
    svkTestUtils::SaveWindow( window, (filename.str()).c_str() );

    overlayController->SetLUT( svkLookupTable::GREY_SCALE );
    window->Render();
    filename.str("");
    filename << globalArgs.outputPath << "/" << rootName.c_str() << "_" << "GREY_SCALE.tiff" ;
    svkTestUtils::SaveWindow( window, (filename.str()).c_str() );

    overlayController->SetLUT( svkLookupTable::HURD );
    window->Render();
    filename.str("");
    filename << globalArgs.outputPath << "/" << rootName.c_str() << "_" << "HURD.tiff" ;
    svkTestUtils::SaveWindow( window, (filename.str()).c_str() );

    overlayController->SetLUT( svkLookupTable::CYAN_HOT );
    window->Render();
    filename.str("");
    filename << globalArgs.outputPath << "/" << rootName.c_str() << "_" << "CYAN_HOT.tiff" ;
    svkTestUtils::SaveWindow( window, (filename.str()).c_str() );

    overlayController->SetLUT( svkLookupTable::FIRE );
    window->Render();
    filename.str("");
    filename << globalArgs.outputPath << "/" << rootName.c_str() << "_" << "FIRE.tiff" ;
    svkTestUtils::SaveWindow( window, (filename.str()).c_str() );

    overlayController->SetOverlayThreshold( 100 );
    overlayController->SetLUT( svkLookupTable::REVERSE_COLOR );
    window->Render();
    filename.str("");
    filename << globalArgs.outputPath << "/" << rootName.c_str() << "_" << "REVERSE_COLOR.tiff" ;
    svkTestUtils::SaveWindow( window, (filename.str()).c_str() );
    
    if( spectra != NULL ) {
        spectra->Delete();
    }
    if( overlay != NULL ) {
        overlay->Delete();
    }

    model->Delete();
    window->Delete();
    overlayController->Delete();
    image->Delete();
    
}

void RenderingTest()
{
    if( globalArgs.firstImageName == NULL  ) {
        DisplayUsage();
        cout << endl << " ERROR: ";
        cout << "At least an image must be specified to run this test! " << endl; 
    }

    string rootName = "";
    string imageRoot = string( globalArgs.firstImageName );
    size_t ext = imageRoot.find_last_of(".");
    size_t path = imageRoot.find_last_of("/");
    rootName = imageRoot.substr(path+1,ext-path-1);


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

    if( globalArgs.disableValidation ) {
        overlayController->GetView()->ValidationOff();
    }
    window->GetRenderers()->GetFirstRenderer()->DrawOff( );
    overlayController->SetInput( image, 0  );

    if( spectra != NULL ) {
        overlayController->SetInput( spectra, 1  );
    }

    overlayController->GetView()->SetOrientation( image->GetDcmHeader()->GetOrientationType());
    svkOverlayView::SafeDownCast(overlayController->GetView())->AlignCamera();

    if( overlay != NULL ) {
        overlayController->SetInput( overlay, 2 );
    }
    window->GetRenderers()->GetFirstRenderer()->DrawOn( );
    
    for( int i = 0; i < image->GetNumberOfSlices(); i++ ) {
        stringstream filename;
        filename << globalArgs.outputPath << "/" << rootName.c_str() << "_" << i << ".tiff" ;
        overlayController->SetSlice( i, image->GetDcmHeader()->GetOrientationType() );
        window->Render();
        svkTestUtils::SaveWindow( window, (filename.str()).c_str() );
    }
    if( spectra != NULL ) {
        spectra->Delete();
    }
    if( overlay != NULL ) {
        overlay->Delete();
    }

    model->Delete();
    window->Delete();
    overlayController->Delete();
    image->Delete();
    
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

    cout << endl << "############  USAGE  ############ " << endl << endl;
    /* ... */
    exit( EXIT_FAILURE );
}
