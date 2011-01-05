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


#include <svkGEPFileMapperUCSF.h>
#include <vtkDebugLeaks.h>


using namespace svk;


vtkCxxRevisionMacro(svkGEPFileMapperUCSF, "$Rev$");
vtkStandardNewMacro(svkGEPFileMapperUCSF);


/*!
 *
 */
svkGEPFileMapperUCSF::svkGEPFileMapperUCSF()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkGEPFileMapperUCSF");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

}


/*!
 *
 */
svkGEPFileMapperUCSF::~svkGEPFileMapperUCSF()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}


/*!
 *  Determine the number of sampled k-space points in the data set. 
 *  This may differ from the number of voxels in the rectalinear grid, 
 *  for example if elliptical or another non rectangular acquisition 
 *  sampling strategy was employed.  GE product sequences pad the
 *  reduced k-space data with zeros so the number of k-space points 
 *  is the same as the number of voxels, but that may not be true for
 *  custom sequences.  
 */
int svkGEPFileMapperUCSF::GetNumKSpacePoints()
{

    int numKSpacePts;

    //  Image user21 indicates that an elliptical k-space sampling radius 
    //  was used.  If true, then number of k-space points differs from numVoxels
    //  in rectaliniear grid. 
    if (this->GetHeaderValueAsInt( "rhi.user21" ) != 0 ) { 

        float ellipticalRad =  this->GetHeaderValueAsFloat( "rhi.user22" ); 
        numKSpacePts = 0; 

        int numVoxels[3]; 
        this->GetNumVoxels( numVoxels ); 

        for( int z = 1; z <= numVoxels[2]; z++ ) {

            float zCorner = 2.0 * z - numVoxels[2] - 1.0;
            if (zCorner > 0) {
                zCorner += 1.0;
            } else {
                zCorner -= 1.0;
            }

            for( int y = 1; y <= numVoxels[1]; y++ ) {

                float yCorner = 2.0 * y - numVoxels[1] - 1.0;
                if (yCorner > 0) {
                    yCorner += 1.0;
                } else {
                    yCorner -= 1.0;
                }

                for( int x = 1; x <= numVoxels[0]; x++ ) {

                    float xCorner = 2.0 * x - numVoxels[0] - 1.0;
                    if (xCorner > 0) {
                        xCorner += 1.0;
                    } else {
                        xCorner -= 1.0;
                    }

                    float test = 
                        ( ( xCorner * xCorner ) / (float)( numVoxels[0] * numVoxels[0] ) )
                      + ( ( yCorner * yCorner ) / (float)( numVoxels[1] * numVoxels[1] ) ) 
                      + ( ( zCorner * zCorner ) / (float)( numVoxels[2] * numVoxels[2] ) );

                    // is it inside ellipse?
                    if ( test <= ellipticalRad ) {
                        numKSpacePts++;
                    }
                }     // for xstep
            }     // for ystep
        }     // for zstep

    } else {
        numKSpacePts = GetNumVoxelsInVol(); 
    }

    if ( this->GetDebug() ) {
        cout << "NUM KSPACE PTS: " << numKSpacePts << endl; 
    }

    return numKSpacePts; 
}


/*!
 *  Determines whether a voxel (index) was sampled, or not, i.e. was it within 
 *  the elliptical sampling volume if reduced k-space elliptical sampling
 *  was used.  Could be extended to support other sparse sampling 
 *  trajectories.       
 */
bool svkGEPFileMapperUCSF::WasIndexSampled(int indexX, int indexY, int indexZ)
{

    bool wasSampled = true;

    //  Image user21 indicates that an elliptical k-space sampling radius 
    //  was used.  If true, then number of k-space points differs from numVoxels
    //  in rectaliniear grid. 
    if (this->GetHeaderValueAsInt( "rhi.user21" ) != 0 ) { 
    
        float ellipticalRad =  this->GetHeaderValueAsFloat( "rhi.user22" ); 

        //  if ellipse is defined by bounding rectangle that 
        //  defines k-space grid, then

        int numVoxels[3]; 
        this->GetNumVoxels( numVoxels ); 

        /*  Get the origin of the ellipse
         *  and length of principle axes defined by 
         *  bounding box (acquisition grid)of ellipse
         *  Also get the exterior corner of the voxel in question 
         *  to see if it is 100% within the sampling ellipse 
         *  radius. 
         */
        float ellipseOrigin[3]; 
        float ellipseRadius[3]; 
        float voxelCorner[3]; 
        voxelCorner[0] = indexX; 
        voxelCorner[1] = indexY; 
        voxelCorner[2] = indexZ; 
        for (int i = 0; i < 3; i++) {

            ellipseOrigin[i] = ( static_cast<float>( numVoxels[i] ) - 1 ) / 2; 
            ellipseRadius[i] =   static_cast<float>( numVoxels[i] ) / 2; 

            if ( voxelCorner[i] < ellipseOrigin[i] ) {
                voxelCorner[i] -= .5; 
            } else {
                voxelCorner[i] += .5; 
            }

        }

        float voxelExteriorRadius = 0.;  
        for (int i = 0; i < 3; i++) {
            voxelExteriorRadius += pow( 
                    ( (voxelCorner[i] - ellipseOrigin[i]) / ellipseRadius[i] ), 
                    2 
                ); 
        }

        /*  See if the exterior corner of the voxel is within the sampled elliptical radius. 
         *   
         *  The eqn of the ellips in a bounding box defined by the MRSI acquisition grid    
         *  with center c (ellipseOrigin) and radius r (ellipseRadius) is if the size of the axes
         *  are ordered correctly such that rx > ry > rz > 0:
         *      (x-cx)^2 + (y-cy)^2 + (z-cz)^2
         *      --------   --------   --------   
         *        rx^2       ry^2       rz^2 
         */
        if ( voxelExteriorRadius <= ellipticalRad) {
            wasSampled = true;  
        } else { 
            wasSampled = false;  
        }

    } 

    return wasSampled; 
}

