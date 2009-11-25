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
 *  Test driver for svkMrsImageFFT. 
 *
 */

#include <svkDataModel.h>
#include <svkImageData.h>
#include <svkMultiCoilPhase.h>
#include <svkPlotGridView.h>
#include <vtkRenderWindow.h>

#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>

using namespace svk;

int main (int argc, char** argv)
{

    vtkObject::SetGlobalWarningDisplay(1);
    string fname(argv[1]);

    svkDataModel* model = svkDataModel::New();
    svkImageData* data = model->LoadFile( fname.c_str() );

    if( !data->IsA("svkMrsImageData") ) {
        cerr << "INPUT MUST BE SPECTRA!" << endl;
        exit(1);
    }

    svkPlotGridViewController* plotGridInput = svkPlotGridViewController::New(); 
    vtkRenderWindow* window = vtkRenderWindow::New();
    plotGridInput->SetRWInteractor( window->MakeRenderWindowInteractor() );
/*
    plotGridInput->SetInput( data, svkPlotGridView::MRS);
    plotGridInput->SetSlice(3);
    plotGridInput->SetWindowLevelRange(-10000000, 26000000, svkPlotGridView::AMPLITUDE);
    window->GetInteractor()->Start();
*/   
    svkMultiCoilPhase* mcPhase = svkMultiCoilPhase::New();
    mcPhase->SetInput( data );
    cout << "UPDATE IT: " << endl;
    mcPhase->Update(); 
    
    svkImageData* outputData = mcPhase->GetOutput();
    outputData->Update(); 

    plotGridInput->SetInput( data, svkPlotGridView::MRS);
    plotGridInput->SetSlice(3);
    plotGridInput->SetComponent(svkBoxPlot::MAGNITUDE);
    plotGridInput->SetWindowLevelRange(0, 2400000000, svkPlotGridView::AMPLITUDE);
    window->GetInteractor()->Start();

    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter(svkImageWriterFactory::DICOM_MRS));
    writer->SetFileName("phased.dcm" );

    writer->SetInput( data );
    writer->Write();

    writer->Delete();
    writerFactory->Delete();
     
    return 0; 
}

