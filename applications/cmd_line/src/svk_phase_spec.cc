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
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_phase_spec -i input_file_name [-z zero_order_phase -l linear_phase -p pivot_point";
    usemsg += " -t output_data_type -o output_root]\n";
    usemsg += "   -i        name    Name of file to phase.                                  \n"; 
    usemsg += "   -f        number  Zero order phase value.                                 \n"; 
    usemsg += "   -l        number  Linear phase value.                                     \n";
    usemsg += "   -p        integer Linear phase pivot point.                               \n";
    usemsg += "   -t        type    Target data type:                                       \n";
    usemsg += "                         2 = UCSF DDF                                        \n";
    usemsg += "                         4 = DICOM_MRS (default)                             \n";
    usemsg += "   -h                Print this help mesage.                                 \n";  
    usemsg += "\n";  
    usemsg += "Applies zero order and/or linear phase to a dataset. \n"; 
    usemsg += "\n";  


    string inputFileName; 
    string outputFileName;
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_MRS;

    float zeroPhase = 0.0;
    float linearPhase = 0.0;
    int pivotPoint = -1;
    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 


    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0; 
    while ( ( i = getopt(argc, argv, "i:z:l:p:o:t:h") ) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 't':
                dataTypeOut = static_cast<svkImageWriterFactory::WriterType>( atoi(optarg) );
                break;
            case 'z':
                zeroPhase = atoi(optarg); 
                break;
            case 'l':
                linearPhase = atoi(optarg); 
                linearPhase /= -360.0;
                break;
            case 'p':
                pivotPoint = atoi(optarg); 
                break;
            case 'h':
                cout << usemsg << endl;
                exit(1);  
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
        inputFileName.length() == 0  ||
        ( dataTypeOut != svkImageWriterFactory::DICOM_MRS && dataTypeOut != svkImageWriterFactory::DDF )
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
    int* start = new int[3];
    int* end = new int[3];
    start[0] = -1;
    start[1] = -1;
    start[2] = -1;
    end[0] = -1;
    end[1] = -1;
    end[2] = -1;
    svkMrsImageData* mrsData = svkMrsImageData::New();
    mrsData->DeepCopy( svkMrsImageData::SafeDownCast(reader->GetOutput()) );
    phaser->SetInput( mrsData );
    phaser->SetChannel( 0 );
    phaser->SetUpdateExtent(start, end );
    int numSpecPoints = reader->GetOutput()->GetCellData()->GetNumberOfTuples();
    if( pivotPoint < 0 ) {
        pivotPoint = numSpecPoints / 2;
    } else if ( pivotPoint >= numSpecPoints ) {
        cerr << "ERROR: Pivot point greater than the number of frequency points!" << endl;
    }
    phaser->SetLinearPhasePivot( pivotPoint );
    phaser->Update();
    phaser->SetPhase0( zeroPhase );
    phaser->Update();
    phaser->SetLinearPhase( linearPhase );
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
    //  Set the input command line into the data set 
    //  provenance: 
    // ===============================================  
    phaser->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

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

