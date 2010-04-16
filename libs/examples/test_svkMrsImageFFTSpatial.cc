/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/libs/examples/test_svkMrsImageFFT.cc $
 *  $Rev: 77 $
 *  $Author: jccrane $
 *  $Date: 2010-01-26 11:16:55 -0800 (Tue, 26 Jan 2010) $
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

    svkPlotGridViewController* plotGridInput = svkPlotGridViewController::New(); 
    vtkRenderWindow* window = vtkRenderWindow::New();
    window->SetSize(800,800);
    plotGridInput->SetRWInteractor( window->MakeRenderWindowInteractor() );
    plotGridInput->SetInput( data, svkPlotGridView::MRS);
    plotGridInput->SetSlice(3);
    double range[2];
    data->GetDataRange( range, 0 );
    cout << "range: " << range[0] << " " << range[1] << endl;
    plotGridInput->SetWindowLevelRange(range[0], range[1], svkPlotGridView::AMPLITUDE);
    plotGridInput->GetView()->SetOrientation(svkDcmHeader::AXIAL);
    plotGridInput->SetComponent(svkPlotLine::MAGNITUDE);
    //plotGridInput->SetWindowLevelRange(0, 1023, svkPlotGridView::FREQUENCY);
    plotGridInput->GetView()->Refresh();
    window->Render();


    //cout << "Printing output data: " << *data << endl;
    window->GetInteractor()->Start();
    for( int i = 0; i < 8; i++ ) {
        plotGridInput->SetSlice(i);
        window->GetInteractor()->Start();
    }
    svkImageData* outputData;
    cout << "Instantiating the algorithm..." << endl;
    svkMrsImageFFT* imageFFT = svkMrsImageFFT::New();
   
    cout << "Setting input to the algorithm..." << endl;
    imageFFT->SetInput( data );
    imageFFT->SetFFTDomain( svkMrsImageFFT::SPATIAL );
    imageFFT->SetFFTMode( svkMrsImageFFT::REVERSE );
    imageFFT->SetPreCorrectCenter( true );
    imageFFT->SetPostCorrectCenter( true );
    
    cout << "Getting the output of the algorithm..." << endl;
    outputData = imageFFT->GetOutput();

    cout << "Updating the output of the algorithm..." << endl;
    imageFFT->Update();
    outputData->Modified();
    outputData->Update(); 

    svkMrsImageFFT* imageFFT2 = svkMrsImageFFT::New();
   
    cout << "Setting input to the algorithm..." << endl;
    imageFFT2->SetInput( data );
    imageFFT2->SetFFTDomain( svkMrsImageFFT::SPATIAL );
    imageFFT2->SetFFTMode( svkMrsImageFFT::REVERSE );
    imageFFT2->phaseOnly = true;;
    
    cout << "Getting the output of the algorithm..." << endl;
    outputData = imageFFT2->GetOutput();

    cout << "Updating the output of the algorithm..." << endl;
    imageFFT2->Update();
    outputData->Modified();
    outputData->Update(); 

    //Lets write out the volume:
    ///svkDdfVolumeWriter* writer = svkDdfVolumeWriter::New();


    cout << "Now lets visualize the output." << endl;
    outputData->GetDataRange( range, 0 );
    cout << "range: " << range[0] << " " << range[1] << endl;
    plotGridInput->SetWindowLevelRange(range[0], range[1], svkPlotGridView::AMPLITUDE);
    //plotGridInput->SetWindowLevelRange(0, 1023, svkPlotGridView::FREQUENCY);
    plotGridInput->GetView()->Refresh();
    window->Render();
    window->GetInteractor()->Start();
    for( int i = 0; i < 8; i++ ) {
        plotGridInput->SetSlice(i);
        window->GetInteractor()->Start();
    }

    cout << "Instantiating the algorithm..." << endl;
    svkMrsImageFFT* imageRFFT = svkMrsImageFFT::New();
   
    cout << "Setting input to the algorithm..." << endl;
    imageRFFT->SetInput( data );
    imageRFFT->SetFFTDomain( svkMrsImageFFT::SPECTRAL );
    imageRFFT->SetFFTMode( svkMrsImageFFT::FORWARD );
    
    cout << "Getting the output of the algorithm..." << endl;
    outputData = imageRFFT->GetOutput();

    cout << "Updating the output of the algorithm..." << endl;
    imageRFFT->Update();
    outputData->Modified();
    outputData->Update(); 

    svkDICOMMRSWriter* writer = svkDICOMMRSWriter::New();
    writer->SetFileName( "svk_recon_phase_neg_half_new.dcm" );    
    writer->SetInput( outputData );
    writer->Write();


    outputData->GetDataRange( range, 0 );
    plotGridInput->SetWindowLevelRange(range[0], range[1], svkPlotGridView::AMPLITUDE);
    //plotGridInput->SetWindowLevelRange(0, 1023, svkPlotGridView::FREQUENCY);
    plotGridInput->GetView()->Refresh();
    window->Render();
    window->GetInteractor()->Start();
    for( int i = 0; i < 8; i++ ) {
        plotGridInput->SetSlice(i);
        window->GetInteractor()->Start();
    }
    
    
     
    return 0; 
}

