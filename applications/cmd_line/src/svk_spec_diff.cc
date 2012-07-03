/*
 *  Copyright © 2009-2012 The Regents of the University of California.
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
 *  Diffs two MRS files
 *
 */

#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkDdfVolumeReader.h>
#include <svkDcmVolumeReader.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDICOMMRSWriter.h>
#include <svkCorrectDCOffset.h>
#include <svkDcmHeader.h>
#include <svkGEPFileReader.h>
#include <svkGEPFileMapper.h>
#include <svkImageAlgorithm.h>
#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#endif

#define UNDEFINED_TEMP -1111

using namespace svk;



int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_gepfile_reader -t test_file_name -r ref_file_name -o output_file_name  \n"; 
    usemsg += "                   [ -h ] \n";
    usemsg += "\n";  
    usemsg += "   -t    name   Name of file to test. \n"; 
    usemsg += "   -r    name   Name of reference file. \n"; 
    usemsg += "   -o    name   Name of outputfile. \n";
    usemsg += "   -h           Print this help mesage. \n";  
    usemsg += "\n";  
    usemsg += "Application that diffs two MRS data files. \n"; 
    usemsg += "\n";  


    string testFileName; 
    string refFileName; 
    string outputFileName;
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DDF;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 

    enum FLAG_NAME {
    }; 


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {0, 0, 0, 0}
    };

    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0; 
    while ( ( i = getopt_long(argc, argv, "t:r:o:h", long_options, &option_index) ) != EOF) {
        switch (i) {
            case 't':
                testFileName.assign( optarg );
                break;
            case 'r':
                refFileName.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
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
    if ( 
        argc != 0 ||  testFileName.length() == 0  
        || refFileName.length() == 0 
        || outputFileName.length() == 0 
    ) {
        cout << usemsg << endl;
        exit(1); 
    }


    cout << "file name (test): " << testFileName << endl;
    cout << "file name (ref) : " << refFileName << endl;

    // ===============================================  
    //  Use a reader factory to get the correct reader  
    //  type . 
    // ===============================================  
    vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 

    svkImageReader2* testReader = svkDdfVolumeReader::SafeDownCast( readerFactory->CreateImageReader2(testFileName.c_str()) );
    if (testReader == NULL) {
        cerr << "Can not determine appropriate reader for test data: " << testFileName << endl;
        exit(1);
    }
    testReader->SetFileName( testFileName.c_str() );
    testReader->Update(); 
    svkMrsImageData* testData = svkMrsImageData::SafeDownCast( testReader->GetOutput() ); 


    svkImageReader2* refReader = svkDdfVolumeReader::SafeDownCast( readerFactory->CreateImageReader2(refFileName.c_str()) );
    if (refReader == NULL) {
        cerr << "Can not determine appropriate reader for ref data: " << refFileName << endl;
        exit(1);
    }
    refReader->SetFileName( refFileName.c_str() );
    refReader->Update(); 
    svkMrsImageData* refData = svkMrsImageData::SafeDownCast( refReader->GetOutput() ); 


    //Only DDF, so only 1 channel of data:
    int cols               = refData->GetDcmHeader()->GetIntValue( "Columns" );
    int rows               = refData->GetDcmHeader()->GetIntValue( "Rows" );
    int slices             = refData->GetDcmHeader()->GetNumberOfSlices();
    int numVoxels          = cols * rows * slices;
    int numTimePts         = refData->GetDcmHeader()->GetNumberOfTimePoints();
    int numVoxelsPerVolume = numVoxels * numTimePts;
    int numSpecPts         = refData->GetDcmHeader()->GetIntValue( "DataPointColumns" );

    string representation = refData->GetDcmHeader()->GetStringValue( "DataRepresentation" );
    int numComponents = 1;
    if (representation.compare("COMPLEX") == 0) {
        numComponents = 2;
    }

    vtkFloatArray* refSpectrum; 
    vtkFloatArray* testSpectrum; 
    float* refPtr; 
    float* testPtr; 

    for( int voxelId = 0; voxelId < numVoxelsPerVolume; voxelId++ ) {

        refSpectrum = static_cast< vtkFloatArray* >( refData->GetSpectrum( voxelId) );
        testSpectrum = static_cast< vtkFloatArray* >( testData->GetSpectrum( voxelId) );
        refPtr = refSpectrum->GetPointer(0);
        testPtr = testSpectrum->GetPointer(0);

        for (int i = 0; i < numSpecPts * numComponents; i++) {

            testPtr[i] = testPtr[i] - refPtr[i]; 
        }
    }


    // ===============================================  
    //  Write the data out to the specified file type.  
    //  Use an svkImageWriterFactory to obtain the
    //  correct writer type. 
    // ===============================================  
    vtkSmartPointer< svkImageWriterFactory > writerFactory = vtkSmartPointer< svkImageWriterFactory >::New(); 
    svkImageWriter* writer = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 

    if ( writer == NULL ) {
        cerr << "Can not determine writer of type: " << dataTypeOut << endl;
        exit(1);
    }

    writer->SetFileName( outputFileName.c_str() );
    writer->SetInput( svkMrsImageData::SafeDownCast( testData ) );

    // ===============================================  
    //  Set the input command line into the data set 
    //  provenance: 
    // ===============================================  
    testData->GetProvenance()->SetApplicationCommand( cmdLine );

    // ===============================================  
    //  Write data to file: 
    // ===============================================  
    writer->Write();

    // ===============================================  
    //  Clean up: 
    // ===============================================  
    writer->Delete();
    refReader->Delete();
    testReader->Delete();

    return 0; 
}

