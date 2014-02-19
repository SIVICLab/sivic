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


#include <svkDataModel.h>
#include <svkImageData.h>
#include <svkMrsImageFFT.h>
#include <svkPlotGridView.h>
#include <vtkRenderWindow.h>
#include <vtkImageFourierCenter.h>
#include <svkDdfVolumeWriter.h>
#include <svkDICOMMRSWriter.h>

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
    double range[2];
/*
    svkPlotGridViewController* plotGridInput = svkPlotGridViewController::New(); 
    vtkRenderWindow* window = vtkRenderWindow::New();
    window->SetSize(800,800);
    plotGridInput->SetRWInteractor( window->MakeRenderWindowInteractor() );
    plotGridInput->SetInput( data, svkPlotGridView::MRS);
    plotGridInput->SetSlice(3);
    data->GetDataRange( range, 0 );
    cout << "range: " << range[0] << " " << range[1] << endl;
    plotGridInput->SetWindowLevelRange(range[0], range[1], svkPlotGridView::AMPLITUDE);
    plotGridInput->GetView()->SetOrientation(svkDcmHeader::AXIAL);
    plotGridInput->SetComponent(svkPlotLine::MAGNITUDE);
    plotGridInput->GetView()->Refresh();
    window->Render();


    window->GetInteractor()->Start();
*/

    // Display Input:
/*
    for( int i = 0; i < 8; i++ ) {
        plotGridInput->SetSlice(i);
        window->GetInteractor()->Start();
    }
*/
    svkImageData* outputData;
    svkDICOMMRSWriter* writer = svkDICOMMRSWriter::New();
    cout << "Instantiating the algorithm..." << endl;
    svkMrsImageFFT* spatialRFFT = svkMrsImageFFT::New();
   
    cout << "Setting input to the algorithm..." << endl;
    spatialRFFT->SetInput( data );
    spatialRFFT->SetFFTDomain( svkMrsImageFFT::SPATIAL );
    spatialRFFT->SetFFTMode( svkMrsImageFFT::REVERSE );
    spatialRFFT->SetPreCorrectCenter( true );
    spatialRFFT->SetPostCorrectCenter( true );
    
    cout << "Getting the output of the algorithm..." << endl;
    outputData = spatialRFFT->GetOutput();

    cout << "Updating the output of the algorithm..." << endl;
    spatialRFFT->Update();
    outputData->Modified();
    outputData->Update(); 

    writer->SetFileName( "svk_spatial_recon.dcm" );    
    writer->SetInput( outputData );
    writer->Write();
    writer->Delete();

    cout << "Now lets visualize the output." << endl;
    outputData->GetDataRange( range, 0 );
    cout << "range: " << range[0] << " " << range[1] << endl;
/*
    plotGridInput->SetWindowLevelRange(range[0], range[1], svkPlotGridView::AMPLITUDE);
    plotGridInput->GetView()->Refresh();
    window->Render();
    window->GetInteractor()->Start();
*/

    // Display Spatial Reconstructed
/*
    for( int i = 0; i < 8; i++ ) {
        plotGridInput->SetSlice(i);
        window->GetInteractor()->Start();
    }

*/
    cout << "Instantiating the algorithm..." << endl;
    svkMrsImageFFT* spectralFFT = svkMrsImageFFT::New();
   
    cout << "Setting input to the algorithm..." << endl;
    spectralFFT->SetInput( data );
    spectralFFT->SetFFTDomain( svkMrsImageFFT::SPECTRAL );
    spectralFFT->SetFFTMode( svkMrsImageFFT::FORWARD );
    
    cout << "Getting the output of the algorithm..." << endl;
    outputData = spectralFFT->GetOutput();

    cout << "Updating the output of the algorithm..." << endl;
    spectralFFT->Update();
    outputData->Modified();
    outputData->Update(); 

    writer = svkDICOMMRSWriter::New();
    writer->SetFileName( "svk_full_recon.dcm" );    
    writer->SetInput( outputData );
    writer->Write();

/*
    outputData->GetDataRange( range, 0 );
    plotGridInput->SetWindowLevelRange(range[0], range[1], svkPlotGridView::AMPLITUDE);
    plotGridInput->GetView()->Refresh();
    window->Render();
    window->GetInteractor()->Start();
*/


    // Display Final Spectra 
/*
    for( int i = 0; i < 8; i++ ) {
        plotGridInput->SetSlice(i);
        window->GetInteractor()->Start();
    }
*/
    
    
     
    return 0; 
}

