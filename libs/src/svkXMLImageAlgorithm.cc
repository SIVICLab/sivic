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
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */



#include <svkXMLImageAlgorithm.h>


using namespace svk;

vtkCxxRevisionMacro(svkXMLImageAlgorithm, "$Rev$");
vtkStandardNewMacro(svkXMLImageAlgorithm);

/*!
 *
 */
svkXMLImageAlgorithm::svkXMLImageAlgorithm()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif
    this->xmlInterpreter = svkXMLInputInterpreter::New();
    this->xmlInterpreter->SetAlgorithm( this );

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

}


/*!
 *
 */
svkXMLImageAlgorithm::~svkXMLImageAlgorithm()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~svkXMLImageAlgorithm()");
    if( this->xmlInterpreter != NULL ) {
        this->xmlInterpreter->Delete();
    }
}


/*!
 * Pass through method to the internal svkXMLInputInterpreter
 */
void svkXMLImageAlgorithm::SetInputPortsFromXML( vtkXMLDataElement* element )
{
    this->xmlInterpreter->SetInputPortsFromXML(element);
}


/*!
 * Returns the interpreter.
 */
svkXMLInputInterpreter* svkXMLImageAlgorithm::GetXMLInterpreter()
{
    return this->xmlInterpreter;
}


/*!
 *  PrintSelf method calls parent class PrintSelf, then prints all parameters using the interpreter.
 *
 */
void svkXMLImageAlgorithm::PrintSelf( ostream &os, vtkIndent indent )
{
    Superclass::PrintSelf( os, indent );
    this->xmlInterpreter->PrintSelf( os, indent );
}


/*!
 * Pass through method to the internal svkXMLInputInterpreter
 */
int svkXMLImageAlgorithm::FillInputPortInformation( int port, vtkInformation* info )
{
    this->Superclass::FillInputPortInformation( port, info );
    this->xmlInterpreter->FillInputPortInformation(port, info );

    return 1;
}
