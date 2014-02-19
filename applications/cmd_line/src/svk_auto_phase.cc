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
    usemsg += "svk_auto_phase -i input_filename -o output_filename [ -t output_data_type ] [ -h ] \n";
    usemsg += "                                                    [ --single ] \n";
    usemsg += "                                                         \n";
    usemsg += "   -i            name   Name of input MRS file.          \n";
    usemsg += "   -o            name   Name of output file.             \n";
    usemsg += "   -t            type   Target data type:            \n";
    usemsg += "                                 2 = UCSF DDF            \n";
    usemsg += "                                 4 = DICOM_MRS (default) \n";
    usemsg += "   --single             Only phase specified file if multiple in series \n";
    usemsg += "   -a            type   Type of phasing.  Options: eries \n";
    usemsg += "                                 1 = FIRST POINT PHASING \n";
    usemsg += "   -h                   Print this help mesage.      \n";
    usemsg += "                                                         \n";
    usemsg += "Auto phase spectra (zero and first order phaseing).      \n";
    usemsg += "                                                         \n";


    string inputFileName;
    string outputFileName;
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_MRS;
    bool onlyPhaseSingle = true;
    svkMRSAutoPhase::phasingModel phaseModelType = svkMRSAutoPhase::FIRST_POINT_0;


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
    while ( ( i = getopt_long(argc, argv, "i:o:t:h", long_options, &option_index) ) != EOF) {
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
            || ( dataTypeOut != svkImageWriterFactory::DICOM_MRS && dataTypeOut != svkImageWriterFactory::DDF )
    ) {
            cout << usemsg << endl;
            exit(1); 
    }

    cout << inputFileName << endl;
    cout << outputFileName << endl;


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
    reader->Update();
    
    //reader->GetOutput()->GetDcmHeader()->PrintDcmHeader(); 

    // ===============================================  
    //  Pass data through your algorithm:
    // ===============================================  
    svkMRSAutoPhase* phaser = svkMRSFirstPointPhase::New();
    //svkMRSAutoPhase* phaser = svkMRSAutoPhase::New();
    phaser->SetInputConnection(0, reader->GetOutputPort(0) ); 
    phaser->OnlyUseSelectionBox();

    //svkMRSAutoPhase::phasingModel model;
    //model =  svkMRSAutoPhase::MAX_PEAK_HTS_0;
        //phaser->SetPhasingModel(svkMRSAutoPhase::MAX_GLOBAL_PEAK_HT_0); 
        //phaser->SetPhasingModel(svkMRSAutoPhase::MAX_PEAK_HTS_0); 
        //phaser->SetPhasingModel(svkMRSAutoPhase::MAX_PEAK_HTS_1); 
        //phaser->SetPhasingModel(svkMRSAutoPhase::MIN_DIFF_FROM_MAG_0); 
        //phaser->SetPhasingModel(svkMRSAutoPhase::MIN_DIFF_FROM_MAG_1); 
        //phaser->SetPhasingModel(svkMRSAutoPhase::MAX_PEAK_HTS_01); 
    //model = svkMRSAutoPhase::MAX_PEAK_HTS_01; 
    //model =  svkMRSAutoPhase::MAX_PEAK_HTS_1;
    //model =  svkMRSAutoPhase::FIRST_POINT_0;

    //phaser->SetPhasingModel( model ); 
    phaser->Update();


    // ===============================================  
    //  Use writer factory to create writer for specified
    //  output file format. 
    // ===============================================  
    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 
    writerFactory->Delete();
    
    if ( writer == NULL ) {
        cerr << "Can not create writer " << endl;
        exit(1);
    }

//stringstream modelString(stringstream::in | stringstream::out);
//modelString << model;
//outputFileName.append(modelString.str());    
    writer->SetFileName( outputFileName.c_str() );

    // ===============================================  
    //  Write output of algorithm to file:    
    // ===============================================  
    //writer->SetInput( phaser->GetOutput()); 
    writer->SetInput( phaser->GetImageDataInput(0) );
    writer->Write();


    //  clean up:
    phaser->Delete(); 
    writer->Delete();
    reader->Delete();

    return 0; 
}


