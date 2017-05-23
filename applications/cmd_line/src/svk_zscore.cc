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
 *  $URL: svn+ssh://jccrane@svn.code.sf.net/p/sivic/code/trunk/applications/cmd_line/src/svk_image_mathematics.cc $
 *  $Rev: 2282 $
 *  $Author: jccrane $
 *  $Date: 2015-07-07 15:53:20 -0700 (Tue, 07 Jul 2015) $
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 *
 *  Application to create a z-score map of the ration of two metabolite maps. 
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
#include <svkImageWriter.h>
#include <svkImageWriterFactory.h>
#include <svkMetaboliteRatioZScores.h>

#define UNDEFINED_THRESHOLD -999

using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_zscore --i1 input_file_name  --i2 input_file_name                        \n";  
    usemsg += "           -o output_file_root [ --spec input_file_name ]                    \n";  
    usemsg += "           [-u upperThreshold && -l lowerThreshold ] [ -vh ]                 \n"; 
    usemsg += "                                                                             \n";  
    usemsg += "   --i1          input_file_name     Name of input file 1 (numerator)        \n"; 
    usemsg += "   --i2          input_file_name     Name of input file 2 (denominator)      \n"; 
    usemsg += "   --spec        input_file_name     Name of MRS file with selectin box def  \n"; 
    usemsg += "   -o            output_file_root    Root name of output (no extension)      \n";  
    usemsg += "   -u            threshold           Upper threshold: number std devs above  \n";  
    usemsg += "                                     regression                              \n";  
    usemsg += "   -l            threshold           Lower threshold: number of std devs     \n";  
    usemsg += "                                     below regression                        \n";  
    usemsg += "   -v                                Verbose output.                         \n";
    usemsg += "   -h                                Print help mesage.                      \n";  
    usemsg += "                                                                             \n";  
    usemsg += "Computes the z-score of the ratio of the two input metabolite maps.          \n";  
    usemsg += "Thrsholds specify the cut off for iterative exclusion of points from         \n";  
    usemsg += "regression.  Both are specified as positive values, e.g. -l 2 tresholds      \n";  
    usemsg += "points 2 std deviations below the regression.  Distance is measured normal   \n";  
    usemsg += "to regression.                                                               \n";  
    usemsg += "                                                                             \n";  

    string inputFileName1; 
    string inputFileName2; 
    string inputFileNameMRS; 
    string outputFileName; 
    float  lowerThreshold = UNDEFINED_THRESHOLD;  
    float  upperThreshold = UNDEFINED_THRESHOLD;  
    bool   verbose = false;
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::UNDEFINED;


    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );
    enum FLAG_NAME {
        FLAG_FILE_1 = 0, 
        FLAG_FILE_2, 
        FLAG_FILE_MRS
    };

    static struct option long_options[] =
    {
        {"i1",    required_argument, NULL,  FLAG_FILE_1},
        {"i2",    required_argument, NULL,  FLAG_FILE_2},
        {"spec",  required_argument, NULL,  FLAG_FILE_MRS},
        {0, 0, 0, 0}
    };

    /*
    *   Process flags and arguments
    */
    int i;
    int option_index = 0; 
    while ((i = getopt_long(argc, argv, "o:l:u:hv", long_options, &option_index)) != EOF) {
        switch (i) {
            case FLAG_FILE_1:
                inputFileName1.assign( optarg );
                break;
            case FLAG_FILE_2:
                inputFileName2.assign( optarg );
                break;
            case FLAG_FILE_MRS:
                inputFileNameMRS.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 'l':
                lowerThreshold = atof(optarg);
                break;
            case 'u':
                upperThreshold = atof(optarg);
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

    if ( argc != 0 || inputFileName1.length() == 0 || 
         inputFileName2.length() == 0 || 
         outputFileName.length() == 0 ) 
    { 
        cout << usemsg << endl;
        exit(1); 
    }

    if ( ( lowerThreshold != UNDEFINED_THRESHOLD &&  upperThreshold == UNDEFINED_THRESHOLD ) || 
         ( lowerThreshold == UNDEFINED_THRESHOLD &&  upperThreshold != UNDEFINED_THRESHOLD ) )
    {
        cout << "If specifying a threshold must specify both upper and lower. " << endl;
        cout << usemsg << endl;
    }
    if( verbose ) {
        cout << "INPUT:    " << inputFileName1 << endl;
        cout << "INPUT:    " << inputFileName2 << endl;
        cout << "OUTPUT:   " << outputFileName << endl;
        cout << "Threshold: " << lowerThreshold << " -> " << upperThreshold << endl;
    }

    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader1 = readerFactory->CreateImageReader2(inputFileName1.c_str());
    svkImageReader2* reader2 = NULL;
    reader2 = readerFactory->CreateImageReader2( inputFileName2.c_str() );

    //  Get input reader type: 
    if ( reader1->IsA("svkDcmMriVolumeReader") ) {
        cout << "Input DCM MRI " << endl;
        dataTypeOut = svkImageWriterFactory::DICOM_MRI;
    } else if ( reader1->IsA("svkDcmEnhancedVolumeReader") ) {
        cout << "Input DCM Enhanced MRI " << endl;
        dataTypeOut = svkImageWriterFactory::DICOM_ENHANCED_MRI;
    } else if ( reader1->IsA("svkIdfVolumeReader") ) {
        cout << "Input IDF " << endl;
        dataTypeOut = svkImageWriterFactory::IDF;
    }


    if (reader1 == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName1 << endl;
        exit(1);
    }
    if (reader2 == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName2 << endl;
        exit(1);
    }

    svkImageReader2* readerMRS = NULL; 
    if ( inputFileNameMRS.length() != 0 ) { 
        readerMRS = readerFactory->CreateImageReader2(inputFileNameMRS.c_str());
        if (readerMRS == NULL) {
            cerr << "Can not determine appropriate reader for: " << inputFileNameMRS << endl;
            exit(1);
        }
    }

    readerFactory->Delete(); 

    reader1->SetFileName( inputFileName1.c_str() );
    reader1->Update(); 
    reader2->SetFileName( inputFileName2.c_str() );
    reader2->Update();
    if (readerMRS != NULL) {
        readerMRS->SetFileName( inputFileNameMRS.c_str() );
        readerMRS->Update();
    }


    //  Set the input command line into the data set provenance:
    reader1->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    //  Scale image by constant factor: 
    svkMetaboliteRatioZScores* zscore = svkMetaboliteRatioZScores::New();
    zscore->SetInputNumerator( reader1->GetOutput() );
    zscore->SetInputDenominator( reader2->GetOutput() );
    if (readerMRS != NULL) {
        zscore->SetInputMrsData( readerMRS->GetOutput() );
        zscore->LimitToSelectedVolume();
    }
    if ( lowerThreshold != UNDEFINED_THRESHOLD && upperThreshold != UNDEFINED_THRESHOLD ) {
        zscore->SetZScoreThresholds( lowerThreshold, upperThreshold); 
    }
    zscore->Update();
    int status = zscore->GetErrorCode();

    // If the type is supported be svkImageWriterFactory then use it, otherwise use the vtkXMLWriter
    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter( dataTypeOut ) );

    if ( writer == NULL ) {
        cerr << "Can not determine writer of type: " << dataTypeOut << endl;
        exit(1);
    }

    //vtkDataArray* out = zscore->GetOutput()->GetPointData()->GetScalars();
    //for ( int i = 0; i < 800; i++ ) {
            //cout << " OUT " << i << " " << out->GetTuple1(i) << endl;;
    //}


    writerFactory->Delete();
    writer->SetFileName( outputFileName.c_str() );
    writer->SetInputData( zscore->GetOutput() );
    writer->Write();
    writer->Delete();

    zscore->Delete();
    reader1->Delete();
    reader2->Delete();
    if (readerMRS != NULL) {
        readerMRS->Delete();
    }

    return 0; 
}

