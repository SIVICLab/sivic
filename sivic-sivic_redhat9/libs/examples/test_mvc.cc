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


