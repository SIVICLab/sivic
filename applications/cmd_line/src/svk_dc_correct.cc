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
 *
 *  Utility application for converting between supported file formats. 
 *
 */

#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#include <unistd.h>
#endif
#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDICOMMRSWriter.h>
#include <svkDcmHeader.h>
#include <svkCorrectDCOffset.h>
#include <vtkSmartPointer.h>
using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) +                                       "\n";   
    usemsg += "svk_apodize -i input_file_name -o output_root                                    \n"; 
    usemsg += "            -t filter_type [-h]                                                  \n";
    usemsg += "                                                                                 \n";  
    usemsg += "   -i        input_file_name   Name of file to convert.                          \n"; 
    usemsg += "   -o        output_root       Root name of outputfile.                          \n";  
    usemsg += "   -t        type              Target data type:                                 \n";
    usemsg += "                                     2 = UCSF DDF                                \n";
    usemsg += "                                     4 = DICOM_MRS (default)                     \n";
    usemsg += "   -h                          Print help mesage.                                \n";  
    usemsg += "                                                                                 \n";  
    usemsg += "Applies DC correction to input time domain data.                                 \n"; 
    usemsg += "                                                                                 \n";  

    string inputFileName; 
    string outputFileName; 
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_MRS; 

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
    };


    static struct option long_options[] =
    {
        {0, 0, 0, 0}
    };

    /*
    *   Process flags and arguments
    */
    int i;
    int option_index = 0; 
    while ((i = getopt_long(argc, argv, "i:o:t:h", long_options, &option_index)) != EOF) {
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


    /*
     *  Validate input. 
     */
    if ( 
        argc != 0 || inputFileName.length() == 0 || outputFileName.length() == 0 ||
        ( dataTypeOut != svkImageWriterFactory::DICOM_MRS && dataTypeOut != svkImageWriterFactory::DDF ) 
    ) {
        cout << usemsg << endl;
        exit(1); 
    }

    if( ! svkUtils::FilePathExists( inputFileName.c_str() ) ) {
        cerr << "Input file can not be loaded (may not exist) " << inputFileName << endl;
        exit(1); 
    }


    cout << "Input File:   " << inputFileName << endl;
    cout << "Output File:  " << outputFileName << endl;


    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());
    readerFactory->Delete(); 

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }
    reader->SetFileName( inputFileName.c_str() );
    reader->Update(); 

    //  ===============================================
    //  Set the input command line into the data set provenance:
    //  ===============================================
    reader->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );


    //  ===============================================
    //  Apply DC Correction` 
    //  ===============================================

    vtkSmartPointer< svkCorrectDCOffset > dc = vtkSmartPointer< svkCorrectDCOffset >::New();
    dc->SetInput( reader->GetOutput() );
    dc->Update();

    //  ===============================================
    //  Write results
    //  ===============================================
    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter( dataTypeOut ) );
    if ( writer == NULL ) {
        cerr << "Can not determine writer of type: " << dataTypeOut << endl;
        exit(1);
    }
    writerFactory->Delete();
    writer->SetFileName( outputFileName.c_str() );
    writer->SetInput( dc->GetOutput() );
    writer->Write();
    writer->Delete();

    dc->Delete();
    reader->Delete();


    return 0; 
}

