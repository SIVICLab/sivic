/*
 *  Copyright © 2009-2012 The Regents of the University of California.
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
 *  Utility application for phasing spectroscopic data. 
 *
 */

#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkPhaseSpec.h>

#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDcmHeader.h>
#include <svkPhaseSpec.h>

#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#endif

#define UNDEFINED -1

using namespace svk;



int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "svkZeroLinearPhaseTest -i input_file_name  -o output_root]\n";
    usemsg += "   -i        name    Name of file to phase.                                  \n"; 
    usemsg += "   -o        name    Name of output file.                                    \n"; 
    usemsg += "Applies zero order and/or linear phase test. \n"; 
    usemsg += "\n";  


    string inputFileName; 
    string outputFileName;
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DDF;


    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0; 
    while ( ( i = getopt(argc, argv, "i:o:") ) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            default:
                ;
        }
    }

    // ===============================================  
    //  validate input: 
    // ===============================================  
    if ( 
        outputFileName.length() == 0 ||
        inputFileName.length() == 0 
    ) {
			cout << "ERROR with arguments!" << endl;
            cout << usemsg << endl;
            exit(1); 
    }

    cout << "input file name: " << inputFileName << endl;

    // ===============================================  
    //  Use a reader factory to get the correct reader  
    //  type and load data. 
    // ===============================================  
    vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }

    reader->SetFileName( inputFileName.c_str() );
    reader->Update(); 

    //  Phase the data.: 
    svkPhaseSpec* phaser = svkPhaseSpec::New();
    svkMrsImageData* mrsData = svkMrsImageData::New();
    mrsData->DeepCopy( svkMrsImageData::SafeDownCast(reader->GetOutput()) );
    phaser->SetInput( mrsData );
    phaser->SetChannel( 0 );

    phaser->SetLinearPhasePivot( 512 );
    phaser->Update();
    phaser->SetPhase0( 32 );
    phaser->Update();
    phaser->SetLinearPhase( 1094.0 / -360.0 );
    phaser->Update();
    phaser->SetLinearPhasePivot( 333 );
    phaser->SetLinearPhasePivot( 111 );
    phaser->SetPhase0( 10  );
    phaser->Update();
    phaser->SetLinearPhase( -1511.0 / -360.0  );
    phaser->Update();
    phaser->SetLinearPhasePivot( 47 );
    phaser->Update();
    phaser->SetPhase0( 99  );
    phaser->Update();
    phaser->SetLinearPhase( 733.0 / -360.0  );
    phaser->Update();

    // ===============================================  
    //  Write the data out to the specified file type.  
    //  Use an svkImageWriterFactory to obtain the
    //  correct writer type. 
    // ===============================================  
    vtkSmartPointer< svkImageWriterFactory > writerFactory = vtkSmartPointer< svkImageWriterFactory >::New(); 
    svkImageWriter* writer = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 
    if ( writer == NULL ) {
        cerr << "Can not determine writer of type: " << dataTypeOut << endl;
        exit(1);
    }

    writer->SetFileName( outputFileName.c_str() );
    writer->SetInput( mrsData );

    // ===============================================  
    //  Write data to file: 
    // ===============================================  
    writer->Write();

    // ===============================================  
    //  Clean up: 
    // ===============================================  
    writer->Delete();
    reader->Delete();
    phaser->Delete(); 
    mrsData->Delete();

    return 0; 
}

