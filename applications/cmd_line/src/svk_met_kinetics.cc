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
 *   
 *  
 *
 *  Utility application for converting between supported file formats. 
 *
 */


#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>

//  Insert your algorithm here in place of "AlgoTemplate":
//#include <svkDynamicMRIAlgoTemplate.h>
#include <svkMRSKinetics.h>

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
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";
    usemsg += "svk_met_kinetics   --i1 name --i2 name [ --i3 name ]                     \n";
    usemsg += "                 [ --mask name ] -o root [ -t output_data_type ]         \n";
    usemsg += "                 [ --model type ] [ -h ]                                 \n";
    usemsg += "                                                                         \n";
    usemsg += "   --i1               name   Name of dynamic pyr signal file             \n";
    usemsg += "   --i2               name   Name of dynamic lac signal file             \n";
    usemsg += "   --i3               name   Name of dynamic urea signal file            \n";
    usemsg += "   --mask             name   Name of mask file                           \n";
    usemsg += "   -o                 root   Root Name of outputfile.  Will write:       \n";
    usemsg += "                                        root_pyr_fit.dcm                 \n";
    usemsg += "                                        root_lac_fit.dcm                 \n";
    usemsg += "                                        root_urea_fit.dcm                \n";
    usemsg += "   --model            type   Model to fit data to:                       \n";
    usemsg += "                                 1 = 2 Site Exchange(default)            \n";
    usemsg += "                                 2 = 2 Site Exchange Perf                \n";
    usemsg += "   -t                 type   Output data type:                           \n";
    usemsg += "                                 3 = UCSF IDF                            \n";
    usemsg += "                                 5 = DICOM_MRI                           \n";
    usemsg += "                                 6 = DICOM_ENHANCED_MRI (default)        \n";
    usemsg += "   --tr               tr    TR,  time in seconds between kinetic samples \n";
    usemsg += "   -h                       Print this help mesage.                      \n";
    usemsg += "                                                                         \n";
    usemsg += "Fit dynamic MRSI to metabolism kinetics model                            \n";
    usemsg += "\n";


    string inputFileName1;
    string inputFileName2;
    string inputFileName3;
    string maskFileName;
    string outputFileName = "";
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_ENHANCED_MRI;
    svkMRSKinetics::MODEL_TYPE modelType = svkMRSKinetics::TWO_SITE_EXCHANGE; 
    int    modelTypeInt = 1;
    float  tr = 0.; 

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_IM_1 = 0, 
        FLAG_IM_2, 
        FLAG_IM_3, 
        FLAG_MASK, 
        FLAG_MODEL, 
        FLAG_TR   
    };


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"i1",      required_argument, NULL,  FLAG_IM_1},
        {"i2",      required_argument, NULL,  FLAG_IM_2},
        {"i3",      required_argument, NULL,  FLAG_IM_3},
        {"mask",    required_argument, NULL,  FLAG_MASK},
        {"model",   required_argument, NULL,  FLAG_MODEL},
        {"tr",      required_argument, NULL,  FLAG_TR},
        {0, 0, 0, 0}
    };


    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0;
    while ( ( i = getopt_long(argc, argv, "o:t:usah", long_options, &option_index) ) != EOF) {
        switch (i) {
            case FLAG_IM_1:
                inputFileName1.assign( optarg );
                break;
            case FLAG_IM_2:
                inputFileName2.assign( optarg );
                break;
            case FLAG_IM_3:
                inputFileName3.assign( optarg );
                break;
            case FLAG_MASK:
                maskFileName.assign( optarg );
                break;
            case FLAG_MODEL:
                modelTypeInt = atoi( optarg );
                if (modelTypeInt == 1 ) { 
                    modelType = svkMRSKinetics::TWO_SITE_EXCHANGE; 
                } else if ( modelTypeInt == 2 ) {
                    modelType = svkMRSKinetics::TWO_SITE_EXCHANGE_PERF; 
                }
                break;
            case FLAG_TR:
                tr = atof( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 't':
                dataTypeOut = static_cast<svkImageWriterFactory::WriterType>( atoi(optarg) );
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

    //  temp kludge: 
    bool writeUrea = true; 
    if ( inputFileName3.length() == 0 ) {
        inputFileName3 = inputFileName2; 
        writeUrea = false; 
    }

    if (
        argc != 0 ||  inputFileName1.length() == 0
            || inputFileName2.length() == 0
            || inputFileName3.length() == 0
            || outputFileName.length() == 0
            || ( 
                   dataTypeOut != svkImageWriterFactory::DICOM_MRI 
                && dataTypeOut != svkImageWriterFactory::IDF 
                && dataTypeOut != svkImageWriterFactory::DICOM_ENHANCED_MRI 
                )
    ) {
            cout << usemsg << endl;
            exit(1); 
    }


    cout << "Input1: " << inputFileName1 << endl;
    cout << "Input2: " << inputFileName2 << endl;
    cout << "Input3: " << inputFileName3 << endl;
    cout << "Mask: " << maskFileName << endl;
    cout << "output root: " << outputFileName << endl;

    // ===============================================  
    //  Create the kinetic modeling instance and initialize the
    //  specific model which defines the number of inputs
    // ===============================================  
    svkMRSKinetics* dynamics = svkMRSKinetics::New();
    dynamics->SetModelType( modelType ); 
    int numberOfModelSignals = dynamics->GetNumberOfModelSignals(); 
    cout << "================" << endl;
    cout << "MODEL INPUTS: " << endl;
    cout << "================" << endl;
    for (int sig = 0; sig < numberOfModelSignals; sig++) {
        cout << "SIGNAL(" << sig << ") = " << dynamics->GetModelOutputDescription(sig) << endl; 
    }

    // ===============================================
    //  Use a reader factory to create a data reader 
    //  of the correct type for the input file format.
    // ===============================================
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader1    = readerFactory->CreateImageReader2( inputFileName1.c_str() );
    svkImageReader2* reader2    = readerFactory->CreateImageReader2( inputFileName2.c_str() );
    svkImageReader2* reader3    = readerFactory->CreateImageReader2( inputFileName3.c_str() );
    svkImageReader2* readerMask = NULL; 
    if ( maskFileName.size() > 0 ) {
        readerMask = readerFactory->CreateImageReader2( maskFileName.c_str() );
    }
    readerFactory->Delete();

    if (reader1 == NULL || reader2 == NULL || reader3 == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName1 << ", " 
             << inputFileName2 << " or " << inputFileName3 <<  endl;
            exit(1);
    }

    //  Read the data to initialize an svkImageData object
    //  If volume files are being read, interpret them as a time series
    if ( reader1->IsA("svkIdfVolumeReader") == true ) {
        svkIdfVolumeReader::SafeDownCast( reader1 )->SetMultiVolumeType(svkIdfVolumeReader::TIME_SERIES_DATA);
    }
    if ( reader2->IsA("svkIdfVolumeReader") == true ) {
        svkIdfVolumeReader::SafeDownCast( reader2 )->SetMultiVolumeType(svkIdfVolumeReader::TIME_SERIES_DATA);
    }
    if ( reader3->IsA("svkIdfVolumeReader") == true ) {
        svkIdfVolumeReader::SafeDownCast( reader3 )->SetMultiVolumeType(svkIdfVolumeReader::TIME_SERIES_DATA);
    }
    reader1->SetFileName( inputFileName1.c_str() );
    reader1->Update();
    reader2->SetFileName( inputFileName2.c_str() );
    reader2->Update();
    reader3->SetFileName( inputFileName3.c_str() );
    reader3->Update();
    if ( readerMask!= NULL ) { 
        readerMask->SetFileName( maskFileName.c_str() );
        readerMask->Update();
    }

    dynamics->SetInputConnection( 0, reader1->GetOutputPort() ); 
    dynamics->SetInputConnection( 1, reader2->GetOutputPort() ); 
    dynamics->SetInputConnection( 2, reader3->GetOutputPort() ); 
    if ( readerMask!= NULL ) { 
        dynamics->SetInputConnection( 3, readerMask->GetOutputPort() ); // input 3 is the mask
    }
    dynamics->SetTR(tr); 
    dynamics->Update();


    // ===============================================  
    //  Use writer factory to create writer for specified
    //  output file format. 
    // ===============================================  
    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();

    int numOutputs = dynamics->GetNumberOfModelOutputPorts(); 
    for ( int outIndex = 0; outIndex < numOutputs; outIndex++ ) {
        svkImageWriter* outWriter    = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 
        
        if ( outWriter == NULL ) {
            cerr << "Can not create writer of type: " << svkImageWriterFactory::DICOM_ENHANCED_MRI << endl;
            exit(1);
        }
    
        string outFile = outputFileName; 
        outFile.append("_"); 
        outFile.append( dynamics->GetModelOutputDescription(outIndex) ); 
        if ( outIndex < numberOfModelSignals ) {
            outFile.append("_fit"); 
        }
    
        outWriter->SetFileName (outFile.c_str() ); 
        outWriter->SetInputData(dynamics->GetOutput( outIndex ) ); 
        outWriter->Write(); 
        outWriter->Delete(); 
    }
    writerFactory->Delete();

    // ===============================================  
    reader1->Delete();
    reader2->Delete();
    reader3->Delete();

    return 0; 
}


