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



#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>

//  Insert your algorithm here in place of "AlgoTemplate":
//#include <svkDynamicMRIAlgoTemplate.h>
#include <svkDCEQuantify.h>

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
    usemsg += "svk_dce_quantify -i input_file_root -o outputfile_root                   \n";
    usemsg += "                 [ --mask name ] [ -t output_data_type ] [ -h ]          \n";
    usemsg += "\n";
    usemsg += "   -i            input_file_root     Root name of DCE file to quantify.  \n";
    usemsg += "   -o            output_file_root    Name of outputfiles.                \n";
    usemsg += "   -t            output_data_type    Target data type:                   \n";
    usemsg += "                                         3 = UCSF IDF                    \n";
    usemsg += "                                         5 = DICOM_MRI                   \n";
    usemsg += "                                         6 = DICOM_Enhanced MRI          \n";
    usemsg += "   --mask        root_name           Root name of mask file              \n";
    usemsg += "   -h                       Print this help mesage.                      \n";
    usemsg += "\n";
    usemsg += "Generate DCE maps.                                                       \n";
    usemsg += "\n";


    string inputFileName; 
    string maskFileName;
    string outputFileName = "";
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_ENHANCED_MRI;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_MASK_NAME = 0 
    };


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"mask",    required_argument, NULL,  FLAG_MASK_NAME},
        {0, 0, 0, 0}
    };


    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0;
    while ( ( i = getopt_long(argc, argv, "i:o:t:h", long_options, &option_index) ) != EOF) {
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
            case FLAG_MASK_NAME:
                maskFileName.assign( optarg );
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

    //  Validate input: required inputFN, outputFN and appropriate output image data type 
    if (
        argc != 0 ||  inputFileName.length() == 0
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


    cout << "Input root:  " << inputFileName << endl;
    cout << "Output root: " << outputFileName << endl;
    cout << "Mask: " << maskFileName << endl;


    // ===============================================
    //  Use a reader factory to create a data reader 
    //  of the correct type for the input file format.
    // ===============================================
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader    = readerFactory->CreateImageReader2( inputFileName.c_str() );
    svkImageReader2* readerMask = NULL; 
    if ( maskFileName.size() > 0 ) {
        readerMask = readerFactory->CreateImageReader2( maskFileName.c_str() );
    }
    readerFactory->Delete();

    if ( reader == NULL ) { 
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl; 
        exit(1);
    }

    //  Read the data to initialize an svkImageData object
    //  If volume files are being read, interpret them as a time series.  This is automatic for DICOM, but must be specified
    //  for IDF where each vol file may represent channel or time. 
    if ( reader->IsA("svkIdfVolumeReader") == true ) {
        svkIdfVolumeReader::SafeDownCast( reader )->SetMultiVolumeType(svkIdfVolumeReader::TIME_SERIES_DATA);
    }
    reader->SetFileName( inputFileName.c_str() );
    reader->Update();
    if ( readerMask!= NULL ) { 
        readerMask->SetFileName( maskFileName.c_str() );
        readerMask->Update();
    }

    // ===============================================  
    //  Initialize DCE Quantify algorithm 
    // ===============================================  
    svkDCEQuantify* dceQuant = svkDCEQuantify::New();
    dceQuant->SetInputConnection( 0, reader->GetOutputPort() ); 
    if ( readerMask!= NULL ) { 
        dceQuant->SetInputConnection( 1, readerMask->GetOutputPort() ); // input 1 is the mask
    }
    dceQuant->Update();


    // ===============================================  
    //  Use writer factory to create writer for specified
    //  output file format. 
    // ===============================================  
    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* peakHtWriter   = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 
    svkImageWriter* peakTimeWriter = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 
    svkImageWriter* maxSlopeWriter    = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 
    writerFactory->Delete();
    
    if ( peakHtWriter == NULL || peakTimeWriter == NULL || maxSlopeWriter == NULL ) { 
        cerr << "Can not create writer of type: " << dataTypeOut << endl;
        exit(1);
    }
  
    string peakHtFile   = outputFileName;  
    string peakTimeFile = outputFileName;  
    string maxSlopeFile = outputFileName;  

    peakHtFile.append("_dce_peak_ht");  
    peakTimeFile.append("_dce_peak_time");  
    maxSlopeFile.append("_dce_max_slpe");  

    peakHtWriter->SetFileName(   peakHtFile.c_str() );
    peakTimeWriter->SetFileName( peakTimeFile.c_str() );
    maxSlopeWriter->SetFileName( maxSlopeFile.c_str() );

    peakHtWriter->SetInput(   dceQuant->GetOutput(0) );      // port 0 is ph map 
    peakTimeWriter->SetInput( dceQuant->GetOutput(1) );      // port 1 is pt map 
    maxSlopeWriter->SetInput( dceQuant->GetOutput(2) );      // port 2 is max slope map  

    peakHtWriter->Write();
    peakTimeWriter->Write();
    maxSlopeWriter->Write();

    // ===============================================  


    //  clean up:
    dceQuant->Delete(); 
    peakHtWriter->Delete();
    peakTimeWriter->Delete();
    maxSlopeWriter->Delete();

    reader->Delete();

    return 0; 
}


