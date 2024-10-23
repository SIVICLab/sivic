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


#ifndef SVK_LCMODEL_READER_H
#define SVK_LCMODEL_READER_H

#include </usr/include/vtk/vtkInformation.h>
#include <map>
#include </usr/include/vtk/vtkStringArray.h>
#include <string>

#include <svkImageReader2.h>


namespace svk {


/*! 
 *  
 */
class svkLCModelReader : public svkImageReader2 
{

    public:

        vtkTypeMacro( svkLCModelReader, svkImageReader2);

        // Description: 
        // A descriptive name for this format
        virtual const char* GetDescriptiveName() {
            return "LCModel Reader";
        }

        //  Methods:
        void                    SetMRSFileName( string mrsFileName );
        void                    SetMetName( string metName );

    protected:

        svkLCModelReader();
        ~svkLCModelReader();

        virtual int                                 FillInputPortInformation( int port, vtkInformation* info ); 
        virtual void                                ExecuteInformation();
        virtual svkDcmHeader::DcmPixelDataFormat    GetFileType();
        string                                      GetSeriesDescription();
        void                                        GetVoxelIndexFromFileName(string lcmodelFileName, int* col, int* row, int* slice); 
        string                                      metName;
        virtual string                              GetFileSeriesDescription( string fileName ); 



    private:

        //  Methods:
        virtual void                                InitDcmHeader();

        //  Members:
        string                                      mrsFileName;

};


}   //svk


#endif //SVK_LCMODEL_READER_H

