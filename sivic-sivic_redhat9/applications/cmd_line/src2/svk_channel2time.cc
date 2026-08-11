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

//  Insert your algorithm here in place of "AlgoTemplate":
//#include <svkDynamicMRIAlgoTemplate.h>

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
    usemsg += "Version " + string(SVK_RELEASE_VERSION) +                       "\n";
    usemsg += "svk_channel2time -i input -o output_root  [ -h ]                 \n";
    usemsg += "\n";
    usemsg += "   -i    input       Name of input volume (IDF multi-channel).   \n";
    usemsg += "   -o    output_root Root name of outputfile.                    \n";
    usemsg += "   -t    type        Target data type:                           \n";
    usemsg += "                                 4 = DICOM_MRS                   \n";
    usemsg += "                                 5 = DICOM_MRI                   \n";
    usemsg += "                                 6 = DICOM_Enhanced MRI(default) \n";
    usemsg += "   -h                Print this help mesage.                     \n";
    usemsg += "\n";
    usemsg += "Convert muiti volume IDF or DDF data set to an explicit DICOM time series   \n"; 
    usemsg += "\n";


    string inputFileName;
    string outputFileName;
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_ENHANCED_MRI;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
    };


    static struct option long_options[] =
    {
        /* This option sets a flag. */
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


    if ( argc != 0 ||  inputFileName.length() == 0 || outputFileName.length() == 0 ) {
            cout << usemsg << endl;
            exit(1); 
    }
    if ( dataTypeOut != svkImageWriterFactory::DICOM_MRI &&
         dataTypeOut != svkImageWriterFactory::DICOM_ENHANCED_MRI && 
         dataTypeOut != svkImageWriterFactory::DICOM_MRS ) {
        cout << "Data type must be 4, 5 or 6. " << endl; 
        cout << usemsg << endl;
        exit(1);

    }

    cout << inputFileName << endl;
    cout << outputFileName << endl;
    cout << dataTypeOut << endl;


    // ===============================================
    //  Use a reader factory to create a data reader 
    //  of the correct type for the input file format.
    // ===============================================
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());
    readerFactory->Delete();

    if (reader == NULL ) { 
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl; 
        exit(1);
    }

    //  Read the data to initialize an svkImageData object
    //  If volume files are being read, interpret them as a time series
    if ( reader->IsA("svkIdfVolumeReader") == true ) {
        if ( dataTypeOut != svkImageWriterFactory::DICOM_MRI &&  
             dataTypeOut != svkImageWriterFactory::DICOM_ENHANCED_MRI ) 
        {
            cerr << "Error IDF data must be written out to an image MRI image format" << endl;
            exit(1);
        }
    }
    if ( reader->IsA("svkDdfVolumeReader") == true ) {
        if ( dataTypeOut != svkImageWriterFactory::DICOM_MRS ) {
            cerr << "Error DDF data must be written out to an MRS image format" << endl;
            exit(1);
        }
    }
    reader->SetFileName( inputFileName.c_str() );
    reader->Update();

    svkDcmHeader* hdr = reader->GetOutput()->GetDcmHeader(); 
    svkDcmHeader::DimensionVector dimVec = hdr->GetDimensionIndexVector(); 
    svkDcmHeader::SwapDimensionIndexLabels( &dimVec, svkDcmHeader::CHANNEL_INDEX, svkDcmHeader::TIME_INDEX); 
    hdr->Redimension( &dimVec ); 

    // ===============================================  
    //  Use writer factory to create writer for specified
    //  output file format. 
    // ===============================================  
    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 
    writerFactory->Delete();
    if ( writer == NULL ) {
        cerr << "Can not create writer of type: " << svkImageWriterFactory::DICOM_MRS << endl;
        exit(1);
    }
    writer->SetFileName( outputFileName.c_str() );
    writer->SetInputData( reader->GetOutput() );

    writer->Write();

    //  clean up:
    writer->Delete();
    reader->Delete();

    return 0; 
}


