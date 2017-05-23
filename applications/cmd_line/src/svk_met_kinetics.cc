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
#include <svkImageMathematics.h>

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
    usemsg += "                 [ --param num --lb value --ub value]                    \n";
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
    usemsg += "                                 3 = 2 Site IM                           \n";
    usemsg += "                                 4 = 2 Site IM_PYR                       \n";
    usemsg += "   -t                 type   Output data type:                           \n";
    usemsg += "                                 3 = UCSF IDF                            \n";
    usemsg += "                                 5 = DICOM_MRI                           \n";
    usemsg += "                                 6 = DICOM_ENHANCED_MRI (default)        \n";
    usemsg += "   --tr               tr    TR,  time in seconds between kinetic samples \n";
    usemsg += "   --------------------------------------------------------------------- \n"; 
    usemsg += "   --param            num   Param number to set fitting bounds for       \n";
    usemsg += "   --lb               value Lower bound for parameter search             \n";
    usemsg += "   --ub               upper Lower bound for parameter search             \n";
    usemsg += "   --------------------------------------------------------------------- \n"; 
    usemsg += "   -h                       Print this help mesage.                      \n";
    usemsg += "                                                                         \n";
    usemsg += "Fit dynamic MRSI to kinetic metabolism model                             \n";
    usemsg += "                                                                         \n";
    usemsg += "To constrain the search space for a particular param set the set of      \n"; 
    usemsg += "3 flags (--param num --lb lowerBound --ub upperBound). The model param   \n"; 
    usemsg += "numbers are printed to stdout at run time.                               \n"; 
    usemsg += "                                                                         \n";
    usemsg += "Output Volumes:                                                          \n"; 
    usemsg += "   *_fit       = fittited signals                                        \n";
    usemsg += "   *_paramName = 3D Maps of fittedm model param                          \n";
    usemsg += "   *_rss       = 3D Map  of residual sum of squares from fitted model    \n";
    usemsg += "   *_residual  = residual of input signals and fitted signals            \n";
    usemsg += "                                                                         \n";
    usemsg += "Example, fit model 3 and constrain the 3rd param to the range 0-5:       \n";
    usemsg += "    svk_met_kinetics  --i1 pyr.dcm --i2 lac.dcm --mask mask.dcm          \n"; 
    usemsg += "                      -o model3 -t 6 --tr 2 --model 3                    \n"; 
    usemsg += "                      --param 3 --lb 0 --ub 5                            \n"; 
    usemsg += "\n";

    vector <string> inputFileNames(10);
    string maskFileName;
    string outputFileName = "";
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_ENHANCED_MRI;
    svkMRSKinetics::MODEL_TYPE modelType = svkMRSKinetics::TWO_SITE_EXCHANGE; 
    int    modelTypeInt = 1;
    float  tr = 0.; 

    //  bounds are sepcified by triplets, paramNum, lowerBound upperBound.  Parse each individually 
    //  from comand line and just verify that the same number if elements is present for each.  
    vector <int>  customBoundsParamNumbers; 
    vector <float> customLowerBounds; 
    vector <float> customUpperBounds; 

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_IM_1 = 0, 
        FLAG_IM_2, 
        FLAG_IM_3, 
        FLAG_MASK, 
        FLAG_MODEL, 
        FLAG_TR,    
        FLAG_PARAM_NUM,    
        FLAG_LOWER_BOUND,    
        FLAG_UPPER_BOUND    
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
        {"param",   required_argument, NULL,  FLAG_PARAM_NUM},
        {"lb",      required_argument, NULL,  FLAG_LOWER_BOUND},
        {"ub",      required_argument, NULL,  FLAG_UPPER_BOUND},
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
                inputFileNames[0] = optarg;
                if( ! svkUtils::FilePathExists( inputFileNames[0].c_str() ) ) {
                    cerr << endl << "Input file can not be loaded (may not exist) " << inputFileNames[0] << endl << endl;
                    exit(1);
                }
                break;
            case FLAG_IM_2:
                inputFileNames[1] = optarg;
                if( ! svkUtils::FilePathExists( inputFileNames[1].c_str() ) ) {
                    cerr << endl << "Input file can not be loaded (may not exist) " << inputFileNames[1] << endl << endl;
                    exit(1);
                }
                break;
            case FLAG_IM_3:
                inputFileNames[2] = optarg;
                if( ! svkUtils::FilePathExists( inputFileNames[2].c_str() ) ) {
                    cerr << endl << "Input file can not be loaded (may not exist) " << inputFileNames[2] << endl << endl;
                    exit(1);
                }
                break;
            case FLAG_MASK:
                maskFileName.assign( optarg );
                if( ! svkUtils::FilePathExists( maskFileName.c_str() ) ) {
                    cerr << endl << "Input file can not be loaded (may not exist) " << maskFileName << endl << endl;
                    exit(1);
                }
                break;
            case FLAG_MODEL:
                modelTypeInt = atoi( optarg );
                if (modelTypeInt == 1 ) { 
                    modelType = svkMRSKinetics::TWO_SITE_EXCHANGE; 
                } else if ( modelTypeInt == 2 ) {
                    modelType = svkMRSKinetics::TWO_SITE_EXCHANGE_PERF; 
                } else if ( modelTypeInt == 3 ) {
                    modelType = svkMRSKinetics::TWO_SITE_IM; 
                } else if ( modelTypeInt == 4 ) {
                    modelType = svkMRSKinetics::TWO_SITE_IM_PYR; 
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
            case FLAG_PARAM_NUM:
                customBoundsParamNumbers.push_back(atoi( optarg ));
                break;
            case FLAG_LOWER_BOUND:
                customLowerBounds.push_back(atof( optarg ));
                break;
            case FLAG_UPPER_BOUND:
                customUpperBounds.push_back(atof( optarg ));
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

    if (
        argc != 0 
        || outputFileName.length() == 0
        || tr == 0
        || ( 
            dataTypeOut != svkImageWriterFactory::DICOM_MRI 
            && dataTypeOut != svkImageWriterFactory::IDF 
            && dataTypeOut != svkImageWriterFactory::DICOM_ENHANCED_MRI 
           )
        || ( customBoundsParamNumbers.size() != customUpperBounds.size() ) 
        || ( customBoundsParamNumbers.size() != customLowerBounds.size() ) 
    ) {
            cout << usemsg << endl;
            exit(1); 
    }

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

    //  set user defined param bounds: 
    dynamics->SetCustomParamSearchBounds(&customBoundsParamNumbers, &customLowerBounds, &customUpperBounds); 

    // ===============================================
    //  Use a reader factory to create a data reader 
    //  of the correct type for the input file format.
    // ===============================================
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    for (int sig = 0; sig < numberOfModelSignals; sig++) {
        svkImageReader2* reader    = readerFactory->CreateImageReader2( inputFileNames[sig].c_str() );

        if (reader == NULL ) {
            cerr << "Can not determine appropriate reader for: " << inputFileNames[sig] << endl; 
            exit(1);
        }

        //  Read the data to initialize an svkImageData object
        //  If volume files are being read, interpret them as a time series
        if ( reader->IsA("svkIdfVolumeReader") == true ) {
            svkIdfVolumeReader::SafeDownCast( reader )->SetMultiVolumeType(svkIdfVolumeReader::TIME_SERIES_DATA);
        }
        reader->SetFileName( inputFileNames[sig].c_str() );
        reader->Update();
        dynamics->SetInputConnection( sig, reader->GetOutputPort() ); 
        reader->Delete(); 
    }

    svkImageReader2* readerMask = NULL; 
    if ( maskFileName.size() > 0 ) {
        readerMask = readerFactory->CreateImageReader2( maskFileName.c_str() );
    }
    if ( readerMask!= NULL ) { 
        readerMask->SetFileName( maskFileName.c_str() );
        readerMask->Update();
    }
    readerFactory->Delete();

    if ( readerMask!= NULL ) { 
        dynamics->SetInputConnection( numberOfModelSignals, readerMask->GetOutputPort() ); // last input is the mask
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

    //  residuals
    for (int sig = 0; sig < numberOfModelSignals; sig++) {

        cout << "Generating residual sig: " << sig << endl; 
        svkImageMathematics* math = svkImageMathematics::New();
        math->SetInput1Data( dynamics->GetImageDataInput( sig ) );
        math->SetInput2Data( dynamics->GetOutput( sig ) );
        math->SetOperationToSubtract();
        //  set output type to 2:     
        math->SetOutputType(2);
        math->Update();

        svkImageWriter* outWriter    = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 
        
        if ( outWriter == NULL ) {
            cerr << "Can not create writer of type: " << svkImageWriterFactory::DICOM_ENHANCED_MRI << endl;
            exit(1);
        }
    
        string outFile = outputFileName; 
        outFile.append("_"); 
        outFile.append( dynamics->GetModelOutputDescription( sig ) ); 
        outFile.append("_residual"); 
    
        outWriter->SetFileName (outFile.c_str() ); 
        outWriter->SetInputData( math->GetOutput() ); 
        outWriter->Write(); 
        outWriter->Delete(); 
        math->Delete(); 
    }


    //  print out command line to visualize data: 
    string sivicCmd = "sivic"; 
    for ( int i = 0; i < numOutputs; i++ ) {

        string outFile = outputFileName; 
        outFile.append("_"); 
        outFile.append( dynamics->GetModelOutputDescription(i) ); 

        if ( i < numberOfModelSignals ) {

            string outFileResidual = outFile; 
            sivicCmd.append(" --id "); 
            sivicCmd.append( inputFileNames[i] ); 
            sivicCmd.append(" --id "); 
            outFile.append("_fit.dcm"); 
            sivicCmd.append( outFile ); 
            sivicCmd.append(" --id "); 
            outFileResidual.append("_residual.dcm"); 
            sivicCmd.append( outFileResidual ); 
        }

        if ( ( i == 0) && (maskFileName.size() > 0 ) ) {
            sivicCmd.append(" -i "); 
            sivicCmd.append( maskFileName ); 
        }

        if ( i >= numberOfModelSignals ) {
            sivicCmd.append(" -i "); 
            sivicCmd.append( outFile); 
            sivicCmd.append( ".dcm" ); 
        }
    }
    cout << endl;
    cout << "Visualize the results with the following command: " << endl; 
    cout << sivicCmd << endl;
    cout << endl;

    writerFactory->Delete();
    dynamics->Delete(); 

    return 0; 
}


