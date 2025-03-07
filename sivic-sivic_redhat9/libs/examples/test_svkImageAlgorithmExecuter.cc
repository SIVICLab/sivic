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
#include <svkImageAlgorithmExecuter.h>
#include <vtkImageFourierCenter.h>
#include <vtkImageGaussianSmooth.h>

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
    vtkRenderer* ren = vtkRenderer::New();
    ren->SetBackground(0.1,0.2,0.4);
    vtkRenderWindow* window = vtkRenderWindow::New();
    vtkRenderWindowInteractor* rwi = vtkRenderWindowInteractor::New();
    svkImageViewer2* viewer = svkImageViewer2::New(); 
    viewer->SetRenderWindow( window );
    viewer->SetRenderer( ren );
    window->SetInteractor( rwi );
    window->AddRenderer( ren);  
    window->SetSize(600,600);  

    // Load the data sets
    svkImageData* image = LoadFile( fileNameImage );
    viewer->SetSlice( 4 ); 
    viewer->SetInput( image ); 
    viewer->SetupInteractor( rwi );
    viewer->GetRenderer()->ResetCamera();
    viewer->SetSliceOrientationToXY();
    viewer->GetImageActor()->InterpolateOff();
    viewer->ResetCamera();
    window->Render();
    for( int i = 20; i <= 20; i++) {
        viewer->SetSlice(i);
        window->Render();
    }

    // Here is the vtk Algorithm
    vtkImageGaussianSmooth* algo = vtkImageGaussianSmooth::New();

    // Here is our executer
    svkImageAlgorithmExecuter* executer = svkImageAlgorithmExecuter::New();

    // Give the executer the image input
    executer->SetInput( image );

    // Give the executer the  vtk algorithm
    executer->SetAlgorithm( algo );
    
    // Update IS necessary
    executer->Update();

    viewer->SetInput( executer->GetOutput() ); 
    window->Render();
    for( int i = 20; i <= 20 ; i++) {
        viewer->SetSlice(i);
        window->Render();
    }
    image->GetDcmHeader()->PrintDcmHeader();
    executer->GetOutput()->GetDcmHeader()->PrintDcmHeader();


    viewer->Delete();
    executer->Delete();
    algo->Delete();
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
