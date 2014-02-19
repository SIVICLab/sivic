/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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
/*!
 *   svk_multi_view is a quick way to view multiple data sets simultaneously.
 *   It allows the input of one spectra volume, one overlay volume, and an 
 *   arbirtrary number of addition image volumes (total number of volumes
 *   maxes out at 9). The user can request debugging messages, specific window
 *   sizes, or a report of the usage.
 */

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <svkOverlayViewController.h>
#include <svkPlotGridViewController.h>
#include <svkImageReader2.h>
#include <svkImageReaderFactory.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <unistd.h>
#endif
#include <string.h>
#include <svkDataModel.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkDataObjectTypes.h>
#include <vtkCornerAnnotation.h>

using namespace svk;

void Usage( );
void DisplayImage( vtkRenderWindow* window, const char* filename, int id,  int xPos, int yPos );
void DisplaySpectra( );
void LoadOverlay( string overlayFileName );
void LoadSpectra( string spectraFileName );
vtkCornerAnnotation* GetNewAnnotation( );
static void KeypressCallback( vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);
static void SelectionCallback(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);

// We are going to use this struct to hold our global vars. This makes it clearer which args are global.
struct globalVariables {
    vtkCornerAnnotation** annotations;
    vtkCornerAnnotation* spectraAnnotation;
    svkImageData* overlay; 
    svkImageData* spectra; 
    svkPlotGridViewController* spectraController; 
    svkDataViewController** viewers; 
    svkDataModel* model;
    vtkRenderWindow* spectraWindow;
    int slice;
    int numberOfSlices;
    int numberOfImages;
    bool debug;
    int winSize; 
    int imageWindowOffset; // This id is used to offset the image window when the spectra is present.
    svkDcmHeader::Orientation orientation;
} globalVars;

// For getopt
static const char *optString = "s:o:w:hd";


/*!
 *  Display usage message.
 */
void Usage( void )
{
    cout << endl << "############  USAGE  ############ " << endl << endl;
    cout << "NAME" << endl;
    cout << "    svk_multi_view" << endl << endl;
    cout << "SYNOPSIS" << endl;
    cout << "    svk_multi_view [-s spectra ] [-o overlay ] [-d] imageOne imageTwo imageThree ..." << endl << endl;
    cout << "DESCRIPTION" << endl;
    cout << "    svk_multi_view is a quick way of seeing an arbitrary number of images synced by slice number." << endl;
    cout << "    Pressing either +/- or the arrow keys will change the slice." << endl << endl;
    cout << "VERSION" << endl;
    cout << "     " << SVK_RELEASE_VERSION << endl; 
    cout << endl << "############  USAGE  ############ " << endl << endl;
    /* ... */
    exit( EXIT_FAILURE );
}


int main ( int argc, char** argv )
{
    globalVars.spectraWindow = NULL;
    globalVars.winSize = 350; 
    globalVars.debug = false;
    globalVars.slice = 0;
    globalVars.model = svkDataModel::New();
    string spectraFileName;
    string overlayFileName;
    /* This varibale will change to a value of 1 if there is a spectra loaded */
    globalVars.imageWindowOffset = 0;
    globalVars.spectraController = NULL;
    globalVars.orientation = svkDcmHeader::UNKNOWN_ORIENTATION;

    int opt = 0;
    opt = getopt( argc, argv, optString);
    while( opt != -1 ) {
        switch( opt ) {
            case 'd':
                globalVars.debug = true;
                break;
            case 'w':
                globalVars.winSize =  atoi(optarg);
                break;
            case 'h':
                Usage();
                break;
            case 's':
                spectraFileName.assign( optarg );
                break;
            case 'o':
                overlayFileName.assign( optarg );
                break;
            default:
                cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
                cout<< endl <<" ERROR: Unrecognized option... " << endl << endl;
                cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
                Usage();
                break;
        }
        opt = getopt( argc, argv, optString );
    }

    globalVars.numberOfImages = argc - optind;
    if( argc < 2 ) {
        Usage();
    } else if( globalVars.numberOfImages > 9 || (!spectraFileName.empty() && globalVars.numberOfImages > 8 ) ) {
        Usage();
    }  

    if( !overlayFileName.empty() && spectraFileName.empty() ) {
        cout << "Error: using the -o flag requires the input of a spectra file!" << endl;
        exit(1);
    }

    globalVars.viewers = new svkDataViewController*[ globalVars.numberOfImages ];
    globalVars.annotations = new vtkCornerAnnotation*[ globalVars.numberOfImages ];
    vtkRenderWindow** renderWindows = new vtkRenderWindow*[ globalVars.numberOfImages ]; 

    LoadOverlay( overlayFileName );

    LoadSpectra( spectraFileName );

    int index; // Relative index

    for( int i= optind; i < argc; i++ ) {
        int index = i-optind;
        globalVars.annotations[index] = GetNewAnnotation();
        renderWindows[index] = vtkRenderWindow::New(); 
        globalVars.viewers[index] = svkOverlayViewController::New();
        if( globalVars.debug ) {
            cout <<"Loading image: " << argv[ i ] << endl;
        }
        int xPos = ((index+globalVars.imageWindowOffset)%3)*(globalVars.winSize+15);
        int yPos = ((index+globalVars.imageWindowOffset)/3)*(globalVars.winSize+20);
        DisplayImage( renderWindows[index], argv[i], index, xPos, yPos );    
        if( globalVars.spectraController != NULL ) {
            globalVars.viewers[i-optind]->SetSlice(globalVars.spectraController->GetSlice() );
            globalVars.slice = svkOverlayViewController::SafeDownCast(globalVars.viewers[i-optind])->GetImageSlice();
        }
    }

    if( globalVars.numberOfImages != 0 && globalVars.spectraController != NULL ) {
        globalVars.spectraController->SetSlice( globalVars.viewers[0]->GetView()->GetSlice());
    }

    // Start the interactor.
    if( globalVars.spectraWindow != NULL ) {
        globalVars.spectraWindow->GetInteractor()->Start();
    } else if ( globalVars.numberOfImages > 0 ) {
        renderWindows[0]->GetInteractor()->Start();
    }

    for( int i= optind; i < argc; i++ ) {
        index = i-optind;
        globalVars.annotations[index]->Delete();
        renderWindows[index]->Delete();
        globalVars.viewers[index]->Delete();
    }

    if( globalVars.model != NULL ) {
        globalVars.model->Delete();
    }
    return 0;
  
}


/*!
 * Display an image in the given window.
 */
void DisplayImage( vtkRenderWindow* window, const char* filename, int id,  int xPos, int yPos )
{

    bool readOnlyOneFile = true; 
    svkImageData* data = globalVars.model->LoadFile( filename, readOnlyOneFile );
    if( data == NULL ) {
        cerr << "ERROR: Could not read input file: " << filename << endl;
        exit(1);
    }
    data->Register(NULL);
    data->Update();
    if( globalVars.debug ) {
        cout << "Data: " << *data << endl;
    }
    
    if( !data->IsA("svkMriImageData")) {
        cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
        cout << "ERROR: All inputs must be images!                " << endl;
        cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
        exit(0);
    }
    if( id == 0 ) {
        globalVars.numberOfSlices = data->GetNumberOfSlices();
    } else if (globalVars.numberOfSlices != data->GetNumberOfSlices()){
        cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
        cout << "ERROR: All inputs must have the same number of slices!  " << endl;
        cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
        exit(0);
    }
    int* extent = data->GetExtent();
    svkDataViewController* dataViewer = globalVars.viewers[id];
    
    vtkRenderWindowInteractor* rwi = window->MakeRenderWindowInteractor();

    dataViewer->SetRWInteractor( rwi );

    window->SetWindowName( "svk_multi_view" );
    window->SetPosition(xPos,yPos);
    window->SetSize(globalVars.winSize,globalVars.winSize);

    window->GetRenderers()->GetFirstRenderer()->DrawOff( );

    // Lets add a text actor with some info in it.
    dataViewer->SetInput( data, 0  );
    if( globalVars.spectra != NULL ) {
        dataViewer->SetInput( globalVars.spectra, 1  );
    }
    if( globalVars.overlay != NULL ) {
        dataViewer->SetInput( globalVars.overlay, 2  );
    }
    if( globalVars.orientation == svkDcmHeader::UNKNOWN_ORIENTATION ) { 
        globalVars.orientation = data->GetDcmHeader()->GetOrientationType();
    } else if ( globalVars.orientation != data->GetDcmHeader()->GetOrientationType() ) {
        cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
        cout << "        ALL INPUTS MUST HAVE THE SAME ORIENTATION!!!       " << endl;
        cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
        exit(0);
    }
    dataViewer->GetView()->SetOrientation( globalVars.orientation );
    svkOverlayView::SafeDownCast( dataViewer->GetView())->AlignCamera( );
    svkOverlayViewController::SafeDownCast(dataViewer)->UseWindowLevelStyle();

    vtkCallbackCommand* keypressCallbackCommand = vtkCallbackCommand::New();
    keypressCallbackCommand->SetCallback( KeypressCallback );
    keypressCallbackCommand->SetClientData( (void*)globalVars.viewers );
    rwi->AddObserver(vtkCommand::KeyPressEvent, keypressCallbackCommand );
    vtkCallbackCommand* selectionCallbackCommand = vtkCallbackCommand::New();
    selectionCallbackCommand->SetCallback( SelectionCallback );
    selectionCallbackCommand->SetClientData( (void*)dataViewer );
    rwi->AddObserver(vtkCommand::SelectionChangedEvent, selectionCallbackCommand );
    globalVars.slice = (extent[5]-extent[4])/2;
    svkOverlayViewController::SafeDownCast(dataViewer)->SetSlice( globalVars.slice, svkDcmHeader::AXIAL );
    stringstream text;
    text<< "USE + AND - TO CHANGE SLICE";

    globalVars.annotations[id]->SetText(0, text.str().c_str() ); 
    text.str("");
    text<< "SLICE: " << globalVars.slice + 1 << "/" << globalVars.numberOfSlices;
    globalVars.annotations[id]->SetText(1, text.str().c_str() ); 
    globalVars.annotations[id]->SetText(2, filename ); 

    window->GetRenderers()->GetFirstRenderer()->AddViewProp( globalVars.annotations[id] );

    window->GetRenderers()->GetFirstRenderer()->DrawOn( );
    dataViewer->GetView()->Refresh();
    window->Render();

    if( data != NULL ) {
        data->Delete();    
    }
}


/*!
 * Display an image in the given window.
 */
void DisplaySpectra( )
{
    globalVars.spectraWindow = vtkRenderWindow::New();

    globalVars.spectraController = svkPlotGridViewController::New();
    
    vtkRenderWindowInteractor* rwi = globalVars.spectraWindow->MakeRenderWindowInteractor();

    globalVars.spectraController->SetRWInteractor( rwi );

    globalVars.spectraWindow->SetSize(globalVars.winSize,globalVars.winSize);
    globalVars.spectraWindow->SetWindowName( "svk_multi_view" );
    globalVars.spectraWindow->SetPosition(0,0);

    // Lets add a text actor with some info in it.
    globalVars.spectraController->SetInput( globalVars.spectra, 0  );
    if( globalVars.overlay != NULL ) {
        globalVars.spectraController->SetInput( globalVars.overlay, 1  );
    }

    globalVars.spectraController->GetView()->SetOrientation( globalVars.spectra->GetDcmHeader()->GetOrientationType() );

    vtkCallbackCommand* selectionCallbackCommand = vtkCallbackCommand::New();
    selectionCallbackCommand->SetCallback( SelectionCallback );
    selectionCallbackCommand->SetClientData( (void*)globalVars.spectraController );
    rwi->AddObserver(vtkCommand::SelectionChangedEvent, selectionCallbackCommand );
    int* extent = globalVars.spectra->GetExtent();
    globalVars.slice = (extent[5]-extent[4])/2;
    globalVars.spectraController->SetSlice( globalVars.slice );

    globalVars.spectraAnnotation = GetNewAnnotation();
    stringstream text;
    text<< "USE + AND - TO CHANGE SLICE";
    globalVars.spectraAnnotation->SetText(0, text.str().c_str() );
    text.str("");
    text<< "SLICE: " << globalVars.slice + 1 << "/" << globalVars.spectra->GetNumberOfSlices();
    globalVars.spectraAnnotation->SetText(1, text.str().c_str() );
    globalVars.spectraWindow->GetRenderers()->GetFirstRenderer()->AddViewProp( globalVars.spectraAnnotation );

    globalVars.spectraController->GetView()->Refresh();
    globalVars.spectraWindow->Render();

}




/* 
 *   Catches keypresses. Currently changes slice use - and +.
 */
void KeypressCallback(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata)
{
    svkDataViewController* dvController = (static_cast<svkDataViewController**>(thisObject))[0];
    stringstream text;
    stringstream specText;
    char keyPressed;
    int newSlice = -1;
    vtkRenderWindowInteractor *rwi =
            vtkRenderWindowInteractor::SafeDownCast( subject );
    keyPressed = rwi->GetKeyCode();

    string keySymbol;
    if( rwi->GetKeySym() != NULL ) {
    	keySymbol = rwi->GetKeySym();
    }

    if( keySymbol.compare("Right") == 0 || keySymbol.compare("Up") == 0 || keyPressed == '+'  ) {
        newSlice = globalVars.slice+1;
    } else if( keySymbol.compare("Left") == 0 || keySymbol.compare("Down") == 0 || keyPressed == '-' ) {
        newSlice = globalVars.slice-1;
    }

    if( newSlice >= 0 && newSlice < globalVars.numberOfSlices ) {
        globalVars.slice = newSlice;
        text<< "SLICE: " << globalVars.slice + 1 << "/" << globalVars.numberOfSlices;
        for( int i= 0; i < globalVars.numberOfImages; i++ ) {
            globalVars.annotations[i]->SetText(1, text.str().c_str() ); 
           ((static_cast<svkOverlayViewController**>(thisObject))[i])->SetSlice(globalVars.slice, svkDcmHeader::AXIAL);
           ((static_cast<svkDataViewController**>(thisObject))[i])->GetView()->Refresh();
        }
        if( globalVars.spectraController!=NULL) {
            globalVars.spectraController->SetSlice(  dvController->GetSlice() );
            specText<< "SLICE: " << dvController->GetSlice() + 1 << "/" << globalVars.spectra->GetNumberOfSlices();
            globalVars.spectraAnnotation->SetText(1, specText.str().c_str() ); 
            globalVars.spectraController->GetView()->Refresh();
        }
    }
}


/* 
 *   Catches keypresses. Currently changes slice use - and +.
 */
void SelectionCallback(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata)
{
    if( globalVars.debug ) {
        cout << "New tlc brc: " << globalVars.spectraController->GetTlcBrc() << endl;
    }
    int* tlcBrc = NULL;
    if( static_cast<vtkObject*>(thisObject)->IsA( "svkOverlayViewController" ) ) {
        tlcBrc = static_cast<svkOverlayViewController*>(thisObject)->GetTlcBrc();
    } else if ( static_cast<vtkObject*>(thisObject)->IsA( "svkPlotGridViewController" ) ) {
        tlcBrc = static_cast<svkPlotGridViewController*>(thisObject)->GetTlcBrc();
    }
    for( int i= 0; i < globalVars.numberOfImages; i++ ) {
       (static_cast<svkOverlayViewController*>(globalVars.viewers[i]))->SetTlcBrc(tlcBrc);
       (static_cast<svkOverlayViewController*>(globalVars.viewers[i]))->GetView()->Refresh();
    }
    if( globalVars.spectraController != NULL ) {
        globalVars.spectraController->SetTlcBrc( tlcBrc );
    }
}


/*!
 *  Loads the overlay that will be used on top of all other windows.
 *  NOTE: The overlay on the spectra requires the extent to match and
 *        you must include a spectra file when loading an overlay.
 */
void LoadOverlay( string overlayFileName ) {
    if( !overlayFileName.empty() ) {
        globalVars.overlay = globalVars.model->LoadFile( overlayFileName );
        if( globalVars.overlay == NULL ) {
            cerr << "ERROR: Could not read input file: " << overlayFileName << endl;
            exit(1);
        }
        globalVars.overlay->Register(NULL);
        globalVars.overlay->Update();
        if( !globalVars.overlay->IsA("svkMriImageData")) {
            cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
            cout << "Error: -o flag must be followed by an image file!" << endl;
            cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
            exit(1);
        }
        if( globalVars.debug ) {
            cout << "Overlay: " << *globalVars.overlay << endl;
        }

    }
}


/*!
 * Loads the spectra data set.
 */
void LoadSpectra( string spectraFileName ) {
    if( !spectraFileName.empty() ) {
        globalVars.spectra = globalVars.model->LoadFile( spectraFileName );
        if( globalVars.spectra == NULL ) {
            cerr << "ERROR: Could not read input file: " << spectraFileName << endl;
            exit(1);
        }
        if( globalVars.spectra == NULL || !globalVars.spectra->IsA("svkMrsImageData")) {
            cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
            cout << "Error: -s flag must be followed by a spectra file!" << endl;
            cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
        }
        globalVars.spectra->Register(NULL);
        globalVars.spectra->Update();
        // If there is a spectra loaded the the first window will be the spectra 
        globalVars.imageWindowOffset = 1;
        if( globalVars.debug ) {
            cout << "Spectra: " << *globalVars.spectra << endl;
        }
        globalVars.orientation=globalVars.spectra->GetDcmHeader()->GetOrientationType();
        DisplaySpectra();
        globalVars.spectraAnnotation->SetText(2, spectraFileName.c_str() );


    }
}


/*!
 *  Sets up a new corner annotation object.
 */
vtkCornerAnnotation* GetNewAnnotation( ) {
    vtkCornerAnnotation* annotation = vtkCornerAnnotation::New();
    annotation->GetTextProperty()->SetColor(1,0,1);
    annotation->GetTextProperty()->BoldOn();
    annotation->GetTextProperty()->SetFontSize(20);
    return annotation;
}
