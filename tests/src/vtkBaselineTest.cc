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
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkConeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkActor.h>
#include <vtkTextActor.h>
#include <vtkRenderer.h>
#include <vtkTIFFWriter.h>
#include <svkVizUtils.h>

using namespace svk;

int main ( int argc, char** argv )
{
    
    vtkConeSource *cone = vtkConeSource::New();
    cone->SetHeight( 3.0 );
    cone->SetRadius( 1.0 );
    cone->SetResolution( 10 );
  
    vtkPolyDataMapper *coneMapper = vtkPolyDataMapper::New();
    coneMapper->SetInput( cone->GetOutput() );

    vtkActor *coneActor = vtkActor::New();
    coneActor->SetMapper( coneMapper );

    vtkTextActor* text = vtkTextActor::New();
    text->SetTextScaleModeToProp();
    text->SetInput("x: null \ny: null \n z: null");
    text->SetLayerNumber(1);

    text->SetPosition(0.1,0.1);
    text->SetPosition2(0.9,0.9);
    text->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    text->GetPosition2Coordinate()->SetCoordinateSystemToNormalizedViewport();



    vtkRenderer *ren= vtkRenderer::New();
    ren->AddActor( coneActor );
    ren->AddActor(text);
    ren->SetBackground( 0.1, 0.2, 0.4 );

    vtkRenderWindow *renWin = vtkRenderWindow::New();
    renWin->AddRenderer( ren );
    renWin->SetSize( 300, 300 );
    renWin->SetNumberOfLayers(2);

    for( int i = 0; i < 100; i++ ) {

        renWin->Render();
    
    }
    svkVizUtils::SaveWindow( renWin, argv[1] );

  
    cone->Delete();
    coneMapper->Delete();
    coneActor->Delete();
    ren->Delete();
    renWin->Delete();
    text->Delete();

    return 0;
}
