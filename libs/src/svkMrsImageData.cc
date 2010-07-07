/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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


vtkCxxRevisionMacro(svkMrsImageData, "$Rev$");
vtkStandardNewMacro(svkMrsImageData);


/*!
 * Constructor.
 */
svkMrsImageData::svkMrsImageData()
{
    topoGenerator = svkMrsTopoGenerator::New();
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    this->numChannels = 0;

}


/*!
 *  This is used by vtkInstantiator and is used in svk algorithms
 */
vtkObject* svkMrsImageData::NewObject()
{
    return vtkObject::SafeDownCast( svkMrsImageData::New() );
}


/*!
 *  Destructor.
 */
svkMrsImageData::~svkMrsImageData()
{
    vtkDebugMacro(<<"svkMrsImageData::~svkMrsImageData");
    if( topoGenerator != NULL ) {
        topoGenerator->Delete();
        topoGenerator = NULL;
    }
}


/*!
 *  Returns the number of voxels in the data set. 
 */
void svkMrsImageData::GetNumberOfVoxels(int numVoxels[3])
{
    numVoxels[0] = (this->GetExtent())[1];
    numVoxels[1] = (this->GetExtent())[3];
    numVoxels[2] = (this->GetExtent())[5];
}

 
/*!
 *  Creates an unstructured frid that represents the selection box.
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
 *   Gets a specified data array from linear index.
 */
vtkDataArray* svkMrsImageData::GetSpectrumFromID( int index, int timePoint, int channel)
{
    int indexX;
    int indexY;
    int indexZ;
    this->GetIndexFromID( index, &indexX, &indexY, &indexZ );
    return this->GetSpectrum( indexX, indexY, indexZ, timePoint, channel);
}


/*!
 *   Gets a specified data array.
 */
vtkDataArray* svkMrsImageData::GetSpectrum( int i, int j, int k, int timePoint, int channel)
{

    // We are getting the number of dimensions and numtimepoints to make sure the variables are up to date
    int* dims = this->GetDimensions();
    int numTimePoints = this->GetDcmHeader()->GetNumberOfTimePoints();

    int linearIndex = i + j * (dims[0]-1) + k * (dims[0]-1) * (dims[1]-1) 
                        + timePoint * (dims[0]-1) * (dims[1]-1) * (dims[2]-1)
                        + channel   * (dims[0]-1) * (dims[1]-1) * (dims[2]-1) * numTimePoints;
    return this->GetSpectrum( linearIndex ); 
}


/*!
 *   Gets a spectrum at the specified linear index 
 */
vtkDataArray* svkMrsImageData::GetSpectrum( int linearIndex )
{
    return this->GetCellData()->GetArray( linearIndex );
}


/*! 
 *  Makes sure the range gets updated when the object is modified. It searches all arrays for each
 *  component to determine maximum and minimums
 */     
void svkMrsImageData::UpdateRange( int component )
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
    int numChannels  = this->GetDcmHeader()->GetNumberOfCoils();
    int numTimePoints  = this->GetDcmHeader()->GetNumberOfTimePoints();
    int numFrequencyPoints = this->GetCellData()->GetNumberOfTuples();
    float* tuple;
    double magnitude;
    for (int z = extent[4]; z <= extent[5]-1; z++) {
        for (int y = extent[2]; y <= extent[3]-1; y++) {
            for (int x = extent[0]; x <= extent[1]-1; x++) {
                for( int channel = 0; channel < numChannels; channel++ ) {
                    for( int timePoint = 0; timePoint < numTimePoints; timePoint++ ) {
                        vtkFloatArray* spectrum = static_cast<vtkFloatArray*>( this->GetSpectrum( x, y, z, timePoint, channel ) );
                        float* spectrumPtr = spectrum->GetPointer(0);
                        for (int i = 0; i < numFrequencyPoints; i++) {
                            tuple = spectrumPtr + 2*i;
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
                }
            }
        }
    }
    this->SetDataRange( realRange, svkImageData::REAL );
    this->SetDataRange( imagRange, svkImageData::IMAGINARY );
    this->SetDataRange( magRange, svkImageData::MAGNITUDE );
}


/*!
 *  Method determines if the given MRS slice is within the selection box.
 *
 *  \return true if the slice is within the selection box, other wise false is returned
 */
bool svkMrsImageData::IsSliceInSelectionBox( int slice, svkDcmHeader::Orientation orientation )
{
    orientation = (orientation == svkDcmHeader::UNKNOWN ) ?
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
 *  Get the last slice index for a given orientation. This is different for cell data so
 *  that is why it is overloaded.
 *
 *  \param sliceOrientation the orientation whose last slice you wish to get
 *  \return the last slice
 */
int svkMrsImageData::GetLastSlice( svkDcmHeader::Orientation sliceOrientation )
{
    return this->Superclass::GetLastSlice( sliceOrientation ) - 1;
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
 *  Get the origin of the selection box as defined in the images coordinate system.
 *  The origin is defined as the point minimum point in the dcos directions.
 *  eg. 
 *      For vector (0,0,1) the origin would be the most negative z value.
 *      For vector (0,0,-1) the origin would be the most positive z value. 
 *
 *  \param orgin target array
 */
void svkMrsImageData::GetSelectionBoxOrigin(  double origin[3] )
{

    vtkUnstructuredGrid* uGrid = vtkUnstructuredGrid::New();
    this->GenerateSelectionBox( uGrid );

    double rowNormal[3]; 
    this->GetDataBasis( rowNormal, svkImageData::ROW );
    double columnNormal[3]; 
    this->GetDataBasis( columnNormal, svkImageData::COLUMN );
    double sliceNormal[3]; 
    this->GetDataBasis( sliceNormal, svkImageData::SLICE );

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

    for( int i = 0; i < selBoxPoints->GetNumberOfPoints(); i++) {
        // Calculate the distance in the three directions of the dcos
        rowDistance = vtkMath::Dot( selBoxPoints->GetPoint(i), rowNormal );
        columnDistance = vtkMath::Dot( selBoxPoints->GetPoint(i), columnNormal );
        sliceDistance = vtkMath::Dot( selBoxPoints->GetPoint(i), sliceNormal );

        // If it is the minimum then that is the origin
        if( rowDistance <= rowDistanceMin && columnDistance <= columnDistanceMin && sliceDistance <= sliceDistanceMin ) {
            originIndex = i;
        }

    }
    origin[0] = selBoxPoints->GetPoint(originIndex)[0];
    origin[1] = selBoxPoints->GetPoint(originIndex)[1];
    origin[2] = selBoxPoints->GetPoint(originIndex)[2];
    uGrid->Delete();
}


/*!
 *  Gets the closests slice for a given LPS coordinate, and a sliceOrientation.
 *
 *  \param posLPS the position in LPS coordinates
 *  \param orientation the orientation of the slice you wish to select 
 */
int svkMrsImageData::GetClosestSlice(double* posLPS, svkDcmHeader::Orientation sliceOrientation )
{
    sliceOrientation = (sliceOrientation == svkDcmHeader::UNKNOWN ) ?
                                this->GetDcmHeader()->GetOrientationType() : sliceOrientation;
    string acquisitionType = this->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
    double* origin = this->GetOrigin();
    int index = this->GetOrientationIndex( sliceOrientation );
    double* spacing = this->GetSpacing();
    if( acquisitionType == "SINGLE VOXEL" ) {
        origin = new double[3];
        spacing = new double[3];
        this->GetSelectionBoxOrigin( origin );
        this->GetSelectionBoxSpacing( spacing );
    } 
    double normal[3];
    this->GetSliceNormal( normal, sliceOrientation );
    double normalDouble[3] = { (double)normal[0], (double)normal[1], (double)normal[2] };
    double imageCenter = vtkMath::Dot( posLPS, normal );
    double idealCenter = ( imageCenter-vtkMath::Dot( origin, normalDouble) )/(spacing[index] );
    int slice = (int) floor( idealCenter );
    if( acquisitionType == "SINGLE VOXEL" ) {
        delete[] origin;
        delete[] spacing;
    } 
    return slice;
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
 *  Calculates the top left corner, and bottom right corner (high index-low index) of all
 *  voxels that lie within the selection box for a given slice. 
 *
 *  \param tlcBrc the destination for the result of the calculation
 *  \param tolerance the tolerance ranges from 0.0-1.0 and determines what fraction of the 
 *                   voxel must be within the selection box to be selected
 *  \param slice the slice you wish to select within
 */
void svkMrsImageData::GetTlcBrcInSelectionBox( int tlcBrc[2], svkDcmHeader::Orientation orientation, int slice )
{
        orientation = (orientation == svkDcmHeader::UNKNOWN ) ?
                                this->GetDcmHeader()->GetOrientationType() : orientation;
        double minPoint[3];
        double maxPoint[3];
        double selection[6];
        this->GetSelectionBoxMaxMin( minPoint, maxPoint );
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
 *  This will get the top left corner, and bottom right hand corner (low index-high index) for 
 *  a given selection and slice. It assumes the userSelection defines a minimum and maximum
 *  range and that the user wishes to select all voxels within that range (for a given slice)
 *  in a regular box aligned with the dcos having corners at the minimum and maximum values.
 *
 *  \param tlcBrc the destination for the result of the calculation
 *  \param userSelection the [minx, maxx, miny, maxy, minz, maxz] selection range 
 *  \param slice the slice within which to make the selection, if slice is outside of the range
 *               all voxels in the volume will be returned.
 */
void svkMrsImageData::GetTlcBrcInUserSelection( int tlcBrc[2], double userSelection[6], 
                                                svkDcmHeader::Orientation orientation, int slice )
{
    orientation = (orientation == svkDcmHeader::UNKNOWN ) ?
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
 *
 */
void  svkMrsImageData::GetImage( vtkImageData* image, int point, int timePoint, int channel ) 
{
    if( image != NULL ) {
        // Setup image dimensions
        image->SetExtent( Extent[0], Extent[1]-1, Extent[2], Extent[3]-1, Extent[4], Extent[5]-1);
        image->SetSpacing( Spacing[0], Spacing[1], Spacing[2] );
        image->SetScalarTypeToDouble( );

        // Create a float array to hold the pixel data
        vtkDoubleArray* pixelData = vtkDoubleArray::New();
        pixelData->SetNumberOfComponents( 2 );
        pixelData->SetNumberOfTuples( (image->GetDimensions()[0])*(image->GetDimensions()[1])*(image->GetDimensions()[2]) );
        double* pixels = pixelData->GetPointer(0);
        int linearIndex = 0;
        int* dims = image->GetDimensions();
        double* tuple;
        vtkDataArray* spectrum;
        for (int z = Extent[4]; z < Extent[5]; z++) {
            for (int y = Extent[2]; y < Extent[3]; y++) {
                for (int x = Extent[0]; x < Extent[1]; x++) {
                    spectrum = this->GetSpectrum( x, y, z, timePoint, channel );

                    linearIndex = ( z * (dims[0]) * (dims[1]) ) + ( y * (dims[0]) ) + x; 
                    tuple = spectrum->GetTuple( point );
                    pixels[2*linearIndex] = tuple[0];
                    pixels[2*linearIndex+1] = tuple[1];
                }
            }
        }
        image->GetPointData()->SetScalars( pixelData );
        image->SetNumberOfScalarComponents(2);
        pixelData->Delete();
    }
}


/*!
 *  Determins the number of slices for a given orientation.
 */
int svkMrsImageData::GetNumberOfSlices( svkDcmHeader::Orientation sliceOrientation)
{
    sliceOrientation = (sliceOrientation == svkDcmHeader::UNKNOWN ) ? 
                                this->GetDcmHeader()->GetOrientationType() : sliceOrientation;
    int numSlices;
    switch ( this->GetDcmHeader()->GetOrientationType()) {
        case svkDcmHeader::AXIAL:
            switch ( sliceOrientation ) {
                case svkDcmHeader::AXIAL:
                    numSlices = this->GetDimensions()[2]-1;
                    break;
                case svkDcmHeader::CORONAL:
                    numSlices = this->GetDimensions()[1]-1;
                    break;
                case svkDcmHeader::SAGITTAL:
                    numSlices = this->GetDimensions()[0]-1;
                    break;
            }
            break;
        case svkDcmHeader::CORONAL:
            switch ( sliceOrientation ) {
                case svkDcmHeader::AXIAL:
                    numSlices = this->GetDimensions()[1]-1;
                    break;
                case svkDcmHeader::CORONAL:
                    numSlices = this->GetDimensions()[2]-1;
                    break;
                case svkDcmHeader::SAGITTAL:
                    numSlices = this->GetDimensions()[0]-1;
                    break;
            }
            break;
        case svkDcmHeader::SAGITTAL:
            switch ( sliceOrientation ) {
                case svkDcmHeader::AXIAL:
                    numSlices = this->GetDimensions()[1]-1;
                    break;
                case svkDcmHeader::CORONAL:
                    numSlices = this->GetDimensions()[0]-1;
                    break;
                case svkDcmHeader::SAGITTAL:
                    numSlices = this->GetDimensions()[2]-1;
                    break;
            }
            break;
    }
    return numSlices;
}
/*
void  svkMrsImageData::GetAllImages( vtkDataSetCollection* imageCollection, int timePoint, int channel ) 
{
    if( imageCollection != NULL ) {
        // Setup image dimensions
        imageCollection->RemoveAllItems();
        int* dims = this->GetDimensions();

        // Create a float array to hold the pixel data
        int linearIndex = 0;
        int* dims;
        double* tuple;
        double* pixels;
        vtkDataArray* spectrum;
        vtkImageData* image;
        vtkDoubleArray* pixelData;
        int numPoints = this->GetCellData()->GetArray(0)->GetNumberOfTuples();
        for (int z = Extent[4]; z < Extent[5]; z++) {
            for (int y = Extent[2]; y < Extent[3]; y++) {
                for (int x = Extent[0]; x < Extent[1]; x++) {
                    spectrum = this->GetSpectrum( x, y, z, timePoint, channel );
                    image = vtkImageData::New();
                    image->SetExtent( Extent[0], Extent[1]-1, Extent[2], Extent[3]-1, Extent[4], Extent[5]-1);
                    image->SetSpacing( Spacing[0], Spacing[1], Spacing[2] );
                    image->SetScalarTypeToDouble( );
                    dims = image->GetDimensions();
                    pixelData = vtkDoubleArray::New();
                    pixelData->SetNumberOfComponents( 2 );
                    pixelData->SetNumberOfTuples( (dims[0])*(dims[1])*(dims[2]) );
                    pixels = pixelData->GetPointer(0);

                    for( int point = 0; point < numPoints; point++ ) {
                        linearIndex = i + j * (dims[0]-1) + k * (dims[0]-1) * (dims[1]-1);
                        tuple = spectrum->GetTuple( point );
                        pixels[2*linearIndex] = tuple[0];
                        pixels[2*linearIndex+1] = tuple[1];
                    }

                    image->GetPointData()->SetScalars( pixelData );
                    image->SetNumberOfScalarComponents(2);
                    imageCollection->AddItem( image );
                    image->Delete();
                    pixelData->Delete();
                }
            }
        }
    }
}
*/

/*!
 *   Method will set a spectral point from a vtkImageData object representing
 *   a single point in the spectra. This is usefull for spatial FFT's.
 *
 *  \param image the point image 
 *  \param point the point in the array you wish operate on 
 *  \param component the component to operate on 
 *  \param timePoint the time point to operate on 
 *  \param channel the the channel to operate on 
 *
 */
void  svkMrsImageData::SetImage( vtkImageData* image, int point, int timePoint, int channel ) 
{
    if( image != NULL ) {
        int linearIndex = 0;
        double* value;
        double range[2];
        vtkDataArray* scalars = image->GetPointData()->GetScalars();
        for (int z = Extent[4]; z < Extent[5]; z++) {
            for (int y = Extent[2]; y < Extent[3]; y++) {
                for (int x = Extent[0]; x < Extent[1]; x++) {
                    vtkDataArray* spectrum = this->GetSpectrum( x, y, z, timePoint, channel );
                    linearIndex = ( z * (Dimensions[0]-1) * (Dimensions[1]-1) ) + ( y * (Dimensions[1]-1) ) + x; 
                    value = scalars->GetTuple2( linearIndex );
                    spectrum->SetTuple2( point, value[0], value[1] );
                   
                }
            }
        }
    }
}

/*
void  svkMrsImageData::SetAllImages( vtkDataSetCollection* imageCollection, int timePoint, int channel ) 
{
    if( image != NULL ) {
        int linearIndex = 0;
        double* value;
        double range[2];
        int numPoints = this->GetCellData()->GetArray(0)->GetNumberOfTuples();
        vtkDataArray* scalars = image->GetPointData()->GetScalars();
        for (int z = Extent[4]; z < Extent[5]; z++) {
            for (int y = Extent[2]; y < Extent[3]; y++) {
                for (int x = Extent[0]; x < Extent[1]; x++) {
                    vtkDataArray* spectrum = this->GetSpectrum( x, y, z, timePoint, channel );
                    for( int point = 0; point < numPoints; point++ ) {
                        linearIndex = x + y * (dims[0]-1) + z * (dims[0]-1) * (dims[1]-1);
                        value = scalars->GetTuple2( linearIndex );
                        spectrum->SetTuple2( point, value[0], value[1] );
                    }
                   
                }
            }
        }
    }
}
*/
