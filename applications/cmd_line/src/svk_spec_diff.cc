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




int main (int argc, char** argv) {

    string usemsg("\n");
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";
    usemsg += "svk_spec_diff --s1 test_file_name --s2 ref_file_name -o output_file_name \n";
    usemsg += "              [ -t dataTypeOut ] [ --norm | --norm_mag ] [ -vh ]         \n";
    usemsg += "                                                                         \n";
    usemsg += "   --s1  name                Name of MRS file 1.                         \n";
    usemsg += "   --s2  name                Name of MRS file 2.                         \n";
    usemsg += "   -o    name                Name of outputfile.                         \n";
    usemsg += "   -t    output_data_type    Output data type:                           \n";
    usemsg += "                                     2 = UCSF DDF (default)              \n";
    usemsg += "                                     4 = DICOM_MRS                       \n";
    usemsg += "   --norm                    Normalize each data set to its max real     \n";
    usemsg += "                             intensity prior to subtracting.             \n";
    usemsg += "   --norm_mag                Take the diff of the normalized magnitude   \n";
    usemsg += "                             valued data prior to subtracting.           \n";
    usemsg += "   -v                        verbose  print diff pixels                  \n";
    usemsg += "   -h                        Print this help mesage.                     \n";
    usemsg += "                                                                         \n";
    usemsg += "Application that diffs two MRS data files: s2 - s1.                      \n";
    usemsg += "                                                                         \n";


    string testFileName;
    string refFileName;
    string outputFileName;
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DDF;
    bool isVerbose = false;
    bool normalizeRealData = false;
    bool normalizeMagData  = false;

    string cmdLine = svkProvenance::GetCommandLineString(argc, argv);

    enum FLAG_NAME {
        FLAG_SPEC_1 = 0,
        FLAG_SPEC_2,
        FLAG_NORM_REAL,
        FLAG_NORM_MAG
    };


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"s1",          required_argument, NULL, FLAG_SPEC_1},
        {"s2",          required_argument, NULL, FLAG_SPEC_2},
        {"norm",        no_argument,       NULL, FLAG_NORM_REAL},
        {"norm_mag",    no_argument,       NULL, FLAG_NORM_MAG},
        {0, 0, 0, 0}
    };


    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0;
    while ((i = getopt_long(argc, argv, "t:o:hv", long_options, &option_index)) != EOF) {
        switch (i) {
            case FLAG_SPEC_1:
                testFileName.assign(optarg);
                break;
            case FLAG_SPEC_2:
                refFileName.assign(optarg);
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 't':
                dataTypeOut = static_cast<svkImageWriterFactory::WriterType>( atoi(optarg));
                break;
            case FLAG_NORM_REAL:
                normalizeRealData = true;
                break;
            case FLAG_NORM_MAG:
                normalizeMagData = true;
                break;
            case 'v':
                isVerbose = true;
                break;
            case 'h':
                cout << usemsg << endl;
                exit(1);
                break;
            default:;
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
        argc != 0 || testFileName.length() == 0
        || refFileName.length() == 0
        || outputFileName.length() == 0
    ) {
        cout << usemsg << endl;
        exit(1);
    }
    if ( normalizeRealData && normalizeMagData ) {
        cout << "norm real OR magnitude: " << usemsg << endl;
        exit(1);
    }

    cout << "file name (test): " << testFileName << endl;
    cout << "file name (ref) : " << refFileName << endl;

    // ===============================================  
    //  Use a reader factory to get the correct reader  
    //  type . 
    // ===============================================  
    vtkSmartPointer<svkImageReaderFactory> readerFactory = vtkSmartPointer<svkImageReaderFactory>::New();

    svkImageReader2 *testReader = readerFactory->CreateImageReader2(testFileName.c_str());
    if (testReader == NULL) {
        cerr << "Can not determine appropriate reader for test data: " << testFileName << endl;
        exit(1);
    }
    testReader->SetFileName(testFileName.c_str());
    testReader->Update();
    svkMrsImageData *testData = svkMrsImageData::SafeDownCast(testReader->GetOutput());

    svkImageReader2 *refReader = readerFactory->CreateImageReader2(refFileName.c_str());
    if (refReader == NULL) {
        cerr << "Can not determine appropriate reader for ref data: " << refFileName << endl;
        exit(1);
    }
    refReader->SetFileName(refFileName.c_str());
    refReader->Update();
    svkMrsImageData *refData = svkMrsImageData::SafeDownCast(refReader->GetOutput());

    svkDcmHeader::DimensionVector fullDimensionVector = refData->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector channelDimensionVector = fullDimensionVector;
    //  analyze one channel at a time: 
    svkDcmHeader::SetDimensionVectorValue(&channelDimensionVector, svkDcmHeader::CHANNEL_INDEX, 0);
    svkDcmHeader::DimensionVector indexVector = fullDimensionVector;

    int numVoxelsPerChannel = svkDcmHeader::GetNumberOfCells(&channelDimensionVector);
    int numSpecPts = refData->GetDcmHeader()->GetIntValue("DataPointColumns");
    int numChannels = svkDcmHeader::GetDimensionVectorValue(&fullDimensionVector, svkDcmHeader::CHANNEL_INDEX) + 1;

    string representation = refData->GetDcmHeader()->GetStringValue("DataRepresentation");
    int numComponents = 1;
    if (representation.compare("COMPLEX") == 0) {
        numComponents = 2;
    }


    svkMrsImageData *outImageRef;
    svkMrsImageData *outImageTest;
    if (normalizeRealData == true || normalizeMagData == true) {
        outImageRef = svkMrsImageData::New();
        outImageTest = svkMrsImageData::New();
        outImageRef->DeepCopy(testData);
        outImageTest->DeepCopy(testData);
    }

    //  First normalize if necessary.  If magnitude was specified, than normalize
    //  against maximum magnitude value
    double normalizationFactorRef;
    double normalizationFactorTest;

    if ( normalizeMagData == true ) {

        //  magnitude component range
        double magMaxRef  = VTK_DOUBLE_MIN;
        double magMaxTest = VTK_DOUBLE_MIN;

        int numCells = svkDcmHeader::GetNumberOfCells( &fullDimensionVector );
        for( int cellID = 0; cellID < numCells; cellID++ ) {

            vtkFloatArray* refSpectrum = static_cast< vtkFloatArray* >( refData->GetSpectrum( cellID) );
            vtkFloatArray* testSpectrum = static_cast< vtkFloatArray* >( testData->GetSpectrum( cellID) );
            float* refPtr;
            float* testPtr;
            refPtr  = refSpectrum->GetPointer(0);
            testPtr = testSpectrum->GetPointer(0);

            double magValRef;
            double magValTest;
            for (int i = 0; i < numSpecPts; i++) {
                magValRef  = refPtr[2*i]  * refPtr[2*i]  + refPtr[2*i+1]  * refPtr[2*i+1];
                magValTest = testPtr[2*i] * testPtr[2*i] + testPtr[2*i+1] * testPtr[2*i+1];
                if ( magValRef > magMaxRef) {
                    magMaxRef = magValRef;
                }
                if ( magValTest > magMaxTest) {
                    magMaxTest = magValTest;
                }
            }
        }
        magMaxRef  = pow(magMaxRef, .5);
        magMaxTest = pow(magMaxTest, .5);
        cout << "Ref  Range Mag: " << magMaxRef << endl;
        cout << "Test Range Mag: " << magMaxTest << endl;

        normalizationFactorRef  = magMaxRef;
        normalizationFactorTest = magMaxTest;

    } else if ( normalizeRealData == true ) {

        //  real component range
        double refRangeReal[2];
        double testRangeReal[2];
        //  imaginary component range
        double refRangeImag[2];
        double testRangeImag[2];

        refData->GetDataRange(refRangeReal, 0);
        testData->GetDataRange(testRangeReal, 0);
        refData->GetDataRange(refRangeImag, 1);
        testData->GetDataRange(testRangeImag, 1);
        cout << "Ref  Range Real: " << refRangeReal[0] << " -> " << refRangeReal[1] << endl;
        cout << "Ref  Range Imag: " << refRangeImag[0] << " -> " << refRangeImag[1] << endl;
        cout << "Test Range Real: " << testRangeReal[0] << " -> " << testRangeReal[1] << endl;
        cout << "Test Range Imag: " << testRangeImag[0] << " -> " << testRangeImag[1] << endl;

        normalizationFactorRef  = refRangeReal[1];
        normalizationFactorTest = testRangeReal[1];
    }


    //  diff each coil separately
    for ( int channel = 0; channel < numChannels; channel++ ) {
        cout << "diff channel " << channel << endl;

        for( int cellID = 0; cellID < numVoxelsPerChannel; cellID++ ) {

            //  Get the dimensions for the single channel.  reset the channel index and get the 
            //  actual cellID for this channel 
            svkDcmHeader::GetDimensionVectorIndexFromCellID(&channelDimensionVector, &indexVector, cellID); 
            svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::CHANNEL_INDEX, channel);
            int absoluteCellID = svkDcmHeader::GetCellIDFromDimensionVectorIndex(&fullDimensionVector, &indexVector); 

            vtkFloatArray* refSpectrum  = static_cast< vtkFloatArray* >( refData->GetSpectrum( absoluteCellID) );
            vtkFloatArray* testSpectrum = static_cast< vtkFloatArray* >( testData->GetSpectrum( absoluteCellID) );
            float* refPtr;
            float* testPtr;
            refPtr  = refSpectrum->GetPointer(0);
            testPtr = testSpectrum->GetPointer(0);

            vtkFloatArray* refSpectrumOut;
            vtkFloatArray* testSpectrumOut;
            float* refPtrOut;
            float* testPtrOut;
            if ( normalizeRealData == true || normalizeMagData == true ) {
                refSpectrumOut  = static_cast< vtkFloatArray * >( outImageRef->GetSpectrum(absoluteCellID));
                testSpectrumOut = static_cast< vtkFloatArray * >( outImageTest->GetSpectrum(absoluteCellID));
                refPtrOut  = refSpectrumOut->GetPointer(0);
                testPtrOut = testSpectrumOut->GetPointer(0);
            }

            bool cellDiff = false;
            for (int i = 0; i < numSpecPts; i++) {

                int indexReal = numComponents * i;
                int indexImag = numComponents * i + 1;

                if ( normalizeMagData == true) {
                    double magTest = testPtr[indexReal] * testPtr[indexReal] + testPtr[indexImag] * testPtr[indexImag];
                    double magRef  = refPtr[indexReal]  * refPtr[indexReal]  + refPtr[indexImag]  * refPtr[indexImag];
                    magTest = pow(magTest, 0.5)/normalizationFactorTest;
                    magRef  = pow(magRef, 0.5)/normalizationFactorRef;

                    testPtr[indexReal]    = magTest - magRef;
                    testPtr[indexImag]    = 0;
                    testPtrOut[indexReal] = magTest;
                    testPtrOut[indexImag] = 0;
                    refPtrOut[indexReal]  = magRef;
                    refPtrOut[indexImag]  = 0;

                } else if (normalizeRealData == true) {

                    double normRealTest = (testPtr[indexReal] / normalizationFactorTest);
                    double normImagTest = (refPtr[indexReal]  / normalizationFactorRef);
                    double normRealRef  = (testPtr[indexImag] / normalizationFactorTest);
                    double normImagRef  = (refPtr[indexImag]  / normalizationFactorRef);

                    testPtr[indexReal] = normRealTest - normRealRef;
                    testPtr[indexImag] = normImagTest - normImagRef;
                    testPtrOut[indexReal] = normRealTest;
                    testPtrOut[indexImag] = normImagTest;
                    refPtrOut[indexReal]  = normRealRef;
                    refPtrOut[indexImag]  = normImagRef;

                } else {
                    testPtr[indexReal] = testPtr[indexReal] - refPtr[indexReal];
                }

                if ( isVerbose ) {
                    if (testPtr[indexReal] != 0 ) {
                        cout << "diff: " <<  i  << " " << testPtr[indexReal] << " - " << refPtr[indexReal]
                             << " = " << testPtr[indexReal] << endl;
                        cellDiff = true; 
                    }
                }
            }
            if ( isVerbose && cellDiff ) {
                cout << "Cell contains diffs: " << absoluteCellID << endl;
            }
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
    writer->SetInputData( svkMrsImageData::SafeDownCast( testData ) );

    // ===============================================  
    //  Set the input command line into the data set 
    //  provenance: 
    // ===============================================  
    testData->GetProvenance()->SetApplicationCommand( cmdLine );

    // ===============================================  
    //  Write data to file: 
    // ===============================================  
    writer->Write();

    if ( normalizeRealData == true ) {

        writer->SetFileName( (outputFileName + "_test_norm").c_str() );
        writer->SetInputData(svkMrsImageData::SafeDownCast(outImageTest));
        outImageTest->GetProvenance()->SetApplicationCommand(cmdLine);
        writer->Write();

        writer->SetFileName( (outputFileName + "_ref_norm").c_str() );
        writer->SetInputData(svkMrsImageData::SafeDownCast(outImageRef));
        outImageTest->GetProvenance()->SetApplicationCommand(cmdLine);
        writer->Write();
    }

    if ( normalizeMagData == true ) {

        writer->SetFileName( (outputFileName + "_test_norm_mag").c_str() ) ;
        writer->SetInputData(svkMrsImageData::SafeDownCast(outImageTest));
        outImageTest->GetProvenance()->SetApplicationCommand(cmdLine);
        writer->Write();

        writer->SetFileName( (outputFileName + "_ref_norm_mag").c_str()  );
        writer->SetInputData(svkMrsImageData::SafeDownCast(outImageRef));
        outImageTest->GetProvenance()->SetApplicationCommand(cmdLine);
        writer->Write();

    }


    // ===============================================  
    //  Clean up: 
    // ===============================================  
    writer->Delete();
    refReader->Delete();
    testReader->Delete();

    return 0; 
}

