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

#include <vtkImageCast.h>


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_dce_cancer --t2 t2_file_name --slope slope_file_name -o output_file_root \n";
    usemsg += "               [--adc adc_file_name]                                         \n"; 
    usemsg += "                                                                             \n";
    usemsg += "   --t2          t2_file_name        Name of T2 image                        \n"; 
    usemsg += "   --slope       slope_file_name     Name of DCE slope image                 \n";
    usemsg += "   --adc         adc_file_name       Name of optional ADC image              \n"; 
    usemsg += "   -o            output_file_root    Root name of output (no extension)      \n";  
    usemsg += "   -v                                Verbose output.                         \n";
    usemsg += "   -h                                Print help mesage.                      \n";  
    usemsg += "                                                                             \n";  
    usemsg += "   Calculates cancer probability map from ADC, T2 and Slope images:          \n";
    usemsg += "      L = -15.0 + 0.00130*T2 + 0.0100*ADC - 0.0419*Slope                     \n";
    usemsg += "      Probability = 1000 / (1 + exp(L))                                      \n";
    usemsg += "                                                                             \n";  

    string adcFileName;
    string slopeFileName; 
    string t2FileName; 
    string outputFileName; 
    bool   verbose = false;

    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::UNDEFINED;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_FILE_1 = 0, 
        FLAG_FILE_2,
        FLAG_FILE_3,
    };

    static struct option long_options[] =
    {
        {"t2", required_argument, NULL,  FLAG_FILE_1},
        {"slope", required_argument, NULL,  FLAG_FILE_2},
        {"adc", required_argument, NULL,  FLAG_FILE_3},
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
                t2FileName.assign(optarg);
                break;
            case FLAG_FILE_2:
                slopeFileName.assign(optarg);
                break;
            case FLAG_FILE_3:
                adcFileName.assign( optarg );
                break;
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

    if ( argc != 0 || t2FileName.length() == 0 || slopeFileName.length() == 0 || outputFileName.length() == 0 ) { 
        cout << usemsg << endl;
        exit(1); 
    }


    if( verbose ) {
        if (adcFileName.length() == 0)
        {
            cout << "ADC:   " << adcFileName << endl;
        }
        cout << "T2:    " << t2FileName << endl;
        cout << "SLOPE: " << slopeFileName << endl;
    }

    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2*       readerT2      = readerFactory->CreateImageReader2(t2FileName.c_str());
    svkImageReader2*       readerSlope   = readerFactory->CreateImageReader2(slopeFileName.c_str());
    svkImageReader2*       readerADC     = NULL;

    if ( adcFileName.size() > 0 ) {
        readerADC = readerFactory->CreateImageReader2( adcFileName.c_str() );
        if ( readerADC->IsA("svkIdfVolumeReader") ) {
            cout << "Assuming ADC datatype is signed int." << endl;
            svkIdfVolumeReader::SafeDownCast(readerADC)->SetReadIntAsSigned(true);
        }
    }

    if ( readerSlope->IsA("svkDcmMriVolumeReader") ) {
        cout << "Input DCM MRI " << endl;
        dataTypeOut = svkImageWriterFactory::DICOM_MRI;
    } else if ( readerSlope->IsA("svkDcmEnhancedVolumeReader") ) {
        cout << "Input DCM Enhanced MRI " << endl;
        dataTypeOut = svkImageWriterFactory::DICOM_ENHANCED_MRI;
    } else if ( readerSlope->IsA("svkIdfVolumeReader") ) {
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

    //  Load readers
    if (readerT2 == NULL) {
        cerr << "Can not determine appropriate reader for: " << t2FileName << endl;
        exit(1);
    }
    readerT2->SetFileName( t2FileName.c_str() );
    readerT2->Update();

    if (readerSlope == NULL) {
        cerr << "Can not determine appropriate reader for: " << slopeFileName << endl;
        exit(1);
    }
    readerSlope->SetFileName( slopeFileName.c_str() );
    readerSlope->Update();
    // Cast data to float for math operations
    // vtkImageCast* castSlope = vtkImageCast::New();
    // castSlope->SetInput( readerSlope->GetOutput() );
    // castSlope->SetOutputScalarTypeToFloat();
    // castSlope->ClampOverflowOn();
    // castSlope->Update();
    // readerSlope->GetImageDataInput(0)->DeepCopy( castSlope->GetOutput() );
    // castSlope->Delete();

    if (readerADC != NULL) {
        readerADC->SetFileName( adcFileName.c_str() );
        readerADC->Update();
        // vtkImageCast* castADC = vtkImageCast::New();
        // castADC->SetInput( readerADC->GetOutput() );
        // castADC->SetOutputScalarTypeToFloat();
        // castADC->ClampOverflowOn();
        // castADC->Update();
        // readerADC->GetImageDataInput(0)->DeepCopy( castADC->GetOutput() );
        // castADC->Delete();
    }

    //  Set the input command line into the data set provenance:
    readerSlope->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    //  Set up math objects:
    svkImageMathematics* doubleADC   = NULL;
    svkImageMathematics* doubleSlope = svkImageMathematics::New();
    svkImageMathematics* doubleT2    = svkImageMathematics::New();
    svkImageMathematics* scaledADC   = NULL;
    svkImageMathematics* scaledSlope = svkImageMathematics::New();
    svkImageMathematics* scaledT2    = svkImageMathematics::New();
    svkImageMathematics* cancerTmp1  = svkImageMathematics::New();
    svkImageMathematics* cancerTmp2  = svkImageMathematics::New();
    svkImageMathematics* cancerMap   = svkImageMathematics::New();

    float DCE_SCALE_FACTOR           = 100.0;

    // // Cast data to float for math operations
    // doubleT2->SetInput1( readerT2->GetOutput() );
    // doubleT2->Update();
    // vtkImageCast* castT2 = vtkImageCast::New();
    // castT2->SetInput( doubleT2->GetOutput() );
    // castT2->SetOutputScalarTypeToDouble();
    // castT2->ClampOverflowOn();
    // castT2->Update();
    // doubleT2->GetImageDataInput(0)->DeepCopy( castT2->GetOutput() );
    // doubleT2->Update();
    // castT2->Delete();

    // doubleSlope->SetInput1( readerSlope->GetOutput() );
    // doubleSlope->Update();
    // vtkImageCast* castSlope = vtkImageCast::New();
    // castSlope->SetInput( doubleSlope->GetOutput() );
    // castSlope->SetOutputScalarTypeToDouble();
    // castSlope->ClampOverflowOn();
    // castSlope->Update();
    // doubleSlope->GetImageDataInput(0)->DeepCopy( castSlope->GetOutput() );
    // doubleSlope->Update();
    // castSlope->Delete();

    // if (readerADC != NULL) {
    //     doubleADC  = svkImageMathematics::New();
    //     doubleADC->SetInput1( readerADC->GetOutput() );
    //     doubleADC->Update();
    //     vtkImageCast* castADC = vtkImageCast::New();
    //     castADC->SetInput( doubleADC->GetOutput() );
    //     castADC->SetOutputScalarTypeToDouble();
    //     castADC->ClampOverflowOn();
    //     castADC->Update();
    //     doubleADC->GetImageDataInput(0)->DeepCopy( castADC->GetOutput() );
    //     doubleADC->Update();
    //     castADC->Delete();
    // }


    // Calculate cancer probability
    if (readerADC != NULL) {
        scaledADC = svkImageMathematics::New();
        scaledADC->SetInput1( readerADC->GetOutput() );
        scaledADC->SetConstantK( 0.01 );
        scaledADC->SetOperationToMultiplyByK();   
        scaledADC->Update();
        scaledT2->SetInput1( readerT2->GetOutput() );
        scaledT2->SetConstantK( 0.0013 );
        scaledT2->SetOperationToMultiplyByK();   
        scaledT2->Update();
        scaledSlope->SetInput1( readerSlope->GetOutput() );
        scaledSlope->SetConstantK( 0.0419 / DCE_SCALE_FACTOR );
        scaledSlope->SetOperationToMultiplyByK();   
        scaledSlope->Update();

        cancerTmp1->SetInput1( scaledT2->GetOutput() );
        cancerTmp1->SetInput2( scaledADC->GetOutput() );
        cancerTmp1->SetOperationToAdd();   
        cancerTmp1->Update();
        cancerTmp2->SetInput1( cancerTmp1->GetOutput() );
        cancerTmp2->SetInput2( scaledSlope->GetOutput() );
        cancerTmp2->SetOperationToSubtract();   
        cancerTmp2->Update();
        cancerMap->SetInput1(cancerTmp2->GetOutput());
        cancerMap->SetConstantC( -15.0 );
        cancerMap->SetOperationToAddConstant();
        cancerMap->Update();
    } else {
        scaledT2->SetInput1( readerT2->GetOutput() );   
        scaledT2->SetConstantK( 0.00158 );
        scaledT2->SetOperationToMultiplyByK();   
        scaledT2->Update();
        scaledSlope->SetInput1( readerSlope->GetOutput() );    
        scaledSlope->SetConstantK( 0.0272 / DCE_SCALE_FACTOR );
        scaledSlope->SetOperationToMultiplyByK();   
        scaledSlope->Update();

        cancerTmp1->SetInput1( scaledT2->GetOutput() );
        cancerTmp1->SetInput2( scaledSlope->GetOutput() );
        cancerTmp1->SetOperationToSubtract();   
        cancerTmp1->Update();
        cancerMap->SetInput1(cancerTmp1->GetOutput());
        cancerMap->SetConstantC( -1.37 );
        cancerMap->SetOperationToAddConstant();
        cancerMap->Update();
    }

    svkImageMathematics* expCancerMap      = svkImageMathematics::New();
    svkImageMathematics* scaledCancerMap   = svkImageMathematics::New();
    svkImageMathematics* invertedCancerMap = svkImageMathematics::New();
    svkImageMathematics* finalCancerMap    = svkImageMathematics::New();

    expCancerMap->SetInput1(cancerMap->GetOutput());
    expCancerMap->SetOperationToExp();
    expCancerMap->Update();
    scaledCancerMap->SetInput1(expCancerMap->GetOutput());
    scaledCancerMap->SetConstantC( 1 );
    scaledCancerMap->SetOperationToAddConstant();
    scaledCancerMap->Update();
    invertedCancerMap->SetInput1(scaledCancerMap->GetOutput());
    invertedCancerMap->SetOperationToInvert();
    invertedCancerMap->Update();
    finalCancerMap->SetInput1( invertedCancerMap->GetOutput() );   
    finalCancerMap->SetConstantK( 1000.0 );
    finalCancerMap->SetOperationToMultiplyByK();   
    finalCancerMap->Update();

    if( verbose ) {
        if (readerADC != NULL) {
            cout << "ADC Min:     " << readerADC->GetOutput()->GetScalarRange()[0] << endl;
            cout << "ADC Max:     " << readerADC->GetOutput()->GetScalarRange()[1] << endl;
            cout << "ADC Type:    " << readerADC->GetOutput()->GetScalarTypeAsString() << endl;
            // cout << "dbADC Min:   " << doublebADC->GetOutput()->GetScalarRange()[0] << endl;
            // cout << "dbADC Max:   " << doublebADC->GetOutput()->GetScalarRange()[1] << endl;
            // cout << "dbADC Type:  " << doublebADC->GetOutput()->GetScalarTypeAsString() << endl;
            cout << "scADC Min:   " << scaledADC->GetOutput()->GetScalarRange()[0] << endl;
            cout << "scADC Max:   " << scaledADC->GetOutput()->GetScalarRange()[1] << endl;
            cout << "scADC Type:  " << scaledADC->GetOutput()->GetScalarTypeAsString() << endl;
        }

        cout << "T2 Min:       " << readerT2->GetOutput()->GetScalarRange()[0] << endl;
        cout << "T2 Max:       " << readerT2->GetOutput()->GetScalarRange()[1] << endl;
        cout << "T2 Type:      " << readerT2->GetOutput()->GetScalarTypeAsString() << endl;
        // cout << "dbT2 Min:     " << doublebT2->GetOutput()->GetScalarRange()[0] << endl;
        // cout << "dbT2 Max:     " << doublebT2->GetOutput()->GetScalarRange()[1] << endl;
        // cout << "dbT2 Type:    " << doublebT2->GetOutput()->GetScalarTypeAsString() << endl;
        cout << "scT2 Min:     " << scaledT2->GetOutput()->GetScalarRange()[0] << endl;
        cout << "scT2 Max:     " << scaledT2->GetOutput()->GetScalarRange()[1] << endl;
        cout << "scT2 Type:    " << scaledT2->GetOutput()->GetScalarTypeAsString() << endl;

        cout << "Slope Min:    " << readerSlope->GetOutput()->GetScalarRange()[0] << endl;
        cout << "Slope Max:    " << readerSlope->GetOutput()->GetScalarRange()[1] << endl;
        cout << "Slope Type:   " << readerSlope->GetOutput()->GetScalarTypeAsString() << endl;
        // cout << "dbSlope Min:  " << doublebSlope->GetOutput()->GetScalarRange()[0] << endl;
        // cout << "dbSlope Max:  " << doublebSlope->GetOutput()->GetScalarRange()[1] << endl;
        // cout << "dbSlope Type: " << doublebSlope->GetOutput()->GetScalarTypeAsString() << endl;
        cout << "scSlope Min:  " << scaledSlope->GetOutput()->GetScalarRange()[0] << endl;
        cout << "scSlope Max:  " << scaledSlope->GetOutput()->GetScalarRange()[1] << endl;
        cout << "scSlope Type: " << scaledSlope->GetOutput()->GetScalarTypeAsString() << endl;

        cout << "Map Min:      " << finalCancerMap->GetOutput()->GetScalarRange()[0] << endl;
        cout << "Map Max:      " << finalCancerMap->GetOutput()->GetScalarRange()[1] << endl;
        cout << "Map Type:     " << finalCancerMap->GetOutput()->GetScalarTypeAsString() << endl;
    }

    // If the type is supported be svkImageWriterFactory then use it, otherwise use the vtkXMLWriter
    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter*        writer        = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter( dataTypeOut ) );

    if ( writer == NULL ) {
        cerr << "Can not determine writer of type: " << dataTypeOut << endl;
        exit(1);
    }

    writerFactory->Delete();
    writer->SetFileName( outputFileName.c_str() );
    writer->SetInput( floatADC->GetOutput() );
    writer->Write();
    writer->Delete();

    if (readerADC != NULL) {
        readerADC->Delete();
        scaledADC->Delete();
    }
    readerT2->Delete();
    readerSlope->Delete();
    scaledT2->Delete();
    scaledSlope->Delete();
    cancerTmp1->Delete();
    cancerTmp2->Delete();
    cancerMap->Delete();
    expCancerMap->Delete();
    scaledCancerMap->Delete();
    invertedCancerMap->Delete();
    finalCancerMap->Delete();
    // signedIntMap->Delete();

    return 0; 
}
