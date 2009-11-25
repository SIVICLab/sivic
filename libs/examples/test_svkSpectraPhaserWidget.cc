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
 *  A widget for phasing and viewing data.
 *
 *
 */

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <svkPlotGridViewController.h>
#include <svkDataModel.h>
#include <svkPhaseSpec.h>

using namespace svk;

int main ( int argc, char** argv )
{
    vtkRenderWindow* window = vtkRenderWindow::New(); 
    svkDataModel* model = svkDataModel::New();
    svkPhaseSpec* phaser = svkPhaseSpec::New();
    svkPlotGridViewController* plotView = svkPlotGridViewController::New();
    plotView->SetRWInteractor( window->MakeRenderWindowInteractor() );

    // Load a file....
    svkImageData* data = model->LoadFile("../testing/data/t3148_1_cor.ddf"); 

    // Phase the data...
    phaser->SetInput( data );
    phaser->SetPhase0( 90 );
    phaser->Update();

    // Set the data into the View
    plotView->SetInput( data );
    plotView->SetSlice( 4 );
    plotView->HighlightSelectionVoxels( );
    plotView->SetWindowLevelRange(150, 350, 0 );
    plotView->SetWindowLevelRange(-98000000, 200000000, 1);

    window->Render();
    window->GetInteractor()->Start();
    window->Delete();
    data->Delete();
    model->Delete();

    return 0;
}
