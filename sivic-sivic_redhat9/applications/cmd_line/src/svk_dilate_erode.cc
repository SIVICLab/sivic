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
 *  Utility application for applying a threshold to images.
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
#include <svkDcmHeader.h>
#include <svkImageDilateErode.h>
#include <svkTypeUtils.h>


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_dilate_erode -i input_file_name -o output_file_name -t output_data_type      \n";
    usemsg += "                   -d dilate_value  -e erode_value -k kernel_size                \n";  
    usemsg += "                   [ --single ] [ -vh ]                                          \n";
    usemsg += "                                                                                 \n";  
    usemsg += "   -i            input_file_name     Name of file to convert.                    \n"; 
    usemsg += "   -o            output_file_name    Name of outputfile.                         \n";  
    usemsg += "   -t            output_data_type    Target data type:                           \n";  
    usemsg += "                                         3 = UCSF IDF                            \n";  
    usemsg += "                                         5 = DICOM_MRI                           \n";  
    usemsg += "                                         6 = DICOM_Enhanced MRI                  \n";  
    usemsg += "   -d            dilate_value        Dilate pix val                              \n";
    usemsg += "   -e            erode_value         Erode pix val                               \n";
    usemsg += "   -k            size                Kernel size                                 \n";
    usemsg += "   --single                          Only converts specified file if multi vol.  \n";
    usemsg += "   -v                                Verbose output.                             \n";
    usemsg += "   -h                                Print help mesage.                          \n";  
    usemsg += "                                                                                 \n";  
    usemsg += "Dilates and erodes an image                                                      \n";
    usemsg += "                                                                                 \n";  

    string inputFileName; 
    string outputFileName; 
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::UNDEFINED; 
    int     dilateValue = -1;
    int     erodeValue = -1;
    int     kernelSize = 0;
    bool    verbose = false;
    bool    onlyLoadSingleFile = false;
    int     volume = -1;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_SINGLE 
    };

    static struct option long_options[] =
    {
        {"single",      no_argument,       NULL,  FLAG_SINGLE},
        {0, 0, 0, 0}
    };


    /*
     *  Process flags and arguments
     */
    int i;
    int option_index = 0; 
    while ((i = getopt_long(argc, argv, "i:o:t:d:e:k:hv", long_options, &option_index)) != EOF) {
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
            case 'd':
                dilateValue = svkTypeUtils::StringToInt(optarg);
                break;
            case 'e':
                erodeValue = svkTypeUtils::StringToInt(optarg);
                break;
            case 'k':
                kernelSize = svkTypeUtils::StringToInt(optarg);
                break;
            case FLAG_SINGLE:
                onlyLoadSingleFile = true;
                break;
            case 'v':
                verbose = true;
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

    if ( argc != 0 || inputFileName.length() == 0 || outputFileName.length() == 0 ||
        dataTypeOut < 0 || dataTypeOut > svkImageWriterFactory::LAST_TYPE || 
        kernelSize == 0 || dilateValue == -1 || erodeValue == -1
    ) {
        cout << usemsg << endl;
        exit(1); 
    }

    if( ! svkUtils::FilePathExists( inputFileName.c_str() ) ) {
        cerr << "Input file can not be loaded (may not exist) " << inputFileName << endl;
        exit(1); 
    }

    if( verbose ) {
        cout << inputFileName << endl;
        cout << outputFileName << endl;
        cout << dataTypeOut << endl;
    }

    if( verbose ) {
        cout << "file name: " << inputFileName << endl;
    }

    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());
    readerFactory->Delete(); 

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }

    reader->SetFileName( inputFileName.c_str() );
    if ( onlyLoadSingleFile == true ) {
        reader->OnlyReadOneInputFile();
    }
    reader->Update(); 

    svkImageData* currentImage =  reader->GetOutput();

    //  Set the input command line into the data set provenance:
	currentImage->GetProvenance()->SetApplicationCommand( cmdLine );

    if( dataTypeOut <= svkImageWriterFactory::LAST_TYPE ) {
		svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
		svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter( dataTypeOut ) );

		if ( writer == NULL ) {
			cerr << "Can not determine writer of type: " << dataTypeOut << endl;
			exit(1);
		}

		writerFactory->Delete();
		writer->SetFileName( outputFileName.c_str() );

        svkImageDilateErode* dilateErode = svkImageDilateErode::New();
        dilateErode->SetInputData(currentImage);
        if( volume >= 0 ) {
            dilateErode->SetVolume( volume );
        }

        cout << "Dilate and Erode" << endl;
        cout << "   Dilate Value: " << dilateValue << endl;
        cout << "   Erode Value : " << erodeValue << endl;
        dilateErode->SetDilateValue( dilateValue );
        dilateErode->SetErodeValue( erodeValue );
        dilateErode->SetKernelSize( kernelSize );
        dilateErode->Update();
        currentImage = dilateErode->GetOutput();
		writer->SetInputData( currentImage );
		writer->Write();
		writer->Delete();
		dilateErode->Delete();

    } else {
        cout << usemsg << endl;
        exit(1);
    }

    reader->Delete();

    return 0; 
}

