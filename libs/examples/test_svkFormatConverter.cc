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
 *  A widget for phasing and viewing data.
 *
 *
 */

#include <svkDataModel.h>
#include <svkImageData.h>
#include <svkImageWriter.h>
#include <svkImageWriterFactory.h>

using namespace svk;

int main ( int argc, char** argv )
{
    svkDataModel* model = svkDataModel::New();
    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer;

    // Load a file....
    svkImageData* data = model->LoadFile("../testing/data/t3148_1_cor.ddf"); 
    writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter(svkImageWriterFactory::DICOM_MRS));
    writer->SetFileName("t3148_1_cor.dcm");
    writerFactory->Delete();

    writer->SetInput( data );

    writer->Write();
    writer->Delete();

    model->Delete();
    return 0;
}
