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


#ifndef SVK_IDF_VOLUME_READER_H
#define SVK_IDF_VOLUME_READER_H

#include </usr/include/vtk/vtkInformation.h>
#include <map>
#include </usr/include/vtk/vtkStringArray.h>
#include <string>

#include <svkUtils.h>
#include <svkImageReader2.h>
#include <svkEnhancedMRIIOD.h>


namespace svk {


/*! 
 *  
 */
class svkIdfVolumeReader : public svkImageReader2 
{

    public:

        static svkIdfVolumeReader* New();
        vtkTypeMacro( svkIdfVolumeReader, svkImageReader2);


        typedef enum {
            BYTE_DATA = 2,
            INT2_DATA = 3,
            FLT_DATA = 7
        } IDF_Data_Type;

        typedef enum {
            TIME_SERIES_DATA = 0,
            MULTI_CHANNEL_DATA 
        } MultiVolumeType;


        // Description: 
        // A descriptive name for this format
        virtual const char* GetDescriptiveName() {
            return "IDF File";
        }

        virtual svkImageReader2::ReaderType GetReaderType()
        {
            return svkImageReader2::IDF;
        }

        void    SetMultiVolumeType(svkIdfVolumeReader::MultiVolumeType volumeType);   

        //  Methods:
        virtual int             CanReadFile(const char* fname);
        void                    SetReadIntAsSigned(bool readIntAsSigned);

    protected:

        svkIdfVolumeReader();
        ~svkIdfVolumeReader();

        virtual int                              FillOutputPortInformation(int port, vtkInformation* info);
        virtual void                             ExecuteInformation();
        virtual void                             ExecuteDataWithInformation(vtkDataObject *output, vtkInformation* outInfo);
        virtual svkDcmHeader::DcmPixelDataFormat GetFileType();


    private:

        //  Methods:
        virtual void    InitDcmHeader();
        void            InitPatientModule();
        void            InitGeneralStudyModule();
        void            InitGeneralSeriesModule();
        void            InitGeneralEquipmentModule();
        void            InitImagePixelModule();
        void            InitMultiFrameFunctionalGroupsModule();
        void            InitAcquisitionContextModule();
        void            InitSharedFunctionalGroupMacros();
        void            InitPerFrameFunctionalGroupMacros();
        void            InitFrameContentMacro();
        void            InitPlanePositionMacro();
        void            InitPixelMeasuresMacro();
        void            InitPlaneOrientationMacro();
        void            InitMRImagingModifierMacro(); 
        void            InitMRReceiveCoilMacro();
        void            InitMRTransmitCoilMacro(); 
        void            ReadVolumeFile();
        int             GetNumPixelsInVol();
        int             GetNumSlices();
        void            ParseIdfComment(
                            string comment, 
                            string* PatientName, 
                            string* seriesDescription, 
                            string* studyDate
                        );
        string  GetDcmPatientPositionString(string patientPosition);
        void            ParseIdf();
        void            PrintKeyValuePairs();
        bool            IsIdfStudyIdAccessionNumber(); 



        //  Members:
        void*                                   pixelData; 
        ifstream*                               volumeHdr;
        map <string, string>                    
                                                idfMap; 
        svkDcmHeader::DcmDataOrderingDirection  dataSliceOrder;
        bool                                    readIntAsSigned;
        svkEnhancedMRIIOD*                      iod; 
        vtkStringArray*                         tmpFileNames;
        int                                     numFrames;
        int                                     numSlices;
        int                                     numVolumes;
        svkIdfVolumeReader::MultiVolumeType     multiVolumeType;

};


}   //svk


#endif //SVK_IDF_VOLUME_READER_H

