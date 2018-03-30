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
 *  Utility application for determine noise SD from MRS data set. 
 *
 */

#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDcmHeader.h>
#include <svkMRSNoise.h>


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
    usemsg += "Version " + string(SVK_RELEASE_VERSION) +                                   "\n";   
    usemsg += "svk_noise -i input_file_name                                                 \n"; 
    usemsg += "         [ -l start_pt -u end_pt ]                                           \n"; 
    usemsg += "         [ -b ] [ -h ]                                                       \n"; 
    usemsg += "                                                                             \n";  
    usemsg += "   -i                name   Name of input file                               \n"; 
    usemsg += "   -p                       Percent of spectrum to use for noise calc.       \n"; 
    usemsg += "                            number between 0 and 1 (default = .05).          \n"; 
    usemsg += "   -b                       Only include spectra in selection box.           \n"; 
    usemsg += "   -l                       lower point (inclusive) to use for noise window  \n"; 
    usemsg += "   -u                       upper point (includive) to use for noise window  \n"; 
    usemsg += "   --single                 Only operates on the single specified file       \n";
    usemsg += "   -h                       Print this help mesage.                          \n";  
    usemsg += "                                                                             \n";  
    usemsg += "Determines noise in baseline of an MRS data set:                             \n";  
    usemsg += "Determines the frequency range to use by identifying a window in the         \n"; 
    usemsg += "average magnitude spectum comprising 5% (default) of the total spectrum      \n"; 
    usemsg += "with the smallest SD.  Then uses that point range to compute the average     \n"; 
    usemsg += "value of the SD from the complex spectra.  Also reports the mean value       \n"; 
    usemsg += "in the noise window range.  Explicit manual window selection is also         \n"; 
    usemsg += "supported (-l -u).                                                           \n"; 
    usemsg += "\n";  


    string inputFileName; 
    bool limitToSelectionBox = false; 
    float noiseWindowPercent = -1; 
    bool  onlyLoadSingleFile = false;
    int noiseStartPoint = -1; 
    int noiseEndPoint = -1; 


    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_MRS;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 

    enum FLAG_NAME {
        FLAG_SINGLE
    }; 


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"single",                  no_argument,       NULL,  FLAG_SINGLE},
        {0, 0, 0, 0}
    };


    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0; 
    while ( ( i = getopt_long(argc, argv, "i:l:u:bp:h", long_options, &option_index) ) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'p':
                noiseWindowPercent = atof(optarg); 
                break;
            case 'b':
                limitToSelectionBox = true; 
                break;
            case 'l':
                noiseStartPoint = atoi(optarg); 
                break;
            case 'e':
                noiseEndPoint = atoi(optarg); 
                break;
            case FLAG_SINGLE:
                onlyLoadSingleFile = true;
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

    // ===============================================  
    //  validate that: 
    //      an output name was supplied
    //      that not suppresses and unsuppressed were both specified 
    //      that only the supported output types was requested. 
    //      
    // ===============================================  
    cout << "file name: " << inputFileName << endl;
    cout << "argc: " << argc << endl;
    if ( argc != 0 ||  inputFileName.length() == 0  ) {
        cout << usemsg << endl;
        exit(1); 
    }

    if (
        ( noiseStartPoint != -1  && noiseEndPoint == -1 ) || 
        ( noiseStartPoint == -1  && noiseEndPoint != -1 ) 
    ) {
        cout << "Must specify both -l and -u or neither." << endl;
        cout << usemsg << endl;
        exit(1); 
    }

    // ===============================================  
    //  Use a reader factory to get the correct reader  
    //  type . 
    // ===============================================  
    vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }

    reader->SetFileName( inputFileName.c_str() );
    if ( onlyLoadSingleFile == true ) {
        reader->OnlyReadOneInputFile();
    }
    reader->Update(); 

    svkDcmHeader* hdr = reader->GetOutput()->GetDcmHeader(); 

    // ===============================================  
    //  Return noise value from data 
    // ===============================================  
    svkMRSNoise* noise = svkMRSNoise::New();
    noise->SetInputData( reader->GetOutput() ); 
    if ( limitToSelectionBox ) {
        noise->OnlyUseSelectionBox(); 
    }
    if ( noiseWindowPercent >= 0 ) {
        noise->SetNoiseWindowPercent( noiseWindowPercent );
    }
    if ( noiseStartPoint >= 0  && noiseEndPoint >= 0 ) {
        noise->SetNoiseStartPoint( noiseStartPoint );     
        noise->SetNoiseEndPoint( noiseEndPoint );     
    }
    noise->Update();
    float noiseSD = noise->GetNoiseSD(); 
    float mean = noise->GetMeanBaseline(); 
    cout << "NOISE SD: " << noiseSD << endl;
    cout << "Mean Baseline: " << mean << endl;
    //float mean = noise->GetMean(); 

    // ===============================================  
    //  Clean up: 
    // ===============================================  
    reader->Delete();
    noise->Delete();

    return 0; 
}

