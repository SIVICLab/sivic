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
#include <svkMrsSingleVoxelSincExtraction.h>
#include <svkTypeUtils.h>
#include <vtkIndent.h>
#include <vtkXMLImageDataWriter.h>


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_interpolate_spectra -i input_file -o output_file -l coord -p coord -s coord\n";
    usemsg += "                         [-r][-V][-h]                                          \n";
    usemsg += "                                                                               \n";
    usemsg += "   -i            input_file    Name of file to convert.                  \n";
    usemsg += "   -o            output_file   Name of outputfile.                       \n";
    usemsg += "   -l            coord         Left coordinate                           \n";
    usemsg += "   -p            coord         Posterior coordinate                      \n";
    usemsg += "   -s            coord         Superior coordinate                       \n";
    usemsg += "   -r                          Retain input extent                       \n";
    usemsg += "   -v                          Verbose output.                           \n";
    usemsg += "   -h                          Print help mesage.                        \n";
    usemsg += "                                                                               \n";
    usemsg += "Applies sinc interpolation to produce a single voxel spectroscopy data.        \n";
    usemsg += "                                                                               \n";

    string inputFileName; 
    string outputFileName; 
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::UNDEFINED; 
    bool   verbose = false;
    bool retainInputExtent = false;
    double newVoxelCenter[] = {0,0,0};
    bool coordinateSet[] = {false, false, false};

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    static struct option long_options[] =
    {
    };


    /*
    *   Process flags and arguments
    */
    int i;
    int option_index = 0; 
    while ((i = getopt_long(argc, argv, "i:o:t:l:p:s:hvr", long_options, &option_index)) != EOF) {
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
            case 'l':
                newVoxelCenter[0] = svkTypeUtils::StringToDouble(optarg);
                coordinateSet[0] = true;
                break;
            case 'p':
                newVoxelCenter[1] = svkTypeUtils::StringToDouble(optarg);
                coordinateSet[1] = true;
                break;
            case 's':
                newVoxelCenter[2] = svkTypeUtils::StringToDouble(optarg);
                coordinateSet[2] = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 'r':
                retainInputExtent = true;
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

    if ( !coordinateSet[0] || !coordinateSet[1] || !coordinateSet[2]) {
        cout << usemsg << endl;
        cout << "ERROR: The LPS location of the new target voxel is required!" << endl;
        exit(1);
    }

    if ( argc != 0 || inputFileName.length() == 0 || outputFileName.length() == 0 ||
        dataTypeOut < 0 || dataTypeOut > svkImageWriterFactory::LAST_TYPE ) {
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
        svkMrsSingleVoxelSincExtraction* interpolator = svkMrsSingleVoxelSincExtraction::New();
        interpolator->SetInput(currentImage);
        interpolator->SetVoxelCenter(newVoxelCenter[0], newVoxelCenter[1], newVoxelCenter[2]);
        interpolator->SetRetainInputExtent(true);
        interpolator->Update();
        writer->SetInput( interpolator->GetOutput() );
        writer->Write();
        writer->Delete();
        interpolator->Delete();

    } else {
        cout << usemsg << endl;
        exit(1);
    }

    reader->Delete();

    return 0; 
}

