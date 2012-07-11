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

        vtkTypeRevisionMacro( svkDcmVolumeReader, svkImageReader2);

        static float                            GetFloatValAttribute7( vtkstd::vector< vtkstd::string > vec ); 
        static int                              GetIntValAttribute8( vtkstd::vector< vtkstd::string > vec ); 



    protected:

        svkDcmVolumeReader();
        ~svkDcmVolumeReader();

        virtual void                            ExecuteInformation();
        virtual void                            ExecuteData(vtkDataObject* output); 
        bool                                    ContainsProprietaryContent( svkImageData* data );
        void                                    InitFileNames(); 
        void                                    OnlyReadInputFile(); 
        void                                    SortFilesByImagePositionPatient(
                                                    vtkstd::vector< vtkstd::vector< vtkstd::string> >& dcmSeriesAttributes,
                                                    bool ascending
                                                );
        void                                    SortFilesByInstanceNumber(
                                                    vtkstd::vector< vtkstd::vector< vtkstd::string> >& dcmSeriesAttributes,
                                                    int numSlicesPerVol, 
                                                    bool ascending
                                                ); 

        int                                     numFrames; 
        svkDcmHeader::DcmDataOrderingDirection  dataSliceOrder;
        virtual void                            InitDcmHeader();
        void                                    InitSliceOrder(); 
        void                                    InitSliceOrder(vtkstd::string hfileStart, vtkstd::string fileEnd); 
        virtual bool                            CheckForMultiVolume(); 
        int                                     numVolumes; 
        float                                   GetSliceSpacing(); 


    private: 

        virtual void                            LoadData(svkImageData* data) = 0; 
        virtual void                            InitPrivateHeader() = 0; 
        vtkStringArray*                         tmpFileNames;
        float                                   sliceSpacing; 
        void                                    SetSliceSpacing( 
                                                    svkDcmHeader* hdr, 
                                                    int numSlicesPerVol, 
                                                    vtkstd::vector< vtkstd::vector< vtkstd::string> >& dcmSeriesAttributes 
                                                );




};


}   //svk


#endif //SVK_DCM_VOLUME_READER_H

