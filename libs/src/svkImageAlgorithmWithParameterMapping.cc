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



#include <svkImageAlgorithmWithParameterMapping.h>


using namespace svk;


/*!
 *
 */
svkImageAlgorithmWithParameterMapping::svkImageAlgorithmWithParameterMapping()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    this->firstParameterPort = 0;
    this->numberOfParameters = 0;
    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

}


/*!
 *
 */
svkImageAlgorithmWithParameterMapping::~svkImageAlgorithmWithParameterMapping()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~svkImageAlgorithmWithParameterMapping()");
}


/*!
 * Fills the input port information for all the input parameter ports.
 */
int svkImageAlgorithmWithParameterMapping::FillInputPortInformation( int port, vtkInformation* info )
{
    this->Superclass::FillInputPortInformation( port, info );

    if ( port >= this->firstParameterPort && port < this->firstParameterPort + this->numberOfParameters ) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
    }

    return 1;
}


/*!
 * Sets the number of input/parameter ports and calls SetupParameterPorts.
 */
void svkImageAlgorithmWithParameterMapping::SetNumberOfInputAndParameterPorts(int numberOfInputPorts, int numberOfParameters )
{
    this->SetNumberOfInputPorts( numberOfInputPorts + numberOfParameters );
    this->firstParameterPort = numberOfInputPorts;
    this->numberOfParameters = numberOfParameters;
    this->SetupParameterPorts();
}


/*!
 * Extracts input port parameters from an XML element and sets them.
 */
void svkImageAlgorithmWithParameterMapping::SetParametersFromXML( vtkXMLDataElement* element )
{
    vtkIndent indent;
    if( element != NULL ) {
        for( int i = this->firstParameterPort; i < this->firstParameterPort + this->numberOfParameters; i++ ) {
            string parameterStringValue = string(element->FindNestedElementWithName(this->GetParameterName(i).c_str())->GetCharacterData());
            int dataType = this->GetParameterPortType( i );
            if( dataType == VTK_DOUBLE ) {
                this->SetDoubleParameter(i, svkUtils::StringToDouble( parameterStringValue ));
            } else if ( dataType == VTK_INT) {
                this->SetIntParameter(i, svkUtils::StringToInt( parameterStringValue ));
            } else if ( dataType == VTK_CHAR) {
                this->SetStringParameter(i, parameterStringValue );
            }
        }
    }

}


/*!
 * Returns the name of the given parameter port.
 */
string svkImageAlgorithmWithParameterMapping::GetParameterName( int port )
{
    string parameterName;
    if( port >= this->firstParameterPort && port < this->firstParameterPort + this->numberOfParameters ) {
        parameterName = this->parameterNames[port];
    } else {
        cout << "ERROR: port " << port << " is not an input parameter port!" << endl;
    }
    return parameterName;
}


/*!
 * Returns the port number for the given parameter name. Returns -1 if the port does not exist.
 */
int svkImageAlgorithmWithParameterMapping::GetParameterPort( string name )
{
    int port = -1;
    for( int i = this->firstParameterPort; i < this->firstParameterPort + this->numberOfParameters; i++ ) {
        if( name == this->parameterNames[i]) {
            port = i;
        }
    }
    if( port == -1 ) {
        cout << "ERROR: No input parameter port named " << name << " found!" << endl;
    }
    return port;
}


/*!
 *  This method initializes a given input port parameter.
 */
void svkImageAlgorithmWithParameterMapping::InitializeParameterPort( int port, string name, int type )
{
    // Only initialize a given input port parameter once.
    if( this->GetInput( port ) == NULL ) {

        // Make sure the parameter name array is large enough to hold the new name
        while ( this->parameterNames.size() < port + 1 ) {
           this->parameterNames.push_back("");
        }
        this->parameterNames[port] = name;

        vtkDataArray* array = vtkDataArray::CreateDataArray( type );
        array->SetNumberOfComponents(1);
        array->SetNumberOfTuples(1);

        // Create a vtkDataObject to hold the Field Data which will hold the vtkDataArray
        vtkDataObject* parameterDataObject = vtkDataObject::New();
        vtkFieldData* parameterFieldData = vtkFieldData::New();
        parameterDataObject->SetFieldData( parameterFieldData );

        // Add the array to the field data
        parameterFieldData->AddArray( array );
        this->SetInput(port, parameterDataObject );

        array->Delete();
        parameterFieldData->Delete();
        parameterDataObject->Delete();
    }
}


/*!
 * Returns the type of a given input port parameter.
 */
int svkImageAlgorithmWithParameterMapping::GetParameterPortType( int port )
{
    vtkDataObject* parameterObject = this->GetInput( port );
    if( parameterObject != NULL ) {
        this->GetInput( port )->GetFieldData()->GetArray(0)->GetDataType();
    } else {
        return -1;
    }
}


/*!
 * Parameter port setter.
 */
void svkImageAlgorithmWithParameterMapping::SetDoubleParameter( int port, double value )
{
    if( this->GetParameterPortType(port) == VTK_DOUBLE ) {
        vtkDataObject* array =  this->GetInput( port );
        vtkDoubleArray::SafeDownCast( array->GetFieldData()->GetArray(0) )->SetTuple1(0, value);
    } else {
        cerr << "ERROR: Input parameter port type mismatch! Port " << port << " is not of type double. " << endl;
    }
}


/*!
 * Parameter port getter.
 */
double svkImageAlgorithmWithParameterMapping::GetDoubleParameter( int port )
{
    if( this->GetParameterPortType(port) == VTK_DOUBLE ) {
        vtkDataObject* array =  this->GetInput( port );
        return vtkDoubleArray::SafeDownCast( array->GetFieldData()->GetArray(0) )->GetValue(0);
    } else {
        cerr << "ERROR: Input parameter port type mismatch! Port " << port << " is not of type double. " << endl;
    }
}


/*!
 * Parameter port setter.
 */
void svkImageAlgorithmWithParameterMapping::SetIntParameter( int port, int value )
{
    // CHECK THAT THIS IS THE CORRECT TYPE FIRS
    if( this->GetParameterPortType(port) == VTK_INT ) {
        vtkDataObject* array =  this->GetInput( port );
        vtkIntArray::SafeDownCast( array->GetFieldData()->GetArray(0) )->SetTuple1(0, value);
    } else {
        cerr << "ERROR: Input parameter port type mismatch! Port " << port << " is not of type int. " << endl;
    }
}


/*!
 * Parameter port getter.
 */
int svkImageAlgorithmWithParameterMapping::GetIntParameter( int port )
{
    if( this->GetParameterPortType(port) == VTK_INT ) {
        vtkDataObject* array =  this->GetInput( port );
        return vtkIntArray::SafeDownCast( array->GetFieldData()->GetArray(0) )->GetValue(0);
    } else {
        cerr << "ERROR: Input parameter port type mismatch! Port " << port << " is not of type int. " << endl;
    }
}


/*!
 * Parameter port setter.
 */
void svkImageAlgorithmWithParameterMapping::SetStringParameter( int port, string value )
{
    if( this->GetParameterPortType(port) == VTK_CHAR ) {
        vtkDataArray* array =  this->GetInput( port )->GetFieldData()->GetArray(0);
        array->SetNumberOfTuples(value.size());
        for( int i = 0; i < value.size(); i++ ) {
            array->SetTuple1(i, value.c_str()[i]);
        }
    } else {
        cerr << "ERROR: Input parameter port type mismatch! Port " << port << " is not of type string. " << endl;
    }
}


/*!
 * Parameter port getter.
 */
string svkImageAlgorithmWithParameterMapping::GetStringParameter( int port )
{
    if( this->GetParameterPortType(port) == VTK_CHAR ) {
        vtkDataObject* dataObject =  this->GetInput( port );
        vtkCharArray* charArray = vtkCharArray::SafeDownCast( dataObject->GetFieldData()->GetArray(0));
        string value;
        value.assign( charArray->GetPointer(0), charArray->GetNumberOfTuples() );
        return value;
    } else {
        cerr << "ERROR: Input parameter port type mismatch! Port " << port << " is not of type string. " << endl;
    }
}


/*!
 *  PrintSelf method calls parent class PrintSelf, then prints all parameters.
 *
 */
void svkImageAlgorithmWithParameterMapping::PrintSelf( ostream &os, vtkIndent indent )
{
    Superclass::PrintSelf( os, indent );
    os << indent << "Svk Parameters:" << endl;
    for( int i = this->firstParameterPort; i < this->firstParameterPort + this->numberOfParameters; i++ ) {
        vtkDataObject* parameterObject =  this->GetInput( i );
        vtkDataArray* array = parameterObject->GetFieldData()->GetArray(0);
        if( array->GetDataType() == VTK_CHAR ) {
            vtkCharArray* charArray = vtkCharArray::SafeDownCast( array );
            string parameterString;
            parameterString.assign( charArray->GetPointer(0), charArray->GetNumberOfTuples() );
            os << indent << indent << this->GetParameterName(i) << ": " << parameterString << endl;
        } else {
            os << indent << indent << this->GetParameterName(i) << ": " << array->GetTuple1(0) << endl;
        }
    }
}
