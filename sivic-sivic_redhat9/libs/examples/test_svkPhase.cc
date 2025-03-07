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

