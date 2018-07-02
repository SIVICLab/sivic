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
#include <svkIdfVolumeReader.h>
#include <svkDdfVolumeReader.h>
#include <svkDcmVolumeReader.h>
#include <svkDcmHeader.h>
#include <vtkIndent.h>


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_print_image_info -i input_file_name                                      \n"; 
    usemsg += "                 [ --info ] ] [-vh]                                          \n";
    usemsg += "                                                                             \n";  
    usemsg += "   -i            input_file_name     Name of file to convert.                \n"; 
    usemsg += "   --single                          Only converts specified file            \n";
    usemsg += "   --info                            Print info about the images, only       \n";                                        
    usemsg += "                                     requires the -i flag.                   \n";
    usemsg += "   -v                                Verbose output.                         \n";
    usemsg += "   -h                                Print help mesage.                      \n";  
    usemsg += "                                                                             \n";  
    usemsg += "============================================================================ \n";  
    usemsg += "                                                                             \n";  

    string inputFileName; 
    bool   verbose = false;
    bool   onlyLoadSingleFile = false;
    bool   infoMode = false;   

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_SINGLE, 
        FLAG_INFO_MODE
    };


    static struct option long_options[] =
    {
        {"single",      no_argument,       NULL,  FLAG_SINGLE},
        {"info",        no_argument,       NULL,  FLAG_INFO_MODE},
        {0, 0, 0, 0}
    };



    /*
    *   Process flags and arguments
    */
    int i;
    int option_index = 0; 
    while ((i = getopt_long(argc, argv, "i:o:t:cbhv", long_options, &option_index)) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case FLAG_SINGLE:
                onlyLoadSingleFile = true;
                break;
            case FLAG_INFO_MODE:
                infoMode = true;
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

    /*
     * In addition to svkImageWriters this converter also supports an xml writer for the vtk
     * vti format. Because this writer is not a vtkImageWriter svkImagerWriterFactory cannot
     * return it so we must instantiate it outside of the factory. To account for this extra
     * type we will support a dataTypeOut equal to svkImageWriterFactory::LAST_TYPE + 1.
     */
    if ( argc != 0 || inputFileName.length() == 0 ) {
        cout << usemsg << endl;
        exit(1); 
    }

    if( ! svkUtils::FilePathExists( inputFileName.c_str() ) ) {
        cerr << "Input file can not be loaded (may not exist) " << inputFileName << endl;
        exit(1); 
    }


    if( verbose ) {
        cout << inputFileName << endl;
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

    if ( infoMode == true ) {
        double window; 
        double level; 
        svkMriImageData::SafeDownCast(reader->GetOutput())->GetAutoWindowLevel( window, level);

        ostringstream info;

        info << "=====================================================================" << endl; 
        info << "IMAGE INFO: "                                                          << endl; 
        info << "=====================================================================" << endl; 
        info << "AUTO WINDOW: " << window << endl;
        info << "AUTO LEVEL:  " << level  << endl;

        info << *reader->GetOutput()  << endl;
        cout << info.str() << endl;

        vtkDataArray* imageArray;
        imageArray = reader->GetOutput()->GetPointData()->GetArray(0);
        int numVoxels[3];

        cout << "num_voxels_0 = " << numVoxels[0] << endl;
        cout << "num_voxels_1 = " << numVoxels[1] << endl;
        cout << "num_voxels_2 = " << numVoxels[2] << endl;

        svkMriImageData::SafeDownCast( reader->GetOutput() )->GetNumberOfVoxels(numVoxels);
        int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];

        float value; 
        for ( int i = 0; i < totalVoxels; i++ ) {
            value = imageArray->GetTuple1( i );
            cout <<  "VALUE:  "  << i << " " << value << endl;
        }


        exit(0); 
    }

    svkImageData* currentImage =  reader->GetOutput();

    //  Set the input command line into the data set provenance:
	currentImage->GetProvenance()->SetApplicationCommand( cmdLine );

    reader->Delete();

    return 0; 
}

