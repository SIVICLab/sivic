/*
 *  Copyright © 2009 The Regents of the University of California.
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

}


/*!
 *
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
 *
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
 *
 */
void svkMrsImageData::GetSelectionBoxCenter( float* selBoxCenter )
{
    vtkUnstructuredGrid* selBox = vtkUnstructuredGrid::New(); 
    this->GenerateSelectionBox( selBox );
    vtkPoints* selBoxPoints = selBox->GetPoints();
    int numPoints = selBoxPoints->GetNumberOfPoints();
    for( int i = 0; i < 3; i++ ) {
        selBoxCenter[i] = 0;
        for( int j = 0; j < numPoints; j++ ) {
            selBoxCenter[i] += selBoxPoints->GetPoint(j)[i];
        }
        selBoxCenter[i] /= numPoints;
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
    char arrayName[30];
    sprintf(arrayName, "%d %d %d %d %d", i, j, k, timePoint, channel);
    return this->GetCellData()->GetArray( arrayName );
}


/*! 
 *
 */     
void svkMrsImageData::UpdateRange()
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
    int numFrequencyPoints = this->GetCellData()->GetNumberOfTuples();
     for (int z = extent[4]; z <= extent[5]-1; z++) {
        for (int y = extent[2]; y <= extent[3]-1; y++) {
            for (int x = extent[0]; x <= extent[1]-1; x++) {
                for( int channel = 0; channel < numChannels; channel++ ) {
                    vtkFloatArray* spectrum = static_cast<vtkFloatArray*>( this->GetSpectrum( x, y, z, 0, channel ) );
                    for (int i = 0; i < numFrequencyPoints; i++) {
                        double* tuple = spectrum->GetTuple( i );
                        realRange[0] = tuple[0] < realRange[0]
                                                 ? tuple[0] : realRange[0];
                        realRange[1] = tuple[0] > realRange[1] 
                                                 ? tuple[0] : realRange[1];

                        imagRange[0] = tuple[1] < imagRange[0]
                                                 ? tuple[1] : imagRange[0];
                        imagRange[1] = tuple[1] > imagRange[1]
                                                 ? tuple[1] : imagRange[1];

                        double magnitude = pow( pow(tuple[0],2)
                                                + pow(tuple[1],2),0.5);

                        magRange[0] = magnitude < magRange[0]
                                                 ? magnitude : magRange[0];
                        magRange[1] = magnitude > magRange[1]
                                                 ? magnitude : magRange[1];
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
 *  Method determines of the current slice is within the selection box.
 *
 *  \return true if the slice is within the selection box, other wise false is returned
 */
bool svkMrsImageData::SliceInSelectionBox( int slice )
{
    int voxelIndex[3];
    voxelIndex[0] = 0;
    voxelIndex[1] = 0;
    voxelIndex[2] = slice;
    double dcos[3][3];
    this->GetDcos( dcos );
    double wVec[3];
    wVec[0] = dcos[2][0];
    wVec[1] = dcos[2][1];
    wVec[2] = dcos[2][2];

    vtkGenericCell* sliceCell = vtkGenericCell::New();
    this->GetCell( this->ComputeCellId(voxelIndex), sliceCell );
    vtkUnstructuredGrid* uGrid = vtkUnstructuredGrid::New();
    this->GenerateSelectionBox( uGrid );
    vtkPoints* selBoxPoints = uGrid->GetPoints();
    double projectedSelBoxRange[2]; 
    projectedSelBoxRange[0] = VTK_DOUBLE_MAX;
    projectedSelBoxRange[1] = -VTK_DOUBLE_MAX;
    double projectedDistance;
    for( int i = 0; i < selBoxPoints->GetNumberOfPoints(); i++) {
        projectedDistance = vtkMath::Dot( selBoxPoints->GetPoint(i), wVec ); 
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
        projectedDistance = vtkMath::Dot( sliceCell->GetPoints()->GetPoint(i), wVec ); 
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
    bool inSlice = 0;
    if( ( projectedSelBoxRange[0] > projectedSliceRange[0] && projectedSelBoxRange[0] < sliceCenter 
 )   || ( projectedSelBoxRange[1] > sliceCenter && projectedSelBoxRange[1] < projectedSliceRange[1]
 )   || ( sliceCenter > projectedSelBoxRange[0] && sliceCenter < projectedSelBoxRange[1]
 ) ){
        inSlice = 1;
    }
    uGrid->Delete();
    return inSlice;
    
}
