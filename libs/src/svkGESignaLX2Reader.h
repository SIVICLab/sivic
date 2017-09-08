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
 *      Don C. Bigler, Ph.D.
 */


#ifndef SVK_GE_SIGNA_LX2_READER_H
#define SVK_GE_SIGNA_LX2_READER_H


#include <svkGESigna5XReader.h>


namespace svk {


/*! 
 *  This is a SIVIC reader for GE Genesis *.MR Signa LX2 files.  
 *  This class parses a GE MR header to initialize an svkMriImageData object 
 *  with an svkDcmHeader corresponding to a DICOM MRI Storage  
 *  SOP class (SOP Class UID: 1.2.840.10008.5.1.4.1.1.4).
 */

class svkGESignaLX2Reader : public svkGESigna5XReader 
{

    public:
        
        static svkGESignaLX2Reader* New();
        vtkTypeMacro( svkGESignaLX2Reader, svkGESigna5XReader);

        //  Methods:
        virtual int CanReadFile(const char* fname);

        // Description:
        // Valid extentsions
        virtual const char* GetFileExtensions() {
              return ".MR";
        }

        // Description: 
        // A descriptive name for this format
        virtual const char* GetDescriptiveName() {
            return "GE Genesis Signa LX2 File";
        }

        virtual svkImageReader2::ReaderType GetReaderType()
        {
            return svkImageReader2::GE_SIGNA_LX2;
        }


    protected:

        svkGESignaLX2Reader();
        ~svkGESignaLX2Reader();


        virtual GESignaHeader*  ReadHeader(const char *FileNameToRead);

};


}   //svk


#endif //SVK_GE_SIGNA_LX2_READER_H
