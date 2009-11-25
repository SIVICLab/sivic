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
 *      svkIdfVolumeReader
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
#include <svkImageData.h>
// #include <svkImageWriterFactory.h>
// #include <svkMultiWindowToImageFilter.h>


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
    //reader->Delete();


    imageData->Delete();
    reader->Delete();


    return 0; 
}


