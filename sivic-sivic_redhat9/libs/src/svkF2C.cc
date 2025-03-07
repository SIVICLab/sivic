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



#include <svkSatBandsXML.h>
#include <svkF2C.h>
#include <svkImageReaderFactory.h>

using namespace svk;


//vtkCxxRevisionMacro(svkF2C, "$Rev$");
vtkStandardNewMacro(svkF2C);


/*!
 *
 */
svkF2C::svkF2C()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif
    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

}


/*!
 *
 */
svkF2C::~svkF2C()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}


/*!
 *  set the path/name to xml file.   
 */
int svkF2C::GetIDFHeader(char* fileRootName, char* headerString)
{
    // We need to get rid of training spaces and add the null terminator character
    int str_length = strcspn(fileRootName, " ");
    strncpy(&fileRootName[str_length], "\0", 1);
    string stringFileName = string(fileRootName);
    cout << "+++++++++++++++++ " << endl;
    cout << "F2C: GetIDFHeader " << endl;
    cout << "+++++++++++++++++ " << endl << endl;;

    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();

    // First check to see if its a file without an extension
    svkImageReader2* reader = NULL;
    if( svkUtils::FilePathExists(stringFileName.c_str())) {
        reader = readerFactory->CreateImageReader2( stringFileName.c_str() );
    } else {
        // If the file does not exist check for files with an extension
        vtkGlobFileNames* glob = vtkGlobFileNames::New();
        stringFileName.append("*");
        glob->AddFileNames(stringFileName.c_str());
        for( int i = 0; i < glob->GetNumberOfFileNames(); i++ ) {
            reader = readerFactory->CreateImageReader2( glob->GetNthFileName(i) );
            if( reader != NULL ) {
                // We have found the correct extension. Break out of loop.
                stringFileName = glob->GetNthFileName(i);
                break;
            }
        }
        glob->Delete();
    }
    reader->SetFileName( stringFileName.c_str() );
    reader->Update();
    svkIdfVolumeWriter* writer = svkIdfVolumeWriter::New();
    writer->SetInputData(reader->GetOutput());
    writer->SetFileName( stringFileName.c_str() );
    string header = writer->GetHeaderString();
    for( int i = 0; i < header.length(); i++ ) {
        headerString[i] = header.c_str()[i];
    }
    readerFactory->Delete();
    writer->Delete();

    return 77; 

}


/*!
 *  Parse the input file and return an array of charater 
 *  arrays representing an IDF header. 
 */
void svkf2c_getidfheader_(char* fileRootName, char* headerString)
{
    printf("In fortran/c/c++ interface %s\n", fileRootName);
    
    int outVal = svkF2C::GetIDFHeader( fileRootName, headerString );
}



