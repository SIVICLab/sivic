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

#ifndef SVK_SDBM_VOLUME_READER_H
#define SVK_SDBM_VOLUME_READER_H


#include </usr/include/vtk/vtkImageData.h>
#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkStringArray.h>

#include <svkImageReader2.h>
#include <svkMRSIOD.h>

#include <map>
#include <string>


namespace svk {


/*! 
 *  Reader for GE sdbm files.   
 */
class svkSdbmVolumeReader : public svkImageReader2 
{

    public:

        static svkSdbmVolumeReader* New();
        vtkTypeMacro( svkSdbmVolumeReader, svkImageReader2 );

        // Description: 
        // A descriptive name for this format
        virtual const char* GetDescriptiveName() {
            return "GE sdbm File";
        }

        virtual svkImageReader2::ReaderType GetReaderType()
        {
            return svkImageReader2::GE_SDBM;
        }

        //  Methods:
        virtual int     CanReadFile( const char* fname );


    protected:

        svkSdbmVolumeReader();
        ~svkSdbmVolumeReader();
  
        virtual int                              FillOutputPortInformation(int port, vtkInformation* info);
        virtual void                             ExecuteInformation();
        virtual void                             ExecuteDataWithInformation( vtkDataObject *output, vtkInformation* outInfo );
        virtual svkDcmHeader::DcmPixelDataFormat GetFileType();


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
        void            ReadComplexFile( vtkImageData* data );
        int             GetNumVoxelsInVol();
        int             GetNumSlices();
        void            SetCellSpectrum( vtkImageData* data, int x, int y, int z, int channel = 0, int timepoint = 0 );
        void            ParseShf();
        void            ParseShfDim( string dimenNum ); 
        string          ReadLineSubstr(istringstream* iss, int start, int stop);
        int             ReadLineKeyValue(istringstream* iss, char delim, string* key, string* value);
        string          ReadLineIgnore(istringstream* iss, char delim);
        void            PrintKeyValuePairs(); 
        int             GetHeaderValueAsInt(map <string, string> hdrMap, 
                            string keyString, int valueIndex = 0); 
        float           GetHeaderValueAsFloat(map <string, string> hdrMap, 
                            string keyString, int valueIndex = 0); 
        int             GetNumPixelsInVol(); 
        bool            IsMultiCoil(); 



        //  Members:
        float*                                  specData; 
        int                                     numFrames;
        int                                     numSlices;
        int                                     numCoils;
        static const string                     MFG_STRING;
        double                                  dcos[3][3];
        svkDcmHeader::DcmDataOrderingDirection  dataSliceOrder;
        ifstream*                               shfHdr;
        map <string, string>                    shfMap;                  
        vtkStringArray*                         tmpFileNames;
        svkMRSIOD*                              iod; 


};


}   //svk

#endif //SVK_SDBM_VOLUME_READER_H

