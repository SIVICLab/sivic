/*
 *  Copyright © 2009-2013 The Regents of the University of California.
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


#include <svkGEPFileMapperUCSFfidcsiDev07t.h>
#include <vtkDebugLeaks.h>


using namespace svk;


vtkCxxRevisionMacro(svkGEPFileMapperUCSFfidcsiDev07t, "$Rev$");
vtkStandardNewMacro(svkGEPFileMapperUCSFfidcsiDev07t);


/*!
 *
 */
svkGEPFileMapperUCSFfidcsiDev07t::svkGEPFileMapperUCSFfidcsiDev07t()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkGEPFileMapperUCSFfidcsiDev07t");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

}


/*!
 */
svkGEPFileMapperUCSFfidcsiDev07t::~svkGEPFileMapperUCSFfidcsiDev07t()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}


/*!
 *  7T version of fidcsi, flips x and y indices
 */
void svkGEPFileMapperUCSFfidcsiDev07t::GetXYZIndices(int dataIndex, int* x, int* y, int* z)
{
    int numVoxels[3];

    this->GetNumVoxels(numVoxels);

    *z = static_cast <int> ( dataIndex/(numVoxels[0] * numVoxels[1]) );

    if ( this->IsSwapOn() ) {

        // If swap is on use numVoxels[1] for x dimension and numVoxels[0] for y dimension
        *x = static_cast <int> ((dataIndex - (*z * numVoxels[0] * numVoxels[1]))/numVoxels[1]);

        // In addition to swapping reverse the y direction
        *y = numVoxels[1] - static_cast <int> ( dataIndex%numVoxels[1] ) - 1;

    } else {
        *x = static_cast <int> ( dataIndex%numVoxels[0] );
        *y = static_cast <int> ((dataIndex - (*z * numVoxels[0] * numVoxels[1]))/numVoxels[0]);
    }

    // flip xdir and ydir
    *x = numVoxels[0] - *x -1;
    *y = numVoxels[1] - *y -1;

}
