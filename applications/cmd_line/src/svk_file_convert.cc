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
#include <vtkIndent.h>
#include <vtkXMLImageDataWriter.h>


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_file_convert -i input_file_name -o output_file_name -t output_data_type  \n"; 
    usemsg += "                 [ --deid_type type [ --deid_id id ] ]                       \n";
    usemsg += "                 [ --list_files ] [ --wl mode ] ] [-vh]                      \n";
    usemsg += "                                                                             \n";  
    usemsg += "   -i            input_file_name     Name of file to convert.                \n"; 
    usemsg += "   -o            output_file_name    Name of outputfile.                     \n";  
    usemsg += "   -t            output_data_type    Target data type:                       \n";  
    usemsg += "                                         2 = UCSF DDF                        \n";  
    usemsg += "                                         3 = UCSF IDF                        \n";  
    usemsg += "                                         4 = DICOM_MRS                       \n";  
    usemsg += "                                         5 = DICOM_MRI                       \n";  
    usemsg += "                                         6 = DICOM_Enhanced MRI              \n";  
    usemsg += "                                         9 = VTI                             \n";
    usemsg += "   -c                                Use lossless compression transfer       \n"; 
    usemsg += "                                     syntax for output_data_type 5 or 6.     \n"; 
    usemsg += "   --deid_type   type                Type of deidentification:               \n";
    usemsg += "                                         1 = limited (default)               \n";
    usemsg += "                                         2 = deidentified                    \n";
    usemsg += "   --deid_id     id                  Replace PHI with this identifier        \n"; 
    usemsg += "   --single                          Only converts specified file if         \n"; 
    usemsg += "   --list_files                      Only list files comprising the data set.\n"; 
    usemsg += "                                     multiple in series.                     \n";
    usemsg += "   --wl          mode                Print(p) only, or encode(e) window &    \n";                                        
    usemsg += "                                     level in output image. e not yet        \n";
    usemsg += "                                     mode = e is not yet supported. p only   \n";
    usemsg += "                                     requirs the -i input_file_name.         \n";
#if defined( UCSF_INTERNAL )
    usemsg += "   -b                                Burn UCSF Radiology Research into pixels of each image. \n";  
#endif
    usemsg += "   -v                                Verbose output.                         \n";
    usemsg += "   -h                                Print help mesage.                      \n";  
    usemsg += "                                                                             \n";  
    usemsg += "Converts the input file to the specified target file type                    \n";  
    usemsg += "                                                                             \n";  

    string inputFileName; 
    string outputFileName; 
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::UNDEFINED; 
    svkDcmHeader::PHIType deidType = svkDcmHeader::PHI_IDENTIFIED;
    string deidStudyId = ""; 
    bool   burnResearchHeader = false;  
    bool   useCompression = false;  
    bool   verbose = false;
    bool   onlyLoadSingleFile = false;
    bool   onlyListFiles = false;
    string windowLevelMode = "n";   //  do not print or encode (default)

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_DEID_TYPE, 
        FLAG_DEID_STUDY_ID, 
        FLAG_SINGLE, 
        FLAG_LIST_FILES, 
        FLAG_WL_MODE
    };


    static struct option long_options[] =
    {
        {"deid_type",   required_argument, NULL,  FLAG_DEID_TYPE}, 
        {"deid_id",     required_argument, NULL,  FLAG_DEID_STUDY_ID},
        {"single",      no_argument,       NULL,  FLAG_SINGLE},
        {"list_files",  no_argument,       NULL,  FLAG_LIST_FILES},
        {"wl",          required_argument, NULL,  FLAG_WL_MODE},
        {0, 0, 0, 0}
    };



    /*
    *   Process flags and arguments
    */
    int i;
    int option_index = 0; 
    while ((i = getopt_long(argc, argv, "i:o:t:cbhv", long_options, &option_index)) != EOF) {
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
            case FLAG_DEID_TYPE:
                deidType = static_cast<svkDcmHeader::PHIType>(atoi( optarg));
                break;
            case FLAG_DEID_STUDY_ID:
                deidStudyId.assign( optarg );
                break;
            case FLAG_SINGLE:
                onlyLoadSingleFile = true;
                break;
            case FLAG_LIST_FILES:
                onlyListFiles = true;
                break;
            case FLAG_WL_MODE:
                windowLevelMode.assign( optarg );
                break;
#if defined( UCSF_INTERNAL )
            case 'b':
                burnResearchHeader = true; 
                break;
#endif
            case 'c':
                useCompression = true; 
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
     * In addition to svkImageWriters this converter also supports an xml writer for the vtk
     * vti format. Because this writer is not a vtkImageWriter svkImagerWriterFactory cannot
     * return it so we must instantiate it outside of the factory. To account for this extra
     * type we will support a dataTypeOut equal to svkImageWriterFactory::LAST_TYPE + 1.
     */
    if ( windowLevelMode.compare("n") == 0 ) {
        //  this is the standard validation
        if ( argc != 0 || inputFileName.length() == 0 || outputFileName.length() == 0 ||
            dataTypeOut < 0 || dataTypeOut > svkImageWriterFactory::LAST_TYPE + 1 ) {
            cout << usemsg << endl;
            exit(1); 
        }
    } else {
        //  if neigher n or p, then exit: 
        if ( windowLevelMode.compare("n") != 0  && windowLevelMode.compare("p") != 0 ) {
            cout << usemsg << endl;
            exit(1); 
        }
        if ( argc != 0 || inputFileName.length() == 0 ) {
            cout << usemsg << endl;
            exit(1); 
        }
    }

    if( ! svkUtils::FilePathExists( inputFileName.c_str() ) ) {
        cerr << "Input file can not be loaded (may not exist) " << inputFileName << endl;
        exit(1); 
    }

    if ( useCompression && 
        (dataTypeOut != svkImageWriterFactory::DICOM_MRI 
            && dataTypeOut != svkImageWriterFactory::DICOM_ENHANCED_MRI ) ) {
        cout << "Can only specify compression with DICOM_MRI or DICOM_ENHANCED_MRI output types" << endl;
        cout << usemsg << endl;
        exit(1); 
    }

    if( verbose ) {
        cout << inputFileName << endl;
        cout << outputFileName << endl;
        cout << dataTypeOut << endl;
    }

    // ===============================================
    //  validate deidentification args:
    // ===============================================

    //  if a deid id was provided, deidentification is implied:
    if ( deidStudyId.compare("") != 0 && deidType == svkDcmHeader::PHI_IDENTIFIED) {
         deidType = svkDcmHeader::PHI_LIMITED;
    }

    if ( deidType != svkDcmHeader::PHI_DEIDENTIFIED &&
         deidType != svkDcmHeader::PHI_LIMITED &&
         deidType != svkDcmHeader::PHI_IDENTIFIED)
    {
        cout << "Error: invalid deidentificatin type: " << deidType <<  endl;
        cout << usemsg << endl;
        exit(1);
    } else if ( verbose ) {
        cout << "Deidentify data: type = " << deidType << endl;
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
    if ( onlyListFiles == true ) {
        reader->OnlyGlobFiles();
    }
    reader->Update(); 

    if ( windowLevelMode.compare("p") == 0 ) {
        double window; 
        double level; 
        svkMriImageData::SafeDownCast(reader->GetOutput())->GetAutoWindowLevel( window, level);
        cout << "AUTO WINDOW: " << window << endl;
        cout << "AUTO LEVEL:  " << level  << endl;
        exit(1); 
    }

    // ===============================================
    //  Set up deidentification options:
    // ===============================================
    if ( deidType != svkDcmHeader::PHI_IDENTIFIED ) {
        if ( deidStudyId.compare("") != 0 ) {
            reader->GetOutput()->GetDcmHeader()->Deidentify( deidType, deidStudyId );
        } else {
            reader->GetOutput()->GetDcmHeader()->Deidentify( deidType );
        }
    }


    svkImageData* currentImage =  reader->GetOutput();

#if defined( UCSF_INTERNAL )

    // ===============================================
    //  If UCSF build and burn RESEARCH into pixels: 
    // ===============================================
    svkBurnResearchPixels* burn = svkBurnResearchPixels::New(); 
    if ( burnResearchHeader ) {

        burn->SetInput( currentImage );  
        burn->Update(); 

        //  if this step is included, then reset the current algo to be
        //  passed to the writer.
        currentImage = burn->GetOutput();
    }

#endif

    //  Set the input command line into the data set provenance:
	currentImage->GetProvenance()->SetApplicationCommand( cmdLine );

	// If the type is supported be svkImageWriterFactory then use it, otherwise use the vtkXMLWriter
    if( dataTypeOut <= svkImageWriterFactory::LAST_TYPE ) {
		svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
		svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter( dataTypeOut ) );

		if ( writer == NULL ) {
			cerr << "Can not determine writer of type: " << dataTypeOut << endl;
			exit(1);
		}

		writerFactory->Delete();
		writer->SetFileName( outputFileName.c_str() );
		writer->SetInput( currentImage );
        if ( useCompression ) {
           svkDICOMImageWriter::SafeDownCast( writer )->UseLosslessCompression();  
        }
		writer->Write();
		writer->Delete();
    } else if (dataTypeOut == 9 ) {
        vtkXMLImageDataWriter* writer = vtkXMLImageDataWriter::New();
        writer->SetFileName( outputFileName.c_str() );
        writer->SetInput( currentImage );

        writer->Write();
        writer->Delete();
    }


    reader->Delete();

#if defined( UCSF_INTERNAL )

    burn->Delete();

#endif

    return 0; 
}

