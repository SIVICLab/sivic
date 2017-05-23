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


#ifndef SVK_DCM_VOLUME_READER_H
#define SVK_DCM_VOLUME_READER_H


#include <svkImageReader2.h>
#include <vector>
#include <algorithm>


namespace svk {


/*! 
 *  
 */

class svkDcmVolumeReader : public svkImageReader2 
{

    public:

        vtkTypeMacro( svkDcmVolumeReader, svkImageReader2);

        static float                            GetFloatValAttribute7( vector< string > vec ); 
        static int                              GetIntValAttribute8( vector< string > vec ); 
        static void                             GetVOILUTScaledPixels( float* floatPixels, unsigned short* shortPixels, float center, float window, int numberOfPixels );



    protected:

        svkDcmVolumeReader();
        ~svkDcmVolumeReader();

        typedef enum {
            DICOM_STD_SOP = 0,
            GE_POSTAGE_STAMP_SOP,
            BRUKER_MRS_SOP
        } ProprietarySOP;


        virtual void                            ExecuteInformation();
        virtual void                            ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo); 
        ProprietarySOP                          ContainsProprietaryContent( svkImageData* data );
        void                                    InitFileNames(); 
        void                                    OnlyReadInputFile(); 
        void                                    SortFilesByImagePositionPatient(
                                                    vector< vector< string> >& dcmSeriesAttributes,
                                                    bool ascending
                                                );
        void                                    SortFilesByInstanceNumber(
                                                    vector< vector< string> >& dcmSeriesAttributes,
                                                    int numSlicesPerVol, 
                                                    bool ascending
                                                ); 

        int                                     numFrames; 
        svkDcmHeader::DcmDataOrderingDirection  dataSliceOrder;
        virtual void                            InitDcmHeader();
        void                                    InitSliceOrder(); 
        void                                    InitSliceOrder(string hfileStart, string fileEnd); 
        virtual bool                            CheckForMultiVolume(); 
        int                                     numVolumes; 
        float                                   GetSliceSpacing(); 
        virtual void                            CleanAttributes( set < string >* uniqueSlices){ return; }; 


    private: 

        virtual void                            LoadData(svkImageData* data) = 0; 
        virtual void                            InitPrivateHeader() = 0; 
        vtkStringArray*                         tmpFileNames;
        float                                   sliceSpacing; 
        void                                    SetSliceSpacing( 
                                                    svkDcmHeader* hdr, 
                                                    int numSlices, 
                                                    vector< vector< string> >& dcmSeriesAttributes 
                                                );




};


}   //svk


#endif //SVK_DCM_VOLUME_READER_H

