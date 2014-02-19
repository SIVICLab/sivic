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
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <unistd.h>
#include <getopt.h>
#endif
#include <string.h>
#include <svkDataModel.h>

using namespace svk;
    
void DefaultTest( );
void MemoryTest( );
void DisplayUsage( );

struct globalArgs_t {
    char *firstImageName;       /* -i  option */
    char *secondImageName;      /* -I option */
    char *firstSpectrumName;    /* -s  option */
    char *secondSpectrumName;   /* -S option */
    char *firstOverlayName;     /* -o  option */
    char *secondOverlayName;    /* -O option */
} globalArgs;

static const struct option longOpts[] = {
    { "test_name",       required_argument, NULL, 't' },
    { "image",           required_argument, NULL, 'i' },
    { "second_image",    required_argument, NULL, 'I' },
    { "spectrum",        required_argument, NULL, 's' },
    { "second_spectrum", required_argument, NULL, 'S' },
    { "overlay",         required_argument, NULL, 'o' },
    { "second_overlay",  required_argument, NULL, 'O' },
    { NULL,              no_argument,       NULL,  0  }
};
static const char *optString = "t:i:I:s:S:o:O:";


int main ( int argc, char** argv )
{
    void (*testFunction)() = NULL;
    int opt = 0;
    int longIndex;
    /* Initialize globalArgs before we get to work. */
    globalArgs.firstImageName = NULL;       /* -i  option */
    globalArgs.secondImageName = NULL;      /* -I option */
    globalArgs.firstSpectrumName = NULL;    /* -s  option */
    globalArgs.secondSpectrumName = NULL;   /* -S option */
    globalArgs.firstOverlayName = NULL;     /* -o  option */
    globalArgs.secondOverlayName = NULL;    /* -O option */

    opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
    if( opt == -1 ) {
        DisplayUsage();
    }
        while( opt != -1 ) {
            switch( opt ) {
                case 't':
                    if( strcmp( optarg, "MemoryTest" ) == 0 ) {
                        testFunction = MemoryTest;
                        cout<<" Executing Memory Check... "<<endl;
                    } else {
                        testFunction = DefaultTest;
                        cout<<" Executing Default Test... "<<endl;
                    }
                    break;
                case 'i':
                    globalArgs.firstImageName = optarg;
                    break;
                case 'I': 
                    globalArgs.secondImageName = optarg;
                    break;
                case 's':
                    globalArgs.firstSpectrumName = optarg;
                    break;
                case 'S': 
                    globalArgs.secondSpectrumName = optarg;
                    break;
                case 'o':
                    globalArgs.firstOverlayName = optarg;
                    break;
                case 'O': 
                    globalArgs.secondOverlayName = optarg;
                    break;
                default:
                    cout<< endl <<" ERROR: Unrecognized option... " << endl << endl;
                    DisplayUsage();
                break;
            }
            opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
        }
    if( testFunction == NULL ) {
        cout<< endl << "No test specified!!" << endl;
        DisplayUsage();
    }

    testFunction();    

    return 0;
  
}

// Checks common operations
void MemoryTest()
{   
    if( globalArgs.firstSpectrumName  == NULL ||
        globalArgs.secondSpectrumName == NULL ||
        globalArgs.firstOverlayName   == NULL ||
        globalArgs.secondOverlayName  == NULL || 
        globalArgs.firstImageName     == NULL ||
        globalArgs.secondImageName    == NULL ) {
        cout << endl << " ERROR: ";
        cout <<" Two images, two spectrum, and  two metabolite files must be specified to run the memory check! " << endl << endl; 
        DisplayUsage();
    }

    svkDataModel* model = svkDataModel::New( );

    svkImageData* firstImage = model->LoadFile( globalArgs.firstImageName );
    firstImage->Register(NULL);
    svkImageData* secondImage = model->LoadFile( globalArgs.secondImageName );
    secondImage->Register(NULL);
    svkImageData* firstSpectrum = model->LoadFile( globalArgs.firstSpectrumName );
    firstSpectrum->Register(NULL);
    svkImageData* secondSpectrum = model->LoadFile( globalArgs.secondSpectrumName );
    secondSpectrum->Register(NULL);
    svkImageData* firstOverlay = model->LoadFile( globalArgs.firstOverlayName );
    firstOverlay->Register(NULL);
    svkImageData* secondOverlay = model->LoadFile( globalArgs.secondOverlayName );
    secondOverlay->Register(NULL);

    firstImage->Update();
    secondImage->Update();
    firstSpectrum->Update();
    secondSpectrum->Update();
    firstOverlay->Update();
    secondOverlay->Update();
    
    model->AddDataObject( "AnatomicalData", firstImage );
    model->ChangeDataObject( "AnatomicalData", secondImage );
    model->AddDataObject( "SpectroscopicData", firstSpectrum );
    model->ChangeDataObject( "SpectroscopicData", secondSpectrum );
    model->AddDataObject( "OverlayData", firstOverlay );
    model->ChangeDataObject( "OverlayData", secondOverlay );

    model->RemoveDataObject( "OverlayData" );
    model->RemoveDataObject( "AnatomicalData" );
    model->RemoveDataObject( "SpectroscopicData");


    firstImage->Delete();
    secondImage->Delete();
    firstSpectrum->Delete();
    secondSpectrum->Delete();
    firstOverlay->Delete();
    secondOverlay->Delete();
    model->Delete();
     
}


void DefaultTest()
{
}

void DisplayUsage( void )
{
    cout << endl << "############  USAGE  ############ " << endl << endl;
    cout << "NAME" << endl;
    cout << "    test_svkDataModel" << endl << endl;
    cout << "SYNOPSIS" << endl;
    cout << "    test_svkDataModel -t testName [-i fileName] [-s fileName] [-o fileName] [-I fileName] [-S fileName] [-O fileName]" << endl << endl;
    cout << "        -t --test_name          Names the test you wish to run. See TESTS below for valid names. " << endl;
    cout << "        -i --image              A spectroscopy file. " << endl;
    cout << "        -s --spectrum           A spectroscopy file. " << endl;
    cout << "        -o --overlay            An overlay (metabolite) file. " << endl;
    cout << "        -I --second_image       A second spectroscopy file.          [Used for MemoryTest.] " << endl;
    cout << "        -S --second_spectrum    A second spectroscopy file.          [Used for MemoryTest.]" << endl;
    cout << "        -O --second_overlay     A second overlay (metabolite) file.  [Used for MemoryTest.]" << endl << endl;
    cout << "DESCRIPTION" << endl;
    cout << "    Testing harness for svkOverlayView. Test name is passed in as argument." << endl << endl;
    cout << "TESTS" << endl;
    cout << "    MemoryTest "<< endl;
    cout << "        Tests many methods of svkOverlayView to discovery memory leaks. " << endl;
    cout << "        Also will load multiple datasets to ensure no memory leaks when  " << endl;
    cout << "        switching between datasets. As such two images, two spectrum, and" << endl;
    cout << "        two overlays (metabolites) are required to run the memory check. " << endl << endl;
    cout << "    DefaultTest "<< endl;
    cout << "        Loads each type of data and creates an interactive window. " << endl;

    cout << endl << "############  USAGE  ############ " << endl << endl;
    /* ... */
    exit( EXIT_FAILURE );
}
