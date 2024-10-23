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


#ifndef SVK_IMAGE_WRITER_FACTORY_H
#define SVK_IMAGE_WRITER_FACTORY_H


#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkImageWriter.h>
#include <svkImageData.h>


namespace svk {


using namespace std;


/*! 
 *  Factory Base class.  
 *  Generates both vtk image writers ( JPEG, TIFF, etc.) and svk image writers. 
 *  many of the vtk image writers are for screen captures from a vtkRenderWindow.  
 *  The output from the vtkWindow2ImageFilter is used as input to the writer, for 
 *  writers that write out screen images. 
 */
class svkImageWriterFactory : public vtkObject
{

    public:

        static svkImageWriterFactory* New();
        vtkTypeMacro( svkImageWriterFactory, vtkObject);

        typedef enum {
            UNDEFINED = -1, 
            JPEG = 0, 
            TIFF, 
            DDF, 
            IDF, 
            DICOM_MRS, 
            DICOM_MRI, 
            DICOM_ENHANCED_MRI, 
            DICOM_SC, 
            PS, 
            LCMODEL,
            LAST_TYPE = PS
        } WriterType;


        //  Create a concrete reader based on the file extension 
        //  Returns the parent class of svkImageWriter so it can be used
        //  To generate vtkWriters as well
        virtual vtkImageWriter* CreateImageWriter( WriterType writerType ); 

        static WriterType  GetDefaultWriterForFilePattern(string newSeriesFilePattern );
        static int         GetNewSeriesFilePattern(svkImageData* imageData, string* newSeriesFilePattern );
        static int         GetNewSeriesNumberOffset();


    protected:

        svkImageWriterFactory();
        ~svkImageWriterFactory();


    private: 
        static int              seriesNumberOffset;  

};


}   //svk


#endif //SVK_IMAGE_WRITER_FACTORY_H


