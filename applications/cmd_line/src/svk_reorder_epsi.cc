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
#include <svkEPSIReorder.h>

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
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_reorder_epsi -i input_file_name -o output_file_name [ -t output_data_type ]      \n"; 
    usemsg += "                  --lobes                 num                                        \n";
    usemsg += "                  --skip                  num                                        \n";
    usemsg += "                [ --first                 num ]                                    \n";
    usemsg += "                  --axis                  axis                                       \n";
    usemsg += "                  --type                  type                                       \n";
    usemsg += "                                                                                     \n";  
    usemsg += "   -i        name    Name of file to convert.                                \n"; 
    usemsg += "   -o        name    Name of outputfile.                                     \n";
    usemsg += "   -t        type    Target data type:                                       \n";
    usemsg += "                         2 = UCSF DDF                                        \n";
    usemsg += "                         4 = DICOM_MRS (default)                             \n";
    usemsg += "   --lobes   num     Num lobes in EPSI waveform                              \n";
    usemsg += "                     Not all samples will be represented in output data      \n"; 
    usemsg += "                     (see skip and first                                     \n";
    usemsg += "   --skip    num     Num samples to skip between in each cycle of waveform   \n";
    usemsg += "   --first   num     First input sample to write out, represents an initial  \n"; 
    usemsg += "                     offset of skipped samples (samples start at 1). By      \n"; 
    usemsg += "                     default first is set to 1, so no initial offset.        \n"; 
    usemsg += "   --axis    axis    EPSI axis 1, 2, 3                                       \n"; 
    usemsg += "   --type    type    Specify 1 (flyback), 2(symmetric), 3(interleaved).      \n";
    usemsg += "   -h                Print this help mesage.                                 \n";  
    usemsg += "\n";  
    usemsg += "Reorderes an EPSI data set into a regular array of k,t ordered data. separating out the\n"; 
    usemsg += "spec and k-space samples from the EPSI waveform. Recomputes the FOV and volume TLC     \n"; 
    usemsg += "based on the reordered spatial dimensions.  Note, this does NOT apply phase correction \n"; 
    usemsg += "offset the time delay between k-space samples in gradient.                             \n"; 
    usemsg += "\n";  


    string inputFileName; 
    string outputFileName;
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_MRS;

    int numLobes            = UNDEFINED; 
    int skip                = UNDEFINED; 
    int first               = 0; 
    int axis                = UNDEFINED; 
    svkEPSIReorder::EPSIType type  = svkEPSIReorder::UNDEFINED_EPSI_TYPE; 

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 

    enum FLAG_NAME {
        FLAG_NUM_LOBES, 
        FLAG_SKIP, 
        FLAG_FIRST,  
        FLAG_AXIS, 
        FLAG_TYPE 
    }; 


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"lobes",                   required_argument, NULL,  FLAG_NUM_LOBES},
        {"skip",                    required_argument, NULL,  FLAG_SKIP},
        {"first",                   required_argument, NULL,  FLAG_FIRST},
        {"axis",                    required_argument, NULL,  FLAG_AXIS},
        {"type",                    required_argument, NULL,  FLAG_TYPE},
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
            case FLAG_NUM_LOBES:
                numLobes = atoi(optarg); 
                break;
            case FLAG_SKIP:
                skip = atoi(optarg); 
                break;
            case FLAG_FIRST:
                first = atoi(optarg) - 1; 
                break;
            case FLAG_AXIS:
                //  axis ordering starts at 0
                axis = atoi( optarg ) - 1; 
                break;
            case FLAG_TYPE:
                type = static_cast<svkEPSIReorder::EPSIType>(atoi(optarg)); 
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
    if ( 
        numLobes == UNDEFINED ||
        skip == UNDEFINED ||
        first < 0 ||
        axis < 0 || axis > 2 ||
        type == svkEPSIReorder::UNDEFINED_EPSI_TYPE ||
        outputFileName.length() == 0 ||
        inputFileName.length() == 0  ||
        ( dataTypeOut != svkImageWriterFactory::DICOM_MRS && dataTypeOut != svkImageWriterFactory::DDF ) ||
        argc != 0 
    ) {
            cout << usemsg << endl;
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

    reader->SetFileName( inputFileName.c_str() );
    reader->Update(); 

    //  Reorder/sample EPSI data: 
    svkEPSIReorder* reorder = svkEPSIReorder::New();
    reorder->SetInput( reader->GetOutput() ); 
    reorder->SetEPSIType( type );
    reorder->SetNumSamplesToSkip( skip );
    reorder->SetNumEPSILobes( numLobes );
    reorder->SetFirstSample( first );
    reorder->SetEPSIAxis( static_cast<svkEPSIReorder::EPSIAxis>( axis ) );
    //int numVoxels[3]; 
    //reader->GetOutput()->GetNumberOfVoxels( numVoxels);
    //reorder->SetNumVoxelsOriginal( numVoxels ); 
    reorder->Update();

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
    writer->SetInput( reorder->GetOutput() );

    // ===============================================  
    //  Set the input command line into the data set 
    //  provenance: 
    // ===============================================  
    reorder->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    // ===============================================  
    //  Write data to file: 
    // ===============================================  
    writer->Write();

    // ===============================================  
    //  Clean up: 
    // ===============================================  
    writer->Delete();
    reader->Delete();
    reorder->Delete(); 

    return 0; 
}

