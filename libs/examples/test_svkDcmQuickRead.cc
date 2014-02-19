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


#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkImageData.h>
#include <svkMriImageData.h>
#include <svkDcmHeader.h>
#include <DICOMParser.h>
#include <DICOMFile.h>

using namespace std; 
using namespace svk; 

bool IsValidRepresentation(DICOMFile* dcmFile, unsigned short rep, int& len, DICOMParser::VRTypes& mytype);

int main (int argc, char** argv)
{

    if ( argc != 2 ) {
        std::cout << " test_svkDcmQuickRead fileName " << endl;
        exit(1); 
    }
    string fileName(argv[1]); 

    svkImageData* tmp = svkMriImageData::New();
    tmp->GetDcmHeader()->ReadDcmFile(  fileName  );

    cout << "check" << endl;

    //cout << "PD EXISTS: " << tmp->GetDcmHeader()->ElementExists("PixelData") << endl;;  


    svkDcmHeader::DimensionVector dimensionVector = tmp->GetDcmHeader()->GetDimensionIndexVector();
    int rows = svkDcmHeader::GetDimensionValue( &dimensionVector, svkDcmHeader::ROW_INDEX) + 1;
    int columns = svkDcmHeader::GetDimensionValue( &dimensionVector, svkDcmHeader::COL_INDEX) + 1;
    int numFrames = tmp->GetDcmHeader()->GetIntValue( "NumberOfFrames" );

    int numPixels = rows * columns * numFrames; 
    int numBytesPerPixel = 2; 
    int pixelBytes = numPixels * numBytesPerPixel; 

    cout << "numPixels: " << numPixels << endl;

    numPixels = 100; 
    for (int i = 5800; i < 6200; i++ ) {
    //for (int i = 0; i < numPixels; i++ ) {
        short pixel = tmp->GetDcmHeader()->GetPixelValue( i ); 
        cout << "PIXEL Orig: " << i << " " << pixel << endl;
    }

    


    ////////////////////////////////////
    //  -----------------------
    //  try to get file offset to start of PixelData element
    //  -----------------------
    ////////////////////////////////////
    cout << "DICOM PARSER" << endl;
    DICOMParser* dcmParser = new DICOMParser(); 
    dcmParser->ClearAllDICOMTagCallbacks();
    dcmParser->OpenFile(fileName);
    //dcmParser->ReadHeader();
    DICOMFile* dcmFile = dcmParser->GetDICOMFile(); 
    dcmParser->IsDICOMFile(dcmFile);  


    long fileSize = dcmFile->GetSize();
    cout << "FS: " << fileSize << endl;
    do
    {
        unsigned short group = dcmFile->ReadDoubleByte();
        unsigned short element = dcmFile->ReadDoubleByte();
        unsigned short representation = dcmFile->ReadDoubleByteAsLittleEndian();

        bool isPixelData = false; 
        //  check for PixelData: 
        if ( group == 0x7FE0 && element == 0x0010 ) {
            cout << "group,element,rep " << group << ":" << element << " -> " << representation << endl;
            isPixelData = true; 
        }

        int length = 0;
        DICOMParser::VRTypes mytype = DICOMParser::VR_UNKNOWN; 
        IsValidRepresentation(dcmFile, representation, length, mytype);

        if ( isPixelData ) {

            cout << "TELL start of PixelData: " << dcmFile->Tell() << endl; 
            unsigned short* pixelData = reinterpret_cast<unsigned short*>(
                dcmFile->ReadAsciiCharArray(length)
            );
            cout << "PixelData length: " << length <<  endl;
            //for (int i = 5800; i < 6200; i++ ) {
            for (int i = 0; i < length/2; i++ ) {
                //cout << "p[" << i << "] = " << pixelData[i] << endl;       
            }
            cout << "TELL start of PixelData: " << dcmFile->Tell() << endl; 
        } else { 
            //cout << "SKIP" << endl;
            if (length > 0 ) {
                dcmFile->Skip(length); 
            }
        }

    } while ((dcmFile->Tell() >= 0) && (dcmFile->Tell() < fileSize));


    delete dcmParser; 
    cout << "DICOM PARSER - END" << endl;


    //  ====================================================== 
    //  ====================================================== 
    //  and finally the svk integrated version:  
    //  ====================================================== 
    //  ====================================================== 

    long int svkPD = svkDcmHeader::GetPixelDataOffset( fileName ); 
    cout << "SVK PD: " << svkPD << endl;

    for (int i = 5800; i < 6200; i++ ) {
        short pixelValue = svkDcmHeader::GetPixelValueAsShort( svkPD, i, fileName ); 
        cout << "LIB VALUE OF PIXEL: " << pixelValue << endl;
    }
    
/*
    long int pdOffset = svkDcmHeader::GetPixelDataOffset( fileName ); 
    int pixelIndex; 
    short pixelValue = svkDcmHeader::GetPixelValueAsShort( pdOffset, pixelIndex, fileName ); 
*/

    
    ifstream* dcmFS = new ifstream();
    dcmFS->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

    try {
        dcmFS->open( fileName.c_str(), ios::binary );
        dcmFS->seekg(0, ios::beg);

        void* pixelValue = new short; 
        //for (int i = 0 ; i < 500; i++) {
        for (int i = 5800; i < 6200; i++ ) {
            long int offset = svkPD + (2 * i); 
            dcmFS->seekg(offset, ios::beg);
            dcmFS->read( static_cast<char*>(pixelValue), 2);
            cout << "SVK PV: " << i << " " << *static_cast<short*>(pixelValue) << endl;
        }
        
        dcmFS->close();

    } catch (ifstream::failure e) {
        cout << "ERROR: Exception opening/reading file " << endl;
    }


 



    return 0; 
}


bool IsValidRepresentation(DICOMFile* dcmFile, unsigned short rep, int& len, DICOMParser::VRTypes& mytype)
{
  switch (rep)
    {
    case DICOMParser::VR_AW:
    case DICOMParser::VR_AE:
    case DICOMParser::VR_AS:
    case DICOMParser::VR_CS:
    case DICOMParser::VR_UI:
    case DICOMParser::VR_DA:
    case DICOMParser::VR_DS:
    case DICOMParser::VR_DT:
    case DICOMParser::VR_IS:
    case DICOMParser::VR_LO:
    case DICOMParser::VR_LT:
    case DICOMParser::VR_PN:
    case DICOMParser::VR_ST:
    case DICOMParser::VR_TM:
    case DICOMParser::VR_UT: // new
    case DICOMParser::VR_SH:
    case DICOMParser::VR_FL: 
    case DICOMParser::VR_SL:
    case DICOMParser::VR_AT:
    case DICOMParser::VR_UL:
    case DICOMParser::VR_US:
    case DICOMParser::VR_SS:
    case DICOMParser::VR_FD:
      len = dcmFile->ReadDoubleByte();
      mytype = DICOMParser::VRTypes(rep);
      return true;

    case DICOMParser::VR_OB: // OB - LE
    case DICOMParser::VR_OW:
    case DICOMParser::VR_UN:
    case DICOMParser::VR_SQ:
      dcmFile->ReadDoubleByte();
      len = dcmFile->ReadQuadByte();
      mytype = DICOMParser::VRTypes(rep);
      return true;

    default:
      //
      //
      // Need to comment out in new paradigm.
      //
      dcmFile->Skip(-2);
      len = dcmFile->ReadQuadByte();
      mytype = DICOMParser::VR_UNKNOWN;
      return false;
    }
}

