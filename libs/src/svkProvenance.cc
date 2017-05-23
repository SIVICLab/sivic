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


#include <svkProvenance.h>


using namespace svk;


//vtkCxxRevisionMacro(svkProvenance, "$Rev$");
vtkStandardNewMacro(svkProvenance);


/*!
 *
 */
svkProvenance::svkProvenance()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif
    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() );

    this->xmlProvenance = vtkXMLDataElement::New();
    xmlProvenance->SetName("SVK_PROVENANCE");
    this->SetVersion(); 
    this->CreateApplicationCommandElement(); 
}


/*!
 *
 */
svkProvenance::~svkProvenance()
{

    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() );

    if ( this->xmlProvenance != NULL )  {
        this->xmlProvenance->Delete();
        this->xmlProvenance = NULL;
    }

}


/*!
 *  Adds a nested XML element to the provance to indicate the svk version
 *  and platform or other run-time information 
 */
void svkProvenance::SetVersion()
{
    vtkXMLDataElement* xmlVersion= vtkXMLDataElement::New();
    xmlVersion->SetName("SVK_VERSION");
    xmlVersion->SetAttribute( "version", SVK_RELEASE_VERSION );
    xmlVersion->SetAttribute( "platform", "NA");
    xmlProvenance->AddNestedElement( xmlVersion);
    xmlVersion->Delete(); 
}


/*!
 *  Creates an place holder element at the top of the object 
 *  to represent the application level command line input.  
 *  The cmdLine attribute gets filled in through a call to  
 *  !SetApplicationCommand(). 
 */
void svkProvenance::CreateApplicationCommandElement()
{
    vtkXMLDataElement* xmlCmdLine = vtkXMLDataElement::New();
    xmlCmdLine->SetName( "SVK_CMD_LINE" );
    xmlProvenance->AddNestedElement( xmlCmdLine );
    xmlCmdLine->Delete(); 
}


/*!
 *  Adds a nested XML element to the provance to record the application level 
 *  command line used to run the SIVIC application. 
 */
void svkProvenance::SetApplicationCommand( string cmdLine )
{
    vtkXMLDataElement* xmlCmdLine = xmlProvenance->FindNestedElementWithName( "SVK_CMD_LINE" ); 
    xmlCmdLine->SetAttribute( "cmd", cmdLine.c_str() );
}


/*!
 *  Adds a nested XML element to the provance as a container for 
 *  representing a discrete algorithm applied to the svkImageData object. 
 */
void svkProvenance::AddAlgorithm(string algoName)
{
    vtkXMLDataElement* xmlAlgoElement = vtkXMLDataElement::New();
    xmlAlgoElement->SetName("SVK_ALGORITHM");
    xmlAlgoElement->SetAttribute( "algoName", algoName.c_str() );
    xmlProvenance->AddNestedElement( xmlAlgoElement );
    xmlAlgoElement->Delete(); 
}


/*!
 *  Adds XML elements to represent an algorithm's input args.  These are 
 *  added as elements within the named AlgorithmElement. 
 *  argNumber, argName, argValue, argType?
 */
template <class ArgType> void svkProvenance::AddAlgorithmArg(string algoName, int argNumber, string argName, ArgType argValue)
{

    vtkXMLDataElement* xmlAlgoArgElement = vtkXMLDataElement::New();
    xmlAlgoArgElement->SetName("SVK_ALGORITHM_ARG");
    xmlAlgoArgElement->SetAttribute( "argName", argName.c_str() );
    xmlAlgoArgElement->SetIntAttribute( "argNumber", argNumber );

    std::ostringstream argValueOss;
    argValueOss << argValue;
    xmlAlgoArgElement->SetAttribute( "argValue", argValueOss.str().c_str() );

    vtkXMLDataElement* xmlAlgoElement = xmlProvenance->FindNestedElementWithNameAndAttribute( 
        "SVK_ALGORITHM", 
        "algoName", 
        algoName.c_str() 
    );

    xmlAlgoElement->AddNestedElement( xmlAlgoArgElement );
    xmlAlgoArgElement->Delete(); 
}


/*!
 *  Prints the XML Provenance object to stdout
 */
void svkProvenance::PrintXML(ostream& out)
{
    vtkIndent indent(0);
    xmlProvenance->PrintXML(out, indent);
}


/*!
 *  Converts argv into a string.  
 */
string svkProvenance::GetCommandLineString( int argc, char** argv )
{
    string cmdLine;
    for (int i = 0; i < argc; i++) {
        cmdLine.append( argv[i] );
        cmdLine.append( " " );
    }
    return cmdLine; 
}


//  Explicit template specialization so compiler generates code and linker doesn't complain 
template void svkProvenance::AddAlgorithmArg <bool>   ( string algoName, int argNumber, string argName, bool argValue ); 
template void svkProvenance::AddAlgorithmArg <int>    ( string algoName, int argNumber, string argName, int argValue ); 
template void svkProvenance::AddAlgorithmArg <float>  ( string algoName, int argNumber, string argName, float argValue ); 
template void svkProvenance::AddAlgorithmArg <double> ( string algoName, int argNumber, string argName, double argValue ); 
template void svkProvenance::AddAlgorithmArg <string> ( string algoName, int argNumber, string argName, string argValue ); 
template void svkProvenance::AddAlgorithmArg <char*>  ( string algoName, int argNumber, string argName, char* argValue ); 
