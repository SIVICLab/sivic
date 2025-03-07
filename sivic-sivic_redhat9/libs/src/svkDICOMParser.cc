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


#include <svkDICOMParser.h>


using namespace svk;



/*!
 *  Constructor.
 */
svkDICOMParser::svkDICOMParser()
{
}


/*!
 *  Destructor.
 */
svkDICOMParser::~svkDICOMParser()
{

    //if (this->dcmFile != NULL) {
        //delete this->dcmFile;
        //this->dcmFile = NULL;
    //}

}


/*
 *  Pares DICOM file to get offset to Pixel Data. 
 */
long int svkDICOMParser::GetPixelDataFileOffset(string fileName)
{

    this->ClearAllDICOMTagCallbacks();
    this->OpenFile(fileName);

    this->dcmFile = this->GetDICOMFile();

#ifdef ITK_BUILD 
    itkdicomparser::DICOMFile(dcmFile);
#else 
    IsDICOMFile(dcmFile);
#endif

    long fileSize = this->dcmFile->GetSize();

    do
    {
        unsigned short group          = this->dcmFile->ReadDoubleByte();
        unsigned short element        = this->dcmFile->ReadDoubleByte();
        unsigned short representation = this->dcmFile->ReadDoubleByteAsLittleEndian();

        bool isPixelData = false;


        int length = 0;
        DICOMParser::VRTypes mytype = DICOMParser::VR_UNKNOWN;
#ifdef ITK_BUILD 
        //IsValidRepresentation(short unsigned int&, int&, itkdicomparser::DICOMParser::VRTypes&)
#else
        this->IsValidRepresentation(representation, length, mytype);
#endif



        //  check for PixelData element, skip all others: 
        if ( group == 0x7FE0 && element == 0x0010 ) {
            isPixelData = true;
        }

        //   If found pixel data, return offset: 
        if ( isPixelData ) {

            return this->dcmFile->Tell();

        } else {
            if (length > 0 ) {
                this->dcmFile->Skip(length);
            }
        }

    } while ( (this->dcmFile->Tell() >= 0) && (this->dcmFile->Tell() < fileSize) );

    return -1; 
}

