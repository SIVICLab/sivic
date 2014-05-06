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
    usemsg += "svk_create_dcmraw -i input_file_name  -o output_dir                          \n"; 
    usemsg += "                [ -u | -s UID && -i UID ]                                    \n"; 
    usemsg += "                [ -a additional_file_name ] [ -xh ]                          \n"; 
    usemsg += "                                                                             \n";  
    usemsg += "   -i    input_file_name     Name of raw file to convert.                    \n"; 
    usemsg += "   -a    associated file     Name of associated files to include in DICOM    \n"; 
    usemsg += "                             output.  May specify -a multiple times.         \n"; 
    usemsg += "   -o    output_dir          Extract Files into the specified dir.           \n"; 
    usemsg += "   -x                        Extract Files from DICOM Raw Data SOP Instance. \n"; 
    usemsg += "   -u                        Generate a unique SeriesInstanceUID (default is \n"; 
    usemsg += "                             to use value from pfile).                       \n"; 
    usemsg += "   -s    UID                 Use the specified SeriesInstanceUID             \n"; 
    usemsg += "   -i    UID                 Use the specified SOPInstanceUID                \n"; 
    usemsg += "   -z                        Skip raw pass file size check                   \n";
    usemsg += "   -h                        Print help mesage.                              \n";  
    usemsg += "                                                                             \n";  
    usemsg += "Encapsulates the specified file GEPFile in a DICOM Raw Data SOP instance     \n"; 
    usemsg += "\n";  

    string inputRawFileName; 
    string outputDir = ""; 
    vector < string >  associatedFiles; 
    bool   extractRaw = false; 
    bool   useNewUID  = false; 
    bool   skipFileSizeCheck = false;
    string seriesInstanceUID; 
    string sopInstanceUID; 

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_SHA1_CKSUM = 0
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
    while ((i = getopt_long(argc, argv, "i:a:xuS:I:o:zh", long_options, &option_index)) != EOF) {
        switch (i) {
            case 'i':
                inputRawFileName.assign( optarg );
                break;
            case 'a':
                associatedFiles.push_back( optarg );
                break;
            case 'x':
                extractRaw = true; 
                break;
            case 'o':
                outputDir.assign( optarg );
                break;
            case 'u':
                useNewUID = true;
                break;
            case 'S':
                seriesInstanceUID.assign( optarg );
                break;
            case 'I':
                sopInstanceUID.assign( optarg );
                break;
            case 'z':
                skipFileSizeCheck = true;
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

    if ( argc != 0 || inputRawFileName.length() == 0 ) {
        cout << usemsg << endl;
        exit(1); 
    }

    if ( useNewUID && (seriesInstanceUID.length() > 0 || sopInstanceUID.length() > 0 )) {
        cout << "ERROR: Specify -u or -s UID -i UID." << endl;
        cout << usemsg << endl;
        exit(1); 
    }

    cout << inputRawFileName << endl;
    for (int i = 0; i <  associatedFiles.size(); i++) { 
            cout << "associated file: " << associatedFiles[i] << endl;
    }


    if ( extractRaw ) {

        svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
        svkImageReader2* reader = readerFactory->CreateImageReader2(inputRawFileName.c_str());
        readerFactory->Delete();

        if (reader == NULL || ! reader->IsA("svkDcmRawDataReader") ) {
            cerr << "File is not a DICOM Raw Data Storage file: " << inputRawFileName << endl;
            exit(1);
        }

        reader->SetFileName( inputRawFileName.c_str() );
        if ( outputDir.size() > 0 ) {
            svkDcmRawDataReader::SafeDownCast( reader )->SetOutputDir( outputDir );
        }
        reader->Update(); 
        svkDcmRawDataReader::SafeDownCast( reader )->ExtractFiles();

        if ( reader->GetErrorCode() != vtkErrorCode::NoError ) {    
            cout << endl;
            cout << "######################################" << endl;
            cout << "ERROR, could not Extract files from "   <<endl; 
            cout << "DICOM Raw Data. "                       << endl;
            cout << "######################################" << endl;
            cout << endl;
            exit(1); 
        }

        reader->Delete();

        //  Now, get the SVK_FILE_SET_SEQUENCE from the DICOM RawStorage object and 
        //  verify that the SVK_FILE_SHA1_DIGEST matches with a hash generated from 
        //  the extracted file. 
        if ( ! VerifyExtractedFileDigests( outputDir, inputRawFileName  ) )  {
            cout << endl;
            cout << "######################################" << endl;
            cout << "ERROR, extracted file digest doesn't "  <<endl; 
            cout << "match input raw data. "                 << endl;
            cout << "######################################" << endl;
            cout << endl;
            exit(1); 
        }
        

    } else {    

        unsigned char* sha1Digest = new unsigned char[SHA_DIGEST_LENGTH];
        string digest = GetHash( inputRawFileName, sha1Digest );
        cout << "DIGEST STRING(" << inputRawFileName << "): " << digest << endl;
    
        svkDICOMRawDataWriter* rawWriter = svkDICOMRawDataWriter::New();
        if( skipFileSizeCheck ) {
            rawWriter->SetSkipFileSizeCheck( true );
        }
        rawWriter->SetFileNameWithExtension( inputRawFileName.c_str() );
        rawWriter->SetSHA1Digest( digest ); 

        for (int i = 0; i < associatedFiles.size(); i++ ) {
            string digest = GetHash( associatedFiles[i], sha1Digest );
            cout << "DIGEST STRING(" << inputRawFileName << "): " << digest << endl;
            rawWriter->AddAssociatedFile( associatedFiles[i], digest );     
        }

        if ( useNewUID ) {
            rawWriter->ReuseSeriesUID( false ); 
            rawWriter->ReuseInstanceUID( false ); 
        } else if ( seriesInstanceUID.length() > 0  && sopInstanceUID.length() > 0  ) {
            rawWriter->SetSeriesUID( seriesInstanceUID ); 
            rawWriter->SetInstanceUID( sopInstanceUID ); 
        }

        rawWriter->Write();

        if ( rawWriter->GetErrorCode() != vtkErrorCode::NoError ) {    
            cout << endl;
            cout << "######################################" << endl;
            cout << "ERROR, could not write DICOM Raw Data. " << endl;
            cout << "######################################" << endl;
            cout << endl;
            exit(1); 
        }
    
        rawWriter->Delete(); 

    }

    return 0; 
}


/*!
 *  Verify the digest of the extracted raw files:
 */
bool VerifyExtractedFileDigests( string outputDir, string inputRawFileName  )  
{

    bool verified = true; 

    svkImageData* tmp = svkMriImageData::New(); 
    tmp->GetDcmHeader()->ReadDcmFile( inputRawFileName );

    svkDcmHeader* hdr = tmp->GetDcmHeader();
    int numberOfEncapsulatedFiles = hdr->GetNumberOfItemsInSequence( "SVK_FILE_SET_SEQUENCE" );

    for ( int fileNum = 0; fileNum < numberOfEncapsulatedFiles; fileNum++ ) {

        vtkstd::string fileName =  
            hdr->GetStringSequenceItemElement(
                "SVK_FILE_SET_SEQUENCE",
                fileNum,
                "SVK_FILE_NAME"
            );

        vtkstd::string sha1Digest =  
            hdr->GetStringSequenceItemElement(
                "SVK_FILE_SET_SEQUENCE",
                fileNum,
                "SVK_FILE_SHA1_DIGEST"
            );


        unsigned char* sha1DigestExtracted = new unsigned char[SHA_DIGEST_LENGTH];
        fileName.append(".extracted"); 
        if ( outputDir.size() > 0 ) {
            fileName = outputDir + "/" + fileName;  
        }
        string digestExtracted = GetHash( fileName, sha1DigestExtracted );
        if ( digestExtracted.compare(sha1Digest) != 0 ) {
            cout << "ERROR DIGEST( " << fileName << "): " <<  digestExtracted << " != " << sha1Digest << endl;
            cout << endl;
            verified = false; 
        }

    }

    tmp->Delete();

    return verified; 
}


/*
 *  Gets the sha1Digest of the specified file.
 */
string GetHash( string rawFileName, unsigned char* sha1Digest )
{
    string sha1DigestString;
    ifstream* inputStream = new ifstream();
    inputStream->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

    try {

        inputStream->open( rawFileName.c_str(), ios::binary);

        // Seek to the end to get the file size
        inputStream->seekg(0, ios::end);

        // get-ptr position is now same as file size
        long int fileSize = inputStream->tellg();

        long int bufferSize = 1000000000;

        // This buffer will hold the data as its loaded in chunks
        void* buffer = new char[ bufferSize ];

        SHA_CTX ctx;

        // Initialize the SHA1 digest
        SHA1_Init(&ctx);

        for ( long int position = 0; position < fileSize; position += bufferSize ) {

            //  First determine how many bytes are in this section (item).
            long int numBytesInSection = bufferSize;

            if( fileSize - position < bufferSize ) {
                numBytesInSection = fileSize - position;
            }

            // Seek to the position
            inputStream->seekg(position, ios::beg);

            // read the bytes
            inputStream->read( static_cast<char*>(buffer), numBytesInSection);

            // update the digest
            SHA1_Update(&ctx, buffer, numBytesInSection);
        }

        // Finalize the digest
        SHA1_Final(sha1Digest, &ctx);

        char buf[3];
        for(int i = 0; i < SHA_DIGEST_LENGTH; i++) {
            sprintf( buf, "%02x", sha1Digest[i] );
            sha1DigestString.append(buf);
        }
        printf(" \n");
        delete [] static_cast<char*>(buffer);

   } catch (ifstream::failure e) {
        cout << "ERROR: Exception opening/reading file " << rawFileName << " => " << e.what() << endl;
        exit(1);
    }
    inputStream->close();
    delete inputStream;
    return sha1DigestString;
}
