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
