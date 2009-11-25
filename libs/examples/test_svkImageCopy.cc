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
 */


#include <svkImageReaderFactory.h>
#include <svkImageWriterFactory.h>
#include <svkImageReader2.h>
#include <svkImageWriter.h>
#include <svkImageData.h>
#include <svkImageCopy.h>

using namespace svk;

int main (int argc, char** argv)
{

    //vtkObject::SetGlobalWarningDisplay(0);

    string fname(argv[1]);

    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2(fname.c_str());

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << fname << endl;
        exit(1);
    }

    reader->SetFileName( fname.c_str() );
    reader->Update(); 

    svkImageData* imageIn = reader->GetOutput();

    cout << "HEADER ORIGINAL: " << endl;
    imageIn->GetDcmHeader()->PrintDcmHeader();

    svkImageCopy* copier = svkImageCopy::New();
    copier->SetInput( imageIn );
    copier->SetSeriesDescription( "MY NEW SERIES" );
    //copier->SetOutputDataType( svkDcmHeader::UNSIGNED_INT_1 );
    copier->SetOutputDataType( svkDcmHeader::UNSIGNED_INT_2 );
    copier->Update();

    cout << "HEADER COPY: " << endl;
    copier->GetOutput()->GetDcmHeader()->PrintDcmHeader();

    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter( svkImageWriterFactory::IDF ));
    writer->SetFileName("image_copy_out" );
    writer->SetInput( copier->GetOutput() );
    writer->Write();
    writer->Delete();
    writerFactory->Delete();
    reader->Delete();
    readerFactory->Delete();

    return 0; 
}

