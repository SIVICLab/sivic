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


#include <svkGEPFileMapperMPCSIOBL.h>
#include </usr/include/vtk/vtkDebugLeaks.h>


using namespace svk;


//vtkCxxRevisionMacro(svkGEPFileMapperMPCSIOBL, "$Rev$");
vtkStandardNewMacro(svkGEPFileMapperMPCSIOBL);


/*!
 *
 */
svkGEPFileMapperMPCSIOBL::svkGEPFileMapperMPCSIOBL()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkGEPFileMapperMPCSIOBL");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );
}


/*!
 *
 */
svkGEPFileMapperMPCSIOBL::~svkGEPFileMapperMPCSIOBL()
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
int svkGEPFileMapperMPCSIOBL::GetNumTimePoints()
{

    int numTimePoints = 1; 

    if ( this->GetDebug() ) {
        cout << "NUM TIME POINTS: " <<  numTimePoints << endl;
    }

    return numTimePoints; 
}


/*!
 *  Get the 3D spatial dimensionality of the data set
 *  Returns an int array with 3 dimensions.  Swaps
 *  if necessary based on freq_dir setting.
 */
void svkGEPFileMapperMPCSIOBL::GetNumVoxels( int numVoxels[3] )
{
        numVoxels[0] = 20;
        numVoxels[1] = 20;
        numVoxels[2] = 4;

    //  Swap dimensions if necessary:
    if ( this->IsSwapOn() ) {
        int temp = numVoxels[0];
        numVoxels[0] = numVoxels[1];
        numVoxels[1] = temp;
    }
}


/*! 
 *  just for testing
 */
int svkGEPFileMapperMPCSIOBL::GetNumDummyScans()
{
    return 32;  
}

