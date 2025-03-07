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
#include <svkMultiCoilPhase.h>
#include <svkPlotGridView.h>
#include <vtkRenderWindow.h>
#include <svkPlotLine.h>

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

/*
    svkPlotGridViewController* plotGridInput = svkPlotGridViewController::New(); 
    vtkRenderWindow* window = vtkRenderWindow::New();
    plotGridInput->SetRWInteractor( window->MakeRenderWindowInteractor() );
    plotGridInput->SetInput( data, svkPlotGridView::MRS);
    plotGridInput->SetSlice(3);
    plotGridInput->SetWindowLevelRange(-10000000, 26000000, svkPlotGridView::AMPLITUDE);
    window->GetInteractor()->Start();
*/   
    data->Update();
    svkMultiCoilPhase* mcPhase = svkMultiCoilPhase::New();
    mcPhase->SetInput( data );
    cout << "UPDATE IT: " << endl;
    mcPhase->Update(); 
    
    svkImageData* outputData = mcPhase->GetOutput();
    outputData->Update(); 
/*
    plotGridInput->SetInput( data, svkPlotGridView::MRS);
    plotGridInput->SetSlice(3);
    plotGridInput->SetComponent(svkPlotLine::MAGNITUDE);
    plotGridInput->SetWindowLevelRange(0, 2400000000, svkPlotGridView::AMPLITUDE);
    window->GetInteractor()->Start();

    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter(svkImageWriterFactory::DICOM_MRS));
    writer->SetFileName("phased.dcm" );

    writer->SetInput( data );
    writer->Write();

    writer->Delete();
    writerFactory->Delete();
*/
     
    return 0; 
}

