/*
 *  Copyright © 2009-2017 The Regents of the University of California.
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
#include <svkImageWriterFactory.h>
#include <svkUtils.h>
#include <svkTypeUtils.h>
#include <svkVizUtils.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
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
void LoadOverlay( vector<string> overlayFileNames, bool isContour = false );
void LoadSpectra( vector<string> spectraFileNames );
bool InitializeIndexArray( vector<int>& indexArray, int size );
void UpdateSpectraSliceAnnotation( );

vtkCornerAnnotation* GetNewAnnotation( );
static void KeypressCallback( vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);
static void SelectionCallback(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);
static void RenderRefreshCallback(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);

void CaptureWindows();

// We are going to use this struct to hold our global vars. This makes it clearer which args are global.
struct globalVariables {
    vtkCornerAnnotation** annotations;
    vtkCornerAnnotation* spectraAnnotation;
    vector<svkImageData*> overlay;
    vector<svkImageData*> overlayContour;
    vector<string> overlayContourFileNames;
    vector<svkImageData*> spectra;
    vector<svkImageData*> referenceImage;
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
    bool setXRange;
    bool setYRange;
    float upperLimit;
    float lowerLimit;
    int beginPoint;
    int endPoint;
    vector<int> timePoint;
    vector<int> channel;
    vector<int> component;
    string justCapture;
    double threshold;
    bool thresholdSet;
    svkImageWriterFactory::WriterType dataTypeOut;
    vector<string> contourColors;
} globalVars;

// For getopt
static const char *optString = "s:o:w:hdu:l:b:e:t:c:p:j:i:r:x:n:";


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
    cout << "                   [-l lowerBound ] [-u upperBound ] [-b beginPoint] [-e endPoint]" << endl;
    cout << "                   [-c channel ] [-t timepoint ] [-p component] [-j captureRoot]" << endl << endl;
    cout << "                   -d debug       Turn on debug messages." << endl;
    cout << "                   -s spectra     A spectra file to load. This can be used multiple times to overlay traces." << endl;
    cout << "                   -o overlay     An overlay image to load. This can be used multiple times." << endl;
    cout << "                   -n overlay     An overlay image to load. Rendered as contours. This can be used multiple times." << endl;
    cout << "                   -l lowerBound  Minimum for Y axis of traces." << endl;
    cout << "                   -u upperBound  Maximum for Y axis of traces." << endl;
    cout << "                   -b beginPoint  First point (index starting at 1) of traces to display." << endl;
    cout << "                   -e endPoint    Final point (index starting at 1) of traces to display." << endl;
    cout << "                   -c channel     Channel (index starting at 1) of traces to display." << endl;
    cout << "                   -t timepoint   Timepoint (index starting at 1) of traces to display." << endl;
    cout << "                   -p component   Component of traces to display. 0=real (default), 1=imag, 2=mag" << endl;
    cout << "                   -i slice       Slice (index starting at 1) to display. Refers to image slice if present." << endl;
    cout << "                   -j captureRoot Just load the data and take screen captures. Results saved with root captureRoot." << endl;
    cout << "                   -r threshold   Overlay threshold by percentage of window (0-1)." << endl;
    cout << "                   -x type        Output data format:" << endl;
    cout << "                                         0 = JPEG" << endl;
    cout << "                                         1 = TIFF" << endl;
    cout << "                    --colors      Comma separated ordered list of colors used for contours." << endl;
    cout << "DESCRIPTION" << endl;
    cout << "    svk_multi_view is a quick way of seeing an arbitrary number of images synced by slice number." << endl;
    cout << "    If multiple overlays are specified then the first will appear on the spectra, the next on the first image etc." << endl;
    cout << "    If a single overlay is specified then all windows will use the same overlay." << endl << endl;
    cout << "    Pressing either +/- or the arrow keys will change the slice. Pressing the p key will save a screen capture." << endl;
    cout << "    Pressing the 'o' key will toggle the visibilty of overlays." << endl << endl;
    cout << "VERSION" << endl;
    cout << "     " << SVK_RELEASE_VERSION << endl; 
    cout << endl << "############  USAGE  ############ " << endl << endl;
    /* ... */
    exit( EXIT_FAILURE );
}


int main ( int argc, char** argv )
{
    globalVars.setXRange = false;
    globalVars.setYRange = false;
    globalVars.upperLimit = 0;
    globalVars.lowerLimit = 0;
    globalVars.beginPoint = 0;
    globalVars.endPoint = 0;
    globalVars.justCapture = "";
    globalVars.spectraWindow = NULL;
    globalVars.winSize = 350; 
    globalVars.debug = false;
    globalVars.slice = -1;
    globalVars.thresholdSet = false;
    globalVars.threshold = 0;
    globalVars.model = svkDataModel::New();
    vector<string> spectraFileNames;
    vector<string> overlayFileNames;
    /* This varibale will change to a value of 1 if there is a spectra loaded */
    globalVars.imageWindowOffset = 0;
    globalVars.spectraController = NULL;
    globalVars.orientation = svkDcmHeader::UNKNOWN_ORIENTATION;
    bool startSliceSet = false;
    globalVars.dataTypeOut = svkImageWriterFactory::TIFF;

    enum FLAG_NAME {
        COLORS
    };

    static struct option long_options[] =
    {
            {"colors",   required_argument, NULL,  COLORS},
            {0, 0, 0, 0}
    };


    int i;
    int option_index = 0;
    while ((i = getopt_long(argc, argv, optString, long_options, &option_index)) != EOF) {
        switch( i ) {
            case 'd':
                globalVars.debug = true;
                break;
            case 'w':
                globalVars.winSize =  atoi(optarg);
                break;
            case 'u':
                globalVars.upperLimit =  svkTypeUtils::StringToFloat(optarg);
                globalVars.setYRange = true;
                break;
            case 'l':
                globalVars.lowerLimit =  svkTypeUtils::StringToFloat(optarg);
                globalVars.setYRange = true;
                break;
            case 'b':
                globalVars.beginPoint =  atoi(optarg)-1;
                globalVars.setXRange = true;
                break;
            case 'e':
                globalVars.endPoint =  atoi(optarg)-1;
                globalVars.setXRange = true;
                break;
            case 't':
                globalVars.timePoint.push_back(atoi(optarg)-1);
                break;
            case 'c':
                globalVars.channel.push_back(atoi(optarg)-1);
                break;
            case 'p':
                globalVars.component.push_back(atoi(optarg));
                break;
            case 'i':
                globalVars.slice = atoi(optarg)-1;
                startSliceSet = true;
                break;
            case 'j':
                globalVars.justCapture = optarg;
                break;
            case 'r':
                globalVars.threshold = svkTypeUtils::StringToFloat(optarg);
                globalVars.thresholdSet = true;
                break;
            case 'h':
                Usage();
                break;
            case 's':
                spectraFileNames.push_back( optarg );
                break;
            case 'o':
                overlayFileNames.push_back( optarg );
                break;
            case 'n':
                globalVars.overlayContourFileNames.push_back( optarg );
                break;
            case 'x':
                globalVars.dataTypeOut = static_cast<svkImageWriterFactory::WriterType>( atoi(optarg) );
                break;
            case COLORS:
                globalVars.contourColors = svkUtils::SplitString(optarg, ",");
                break;
            default:
                cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
                cout<< endl <<" ERROR: Unrecognized option... " << endl << endl;
                cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
                Usage();
                break;
        }
    }

    if( globalVars.debug ) {
        for( int i = 0; i < spectraFileNames.size(); i++ ) {
            cout << "spectraFileNames: " << spectraFileNames[i] << endl;
        }
    }

    globalVars.numberOfImages = argc - optind;
    if( argc < 2 ) {
        Usage();
    } else if( globalVars.numberOfImages > 9 || (!spectraFileNames.empty() && globalVars.numberOfImages > 8 ) ) {
        Usage();
    }  

    // Initialize the index arrays for component, channel, and timepoint
    if( InitializeIndexArray(globalVars.component, spectraFileNames.size() ) == false ) {
        cout << endl <<  "ERROR! You must either specify 1 component, OR 1 component per trace!" << endl;
        Usage();
    }
    if( InitializeIndexArray(globalVars.channel, spectraFileNames.size() ) == false ) {
        cout << endl <<  "ERROR! You must either specify 1 channel, OR 1 channel per trace!" << endl;
        Usage();
    }
    if( InitializeIndexArray(globalVars.timePoint, spectraFileNames.size() ) == false ) {
        cout << endl <<  "ERROR! You must either specify 1 timepoint, OR 1 timepoint per trace!" << endl;
        Usage();
    }


    globalVars.viewers = new svkDataViewController*[ globalVars.numberOfImages ];
    globalVars.annotations = new vtkCornerAnnotation*[ globalVars.numberOfImages ];
    vtkRenderWindow** renderWindows = new vtkRenderWindow*[ globalVars.numberOfImages ]; 

    LoadOverlay( overlayFileNames );
    bool isContour = true;
    LoadOverlay( globalVars.overlayContourFileNames, isContour );

    LoadSpectra( spectraFileNames );

    int index; // Relative index

    for( int i= optind; i < argc; i++ ) {
        int index = i-optind;
        globalVars.annotations[index] = GetNewAnnotation();
        renderWindows[index] = vtkRenderWindow::New(); 
        globalVars.viewers[index] = svkOverlayViewController::New();
        if( index > 0 ) {
            globalVars.viewers[index]->GetView()->GetRenderer(0)->SetActiveCamera( globalVars.viewers[0]->GetView()->GetRenderer(0)->GetActiveCamera( ) );
        }
        if( globalVars.debug ) {
            cout <<"Loading image: " << argv[ i ] << endl;
        }
        int xPos = ((index+globalVars.imageWindowOffset)%3)*(globalVars.winSize+15);
        int yPos = ((index+globalVars.imageWindowOffset)/3)*(globalVars.winSize+20);
        DisplayImage( renderWindows[index], argv[i], index, xPos, yPos );    
        if( globalVars.spectraController != NULL ) {
            if( !startSliceSet ) {
                globalVars.viewers[i-optind]->SetSlice(globalVars.spectraController->GetSlice() );
                globalVars.slice = svkOverlayViewController::SafeDownCast(globalVars.viewers[i-optind])->GetImageSlice();
            }
        }
    }

    if( globalVars.numberOfImages != 0 && globalVars.spectraController != NULL ) {
        globalVars.spectraController->SetSlice( globalVars.viewers[0]->GetView()->GetSlice());
    }
    // Start the interactor.
    if( globalVars.justCapture.size() > 0 ) {
        CaptureWindows();
    } else {
        if( globalVars.spectraWindow != NULL ) {
            globalVars.spectraWindow->GetInteractor()->Start();
        } else if ( globalVars.numberOfImages > 0 ) {
            renderWindows[0]->GetInteractor()->Start();
        }
    }
    if( globalVars.spectraController != NULL ) {
        globalVars.spectraController->Delete();
        globalVars.spectraWindow->Delete();
    }
    for( int i= optind; i < argc; i++ ) {
        index = i-optind;
        globalVars.annotations[index]->Delete();
        globalVars.viewers[index]->Delete();
        // We have to remove all the renderes to avoid on open gl error.
        for( int j = 0; j < renderWindows[index]->GetRenderers()->GetNumberOfItems(); j++) {
            vtkRenderer::SafeDownCast(renderWindows[index]->GetRenderers()->GetItemAsObject(j))->RemoveAllViewProps();
        }
        renderWindows[index]->Delete();
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
    stringstream contourText;
    svkImageData* data = globalVars.model->LoadFile( filename, readOnlyOneFile );
    if( data == NULL ) {
        cerr << "ERROR: Could not read input file: " << filename << endl;
        exit(1);
    }
    data->Register(NULL);
    globalVars.referenceImage.push_back(data);
    //data->Update();
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
    } else {
        svkDataValidator* validator = svkDataValidator::New();
        if( !validator->AreDataGeometriesSame( globalVars.referenceImage[0], data) ) {
            cout << "ERROR! Data geometries do not match, all images must have the same geometry!" << endl;
            validator->Delete();
            exit(1);
        }
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

    // Determine what the correct overlay id is. If only one overlay is specified use it for all windows.
    int overlayId = 0;
    if( globalVars.overlay.size() > 1 ) {
        if( !globalVars.spectra.empty() ) {
            // If we have spectra and multiple overlays, each window gets a different overlay starting with the spectra.
            overlayId = id + 1;
        } else {
            overlayId = id;
        }
    }

    if( globalVars.overlay.size() > overlayId) {
        dataViewer->SetInput( globalVars.overlay[overlayId], 2  );
        if( globalVars.thresholdSet ) {
            svkOverlayViewController::SafeDownCast(dataViewer)->SetOverlayThreshold( globalVars.threshold );
        }
    }
    if( globalVars.spectra.size() > 0 ) {
        dataViewer->SetInput( globalVars.spectra[0], 1  );
    }
    if( globalVars.overlayContour.size() > 0 ) {
        for( int i = 0; i < globalVars.overlayContour.size(); i++) {
            dataViewer->SetInput(globalVars.overlayContour[i], svkOverlayView::OVERLAY_CONTOUR);
            if( globalVars.contourColors.size() > i) {
                svkOverlayContourDirector::ContourColor color;
                string colorString = globalVars.contourColors[i];
                if( colorString.compare("GREEN") == 0) {
                    color = svkOverlayContourDirector::GREEN;
                } else if ( colorString.compare("RED") == 0) {
                    color = svkOverlayContourDirector::RED;
                } else if ( colorString.compare("BLUE") == 0) {
                    color = svkOverlayContourDirector::BLUE;
                } else if ( colorString.compare("PINK") == 0) {
                    color = svkOverlayContourDirector::PINK;
                } else if ( colorString.compare("YELLOW") == 0) {
                    color = svkOverlayContourDirector::YELLOW;
                } else if ( colorString.compare("CYAN") == 0) {
                    color = svkOverlayContourDirector::CYAN;
                } else if ( colorString.compare("ORANGE") == 0) {
                    color = svkOverlayContourDirector::ORANGE;
                } else if ( colorString.compare("GRAY") == 0) {
                    color = svkOverlayContourDirector::GRAY;
                } else {
                    cout << "ERROR: Unrecognized color \"" << colorString << "\". Please choose from: ";
                    cout << "GREEN,RED,BLUE,PINK,YELLOW,CYAN,ORANGE,GRAY" << endl;
                    exit(1);
                }
                contourText << endl << colorString << ":" << svkUtils::GetFilenameFromFullPath(globalVars.overlayContourFileNames[i]);
                svkOverlayView::SafeDownCast(dataViewer->GetView())->SetContourColor(i, color);
            }
        }
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
    vtkCallbackCommand* renderRefreshCallbackCommand = vtkCallbackCommand::New();
    renderRefreshCallbackCommand->SetCallback( RenderRefreshCallback );
    renderRefreshCallbackCommand->SetClientData( (void*)dataViewer );
    rwi->AddObserver(vtkCommand::SelectionChangedEvent, selectionCallbackCommand );
    rwi->AddObserver(vtkCommand::RenderEvent, renderRefreshCallbackCommand );
    if( globalVars.slice == -1 ){
        globalVars.slice = (extent[5]-extent[4])/2;
    }
    svkOverlayViewController::SafeDownCast(dataViewer)->SetSlice( globalVars.slice, globalVars.orientation );
    if( globalVars.spectraController != NULL ) {
        globalVars.spectraController->SetSlice( dataViewer->GetSlice() );
        UpdateSpectraSliceAnnotation();
    }
    stringstream text;
    text<< "USE + AND - TO CHANGE SLICE";

    globalVars.annotations[id]->SetText(0, text.str().c_str() ); 
    text.str("");
    text<< "SLICE: " << globalVars.slice + 1 << "/" << globalVars.numberOfSlices;
    globalVars.annotations[id]->SetText(1, text.str().c_str() ); 

    if( globalVars.justCapture == "") {
        string contourTextString = contourText.str();
        stringstream fileAnnotationText;
        fileAnnotationText << filename;
        fileAnnotationText << contourText.str().c_str();
        string fileAnnotationTextString = fileAnnotationText.str().c_str();
        globalVars.annotations[id]->SetText(2, fileAnnotationTextString.c_str() );
    }

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
    vtkCallbackCommand* keypressCallbackCommand = vtkCallbackCommand::New();
    keypressCallbackCommand->SetCallback( KeypressCallback );
    keypressCallbackCommand->SetClientData( (void*)globalVars.viewers );
    rwi->AddObserver(vtkCommand::KeyPressEvent, keypressCallbackCommand );
    keypressCallbackCommand->Delete();

    globalVars.spectraController->SetRWInteractor( rwi );

    globalVars.spectraWindow->SetSize(globalVars.winSize,globalVars.winSize);
    globalVars.spectraWindow->SetWindowName( "svk_multi_view" );
    globalVars.spectraWindow->SetPosition(0,0);

    // Lets add a text actor with some info in it.
    for( int i = 0; i < globalVars.spectra.size(); i++ ) {
        if( i == 0 ) {
            globalVars.spectraController->SetInput( globalVars.spectra[i], 0  );
        } else {
            svkPlotGridView::SafeDownCast(globalVars.spectraController->GetView())->AddReferenceInput( globalVars.spectra[i] );
        }
        svkPlotGridView::SafeDownCast(globalVars.spectraController->GetView())->SetComponent( (svkPlotLine::PlotComponent)globalVars.component[i], i );
        svkPlotGridView::SafeDownCast(globalVars.spectraController->GetView())->SetVolumeIndex( globalVars.channel[i], svkMrsImageData::CHANNEL, i );
        svkPlotGridView::SafeDownCast(globalVars.spectraController->GetView())->SetVolumeIndex( globalVars.timePoint[i], svkMrsImageData::TIMEPOINT, i );
    }
    if( globalVars.overlay.size() > 0 ) {
        globalVars.spectraController->SetInput( globalVars.overlay[0], 1  );
    }

    globalVars.spectraController->GetView()->SetOrientation( globalVars.spectra[0]->GetDcmHeader()->GetOrientationType() );
    if( globalVars.setXRange ) {
        globalVars.spectraController->SetWindowLevelRange( globalVars.beginPoint , globalVars.endPoint , svkPlotGridView::FREQUENCY);
    }
    if( globalVars.setYRange ) {
        globalVars.spectraController->SetWindowLevelRange( globalVars.lowerLimit , globalVars.upperLimit , svkPlotGridView::AMPLITUDE);
    }

    vtkCallbackCommand* selectionCallbackCommand = vtkCallbackCommand::New();
    selectionCallbackCommand->SetCallback( SelectionCallback );
    selectionCallbackCommand->SetClientData( (void*)globalVars.spectraController );
    rwi->AddObserver(vtkCommand::SelectionChangedEvent, selectionCallbackCommand );
    int* extent = globalVars.spectra[0]->GetExtent();
    int specSlice = (extent[5]-extent[4])/2;
    if( globalVars.numberOfImages == 0 ){
        if( globalVars.slice == -1  ){
            globalVars.slice = specSlice;
        } else {
            specSlice = globalVars.slice;
        }
    }

    globalVars.spectraController->SetSlice( specSlice );

    globalVars.spectraAnnotation = GetNewAnnotation();
    stringstream text;
    text<< "USE + AND - TO CHANGE SLICE";
    globalVars.spectraAnnotation->SetText(0, text.str().c_str() );
    UpdateSpectraSliceAnnotation();
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
    if( keySymbol.compare("p") == 0  ) {
        CaptureWindows();
    } else if (  keySymbol.compare("o") == 0 ){
        for( int i= 0; i < globalVars.numberOfImages; i++ ) {
            double currentOpacity = ((static_cast<svkOverlayViewController**>(thisObject))[i])->GetOverlayOpacity();
            double newOpacity = currentOpacity == 0?0.35:0;
            globalVars.annotations[i]->SetText(1, globalVars.annotations[i]->GetText(1));
            ((static_cast<svkOverlayViewController**>(thisObject))[i])->SetOverlayOpacity(newOpacity);
            ((static_cast<svkDataViewController**>(thisObject))[i])->GetView()->Refresh();
        }
    } else {
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
               ((static_cast<svkOverlayViewController**>(thisObject))[i])->SetSlice(globalVars.slice, globalVars.orientation);
               ((static_cast<svkDataViewController**>(thisObject))[i])->GetView()->Refresh();
            }
            if( globalVars.spectraController!=NULL) {
                if( dvController == NULL ) {
                    globalVars.spectraController->SetSlice(  newSlice );
                } else {
                    globalVars.spectraController->SetSlice(  dvController->GetSlice() );
                }
                UpdateSpectraSliceAnnotation();
                globalVars.spectraController->GetView()->Refresh();
            }
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


/*
 *   Catches render events. Ensures all windows are refreshed.
 */
void RenderRefreshCallback(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata) {
    if (globalVars.debug) {
        cout << "Refreshing... " << endl;
    }
    svkOverlayViewController* activeController = static_cast<svkOverlayViewController *>(thisObject);
    if( activeController != NULL ) {
        vtkRenderer* activeRenderer = static_cast<svkOverlayViewController *>(thisObject)->GetView()->GetRenderer(svkOverlayView::PRIMARY);
        for (int i = 0; i < globalVars.numberOfImages; i++) {
            if (globalVars.viewers[i] != NULL) {
                vtkRenderer*renderer = static_cast<svkOverlayViewController *>(globalVars.viewers[i])->GetView()->GetRenderer(svkOverlayView::PRIMARY);
                if( activeRenderer->GetMTime() > renderer->GetMTime() ) {
                    (static_cast<svkOverlayViewController *>(globalVars.viewers[i]))->GetView()->Refresh();
                }
            }
        }

    }
}


/*!
 *  Takes a screencapture of each window.
 */
void CaptureWindows()
{
    string extension = "tiff"; 
    if (globalVars.dataTypeOut == svkImageWriterFactory::TIFF) {
        extension = "tiff"; 
    } else if (globalVars.dataTypeOut == svkImageWriterFactory::JPEG) {
        extension = "jpeg"; 
    }

    if(globalVars.spectraWindow != NULL ) {
        stringstream filename;
        filename << globalVars.justCapture;
        filename << "_traces." << extension;
        globalVars.spectraController->GetView()->Refresh();
        globalVars.spectraWindow->Render();
        svkVizUtils::SaveWindow( globalVars.spectraWindow, filename.str());
    }
    for( int i= 0; i < globalVars.numberOfImages; i++ ) {
        globalVars.viewers[i]->GetView()->Refresh();
        globalVars.viewers[i]->GetRWInteractor()->GetRenderWindow()->Render();
        stringstream filename;
        filename << globalVars.justCapture;
        filename << "_image_" << i << "." << extension; 
        svkVizUtils::SaveWindow(globalVars.viewers[i]->GetRWInteractor()->GetRenderWindow(), filename.str());
    }
}


/*!
 *  Loads the overlay that will be used on top of all other windows.
 *  NOTE: The overlay on the spectra requires the extent to match and
 *        you must include a spectra file when loading an overlay.
 */
void LoadOverlay( vector<string> overlayFileNames, bool isContour ) {

    svkImageData* overlayImage = NULL;
    for( int i = 0; i < overlayFileNames.size(); i++ ) {
        if( isContour ) {
            globalVars.overlayContour.push_back(globalVars.model->LoadFile(overlayFileNames[i] ));
            overlayImage = globalVars.overlayContour[i];
        } else {
            globalVars.overlay.push_back(globalVars.model->LoadFile(overlayFileNames[i] ));
            overlayImage = globalVars.overlay[i];
        }
        if(overlayImage == NULL ) {
            cerr << "ERROR: Could not read input file: " << overlayFileNames[i] << endl;
            exit(1);
        }
        overlayImage->Register(NULL);
        //globalVars.overlay->Update();
        if( !overlayImage->IsA("svkMriImageData")) {
            cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
            cout << "Error: -o and -n flag must be followed by an image file!" << endl;
            cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
            exit(1);
        }
        if( globalVars.debug ) {
            cout << "Overlay: " << *overlayImage;
            if( isContour ) {
                cout << " will be rendered as a contour.";
            }
            cout << endl;
        }

    }
}


/*!
 * Loads the spectra data set.
 */
void LoadSpectra( vector<string> spectraFileNames ) {
    bool loadedSpectra = false;
    for( int i = 0; i < spectraFileNames.size(); i++ ) {
        globalVars.spectra.push_back (globalVars.model->LoadFile( spectraFileNames[i] ));
        if( globalVars.spectra[i] == NULL ) {
            cerr << "ERROR: Could not read input file: " << spectraFileNames[i] << endl;
            exit(1);
        }
        if( globalVars.spectra[i] == NULL || !globalVars.spectra[i]->IsA("svkMrsImageData")) {
            cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
            cout << "Error: -s flag must be followed by a spectra file!" << endl;
            cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
        }
        globalVars.spectra[i]->Register(NULL);
        //globalVars.spectra[i]->Update();
        // If there is a spectra loaded the the first window will be the spectra 
        globalVars.imageWindowOffset = 1;
        if( globalVars.debug ) {
            cout << "Spectra: " << *globalVars.spectra[i] << endl;
        }
        if( i == 0 ) {
            globalVars.orientation=globalVars.spectra[i]->GetDcmHeader()->GetOrientationType();
            globalVars.numberOfSlices = globalVars.spectra[i]->GetNumberOfSlices();
        } else {
            svkDataValidator* validator = svkDataValidator::New();
            if( !validator->AreDataGeometriesSame( globalVars.spectra[0], globalVars.spectra[i]) ) {
                cout << "ERROR! Data geometries do not match, all spectra must have the same geometry!" << endl;
                validator->Delete();
                exit(1);
            }
        }
        loadedSpectra = true;
    }
    if( loadedSpectra ) {
        DisplaySpectra();
        // Remove file name when capturing image.
        if( globalVars.justCapture == "") {
            globalVars.spectraAnnotation->SetText(2, spectraFileNames[0].c_str() );
        }
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


/*!
 *  Creates default index arrays. If the array has length 0 then each index will be set to zero up
 *  to the length input by the 'size' argument. If the array has length 1 then the value at each
 *  index up to the length 'size' will have the same value of the first element. If the array
 *  already is length of the 'size' argument then nothing is done. Any other case and nothing is
 *  done but false is returned to indicate failure.
 */
bool InitializeIndexArray( vector<int>& indexArray, int size )
{
    if( indexArray.size() != size ) {
        int defaultValue = 0;
        if( indexArray.size() == 1 ) {
            defaultValue = indexArray[0];
        } else if ( indexArray.size() > 1 )  {
            return false;
        }
        for( int i = indexArray.size(); i < size; i++ ) {
            indexArray.push_back(defaultValue);
        }
    }
    return true;
 }


/*!
 * Updates the annotation on the spectra window.
 */
void UpdateSpectraSliceAnnotation( )
{
    if(globalVars.spectraController != NULL  ) {
        stringstream specText;
        specText<< "SLICE: " << globalVars.spectraController->GetSlice() + 1 << "/" << globalVars.spectra[0]->GetNumberOfSlices();
        globalVars.spectraAnnotation->SetText(1, specText.str().c_str() );
    }
}
