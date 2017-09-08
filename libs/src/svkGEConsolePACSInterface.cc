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



#include <svkGEConsolePACSInterface.h>

using namespace svk;

//vtkCxxRevisionMacro(svkGEConsolePACSInterface, "$Rev$");
vtkStandardNewMacro(svkGEConsolePACSInterface);


//! Constructor
svkGEConsolePACSInterface::svkGEConsolePACSInterface()
{
    char* importImageDir = getenv ("IMPORT_IMAGE_DIR");
    if ( importImageDir != NULL ) {
        this->pacsTarget = string( importImageDir );
    } else {
        this->pacsTarget = "/tmp";
    }
}


//! Destructor
svkGEConsolePACSInterface::~svkGEConsolePACSInterface()
{
}


/*!
 *
 *  To prepare to send to PACS we will create the temp directory and 
 *  make sure we can write to it.
 *
 *  \return was the preparation successful
 */
bool svkGEConsolePACSInterface::Connect()
{
    bool success = true;
    int result = 0;
    
    // Make sure we can write to the PACS directary
    if ( !svkUtils::CanWriteToPath(this->pacsTarget.c_str())) { // Can the user get a file handle
        cout << "ERROR: Could not write to PACS directory at: " << this->pacsTarget << endl;
        success = false;
    } 

    return success;
}


/*!
 *  Sends deidentified images to PACS (they become reidentified in transit)
 *
 *  \param sourceDirectory a string containing the source directory.
 *  \return true of the copy succeeds, otherwise false.
 *  
 *  If this is for the Brain group, reidentify images using BRAIN DB (type 0). 
 *  For prostate (type 1), images are aleady identified. 
 */
bool svkGEConsolePACSInterface::SendImagesToPACS( string sourceDirectory, svkTypes::AnatomyType anatomyType  )
{
    bool success = true;
    int result = 0;

    stringstream cpCommand;

    cpCommand << "cp -r " << sourceDirectory << " ";
    cpCommand << this->pacsTarget;

    cout << "Send to PACS command: " << cpCommand.str() << endl;
    result = system( cpCommand.str().c_str() );

    if( result != 0 ) {
        cout << "ERROR: Could not send to PACS! " << endl;
        success = false;
    } else {
        success = true;
    }

    if ( success ) {

        //  Get the name of the directory containing the SC iamges to import
        //  This is the full path in the new IMPORT_IMAGE_DIR location on the console
        string dicomDir = this->pacsTarget;  
        dicomDir.append( "/" ); 
        dicomDir.append( svkUtils::GetFilenameFromFullPath( sourceDirectory ) ); 
        
        //  move to .sdcopen to trigger import: 
        stringstream renameCommand;
        renameCommand << "mv " << dicomDir << " " << dicomDir << ".sdcopen";

        cout << "Send to PACS command: " << renameCommand.str() << endl;
        result = system( renameCommand.str().c_str() );

        if( result != 0 ) {
            cout << "ERROR: Could not send to PACS! " << endl;
            success = false;
        } else {
            success = true;
        }
    }



    return success;
}


/*!
 *
 */
bool svkGEConsolePACSInterface::Disconnect()
{
    bool success = true;
    return success;
}
