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
 *
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
    if( argc == 2 ) {
        fileNameImage = argv[1];
    } else {
        cerr<<"Too many input arguments."<<endl;
        return 1;
    } 
    double* bounds;
    double* range;
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
    int* extent = image->GetExtent();

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
    int slice = 4; 
    viewer->SetSlice( slice ); 
    viewer->SetInput( image ); 
    viewer->SetupInteractor( rwi );
    viewer->GetRenderer()->ResetCamera();
    viewer->SetSliceOrientationToXY();
    viewer->GetImageActor()->InterpolateOff();
    viewer->ResetCamera();
    window->Render();
    vtkInteractorStyleTrackballCamera* style = vtkInteractorStyleTrackballCamera::New();
    //rwi->SetInteractorStyle( vtkInteractorStyleSwitch::New() );
    
    //rwi->SetInteractorStyle( style );

    viewer->SetOrientation( svkDcmHeader::CORONAL);
    //viewer->SetSliceOrientationToXZ();
    viewer->ResetCamera();
    rwi->Start();
    for( int i = extent[2]; i < extent[3]; i++ ) {
        viewer->SetSlice( i, svkDcmHeader::CORONAL );
        window->Render();
    }
    //viewer->SetSliceOrientationToYZ();
    viewer->SetOrientation( svkDcmHeader::SAGITTAL);
    viewer->ResetCamera();
    for( int i = extent[0]; i < extent[1]; i++ ) {
        viewer->SetSlice( i, svkDcmHeader::SAGITTAL );
        window->Render();
    }
    viewer->SetOrientation( svkDcmHeader::AXIAL);
    viewer->ResetCamera();
    for( int i = extent[4]; i < extent[5]; i++ ) {
        viewer->SetSlice( i );
        window->Render();
    }
    for( int i = extent[4]; i < extent[5]; i++ ) {
        viewer->SetSlice( i, svkDcmHeader::AXIAL );
        window->Render();
    }
    rwi->Start();

    obliqueImage->Delete();
    viewer->Delete();
    ren->Delete();
    window->Delete();
    rwi->Delete();
    image->Delete();
    fileNameImage = NULL;
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
