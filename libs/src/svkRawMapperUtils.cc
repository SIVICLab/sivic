/*
 *  Copyright © 2009-2012 The Regents of the University of California.
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
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/libs/src/svkRawMapperUtils.cc $
 *  $Rev: 1346 $
 *  $Author: jccrane $
 *  $Date: 2012-09-13 14:39:01 -0700 (Thu, 13 Sep 2012) $
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#include <svkRawMapperUtils.h>
#include <svkGEPFileMapper.h>
#include <vtkDebugLeaks.h>


using namespace svk;


vtkCxxRevisionMacro(svkRawMapperUtils, "$Rev: 1346 $");



/*!
 *  Redimension data after changing number of voxels and time points.  Requires explicitly setting the numCoils/Channels:  
 */
void svkRawMapperUtils::RedimensionData( svkImageData* data, int* numVoxelsOriginal, int* numVoxelsReordered, int numFreqPts, int numCoils )
{

    svkDcmHeader* hdr = data->GetDcmHeader();     

    int numTimePts = hdr->GetNumberOfTimePoints();

    //  This is the original origin based on the reduced dimensionality in the EPSI direction
    double origin[3];
    hdr->GetOrigin( origin, 0 );

    double voxelSpacing[3];
    hdr->GetPixelSpacing( voxelSpacing );

    double dcos[3][3];
    hdr->GetDataDcos( dcos );

    double center[3]; 
    svkGEPFileMapper::GetCenterFromOrigin( origin, numVoxelsOriginal, voxelSpacing, dcos, center); 

    //  Now calcuate the new origin based on the reordered dimensionality: 
    double newOrigin[3]; 
    svkGEPFileMapper::GetOriginFromCenter( center, numVoxelsReordered, voxelSpacing, dcos, newOrigin ); 

    hdr->SetValue( "Columns", numVoxelsReordered[0]);
    hdr->SetValue( "Rows", numVoxelsReordered[1]);
    hdr->SetValue( "DataPointColumns", numFreqPts );

    hdr->InitPerFrameFunctionalGroupSequence(
        newOrigin,
        voxelSpacing,
        dcos,
        numVoxelsReordered[2], 
        numTimePts,
        numCoils 
    );

    data->SyncVTKImageDataToDcmHeader(); 
}

