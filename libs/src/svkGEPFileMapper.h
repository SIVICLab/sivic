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

#ifndef SVK_GE_PFILE_MAPPER_H
#define SVK_GE_PFILE_MAPPER_H


#include <vtkObjectFactory.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkShortArray.h>
#include <vtkCellData.h>
#include <vtkFieldData.h>
#include <vtkDataObject.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkDebugLeaks.h>
#include <vtkCallbackCommand.h>

#include <svkDcmHeader.h>
#include <svkByteSwap.h>
#include <svkSpecUtils.h>
#include <svkImageData.h>
#include <svkMrsImageData.h>
#include <svkImageReader2.h>

#include <map>
#include <vector>


namespace svk {


using namespace std;


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
            // load the entire data set as is, no averaging or data extraction.
            LOAD_RAW, 
            // if suppressed and unsuppressed data, load only the unsuppressed acquisitions
            LOAD_RAW_UNSUPPRESSED, 
            // if suppressed and unsuppressed data, load only the suppressed acquisitions
            LOAD_RAW_SUPPRESSED,       
            // if suppressed and unsuppressed data, load only the average of the unsuppressed acquisitions
            LOAD_AVG_UNSUPPRESSED, 
            // if suppressed and unsuppressed data, load only the average of the suppressed acquisitions
            LOAD_AVG_SUPPRESSED    
        };

        void            SetMapperBehavior(MapperBehavior behaviorFlag);


        virtual void    InitializeDcmHeader(
                            map <string, vector< string > >  pfMap, 
                            svkDcmHeader* header, 
                            float pfileVersion, 
                            bool swapBytes
                        );
        void            ReadData( string pFileName, svkImageData* data );
        string          GetProgressText( );
        void            SetProgressText( string progressText );


    protected:

        svkGEPFileMapper();
        ~svkGEPFileMapper();
  

        void            InitPatientModule();
        void            InitGeneralStudyModule();
        void            InitGeneralSeriesModule();
        void            InitFrameOfReferenceModule();
        void            InitGeneralEquipmentModule();
        void            InitEnhancedGeneralEquipmentModule(); 
        virtual void    InitMultiFrameFunctionalGroupsModule();
        virtual void    InitAcquisitionContextModule();
        virtual void    InitMRSpectroscopyPulseSequenceModule();
        virtual void    InitSharedFunctionalGroupMacros();
        virtual void    InitPerFrameFunctionalGroupMacros();
        virtual void    InitPixelMeasuresMacro();
        virtual void    InitPlaneOrientationMacro();
        virtual void    InitFrameAnatomyMacro();
        virtual void    InitMRSpectroscopyFrameTypeMacro();
        virtual void    InitMRTimingAndRelatedParametersMacro();
        virtual void    InitMRSpectroscopyFOVGeometryMacro();
        virtual void    InitMREchoMacro();
        virtual void    InitMRModifierMacro();
        virtual void    InitMRReceiveCoilMacro();
        virtual void    InitMRTransmitCoilMacro();
        virtual void    InitMRAveragesMacro();
        virtual void    InitMRSpatialSaturationMacro();
        virtual void    InitSatBand( float satRAS[3], float translation, int satBandNumber ); 
        virtual void    InitMRSpectroscopyModule();
        virtual void    InitVolumeLocalizationSeq();
        virtual void    InitMRSpectroscopyDataModule();

        virtual void    GetCenterFromRawFile( double* center );
        virtual float   GetFrequencyOffset(); 
        int             GetNumVoxelsInVol();
        void            GetNumVoxels( int numVoxels[3] ); 
        virtual int     GetNumKSpacePoints(); 
        void            GetVoxelSpacing( double voxelSpacing[3] ); 
        void            GetFOV( float fov[3] ); 
        int             GetNumCoils(); 
        int             GetNumFrames(); 
        virtual int     GetNumTimePoints(); 
        void            GetDcos( double dcos[3][3] ); 
        float           GetPPMRef(); 
        bool            IsSwapOn(); 
        bool            Is2D(); 
        bool            IsChopOn(); 
        void            GetXYZIndices(int index0, int index1, int index2, int* x, int* y, int* z); 
        void            UpdateProgress(double amount);
        virtual void    ModifyBehavior( svkImageData* data ); 
        void            RedimensionModifiedSVData( svkImageData* data ); 
        virtual int     GetNumberUnsuppressedAcquisitions(); 
        virtual int     GetNumberSuppressedAcquisitions(); 
        virtual void    GetSelBoxCenter( float selBoxCenter[3] ); 

        void            SetCellSpectrum( 
                            vtkImageData* data, 
                            bool wasSampled, 
                            int offset, 
                            int index, 
                            int x, int y, int z, 
                            int timePoint = 0, 
                            int channel = 0
                        );

        int             GetHeaderValueAsInt(string key);
        float           GetHeaderValueAsFloat(string key);
        string          GetHeaderValueAsString(string key);
        virtual bool    WasIndexSampled(int xIndex, int yIndex, int zIndex); 
        string          ConvertGEDateToDICOM( string geDate ); 


        string                                  progressText;
        vtkCallbackCommand*                     progressCallback;
        map <string, vector< string > >         pfMap;
        svkDcmHeader*                           dcmHeader; 
        float                                   pfileVersion; 
        bool                                    swapBytes; 
        int*                                    specData; 
        svkDcmHeader::DcmDataOrderingDirection  dataSliceOrder;
        int                                     chopVal; 
        MapperBehavior                          behaviorFlag;

};


}   //svk

#endif //SVK_GE_PFILE_MAPPER_H

