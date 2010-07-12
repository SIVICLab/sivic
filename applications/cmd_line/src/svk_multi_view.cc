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
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/applications/cmd_line/src/svk_quick_view.cc $ 
 *  $Rev: 221 $
 *  $Author: beckn8tor $
 *  $Date: 2010-03-08 12:42:59 -0800 (Mon, 08 Mar 2010) $
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
#include <unistd.h>
#include <string.h>
#include <svkDataModel.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkDataObjectTypes.h>
#include <vtkCornerAnnotation.h>

using namespace svk;

void QuickView( vtkRenderWindow* window, svkDataModel* model, const char* filename, int id,  int xPos, int yPos );
void DisplayUsage( );
static void KeypressCallback(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);


struct globalArgs_t {
} globalArgs;

static const char *optString = "h";

vtkCornerAnnotation** annotations;
svkImageData** data; 
svkDataViewController** viewers; 
int slice;
int numberOfSlices;
int numberOfImages;


int main ( int argc, char** argv )
{
    int opt = 0;
    data = NULL; 
    slice = 0;
    svkDataModel* model = svkDataModel::New();

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
    } else if( argc > 10 ) {
        DisplayUsage();
    } else { 
/*
        if( data->IsA("svkMriImageData")) {
            dataViewer = svkOverlayViewController::New(); 
            dataViewer2 = svkOverlayViewController::New(); 
        } else if( data->IsA("svkMrsImageData") ) {
            dataViewer = svkPlotGridViewController::New(); 
            dataViewer2 = svkPlotGridViewController::New(); 
        }
        
*/
        numberOfImages = argc-1;
        viewers = new svkDataViewController*[ numberOfImages ];
        annotations = new vtkCornerAnnotation*[ numberOfImages ];
        vtkRenderWindow** windows = new vtkRenderWindow*[ numberOfImages ]; 
        for( int i= 0; i < numberOfImages; i++ ) {
            annotations[i] = vtkCornerAnnotation::New();
            annotations[i]->GetTextProperty()->SetColor(1,0,1);
            annotations[i]->GetTextProperty()->BoldOn();
            annotations[i]->GetTextProperty()->SetFontSize(20);
            windows[i] = vtkRenderWindow::New(); 
            viewers[i] = svkOverlayViewController::New();
            QuickView( windows[i], model, argv[ i+1 ], i, (i%3)*310 , (i/3)*320 );    
        }
        windows[0]->GetInteractor()->Start();
    }
/*
    if( annotation != NULL ) {
        annotation->Delete();
        annotation = NULL;
    }
*/
    return 0;
  
}


void QuickView( vtkRenderWindow* window, svkDataModel* model, const char* filename, int id,  int xPos, int yPos )
{

    svkImageData* data = model->LoadFile( filename );
    data->Register(NULL);
    data->Update();
    cout << "Data: " << *data << endl;
    if( !data->IsA("svkMriImageData")) {
        cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
        cout << "          ALL INPUTS MUST BE IMAGES!!!" << endl;
        cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
        DisplayUsage();
        exit(0);
    }
    numberOfSlices = data->GetNumberOfSlices();
    int* extent = data->GetExtent();
    svkDataViewController* dataViewer = viewers[id];
    
    vtkRenderWindowInteractor* rwi = window->MakeRenderWindowInteractor();

    dataViewer->SetRWInteractor( rwi );

    window->SetSize(300,300);
    window->SetWindowName( "svk_multi_view" );
    window->SetPosition(xPos,yPos);

    // Lets add a text actor with some info in it.
    dataViewer->SetInput( data, 0  );

    dataViewer->GetView()->SetOrientation( data->GetDcmHeader()->GetOrientationType() );
    svkOverlayView::SafeDownCast( dataViewer->GetView())->AlignCamera( );
    svkOverlayViewController::SafeDownCast(dataViewer)->UseWindowLevelStyle();

    vtkCallbackCommand* keypressCallbackCommand = vtkCallbackCommand::New();
    keypressCallbackCommand->SetCallback( KeypressCallback );
    keypressCallbackCommand->SetClientData( (void*)viewers );
    rwi->AddObserver(vtkCommand::KeyPressEvent, keypressCallbackCommand );
    slice = (extent[5]-extent[4])/2;
    dataViewer->SetSlice( slice );
    stringstream text;
    text<< "USE + AND - TO CHANGE SLICE";

    annotations[id]->SetText(0, text.str().c_str() ); 
    text.str("");
    text<< "SLICE: " << slice + 1 << "/" << numberOfSlices;
    annotations[id]->SetText(1, text.str().c_str() ); 
    annotations[id]->SetText(2, filename ); 

    window->GetRenderers()->GetFirstRenderer()->AddViewProp( annotations[id] );

    dataViewer->GetView()->Refresh();
    window->Render();

    if( data != NULL ) {
        data->Delete();    
    }
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
    svkDataViewController* dvController = (static_cast<svkDataViewController**>(thisObject))[0];
    svkDataViewController* dvController2 = (static_cast<svkDataViewController**>(thisObject))[1];
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

    if( newSlice >= 0 && newSlice < numberOfSlices ) {
        slice = newSlice;
        text<< "SLICE: " << slice + 1 << "/" << numberOfSlices;
        for( int i= 0; i < numberOfImages; i++ ) {
            annotations[i]->SetText(1, text.str().c_str() ); 
           ((static_cast<svkDataViewController**>(thisObject))[i])->SetSlice(slice);
           ((static_cast<svkDataViewController**>(thisObject))[i])->GetView()->Refresh();
        }
    }
}

