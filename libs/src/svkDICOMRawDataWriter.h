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


#ifndef SVK_DICOM_RAW_DATARITER_H
#define SVK_DICOM_RAW_DATARITER_H


#include <vtkErrorCode.h>
#include <vtkObjectFactory.h>
#include <vtkImageLuminance.h>
#include <vtkInformation.h>
#include <vtkExecutive.h>
#include <vtkstd/vector>

#include <svkImageWriter.h>

#include <sys/stat.h>
#include <time.h>


namespace svk {


using namespace std;


/*! 
 *  Concrete svkImageWriter instance for creating a Raw Data SOP instance. 
 *  Used within svk primarily for encapsulating a non DICOM raw file (e.g. 
 *  a GE PFile) into a DICOM object such that it may managed via DICOM C-STORE,
 *  C-GET, etc. Encapsulates a raw file as well as any associated files into 
 *  a single object. 
 */
class svkDICOMRawDataWriter : public svkImageWriter
{

    public:

        static svkDICOMRawDataWriter* New();
        vtkTypeRevisionMacro( svkDICOMRawDataWriter, svkImageWriter);

        void            SetSHA1Digest( string sha1Digest);
        void            AddAssociatedFile( string fileName, string sha1Digest ); 
        void            ReuseSeriesUID( bool reuseUID ); 
        void            SetSeriesUID( vtkstd::string UID ); 
        virtual void    Write();



    protected:

        svkDICOMRawDataWriter();
        ~svkDICOMRawDataWriter();

        virtual int     FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info );


    private:

        void            InitDcmHeader();
        void            InitPatientModule(); 
        void            InitGeneralStudyModule(); 
        void            InitGeneralSeriesModule(); 
        void            InitGeneralEquipmentModule(); 
        void            InitRawDataModule(); 
        int             GetHeaderValueAsInt(vtkstd::string key); 

        //  Members:
        svkDcmHeader*                   dcmHeader;
        string                          sha1Digest; 
        vector < vector <string > >     associatedFiles; 
        int                             computedPFileSize; 
        vtkstd::map <vtkstd::string, vtkstd::vector< vtkstd::string > >     pfMap;
        bool                            reuseSeriesUID; 
        vtkstd::string                  seriesInstanceUID; 
};


}   //svk

#endif //SVK_DICOM_RAW_DATARITER_H

