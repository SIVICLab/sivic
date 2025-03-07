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


#include <svkXMLUtils.h>
#include <svkTypeUtils.h>

using namespace svk;

//vtkCxxRevisionMacro(svkXMLUtils, "$Rev$");
vtkStandardNewMacro(svkXMLUtils);

//! Constructor
svkXMLUtils::svkXMLUtils()
{
}


//! Destructor
svkXMLUtils::~svkXMLUtils()
{
}


/*!
 *  This method takes as input a parent xml data element in which to nest a new xml data element.
 *  It also takes a name, and value as input.
 */
vtkXMLDataElement* svkXMLUtils::CreateNestedXMLDataElement( vtkXMLDataElement* parent, string name, string value )
{
    vtkXMLDataElement* child = NULL;
    if( parent != NULL ) {
        child = vtkXMLDataElement::New();
        child->SetName(name.c_str());
        child->SetCharacterData(value.c_str(), value.size());
        parent->AddNestedElement(child);
    }
    return child;
}


/*!
 * This method reads an xml file and then substitutes varibales from variables vectors. Each element
 * in the variables vector is expected to have the format: VARIBALE_NAME=value. For this case any
 * string the xml matching $VARIABLE_NAME will be replaced with 'value'.
 */
vtkXMLDataElement* svkXMLUtils::ReadXMLAndSubstituteVariables(string xmlFileName, vector<string> xmlVariables )
{
    vtkXMLDataElement* xml = NULL;
    // Replace any variables in the configuration file here..
    string line;
    string xmlFileString;
    ifstream xmlFile (xmlFileName.c_str());

    // Replace variables in XML
    if (xmlFile.is_open()) {
        while ( getline (xmlFile,line) ) {
            for( int i = 0; i< xmlVariables.size(); i++ ) {
                std::size_t pos = xmlVariables[i].find("=");
                string variable = "$";
                variable.append( xmlVariables[i].substr(0,pos) );
                string value = xmlVariables[i].substr(pos+1);
                pos = line.find(variable);
                while(pos != string::npos  ) {
                    line.erase(pos, variable.size() );
                    line.insert(pos, value);
                    pos = line.find(variable, pos+1);
                }
            }

            xmlFileString.append( line.c_str() );
        }
        xmlFile.close();
    }
    // Lets start by reading the configuration file
    xml = vtkXMLUtilities::ReadElementFromString( xmlFileString.c_str()  );
    return xml;
}


/*!
 *  Finds a nested element at a depth greater than one. Searches from the root
 *  node, and assumes a '/' separated list of nested elements. Returns null if
 *  no element exists at the requested path.
 */
vtkXMLDataElement* svkXMLUtils::FindNestedElementWithPath( vtkXMLDataElement* root, string xmlPath)
{
    vector<string> elements = svkXMLUtils::SplitString( xmlPath, "/");
    vtkXMLDataElement* elem = root;
//    cout << "Reading element at path: " << xmlPath << endl;
    if( elements.size() > 0 ) {
        for( int i = 0; i < elements.size(); i++ ) {
            if( elem != NULL ) {
                vector<string> parsedName = svkXMLUtils::SplitString(elements[i], "[");
                string attribute = "";
                string attributeValue = "";
                if( parsedName.size() > 1 ) {
                    vector<string> subparsedName = svkXMLUtils::SplitString(parsedName[1], "]");
                    vector<string> subsubparsedName = svkXMLUtils::SplitString(subparsedName[0], "=");
                    attribute = subsubparsedName[0];
                    attributeValue = subsubparsedName[1];
                    int numElems = elem->GetNumberOfNestedElements();
                    vtkXMLDataElement* it = elem->GetNestedElement(3)->FindNestedElementWithName("dx");

                    elem = elem->FindNestedElementWithNameAndAttribute(parsedName[0].c_str(), attribute.c_str(), attributeValue.c_str());
                }  else {
                    elem = elem->FindNestedElementWithName(elements[i].c_str());
                }

            } else {
                break;
            }
        }
    } else {
        elem = NULL;
    }
    return elem;
}

/*!
 *  Finds a nested element at a depth greater than one. Searches from the root
 *  node, and assumes a '/' separated list of nested elements. If element is not
 *  found then it is created and returned.
 */
vtkXMLDataElement* svkXMLUtils::FindOrCreateNestedElementWithPath( vtkXMLDataElement* root, string parentPath, string elementName)
{
    vtkXMLDataElement* elem = NULL;
    string elementPath = parentPath;
    elementPath.append("/");
    elementPath.append(elementName);
    elem = svkXMLUtils::FindNestedElementWithPath(root, elementPath);
    if( elem == NULL ) {
        vtkXMLDataElement* parent = svkXMLUtils::FindNestedElementWithPath(root, parentPath);
        elem = svkXMLUtils::CreateNestedXMLDataElement( parent, elementName, "" );
    }
    return elem;
}



/*!
 * Method finds a nested element and then grabs the character data and puts it
 * into the data string provided as an argument. If the character data is
 * retrieved then the method returns true, otherwise false.
 */
bool svkXMLUtils::GetNestedElementCharacterDataWithPath( vtkXMLDataElement* root, string xmlPath, string* data )
{
    bool dataFound = false;
    vtkXMLDataElement* elem = svkXMLUtils::FindNestedElementWithPath( root, xmlPath );
    if( elem != NULL ) {
        *data = elem->GetCharacterData();
        dataFound = true;
    }
    return dataFound;
}


/*!
 * Sets the character data for a given xml path relative to a root element. If
 * the element exists then it will be set and true will be returned. Otherwise
 * false will be returned.
 */
bool svkXMLUtils::SetNestedElementWithPath( vtkXMLDataElement* root, string xmlPath, string value)
{
    bool wasSet = false; // Assume failure
    vtkXMLDataElement* elem = svkXMLUtils::FindNestedElementWithPath(root, xmlPath );
    if( elem != NULL ) {
        elem->SetCharacterData(value.c_str(), value.size());
        wasSet = true;
    }
    return wasSet;
}


/*! 
 *  Utility function to read a single line from a file stream.
 */
void svkXMLUtils::ReadLine(ifstream* fs, istringstream* iss)    
{
    char line[2000];
    iss->clear();    
    fs->getline(line, 2000);
    iss->str(string(line));
}


/*!
 *
 */
vector<string> svkXMLUtils::SplitString( string str, string token )
{
    vector<string> result;
    int nPos;
    while( (nPos = str.find_first_of(token)) != str.npos ) {
        if(nPos > 0) {
            result.push_back(str.substr(0,nPos));
        }
        str = str.substr(nPos+1);
    }
    if(str.length() > 0) {
        result.push_back(str);
    }
    return result;
}

