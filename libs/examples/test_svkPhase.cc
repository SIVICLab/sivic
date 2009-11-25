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
#include <svkPhaseSpec.h>
#include <svkImageClip.h>
#include <vtkIndent.h>

using namespace svk;

int main (int argc, char** argv)
{

    vtkObject::SetGlobalWarningDisplay(1);

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

    cout << " Input svkImageData address" << *(reader->GetOutput()) << endl;

    vtkCellData* cellData = reader->GetOutput()->GetCellData(); 
    int numPoints = cellData->GetNumberOfTuples();
    int numVoxels[3]; 
    reader->GetOutput()->GetNumberOfVoxels(numVoxels); 


    int spectrum = 0;  
    for (int z = 0; z < numVoxels[2]; z++) {
        for (int y = 0; y < numVoxels[1]; y++) {
            for (int x = 0; x < numVoxels[0]; x++) {
                //cout << "spectrum: " << spectrum << " " << x << " " << y << " " << z << endl; 
                vtkFloatArray* specArray = static_cast<vtkFloatArray*>( 
                            cellData->GetArray( spectrum ) ); 
                for( int i = 0; i < numPoints; i++ ) {
                    //cout << "tuple: " << i << endl; 
                    (specArray->GetTuple(i))[0];
                }
                spectrum++; 
            }
        }
    }
    

    //  Apply algo to data from reader:
    svkPhaseSpec* phase = svkPhaseSpec::New();
    cout << " Input svkImageData address" << reader->GetOutput() << endl;
    phase->SetInput( reader->GetOutput() );
    phase->SetPhase0( 180 );
    phase->Update();

    //  Apply algo to data from reader:

    svkPhaseSpec* phase2 = svkPhaseSpec::New();
    cout << " PASS 2" << endl;
    phase2->SetInput( phase->GetOutput() );
    phase2->SetPhase0( 90 );
    phase2->Update();


    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter(svkImageWriterFactory::DICOM_MRS));
    writer->SetFileName("phased_mrs.dcm" );

    //  This only works becuase phase is an in place filter (i think)
    writer->SetInput( phase2->GetOutput() );
    writer->Write();

    phase->Delete(); 
    phase2->Delete(); 
    writer->Delete();
    reader->Delete();

    return 0; 
}

