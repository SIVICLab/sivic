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


#ifndef SVK_DDF_VOLUME_WRITER_H
#define SVK_DDF_VOLUME_WRITER_H

#include </usr/include/vtk/vtkInformation.h>

#include <svkImageWriter.h>
#include <svkImageData.h>



namespace svk {


/*! 
 *  Concrete writer instance for UCSF DDF image output.  
 */
class svkDdfVolumeWriter : public svkImageWriter
{

    public:

        static svkDdfVolumeWriter* New();
        vtkTypeMacro( svkDdfVolumeWriter, svkImageWriter);

        //  Methods:
        vtkDataObject*  GetInput(int port);
        vtkDataObject*  GetInput() { return this->GetInput(0); };
        svkImageData*   GetImageDataInput(int port);
        void            SetOneTimePointsPerFile();
        int             GetNumberTimePointsPerFile(); 
        virtual void    Write();
        void            UseDescriptiveFileNames();


    protected:

        svkDdfVolumeWriter();
        ~svkDdfVolumeWriter();

        virtual int     FillInputPortInformation(int port, vtkInformation* info);
        virtual void    SetProvenance();


    private:
        void             InitImageData();
        void             WriteFiles();
        void             GetDDFCenter(float center[3], std::string centerType = "current" );
        void             GetDDFOrientation(float orientation[6]);
        std::string   GetDimensionDomain( std::string dimensionDomainString ); 
        std::string   GetDDFPatientName(std::string PatientName);
        std::string   GetFileRootName(std::string fileRoot, svkDcmHeader::DimensionVector* dimensionVector, int frame); 
        void             SetNumberTimePointsPerFile(int numTimePts);
        bool             AllTimePointsInEachFile(); 
        void             InitSpecData(float* specData, svkDcmHeader::DimensionVector* dimensionVector, svkDcmHeader::DimensionVector* indexVector); 
        void             InitHeader(ofstream* out, std::string fileName);

        int              numTimePtsPerFile; 
        static const int ALL_TIME_PTS_IN_FILE; 
        bool             useDescriptiveFileNames;

};


}   //svk


#endif //SVK_DDF_VOLUME_WRITER_H

