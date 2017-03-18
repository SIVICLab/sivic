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



#include <svkImageTopologyGenerator.h>
#include <svkMrsImageData.h>

using namespace svk;


//vtkCxxRevisionMacro(svkImageTopologyGenerator, "$Rev$");


/*!
 * Constructor.
 */
svkImageTopologyGenerator::svkImageTopologyGenerator()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif
    
}


/*!
 * Deconstructor.
 */
svkImageTopologyGenerator::~svkImageTopologyGenerator()
{

    vtkDebugMacro(<<"svkImageTopologyGenerator::~svkImageTopologyGenerator");

}


/*!
 *  makeGridVoxelActor generates Actors that represent the spectroscopic
 *  voxels. It uses a vtkCubeSource object to generate polydata based on the bounds
 *  given to it.
 *
 *  \param 
 *   bounds        a pointer to a length 6 double array representing the bounds of the voxel. 
 *                 (xmin, xmax, ymin, ymax, zmin, zmax)
 *
 *  \return 
 *   vtkActor*     a green wireframe actor to represent the grid voxel
 */
vtkActor* svkImageTopologyGenerator::MakeGridVoxelActor( double* bounds )
{

    // create set of vtkPoints that represent a single cell (voxel).
    vtkCubeSource* gridVoxel = vtkCubeSource::New();
    gridVoxel->SetBounds( bounds );

    // Create Mapper
    vtkPolyDataMapper* gridVoxelMapper = vtkPolyDataMapper::New();
    gridVoxelMapper->SetInputConnection( gridVoxel->GetOutputPort() );

    // Setup Visual Properties
    vtkProperty* visualProperties = vtkProperty::New();
    visualProperties->SetDiffuseColor( 0.0, 1.0, 0.0 );
    visualProperties->SetRepresentationToWireframe();

    // Build Actor
    vtkActor* gridVoxelActor = vtkActor::New();
    gridVoxelActor->SetMapper( gridVoxelMapper );
    gridVoxelActor->SetProperty( visualProperties );
    
    gridVoxel->Delete();
    gridVoxelMapper->Delete();
    visualProperties->Delete();
    return gridVoxelActor;
}


/*!
 *  makeGridVoxelActor generates Actors that represent the spectroscopic
 *  voxels. It uses a vtkCubeSource object to generate polydata based on the bounds
 *  given to it.
 *
 *  \param 
 *   bounds        a pointer to a length 6 double array representing the bounds of the voxel. 
 *                 (xmin, xmax, ymin, ymax, zmin, zmax)
 *
 *  \return 
 *   vtkActor*     a green wireframe actor to represent the grid voxel
 */
vtkActor* svkImageTopologyGenerator::MakeRectGridVoxelActor( double* bounds )
{
    vtkFloatArray *xCoords = vtkFloatArray::New();
    vtkFloatArray *yCoords = vtkFloatArray::New();
    vtkFloatArray *zCoords = vtkFloatArray::New();
    xCoords->InsertNextValue(bounds[0]);
    xCoords->InsertNextValue(bounds[1]);
    yCoords->InsertNextValue(bounds[2]);
    yCoords->InsertNextValue(bounds[3]);
    zCoords->InsertNextValue(bounds[4]);
    zCoords->InsertNextValue(bounds[5]);

    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(2,2,2);
    rgrid->SetXCoordinates(xCoords);
    rgrid->SetYCoordinates(yCoords);
    rgrid->SetZCoordinates(zCoords);

    vtkDataSetMapper *rgridMapper = vtkDataSetMapper::New();
    rgridMapper->SetInputData(rgrid);
    rgridMapper->ImmediateModeRenderingOn();
    rgridMapper->StaticOn();

    vtkActor *wireActor = vtkActor::New();
    wireActor->SetMapper(rgridMapper);
    wireActor->GetProperty()->SetRepresentationToWireframe();
    wireActor->GetProperty()->SetColor(0,1,0);

    xCoords->Delete();
    yCoords->Delete();
    zCoords->Delete();
    rgrid->Delete();
    rgridMapper->Delete();

    return wireActor;
}
/*! 
 *  Generate the voxel grid. The grid consists of vtkActors that are constructed based on
 *  the output of a vtkCubeSource object (can be any reqular hexahedron) which is polyData. 
 *  The resulting actor collection contains one actor per cell in the data, and each actor
 *  will have the same boundries as the cell it represents. The actors are set into the collection
 *  in the same order as the vtkID's of the cells in the data set. This means that the id of a cell
 *  in the data set is the same as its index in the collection. This fact is important, and is assumed
 *  elsewhere (svkPlotGridView, svkOverlayView).  
 *
 *  \param
 *   data 
 *        is a pointer to the svkImageData object whoms cell structure you want the grid to 
 *        represent.
 *
 *  \return 
 *   vtkActorCollection* 
 *        is the collection of all actors representing the cell structure of
 *        the data set
 */
vtkActorCollection* svkImageTopologyGenerator::GenerateVoxelGrid( svkImageData* data ) 
{
    vtkActorCollection* allActors = NULL; 
    if( data != NULL ) {
        vtkImageData* gridData = data;
        if( gridData != NULL ) {
            allActors = vtkActorCollection::New();
            vtkActor* currentActor;
            vtkIdType currentID;
            int dim[3];
            int index[3];
            double bounds[6];
            int counter;
            gridData->GetDimensions( dim );

            // One must be subtracted from each dimension to get the correct cell
            // dimensionality.
            dim[0] -= 1;
            dim[1] -= 1;
            dim[2] -= 1;
            for( int k = 0; k < dim[2]; k++) {
                for( int j = 0; j < dim[1]; j++) {
                    for( int i = 0; i < dim[0]; i++) {
                        index[0] = i;
                        index[1] = j;
                        index[2] = k;
                        currentID = gridData->ComputeCellId(index);
                        gridData->GetCellBounds( currentID, bounds );
                        currentActor = MakeGridVoxelActor(bounds);
                        currentActor->SetVisibility(1);
                        allActors->AddItem(currentActor);
                        currentActor->Delete();
                    }
                }
            }
        }
    } 
    return allActors;
}


/*!
 *
 */
void svkImageTopologyGenerator::GenerateVoxelGridActor( svkImageData* data, vtkActor* targetActor ) 
{
    if( targetActor != NULL ) {
        vtkPolyDataMapper* entireGridMapper = vtkPolyDataMapper::New();
        entireGridMapper->ScalarVisibilityOff( );
        entireGridMapper->InterpolateScalarsBeforeMappingOff();
        entireGridMapper->ReleaseDataFlagOn();
        entireGridMapper->ImmediateModeRenderingOn();

        // We need a filter to pull out the edges of the data cells (voxels)
        vtkExtractEdges* edgeExtractor = vtkExtractEdges::New();
        double dcos[3][3];

        // Here we are making a copy of the image for the grid.
        // For some reason valgrind reports massive read errors
        // if the data arrays are present when passed to
        // vtkExtractEdges.
// Can this be Mri or Mrs ??  Shoudl I get the type from svkImageData and make a copy of that type?
        svkImageData* geometryData;// = svkImageData::New();
        //if ( data->IsA("svkMriImageData") )  {
            //svkImageData* geometryData = svkMriImageData::New();
        //} else if ( data->IsA("svkMrsImageData") )  {
            //svkImageData* geometryData = svkMrsImageData::New();
        //}
        
        geometryData->SetOrigin( data->GetOrigin() );
        geometryData->SetSpacing( data->GetSpacing() );
        geometryData->SetExtent( data->GetExtent() );
        data->GetDcos(dcos);
        geometryData->SetDcos(dcos);
        edgeExtractor->SetInputData( geometryData );
        geometryData->Delete();

        // Pipe the edges into the mapper
        entireGridMapper->SetInputData( edgeExtractor->GetOutput() );
        edgeExtractor->Delete();
        targetActor->SetMapper( entireGridMapper );
        entireGridMapper->Delete();
        targetActor->GetProperty()->SetDiffuseColor( 0, 1, 0 );
    } else {
        cout << "svkImageTopologyGenerator::GenerateVoxelGridActor targetActor must not be null!! " << endl;
    }

}

vtkPolyData* svkImageTopologyGenerator::GenerateVoxelGridPolyData( svkImageData* data) 
{
    // We need a filter to pull out the edges of the data cells (voxels)
    vtkExtractEdges* edgeExtractor = vtkExtractEdges::New();
    edgeExtractor->SetInputData( data );
    
    vtkPolyData* pdata = edgeExtractor->GetOutput();
cout <<"TOPO5" << endl;
    pdata->Register(NULL);
cout <<"TOPO6" << endl;
    return pdata;
}


/*!
 *  GenerateSelectionBox creates a vtkActorCollection with one element in it, a vtkActor 
 *  that represents the selection box. This is done by going through the header and pulling
 *  out the slabs that represent the selection box. The slabs are split up into two planes
 *  each, and placed into a vtkPlanesIntersection object. This object is designed to define
 *  any convex region that can be described by a set of boundry planes. Once this region is
 *  described the vertecies are extracted, and we use them to create a polyhedron. Right now
 *  only a hexaderon is possible, and hence three slabs must be present in the header otherwise
 *  NULL is returned.
 *
 *
 *  \param
 *   data 
 *        is a pointer to the svkImageData object whoms selection box you want to represent.
 *
 *  \return 
 *   vtkActorCollection* 
 *        is a collection of one actor, the hexahdron representing the selection box, if
 *        the header does not define 3 slabs, NULL is returned.
 */
vtkActorCollection* svkImageTopologyGenerator::GenerateSelectionBox ( svkImageData* data ) 
{
    vtkActorCollection* topology = NULL;
    vtkUnstructuredGrid* selectionBoxGrid = vtkUnstructuredGrid::New(); 
    svkMrsImageData::SafeDownCast( data )->GenerateSelectionBox( selectionBoxGrid );
    vtkPoints* selectionBoxPoints = selectionBoxGrid->GetPoints();
    // Case for no selection box
    if( selectionBoxPoints == NULL ) {
        return NULL;
    }
    vtkCell* selectionBox = selectionBoxGrid->GetCell(0);
    
    vtkPolyData* polyData = vtkPolyData::New();
    polyData->Allocate(12, 12);
    polyData->SetPoints(selectionBoxPoints);

    for( int i = 0; i < selectionBox->GetNumberOfEdges(); i++) {
        vtkCell* edge = selectionBox->GetEdge(i);
        polyData->InsertNextCell(edge->GetCellType(), edge->GetPointIds());
    }

        
    // We will need a mapper to draw the data.
    vtkPolyDataMapper* selectionBoxMapper = vtkPolyDataMapper::New();
    selectionBoxMapper->SetInputData(polyData);
    
    /*
    // In case we want to return to using the UnstructuredGrid...
    vtkDataSetMapper* selectionBoxMapper = vtkDataSetMapper::New();
    selectionBoxMapper->SetInput(selectionBoxGrid);
    */

    // Now we will create the actual actor
    vtkActor* selectionBoxActor = vtkActor::New();    
    selectionBoxActor->SetMapper(selectionBoxMapper);
    selectionBoxActor->GetProperty()->SetAmbientColor( 1.0, 1.0, 0.0 );
    //selectionBoxActor->GetProperty()->SetRepresentationToWireframe();
    selectionBoxActor->GetProperty()->SetEdgeColor(1,0,0);
    selectionBoxActor->GetProperty()->SetAmbient(1.0);
    selectionBoxActor->GetProperty()->SetLineWidth(3.0);
    selectionBoxActor->GetProperty()->SetDiffuse(0.0);
    selectionBoxActor->GetProperty()->SetSpecular(0.0);
    selectionBoxActor->SetPickable(0); 

    // And push the actor into the collection
    topology = vtkActorCollection::New();
    topology->AddItem( selectionBoxActor ); 

    selectionBoxMapper->Delete();
    selectionBoxGrid->Delete();
    polyData->Delete();
    selectionBoxActor->Delete();    
    return topology;
}

