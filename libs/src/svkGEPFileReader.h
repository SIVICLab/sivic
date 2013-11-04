/*
 *  Copyright © 2009 The Regents of the University of California.
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

#ifndef SVK_GE_PFILE_READER_H
#define SVK_GE_PFILE_READER_H

#include <vtkCallbackCommand.h>
#include <vtkInformation.h>

#include <svkImageReader2.h>
#include <svkGEPFileMapper.h>
#include <svkGEPFileMapperMBrease.h>
#include <svkGEPFileMapperUCSF.h>
#include <svkGEPFileMapperUCSFfidcsi.h>
#include <svkGEPFileMapperUCSFfidcsiDev0.h>
#include <svkGEPFileMapperUCSFfidcsiDev07t.h>
#include <svkGEPFileMapperMPCSIOBL.h>
#include <svkEPSIReorder.h>

#include <vtkstd/map>
#include <vtkstd/vector>
#include <vtkstd/string>


namespace svk {


/*! 
 *
 *  This is an svkImageReader2 for reading GE P-files.  
 *  The reader supports 9.x - 20.x raw file versions.  The reader
 *  reads fields from GE P-files, putting the content into a string map.
 *  The reader then obtains a svkGEPFileMapper for the specific PSD in order to map 
 *  that content to an svkMrsImageData object (and svkDcmHeader representing 
 *  a DICOM MR Spectroscopy SOP Class UID).  The mapping will depend on the
 *  specific psd context, for example because of variability in meaning of 
 *  user cvs.  
 *  
 *  A mapper factory will ultimatly be developed to obtain the appropriate svkPFileMapper
 *  for a given Pfile.  
 *       
 *  Thanks to Mike Figueira of GE Medical for helful discussions about the class design. 
 *  Many thanks also to Sanjay Joshi( GE Medical recon ), Mike Figueira (GEMS) and Katie McMillan (GEMS) 
 *  for their help making this possible.   
 *       
 *  Acknowledgments:  Dr. Ping-Hong Yeh for providing sample data for validating 21.x 
 *  support. 
 */
class svkGEPFileReader : public svkImageReader2 
{

    public:

        vtkTypeRevisionMacro( svkGEPFileReader, svkImageReader2 );
        static              svkGEPFileReader* New();
        virtual int         CanReadFile( const char* fname );
        svkGEPFileMapper*   GetMapper(); 
        void                SetMapperBehavior(svkGEPFileMapper::MapperBehavior type);
        void                SetDeidentify( svkDcmHeader::PHIType phiType ); 
        void                SetDeidentify( svkDcmHeader::PHIType phiType, vtkstd::string deidentificationId ); 
        void                SetDeidentify( svkDcmHeader::PHIType phiType, vtkstd::string patientId, 
                                           vtkstd::string studyId ); 
        void                SetDeidentificationStudyUID(   vtkstd::string deidStudyUID); 
        void                SetDeidentificationSeriesUID(  vtkstd::string deidSeriesUID); 
        void                SetDeidentificationInstanceUID(vtkstd::string deidImageUID); 
        void                SetDeidentificationLandmarkUID(vtkstd::string deidLandmarkUID); 

        void                SetTemperature( float temp ); 
        void                SetChop( bool chop ); 
        void                PrintHeader();
        void                SetEPSIParams( 
                                svkEPSIReorder::EPSIType type, 
                                svkEPSIReorder::EPSIAxis axis, 
                                int first, 
                                int numLobes, 
                                int numSkip ); 
        void                OnlyParseHeader();
        void                Deidentify( ); 
        void                ModifyRawField( string rawField, string value); 

        vtkstd::map <vtkstd::string, vtkstd::vector< vtkstd::string > >          
                                                 GetPFMap(); 
        static float        LookupRawVersion(float rdbmRev, float rdbmRevSwapped); 



        // Description: 
        // A descriptive name for this format
        virtual const char* GetDescriptiveName() {
            return "GE pfile";
        } 

        bool                             checkSeriesUID;  


    protected:

        svkGEPFileReader();
        ~svkGEPFileReader();
  
        virtual int                              FillOutputPortInformation(int port, vtkInformation* info);
        virtual void                             ExecuteInformation();
        virtual void                             ExecuteData( vtkDataObject *output );
        virtual svkDcmHeader::DcmPixelDataFormat GetFileType();
        void                                     ReadGEPFile();
        void                                     ParsePFile();
        void                                     InitOffsetsMap();
        virtual void                             SetProvenance(); 

        vtkstd::map <vtkstd::string, vtkstd::vector< vtkstd::string > >          
                                                 pfMap;


    private:

        //  Methods:
        vtkstd::string      GetOffsetsString();
        virtual void        InitDcmHeader();
        void                PrintOffsets(); 
        void                DumpHeader();
        void                PrintKeyValuePairs();
        vtkstd::string      GetFieldAsString(vtkstd::string key); 
        int                 GEUncompressUID(unsigned char* short_uid, char* long_uid); 
        vtkstd::string      UncompressUID(const char* compressedUID); 
        int                 GECompressUID(unsigned char* short_uid, char* long_uid); 
        vtkstd::string      CompressUID(char* compressedUID); 
        float               GetPFileVersion(); 
        svkGEPFileMapper*   GetPFileMapper(); 
        static void         UpdateProgressCallback(
                                vtkObject* subject, 
                                unsigned long, 
                                void* thisObject, 
                                void* callData
                            );
        void                FillInMissingInfo(); 
        void                SetDataByteOrder(); 
        void                DeidentifyData(); 
        int                 GetNumElementsInField( vtkstd::string key ); 
        int                 GetNumBytesInField( vtkstd::string key ); 
        void                DeidentifyField( fstream* fs, vtkstd::string key, vtkstd::string deidString); 
        bool                IsFieldChar( vtkstd::string key ); 
        bool                IsFieldUID( vtkstd::string key ); 
        bool                IsFieldInt2( vtkstd::string key );
        bool                IsFieldFloat4( vtkstd::string key );
        bool                IsFieldInt4( vtkstd::string key );
        bool                IsFieldLInt4( vtkstd::string key );
        bool                IsFieldLInt8( vtkstd::string key );
        vtkstd::string      GetSeriesUID(const char* fname); 

        //  Members:
        ifstream*                       gepf;
        float                           pfileVersion;
        svkGEPFileMapper*               mapper;
       
        vtkstd::map < vtkstd::string, void* >            
                                        inputArgs; 

        vtkCallbackCommand*             progressCallback;
        bool                            onlyParseHeader;  
        vtkStringArray*                 tmpFileNames;

        svkDcmHeader::PHIType           phiType; 
        vtkstd::string                  deidStudyId; 
        vtkstd::string                  deidStudyUID; 
        vtkstd::string                  deidSeriesUID; 
        vtkstd::string                  deidImageUID; 
        vtkstd::string                  deidLandmarkUID; 

};


}   //svk

#endif //SVK_GE_PFILE_READER_H

