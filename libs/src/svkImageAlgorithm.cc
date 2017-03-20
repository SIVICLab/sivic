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


#include <svkImageAlgorithm.h>


using namespace svk;


//vtkCxxRevisionMacro(svkImageAlgorithm, "$Rev$");


/*!
 *
 */
svkImageAlgorithm::svkImageAlgorithm()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    vtkInstantiator::RegisterInstantiator("svkMriImageData", svkMriImageData::NewObject);
    vtkInstantiator::RegisterInstantiator("svkMrsImageData", svkMrsImageData::NewObject);
    vtkInstantiator::RegisterInstantiator("svk4DImageData",  svk4DImageData::NewObject);
}


/*!
 *
 */
svkImageAlgorithm::~svkImageAlgorithm()
{
}


/*!
 *  
 */
svkImageData* svkImageAlgorithm::GetOutput()
{
    return this->GetOutput(0); 
}


/*!
 *  
 */
svkImageData* svkImageAlgorithm::GetOutput(int port)
{
    return svkImageData::SafeDownCast(this->GetOutputDataObject(port));
}


/*!
 *
 */
svkImageData* svkImageAlgorithm::GetImageDataInput(int port)
{
    return svkImageData::SafeDownCast( this->GetInput(port) );
}


/*!
 *  Default output type is same concrete sub class type as the input data.  Override with 
 *  specific concrete type in sub-class if necessary.  
 */
int svkImageAlgorithm::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    if( this->GetImageDataInput(0) != NULL ) {
        info->Set( vtkDataObject::DATA_TYPE_NAME(), this->GetImageDataInput(0)->GetClassName() );
    } else {
        cout << "ERROR: Input was not set to algorithm so output port information is undefined!" << endl;
    }
    return 1;
}


/*!
 *  Default input type is svkImageData base class. Override with a specific concrete type in 
 *  sub class if necessary. 
 */
int svkImageAlgorithm::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkImageData");
    return 1;
}

