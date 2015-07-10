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
#include <svkImageMathematics.h>
#include <svkImageCopy.h>
#include <svkDcmHeader.h>


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_dce_high_grade --t2 t2_file_name --wash washout_file_name                \n";
    usemsg += "                   [--adc adc_file_name] [--slope slope_file_name]           \n";
    usemsg += "                   -o output_file_root                                       \n"; 
    usemsg += "                                                                             \n"; 
    usemsg += "   --t2          t2_file_name        Name of T2 image                        \n"; 
    usemsg += "   --wash        washout_file_name   Name of DCE washout image               \n";
    usemsg += "   --adc         adc_file_name       Name of optional* ADC image             \n";
    usemsg += "   --slope       slope_file_name     Name of optional* DCE slope image, for  \n";
    usemsg += "                                     use when there is no ADC image          \n";
    usemsg += "   -o            output_file_root    Root name of output (no extension)      \n";  
    usemsg += "   -v                                Verbose output.                         \n";
    usemsg += "   -h                                Print help mesage.                      \n";  
    usemsg += "                                                                             \n";
    usemsg += "   *Requires -EITHER- an ADC -OR- slope map.                                 \n";
    usemsg += "                                                                             \n";
    usemsg += "   Calculates probability high grade (>=4+3), based on JMP model:            \n";
    usemsg += "      L = -13.6 + 0.00205*T2 + 0.00821*ADC + 0.0113*Washout                  \n";
    usemsg += "          OR                                                                 \n";
    usemsg += "      L = -1.6 + 0.00137*T2 - 0.0000928*Slope + 0.00246*Washout                 \n";
    usemsg += "      Probability = 1000 / (1 + exp(L))                                      \n";
    usemsg += "                                                                             \n";  

    string t2FileName;
    string washoutFileName;
    string adcFileName; 
    string slopeFileName; 
    string outputFileName; 
    bool   verbose = false;

    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::UNDEFINED;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_FILE_1 = 0, 
        FLAG_FILE_2,
        FLAG_FILE_3,
        FLAG_FILE_4
    };

    static struct option long_options[] =
    {
        {"t2", required_argument, NULL,  FLAG_FILE_1},
        {"wash", required_argument, NULL,  FLAG_FILE_2},
        {"adc", required_argument, NULL,  FLAG_FILE_3},
        {"slope", required_argument, NULL,  FLAG_FILE_4},
        {0, 0, 0, 0}
    };


    /*
    *   Process flags and arguments
    */
    int i;
    int option_index = 0; 
    while ((i = getopt_long(argc, argv, "o:hv", long_options, &option_index)) != EOF) {
        switch (i) {
            case FLAG_FILE_1:
                t2FileName.assign( optarg );;
                break;
            case FLAG_FILE_2:
                washoutFileName.assign( optarg );;
                break;
            case FLAG_FILE_3:
                adcFileName.assign( optarg );
                break;
            case FLAG_FILE_4:
                slopeFileName.assign( optarg );;
            case 'o':
                outputFileName.assign(optarg);
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

    if ( argc != 0 || washoutFileName.length() == 0 || t2FileName.length() == 0 || outputFileName.length() == 0 ) { 
        cout << usemsg << endl;
        exit(1); 
    }
    // Needs either and adc or slope, not none and not both
    if ( ( adcFileName.length() == 0 && slopeFileName.length() == 0 ) || ( adcFileName.length() != 0 && slopeFileName.length() != 0 ) ) { 
        cout << "Please supply either an ADC -OR- slope map." << endl;
        cout << endl;
        cout << usemsg << endl;
        exit(1); 
    }

    if( verbose ) {
        if (adcFileName.length() == 0)
        {
            cout << "ADC:     " << adcFileName << endl;
        } else {
            cout << "SLOPE:   " << slopeFileName << endl;
        }
        cout << "T2:      " << t2FileName << endl;
        cout << "WASHOUT: " << washoutFileName << endl;
    }

    // Image reader set-up
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2*       readerT2      = readerFactory->CreateImageReader2(t2FileName.c_str());
    svkImageReader2*       readerWashout = readerFactory->CreateImageReader2(washoutFileName.c_str());
    svkImageReader2*       readerADC;
    svkImageReader2*       readerSlope;
    if ( adcFileName.size() > 0 ) {
        readerADC = readerFactory->CreateImageReader2( adcFileName.c_str() );
        if ( readerADC->IsA("svkDcmMriVolumeReader") ) {
            cout << "Input DCM MRI " << endl;
        } else if ( readerADC->IsA("svkDcmEnhancedVolumeReader") ) {
            cout << "Input DCM Enhanced MRI " << endl;
        } else if ( readerADC->IsA("svkIdfVolumeReader") ) {
            cout << "Input IDF " << endl;
        }
    } else {
        readerSlope = readerFactory->CreateImageReader2( slopeFileName.c_str() );
        if ( readerSlope->IsA("svkDcmMriVolumeReader") ) {
            cout << "Input DCM MRI " << endl;
        } else if ( readerSlope->IsA("svkDcmEnhancedVolumeReader") ) {
            cout << "Input DCM Enhanced MRI " << endl;
        } else if ( readerSlope->IsA("svkIdfVolumeReader") ) {
            cout << "Input IDF " << endl;
        }
    }

    if ( readerWashout->IsA("svkDcmMriVolumeReader") ) {
        cout << "Input DCM MRI " << endl;
        dataTypeOut = svkImageWriterFactory::DICOM_MRI;
    } else if ( readerWashout->IsA("svkDcmEnhancedVolumeReader") ) {
        cout << "Input DCM Enhanced MRI " << endl;
        dataTypeOut = svkImageWriterFactory::DICOM_ENHANCED_MRI;
    } else if ( readerWashout->IsA("svkIdfVolumeReader") ) {
        cout << "Input IDF " << endl;
        dataTypeOut = svkImageWriterFactory::IDF;
    }

    if ( readerT2->IsA("svkDcmMriVolumeReader") ) {
        cout << "Input DCM MRI " << endl;
    } else if ( readerT2->IsA("svkDcmEnhancedVolumeReader") ) {
        cout << "Input DCM Enhanced MRI " << endl;
    } else if ( readerT2->IsA("svkIdfVolumeReader") ) {
        cout << "Input IDF " << endl;
    }

    readerFactory->Delete();

    // Load readers
    if (readerT2 == NULL) {
        cerr << "Can not determine appropriate reader for: " << t2FileName << endl;
        exit(1);
    }
    readerT2->SetFileName( t2FileName.c_str() );
    readerT2->Update();

    if (readerWashout == NULL) {
        cerr << "Can not determine appropriate reader for: " << washoutFileName << endl;
        exit(1);
    }
    readerWashout->SetFileName( washoutFileName.c_str() );
    readerWashout->Update();

    if (readerADC == NULL) {
        if (readerSlope == NULL) {
            cerr << "Can not determine appropriate ADC/Slope reader" << endl;
            exit(1);
        } else {
            readerSlope->SetFileName( slopeFileName.c_str() );
            readerSlope->Update();
        }
    } else {
        readerADC->SetFileName( adcFileName.c_str() );
        readerADC->Update();
    }

    //  Set the input command line into the data set provenance:
    readerWashout->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    //  Set up math objects:
    svkImageMathematics* scaledADC     = NULL;
    svkImageMathematics* scaledSlope   = NULL;
    svkImageMathematics* scaledWashout = svkImageMathematics::New();
    svkImageMathematics* scaledT2      = svkImageMathematics::New();
    svkImageMathematics* highGrMap     = svkImageMathematics::New();
    svkImageMathematics* highGrTmp1    = svkImageMathematics::New();
    svkImageMathematics* highGrTmp2    = svkImageMathematics::New();

    float DCE_SCALE_FACTOR             = 100.0;

    // Create highGr map
    if (readerADC != NULL) {
        scaledADC = svkImageMathematics::New();
        scaledADC->SetInput1( readerADC->GetOutput() );   
        scaledADC->SetConstantK( 0.00821 );
        scaledADC->SetOperationToMultiplyByK();   
        scaledADC->Update();
        scaledT2->SetInput1( readerT2->GetOutput() );   
        scaledT2->SetConstantK( 0.00205 );
        scaledT2->SetOperationToMultiplyByK();   
        scaledT2->Update();
        scaledWashout->SetInput1( readerWashout->GetOutput() );    
        scaledWashout->SetConstantK( 1.13 / DCE_SCALE_FACTOR );
        scaledWashout->SetOperationToMultiplyByK();   
        scaledWashout->Update();

        highGrTmp1->SetInput1( scaledT2->GetOutput() );
        highGrTmp1->SetInput2( scaledADC->GetOutput() );
        highGrTmp1->SetOperationToAdd();   
        highGrTmp1->Update();
        highGrTmp2->SetInput1( highGrTmp1->GetOutput() );
        highGrTmp2->SetInput2( scaledWashout->GetOutput() );
        highGrTmp2->SetOperationToSubtract();   
        highGrTmp2->Update();
        highGrMap->SetInput1( highGrTmp2->GetOutput() );
        highGrMap->SetConstantC( -13.6 );
        highGrMap->SetOperationToAddConstant();
        highGrMap->Update();
    } else {
        scaledSlope = svkImageMathematics::New();
        scaledSlope->SetInput1( readerSlope->GetOutput() );   
        scaledSlope->SetConstantK( 0.00928 / DCE_SCALE_FACTOR);
        scaledSlope->SetOperationToMultiplyByK();   
        scaledSlope->Update();
        scaledT2->SetInput1( readerT2->GetOutput() );   
        scaledT2->SetConstantK( 0.00137 );
        scaledT2->SetOperationToMultiplyByK();   
        scaledT2->Update();
        scaledWashout->SetInput1( readerWashout->GetOutput() );    
        scaledWashout->SetConstantK( 0.246 / DCE_SCALE_FACTOR );
        scaledWashout->SetOperationToMultiplyByK();   
        scaledWashout->Update();

        highGrTmp1->SetInput1( scaledT2->GetOutput() );
        highGrTmp1->SetInput2( scaledSlope->GetOutput() );
        highGrTmp1->SetOperationToAdd();   
        highGrTmp1->Update();
        highGrTmp2->SetInput1( highGrTmp1->GetOutput() );
        highGrTmp2->SetInput2( scaledWashout->GetOutput() );
        highGrTmp2->SetOperationToSubtract();   
        highGrTmp2->Update();
        highGrMap->SetInput1( highGrTmp2->GetOutput() );
        highGrMap->SetConstantC( -1.64 );
        highGrMap->SetOperationToAddConstant();
        highGrMap->Update();
    }

    svkImageMathematics* expHighGrMap      = svkImageMathematics::New();
    svkImageMathematics* scaledHighGrMap   = svkImageMathematics::New();
    svkImageMathematics* invertedHighGrMap = svkImageMathematics::New();
    svkImageMathematics* finalHighGrMap    = svkImageMathematics::New();

    expHighGrMap->SetInput1(highGrMap->GetOutput());
    expHighGrMap->SetOperationToExp();
    expHighGrMap->Update();
    scaledHighGrMap->SetInput1(expHighGrMap->GetOutput());
    scaledHighGrMap->SetConstantC( 1 );
    scaledHighGrMap->SetOperationToAddConstant();
    scaledHighGrMap->Update();
    invertedHighGrMap->SetInput1(scaledHighGrMap->GetOutput());
    invertedHighGrMap->SetOperationToInvert();
    invertedHighGrMap->Update();
    finalHighGrMap->SetInput1( invertedHighGrMap->GetOutput() );   
    finalHighGrMap->SetConstantK( 1000.0 );
    finalHighGrMap->SetOperationToMultiplyByK();   
    finalHighGrMap->Update();

    // Make final written image compatible with negative values
    svkImageCopy* signedIntMap = svkImageCopy::New();
    signedIntMap->SetInput( finalHighGrMap->GetOutput() );
    signedIntMap->SetSeriesDescription( "HighGr Map" );
    signedIntMap->SetOutputDataType( svkDcmHeader::SIGNED_INT_2 );
    signedIntMap->Update();

    // Write out probability map
    // If the type is supported be svkImageWriterFactory then use it, otherwise use the vtkXMLWriter

    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter*        writer        = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter( dataTypeOut ) );

    if ( writer == NULL ) {
        cerr << "Can not determine writer of type: " << dataTypeOut << endl;
        exit(1);
    }

    writerFactory->Delete();
    writer->SetFileName( outputFileName.c_str() );
    writer->SetInput( signedIntMap->GetOutput() );
    writer->Write();
    writer->Delete();

    if (readerADC != NULL) {
        readerADC->Delete();
        scaledADC->Delete();
    }
    readerT2->Delete();
    readerWashout->Delete();
    readerSlope->Delete();
    scaledWashout->Delete();
    scaledT2->Delete();
    scaledSlope->Delete();
    highGrTmp1->Delete();
    highGrTmp2->Delete();
    highGrMap->Delete();
    expHighGrMap->Delete();
    scaledHighGrMap->Delete();
    invertedHighGrMap->Delete();
    finalHighGrMap->Delete();
    signedIntMap->Delete();

    return 0; 
}
