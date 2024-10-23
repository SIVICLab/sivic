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


#include <svkGEPFileMapperMBrease.h>
#include </usr/include/vtk/vtkDebugLeaks.h>


using namespace svk;


//vtkCxxRevisionMacro(svkGEPFileMapperMBrease, "$Rev$");
vtkStandardNewMacro(svkGEPFileMapperMBrease);


/*!
 *
 */
svkGEPFileMapperMBrease::svkGEPFileMapperMBrease()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkGEPFileMapperMBrease");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );
}


/*!
 *
 */
svkGEPFileMapperMBrease::~svkGEPFileMapperMBrease()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}


/*!
 *  Determine number of time points in the PFile. 
 *  For mbrease, there are suppressed, unsuppressed 
 *  acquisitions in the same data set.  There is 1 dummy
 *  scan before each coil:
 *      coil 1: dummy + unsuppressed_scans + suppressed_scans
 *      coil 2: ""
 */
int svkGEPFileMapperMBrease::GetNumTimePoints()
{
    int numSuppressedScans = this->GetNumberSuppressedAcquisitions(); 
    int numUnsuppressedScans = this->GetNumberUnsuppressedAcquisitions(); 

    int numTimePoints = numUnsuppressedScans + numSuppressedScans; 

    if ( this->GetDebug() ) {
        cout << "NUM TIME POINTS: " <<  numTimePoints << endl;
    }

    return numTimePoints; 
}


/*!
 *  For single voxel acquisitions, return the number of
 *  unsuppressed acquisitions.
 */
int svkGEPFileMapperMBrease::GetNumberUnsuppressedAcquisitions()
{
    int numUnsuppressed = this->GetHeaderValueAsInt( "rhr.rh_user44" ); 
    return numUnsuppressed;
}


/*!
 *  For single voxel acquisitions, return the number of
 *  suppressed acquisitions.
 */
int svkGEPFileMapperMBrease::GetNumberSuppressedAcquisitions()
{
    int numSuppressed = this->GetHeaderValueAsInt( "rhr.rh_user4" ); 
    return numSuppressed;
}


/*!
 *  Is data chopped?
 *  suppressed data:  yes
 */
bool svkGEPFileMapperMBrease::IsChopOn()
{
    //  Set default value: 
    bool chop = true ;

    //  reset if manually overridden by user:
    this->GetInputArgBoolValue("chop", &chop); 

    return chop;
}

