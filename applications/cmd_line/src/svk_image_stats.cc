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
#include <svkImageStatistics.h>


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_image_stats -i input_file_name [-m roi_file_name] [-c] config_file_name \n";
    usemsg += "                [-v] [-h]                                                    \n";
    usemsg += "                                                                             \n";  
    usemsg += "   -i            input_file_name     Name of file to convert.                \n"; 
    usemsg += "   -m            roi_file_name      Name of the roi file.                  \n";
    usemsg += "   -c            config_file_name    Name of the config file.                \n";
    usemsg += "   -v                                Verbose output.                         \n";
    usemsg += "   -h                                Print help mesage.                      \n";  
    usemsg += "                                                                             \n";  
    usemsg += "Computes histogram-based statistics on an input image. A roi can be         \n";
    usemsg += "specified to limit the computation to a given region of the roi.            \n";
    usemsg += "                                                                             \n";  

    string inputFileName; 
    string roiFileName;
    string configFileName;
    bool   verbose = false;
    static struct option long_options[] =
    {
        //{"deid_type",   required_argument, NULL,  FLAG_DEID_TYPE},
        //{"deid_id",     required_argument, NULL,  FLAG_DEID_STUDY_ID},
        {0, 0, 0, 0}
    };

    /*
    *   Process flags and arguments
    */
    int i;
    int option_index = 0; 
    while ((i = getopt_long(argc, argv, "i:m:c:hv", long_options, &option_index)) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'm':
                roiFileName.assign(optarg);
                break;
            case 'c':
                configFileName.assign(optarg);
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
     * Check for extra arguments or an empty input file name.
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
        cout << "Input:  " << inputFileName << endl;
        cout << "roi:   " << roiFileName << endl;
        cout << "Config: " << configFileName << endl;
    }

    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();


    // READ THE IMAGE
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }

    reader->SetFileName( inputFileName.c_str() );

    reader->Update(); 

    svkMriImageData* image = svkMriImageData::SafeDownCast(reader->GetOutput());
    image->Register( NULL );

    reader->Delete();
    // READ THE ROI
    reader = readerFactory->CreateImageReader2(roiFileName.c_str());
    readerFactory->Delete();

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << roiFileName << endl;
        exit(1);
    }

    reader->SetFileName( roiFileName.c_str() );

    reader->Update();
    svkMriImageData* roi = svkMriImageData::SafeDownCast(reader->GetOutput());
    roi->Register(NULL);

    svkImageStatistics* stats = svkImageStatistics::New();
    stats->AddInput( image );
    stats->AddROI( roi );
    stats->Update();
    stats->PrintStatistics();
    stats->Delete();
    reader->Delete();
    roi->Delete();
    image->Delete();

    return 0; 
}

