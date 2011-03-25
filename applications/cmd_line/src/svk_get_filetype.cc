/*
 *  Copyright © 2009-2011 The Regents of the University of California.
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
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/applications/cmd_line/src/svk_file_convert.cc $
 *  $Rev: 893 $
 *  $Author: jccrane $
 *  $Date: 2011-03-16 16:30:10 -0700 (Wed, 16 Mar 2011) $
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


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_get_filetype -i input_file_name  [ -h ]              \n"; 
    usemsg += "\n";  
    usemsg += "   -i    input_file_name     Name of file to convert.    \n"; 
    usemsg += "   -h                        Print help mesage.          \n";  
    usemsg += " \n";  
    usemsg += "Returns a string file type.                              \n";
    usemsg += "\n";  

    string inputFileName; 

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );


    static struct option long_options[] =
    {
        {0, 0, 0, 0}
    };


    /*
    *   Process flags and arguments
    */
    int i;
    int option_index = 0; 
    while ((i = getopt_long(argc, argv, "i:h", long_options, &option_index)) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
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

    if ( argc != 0 || inputFileName.length() == 0 ) {
        cout << usemsg << endl;
        exit(1); 
    }

    cout << inputFileName << endl;


    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    readerFactory->QuickParse();
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());
    readerFactory->Delete(); 

    if (reader == NULL) {
        cerr << "Can not determine file type for: " << inputFileName << endl;
        exit(1);
    }

    cout << "File type: " << reader->GetDescriptiveName() << endl; 

    reader->Delete();

    return 0; 
}

