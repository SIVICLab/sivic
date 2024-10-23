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



#include <svkMrsImageFlip.h>
#include </usr/include/vtk/vtkImageFlip.h>


using namespace svk;


//vtkCxxRevisionMacro(svkMrsImageFlip, "$Rev$");
vtkStandardNewMacro(svkMrsImageFlip);


/*!
 *  Constructor.  Initialize any member variables. 
 */
svkMrsImageFlip::svkMrsImageFlip()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    //  Initialize any member variables
    this->filteredAxis = 0; 
    this->filterDimVector = NULL; 
}


/*!
 *  Clean up any allocated member variables. 
 */
svkMrsImageFlip::~svkMrsImageFlip()
{
}


/*
 *  Sets a set of indices to flip. By default will iterate through all non spatial domains 
 *  and flip all 3D volumes.  If this dimVector is set, any index that is >=0 will be used
 *  to limit which volumes get flipped.  For example the following input would limit the 
 *  flips to only volumes with EPSI_ACQ_INDEX == 1: 
 *      SLICE_INDEX     = -1; 
 *      CHANNEL_INDEX   = -1; 
 *      EPSI_ACQ_INDEX  = 1; 
 *  By default filters all the dimVector is null and all vols get flipped in the specified orientation. 
 */
void svkMrsImageFlip::SetFilterDomainIndices( svkDcmHeader::DimensionVector* dimVector)
{
    this->filterDimVector = dimVector; 
}


/*
 *  Sets the axis to reverse: 0 = cols(x), 1 = rows(y), 2 = slice(z)
 *  Data is reversed along this axis, e.g. if axis is set to 0, the 
 *  voxel columns are revsed (voxel in col 0 becomes voxel n col N-1, 
 *  and voxels in col N-1 becomes voxels in col 0, etc.  This can also 
 *  be thought of as reverse the data in each row or along the x direction. 
 */
void svkMrsImageFlip::SetFilteredAxis( int axis )
{
    this->filteredAxis = axis; 
}


/*! 
 *  This method is called during pipeline execution.  This is where you should implement your algorithm. 
 */
int svkMrsImageFlip::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    //  Get pointer to input data set. 
    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 

    //  Get pointer to data's meta-data header (DICOM object). 
    svkDcmHeader* hdr = mrsData->GetDcmHeader();  

    int numSpecPts = hdr->GetIntValue( "DataPointColumns" );

    vtkImageData* tmpData = NULL;
    svkMriImageData* singleFreqImage = svkMriImageData::New();

    //  Initialize the dim vector representing the input volume: 
    svkDcmHeader::DimensionVector dimVector = hdr->GetDimensionIndexVector();

    //  Retain the spatial info to put into the target DimVector for GetImage
    int voxelDims[3];
    svkDcmHeader::GetSpatialDimensions(&dimVector, voxelDims);
    
    //  Initialze the dim vector that limits filtering
    //  if not set initialize it to all -1 values, implying that all volumes are filtered. 
    if ( this->filterDimVector == NULL )  {
        *this->filterDimVector = dimVector; 
        for ( int i = 0; i < this->filterDimVector->size(); i++ ) {
            svkDcmHeader::SetDimensionVectorValue(this->filterDimVector, i, -1); 
        }
    }

    //  Only loop over non spatial voxels: 
    svkDcmHeader::SetDimensionVectorValue(&dimVector, svkDcmHeader::COL_INDEX,   0);
    svkDcmHeader::SetDimensionVectorValue(&dimVector, svkDcmHeader::ROW_INDEX,   0);
    svkDcmHeader::SetDimensionVectorValue(&dimVector, svkDcmHeader::SLICE_INDEX, 0);
    svkDcmHeader::DimensionVector loopVector = dimVector; 

    //  get number of volumes (cells in this case with spatial dims set to 0)
    int numCells = svkDcmHeader::GetNumberOfCells( &dimVector );

    if (this->GetDebug()) {
        svkDcmHeader::PrintDimensionIndexVector( &dimVector ); 
    }

    for ( int cellID = 0; cellID < numCells; cellID++ ) {

        //  Get the dim vector for this loop iteration: 
        svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimVector, &loopVector, cellID );

        //  for the indices in this iteration, check if any are restriced by this->filterDimVector 
        //  if the filterDimVector index is set, but it doesn't match this iteration, then skip to next
        bool flipVolume = true; 
        for ( int dimIndex = 0; dimIndex < this->filterDimVector->size(); dimIndex++ ) {
            int loopIndexValue = svkDcmHeader::GetDimensionVectorValue(&loopVector, dimIndex); 
            int filterDimValue = svkDcmHeader::GetDimensionVectorValue(this->filterDimVector, dimIndex); 
            if ( filterDimValue >=0 && loopIndexValue != filterDimValue ) {
                if (this->GetDebug()) {
                    svkDcmHeader::PrintDimensionIndexVector( this->filterDimVector); 
                }
                flipVolume = false;  
                break; 
            }
        }
        
        if ( flipVolume == true ) { 

            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::COL_INDEX,   voxelDims[0] -1 );
            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::ROW_INDEX,   voxelDims[1] -1 );
            svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::SLICE_INDEX, voxelDims[2] -1 );

            for( int freq = 0; freq < numSpecPts; freq++ ) {

                //mrsData->GetImage( singleFreqImage, freq, timePt, coil, 2, "");
                mrsData->GetImage( singleFreqImage, freq, &loopVector, 2, "");  

                singleFreqImage->Modified();

                tmpData = singleFreqImage;

                vtkImageFlip* flip = vtkImageFlip::New();
            
                flip->SetFilteredAxis( this->filteredAxis ); 

                flip->SetInputData( tmpData ); 
                tmpData = flip->GetOutput();
                flip->Update();
           

                mrsData->SetImage( tmpData, freq, &loopVector); 
                //mrsData->SetImage( tmpData, freq, timePt, coil);

                flip->Delete(); 

            }
        }
    }

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();

    return 1; 
} 


/*!
 *  Set the input data type to svkMrsImageData.
 */
int svkMrsImageFlip::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


