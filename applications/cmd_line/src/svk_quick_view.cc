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
#include <svkPlotGridViewController.h>
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
#include <vtkTextActor.h>

using namespace svk;

void QuickView(const char* filename );
void DisplayUsage( );
static void KeypressCallback(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);

struct globalArgs_t {
} globalArgs;

static const struct option longOpts[] = {
};
static const char *optString = "h";

vtkTextActor* infoActor;


int main ( int argc, char** argv )
{
    int opt = 0;
    int longIndex;

    opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
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
        opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
    }
    if( argc < 2 ) {
        DisplayUsage();
    } else { 
        QuickView( argv[1] );    
    }


    return 0;
  
}


void QuickView( const char* filename)
{

    svkDataModel* model = svkDataModel::New();
    
    svkImageData* data = NULL; 
    data = model->LoadFile(filename );
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

    // Lets add a text actor with some info in it.
    dataViewer->SetInput( data, 0  );
    dataViewer->SetSlice( (extent[5]-extent[4])/2 );
    infoActor = vtkTextActor::New();
    stringstream text;
    text<< "USE + AND - TO CHANGE SLICE. CURRENT SLICE:" << dataViewer->GetSlice() + 1;
    infoActor->SetInput( (text.str()).c_str() );
    infoActor->GetTextProperty()->SetFontSize(20);
    infoActor->GetTextProperty()->SetColor(1,0,1);
    infoActor->SetPosition( 15, 475 );
    window->GetRenderers()->GetFirstRenderer()->AddViewProp( infoActor );
    infoActor->Delete();

    // Lets add a text actor with the name of the loaded file
    vtkTextActor* fileName = vtkTextActor::New();
    fileName->SetInput( filename );
    fileName->GetTextProperty()->SetFontSize(20);
    fileName->GetTextProperty()->SetColor(1,0,1);
    fileName->SetPosition( 15, 15 );
    window->GetRenderers()->GetFirstRenderer()->AddViewProp( fileName );
    fileName->Delete();

    if( data->IsA("svkMriImageData")) {
        svkOverlayViewController::SafeDownCast(dataViewer)->UseWindowLevelStyle();
    }
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
    text<< "USE + AND - TO CHANGE SLICE. CURRENT SLICE:" << dvController->GetSlice() + 1;
    infoActor->SetInput( (text.str()).c_str() );
    char keyPressed;
    vtkRenderWindowInteractor *rwi =
            vtkRenderWindowInteractor::SafeDownCast( subject );
    keyPressed = rwi->GetKeyCode();
    if ( keyPressed == '+' ) {
        dvController->SetSlice( dvController->GetSlice() + 1 );
    } else if ( keyPressed == '-' ) {
        dvController->SetSlice( dvController->GetSlice() - 1 );
    }
    dvController->GetView()->Refresh();
}

