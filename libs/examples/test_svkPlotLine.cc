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
 *  Test driver for the svkPlotLine. Just checks new/delete.
 *
 *  The following classes are utilized in this driver.
 *      svkPlotLine
 *
 */

#include <svkPlotLine.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkDataSetMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkUnstructuredGrid.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCubeSource.h>
#include <math.h>

#define PI 3.1459

using namespace svk;

int main ( int argc, char** argv )
{
    vtkRenderer* renderer = vtkRenderer::New();
    vtkRenderWindow* window = vtkRenderWindow::New(); 
    vtkRenderWindowInteractor* rwi = vtkRenderWindowInteractor::New();
    window->SetInteractor( rwi );
    rwi->SetRenderWindow(window);
    window->AddRenderer( renderer );

    svkPlotLine* testBoxPlot = svkPlotLine::New();

    vtkFloatArray* defaultPlotData = vtkFloatArray::New();
    defaultPlotData->SetNumberOfComponents( 2 );
    int n = 30;
    vtkPoints* points = vtkPoints::New();
    points->SetNumberOfPoints( n );

    defaultPlotData->SetNumberOfTuples( n );
    testBoxPlot->GetPointIds()->SetNumberOfIds( n );
    testBoxPlot->SetDataPoints( points );

    // Create dummy array
    for( int i = 0; i < n; i++ ) {
        testBoxPlot->GetPointIds()->SetId( i,i );
        defaultPlotData->SetTuple2(i,sin( (2*PI*i)/((float)(n)) ),cos( (2*PI*i)/((float)(n)) ));
        cout << "value: " << sin((2*PI*i)/n) << endl;
    }

    testBoxPlot->SetPointRange( 0, 30 );
    testBoxPlot->SetValueRange( -0.75, 0.75  );

    double defaultBounds[6] = {1,2,1,2,0,1};
    double origin[3] = { 1,1,1 };
    double spacing[3] = { 1,1,1 };
    double dcos[3][3] = { {1,0,0}, {0,1,0}, {0,0,-1} };
    //testBoxPlot->SetPlotAreaBounds( defaultBounds );
    testBoxPlot->SetOrigin( origin );
    testBoxPlot->SetSpacing( spacing );
    testBoxPlot->SetDcos( dcos );
      
    testBoxPlot->SetData( defaultPlotData );

    // Create default plot parameters
    
    cout << " plot line " << *testBoxPlot << endl;

/////////////////////////////////////////////
    vtkUnstructuredGrid* grid = vtkUnstructuredGrid::New(); 
    grid->Allocate(1,1);
    grid->SetPoints( testBoxPlot->GetDataPoints() );
    grid->InsertNextCell(testBoxPlot->GetCellType(), testBoxPlot->GetPointIds()); 

    vtkDataSetMapper* mapper = vtkDataSetMapper::New();
    mapper->SetInput( grid );    

    vtkActor* actor = vtkActor::New();
    actor->SetMapper( mapper );

    renderer->AddActor( actor );

    vtkCubeSource* refBox = vtkCubeSource::New();
    refBox->SetBounds( defaultBounds );

    vtkPolyDataMapper* refMapper = vtkPolyDataMapper::New();
    refMapper->SetInput( refBox->GetOutput() );

    vtkActor* refActor = vtkActor::New();
    refActor->SetMapper( refMapper );
    refActor->GetProperty()->SetRepresentationToWireframe();
    refActor->GetProperty()->SetAmbientColor(1,0,0);
    refActor->GetProperty()->SetAmbient(1);
    refActor->GetProperty()->SetDiffuse(0);
    renderer->AddActor( refActor );
    window->Render();
    renderer->ResetCamera();
    rwi->Start();
    
    testBoxPlot->Delete(); 
    renderer->Delete();
    window->Delete();
    rwi->Delete();
    return 0;
}
