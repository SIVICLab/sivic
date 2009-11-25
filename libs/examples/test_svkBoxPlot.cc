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
 *  Test driver for the svkBoxPlot. Just checks new/delete.
 *
 *  The following classes are utilized in this driver.
 *      svkBoxPlot
 *
 */

#include <svkBoxPlot.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

using namespace svk;

int main ( int argc, char** argv )
{
    vtkRenderer* renderer = vtkRenderer::New();
    vtkRenderWindow* window = vtkRenderWindow::New(); 
    vtkRenderWindowInteractor* rwi = vtkRenderWindowInteractor::New();
    window->SetInteractor( rwi );
    rwi->SetRenderWindow(window);
    window->AddRenderer( renderer );

    svkBoxPlot* testBoxPlot = svkBoxPlot::New();
//    testBoxPlot->Initialize();
////////////////////////////////////////////////////
    // Create default object...
    vtkFloatArray* defaultPlotData = vtkFloatArray::New();
    defaultPlotData->SetNumberOfComponents( 2 );
    int n = 30;
    defaultPlotData->SetNumberOfTuples( n );

    // Create 0 value array
    for( int i = 0; i < n; i++ ) {
        if( i%2 == 0 ) {
            defaultPlotData->SetTuple2(i,-1000,-1000);
        } else {
            defaultPlotData->SetTuple2(i,1000,1000);
        }
    }

    testBoxPlot->SetData( defaultPlotData );

    double defaultBounds[6] = {0,1000,0,1000,0,1000};
    testBoxPlot->SetPlotAreaBounds( defaultBounds );

    // Create default plot parameters
    testBoxPlot->SetPointRange( 0, n );
    testBoxPlot->SetValueRange( -2000, 2000  );

/////////////////////////////////////////////
    renderer->AddActor( testBoxPlot );
    window->Render();
    renderer->ResetCamera();
    rwi->Start();
    
    testBoxPlot->Delete(); 
    renderer->Delete();
    window->Delete();
    rwi->Delete();
    return 0;
}
