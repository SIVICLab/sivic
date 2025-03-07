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
