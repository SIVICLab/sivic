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
#include <vtkRendererCollection.h>
#include <vtkXYPlotActor.h>
#include <vtkRenderWindowInteractor.h>
#include <svkDataModel.h>
#include <vtkFieldData.h>
#include <vtkDataObject.h>

using namespace svk;

int main ( int argc, char** argv )
{
    vtkRenderWindow* window = vtkRenderWindow::New(); 
    window->MakeRenderWindowInteractor();
    window->SetSize( 600,600 );
    svkDataModel* model = svkDataModel::New();
    vtkRenderer* renderer = vtkRenderer::New();
    window->AddRenderer( renderer ); 
    vtkXYPlotActor* plot = vtkXYPlotActor::New();
    vtkFieldData* fieldData = vtkFieldData::New();
    vtkDataObject* dataStore = vtkDataObject::New();

    // Load a file....
    svkImageData* data = model->LoadFile("../testing/data/t3148_1_cor.ddf"); 
    data->Update(); 
    fieldData->Initialize();
    fieldData->AddArray( data->GetCellData()->GetArray( "4 4 4" ) );
    dataStore->SetFieldData( fieldData );
    
    plot->AddDataObjectInput( dataStore ); 
    plot->SetDataObjectYComponent( 0,0);
    plot->SetPosition( 0.1,0.1);
    plot->SetPosition2( 0.8,0.8);
    plot->SetXRange( 150,350);
    plot->SetYRange( -60000000,140000000);
   
    renderer->AddActor( plot ) ;

    window->Render();
    window->GetInteractor()->Start();

    window->Delete();
    model->Delete();
    return 0;
}
