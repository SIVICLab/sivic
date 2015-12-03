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



#include <svkDataAcquisitionDescriptionXML.h>
#include <svkSatBandsXML.h>
#include <svkTypeUtils.h>
#include <svkFileUtils.h>
#include <svkUtils.h>
#include <vtkXMLUtilities.h>
#include <vtkXMLDataParser.h>
#include <vtkMath.h>
#include <stdexcept>

using namespace svk;



vtkCxxRevisionMacro(svkDataAcquisitionDescriptionXML, "$Rev$");
vtkStandardNewMacro(svkDataAcquisitionDescriptionXML);


/*!
 *
 */
svkDataAcquisitionDescriptionXML::svkDataAcquisitionDescriptionXML()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    this->isVerbose = false; 
    this->dataAcquisitionDescriptionXML = NULL; 
    this->versionElement = NULL;     
    this->satBandsElement = NULL;     
    this->satBandsXML = NULL;     
    this->xmlFileName ="";
    
    this->versionNumber = -0.0;
}


/*!
 *
 */
svkDataAcquisitionDescriptionXML::~svkDataAcquisitionDescriptionXML()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());

    if( this->satBandsXML != NULL ) {
        this->satBandsXML->Delete();
    }
    if( this->dataAcquisitionDescriptionXML != NULL ) {
        this->dataAcquisitionDescriptionXML->Delete();
    }
}


/*!
 * Set the trajectory type
 */
void svkDataAcquisitionDescriptionXML::SetTrajectoryType( vtkstd::string type )
{
	this->SetNestedElementWithPath("encoding/trajectory", type );
}


/*!
 * Get the trajectory type
 */
vtkstd::string svkDataAcquisitionDescriptionXML::GetTrajectoryType( )
{
	return this->GetNestedElementCharacterDataWithPath( "encoding/trajectory" );
}


/*!
 * Set the trajectory id
 */
void svkDataAcquisitionDescriptionXML::SetTrajectoryID( vtkstd::string ID )
{
    this->SetNestedElementWithPath("encoding/trajectoryDescription/identifier", ID);
}


/*!
 * Get the trajectory id
 */
vtkstd::string svkDataAcquisitionDescriptionXML::GetTrajectoryID( )
{
	return this->GetNestedElementCharacterDataWithPath( "encoding/trajectoryDescription/identifier" );
}


/*!
 * Set the trajectory comment
 */
void svkDataAcquisitionDescriptionXML::SetTrajectoryComment( vtkstd::string comment )
{
    this->SetNestedElementWithPath("encoding/trajectoryDescription/comment", comment);
}


/*!
 * Get the trajectory comment
 */
vtkstd::string svkDataAcquisitionDescriptionXML::GetTrajectoryComment( )
{
	return this->GetNestedElementCharacterDataWithPath( "encoding/trajectoryDescription/comment" );
}


/*!
 * Set a trajectory user parameter long
 */
void svkDataAcquisitionDescriptionXML::SetTrajectoryParameter( vtkstd::string name, long value  )
{
    vtkstd::string valueString = svkTypeUtils::IntToString(value);
    this->SetTrajectoryParameter("userParameterLong", name, valueString );
}


/*!
 * Get a trajectory user parameter long
 */
long svkDataAcquisitionDescriptionXML::GetTrajectoryLongParameter( vtkstd::string name  )
{
	vtkstd::string parameterString = this->GetTrajectoryParameter("userParameterLong", name );
	return svkTypeUtils::StringToLInt( parameterString );
}


/*!
 * Get a trajectory user parameter double
 */
double svkDataAcquisitionDescriptionXML::GetTrajectoryDoubleParameter( vtkstd::string name  )
{
	vtkstd::string parameterString = this->GetTrajectoryParameter("userParameterDouble", name );
	return svkTypeUtils::StringToDouble( parameterString );
}


/*!
 * Set a trajectory user parameter double
 */
void svkDataAcquisitionDescriptionXML::SetTrajectoryParameter( vtkstd::string name, double value  )
{
    vtkstd::string valueString = svkTypeUtils::DoubleToString(value);
    this->SetTrajectoryParameter("userParameterDouble", name, valueString );
}


/*!
 * Initialize the skeleton of the xml file.
 */
void svkDataAcquisitionDescriptionXML::InitializeEmptyXMLFile()
{
    this->ClearXMLFile();

    // Create Root element
    if( this->dataAcquisitionDescriptionXML != NULL ) {
    	this->dataAcquisitionDescriptionXML->Delete();
    }
    this->dataAcquisitionDescriptionXML = vtkXMLDataElement::New();
    this->dataAcquisitionDescriptionXML->SetName("svk_data_acquisition_description");

    // Create Encoding Element
    vtkXMLDataElement* encodingElem = svkUtils::CreateNestedXMLDataElement( this->dataAcquisitionDescriptionXML, "encoding" , "" );
    vtkXMLDataElement* trajectoryElem = svkUtils::CreateNestedXMLDataElement( encodingElem, "trajectory" , "" );
    vtkXMLDataElement* trajectoryDescElem = svkUtils::CreateNestedXMLDataElement( encodingElem, "trajectoryDescription" , "" );
    vtkXMLDataElement* trajectoryIDElem = svkUtils::CreateNestedXMLDataElement( trajectoryDescElem, "identifier" , "" );
    vtkXMLDataElement* trajectoryCommentElem = svkUtils::CreateNestedXMLDataElement( trajectoryDescElem, "comment" , "" );
    vtkXMLDataElement* spaceElem = svkUtils::CreateNestedXMLDataElement( encodingElem, "encodedSpace" , "" );
    vtkXMLDataElement* matrixElem = svkUtils::CreateNestedXMLDataElement( spaceElem, "matrixSize" , "" );
    vtkXMLDataElement* matrixXElem = svkUtils::CreateNestedXMLDataElement( matrixElem, "x" , "" );
    vtkXMLDataElement* matrixYElem = svkUtils::CreateNestedXMLDataElement( matrixElem, "y" , "" );
    vtkXMLDataElement* matrixZElem = svkUtils::CreateNestedXMLDataElement( matrixElem, "z" , "" );
    vtkXMLDataElement* fovElem = svkUtils::CreateNestedXMLDataElement( spaceElem, "fieldOfView_mm" , "" );
    vtkXMLDataElement* fovXElem = svkUtils::CreateNestedXMLDataElement( fovElem, "x" , "" );
    vtkXMLDataElement* fovYElem = svkUtils::CreateNestedXMLDataElement( fovElem, "y" , "" );
    vtkXMLDataElement* fovZElem = svkUtils::CreateNestedXMLDataElement( fovElem, "z" , "" );

    encodingElem->Delete();
    trajectoryElem->Delete();
    trajectoryDescElem->Delete();
    trajectoryIDElem->Delete();
    trajectoryCommentElem->Delete();
    spaceElem->Delete();
    matrixElem->Delete();
    matrixXElem->Delete();
    matrixYElem->Delete();
    matrixZElem->Delete();
    fovElem->Delete();
    fovXElem->Delete();
    fovYElem->Delete();
    fovZElem->Delete();
}


/*!
 *  set the path/name to xml file and parse.
 *      
 */
int svkDataAcquisitionDescriptionXML::SetXMLFileName( string xmlFileName )
{

    this->xmlFileName = xmlFileName;  
    // Now we have to remove the old xml file
    this->ClearXMLFile();
    if( !svkUtils::FilePathExists( this->xmlFileName.c_str() ) ) {
        cout << "ERROR, XML file not found:" << this->xmlFileName << endl;
        return 1;

    }
    this->dataAcquisitionDescriptionXML = vtkXMLUtilities::ReadElementFromFile( this->xmlFileName.c_str() );
    if (this->dataAcquisitionDescriptionXML == NULL ) { 
        cout << "ERROR: xml file could not be parsed:" << this->xmlFileName << endl;
        return 1; 
    } 

    // parse the 3 top level elements: 
    this->satBandsElement = this->dataAcquisitionDescriptionXML->FindNestedElementWithName("svk_sat_bands");
    if( this->satBandsElement != NULL ) {
        this->satBandsXML = svkSatBandsXML::New();
        int result = this->satBandsXML->ParseXML(this->satBandsElement );
        if( result != 0 ) {
        	cout << "WARNING: Could not parse sat bands element from xml file " << this->xmlFileName << endl;
        	this->satBandsXML->Delete();
        	this->satBandsXML = NULL;

        }
    } 
    this->versionElement = this->dataAcquisitionDescriptionXML->FindNestedElementWithName("version");
    if( this->versionElement != NULL ) {
        this->versionNumber = svkTypeUtils::StringToFloat(string( versionElement->GetCharacterData() ));
    } else {
    	cout << "ERROR: Could not find version element in xml file: " << this->xmlFileName << endl;
    	return 1;
    }
    if( this->GetDebug() ) {
    	if(this->dataAcquisitionDescriptionXML != NULL ) {
            this->dataAcquisitionDescriptionXML->PrintXML(cout, vtkIndent());
    	}
    	if( this->versionElement != NULL ) {
            this->versionElement->PrintXML(cout, vtkIndent());
    	}
    	if( this->satBandsElement != NULL ) {
            this->satBandsElement->PrintXML(cout, vtkIndent());
    	}
    }

    return 0; 

}


/*!
 *  Sets the current XML data to NULL
 *  so the file will be re-read.
 */
void svkDataAcquisitionDescriptionXML::ClearXMLFile( )
{
    if( this->dataAcquisitionDescriptionXML != NULL ) {
        this->dataAcquisitionDescriptionXML->Delete();
        this->dataAcquisitionDescriptionXML = NULL;
    }
    if( this->satBandsXML != NULL ) {
        this->satBandsXML->Delete();
        this->satBandsXML = NULL;
    }

}


/*!
 *  Get the path to the current XML file
 */
string svkDataAcquisitionDescriptionXML::GetXMLFileName( )
{
    return this->xmlFileName;
}


/*!
 * Set the verbose flag.
 */
void svkDataAcquisitionDescriptionXML::SetVerbose( bool isVerbose )
{
    this->isVerbose = isVerbose; 
}


/*!
 * Write out the XML file. Returns 0 if the file is successfully written
 * otherwise returns -1.
*/
int svkDataAcquisitionDescriptionXML::WriteXMLFile( string xmlFileName )
{
    int status = -1;
    if( this->dataAcquisitionDescriptionXML != NULL ) {
        vtkIndent indent;
        if( this->GetDebug() ) {
            this->dataAcquisitionDescriptionXML->PrintXML(cout, vtkIndent());
        }
        vtkXMLUtilities::WriteElementToFile( this->dataAcquisitionDescriptionXML, xmlFileName.c_str(), &indent );
        if( svkUtils::FilePathExists(xmlFileName.c_str())) {
        	status = 0;
        }
    }
    return status;
}


/*!
 * Generically sets a trajectory parameter.
 */
void svkDataAcquisitionDescriptionXML::SetTrajectoryParameter( vtkstd::string type, vtkstd::string name, vtkstd::string value  )
{
	// Assume failure
    bool errorFound = true;
	if( type == "userParameterLong" || type == "userParameterDouble") {
        vtkXMLDataElement* trajDescElem  = this->FindNestedElementWithPath("encoding/trajectoryDescription");
        vtkXMLDataElement* paramElem = NULL;
        vtkXMLDataElement* nameElem = NULL;
        vtkXMLDataElement* valueElem = NULL;
        if( trajDescElem != NULL ) {
            paramElem =  svkUtils::CreateNestedXMLDataElement(trajDescElem, type, "");
            nameElem =  svkUtils::CreateNestedXMLDataElement(paramElem, "name", name);
            valueElem =  svkUtils::CreateNestedXMLDataElement(paramElem, "value", value);
            errorFound = false;
        }
        if( paramElem != NULL ) {
            paramElem->Delete();
        } else {
            errorFound = true;
        }
        if( nameElem != NULL ) {
            nameElem->Delete();
        } else {
            errorFound = true;
        }
        if( valueElem != NULL ) {
            valueElem->Delete();
        } else {
            errorFound = true;
        }
	}
    if( errorFound ) {
        cout << "ERROR: Could not set trajectory parameter of type " << type << " with name " << name << " to value " << value << endl;
    }
}


/*!
 * Get a string representation of a trajectory parameter.
 */
vtkstd::string svkDataAcquisitionDescriptionXML::GetTrajectoryParameter( vtkstd::string type, vtkstd::string name )
{
    bool parameterFound = false;
    vtkstd::string parameterValue = "";
    vtkXMLDataElement* trajDescElem  = this->FindNestedElementWithPath("encoding/trajectoryDescription");
    if(trajDescElem != NULL ) {
        for( int i = 0; i < trajDescElem->GetNumberOfNestedElements(); i++ ) {
            vtkXMLDataElement* paramElem = trajDescElem->GetNestedElement(i);
            vtkstd::string paramElemName = "";
            if( paramElem != NULL ) {
            	paramElemName = paramElem->GetName();
            }
            if( !paramElemName.empty() && paramElemName == type ) {
                vtkXMLDataElement* nameElem =  svkUtils::FindNestedElementWithPath(paramElem, "name");
                vtkstd::string paramName = "";
                if( nameElem != NULL ) {
                	paramName = nameElem->GetCharacterData();
                }
                vtkXMLDataElement* valueElem =  svkUtils::FindNestedElementWithPath(paramElem, "value");
                if( !paramName.empty() && paramName == name && valueElem != NULL ) {
                    parameterValue = valueElem->GetCharacterData();
                    parameterFound = true;
                    break;
                }
            }
        }
    } else {
    	parameterFound = false;
    }
    if(!parameterFound ) {
    	cout << "ERROR: Could not find parameter of type  " << type << " and name " << name << endl;
    }
    return parameterValue;
}


/*!
 *  Get the version of the xml file
 */
int svkDataAcquisitionDescriptionXML::GetXMLVersion() 
{
    return this->versionNumber; 
}


/*!
 *  Finds an xml element. Reports an error and returns null if nothing is found at the given xpath.
 */
vtkXMLDataElement* svkDataAcquisitionDescriptionXML::FindNestedElementWithPath( string xmlPath )
{
	vtkXMLDataElement* elem = NULL;
    elem = svkUtils::FindNestedElementWithPath(this->dataAcquisitionDescriptionXML, xmlPath );
    if( elem == NULL ) {
    	cout << "ERROR: Could not locate element at path: " << xmlPath << endl;
    }
	return elem;
}


/*!
 * Returns the character data at a specific path. Reports an error if the path is not found.
 */
vtkstd::string svkDataAcquisitionDescriptionXML::GetNestedElementCharacterDataWithPath( string xmlPath )
{
	string data = "";
	bool foundData = svkUtils::GetNestedElementCharacterDataWithPath( this->dataAcquisitionDescriptionXML, xmlPath, data );
	if( !foundData ) {
		cout << "ERROR: Could get character data at path: " << xmlPath << endl;
	}
	return data;
}


/*!
 *  Sets the character data for an xml element. Reports an error and returns
 *  null if nothing is found at the given xpath.
 */
void svkDataAcquisitionDescriptionXML::SetNestedElementWithPath( string xmlPath, string value )
{
	bool wasSet = svkUtils::SetNestedElementWithPath( this->dataAcquisitionDescriptionXML, xmlPath, value);
	if( !wasSet ) {
		cout << "ERROR: Could not set value <" << value << "> for element " << xmlPath << endl;
	}
}


/*!
 *  Get the svkSatBandsXML object.
 */
svkSatBandsXML* svkDataAcquisitionDescriptionXML::GetSatBandsXML( )
{
    return this->satBandsXML;
}


/*!
 * Get the main XML data object.
 */
vtkXMLDataElement* svkDataAcquisitionDescriptionXML::GetXMLDataElement()
{
    return this->dataAcquisitionDescriptionXML;
}


/*!
 * C method for instantiating the object:
 */
void* svkDataAcquisitionDescriptionXML_New()
{
    svkDataAcquisitionDescriptionXML* xml = svkDataAcquisitionDescriptionXML::New();
    xml->InitializeEmptyXMLFile();
    return xml;
}


/*!
 * C method for releasing the object:
 */
void* svkDataAcquisitionDescriptionXML_Delete( void* xml )
{
    ((svkDataAcquisitionDescriptionXML*)xml)->Delete();
    return NULL;
}


/*!
 * C method reading in an xml file:
 */
void* svkDataAcquisitionDescriptionXML_Read(const char* xmlFileName, int *status)
{
    svkDataAcquisitionDescriptionXML* xml = svkDataAcquisitionDescriptionXML::New();
    *status = xml->SetXMLFileName(xmlFileName);
    if (*status == 1 ) {
        xml->Delete();
        xml = NULL;
    }
    return ((void*)xml);
};



/*!
 * C method for writing the xml to disk.
 */
int svkDataAcquisitionDescriptionXML_WriteXMLFile(const char* filepath, void* xml )
{
	if( xml != NULL ) {
		return ((svkDataAcquisitionDescriptionXML*)xml)->WriteXMLFile( filepath );
    } else {
    	return false;
    }
}


/*!
 * C method for getting the sat bands xml
 */
void* svkDataAcquisitionDescriptionXML_GetSatBandsXML( void* dataAcquisitionDescriptionXML )
{
    void* satBandsXML = NULL;
    svkDataAcquisitionDescriptionXML* dadXML =(svkDataAcquisitionDescriptionXML*)dataAcquisitionDescriptionXML;
    if( dadXML != NULL ) {
        satBandsXML = dadXML->GetSatBandsXML();
    }
    return satBandsXML;
}


/*!
 * C method for setting the trajectory element.
 */
void svkDataAcquisitionDescriptionXML_SetTrajectory(const char* type, const char* id, const char* comment, void* xml)
{
    ((svkDataAcquisitionDescriptionXML*)xml)->SetTrajectoryType( type );
    ((svkDataAcquisitionDescriptionXML*)xml)->SetTrajectoryID( id );
    ((svkDataAcquisitionDescriptionXML*)xml)->SetTrajectoryComment( comment );
}


/*!
 * C method for getting the trajectory type.
 */
const char* svkDataAcquisitionDescriptionXML_GetTrajectoryType(void* xml)
{
    return (((svkDataAcquisitionDescriptionXML*)xml)->GetTrajectoryType( )).c_str();
}


/*!
 * C method for getting the trajectory id.
 */
const char* svkDataAcquisitionDescriptionXML_GetTrajectoryID(void* xml)
{
    return (((svkDataAcquisitionDescriptionXML*)xml)->GetTrajectoryID( )).c_str();
}


/*!
 * C method for getting the trajectory comment.
 */
const char* svkDataAcquisitionDescriptionXML_GetTrajectoryComment(void* xml)
{
    return (((svkDataAcquisitionDescriptionXML*)xml)->GetTrajectoryComment( )).c_str();
}

/*!
 * C method for setting a user parameter long.
 */
void svkDataAcquisitionDescriptionXML_SetTrajectoryLongParameter(const char* name, long value, void* xml )
{
    ((svkDataAcquisitionDescriptionXML*)xml)->SetTrajectoryParameter( name, value );
}


/*!
 * C method for getting a user parameter long.
 */
long svkDataAcquisitionDescriptionXML_GetTrajectoryLongParameter(const char* name, void* xml )
{
    return ((svkDataAcquisitionDescriptionXML*)xml)->GetTrajectoryLongParameter( name );
}


/*!
 * C method for setting a user parameter double.
 */
void svkDataAcquisitionDescriptionXML_SetTrajectoryDoubleParameter(const char* name, double value, void* xml )
{
    ((svkDataAcquisitionDescriptionXML*)xml)->SetTrajectoryParameter( name, value );
}


/*!
 * C method for setting a user parameter double.
 */
double svkDataAcquisitionDescriptionXML_GetTrajectoryDoubleParameter(const char* name, void* xml )
{
    return ((svkDataAcquisitionDescriptionXML*)xml)->GetTrajectoryDoubleParameter( name );
}


