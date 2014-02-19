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
