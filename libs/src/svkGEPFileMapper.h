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

#ifndef SVK_GE_PFILE_MAPPER_H
#define SVK_GE_PFILE_MAPPER_H


#include <vtkImageData.h>
#include <vtkCallbackCommand.h>

#include <svkDcmHeader.h>
#include <svkImageReader2.h>
#include <svkMRSIOD.h>

#include <vtkstd/map>
#include <vtkstd/vector>
#include <vtkstd/string>
#include <vtkStringArray.h>


namespace svk {


/*! 
 *  Mapper from pfile header to DICOM IOD/SOP Class instance.   
 */
class svkGEPFileMapper : public vtkObject 
{

    public:

        vtkTypeRevisionMacro( svkGEPFileMapper, vtkObject );
        static          svkGEPFileMapper* New();

        /*!
         *  Data Loading Behavior options:
         */
        enum MapperBehavior {
            UNDEFINED = 0, 
            //  load the entire data set as is, no averaging or data extraction.
            LOAD_RAW, 
            //  if suppressed and unsuppressed data, load only the unsuppressed acquisitions
            LOAD_RAW_UNSUPPRESSED, 
            //  if suppressed and unsuppressed data, load only the suppressed acquisitions
            LOAD_RAW_SUPPRESSED,       
            //  if suppressed and unsuppressed data, load only the average of the unsuppressed acquisitions
            LOAD_AVG_UNSUPPRESSED, 
            //  if suppressed and unsuppressed data, load only the average of the suppressed acquisitions
            LOAD_AVG_SUPPRESSED,
            //  if EPSI data not handled intrinsically by the reader needs to be handled through command
            //  line args:
            LOAD_EPSI
        };

        virtual void            InitializeDcmHeader(
                                    vtkstd::map <vtkstd::string, vtkstd::vector< vtkstd::string > >  pfMap, 
                                    svkDcmHeader* header, 
                                    float pfileVersion, 
                                    int swapBytes, 
                                    vtkstd::map < vtkstd::string, void* >  inputArgs
                                );
        virtual void            ReadData( vtkStringArray* pFileNames, svkImageData* data );
        vtkstd::string          GetProgressText( );
        void                    SetProgressText( vtkstd::string progressText );
        static vtkstd::string   ConvertGEDateToDICOM( vtkstd::string geDate ); 
        static void             GetOriginFromCenter( 
                                             double center[3], 
                                             int numVoxels[3], 
                                             double voxelSpacing[3], 
                                             double dcos[3][3], 
                                             double origin[3] 
                                ); 
        static void             GetCenterFromOrigin( 
                                             double origin[3], 
                                             int numVoxels[3], 
                                             double voxelSpacing[3], 
                                             double dcos[3][3], 
                                             double center[3] 
                                ); 


    protected:

        svkGEPFileMapper();
        ~svkGEPFileMapper();
  

        void                    InitPatientModule();
        void                    InitGeneralStudyModule();
        void                    InitGeneralSeriesModule();
        void                    InitFrameOfReferenceModule();
        void                    InitGeneralEquipmentModule();
        void                    InitEnhancedGeneralEquipmentModule(); 
        virtual void            InitMultiFrameFunctionalGroupsModule();
        virtual void            InitAcquisitionContextModule();
        virtual void            InitMRSpectroscopyPulseSequenceModule();
        virtual void            InitSharedFunctionalGroupMacros();
        virtual void            InitPerFrameFunctionalGroupMacros();
        virtual void            InitPixelMeasuresMacro();
        virtual void            InitPlaneOrientationMacro();
        virtual void            InitMRSpectroscopyFrameTypeMacro();
        virtual void            InitMRTimingAndRelatedParametersMacro();
        virtual void            InitMRSpectroscopyFOVGeometryMacro();
        virtual void            InitMREchoMacro();
        virtual void            InitMRModifierMacro();
        virtual void            InitMRReceiveCoilMacro();
        virtual void            InitMRTransmitCoilMacro();
        virtual void            InitMRAveragesMacro();
        virtual void            InitMRSpatialSaturationMacro();
        virtual void            InitSatBand( float satRAS[3], float translation, int satBandNumber ); 
        virtual void            InitMRSpectroscopyModule();
        virtual vtkstd::string  GetVolumeLocalizationTechnique();
        virtual void            InitVolumeLocalizationSeq();
        virtual void            InitMRSpectroscopyDataModule();
        virtual void            InitK0Sampled( svkDcmHeader* hdr );

        virtual void            GetCenterFromRawFile( double* center );
        virtual float           GetFrequencyOffset(); 
        int                     GetNumVoxelsInVol();
        virtual void            GetNumVoxels( int numVoxels[3] ); 
        virtual int             GetNumKSpacePoints(); 
        void                    GetVoxelSpacing( double voxelSpacing[3] ); 
        void                    GetFOV( float fov[3] ); 
        int                     GetNumCoils(); 
        int                     GetNumFrames(); 
        virtual int             GetNumTimePoints(); 
        virtual int             GetNumDummyScans(); 
        virtual int             GetNumEPSIAcquisitions(); 
        bool                    AddDummy( int offset, int coilNum, int timePt ); 
        void                    GetDcos( double dcos[3][3] ); 
        virtual float           GetPPMRef(); 
        bool                    IsSwapOn(); 
        bool                    Is2D(); 
        virtual bool            IsChopOn(); 
        void                    GetXYZIndices(int dataIndex, int* x, int* y, int* z);
        void                    UpdateProgress(double amount);
        virtual void            ModifyBehavior( svkImageData* data ); 
        void                    RedimensionModifiedSVData( svkImageData* data ); 
        virtual int             GetNumberUnsuppressedAcquisitions(); 
        virtual int             GetNumberSuppressedAcquisitions(); 
        virtual void            GetSelBoxCenter( float selBoxCenter[3] ); 
        virtual void            GetSelBoxSize( float selBoxSize[3] ); 


        void                    SetCellSpectrum( 
                                    vtkImageData* data, 
                                    bool wasSampled, 
                                    int offset, 
                                    int index, 
                                    int x, int y, int z, 
                                    int timePoint = 0, 
                                    int channel = 0
                                );

        int                     GetHeaderValueAsInt(vtkstd::string key);
        long long int           GetHeaderValueAsLongInt(vtkstd::string key); 
        float                   GetHeaderValueAsFloat(vtkstd::string key);
        vtkstd::string          GetHeaderValueAsString(vtkstd::string key);
        virtual bool            WasIndexSampled(int xIndex, int yIndex, int zIndex); 


        vtkstd::string                          progressText;
        vtkstd::map <vtkstd::string, vtkstd::vector< vtkstd::string > >         
                                                pfMap;
        svkDcmHeader*                           dcmHeader; 
        float                                   pfileVersion;
        int*                                    specData; 
        svkDcmHeader::DcmDataOrderingDirection  dataSliceOrder;
        int                                     chopVal; 
        vtkstd::map < vtkstd::string, void* >   inputArgs; 
        int                                     swapBytes;
        svkMRSIOD*                              iod;
        double                                  progress; 

        bool                                    isInputArgSet(vtkstd::string argName);
        bool                                    GetInputArgStringValue(vtkstd::string argName, vtkstd::string* argValue);
        bool                                    GetInputArgBoolValue(vtkstd::string argName, bool* argValue);

        virtual void                            ReorderEPSI( svkMrsImageData* data );


};


}   //svk

#endif //SVK_GE_PFILE_MAPPER_H

