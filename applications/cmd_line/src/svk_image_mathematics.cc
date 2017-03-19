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
    usemsg += "svk_image_mathematics --i1 input_file_name [ --i2 input_file_name ]          \n";  
    usemsg += "                      -o output_file_root -p operation                       \n";
    usemsg += "                      [-s scale_factor] [--output_type typeID]               \n"; 
    usemsg += "                      [ -vh ]                                                \n"; 
    usemsg += "                                                                             \n";  
    usemsg += "   --i1          input_file_name     Name of input file 1                    \n"; 
    usemsg += "   --i2          input_file_name     Name of input file 2 (binary operation) \n"; 
    usemsg += "   -o            output_file_root    Root name of output (no extension)      \n";  
    usemsg += "   -p            operation           Operator:                               \n";  
    usemsg += "                                         1 = +                               \n";  
    usemsg += "                                         2 = -                               \n";  
    usemsg += "                                         3 = *                               \n";  
    usemsg += "                                         4 = /                               \n";  
    usemsg += "                                         5 = * k (Scale by constant)         \n";  
    usemsg += "   -s            scale_factor        float scaling factor                    \n";
    usemsg += "   --output_type typeID              Optional output type:                   \n";
    usemsg += "                                         1 = Unsigned Integer                \n";  
    usemsg += "                                         2 = Float                           \n";
    usemsg += "   -v                                Verbose output.                         \n";
    usemsg += "   -h                                Print help mesage.                      \n";  
    usemsg += "                                                                             \n";  
    usemsg += "Applies specified operation to one or more input images. Dynamically         \n";  
    usemsg += "determines datatype of output image, unless specified.                       \n"; 
    usemsg += "                                                                             \n"; 

    string inputFileName1; 
    string inputFileName2; 
    string outputFileName; 
    int    operation     = 0;  
    float  scalingFactor = 1;
    bool   verbose       = false;
    int    outputType = 0;
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::UNDEFINED;


    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_FILE_1 = 0, 
        FLAG_FILE_2,
        OUTPUT_TYPE
    };

    static struct option long_options[] =
    {
        {"i1",          required_argument, NULL,  FLAG_FILE_1},
        {"i2",          required_argument, NULL,  FLAG_FILE_2},
        {"output_type", required_argument, NULL,  OUTPUT_TYPE},
        {0, 0, 0, 0}
    };

    /*
    *   Process flags and arguments
    */
    int i;
    int option_index = 0; 
    while ((i = getopt_long(argc, argv, "i:o:s:p:fhv", long_options, &option_index)) != EOF) {
        switch (i) {
            case FLAG_FILE_1:
                inputFileName1.assign( optarg );
                break;
            case FLAG_FILE_2:
                inputFileName2.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 'p':
                operation = atoi(optarg);
                break;
            case 's':
                scalingFactor = atof(optarg);
                break;
            case OUTPUT_TYPE:
                outputType = atoi(optarg);
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

    if ( argc != 0 || inputFileName1.length() == 0 || outputFileName.length() == 0 ) { 
        cout << usemsg << endl;
        exit(1); 
    }

    if ( operation < 1 || operation > 5 ) {
        cout << "Invalid operation: " << operation << endl;
        cout << usemsg << endl;
        exit(1); 
    }

    if (outputType && (outputType != 1 && outputType != 2) ) {
        cout << "Output type must be 1 for Integer, 2 for Float." << endl;
        exit(1);
    }

    if( verbose ) {
        cout << "INPUT:    " << inputFileName1 << endl;
        cout << "OUTPUT:   " << outputFileName << endl;
        cout << "OPERATOR: " << operation << endl;
    }

    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader1 = readerFactory->CreateImageReader2(inputFileName1.c_str());
    svkImageReader2* reader2 = NULL;
    if ( inputFileName2.size() > 0 ) {
        reader2 = readerFactory->CreateImageReader2( inputFileName2.c_str() );
    }

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

    readerFactory->Delete(); 
    if (reader1 == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName1 << endl;
        exit(1);
    }
    reader1->SetFileName( inputFileName1.c_str() );
    reader1->Update(); 
    if ( reader2 != NULL ) {
        reader2->SetFileName( inputFileName2.c_str() );
        reader2->Update();
    }


    //  Set the input command line into the data set provenance:
    reader1->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    //  Set up math operations: 
    svkImageMathematics* math = svkImageMathematics::New();
    math->SetInput1Data( reader1->GetOutput() );
    if ( reader2 != NULL ) {
        math->SetInput2Data( reader2->GetOutput() ); 
    }

    if ( operation == 1 ) {
        math->SetOperationToAdd();   
    } else if ( operation == 2 ) {
        math->SetOperationToSubtract();   
    } else if ( operation == 3 ) {
        math->SetOperationToMultiply();   
    } else if ( operation == 4 ) {
        math->SetOperationToDivide();   
    } else if ( operation == 5 ) {
        math->SetOperationToMultiplyByK();   
        math->SetConstantK( scalingFactor );
    }

    //  By default, output is the greater of the input datatypes
    //  Explicity request float math on two integer inputs for float results
    if(outputType) {
        math->SetOutputType(outputType);
    }
    math->Update();

    // If the type is supported be svkImageWriterFactory then use it, otherwise use the vtkXMLWriter
    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter( dataTypeOut ) );

    if ( writer == NULL ) {
        cerr << "Can not determine writer of type: " << dataTypeOut << endl;
        exit(1);
    }

    writerFactory->Delete();
    writer->SetFileName( outputFileName.c_str() );
    writer->SetInputData( math->GetOutput() );
    writer->Write();
    writer->Delete();

    reader1->Delete();
    if ( reader2 != NULL ) {
        reader2->Delete();
    }
    math->Delete();

    return 0; 
}

