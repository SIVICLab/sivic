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



#include <svkUCSFPACSInterface.h>

using namespace svk;

//vtkCxxRevisionMacro(svkUCSFPACSInterface, "$Rev$");
vtkStandardNewMacro(svkUCSFPACSInterface);


//! Constructor
svkUCSFPACSInterface::svkUCSFPACSInterface()
{
    this->pacsTarget = string( "" );
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
bool svkUCSFPACSInterface::SendImagesToPACS( string sourceDirectory, svkTypes::AnatomyType anatomyType  )
{
    bool success = true;
    int result = 0;

#ifndef WIN32
    stringstream sendToPACSCommand;

    sendToPACSCommand << "send_to_pacs --in_dir " << sourceDirectory;
    if( this->pacsTarget.compare("") != 0 ) {
        sendToPACSCommand << " --test_dir " << this->pacsTarget;
    }

    //  If not brain group data, then input images should already be identified
    //  therefore do not try to look up PHI in brain group DB when pushing to PACS.
    if ( anatomyType == svkTypes::ANATOMY_PROSTATE ) { 
        sendToPACSCommand << " --no_reid " ;
        sendToPACSCommand << " --test_dir /tmp " ;
    }

    cout << "Send to PACS command: " << sendToPACSCommand.str() << endl;
    result = system( sendToPACSCommand.str().c_str() );
    if( result != 0 ) {
        cout << "ERROR: Could not send to PACS! " << endl;
        success = false;
    } else {
        success = true;
    }
#else
    // Not supported in windows
#endif

    return success;
}


/*!
 *
 */
bool svkUCSFPACSInterface::Disconnect()
{
    bool success = true;
    return success;
}
