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

#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkEPSIReorder.h>
#include <svkEPSIPhaseCorrect.h>

#include <svkDcmVolumeReader.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDICOMMRSWriter.h>
#include <svkDcmHeader.h>

#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#endif

#define UNDEFINED -1

using namespace svk;



int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) +                                           "\n";   
    usemsg += "svk_reorder_epsi -i input_file_name -o output_file_name [ -t output_data_type ]      \n"; 
    usemsg += "                  --lobes                 num                                        \n";
    usemsg += "                [ --samples               num ]                                      \n";
    usemsg += "                [ --epsi_origin           num ]                                      \n";
    usemsg += "                  --skip                  num                                        \n";
    usemsg += "                [ --first                 num ]                                      \n";
    usemsg += "                  --axis                  axis                                       \n";
    usemsg += "                  --type                  type                                       \n";
    usemsg += "                [ --combine                   ]                                      \n";
    usemsg += "                [ --phase                     ]                                      \n";
    usemsg += "                                                                                     \n";  
    usemsg += "   -i            name    Name of file to convert.                                    \n"; 
    usemsg += "   -o            name    Name of outputfile.                                         \n";
    usemsg += "   -t            type    Target data type:                                           \n";
    usemsg += "                             2 = UCSF DDF                                            \n";
    usemsg += "                             4 = DICOM_MRS (default)                                 \n";
    usemsg += "   --lobes       num     Num lobes in EPSI waveform                                  \n";
    usemsg += "                         Not all samples will be represented in output data          \n"; 
    usemsg += "                         (see skip and first                                         \n";
    usemsg += "   --samples     num     Num samples per lobe.  May be inferred from                 \n";
    usemsg += "                         lobes and skips if not set.                                 \n";
    usemsg += "   --epsi_origin num     EPSI k-space point at origin, after reordering this may     \n";
    usemsg += "                         differ from the center of \"sample\" if ramps are cut off.  \n";
    usemsg += "                         (based on 0 index array)                                    \n";
    usemsg += "   --skip        num     Num samples to skip between in each cycle of waveform       \n";
    usemsg += "   --first       num     First input sample to write out, represents an initial      \n"; 
    usemsg += "                         offset of skipped samples (samples start at 1). By          \n"; 
    usemsg += "                         default first is set to 1, so no initial offset.            \n"; 
    usemsg += "   --axis        axis    EPSI axis 1, 2, 3                                           \n"; 
    usemsg += "   --type        type    Specify 1 (flyback), 2(symmetric), 3(interleaved).          \n";
    usemsg += "   --reorder     bool    Reorder the data (true or false, default = true). Can be set\n";
    usemsg += "                         to fasle if only phasing or combination is required.        \n";
    usemsg += "   --phase               Phase correct EPSI samples in each lobe.                    \n";
    usemsg += "   --combine             Combine lobes using sum of squares from existing EPSI       \n";
    usemsg += "                         data set (only for type = 2 (symmetric))                    \n";
    usemsg += "   --single              Only operates on the single specified file if               \n";
    usemsg += "   -h                    Print this help mesage.                                     \n";
    usemsg += "\n";  
    usemsg += "Reorderes an EPSI data set into a regular array of k,t ordered data separating       \n";
    usemsg += "out the spec and k-space samples from the EPSI waveform. Recomputes the FOV and      \n";  
    usemsg += "volume TLC based on the reordered spatial dimensions.  Optionally applies            \n";
    usemsg += "phase correction offset for the time delay between k-space samples in gradient.      \n"; 
    usemsg += "\n";
    usemsg += "=================================================================================    \n";
    usemsg += "Example (reorder):                                                                   \n";
    usemsg += "     svk_reorder_epsi -i input.dcm -o out_root                                       \n";
    usemsg += "                      --axis 3 --samples 16 --skip 1 --first 2                       \n";
    usemsg += "                      --lobes 53 --type 1                                            \n";
    usemsg += "\n";
    usemsg += "Example (phase only): Assumes data has already been reordered into kx,ky,kz,kt       \n";
    usemsg += "     svk_reorder_epsi -i input.dcm -o out_root --reorder false --phase               \n";
    usemsg += "                      --axis 3 --samples 16                                          \n";
    usemsg += "\n";
    usemsg += "Example (reorder and phase):                                                         \n";
    usemsg += "     svk_reorder_epsi -i input.dcm -o out_root --reorder true --phase                \n";
    usemsg += "                      --axis 3 --samples 16 --skip 1 --first 2                       \n";
    usemsg += "                      --lobes 53 --type 1                                            \n";
    usemsg += "\n";
    usemsg += "Example (combine lobes only):                                                        \n";
    usemsg += " svk_reorder_epsi -i input.dcm -o out_root --reorder false --combine                 \n";


    string inputFileName; 
    string outputFileName;
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_MRS;

    int   numLobes           = UNDEFINED; 
    int   numSamplesPerLobe  = UNDEFINED; 
    float epsiOrigin         = UNDEFINED; 
    int   skip               = UNDEFINED; 
    int   first              = 0; 
    int   epsiAxis           = UNDEFINED;
    bool  onlyLoadSingleFile = false;
    bool  reorder            = true; // default
    bool  phaseCorrect       = false;
    bool  combineLobes       = false;
    EPSIType type  = UNDEFINED_EPSI_TYPE;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 

    enum FLAG_NAME {
        FLAG_NUM_LOBES, 
        FLAG_NUM_SAMPLES_PER_LOBE, 
        FLAG_EPSI_ORIGIN, 
        FLAG_SKIP, 
        FLAG_FIRST,  
        FLAG_AXIS, 
        FLAG_TYPE, 
        FLAG_COMBINE_LOBES, 
        FLAG_PHASE,
        FLAG_REORDER,
        FLAG_SINGLE
    };


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"lobes",                   required_argument, NULL,  FLAG_NUM_LOBES},
        {"samples",                 required_argument, NULL,  FLAG_NUM_SAMPLES_PER_LOBE},
        {"epsi_origin",             required_argument, NULL,  FLAG_EPSI_ORIGIN},
        {"skip",                    required_argument, NULL,  FLAG_SKIP},
        {"first",                   required_argument, NULL,  FLAG_FIRST},
        {"axis",                    required_argument, NULL,  FLAG_AXIS},
        {"type",                    required_argument, NULL,  FLAG_TYPE},
        {"combine",                 no_argument,       NULL,  FLAG_COMBINE_LOBES},
        {"phase",                   no_argument,       NULL,  FLAG_PHASE},
        {"reorder",                 required_argument, NULL,  FLAG_REORDER},
        {"single",                  no_argument,       NULL,  FLAG_SINGLE},
        {0, 0, 0, 0}
    };

    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    string reorderBoolString;
    int i;
    int option_index = 0; 
    while ( ( i = getopt_long(argc, argv, "i:o:t:h", long_options, &option_index) ) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                if( ! svkUtils::FilePathExists( inputFileName.c_str() ) ) {
                    cerr << endl << "Input file can not be loaded (may not exist) " << inputFileName << endl << endl;
                    exit(1);
                }
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 't':
                dataTypeOut = static_cast<svkImageWriterFactory::WriterType>( atoi(optarg));
                break;
            case FLAG_NUM_LOBES:
                numLobes = atoi(optarg);
                break;
            case FLAG_NUM_SAMPLES_PER_LOBE:
                numSamplesPerLobe = atoi(optarg);
                break;
            case FLAG_EPSI_ORIGIN:
                epsiOrigin = atof(optarg);
                break;
            case FLAG_SKIP:
                skip = atoi(optarg);
                break;
            case FLAG_FIRST:
                first = atoi(optarg) - 1;
                break;
            case FLAG_AXIS:
                //  axis ordering starts at 0
                epsiAxis = atoi(optarg) - 1;
                break;
            case FLAG_TYPE:
                type = static_cast<EPSIType>(atoi(optarg));
                break;
            case FLAG_COMBINE_LOBES:
                combineLobes = true;
                break;
            case FLAG_PHASE:
                phaseCorrect = true;
                break;
            case FLAG_REORDER:
                reorderBoolString.assign(optarg);
                if (reorderBoolString.compare("false") == 0) {
                    reorder = false;
                }
                break;
            case FLAG_SINGLE:
                onlyLoadSingleFile = true;
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
    //  validate input: 
    // ===============================================  

    cout << "REORDER: " << reorder << endl;
    cout << "PHASE  : " << phaseCorrect << endl;
    cout << "COMBINE: " << combineLobes << endl;
    if ( reorder == true ) {

        if ( 
            numLobes == UNDEFINED ||
            skip == UNDEFINED ||
            first < 0 ||
            epsiAxis < 0 || epsiAxis > 2 ||
            type == UNDEFINED_EPSI_TYPE ||
            outputFileName.length() == 0 ||
            inputFileName.length() == 0  ||
            ( dataTypeOut != svkImageWriterFactory::DICOM_MRS && dataTypeOut != svkImageWriterFactory::DDF ) ||
            argc != 0 
        ) {
                cout << usemsg << endl;
                exit(1); 
        }
    }

    if ( phaseCorrect == true ) {
        if (
                outputFileName.length() == 0 ||
                inputFileName.length() == 0  ||
                ( dataTypeOut != svkImageWriterFactory::DICOM_MRS && dataTypeOut != svkImageWriterFactory::DDF ) ||
                argc != 0
                ) {
            cout << "ARGC: " << argc << endl;
            for (int x=0; x < argc; x++) {
                cout << "arg: " << argv[x] << endl;
            }
            cout << "USAGE: " << usemsg << endl;
            exit(1);
        }
    }


    if ( combineLobes == true ) {
        if (
                outputFileName.length() == 0 ||
                inputFileName.length() == 0  ||
                ( dataTypeOut != svkImageWriterFactory::DICOM_MRS && dataTypeOut != svkImageWriterFactory::DDF ) ||
                argc != 0
                ) {
            cout << "USAGE: " << usemsg << endl;
            exit(1);
        }
    }

    if( ! svkUtils::FilePathExists( inputFileName.c_str() ) ) {
        cerr << "Input file can not be loaded (may not exist) " << inputFileName << endl;
        exit(1);
    }

    cout << "file name: " << inputFileName << endl;

    // ===============================================  
    //  Use a reader factory to get the correct reader  
    //  type and load data. 
    // ===============================================  
    vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }

    if ( onlyLoadSingleFile == true ) {
        reader->OnlyReadOneInputFile();
    }
    reader->SetFileName( inputFileName.c_str() );
    reader->Update(); 


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

    svkImageData* currentImage = svkMrsImageData::SafeDownCast( reader->GetOutput() );

    svkEPSIReorder* reorderAlgo = NULL;

    if ( numSamplesPerLobe == UNDEFINED ) {
        // infer from num lobes and total number of points
        int numFrequencyPoints  = currentImage->GetDcmHeader()->GetIntValue( "DataPointColumns" );
        numSamplesPerLobe = static_cast<int>( numFrequencyPoints / numLobes ); 
    }
    cout << "Num Samples Per Lobe: " << numSamplesPerLobe << endl;

    if ( reorder == true ) {

        //  Reorder/sample EPSI data: 
        reorderAlgo = svkEPSIReorder::New();
        reorderAlgo->SetInputData( currentImage );
        reorderAlgo->SetEPSIType( type );
        reorderAlgo->SetNumSamplesToSkip( skip );
        reorderAlgo->SetNumEPSILobes( numLobes );
        reorderAlgo->SetFirstSample( first );
        reorderAlgo->SetEPSIAxis( static_cast<svkEPSIReorder::EPSIAxis>( epsiAxis ) );
        
        if ( numSamplesPerLobe != UNDEFINED ) { 
            reorderAlgo->SetNumSamplesPerLobe( numSamplesPerLobe );
        }

        reorderAlgo->Update();

        currentImage = svkMrsImageData::SafeDownCast( reorderAlgo->GetOutput() );

    }

    svkEPSIPhaseCorrect* epsiPhase = NULL;
    if ( phaseCorrect == true ) {

        // TODO: verify that input data exists
        // TODO: verify that algorithm checks for the correct domain (spectral freq?)
        epsiPhase = svkEPSIPhaseCorrect::New();
        epsiPhase->SetNumEPSIkRead( numSamplesPerLobe );
        epsiPhase->SetEPSIAxis( epsiAxis );
        epsiPhase->SetEPSIOrigin( epsiOrigin ); 
        epsiPhase->SetInputData( currentImage );
        epsiPhase->Update();
        epsiPhase->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );
        currentImage = svkMrsImageData::SafeDownCast( epsiPhase->GetOutput() );

    }

    if ( combineLobes == true ) {
        svkEPSIReorder::CombineLobes( currentImage );
    }

    // ===============================================
    //  Write data to file:
    // ===============================================
    writer->SetInputData( currentImage );
    writer->Write();

    // ===============================================
    //  Clean up: 
    // ===============================================  
    if ( reorderAlgo != NULL ) {
        reorderAlgo->Delete();
    }
    if ( epsiPhase != NULL ) {
        epsiPhase->Delete();
    }
    writer->Delete();
    reader->Delete();

    return 0; 
}

