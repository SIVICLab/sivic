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

#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkDdfVolumeReader.h>
#include <svkDcmVolumeReader.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkLCModelCSVReader.h>
#include <svkDICOMMRSWriter.h>
#include <svkDdfVolumeWriter.h>
#include <svkDcmHeader.h>
#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#endif
#define UNDEFINED_TEMP -1111

#define UNDEFINED -1

using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_lcmodel_reader -i input_file_name -o output_file_root --csf csv_file_name        \n"; 
    usemsg += "                   [ -bh ]                                                           \n";
    usemsg += "                                                                                     \n";  
    usemsg += "   -i                name    Name of template MRS file.                              \n"; 
    usemsg += "   -o                name    Root name of outputfile.                                \n";
    usemsg += "   --csv             name    Name of csv file to convert.                            \n";
    usemsg += "   -b                        Set up for selection box analysis only.                 \n";
    usemsg += "   -h                        Print this help mesage.                                 \n";  
    usemsg += "                                                                                     \n";  
    usemsg += "Reads LCModel output and converts to DICOM metabolite maps.                          \n";  
    usemsg += "                                                                                     \n";  

    string  inputFileName; 
    string  outputFileName;
    string  csvFileName;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 

    enum FLAG_NAME {
        FLAG_CSV_FILE
    }; 

    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"csv",         required_argument, NULL,  FLAG_CSV_FILE}, 
        {0, 0, 0, 0}
    };

    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0; 
    while ( ( i = getopt_long(argc, argv, "i:o:bh", long_options, &option_index) ) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case FLAG_CSV_FILE:
                csvFileName.assign( optarg );
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
        || csvFileName.length() == 0 
    ) {
        cout << usemsg << endl;
        exit(1); 
    }

    if( ! svkUtils::FilePathExists( inputFileName.c_str() ) ) {
        cerr << "Input file can not be loaded (may not exist) " << inputFileName << endl; 
        exit(1); 
    }

    if( ! svkUtils::FilePathExists( csvFileName.c_str() ) ) {
        cerr << "Input file can not be loaded (may not exist) " << csvFileName << endl; 
        exit(1); 
    }
    cout << "file name: " << inputFileName << endl;
    cout << "csv file name: " << inputFileName << endl;


    // ===============================================  
    // ===============================================  
    svkLCModelCSVReader* csvReader = svkLCModelCSVReader::New(); 
    csvReader->SetFileName(csvFileName.c_str() ); 
    //csvReader->SetMRSTemplate(inputFileName.c_str() ); 
    csvReader->Update(); 
exit(0); 

    // ===============================================  
    //  Use a reader factory to get the correct reader  
    //  type for the template MRS data. 
    // ===============================================  
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* mrsReader = readerFactory->CreateImageReader2(inputFileName.c_str());
    readerFactory->Delete();

    if ( mrsReader == NULL ) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }

    mrsReader->SetFileName( inputFileName.c_str() );
    mrsReader->Update();



    svkImageData* currentImage = svkMrsImageData::SafeDownCast( mrsReader->GetOutput() ); 

    // ===============================================  
    //  Write the data out to the specified file type.  
    //  Use an svkImageWriterFactory to obtain the
    //  correct writer type. 
    // ===============================================  
    vtkSmartPointer< svkImageWriterFactory > writerFactory = vtkSmartPointer< svkImageWriterFactory >::New(); 
    svkImageWriter* writer = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( svkImageWriterFactory::DICOM_ENHANCED_MRI) ); 

    if ( writer == NULL ) {
        cerr << "Can not create writer of type: " << svkImageWriterFactory::DICOM_ENHANCED_MRI << endl;
        exit(1);
    }

    writer->SetFileName( outputFileName.c_str() );
    writer->SetInput( svkMrsImageData::SafeDownCast( currentImage ) );

    // ===============================================  
    //  Set the input command line into the data set 
    //  provenance: 
    // ===============================================  
    mrsReader->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    // ===============================================  
    //  Write data to file: 
    // ===============================================  
    //writer->Write();

    // ===============================================  
    //  Clean up: 
    // ===============================================  
    writer->Delete();
    mrsReader->Delete();

    return 0; 
}

