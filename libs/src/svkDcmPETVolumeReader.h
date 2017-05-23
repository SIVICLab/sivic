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


#ifndef SVK_DCM_PET_VOLUME_READER_H
#define SVK_DCM_PET_VOLUME_READER_H


#include <svkDcmMriVolumeReader.h>
#include <vector>
#include <string>


namespace svk {


/*! 
 *  Reads a DICOM PET file and initializes a DICOM Enhanced MRI file with BW SUV weighted pixel values.
 *  In memory the data is represented as a DICOM E MRI instance. 
 *
 *  Read the pixels and scale to PET SUV (bw scaled).
 *  Use vendor agnostic approach from QIBA website:
 *  http://qibawiki.rsna.org/index.php/Standardized_Uptake_Value_(SUV)
 *  http://qibawiki.rsna.org/images/6/60/SUV_vendorneutral_pseudocode_happypathonly_20121015_DAC.doc
 */
class svkDcmPETVolumeReader : public svkDcmMriVolumeReader
{

    public:

        static svkDcmPETVolumeReader* New();
        vtkTypeMacro( svkDcmPETVolumeReader, svkDcmMriVolumeReader );

        // Description: 
        // A descriptive name for this format
        virtual const char* GetDescriptiveName() {
            return "DICOM PET File";
        }

        virtual svkImageReader2::ReaderType GetReaderType()
        {
            return svkImageReader2::DICOM_PET;
        }

        //  Methods:
        virtual int CanReadFile(const char* fname);

    protected:

        svkDcmPETVolumeReader();
        ~svkDcmPETVolumeReader();
        void                                        GetSUVScaledPixels( 
                                                        float* floatPixels, 
                                                        unsigned short* shortPixels, 
                                                        int numberOfPixels,
                                                        vector < float> rescaleSlopeVector );
        virtual svkDcmHeader::DcmPixelDataFormat    GetFileType();

    private:

        virtual void    LoadData(svkImageData* data); 

};


}   //svk


#endif //SVK_DCM_PET_VOLUME_READER_H

