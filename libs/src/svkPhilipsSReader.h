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
 */


#ifndef SVK_PHILIPS_SREADER_H
#define SVK_PHILIPS_SREADER_H




#include <svkPhilipsSMapper.h>
#include <svkImageReader2.h>
#include <vtkInformation.h>
#include <vtkStringArray.h>
#include <vtkCallbackCommand.h>
#include <map>
#include <string>


namespace svk {


/*! 
 *  Reader for Philips MRS "S" files: SPAR and SDAT.  Parses the SPAR file to create a map of header values which 
 *  are used to initialize the DICOM header through the mapper instance.  The specific mapper instance
 *  is likely a function of the acquisition type / pulse sequence and is obtained from a mapper factory
 *  with rules TBD.  Data is parsed from the SDAT file.   
 */
class svkPhilipsSReader : public svkImageReader2 
{

    public:

        static svkPhilipsSReader* New();
        vtkTypeMacro( svkPhilipsSReader, svkImageReader2 );

        // Description: 
        // A descriptive name for this format
        virtual const char* GetDescriptiveName() {
            return "PHILIPS S File";
        }

        virtual svkImageReader2::ReaderType GetReaderType()
        {
            return svkImageReader2::PHILIPS_S;
        }

        //  Methods:
        virtual int             CanReadFile(const char* fname);


    protected:

        svkPhilipsSReader();
        ~svkPhilipsSReader();

        virtual int     FillOutputPortInformation(int port, vtkInformation* info);
        virtual void    ExecuteInformation();
        virtual void    ExecuteDataWithInformation(vtkDataObject *output, vtkInformation* outInfo);
        void            SetProgressText( string progressText );


    private:

        //  Methods:
        svkPhilipsSMapper*               GetPhilipsSMapper(); 
        virtual void                     InitDcmHeader();

        svkDcmHeader::DcmPixelDataFormat GetFileType();
        void                             ParseSDAT();
        void                             ParseSPAR(string path);
        void                             GetSKeyValuePair( vtkStringArray* keySet = NULL);
        static void                      UpdateProgressCallback(
                                            vtkObject* subject, 
                                            unsigned long, 
                                            void* thisObject, 
                                            void* callData
                                         );
        void                             UpdateProgress(double amount);
        void                             PrintSparKeyValuePairs();
        int                              GetSparKeyValuePair();


        //  Members:
        ifstream*                       sdatFile;
        ifstream*                       sparFile;
        map <string, string >           sparMap;  
        svkPhilipsSMapper*              mapper;
        string                          progressText;
        vtkCallbackCommand*             progressCallback;
        long                            sparFileSize;

};


}   //svk


#endif //SVK_PHILIPS_SREADER_H

