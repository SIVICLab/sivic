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
    usemsg += "svk_dce_ductal --adc adc_file_name --peak peak_ht_file_name                  \n"; 
    usemsg += "               -o output_file_root                                           \n"; 
    usemsg += "                                                                             \n";  
    usemsg += "   --adc         adc_file_name       Name of ADC image                       \n"; 
    usemsg += "   --peak        peak_ht_file_name   Name of Peak Height image               \n";  
    usemsg += "   -o            output_file_root    Root name of output (no extension)      \n";  
    usemsg += "   -v                                Verbose output.                         \n";
    usemsg += "   -h                                Print help mesage.                      \n";  
    usemsg += "                                                                             \n";  
    usemsg += "Combines ADC and DCE Peak Height images, creating a Ductal map               \n";  
    usemsg += "                                                                             \n";  

    string adcFileName;
    string peakHtFileName; 
    string outputFileName; 
    bool   verbose = false;

    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::UNDEFINED;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_FILE_1 = 0, 
        FLAG_FILE_2
    };

    static struct option long_options[] =
    {
        {"adc", required_argument, NULL,  FLAG_FILE_1},
        {"peak", required_argument, NULL,  FLAG_FILE_2},
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
                adcFileName.assign(optarg);
                break;
            case FLAG_FILE_2:
                peakHtFileName.assign(optarg);
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

    if ( argc != 0 || adcFileName.length() == 0 || peakHtFileName.length() == 0 || outputFileName.length() == 0 ) { 
        cout << usemsg << endl;
        exit(1); 
    }


    if( verbose ) {
        cout << "ADC:     " << adcFileName << endl;
        cout << "PEAK HT: " << peakHtFileName << endl;
        cout << "OUTPUT:  " << outputFileName << endl;
    }

    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* readerADC           = readerFactory->CreateImageReader2(adcFileName.c_str());
    svkImageReader2* readerPeakHt        = readerFactory->CreateImageReader2(peakHtFileName.c_str());

    //  Get ADC reader type: 
    if ( readerADC->IsA("svkDcmMriVolumeReader") ) {
        cout << "Input DCM MRI " << endl;
    } else if ( readerADC->IsA("svkDcmEnhancedVolumeReader") ) {
        cout << "Input DCM Enhanced MRI " << endl;
    } else if ( readerADC->IsA("svkIdfVolumeReader") ) {
        cout << "Input IDF " << endl;
    }

    //  Get PeakHt reader type: 
    if ( readerPeakHt->IsA("svkDcmMriVolumeReader") ) {
        cout << "Input DCM MRI " << endl;
        dataTypeOut = svkImageWriterFactory::DICOM_MRI;
    } else if ( readerPeakHt->IsA("svkDcmEnhancedVolumeReader") ) {
        cout << "Input DCM Enhanced MRI " << endl;
        dataTypeOut = svkImageWriterFactory::DICOM_ENHANCED_MRI;
    } else if ( readerPeakHt->IsA("svkIdfVolumeReader") ) {
        cout << "Input IDF " << endl;
        dataTypeOut = svkImageWriterFactory::IDF;
    }

    readerFactory->Delete();

    //  Load ADC and PeakHt readers
    if (readerADC == NULL) {
        cerr << "Can not determine appropriate reader for: " << adcFileName << endl;
        exit(1);
    }
    readerADC->SetFileName( adcFileName.c_str() );
    readerADC->Update();

    if (readerPeakHt == NULL) {
        cerr << "Can not determine appropriate reader for: " << peakHtFileName << endl;
        exit(1);
    }
    readerPeakHt->SetFileName( peakHtFileName.c_str() );
    readerPeakHt->Update();


    //  Set the input command line into the data set provenance:
    // readerPeakHt->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    //  Cast data to float

    //  Set up math objects: 
    svkImageMathematics* scaledADC    = svkImageMathematics::New();
    svkImageMathematics* scaledPeakHt = svkImageMathematics::New();
    svkImageMathematics* ductalTmp    = svkImageMathematics::New();
    svkImageMathematics* ductalMap    = svkImageMathematics::New();

    // Scale ADC and PeakHt
    scaledADC->SetInput1( readerADC->GetOutput() );
    scaledADC->SetConstantK( 669.0/1000.0 );
    scaledADC->SetOperationToMultiplyByK();   
    scaledADC->Update();
    scaledPeakHt->SetInput1( readerPeakHt->GetOutput() );
    scaledPeakHt->SetConstantK( 10.7/10.0 );
    scaledPeakHt->SetOperationToMultiplyByK();   
    scaledPeakHt->Update();

    // Ductal math
    ductalTmp->SetInput1( scaledADC->GetOutput() );
    ductalTmp->SetInput2( scaledPeakHt->GetOutput() );
    ductalTmp->SetOperationToSubtract();   
    ductalTmp->Update();

    ductalMap->SetInput1( ductalTmp->GetOutput() );
    ductalMap->SetConstantC( 448.0 );
    ductalMap->SetOperationToAddConstant();
    ductalMap->Update();

    svkImageMathematics* expDuctalMap      = svkImageMathematics::New();
    svkImageMathematics* scaledDuctalMap   = svkImageMathematics::New();
    svkImageMathematics* invertedDuctalMap = svkImageMathematics::New();
    svkImageMathematics* finalDuctalMap    = svkImageMathematics::New();

    expDuctalMap->SetInput1(ductalMap->GetOutput());
    expDuctalMap->SetOperationToExp();
    expDuctalMap->Update();
    scaledDuctalMap->SetInput1(expDuctalMap->GetOutput());
    scaledDuctalMap->SetConstantC( 1 );
    scaledDuctalMap->SetOperationToAddConstant();
    scaledDuctalMap->Update();
    invertedDuctalMap->SetInput1(scaledDuctalMap->GetOutput());
    invertedDuctalMap->SetOperationToInvert();
    invertedDuctalMap->Update();
    finalDuctalMap->SetInput1( invertedDuctalMap->GetOutput() );   
    finalDuctalMap->SetConstantK( 1000.0 );
    finalDuctalMap->SetOperationToMultiplyByK();   
    finalDuctalMap->Update();

    if( verbose ) {
        cout << "ADC Min:     " << readerADC->GetOutput()->GetScalarRange()[0] << endl;
        cout << "ADC Max:     " << readerADC->GetOutput()->GetScalarRange()[1] << endl;
        cout << "ADC Type:    " << readerADC->GetOutput()->GetScalarTypeAsString() << endl;

        cout << "Peak Min:    " << readerPeakHt->GetOutput()->GetScalarRange()[0] << endl;
        cout << "Peak Max:    " << readerPeakHt->GetOutput()->GetScalarRange()[1] << endl;
        cout << "Peak Type:   " << readerPeakHt->GetOutput()->GetScalarTypeAsString() << endl;

        cout << "ScADC Min:   " << scaledADC->GetOutput()->GetScalarRange()[0] << endl;
        cout << "ScADC Max:   " << scaledADC->GetOutput()->GetScalarRange()[1] << endl;
        cout << "ScADC Type:  " << scaledADC->GetOutput()->GetScalarTypeAsString() << endl;

        cout << "ScPeak Min:  " << scaledPeakHt->GetOutput()->GetScalarRange()[0] << endl;
        cout << "ScPeak Max:  " << scaledPeakHt->GetOutput()->GetScalarRange()[1] << endl;
        cout << "ScPeak Type: " << scaledPeakHt->GetOutput()->GetScalarTypeAsString() << endl;

        cout << "Tmp Min:     " << ductalTmp->GetOutput()->GetScalarRange()[0] << endl;
        cout << "Tmp Max:     " << ductalTmp->GetOutput()->GetScalarRange()[1] << endl;
        cout << "Tmp Type:    " << ductalTmp->GetOutput()->GetScalarTypeAsString() << endl;

        cout << "Map Min:     " << ductalMap->GetOutput()->GetScalarRange()[0] << endl;
        cout << "Map Max:     " << ductalMap->GetOutput()->GetScalarRange()[1] << endl;
        cout << "Map Type:    " << ductalMap->GetOutput()->GetScalarTypeAsString() << endl;
    }

    // Make final written image compatible with negative values
    svkImageCopy* signedIntMap = svkImageCopy::New();
    signedIntMap->SetInput( finalDuctalMap->GetOutput() );
    signedIntMap->SetSeriesDescription( "HighGr Map" );
    signedIntMap->SetOutputDataType( svkDcmHeader::SIGNED_INT_2 );
    signedIntMap->Update();
   
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

    readerADC->Delete();
    readerPeakHt->Delete();
    scaledADC->Delete();
    scaledPeakHt->Delete();
    ductalMap->Delete();
    expDuctalMap->Delete();
    scaledDuctalMap->Delete();
    invertedDuctalMap->Delete();
    finalDuctalMap->Delete();
    signedIntMap->Delete();

    return 0; 
}
