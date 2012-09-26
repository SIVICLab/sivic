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
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/applications/cmd_line/src/svk_gepfile_reader.cc $
 *  $Rev: 1355 $
 *  $Author: jccrane $
 *  $Date: 2012-09-17 16:16:46 -0700 (Mon, 17 Sep 2012) $
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
    usemsg += "svk_gepfile_modify -i input_file_name -f field_name -v value\n"; 
    usemsg += "\n";  
    usemsg += "   -i    name        Name of file to modify. \n"; 
    usemsg += "   -f    field_name  e.g. rhs.series_desc \n";
    usemsg += "   -v    value       value \n";
    usemsg += "   -h                        Print this help mesage. \n";  
    usemsg += "\n";  
    usemsg += "Modify a GE PFile.\n";  
    usemsg += "\n";  


    string inputFileName; 
    string field = ""; 
    string value = ""; 

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
    while ( ( i = getopt_long(argc, argv, "i:f:v:h", long_options, &option_index) ) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'f':
                field.assign( optarg );
                break;
            case 'v':
                value.assign( optarg );
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
    if ( inputFileName.length() == 0 ) {
        cout << "An input file must be specified. " << endl; 
        exit(1); 
    } 

    // ===============================================  
    //  If only anonymizing the raw file, just do that 
    //  and exit
    // ===============================================  
    if ( field.compare("") != 0 ) {
        vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 
        readerFactory->QuickParse();
        svkGEPFileReader* reader = svkGEPFileReader::SafeDownCast( readerFactory->CreateImageReader2(inputFileName.c_str()) );
        if (reader == NULL) {
            cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
            exit(1);
        }
        reader->SetFileName( inputFileName.c_str() );
        reader->ModifyRawField(field, value);
        reader->Delete();
        exit(0);
    }


    // ===============================================  
    //  Clean up: 
    // ===============================================  

    return 0; 
}

