/*
 *  Copyright © 2009 The Regents of the University of California.
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
 *  $URL: https://intrarad.ucsf.edu/svn/rad_software/surbeck/brain/sivic/trunk/libs/src/svkDdfVolumeWriter.h $
 *  $Rev: 15561 $
 *  $Author: jasonc@RADIOLOGY.UCSF.EDU $
 *  $Date: 2009-11-05 10:52:19 -0800 (Thu, 05 Nov 2009) $
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#ifndef SVK_DDF_VOLUME_WRITER_H
#define SVK_DDF_VOLUME_WRITER_H


#include <vtkErrorCode.h>
#include <vtkObjectFactory.h>
#include <vtkCellData.h>
#include <vtkImageAccumulate.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkExecutive.h>

#include <svkImageWriter.h>
#include <svkImageData.h>
#include <svkByteSwap.h>


namespace svk {


using namespace std;


/*! 
 *  Concrete writer instance for UCSF DDF image output.  
 */
class svkDdfVolumeWriter : public svkImageWriter
{

    public:

        static svkDdfVolumeWriter* New();
        vtkTypeRevisionMacro( svkDdfVolumeWriter, svkImageWriter);

        //  Methods:
        //void            SetInput( vtkDataObject* input ); 
        //void            SetInput(int index, vtkDataObject* input); 
        vtkDataObject*  GetInput(int port);
        vtkDataObject*  GetInput() { return this->GetInput(0); };
        svkImageData*   GetImageDataInput(int port);
        virtual void    Write();


    protected:

        svkDdfVolumeWriter();
        ~svkDdfVolumeWriter();

        virtual int     FillInputPortInformation(int port, vtkInformation* info);


    private:
        void            InitImageData();
        void            WriteData();
        void            WriteHeader();
        void            GetDDFCenter(float center[3]);
        void            GetDDFOrientation(float orientation[6]);
        string          GetDDFPatientsName(string patientsName);

};


}   //svk


#endif //SVK_DDF_VOLUME_WRITER_H

