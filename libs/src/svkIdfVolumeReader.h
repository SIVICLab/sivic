/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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

#include <vtkInformation.h>
#include <vtkstd/map>
#include <vtkstd/string>

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
        vtkTypeRevisionMacro( svkIdfVolumeReader, svkImageReader2);

        // Description: 
        // A descriptive name for this format
        virtual const char* GetDescriptiveName() {
            return "IDF File";
        }

        //  Methods:
        virtual int             CanReadFile(const char* fname);
        void                    OnlyReadHeader(bool onlyReadHeader);

        typedef enum {
            BYTE_DATA = 2,
            INT2_DATA = 3,
            FLT_DATA = 7
        } IDF_Data_Type;


    protected:

        svkIdfVolumeReader();
        ~svkIdfVolumeReader();

        virtual int                              FillOutputPortInformation(int port, vtkInformation* info);
        virtual void                             ExecuteInformation();
        virtual void                             ExecuteData(vtkDataObject *output);
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
        void            InitMultiFrameDimensionModule();
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
        void            InitNonIdfTags();
        void            ReadVolumeFile();
        int             GetNumPixelsInVol();
        int             GetNumSlices();
        void            ParseIdfComment(
                            vtkstd::string comment, 
                            vtkstd::string* patientsName, 
                            vtkstd::string* seriesDescription, 
                            vtkstd::string* studyDate
                        );
        vtkstd::string  GetDcmPatientPositionString(vtkstd::string patientPosition);
        void            ParseIdf();
        void            PrintKeyValuePairs();


        //  Members:
        void*                                   pixelData; 
        ifstream*                               volumeHdr;
        vtkstd::map <vtkstd::string, vtkstd::string>                    
                                                idfMap; 
        int                                     numSlices; 
        svkDcmHeader::DcmDataOrderingDirection  dataSliceOrder;
        bool                                    onlyReadHeader; 
        svkEnhancedMRIIOD*                      iod; 

};


}   //svk


#endif //SVK_IDF_VOLUME_READER_H

