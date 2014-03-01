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
    usemsg += "svk_met_kinetics -i1 met1 -i2 met2 -i3 met3 -o output_file_name [ -t output_data_type ] [ -h ] \n";
    usemsg += "\n";
    usemsg += "   --i1               name   Name of met 1. \n";
    usemsg += "   --i2               name   Name of met 2. \n";
    usemsg += "   --i3               name   Name of met 3. \n";
    usemsg += "   -o                 name   Name of outputfile. \n";
    usemsg += "   -t                 type   Target data type: \n";
    usemsg += "                                 3 = UCSF IDF    \n";
    usemsg += "                                 5 = DICOM_MRI (default)    \n";
    usemsg += "   -h                       Print this help mesage. \n";
    usemsg += "\n";
    usemsg += "Fit dynamic MRSI to metabolism kinetics model.\n";
    usemsg += "\n";


    string inputFileName1;
    string inputFileName2;
    string inputFileName3;
    string outputFileName;
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_MRI;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_IM_1 = 0, 
        FLAG_IM_2, 
        FLAG_IM_3
    };


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"i1",      required_argument, NULL,  FLAG_IM_1},
        {"i2",      required_argument, NULL,  FLAG_IM_2},
        {"i3",      required_argument, NULL,  FLAG_IM_3},
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


    if (
        argc != 0 ||  inputFileName1.length() == 0
            || inputFileName2.length() == 0
            || inputFileName3.length() == 0
            || outputFileName.length() == 0
            || ( dataTypeOut != svkImageWriterFactory::DICOM_MRI && dataTypeOut != svkImageWriterFactory::IDF )
    ) {
            cout << usemsg << endl;
            exit(1); 
    }


    cout << inputFileName1 << endl;
    cout << inputFileName2 << endl;
    cout << inputFileName3 << endl;
    cout << outputFileName << endl;


    // ===============================================
    //  Use a reader factory to create a data reader 
    //  of the correct type for the input file format.
    // ===============================================
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader1 = readerFactory->CreateImageReader2(inputFileName1.c_str());
    svkImageReader2* reader2 = readerFactory->CreateImageReader2(inputFileName2.c_str());
    svkImageReader2* reader3 = readerFactory->CreateImageReader2(inputFileName3.c_str());
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


    // ===============================================  
    //  Pass data through your algorithm:
    // ===============================================  
    svkMRSKinetics* dynamics = svkMRSKinetics::New();
    //svkDynamicMRIAlgoTemplate* dynamics = svkDynamicMRIAlgoTemplate::New();
    dynamics->SetInputConnection( 0, reader1->GetOutputPort() ); 
    dynamics->SetInputConnection( 1, reader2->GetOutputPort() ); 
    dynamics->SetInputConnection( 2, reader3->GetOutputPort() ); 
    dynamics->Update();


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

    // ===============================================  
    //  Write output of algorithm to file:    
    // ===============================================  
    writer->SetInput( dynamics->GetOutput(0) );     // port 0 is pyr fitted values
    //writer->SetInput( dynamics->GetOutput() );
    writer->Write();


    //  clean up:
    dynamics->Delete(); 
    writer->Delete();
    reader1->Delete();
    reader2->Delete();
    reader3->Delete();

    return 0; 
}


