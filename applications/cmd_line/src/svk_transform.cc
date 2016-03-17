/*
 *  Copyright © 2009-2015 The Regents of the University of California.
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
 */


#include <vtkSmartPointer.h>
#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkTransform.h>
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
    usemsg += "Version " + string(SVK_RELEASE_VERSION) +                                       "\n";
    usemsg += "svk_transform -i input_file_name -o output_file_root -t output_data_type         \n"; 
    usemsg += "              --dl --dp --ds  [-h]                                               \n";
    usemsg += "                                                                                 \n";
    usemsg += "   -i        input_file_name     Name of file to resample.                       \n";
    usemsg += "   -o        output_file_root    Root name of outputfile.                        \n";
    usemsg += "   -t        output_data_type    Output data type:                               \n";
    usemsg += "                                     2 = UCSF DDF                                \n";
    usemsg += "                                     3 = UCSF IDF                                \n";
    usemsg += "                                     4 = DICOM_MRS                               \n";
    usemsg += "                                     5 = DICOM_MRI                               \n";
    usemsg += "                                     6 = DICOM_Enhanced MRI                      \n";
    usemsg += "   --dl      delta_L             Change in position along L direction            \n";
    usemsg += "   --dp      delta_P             Change in position along P direction            \n";
    usemsg += "   --ds      delta_S             Change in position along S direction            \n";
    usemsg += "   -h                            print help mesage.                              \n";
    usemsg += "                                                                                 \n";
    usemsg += "Applies 3 DOF translation to input image.  Input shifts are in LPS frame, not    \n";   
    usemsg += "image (col, row, slice) frame.                                                   \n";   
    usemsg += "\n";

    string inputFileName;
    string outputFileName;

    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::UNDEFINED;
    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );
    float dl = 0. ; 
    float dp = 0. ; 
    float ds = 0. ; 


    enum FLAG_NAME {
        FLAG_DL = 0, 
        FLAG_DP, 
        FLAG_DS 
    }; 
    
    
    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"dl",        required_argument, NULL,  FLAG_DL},
        {"dp",        required_argument, NULL,  FLAG_DP},
        {"ds",        required_argument, NULL,  FLAG_DS},
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
            case FLAG_DL:
                dl = ( atof(optarg) );
                break;
            case FLAG_DP:
                dp = ( atof(optarg) );
                break;
            case FLAG_DS:
                ds = ( atof(optarg) );
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
    //      an input, target and output name was supplied
    //      an output data type was supplied
    // ===============================================
    if (
        argc != 0 ||  inputFileName.length() == 0
        || outputFileName.length() == 0
        || ( dataTypeOut != svkImageWriterFactory::DICOM_MRI 
             && dataTypeOut != svkImageWriterFactory::IDF 
             && dataTypeOut != svkImageWriterFactory::DDF 
             && dataTypeOut != svkImageWriterFactory::DICOM_MRS
             && dataTypeOut != svkImageWriterFactory::DICOM_ENHANCED_MRI )
    ) {
        cout << usemsg << endl;
        exit(1); 
    }

    // ===============================================
    //  Use a reader factory to get the correct reader
    //  type .
    // ===============================================
    vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New();
    svkImageReader2* inputReader = readerFactory->CreateImageReader2(inputFileName.c_str());

    if ( inputReader == NULL ) { 
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }
    inputReader->SetFileName( inputFileName.c_str() );
    inputReader->Update();

    svkTransform* transform = svkTransform::New();
    transform->SetInputData( inputReader->GetOutput() ); 
    transform->SetTranslationLPS( dl, dp, ds ); 
    transform->Update();

    vtkSmartPointer< svkImageWriterFactory > writerFactory = vtkSmartPointer< svkImageWriterFactory >::New();
    svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter( dataTypeOut ) ); 
    writer->SetFileName( outputFileName.c_str() );

    writer->SetInputData( transform->GetOutput() );
    writer->Write();
    writer->Delete();
    inputReader->Delete();
    transform->Delete();

    return 0; 
}

