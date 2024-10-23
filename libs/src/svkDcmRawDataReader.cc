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


#include </usr/include/vtk/vtkZLibDataCompressor.h>

#include <svkDcmRawDataReader.h>
#include <svkDcmHeader.h>
#include <svkMriImageData.h>

#include </usr/include/vtk/vtkErrorCode.h>
#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkDebugLeaks.h>

#include <sstream>


using namespace svk;


//vtkCxxRevisionMacro(svkDcmRawDataReader, "$Rev$");
vtkStandardNewMacro(svkDcmRawDataReader);


/*!
 *
 */
svkDcmRawDataReader::svkDcmRawDataReader()
{
#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkDcmRawDataReader");
#endif
    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");
        
    this->SetErrorCode(vtkErrorCode::NoError);
    this->outDir = "";

}


/*!
 *
 */
svkDcmRawDataReader::~svkDcmRawDataReader()
{
    vtkDebugMacro(<<this->GetClassName() << "::~" << this->GetClassName() << "()");
}


/*!
 *  Mandator, must be overridden to get the factory to check for proper
 *  type of vtkImageReader2 to return. 
 */
int svkDcmRawDataReader::CanReadFile(const char* fname)
{

    std::string fileToCheck(fname);

    bool isDcmRaw = false; 

    if ( svkDcmHeader::IsFileDICOM( fname ) ) {

        svkImageData* tmp = svkMriImageData::New(); 

        tmp->GetDcmHeader()->ReadDcmFile( fname );
        std::string SOPClassUID = tmp->GetDcmHeader()->GetStringValue( "SOPClassUID" ) ;

        //verify that this isn't a proprietary use of DICOM MR ImageStorage: 
        if( tmp->GetDcmHeader()->ElementExists( "SVK_PRIVATE_TAG", "top") ) {
                    
            if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.66" ) {           
                this->SetFileName(fname);
                isDcmRaw = true; 
            }
        
        }

        tmp->Delete(); 
    }

    if ( isDcmRaw ) {
        cout << this->GetClassName() << "::CanReadFile(): It's a DICOM Raw Data File: " <<  fileToCheck << endl;
        return 1;
    }

    vtkDebugMacro(<<this->GetClassName() << "::CanReadFile() It's Not a DICOM Raw Data file " << fileToCheck );

    return 0;
}


/*!
 *  Initializes any private DICOM attributes that are needed internally
 */
void svkDcmRawDataReader::InitPrivateHeader()
{
}


/*!
 *
 */
void svkDcmRawDataReader::LoadData( svkImageData* data )
{
}

/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called before ExecuteData()
 */
void svkDcmRawDataReader::ExecuteInformation()
{
    this->InitDcmHeader();
}



/*! 
 *  Init the Shared and PerFrame sequences and load the pixel data into the svkImageData object. 
 */
void svkDcmRawDataReader::ExtractFiles()
{

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    int numberOfEncapsulatedFiles = hdr->GetNumberOfItemsInSequence( "SVK_FILE_SET_SEQUENCE" );

    for ( int fileNum = 0; fileNum < numberOfEncapsulatedFiles; fileNum++ ) {

        std::string fileName =  
            hdr->GetStringSequenceItemElement(
                "SVK_FILE_SET_SEQUENCE",
                fileNum,
                "SVK_FILE_NAME"
            );

        std::string sha1Digest =  
            hdr->GetStringSequenceItemElement(
                "SVK_FILE_SET_SEQUENCE",
                fileNum,
                "SVK_FILE_SHA1_DIGEST"
            );

        //  For backward compatibility with old element
        long unsigned int pfileSize;   
        if ( hdr->ElementExists( "SVK_FILE_NUM_BYTES", "SVK_FILE_SET_SEQUENCE") ) {
            pfileSize =  
                hdr->GetIntSequenceItemElement(
                    "SVK_FILE_SET_SEQUENCE",
                    fileNum,
                    "SVK_FILE_NUM_BYTES"
                );
        } else {
            pfileSize =  
                hdr->GetLongIntSequenceItemElement(
                    "SVK_FILE_SET_SEQUENCE",
                    fileNum,
                    "SVK_FILE_NUM_BYTES_LONG"
                );
        }


        //  For backward compatibility, check to see if 
        //  SVK_FILE_CONTENT_SEQUENCE element exists.
        //  If not use legacy read. Otherwise:
        //  loop over each item in the SVK_FILE_CONTENT_SEQUENCE 
        //  First, determine number of items in this sequence.  
        if ( !hdr->ElementExists( "SVK_FILE_CONTENT_SEQUENCE") ) {

            this->LegacyParsing( fileName, fileNum, pfileSize);

        } else {

            //  If an output directory was provided, prepend the filename:
            std::string outputFileName = fileName; 
            if ( this->outDir.size() > 0 ) {
                outputFileName = outDir + "/" + fileName; 
            }
            outputFileName.append(".extracted"); 
            ofstream outputFile ( outputFileName.c_str(), ios::binary );
            if( !outputFile) { 
                cout << "Cannot open file to extract raw data to: " << outputFileName << endl;
                this->SetErrorCode( vtkErrorCode::CannotOpenFileError );
            }
            

            //  if the size isn't divisible by sizeof(float), 
            //  then pad to include partial word
            long int readStrideBytes = 1000000000;

            long unsigned int numValues =  readStrideBytes / sizeof(float); 
            if (  pfileSize % sizeof(float) != 0 ) {
                numValues += 1;
            }
            float* fileBuffer = new float[ numValues ]; 

            int pfileSection = 0; 
            int byte = 0; 

            int numberOfItems = hdr->GetNumberOfItemsInSequence("SVK_FILE_CONTENT_SEQUENCE", "SVK_FILE_SET_SEQUENCE", fileNum); 

            //cout << "NUMBER OF ITEMS COMPRISING PFILE: " << numberOfItems << endl;

            for ( int item = 0; item < numberOfItems; item++ ) {

                //  To reduce footprint, read header without long fields, and 
                //  loadElement on demand.  
                svkImageData* tmpDcm = svkMriImageData::New();
                tmpDcm->GetDcmHeader()->ReadDcmFile( this->FileName);

                //  First determine how many bytes are in this section (item). 
                long int numBytesInSection; 
                long int numWordsInSection; 

                numBytesInSection = pfileSize - pfileSection * readStrideBytes; 
                numWordsInSection = numBytesInSection / sizeof(float); 
                if ( numBytesInSection > readStrideBytes ) {
                    numBytesInSection = readStrideBytes; 
                    numWordsInSection = numBytesInSection / sizeof(float); 
                } else {
                    //  Handle remainder bytes to ensure they are of size divisible 
                    //  by sizeof(float). 

                    //  if the size isn't divisible by sizeof(float), 
                    //  then pad to include partial word
                    if ( numBytesInSection % sizeof(float) != 0 ) { 
                        long int numBytesInSectionTmp = numBytesInSection + sizeof(float);
                        numWordsInSection = numWordsInSection + 1; 
                    } 
                }
                //cout << "READ ITEM: " << item << endl;
                //cout << "READ ITEM: numWordsInSection: " << numWordsInSection << endl;
                //cout << "READ ITEM: numBytesInSection: " << numBytesInSection << endl;

                //  If the element was zlib deflated, then detect that and load appropriately 
                bool dataDeflated = false; 
                if( tmpDcm->GetDcmHeader()->ElementExists( "SVK_ZLIB_INFLATED_SIZE") ) {
                    // Get length to read (byes); 
                    long int lengthBytesDeflated = tmpDcm->GetDcmHeader()->GetSequenceItemElementLength(
                        "SVK_FILE_CONTENT_SEQUENCE",
                        item,
                        "SVK_FILE_CONTENTS",
                        "SVK_FILE_SET_SEQUENCE",
                        fileNum
                    ); 
                    long int numDeflatedWords = lengthBytesDeflated / sizeof(float); 
                    numWordsInSection = numDeflatedWords; 
                    dataDeflated = true; 
                }
               
                tmpDcm->GetDcmHeader()->GetFloatSequenceItemElement(
                    "SVK_FILE_CONTENT_SEQUENCE",
                    item,
                    "SVK_FILE_CONTENTS",
                    fileBuffer, 
                    numWordsInSection, 
                    "SVK_FILE_SET_SEQUENCE",
                    fileNum
                );

                if ( dataDeflated ) {
                    vtkZLibDataCompressor* zlib = vtkZLibDataCompressor::New(); 
                    size_t originalInflatedSize = tmpDcm->GetDcmHeader()
                        ->GetLongIntSequenceItemElement(
                        "SVK_FILE_CONTENT_SEQUENCE",
                        item,
                        "SVK_ZLIB_INFLATED_SIZE",
                        "SVK_FILE_SET_SEQUENCE",
                        fileNum
                    );
                    unsigned char* deflatedBuffer = new unsigned char[originalInflatedSize]; 

                    size_t inflatedSize = zlib->Uncompress( 
                            (unsigned char*)(fileBuffer), numWordsInSection * sizeof(float), 
                            deflatedBuffer, originalInflatedSize ); 
                    if ( inflatedSize == originalInflatedSize ) {
                        outputFile.write( (char *)deflatedBuffer, originalInflatedSize); 
                    } else {
                        cout << "ERROR: could not deflate SVK_FILE_CONTENTS" << endl;
                        exit(1); 
                    }

                    delete [] deflatedBuffer; 
                    zlib->Delete(); 

                } else {

                    outputFile.write( (char *)fileBuffer, numBytesInSection); 
                }

                //cout << "done Write ITEM " << endl;

                if( outputFile.bad() ) { 
                    cout << "Error extracting DICOM Raw Data to disk: " << outputFileName << endl;
                    this->SetErrorCode( vtkErrorCode::CannotOpenFileError );
                }

                byte += readStrideBytes; 
                pfileSection ++; 

                tmpDcm->Delete();

            }

            outputFile.close(); 

            if (fileBuffer != NULL) {
                delete[] fileBuffer; 
            }


        }
    }

}


void svkDcmRawDataReader::LegacyParsing(string fileName, int fileNum, long int numBytes)
{

    //  if the size isn't divisible by sizeof(float), 
    //  then pad to include partial word
    long unsigned int numValues =  numBytes / sizeof(float); 
    if ( numBytes % sizeof(float) != 0 ) {
        numValues += 1;
    }
    float* fileBuffer = new float[ numValues ]; 

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    hdr->GetFloatSequenceItemElement(
        "SVK_FILE_SET_SEQUENCE",
        fileNum,
        "SVK_FILE_CONTENTS",
        fileBuffer, 
        numValues
    );

    //  If an output directory was provided, prepend the filename:
    std::string outputFileName = fileName; 
    if ( this->outDir.size() > 0 ) {
        outputFileName = outDir + "/" + fileName; 
    }
    outputFileName.append(".extracted"); 

    ofstream outputFile ( outputFileName.c_str(), ios::binary );
    if( !outputFile) { 
        cout << "Cannot open file to extract raw data to: " << outputFileName << endl;
        this->SetErrorCode( vtkErrorCode::CannotOpenFileError );
    }
        
    outputFile.write( (char *)fileBuffer, numBytes); 

    if( outputFile.bad() ) { 
        cout << "Error extracting DICOM Raw Data to disk: " << outputFileName << endl;
        this->SetErrorCode( vtkErrorCode::CannotOpenFileError );
    }

    outputFile.close(); 

    if (fileBuffer != NULL) {
        delete[] fileBuffer; 
    }

}


/*!
 *  Set the path to extract files into
 */
void svkDcmRawDataReader::SetOutputDir( std::string outDir )
{
    this->outDir = outDir; 
}

/*!
 *  Returns the pixel type
 */
svkDcmHeader::DcmPixelDataFormat svkDcmRawDataReader::GetFileType()
{
    return svkDcmHeader::UNDEFINED;
}


/*!
 *
 */
int svkDcmRawDataReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData");
    return 1;
}

