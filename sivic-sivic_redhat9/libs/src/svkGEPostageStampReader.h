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


#ifndef SVK_GE_POSTAGE_STAMP_READER_H
#define SVK_GE_POSTAGE_STAMP_READER_H

#include <svkDcmVolumeReader.h>


namespace svk {


/*! 
 *  
 */

class svkGEPostageStampReader : public svkDcmVolumeReader 
{

    public:

        static svkGEPostageStampReader* New();
        vtkTypeMacro( svkGEPostageStampReader, svkDcmVolumeReader );


        // Description: 
        // A descriptive name for this format
        virtual const char* GetDescriptiveName() {
            return "GE Postage Stamp File";
        }

        virtual svkImageReader2::ReaderType GetReaderType()
        {
            return svkImageReader2::GE_POSTAGE_STAMP;
        }


        //  Methods:
        virtual int CanReadFile(const char* fname);


    protected:

        svkGEPostageStampReader();
        ~svkGEPostageStampReader();

        virtual int                              FillOutputPortInformation(int port, vtkInformation* info);
        virtual svkDcmHeader::DcmPixelDataFormat GetFileType();
        virtual void                             InitDcmHeader(); 
        virtual bool                             CheckForMultiVolume(); 


    private:

        void            InitMultiFrameFunctionalGroupsModule(); 
        void            InitSharedFunctionalGroupMacros();
        void            InitMRTimingAndRelatedParametersMacro(); 
        void            InitMREchoMacro(); 
        void            InitMRReceiveCoilMacro();
        void            InitMRAveragesMacro(); 
        void            InitMRSpectroscopyFOVGeometryMacro(); 
        void            InitMRSpectroscopyModule(); 
        void            InitMRSpectroscopyDataModule(); 
        void            InitPerFrameFunctionalGroupMacros(); 
        void            InitPixelMeasuresMacro(); 
        void            InitVolumeLocalizationSeq(); 
        void            InitMRSpatialSaturationMacro(); 
        void            InitSatBand( float satRAS[3], float translation, int satBandNumber ); 
        void            GetColsAndRows(int* numCols, int* numRows); 


        virtual void    LoadData(svkImageData* data); 
        virtual void    InitPrivateHeader(); 
        void            SetCellSpectrum(
                            svkImageData* data, 
                            int x, 
                            int y, 
                            int z, 
                            int timePt, 
                            int coilNum, 
                            int numComponents, 
                            short* specData
                        ); 

        int             numFreqPts;
        int             numTimePts;
        int             imageCols; 
        int             imageRows; 

};


}   //svk


#endif //SVK_GE_POSTAGE_STAMP_READER_H

