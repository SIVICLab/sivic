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
#include <vtkLookupTable.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkImageMapToColors.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <svkImageMapToWindowLevelColors.h>
#include <svkOrientedImageActorFactory.h>
#include <vtkPolyLine.h>

using namespace svk;

svkImageData* LoadFile( char* fileName );
vtkActor* GenerateGridActor( svkImageData* data );

int main ( int argc, char** argv )
{
    char* fileNameImage = NULL;
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
    window->SetInteractor( rwi );
    window->AddRenderer( ren);  
    window->SetSize(600,600);  

    // Load the data set
    svkImageData* image = LoadFile( fileNameImage );

    // Throw in an axes for reference
    vtkAxesActor* myAxesNormal = vtkAxesActor::New();
    vtkTransform* optimus = vtkTransform::New();
    optimus->Translate(image->GetOrigin());
    myAxesNormal->SetTotalLength(100,100,100); 
    myAxesNormal->SetUserTransform(optimus);
    optimus->Delete();
    ren->AddActor( myAxesNormal );
    myAxesNormal->Delete();
    

    svkOrientedImageActorFactory* factory = svkOrientedImageActorFactory::New();
    vtkObjectFactory::RegisterFactory( factory );
    factory->Delete();
    vtkImageActor* actor = vtkImageActor::New();
    vtkObjectFactory::UnRegisterAllFactories();
    actor->InterpolateOff();
    svkImageMapToWindowLevelColors *color = svkImageMapToWindowLevelColors::New();
    color->SetInput( image ); 
    color->Update();

    actor->SetInput( color->GetOutput() ); 
    color->Delete();
    extent = image->GetExtent();
    actor->SetDisplayExtent(extent[0], extent[1], extent[2], extent[3], 19, 19 );
    bounds = actor->GetBounds();
    ren->AddActor(actor);
    actor->Delete();

    vtkActor* boundsActor = vtkActor::New();
    vtkCubeSource* boundsCube = vtkCubeSource::New();
    boundsCube->SetBounds( bounds );
    vtkDataSetMapper* mapper = vtkDataSetMapper::New();
    mapper->SetInput( boundsCube->GetOutput() );
    boundsCube->Delete();
    boundsActor->SetMapper(mapper);
    mapper->Delete();
    boundsActor->GetProperty()->SetRepresentationToWireframe();
    boundsActor->GetProperty()->SetSpecular(0);
    boundsActor->GetProperty()->SetDiffuse(0);
    boundsActor->GetProperty()->SetAmbient(1);
    ren->AddActor( boundsActor );
    boundsActor->Delete();    
    ren->AddActor( GenerateGridActor(image) );
    window->Render();
    vtkInteractorStyleTrackballCamera* style = vtkInteractorStyleTrackballCamera::New();
    rwi->SetInteractorStyle( style ); 
    style->Delete();
    ren->ResetCamera(); 
    window->Render();
    rwi->Start();
   
    ren->Delete();
    window->Delete();
    rwi->Delete();
    fileNameImage = NULL;
    image->Delete();
    cout<<"Post Delete"<<endl;
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

vtkActor* GenerateGridActor( svkImageData* data )
{

    vtkPoints* selectionBoxPoints = vtkPoints::New();
    selectionBoxPoints->SetNumberOfPoints(8);
    selectionBoxPoints->InsertPoint(0, 100, 0, 0 ) ;
    selectionBoxPoints->InsertPoint(1, 100,100, 0 ) ;
    selectionBoxPoints->InsertPoint(2, 100, 0, 100 ) ;
    selectionBoxPoints->InsertPoint(3, 100, 100, 100 ) ;
    selectionBoxPoints->InsertPoint(4, 0, 0, 0 ) ;
    selectionBoxPoints->InsertPoint(5, 0, 0, 100 ) ;
    selectionBoxPoints->InsertPoint(6, 0, 100, 0 ) ;
    selectionBoxPoints->InsertPoint(7, 0, 100, 100 ) ;
    // And now lets use a Hexahedron to represent them.
    // The point ID's must be in a specific order for this to work,
    // hence the variation in SetId calls. See the vtkHexahedron documentation.
    vtkUnstructuredGrid* selectionBoxGrid = vtkUnstructuredGrid::New();
    selectionBoxGrid->Allocate(1, 1);
    selectionBoxGrid->SetPoints( selectionBoxPoints );
    vtkPolyLine* line = vtkPolyLine::New();
    line->GetPointIds()->SetNumberOfIds( 2 );
    line->GetPointIds()->SetId(0, 0);
    line->GetPointIds()->SetId(1, 1);
    selectionBoxGrid->InsertNextCell( line->GetCellType(), line->GetPointIds() );
    line->Delete();
    line = vtkPolyLine::New();
    line->GetPointIds()->SetNumberOfIds( 2 );
    line->GetPointIds()->SetId(0, 0);
    line->GetPointIds()->SetId(1, 2);
    selectionBoxGrid->InsertNextCell( line->GetCellType(), line->GetPointIds() );
    line->Delete();
    line = vtkPolyLine::New();
    line->GetPointIds()->SetNumberOfIds( 2 );
    line->GetPointIds()->SetId(0, 0);
    line->GetPointIds()->SetId(1, 3);
    selectionBoxGrid->InsertNextCell( line->GetCellType(), line->GetPointIds() );

    //selectionBox->GetPointIds()->SetId(2, 2);
    //selectionBox->GetPointIds()->SetId(3, 3);
    //selectionBox->GetPointIds()->SetId(4, 4);
    //selectionBox->GetPointIds()->SetId(5, 5);
    //selectionBox->GetPointIds()->SetId(6, 6);
    //selectionBox->GetPointIds()->SetId(7, 7);

    // We need an object that can be mapped, so we but the hexahedron into on UnstructuredGrid

    // We will need a mapper to draw the data.
    vtkDataSetMapper* selectionBoxMapper = vtkDataSetMapper::New();
    selectionBoxMapper->SetInput(selectionBoxGrid);

    // Now we will create the actual actor
    vtkActor* selectionBoxActor = vtkActor::New();
    selectionBoxActor->SetMapper(selectionBoxMapper);
	return selectionBoxActor;
}
