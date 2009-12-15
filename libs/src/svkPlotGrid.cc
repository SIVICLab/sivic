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


#include <svkPlotGrid.h>

#define DEBUG 0


using namespace svk;
using namespace std;


vtkCxxRevisionMacro(svkPlotGrid, "$Rev$");
vtkStandardNewMacro(svkPlotGrid);

//! Constructor 
svkPlotGrid::svkPlotGrid()
{
    this->voxelIndexTLC = new int[2];
    this->voxelIndexBRC = new int[2];
    this->data = NULL;
    this->channel = 0;
    this->slice = 0;
    this->renderer = NULL;
    this->selectionBoxActor = NULL;
    this->projectedSelBoxActor = NULL;
    this->viewBounds = NULL;
    this->xyPlots = NULL;
    this->renderer = NULL;


    /*
     * This booleans are used for improving perforamce. They keep track
     * of whether modifications have been made on the current slice, so that
     * when the slice eventually does change the new slice can be updated
     * accordingly.
     *
     * To sum up the "Changed" variables keep track of whether or not
     * things need to be changed at the next Update, and "ChangedThisSlice"
     * keeps track of whether or not things need to be update at SetSlice.
     */
    this->freqRangeChanged = 1;
    this->ampRangeChanged = 1;
    this->componentChanged = 1;
    this->freqRangeChangeThisSlice = 0;
    this->ampRangeChangeThisSlice = 0;
    this->componentChangeThisSlice = 0;
    this->phaseChangeThisSlice = 0;
    this->dataModifiedCB = vtkCallbackCommand::New();
    this->dataModifiedCB->SetCallback( UpdateData );
    this->dataModifiedCB->SetClientData( (void*)this );
    this->cellIdTlcBrc = new int[2];
    this->maxPlotsX = 0;
    this->maxPlotsY = 0;
    this->plotRangeX1 = 0;
    this->plotRangeX2 = 0;
    this->plotRangeY1 = 0;
    this->plotRangeY2 = 0;
    this->plotComponent = svkBoxPlot::REAL; 

}


//! Destructor
svkPlotGrid::~svkPlotGrid()
{
    
    if( selectionBoxActor != NULL ) {
        selectionBoxActor->Delete();
        selectionBoxActor = NULL;
    }

    if( projectedSelBoxActor != NULL ) {
        projectedSelBoxActor->Delete();
        projectedSelBoxActor = NULL;
    }

    if( data != NULL ) {
        data->Delete();
        data = NULL;
    }
    
    if( dataModifiedCB != NULL ) {
        dataModifiedCB->Delete();
        dataModifiedCB = NULL;
    }
    if( xyPlots != NULL ) {
        xyPlots->Delete();
        xyPlots = NULL;
    }

    if( renderer != NULL ) {
        renderer->Delete();
        renderer= NULL;
    }
    
    delete[] viewBounds;
    delete[] voxelIndexTLC;
    delete[] voxelIndexBRC;
    delete[] cellIdTlcBrc; 
}


/*! 
 * Set input data and initialize default range values. Then it generates the actors
 * for the entire dataset upfront, to allow for fast SetSlice operations.
 *
 * \param data the data to be viewed
 */
void svkPlotGrid::SetInput(svkMrsImageData* data)
{

    if( this->data != NULL ) {
        this->data->Delete();
        this->data = NULL;
    }
    this->data = data;
    this->data->Register(this);
    this->data->AddObserver(vtkCommand::ModifiedEvent, dataModifiedCB);

    if( this->selectionBoxActor != NULL ) {
        this->selectionBoxActor->Delete();
        this->selectionBoxActor = NULL;
    }
    vtkActorCollection* selectionBoxTopology = data->GetTopoActorCollection(svkMrsImageData::VOL_SELECTION);
    selectionBoxTopology->InitTraversal();
    this->selectionBoxActor = selectionBoxTopology->GetNextActor();
    this->selectionBoxActor->Register(this);
    selectionBoxTopology->Delete();
    int* extent = this->data->GetExtent(); 

    // maxPlots are saved to improve performance
    this->maxPlotsX = extent[1]; 
    this->maxPlotsY = extent[3]; 
    //  Set default voxels to plot to entire slice:
    this->SetPlotVoxels(0, this->maxPlotsX * this->maxPlotsY - 1);


    //  Set default plot range to full scale:
    int numFrequencyPoints = this->data->GetCellData()->GetNumberOfTuples();

    //  Get these from data or our subclass of vtkImageData tbd:
    this->plotRangeX1 = 0;
    this->plotRangeX2 = numFrequencyPoints;
    this->plotRangeY1 = -60000000;
    this->plotRangeY2 = 350000000;
    if( this->xyPlots != NULL ) {
        this->xyPlots->Delete();
        this->xyPlots = NULL;
    }
    this->GenerateActors();
    this->SetSlice( this->slice );
    this->Update();
}


//! Allocates plot and grid vectors
void svkPlotGrid::AllocateXYPlots()
{
    if( this->xyPlots != NULL ) {
        this->xyPlots->Delete();
        this->xyPlots = NULL;
    }
    this->xyPlots = vtkActorCollection::New();
    svkBoxPlot* tmpXYPlot;
    for (int yInd = 0; yInd < maxPlotsY; yInd++) {
        for (int xInd = 0; xInd < maxPlotsX; xInd++) {
            tmpXYPlot = svkBoxPlot::New();
            this->xyPlots->AddItem( tmpXYPlot );
            tmpXYPlot->Delete();
        }
    }
}


/*! 
 *  Set the slice number to plot in the data object.
 *
 *  \param slice the slice to be viewed
 */
void svkPlotGrid::SetSlice(int slice)
{
    
    this->slice = slice; 
    if( data != NULL ) { 
        int* extent = data->GetExtent();
        if( slice < extent[5] && slice >= extent[4] ) { 
            this->UpdateDataArrays( this->voxelIndexTLC, this->voxelIndexBRC);
            if( this->data->SliceInSelectionBox( slice )  ) {
                this->projectedSelBoxActor->SetVisibility(1);
                this->projectedSelBoxActor->SetPickable(0);
            } else {
                this->projectedSelBoxActor->SetVisibility(0);
            }
            ////////////////////////////////////////////////////
           
            if( this->freqRangeChangeThisSlice == 1 ) { 
                this->freqRangeChanged = 1;
            }
            if( this->ampRangeChangeThisSlice == 1 ) { 
                this->ampRangeChanged = 1;
            }
            if( this->componentChangeThisSlice == 1 ) { 
                this->componentChanged = 1;
            }
            if( this->phaseChangeThisSlice == 1 ) { 
                this->phaseChanged = 1;
            }
           
            this->UpdatePlotRange();
            this->UpdateComponent();
            this->freqRangeChangeThisSlice = 0;
            this->ampRangeChangeThisSlice = 0;
            this->componentChangeThisSlice = 0;
        }
    }
}


/*! 
 *  Set the plot indices corresponding to the tlc and brc 
 *  voxels to plot in the current slice (i.e. 2D space). 
 *  This together with the slice index defines the 
 *  corresponding data in data. 
 *
 *  \param tlcID the id of the top left corner 
 *
 *  \param brcID the id of the bottom right corner 
 */
void svkPlotGrid::SetPlotVoxels(int tlcID, int brcID)
{
    
    int tlcX, tlcY, tlcZ;
    int brcX, brcY, brcZ;
    int gridSize = maxPlotsX* maxPlotsY;   

    if( data == NULL ) {
        return;
        // Check to see if the voxels asked for are within slice...
    } else if( tlcID < slice*gridSize     || brcID < slice*gridSize 
            || tlcID > (slice+1)*gridSize || brcID > (slice+1)*gridSize ) {
    
        this->voxelIndexTLC[0] = 0; 
        this->voxelIndexTLC[1] = 0;
        this->voxelIndexBRC[0] = this->maxPlotsX - 1;
        this->voxelIndexBRC[1] = this->maxPlotsY - 1;
    } else {
        this->data->GetIndexFromID(tlcID, &tlcX, &tlcY, &tlcZ);
        this->data->GetIndexFromID(brcID, &brcX, &brcY, &brcZ);
        this->voxelIndexTLC[0] = tlcX; 
        this->voxelIndexTLC[1] = tlcY; 
        this->voxelIndexBRC[0] = brcX;
        this->voxelIndexBRC[1] = brcY;
        if (DEBUG) {
            cout << " Set Plot Voxels ID: " << tlcX << "," << tlcY << "," <<  brcX <<"," << brcY << endl;
        }
    }
}


/*! 
 *  Sets the renderer to be used. Also repositions the camera
 *  and sets it to parallel projection mode.
 *
 *  \param renderer the renderer you wish this plotGrid to be presented in.
 *
 */
void svkPlotGrid::SetRenderer(vtkRenderer* renderer)
{
    if( this->renderer != NULL ) {
        this->renderer->Delete();
    }
    this->renderer = renderer; 
    this->renderer->Register( this );
    renderer->GetActiveCamera()->Azimuth(180);
    renderer->GetActiveCamera()->Roll(180);
    this->renderer->GetActiveCamera()->ParallelProjectionOn();
    this->renderer->RemoveAllLights();
}


//! Update the view based on voxelIndexTLC/BRC
void svkPlotGrid::Update()
{
    if( data == NULL ) {
        return;
    }
    int ID;
    double* cellBounds;
    double* tmpViewBounds = new double[6];
    svkBoxPlot* tmpXYPlot;
    
    for (int yInd = 0; yInd < maxPlotsY; yInd++) {
        for (int xInd = 0; xInd < maxPlotsX; xInd++) {
            ID = this->data->GetIDFromIndex(xInd, yInd, this->slice) - 
                    (this->slice * this->maxPlotsX * this->maxPlotsY );
            tmpXYPlot = static_cast<svkBoxPlot*>( xyPlots->GetItemAsObject(ID) ); 
            if( yInd >= voxelIndexTLC[1] && yInd <= voxelIndexBRC[1] &&
                xInd >= voxelIndexTLC[0] && xInd <= voxelIndexBRC[0]) {
            
                // If the data has been update since the actors- then regenerate them
                if( data->GetMTime() > tmpXYPlot->GetMTime() || this->phaseChanged || 
                                                                  this->componentChanged ) {
                    tmpXYPlot->GeneratePolyData();
                }
            
                if (DEBUG) {
                    cout << "Make Visible: " << xInd << " " << yInd << " " << this->slice << endl;
                }
    
                // We use the cellbounds to reset the camera's fov        
                if (yInd == voxelIndexTLC[1] && xInd == voxelIndexTLC[0]) {
                    cellBounds = tmpXYPlot->GetBounds();   
                    tmpViewBounds[0] = cellBounds[0];
                    tmpViewBounds[2] = cellBounds[2];
                } 

                if (yInd == voxelIndexBRC[1] && xInd == voxelIndexBRC[0]) {
                    cellBounds = tmpXYPlot->GetBounds();   
                    tmpViewBounds[1] = cellBounds[1];
                    tmpViewBounds[3] = cellBounds[3];
                    tmpViewBounds[4] = cellBounds[4];
                    tmpViewBounds[5] = cellBounds[5];
                } 
                tmpXYPlot->GetProperty()->SetOpacity(1);
            } else {
                // Can't set the Visibility to zero or the actor becomes unpickable
                tmpXYPlot->GetProperty()->SetOpacity(0.0001);
            
            }
        }
    }
    this->phaseChanged = 0;
    this->componentChanged = 0;
    
    // Reset the fov to include the selection box
    if( selectionBoxActor->GetVisibility() == 1 ) {
        double* selectionBounds = selectionBoxActor->GetBounds();
        if( tmpViewBounds[4] > selectionBounds[4] ) {
            tmpViewBounds[4] = selectionBounds[4];
        }
        if( tmpViewBounds[5] < selectionBounds[5] ) {
            tmpViewBounds[5] = selectionBounds[5];
        }
    }
    // verify camera positioning
    if( viewBounds == NULL ) {
        viewBounds = tmpViewBounds;
        AlignCamera();
    } else if(  viewBounds[0] != tmpViewBounds[0] ||
                viewBounds[1] != tmpViewBounds[1] ||
                viewBounds[2] != tmpViewBounds[2] ||
                viewBounds[3] != tmpViewBounds[3] ) { 
        delete[] viewBounds;
        viewBounds = tmpViewBounds; 
        AlignCamera();
    } else {
        delete[] tmpViewBounds;
    }
}


//! Generate all of the actors to be used 
void svkPlotGrid::GenerateActors()
{

    int* voxelIndex = new int[3];
    int ID;
    double* cellBounds;
    svkBoxPlot* tmpXYPlot;
    this->RemoveActors();
    string acquisitionType = data->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");


    cout << "Warning(svkPlotGrid::GenerateActors):  Stabilize conventions for data ordering CellID ordering and x,y sign convention, tlc, brc, etc. (TODO) " << endl;

    //  Note that 2D selection range is defined from tlc to brc of rubber band with y 
    //  increasing in negative direction.
    AllocateXYPlots();
    this->GenerateSelectionBox();
    double* origin = this->data->GetOrigin();
    double* spacing = this->data->GetSpacing();
    for (int yInd = this->voxelIndexTLC[1]; yInd <= this->voxelIndexBRC[1]; yInd++) {
        for (int xInd = this->voxelIndexTLC[0]; xInd <= this->voxelIndexBRC[0]; xInd++) {
             
            //  =====================================  
            //  Load data arrays into plots: 
            //  =====================================  
            voxelIndex[0] = xInd;
            voxelIndex[1] = yInd;
            voxelIndex[2] = 0;
            
            ID = this->data->GetIDFromIndex(xInd, yInd, this->slice) - 
                    (this->slice * this->maxPlotsX * this->maxPlotsY );
            tmpXYPlot = static_cast<svkBoxPlot*>( xyPlots->GetItemAsObject(ID) ); 

            if (DEBUG) {
                cout << "CREATE: " << xInd << " " << yInd << " " << 0;
                cout << " Local ID: " << ID << " Full ID " << (ID + 0*maxPlotsX*maxPlotsY);
                cout << " True ID " << this->data->ComputeCellId( voxelIndex ) << endl;
            }

            tmpXYPlot->SetData( 
                vtkFloatArray::SafeDownCast(
                    this->data->GetSpectrum(xInd, yInd, 0, 0, channel )) );

            tmpXYPlot->GeneratePolyData();
            vtkGenericCell* myCell = vtkGenericCell::New();
            this->data->GetCell( this->data->ComputeCellId(voxelIndex), myCell );   
            cellBounds = myCell->GetBounds();
            double plotAreaBounds[6];
            myCell->Delete();
            plotAreaBounds[0] = xInd*spacing[0]; 
            plotAreaBounds[1] = (xInd+1)*spacing[0]; 
            plotAreaBounds[2] = yInd*spacing[1]; 
            plotAreaBounds[3] = (yInd+1)*spacing[1]; 
            plotAreaBounds[4] = 0; 
            plotAreaBounds[5] = 0; 

            if( acquisitionType == "SINGLE VOXEL" ) {
                tmpXYPlot->SetPlotAreaBounds( this->projectedSelBoxActor->GetBounds() );   
            } else {
                tmpXYPlot->SetPlotAreaBounds( plotAreaBounds );   
            }
            tmpXYPlot->SetPointRange(plotRangeX1, plotRangeX2);
            tmpXYPlot->SetValueRange(plotRangeY1, plotRangeY2);
                
            this->renderer->AddActor(tmpXYPlot);
            
        }
    }

    // Now lets generate the selection box actor

    delete[] voxelIndex;

}

/*!
 *  Method generates the selection box actor. Since the coordinate system is
 *  in the frame of the data the dcos can be ignored. The selection box generated
 *  by the svkImageData does use the dcos so we have to convert it back to the
 *  reference frame of the data.
 */
void svkPlotGrid::GenerateSelectionBox() 
{
    double* origin = this->data->GetOrigin();
    double* spacing = this->data->GetSpacing();
    if( this->projectedSelBoxActor != NULL ) {  
        this->renderer->RemoveActor( projectedSelBoxActor );
        this->projectedSelBoxActor->Delete();
        this->projectedSelBoxActor = NULL;
    }
    this->projectedSelBoxActor = vtkActor::New();
    vtkPoints* selBoxPoints = vtkPointSet::SafeDownCast(this->selectionBoxActor->GetMapper()->GetInput())->GetPoints();
    double dcos[3][3];
    this->data->GetDcos( dcos );
    double uVec[3];
    uVec[0] = dcos[0][0];
    uVec[1] = dcos[0][1];
    uVec[2] = dcos[0][2];
    double vVec[3];
    vVec[0] = dcos[1][0];
    vVec[1] = dcos[1][1];
    vVec[2] = dcos[1][2];
    double selBoxBounds[6]; 
    double projectedPosition[2];
    selBoxBounds[0] = VTK_DOUBLE_MAX;    
    selBoxBounds[1] = -VTK_DOUBLE_MAX;    
    selBoxBounds[2] = VTK_DOUBLE_MAX;    
    selBoxBounds[3] = -VTK_DOUBLE_MAX;    
    selBoxBounds[4] = 0;
    selBoxBounds[5] = 0;
    // Lets find the maxs and mins in the u and v dimensions
    for( int i = 0; i < selBoxPoints->GetNumberOfPoints(); i++ ) {
        projectedPosition[0] = vtkMath::Dot( selBoxPoints->GetPoint(i), uVec ) - vtkMath::Dot( origin, uVec) ;    
        projectedPosition[1] = vtkMath::Dot( selBoxPoints->GetPoint(i), vVec ) - vtkMath::Dot( origin, vVec) ;    
        if( selBoxBounds[0] > projectedPosition[0] ) {
            selBoxBounds[0] = projectedPosition[0];
        }
        if( selBoxBounds[1] < projectedPosition[0] ) {
            selBoxBounds[1] = projectedPosition[0];
        }
        if( selBoxBounds[2] > projectedPosition[1] ) {
            selBoxBounds[2] = projectedPosition[1];
        }
        if( selBoxBounds[3] < projectedPosition[1] ) {
            selBoxBounds[3] = projectedPosition[1];
        }
    }
    vtkCubeSource* projectedSelBox = vtkCubeSource::New();
    projectedSelBox->SetBounds( selBoxBounds );
    vtkExtractEdges* edgeExtractor = vtkExtractEdges::New();
    edgeExtractor->SetInput( projectedSelBox->GetOutput() );
    edgeExtractor->Update();
    vtkPolyDataMapper* projectedSelBoxMapper = vtkPolyDataMapper::New(); 
    projectedSelBoxMapper->SetInput( edgeExtractor->GetOutput() );
    edgeExtractor->Delete();
    this->projectedSelBoxActor->SetMapper( projectedSelBoxMapper );
    this->projectedSelBoxActor->GetProperty()->SetDiffuseColor(1,1,0);
    this->projectedSelBoxActor->GetProperty()->SetEdgeColor(1,1,0);
    this->projectedSelBoxActor->GetProperty()->SetLineWidth(2);
    this->projectedSelBoxActor->GetProperty()->EdgeVisibilityOn();
    this->renderer->AddActor( projectedSelBoxActor );
    projectedSelBox->Delete();
    projectedSelBoxMapper->Delete();

}


//!  Remove Actors for updating view: 
void svkPlotGrid::RemoveActors()
{
    renderer->RemoveAllViewProps();
    if( xyPlots != NULL ) {
        xyPlots->Delete();   
        xyPlots = NULL;
    }
}


/*!
 *  Sets the frequency range to be between lower and upper.
 *
 *  \param lower the minimum frequency
 *  \param upper the maximum frequency
 *
 *  TODO: Currently this method only works with points, it
 *        should be modified for Hz, and PPM.
 */
void svkPlotGrid::SetFrequencyWLRange(int lower, int upper)
{
    this->plotRangeX1 = lower; 
    this->plotRangeX2 = upper; 
    freqRangeChanged = 1;
    freqRangeChangeThisSlice = 1;
    this->UpdatePlotRange();
}


/*!
 *  Sets the intesity range to be between lower and upper.
 *
 *  \param lower the minimum amplitude
 *  \param upper the maximum amplitude
 *
 */
void svkPlotGrid::SetIntensityWLRange(double lower, double upper)
{
    this->plotRangeY1 = lower;
    this->plotRangeY2 = upper;  
    ampRangeChanged = 1;
    ampRangeChangeThisSlice = 1;
    this->UpdatePlotRange();
}


/*!
 *  Sets the X,Y range for all plots in current slice.
 */
void svkPlotGrid::UpdatePlotRange()
{
    if( DEBUG ) {
        cout<<"Plot Range Being Updated"<<endl;   
    }
    if( ampRangeChanged || freqRangeChanged ) { 
        vtkCollectionIterator* iterator = vtkCollectionIterator::New();
        svkBoxPlot* tmpBoxPlot;
        iterator->SetCollection(xyPlots);
        iterator->InitTraversal();
      
        while(!iterator->IsDoneWithTraversal()) {
            tmpBoxPlot = static_cast<svkBoxPlot*>( iterator->GetCurrentObject()); 
            if( freqRangeChanged ) {
                tmpBoxPlot->SetPointRange(this->plotRangeX1, this->plotRangeX2);
            }
            if( ampRangeChanged ) {
                tmpBoxPlot->SetValueRange(this->plotRangeY1, this->plotRangeY2);
            }
            iterator->GoToNextItem();
        }
        if( DEBUG ){
            cout << " Reset Plot range " << this->plotRangeX2 << endl;
        }
        iterator->Delete();
        ampRangeChanged = 0; 
        freqRangeChanged = 0; 
        Update();
    }
}


/*! 
 *  Get the ID's of the top left corner and bottom right corner of the view.
 *
 *  \return a integer point to an array of two integers, the id of tlc, then brc
 */
int* svkPlotGrid::GetCurrentTlcBrc()
{
    if( data != NULL ) { 
        this->cellIdTlcBrc[0] = this->data->GetIDFromIndex(voxelIndexTLC[0], voxelIndexTLC[1], this->slice); 
        this->cellIdTlcBrc[1] = this->data->GetIDFromIndex(voxelIndexBRC[0], voxelIndexBRC[1], this->slice);
    } else {
        this->cellIdTlcBrc[0] = -1; 
        this->cellIdTlcBrc[1] = -1;
    }
    return this->cellIdTlcBrc;
}


/*!
 *   Selects a set of actors based on the a 2D bounding box in window coordinates. 
 *  
 *   \param selectionArea the area in which objects should be highlighted
 */
void svkPlotGrid::SetSelection( int* selectionArea )
{
    cellIdTlcBrc[0] = -1;
    cellIdTlcBrc[1] = -1;
    int ID;
    int location;
    svkAreaPicker* dragBoxPicker = svkAreaPicker::New();
    dragBoxPicker->AreaPick( selectionArea[0], selectionArea[1], selectionArea[2], selectionArea[3], renderer );
    vtkProp3DCollection* allProps = dragBoxPicker->GetProp3Ds();
    if( allProps->GetNumberOfItems()>0){
        vtkCollectionIterator* myIterator = vtkCollectionIterator::New();
        myIterator->SetCollection( allProps );
        myIterator->InitTraversal();

        while( !myIterator->IsDoneWithTraversal() ) {
            ID = xyPlots->IsItemPresent( myIterator->GetCurrentObject() ) - 1; 
            if( ID >= 0 ) {
                if( cellIdTlcBrc[0] == -1 || ID < cellIdTlcBrc[0] ) {
                    cellIdTlcBrc[0] = ID;
                } 
                if( cellIdTlcBrc[1] == -1 || ID > cellIdTlcBrc[1] ) {
                    cellIdTlcBrc[1] = ID;
                } 
            } 
            myIterator->GoToNextItem();
        
        }
        
        cellIdTlcBrc[0]+= (data->GetExtent())[1] * (data->GetExtent())[3]*slice;
        cellIdTlcBrc[1]+= (data->GetExtent())[1] * (data->GetExtent())[3]*slice;
       
        myIterator->Delete();
        SetPlotVoxels(cellIdTlcBrc[0], cellIdTlcBrc[1]);
        Update();
        AlignCamera();
         
    }

    dragBoxPicker->Delete();
}


//! Regenerates the plot data, used when data is modified.
void svkPlotGrid::RegeneratePlots()
{
    int* voxelIndex = new int[3];
    double* cellBounds;
    voxelIndex[2] = slice;
    for (int yInd = 0; yInd < maxPlotsY; yInd++) {
        for (int xInd = 0; xInd < maxPlotsX; xInd++) {
            static_cast<svkBoxPlot*>( xyPlots->GetItemAsObject(
                this->data->GetIDFromIndex(xInd, yInd, this->slice) - 
                    (this->slice * this->maxPlotsX * this->maxPlotsY )
            ) )->GeneratePolyData(); 
        }
    }
    delete[] voxelIndex;
    
}


//! Resets the camera to look at the new selection
void svkPlotGrid::AlignCamera( bool invertView ) 
{
    if( renderer != NULL && viewBounds != NULL ) {
        double zoom;
        double viewWidth = viewBounds[1] - viewBounds[0];
        double viewHeight = viewBounds[3] - viewBounds[2];
        viewBounds[4] = renderer->ComputeVisiblePropBounds()[4];
        viewBounds[5] = renderer->ComputeVisiblePropBounds()[5];
        double viewDepth = viewBounds[5] - viewBounds[4];
        double diagonal = sqrt( pow(viewWidth,2) + pow(viewHeight,2) + pow(viewDepth,2) );
        renderer->DrawOff();
        renderer->ResetCamera( viewBounds );
        double dcos[3][3];
        data->GetDcos( dcos );
        double wVec[3];
        wVec[0] = dcos[0][2];
        wVec[1] = dcos[1][2];
        wVec[2] = dcos[2][2];
        // if the data set is not axial, move the camera
        if( pow( wVec[2], 2) < pow( wVec[1], 2 ) || pow( wVec[2], 2) < pow( wVec[0], 2 ) ) {
            double* focalPoint = renderer->GetActiveCamera()->GetFocalPoint();
            double* cameraPosition = renderer->GetActiveCamera()->GetPosition();
            double distance = sqrt( pow( focalPoint[0] - cameraPosition[0], 2 ) +
                                    pow( focalPoint[1] - cameraPosition[1], 2 ) +
                                    pow( focalPoint[2] - cameraPosition[2], 2 ) );
            double newCameraPosition[3];
            newCameraPosition[0] = focalPoint[0]; 
            newCameraPosition[1] = focalPoint[1];
            newCameraPosition[2] = focalPoint[2] - distance;
            this->renderer->GetActiveCamera()->SetPosition( newCameraPosition );
        } 

        // Default for axial, above if block makes it work for all
        this->renderer->GetActiveCamera()->SetViewUp( 0, -1, 0 );
        renderer->ResetCamera( viewBounds );

        if( viewWidth >= viewHeight ) {
            zoom = diagonal/viewWidth;        
        } else {
            zoom = diagonal/viewHeight;        
        }
        // We'll back off the zoom to 90% to leave some edges
        renderer->GetActiveCamera()->Zoom(0.95*zoom);
        renderer->DrawOn();
    }
}


//! Callback tied to data modified events. Regenerates plot data when the vtkImageData is modified.
void svkPlotGrid::UpdateData(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata)
{
    static_cast<svkPlotGrid*>(thisObject)->RegeneratePlots();
}


/*!
 *  Selects all voxels completely within the selection box.
 *  
 */
void svkPlotGrid::HighlightSelectionVoxels()
{
    if( data != NULL ) {
        double* selectionBounds = projectedSelBoxActor->GetBounds();
        double* actorBounds;
        int newTlcBrc[2];
        int ID;
        int* extent = data->GetExtent();
        newTlcBrc[0] = -1 ;
        newTlcBrc[1] = -1 ;
        vtkCollectionIterator* myIterator = vtkCollectionIterator::New();
        vtkActor* currentActor;
        myIterator->SetCollection( xyPlots );
        myIterator->InitTraversal();
        while( !myIterator->IsDoneWithTraversal() ) {
            currentActor= vtkActor::SafeDownCast( myIterator->GetCurrentObject() );
            // Get location of the data referenced by the actor
            ID = xyPlots->IsItemPresent( currentActor ) - 1 + slice*extent[1]*extent[3];
            actorBounds = currentActor->GetBounds();
            if( (newTlcBrc[0] == -1 || ID < newTlcBrc[0] )   &&
                actorBounds[0] >= selectionBounds[0] &&
                actorBounds[1] <= selectionBounds[1] &&
                actorBounds[2] >= selectionBounds[2] &&
                actorBounds[3] <= selectionBounds[3] ) {
                newTlcBrc[0] = ID;
            }
            if( (newTlcBrc[1] == -1 || ID > newTlcBrc[1])   &&
                actorBounds[0] >= selectionBounds[0] &&
                actorBounds[1] <= selectionBounds[1] &&
                actorBounds[2] >= selectionBounds[2] &&
                actorBounds[3] <= selectionBounds[3] ) {
                newTlcBrc[1] = ID;
            }
            myIterator->GoToNextItem();
        }
        myIterator->Delete();

        if( newTlcBrc[0] >= 0 && newTlcBrc[0] >= 0 ) {
            SetPlotVoxels(newTlcBrc[0], newTlcBrc[1]);
            Update();
            AlignCamera();
        } 
    } 
}


/*!
 *  Sets the component to be displayed. 
 * 
 *  \param component the component you wish to view, REAL, IMAGINARY, or MAGNITUDE
 */
void svkPlotGrid::SetComponent( svkBoxPlot::PlotComponent component )
{
    this->plotComponent = component; 
    this->componentChanged = 1;
    this->componentChangeThisSlice = 1;
    this->UpdateComponent();
}


/*!
 *  Gets the current component.
 *
 *  \return component the current component: REAL, IMAGINARY, or MAGNITUDE
 */
int svkPlotGrid::GetComponent( )
{
    return this->plotComponent;
}


/*!
 *  Checks to see if the component has change, if it has it updates.
 */
void svkPlotGrid::UpdateComponent()
{

    if( this->componentChanged ) { 
        vtkCollectionIterator* iterator = vtkCollectionIterator::New();
        svkBoxPlot* tmpBoxPlot;
        iterator->SetCollection(xyPlots);
        iterator->InitTraversal();
        int count = 0; 
        while(!iterator->IsDoneWithTraversal()) {
            tmpBoxPlot = static_cast<svkBoxPlot*>( iterator->GetCurrentObject()); 
            tmpBoxPlot->SetComponent( this->plotComponent );
            tmpBoxPlot->GeneratePolyData();
            iterator->GoToNextItem();
            count++;
        }
    
        cout << "Number of updated boxPlots: " << count << endl;
        if( DEBUG ){
            cout << " Reset component " << plotComponent << endl;
        }
        iterator->Delete();
        this->Update();
        this->componentChanged = 0; 
    }
}


/*!
 *
 */
void svkPlotGrid::SetChannel( int channel )
{
    this->channel = channel;
    int tlcRange[2] = {0,0};
    int brcRange[2] = {maxPlotsX-1, maxPlotsY-1};
    this->UpdateDataArrays( tlcRange, brcRange);
}


/*!
 *
 */
void svkPlotGrid::UpdateDataArrays( int* tlcRange, int* brcRange)
{
    svkBoxPlot* tmpXYPlot;
    int ID;
    if( data != NULL ) { 
        int* extent = data->GetExtent();
        double* spacing = data->GetSpacing();
        double plotAreaBounds[6];
        string acquisitionType = data->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
        if( slice < extent[5] && slice >= extent[4] ) { 
            for (int yInd = tlcRange[1]; yInd <= brcRange[1]; yInd++) {
                for (int xInd = tlcRange[0]; xInd <= brcRange[0]; xInd++) {
                    ID = this->data->GetIDFromIndex(xInd, yInd, this->slice) - 
                        (this->slice * this->maxPlotsX * this->maxPlotsY );
                    tmpXYPlot = static_cast<svkBoxPlot*>( xyPlots->GetItemAsObject(ID) ); 
                    plotAreaBounds[0] = xInd*spacing[0]; 
                    plotAreaBounds[1] = (xInd+1)*spacing[0]; 
                    plotAreaBounds[2] = yInd*spacing[1]; 
                    plotAreaBounds[3] = (yInd+1)*spacing[1]; 
                    plotAreaBounds[4] = (slice-0.5)*spacing[2]; 
                    plotAreaBounds[5] = (slice-0.5)*spacing[2]; 
                    tmpXYPlot->SetData( vtkFloatArray::SafeDownCast(this->data->GetSpectrum(xInd, yInd, this->slice, 0, this->channel)));
                    if( acquisitionType != "SINGLE VOXEL" ) {
                        tmpXYPlot->SetPlotAreaBounds( plotAreaBounds );   
                    }
                }
            }
        }
    }
}
