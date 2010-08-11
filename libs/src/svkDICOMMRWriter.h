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
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/libs/src/svkDICOMMRWriter.h $
 *  $Rev: 76 $
 *  $Author: jccrane $
 *  $Date: 2010-01-26 14:05:15 -0500 (Tue, 26 Jan 2010) $
 *
 *  Authors:
 *      Don C. Bigler, Ph.D.
 */


#ifndef SVK_DICOM_MRI_WRITER_H
#define SVK_DICOM_MRI_WRITER_H


#include <vtkInformation.h>

#include <svkImageWriter.h>
#include <svkImageData.h>


namespace svk {


/*! 
 *  Concrete writer instance for DICOM MRI SOP class.  
 */
class svkDICOMMRWriter : public svkImageWriter
{

    public:

        static svkDICOMMRWriter* New();
        vtkTypeRevisionMacro( svkDICOMMRWriter, svkImageWriter);

        //  Methods:
        void            SetInput( vtkDataObject* input );
        void            SetInput(int index, vtkDataObject* input);
        vtkDataObject*  GetInput(int port);
        svkImageData*   GetImageDataInput(int port);
        virtual void    Write();


    protected:

        svkDICOMMRWriter();
        ~svkDICOMMRWriter();

        virtual int     FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info );

    private:
        void            InitPixelData();


};


}   //svk 

#endif //SVK_DICOM_MRI_WRITER_H

