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


#include <vtkXMLDataElement.h>
#include <vtkXMLUtilities.h>

#include <svkSatBandsXML.h>

#include "svkUtils.h"


using namespace svk;


vtkCxxRevisionMacro(svkSatBandsXML, "$Rev$");
vtkStandardNewMacro(svkSatBandsXML);


/*!
 *
 */
svkSatBandsXML::svkSatBandsXML()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    this->isVerbose = false; 
    this->satBandsXML = NULL; 
    this->versionElement = NULL;     
    this->pressBoxElement = NULL;     
    this->autoSatsElement = NULL; 
}


/*!
 *
 */
svkSatBandsXML::~svkSatBandsXML()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}


/*!
 *  set the path/name to xml file.   
 */
void svkSatBandsXML::SetXMLFileName( string xmlFileName )
{
    this->xmlFileName = xmlFileName;  
    // Now we have remove the old xml file
    this->ClearXMLFile();
    this->satBandsXML = vtkXMLUtilities::ReadElementFromFile( this->xmlFileName.c_str() );

    // parse the 3 top level elements: 
    this->versionElement = this->satBandsXML->FindNestedElementWithName("version");
    this->pressBoxElement = this->satBandsXML->FindNestedElementWithName("press_box");
    this->autoSatsElement = this->satBandsXML->FindNestedElementWithName("auto_sats");

    if( this->GetDebug() ) {
        //this->satBandsXML->PrintXML(cout, vtkIndent());
        //this->versionElement->PrintXML(cout, vtkIndent());
        //this->pressBoxElement->PrintXML(cout, vtkIndent());
        //this->autoSatsElement->PrintXML(cout, vtkIndent());
    }

}


/*! Sets the current XML data to NULL
 *  so the file will be re-read.
 */
void  svkSatBandsXML::ClearXMLFile( )
{
    if( this->satBandsXML != NULL ) {
        this->satBandsXML->Delete();
        this->satBandsXML = NULL;
    }

}


/*!
 *  Get the path to the current XML file
 */
string svkSatBandsXML::GetXMLFileName( )
{
    return this->xmlFileName;
}


/*!
 *  Write the integrals for each voxel to stdout. Default is false.  
 */
void svkSatBandsXML::SetVerbose( bool isVerbose )
{
    this->isVerbose = isVerbose; 
}


/*!
 *  Get the specified sat band definition: 
 */
void svkSatBandsXML::GetPressBoxSat(int satNumber, string* label, float *normalX, float *normalY, float* normalZ, float* thickness, float* distance)
{

    vtkXMLDataElement* satBandElement; 
    satBandElement = this->pressBoxElement->FindNestedElementWithNameAndId(
                "sat_band", 
                svkUtils::IntToString(satNumber).c_str() 
            ); 

    this->InitSatBandInfo( satBandElement, label, normalX, normalY, normalZ, thickness, distance); 

}


/*!
 *  Get the specified sat band definition: 
 */
void svkSatBandsXML::GetAutoSat(int satNumber, string* label, float *normalX, float *normalY, float* normalZ, float* thickness, float* distance)
{
    vtkXMLDataElement* satBandElement; 
    satBandElement = this->autoSatsElement->FindNestedElementWithNameAndId(
                "sat_band", 
                svkUtils::IntToString(satNumber).c_str() 
            ); 

    this->InitSatBandInfo( satBandElement, label, normalX, normalY, normalZ, thickness, distance); 

}


/*
 */
void svkSatBandsXML::InitSatBandInfo( vtkXMLDataElement* satBandElement, string* label, float *normalX, float *normalY, float* normalZ, float* thickness, float* distance) 
{

    *label = string( satBandElement->GetAttribute("label") ); 

    *normalX   = this->GetFloatElementData( satBandElement->
                    FindNestedElementWithName( "normal_x" ) ); 
    *normalY   = this->GetFloatElementData( satBandElement->
                    FindNestedElementWithName( "normal_y" ) ); 
    *normalZ   = this->GetFloatElementData( satBandElement->
                    FindNestedElementWithName( "normal_z" ) ); 
    *thickness = this->GetFloatElementData( satBandElement->
                    FindNestedElementWithName( "thickness" ) ); 
    *distance  = this->GetFloatElementData( satBandElement->
                    FindNestedElementWithName( "distance_from_origin" ) ); 
            
    if( this->GetDebug() ) {
        //satBandElement->PrintXML(cout, vtkIndent());
    }   

}


float svkSatBandsXML::GetFloatElementData( vtkXMLDataElement* element )
{
    return svkUtils::StringToFloat(
            string( element->GetCharacterData() )
            ); 
}



void svkSatBandsXML::GetCurrentXMLVersion(string* v1, string* v2, string* v3)
{
    //  first check if the file exists (returns 0):
    struct stat buf;
    bool fileExists;
    if (stat(this->GetXMLFileName( ).c_str(), &buf) == 0) {
        vtkXMLDataElement* xml = vtkXMLUtilities::ReadElementFromFile(
                this->GetXMLFileName( ).c_str()
        );
        string xmlVersion( xml->GetAttributeValue( 0 ) );
        //  Parse into 3 components:
        size_t delim;
        delim = xmlVersion.find_first_of('.');
        *v1 = xmlVersion.substr(0, delim );
        xmlVersion.assign( xmlVersion.substr( delim + 1 ));

        delim = xmlVersion.find_first_of('.');
        *v2 = xmlVersion.substr( 0, delim );
        xmlVersion.assign( xmlVersion.substr( delim +1 ));

        delim = xmlVersion.find_first_of('.');
        *v3 = xmlVersion.substr( 0, delim );
    } else {
        *v1 = "0";
        *v2 = "0";
        *v3 = "0";
    }
}


/*!
 * 
 */
int svkSatBandsXML::GetNumberOfPressBoxSats() 
{
    int numberOfSats = this->pressBoxElement->GetNumberOfNestedElements(); 
    return numberOfSats; 
}


/*!
 * 
 */
int svkSatBandsXML::GetNumberOfAutoSats() 
{
    int numberOfSats = this->autoSatsElement->GetNumberOfNestedElements(); 
    return numberOfSats; 

}


/*! 
 * External C interface: 
 */
void* svkSatBandsXML_New(char* xmlFileName){
    svkSatBandsXML* xml = svkSatBandsXML::New();     
    xml->SetXMLFileName(xmlFileName); 
    return ((void*)xml); 
}; 


/*!
 * 
 */
int svkSatBandsXML_GetNumberOfPressBoxSats( void* xml ) 
{
    return ((svkSatBandsXML*)xml)->GetNumberOfPressBoxSats(); 
}


/*!
 * 
 */
int svkSatBandsXML_GetNumberOfAutoSats( void* xml ) 
{
    return ((svkSatBandsXML*)xml)->GetNumberOfAutoSats(); 
}



/*!
 * 
 */
void  svkSatBandsXML_GetPressBoxSat(void* xml, int satNumber, float* normalX, float* normalY, float* normalZ, float* thickness, float* distance ) 
{
    string label; 
    ((svkSatBandsXML*)xml)->GetPressBoxSat( satNumber, &label, normalX, normalY, normalZ, thickness, distance);

}


/*!
 * 
 */
void  svkSatBandsXML_GetAutoSat(void* xml, int satNumber, float* normalX, float* normalY, float* normalZ, float* thickness, float* distance ) 
{

    string label; 
    ((svkSatBandsXML*)xml)->GetAutoSat( satNumber, &label, normalX, normalY, normalZ, thickness, distance);

}
