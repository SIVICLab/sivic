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
}


/*!
 * Set the trajectory type
 */
void svkDataAcquisitionDescriptionXML::SetTrajectoryType( vtkstd::string type )
{
   vtkXMLDataElement* trajectoryDescElem = svkUtils::FindNestedElementWithPath(this->dataAcquisitionDescriptionXML, "encoding/trajectory");
    trajectoryDescElem->SetCharacterData(type.c_str(),type.size());
}


/*!
 * Get the trajectory type
 */
vtkstd::string svkDataAcquisitionDescriptionXML::GetTrajectoryType( )
{
    vtkXMLDataElement* trajectoryDescElem = svkUtils::FindNestedElementWithPath(this->dataAcquisitionDescriptionXML, "encoding/trajectory");
    return trajectoryDescElem->GetCharacterData();
}


/*!
 * Set the trajectory id
 */
void svkDataAcquisitionDescriptionXML::SetTrajectoryID( vtkstd::string ID )
{
   vtkXMLDataElement* idElem = svkUtils::FindNestedElementWithPath(this->dataAcquisitionDescriptionXML, "encoding/trajectoryDescription/identifier");
    idElem->SetCharacterData(ID.c_str(),ID.size());
}


/*!
 * Get the trajectory id
 */
vtkstd::string svkDataAcquisitionDescriptionXML::GetTrajectoryID( )
{
    vtkXMLDataElement* trajectoryIDElem = svkUtils::FindNestedElementWithPath(this->dataAcquisitionDescriptionXML, "encoding/trajectoryDescription/identifier");
    return trajectoryIDElem->GetCharacterData();
}


/*!
 * Set the trajectory comment
 */
void svkDataAcquisitionDescriptionXML::SetTrajectoryComment( vtkstd::string comment )
{
   vtkXMLDataElement* commentElem = svkUtils::FindNestedElementWithPath(this->dataAcquisitionDescriptionXML, "encoding/trajectoryDescription/comment");
    commentElem->SetCharacterData(comment.c_str(),comment.size());
}


/*!
 * Get the trajectory comment
 */
vtkstd::string svkDataAcquisitionDescriptionXML::GetTrajectoryComment( )
{
    vtkXMLDataElement* trajectoryCommentElem = svkUtils::FindNestedElementWithPath(this->dataAcquisitionDescriptionXML, "encoding/trajectoryDescription/comment");
    return trajectoryCommentElem->GetCharacterData();
}


/*!
 * Set a trajectory user parameter long
 */
void svkDataAcquisitionDescriptionXML::SetTrajectoryParameter( vtkstd::string name, long value  )
{
    vtkXMLDataElement* trajDescElem  = svkUtils::FindNestedElementWithPath(this->dataAcquisitionDescriptionXML, "encoding/trajectoryDescription");
    vtkXMLDataElement* paramElem =  svkUtils::CreateNestedXMLDataElement(trajDescElem, "userParameterLong", "");
    vtkXMLDataElement* nameElem =  svkUtils::CreateNestedXMLDataElement(paramElem, "name", name);
    vtkXMLDataElement* valueElem =  svkUtils::CreateNestedXMLDataElement(paramElem, "value", svkTypeUtils::IntToString(value));
    paramElem->Delete();
    nameElem->Delete();
    valueElem->Delete();
}


/*!
 * Get a trajectory user parameter long
 */
long svkDataAcquisitionDescriptionXML::GetTrajectoryLongParameter( vtkstd::string name  )
{
    vtkXMLDataElement* trajDescElem  = svkUtils::FindNestedElementWithPath(this->dataAcquisitionDescriptionXML, "encoding/trajectoryDescription");
    for( int i = 0; i < trajDescElem->GetNumberOfNestedElements(); i++ ) {
        vtkXMLDataElement* paramElem = trajDescElem->GetNestedElement(i);
        vtkstd::string paramElemName = paramElem->GetName();
        if( paramElemName == "userParameterLong" ) {
            vtkXMLDataElement* nameElem =  svkUtils::FindNestedElementWithPath(paramElem, "name");
            vtkstd::string paramName = nameElem->GetCharacterData();
            vtkXMLDataElement* valueElem =  svkUtils::FindNestedElementWithPath(paramElem, "value");
            if( paramName == name ) {
                return svkTypeUtils::StringToLInt( valueElem->GetCharacterData() );
            }
        } 
    }
    return VTK_LONG_MAX;
}


/*!
 * Get a trajectory user parameter double
 */
double svkDataAcquisitionDescriptionXML::GetTrajectoryDoubleParameter( vtkstd::string name  )
{
    vtkXMLDataElement* trajDescElem  = svkUtils::FindNestedElementWithPath(this->dataAcquisitionDescriptionXML, "encoding/trajectoryDescription");
    for( int i = 0; i < trajDescElem->GetNumberOfNestedElements(); i++ ) {
        vtkXMLDataElement* paramElem = trajDescElem->GetNestedElement(i);
        vtkstd::string paramElemName = paramElem->GetName();
        if( paramElemName == "userParameterDouble" ) {
            vtkXMLDataElement* nameElem =  svkUtils::FindNestedElementWithPath(paramElem, "name");
            vtkstd::string paramName = nameElem->GetCharacterData();
            vtkXMLDataElement* valueElem =  svkUtils::FindNestedElementWithPath(paramElem, "value");
            if( paramName == name ) {
                double value = svkTypeUtils::StringToDouble( valueElem->GetCharacterData() );
                return value;
            }
        } 
    }
    return -1;
}


/*!
 * Set a trajectory user parameter double
 */
void svkDataAcquisitionDescriptionXML::SetTrajectoryParameter( vtkstd::string name, double value  )
{
    vtkXMLDataElement* trajDescElem  = svkUtils::FindNestedElementWithPath(this->dataAcquisitionDescriptionXML, "encoding/trajectoryDescription");
    vtkXMLDataElement* paramElem =  svkUtils::CreateNestedXMLDataElement(trajDescElem, "userParameterDouble", "");
    vtkXMLDataElement* nameElem =  svkUtils::CreateNestedXMLDataElement(paramElem, "name", name);
    vtkXMLDataElement* valueElem =  svkUtils::CreateNestedXMLDataElement(paramElem, "value", svkTypeUtils::DoubleToString(value));
    paramElem->Delete();
    nameElem->Delete();
    valueElem->Delete();
}


/*!
 * Initialize the skeleton of the xml file.
 */
void svkDataAcquisitionDescriptionXML::InitializeEmptyXMLFile()
{
    this->ClearXMLFile();

    // Create Root element
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
    this->dataAcquisitionDescriptionXML = vtkXMLUtilities::ReadElementFromFile( this->xmlFileName.c_str() );
    if (this->dataAcquisitionDescriptionXML == NULL ) { 
        cout << "ERROR, could not parse element from " << this->xmlFileName << endl;
        return 1; 
    } 

    // parse the 3 top level elements: 
    this->versionElement = this->dataAcquisitionDescriptionXML->FindNestedElementWithName("version");
    this->satBandsElement = this->dataAcquisitionDescriptionXML->FindNestedElementWithName("svk_sat_bands");
    if( this->satBandsElement != NULL ) {
        this->satBandsXML = svkSatBandsXML::New();
        vtkstd::string other_test = this->satBandsXML->GetXMLFileName( );
        int test = this->satBandsXML->ParseXML(this->satBandsElement ); 
    } 

    this->versionNumber = this->GetFloatElementData(this->versionElement);
    if( this->GetDebug() ) {
        this->dataAcquisitionDescriptionXML->PrintXML(cout, vtkIndent());
        this->versionElement->PrintXML(cout, vtkIndent());
        this->satBandsElement->PrintXML(cout, vtkIndent());
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
 * Write out the XML file.
*/
void svkDataAcquisitionDescriptionXML::WriteXMLFile( string xmlFileName )
{
    vtkIndent indent;
    this->dataAcquisitionDescriptionXML->PrintXML(cout, vtkIndent());
    vtkXMLUtilities::WriteElementToFile( this->dataAcquisitionDescriptionXML, xmlFileName.c_str(), &indent );
}


/*!
 *  Convenience method.
 */
float svkDataAcquisitionDescriptionXML::GetFloatElementData( vtkXMLDataElement* element )
{
    return svkTypeUtils::StringToFloat(
            string( element->GetCharacterData() )
            ); 
}


/*!
 *  Get the version of the xml file
 */
int svkDataAcquisitionDescriptionXML::GetXMLVersion() 
{
    return this->versionNumber; 
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
void svkDataAcquisitionDescriptionXML_WriteXMLFile(const char* filepath, void* xml )
{
    ((svkDataAcquisitionDescriptionXML*)xml)->WriteXMLFile( filepath );
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


