/*
 *  Copyright © 2009-2012 The Regents of the University of California.
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


#ifndef SVK_FDF_VOLUME_READER_H
#define SVK_FDF_VOLUME_READER_H

#include <vtkInformation.h>
#include <vtkFloatArray.h>
#include <vtkStringArray.h>

#include <svkVarianReader.h>

#include <vtkstd/map>
#include <vtkstd/string>
#include <vtkstd/vector>


namespace svk {


/*! 
 *  This is a SIVIC reader for Varian FDF files.  The reader parses multiple one or more *.fdf files 
 *  from a directory as well as the procpar file (if present) to create an svkImageData object. The 
 *  svkImageData's header (svkDcmHeader) Is initialized by parsing the Varian header information  
 *  from the fdf files (and/or procpar file) and mapping it to a DICOM EnhancedMRImageStorage 
 *  SOP class (1.2.840.10008.5.1.4.1.1.4.1) instance of the svkDcmHeader.
 *  
 *  float type Varian fdf pixel data is by default mapped to short valued pixels (16 bit). Reader methods 
 *  permit the output type/dynamic range to be set as needed.      
 * 
 */
class svkFdfVolumeReader : public svkVarianReader
{

    public:

        static svkFdfVolumeReader* New();
        vtkTypeRevisionMacro( svkFdfVolumeReader, svkVarianReader);

        // Description: 
        // A descriptive name for this format
        virtual const char* GetDescriptiveName() {
            return "Varian FDF File";
        }

        //  Methods:
        virtual int             CanReadFile(const char* fname);


    protected:

        svkFdfVolumeReader();
        ~svkFdfVolumeReader();

        virtual int                      FillOutputPortInformation(int port, vtkInformation* info);
        virtual void                     ExecuteInformation();
        virtual void                     ExecuteData(vtkDataObject *output);
        svkDcmHeader::DcmPixelDataFormat GetFileType();
        void                             ScaleTo16Bit( bool scaleTo16Bit, bool scaleToSignedShort, bool scaleToPositiveRange ); 


    private:

        //  Methods:
        virtual void                     InitDcmHeader();
        void                             InitPatientModule();
        void                             InitGeneralStudyModule();
        void                             InitGeneralSeriesModule();
        void                             InitGeneralEquipmentModule();
        void                             InitImagePixelModule();
        void                             InitMultiFrameFunctionalGroupsModule();
        void                             InitMultiFrameDimensionModule();
        void                             InitAcquisitionContextModule();
        void                             InitSharedFunctionalGroupMacros();
        void                             InitPerFrameFunctionalGroupMacros();
        void                             InitFrameContentMacro();
        void                             InitPlanePositionMacro();
        void                             InitPixelMeasuresMacro();
        void                             InitPlaneOrientationMacro();
        void                             InitMRReceiveCoilMacro();
        void                             ReadFdfFiles();
        vtkstd::string                   VarianToDicomDate(vtkstd::string* volumeDate);
        vtkstd::string                   GetDcmPatientPositionString();
        void                             ParseFdf();
        int                              GetFdfKeyValuePair( vtkStringArray* keySet = NULL);
        void                             SetKeysToSearch(vtkStringArray* fltArray, int fileIndex);
        int                              GetDataBufferSize();
        int                              GetHeaderValueAsInt(vtkstd::string keyString, int valueIndex = 0); 
        float                            GetHeaderValueAsFloat(vtkstd::string keyString, int valueIndex = 0); 
        vtkstd::string                   GetHeaderValueAsString(vtkstd::string keyString, int valueIndex = 0);
        bool                             IsKeyInHeader(vtkstd::string keyString); 
        void                             ParseAndSetStringElements(vtkstd::string key, vtkstd::string valueArrayString);
        void                             ConvertCmToMm();  
        void                             ConvertUserToMagnetFrame(); 
        vtkstd::string                   GetStringFromFloat(float floatValue); 
        void                             AddDimensionTo2DData();
        void                             PrintKeyValuePairs();
        void                             MapFloatValuesTo16Bit(
                                                vtkFloatArray* fltArray, 
                                                vtkDataArray* dataArray
                                         );

        //  Members:
        void*                                       pixelData; 
        ifstream*                                   fdfFile;
        vtkstd::map <vtkstd::string, vtkstd::vector<vtkstd::string> >       
                                                    fdfMap; 
        long                                        fileSize; 
        vtkStringArray*                             tmpFileNames;
        bool                                        scaleTo16Bit;
        bool                                        scaleToSignedShort;
        bool                                        scaleToPositiveRange;


};


}   //svk


#endif //SVK_FDF_VOLUME_READER_H

