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



#include <svkGenericAlgorithmWithPortMapper.h>


using namespace svk;

//vtkCxxRevisionMacro(svkGenericAlgorithmWithPortMapper, "$Rev$");


/*!
 *
 */
svkGenericAlgorithmWithPortMapper::svkGenericAlgorithmWithPortMapper()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif
    this->portMapper = NULL;

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");
    vtkInstantiator::RegisterInstantiator("svkXML",  svkXML::NewObject);

}


/*!
 *
 */
svkGenericAlgorithmWithPortMapper::~svkGenericAlgorithmWithPortMapper()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~svkGenericAlgorithmWithPortMapper()");
    if( this->portMapper != NULL ) {
        this->portMapper->Delete();
        this->portMapper = NULL;
    }
}


/*!
 * Pass through method to the internal svkAlgorithmPortMapper
 */
void svkGenericAlgorithmWithPortMapper::SetInputPortsFromXML( vtkXMLDataElement* element )
{
    this->GetPortMapper()->SetInputPortsFromXML(element);
}


/*!
 * Returns the port mapper. Performs lazy initialization.
 */
svkAlgorithmPortMapper* svkGenericAlgorithmWithPortMapper::GetPortMapper()
{
    if( this->portMapper == NULL ) {
        this->portMapper = svkAlgorithmPortMapper::New();
        this->portMapper->SetAlgorithm( this );
    }
    return this->portMapper;
}


/*!
 *  PrintSelf method calls parent class PrintSelf, then prints all parameters using the port mapper.
 *
 */
void svkGenericAlgorithmWithPortMapper::PrintSelf( ostream &os, vtkIndent indent )
{
    Superclass::PrintSelf( os, indent );
    this->GetPortMapper()->PrintSelf( os, indent );
}


/*!
 *  This method forwards data request to the RequestData method.
 */
int svkGenericAlgorithmWithPortMapper::ProcessRequest(vtkInformation* request,
                                      vtkInformationVector** inputVector,
                                      vtkInformationVector* outputVector)
{
    // generate the data
    if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA())) {
        return this->RequestData(request, inputVector, outputVector);
    }
    return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}


/*!
 * Pass through method to the internal svkAlgorithmPortMapper
 */
int svkGenericAlgorithmWithPortMapper::FillInputPortInformation( int port, vtkInformation* info )
{
    this->GetPortMapper()->FillInputPortInformation(port, info );

    return 1;
}


/*!
 * Pass through method to the internal svkAlgorithmPortMapper
 */
int svkGenericAlgorithmWithPortMapper::FillOutputPortInformation( int port, vtkInformation* info )
{
    this->GetPortMapper()->FillOutputPortInformation(port, info );

    return 1;
}
