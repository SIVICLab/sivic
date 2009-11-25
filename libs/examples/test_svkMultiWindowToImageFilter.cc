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
 *  Test driver for readers and writers.
 *  
 *  The following classes are utilized in this driver. 
 *      svkImageReaderFactory
 *      mrkIDFVolumeReader
 *      svkMultiWindowToImageFilter
 *      svkImageWriterFactory
 *      svkDICOMSCWriter
 *
 */


#include <vtkImageReader2.h>
#include <vtkImageData.h>
#include <vtkImageWriter.h>

#include <vtkImageViewer2.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageActor.h>

#include <svkImageReaderFactory.h>
#include <svkIdfVolumeReader.h>
#include <svkImageWriterFactory.h>
#include <svkMultiWindowToImageFilter.h>

using namespace svk;

void render(vtkImageData* data, int slice, int write);
void writeFile(vtkRenderWindow* renWin);


int main (int argc, char** argv)
{

    string fname(argv[1]);

    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();    
    svkImageReader2* reader = readerFactory->CreateImageReader2(fname.c_str());
    readerFactory->Delete(); 

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << fname << endl;
        exit(1);
    }

    reader->SetFileName( fname.c_str() );
    reader->Update(); 
    //cout << "reader output " <<  *(reader->GetOutput()) << endl;

    render(reader->GetOutput(), 32, 1);

    reader->Delete();

    return 1; 
}


void render(vtkImageData* data, int slice, int write)
{

    vtkImageViewer2* viewer = vtkImageViewer2::New();
    viewer->SetSlice(slice);
    viewer->SetInput( data );
    viewer->GetImageActor()->InterpolateOff();
    viewer->SetSliceOrientationToXY();
    
    viewer->SetSize(400,150);

    vtkRenderWindow* renWin = vtkRenderWindow::New();
    vtkRenderer* ren = vtkRenderer::New();
    renWin->AddRenderer(ren);
    vtkRenderWindowInteractor* rwi = vtkRenderWindowInteractor::New();
    rwi->SetRenderWindow(renWin);
    viewer->SetupInteractor(rwi);
    viewer->GetRenderer()->ResetCamera();

    if (write) {
        writeFile(viewer->GetRenderer()->GetRenderWindow());
    }


    rwi->Initialize();
    //rwi->Start();
    
    rwi->Delete();
    ren->Delete();
    renWin->Delete();
    viewer->Delete();
    
}


/*!
 *  Write out a multi window panel to a tiff. 
 */
void writeFile(vtkRenderWindow* renWin)
{

    renWin->Render();

    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    vtkImageWriter* writer = (writerFactory->CreateImageWriter(svkImageWriterFactory::TIFF));
    writerFactory->Delete();
    writer->SetFileName("image.tiff" );

    // Test multi-window writer:
    svkMultiWindowToImageFilter* mw2if = svkMultiWindowToImageFilter::New();
    mw2if->SetInput( renWin, 0, 1 );
    mw2if->SetInput( renWin, 1, 0 );
    mw2if->SetInput( renWin, 2, 1 );
    mw2if->Update();

    writer->SetInput( mw2if->GetOutput() );
    cout << "RENDER TEST APPENDED OUTPUT" << endl; 
    render(mw2if->GetOutput(), 0, 0);

    writer->Write();
    cout << " check 1" << endl;
    writer->Delete();
    cout << " check 2" << endl;

    mw2if->Delete();
    cout << " check 3" << endl;
}

