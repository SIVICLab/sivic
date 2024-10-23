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


#ifndef SVK_DICOM_SC_WRITER_H
#define SVK_DICOM_SC_WRITER_H


#include </usr/include/vtk/vtkErrorCode.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkImageData.h>
#include </usr/include/vtk/vtkImageLuminance.h>
#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkExecutive.h>

#include <svkImageWriter.h>
#include <svkImageData.h>
#include <svkIOD.h>
#include <svkSCIOD.h>
#include <string>

#include <sys/stat.h>


namespace svk {


using namespace std;


/*! 
 *  Concrete svkImageWriter instance for creating single frame DICOM Secondary 
 *  Capture SOP class output (SOPClassUID:  1.2.840.10008.5.1.4.1.1.7).  
 *  The DICOM SC header is generated from scratch with the following elements
 *  copied from the svkDcmHeader of the input svkImageData object:   PatientID, 
 *  StudyDate, StudyID, PatientName, StudyInstanceUID.  All instances have the 
 *  same "unique" SeriesInstanceUID, generated from scratch.  RGB output is the
 *  default.  Grayscale output may be specified, in which case the input 
 *  image is mapped via vtkImageLuminance to a gray scale output, with 
 *  PhotometricInterpretation set at MONOCHROME2 (highest intensity bright). 
 */
class svkDICOMSCWriter : public svkImageWriter
{

    public:

        static svkDICOMSCWriter* New();
        vtkTypeMacro( svkDICOMSCWriter, svkImageWriter);

        // Description:
        void            SetInput( vtkDataObject* input );
        void            SetInput(int index, vtkDataObject* input);
        vtkDataObject*  GetInput(int port);
        svkImageData*   GetImageDataInput(int port);
        virtual void    Write();

        //  Return the default file name pattern
        static string   GetFilePattern( svkImageData* imageData ); 
        void            CreateNewSeries();
        void            SetOutputToGrayscale( bool isOutputGray ); 

        //! Set to true if you want to use the instance number as the file number
        void            SetUseInstanceNumber( bool useInstanceNumber );


    protected:

        svkDICOMSCWriter();
        ~svkDICOMSCWriter();

        virtual int     FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info );


    private:

        void    WriteSlice();
        void    InitDcmHeader();

        //  Members:
        svkDcmHeader*   dcmHeader;
        svkDcmHeader*   dcmHeaderTemplate;
        bool            isGray; 
        bool            useInstanceNumber; 
        

};


}   //svk

#endif //SVK_DICOM_SC_WRITER_H

