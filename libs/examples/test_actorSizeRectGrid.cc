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
#include<vtkPolyDataMapper.h>
#include<vtkPolyDataMapper2D.h>
#include<vtkDataSetMapper.h>
#include<vtkFloatArray.h>
#include<vtkRectilinearGrid.h>
#include<vtkRectilinearGridGeometryFilter.h>
#include<vtkCamera.h>
    
int main ( int argc, char** argv )
{

    vtkProperty* prop = vtkProperty::New();
    prop->SetRepresentationToWireframe();
    double* bounds = new double[6];
    bounds[0] = 0;
    bounds[1] = 1;
    bounds[2] = 0;
    bounds[3] = 1;
    bounds[4] = 0;
    bounds[5] = 1;
    vtkActorCollection* myCollection = vtkActorCollection::New(); 
    //vtkActor2DCollection* myCollection = vtkActor2DCollection::New(); 
      static float x[2]={bounds[0],bounds[1]};
      static float y[2]={bounds[2],bounds[3]};
      static float z[2]={bounds[4],bounds[5]};
    for( int i = 0; i < 1152 ; i++ ) {
      vtkFloatArray *xCoords = vtkFloatArray::New();
      for (int i=0; i<2; i++) xCoords->InsertNextValue(x[i]);
      
      vtkFloatArray *yCoords = vtkFloatArray::New();
      for (int i=0; i<2; i++) yCoords->InsertNextValue(y[i]);
      
      vtkFloatArray *zCoords = vtkFloatArray::New();
      for (int i=0; i<2; i++) zCoords->InsertNextValue(z[i]);
      
      vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
      rgrid->SetDimensions(2,2,2);
      rgrid->SetXCoordinates(xCoords);
      rgrid->SetYCoordinates(yCoords);
      rgrid->SetZCoordinates(zCoords);

      vtkDataSetMapper *rgridMapper = vtkDataSetMapper::New();
      rgridMapper->SetInput(rgrid);

      //vtkActor2D *wireActor = vtkActor2D::New();
      vtkActor *wireActor = vtkActor::New();
      wireActor->SetMapper(rgridMapper);
      wireActor->SetProperty( prop );
      wireActor->GetProperty()->SetRepresentationToWireframe();
      wireActor->GetProperty()->SetColor(0,0,0);
      myCollection->AddItem(wireActor);

      xCoords->Delete();
      yCoords->Delete();
      zCoords->Delete();
      rgrid->Delete();
      rgridMapper->Delete();
      wireActor->Delete();

    }
    prop->Delete();
    myCollection->Delete();
    delete[] bounds;
  return 0;
}
