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
 */

#include<vtkRenderer.h>
#include<vtkRenderWindow.h>
#include<vtkRenderWindowInteractor.h>
#include<vtkCubeSource.h>
#include<vtkActor.h>
#include<vtkActor2D.h>
#include<vtkActor2DCollection.h>
#include<vtkMapper.h>
#include<vtkProperty.h>
#include<vtkProperty2D.h>
#include<vtkPolyDataMapper.h>
#include<vtkPolyDataMapper2D.h>
#include<vtkDataSetMapper.h>
#include<vtkFloatArray.h>
#include<vtkRectilinearGrid.h>
#include<vtkRectilinearGridGeometryFilter.h>
#include<vtkCamera.h>
#include<vtkCoordinate.h>
#include<vtkExtractEdges.h>
    
int main ( int argc, char** argv )
{

  vtkCoordinate* myCoordinate = vtkCoordinate::New();
  vtkRenderer *renderer = vtkRenderer::New();
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  renWin->AddRenderer(renderer);
  vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
  iren->SetRenderWindow(renWin);


    double* bounds = new double[6];
    bounds[0] = 10;
    bounds[1] = 200;
    bounds[2] = 30;
    bounds[3] = 200;
    bounds[4] = 10;
    bounds[5] = 150;
    myCoordinate->SetCoordinateSystemToWorld();
    vtkActorCollection* myCollection3D = vtkActorCollection::New(); 
    vtkActor2DCollection* myCollection = vtkActor2DCollection::New(); 
    for( int i = 0; i < 1 ; i++ ) {

        vtkCubeSource* gridVoxel = vtkCubeSource::New();
        gridVoxel->SetBounds( bounds );

        vtkPolyDataMapper* gridVoxelMapper3D = vtkPolyDataMapper::New();
        vtkPolyDataMapper2D* gridVoxelMapper = vtkPolyDataMapper2D::New();

        gridVoxelMapper->SetInput( gridVoxel->GetOutput() );
        gridVoxelMapper3D->SetInput( gridVoxel->GetOutput() );
        //gridVoxelMapper->SetTransformCoordinate(myCoordinate);

        vtkProperty* visualProperties3D = vtkProperty::New();
        vtkProperty2D* visualProperties = vtkProperty2D::New();
        visualProperties->SetColor( 0.0, 1.0, 0.0 );
        visualProperties->SetOpacity( 0.1 );
        visualProperties3D->SetDiffuseColor( 1.0, 0.0, 1.0 );
        visualProperties3D->SetRepresentationToWireframe();
        visualProperties3D->SetLineWidth(4);

        vtkActor* gridVoxelActor3D = vtkActor::New();
        vtkActor2D* gridVoxelActor = vtkActor2D::New();
        gridVoxelActor->SetMapper( gridVoxelMapper );
        gridVoxelActor3D->SetMapper( gridVoxelMapper3D );
        gridVoxelActor->SetProperty( visualProperties );
        gridVoxelActor3D->SetProperty( visualProperties3D );
        cout<<" actor size:"<<sizeof( *gridVoxelActor )<<endl;
        cout<<" mapper size:"<<sizeof( *visualProperties )<<endl;
        cout<<" cube source size:"<<sizeof( *gridVoxel )<<endl;
        myCollection->AddItem(gridVoxelActor);
        myCollection3D->AddItem(gridVoxelActor3D);
        renderer->AddActor( gridVoxelActor );
        renderer->AddActor( gridVoxelActor3D );
        gridVoxel->Delete();
        gridVoxelMapper->Delete();
        visualProperties->Delete();
        gridVoxelActor->Delete();
    }
    renderer->SetBackground( 0.2,0.2,0.2 );
    
    renWin->Render();
    iren->Start();
    myCollection->Delete();
    // Clean up
    renderer->Delete();
    renWin->Delete();
    iren->Delete();
    delete[] bounds;
  return 0;
}
