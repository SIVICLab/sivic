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


#ifndef SVK_BRUKER_RAW_MRS_READER_H
#define SVK_BRUKER_RAW_MRS_READER_H

#include <svkImageReader2.h>
#include <svkBrukerRawMRSMapper.h>
#include </usr/include/vtk/vtkInformation.h>

#include <map>
#include <vector>
#include <string>

namespace svk {


/*! 
 *  Bruker Raw MRS reader class.  Reads Bruker ser files for MRSI experiments and converts the output to 
 *  a cartesian DICOM MRS output. 
 */
class svkBrukerRawMRSReader : public svkImageReader2 
{

    public:

        static svkBrukerRawMRSReader* New();
        vtkTypeMacro( svkBrukerRawMRSReader, svkImageReader2);

        // Description: 
        // A descriptive name for this format
        virtual const char* GetDescriptiveName() {
            return "Bruker RAW MRS File";
        }

        virtual svkBrukerRawMRSReader::ReaderType GetReaderType()
        {
            return svkImageReader2::BRUKER_RAW_MRS;
        }

        //  Methods:
        virtual int             CanReadFile(const char* fname);


    protected:

        svkBrukerRawMRSReader();
        ~svkBrukerRawMRSReader();


        //  Methods:
        virtual int     FillOutputPortInformation(int port, vtkInformation* info);
        virtual void    ExecuteInformation();
        virtual void    ExecuteDataWithInformation(vtkDataObject *output, vtkInformation* outInfo);

        void            SetProgressText( string progressText );

        int             GetNumPixelsInVol();
        int             GetNumSlices();
	    void            ParseParamFiles( );
        int             GetParamKeyValuePair();
        void            ReadLine(ifstream* fs, istringstream* iss);
        void            ParseAndSetParamStringElements(
                                             string key, 
                                             string valueArray 
                        );
        void            PrintParamKeyValuePairs();
        int             GetNumberOfParamElements( string* valueString );
        void            GetParamValueArray( string* valueString );
        void            AssignParamVectorElements(
                                            vector<string>* paramVector,
                                            string valueArray
                        );


    private:

        //  Methods:
        svkBrukerRawMRSMapper*              GetBrukerRawMRSMapper();
        virtual void                        InitDcmHeader();

        svkDcmHeader::DcmPixelDataFormat    GetFileType();
        static void                         UpdateProgressCallback(
                                                vtkObject* subject,
                                                unsigned long,
                                                void* thisObject,
                                                void* callData
                                            );
        void                                UpdateProgress(double amount);
        void                                ParseParamFileToMap( string paramFileName ); 


        //  Members:
        ifstream*                                   paramFile;
        long                                        paramFileSize;
        map <string, vector < string>  >            paramMap;  
        svkBrukerRawMRSMapper*                      mapper;
        svkDcmHeader::DcmDataOrderingDirection      dataSliceOrder;
        string                                      progressText;
        vtkCallbackCommand*                         progressCallback;


};


}   //svk


#endif //SVK_BRUKER_RAW_MRS_READER_H

