/*
 *  Copyright © 2009-2012 The Regents of the University of California.
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
#include <svkDdfVolumeReader.h>
#include <svkDcmVolumeReader.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDICOMMRSWriter.h>
#include <svkDdfVolumeWriter.h>
#include <svkCorrectDCOffset.h>
#include <svkDcmHeader.h>
#include <svkGEPFileReader.h>
#include <svkGEPFileMapper.h>
#include <svkImageAlgorithm.h>
#include <svkMultiCoilPhase.h>
#include <svkCoilCombine.h>
#include <svkMrsImageFFT.h>
#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#endif
#define UNDEFINED_TEMP -1111

using namespace svk;



int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_fft -i input_file_name -o output_file_name [ -t output_data_type ] \n"; 
    usemsg += "                   --spec [ -h ] \n";
    usemsg += "\n";  
    usemsg += "   -i                name   Name of file to convert. \n"; 
    usemsg += "   -o                name   Name of outputfile. \n";
    usemsg += "   -t                type   Target data type: \n";
    usemsg += "                                 2 = UCSF DDF      \n";
    usemsg += "                                 4 = DICOM_MRS (default)    \n";
    usemsg += "   --spec                   transform spectral domain only\n"; 
    usemsg += "   --spatial                transform spatial domain only\n"; 
    usemsg += "   -h                       Print this help mesage. \n";  
    usemsg += "\n";  
    usemsg += "Performs spatial/spectral FFTs.  If specified will transform only the specified domain.\n";  
    usemsg += "\n";  


    string inputFileName; 
    string outputFileName;
    bool transformSpecDomain = true; 
    bool transformSpatialDomain = true; 

    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_MRS;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 

    enum FLAG_NAME {
        FLAG_TRANSFORM_SPEC_DOMAIN = 0, 
        FLAG_TRANSFORM_SPATIAL_DOMAIN = 1
    }; 


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"spec",      no_argument,       NULL,  FLAG_TRANSFORM_SPEC_DOMAIN},
        {"spatial",   no_argument,       NULL,  FLAG_TRANSFORM_SPATIAL_DOMAIN},
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
            case FLAG_TRANSFORM_SPEC_DOMAIN:
                transformSpecDomain = true;
                transformSpatialDomain = false; 
                break;
            case FLAG_TRANSFORM_SPATIAL_DOMAIN:
                transformSpecDomain = false;
                transformSpatialDomain = true; 
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
    //  validate that: 
    //      an output name was supplied
    //      that not suppresses and unsuppressed were both specified 
    //      that only the supported output types was requested. 
    //      
    // ===============================================  
    if ( argc != 0 ||  inputFileName.length() == 0  
         || outputFileName.length() == 0 
         || ( dataTypeOut != svkImageWriterFactory::DICOM_MRS && dataTypeOut != svkImageWriterFactory::DDF ) 
         
    ) {
        cout << usemsg << endl;
        exit(1); 
    }

    cout << "file name: " << inputFileName << endl;

    // ===============================================  
    //  Use a reader factory to get the correct reader  
    //  type . 
    // ===============================================  
    vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }

    reader->SetFileName( inputFileName.c_str() );
    reader->Update(); 

    // ===============================================  
    //  fft
    // ===============================================  

    svkDcmHeader* hdr = reader->GetOutput()->GetDcmHeader();
    if ( transformSpecDomain ) {

        svkMrsImageFFT* imageFFT = svkMrsImageFFT::New();
        imageFFT->SetInput( reader->GetOutput() );

        imageFFT->SetFFTDomain( svkMrsImageFFT::SPECTRAL ); 
        string specDomain = hdr->GetStringValue( "SignalDomainColumns"); 
        if ( specDomain.compare("TIME") == 0 ) {
            //  time to frequency: 
            imageFFT->SetFFTMode( svkMrsImageFFT::FORWARD ); 
        } else {
            //  frequency to time: 
            imageFFT->SetFFTMode( svkMrsImageFFT::REVERSE ); 
        }

        imageFFT->Update();
        imageFFT->Delete();
    }

    if (transformSpatialDomain ) { 

        svkMrsImageFFT* spatialRFFT = svkMrsImageFFT::New();

        spatialRFFT->SetInput( reader->GetOutput() );
        spatialRFFT->SetFFTDomain( svkMrsImageFFT::SPATIAL );

        string domainCol = hdr->GetStringValue( "SVK_ColumnsDomain");
        if ( domainCol.compare("SPACE") == 0) {
            spatialRFFT->SetFFTMode( svkMrsImageFFT::FORWARD);
        } else {
            spatialRFFT->SetFFTMode( svkMrsImageFFT::REVERSE );
        }
        spatialRFFT->SetPreCorrectCenter( true );
        spatialRFFT->SetPostCorrectCenter( true );
        spatialRFFT->Update();
        spatialRFFT->Delete();

    }



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
    writer->SetInput( svkMrsImageData::SafeDownCast( reader->GetOutput() ) );

    // ===============================================  
    //  Set the input command line into the data set 
    //  provenance: 
    // ===============================================  
    reader->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    // ===============================================  
    //  Write data to file: 
    // ===============================================  
    writer->Write();

    // ===============================================  
    //  Clean up: 
    // ===============================================  
    writer->Delete();
    reader->Delete();

    return 0; 
}

