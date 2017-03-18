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


#include <svkVoxelTaggingUtils.h>
#include <svkTypeUtils.h>

using namespace svk;

//vtkCxxRevisionMacro(svkVoxelTaggingUtils, "$Rev$");
vtkStandardNewMacro(svkVoxelTaggingUtils);

//! Constructor
svkVoxelTaggingUtils::svkVoxelTaggingUtils()
{
}


//! Destructor
svkVoxelTaggingUtils::~svkVoxelTaggingUtils()
{
}


/*!
 * Determines if image data object contains tag information. This is
 * determined simply by checking for the existence of the
 * SVK_VOXEL_TAGGING_SEQUENCE in the header.
 */
bool svkVoxelTaggingUtils::IsImageVoxelTagData( svkImageData* data )
{
	if( data != NULL && data->IsA("svkMriImageData") ) {
		return data->GetDcmHeader()->ElementExists("SVK_VOXEL_TAGGING_SEQUENCE");
	} else {
		return false;
	}
}


/*!
 *  Tag the given voxel for the input dataset. The tag applied is based
 *  on the active scalar volume. If the voxel is tagged already it will
 *  be untagged.
 */
void svkVoxelTaggingUtils::ToggleVoxelTag( svkImageData* voxelTagData, int voxelID, int tagVolume )
{
	if( svkVoxelTaggingUtils::IsImageVoxelTagData( voxelTagData )) {
		if( tagVolume == -1) {
			tagVolume = svkVoxelTaggingUtils::GetPointDataScalarVolumeIndex( voxelTagData );
		}
		svkMriImageData* mriData = svkMriImageData::SafeDownCast( voxelTagData );
		double* pixelValue = mriData->GetImagePixel(voxelID);
		int value = svkVoxelTaggingUtils::GetTagValue( mriData, tagVolume );
		if( pixelValue[0] == value ) {
			mriData->SetImagePixel(voxelID, 0 );
		} else {
			mriData->SetImagePixel(voxelID, value );
		}
		voxelTagData->Modified();
	}
}


/*!
 *  Gets the tag value for the given volume.
 */
int svkVoxelTaggingUtils::GetTagValue( svkImageData* voxelTagData, int tagVolume )
{
	int tagValue = 0;
	if( svkVoxelTaggingUtils::IsImageVoxelTagData( voxelTagData )) {
		tagValue = voxelTagData->GetDcmHeader()->GetIntSequenceItemElement(
			"SVK_VOXEL_TAGGING_SEQUENCE",
			tagVolume,
			"SVK_VOXEL_TAG_VALUE"
		);
	}
	return tagValue;
}


/*!
 *  Sets the tag value for the given volume.
 */
void svkVoxelTaggingUtils::SetTagValue( svkImageData* voxelTagData, int tagValue, int tagVolume )
{
	if( svkVoxelTaggingUtils::IsImageVoxelTagData( voxelTagData ) && tagValue != 0 ) {
		int numTags = svkVoxelTaggingUtils::GetNumberOfTags( voxelTagData );
		if( numTags > tagVolume ) {
			voxelTagData->GetDcmHeader()->AddSequenceItemElement("SVK_VOXEL_TAGGING_SEQUENCE", tagVolume, "SVK_VOXEL_TAG_VALUE", tagValue );
			vtkDataArray* tagData = voxelTagData->GetPointData()->GetArray(tagVolume);
			int numTuples = tagData->GetNumberOfTuples();
			for( int i = 0; i < numTuples; i++ ) {
				double oldValue = tagData->GetComponent(i,0);
				if( oldValue != 0 ) {
					tagData->SetComponent(i,0,tagValue);
				}
			}
			voxelTagData->Modified();
		}
	}
}


/*!
 *  Gets the tag name for the given volume.
 */
string svkVoxelTaggingUtils::GetTagName( svkImageData* voxelTagData, int tagVolume )
{
	string tagName = "";
	if( svkVoxelTaggingUtils::IsImageVoxelTagData( voxelTagData )) {
		tagName = voxelTagData->GetDcmHeader()->GetStringSequenceItemElement(
			"SVK_VOXEL_TAGGING_SEQUENCE",
			tagVolume,
			"SVK_VOXEL_TAG_NAME"
		);
	}
	return tagName;
}


/*!
 *  Sets the tag name for the given volume.
 */
void svkVoxelTaggingUtils::SetTagName( svkImageData* voxelTagData, string tagName, int tagVolume )
{
	if( svkVoxelTaggingUtils::IsImageVoxelTagData( voxelTagData )) {
		int numTags = svkVoxelTaggingUtils::GetNumberOfTags( voxelTagData );
		if( numTags > tagVolume ) {
			voxelTagData->GetDcmHeader()->AddSequenceItemElement("SVK_VOXEL_TAGGING_SEQUENCE", tagVolume, "SVK_VOXEL_TAG_NAME", tagName.c_str() );
		}
		voxelTagData->Modified();
	}
}


/*!
 * Returns the number of tags;
 */
int svkVoxelTaggingUtils::GetNumberOfTags( svkImageData* voxelTagData )
{
	int numTags = -1;
	if( svkVoxelTaggingUtils::IsImageVoxelTagData( voxelTagData )) {
		numTags = voxelTagData->GetDcmHeader()->GetNumberOfItemsInSequence("SVK_VOXEL_TAGGING_SEQUENCE");
	}
	return numTags;

}


/*!
 * Creates a voxel tagging volume from the input 4D dataset.
 */
svkMriImageData* svkVoxelTaggingUtils::CreateVoxelTagData( svk4DImageData* volumeToTag, vector<string> tagNames, vector<int> tagValues )
{

	svkMriImageData* voxelTagData = NULL;
	if( tagNames.size() > 0 && tagNames.size() == tagValues.size() ) {
		voxelTagData = svkMriImageData::New();
		volumeToTag->GetZeroImage( voxelTagData );
		voxelTagData->GetDcmHeader()->SetValue( "SVK_PRIVATE_TAG",  "SVK_PRIVATE_CREATOR");
	    voxelTagData->GetDcmHeader()->InsertEmptyElement( "SVK_VOXEL_TAGGING_SEQUENCE" );
		// Get the number of tuples
		int numTuples = voxelTagData->GetPointData()->GetArray(0)->GetNumberOfTuples();

		// Remove any previous data
		voxelTagData->GetPointData()->RemoveArray( voxelTagData->GetPointData()->GetArray(0)->GetName());

        vtkDataObject::SetPointDataActiveScalarInfo(
             voxelTagData->GetInformation(),
             VTK_UNSIGNED_SHORT,
             voxelTagData->GetNumberOfScalarComponents()
        );


        voxelTagData->GetDcmHeader()->SetPixelDataType(svkDcmHeader::UNSIGNED_INT_2);
		for( int i = 0; i < tagNames.size(); i ++ ) {
			svkVoxelTaggingUtils::AddTagToVoxelData(voxelTagData, tagNames[i], tagValues[i] );
		}

		string volumeToTagDescription = volumeToTag->GetDcmHeader()->GetStringValue("SeriesDescription");
		volumeToTagDescription.append(" Tag Data");
        voxelTagData->GetDcmHeader()->SetValue( "SeriesDescription", volumeToTagDescription.c_str());
	}

	return voxelTagData;
}


/*!
 * Adds a new tagging volume to the dataset
 */
void svkVoxelTaggingUtils::AddTagToVoxelData( svkImageData* voxelTagData, string tagName, int tagValue )
{
	if( voxelTagData != NULL &&  voxelTagData->IsA("svkMriImageData")) {
		int numTuples = voxelTagData->GetNumberOfPoints();
		int newIndex = voxelTagData->GetDcmHeader()->GetNumberOfItemsInSequence("SVK_VOXEL_TAGGING_SEQUENCE");
		svkVoxelTaggingUtils::InsertTagIntoHeader( voxelTagData->GetDcmHeader(), tagName, tagValue);
		vtkDataArray* newVolumeArray = vtkDataArray::CreateDataArray( VTK_UNSIGNED_SHORT );
		newVolumeArray->SetNumberOfComponents(1);
		newVolumeArray->SetNumberOfTuples(numTuples);
		newVolumeArray->FillComponent(0,0);
		string arrayName = "pixels";
		arrayName.append( svkTypeUtils::IntToString(newIndex) );
		newVolumeArray->SetName(arrayName.c_str());
		voxelTagData->GetPointData()->AddArray( newVolumeArray );
		newVolumeArray->Delete();

		svkVoxelTaggingUtils::FixPerFrameFunctionalGroupSequence( voxelTagData );
		voxelTagData->Modified();

	}
}


/*!
 * Remove a tag volume from the dataset.
 */
void svkVoxelTaggingUtils::RemoveTagFromVoxelData( svkImageData* voxelTagData, int tagVolume )
{
	if( svkVoxelTaggingUtils::IsImageVoxelTagData( voxelTagData ) ){
		int numTags = svkVoxelTaggingUtils::GetNumberOfTags( voxelTagData );
		if( tagVolume >= 0 && tagVolume < numTags ) {
			vector<string> tagNames;
			vector<int> tagValues;
			for( int i = 0; i < numTags; i++ ) {
				tagNames.push_back( svkVoxelTaggingUtils::GetTagName( voxelTagData, i));
				tagValues.push_back( svkVoxelTaggingUtils::GetTagValue( voxelTagData, i));
			}
			voxelTagData->GetDcmHeader()->ClearSequence("SVK_VOXEL_TAGGING_SEQUENCE");

			// Lets rebuild the voxel tagging sequence
			for( int i = 0; i < numTags; i++ ) {
				if( i != tagVolume ) {
					svkVoxelTaggingUtils::InsertTagIntoHeader( voxelTagData->GetDcmHeader(), tagNames[i], tagValues[i]);
				}
			}

			// We need to track the current scalars so we can reset it once the extra volume is removed
			int scalarIndex = svkVoxelTaggingUtils::GetPointDataScalarVolumeIndex( voxelTagData );
			if( scalarIndex >= tagVolume && scalarIndex != 0 ) {
				scalarIndex--;
			}

			char* arrayToRemoveName = voxelTagData->GetPointData()->GetArray( tagVolume )->GetName();

			// Now lets remove the extra array
			voxelTagData->GetPointData()->RemoveArray( arrayToRemoveName  );

			// And rename the other arrays....
			for( int i = 0; i < voxelTagData->GetPointData()->GetNumberOfArrays(); i++ ) {
				string arrayName = "pixels";
				arrayName.append( svkTypeUtils::IntToString(i) );
				voxelTagData->GetPointData()->GetArray(i)->SetName(arrayName.c_str());
				if( i == scalarIndex ) {
					voxelTagData->GetPointData()->SetActiveScalars( arrayName.c_str());
				}
			}
			svkVoxelTaggingUtils::FixPerFrameFunctionalGroupSequence( voxelTagData );
			voxelTagData->Modified();
		}
	}
}


/*!
 *  Creates a map of values to tag names.
 */
map<int, string> svkVoxelTaggingUtils::GetTagValueToNameMap( svkImageData* voxelTagData )
{
	map<int, string> voxelTagDefinitions;
	if( svkVoxelTaggingUtils::IsImageVoxelTagData( voxelTagData ) ) {
		int numTags = svkVoxelTaggingUtils::GetNumberOfTags( voxelTagData );
		for( int i = 0; i < numTags; i++ ) {
			string tagName = svkVoxelTaggingUtils::GetTagName( voxelTagData, i );
			int tagValue = svkVoxelTaggingUtils::GetTagValue( voxelTagData, i );
			voxelTagDefinitions[tagValue] = tagName;
		}
	}

	return voxelTagDefinitions;
}


/*!
 *  Gets the maximum tag value.
 */
int svkVoxelTaggingUtils::GetMaximumTagValue( svkImageData* voxelTagData )
{
	int max = -1;
	if( svkVoxelTaggingUtils::IsImageVoxelTagData( voxelTagData )) {
		int numTags = svkVoxelTaggingUtils::GetNumberOfTags( voxelTagData );
		for( int i = 0; i < numTags; i++ ) {
			int tagValue = svkVoxelTaggingUtils::GetTagValue( voxelTagData, i );
			if( tagValue > max ) {
				max = tagValue;
			}
		}
	}
	return max;
}


/*!
 *  Gets the minimum tag value.
 */
int svkVoxelTaggingUtils::GetMinimumTagValue( svkImageData* voxelTagData )
{
	int min = -1;
	if( svkVoxelTaggingUtils::IsImageVoxelTagData( voxelTagData )) {
		int numTags = svkVoxelTaggingUtils::GetNumberOfTags( voxelTagData );
		for( int i = 0; i < numTags; i++ ) {
			int tagValue = svkVoxelTaggingUtils::GetTagValue( voxelTagData, i );
			if( tagValue < min || i == 0 ) {
				min = tagValue;
			}
		}
	}
	return min;
}


/*!
 *  Gets the index of the scalar data array in the point data.
 */
int svkVoxelTaggingUtils::GetPointDataScalarVolumeIndex( svkImageData* voxelTagData )
{
	int scalarVolume = -1;
	if( svkVoxelTaggingUtils::IsImageVoxelTagData(voxelTagData)) {
		vtkDataArray* scalars = voxelTagData->GetPointData()->GetScalars();
		for( int i = 0; i < voxelTagData->GetPointData()->GetNumberOfArrays(); i++) {
			if( voxelTagData->GetPointData()->GetArray(i) == scalars ) {
				scalarVolume = i;
			}
		}
	}
	return scalarVolume;
}


/*!
 * Inserts tag information into the header.
 */
void svkVoxelTaggingUtils::InsertTagIntoHeader( svkDcmHeader* header, string tagName, int tagValue )
{
	int newIndex = header->GetNumberOfItemsInSequence("SVK_VOXEL_TAGGING_SEQUENCE");
	header->AddSequenceItemElement("SVK_VOXEL_TAGGING_SEQUENCE", newIndex, "SVK_PRIVATE_TAG", "SVK_PRIVATE_CREATOR" );
	header->AddSequenceItemElement("SVK_VOXEL_TAGGING_SEQUENCE", newIndex, "SVK_VOXEL_TAG_NAME", tagName.c_str() );
	header->AddSequenceItemElement("SVK_VOXEL_TAGGING_SEQUENCE", newIndex, "SVK_VOXEL_TAG_VALUE", tagValue );
}


/*!
 * Fixes the per frames functional groups sequence when a volume/tag is added or removed.
 */
void svkVoxelTaggingUtils::FixPerFrameFunctionalGroupSequence( svkImageData* voxelTagData )
{
	//Now we have to fix the per frame functional groups sequence:
	int numSlices = voxelTagData->GetDcmHeader()->GetNumberOfSlices();
	double dcos[3][3];
	voxelTagData->GetDcmHeader()->GetDataDcos( dcos );

	double pixelSpacing[3];
	voxelTagData->GetDcmHeader()->GetPixelSpacing( pixelSpacing );

	double toplc[3];
	voxelTagData->GetDcmHeader()->GetOrigin( toplc, 0 );
	int numVolumes = voxelTagData->GetPointData()->GetNumberOfArrays();
	//Update the per frames functional group sequence.
	
    svkDcmHeader::DimensionVector dimensionVector = voxelTagData->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, numSlices-1);
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::TIME_INDEX, numVolumes-1);
	voxelTagData->GetDcmHeader()->InitPerFrameFunctionalGroupSequence(toplc, pixelSpacing, dcos, &dimensionVector); 
}
