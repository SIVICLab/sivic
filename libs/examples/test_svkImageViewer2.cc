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
 *  Test driver for svkImageData
 */

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <svkImageReader2.h>
#include <svkImageReaderFactory.h>
#include <svkImageData.h>
#include <vtkCamera.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkExtractEdges.h>
#include <vtkImageViewer2.h>
#include <svkImageData.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkWindowLevelLookupTable.h>
#include <vtkPlane.h>
#include <vtkAxesActor.h>
#include <vtkCubeSource.h>
#include <vtkTransform.h>
#include <svkImageViewer2.h>
#include <vtkImageViewer2.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkInteractorStyleSwitch.h>

using namespace svk;

svkImageData* LoadFile( char* fileName );

int main ( int argc, char** argv )
{
    char* fileNameImage = NULL;
    char* fileNameSpectra = NULL;
    if( argc < 2 ) {
        cerr<<"Not enough input arguments."<<endl;
        return 1;
    } else if( argc == 2 ) {
        fileNameImage = argv[1];
    } else if( argc > 2) {
        cerr<<"Too many input arguments."<<endl;
        return 1;
    } 
    double* bounds;
    double* range;
    int* extent;
    // initialize window
    vtkRenderer* ren = vtkRenderer::New();
    ren->SetBackground(0.1,0.2,0.4);
    vtkRenderWindow* window = vtkRenderWindow::New();
    vtkRenderWindowInteractor* rwi = vtkRenderWindowInteractor::New();
    svkImageViewer2* viewer = svkImageViewer2::New(); 
    //vtkImageViewer2* viewer = vtkImageViewer2::New(); 
    viewer->SetRenderWindow( window );
    viewer->SetRenderer( ren );
    window->SetInteractor( rwi );
    window->AddRenderer( ren);  
    window->SetSize(600,600);  

    // Load the data sets
    svkImageData* image = LoadFile( fileNameImage );

    // Throw in an axes
    vtkAxesActor* myAxesNormal = vtkAxesActor::New();
    vtkTransform* optimus = vtkTransform::New();
    optimus->Translate(image->GetOrigin());
    myAxesNormal->SetTotalLength(100,100,100); 
    myAxesNormal->SetUserTransform(optimus);
    optimus->Delete();
    myAxesNormal->AxisLabelsOff();
    ren->AddActor( myAxesNormal );
    myAxesNormal->Delete();

    // Setup cosine matrix, this should be grabbed from headers
    double dcos[3][3];
    dcos[0][0] = 1;
    dcos[0][1] = 0;
    dcos[0][2] = 0;
    dcos[1][0] = 0;
    dcos[1][1] = 0.99284;
    dcos[1][2] = 0.11948;
    dcos[2][0] = 0;
    dcos[2][1] = 0.11948;
    dcos[2][2] = -0.99284;

    // Create oblique image
    svkImageData* obliqueImage = svkMriImageData::New();
    obliqueImage->CopyVtkImage( image, dcos );
    int slice = 22; 
    viewer->SetSlice( slice ); 
    viewer->SetInput( obliqueImage ); 
    viewer->SetupInteractor( rwi );
    //viewer->GetRenderer()->ResetCamera();
    viewer->SetSliceOrientationToXY();
    viewer->GetImageActor()->InterpolateOff();
    viewer->ResetCamera();
    window->Render();
    vtkInteractorStyleTrackballCamera* style = vtkInteractorStyleTrackballCamera::New();
    //rwi->SetInteractorStyle( vtkInteractorStyleSwitch::New() );
    rwi->SetInteractorStyle( style );
    rwi->Start();

    obliqueImage->Delete();
    viewer->Delete();
    ren->Delete();
    window->Delete();
    rwi->Delete();
    image->Delete();
    fileNameImage = NULL;
    fileNameSpectra = NULL;
    return 0;
    
}

svkImageData* LoadFile( char* fileName )
{
    svkImageData* myData;
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2(fileName);
    readerFactory->Delete();

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << fileName << endl;
        exit(1);
    }

    reader->SetFileName( fileName );
    reader->Update();
    myData = reader->GetOutput();
    myData->Register(NULL);
    reader->Delete();
    return myData;
}
