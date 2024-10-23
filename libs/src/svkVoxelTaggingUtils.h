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


#ifndef SVK_VOXEL_TAGGING_UTILS_H
#define SVK_VOXEL_TAGGING_UTILS_H

#include </usr/include/vtk/vtkObjectFactory.h>
#include <svkImageData.h>
#include <svkMriImageData.h>
#include <svkUtils.h>
#include <string>
#include <map>

namespace svk {


using namespace std;

/*! 
 *  Voxel tagging utility methods specific utilities.
 */
class svkVoxelTaggingUtils : public vtkObject
{

    public:

		static bool             IsImageVoxelTagData( svkImageData* data );
		static void             ToggleVoxelTag( svkImageData* voxelTagData, int voxelID, int tagVolume = -1 );
		static int              GetTagValue( svkImageData* voxelTagData, int tagVolume );
		static void             SetTagValue( svkImageData* voxelTagData, int tagValue, int tagVolume );
		static string           GetTagName( svkImageData* voxelTagData, int tagVolume );
		static void             SetTagName( svkImageData* voxelTagData, string tagName, int tagVolume );
		static int              GetNumberOfTags( svkImageData* voxelTagData);
		static svkMriImageData* CreateVoxelTagData( svk4DImageData* volumeToTag, vector<string> tagNames, vector<int> tagValues );
		static void             AddTagToVoxelData( svkImageData* voxelTagData, string tagName, int tagValue );
		static void             RemoveTagFromVoxelData( svkImageData* voxelTagData, int tagVolume );
		static map<int, string> GetTagValueToNameMap( svkImageData* voxelTagData );
		static int              GetMaximumTagValue( svkImageData* voxelTagData );
		static int              GetMinimumTagValue( svkImageData* voxelTagData );
		static int              GetPointDataScalarVolumeIndex( svkImageData* voxelTagData );

        // vtk type revision macro
        vtkTypeMacro( svkVoxelTaggingUtils, vtkObject );
  
        // vtk initialization 
        static svkVoxelTaggingUtils* New();


	protected:

		static void             InsertTagIntoHeader( svkDcmHeader* header, string tagName, int tagValue );
		static void             FixPerFrameFunctionalGroupSequence( svkImageData* voxelTagData );

       svkVoxelTaggingUtils();
       ~svkVoxelTaggingUtils();
        
};


}   //svk



#endif //SVK_VOXEL_TAGGING_UTILS_H
