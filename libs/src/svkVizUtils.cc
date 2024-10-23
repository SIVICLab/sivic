/*
 *  Copyright © 2009-2017 The Regents of the University of California.
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


#include <svkVizUtils.h>
#include </usr/include/vtk/vtkGlobFileNames.h>

using namespace svk;

//vtkCxxRevisionMacro(svkVizUtils, "$Rev$");
vtkStandardNewMacro(svkVizUtils);

//! Constructor
svkVizUtils::svkVizUtils()
{
}


//! Destructor
svkVizUtils::~svkVizUtils()
{
}


/*
 * Saves the input window to a tiff image.
 *
 * \param window the window you want to save
 * \param filename the name you want to save it as
 *
 */
void svkVizUtils::SaveWindow( vtkWindow* window, string fileName )
{

    size_t pos = fileName.find_last_of(".");

    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    vtkImageWriter* writer = NULL;

    if( strcmp( fileName.substr(pos).c_str(), ".ps" ) == 0 ) {
        writer = writerFactory->CreateImageWriter( svkImageWriterFactory::PS );
    } else if( strcmp( fileName.substr(pos).c_str(), ".tiff" ) == 0 ) {
        writer = writerFactory->CreateImageWriter( svkImageWriterFactory::TIFF );
        vtkTIFFWriter* tmp = vtkTIFFWriter::SafeDownCast( writer );
        tmp->SetCompression(0);
    } else if( strcmp( fileName.substr(pos).c_str(), ".jpeg" ) == 0 ) {
        writer = writerFactory->CreateImageWriter( svkImageWriterFactory::JPEG );
        vtkJPEGWriter* tmp = vtkJPEGWriter::SafeDownCast( writer );
        tmp->SetQuality(100);
    }
    writerFactory->Delete();
    vtkWindowToImageFilter* w2i = vtkWindowToImageFilter::New();
    w2i->ReadFrontBufferOff();
    w2i->SetInput(window);
    w2i->Update();

    writer->SetInputData(w2i->GetOutput());
    writer->SetFileName( fileName.c_str() );
    writer->Write();
    writer->Delete();
    w2i->Delete();
}
