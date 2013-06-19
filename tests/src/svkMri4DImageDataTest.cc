/*
 *  Copyright © 2009-2011 The Regents of the University of California.
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
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 *
 */


#include <svkDataModel.h>
#include <svkImageData.h>
#include <svkTestUtils.h>
#include <svkMriZeroFill.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <svkPlotGridViewController.h>
#include <svkUtils.h>
using namespace svk;

int main (int argc, char** argv)
{
    string filename(argv[1]);
    string outputPath(argv[2]);
    int tlcID = svkUtils::StringToInt( string(argv[3]));
    int brcID = svkUtils::StringToInt( string(argv[4]));
    int numSlices = svkUtils::StringToInt( string(argv[5]));

    svkDataModel* model = svkDataModel::New();

    svkImageData* data = model->LoadFile( filename.c_str() );
    data->Register(NULL); 
    data->Update();
    if( !data->IsA("svkMriImageData") ) {
        cerr << "INPUT MUST BE AN IMAGE!" << endl;
        data->Delete();
        exit(1);
    }

    vtkRenderWindow* window = vtkRenderWindow::New();
    vtkRenderWindowInteractor* rwi = window->MakeRenderWindowInteractor();
    svkPlotGridViewController* plotController = svkPlotGridViewController::New();
    window->SetSize( 640, 640 );
    plotController->SetRWInteractor( rwi );
    plotController->SetInput( svkMriImageData::SafeDownCast(data)->GetCellDataRepresentation(), 0 );
    int midSlice = data->GetNumberOfSlices()/2;
    int tlcBrc[2] = {tlcID,brcID};
    plotController->SetTlcBrc(tlcBrc);
    for( int i = midSlice - numSlices; i < midSlice + numSlices; i++ ) {
        stringstream outputName;
        outputName << outputPath << "/capture" << "_" << i << ".tiff" ;
        plotController->SetSlice( i );
        plotController->GetView()->Refresh();
        window->Render();
        svkTestUtils::SaveWindow( window, (outputName.str()).c_str() );
    }

    plotController->Delete();
    window->Delete();
    data->Delete();
    model->Delete();

    return 0; 
}
