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


#include <svkUCSFUtils.h>


using namespace svk;


//vtkCxxRevisionMacro(svkUCSFUtils, "$Rev$");
vtkStandardNewMacro(svkUCSFUtils);

// We have to initialize static variables here.
bool svkUCSFUtils::mapCreated = false;
map<string, string> svkUCSFUtils::metaboliteMap;

//! Constructor
svkUCSFUtils::svkUCSFUtils()
{
}


//! Destructor
svkUCSFUtils::~svkUCSFUtils()
{
}


/*!
 *  Creates our hash.
 *
 *  TODO: Consider making the hash read from a text file so we don't have to recompile to add mappings
 */
void svkUCSFUtils::CreateMap()
{
    svkUCSFUtils::metaboliteMap["CHO-area"]     = "r01";
    svkUCSFUtils::metaboliteMap["CHO-ht"]       = "rh01";
    svkUCSFUtils::metaboliteMap["CRE-area"]     = "r02";
    svkUCSFUtils::metaboliteMap["CRE-ht"]       = "rh02";
    svkUCSFUtils::metaboliteMap["NAA-area"]     = "r03";
    svkUCSFUtils::metaboliteMap["NAA-ht"]       = "rh03";
    svkUCSFUtils::metaboliteMap["LAC/LIP-area"] = "r04";
    svkUCSFUtils::metaboliteMap["LAC/LIP-ht"]   = "rh04";
    svkUCSFUtils::metaboliteMap["CCRI-area"]    = "r01ares02";
    svkUCSFUtils::metaboliteMap["CCRI-ht"]      = "r01hres02";
    svkUCSFUtils::metaboliteMap["CNI-area"]     = "r01ares03";
    svkUCSFUtils::metaboliteMap["CNI-ht"]       = "r01hres03";
    svkUCSFUtils::mapCreated = true;
}


/*!
 *  Gets the metabolite name for a given metabolite file
 */
string svkUCSFUtils::GetMetaboliteName( string fileName )
{
    size_t corPos = fileName.rfind("_cor"); 
    size_t extention = fileName.find_last_of("."); 
    string metaboliteName;
    string postfix;
    if ( corPos != fileName.npos && extention != fileName.npos ) {
        postfix = fileName.substr( corPos + 4, extention - corPos - 4  );
        metaboliteName = svkUCSFUtils::GetMetaboliteFromPostfix(postfix);
    }
    return metaboliteName;
}


/*!
 *  Method takes a spectra file name and determines the name of the root for the metabolites
 *
 *  \param spectraFileName the name of the spectra file that you want to find the metabolite root
 */
string svkUCSFUtils::GetMetaboliteRoot( string spectraFileName )
{
    size_t corPos = spectraFileName.rfind("_cor"); 
    size_t lastPath = spectraFileName.find_last_of("/"); 
    string root;
    if ( corPos != spectraFileName.npos ) {
        if ( lastPath != spectraFileName.npos ) {
            root = spectraFileName.substr( lastPath + 1, corPos + 4 - lastPath - 1  );
        } else {
            root = spectraFileName.substr( 0, corPos + 4 );
        }
    }
    return root;

}


/*!
 *  Method takes a spectra file name and determines the name of the directory in which it should reside.
 *
 *  \param spectraFileName the name of the spectra file that you want to find the metabolite
 */
string svkUCSFUtils::GetMetaboliteDirectoryName( string spectraFileName )
{
    string root = svkUCSFUtils::GetMetaboliteRoot( spectraFileName );
    string directory;
    if ( !root.empty() ) {
        directory = root + "_met";
    }
    return directory;

}


/*!
 *  Method takes a spectra file name and determines the filename of  a given metabolite for that
 *  spectra.
 *
 *  \param spectraFileName the name of the spectra file that you want to find the metabolite
 *  \param metaboliteName  the metabolite name you are looking for
 *  \param incluePath      should we include the name of the path in which the file should exist
 */
string svkUCSFUtils::GetMetaboliteFileName( string spectraFileName, string metaboliteName, bool includePath )
{
    string metaboliteFileName;
    if( includePath ) {
        size_t lastPath = spectraFileName.find_last_of("/"); 
        if ( lastPath != spectraFileName.npos ) {
            metaboliteFileName += spectraFileName.substr(0,lastPath);
            metaboliteFileName += "/";
        }
        metaboliteFileName += svkUCSFUtils::GetMetaboliteDirectoryName( spectraFileName );
        metaboliteFileName += "/";
    }
    metaboliteFileName += svkUCSFUtils::GetMetaboliteRoot( spectraFileName );
    metaboliteFileName += svkUCSFUtils::GetMetabolitePostfix( metaboliteName );
    metaboliteFileName += ".idf";

    return metaboliteFileName;
}


/*!
 *  Get the file name postfix for a given metabolite.
 *
 *  \param metaboliteName the name of the metabolite you are looking for
 */
string svkUCSFUtils::GetMetabolitePostfix( string metaboliteName )
{
    if ( !svkUCSFUtils::mapCreated ) {
        svkUCSFUtils::CreateMap();
    }
    
    return svkUCSFUtils::metaboliteMap[metaboliteName];
}


/*!
 *  Get the metabolite name for a given postfix.
 *
 *  \param postfix for the name of the metabolite you are looking for
 */
string svkUCSFUtils::GetMetaboliteFromPostfix( string postfixName )
{
    string metaboliteName;
    if ( !svkUCSFUtils::mapCreated ) {
        svkUCSFUtils::CreateMap();
    }

    bool found = false;
    map<string,string>::iterator it = svkUCSFUtils::metaboliteMap.begin();

    while(it != svkUCSFUtils::metaboliteMap.end())
    {
        if( it->second == postfixName ) {
            metaboliteName = it->first;
            break;
        }
        ++it;
    }

    return metaboliteName;
}


/*!
 *  Gets the names of all the metabolites in the current hash.
 *
 *  \return all the names in a vector of strings
 */
vector<string> svkUCSFUtils::GetAllMetaboliteNames()
{
    vector<string> names;
    if ( !svkUCSFUtils::mapCreated ) {
        svkUCSFUtils::CreateMap();
    }

    map<string,string>::iterator it = svkUCSFUtils::metaboliteMap.begin(); 

    while(it != svkUCSFUtils::metaboliteMap.end())
    {
        names.push_back( it->first );
        ++it;
    }
    return names;
}


string svkUCSFUtils::GetDICOMFileName( string fileName, svkDcmHeader* dcmHeader )
{
    // First we construct the filename of a .dcm file from the same series
    size_t pos;
    pos = fileName.find_last_of("/");
    pos = fileName.substr(0,pos).find_last_of("/");
    

    // First we construct the filename of a .dcm file from the same series
    string dcmFileName(fileName.substr(0,pos).c_str());

    stringstream ss;
    ss << "/E";
    ss << dcmHeader->GetStringValue("StudyID");
    ss << "/";
    ss << dcmHeader->GetIntValue("SeriesNumber");
    ss << "/E";
    ss << dcmHeader->GetStringValue("StudyID");
    ss << "S";
    ss << dcmHeader->GetIntValue("SeriesNumber");
    ss << "I";
    ss << "1.DCM";
    dcmFileName.append( ss.str() );
    return dcmFileName;
}


/*
 * Reidentifies images in the given directory using the ucsf database.
 */
int svkUCSFUtils::ReidentifyImages( string directory ) 
{
    int result = 1;  // this indicates failure
#ifndef WIN32
    stringstream reidentifyCommand;
    reidentifyCommand << "reidentify_images --in_dir " << directory.c_str();
    result = system( reidentifyCommand.str().c_str() );
#endif
    return result;
}

