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

#ifndef SVK_VARIAN_FID_MAPPER_H
#define SVK_VARIAN_FID_MAPPER_H


#include </usr/include/vtk/vtkImageData.h>
#include <svkImageData.h>

#include <svkDcmHeader.h>
#include <svkMRSIOD.h>

#include <map>
#include <string>
#include <vector>


namespace svk {


/*! 
 *  Mapper base class for converting from Varian FID Procpar header format to DICOM MR Spectrosocpy IOD/SOP 
 *  Class instance.  The mapper receives the procpar fields from the svkVarianFidReader.  The procpar fields are 
 *  in the form of a map of of key value pairs (procparMap). 
 *  Map values are vectors of strings and can be accessed by key string name and value index (GetHeaderValueAsType 
 *  methods).  
 *
 *  Concrete mappers need to be implemented to map sequence specific content from Varian acquisitions to DICOM MR
 *  Spectroscopy.  It is the svkVarianFidReader's responsibility to select the appropriate svkVarianFidMapper 
 *  instance for any give data set. 
 *  
 *  Thanks to Dr. Sukumar Subramaniam (UCSF NMR Lab) for help understanding the Varian data format and for his 
 *  assistance validating SIVIC's Varian data reading functionality.  
 */
class svkVarianFidMapper : public vtkObject 
{

    public:

        vtkTypeMacro( svkVarianFidMapper, vtkObject );

        virtual void    InitializeDcmHeader(
                            map <string, vector < vector<string> > >    procparMap,
                            svkDcmHeader* header,
                            svkMRSIOD* iod,
                            int        swapBytes  
                        );

        virtual void    ReadFidFile( string fidFileName, svkImageData* data );

        
    protected:

        svkVarianFidMapper();
        ~svkVarianFidMapper();
  
        void            InitPatientModule();
        void            InitGeneralStudyModule();
        void            InitGeneralSeriesModule();
        void            InitGeneralEquipmentModule();
        virtual void    InitMultiFrameFunctionalGroupsModule();
        virtual void    InitMultiFrameDimensionModule();
        virtual void    InitAcquisitionContextModule();
        virtual void    InitSharedFunctionalGroupMacros();
        virtual void    InitPerFrameFunctionalGroupMacros();
        virtual void    InitMRModifierMacro(); 
        virtual void    InitMRTransmitCoilMacro(); 
        virtual void    InitMRAveragesMacro(); 
        virtual void    InitPixelMeasuresMacro();
        virtual void    InitPlaneOrientationMacro();
        virtual void    InitMREchoMacro();
        virtual void    InitMRTimingAndRelatedParametersMacro();
        virtual void    InitMRReceiveCoilMacro();
        virtual void    InitMRSpectroscopyPulseSequenceModule() = 0;
        virtual void    InitMRSpectroscopyModule();
        virtual void    InitMRSpectroscopyFOVGeometryMacro();
        virtual void    InitMRSpectroscopyDataModule();
        string          GetDcmPatientPositionString(); 

        virtual void    ConvertCmToMm();

        int             GetHeaderValueAsInt(
                            string keyString, int valueIndex = 0, int procparRow = 0
                        );
        float           GetHeaderValueAsFloat(
                            string keyString, int valueIndex = 0, int procparRow = 0
                        );
        string          GetHeaderValueAsString(
                            string keyString, int valueIndex = 0, int procparRow = 0
                        );
        bool            HeaderFieldExists( string keyString ); 

        virtual void    SetCellSpectrum(
                            vtkImageData* data, 
                            int x, int y, int z, 
                            int timePt, 
                            int coilNum
                        );



        map <string, vector < vector<string> > >    
                                                    procparMap;
        svkDcmHeader*                               dcmHeader; 
        float*                                      specData; 
        svkDcmHeader::DcmDataOrderingDirection      dataSliceOrder;
        int                                         numSlices; 
        int                                         numFrames; 
        svkMRSIOD*                                  iod;
        int                                         swapBytes; 

};


}   //svk

#endif //SVK_VARIAN_FID_MAPPER_H

