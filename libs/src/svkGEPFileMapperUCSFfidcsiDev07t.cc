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


#include <svkGEPFileMapperUCSFfidcsiDev07t.h>
#include </usr/include/vtk/vtkDebugLeaks.h>


using namespace svk;


//vtkCxxRevisionMacro(svkGEPFileMapperUCSFfidcsiDev07t, "$Rev$");
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
 *  Virtual method for initializing the spectrum array for a given cell. 
 *  Frequency is reversed in this psd. 
 */
void svkGEPFileMapperUCSFfidcsiDev07t::InitSpecTuple( int numFreqPts, int freqPt, float* tuple, vtkDataArray* dataArray )
{

    //  write the complex conjugate of the data: 
    //  z = RE + iIM
    //  z' = RE - iIM
    tuple[1] = -1 * tuple[1];  
    dataArray->SetTuple( freqPt, tuple );
}


/*!
 *  Gets the chemical shift reference taking into account acquisition frequency offset
 *  and the acquisition sample temperature.
 */
float svkGEPFileMapperUCSFfidcsiDev07t::GetPPMRef()
{
    return 178;
}

