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


#include <vtkIndent.h>

#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkIdfVolumeReader.h>
#include <svkDdfVolumeReader.h>
#include <svkDcmVolumeReader.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDICOMMRSWriter.h>
#include <svkIdfVolumeWriter.h>
#include <svkDcmHeader.h>


using namespace svk;

int main (int argc, char** argv)
{

    vtkObject::SetGlobalWarningDisplay(1);

    string fname(argv[1]);

    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2(fname.c_str());

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << fname << endl;
        exit(1);
    }

    reader->SetFileName( fname.c_str() );
    reader->Update(); 

    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer = static_cast<svkImageWriter*>(
                                writerFactory->CreateImageWriter(svkImageWriterFactory::IDF )
                             );
    writer->SetFileName("idf_out" );

    writer->SetInput( reader->GetOutput() );
    writer->Write();

    writer->Delete(); 
    reader->Delete(); 
    readerFactory->Delete(); 
    writerFactory->Delete(); 

    return 0; 
}

