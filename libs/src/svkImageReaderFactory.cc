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
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#include <svkImageReaderFactory.h>


using namespace svk;


vtkCxxRevisionMacro(svkImageReaderFactory, "$Rev$");
vtkStandardNewMacro(svkImageReaderFactory);


/*!
 *
 */
svkImageReaderFactory::svkImageReaderFactory() 
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    this->dcmMriVolReader = svkDcmMriVolumeReader::New();
    this->dcmMrsVolReader = svkDcmMrsVolumeReader::New();
    this->idfVolReader    = svkIdfVolumeReader::New();
    this->ddfVolReader    = svkDdfVolumeReader::New();
    this->fdfVolReader    = svkFdfVolumeReader::New();
    this->sdbmVolReader    = svkSdbmVolumeReader::New();

    vtkImageReader2Factory::RegisterReader( this->dcmMriVolReader );
    vtkImageReader2Factory::RegisterReader( this->dcmMrsVolReader );
    vtkImageReader2Factory::RegisterReader( this->idfVolReader );
    vtkImageReader2Factory::RegisterReader( this->ddfVolReader );
    vtkImageReader2Factory::RegisterReader( this->fdfVolReader );
    vtkImageReader2Factory::RegisterReader( this->sdbmVolReader );

}


/*!
 *
 */
svkImageReaderFactory::~svkImageReaderFactory()
{
    vtkDebugMacro(<<"svkImageReaderFactory::~svkImageReaderFactory()");
    

    if (this->idfVolReader != NULL) {
        this->idfVolReader->Delete();
        this->idfVolReader = NULL;
    }

    if (this->ddfVolReader != NULL) {
        this->ddfVolReader->Delete();
        this->ddfVolReader = NULL;
    }

    if (this->dcmMriVolReader != NULL) {
        this->dcmMriVolReader->Delete();
        this->dcmMriVolReader = NULL;
    }

    if (this->dcmMrsVolReader != NULL) {
        this->dcmMrsVolReader->Delete();
        this->dcmMrsVolReader = NULL;
    }

    if (this->fdfVolReader != NULL) {
        this->fdfVolReader->Delete();
        this->fdfVolReader = NULL;
    }

    if (this->sdbmVolReader != NULL) {
        this->sdbmVolReader->Delete();
        this->sdbmVolReader = NULL;
    }

}


/*!
 *
 */
svkImageReader2* svkImageReaderFactory::CreateImageReader2(const char* path)
{
    return static_cast<svkImageReader2*>( this->Superclass::CreateImageReader2( path ) );
}
