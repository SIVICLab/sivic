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


#ifndef SVK_IMAGE_READER_2_H
#define SVK_IMAGE_READER_2_H


#include </usr/include/vtk/vtkImageReader2.h>
#include </usr/include/vtk/vtkImageData.h>

#include <svkImageData.h>
#include <svkUtils.h>


namespace svk {


/*! 
 *  
 */
class svkImageReader2 : public vtkImageReader2 
{

    public:

        vtkTypeMacro( svkImageReader2, vtkImageReader2);

        typedef enum {
            UNDEFINED = -1,
            DICOM_MRS=0,
            DICOM_MRI,
            DICOM_PET,
            DICOM_ENHANCED_MRI,
            DICOM_SEGMENTATION,
            DICOM_RAW,
            SIEMENS_RDA,
            VARIAN_FID,
            VARIAN_FDF,
            GE_PFILE,
            GE_SDBM,
            GE_POSTAGE_STAMP,
            GE_SIGNA_LX2,
            GE_SIGNA_5X,
            LC_MODEL_COORD, 
            LC_MODEL_CSV, 
            LC_MODEL_TABLE, 
            VARIABLE_FLIP_DAT, 
            BRUKER_MRS,
            BRUKER_RAW_MRS,
            PHILIPS_S,
            NIFTI,
            DDF,
            IDF,
            LAST_TYPE = IDF
        } ReaderType;


        //  Methods:
        svkImageData*                        GetOutput();
        svkImageData*                        GetOutput(int);
        svkDcmHeader*                        GetDcmHeader( const char* fileName );
        static string                        StripWhite(string in);
        static string                        RemoveDelimFromDate(string* slashDate, char delimChar = '/'); 
        static string                        GetFileRoot(const char* fname);
        static string                        GetFileExtension(const char* fname);
        static string                        GetFilePath(const char* fname);
        static string                        GetFileNameWithoutPath(const char* fname); 
        void                                 OnlyReadOneInputFile();
        void                                 OnlyGlobFiles();
        void                                 GlobFileNames(); 
        static long                          GetFileSize(ifstream* fs); 
        void                                 OnlyReadHeader(bool onlyReadHeader);
        virtual svkImageReader2::ReaderType  GetReaderType() = 0;


    protected:

        svkImageReader2();
        ~svkImageReader2();

        //  Methods:
        virtual int                              FillOutputPortInformation(int port, vtkInformation* info);
        virtual svkDcmHeader::DcmPixelDataFormat GetFileType() = 0;
        void                                     SetupOutputInformation(); 
        void                                     SetupOutputExtent(); 
        void                                     SetupOutputScalarData(); 

        virtual void                             ReadLine(ifstream* hdr, istringstream* iss);
        void                                     ReadLineIgnore(
                                                    ifstream* hdr, 
                                                    istringstream* iss, 
                                                    char delim
                                                 ); 
        string                                   ReadLineSubstr(
                                                    ifstream* hdr, 
                                                    istringstream* iss, 
                                                    int start, 
                                                    int stop
                                                 ); 
        string                                   ReadLineValue( 
                                                    ifstream* hdr, 
                                                    istringstream* iss, 
                                                    char delim
                                                 ); 
        int                                      ReadLineKeyValue( 
                                                    ifstream* hdr, 
                                                    istringstream* iss, 
                                                    char delim, 
                                                    string* key, 
                                                    string* value
                                                ); 


        virtual void                             SetProvenance(); 


        //  Members:
        vtkDataArray*                            dataArray;
        bool                                     readOneInputFile;
        bool                                     onlyGlobFiles;
        bool                                     onlyReadHeader;
        virtual string                           GetFileSeriesDescription( string fileName ); 
        void                                     SetReadLength(int length); 


    private:

        virtual void                            InitDcmHeader() = 0;
        int                                     readLength;


};


}   //svk


#endif //SVK_IMAGE_READER_2_H

