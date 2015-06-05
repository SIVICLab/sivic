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
 */


#include <vtkSmartPointer.h>
#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkObliqueReslice.h>
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
    usemsg += "Version " + string(SVK_RELEASE_VERSION) +                                       "\n";
    usemsg += "svk_reslice -i input_to_resample --target file_to_resample_to -o output_file_name\n"; 
    usemsg += "            -t output_data_type [ --mx magX  --my magY --mz magZ ] [-h]          \n";
    usemsg += "                                                                                 \n";
    usemsg += "   -i        input_file_name     Name of file to resample.                       \n";
    usemsg += "   --target  target_file_name    File to resample input to.                      \n";
    usemsg += "   -o        output_file_root    Root name of outputfile.                        \n";
    usemsg += "   --mx      magnificationX      Magnification factor for image columns:         \n";
    usemsg += "                                 >1 downsamples (larger voxel size)              \n";
    usemsg += "                                 <1 upsamples (smaller voxel size)               \n";
    usemsg += "   --my      magnificationY      Magnification factor for image rows             \n";
    usemsg += "   --mz      magnificationZ      Magnification factor for image slices           \n";
    usemsg += "   -t        output_data_type    Output data type:                               \n";
    usemsg += "                                     3 = UCSF IDF                                \n";
    usemsg += "                                     5 = DICOM_MRI                               \n";
    usemsg += "                                     6 = DICOM_Enhanced MRI                      \n";
    usemsg += "   -h                            print help mesage.                              \n";
    usemsg += "                                                                                 \n";
    usemsg += "Resamples the input file to the orientation params specified in the target file. \n";
    usemsg += "To up/downsample it is possible to use the same input(-i) and target(--target)   \n"; 
    usemsg += "and just specify the resample magnification factors.                             \n"; 
    usemsg += "\n";

    string inputFileName;
    string targetFileName;
    string outputFileName;
    float magX = 1; 
    float magY = 1; 
    float magZ = 1; 

    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::UNDEFINED;
    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );


    enum FLAG_NAME {
        FLAG_TARGET_FILE_NAME = 0, 
        FLAG_MAG_COLS, 
        FLAG_MAG_ROWS, 
        FLAG_MAG_SLICES
    }; 
    
    
    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"target",      required_argument, NULL,  FLAG_TARGET_FILE_NAME},
        {"mx",          required_argument, NULL,  FLAG_MAG_COLS},
        {"my",          required_argument, NULL,  FLAG_MAG_ROWS},
        {"mz",          required_argument, NULL,  FLAG_MAG_SLICES},
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
            case FLAG_TARGET_FILE_NAME:
                targetFileName.assign( optarg );
                break;
            case FLAG_MAG_COLS:
                magX = atof( optarg );
                break;
            case FLAG_MAG_ROWS:
                magY = atof( optarg );
                break;
            case FLAG_MAG_SLICES:
                magZ = atof( optarg );
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
    //      an input, target and output name was supplied
    //      an output data type was supplied
    // ===============================================
    if (
        argc != 0 ||  inputFileName.length() == 0
        || targetFileName.length() == 0
        || outputFileName.length() == 0
        || ( dataTypeOut != svkImageWriterFactory::DICOM_MRI 
             && dataTypeOut != svkImageWriterFactory::IDF 
             && dataTypeOut != svkImageWriterFactory::DICOM_ENHANCED_MRI )
    ) {
        cout << usemsg << endl;
        exit(1); 
    }

    // ===============================================
    //  Use a reader factory to get the correct reader
    //  type .
    // ===============================================
    vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New();
    svkImageReader2* inputReader = readerFactory->CreateImageReader2(inputFileName.c_str());

    if ( inputReader == NULL ) { 
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }
    inputReader->SetFileName( inputFileName.c_str() );
    inputReader->Update();


    svkObliqueReslice* reslicer = svkObliqueReslice::New();
    reslicer->SetInput( inputReader->GetOutput() ); 

    svkImageReader2* targetReader = readerFactory->CreateImageReader2(targetFileName.c_str());
    if ( targetReader == NULL ) { 
        cerr << "Can not determine appropriate reader for: " << targetFileName << endl;
        exit(1);
    }
    targetReader->SetFileName( targetFileName.c_str() );
    targetReader->Update(); 
    reslicer->SetTargetDcosFromImage( targetReader->GetOutput() ); 

    if ( magX != 1 || magY != 1 || magZ !=1 ) {
        reslicer->SetMagnificationFactors( magX, magY, magZ);
    }
    reslicer->Update();

    vtkSmartPointer< svkImageWriterFactory > writerFactory = vtkSmartPointer< svkImageWriterFactory >::New();
    svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter( dataTypeOut ));
    writer->SetFileName( outputFileName.c_str() );

    writer->SetInput( reslicer->GetOutput() );
    writer->Write();
    writer->Delete();
    inputReader->Delete();
    targetReader->Delete();
    reslicer->Delete();

    return 0; 
}

