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
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/libs/src/svkGEPFileReader.h $
 *  $Rev: 1 $
 *  $Author: jccrane $
 *  $Date: 2009-11-25 02:04:25 -0800 (Wed, 25 Nov 2009) $
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */

#ifndef SVK_GE_PFILE_READER_H
#define SVK_GE_PFILE_READER_H


#include <vtkObjectFactory.h>
#include <vtkGlobFileNames.h>
#include <vtkSortFileNames.h>
#include <vtkStringArray.h>

#include <svkImageReader2.h>
#include <svkIOD.h>
#include <svkMRSIOD.h>
#include <svkByteSwap.h>
#include <svkGEPFileMapper.h>

#include <sys/stat.h>
#include <map>


namespace svk {


using namespace std;


/*! 
 *  Reader for GE P-files 
 */
class svkGEPFileReader : public svkImageReader2 
{

    public:

        vtkTypeRevisionMacro( svkGEPFileReader, svkImageReader2 );
        static              svkGEPFileReader* New();
        virtual int         CanReadFile( const char* fname );


    protected:

        svkGEPFileReader();
        ~svkGEPFileReader();
  
        virtual int                              FillOutputPortInformation(int port, vtkInformation* info);
        virtual void                             ExecuteInformation();
        virtual void                             ExecuteData( vtkDataObject *output );
        virtual svkDcmHeader::DcmPixelDataFormat GetFileType();
        void                                     ReadGEPFile();
        void                                     ParsePFile();
        void                                     InitOffsetsMap( float pfileVersion );

        map <string, vector< string > >          pfMap;


    private:

        //  Methods:
        string              GetOffsetsString( float pfileVersion );
        virtual void        InitDcmHeader();
        void                PrintOffsets(); 
        void                PrintKeyValuePairs();
        string              GetFieldAsString(string key); 
        int                 GEUncompressUID(unsigned char* short_uid, char* long_uid); 
        string              UncompressUID(const char* compressedUID); 
        float               GetPFileVersion(); 

        //  Members:
        ifstream*           gepf;
        svkGEPFileMapper*   mapper;
        float               pfileVersion;

};


}   //svk

#endif //SVK_GE_PFILE_READER_H

