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
 *
 *
 *  Test driver for mrk MVC
 *  
 *  The following classes are utilized in this driver. 
 *      svkDataView
 *      svkDataViewController
 *      svkImageView2D
 *      svkImageView2DController
 *      
 *      svkImageReaderFactory
 *      svkImageReader2
 *      svkIdfVolumeReader
 *
 */


#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#include <svkImageReaderFactory.h>
#include <svkImageView2DController.h>
#include <svkImageView2D.h>

using namespace svk;

int main (int argc, char** argv)
{

    string fname(argv[1]);

    //===================================================
    //  Get Reader and load data from file:      
    //===================================================
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();    
    svkImageReader2* reader = readerFactory->CreateImageReader2(fname.c_str());
    readerFactory->Delete(); 

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << fname << endl;
        exit(1);
    }

    reader->SetFileName( fname.c_str() );
    reader->Update(); 


    //===================================================
    //   MVC: Load image data into 2D image viewer:
    //===================================================
    vtkRenderWindow*           renWin = vtkRenderWindow::New();
    vtkRenderWindowInteractor* rwi    = vtkRenderWindowInteractor::New();

    rwi->SetRenderWindow(renWin);

    svkImageView2D*           iv  = svkImageView2D::New();
    svkImageView2DController* ivc = svkImageView2DController::New();
    ivc->SetView( iv );
    iv->SetController( ivc );

    ivc->SetInput( reader->GetOutput() );
    ivc->SetRWInteractor(rwi);
    ivc->SetSlice(20);
    //ivc->SetSize(400, 400);
    //===================================================

    rwi->Initialize();
    rwi->Start();

    iv->Delete();
    ivc->Delete();
    renWin->Delete();
    rwi->Delete();
    reader->Delete();

    return 0; 
}


