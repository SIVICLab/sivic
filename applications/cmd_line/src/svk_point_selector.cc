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
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/applications/cmd_line/src/svk_point_selector.cc $
 *  $Rev: 1252 $
 *  $Author: jccrane $
 *  $Date: 2012-03-22 15:13:33 -0700 (Thu, 22 Mar 2012) $
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
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
#include <vtkDataObjectTypes.h>
#include <vtkCornerAnnotation.h>
#include <vtkSphereSource.h>
#include <vtkPoints.h>
#include <vtkDoubleArray.h>
#include <vtkLineSource.h>
#include <vtkTextProperty.h>
#include <vtkGlyph3D.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <svkImageViewer2.h>
#include <svkImageMapToColors.h>
#include <svkLookupTable.h>
#include <svkDataValidator.h>
#include <svkObliqueReslice.h>

using namespace svk;

void SetupImageViewer(double position[4], svkDcmHeader::Orientation orientation, vtkActor* sphere);
void SetSlice( int slice, svkDcmHeader::Orientation orientation ) ;
void SetSlices( double coordinates[3] ) ;
void SetupCursor();
void UpdateCursor( double location[3]);
static void UpdateCursorLocation(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);
static void KeypressCallback(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);
void DisplayUsage( );

struct globalArgs_t {
} globalArgs;

static const char *optString = "hs:o:i:L:P:S:";

struct globalVariables {
	string                               imageFilename;
	string                               overlayFilename;
	string                               screenshotFilename;
	svkImageData*                        data;
	svkImageData*                        overlayData;
	vtkImageData*                        screenshot;
	vtkDoubleArray*                      cursorPosition;
	vtkPoints*                           cursorPoint;
	vector<vtkCornerAnnotation*>         annotations;
	vector<svkImageMapToColors*>         colorMappers;
	vector<svkImageViewer2*>             imageViewers;
	vector<svkOpenGLOrientedImageActor*> overlayActors;
	vector<vtkRenderer*>                 renderers;
	vector<vtkTransform*>                transformers;
	vtkSphereSource*                     sphere;
	svkLookupTable*                      colorTransfer;
} globalVars;


int main ( int argc, char** argv )
{

	// Parse Command line arguments
    int opt = 0;
    double startingLPS[3] = {0,0,0};
    double* startingL = NULL;
    double* startingP = NULL;
    double* startingS = NULL;
    opt = getopt( argc, argv, optString);
    while( opt != -1 ) {
        switch( opt ) {
            case 'h':
                DisplayUsage();
                break;
            case 's':
                globalVars.screenshotFilename.assign( optarg );
                break;
            case 'o':
                globalVars.overlayFilename.assign( optarg );
                break;
            case 'i':
                globalVars.imageFilename.assign( optarg );
                break;
            case 'L':
            	startingLPS[0] = svkUtils::StringToDouble(optarg);
            	startingL = startingLPS;
                break;
            case 'P':
            	startingLPS[1] = svkUtils::StringToDouble(optarg);
            	startingP = startingLPS + 1;
                break;
            case 'S':
            	startingLPS[2] = svkUtils::StringToDouble(optarg);
            	startingS = startingLPS + 2;
                break;
            default:
                cout<< endl <<" ERROR: Unrecognized option... " << endl << endl;
                DisplayUsage();
            break;
        }
        opt = getopt( argc, argv, optString );
    }
    if( globalVars.imageFilename.empty() ) {
    	DisplayUsage();
    }

	globalVars.data        = NULL;
	globalVars.overlayData = NULL;
	globalVars.screenshot  = NULL;

	/*
	 *  First step is to load the data provided.
	 */
	svkDataModel* model = svkDataModel::New();
	bool readOnlyOneFile = true;

	// Lets try to reference image first
	globalVars.data = model->LoadFile(globalVars.imageFilename, readOnlyOneFile );
	if( globalVars.data == NULL ) {
		cerr << "ERROR: Could not read input file: " << globalVars.imageFilename << endl;
		exit(1);
	}
	globalVars.data->Register(NULL);
	globalVars.data->Update();

	// And now if there is an overlay lets load that
    if ( !globalVars.overlayFilename.empty() ) {
		globalVars.overlayData = model->LoadFile(globalVars.overlayFilename, readOnlyOneFile );
		if( globalVars.overlayData == NULL ) {
			cerr << "ERROR: Could not read input file: " << globalVars.overlayFilename << endl;
			exit(1);
		}
		globalVars.overlayData->Register(NULL);
		globalVars.overlayData->Update();

    }

    // Now lets read the screenshots if provided.
	if( !globalVars.screenshotFilename.empty() ) {
		vtkImageReader2Factory* factory = vtkImageReader2Factory::New();
		vtkImageReader2* reader = factory->CreateImageReader2(globalVars.screenshotFilename.c_str());
		reader->SetFileName( globalVars.screenshotFilename.c_str() );
		globalVars.screenshot = reader->GetOutput();
		reader->Update();
		globalVars.screenshot->Update();
	}


	//Prepare overlay objects if we have overlay data and reslice if necessary
	svkLookupTable* colorTransfer = svkLookupTable::New();
	if( globalVars.overlayData != NULL ) {
		svkDataValidator* validator = svkDataValidator::New();
		bool dcosMatch = validator->AreDataGeometriesSame(globalVars.data, globalVars.overlayData);
		if( !dcosMatch ) {
			cout << "WARNING: Overlay is being resliced to image " << endl;
			svkObliqueReslice* reslicer = svkObliqueReslice::New();
			reslicer->SetInput( globalVars.overlayData );
			reslicer->SetTargetDcosFromImage( globalVars.data );
			reslicer->Update();
			globalVars.overlayData = reslicer->GetOutput();
		}
		double win;
		double lev;
		svkMriImageData::SafeDownCast(globalVars.overlayData)->GetAutoWindowLevel(win, lev);
		colorTransfer->SetRange( lev - win/2.0, lev + win/2.0);
		colorTransfer->SetLUTType( svkLookupTable::COLOR );
		colorTransfer->SetAlphaThreshold( 0.15 );
		colorTransfer->SetAlpha( 0.5 );
	}


    SetupCursor();

    // Lets create a sphere actor to sit exactly at our cursor.
	globalVars.sphere = vtkSphereSource::New();
	globalVars.sphere->SetCenter(0,0,0);
	double largestPixelSize = globalVars.data->GetSpacing()[0];
	if(globalVars.data->GetSpacing()[1] > largestPixelSize ) {
		largestPixelSize = globalVars.data->GetSpacing()[1];
	}
	if(globalVars.data->GetSpacing()[2] > largestPixelSize ) {
		largestPixelSize = globalVars.data->GetSpacing()[2];
	}
	globalVars.sphere->SetRadius(largestPixelSize/1.8);
	vtkActor* sphereActor = vtkActor::New();
	sphereActor->GetProperty()->SetOpacity(0.5);
	vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
	sphereActor->SetMapper( mapper);
	sphereActor->GetProperty()->SetDiffuseColor(1,1,1 );
	mapper->SetInput( globalVars.sphere->GetOutput() );

    // Lets create setup our window and renderers
	vtkRenderWindow* window = vtkRenderWindow::New();
	vtkRenderWindowInteractor* rwi = window->MakeRenderWindowInteractor();
	vtkInteractorStyleImage* style = vtkInteractorStyleImage::New();

	rwi->SetInteractorStyle( style );

	int* extent = globalVars.data->GetExtent();
	for( int i = 0; i < 3; i++ ){
		globalVars.annotations.push_back(vtkCornerAnnotation::New());
		globalVars.annotations[i]->GetTextProperty()->SetColor(1,0,1);
		globalVars.annotations[i]->GetTextProperty()->BoldOn();
		globalVars.annotations[i]->GetTextProperty()->SetFontSize(20);
		int newExtent[6] = {extent[0], extent[1], extent[2], extent[3], extent[4], extent[5]};
		globalVars.colorMappers.push_back(svkImageMapToColors::New());
		globalVars.colorMappers[i]->SetOutputFormatToRGBA();
		globalVars.colorMappers[i]->SetLookupTable(colorTransfer);

		if( globalVars.overlayData != NULL ) {
			globalVars.transformers.push_back(vtkTransform::New());
			globalVars.colorMappers[i]->SetInput(globalVars.overlayData);
			globalVars.overlayActors.push_back( svkOpenGLOrientedImageActor::New() );
			globalVars.overlayActors[i]->SetInput(globalVars.colorMappers[i]->GetOutput());
			globalVars.overlayActors[i]->SetUserTransform( globalVars.transformers[i] );
			globalVars.overlayActors[i]->InterpolateOff();
		} else {
			globalVars.overlayActors.push_back( NULL );
			globalVars.transformers.push_back( NULL );
		}

		globalVars.imageViewers.push_back( svkImageViewer2::New() );
		globalVars.imageViewers[i]->SetInput( globalVars.data );
		globalVars.imageViewers[i]->SetInteractorStyle( vtkInteractorStyleImage::SafeDownCast( window->GetInteractor()->GetInteractorStyle()));
		globalVars.imageViewers[i]->SetupInteractor( window->GetInteractor() );
		globalVars.imageViewers[i]->SetRenderWindow( window );
		globalVars.imageViewers[i]->GetRenderer()->SetBackground(0.0,0.0,0.0);
		globalVars.renderers.push_back(vtkRenderer::New());
		globalVars.renderers[i]->AddActor2D(globalVars.annotations[i]);
	}

	// Now lets finish setting up the image viewers
	double position[4] = {0,0,0.5,0.5};
	SetupImageViewer( position, svkDcmHeader::CORONAL, sphereActor );
	position[0] = 0.5;
	position[1] = 0.5;
	position[2] = 1;
	position[3] = 1;
	SetupImageViewer( position, svkDcmHeader::SAGITTAL, sphereActor );
	position[0] = 0;
	position[1] = 0.5;
	position[2] = 0.5;
	position[3] = 1;
	SetupImageViewer( position, svkDcmHeader::AXIAL, sphereActor );

	// If we have a screenshot then lets load it in the last renderer
	vtkRenderer* screenshotRenderer = vtkRenderer::New();
	screenshotRenderer->SetViewport(0.5, 0, 1, 0.5);
	screenshotRenderer->SetBackground(0,0,0);
	window->AddRenderer( screenshotRenderer );
	if( globalVars.screenshot != NULL ) {
		vtkImageActor* actor = vtkImageActor::New();
		actor->SetInput( globalVars.screenshot );
		screenshotRenderer->AddActor( actor );
	}

	if( globalVars.data != NULL ) {
		globalVars.data->Delete();
	}

	double center[3];
	globalVars.data->GetCenter(center);
	if( startingL != NULL ) {
		center[0] = *startingL;
	}
	if( startingP != NULL ) {
		center[1] = *startingP;
	}
	if( startingS != NULL ) {
		center[2] = *startingS;
	}
	UpdateCursor( center );
	SetSlices( center );



	// Setup Callbacks
	vtkCallbackCommand* cursorLocationCB = vtkCallbackCommand::New();
	cursorLocationCB->SetCallback( UpdateCursorLocation );
	cursorLocationCB->SetClientData( NULL );

	rwi->AddObserver(vtkCommand::MouseMoveEvent, cursorLocationCB);
	window->Render();
	rwi->Start();
	cout << "Selected Location: L:" << globalVars.cursorPoint->GetPoint(0)[0]
	                       << " P:" << globalVars.cursorPoint->GetPoint(0)[1]
	                       << " S:" << globalVars.cursorPoint->GetPoint(0)[2] << endl;
	model->Delete();
	window->Delete();
    return 0;
  
}


void SetupImageViewer( double position[4], svkDcmHeader::Orientation orientation, vtkActor* sphereActor )
{
	svkImageViewer2* imageViewer = globalVars.imageViewers[orientation];
	vtkRenderer* ren = globalVars.renderers[orientation];
	imageViewer->SetRenderer( ren );
	imageViewer->SetOrientation( orientation );
	imageViewer->ResetCamera();
	imageViewer->GetRenderer()->GetActiveCamera()->SetParallelProjection(1);
    ren->SetViewport(position);
	imageViewer->GetImageActor()->PickableOff();
	globalVars.cursorPoint->SetNumberOfPoints(1);
	globalVars.cursorPoint->SetData(globalVars.cursorPosition);
	vtkPolyData* pd = vtkPolyData::New();
	pd->SetPoints( globalVars.cursorPoint );

	double sliceNormal[3];
	globalVars.data->GetSliceNormal(sliceNormal, orientation );
    int sliceIndex = globalVars.data->GetOrientationIndex( orientation );

	for( int i = 0; i < 3; i++ ) {
		if( i != orientation ) {
			vtkLineSource* line = vtkLineSource::New();
			double orthSliceNormal[3];
			globalVars.data->GetSliceNormal(orthSliceNormal, (svkDcmHeader::Orientation)i );
			int lineLength = 100;
			double disp = globalVars.data->GetSpacing()[sliceIndex];
			if( orientation == svkDcmHeader::CORONAL ) {
				disp *=-1;
			}
			line->SetPoint1(lineLength*orthSliceNormal[0] + disp*sliceNormal[0], lineLength*orthSliceNormal[1] + disp*sliceNormal[1], lineLength*orthSliceNormal[2] + disp*sliceNormal[2] );
			line->SetPoint2(-lineLength*orthSliceNormal[0] + disp*sliceNormal[0], -lineLength*orthSliceNormal[1] + disp*sliceNormal[1], -lineLength*orthSliceNormal[2] + disp*sliceNormal[2] );
			vtkGlyph3D* glyph = vtkGlyph3D::New();
			glyph->SetSource( line->GetOutput() );
			glyph->SetInput(pd);
			vtkActor* cursorActor = vtkActor::New();
			vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
			cursorActor->GetProperty()->SetLineWidth(1);
			mapper->SetInput( glyph->GetOutput() );
			cursorActor->SetMapper( mapper );
			cursorActor->GetProperty()->SetColor(1,1,0);
			imageViewer->GetRenderer()->AddActor( cursorActor );
		}
	}

	imageViewer->GetRenderer()->AddActor( sphereActor );
	if( globalVars.overlayActors[orientation] != NULL ) {
		imageViewer->GetRenderer()->AddActor( globalVars.overlayActors[orientation] );
	}

	imageViewer->GetRenderer()->AddActor( sphereActor );
	double win;
	double lev;
	svkMriImageData::SafeDownCast(globalVars.data)->GetAutoWindowLevel(win, lev);
	imageViewer->SetColorWindow( win );
	imageViewer->SetColorLevel( lev );

}


/*!
 * Sets the slice for a given orientation.
 */
void SetSlice( int slice, svkDcmHeader::Orientation orientation )
{
	globalVars.imageViewers[ orientation ]->SetSlice( slice );

	if( globalVars.overlayData != NULL ) {
		int* overlayExtent = globalVars.overlayData->GetExtent();
		int overlaySliceExtent[6] = {overlayExtent[0], overlayExtent[1], overlayExtent[2], overlayExtent[3], overlayExtent[4], overlayExtent[5]};
		int overlaySlice = -1;
		double sliceCenter[3];
		globalVars.data->GetSliceOrigin( slice, sliceCenter, orientation );
		overlaySlice = globalVars.overlayData->GetClosestSlice( sliceCenter, orientation, 0 );
		int orientationIndex = globalVars.data->GetOrientationIndex( orientation );
		overlaySliceExtent[2*orientationIndex ] = overlaySlice;
		overlaySliceExtent[2*orientationIndex+1 ] = overlaySlice;
		double* imageOrigin = globalVars.data->GetOrigin();
		double* overlayOrigin = globalVars.overlayData->GetOrigin();
		double* imageSpacing = globalVars.data->GetSpacing();
		double* overlaySpacing = globalVars.overlayData->GetSpacing();
		double normal[3];
		globalVars.overlayData->GetSliceNormal(normal, orientation );

		double delta  = (vtkMath::Dot(imageOrigin, normal )+(slice)*imageSpacing[orientationIndex] -
								 (vtkMath::Dot( overlayOrigin, normal ) +
								 overlaySpacing[orientationIndex] * overlaySlice));
		double tol = 0.05;
		if( orientation == svkDcmHeader::CORONAL ) {
			tol *=-1;
		}

		globalVars.transformers[orientation]->Delete();
		globalVars.transformers[orientation] = vtkTransform::New();
		globalVars.transformers[orientation]->Translate( (delta+tol)*normal[0],
								   (delta+tol)*normal[1],
								   (delta+tol)*normal[2]);
		//globalVars.transformers[orientation]->Translate( 0,0,0);
		globalVars.overlayActors[orientation]->SetUserTransform(globalVars.transformers[orientation]);
		globalVars.overlayActors[orientation]->SetDisplayExtent(overlaySliceExtent);
	}
}


/*!
 * Sets all three slices to the closest slice to the given coordinate.
 */
void SetSlices( double coordinates[3] )
{
	for( int i = 0; i < 3; i++ ) {
		int slice = globalVars.data->GetClosestSlice( coordinates, (svkDcmHeader::Orientation)i, 0 );
		SetSlice( slice, (svkDcmHeader::Orientation)i );
	}
}


/* 
 *   Catches keypresses. Currently changes slice use - and +.
 */
void KeypressCallback(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata)
{
    char keyPressed;
    int newSlice = -1;
    vtkRenderWindowInteractor *rwi =
            vtkRenderWindowInteractor::SafeDownCast( subject );
    keyPressed = rwi->GetKeyCode();
            
    if ( keyPressed == '+' ) {
    } else if ( keyPressed == '-' ) {
    }
}


/*!
 *  Creates the cursor point object.
 */
void SetupCursor()
{
	globalVars.cursorPoint = vtkPoints::New();
	globalVars.cursorPoint->SetNumberOfPoints(1);
	globalVars.cursorPosition = vtkDoubleArray::New();
	globalVars.cursorPosition->SetNumberOfComponents(3);
	globalVars.cursorPosition->SetNumberOfTuples(1);
	globalVars.cursorPosition->SetTuple3(0,0,0,0);
	globalVars.cursorPoint->SetData( globalVars.cursorPosition );
}


/*!
 * Updates the cursor and annotations to the given location.
 */
void UpdateCursor(double location[3])
{
		globalVars.sphere->SetCenter( location );
		globalVars.cursorPosition->SetTuple3(0,location[0], location[1], location[2]);
		globalVars.cursorPoint->Modified();

	    std::stringstream outL;
        outL.setf(ios::fixed,ios::floatfield);
        outL.precision(2);
        outL << "L: " << location[0] << endl;
		globalVars.annotations[svkDcmHeader::SAGITTAL]->SetText(2, outL.str().c_str());
	    std::stringstream outP;
        outP.setf(ios::fixed,ios::floatfield);
        outP.precision(2);
        outP << "P: " << location[1] << endl;
		globalVars.annotations[svkDcmHeader::CORONAL]->SetText(2, outP.str().c_str());
	    std::stringstream outS;
        outS.setf(ios::fixed,ios::floatfield);
        outS.precision(2);
        outS << "S: " << location[2];
		globalVars.annotations[svkDcmHeader::AXIAL]->SetText(2, outS.str().c_str());
}


/*!
 *  Callback caught when the mouse moves.
 */
void UpdateCursorLocation(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata)
{
    vtkRenderWindowInteractor *rwi =
            vtkRenderWindowInteractor::SafeDownCast( subject );
    int keyPressed = rwi->GetShiftKey();
    if( keyPressed == 1  ) {
		int pos[2];
		pos[0] = rwi->GetEventPosition()[0];
		pos[1] = rwi->GetEventPosition()[1];
		vtkRenderer* ren = rwi->FindPokedRenderer( pos[0], pos[1]);
		vtkCoordinate* mousePosition = vtkCoordinate::New();
		mousePosition->SetCoordinateSystemToDisplay();
		mousePosition->SetValue( pos[0], pos[1], 0);
		double* imageCords;
		imageCords = mousePosition->GetComputedWorldValue( ren );
		rwi->GetRenderWindow()->Render();

		double* origin;
		double* spacing;
		double planeOrigin[3];
		double projection[3];
		svkDcmHeader::Orientation orientation = svkDcmHeader::UNKNOWN_ORIENTATION;
		// We need the anatomical slice to calculate a point on the image
		int slice;
		for( int i = 0; i < 3; i++ ) {
			if( ren == globalVars.renderers[i] ) {
				orientation = (svkDcmHeader::Orientation)i;
			}
		}
		if( orientation != svkDcmHeader::UNKNOWN_ORIENTATION ) {
			origin = globalVars.data->GetOrigin();
			slice = globalVars.imageViewers[ orientation ]->GetSlice( orientation );
			int index[3] = {0,0,0};
			index[ globalVars.data->GetOrientationIndex( orientation ) ] = slice;
			globalVars.data->GetPositionFromIndex(index, planeOrigin);
			double viewNormal[3];
			globalVars.data->GetSliceNormal( viewNormal, orientation );
			double viewNormalDouble[3] = { viewNormal[0], viewNormal[1], viewNormal[2] };
			vtkPlane::GeneralizedProjectPoint( imageCords, planeOrigin, viewNormalDouble, projection );
			UpdateCursor( projection );
			SetSlices(projection);
		}
    }
}


/*!
 * Displays the usage message.
 */
void DisplayUsage( void )
{
    cout << endl << "############  USAGE  ############ " << endl << endl;
    cout << "NAME" << endl;
    cout << "    svk_point_selector" << endl << endl;
    cout << "SYNOPSIS" << endl;
    cout << "    svk_point_selector -i reference_image [ -o overlay ] [-s screenshot ]     " << endl;
    cout << "                                          [-L pos] [-P pos] [-S pos]          " << endl << endl;
    cout << "DESCRIPTION" << endl;
    cout << "    svk_point_selector is used to examine an image with an overlay in the     " << endl;
    cout << "    three orthogonal directions. A cursor is placed over the image which will " << endl;
    cout << "    follow the mouse if shift is held. When the program terminates the        " << endl;
    cout << "    coordinates of the cursor are printed to the command line. A starting     " << endl;
    cout << "    position can be defined for the cursor by using the -L -P and/or -S flags." << endl;
    cout << "    Coordinate arguments must be quoted.                                      " << endl << endl;
    cout << "EXAMPLE" << endl;
    cout << "    svk_point_selector -i t1234_t1ca.idf -o 12A34.idf -s 12A34.jpg -L \"1.03\""<< endl;
    cout << "VERSION" << endl;
    cout << "     " << SVK_RELEASE_VERSION << endl;
    cout << endl << "############  USAGE  ############ " << endl << endl;
    /* ... */
    exit( EXIT_FAILURE );
}
