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
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/applications/cmd_line/src/svk_create_dcmraw.cc $
 *  $Rev: 1493 $
 *  $Author: jccrane $
 *  $Date: 2013-02-28 15:52:23 -0800 (Thu, 28 Feb 2013) $
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
#include <svkDICOMRawDataWriter.h>
#include <svkImageReaderFactory.h> 

#include <openssl/sha.h>

using namespace svk;


//  function declarations
bool VerifyExtractedFileDigests( string outputDir,  string rawFileName  ); 
string GetHash( string rawFileName, unsigned char* sha1Digest ); 


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) +                                   "\n";   
    usemsg += "svk_dcm_deid -i input_file_name -r replacementID                             \n"; 
    usemsg += "                                             -h                              \n"; 
    usemsg += "                                                                             \n";  
    usemsg += "   -i    input_file_name     Name of dcm file to deidentify                  \n"; 
    usemsg += "   -r    deid id             replacement id                                  \n"; 
    usemsg += "   --stu UID                 use the specified StudyInstanceUID              \n";
    usemsg += "   --seu UID                 use the specified SeriesInstanceUID             \n";
    usemsg += "   -h                        Print help mesage.                              \n";  
    usemsg += "                                                                             \n";  
    usemsg += "Deidentify a DICOM file.                                                     \n"; 
    usemsg += "\n";  

    string inputFileName; 
    string deidID = ""; 
    string studyUID = ""; 
    string seriesUID = ""; 

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_STUDY_UID = 0,
        FLAG_SERIES_UID
    };


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"stu",             required_argument, NULL,  FLAG_STUDY_UID},
        {"seu",             required_argument, NULL,  FLAG_SERIES_UID},
        {0, 0, 0, 0}
    };


    /*
    *   Process flags and arguments
    */
    int i;
    int option_index = 0; 
    while ((i = getopt_long(argc, argv, "i:r:h", long_options, &option_index)) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'r':
                deidID.assign( optarg );
                break;
            case FLAG_STUDY_UID:
                studyUID.assign(optarg); 
                break;
            case FLAG_SERIES_UID:
                seriesUID.assign( optarg ); 
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

    svkImageData* image = svkMriImageData::New(); 
    image->GetDcmHeader()->ReadDcmFile( inputFileName, 10000000000 );
    image->GetDcmHeader()->Deidentify(svkDcmHeader::PHI_DEIDENTIFIED, deidID); 
    if ( studyUID.size() == 0 ) {
        image->GetDcmHeader()->InsertUniqueUID("StudyInstanceUID");
    } else {
        image->GetDcmHeader()->SetValue("StudyInstanceUID", studyUID );
    }
    if ( seriesUID.size() == 0 ) {
        image->GetDcmHeader()->InsertUniqueUID("SeriesInstanceUID");
    } else {
        image->GetDcmHeader()->SetValue("SeriesInstanceUID", seriesUID );
    }
    image->GetDcmHeader()->InsertUniqueUID("SOPInstanceUID");
    image->GetDcmHeader()->WriteDcmFile(inputFileName); 
    image->Delete(); 
        

    return 0; 
}

