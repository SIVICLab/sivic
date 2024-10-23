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


#ifndef SVK_SIEMENS_RDA_READER_H
#define SVK_SIEMENS_RDA_READER_H


#include </usr/include/vtk/vtkImageData.h>
#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkStringArray.h>

#include <svkImageReader2.h>
#include <svkMRSIOD.h>

#include <map>
#include <vector>
#include <string>


namespace svk {


/*! 
 *  This is a SIVIC reader for Siemens *.rda files.  
 *  This class parses a Siemens rda header to initialize an svkMrsImageData object 
 *  with an svkDcmHeader corresponding to a DICOM MR Spectroscopy Storage  
 *  SOP class (SOP Class UID: 1.2.840.10008.5.1.4.1.1.4.2). 
 *
 *  Thanks to Jeff Yager (Ph.D Candidate) and Vincent A. Magnotta, Ph.D. of the University of Iowa
 *  Department of Radiology for providing sample Siemens .rda phantom data. 
 *
 *  Thanks to Bjoern Menze, Ph.D. of the MIT CSAIL Medical Vision Group 
 *  (http://groups.csail.mit.edu/vision/medical-vision/members.html) for providing sample data 
 *  and for help implementing and validating this class.  
 * 
 *  This is a beta release of this class.  Spatial localization parameters are  still being validated 
 *  and DICOM IE initialization is being refactored into svkMRSIOD. 
 */
class svkSiemensRdaReader : public svkImageReader2
{

    public:

        static svkSiemensRdaReader* New();
        vtkTypeMacro( svkSiemensRdaReader, svkImageReader2);

        // Description: 
        // A descriptive name for this format
        virtual const char* GetDescriptiveName() {
            return "Siemens RDA File";
        }

        virtual svkImageReader2::ReaderType GetReaderType()
        {
            return svkImageReader2::SIEMENS_RDA;
        }

        //  Methods:
        virtual int             CanReadFile(const char* fname);


    protected:

        svkSiemensRdaReader();
        ~svkSiemensRdaReader();

        virtual int                      FillOutputPortInformation(int port, vtkInformation* info);
        virtual void                     ExecuteInformation();
        virtual void                     ExecuteDataWithInformation(vtkDataObject *output, vtkInformation* outInfo);
        svkDcmHeader::DcmPixelDataFormat GetFileType();


    private:

        //  Methods:
        virtual void    InitDcmHeader();
        void            InitPatientModule();
        void            InitGeneralStudyModule();
        void            InitGeneralSeriesModule();
        void            InitGeneralEquipmentModule();
        void            InitMultiFrameFunctionalGroupsModule();
        void            InitMultiFrameDimensionModule();
        void            InitAcquisitionContextModule();
        void            InitMRSpectroscopyPulseSequenceModule();
        void            InitSharedFunctionalGroupMacros();
        void            InitPerFrameFunctionalGroupMacros();
        void            InitPixelMeasuresMacro();
        void            InitFrameContentMacro();
        void            InitPlanePositionMacro();
        void            InitPlaneOrientationMacro();
        void            InitMRSpectroscopyFrameTypeMacro();
        void            InitMRTimingAndRelatedParametersMacro();
        void            InitMRSpectroscopyFOVGeometryMacro();
        void            InitMREchoMacro();
        void            InitMRModifierMacro();
        void            InitMRReceiveCoilMacro();
        void            InitMRTransmitCoilMacro();
        void            InitMRAveragesMacro();
        void            InitMRSpatialSaturationMacro();
        void            InitMRSpatialVelocityEncodingMacro();
        void            InitMRSpectroscopyModule();
        void            InitVolumeLocalizationSeq();
        void            InitMRSpectroscopyDataModule();

        void            ReadRdaFiles(vtkImageData* data);
        int             GetNumPixelsInVol(); 
        string          GetDcmPatientPositionString(string patientPosition);
        void            SetCellSpectrum( vtkImageData* data, int x, int y, int z, int timePt = 0, int coilNum = 0 );
        void            ParseRda();
        int             GetRdaKeyValuePair( );
        int             GetHeaderValueAsInt(string keyString, int valueIndex = 0); 
        float           GetHeaderValueAsFloat(string keyString, int valueIndex = 0); 
        string          GetHeaderValueAsString(string keyString, int valueIndex = 0);
        void            ParseAndSetStringElements(string key, string valueArrayString);
        string          GetStringFromFloat(float floatValue); 
        void            PrintKeyValuePairs();
        void            MapDoubleValuesToFloat(double* specDataDbl, float* specData, int numVals); 
        void            GetDcosFromRda(double dcos[3][3]); 
        void            GetDcosFromRda(float dcos[3][3]); 
        float           GetPPMRef(); 


        //  Members:
        float*                                  specData; 
        ifstream*                               rdaFile;
        map <string,  vector<string> >          rdaMap; 
        long                                    fileSize; 
        vtkStringArray*                         tmpFileNames;
        int                                     numSlices;
        int                                     numCoils;
        int                                     numTimePts;
        svkDcmHeader::DcmDataOrderingDirection  dataSliceOrder;
        svkMRSIOD*                              iod; 
        int                                     endOfHeaderPos; 

};


}   //svk


#endif //SVK_SIEMENS_RDA_READER_H

