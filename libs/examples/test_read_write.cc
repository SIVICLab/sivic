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


#include <vtkImageReader2.h>
#include <vtkImageData.h>
#include <vtkImageWriter.h>

#include <vtkImageViewer2.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageActor.h>
#include <vtkLookupTable.h>

#include <svkImageReaderFactory.h>
#include <svkImageData.h>
// #include <svkImageWriterFactory.h>
// #include <svkMultiWindowToImageFilter.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkImageMapToWindowLevelColors.h>

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
    svkImageData* imageData = reader->GetOutput(); 
    cout << "DELETE READER START" << endl;
    //reader->Delete();
    cout << "DELETE READER END" << endl << endl << endl;
/*
    cout << "reader output(svkImageData) " <<  *(reader->GetOutput()) << endl;
    cout << "reader output Data Object (vtkImageData) " <<  *(reader->GetOutputDataObject(0)) << endl;

    cout << "svkDcmHeader " <<  *(imageData->GetDcmHeader()) << endl;
    imageData->GetDcmHeader()->PrintHeader();
*/

    render(reader->GetOutput(), 32, 1);

    reader->Delete();


    return 0; 
}


void render(vtkImageData* data, int slice, int write)
{

    //cout << "=============================================" <<  endl;
    //cout << "=============================================" <<  endl;
    //cout << "RENDER: " << *(data) << endl;
    //cout << "=============================================" <<  endl;
    //cout << "=============================================" <<  endl;

    vtkImageViewer2* viewer = vtkImageViewer2::New();
    viewer->SetSlice(slice);
    viewer->SetInput( data );
    viewer->GetImageActor()->InterpolateOff();
    viewer->SetSliceOrientationToXY();
    
    viewer->SetSize(500,500);

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
    rwi->SetInteractorStyle(vtkInteractorStyleTrackballCamera::New());
    rwi->Start();
    rwi->Start();
    
    rwi->Delete();
    ren->Delete();
    renWin->Delete();
    viewer->Delete();
    
}


/*!
 *  Write out a multi window panel to a DICOM SC instance. 
 */
void writeFile(vtkRenderWindow* renWin)
{
/*
    renWin->Render();

    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    vtkImageWriter* writer = writerFactory->CreateImageWriter(DICOM_SC);
    writer->SetFileName("dicom_sc.dcm" );

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
    writer->Delete();

    mw2if->Delete();
*/
}

