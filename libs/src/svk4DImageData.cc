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
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/libs/src/svk4DImageData.cc $
 *  $Rev: 1054 $
 *  $Author: jccrane $
 *  $Date: 2011-09-16 16:01:45 -0700 (Fri, 16 Sep 2011) $
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#include <svk4DImageData.h>
#include <svkEnhancedMRIIOD.h>


using namespace svk;


vtkCxxRevisionMacro(svk4DImageData, "$Rev: 1054 $");
vtkStandardNewMacro(svk4DImageData);


/*!
 * Constructor.
 */
svk4DImageData::svk4DImageData()
{

    this->topoGenerator = NULL;

}


/*!
 *  This is used by vtkInstantiator and is used in svk algorithms
 */
vtkObject* svk4DImageData::NewObject()
{
    return vtkObject::SafeDownCast( svk4DImageData::New() );
}


/*!
 *  Destructor.
 */
svk4DImageData::~svk4DImageData()
{
    vtkDebugMacro(<<"svk4DImageData::~svk4DImageData");
    if( topoGenerator != NULL ) {
        topoGenerator->Delete();
        topoGenerator = NULL;
    }
}


/*!
 *  Returns the number of voxels in the data set. 
 */
void svk4DImageData::GetNumberOfVoxels(int numVoxels[3])
{
    numVoxels[0] = (this->GetExtent())[1];
    numVoxels[1] = (this->GetExtent())[3];
    numVoxels[2] = (this->GetExtent())[5];
}

 
/*! 
 *  Makes sure the range gets updated when the object is modified. It searches all arrays for each
 *  component to determine maximum and minimums
 */     
void svk4DImageData::UpdateRange( int component )
{
    int* extent = this->GetExtent(); 
    double realRange[2];
    double imagRange[2];
    double magRange[2];
    realRange[0] = VTK_DOUBLE_MAX;
    realRange[1] = VTK_DOUBLE_MIN;
    imagRange[0] = VTK_DOUBLE_MAX;
    imagRange[1] = VTK_DOUBLE_MIN;
    magRange[0] = VTK_DOUBLE_MAX;
    magRange[1] = VTK_DOUBLE_MIN;
    int numFrequencyPoints = this->GetCellData()->GetNumberOfTuples();
    float* tuple;
    double magnitude;
    for( int i = 0; i < this->GetCellData()->GetNumberOfArrays(); i ++ ) {
        vtkFloatArray* array = static_cast<vtkFloatArray*>( this->GetArray(i) );
        float* arrayPtr = array->GetPointer(0);
            for (int i = 0; i < numFrequencyPoints; i++) {
                tuple = arrayPtr + 2*i;
                if( component == 0 ){
                    realRange[0] = tuple[0] < realRange[0]
                        ? tuple[0] : realRange[0];
                    realRange[1] = tuple[0] > realRange[1]
                        ? tuple[0] : realRange[1];
                } else if (component == 1 ) {
                    imagRange[0] = tuple[1] < imagRange[0]
                        ? tuple[1] : imagRange[0];
                    imagRange[1] = tuple[1] > imagRange[1]
                        ? tuple[1] : imagRange[1];
                } else if (component == 2 ) {

                    magnitude = pow( pow(static_cast<double>(tuple[0]), static_cast<double>(2) )
                            + pow( static_cast<double>(tuple[1]), static_cast<double>(2)),0.5);

                    magRange[0] = magnitude < magRange[0]
                        ? magnitude : magRange[0];
                    magRange[1] = magnitude > magRange[1]
                        ? magnitude : magRange[1];
                }
            }
    }
    this->SetDataRange( realRange, svkImageData::REAL );
    this->SetDataRange( imagRange, svkImageData::IMAGINARY );
    this->SetDataRange( magRange, svkImageData::MAGNITUDE );
}


/*!
 *  Get the last slice index for a given orientation. This is different for cell data so
 *  that is why it is overloaded.
 *
 *  \param sliceOrientation the orientation whose last slice you wish to get
 *  \return the last slice
 */
int svk4DImageData::GetLastSlice( svkDcmHeader::Orientation sliceOrientation )
{
    return this->Superclass::GetLastSlice( sliceOrientation ) - 1;
}


/*!
 *  Gets the closests slice for a given LPS coordinate, and a sliceOrientation.
 *
 *  \param posLPS the position in LPS coordinates
 *  \param orientation the orientation of the slice you wish to select 
 */
int svk4DImageData::GetClosestSlice(double* posLPS, svkDcmHeader::Orientation sliceOrientation )
{

    double origin[3] = {0,0,0};
    this->GetDcmHeader()->GetOrigin(origin, 0);
    double* spacing = this->GetSpacing();

    int slice = this->FindMatchingSlice( posLPS, sliceOrientation, origin, spacing ); 

    return slice;
}


/*!
 *  This will get the top left corner, and bottom right hand corner (low index-high index) for 
 *  a given selection and slice. It assumes the userSelection defines a minimum and maximum
 *  range and that the user wishes to select all voxels within that range (for a given slice)
 *  in a regular box aligned with the dcos having corners at the minimum and maximum values.
 *  If no slice is defined 
 *
 *  \param tlcBrc the destination for the result of the calculation
 *  \param userSelection the [minx, maxx, miny, maxy, minz, maxz] selection range 
 *  \param slice the slice within which to make the selection, if slice is outside of the range
 *               then the resulting tlcBrc will span the slices in the given selection
 */
void svk4DImageData::GetTlcBrcInUserSelection( int tlcBrc[2], double userSelection[6], 
                                                svkDcmHeader::Orientation orientation, int slice )
{
    orientation = (orientation == svkDcmHeader::UNKNOWN_ORIENTATION ) ?
                                this->GetDcmHeader()->GetOrientationType() : orientation;
    if( userSelection != NULL ) {
        double worldStart[3]; 
        double worldEnd[3]; 
        worldStart[0] = userSelection[0]; 
        worldStart[1] = userSelection[2]; 
        worldStart[2] = userSelection[4]; 
        worldEnd[0] = userSelection[1]; 
        worldEnd[1] = userSelection[3]; 
        worldEnd[2] = userSelection[5]; 

        int tlcIndex[3];
        int brcIndex[3];
        this->GetIndexFromPosition( worldStart, tlcIndex );
        this->GetIndexFromPosition( worldEnd, brcIndex );
        int* extent = this->GetExtent();
        
        int tmp;
        for( int i = 0; i < 3; i++ ) {
            if( tlcIndex[i] > brcIndex[i] ) {
                tmp = brcIndex[i]; 
                brcIndex[i] = tlcIndex[i];
                tlcIndex[i] = tmp;
            }
        }

        // This checks for out of bounds, if out of bounds use the end of the extent
        tlcIndex[2] = (tlcIndex[2] >= extent[5]) ? extent[5]-1 : tlcIndex[2];
        tlcIndex[1] = (tlcIndex[1] >= extent[3]) ? extent[3]-1 : tlcIndex[1];
        tlcIndex[0] = (tlcIndex[0] >= extent[1]) ? extent[1]-1 : tlcIndex[0];
        brcIndex[2] = (brcIndex[2] >= extent[5]) ? extent[5]-1 : brcIndex[2];
        brcIndex[1] = (brcIndex[1] >= extent[3]) ? extent[3]-1 : brcIndex[1];
        brcIndex[0] = (brcIndex[0] >= extent[1]) ? extent[1]-1 : brcIndex[0];
        tlcIndex[2] = (tlcIndex[2] < extent[4]) ? extent[4] : tlcIndex[2];
        tlcIndex[1] = (tlcIndex[1] < extent[2]) ? extent[2] : tlcIndex[1];
        tlcIndex[0] = (tlcIndex[0] < extent[0]) ? extent[0] : tlcIndex[0];
        brcIndex[2] = (brcIndex[2] < extent[4]) ? extent[4] : brcIndex[2];
        brcIndex[1] = (brcIndex[1] < extent[2]) ? extent[2] : brcIndex[1];
        brcIndex[0] = (brcIndex[0] < extent[0]) ? extent[0] : brcIndex[0];
        int lastSlice  = this->GetLastSlice( orientation );
        int firstSlice = this->GetFirstSlice( orientation );
        if( slice >= firstSlice && slice <= lastSlice ) {
            brcIndex[ this->GetOrientationIndex( orientation) ] = slice; 
            tlcIndex[ this->GetOrientationIndex( orientation) ] = slice; 
        }
        tlcBrc[0] = tlcIndex[2]*extent[3] * extent[1] + tlcIndex[1]*extent[1] + tlcIndex[0]; 
        tlcBrc[1] = brcIndex[2]*extent[3] * extent[1] + brcIndex[1]*extent[1] + brcIndex[0]; 
    }
} 


/*!
 *   Method will extract a volume into a vtkImageData object representing
 *   a single point in the spectra. This is usefull for spatial FFT's.
 *
 *  \param target image the point image (must be initialized)
 *  \param point the point in the array you wish operate on 
 *  \param component the component to operate on 
 *  \param timePoint the time point to operate on 
 *  \param channel the the channel to operate on 
 *  \param component (0 = real, 1=im, 2=cmplx) 
 *
 */
void  svk4DImageData::GetImage(  svkImageData* image,
                                 int point,
                                 vtkstd::string seriesDescription,
                                 int* indexArray,
                                 int component )
{
    if( image != NULL ) {

        svkEnhancedMRIIOD* iod = svkEnhancedMRIIOD::New();
        iod->SetDcmHeader( image->GetDcmHeader() );
        iod->InitDcmHeader();
        iod->Delete(); 
        
        this->GetDcmHeader()->ConvertMrsToMriHeader( 
            image->GetDcmHeader(), 
            VTK_DOUBLE, 
            seriesDescription
        );

        this->GetImage( image, point, indexArray, component );
    }
}


/*!
 *   Method will extract a volume into a vtkImageData object representing
 *   a single point in the spectra. This is usefull for spatial FFT's.
 *
 *  \param target image the point image (must be initialized)
 *  \param point the point in the array you wish operate on 
 *  \param component the component to operate on 
 *  \param timePoint the time point to operate on 
 *  \param channel the the channel to operate on 
 *  \param component (0 = real, 1=im, 2=cmplx) 
 *
 */
void  svk4DImageData::GetImage(  svkImageData* image,
                                 int point,
                                 int* indexArray,
                                 int component )
{
    if( image != NULL ) {

        int numComponents = 2; 
        if ( component < 2 ) {
            numComponents = 1; 
        } 
        //this->Update();
        // Setup image dimensions
        image->SetExtent( Extent[0], Extent[1]-1, Extent[2], Extent[3]-1, Extent[4], Extent[5]-1);

        image->SetScalarTypeToDouble( );
        image->SetNumberOfScalarComponents( numComponents );
        image->AllocateScalars();

        image->CopyDcos( this );
        image->GetIncrements();

        // Create a float array to hold the pixel data
        vtkDoubleArray* pixelData = vtkDoubleArray::New();
        pixelData->SetNumberOfComponents( numComponents );
        pixelData->SetNumberOfTuples( (image->GetDimensions()[0])*(image->GetDimensions()[1])*(image->GetDimensions()[2]) );
        pixelData->SetName("pixels");
        double* pixels = pixelData->GetPointer(0);
        int linearIndex = 0;
        int* dims = image->GetDimensions();
        double* tuple;
        vtkDataArray* array;
        for (int z = Extent[4]; z < Extent[5]; z++) {
            for (int y = Extent[2]; y < Extent[3]; y++) {
                for (int x = Extent[0]; x < Extent[1]; x++) {
                    array = this->GetArray( x, y, z, indexArray );

                    linearIndex = ( z * (dims[0]) * (dims[1]) ) + ( y * (dims[0]) ) + x;
                    tuple = array->GetTuple( point );
                    if ( numComponents == 2 ) {
                        pixels[2*linearIndex] = tuple[0];
                        pixels[2*linearIndex+1] = tuple[1];
                    } else {
                        pixels[linearIndex] = tuple[ component ];
                    }
                }
            }
        }
        image->GetPointData()->SetScalars( pixelData );
        image->Modified();

        pixelData->Delete();
    }

}


/*!
 *  Determins the number of slices for a given orientation.
 */
int svk4DImageData::GetNumberOfSlices( svkDcmHeader::Orientation sliceOrientation)
{
    sliceOrientation = (sliceOrientation == svkDcmHeader::UNKNOWN_ORIENTATION ) ? 
                                this->GetDcmHeader()->GetOrientationType() : sliceOrientation;
    int index = this->GetOrientationIndex( sliceOrientation );
    return this->GetDimensions()[index] - 1;
 
}


/*!
 *   Attempts to estimate the data range for a given frequency range. It finds the
 *   average maximum/minimum then adds three standard deviations. This should cover
 *   around 99% of the peaks.
 *
 * @param range
 * @param minPt
 * @param maxPt
 * @param component
 * @param tlcBrc
 * @param indexArray
 */
void svk4DImageData::EstimateDataRange( double range[2], int minPt, int maxPt, int component, int* tlcBrc, int* indexArray )
{
        range[0] = VTK_DOUBLE_MAX;
        range[1] = VTK_DOUBLE_MIN;
        vtkDataArray* array = NULL;
        int min[3] = {Extent[0], Extent[2], Extent[4]}; 
        int max[3] = {Extent[1], Extent[3], Extent[5]}; 
        if( tlcBrc != NULL ) {
            this->GetIndexFromID( tlcBrc[0], min );
            this->GetIndexFromID( tlcBrc[1], max );
        }
        double rangeAverage[2] = {0,0};
        double stdDeviation[2] = {0,0};
        int numVoxels = (max[0]-min[0]+1)*(max[1]-min[1]+1)*(max[2]-min[2]+1);
        vector<double> maxValues;
        vector<double> minValues;

        // Find the average and the max/min for each voxel
        for (int z = min[2]; z <= max[2]; z++) {
            for (int y = min[1]; y <= max[1]; y++) {
                for (int x = min[0]; x <= max[0]; x++) {
                    array = this->GetArray( x, y, z, indexArray );
                    if( array != NULL ) {
                        double localRange[2] = {VTK_DOUBLE_MAX, VTK_DOUBLE_MIN};
                        for( int i = minPt; i < maxPt; i++ ) {
                            double value = array->GetTuple(i)[component];
                            localRange[0] = value < localRange[0] ? value: localRange[0];
                            localRange[1] = value > localRange[1] ? value: localRange[1];
                        }
                        rangeAverage[0]+=localRange[0] / numVoxels; 
                        rangeAverage[1]+=localRange[1] / numVoxels; 
                        minValues.push_back( localRange[0] );
                        maxValues.push_back( localRange[1] );
                    }
                }
            }
        }

        for( int i = 0; i < numVoxels; i++ ) {
            stdDeviation[0] += pow(minValues[i]-rangeAverage[0],2 )/numVoxels;
            stdDeviation[1] += pow(maxValues[i]-rangeAverage[1],2 )/numVoxels;
        }
        stdDeviation[0] = pow(stdDeviation[0], 0.5);
        stdDeviation[1] = pow(stdDeviation[1], 0.5);
        range[0] = rangeAverage[0] - 3*stdDeviation[0];
        range[1] = rangeAverage[1] + 3*stdDeviation[1];

}


/*!
 *   Method will set a spectral point from a vtkImageData object representing
 *   a single point in the spectra. This is usefull for spatial FFT's.
 *
 *  \param image the point image 
 *  \param point the point in the array you wish operate on 
 *  \param timePoint the time point to operate on 
 *  \param channel the the channel to operate on 
 *
 */
void  svk4DImageData::SetImage( vtkImageData* image, int point, int* indexArray )
{
    if( image != NULL ) {
        int linearIndex = 0;
        double value;
        double range[2];
        vtkDataArray* imageScalars = image->GetPointData()->GetScalars();
        for (int z = Extent[4]; z < Extent[5]; z++) {
            for (int y = Extent[2]; y < Extent[3]; y++) {
                for (int x = Extent[0]; x < Extent[1]; x++) {
                    vtkDataArray* array = this->GetArray( x, y, z, indexArray );
                    linearIndex = ( z * (Dimensions[0]-1) * (Dimensions[1]-1) ) + ( y * (Dimensions[0]-1) ) + x; 
                    for( int i = 0; i < array->GetNumberOfComponents(); i++ ) {
                        value = imageScalars->GetComponent( linearIndex, i );
                        array->SetComponent( point, i, value );

                    }
                }
            }
        }
    }
}


/*!
 * Gets an array based on the xyz index of the voxel and
 * a volumetric index array.
 *
 * @param x
 * @param y
 * @param z
 * @param indexArray
 * @return
 */
vtkDataArray* svk4DImageData::GetArray(int x, int y, int z, int* indexArray )

{
    vtkstd::vector<int> newIndexArray;
    // First three index are spatial
    newIndexArray.push_back( x );
    newIndexArray.push_back( y );
    newIndexArray.push_back( z );


    int* dims = this->GetDimensions();
    for( int i = 0; i < this->GetNumberOfVolumeDimensions(); i++ ) {
        newIndexArray.push_back( indexArray[i] );
    }

    return this->GetArray(  static_cast<int*>(newIndexArray.data()) );

}


/*!
 * Gets an array based on the index array input. It assumes that
 * the order of the indecies represent the data ordering of the
 * arrays in the data set.
 *
 * @param indexArray
 * @return
 */
vtkDataArray* svk4DImageData::GetArray( int* indexArray )
{
    int linearIndex = 0;
    // We have three spatial dimension + our volume dimensions
    int numIndices = 3 + this->GetNumberOfVolumeDimensions();
    int* linearIndexSize = new int[ 3 + this->GetNumberOfVolumeDimensions()];
    linearIndexSize[0] = this->Dimensions[0] - 1;
    linearIndexSize[1] = this->Dimensions[1] - 1;
    linearIndexSize[2] = this->Dimensions[2] - 1;
    for( int i = 0; i < this->GetNumberOfVolumeDimensions(); i++ ) {
        linearIndexSize[ 3 + i ] = this->GetVolumeIndexSize( i );
    }

    // Calculate the linear index.
    for( int i = 0; i < numIndices; i++ ) {
        int stepSize = 1;
        for( int j = i - 1; j >= 0; j-- ) {
            stepSize *= linearIndexSize[j];
        }
        linearIndex += indexArray[i] * stepSize;
    }
    delete[] linearIndexSize;
    return this->GetArray( linearIndex );
}


/*!
 * Gets the array by linear index.
 *
 * @param linearIndex
 * @return
 */
vtkDataArray* svk4DImageData::GetArray( int linearIndex )
{
    return this->GetCellData()->GetArray( linearIndex );
}


/*!
 *   Gets a specified data array from linear index.
 */
vtkDataArray* svk4DImageData::GetArrayFromID( int index, int* indexArray )
{
    int indexX;
    int indexY;
    int indexZ;
    this->GetIndexFromID( index, &indexX, &indexY, &indexZ );
    return this->GetArray( indexX, indexY, indexZ, indexArray );
}


/*!
 * Gets the size of the given volume Index
 *
 * @param volumeIndex
 * @return
 */
int svk4DImageData::GetVolumeIndexSize( int volumeIndex )
{
    if( volumeIndex == 0 ) {
        return this->GetDcmHeader()->GetNumberOfCoils();
    } else {
        return -1;
    }
}


/*!
 * Currently assumes one dimension.
 * TODO: Use the header to dynamically determine the number of dimensions
 *
 * @return
 */
int svk4DImageData::GetNumberOfVolumeDimensions( )
{
    return 1;
}
