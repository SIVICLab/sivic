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
 *      Don C. Bigler, Ph.D.
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#ifndef SVK_DICOM_IMAGE_WRITER_H
#define SVK_DICOM_IMAGE_WRITER_H


#include </usr/include/vtk/vtkInformation.h>

#include <svkDICOMWriter.h>
#include <svkImageData.h>
#include <svkUtils.h>


namespace svk {


/*! 
 *  Base Class for DICOM MRI writers. 
 */
class svkDICOMImageWriter : public svkDICOMWriter
{

    public:

        vtkTypeMacro( svkDICOMImageWriter, svkDICOMWriter);

        //  Methods:
        void            UseLosslessCompression(); 

    protected:

        svkDICOMImageWriter();
        ~svkDICOMImageWriter();

        virtual int     FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info );
        void            GetShortScaledPixels( unsigned short* shortPixels, double& slope, double& intercept, int sliceNumber, int volNumber );
        void            GetScaledPixels( unsigned short* shortPixels, double slope, double intercept, int sliceNumber, int volNumber );
        void            GetPixelRange(double& min, double& max, int volNumber); 
        virtual int     GetDataLength() = 0;
        bool            useLosslessCompression; 
        
};


}   //svk 

#endif //SVK_DICOM_IMAGE_WRITER_H

