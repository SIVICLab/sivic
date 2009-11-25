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
#include <svkIdfVolumeReader.h>
#include <svkDdfVolumeReader.h>
#include <svkDcmVolumeReader.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDICOMMRSWriter.h>
#include <svkDcmHeader.h>
#include <vtkIndent.h>

using namespace svk;

int main (int argc, char** argv)
{

    vtkObject::SetGlobalWarningDisplay(2);

    string fname(argv[1]);

    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2(fname.c_str());

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << fname << endl;
        exit(1);
    }

    reader->SetFileName( fname.c_str() );
    reader->Update(); 
    cout << *( reader->GetOutput() ) << endl;

    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter(svkImageWriterFactory::DICOM_MRS));
    writer->SetFileName("dicom_mrs.dcm" );

    writer->SetInput( reader->GetOutput() );
    writer->Write();

    writer->Delete();
    writerFactory->Delete();
    reader->Delete();
    readerFactory->Delete(); 

    return 0; 
}

