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


#ifndef SVK_GE_SIGNA_5X_READER_H
#define SVK_GE_SIGNA_5X_READER_H


#include </usr/include/vtk/vtkStringArray.h>

#include <svkImageReader2.h>
#include <svkGEImageHeader.h>
#include <svkEnhancedMRIIOD.h>
#include <algorithm>

namespace svk {


/*! 
 *  This is a SIVIC reader for GE Genesis *.MR Signa 5X files.  
 *  This class parses a GE MR header to initialize an svkMriImageData object 
 *  with an svkDcmHeader corresponding to a DICOM Enhanced MRI Storage  
 *  SOP class (SOP Class UID: 1.2.840.10008.5.1.4.1.1.4).
 *
 *  The ITK itkGE5ImageIO.cxx and VTK vtkGESignaReader.cxx files were used
 *  as guides when creating this class.  The CopyGenesisImage
 *  method is the vtkcopygenesisimage function from vtkGESignaReader.cxx
 *
 */

class svkGESigna5XReader : public svkImageReader2 
{

    public:
        
        static svkGESigna5XReader* New();
        vtkTypeMacro( svkGESigna5XReader, svkImageReader2);

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
            return "GE Genesis Signa 5X File";
        }

        virtual svkImageReader2::ReaderType GetReaderType()
        {
            return svkImageReader2::GE_SIGNA_5X;
        }


    protected:

        svkGESigna5XReader();
        ~svkGESigna5XReader();

        virtual void                             ExecuteInformation();
        virtual void                             ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo);
        virtual int                              FillOutputPortInformation(int port, vtkInformation* info);
        virtual svkDcmHeader::DcmPixelDataFormat GetFileType(); 

        int                                      numFrames;
        svkDcmHeader::DcmDataOrderingDirection   dataSliceOrder;
        virtual GESignaHeader*                   ReadHeader(const char *FileNameToRead);
        bool                                     CalculateTopLeftHandCornerAndRowColumnAndNormalVectors(GESignaHeader* hdr, 
                                                     double tlhc[3], double row[3], double column[3], double normal[3]);
        bool                                     SortFilesByImagePositionPatient(GESignaHeader* selectedFile, 
                                                     vtkStringArray* fileNames, bool ascending);
        bool                                     CopyGenesisImage(FILE *infp, int width, int height, int compress,
                                                     short *map_left, short *map_wide, unsigned short *output);
        bool                                     LoadData(const char *filename, unsigned short *outPtr, 
                                                     int *outExt, vtkIdType *);
        void                                     statTimeAndDateToAscii (void *clock, char *time, char *date);
        
        
        // Methods for initializing the DICOM header.
        void                InitDcmHeader();
        void                InitEnhancedMRImageModule();
        void                InitPatientModule();
        void                InitGeneralStudyModule();
        void                InitGeneralSeriesModule();
        void                InitGeneralEquipmentModule();
        void                InitEnhancedGeneralEquipmentModule();
        void                InitMultiFrameFunctionalGroupsModule(); 
        void                InitMRImageAndSpectroscopyInstanceMacro();
        void                InitMRPulseSequenceModule();
        void                InitSharedFunctionalGroupMacros(); 
        void                InitPixelMeasuresMacro();
        void                InitPlaneOrientationMacro(); 
        void                InitPixelValueTransformationMacro(); 
        void                InitPerFrameFunctionalGroupMacros(); 
        void                InitMRImageFrameTypeMacro(); 
        void                InitMRTimingAndRelatedParametersMacro();
        void                InitMRFOVGeometryMacro();
        void                InitMREchoMacro();
        void                InitMRModifierMacro();
        void                InitMRImagingModifierMacro();
        void                InitMRReceiveCoilMacro();
        void                InitMRTransmitCoilMacro();
        void                InitMRAveragesMacro();
        GESignaHeader*      imageHeader;
        svkEnhancedMRIIOD*  iod;

         

};


}   //svk


#endif //SVK_GE_SIGNA_5X_READER_H

