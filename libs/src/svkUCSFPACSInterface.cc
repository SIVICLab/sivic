/*
 *  Copyright © 2009-2011 The Regents of the University of California.
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



#include <svkUCSFPACSInterface.h>

using namespace svk;

vtkCxxRevisionMacro(svkUCSFPACSInterface, "$Rev$");
vtkStandardNewMacro(svkUCSFPACSInterface);


//! Constructor
svkUCSFPACSInterface::svkUCSFPACSInterface()
{
    this->pacsTarget        = string( PACS_DIRECTORY );
    this->pacsTempDirectory = string( this->pacsTarget );
    this->pacsTempDirectory.append( PACS_TEMP_DIRECTORY );
}

//! Destructor
svkUCSFPACSInterface::~svkUCSFPACSInterface()
{

}


/*!
 *
 *  To prepare to send to PACS we will create the temp directory and 
 *  make sure we can write to it.
 *
 *  \return was the preparation successful
 */
bool svkUCSFPACSInterface::Connect()
{
    bool success = true;
    int result = 0;
    
    // Create Temp PACS directary
    if(svkUtils::FilePathExists( this->pacsTempDirectory.c_str()) ) {
        result = vtkDirectory::DeleteDirectory( this->pacsTempDirectory.c_str() );
        
        // Make sure the delete was successful
        if( result == 0 ) { // vtkDirectory uses an int for return, it is actually a true/false result
            cout << "ERROR: Could not delete previous temp folder at: " << this->pacsTempDirectory << endl;
            return false;
        }
    }

    result = vtkDirectory::MakeDirectory( this->pacsTempDirectory.c_str() );
    if ( result == 0 ) {  // vtkDirectory uses an int for return, it is actually a true/false result
        cout << "ERROR: Could not create temparary directory at: " << this->pacsTempDirectory << endl;
        return false;
    }
    
    // Make sure we can write to the new directary
    if ( !svkUtils::CanWriteToPath(this->pacsTempDirectory.c_str())) { // Can the user get a file handle
        cout << "ERROR: Could not write to temparary directory at: " << this->pacsTempDirectory << endl;
        success = false;
    } 

    return success;
}


/*!
 *  Copies images to the temp directory, reidentifies them, and the copies them to 
 *  the PACS directory.
 *
 *  \param files a vector of image file names to be sent.
 *  \param sourceDirectory a string containing the source directory.
 *  \return true of the copy succeeds, otherwise false.
 */
bool svkUCSFPACSInterface::SendImagesToPACS( vector<string> files, string sourceDirectory  )
{
    bool success = true;
    int result = 1;

    // Copy images
    for( vector<string>::iterator iter = files.begin();
        iter != files.end(); ++iter) {
        string source = string(sourceDirectory);
        source.append( *iter );
        string destination = string(this->pacsTempDirectory);
        destination.append(*iter);
        result = svkUtils::CopyFile( source.c_str(), destination.c_str() );
        if( result != 0 ) { // This is a system results so 0 is success
            cout<< "ERROR: COULD NOT COPY: " << source << " to " << destination << ". Halting sending images." << endl;
            return false; 
        }
    }

    result = svkUCSFUtils::ReidentifyImages( this->pacsTempDirectory );
    if( result != 0 ) { // This is a system result so 0 is success
        cout << "ERROR: COULD NOT REIDENTIFY IMAGES!" << endl;
        return false;
    }

    // Move images
    for( vector<string>::iterator iter = files.begin();
        iter != files.end(); ++iter) {
        string source = string(pacsTempDirectory);
        source.append( *iter );
        string destination = string(this->pacsTarget);
        destination.append(*iter);
        result = svkUtils::MoveFile( source.c_str(), destination.c_str() );
        if( result != 0 ) { // This is a system result so 0 is success
            cout<< "COULD NOT MOVE: " << source << " to " << destination << ". Halting sending images." << endl;
            return false; 
        }
    }
    return success;

}


/*!
 *  Deletes the temporary directory.
 */
bool svkUCSFPACSInterface::Disconnect()
{
    bool success = true;
    int result = vtkDirectory::DeleteDirectory( this->pacsTempDirectory.c_str() );
    if ( result == 0 ) {  // vtkDirectory uses an int for return, it is actually a true/false result
        cout << "ERROR: Could not delete temparary directory at: " << this->pacsTempDirectory << endl;
        success = false;
    }
    return success;
}
