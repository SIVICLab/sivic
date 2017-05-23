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
 *   
 *  
 *
 *  Utility application for converting between supported file formats. 
 *
 */


#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkMRSAutoPhase.h>
#include <svkMRSFirstPointPhase.h>
#include <svkMRSZeroOrderPhase.h>

#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#endif


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ;
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";
    usemsg += "svk_auto_phase -i input_filename -o output_filename -a type [ -t output_data_type ]  \n";
    usemsg += "               [ --single ] [ -h ]                                                   \n";
    usemsg += "                                                                                     \n";
    usemsg += "   -i            name   Name of input MRS file.                                      \n";
    usemsg += "   -o            root   Root of output file.                                         \n";
    usemsg += "   -t            type   Target data type:                                            \n";
    usemsg += "                                 2 = UCSF DDF and IDF phase map                      \n";
    usemsg += "                                 4 = DICOM_MRS and DCM phase map (default)           \n";
    usemsg += "   -a            type   Type of phasing.  Options: eries                             \n";
    usemsg += "                                 1 = FIRST POINT PHASING                             \n";
    usemsg += "                                 2 = ZERO ORDER PHASING (Max Peak Hts)               \n";
    usemsg += "                                 3 = ZERO ORDER PHASING (Max Peak Ht)                \n";
    usemsg += "   --single             Only phase specified file if multiple in series              \n";
    usemsg += "   -h                   Print this help mesage.                                      \n";
    usemsg += "                                                                                     \n";
    usemsg += "Auto phase spectra (zero and first order phaseing).                                  \n";
    usemsg += "     2:  max peak ht of autodetected peaks                                           \n";
    usemsg += "     3:  max peak ht of specified peak                                               \n";
    usemsg += "                                                                                     \n";


    string inputFileName;
    string outputFileName;
    svkImageWriterFactory::WriterType mrsDataTypeOut = svkImageWriterFactory::DICOM_MRS;
    svkImageWriterFactory::WriterType mriDataTypeOut = svkImageWriterFactory::DICOM_ENHANCED_MRI;
    svkMRSAutoPhase::PhasingModel phasingType = svkMRSAutoPhase::UNDEFINED_PHASE_MODEL; 

    bool onlyPhaseSingle = false;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_SINGLE = 0
    };


    static struct option long_options[] =
    {
        {"single",    no_argument,       NULL,  FLAG_SINGLE},
        {0, 0, 0, 0}
    };


    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0;
    while ( ( i = getopt_long(argc, argv, "i:o:t:a:h", long_options, &option_index) ) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                //  Make sure the file name doesn't contain an extension: 
                outputFileName = svkImageReader2::GetFileRoot(outputFileName.c_str()); 
                break;
            case 't':
                mrsDataTypeOut = static_cast<svkImageWriterFactory::WriterType>( atoi(optarg) );
                break;
            case 'a':
                phasingType = static_cast<svkMRSAutoPhase::PhasingModel>( atoi(optarg) );
                break;
            case FLAG_SINGLE:
                onlyPhaseSingle = true;
                break;
            case 'h':
                cout << usemsg << endl;
                exit(1);
                break;
            default:
                ;
        }
    }

    argc -= optind;
    argv += optind;


    if (
        argc != 0 ||  inputFileName.length() == 0
            || outputFileName.length() == 0
            || ( mrsDataTypeOut != svkImageWriterFactory::DICOM_MRS && mrsDataTypeOut != svkImageWriterFactory::DDF )
            || outputFileName.length() == 0
            || phasingType == svkMRSAutoPhase::UNDEFINED_PHASE_MODEL 
    ) {
            cout << usemsg << endl;
            exit(1); 
    }

    if ( phasingType <= svkMRSAutoPhase::UNDEFINED_PHASE_MODEL || phasingType >= svkMRSAutoPhase::LAST_MODEL ) {
        cout << "Specified phasing type is not recognized." << endl;
        cout << usemsg << endl;
        exit(1); 
    }

    // ===============================================
    //  Output phase image type is implied by output MRS data type:  
    // ===============================================
    if ( mrsDataTypeOut == svkImageWriterFactory::DDF ) {
        mriDataTypeOut = svkImageWriterFactory::IDF;
    }

    if( ! svkUtils::FilePathExists( inputFileName.c_str() ) ) {
        cerr << "Input file can not be loaded (may not exist) " << inputFileName << endl;
        exit(1);
    }

    cout << "Input File:  " << inputFileName << endl;
    cout << "Output File: " << outputFileName << endl;
    cout << "Phase Type:  " << phasingType << endl;


    // ===============================================
    //  Use a reader factory to create a data reader 
    //  of the correct type for the input file format.
    // ===============================================
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());
    readerFactory->Delete();

    if (reader == NULL ) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl; 
        exit(1);
    }

    //  Read the data to initialize an svkImageData object
    reader->SetFileName( inputFileName.c_str() );
    if ( onlyPhaseSingle == true ) {
        reader->OnlyReadOneInputFile();
    }
    //reader->Update();
    reader->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );
    

    // ===============================================  
    //  Pass data through phasing algo.  Should use a factory to get
    //  correcty phasing algo:
    // ===============================================  
    svkMRSAutoPhase* phaser;
    if ( phasingType == svkMRSAutoPhase::FIRST_POINT_0 ) {  
        phaser = svkMRSFirstPointPhase::New();
    } else if ( phasingType == svkMRSAutoPhase::MAX_PEAK_HTS_0 ) {  
        phaser = svkMRSZeroOrderPhase::New();
    } else {
        cout << "Specified phasing type is not recognized." << endl;
        exit(1); 
    }
    phaser->SetInputConnection(0, reader->GetOutputPort(0) ); 
    phaser->OnlyUseSelectionBox();
    phaser->Update();
    phaser->GetOutput(0)->GetProvenance()->SetApplicationCommand( cmdLine );


    // ===============================================  
    //  Use writer factory to create writer for specified
    //  output file format. 
    // ===============================================  
    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* mrsWriter = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( mrsDataTypeOut ) ); 
    svkImageWriter* mriWriter = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( mriDataTypeOut ) ); 
    writerFactory->Delete();
    
    if ( mrsWriter == NULL || mriWriter == NULL ) {
        cerr << "Can not create writer " << endl;
        exit(1);
    }

    mrsWriter->SetFileName( outputFileName.c_str() );
    string phaseMapName = outputFileName;
    if ( phasingType == svkMRSAutoPhase::FIRST_POINT_0 ) {  
        phaseMapName.append("_FirstPointPhaseMap"); 
    } else if ( phasingType == svkMRSAutoPhase::MAX_PEAK_HTS_0 ) {  
        phaseMapName.append("_ZeroOrderPhaseMap"); 
    }
    mriWriter->SetFileName( phaseMapName.c_str() );

    // ===============================================  
    //  Write output of algorithm to file:    
    // ===============================================  
    mrsWriter->SetInputData( phaser->GetImageDataInput(0) );
    mrsWriter->Write();
    mriWriter->SetInputData( phaser->GetOutput(0) );
    mriWriter->Write();

    //  clean up:
    phaser->Delete(); 
    mrsWriter->Delete();
    mriWriter->Delete();
    reader->Delete();

    return 0; 
}


