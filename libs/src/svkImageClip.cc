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


#include <svkImageClip.h>


using namespace svk;


//vtkCxxRevisionMacro(svkImageClip, "$Rev$");
vtkStandardNewMacro(svkImageClip);


//! Constructor
svkImageClip::svkImageClip()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif
    vtkInstantiator::RegisterInstantiator("svkMriImageData", svkMriImageData::NewObject);
}


//! Destructor
svkImageClip::~svkImageClip()
{
}


/*!
 *  We just need to make sure the dcos is copied here.
 */
int svkImageClip::RequestData(
  vtkInformation *request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
    double dcos[3][3];

    if( this->GetInput() != NULL ) {
        svkImageData::SafeDownCast(this->GetInput())->GetDcos(dcos); 
        svkImageData::SafeDownCast(this->GetOutputDataObject(0))->SetDcos(dcos); 
        svkImageData::SafeDownCast(this->GetOutputDataObject(0))->SetDcmHeader( svkImageData::SafeDownCast(this->GetInput())->GetDcmHeader());
    }

    return vtkImageClip::RequestData( request, inputVector, outputVector);
}


/*!
 *  Default output type is same concrete sub class type as the input data.  Override with
 *  specific concrete type in sub-class if necessary.
 */
int svkImageClip::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData" );
    return 1;
}


/*!
 *
 */
svkImageData* svkImageClip::GetImageDataInput(int port)
{
    return svkImageData::SafeDownCast( this->GetInput(port) );
}

