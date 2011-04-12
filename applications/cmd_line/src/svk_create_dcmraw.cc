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

#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#include <unistd.h>
#endif
#include <svkDICOMRawDataWriter.h>

#include <openssl/sha.h>

using namespace svk;


//  function declarations
string GetHash( string inputFileName, unsigned char* sha1Digest ); 


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_get_filetype -i input_file_name  --md5 md5_sum [ -h ]              \n"; 
    usemsg += "\n";  
    usemsg += "   -i    input_file_name     Name of file to convert.    \n"; 
    usemsg += "   -h                        Print help mesage.          \n";  
    usemsg += " \n";  
    usemsg += "Encapsulates the specified file GEPFile in a DICOM Raw Data SOP instance\n"; 
    usemsg += "\n";  

    string inputFileName; 

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_MD5_CKSUM = 0
    }; 
    

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

    unsigned char* sha1Digest = new unsigned char[SHA_DIGEST_LENGTH];
    string digest = GetHash( inputFileName, sha1Digest ); 
    cout << "DIGEST STRING: " << digest << endl;

    svkDICOMRawDataWriter* rawWriter = svkDICOMRawDataWriter::New();
    rawWriter->SetFileName( inputFileName.c_str() );
    rawWriter->SetSHA1Digest( digest ); 
    rawWriter->Write();

    rawWriter->Delete(); 

    return 0; 
}



/*
 *  Gets the sha1Digest of the specified file.  
 */
string GetHash( string inputFileName, unsigned char* sha1Digest ) {

    string sha1DigestString; 

    ifstream* inputStream = new ifstream();
    inputStream->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

    try {

        inputStream->open( inputFileName.c_str(), ios::binary);

        inputStream->seekg(0, ios::end);

        // get-ptr position is now same as file size
        ios::pos_type fileSize = inputStream->tellg();  
        cout << "FILE SIZE = " << static_cast<int>(fileSize) << endl;

        void* fileBuffer = new char[static_cast<int>(fileSize)];
        inputStream->seekg(0, ios::beg);
        inputStream->read( static_cast<char*>(fileBuffer), fileSize);

        SHA1( static_cast<unsigned char*>(fileBuffer),  fileSize,  sha1Digest);

        printf("Digest(sha1): ");
        char buf[3]; 
        for(int i = 0; i < SHA_DIGEST_LENGTH; i++) {
            sprintf( buf, "%02x", sha1Digest[i] );
            sha1DigestString.append(buf); 
        }
        printf(" \n");

   } catch (ifstream::failure e) {
        cout << "ERROR: Exception opening/reading file " << inputFileName << " => " << e.what() << endl;
        exit(1);
    }

    inputStream->close();
    return sha1DigestString; 
} 
