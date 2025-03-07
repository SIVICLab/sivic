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
#include <svkImageWriter.h>
#include <svkImageWriterFactory.h>
#include <svkImageMathematics.h>


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_scale_image -i input_file_name -o output_file_root -s scale factor       \n"; 
    usemsg += "                [ --mask mask_file_name ]                                    \n"; 
    usemsg += "                                                                             \n";  
    usemsg += "   -i            input_file_name     Name of file to scale                   \n"; 
    usemsg += "   -o            output_file_root    Root name of output (no extension)      \n";  
    usemsg += "   --mask        mask_file_name      Name of mask file                       \n";
    usemsg += "   -s            scale factor        float scaling factor                    \n";  
    usemsg += "   -v                                Verbose output.                         \n";
    usemsg += "   -h                                Print help mesage.                      \n";  
    usemsg += "                                                                             \n";  
    usemsg += "Scales all pixels in input image by scaling factor                           \n";  
    usemsg += "                                                                             \n";  

    string inputFileName; 
    string outputFileName; 
    string maskFileName; 
    float  scalingFactor = 1;  
    bool   verbose = false;
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::UNDEFINED;


    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );
    enum FLAG_NAME {
        FLAG_MASK = 0
    };

    static struct option long_options[] =
    {
        {"mask",    required_argument, NULL,  FLAG_MASK},
        {0, 0, 0, 0}
    };

    /*
    *   Process flags and arguments
    */
    int i;
    int option_index = 0; 
    while ((i = getopt_long(argc, argv, "i:o:s:hv", long_options, &option_index)) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 's':
                scalingFactor = atof(optarg);
                break;
            case FLAG_MASK:
                maskFileName.assign( optarg );
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

    if ( argc != 0 || inputFileName.length() == 0 || outputFileName.length() == 0 ) { 
        cout << usemsg << endl;
        exit(1); 
    }


    if( verbose ) {
        cout << "INPUT:   " << inputFileName << endl;
        cout << "OUTPUT:  " << outputFileName << endl;
        cout << "SCALING: " << scalingFactor << endl;
        cout << "MASK:    " << maskFileName  << endl;
    }

    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());
    svkImageReader2* readerMask = NULL;
    if ( maskFileName.size() > 0 ) {
        readerMask = readerFactory->CreateImageReader2( maskFileName.c_str() );
    }

    //  Get input reader type: 
    if ( reader->IsA("svkDcmMriVolumeReader") ) {
        cout << "Input DCM MRI " << endl;
        dataTypeOut = svkImageWriterFactory::DICOM_MRI;
    } else if ( reader->IsA("svkDcmEnhancedVolumeReader") ) {
        cout << "Input DCM Enhanced MRI " << endl;
        dataTypeOut = svkImageWriterFactory::DICOM_ENHANCED_MRI;
    } else if ( reader->IsA("svkIdfVolumeReader") ) {
        cout << "Input IDF " << endl;
        dataTypeOut = svkImageWriterFactory::IDF;
    }

    readerFactory->Delete(); 
    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }
    reader->SetFileName( inputFileName.c_str() );
    reader->Update(); 
    if ( readerMask!= NULL ) {
        readerMask->SetFileName( maskFileName.c_str() );
        readerMask->Update();
    }

    svkImageData* currentImage =  reader->GetOutput();

    //  Set the input command line into the data set provenance:
    reader->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    //  Scale image by constant factor: 
    svkImageMathematics* mathScaling = svkImageMathematics::New();
    mathScaling->SetInput1Data( reader->GetOutput() );
    //  for some reason the binary operations do not work unless a
    //  second input is defined.     
    mathScaling->SetInput2Data( reader->GetOutput() ); 
    mathScaling->SetConstantK( scalingFactor );
    mathScaling->SetOperationToMultiplyByK();   
    if ( readerMask!= NULL ) {
        mathScaling->SetInputConnection( 1, readerMask->GetOutputPort() ); // input 1 is the mask
    }
    mathScaling->Update();

    vtkImageData* output = mathScaling->GetOutput();  

    if ( readerMask!= NULL ) {
        svkImageMathematics* mathMasking = svkImageMathematics::New();
        mathMasking->SetInput1Data( mathScaling->GetOutput() );
        mathMasking->SetInputConnection( 1, readerMask->GetOutputPort() ); // input 1 is the mask
        //mathScaling->SetInput2( reader->GetOutput() ); 
        mathMasking->SetOperationToMultiply();   
        mathMasking->Update();
        output = mathMasking->GetOutput();  
    }


    // If the type is supported be svkImageWriterFactory then use it, otherwise use the vtkXMLWriter
    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter( dataTypeOut ) );

    if ( writer == NULL ) {
        cerr << "Can not determine writer of type: " << dataTypeOut << endl;
        exit(1);
    }

    writerFactory->Delete();
    writer->SetFileName( outputFileName.c_str() );
    writer->SetInputData( output );
    //writer->SetInput( mathScaling->GetOutput() );
    writer->Write();
    writer->Delete();

    reader->Delete();
    mathScaling->Delete();

    return 0; 
}

