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
