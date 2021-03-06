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
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#include <svkImageReader2.h>
#include <svkImageReaderFactory.h>
#include <svkDataModel.h>
#include <svkImageData.h>
#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#include <unistd.h>
#endif

using namespace svk;
static const char *optString = "v";
void DisplayUsage();

int main ( int argc, char** argv )
{
    svkDataModel* model = svkDataModel::New();

    bool printVtkObject = false;
    int opt = 0;
    opt = getopt( argc, argv, optString);
    while( opt != -1 ) {
        switch (opt) {
            case 'v':
                 printVtkObject = true;
                break;
            default:
                cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
                cout<< endl <<" ERROR: Unrecognized option... " << endl << endl;
                cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
                DisplayUsage();
                break;
        }
        opt = getopt( argc, argv, optString );
    }
    char* filename = NULL;
    if( printVtkObject && argc == 3) {
        filename = argv[2];
    } else if ( argc == 2 ) {
        filename = argv[1];
    } else {
        DisplayUsage();
    }
    bool readOnlyOneFile = true; 
    svkImageData* data = model->LoadFile(filename, readOnlyOneFile);
    if( data == NULL ) {
        cout << "Cannot read file: " << filename << endl;
        exit(1);
    }
    if( printVtkObject ) {
        cout << "File: " << filename << endl <<  "VTK Object: " << endl << *data << endl;
        cout << "SVK DICOM Header: " << endl;
    }
    data->GetDcmHeader()->PrintDcmHeader();
    return 0;
  
}


void DisplayUsage( )
{
    cout << endl << "############  USAGE  ############ " << endl << endl;
    cout << "NAME" << endl;
    cout << "    svk_dcmdump" << endl << endl;
    cout << "SYNOPSIS" << endl;
    cout << "    svk_dcmdump fileName" << endl << endl;
    cout << "DESCRIPTION" << endl;
    cout << "    svk_dcmdump is used to give a quick way to view svk's interpreted DICOM header of a given image file." << endl << endl;
    cout << "    -v will also print vtk obect information." << endl << endl;
    cout << "VERSION" << endl;
    cout << "     " << SVK_RELEASE_VERSION << endl; 
    cout << endl << "############  USAGE  ############ " << endl << endl;
    /* ... */
    exit( EXIT_FAILURE );
}
