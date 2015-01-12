/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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
 *  $URL: svn+ssh://jccrane@svn.code.sf.net/p/sivic/code/trunk/libs/src/svkF2C.cc $
 *  $Rev: 2120 $
 *  $Author: jccrane $
 *  $Date: 2014-12-19 13:13:43 -0800 (Fri, 19 Dec 2014) $
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */



#include <svkSatBandsXML.h>
#include <svkF2C.h>
#include <svkImageReaderFactory.h>

using namespace svk;


vtkCxxRevisionMacro(svkF2C, "$Rev: 2120 $");
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
int svkF2C::GetIDFHeader(char* idfFileName)
{
    cout << "+++++++++++++++++ " << endl;
    cout << "F2C: GetIDFHeader " << endl;
    cout << "+++++++++++++++++ " << endl << endl;;
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    readerFactory->Delete();
    cout << "created and delected factory" << endl;

    return 77; 

}


/*!
 *  Parse the input file and return an array of charater 
 *  arrays representing an IDF header. 
 */
void svkf2c_getidfheader_(char* idfFileName)
{
    printf("In fortran/c/c++ interface %s\n", idfFileName); 
    
    /*int status; 
    void* xml = svkSatBandsXML_New( idfFileName, &status );
    printf("In fortran/c/c++ interface : AFTER XML\n"); 
    */

    //svkF2C* f2c = svkF2C::New(); 
    int outVal = svkF2C::GetIDFHeader( idfFileName ); 
    printf("In fortran/c/c++ interface %d\n", outVal); 

    /*
    f2c->GetIDFHeader( idfFileName ); 
    f2c->Delete(); 
    */
}



