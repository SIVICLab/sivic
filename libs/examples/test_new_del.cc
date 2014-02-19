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


#include <svkImageReaderFactory.h>
#include <svkIdfVolumeReader.h>
#include <svkImageData.h>

#include <vtkBoxClipDataSet.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellArray.h>
#include <vtkPointSet.h>

#include <vtkDataSetMapper.h>
#include <vtkActor.h>

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>

using namespace svk;

int main (int argc, char** argv)
{
    string fname(argv[1]);

    svkImageData* id; 

    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2( fname.c_str() );
    reader->SetFileName( fname.c_str() );
    reader->Update();
    id = reader->GetOutput();


    vtkBoxClipDataSet* volSel = vtkBoxClipDataSet::New();
    volSel->GenerateClippedOutputOn();
    volSel->SetInput(id);

    //  Z (up/down)
    //  x (l/r)
    //  Y(in/out of screen)

    //  Bottom floor
    double p1 = (0., 0., -1.);
    double n1 = (0., 0.,  1.);

    // Right Wall 
    double p2 = (10., 0., 0.);
    double n2 = (1., 0., 0.);

    // Back Wall 
    double p3 = (0., 15., 0.);
    double n3 = (0., 1., 0.);

    // Front Wall 
    double p4 = (0., -15., 0.);
    double n4 = (0., 1., 0.);

    // Left Wall 
    double p5 = (-10., 0., 0.);
    double n5 = (1., 0., 0.);

    //  top ceiling
    double p6 = (0., 0., 1.);
    double n6 = (0., 0., 1.);

    volSel->SetBoxClip(&n1, &p1, &n2, &p2, &n3, &p3, &n4, &p4, &n5, &p5, &n6, &p6);
    volSel->Update();
    vtkUnstructuredGrid* boxGrid = volSel->GetClippedOutput();
    //cout << *volSel << endl;
    cout << *boxGrid << endl;

    vtkDataSetMapper* selBoxMapper = vtkDataSetMapper::New();
    selBoxMapper->SetInput(boxGrid);
    vtkActor* selBoxActor = vtkActor::New();
    selBoxActor->SetMapper(selBoxMapper);

// Create the usual rendering stuff.
vtkRenderer* ren = vtkRenderer::New();
vtkRenderWindow* renWin = vtkRenderWindow::New();
renWin->AddRenderer(ren);
renWin->SetSize(200, 200);
vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::New();
iren->SetRenderWindow(renWin);

ren->SetBackground(.1, .2, .4);

ren->AddActor(selBoxActor);

ren->ResetCamera();
//ren->GetActiveCamera()->Azimuth(30);
//ren->GetActiveCamera()->Elevation(20);
//ren->GetActiveCamera()->Dolly(2.8);
ren->ResetCameraClippingRange();

// Render the scene and start interaction.
iren->Initialize();
renWin->Render();
iren->Start();


/*
    string fname(argv[1]);

    svkImageData* id; 

    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    readerFactory->Delete();
    svkImageReader2* reader = readerFactory->CreateImageReader2( fname.c_str() );
    readerFactory->Delete();
    readerFactory = NULL;

    reader->SetFileName( fname.c_str() );
    reader->Update();
    id = reader->GetOutput();
    reader->Delete();
    reader = NULL;

    cout << *id << endl;
    cout << *(id) << endl;
    cout << *(id->GetDcmHeader()) << endl;
    id->Delete();
    id = NULL;
*/

    return 0; 
}


