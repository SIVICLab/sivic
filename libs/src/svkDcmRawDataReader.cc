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
 */



#include <svkDcmRawDataReader.h>
#include <svkDcmHeader.h>
#include <svkMriImageData.h>

#include <vtkErrorCode.h>
#include <vtkInformation.h>
#include <vtkDebugLeaks.h>

#include <sstream>


using namespace svk;


vtkCxxRevisionMacro(svkDcmRawDataReader, "$Rev$");
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

    vtkstd::string fileToCheck(fname);

    bool isDcmRaw = false; 

    if ( svkDcmHeader::IsFileDICOM( fname ) ) {

        svkImageData* tmp = svkMriImageData::New(); 

        tmp->GetDcmHeader()->ReadDcmFile( fname );
        vtkstd::string SOPClassUID = tmp->GetDcmHeader()->GetStringValue( "SOPClassUID" ) ;

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

        long unsigned int numBytes=  
            hdr->GetIntSequenceItemElement(
                "SVK_FILE_SET_SEQUENCE",
                fileNum,
                "SVK_FILE_NUM_BYTES"
            );

        long unsigned int numValues =  numBytes / sizeof(float); 
        float* fileBuffer = new float[ numValues ]; 

        hdr->GetFloatSequenceItemElement(
                "SVK_FILE_SET_SEQUENCE",
                fileNum,
                "SVK_FILE_CONTENTS",
                fileBuffer, 
                numValues
            );

        vtkstd::string outputFileName( fileName );  
        outputFileName.append(".extracted"); 
        ofstream outputFile ( outputFileName.c_str(), ios::binary );
        if( !outputFile) { 
            cout << "Cannot open file to extract raw data to: " << fileName << endl;
            this->SetErrorCode( vtkErrorCode::CannotOpenFileError );
        }
        
        outputFile.write( (char *)fileBuffer, numBytes); 

        if( outputFile.bad() ) { 
            cout << "Error extracting DICOM Raw Data to disk: " << fileName << endl;
            this->SetErrorCode( vtkErrorCode::CannotOpenFileError );
        }

        outputFile.close(); 

        if (fileBuffer != NULL) {
            delete[] fileBuffer; 
        }

    }

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

