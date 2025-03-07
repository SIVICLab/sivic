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
 *  This test is designed to show that there is a memory leak in vtk
 *  according to valgrind. These EXACT values comes up in our other
 *  valgrind tests, and this shows that it is not internal.
 *
 *  Notice that if you do not make the window->Render() call, then
 *  there is no memory leak nor errors
 *
 *
 */

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkGL2PSExporter.h>
#include <vtkRenderWindowInteractor.h>
#include "vtkConeSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkCamera.h"
#include "vtkActor.h"
#include "vtkRenderer.h"

int main ( int argc, char** argv )
{
    vtkConeSource *cone = vtkConeSource::New();
    cone->SetHeight( 3.0 );
    cone->SetRadius( 1.0 );
    cone->SetResolution( 10 );
  
    vtkPolyDataMapper *coneMapper = vtkPolyDataMapper::New();
    coneMapper->SetInputConnection( cone->GetOutputPort() );

    vtkActor *coneActor = vtkActor::New();
    coneActor->SetMapper( coneMapper );

    vtkRenderer *ren1= vtkRenderer::New();
    ren1->AddActor( coneActor );
    ren1->SetBackground( 0.1, 0.2, 0.4 );

    vtkRenderWindow *renWin = vtkRenderWindow::New();
    renWin->AddRenderer( ren1 );
    renWin->SetSize( 300, 300 );
    //vtkRenderWindow *renWin2 = vtkRenderWindow::New();
    //renWin2->AddRenderer( ren1 );
    //renWin2->SetSize( 300, 600 );

    vtkRenderWindowInteractor* rwi = vtkRenderWindowInteractor::New();

    renWin->SetInteractor(rwi);
    renWin->Render();

    vtkGL2PSExporter* epsWriter = vtkGL2PSExporter::New();
    epsWriter->SetRenderWindow(renWin);
    epsWriter->SetFilePrefix("testGraphics");
    epsWriter->SetFileFormatToPDF();
    epsWriter->CompressOff();
    epsWriter->Write3DPropsAsRasterImageOn();
/*
      for (int i = 0; i < 360; ++i)
    {
    // render the image
    renWin2->Render();
    // rotate the active camera by one degree
    ren1->GetActiveCamera()->Azimuth( 1 );
    }
*/
    rwi->Start(); 
    epsWriter->Write();


    cone->Delete();
    coneMapper->Delete();
    coneActor->Delete();
    ren1->Delete();
    renWin->Delete();

    return 0;
}


