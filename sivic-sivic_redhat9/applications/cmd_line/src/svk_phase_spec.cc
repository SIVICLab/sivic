/*
 *  Copyright © 2009-2017 The Regents of the University of California.
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
    usemsg += "svk_phase_spec -i input_file_name -o output_root [ -t output_data_type ]     \n"; 
    usemsg += "     [ --map0 zero_order_phase_map  || -z zero_order_phase [ -l linear_phase -p pivot_point ] ]  \n"; 
    usemsg += "                                                                             \n"; 
    usemsg += "   -i        name        Name of input data file to phase.                   \n"; 
    usemsg += "   -o        root_name   Name of input data file to phase.                   \n"; 
    usemsg += "   -t        type        Target data type:                                   \n";
    usemsg += "                         2 = UCSF DDF                                        \n";
    usemsg += "                         4 = DICOM_MRS (default)                             \n";
    usemsg += "   -z        phase       Global zero order phase value.                      \n"; 
    usemsg += "   --map0    map_file    Image with values of zero order phases to apply.    \n"; 
    usemsg += "   -l        phase       Global first order phase value.                     \n";
    usemsg += "   -p        pivot       Global first order phase pivot point.               \n";
    usemsg += "   --single              Only phase the specified file if multiple in series.\n";
    usemsg += "   -h                    Print this help mesage.                             \n";  
    usemsg += "\n";  
    usemsg += "Applies zero order and/or linear phase to a dataset. Phase is specified in   \n"; 
    usemsg += "degrees.                                                                     \n"; 
    usemsg += "\n";  


    string inputFileName; 
    string outputFileName;
    string map0FileName;
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_MRS;
    bool usePhaseMap0 = false; 
    bool onlyPhaseSingleFile = false;


    float zeroPhase = 0.0;
    float linearPhase = 0.0;
    int pivotPoint = -1;
    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 

    enum FLAG_NAME {
        FLAG_MAP_0 = 0, 
        FLAG_SINGLE 
    };


    static struct option long_options[] =
    {
        {"map0",   required_argument, NULL,  FLAG_MAP_0},
        {"single",    no_argument,       NULL,  FLAG_SINGLE},
        {0, 0, 0, 0}
    };


    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0; 
    while ( ( i = getopt_long(argc, argv, "i:z:l:p:o:t:h", long_options, &option_index) ) != EOF) {
        
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
            case FLAG_MAP_0:
                map0FileName.assign( optarg );
                break;
            case FLAG_SINGLE:
                onlyPhaseSingleFile = true;
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

    if ( map0FileName.length() == 0 ) {
        if ( linearPhase == 0 && zeroPhase == 0 ) {
            cout << "ERROR: Must specify a phase to apply!" << endl;
            cout << usemsg << endl;
            exit(1); 
        }
    }  

    if ( map0FileName.length() != 0 ) {
        usePhaseMap0 = true; 
        if ( linearPhase != 0 || zeroPhase != 0 ) {
            cout << "ERROR: Must specify either a global phase or a phase map, but not both." << endl;
            cout << usemsg << endl;
            exit(1); 
        }
    }  

    cout << "input file name:  " << inputFileName << endl;
    cout << "output file root: " << outputFileName << endl;
    cout << "map0 file name:   " << map0FileName << endl;
    cout << "zero order phase: " << zeroPhase << endl;
    cout << "first order phase:" << linearPhase << endl;
    cout << "first order pivot:" << pivotPoint << endl;


    // ===============================================  
    //  Use a reader factory to get the correct reader  
    //  type and load data. 
    // ===============================================  
    vtkSmartPointer< svkImageReaderFactory > mrsReaderFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 
    // ===============================================  
    //  Input Spec data    
    // ===============================================  
    svkImageReader2* reader = mrsReaderFactory->CreateImageReader2(inputFileName.c_str());
    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }
    reader->SetFileName( inputFileName.c_str() );
    if ( onlyPhaseSingleFile == true ) {
        reader->OnlyReadOneInputFile();
    }
    reader->Update(); 


    // ===============================================  
    //  Optional Phase map image 
    // ===============================================  
    vtkSmartPointer< svkImageReaderFactory > mriReaderFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 
    svkImageReader2* map0Reader;
    if ( usePhaseMap0 ) {
        map0Reader = mriReaderFactory->CreateImageReader2( map0FileName.c_str() );
        if ( map0Reader == NULL ) {
            cerr << "Can not determine appropriate reader for: " << map0FileName << endl;
            exit(1);
        }
        map0Reader->SetFileName( map0FileName.c_str() );
        if ( onlyPhaseSingleFile == true ) {
            map0Reader->OnlyReadOneInputFile();
        }
    }


    // ===============================================  
    //  Phase the data.: 
    // ===============================================  
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
    phaser->SetInputData( mrsData );
    phaser->SetChannel( 0 );
    phaser->SetUpdateExtent(start, end );

    if ( usePhaseMap0 ) {

        // to break the pipeline and avoid vtk issues with conflicting extents in the output port, 
        // a copy of the mri data object:     
        phaser->SetInputConnection( 1, map0Reader->GetOutputPort() ); 
        phaser->Update();

    } else {

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

    }


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
    writer->SetInputData( mrsData );

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

