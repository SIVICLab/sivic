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
#include <svkDdfVolumeReader.h>
#include <svkDcmVolumeReader.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDICOMMRSWriter.h>
#include <svkDcmHeader.h>
#include <svkMrsZeroFill.h>

using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) +                                       "\n";   
    usemsg += "svk_zerofill -i input_file_name -o output_root                                   \n"; 
    usemsg += "                 --custom num_pts                                                \n";
    usemsg += "                 --customx num_pts --customy num_pts --customz num_pts           \n";
    usemsg += "                 --double --doublex --doubley --doublez                          \n";
    usemsg += "                 --pow2 --pow2x --pow2y --pow2z                                  \n";
    usemsg += "                 [-vh]                                                           \n";
    usemsg += "                                                                                 \n";  
    usemsg += "   -i        input_file_name   Name of file to convert.                          \n"; 
    usemsg += "   -o        output_root       Root name of outputfile.                          \n";  
    usemsg += "   -t        type              Target data type:                                 \n";
    usemsg += "                                     2 = UCSF DDF                                \n";
    usemsg += "                                     4 = DICOM_MRS (default)                     \n";
    usemsg += "                                                                                 \n";  
    usemsg += "   --custom  num_pts           Total number of spectral points to fill to.       \n";
    usemsg += "   --customx num_pts           Total number of spatial points to fill to in X.   \n";
    usemsg += "   --customy num_pts           Total number of spatial points to fill to in Y.   \n";
    usemsg += "   --customz num_pts           Total number of spatial points to fill to in Z.   \n";
    usemsg += "                                                                                 \n";  
    usemsg += "   --double                    Double the number of spectral ponits.             \n";
    usemsg += "   --doublex                   Double the number of spatial points in X.         \n";
    usemsg += "   --doubley                   Double the number of spatial points in Y.         \n";
    usemsg += "   --doublez                   Double the number of spatial points in Z.         \n";
    usemsg += "                                                                                 \n";  
    usemsg += "   --pow2                      Increse spectral ponits to next power of 2.       \n";
    usemsg += "   --pow2x                     Increase spatial points to next power of 2 in X.  \n";
    usemsg += "   --pow2y                     Increase spatial points to next power of 2 in Y.  \n";
    usemsg += "   --pow2z                     Increase spatial points to next power of 2 in Z.  \n";
    usemsg += "                                                                                 \n";  
    usemsg += "   -v                          Verbose output.                                   \n";
    usemsg += "   -h                          Print help mesage.                                \n";  
    usemsg += "                                                                                 \n";  
    usemsg += "Zero fills spectra or spatial domains.                                           \n"; 
    usemsg += "                                                                                 \n";  

    string inputFileName; 
    string outputFileName; 
    int customPtsSpec  = -1;  
    int customPtsX     = -1;  
    int customPtsY     = -1;  
    int customPtsZ     = -1;  
    int doublePtsSpec  = -1;  
    int doublePtsX     = -1;  
    int doublePtsY     = -1;  
    int doublePtsZ     = -1;  
    int pow2PtsSpec    = -1;  
    int pow2PtsX       = -1;  
    int pow2PtsY       = -1;  
    int pow2PtsZ       = -1;  
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_MRS; 
    bool   verbose = false;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_CUSTOM_PTS_SPEC = 0,  
        FLAG_CUSTOM_PTS_X, 
        FLAG_CUSTOM_PTS_Y, 
        FLAG_CUSTOM_PTS_Z, 
        FLAG_DOUBLE_SPEC, 
        FLAG_DOUBLE_X, 
        FLAG_DOUBLE_Y,  
        FLAG_DOUBLE_Z,  
        FLAG_POW2_SPEC, 
        FLAG_POW2_X, 
        FLAG_POW2_Y,  
        FLAG_POW2_Z  
    };


    static struct option long_options[] =
    {
        {"custom",      required_argument, NULL,  FLAG_CUSTOM_PTS_SPEC},
        {"customx",     required_argument, NULL,  FLAG_CUSTOM_PTS_X},
        {"customy",     required_argument, NULL,  FLAG_CUSTOM_PTS_Y},
        {"customz",     required_argument, NULL,  FLAG_CUSTOM_PTS_Z},
        {"double",      no_argument,       NULL,  FLAG_DOUBLE_SPEC},
        {"doublex",     no_argument,       NULL,  FLAG_DOUBLE_X},
        {"doubley",     no_argument,       NULL,  FLAG_DOUBLE_Y},
        {"doublez",     no_argument,       NULL,  FLAG_DOUBLE_Z},
        {"pow2",        no_argument,       NULL,  FLAG_POW2_SPEC},
        {"pow2x",       no_argument,       NULL,  FLAG_POW2_X},
        {"pow2y",       no_argument,       NULL,  FLAG_POW2_Y},
        {"pow2z",       no_argument,       NULL,  FLAG_POW2_Z},
        {0, 0, 0, 0}
    };



    /*
    *   Process flags and arguments
    */
    int i;
    int option_index = 0; 
    while ((i = getopt_long(argc, argv, "i:o:t:vh", long_options, &option_index)) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 't':
                dataTypeOut = static_cast<svkImageWriterFactory::WriterType>( atoi(optarg) );
                break;
            case FLAG_CUSTOM_PTS_SPEC:
                customPtsSpec = atoi( optarg);
                break;
            case FLAG_CUSTOM_PTS_X:
                customPtsX = atoi( optarg);
                break;
            case FLAG_CUSTOM_PTS_Y:
                customPtsY = atoi( optarg);
                break;
            case FLAG_CUSTOM_PTS_Z:
                customPtsZ = atoi( optarg);
                break;
            case FLAG_DOUBLE_SPEC:
                doublePtsSpec = atoi( optarg);
                break;
            case FLAG_DOUBLE_X:
                doublePtsX = atoi( optarg);
                break;
            case FLAG_DOUBLE_Y:
                doublePtsY = atoi( optarg);
                break;
            case FLAG_DOUBLE_Z:
                doublePtsZ = atoi( optarg);
                break;
            case FLAG_POW2_SPEC:
                pow2PtsSpec = atoi( optarg);
                break;
            case FLAG_POW2_X:
                pow2PtsX = atoi( optarg);
                break;
            case FLAG_POW2_Y:
                pow2PtsY = atoi( optarg);
                break;
            case FLAG_POW2_Z:
                pow2PtsZ = atoi( optarg);
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


    /*
     *  Validate input. 
     */
    if ( 
        argc != 0 || inputFileName.length() == 0 || outputFileName.length() == 0 ||
        ( dataTypeOut != svkImageWriterFactory::DICOM_MRS && dataTypeOut != svkImageWriterFactory::DDF ) 
    ) {
        cout << usemsg << endl;
        exit(1); 
    }

    if( ! svkUtils::FilePathExists( inputFileName.c_str() ) ) {
        cerr << "Input file can not be loaded (may not exist) " << inputFileName << endl;
        exit(1); 
    }


    if( verbose ) {
        cout << "Input File:   " << inputFileName << endl;
        cout << "Output File:  " << outputFileName << endl;
    }


    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());
    readerFactory->Delete(); 

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }
    reader->SetFileName( inputFileName.c_str() );
    reader->Update(); 

    //  ===============================================
    //  Set the input command line into the data set provenance:
    //  ===============================================
    reader->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    //  ===============================================
    //  Zero fill data
    //  ===============================================
    svkMrsZeroFill* zeroFill = svkMrsZeroFill::New();
    zeroFill->SetInputData( reader->GetOutput() );

    if ( customPtsSpec != -1 ) {
        zeroFill->SetNumberOfSpecPoints( customPtsSpec ); 
    }
    if ( customPtsX != -1 ) {
        zeroFill->SetNumberOfColumns( customPtsX ); 
    }
    if ( customPtsY != -1 ) {
        zeroFill->SetNumberOfRows( customPtsY ); 
    }
    if ( customPtsZ != -1 ) {
        zeroFill->SetNumberOfSlices( customPtsZ ); 
    }

    if ( doublePtsSpec != -1 ) {
        zeroFill->SetNumberOfSpecPointsToDouble( ); 
    }
    if ( doublePtsX != -1 ) {
        zeroFill->SetNumberOfColumnsToDouble( ); 
    }
    if ( doublePtsY != -1 ) {
        zeroFill->SetNumberOfRowsToDouble( ); 
    }
    if ( doublePtsZ != -1 ) {
        zeroFill->SetNumberOfSlicesToDouble( ); 
    }

    if ( pow2PtsSpec != -1 ) {
        zeroFill->SetNumberOfSpecPointsToNextPower2( ); 
    }
    if ( pow2PtsX != -1 ) {
        zeroFill->SetNumberOfColumnsToNextPower2( ); 
    }
    if ( pow2PtsY != -1 ) {
        zeroFill->SetNumberOfRowsToNextPower2( ); 
    }
    if ( pow2PtsZ != -1 ) {
        zeroFill->SetNumberOfSlicesToNextPower2( ); 
    }

    zeroFill->Update();

    //  ===============================================
    //  Write results
    //  ===============================================
    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter( dataTypeOut ) );

    if ( writer == NULL ) {
        cerr << "Can not determine writer of type: " << dataTypeOut << endl;
        exit(1);
    }

    writerFactory->Delete();
    writer->SetFileName( outputFileName.c_str() );
    writer->SetInputData( zeroFill->GetOutput() );
    writer->Write();
    writer->Delete();

    zeroFill->Delete();
    reader->Delete();


    return 0; 
}

/*
        bool executeZeroFill = false;
        svkMrsZeroFill* zeroFill = svkMrsZeroFill::New();
        zeroFill->SetInputData(data);

        if( zeroFillSpec.compare("double") == 0 ) {
            zeroFill->SetNumberOfSpecPointsToDouble();
            executeZeroFill = true;
        } else if( zeroFillSpec.compare("next y^2") == 0 ) {
            zeroFill->SetNumberOfSpecPointsToNextPower2();
            executeZeroFill = true;
        } else if( zeroFillSpec.compare("custom") == 0 ) {
            zeroFill->SetNumberOfSpecPoints(this->customValueEntry->GetValueAsInt());
            executeZeroFill = true;
        }
        if( zeroFillRows.compare("double") == 0 ) {
            zeroFill->SetNumberOfRowsToDouble();
            executeZeroFill = true;
        } else if( zeroFillRows.compare("next y^2") == 0 ) {
            zeroFill->SetNumberOfRowsToNextPower2();
            executeZeroFill = true;
        }

        if( zeroFillCols.compare("double") == 0 ) {
            zeroFill->SetNumberOfColumnsToDouble();
            executeZeroFill = true;
        } else if( zeroFillCols.compare("next y^2") == 0 ) {
            zeroFill->SetNumberOfColumnsToNextPower2();
            executeZeroFill = true;
        }

        if( zeroFillSlices.compare("double") == 0 ) {
            zeroFill->SetNumberOfSlicesToDouble();
            executeZeroFill = true;
        } else if( zeroFillSlices.compare("next y^2") == 0 ) {
            zeroFill->SetNumberOfSlicesToNextPower2();
            executeZeroFill = true;
        }

        if( executeZeroFill ) {
            zeroFill->AddObserver(vtkCommand::ProgressEvent, progressCallback);
            this->GetApplication()->GetNthWindow(0)->SetStatusText("Executing Zero Fill...");
            zeroFill->Update();
*/
