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
#include <vtkImageViewer2.h>
#include <time.h>

using namespace svk;

int main (int argc, char** argv)
{

    vtkObject::SetGlobalWarningDisplay(1);
    string fname(argv[1]);

    cout << "Testing image extracting on " << fname.c_str() << endl;
    
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
    vtkRenderWindow* gridWindow = vtkRenderWindow::New();
    plotGridInput->SetRWInteractor( gridWindow->MakeRenderWindowInteractor() );
    plotGridInput->SetInput( data, svkPlotGridView::MRS);
    plotGridInput->SetSlice(3);
    plotGridInput->GetView()->SetOrientation( svkDcmHeader::AXIAL );
    time_t time_before;
    time_t time_after;

    time_before = time (NULL);

    for( int i = 0; i < 512; i++ ) {
        svkMriImageData* pointImage =  svkMriImageData::New();
        svkMrsImageData::SafeDownCast( data )->GetImage( pointImage, i, 0, 0, 2, "" );
    }
    time_after = time (NULL);

    cout << "Time for get: " << time_after-time_before << endl;

    time_before = time (NULL);
    svkMriImageData* pointImage =  svkMriImageData::New();
    svkMrsImageData::SafeDownCast( data )->GetImage( pointImage, 187, 0, 0, 2, "" );
    for( int i = 0; i < 512; i++ ) {
        svkMrsImageData::SafeDownCast( data )->SetImage( pointImage, i );
    }
    time_after = time (NULL);
    cout << "Time for get and set: " << time_after-time_before << endl;
    

    vtkRenderWindow* imageWindow = vtkRenderWindow::New();
    vtkImageViewer2* viewer = vtkImageViewer2::New();
    viewer->GetImageActor()->InterpolateOff();
    viewer->SetupInteractor( imageWindow->MakeRenderWindowInteractor() );
    viewer->SetInput( pointImage );
    viewer->GetRenderer()->ResetCamera( );
    viewer->Render(); 
    

    cout << "Now lets visualize the output." << endl;
    //gridWindow->GetInteractor()->Start();
    gridWindow->Render();
    imageWindow->GetInteractor()->Start();

    vtkImageFFT* imageFFT = vtkImageFFT::New();
    imageFFT->SetInput( pointImage );
    imageFFT->Update( );
    viewer->SetInput( imageFFT->GetOutput() );

    svkMriImageData* pointImage2 =  svkMriImageData::New();
    svkMrsImageData::SafeDownCast( data )->GetImage( pointImage2, 90, 0, 0, 2, "" );
    svkMrsImageData::SafeDownCast(data)->SetImage( pointImage, 90 );
    plotGridInput->GetView()->Refresh();
    gridWindow->GetInteractor()->Start();
    plotGridInput->SetComponent(svkPlotLine::IMAGINARY);
    plotGridInput->GetView()->Refresh();
    gridWindow->GetInteractor()->Start();
      
     
    return 0; 
}

