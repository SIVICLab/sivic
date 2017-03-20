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


#include <svkMrsImageData.h>


using namespace svk;


//vtkCxxRevisionMacro(svkMrsImageData, "$Rev$");
vtkStandardNewMacro(svkMrsImageData);


/*!
 * Constructor.
 */
svkMrsImageData::svkMrsImageData()
{
    this->numChannels = 0;
}


/*!
 *  Destructor.
 */
svkMrsImageData::~svkMrsImageData()
{
}


/*!
 *  This is used by vtkInstantiator and is used in svk algorithms
 */
vtkObject* svkMrsImageData::NewObject()
{
    return vtkObject::SafeDownCast( svkMrsImageData::New() );
}


/*!
 * Populates the argument indexArray
 * based on the input paramateres of channel and timePoint. These
 * arrays are used in the parent class for general data access.
 *
 * @param channel
 * @param timePoint
 * @param indexArray
 */
void svkMrsImageData::GetIndexArray( int timePoint, int channel,  int* indexArray )
{
    indexArray[svkMrsImageData::TIMEPOINT] = timePoint;
    indexArray[svkMrsImageData::CHANNEL] = channel;
}


/*!
 * Gets the spectrum at the linear index i
 *
 * @param i
 * @return
 */
vtkDataArray* svkMrsImageData::GetSpectrum( int i )
{
    return this->GetArray( i );
}


/*!
 *   Gets a specified data array from linear index.
 */
vtkDataArray* svkMrsImageData::GetSpectrumFromID( int index, int timePoint, int channel)
{
    int indexArray[2] = { -1, -1 };
    this->GetIndexArray( timePoint, channel, indexArray );
    return this->GetArrayFromID( index, indexArray );

}


/*!
 *   Gets a specified data array.
 */
vtkDataArray* svkMrsImageData::GetSpectrum( int i, int j, int k, int timePoint, int channel)
{
    int indexArray[2] = { -1, -1 };
    this->GetIndexArray( timePoint, channel, indexArray );
    return this->GetArray( i, j, k, indexArray );
}


/*!
 * Gets the point volume image for the given index. User can specify a series description
 * for the new volume.
 *
 * @param image
 * @param point
 * @param timePoint
 * @param channel
 * @param component
 * @param seriesDescription
 */
void  svkMrsImageData::GetImage( svkMriImageData* image, int point, int timePoint, int channel,
int component, string seriesDescription, int vtkDataType)
{
    int indexArray[2] = { -1, -1 };
    this->GetIndexArray( timePoint, channel, indexArray );
    this->Superclass::GetImage( image, point, seriesDescription, indexArray, component, vtkDataType );
}


/*!
 * Gets the 3D point volume image for the given dimensionIndex. User can specify a series description
 * for the new volume.
 *
 * @param image
 * @param point (frequency or time point from spectral dimension)
 * @param dimensionIndexVector with non spatial indices representing 3D volume to retrieve 
 *          only the non spatial indices are relevant. 
 * @param channel
 * @param component
 * @param seriesDescription
 */
void  svkMrsImageData::GetImage( svkMriImageData* image, int point, svkDcmHeader::DimensionVector* dimensionVector, 
int component, string seriesDescription, int vtkDataType)
{
    this->Superclass::GetImage( image, point, seriesDescription, dimensionVector, component, vtkDataType );
}


/*!
 * Sets the point volume image for the given index.
 *
 * @param image
 * @param point
 * @param timePoint
 * @param channel
 */
void svkMrsImageData::SetImage( vtkImageData* image, int point, int timePoint, int channel )
{
    int indexArray[2] = { -1, -1 };
    this->GetIndexArray( timePoint, channel, indexArray );
    this->Superclass::SetImage( image, point, indexArray );

}


/*!
 * Sets the point volume image for the given index.
 *
 * @param image
 * @param point (frequency or time point from spectral domain)
 * @param dimensionVector represents non spatial volume indices to inset 3D image into
 * @param channel
 */
void svkMrsImageData::SetImage( vtkImageData* image, int point, svkDcmHeader::DimensionVector* dimensionVector )
{
    this->Superclass::SetImage( image, point, dimensionVector);
}


/*!
 * Sets the point volume image for the given index.
 *
 * @param image
 * @param point
 * @param timePoint
 * @param channel
 */
void svkMrsImageData::SetImageComponent( vtkImageData* image, int point, int timePoint, int channel, int component )
{
    int indexArray[2] = { -1, -1 };
    this->GetIndexArray( timePoint, channel, indexArray );
    this->Superclass::SetImageComponent( image, point, indexArray, component );

}

/*!
 *  Gets the closests slice for a given LPS coordinate, and a sliceOrientation.
 *
 *  \param posLPS the position in LPS coordinates
 *  \param orientation the orientation of the slice you wish to select
 */
int svkMrsImageData::GetClosestSlice(double* posLPS, svkDcmHeader::Orientation sliceOrientation, double tolerance )
{

    int slice = -1;
    //  this should be the origin of the selection box
    //  (i.e. treat as a single slice).
    string acquisitionType = this->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
    if( acquisitionType == "SINGLE VOXEL" && this->HasSelectionBox() ) {
        double origin[3] = {0,0,0};
        double spacing[3] = {0,0,0};
        this->GetSelectionBoxCenter( origin );
        this->GetSelectionBoxSpacing( spacing );
        slice = this->FindMatchingSlice( posLPS, sliceOrientation, origin, spacing, tolerance );
    } else {
       slice = Superclass::GetClosestSlice( posLPS, sliceOrientation, tolerance );
    }

    return slice;

}


/*!
 *  Gets the size for a given volume index.
 *
 * @param volumeIndex
 * @return
 */
int svkMrsImageData::GetVolumeIndexSize( int volumeIndex )
{
    if( volumeIndex == 0 ) {
        return this->GetDcmHeader()->GetNumberOfTimePoints();
    } else if( volumeIndex == 1 ) {
        return this->GetDcmHeader()->GetNumberOfCoils();
    } else {
        return -1;
    }
}


/*!
 *  svkMrsImageData has only two dimension indices: channel, timepoint.
 *
 * @return
 */
int svkMrsImageData::GetNumberOfVolumeDimensions( )
{
    return 2;
}
/*!
 *  Gets the number of channels in the dataset. The first time it is called
 *  it gets the number of channels from the header, after that it stores
 *  the value in a member variable.
 */
int svkMrsImageData::GetNumberOfChannels()
{
    if( this->numChannels == 0 ) {
        this->numChannels = this->GetDcmHeader()->GetNumberOfCoils();
    }
    return this->numChannels;
}


/*!
 * Estimates the data range for the given timepoint, channel, index range, and voxel range.
 *
 * @param range output object
 * @param minPt lowest point to consider in determining range
 * @param maxPt highest point to consider in determining range
 * @param component the component you want to calculate
 * @param tlcBrc the voxel range you are interested in
 * @param timePoint the timePoint you are interested in
 * @param channel the channel you are interested in
 */
void svkMrsImageData::EstimateDataRange( double range[2], int minPt, int maxPt, int component, int* tlcBrc, int timePoint, int channel)
{
        string acquisitionType = this->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
        int indexArray[2] = { -1, -1 };
        this->GetIndexArray( timePoint, channel, indexArray );
        if( acquisitionType != "SINGLE VOXEL" ) {
            int tlcBrcInSelection[2];
            this->GetTlcBrcInSelectionBox( tlcBrcInSelection );
            Superclass::EstimateDataRange(range, minPt, maxPt, component, tlcBrcInSelection, indexArray );
        } else {
            Superclass::EstimateDataRange(range, minPt, maxPt, component, tlcBrc, indexArray );
        }
}


bool svkMrsImageData::HasSelectionBox( )
{
	bool hasSelectionBox = false;
    int numberOfItems = this->GetDcmHeader()->GetNumberOfItemsInSequence("VolumeLocalizationSequence");
    if( numberOfItems == 3) {
    	hasSelectionBox = true;
    }
    return hasSelectionBox;
}


/*!
 *  Creates an unstructured grid that represents the selection box.
 *
 *  \param selectionBoxGrid the object to populate
 */
void svkMrsImageData::GenerateSelectionBox( vtkUnstructuredGrid* selectionBoxGrid )
{
    svkDcmHeader* header = this->GetDcmHeader();
    int numberOfItems = header->GetNumberOfItemsInSequence("VolumeLocalizationSequence");
    if( numberOfItems == 3 && selectionBoxGrid != NULL) {

        double thickness;
        double* center = new double[3];
        double* normal = new double[3];

        vtkPoints* planePoints = vtkPoints::New();
        vtkFloatArray* planeNormals = vtkFloatArray::New();
        planeNormals->SetNumberOfComponents(3);
        planeNormals->SetNumberOfTuples(numberOfItems * 2);

        int index = 0;   // Used to index planes, as opposed to slabs

        for (int i = 0; i < numberOfItems; i++) {

            thickness = header->GetFloatSequenceItemElement("VolumeLocalizationSequence", i, "SlabThickness" );

            center[0] = strtod(header->GetStringSequenceItemElement(
                        "VolumeLocalizationSequence", i, "MidSlabPosition", 0).c_str(),NULL);
            center[1] = strtod(header->GetStringSequenceItemElement(
                        "VolumeLocalizationSequence", i, "MidSlabPosition", 1).c_str(),NULL);
            center[2] = strtod(header->GetStringSequenceItemElement(
                        "VolumeLocalizationSequence", i, "MidSlabPosition", 2).c_str(),NULL);

            normal[0] = strtod(header->GetStringSequenceItemElement(
                        "VolumeLocalizationSequence", i, "SlabOrientation", 0).c_str(),NULL);
            normal[1] = strtod(header->GetStringSequenceItemElement(
                        "VolumeLocalizationSequence", i, "SlabOrientation", 1).c_str(),NULL);
            normal[2] = strtod(header->GetStringSequenceItemElement(
                        "VolumeLocalizationSequence", i, "SlabOrientation", 2).c_str(),NULL);

            //  Top Point of plane
            planePoints->InsertPoint(index, center[0] + normal[0] * (thickness/2),
                    center[1] + normal[1] * (thickness/2),
                    center[2] + normal[2] * (thickness/2) );
            planeNormals->SetTuple3( index, normal[0], normal[1], normal[2] );

            index++; // index is per plane

            //  Bottom Point of plane
            planePoints->InsertPoint(index, center[0] - normal[0] * (thickness/2),
                    center[1] - normal[1] * (thickness/2),
                    center[2] - normal[2] * (thickness/2) );
            planeNormals->SetTuple3( index, -normal[0], -normal[1], -normal[2] );

            index++;
        }

        // vtkPlanesIntersection represents a convex region defined by an arbitrary number of planes
        vtkPlanesIntersection* region = vtkPlanesIntersection::New();
        region->SetPoints( planePoints );
        region->SetNormals( planeNormals );
        int numVerticies = 8; // a hexahedron has 8 verticies
        int numDims = 3;
        double* vertices = new double[numVerticies*numDims];
        // Here we get the vertices of the region
        region->GetRegionVertices( vertices, numVerticies );

        // Lets put those vertices into a vtkPoints object
        vtkPoints* selectionBoxPoints = vtkPoints::New();
        selectionBoxPoints->SetNumberOfPoints(numVerticies);
        for( int i = 0; i < numVerticies; i++ ) {
            selectionBoxPoints->InsertPoint(i, vertices[i*numDims],
                    vertices[i*numDims + 1],
                    vertices[i*numDims + 2]   );
        }
        // And now lets use a Hexahedron to represent them.
        // The point ID's must be in a specific order for this to work,
        // hence the variation in SetId calls. See the vtkHexahedron documentation.
        vtkHexahedron* selectionBox = vtkHexahedron::New();
        selectionBox->GetPointIds()->SetNumberOfIds( numVerticies );
        selectionBox->GetPointIds()->SetId(0, 0);
        selectionBox->GetPointIds()->SetId(1, 4);
        selectionBox->GetPointIds()->SetId(2, 6);
        selectionBox->GetPointIds()->SetId(3, 2);
        selectionBox->GetPointIds()->SetId(4, 1);
        selectionBox->GetPointIds()->SetId(5, 5);
        selectionBox->GetPointIds()->SetId(6, 7);
        selectionBox->GetPointIds()->SetId(7, 3);



        // We need an object that can be mapped, so we but the hexahedron into on UnstructuredGrid
        /*
         * We are going to convert the hexahedron to polydata. This is so we can make only consist of edges.
         * The reason for this that when using vtkRenderLargeImage SetRepresentationToWireframe fails.
         */
        selectionBoxGrid->Initialize();
        selectionBoxGrid->Allocate(1, 1);
        selectionBoxGrid->SetPoints(selectionBoxPoints);
        selectionBoxGrid->InsertNextCell(selectionBox->GetCellType(), selectionBox->GetPointIds());
        planePoints->Delete();
        selectionBoxPoints->Delete();
        planeNormals->Delete();
        selectionBox->Delete();
        region->Delete();
        delete[] center;
        delete[] normal;
        delete[] vertices;


    }
}


/*!
 *  Calculate the center of the selection box.
 *
 *  \param selBoxCenter the array to populate in LPS coords
 */
void svkMrsImageData::GetSelectionBoxCenter( double* selBoxCenter )
{

    selBoxCenter[0] = 0;
    selBoxCenter[1] = 0;
    selBoxCenter[2] = 0;

    vtkUnstructuredGrid* selBox = vtkUnstructuredGrid::New();
    this->GenerateSelectionBox( selBox );
    vtkPoints* selBoxPoints = selBox->GetPoints();

    if( selBoxPoints != NULL ) {
        int numPoints = selBoxPoints->GetNumberOfPoints();
        for( int i = 0; i < 3; i++ ) {
            for( int j = 0; j < numPoints; j++ ) {
                selBoxCenter[i] += selBoxPoints->GetPoint(j)[i];
            }
            selBoxCenter[i] /= numPoints;
        }
        selBox->Delete();
    }

}


/*!
 *  Gets the dimension of the selection box.
 */
void svkMrsImageData::GetSelectionBoxDimensions( float* dims )
{
    svkDcmHeader* header = this->GetDcmHeader();
    int numberOfItems = header->GetNumberOfItemsInSequence("VolumeLocalizationSequence");
    if( numberOfItems == 3 ) {
        for (int i = 0; i < numberOfItems; i++) {
            dims[i] = header->GetFloatSequenceItemElement("VolumeLocalizationSequence", i, "SlabThickness" );
        }
    } else {
        dims[0] = 0;
        dims[1] = 0;
        dims[2] = 0;
    }

}


/*!
 *  Method determines if the given MRS slice is within the selection box.
 *
 *  \return true if the slice is within the selection box, other wise false is returned
 */
bool svkMrsImageData::IsSliceInSelectionBox( int slice, svkDcmHeader::Orientation orientation )
{
    orientation = (orientation == svkDcmHeader::UNKNOWN_ORIENTATION ) ?
        this->GetDcmHeader()->GetOrientationType() : orientation;

    int voxelIndex[3] = {0,0,0};

    double normal[3];
    this->GetSliceNormal( normal, orientation );
    double sliceNormal[3] = { normal[0], normal[1], normal[2] };
    voxelIndex[ this->GetOrientationIndex( orientation ) ] = slice;
    if( slice > this->GetLastSlice( orientation ) || slice < this->GetFirstSlice( orientation ) ) {
        return false;
    }

    vtkGenericCell* sliceCell = vtkGenericCell::New();
    this->GetCell( this->ComputeCellId(voxelIndex), sliceCell );
    vtkUnstructuredGrid* uGrid = vtkUnstructuredGrid::New();
    this->GenerateSelectionBox( uGrid );
    vtkPoints* selBoxPoints = uGrid->GetPoints();
    double projectedSelBoxRange[2];
    projectedSelBoxRange[0] = VTK_DOUBLE_MAX;
    projectedSelBoxRange[1] = -VTK_DOUBLE_MAX;
    double projectedDistance;
    if( selBoxPoints == NULL ) {
        return false;
    }
    for( int i = 0; i < selBoxPoints->GetNumberOfPoints(); i++) {
        projectedDistance = vtkMath::Dot( selBoxPoints->GetPoint(i), sliceNormal );
        if( projectedDistance < projectedSelBoxRange[0]) {
            projectedSelBoxRange[0] = projectedDistance;
        }
        if( projectedDistance > projectedSelBoxRange[1]) {
            projectedSelBoxRange[1] = projectedDistance;
        }
    }

    double projectedSliceRange[2];
    projectedSliceRange[0] = VTK_DOUBLE_MAX;
    projectedSliceRange[1] = -VTK_DOUBLE_MAX;
    for( int i = 0; i < sliceCell->GetPoints()->GetNumberOfPoints(); i++) {
        projectedDistance = vtkMath::Dot( sliceCell->GetPoints()->GetPoint(i), sliceNormal );
        if( projectedDistance < projectedSliceRange[0]) {
            projectedSliceRange[0] = projectedDistance;
        }
        if( projectedDistance > projectedSliceRange[1]) {
            projectedSliceRange[1] = projectedDistance;
        }
    }
    sliceCell->Delete();
    double selBoxCenter = projectedSelBoxRange[0] +  (projectedSelBoxRange[1] - projectedSelBoxRange[0])/2;
    double sliceCenter = projectedSliceRange[0] + (projectedSliceRange[1] - projectedSliceRange[0])/2;
    bool inSlice = false;
    if( ( projectedSelBoxRange[0] > projectedSliceRange[0] && projectedSelBoxRange[0] < sliceCenter
        )   || ( projectedSelBoxRange[1] > sliceCenter && projectedSelBoxRange[1] < projectedSliceRange[1]
            )   || ( sliceCenter > projectedSelBoxRange[0] && sliceCenter < projectedSelBoxRange[1]
                ) ){
        inSlice = true;
    }
    uGrid->Delete();
    return inSlice;
}


/*!
 *  Get the spacing of the selection box
 *
 *  \param spacing target array
 */
void svkMrsImageData::GetSelectionBoxSpacing( double spacing[3] )
{
    int numberOfItems = this->GetDcmHeader()->GetNumberOfItemsInSequence("VolumeLocalizationSequence");
    if( numberOfItems == 3 ) {
        spacing[0] = this->GetDcmHeader()->GetFloatSequenceItemElement("VolumeLocalizationSequence", 0, "SlabThickness" );
        spacing[1] = this->GetDcmHeader()->GetFloatSequenceItemElement("VolumeLocalizationSequence", 1, "SlabThickness" );
        spacing[2] = this->GetDcmHeader()->GetFloatSequenceItemElement("VolumeLocalizationSequence", 2, "SlabThickness" );
    } else {
        spacing[0] = 0;
        spacing[1] = 0;
        spacing[2] = 0;
    }

}


/*!
 *  Get the origin of the selection box as defined in the images coordinate system (dcos).
 *  The origin is defined as the point minimum point in the 3 dcos directions.
 *
 *  eg.
 *      For vector (0,0,1) the origin would be the most negative z value.
 *      For vector (0,0,-1) the origin would be the most positive z value.
 *
 *  To calculate the origin we start by getting the vertecies of the selection
 *  box. The gemoetry of the selection box is determined by the method
 *  GenerateSelectionBox which stores the results as an unstructured grid of
 *  points with a single cell that represent the volume of the selection. As
 *  such the points are not ordered so we have no idea which point is the
 *  origin relative to our dcos.
 *
 *  To make this determination we take the dot product of each point with each axis' unit
 *  vector in the dcos. This gives us a projection of the point onto a vector that points
 *  along the direction of the axis of the dataset and intersects 0. If we project each
 *  point onto each axis the "origin" is going to have the lowest value when projected
 *  onto each axis.
 *
 *  To avoid precision errors we are going to sum the distances in all three projected
 *  directions and choose the minimum value. These precision errors are caused when
 *  you have four points that define a surface of the selection box and you project
 *  them onto the axis perpendicular to the plane. Geometrically these should all
 *  project to exactly the same value (assuming the selection box is perfectly aligned
 *  with the dcos), but in practice this calculation can be off by a very small amount.
 *  This error becomes insignificant when you sum the distances in all three directions.
 *
 *
 *  \param orgin target array
 */
void svkMrsImageData::GetSelectionBoxOrigin(  double origin[3] )
{

    vtkUnstructuredGrid* uGrid = vtkUnstructuredGrid::New();
    this->GenerateSelectionBox( uGrid );

    // Row Normal is Parallel to the DICOM colums
    double rowNormal[3];
    this->GetDataBasis( rowNormal, svkImageData::ROW );

    // Column Normal is Parallel to the DICOM rows
    double columnNormal[3];
    this->GetDataBasis( columnNormal, svkImageData::COLUMN );

    double sliceNormal[3];
    this->GetDataBasis( sliceNormal, svkImageData::SLICE );

    // We can get the points from the unstructured grid but the order is arbitrary
    vtkPoints* selBoxPoints = uGrid->GetPoints();
    int originIndex;

    // All distances are from the origin (0,0,0) to the point along the given normal vector.
    double rowDistanceMin;
    double columnDistanceMin;
    double sliceDistanceMin;
    double rowDistance;
    double columnDistance;
    double sliceDistance;

    // Lets start by assuming the first point is the minimum, then compare
    originIndex = 0;
    rowDistanceMin = vtkMath::Dot( selBoxPoints->GetPoint(originIndex), rowNormal );
    columnDistanceMin = vtkMath::Dot( selBoxPoints->GetPoint(originIndex), columnNormal );
    sliceDistanceMin = vtkMath::Dot( selBoxPoints->GetPoint(originIndex), sliceNormal );
    double minDistance = rowDistanceMin + sliceDistanceMin + columnDistanceMin;

    // Just initialize to a safe value
    double distance = VTK_DOUBLE_MAX;

    for( int i = 0; i < selBoxPoints->GetNumberOfPoints(); i++) {
        // Lets project the point onto each axis
        rowDistance = vtkMath::Dot( selBoxPoints->GetPoint(i), rowNormal );
        columnDistance = vtkMath::Dot( selBoxPoints->GetPoint(i), columnNormal );
        sliceDistance = vtkMath::Dot( selBoxPoints->GetPoint(i), sliceNormal );

        // Here we will sum the 3 distances to avoid precision errors
        distance = rowDistance + columnDistance + sliceDistance;

        // If it is the minimum then that is the origin
        if( distance < minDistance ) {
            minDistance = distance;
            originIndex = i;
        }

    }
    origin[0] = selBoxPoints->GetPoint(originIndex)[0];
    origin[1] = selBoxPoints->GetPoint(originIndex)[1];
    origin[2] = selBoxPoints->GetPoint(originIndex)[2];
    uGrid->Delete();
}


/*!
 *  Method takes all points that define our selection box and determines the maximum and minimum point.
 *
 *  \param minPoint the lowest point (most negative)
 *  \param maxPoint the highest point (most positive)
 *  \param tolerance ranges from 0-1, allows you to push slightly past the points for selection purposes
 */
void svkMrsImageData::GetSelectionBoxMaxMin( double minPoint[3], double maxPoint[3], double tolerance )
{
        double* spacing = this->GetSpacing();
        double* corner;
        double* minCorner;
        double* maxCorner;
        double projectedCorner[6];
        double thresholdBounds[6];

        double LRNormal[3];
        this->GetDataBasis(LRNormal, svkImageData::LR );
        double PANormal[3];
        this->GetDataBasis(PANormal, svkImageData::PA );
        double SINormal[3];
        this->GetDataBasis(SINormal, svkImageData::SI );
        vtkUnstructuredGrid* selectionBox = vtkUnstructuredGrid::New();
        this->GenerateSelectionBox( selectionBox );
        vtkPoints* cellBoxPoints = selectionBox->GetPoints();

        int minCornerIndex;
        int maxCornerIndex;
        // Case for no selection box
        if( cellBoxPoints == NULL ) {
            minPoint[0] = 0;
            minPoint[1] = 0;
            minPoint[2] = 0;
            maxPoint[0] = 0;
            maxPoint[1] = 0;
            maxPoint[2] = 0;
            return;
        }
        for( int i = 0; i < cellBoxPoints->GetNumberOfPoints(); i++ ) {
            corner = cellBoxPoints->GetPoint(i);
            if( i == 0 ) {
                minCorner = corner;
                maxCorner = corner;
                minCornerIndex = i;
                maxCornerIndex = i;
            }
            if( corner[0] <= minCorner[0]  &&  corner[1] <= minCorner[1] && corner[2] <= minCorner[2] ) {
                minCornerIndex = i;
                minCorner = corner;
            } else if( corner[0] >= maxCorner[0]   &&  corner[1] >= maxCorner[1] && corner[2] >= maxCorner[2] ) {
                maxCornerIndex = i;
                maxCorner = corner;
            }
        }

        minPoint[0] =(cellBoxPoints->GetPoint(minCornerIndex))[0]
                                             + vtkMath::Dot(spacing, LRNormal)*tolerance;
        minPoint[1] =(cellBoxPoints->GetPoint(minCornerIndex))[1]
                                             + vtkMath::Dot(spacing, PANormal)*tolerance;
        minPoint[2] =(cellBoxPoints->GetPoint(minCornerIndex))[2]
                                             + vtkMath::Dot(spacing, SINormal)*tolerance;
        maxPoint[0] =(cellBoxPoints->GetPoint(maxCornerIndex))[0]
                                             - vtkMath::Dot(spacing, LRNormal)*tolerance;
        maxPoint[1] =(cellBoxPoints->GetPoint(maxCornerIndex))[1]
                                             - vtkMath::Dot(spacing, PANormal)*tolerance;
        maxPoint[2] =(cellBoxPoints->GetPoint(maxCornerIndex))[2]
                                             - vtkMath::Dot(spacing, SINormal)*tolerance;
        selectionBox->Delete();

}


/*!
 *
 *  Calculates the top left corner, and bottom right corner (high index-low index) of all
 *  voxels that lie within the selection box for projected onto the given slice. Checks
 *  to make sure the given slices is within the range of the volume, if it is not then
 *  -1,-1 is returned as tlcBrc.
 *
 *  \param tlcBrc the destination for the result of the calculation
 *  \param slice the slice you wish to project the results onto.
 *  \param tolerance the tolerance ranges from 0.0-1.0 and determines what fraction of the
 *                   voxel must be within the selection box to be selected
 */
void svkMrsImageData::Get2DProjectedTlcBrcInSelectionBox( int tlcBrc[2], svkDcmHeader::Orientation orientation, int slice, double tolerance )
{
    int lastSlice  = this->GetLastSlice( orientation );
    int firstSlice = this->GetFirstSlice( orientation );
    if( slice >= firstSlice && slice <= lastSlice ) {
        this->GetTlcBrcInSelectionBox( tlcBrc, orientation, slice, tolerance );
    } else {
        tlcBrc[0] = -1;
        tlcBrc[1] = -1;
    }
}


/*!
 *  Calculates the top left corner, and bottom right corner (high index-low index) of all
 *  voxels that lie within the selection box for a given slice.
 *
 *  \param tlcBrc the destination for the result of the calculation
 *  \param slice the slice you wish to project the results onto. If the slice is not in the volume a 3D range is returned.
 *  \param tolerance the tolerance ranges from 0.0-1.0 and determines what fraction of the
 *                   voxel must be within the selection box to be selected
 */
void svkMrsImageData::GetTlcBrcInSelectionBox( int tlcBrc[2], svkDcmHeader::Orientation orientation, int slice, double tolerance )
{
        orientation = (orientation == svkDcmHeader::UNKNOWN_ORIENTATION ) ?
                                this->GetDcmHeader()->GetOrientationType() : orientation;
        double minPoint[3];
        double maxPoint[3];
        double selection[6];
        this->GetSelectionBoxMaxMin( minPoint, maxPoint, tolerance );
        // Case for no selection box.. select all voxels
        if( minPoint[0] == 0 && maxPoint[0] == 0 &&
            minPoint[1] == 0 && maxPoint[1] == 0 &&
            minPoint[2] == 0 && maxPoint[2] == 0 ) {
            // We are going to use min/max of int/2 to avoid overflow
            // The goal is to select All voxels
            selection[0] = VTK_INT_MIN/2;
            selection[1] = VTK_INT_MAX/2;
            selection[2] = VTK_INT_MIN/2;
            selection[3] = VTK_INT_MAX/2;
            selection[4] = VTK_INT_MIN/2;
            selection[5] = VTK_INT_MAX/2;
        } else {
            selection[0] = minPoint[0];
            selection[1] = maxPoint[0];
            selection[2] = minPoint[1];
            selection[3] = maxPoint[1];
            selection[4] = minPoint[2];
            selection[5] = maxPoint[2];
        }
        this->GetTlcBrcInUserSelection( tlcBrc, selection, orientation, slice );

}


/*!
 *  Calculates the top left corner, and bottom right corner (high index-low index) of all
 *  voxels that lie within the selection box for a given slice. Returns the index of the
 *  each voxels.
 *
 *  \param tlcVoxel the destination for the result
 *  \param brcVoxel the destination for the result
 *  \param tolerance the tolerance ranges from 0.0-1.0 and determines what fraction of the
 *                   voxel must be within the selection box to be selected
 */
void svkMrsImageData::Get3DVoxelsInSelectionBox( int tlcVoxel[3], int brcVoxel[3], double tolerance )
{
    int tlcBrc[2];
    this->GetTlcBrcInSelectionBox( tlcBrc, svkDcmHeader::UNKNOWN_ORIENTATION, -1, tolerance );
    this->GetIndexFromID( tlcBrc[0], tlcVoxel );
    this->GetIndexFromID( tlcBrc[1], brcVoxel );
}


/*!
 *  Calculates the top left corner, and bottom right corner (high index-low index) of all
 *  voxels that lie within the selection box for a given slice.
 *
 *  \param tlcBrc the destination for the result of the calculation
 *  \param tolerance the tolerance ranges from 0.0-1.0 and determines what fraction of the
 *                   voxel must be within the selection box to be selected
 */
void svkMrsImageData::Get3DTlcBrcInSelectionBox( int tlcBrc[3], double tolerance )
{
    this->GetTlcBrcInSelectionBox( tlcBrc, svkDcmHeader::UNKNOWN_ORIENTATION, -1, tolerance );
}


/*!
 *  Generates a 3D data array representing binary mask indicating whether a given voxel is within
 *  the selection box or the specified fraction (tolerane) of a voxel is within the selection box.
 *  length of mask array is number of voxels in data set.  mask values are 0, not in selected
 *  volume, or 1, in selected volume.  Mask must be pre allocated.
 */
void svkMrsImageData::GetSelectionBoxMask( short* mask, double tolerance )
{

    //  min/max represent the xyz inidices representing
    //  the rectanglular volume of voxels to quantify.
    //  Default is for the min/max to include all voxels
    int extent[6];
    this->GetExtent( extent );

    int min[3] = {extent[0], extent[2], extent[4]};
    int max[3] = {extent[1], extent[3], extent[5]};

    int numVoxels[3];
    this->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];

    if ( tolerance > 0 ) {

        //  Get the ID of the voxels that define the TLC and BRC of the
        //  the selection box for this slice:
        this->Get3DVoxelsInSelectionBox(
            min,
            max,
            tolerance
        );

        int voxelIndex[3];

        for (int voxelID = 0; voxelID < totalVoxels; voxelID++ ) {

            this->GetIndexFromID( voxelID, voxelIndex );


            //  compare the current voxel's indices to the tlcBrcID range:
            if (
                   ( (min[0] <= voxelIndex[0]) && (voxelIndex[0] <= max[0]) )
                && ( (min[1] <= voxelIndex[1]) && (voxelIndex[1] <= max[1]) )
                && ( (min[2] <= voxelIndex[2]) && (voxelIndex[2] <= max[2]) )
            ) {
                mask[voxelID] = 1;
            } else {
                mask[voxelID] = 0;
            }
        }

    } else {

        for (int voxelID = 0; voxelID < totalVoxels; voxelID++ ) {
            mask[voxelID] = 1;
        }

    }
}


/*!
 * Redimensions the image data object.
 *
 * WARNING: This will re-initialize all the arrays in the result to zero for all components!
 *
 */
void svkMrsImageData::Redimension( svkDcmHeader::DimensionVector* dimensionVector, double* newOrigin, double* newSpacing, bool resizeSelectionBoxToFOV )
{
    this->GetDcmHeader()->Redimension( dimensionVector, newOrigin, newSpacing );
    this->SyncVTKImageDataToDcmHeader();
    this->InitializeDataArrays( );
    if( resizeSelectionBoxToFOV ) {
        double dcosD[3][3];
        this->GetDcmHeader()->GetDataDcos(dcosD);
        float dcos[3][3];
        for (int i=0; i<3; i++) {
            for (int j=0; j<3; j++) {
                dcos[i][j] = static_cast<float>(dcosD[i][j]);
            }
        }
        double centerD[3] = {0};
        this->GetCenter(centerD);
        int* dims = this->GetDimensions();
        float size[3] = { static_cast<float>(newSpacing[0] * (dims[0]-1))
                         ,static_cast<float>(newSpacing[1] * (dims[1]-1))
                         ,static_cast<float>(newSpacing[2] * (dims[2]-1))};

        float center[3] = { static_cast<float>(centerD[0])
                           ,static_cast<float>(centerD[1])
                           ,static_cast<float>(centerD[2]) };

        this->GetDcmHeader()->InitVolumeLocalizationSeq(size, center, dcos);
    }
}


/*!
 *
 */
void svkMrsImageData::InitializeDataArrays( )
{
    // Remove All Existing Arrays
    int numArrays = this->GetCellData()->GetNumberOfArrays();
    int dataType = VTK_FLOAT;
    int numComponents = 2;
    for( int i = 0; i < numArrays; i++ ) {
        if( i == 0 ) {
            dataType = this->GetCellData()->GetArray(0)->GetDataType();
            numComponents = this->GetCellData()->GetArray(0)->GetNumberOfComponents();
        }
        this->GetCellData()->RemoveArray(this->GetCellData()->GetArray(0)->GetName());
    }
    // Read dimension index vector, and initialize arrays with appropriate names...
    svkDcmHeader::DimensionVector dimVec  = this->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector loopVec = dimVec;

    int numPts = this->GetDcmHeader()->GetIntValue( "DataPointColumns" );
    int numInputCells = svkDcmHeader::GetNumberOfCells( &dimVec );
    for (int cellID = 0; cellID < numInputCells; cellID++) {
        svkDcmHeader::GetDimensionVectorIndexFromCellID(&dimVec, &loopVec, cellID);
        vtkDataArray* dataArray = vtkDataArray::CreateDataArray(dataType);
        dataArray->SetNumberOfComponents(numComponents);
        dataArray->SetNumberOfTuples(numPts);
        dataArray->SetName( this->GetArrayName(&loopVec).c_str());
        for( int i = 0; i < numComponents; i++ ) {
            dataArray->FillComponent(i,0);
        }
        this->GetCellData()->AddArray(dataArray);
    }
}
