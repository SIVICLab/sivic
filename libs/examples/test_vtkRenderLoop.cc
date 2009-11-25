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
#include <vtkRenderWindowInteractor.h>

int main ( int argc, char** argv )
{
    vtkRenderer* renderer = vtkRenderer::New();
    vtkRenderWindow* window = vtkRenderWindow::New(); 
    vtkRenderWindowInteractor* rwi = vtkRenderWindowInteractor::New();

    window->SetInteractor( rwi );
    window->AddRenderer( renderer );
    window->Render();
    
    renderer->Delete();
    window->Delete();
    rwi->Delete();

    return 0;
}
