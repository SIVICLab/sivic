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
#include <svkMrsImageFFT.h>
#include <svkPlotGridView.h>
#include <vtkRenderWindow.h>

using namespace svk;

int main (int argc, char** argv)
{

    vtkObject::SetGlobalWarningDisplay(1);
    string fname(argv[1]);

    cout << "Testing svkMrsImageFFT on " << fname.c_str() << endl;
    
    svkDataModel* model = svkDataModel::New();


    cout << "Loading file....." << endl;
    svkImageData* data = model->LoadFile( fname.c_str() );

    cout << "Checking to make sure its an MrsImageData....." << endl;
    if( !data->IsA("svkMrsImageData") ) {
        cerr << "INPUT MUST BE SPECTRA!" << endl;
        exit(1);
    }

    // Lets look at the data...

    svkPlotGridViewController* plotGridInput = svkPlotGridViewController::New(); 
    vtkRenderWindow* window = vtkRenderWindow::New();
    plotGridInput->SetRWInteractor( window->MakeRenderWindowInteractor() );
    plotGridInput->SetInput( data, svkPlotGridView::MRS);
    plotGridInput->SetSlice(3);
    plotGridInput->SetWindowLevelRange(-10000000, 26000000, svkPlotGridView::AMPLITUDE);

    //cout << "Printing output data: " << *data << endl;
    window->GetInteractor()->Start();
    
    cout << "Instantiating the algorithm..." << endl;
    svkMrsImageFFT* imageFFT = svkMrsImageFFT::New();
   
    cout << "Setting input to the algorithm..." << endl;
    imageFFT->SetInput( data );
    
    cout << "Getting the output of the algorithm..." << endl;
    svkImageData* outputData = imageFFT->GetOutput();

    cout << "Updating the output of the algorithm..." << endl;
    int updateExtentStart[3] = {0,0,0};
    int updateExtentEnd[3] = {9,9,6};
    imageFFT->SetUpdateExtent( updateExtentStart, updateExtentEnd );
    imageFFT->Update();
    outputData->Update(); 

    //cout << "Printing output data: " << *outputData << endl;

    cout << "Now lets visualize the output." << endl;
    plotGridInput->SetComponent(svkBoxPlot::MAGNITUDE);
    //plotGridInput->SetWindowLevelRange(0, 8000000000, svkPlotGridView::AMPLITUDE);
    window->GetInteractor()->Start();
    
     
    return 0; 
}

