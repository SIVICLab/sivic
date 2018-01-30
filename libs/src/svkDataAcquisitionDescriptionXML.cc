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



#include <svkDataAcquisitionDescriptionXML.h>
#include <svkSatBandsXML.h>
#include <svkTypeUtils.h>
#include <svkFileUtils.h>
#include <svkXMLUtils.h>
#include <vtkXMLUtilities.h>
#include <vtkXMLDataParser.h>
#include <vtkMath.h>
#include <stdexcept>
#define NULL_XML_ERROR "ERROR: Data Acquisition Description XML is NULL!\n"
#define XML_VERSION "0"

using namespace svk;



//vtkCxxRevisionMacro(svkDataAcquisitionDescriptionXML, "$Rev$");
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
 * Sets the contents of the encoding/trajectory element.
 * \param type the desired contents of the encoding/trajectory element
 *
 */
void svkDataAcquisitionDescriptionXML::SetTrajectoryType( string type )
{
    this->SetDataWithPath("encoding/trajectory", type.c_str() );
}


/*!
 * Get the contents of the encoding/trajectory element.
 * \return the contents of the encoding/trajectory element
 */
string svkDataAcquisitionDescriptionXML::GetTrajectoryType( )
{
    return this->GetDataWithPath( "encoding/trajectory" );
}


/*!
 * Sets the contents of the encoding/trajectoryDescription/identifier element.
 * \param ID the desired contents of the encoding/trajectoryDescription/identifier element
 *
 */
void svkDataAcquisitionDescriptionXML::SetTrajectoryID( string ID )
{
    this->SetDataWithPath("encoding/trajectoryDescription/identifier", ID.c_str());
}


/*!
 * Get the contents of the encoding/trajectoryDescription/identifier element.
 * \return the contents of the encoding/trajectoryDescription/identifier element
 */
string svkDataAcquisitionDescriptionXML::GetTrajectoryID( )
{
    return this->GetDataWithPath( "encoding/trajectoryDescription/identifier" );
}


/*!
 * Sets the contents of the encoding/trajectoryDescription/comment element.
 * \param comment the desired contents of the encoding/trajectoryDescription/comment element
 *
 */
void svkDataAcquisitionDescriptionXML::SetTrajectoryComment( string comment )
{
    this->SetDataWithPath("encoding/trajectoryDescription/comment", comment.c_str());
}


/*!
 * Get the contents of the encoding/trajectoryDescription/comment element.
 * \return the contents of the encoding/trajectoryDescription/comment element
 */
string svkDataAcquisitionDescriptionXML::GetTrajectoryComment( )
{
    return this->GetDataWithPath( "encoding/trajectoryDescription/comment" );
}


/*!
 * Creates a new encoding/trajectoryDescription/userParameterLong element and
 * creates its child elements 'name' and 'value' with the given values.
 * \param name the of the new user parameter element
 * \param value the value of the new user parameter
 *
 */
void svkDataAcquisitionDescriptionXML::SetTrajectoryParameter( string name, long value  )
{
    string valueString = svkTypeUtils::IntToString(value);
    this->SetTrajectoryParameter("userParameterLong", name, valueString );
}


/*!
 * Gets the contents of an encoding/trajectoryDescription/userParameterLong element.
 * \param name the name of the user parameter
 * \return the value of the user parameter
 */
long svkDataAcquisitionDescriptionXML::GetTrajectoryLongParameter( string name  )
{
    string parameterString = this->GetTrajectoryParameter("userParameterLong", name );
    return svkTypeUtils::StringToLInt( parameterString );
}


/*!
 * Gets the contents of an encoding/trajectoryDescription/userParameterDouble element.
 * \param name the name of the user parameter
 * \return the value of the user parameter
 */
double svkDataAcquisitionDescriptionXML::GetTrajectoryDoubleParameter( string name  )
{
    string parameterString = this->GetTrajectoryParameter("userParameterDouble", name );
    return svkTypeUtils::StringToDouble( parameterString );
}


/*!
 * Creates a new encoding/trajectoryDescription/userParameterDouble element and
 * creates its child elements 'name' and 'value' with the given values.
 * \param name the of the new user parameter element
 * \param value the value of the new user parameter
 *
 */
void svkDataAcquisitionDescriptionXML::SetTrajectoryParameter( string name, double value  )
{
    string valueString = svkTypeUtils::DoubleToString(value);
    this->SetTrajectoryParameter("userParameterDouble", name, valueString );
}


/*!
 * Initialize the skeleton of the xml file. This will create the following elements:
 * svk_data_acquisition_description
 * svk_data_acquisition_description/version
 * svk_data_acquisition_description/encoding
 * svk_data_acquisition_description/encoding/trajectory
 * svk_data_acquisition_description/encoding/trajectoryDescription
 * svk_data_acquisition_description/encoding/trajectoryDescription/identifier
 * svk_data_acquisition_description/encoding/trajectoryDescription/comment
 * svk_data_acquisition_description/encoding/encodedSpace
 * svk_data_acquisition_description/encoding/encodedSpace/matrixSize
 * svk_data_acquisition_description/encoding/encodedSpace/matrixSize/x
 * svk_data_acquisition_description/encoding/encodedSpace/matrixSize/y
 * svk_data_acquisition_description/encoding/encodedSpace/matrixSize/z
 * svk_data_acquisition_description/encoding/encodedSpace/fieldOfView_mm/x
 * svk_data_acquisition_description/encoding/encodedSpace/fieldOfView_mm/y
 * svk_data_acquisition_description/encoding/encodedSpace/fieldOfView_mm/z
 */
void svkDataAcquisitionDescriptionXML::InitializeEmptyXMLFile()
{
    this->ClearXMLFile();

    this->dataAcquisitionDescriptionXML = vtkXMLDataElement::New();
    this->dataAcquisitionDescriptionXML->SetName("svk_data_acquisition_description");

    // Create Encoding Element
    vtkXMLDataElement* versionElem = svkXMLUtils::CreateNestedXMLDataElement( 
            this->dataAcquisitionDescriptionXML, "version" , XML_VERSION );
    vtkXMLDataElement* encodingElem = svkXMLUtils::CreateNestedXMLDataElement( 
            this->dataAcquisitionDescriptionXML, "encoding" , "" );
    vtkXMLDataElement* trajectoryElem = svkXMLUtils::CreateNestedXMLDataElement( 
            encodingElem, "trajectory" , "" );
    vtkXMLDataElement* trajectoryDescElem = svkXMLUtils::CreateNestedXMLDataElement( 
            encodingElem, "trajectoryDescription" , "" );
    vtkXMLDataElement* trajectoryIDElem = svkXMLUtils::CreateNestedXMLDataElement( 
            trajectoryDescElem, "identifier" , "" );
    vtkXMLDataElement* trajectoryCommentElem = svkXMLUtils::CreateNestedXMLDataElement( 
            trajectoryDescElem, "comment" , "" );
    vtkXMLDataElement* spaceElem = svkXMLUtils::CreateNestedXMLDataElement( 
            encodingElem, "encodedSpace" , "" );
    vtkXMLDataElement* matrixElem = svkXMLUtils::CreateNestedXMLDataElement( 
            spaceElem, "matrixSize" , "" );
    vtkXMLDataElement* matrixXElem = svkXMLUtils::CreateNestedXMLDataElement( 
            matrixElem, "x" , "" );
    vtkXMLDataElement* matrixYElem = svkXMLUtils::CreateNestedXMLDataElement( 
            matrixElem, "y" , "" );
    vtkXMLDataElement* matrixZElem = svkXMLUtils::CreateNestedXMLDataElement( 
            matrixElem, "z" , "" );
    vtkXMLDataElement* fovElem = svkXMLUtils::CreateNestedXMLDataElement( 
            spaceElem, "fieldOfView_mm" , "" );
    vtkXMLDataElement* fovXElem = svkXMLUtils::CreateNestedXMLDataElement( 
            fovElem, "x" , "" );
    vtkXMLDataElement* fovYElem = svkXMLUtils::CreateNestedXMLDataElement( 
            fovElem, "y" , "" );
    vtkXMLDataElement* fovZElem = svkXMLUtils::CreateNestedXMLDataElement( 
            fovElem, "z" , "" );

    versionElem->Delete();
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
 *  Set the path/name to an xml file, parse the file and initialize the object.
 *  \param xmlFileName the path of the file to be read
 *  \return 0 for success
 *      
 */
int svkDataAcquisitionDescriptionXML::SetXMLFileName( string xmlFileName )
{

    // Now we have to remove the old xml file
    this->ClearXMLFile();
    this->xmlFileName = xmlFileName;
    if( !svkFileUtils::FilePathExists( this->xmlFileName.c_str() ) ) {
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
 * Deletes the internal XML objects.
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
    this->xmlFileName = "";

}


/*!
 *  Get the path to the current XML file.
 *  \return the path to the current xml file;
 */
string svkDataAcquisitionDescriptionXML::GetXMLFileName( )
{
    return this->xmlFileName;
}


/*!
 * Set the verbose flag.
 * \param verbose boolean flag
 */
void svkDataAcquisitionDescriptionXML::SetVerbose( bool isVerbose )
{
    this->isVerbose = isVerbose; 
}


/*!
 * Write out the internal XML object to file.
 * \param xmlFileName target file name to be written to
 * \return 0 on success otherwise -1
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
        if( svkFileUtils::FilePathExists(xmlFileName.c_str())) {
            status = 0;
        }
    }
    return status;
}


/*!
 * Sets the value of an encoding/trajectoryDescription/userParameter[Double|Long] element
 * If it does not exist it is created as well as its child elements 'name' and
 * 'value' with the given input.
 *
 * \param type the of type new user parameter element. Must be userParameterLong or userParameterDouble
 * \param name the of the new user parameter element
 * \param value the value of the new user parameter
 *
 */
void svkDataAcquisitionDescriptionXML::SetTrajectoryParameter( string type, string name, string value  )
{
    // Assume failure
    bool errorFound = true;
    if( type == "userParameterLong" || type == "userParameterDouble") {
        vtkXMLDataElement* trajDescElem  = this->FindNestedElementWithPath("encoding/trajectoryDescription");
        vtkXMLDataElement* paramElem = this->GetTrajectoryParameterElement( type, name );
        vtkXMLDataElement* nameElem  = NULL;
        vtkXMLDataElement* valueElem = NULL;
        if( trajDescElem != NULL ) {
            // Check if the named element exists
            if( paramElem != NULL ) {
                valueElem = paramElem->FindNestedElementWithName("value");
                if( valueElem != NULL ) {
                    valueElem->SetCharacterData(value.c_str(), value.size());
                } else {
                    cout << "ERROR: " << type << " " << name << " does not contain a value tag." << endl;
                    errorFound = true;
                }
            } else {
                paramElem =  svkXMLUtils::CreateNestedXMLDataElement(trajDescElem, type, "");
                nameElem =  svkXMLUtils::CreateNestedXMLDataElement(paramElem, "name", name);
                valueElem =  svkXMLUtils::CreateNestedXMLDataElement(paramElem, "value", value);
                errorFound = false;
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
        } else {
            errorFound = true;
        }
    }
    if( errorFound ) {
        cout << "ERROR: Could not set trajectory parameter of type " << type << " with name " << name << " to value " << value << endl;
    }
}


/*!
 * Gets the contents of an encoding/trajectoryDescription/userParameter[Double|Long] element.
 * \param type the of type new user parameter element. Must be userParameterLong or userParameterDouble
 * \param name the name of the user parameter
 * \return the value of the user parameter
 */
string svkDataAcquisitionDescriptionXML::GetTrajectoryParameter( string type, string name )
{
    string parameterValue = "";
    vtkXMLDataElement* userParamElem = this->GetTrajectoryParameterElement(type, name);
    vtkXMLDataElement* valueElem =  svkXMLUtils::FindNestedElementWithPath(userParamElem, "value");
    if( valueElem != NULL ) {
        parameterValue = valueElem->GetCharacterData();
    } else {
        cout << "ERROR: Could not find parameter of type  " << type << " and name " << name << endl;
    }
    return parameterValue;
}


/*!
 * Gets an encoding/trajectoryDescription/userParameter[Double|Long] element.
 * \param type the of type new user parameter element. Must be userParameterLong or userParameterDouble
 * \param name the name of the user parameter
 * \return the vtkXMLDataElement of the user parameter. NULL is returned if the parameter is not found.
 */
vtkXMLDataElement* svkDataAcquisitionDescriptionXML::GetTrajectoryParameterElement( string type, string name )
{
    vtkXMLDataElement* userParamElem = NULL;
    vtkXMLDataElement* trajDescElem  = this->FindNestedElementWithPath("encoding/trajectoryDescription");
    if(trajDescElem != NULL ) {
        for( int i = 0; i < trajDescElem->GetNumberOfNestedElements(); i++ ) {
            vtkXMLDataElement* paramElem = trajDescElem->GetNestedElement(i);
            string paramElemName = "";
            if( paramElem != NULL ) {
                paramElemName = paramElem->GetName();
            }
            if( !paramElemName.empty() && paramElemName == type ) {
                vtkXMLDataElement* nameElem =  svkXMLUtils::FindNestedElementWithPath(paramElem, "name");
                string paramName = "";
                if( nameElem != NULL ) {
                    paramName = nameElem->GetCharacterData();
                }
                vtkXMLDataElement* valueElem =  svkXMLUtils::FindNestedElementWithPath(paramElem, "value");
                if( !paramName.empty() && paramName == name && valueElem != NULL ) {
                    userParamElem = paramElem;
                    break;
                }
            }
        }
    }
    return userParamElem;
}


/*!
 *  Get the version of the xml file
 *  \return the version of the xml file
 */
int svkDataAcquisitionDescriptionXML::GetXMLVersion() 
{
    return this->versionNumber; 
}


/*!
 *  Finds an xml element. Reports an error and returns null if nothing is found at the given xpath.
 *  \param xmlPath the xpath to the element
 *  \return the element or NULL if not found
 */
vtkXMLDataElement* svkDataAcquisitionDescriptionXML::FindNestedElementWithPath( string xmlPath )
{
    vtkXMLDataElement* elem = NULL;
    elem = svkXMLUtils::FindNestedElementWithPath(this->dataAcquisitionDescriptionXML, xmlPath );
    if( elem == NULL ) {
        cout << "ERROR: Could not locate element at path: " << xmlPath << endl;
    }
    return elem;
}


/*!
 * Returns the character data at a specific path. Reports an error if the path is not found.
 *  \param xmlPath the xpath to the element
 *  \return the contents of the element at the given path
 */
const char * svkDataAcquisitionDescriptionXML::GetDataWithPath( const char* xmlPath )
{
    string data = "";
    bool foundData = svkXMLUtils::GetNestedElementCharacterDataWithPath( this->dataAcquisitionDescriptionXML, xmlPath, &data );
    if( !foundData ) {
        cout << "ERROR: Could get character data at path: " << xmlPath << endl;
    }
    return data.c_str();
}


/*!
 *  Sets the character data for an xml element. Reports an error and returns
 *  null if nothing is found at the given xpath.
 *  \param xmlPath the xpath to the element
 *  \param value the value to set into the element at the given path
 *  \return 0 on success otherwise -1
 */
int svkDataAcquisitionDescriptionXML::SetDataWithPath( const char* xmlPath, const char* value )
{
    bool wasSet = svkXMLUtils::SetNestedElementWithPath( this->dataAcquisitionDescriptionXML, xmlPath, value);
    if( !wasSet ) {
        cout << "ERROR: Could not set value <" << value << "> for element " << xmlPath << endl;
        return -1;
    } else {
        return 0;
    }
}


/*!
 * Adds an xml data element. Note the xmlPath is of the parent where you wish
 * to add the named element.
 *  \param parentPath the xpath to the parent element
 *  \param name the name of the element to add
 */
vtkXMLDataElement* svkDataAcquisitionDescriptionXML::AddElementWithParentPath( const char* parentPath, const char* name )
{
    vtkXMLDataElement* parent = this->FindNestedElementWithPath(parentPath);
    vtkXMLDataElement* elem = svkXMLUtils::CreateNestedXMLDataElement( parent, name, "" );
    if( elem == NULL ) {
        cout << "ERROR: Could not add element " << name << " with parent path " << parentPath << endl;
    }
    return elem;
}


/*!
 * Remove an xml data element. Note the xmlPath is of the parent where you wish
 * to add the named element.
 *  \param parentPath the xpath to the parent element
 *  \param name the name of the element to remove
 *  \return 0 on success otherwise -1
 */
int svkDataAcquisitionDescriptionXML::RemoveElementWithParentPath( const char* parentPath, const char* name )
{
    int status = -1;
    vtkXMLDataElement* parent = this->FindNestedElementWithPath(parentPath);
    string targetPath = parentPath;
    targetPath.append("/");
    targetPath.append(name);
    vtkXMLDataElement* target = this->FindNestedElementWithPath(targetPath);
    if( parent != NULL && target != NULL ) {
        int numChildren = parent->GetNumberOfNestedElements();
        parent->RemoveNestedElement(target);
        if( parent->GetNumberOfNestedElements() == numChildren -1 ) {
            status = 0;
        } else {
            status = -1;
        }
    }  else {
        status = -1;
    }
    if(status != 0) {
        cout << "ERROR: Could not remove element " << name << " with parent path " << parentPath << endl;
    }
    return status;
}


/*!
 *  Get the internal svk::svkSatBandsXML object.
 *  \return the internal svk::svkSatBandsXML object
 */
svkSatBandsXML* svkDataAcquisitionDescriptionXML::GetSatBandsXML( )
{
    return this->satBandsXML;
}


/*!
 * Get the root XML data object.
 * \return the root XML data object.
 */
vtkXMLDataElement* svkDataAcquisitionDescriptionXML::GetRootXMLDataElement()
{
    return this->dataAcquisitionDescriptionXML;
}


/*!
 * C method for instantiating a new svk::svkDataAcquisitionDescriptionXML object.
 * This will initialized the root xml data element.
 *
 * \return a void pointer to the svk::svkDataAcquisitionDescriptionXML object
 *
 */
void* svkDataAcquisitionDescriptionXML_New()
{
    svkDataAcquisitionDescriptionXML* xml = svkDataAcquisitionDescriptionXML::New();
    xml->InitializeEmptyXMLFile();
    return xml;
}


/*!
 * C method for releasing an svk::svkDataAcquisitionDescriptionXML object.
 *
 * \param xml a void pointer to the svk::svkDataAcquisitionDescriptionXML to be deleted
 * \return always NULL
 */
void* svkDataAcquisitionDescriptionXML_Delete( void* xml )
{
    if( xml != NULL ) {
        ((svkDataAcquisitionDescriptionXML*)xml)->Delete();
    } else {
        printf(NULL_XML_ERROR);
    }
    return NULL;
}


/*!
 * C method for a new svk::svkDataAcquisitionDescriptionXML object and initializing
 * it from a file.
 *
 * \param xmlFileName the xml file to be read
 * \param status reference to a status variable. Will be set to 1 on error.
 * \return a void pointer to the svk::svkDataAcquisitionDescriptionXML object
 */
void* svkDataAcquisitionDescriptionXML_Read(const char* xmlFileName, int *status)
{
    svkDataAcquisitionDescriptionXML* xml = svkDataAcquisitionDescriptionXML::New();
    *status = xml->SetXMLFileName(xmlFileName);
    if (*status == 1 ) {
        xml->Delete();
        xml = NULL;
        printf("Error reading file: %s!\n", xmlFileName);
    }
    return ((void*)xml);
};



/*!
 * C method for writing the xml to disk.
 *
 * \param filepath the path to the file to be written.
 * \param xml a void pointer to the svk::svkDataAcquisitionDescriptionXML to be written
 */
int svkDataAcquisitionDescriptionXML_WriteXMLFile(const char* filepath, void* xml )
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->WriteXMLFile( filepath );
    } else {
        printf(NULL_XML_ERROR);
        return false;
    }
}


/*!
 * Generic getter for pulling the data or contents of an xml element.
 *
 * \param xml a void pointer to the svk::svkDataAcquisitionDescriptionXML object
 * \param path the xpath of the requested XML element
 * \return the contents of the requested element
 */
const char* svkDataAcquisitionDescriptionXML_GetDataWithPath( void* xml, const char* path )
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->GetDataWithPath( path );
    } else {
        printf(NULL_XML_ERROR);
        return NULL;
    }
}


/*!
 * Generic setter for the data or contents of a given xml element.
 *
 * \param xml a void pointer to the svk::svkDataAcquisitionDescriptionXML object
 * \param path the xpath of the requested element to set the contents of
 * \param data the data to set into the requested elment.
 * \return 0 on success
 */
int svkDataAcquisitionDescriptionXML_SetDataWithPath( void* xml, const char* path, const char* data )
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->SetDataWithPath( path, data );
    } else {
        printf(NULL_XML_ERROR);
        return -1;
    }
}


/*!
 * Add an xml element with the given parent path and name. The element will be
 * initially empty but its value should be set with
 * svkDataAcquisitionDescriptionXML_SetDataWithPath().
 *
 * \param xml a void pointer to the svk::svkDataAcquisitionDescriptionXML object
 * \param path the xpath of the parent to which you wish add the element to
 * \param name the name of the element to add
 * \return 0 on success
 */
int svkDataAcquisitionDescriptionXML_AddElementWithParentPath( void* xml, const char* path, const char* name )
{
    int status = -1;
    if( xml != NULL ) {
        void* elem = ((svkDataAcquisitionDescriptionXML*)xml)->AddElementWithParentPath( path, name );
        if( elem == NULL  ){
            status = -1;
        } else {
            status = 0;
        }
    } else {
        printf(NULL_XML_ERROR);
        status = -1;
    }
    return status;

}


/*!
 * Remove a data element with the given parent path and name.
 *
 * \param xml a void pointer to the svk::svkDataAcquisitionDescriptionXML object
 * \param path the xpath of the parent from which you wish to remove an element
 * \param name the name of the element to remove
 * \return 0 on success
 */
int svkDataAcquisitionDescriptionXML_RemoveElementWithParentPath( void* xml, const char* path, const char* name )
{
    int status = -1;
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->RemoveElementWithParentPath( path, name );
    } else {
        printf(NULL_XML_ERROR);
        status = -1;
    }
    return status;

}


/*!
 * C method for getting the internal svk::svkSatBandsXML object.
 * \param xml a void pointer to the svk::svkDataAcquisitionDescriptionXML object
 * \return a void pointer to the internal svk::svkSatBandsXML object.
 */
void* svkDataAcquisitionDescriptionXML_GetSatBandsXML( void* xml )
{
    void* satBandsXML = NULL;
    svkDataAcquisitionDescriptionXML* dadXML =(svkDataAcquisitionDescriptionXML*)xml;
    if( dadXML != NULL ) {
        satBandsXML = dadXML->GetSatBandsXML();
    } else {
        printf(NULL_XML_ERROR);
    }
    return satBandsXML;
}


/*!
 * C method for setting the basic trajectory element. This includes setting the
 * encoding/trajectory element the encoding/trajectoryDescription/identifier
 * element and the encoding/trajectoryDescription/comment element.
 * \param type the contents of the trajectory element
 * \param id the contents of the encoding/trajectoryDescription/identifier element
 * \param comment the contents of the encoding/trajectoryDescription/comment element
 * \param xml a void pointer to the svk::svkDataAcquisitionDescriptionXML
 */
void svkDataAcquisitionDescriptionXML_SetTrajectory(const char* type, const char* id, const char* comment, void* xml)
{
    if( xml != NULL ) {
        ((svkDataAcquisitionDescriptionXML*)xml)->SetTrajectoryType( type );
        ((svkDataAcquisitionDescriptionXML*)xml)->SetTrajectoryID( id );
        ((svkDataAcquisitionDescriptionXML*)xml)->SetTrajectoryComment( comment );
    } else {
        printf(NULL_XML_ERROR);
    }
}


/*!
 * C method for getting the contents of the encoding/trajectory element.
 *
 * \param xml a void pointer to the svk::svkDataAcquisitionDescriptionXML object
 * \return the contents of the encoding/trajectory element
 */
const char* svkDataAcquisitionDescriptionXML_GetTrajectoryType(void* xml)
{
    if( xml != NULL ) {
        return (((svkDataAcquisitionDescriptionXML*)xml)->GetTrajectoryType( )).c_str();
    } else {
        printf(NULL_XML_ERROR);
        return NULL;
    }
}


/*!
 * C method for getting the contents of the encoding/trajectoryDescription/identifier element.
 * \param xml a void pointer to the svk::svkDataAcquisitionDescriptionXML object
 * \return the contents of the encoding/trajectoryDescription/identifier element
 */
const char* svkDataAcquisitionDescriptionXML_GetTrajectoryID(void* xml)
{
    if( xml != NULL ) {
        return (((svkDataAcquisitionDescriptionXML*)xml)->GetTrajectoryID( )).c_str();
    } else {
        printf(NULL_XML_ERROR);
        return NULL;
    }
}


/*!
 * C method for getting the contents of the encoding/trajectoryDescription/comment element.
 * \param xml a void pointer to the svk::svkDataAcquisitionDescriptionXML object
 * \return the contents of the encoding/trajectoryDescription/comment element
 */
const char* svkDataAcquisitionDescriptionXML_GetTrajectoryComment(void* xml)
{
    if( xml != NULL ) {
        return (((svkDataAcquisitionDescriptionXML*)xml)->GetTrajectoryComment( )).c_str();
    } else {
        printf(NULL_XML_ERROR);
        return NULL;
    }
}


/*!
 * C method for setting the contents of an encoding/trajectoryDescription/userParameterLong
 * element. This includes setting name and value child elements.
 * \param name the name of the user parameter
 * \param value the value of the user parameter
 * \param xml a void pointer to the svk::svkDataAcquisitionDescriptionXML
 */
void svkDataAcquisitionDescriptionXML_SetTrajectoryLongParameter(const char* name, long value, void* xml )
{
    if( xml != NULL ) {
        ((svkDataAcquisitionDescriptionXML*)xml)->SetTrajectoryParameter( name, value );
    } else {
        printf(NULL_XML_ERROR);
    }
}


/*!
 * C method for getting the contents of an encoding/trajectoryDescription/userParameterLong
 * element.
 * \param name the name of the user parameter
 * \param xml a void pointer to the svk::svkDataAcquisitionDescriptionXML
 * \return the value of the user element as a long
 */
long svkDataAcquisitionDescriptionXML_GetTrajectoryLongParameter(const char* name, void* xml )
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->GetTrajectoryLongParameter( name );
    } else {
        printf(NULL_XML_ERROR);
        return VTK_LONG_MIN;
    }
}


/*!
 * C method for setting the contents of an encoding/trajectoryDescription/userParameterDouble
 * element. This includes setting name and value child elements.
 * \param name the name of the user parameter
 * \param value the value of the user parameter
 * \param xml a void pointer to the svk::svkDataAcquisitionDescriptionXML
 */
void svkDataAcquisitionDescriptionXML_SetTrajectoryDoubleParameter(const char* name, double value, void* xml )
{
    if( xml != NULL ) {
        ((svkDataAcquisitionDescriptionXML*)xml)->SetTrajectoryParameter( name, value );
    } else {
        printf(NULL_XML_ERROR);
    }
}



/*!
 * C method for getting the contents of an encoding/trajectoryDescription/userParameterDouble
 * element.
 * \param name the name of the user parameter
 * \param xml a void pointer to the svk::svkDataAcquisitionDescriptionXML
 * \return the value of the user element as a double
 */
double svkDataAcquisitionDescriptionXML_GetTrajectoryDoubleParameter(const char* name, void* xml )
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->GetTrajectoryDoubleParameter( name );
    } else {
        printf(NULL_XML_ERROR);
        return VTK_DOUBLE_MIN;
    }
}
