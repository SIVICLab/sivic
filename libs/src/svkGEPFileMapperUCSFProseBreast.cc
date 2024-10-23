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
 *  $URL: svn+ssh://agentmess@svn.code.sf.net/p/sivic/code/trunk/libs/src/svkGEPFileMapper.cc $
 *  $Rev: 2119 $
 *  $Author: jccrane $
 *  $Date: 2014-12-19 13:13:17 -0800 (Fri, 19 Dec 2014) $
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#include <svkGEPFileMapperUCSFProseBreast.h>
#include </usr/include/vtk/vtkDebugLeaks.h>

using namespace svk;


//vtkCxxRevisionMacro(svkGEPFileMapperUCSFProseBreast, "$Rev: 2119 $");
vtkStandardNewMacro(svkGEPFileMapperUCSFProseBreast);


/*!
 *
 */
svkGEPFileMapperUCSFProseBreast::svkGEPFileMapperUCSFProseBreast()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkGEPFileMapperUCSFProseBreast");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

}


/*!
 *
 */
svkGEPFileMapperUCSFProseBreast::~svkGEPFileMapperUCSFProseBreast()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}



/*!
 *  Get the voxel spacing in 3D, with option for anisotropic FOV with rhuser19. Note that the slice spacing may 
 *  include a skip. 
 */
void svkGEPFileMapperUCSFProseBreast::GetVoxelSpacing( double voxelSpacing[3] )
{

    float user19 =  this->GetHeaderValueAsFloat( "rhi.user19" ); 


    if ( user19 > 0  && this->pfileVersion >= 9 ) {

        voxelSpacing[0] = user19; 
        voxelSpacing[1] = user19; 
        voxelSpacing[2] = this->GetHeaderValueAsFloat( "rhi.scanspacing" );

    } else {
       
        float fov[3];  
        this->GetFOV( fov ); 

        int numVoxels[3]; 
        this->GetNumVoxels( numVoxels ); 

        voxelSpacing[0] = fov[0]/numVoxels[0];
        voxelSpacing[1] = fov[1]/numVoxels[1];
        voxelSpacing[2] = fov[2]/numVoxels[2];
    }
}

/*!
 *  Get the data dcos 
 */
void svkGEPFileMapperUCSFProseBreast::GetDcos( double dcos[3][3] )
{


    dcos[0][0] = -( this->GetHeaderValueAsFloat( "rhi.trhc_R" ) - this->GetHeaderValueAsFloat( "rhi.tlhc_R" ) );
    dcos[0][1] = -( this->GetHeaderValueAsFloat( "rhi.trhc_A" ) - this->GetHeaderValueAsFloat( "rhi.tlhc_A" ) );
    dcos[0][2] =  ( this->GetHeaderValueAsFloat( "rhi.trhc_S" ) - this->GetHeaderValueAsFloat( "rhi.tlhc_S" ) );

    float dcosLengthX = sqrt( dcos[0][0] * dcos[0][0]
                            + dcos[0][1] * dcos[0][1]
                            + dcos[0][2] * dcos[0][2] );

    dcos[0][0] /= dcosLengthX;
    dcos[0][1] /= dcosLengthX;
    dcos[0][2] /= dcosLengthX;


    dcos[1][0] = -( this->GetHeaderValueAsFloat( "rhi.brhc_R" ) - this->GetHeaderValueAsFloat( "rhi.trhc_R" ) );
    dcos[1][1] = -( this->GetHeaderValueAsFloat( "rhi.brhc_A" ) - this->GetHeaderValueAsFloat( "rhi.trhc_A" ) );
    dcos[1][2] =  ( this->GetHeaderValueAsFloat( "rhi.brhc_S" ) - this->GetHeaderValueAsFloat( "rhi.trhc_S" ) );

    float dcosLengthY = sqrt( dcos[1][0] * dcos[1][0]
                            + dcos[1][1] * dcos[1][1]
                            + dcos[1][2] * dcos[1][2] );

    dcos[1][0] /= dcosLengthY;
    dcos[1][1] /= dcosLengthY;
    dcos[1][2] /= dcosLengthY;


    // third row is the vector product of the first two rows
    // General Mapper is -1* vector product "at least for the axial and axial oblique which is all that we support now"
    // Based on 1 data set, for coronal Rx maybe +1*vector product 
    dcos[2][0] = dcos[0][1] * dcos[1][2] - dcos[0][2] * dcos[1][1];
    dcos[2][1] = dcos[0][2] * dcos[1][0] - dcos[0][0] * dcos[1][2];
    dcos[2][2] = dcos[0][0] * dcos[1][1] - dcos[0][1] * dcos[1][0];

    //
    //  change -0.0000 to 0.0000
    //
    for( int i = 0; i <= 2; i++ ) {
        for( int j = 0; j <= 2; j++ ) {
            dcos[i][j] = (dcos[i][j] == 0.0)? 0.0 : dcos[i][j];
        }
    }

    this->ModifyForPatientEntry( dcos ); 



}

