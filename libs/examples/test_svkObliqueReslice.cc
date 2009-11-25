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
 *  Test driver for DICOM MRS writer:  Converts a ddf to a DICOM MRS mutli frame object. 
 *
 */


#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkImageWriterFactory.h>
#include <svkIdfVolumeWriter.h>
#include <svkDcmHeader.h>
#include <svkObliqueReslice.h>

using namespace svk;

int main (int argc, char** argv)
{

    vtkObject::SetGlobalWarningDisplay(2);

    //  Input image and image with target orientation 
    string fname(argv[1]);
    string target(argv[2]);

    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2(fname.c_str());
    reader->SetFileName( fname.c_str() );
    reader->Update(); 
cout << "READER OUTPUT: " << fname << " " << *( reader->GetOutput() ) << endl;

    svkObliqueReslice* reslicer = svkObliqueReslice::New();
    reslicer->SetInput( reader->GetOutput() ); 

    reader = readerFactory->CreateImageReader2(target.c_str());
    reader->SetFileName( target.c_str() );
    reader->Update(); 
    reslicer->SetTargetDcosFromImage( reader->GetOutput() ); 
    reslicer->Update();

    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter( svkImageWriterFactory::IDF ));
    writer->SetFileName("resliced_output" );

    writer->SetInput( reslicer->GetOutput() );
    writer->Write();
    writer->Delete();

    return 0; 
}

