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
 *  Utility application for applying a threshold to images.
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
#include <svkDICOMImageWriter.h>
#include <svkIdfVolumeReader.h>
#include <svkDdfVolumeReader.h>
#include <svkDcmVolumeReader.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDICOMMRSWriter.h>
#include <svkIdfVolumeWriter.h>
#include <svkDcmHeader.h>
#include <svkBurnResearchPixels.h>
#include <svkImageThreshold.h>
#include <svkTypeUtils.h>
#include <vtkIndent.h>
#include <vtkXMLImageDataWriter.h>


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_image_threshold -i input_file_name -o output_file_name -t output_data_type   \n";
    usemsg += "                 [ -v mask_value ][-l lower_bound] [-u upper_bound][-b][-V][-h]  \n";
    usemsg += "                                                                                 \n";  
    usemsg += "   -i            input_file_name     Name of file to convert.                    \n"; 
    usemsg += "   -o            output_file_name    Name of outputfile.                         \n";  
    usemsg += "   -t            output_data_type    Target data type:                           \n";  
    usemsg += "                                         3 = UCSF IDF                            \n";  
    usemsg += "                                         5 = DICOM_MRI                           \n";  
    usemsg += "                                         6 = DICOM_Enhanced MRI                  \n";  
    usemsg += "   -v            mask_value          The integer output pixel value(default=1).  \n";
    usemsg += "   -r            mask_volume         Restrict the mask to the given volume.      \n";
    usemsg += "   -l            lower_bound         The lower bound for thresholding.           \n";
    usemsg += "                                     Intensities greater than this value will    \n";
    usemsg += "                                     be included in the mask.                    \n";
    usemsg += "   -u            upper_bound         The upper bound for thresholding.           \n";
    usemsg += "                                     Intensities less than this value will       \n";
    usemsg += "                                     be included in the mask.                    \n";
    usemsg += "   --pm          percent             threshold is 'percent' of intensity range   \n"; 
    usemsg += "                                     above minimum intensity                     \n"; 
    usemsg += "   -b                                Set output data type to byte.               \n";
    usemsg += "   --single                          Only converts specified file if multi vol.  \n";
    usemsg += "   -V                                Verbose output.                             \n";
    usemsg += "   -h                                Print help mesage.                          \n";  
    usemsg += "                                                                                 \n";  
    usemsg += "Applies a thresholding to an input image.                                        \n";
    usemsg += "                                                                                 \n";  

    string inputFileName; 
    string outputFileName; 
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::UNDEFINED; 
    bool    convertToByteMask = false;
    double  upperValue = VTK_DOUBLE_MAX;
    double  lowerValue = VTK_DOUBLE_MIN;
    double  percentOfRange = VTK_DOUBLE_MIN; 
    int     outputValue = 1;
    bool    verbose = false;
    bool    onlyLoadSingleFile = false;
    int     volume = -1;


    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_SINGLE, 
        FLAG_PERCENT_OF_RANGE
    };

    static struct option long_options[] =
    {
        {"single",      no_argument,       NULL,  FLAG_SINGLE},
        {"pm",          required_argument, NULL,  FLAG_PERCENT_OF_RANGE},
        {0, 0, 0, 0}
    };


    /*
     *  Process flags and arguments
     */
    int i;
    int option_index = 0; 
    while ((i = getopt_long(argc, argv, "i:o:t:v:l:u:r:bhV", long_options, &option_index)) != EOF) {
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
            case 'b':
                convertToByteMask = true;
                break;
            case 'l':
                lowerValue = svkTypeUtils::StringToDouble(optarg);
                break;
            case 'u':
                upperValue = svkTypeUtils::StringToDouble(optarg);
                break;            
            case 'r':
                volume = svkTypeUtils::StringToInt(optarg) - 1;
                break;
            case FLAG_SINGLE:
                onlyLoadSingleFile = true;
                break;
            case FLAG_PERCENT_OF_RANGE:
                percentOfRange = svkTypeUtils::StringToDouble(optarg);
                break;
            case 'v':
                outputValue = svkTypeUtils::StringToInt(optarg);
                break;
            case 'V':
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

    if ( argc != 0 || inputFileName.length() == 0 || outputFileName.length() == 0 ||
        dataTypeOut < 0 || dataTypeOut > svkImageWriterFactory::LAST_TYPE ) {
        cout << usemsg << endl;
        exit(1); 
    }

    if( ! svkUtils::FilePathExists( inputFileName.c_str() ) ) {
        cerr << "Input file can not be loaded (may not exist) " << inputFileName << endl;
        exit(1); 
    }

    if( verbose ) {
        cout << inputFileName << endl;
        cout << outputFileName << endl;
        cout << dataTypeOut << endl;
    }

    if( verbose ) {
        cout << "file name: " << inputFileName << endl;
    }

    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());
    readerFactory->Delete(); 

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }

    reader->SetFileName( inputFileName.c_str() );
    if ( onlyLoadSingleFile == true ) {
        reader->OnlyReadOneInputFile();
    }
    reader->Update(); 

    svkImageData* currentImage =  reader->GetOutput();

    //  Set the input command line into the data set provenance:
	currentImage->GetProvenance()->SetApplicationCommand( cmdLine );

    if( dataTypeOut <= svkImageWriterFactory::LAST_TYPE ) {
		svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
		svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter( dataTypeOut ) );

		if ( writer == NULL ) {
			cerr << "Can not determine writer of type: " << dataTypeOut << endl;
			exit(1);
		}

		writerFactory->Delete();
		writer->SetFileName( outputFileName.c_str() );
        svkImageThreshold* thresholder = svkImageThreshold::New();
        thresholder->SetInputData(currentImage);
        if( volume >= 0 ) {
            thresholder->SetVolume( volume );
        }
        if( convertToByteMask ) {
            thresholder->SetOutputScalarType( VTK_UNSIGNED_CHAR );
        }
        if ( percentOfRange != VTK_DOUBLE_MIN ) {
            double* range; 
            range = reader->GetOutput()->GetScalarRange(); 
            lowerValue = range[0] + percentOfRange * (range[1] - range[0]); 
            cout << "Threshold at " << percentOfRange << "* the intensity range above the lowest intensity:" << endl;
            cout << "             "range[0]  << endl;
        }
        cout << "Intensities within the following range will be included in the output mask" << endl;
        cout << "   LOWER THRESHOLD: " << lowerValue << endl;
        cout << "   UPPER THRESHOLD: " << upperValue << endl;
        thresholder->SetThresholdMin( lowerValue );
        thresholder->SetThresholdMax( upperValue );
        thresholder->SetMaskOutputValue( outputValue );
        thresholder->Update();
        currentImage = thresholder->GetOutput();
		writer->SetInputData( currentImage );
		writer->Write();
		writer->Delete();
		thresholder->Delete();

    } else {
        cout << usemsg << endl;
        exit(1);
    }

    reader->Delete();

    return 0; 
}

