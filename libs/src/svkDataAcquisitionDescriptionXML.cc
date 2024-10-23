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
#include </usr/include/vtk/vtkXMLUtilities.h>
#include </usr/include/vtk/vtkXMLDataParser.h>
#include </usr/include/vtk/vtkMath.h>
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
 *
 * @return
 */
int svkDataAcquisitionDescriptionXML::GetTrajectoryNumberOfDimensions()
{
    string parentPath = "encoding/trajectoryDescription/dimensions";
    vtkXMLDataElement* parentElement = svkXMLUtils::FindNestedElementWithPath(this->GetRootXMLDataElement(), parentPath);
    if( parentElement != NULL ) {
        return parentElement->GetNumberOfNestedElements();
    } else {
        return -1;
    }
}


/*!
 *
 * @param id
 * @param logical
 * @param description
 */
void svkDataAcquisitionDescriptionXML::AddTrajectoryDimension(string id, string logical, string description)
{
    string parentPath = "encoding/trajectoryDescription/dimensions";
    string elementName = parentPath;
    elementName.append("/dim");
    vtkXMLDataElement* dimElement = this->AddElementWithParentPath(parentPath.c_str(), "dim");
    dimElement->SetAttribute("id", id.c_str());
    if( !logical.empty()) {
        vtkXMLDataElement* logicalElement = vtkXMLDataElement::New();
        logicalElement->SetCharacterData(logical.c_str(), logical.size());
        logicalElement->SetName("logical");
        dimElement->AddNestedElement(logicalElement);
        logicalElement->Delete();
    }
    if( !description.empty()) {
        vtkXMLDataElement* descriptionElement = vtkXMLDataElement::New();
        descriptionElement->SetCharacterData(description.c_str(), description.size());
        descriptionElement->SetName("description");
        dimElement->AddNestedElement(descriptionElement);
        descriptionElement->Delete();
    }
}


/*!
 *
 * @param index
 * @return
 */
string svkDataAcquisitionDescriptionXML::GetTrajectoryDimensionId(int index)
{
    string parentPath = "encoding/trajectoryDescription/dimensions/";
    vtkXMLDataElement* element = this->GetNestedElementByIndexWithParentPath(index, parentPath.c_str());
    if( element != NULL) {
        return element->GetAttribute("id");
    }
    return "";
}


/*!
 *
 * @param index
 * @return
 */
string svkDataAcquisitionDescriptionXML::GetTrajectoryDimensionLogical(int index)
{
    string parentPath = "encoding/trajectoryDescription/dimensions/";
    vtkXMLDataElement* element = this->GetNestedElementByIndexWithParentPath(index, parentPath.c_str());
    if( element != NULL ) {
        vtkXMLDataElement* logical = svkXMLUtils::FindNestedElementWithPath( element, "logical");
        if( logical != NULL ) {
            return logical->GetCharacterData();
        }
    }
    return "";
}


/*!
 *
 * @param index
 * @return
 */
string svkDataAcquisitionDescriptionXML::GetTrajectoryDimensionDescription(int index)
{
    string parentPath = "encoding/trajectoryDescription/dimensions/";
    vtkXMLDataElement* element = this->GetNestedElementByIndexWithParentPath(index, parentPath.c_str());
    if( element != NULL ) {
        vtkXMLDataElement* description = svkXMLUtils::FindNestedElementWithPath( element, "description");
        if( description != NULL ) {
            return description->GetCharacterData();
        }
    }
    return "";
}


/*!
 *
 * @param type
 */
void svkDataAcquisitionDescriptionXML::SetEPSIType( EPSIType type ) {

    string parentPath = "encoding/trajectoryDescription/epsiEncoding";
    // Thes meethod coll ensures the EPSI encoding element is created already.
    this->GetEPSIEncodingElement();

    string elementName = parentPath;
    elementName.append("/epsiType");
    vtkXMLDataElement* epsiTypeElement = this->FindNestedElementWithPath(elementName);
    if( epsiTypeElement == NULL ) {
        epsiTypeElement = this->AddElementWithParentPath(parentPath.c_str(), "epsiType");
    }
    if( type == FLYBACK ) {
        epsiTypeElement->SetCharacterData("FLYBACK", 7);
    } else if (type == SYMMETRIC) {
        epsiTypeElement->SetCharacterData("SYMMETRIC", 9);
    } else if (type == INTERLEAVED) {
        epsiTypeElement->SetCharacterData("INTERLEAVED", 11);
    } else {
        epsiTypeElement->SetCharacterData("UNDEFINED_EPSI_TYPE", 19);
    }
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
 *
 * @return
 */
vtkXMLDataElement* svkDataAcquisitionDescriptionXML::GetEPSIEncodingElement()
{
    string parentPath = "encoding/trajectoryDescription";
    string elementPath = "encoding/trajectoryDescription/epsiEncoding";
    vtkXMLDataElement* epsiEncodingElement = this->FindNestedElementWithPath(elementPath);
    if( epsiEncodingElement == NULL ) {
        epsiEncodingElement = this->AddElementWithParentPath(parentPath.c_str(), "epsiEncoding");
    }
    return epsiEncodingElement;
}


/*!
 * Initialize the skeleton of the xml file. This will create the following elements:
 * svk_data_acquisition_description
 * svk_data_acquisition_description/version
 * svk_data_acquisition_description/encoding
 * svk_data_acquisition_description/encoding/trajectory
 * svk_data_acquisition_description/encoding/trajectoryDescription
 * svk_data_acquisition_description/encoding/trajectoryDescription/identifier
 * svk_data_acquisition_description/encoding/trajectoryDescription/dimensions
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
    vtkXMLDataElement* trajectoryDimensionElem = svkXMLUtils::CreateNestedXMLDataElement(
            trajectoryDescElem, "dimensions" , "" );
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
    trajectoryDimensionElem->Delete();
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
    //  check for svk_data_acquisition_description element: 
    string dadRootName = this->dataAcquisitionDescriptionXML->GetRoot()->GetName();
    cout << "NAME: " <<  dadRootName << endl; 
    if (dadRootName.compare("svk_data_acquisition_description") != 0 )  {
        cout << "NOT A DAD FILE" << endl;
        return 1;
    }

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
                    errorFound = false;
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
    return elem;
}


/*!
 *
 * @param parentPath
 * @param elementName
 * @return
 */
vtkXMLDataElement* svkDataAcquisitionDescriptionXML::FindOrCreateNestedElementWithPath( string parentPath, string elementName)
{
    return svkXMLUtils::FindOrCreateNestedElementWithPath(this->dataAcquisitionDescriptionXML, parentPath, elementName);
}


/*!
 * Returns the character data at a specific path. Reports an error if the path is not found.
 *  \param xmlPath the xpath to the element
 *  \return the contents of the element at the given path
 */
string svkDataAcquisitionDescriptionXML::GetDataWithPath( const char* xmlPath )
{
    string data = "";
    bool foundData = svkXMLUtils::GetNestedElementCharacterDataWithPath( this->dataAcquisitionDescriptionXML, xmlPath, &data );
    if( !foundData ) {
        cout << "ERROR: Could not get character data at path: " << xmlPath << endl;
    }
    return data;
}


/*!
 *
 * @param index
 * @param parentPath
 * @return
 */
int svkDataAcquisitionDescriptionXML::GetIntByIndexWithParentPath( int index, const char* parentPath )
{
    vtkXMLDataElement* parentElement = svkXMLUtils::FindNestedElementWithPath(this->GetRootXMLDataElement(), parentPath);
    vtkXMLDataElement* element = parentElement->GetNestedElement(index);
    string data = element->GetCharacterData();
    if( data.compare("") != 0 ){
        return svkTypeUtils::StringToInt(data);
    }
    return -1;
}


/*!
 *
 * @param elementPath
 * @return
 */
int svkDataAcquisitionDescriptionXML::GetIntWithPath( const char* elementPath )
{
    string data = this->GetDataWithPath( elementPath );
    if( data.compare("") != 0 ){
        return svkTypeUtils::StringToInt(data);
    }
    return -1;
}


/*!
 *
 * @param parentPath
 * @param elementName
 * @param value
 */
void svkDataAcquisitionDescriptionXML::SetIntWithPath( const char* parentPath, const char* elementName, int value)
{
    vtkXMLDataElement* elem = this->FindOrCreateNestedElementWithPath( parentPath, elementName );
    string data = svkTypeUtils::IntToString(value);
    if( elem != NULL ){
        elem->SetCharacterData(data.c_str(), data.size());
    }
}


/*!
 *
 * @param elementPath
 * @return
 */
float svkDataAcquisitionDescriptionXML::GetFloatWithPath( const char* elementPath )
{
    string data = this->GetDataWithPath( elementPath );
    if( data.compare("") != 0 ){
        return svkTypeUtils::StringToFloat(data);
    }
    return -1;
}


/*!
 *
 * @param parentPath
 * @param elementName
 * @param value
 */
void svkDataAcquisitionDescriptionXML::SetFloatWithPath( const char* parentPath, const char* elementName, float value)
{
    vtkXMLDataElement* elem = this->FindOrCreateNestedElementWithPath( parentPath, elementName );
    string data = svkTypeUtils::DoubleToString(value);
    if( elem != NULL ){
        elem->SetCharacterData(data.c_str(), data.size());
    }
}


/*!
 *
 * @param index
 * @param parentPath
 * @return
 */
string svkDataAcquisitionDescriptionXML::GetDataByIndexWithParentPath( int index, const char* parentPath )
{
    vtkXMLDataElement* element = this->GetNestedElementByIndexWithParentPath(index, parentPath);
    if( element != NULL ) {
        return element->GetName();
    }
    return NULL;
}


/*!
 *
 * @param index
 * @param parentPath
 * @return
 */
vtkXMLDataElement* svkDataAcquisitionDescriptionXML::GetNestedElementByIndexWithParentPath( int index, const char* parentPath )
{
    vtkXMLDataElement* element = NULL;
    string data = this->GetDataWithPath( parentPath );
    vtkXMLDataElement* parentElement = svkXMLUtils::FindNestedElementWithPath(this->GetRootXMLDataElement(), parentPath);
    if( parentElement != NULL ) {
        element = parentElement->GetNestedElement(index);
    }

    return element;
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
 *
 * @param parentPath
 * @param elementName
 * @param value
 */
void svkDataAcquisitionDescriptionXML::SetDataWithPath( const char* parentPath, const char* elementName, string value)
{
    vtkXMLDataElement* elem = this->FindOrCreateNestedElementWithPath( parentPath, elementName );
    if( elem != NULL ){
        elem->SetCharacterData(value.c_str(), value.size());
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
 *
 * @return
 */
EPSIType svkDataAcquisitionDescriptionXML::GetEPSIType()
{
    string epsiTypeFromDad = this->GetDataWithPath("/encoding/trajectoryDescription/epsiEncoding/epsiType");
    EPSIType epsiType = UNDEFINED_EPSI_TYPE;
    if( epsiTypeFromDad.compare("SYMMETRIC") == 0 ){
        epsiType = SYMMETRIC;
    } else if ( epsiTypeFromDad.compare("FLYBACK") == 0 ) {
        epsiType = FLYBACK;
    } else if (epsiTypeFromDad.compare("INTERLEAVED") == 0 ) {
        epsiType = INTERLEAVED;
    }

    return epsiType;
}


/*!
 *
 * @return
 */
string svkDataAcquisitionDescriptionXML::GetEPSITypeString()
{
    string epsiTypeFromDad = this->GetDataWithPath("/encoding/trajectoryDescription/epsiEncoding/epsiType");
    return epsiTypeFromDad;
}


/*!
 *
 * @param numberOfInterleaves
 */
void svkDataAcquisitionDescriptionXML::SetEPSINumberOfInterleaves( int numberOfInterleaves )
{
    this->SetIntWithPath("/encoding/trajectoryDescription/epsiEncoding", "numInterleaves", numberOfInterleaves);
}


/*!
 *
 * @return
 */
int svkDataAcquisitionDescriptionXML::GetEPSINumberOfInterleaves( )
{
    return this->GetIntWithPath("/encoding/trajectoryDescription/epsiEncoding/numInterleaves");
}


/*!
 *
 * @param gradientAmplitude
 * @param lobe
 */
void svkDataAcquisitionDescriptionXML::SetEPSIGradientAmplitude( float gradientAmplitude, enum EPSILobe lobe )
{
    string element = "";
    if( lobe == ODD ) {
        element = "gradientAmplitudeOddMTM";
    } else if( lobe == EVEN ) {
        element = "gradientAmplitudeEvenMTM";
    } else {
        cout << "ERROR: EPSI lobe must be either EVEN or ODD." << endl;
        return;
    }
    this->SetFloatWithPath("/encoding/trajectoryDescription/epsiEncoding", element.c_str(), gradientAmplitude);

}


/*!
 *
 * @param lobe
 * @return
 */
float svkDataAcquisitionDescriptionXML::GetEPSIGradientAmplitude( enum EPSILobe lobe )
{
    string path = "/encoding/trajectoryDescription/epsiEncoding/";
    if( lobe == ODD ) {
        path.append("gradientAmplitudeOddMTM");
    } else if( lobe == EVEN ) {
        path.append("gradientAmplitudeEvenMTM");
    } else {
        cout << "ERROR: EPSI lobe must be either EVEN or ODD." << endl;
        return 0;
    }
    return this->GetFloatWithPath(path.c_str());

}


/*!
 *
 * @param rampDuration
 * @param lobe
 */
void svkDataAcquisitionDescriptionXML::SetEPSIRampDuration( float rampDuration, enum EPSILobe lobe )
{
    string element = "";
    if( lobe == ODD ) {
        element = "rampDurationOddMs";
    } else if( lobe == EVEN ) {
        element = "rampDurationEvenMs";
    } else {
        cout << "ERROR: EPSI lobe must be either EVEN or ODD." << endl;
        return;
    }
    this->SetFloatWithPath("/encoding/trajectoryDescription/epsiEncoding", element.c_str(), rampDuration);

}


/*!
 *
 * @param lobe
 * @return
 */
float svkDataAcquisitionDescriptionXML::GetEPSIRampDuration( enum EPSILobe lobe )
{
    string path = "/encoding/trajectoryDescription/epsiEncoding/";
    if( lobe == ODD ) {
        path.append("rampDurationOddMs");
    } else if( lobe == EVEN ) {
        path.append("rampDurationEvenMs");
    } else {
        cout << "ERROR: EPSI lobe must be either EVEN or ODD." << endl;
        return 0;
    }
    return this->GetFloatWithPath(path.c_str());

}


/*!
 *
 * @param plateauDuration
 * @param lobe
 */
void svkDataAcquisitionDescriptionXML::SetEPSIPlateauDuration( float plateauDuration, enum EPSILobe lobe )
{
    string element = "";
    if( lobe == ODD ) {
        element = "plateauDurationOddMs";
    } else if( lobe == EVEN ) {
        element = "plateauDurationEvenMs";
    } else {
        cout << "ERROR: EPSI lobe must be either EVEN or ODD." << endl;
        return;
    }
    this->SetFloatWithPath("/encoding/trajectoryDescription/epsiEncoding", element.c_str(), plateauDuration);

}


/*!
 *
 * @param lobe
 * @return
 */
float svkDataAcquisitionDescriptionXML::GetEPSIPlateauDuration( enum EPSILobe lobe )
{
    string path = "/encoding/trajectoryDescription/epsiEncoding/";
    if( lobe == ODD ) {
        path.append("plateauDurationOddMs");
    } else if( lobe == EVEN ) {
        path.append("plateauDurationEvenMs");
    } else {
        cout << "ERROR: EPSI lobe must be either EVEN or ODD." << endl;
        return 0;
    }
    return this->GetFloatWithPath(path.c_str());

}


/*!
 *
 * @param numberOfLobes
 * @param lobe
 */
void svkDataAcquisitionDescriptionXML::SetEPSINumberOfLobes( int numberOfLobes, enum EPSILobe lobe )
{
    string element = "";
    if( lobe == ODD ) {
        element = "numberOfLobesOdd";
    } else if( lobe == EVEN ) {
        element = "numberOfLobesEven";
    } else {
        cout << "ERROR: EPSI lobe must be either EVEN or ODD." << endl;
        return;
    }
    this->SetIntWithPath("/encoding/trajectoryDescription/epsiEncoding", element.c_str(), numberOfLobes);

}


/*!
 *
 * @param lobe
 * @return
 */
int svkDataAcquisitionDescriptionXML::GetEPSINumberOfLobes( enum EPSILobe lobe )
{
    string path = "/encoding/trajectoryDescription/epsiEncoding/";
    if( lobe == ODD ) {
        path.append("numberOfLobesOdd");
    } else if( lobe == EVEN ) {
        path.append("numberOfLobesEven");
    } else {
        cout << "ERROR: EPSI lobe must be either EVEN or ODD." << endl;
        return 0;
    }
    return this->GetIntWithPath(path.c_str());

}


/*!
 *
 * @param sampleSpacing
 */
void svkDataAcquisitionDescriptionXML::SetEPSISampleSpacing( float sampleSpacing )
{
    this->SetFloatWithPath("/encoding/trajectoryDescription/epsiEncoding", "sampleSpacingTimeMs", sampleSpacing);
}


/*!
 *
 * @return
 */
float svkDataAcquisitionDescriptionXML::GetEPSISampleSpacing( )
{
    string path = "/encoding/trajectoryDescription/epsiEncoding/sampleSpacingTimeMs";
    return this->GetFloatWithPath(path.c_str());
}


/*!
 *
 * @param acquisitionDelay
 */
void svkDataAcquisitionDescriptionXML::SetEPSIAcquisitionDelay( float acquisitionDelay )
{
    this->SetFloatWithPath("/encoding/trajectoryDescription/epsiEncoding", "acquisitionDelayTimeMs", acquisitionDelay);
}


/*!
 *
 * @return
 */
float svkDataAcquisitionDescriptionXML::GetEPSIAcquisitionDelay( ) {
    string path = "/encoding/trajectoryDescription/epsiEncoding/acquisitionDelayTimeMs";
    return this->GetFloatWithPath(path.c_str());
}


/*!
 *
 * @param echoDelay
 */
void svkDataAcquisitionDescriptionXML::SetEPSIEchoDelay( float echoDelay )
{
    this->SetFloatWithPath("/encoding/trajectoryDescription/epsiEncoding", "echoDelayTimeMs", echoDelay);
}


/*!
 *
 * @return
 */
float svkDataAcquisitionDescriptionXML::GetEPSIEchoDelay( ) {
    string path = "/encoding/trajectoryDescription/epsiEncoding/echoDelayTimeMs";
    return this->GetFloatWithPath(path.c_str());
}


/*!
 *
 * @param gradientAxis
 */
void svkDataAcquisitionDescriptionXML::SetEPSIGradientAxis( int gradientAxis )
{
    vtkXMLDataElement* element = this->GetNestedElementByIndexWithParentPath(gradientAxis - 1, "encoding/trajectoryDescription/dimensions");
    string idName = element->GetAttribute("id");
    this->SetDataWithPath("/encoding/trajectoryDescription/epsiEncoding", "gradientAxis", idName.c_str());
}


/*!
 *
 * @return
 */
string svkDataAcquisitionDescriptionXML::GetEPSIGradientAxisId( )
{
    return this->GetDataWithPath( "/encoding/trajectoryDescription/epsiEncoding/gradientAxis" );
}


/*!
 *
 * @return
 */
int svkDataAcquisitionDescriptionXML::GetEPSIGradientAxisIndex( )
{

    vtkXMLDataElement* dimensionsElement = this->FindNestedElementWithPath("encoding/trajectoryDescription/dimensions");
    int numberOfDimensions = this->GetTrajectoryNumberOfDimensions();
    for( int i = 0; i < numberOfDimensions; i++ ) {
        vtkXMLDataElement* element = dimensionsElement->GetNestedElement(i);
        string idName = element->GetAttribute("id");
        if( idName.compare( this->GetEPSIGradientAxisId() ) == 0) {
            return i + 1;
        }

    }
    return -1;
}


/*!
 *
 * @param name
 * @param value
 */
void svkDataAcquisitionDescriptionXML::AddEncodedMatrixSizeDimension( string name, int value  )
{
    string parentPath = "encoding/encodedSpace/matrixSize";
    string elementName = parentPath;
    elementName.append("/");
    elementName.append(name);
    vtkXMLDataElement* elem = this->FindNestedElementWithPath(elementName);
    if (elem == NULL ) {
        this->AddElementWithParentPath(parentPath.c_str(), name.c_str());

    }
    this->SetDataWithPath(elementName.c_str(), svkTypeUtils::IntToString(value).c_str());
}


/*!
 *
 * @param index
 * @return
 */
int svkDataAcquisitionDescriptionXML::GetEncodedMatrixSizeDimensionValue(int index)
{
    string parentPath = "encoding/encodedSpace/matrixSize";
    return this->GetIntByIndexWithParentPath(index, parentPath.c_str());
}


/*!
 *
 * @param index
 * @return
 */
string svkDataAcquisitionDescriptionXML::GetEncodedMatrixSizeDimensionName(int index)
{
    string parentPath = "encoding/encodedSpace/matrixSize";
    return this->GetDataByIndexWithParentPath(index, parentPath.c_str());
}


/*!
 *
 * @return
 */
int svkDataAcquisitionDescriptionXML::GetEncodedMatrixSizeNumberOfDimensions()
{
    string parentPath = "encoding/encodedSpace/matrixSize";
    vtkXMLDataElement* parentElement = svkXMLUtils::FindNestedElementWithPath(this->GetRootXMLDataElement(), parentPath);
    return parentElement->GetNumberOfNestedElements();
}


void svkDataAcquisitionDescriptionXML::GetSamplingIndicies(int *indicies) {
    string indiciesString = this->GetDataWithPath("/encoding/trajectoryDescription/epsiEncoding/indicies");
    vector<string> splitString = svkUtils::SplitString(indiciesString, " ");

    for( int i = 0; i < splitString.size(); i++){
        // TODO: Make array 0 index array
        indicies[i] = svkTypeUtils::StringToInt(splitString[i])-1;
    }
}

void svkDataAcquisitionDescriptionXML::GetSamplingMask(int *samplingMask) {
    string samplingMaskString = this->GetDataWithPath("/encoding/trajectoryDescription/epsiEncoding/samplingMask");
    vector<string> splitString = svkUtils::SplitString(samplingMaskString, " ");

    for( int i = 0; i < splitString.size(); i++){
        samplingMask[i] = svkTypeUtils::StringToInt(splitString[i]);
    }
}

void svkDataAcquisitionDescriptionXML::GetBlips( int index, string blipDimension, int* blips) {
    string xmlPath = "/encoding/trajectoryDescription/epsiEncoding/blips[index=";
    xmlPath.append(svkTypeUtils::IntToString(index));
    xmlPath.append("]/d");
    xmlPath.append(blipDimension);
    string blipsString = this->GetDataWithPath(xmlPath.c_str());
    vector<string> splitString = svkUtils::SplitString(blipsString, " ");

    for( int i = 0; i < splitString.size(); i++){
        blips[i] = svkTypeUtils::StringToInt(splitString[i]);
    }
}



/*!
 *
 * @return
 */
svkCString GetEmptySvkCString()
{
    svkCString data;
    data.c_str[0] = '\0';
    return data;
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
svkCString svkDataAcquisitionDescriptionXML_GetDataWithPath( void* xml, const char* path )
{
    svkCString data;
    if( xml != NULL ) {
        string stringData = ((svkDataAcquisitionDescriptionXML*)xml)->GetDataWithPath( path );
        strcpy(data.c_str, stringData.c_str());
    }
    return data;
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
svkCString svkDataAcquisitionDescriptionXML_GetTrajectoryType(void* xml)
{
    svkCString data;
    if( xml != NULL ) {
        string stringData = (((svkDataAcquisitionDescriptionXML*)xml)->GetTrajectoryType( )).c_str();
        strcpy(data.c_str, stringData.c_str());
    } else {
        printf(NULL_XML_ERROR);
    }
    return data;
}


/*!
 * C method for getting the contents of the encoding/trajectoryDescription/identifier element.
 * \param xml a void pointer to the svk::svkDataAcquisitionDescriptionXML object
 * \return the contents of the encoding/trajectoryDescription/identifier element
 */
svkCString svkDataAcquisitionDescriptionXML_GetTrajectoryID(void* xml)
{
    svkCString data;
    if( xml != NULL ) {
        string stringData = (((svkDataAcquisitionDescriptionXML*)xml)->GetTrajectoryID( )).c_str();
        strcpy(data.c_str, stringData.c_str());
    } else {
        printf(NULL_XML_ERROR);
    }
    return data;
}


/*!
 *
 * @param id
 * @param logical
 * @param description
 * @param xml
 */
void svkDataAcquisitionDescriptionXML_AddTrajectoryDimension(const char* id, const char* logical,
                                                                          const char* description, void *xml)
{
    if( xml != NULL ) {
        ((svkDataAcquisitionDescriptionXML*)xml)->AddTrajectoryDimension( id, logical, description );
    } else {
        printf(NULL_XML_ERROR);
    }
}


/*!
 * C method for getting the contents of the encoding/trajectoryDescription/comment element.
 * \param xml a void pointer to the svk::svkDataAcquisitionDescriptionXML object
 * \return the contents of the encoding/trajectoryDescription/comment element
 */
svkCString svkDataAcquisitionDescriptionXML_GetTrajectoryComment(void* xml)
{
    svkCString data;
    if( xml != NULL ) {
        string stringData = (((svkDataAcquisitionDescriptionXML*)xml)->GetTrajectoryComment( )).c_str();
        strcpy(data.c_str, stringData.c_str());
    } else {
        printf(NULL_XML_ERROR);
    }
    return data;
}


/*!
 *
 * @param xml
 * @return
 */
int svkDataAcquisitionDescriptionXML_GetTrajectoryNumberOfDimensions(void *xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->GetTrajectoryNumberOfDimensions();
    } else {
        printf(NULL_XML_ERROR);
        return -1;
    }
}


/*!
 *
 * @param index
 * @param xml
 * @return
 */
struct svkCString svkDataAcquisitionDescriptionXML_GetTrajectoryDimensionId(int index, void *xml) {
    svkCString data = GetEmptySvkCString();
    if( xml != NULL ) {
        string stringData = ((svkDataAcquisitionDescriptionXML*)xml)->GetTrajectoryDimensionId(index);
        strcpy(data.c_str, stringData.c_str());
    } else {
        printf(NULL_XML_ERROR);
    }
    return data;
}


/*!
 *
 * @param index
 * @param xml
 * @return
 */
struct svkCString svkDataAcquisitionDescriptionXML_GetTrajectoryDimensionLogical(int index, void *xml) {

    svkCString data = GetEmptySvkCString();
    if( xml != NULL ) {
        string stringData = ((svkDataAcquisitionDescriptionXML*)xml)->GetTrajectoryDimensionLogical(index);
        strcpy(data.c_str, stringData.c_str());
    } else {
        printf(NULL_XML_ERROR);
    }
    return data;

}


/*!
 *
 * @param index
 * @param xml
 * @return
 */
struct svkCString svkDataAcquisitionDescriptionXML_GetTrajectoryDimensionDescription(int index, void *xml) {
    svkCString data = GetEmptySvkCString();
    if( xml != NULL ) {
        string stringData = ((svkDataAcquisitionDescriptionXML*)xml)->GetTrajectoryDimensionDescription(index);
        strcpy(data.c_str, stringData.c_str());
    } else {
        printf(NULL_XML_ERROR);
    }
    return data;
}


/*!
 *
 * @param xml
 */
void svkDataAcquisitionDescriptionXML_SetEPSITypeToFlyback(void* xml) {
    svkDataAcquisitionDescriptionXML_SetEPSIType( FLYBACK, xml );
}


/*!
 *
 * @param xml
 */
void svkDataAcquisitionDescriptionXML_SetEPSITypeToSymmetric(void* xml) {
    svkDataAcquisitionDescriptionXML_SetEPSIType( SYMMETRIC, xml );
}


/*!
 *
 * @param type
 * @param xml
 */
void svkDataAcquisitionDescriptionXML_SetEPSIType( EPSIType type, void* xml) {
    if( xml != NULL ) {
        ((svkDataAcquisitionDescriptionXML*)xml)->SetEPSIType( type );
    } else {
        printf(NULL_XML_ERROR);
    }
}


/*!
 *
 * @param xml
 * @return
 */
EPSIType svkDataAcquisitionDescriptionXML_GetEPSIType(void* xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->GetEPSIType( );
    } else {
        printf(NULL_XML_ERROR);
    }
    return UNDEFINED_EPSI_TYPE;
}


/*!
 *
 * @param numberOfInterleaves
 * @param xml
 */
void svkDataAcquisitionDescriptionXML_SetEPSINumberOfInterleaves( int numberOfInterleaves, void* xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->SetEPSINumberOfInterleaves( numberOfInterleaves );
    } else {
        printf(NULL_XML_ERROR);
    }
}


/*!
 *
 * @param xml
 * @return
 */
int svkDataAcquisitionDescriptionXML_GetEPSINumberOfInterleaves( void* xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->GetEPSINumberOfInterleaves( );
    } else {
        printf(NULL_XML_ERROR);
        return -1;
    }
}


/*!
 *
 * @param amplitude
 * @param lobe
 * @param xml
 */
void svkDataAcquisitionDescriptionXML_SetEPSIGradientAmplitude(float amplitude, enum EPSILobe lobe, void* xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->SetEPSIGradientAmplitude( amplitude, lobe );
    } else {
        printf(NULL_XML_ERROR);
    }
}


/*!
 *
 * @param lobe
 * @param xml
 * @return
 */
float svkDataAcquisitionDescriptionXML_GetEPSIGradientAmplitude(enum EPSILobe lobe, void *xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->GetEPSIGradientAmplitude( lobe );
    } else {
        printf(NULL_XML_ERROR);
        return 0;
    }
}


/*!
 *
 * @param duration
 * @param lobe
 * @param xml
 */
void svkDataAcquisitionDescriptionXML_SetEPSIRampDuration(float duration, enum EPSILobe lobe, void* xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->SetEPSIRampDuration( duration, lobe );
    } else {
        printf(NULL_XML_ERROR);
    }
}


/*!
 *
 * @param lobe
 * @param xml
 * @return
 */
float svkDataAcquisitionDescriptionXML_GetEPSIRampDuration(enum EPSILobe lobe, void *xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->GetEPSIRampDuration( lobe );
    } else {
        printf(NULL_XML_ERROR);
        return 0;
    }
}


/*!
 *
 * @param duration
 * @param lobe
 * @param xml
 */
void svkDataAcquisitionDescriptionXML_SetEPSIPlateauDuration(float duration, enum EPSILobe lobe, void* xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->SetEPSIPlateauDuration( duration, lobe );
    } else {
        printf(NULL_XML_ERROR);
    }
}


/*!
 *
 * @param lobe
 * @param xml
 * @return
 */
float svkDataAcquisitionDescriptionXML_GetEPSIPlateauDuration(enum EPSILobe lobe, void *xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->GetEPSIPlateauDuration( lobe );
    } else {
        printf(NULL_XML_ERROR);
        return 0;
    }
}


/*!
 *
 * @param numberOfLobes
 * @param lobe
 * @param xml
 */
void svkDataAcquisitionDescriptionXML_SetEPSINumberOfLobes(int numberOfLobes, enum EPSILobe lobe, void* xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->SetEPSINumberOfLobes( numberOfLobes, lobe );
    } else {
        printf(NULL_XML_ERROR);
    }
}


/*!
 *
 * @param lobe
 * @param xml
 * @return
 */
float svkDataAcquisitionDescriptionXML_GetEPSINumberOfLobes(enum EPSILobe lobe, void *xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->GetEPSINumberOfLobes( lobe );
    } else {
        printf(NULL_XML_ERROR);
        return 0;
    }
}


/*!
 *
 * @param sampleSpacing
 * @param xml
 */
void svkDataAcquisitionDescriptionXML_SetEPSISampleSpacing(float sampleSpacing, void* xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->SetEPSISampleSpacing( sampleSpacing );
    } else {
        printf(NULL_XML_ERROR);
    }
}


/*!
 *
 * @param xml
 * @return
 */
float svkDataAcquisitionDescriptionXML_GetEPSISampleSpacing(void *xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->GetEPSISampleSpacing();
    } else {
        printf(NULL_XML_ERROR);
        return 0;
    }
}


/*!
 *
 * @param acquisitionDelay
 * @param xml
 */
void svkDataAcquisitionDescriptionXML_SetEPSIAcquisitionDelay(float acquisitionDelay, void* xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->SetEPSIAcquisitionDelay( acquisitionDelay );
    } else {
        printf(NULL_XML_ERROR);
    }
}


/*!
 *
 * @param xml
 * @return
 */
float svkDataAcquisitionDescriptionXML_GetEPSIAcquisitionDelay(void *xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->GetEPSIAcquisitionDelay();
    } else {
        printf(NULL_XML_ERROR);
        return 0;
    }
}


/*!
 *
 * @param echoDelay
 * @param xml
 */
void svkDataAcquisitionDescriptionXML_SetEPSIEchoDelay(float echoDelay, void* xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->SetEPSIEchoDelay( echoDelay );
    } else {
        printf(NULL_XML_ERROR);
    }
}


/*!
 *
 * @param xml
 * @return
 */
float svkDataAcquisitionDescriptionXML_GetEPSIEchoDelay(void *xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->GetEPSIEchoDelay();
    } else {
        printf(NULL_XML_ERROR);
        return 0;
    }
}


/*!
 *
 * @param gradientAxis
 * @param xml
 */
void svkDataAcquisitionDescriptionXML_SetEPSIGradientAxis(int gradientAxis, void* xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->SetEPSIGradientAxis( gradientAxis );
    } else {
        printf(NULL_XML_ERROR);
    }
}


/*!
 *
 * @param xml
 * @return
 */
struct svkCString svkDataAcquisitionDescriptionXML_GetEPSIGradientAxisId(void *xml)
{
    svkCString data = GetEmptySvkCString();
    if( xml != NULL ) {
        string stringData = ((svkDataAcquisitionDescriptionXML*)xml)->GetEPSIGradientAxisId();
        strcpy(data.c_str, stringData.c_str());
    } else {
        printf(NULL_XML_ERROR);
    }
    return data;
}


/*!
 *
 * @param xml
 * @return
 */
int svkDataAcquisitionDescriptionXML_GetEPSIGradientAxisIndex(void *xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->GetEPSIGradientAxisIndex();
    } else {
        printf(NULL_XML_ERROR);
        return -0;
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


/*!
 *
 * @param name
 * @param value
 */
void svkDataAcquisitionDescriptionXML_AddEncodedMatrixSizeDimension(const char* name, int value, void* xml)
{
    if( xml != NULL ) {
        ((svkDataAcquisitionDescriptionXML*)xml)->AddEncodedMatrixSizeDimension( name, value );
    } else {
        printf(NULL_XML_ERROR);
    }
}


/*!
 *
 * @param xml
 * @return
 */
int svkDataAcquisitionDescriptionXML_GetEncodedMatrixSizeNumberOfDimensions(void* xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->GetEncodedMatrixSizeNumberOfDimensions();
    } else {
        printf(NULL_XML_ERROR);
        return -1;
    }
}


/*!
 * 
 * @param index
 * @param xml
 * @return
 */
int svkDataAcquisitionDescriptionXML_GetEncodedMatrixSizeDimensionValue(int index, void* xml)
{
    if( xml != NULL ) {
        return ((svkDataAcquisitionDescriptionXML*)xml)->GetEncodedMatrixSizeDimensionValue(index);
    } else {
        printf(NULL_XML_ERROR);
        return -1;
    }
}


/*!
 *
 * @param index
 * @param xml
 * @return
 */
svkCString svkDataAcquisitionDescriptionXML_GetEncodedMatrixSizeDimensionName(int index, void* xml)
{
    svkCString data = GetEmptySvkCString();
    if( xml != NULL ) {
        string stringData = ((svkDataAcquisitionDescriptionXML*)xml)->GetEncodedMatrixSizeDimensionName(index);
        strcpy(data.c_str, stringData.c_str());
    } else {
        printf(NULL_XML_ERROR);
    }
    return data;
}

