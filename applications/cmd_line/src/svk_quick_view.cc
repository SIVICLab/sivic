/*
 *  Copyright © 2009-2012 The Regents of the University of California.
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

void QuickView(const char* filename );
void DisplayUsage( );
static void KeypressCallback(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);

struct globalArgs_t {
} globalArgs;

static const char *optString = "h";

vtkCornerAnnotation* annotation;
svkImageData* data; 
int slice;


int main ( int argc, char** argv )
{
    int opt = 0;
    annotation = vtkCornerAnnotation::New();
    annotation->GetTextProperty()->SetColor(1,0,1);
    annotation->GetTextProperty()->BoldOn();
    annotation->GetTextProperty()->SetFontSize(20);
    data = NULL; 
    slice = 0;

    opt = getopt( argc, argv, optString);
    while( opt != -1 ) {
        switch( opt ) {
            case 'h':
                DisplayUsage();
                break;
            default:
                cout<< endl <<" ERROR: Unrecognized option... " << endl << endl;
                DisplayUsage();
            break;
        }
        opt = getopt( argc, argv, optString );
    }
    if( argc < 2 ) {
        DisplayUsage();
    } else { 
        QuickView( argv[1] );    
    }

    if( annotation != NULL ) {
        annotation->Delete();
        annotation = NULL;
    }
    return 0;
  
}


void QuickView( const char* filename)
{

    svkDataModel* model = svkDataModel::New();
    bool readOnlyOneFile = true; 
    data = model->LoadFile(filename, readOnlyOneFile );
    if( data == NULL ) {
        cerr << "ERROR: Could not read input file: " << filename << endl;
        exit(1);
    }
    data->Register(NULL);
    data->Update();
    int* extent = data->GetExtent();
    svkDataViewController* dataViewer;
    if( data->IsA("svkMriImageData")) {
        dataViewer = svkOverlayViewController::New(); 
    } else if( data->IsA("svkMrsImageData") ) {
        dataViewer = svkPlotGridViewController::New(); 
    }
    
    vtkRenderWindow* window = vtkRenderWindow::New(); 
    vtkRenderWindowInteractor* rwi = window->MakeRenderWindowInteractor();

    dataViewer->SetRWInteractor( rwi );
    vtkCallbackCommand* keypressCallbackCommand = vtkCallbackCommand::New();
    keypressCallbackCommand->SetCallback( KeypressCallback );
    keypressCallbackCommand->SetClientData( (void*)dataViewer );
    rwi->AddObserver(vtkCommand::KeyPressEvent, keypressCallbackCommand );

    window->SetSize(500,500);
    window->SetWindowName( "svk_quick_view" );
    window->GetRenderers()->GetFirstRenderer()->DrawOff( );

    // Lets add a text actor with some info in it.
    dataViewer->SetInput( data, 0  );

    dataViewer->GetView()->SetOrientation( data->GetDcmHeader()->GetOrientationType() );
    if( data->IsA("svkMriImageData")) {
        svkOverlayView::SafeDownCast( dataViewer->GetView())->AlignCamera( );
        svkOverlayViewController::SafeDownCast(dataViewer)->UseWindowLevelStyle();
    }

    slice = (extent[5]-extent[4])/2;
    dataViewer->SetSlice( slice );
    stringstream text;
    text<< "USE + AND - TO CHANGE SLICE";

    annotation->SetText(0, text.str().c_str() ); 
    text.str("");
    text<< "SLICE: " << slice + 1 << "/" << data->GetDcmHeader()->GetNumberOfSlices();
    annotation->SetText(1, text.str().c_str() ); 
    annotation->SetText(2, filename ); 

    window->GetRenderers()->GetFirstRenderer()->AddViewProp( annotation );

    window->GetRenderers()->GetFirstRenderer()->DrawOn( );
    dataViewer->GetView()->Refresh();
    window->Render();
    rwi->Start();

    if( data != NULL ) {
        data->Delete();    
    }
    model->Delete();
    dataViewer->Delete();
    window->Delete();
}

void DisplayUsage( void )
{
    cout << endl << "############  USAGE  ############ " << endl << endl;
    cout << "NAME" << endl;
    cout << "    svk_quick_view" << endl << endl;
    cout << "SYNOPSIS" << endl;
    cout << "    svk_quick_view fileName" << endl << endl;
    cout << "DESCRIPTION" << endl;
    cout << "    svk_quick_view is used to give a quick way to load a volume file. Pressing + and - change the slice." << endl << endl;
    cout << "VERSION" << endl;
    cout << "     " << SVK_RELEASE_VERSION << endl; 
    cout << endl << "############  USAGE  ############ " << endl << endl;
    /* ... */
    exit( EXIT_FAILURE );
}


/* 
 *   Catches keypresses. Currently changes slice use - and +.
 */
void KeypressCallback(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata)
{
    svkDataViewController* dvController = static_cast<svkDataViewController*>(thisObject);
    stringstream text;
    char keyPressed;
    int newSlice = -1;
    vtkRenderWindowInteractor *rwi =
            vtkRenderWindowInteractor::SafeDownCast( subject );
    keyPressed = rwi->GetKeyCode();
            
    if ( keyPressed == '+' ) {
        newSlice = slice+1;
    } else if ( keyPressed == '-' ) {
        newSlice = slice-1;
    }

    if( newSlice >= 0 && newSlice < data->GetDcmHeader()->GetNumberOfSlices() ) {
        slice = newSlice;
        text<< "SLICE: " << slice + 1 << "/" << data->GetDcmHeader()->GetNumberOfSlices();
        annotation->SetText(1, text.str().c_str() ); 
        dvController->SetSlice( slice );
        dvController->GetView()->Refresh();
    }
}

