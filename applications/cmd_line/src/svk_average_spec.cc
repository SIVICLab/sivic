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
 *
 */

#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDcmHeader.h>
#include <svkMRSAverageSpectra.h>

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
    usemsg += "svk_average_spec -i input_file_name -o output_file_root                      \n"; 
    usemsg += "                   --mask mask_file_name | -b                                \n"; 
    usemsg += "                   [ -mvh ]                                                  \n";
    usemsg += "                                                                             \n";  
    usemsg += "   -i            input_file_name     Name of file to scale                   \n";
    usemsg += "   -o            output_file_root    Root name of output (no extension)      \n";
    usemsg += "   --mask        mask_file_name      Name of mask file                       \n";
    usemsg += "   --all                             Average over all non-spatial dims.  By  \n";
    usemsg += "                                     default returns the average spectrum in \n";
    usemsg += "                                     the spatial ROI separately for each     \n";
    usemsg += "                                     channel, etc.                           \n";
    usemsg += "   -b                                Average spectral within selection box.  \n"; 
    usemsg += "   -m                                Average of magnitude spectra in ROI.    \n"; 
    usemsg += "   -v                                Verbose output                          \n"; 
    usemsg += "   -h                                Print this help mesage.                 \n";  
    usemsg += "\n";  
    usemsg += "Application that averages spectral voxles in the specified ROI or selection  \n";
    usemsg += "box.  Exports a single voxel data set with average spectrum and voxel size   \n";
    usemsg += "= FOV. By default this returns a complex spectrum, but can average the       \n"; 
    usemsg += "magnitude spectra within the ROI as well (-m flag).                          \n"; 
    usemsg += "\n";  

    string inputFileName; 
    string outputFileName;
    string maskFileName; 
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::UNDEFINED;
    bool isVerbose = false; 
    bool limitToSelectionBox = false; 
    bool useMagnitudeSpectra = false; 
    bool averageOverNonSpatialDims = false; 


    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 

    enum FLAG_NAME {
        FLAG_MASK = 0, 
        FLAG_AVERAGE_NON_SPATIAL
    };

    static struct option long_options[] =
    {
        {"mask",    required_argument, NULL,  FLAG_MASK},
        {"all",     no_argument,       NULL,  FLAG_AVERAGE_NON_SPATIAL},
        {0, 0, 0, 0}
    };


    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0; 
    while ( ( i = getopt_long(argc, argv, "i:o:bmhv", long_options, &option_index) ) != EOF) {
        switch (i) {
           case 'i':
                inputFileName.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case FLAG_MASK:
                maskFileName.assign( optarg );
                break;
            case FLAG_AVERAGE_NON_SPATIAL:
                averageOverNonSpatialDims = true; 
                break;
            case 'b':
                limitToSelectionBox = true; 
                break;
            case 'm':
                useMagnitudeSpectra = true; 
                break;
            case 'v':
                isVerbose = true; 
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
    if ( argc != 0 || 
        inputFileName.length() == 0 || 
        outputFileName.length() == 0 ||
        ( maskFileName.length() == 0  && limitToSelectionBox == false )  || 
        ( maskFileName.length() > 0  && limitToSelectionBox == true )  
    )  {
        cout << usemsg << endl;
        exit(1);
    }

    if( isVerbose ) {
        cout << "INPUT:   " << inputFileName << endl;
        cout << "OUTPUT:  " << outputFileName << endl;
    }

    // ===============================================  
    //  Use a reader factory to get the correct reader  
    //  type . 
    // ===============================================  
    vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 

    svkImageReader2* mrsReader = readerFactory->CreateImageReader2( inputFileName.c_str() );
    if ( mrsReader == NULL ) {
        cerr << "Can not determine appropriate reader for input: " << inputFileName << endl;
        exit(1);
    }
    mrsReader->SetFileName( inputFileName.c_str() );
    //mrsReader->Update(); 

    svkImageReader2* maskReader = NULL;
    if ( maskFileName.length() > 0 ) {
        maskReader = readerFactory->CreateImageReader2( maskFileName.c_str() );
        if ( maskReader == NULL ) {
            cerr << "Can not determine appropriate reader for mask: " << maskFileName << endl;
            exit(1);
        }
        maskReader->SetFileName( maskFileName.c_str() );
        //maskReader->Update(); 
    }

    svkMRSAverageSpectra* avSpec = svkMRSAverageSpectra::New(); 
    avSpec->SetInputConnection(0, mrsReader->GetOutputPort() ); 
    if ( maskReader != NULL ) {
        avSpec->SetInputConnection(1, maskReader->GetOutputPort() ); 
    } else if ( limitToSelectionBox == true ) {
        avSpec->LimitToSelectionBox(); 
    }

    if ( useMagnitudeSpectra == true ) {
        avSpec->AverageMagnitudeSpectra();
    }

    if ( averageOverNonSpatialDims == true ) { 
        avSpec->AverageOverNonSpatialDims();
    }
    avSpec->Update();


    // ===============================================  
    //  Write the data out to the specified file type.  
    //  Use an svkImageWriterFactory to obtain the
    //  correct writer type. 
    // ===============================================  

    //  Get input reader type: 
    if ( mrsReader->IsA("svkDcmMrsVolumeReader") ) {
        cout << "Input DCM MRS " << endl;
        dataTypeOut = svkImageWriterFactory::DICOM_MRS;
    } else if ( mrsReader->IsA("svkDdfVolumeReader") ) {
        cout << "Input DDF " << endl;
        dataTypeOut = svkImageWriterFactory::DDF;
    }

    vtkSmartPointer< svkImageWriterFactory > writerFactory = vtkSmartPointer< svkImageWriterFactory >::New(); 
    svkImageWriter* writer = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 

    if ( writer == NULL ) {
        cerr << "Can not determine writer of type: " << dataTypeOut << endl;
        exit(1);
    }

    writer->SetFileName( outputFileName.c_str() );
    writer->SetInput( svkMrsImageData::SafeDownCast( avSpec->GetOutput()) );

    // ===============================================  
    //  Set the input command line into the data set 
    //  provenance: 
    // ===============================================  
    avSpec->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    // ===============================================  
    //  Write data to file: 
    // ===============================================  
    writer->Write();

    // ===============================================  
    //  Clean up: 
    // ===============================================  
    writer->Delete();
    mrsReader->Delete();
    if ( maskReader != NULL ) {
        maskReader->Delete();
    }

    return 0; 
}

