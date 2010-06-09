/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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
 *  License: TBD
 *
 *  Utility application for converting between supported file formats. 
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
#include <svkIdfVolumeWriter.h>
#include <svkDcmHeader.h>
#include <vtkIndent.h>


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_file_convert -i input_file_name -o output_file_name -t output_data_type [-h] \n";
    usemsg += "\n";  
    usemsg += "   -i input_file_name      name of file to convert. \n"; 
    usemsg += "   -o output_file_name     name of outputfile. \n";  
    usemsg += "   -t output_data_type     target data type: \n";  
    usemsg += "                               2 = UCSF DDF      \n";  
    usemsg += "                               3 = UCSF IDF      \n";  
    usemsg += "                               4 = DICOM_MRS     \n";  
    usemsg += "   -h                      print help mesage. \n";  
    usemsg += " \n";  
    usemsg += "Converts the input file to the specified target file type \n";  
    usemsg += "\n";  

    string inputFileName; 
    string outputFileName; 
    svkImageWriterFactory::WriterType dataTypeOut; 

    /*
    *   Process flags and arguments
    */
    int i;
    while ((i = getopt(argc, argv, "i:o:t:h")) != EOF) {
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

    if ( argc != 0 ||  inputFileName.length() == 0 || outputFileName.length() == 0 ||
        dataTypeOut < 0) {
        cout << usemsg << endl;
        exit(1); 
    }

    cout << inputFileName << endl;
    cout << outputFileName << endl;
    cout << dataTypeOut << endl;

    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());
    readerFactory->Delete(); 

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }

    reader->SetFileName( inputFileName.c_str() );
    reader->Update(); 

    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter( dataTypeOut ) );

    if ( writer == NULL ) {
        cerr << "Can not determine writer of type: " << dataTypeOut << endl;
        exit(1);
    }

    writerFactory->Delete();
    writer->SetFileName( outputFileName.c_str() );
    writer->SetInput( reader->GetOutput() );
    writer->Write();
    writer->Delete();
    reader->Delete();


    return 0; 
}

