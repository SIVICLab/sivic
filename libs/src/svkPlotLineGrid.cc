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


#include <svkPlotLineGrid.h>

#define DEBUG 0


using namespace svk;
using namespace std;


vtkCxxRevisionMacro(svkPlotLineGrid, "$Rev$");
vtkStandardNewMacro(svkPlotLineGrid);

//! Constructor 
svkPlotLineGrid::svkPlotLineGrid()
{
    this->voxelIndexTLC = new int[2];
    this->voxelIndexBRC = new int[2];
    this->voxelIndexTLC[0] = 0;
    this->voxelIndexTLC[1] = 0;
    this->voxelIndexBRC[0] = 0;
    this->voxelIndexBRC[1] = 0;
    this->data = NULL;
    this->channel = 0;
    this->slice = 0;
    this->renderer = NULL;
    this->selectionBoxActor = NULL;
    this->viewBounds = NULL;
    this->xyPlots = NULL;
    this->renderer = NULL;
    this->plotGridActor = NULL;
    this->freqUpToDate = NULL;
    this->ampUpToDate = NULL;


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
    this->dataModifiedCB = vtkCallbackCommand::New();
    this->dataModifiedCB->SetCallback( UpdateData );
    this->dataModifiedCB->SetClientData( (void*)this );
    this->maxPlotsX = 0;
    this->maxPlotsY = 0;
    this->plotRangeX1 = 0;
    this->plotRangeX2 = 0;
    this->plotRangeY1 = 0;
    this->plotRangeY2 = 0;
    this->plotComponent = svkPlotLine::REAL; 

}


//! Destructor
svkPlotLineGrid::~svkPlotLineGrid()
{
    
    if( selectionBoxActor != NULL ) {
        selectionBoxActor->Delete();
        selectionBoxActor = NULL;
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

    if( this->renderer != NULL ) {
        this->renderer->Delete();
        this->renderer= NULL;
    }
    
    delete[] viewBounds;
    delete[] voxelIndexTLC;
    delete[] voxelIndexBRC;
    delete[] freqUpToDate;
    delete[] ampUpToDate;
}


/*! 
 * Set input data and initialize default range values. Then it generates the actors
 * for the entire dataset upfront, to allow for fast SetSlice operations.
 *
 * \param data the data to be viewed
 */
void svkPlotLineGrid::SetInput(svkMrsImageData* data)
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
    this->renderer->AddActor( selectionBoxActor );
    selectionBoxTopology->Delete();
    int* extent = this->data->GetExtent(); 
    if( this->freqUpToDate != NULL ) {
        delete[] this->freqUpToDate;
    }
    this->freqUpToDate = new bool[ this->data->GetDcmHeader()->GetNumberOfSlices() ];
    for( int i = 0; i < this->data->GetDcmHeader()->GetNumberOfSlices(); i++ ) {
        this->freqUpToDate[i] = 0;
    }
    if( this->ampUpToDate != NULL ) {
        delete[] this->ampUpToDate;
    }
    this->ampUpToDate = new bool[ this->data->GetDcmHeader()->GetNumberOfSlices() ];
    for( int i = 0; i < this->data->GetDcmHeader()->GetNumberOfSlices(); i++ ) {
        this->ampUpToDate[i] = 0;
    }
    // maxPlots are saved to improve performance
    this->maxPlotsX = extent[1]; 
    this->maxPlotsY = extent[3]; 

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
    this->GenerateActor();
    this->SetSlice( this->slice );
    this->Update();
    this->AlignCamera();
}


//! Allocates plot and grid vectors
void svkPlotLineGrid::AllocateXYPlots()
{
    if( this->xyPlots != NULL ) {
        this->xyPlots->Delete();
        this->xyPlots = NULL;
    }
    this->xyPlots = vtkCollection::New();
    svkPlotLine* tmpXYPlot;
    int* extent = this->data->GetExtent();
    for (int zInd = extent[4] ; zInd <= extent[5]; zInd++) {
        for (int yInd = extent[2] ; yInd <= extent[3]; yInd++) {
            for (int xInd = extent[0]; xInd <= extent[1]; xInd++) {
                tmpXYPlot = svkPlotLine::New();
                this->xyPlots->AddItem( tmpXYPlot );
                tmpXYPlot->Delete();
            }
        }
    }
}


/*! 
 *  Set the slice number to plot in the data object.
 *
 *  \param slice the slice to be viewed
 */
void svkPlotLineGrid::SetSlice(int slice)
{
    this->slice = slice; 
    if( data != NULL ) { 
        int* extent = data->GetExtent();
        if( slice < extent[5] && slice >= extent[4] ) { 
            if( this->data->SliceInSelectionBox( slice )  ) {
                this->selectionBoxActor->SetVisibility(1);
                this->selectionBoxActor->SetPickable(0);
            } else {
                this->selectionBoxActor->SetVisibility(0);
            }

            this->UpdatePlotRange();
            this->AlignCamera();
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
void svkPlotLineGrid::SetPlotVoxels(int tlcID, int brcID)
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
    this->Update();
}


/*! 
 *  Sets the renderer to be used. Also repositions the camera
 *  and sets it to parallel projection mode.
 *
 *  \param renderer the renderer you wish this plotGrid to be presented in.
 *
 */
void svkPlotLineGrid::SetRenderer(vtkRenderer* renderer)
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
void svkPlotLineGrid::Update()
{
    if( data == NULL ) {
        return;
    }
    int ID;
    double* cellBounds;
    double* tmpViewBounds = new double[6];
    svkPlotLine* tmpXYPlot;

    for (int yInd = voxelIndexTLC[1]; yInd <= voxelIndexBRC[1]; yInd++) {
        for (int xInd = voxelIndexTLC[0]; xInd <= voxelIndexBRC[0]; xInd++) {
            ID = this->data->GetIDFromIndex(xInd, yInd, this->slice); 
            tmpXYPlot = static_cast<svkPlotLine*>( xyPlots->GetItemAsObject(ID) ); 

            // If the data has been update since the actors- then regenerate them
            if( data->GetMTime() > tmpXYPlot->GetMTime() ) {
                tmpXYPlot->GeneratePolyData();
            }

            if (DEBUG) {
                cout << "Make Visible: " << xInd << " " << yInd << " " << this->slice << endl;
            }

            // We use the cellbounds to reset the camera's fov        
            if (yInd == voxelIndexTLC[1] && xInd == voxelIndexTLC[0]) {
                cellBounds = tmpXYPlot->plotAreaBounds;   
                tmpViewBounds[0] = cellBounds[0];
                tmpViewBounds[2] = cellBounds[2];
            } 

            if (yInd == voxelIndexBRC[1] && xInd == voxelIndexBRC[0]) {
                cellBounds = tmpXYPlot->plotAreaBounds;   
                tmpViewBounds[1] = cellBounds[1];
                tmpViewBounds[3] = cellBounds[3];
                tmpViewBounds[4] = cellBounds[4];
                tmpViewBounds[5] = cellBounds[5];
            } 
        }
    }

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
void svkPlotLineGrid::GenerateActor()
{
    int* voxelIndex = new int[3];
    int ID;
    double* cellBounds;
    svkPlotLine* tmpXYPlot;
    this->RemoveActors();
    this->renderer->AddActor( selectionBoxActor );
    string acquisitionType = data->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");


    cout << "Warning(svkPlotLineGrid::GenerateActor):  Stabilize conventions for data ordering CellID ordering and x,y sign convention, tlc, brc, etc. (TODO) " << endl;

    //  Note that 2D selection range is defined from tlc to brc of rubber band with y 
    //  increasing in negative direction.
    this->AllocateXYPlots();
    double* origin = this->data->GetOrigin();
    double* spacing = this->data->GetSpacing();
    int arrayLength = this->data->GetCellData()->GetArray(0)->GetNumberOfTuples();
    int* extent = this->data->GetExtent();
    double dcos[3][3];
    this->data->GetDcos( dcos );
    vtkPolyData* boxPlot = vtkPolyData::New();
    boxPlot->Allocate(1,1);
    vtkPoints* points = vtkPoints::New();
    points->SetNumberOfPoints((extent[5]-extent[4])*(extent[3]-extent[2])*(extent[1]-extent[0])*arrayLength );
    boxPlot->SetPoints( points );
    for (int zInd = extent[4]; zInd < extent[5]; zInd++) {
        for (int yInd = extent[2]; yInd < extent[3]; yInd++) {
            for (int xInd = extent[0]; xInd < extent[1]; xInd++) {

                //  =====================================  
                //  Load data arrays into plots: 
                //  =====================================  
                voxelIndex[0] = xInd;
                voxelIndex[1] = yInd;
                voxelIndex[2] = zInd;

                ID = this->data->GetIDFromIndex(xInd, yInd, zInd); 
                tmpXYPlot = static_cast<svkPlotLine*>( xyPlots->GetItemAsObject(ID) ); 
                tmpXYPlot->GetPointIds()->SetNumberOfIds(arrayLength);
                tmpXYPlot->SetDataPoints( points );
                for( int i = 0; i < arrayLength; i++ ) {
                    tmpXYPlot->GetPointIds()->SetId(i,i+ID*arrayLength );
                }
                boxPlot->InsertNextCell(tmpXYPlot->GetCellType(), tmpXYPlot->GetPointIds());
                boxPlot->Update();

                if (DEBUG) {
                    cout << "CREATE: " << xInd << " " << yInd << " " << zInd;
                    cout << " Local ID: " << ID << " Full ID " << (ID + 0*maxPlotsX*maxPlotsY);
                    cout << " True ID " << this->data->ComputeCellId( voxelIndex ) << endl;
                }
                tmpXYPlot->SetDcos( dcos );
                tmpXYPlot->SetSpacing( spacing );
                tmpXYPlot->SetData( 
                        vtkFloatArray::SafeDownCast(
                            this->data->GetSpectrum(xInd, yInd, zInd , 0, channel )) );

                tmpXYPlot->GeneratePolyData();
                vtkGenericCell* myCell = vtkGenericCell::New();
                this->data->GetCell( this->data->ComputeCellId(voxelIndex), myCell );   
                cellBounds = myCell->GetBounds();
                //double plotAreaBounds[6];
                double plotOrigin[3];
                myCell->Delete();
                plotOrigin[0] = origin[0] + xInd * spacing[0] * dcos[0][0]
                                          + yInd * spacing[1] * dcos[1][0]
                                          + zInd * spacing[2] * dcos[2][0];
                plotOrigin[1] = origin[1] + xInd * spacing[0] * dcos[0][1]
                                          + yInd * spacing[1] * dcos[1][1]
                                          + zInd * spacing[2] * dcos[2][1];
                plotOrigin[2] = origin[2] + xInd * spacing[0] * dcos[0][2]
                                          + yInd * spacing[1] * dcos[1][2]
                                          + zInd * spacing[2] * dcos[2][2];

                tmpXYPlot->SetOrigin( plotOrigin );   

                if( acquisitionType == "SINGLE VOXEL" ) {
                    //tmpXYPlot->SetPlotAreaBounds( this->selectionBoxActor->GetBounds() );   
                    cout << "Single voxel NOT SUPPORTED!!!!" << endl;
                } 
                tmpXYPlot->SetPointRange(plotRangeX1, plotRangeX2);
                tmpXYPlot->SetValueRange(plotRangeY1, plotRangeY2);


            }
        }
    }
    vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
    mapper->SetInput( boxPlot );
                boxPlot->Delete();

    if( this->plotGridActor != NULL) {
        this->renderer->RemoveViewProp( plotGridActor );
        this->plotGridActor->Delete(); 
    }
    this->plotGridActor = vtkActor::New();

    plotGridActor->SetMapper( mapper );
    mapper->Delete();

    renderer->AddActor( plotGridActor );


    // Now lets generate the selection box actor

    delete[] voxelIndex;

}




//!  Remove Actors for updating view: 
void svkPlotLineGrid::RemoveActors()
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
void svkPlotLineGrid::SetFrequencyWLRange(int lower, int upper)
{
    this->plotRangeX1 = lower; 
    this->plotRangeX2 = upper; 
    if( this->data != NULL ) {
        for( int i = 0; i < this->data->GetDcmHeader()->GetNumberOfSlices(); i++ ) {
            this->freqUpToDate[i] = 0;
        }
    }
    this->UpdatePlotRange();
}


/*!
 *  Sets the intesity range to be between lower and upper.
 *
 *  \param lower the minimum amplitude
 *  \param upper the maximum amplitude
 *
 */
void svkPlotLineGrid::SetIntensityWLRange(double lower, double upper)
{
    this->plotRangeY1 = lower;
    this->plotRangeY2 = upper;  
    if( this->data != NULL ) {
        for( int i = 0; i < this->data->GetDcmHeader()->GetNumberOfSlices(); i++ ) {
            this->ampUpToDate[i] = 0;
        }
    }
    this->UpdatePlotRange();
}


/*!
 *  Sets the X,Y range for all plots in current slice.
 */
void svkPlotLineGrid::UpdatePlotRange()
{
    svkPlotLine* tmpXYPlot;
    int ID;
    int* extent = data->GetExtent();
    if( data != NULL && (!this->ampUpToDate[slice] || (!this->freqUpToDate[slice]) )) { 
        if( slice < extent[5] && slice >= extent[4] ) { 
            for (int yInd = 0; yInd <= maxPlotsY; yInd++) {
                for (int xInd = 0; xInd <= maxPlotsX; xInd++) {
                    ID = this->data->GetIDFromIndex(xInd, yInd, this->slice);
                    tmpXYPlot = static_cast<svkPlotLine*>( xyPlots->GetItemAsObject(ID) ); 
                    if( !this->freqUpToDate[slice]) {
                        tmpXYPlot->SetPointRange(this->plotRangeX1, this->plotRangeX2);
                    }
                    if( !this->ampUpToDate[slice] ) {
                        tmpXYPlot->SetValueRange(this->plotRangeY1, this->plotRangeY2);
                    }
                }
            }
        }
    }
    if( !this->ampUpToDate[slice]) {
        this->ampUpToDate[slice] = 1;
    }
    if( !this->freqUpToDate[slice]) {
        this->freqUpToDate[slice] = 1;
    }
    this->Update();
}


//! Regenerates the plot data, used when data is modified.
void svkPlotLineGrid::RegeneratePlots()
{
    for (int yInd = 0; yInd < maxPlotsY; yInd++) {
        for (int xInd = 0; xInd < maxPlotsX; xInd++) {
            static_cast<svkPlotLine*>( xyPlots->GetItemAsObject(
                        this->data->GetIDFromIndex(xInd, yInd, this->slice)))->GeneratePolyData(); 
        }
    }

}


//! Resets the camera to look at the new selection
void svkPlotLineGrid::AlignCamera( bool invertView ) 
{  
    if( this->renderer != NULL && viewBounds != NULL ) {
        double zoom;
        double viewWidth = viewBounds[1] - viewBounds[0];
        double viewHeight = viewBounds[3] - viewBounds[2];
        viewBounds[4] = this->renderer->ComputeVisiblePropBounds()[4];
        viewBounds[5] = this->renderer->ComputeVisiblePropBounds()[5];
        double viewDepth = viewBounds[5] - viewBounds[4];
        double diagonal = sqrt( pow(viewWidth,2) + pow(viewHeight,2) + pow(viewDepth,2) );
        int toggleDraw = this->renderer->GetDraw();
        if( toggleDraw ) {
            this->renderer->DrawOff();
        }
        this->renderer->ResetCamera( viewBounds );
        double dcos[3][3];
        data->GetDcos( dcos );
        double wVec[3];
        wVec[0] = dcos[0][2];
        wVec[1] = dcos[1][2];
        wVec[2] = dcos[2][2];
        // if the data set is not axial, move the camera
        double* focalPoint = this->renderer->GetActiveCamera()->GetFocalPoint();
        double* cameraPosition = this->renderer->GetActiveCamera()->GetPosition();
        double distance = sqrt( pow( focalPoint[0] - cameraPosition[0], 2 ) +
                pow( focalPoint[1] - cameraPosition[1], 2 ) +
                pow( focalPoint[2] - cameraPosition[2], 2 ) );
        double newCameraPosition[3];

        if( pow( wVec[2], 2) < pow( wVec[1], 2 ) || pow( wVec[2], 2) < pow( wVec[0], 2 ) ) {
            newCameraPosition[0] = focalPoint[0] - distance*wVec[0]; 
            newCameraPosition[1] = focalPoint[1] - distance*wVec[1];
            newCameraPosition[2] = focalPoint[2] - distance*wVec[2];
        } else {
            newCameraPosition[0] = focalPoint[0] + distance*wVec[0]; 
            newCameraPosition[1] = focalPoint[1] + distance*wVec[1];
            newCameraPosition[2] = focalPoint[2] + distance*wVec[2];

        }
        this->renderer->GetActiveCamera()->SetPosition( newCameraPosition );

        // Default for axial, above if block makes it work for all
        //this->renderer->GetActiveCamera()->SetViewUp( 0, -1, 0 );
        this->renderer->ResetCamera( viewBounds );

        if( viewWidth >= viewHeight ) {
            zoom = diagonal/viewWidth;        
        } else {
            zoom = diagonal/viewHeight;        
        }
        // We'll back off the zoom to 90% to leave some edges
        this->renderer->GetActiveCamera()->Zoom(0.95*zoom);
        if( toggleDraw ) {
            this->renderer->DrawOn();
        }
    }
}


//! Callback tied to data modified events. Regenerates plot data when the vtkImageData is modified.
void svkPlotLineGrid::UpdateData(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata)
{
    static_cast<svkPlotLineGrid*>(thisObject)->RegeneratePlots();
}


/*!
 *  Selects all voxels completely within the selection box.
 *  
 */
void svkPlotLineGrid::HighlightSelectionVoxels()
{
    if( data != NULL ) {
        double* selectionBounds = this->selectionBoxActor->GetBounds();
        double* actorBounds;
        int newTlcBrc[2];
        int ID;
        int* extent = data->GetExtent();
        newTlcBrc[0] = -1 ;
        newTlcBrc[1] = -1 ;
        vtkCollectionIterator* myIterator = vtkCollectionIterator::New();
        svkPlotLine* currentPlot;
        myIterator->SetCollection( xyPlots );
        myIterator->InitTraversal();
        while( !myIterator->IsDoneWithTraversal() ) {
            currentPlot= svkPlotLine::SafeDownCast( myIterator->GetCurrentObject() );
            // Get location of the data referenced by the actor
            ID = xyPlots->IsItemPresent( currentPlot ) - 1 + slice*extent[1]*extent[3];
            actorBounds = currentPlot->GetBounds();
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
            this->Update();
            AlignCamera();
        } 
    } 
}


/*!
 *  Sets the component to be displayed. 
 * 
 *  \param component the component you wish to view, REAL, IMAGINARY, or MAGNITUDE
 */
void svkPlotLineGrid::SetComponent( svkPlotLine::PlotComponent component )
{
    this->plotComponent = component; 
    this->UpdateComponent();
}


/*!
 *  Gets the current component.
 *
 *  \return component the current component: REAL, IMAGINARY, or MAGNITUDE
 */
int svkPlotLineGrid::GetComponent( )
{
    return this->plotComponent;
}


/*!
 *  Checks to see if the component has change, if it has it updates.
 */
void svkPlotLineGrid::UpdateComponent()
{

    vtkCollectionIterator* iterator = vtkCollectionIterator::New();
    svkPlotLine* tmpBoxPlot;
    iterator->SetCollection(xyPlots);
    iterator->InitTraversal();
    while(!iterator->IsDoneWithTraversal()) {
        tmpBoxPlot = static_cast<svkPlotLine*>( iterator->GetCurrentObject()); 
        tmpBoxPlot->SetComponent( this->plotComponent );
        tmpBoxPlot->GeneratePolyData();
        iterator->GoToNextItem();
    }

    if( DEBUG ){
        cout << " Reset component " << plotComponent << endl;
    }
    iterator->Delete();
    this->Update();
}


/*!
 *
 */
void svkPlotLineGrid::SetChannel( int channel )
{
    this->channel = channel;
    int tlcRange[2] = {0,0};
    int brcRange[2] = {maxPlotsX-1, maxPlotsY-1};
    this->UpdateDataArrays( tlcRange, brcRange);
}


/*!
 *
 */
void svkPlotLineGrid::UpdateDataArrays( int* tlcRange, int* brcRange)
{
    svkPlotLine* tmpXYPlot;
    int ID;
    if( data != NULL ) { 
        int* extent = data->GetExtent();
        double* spacing = data->GetSpacing();
        double* origin = data->GetOrigin();
        //double plotAreaBounds[6];
        if( slice < extent[5] && slice >= extent[4] ) { 
            for (int yInd = tlcRange[1]; yInd <= brcRange[1]; yInd++) {
                for (int xInd = tlcRange[0]; xInd <= brcRange[0]; xInd++) {
                    ID = this->data->GetIDFromIndex(xInd, yInd, this->slice) - 
                        (this->slice * this->maxPlotsX * this->maxPlotsY );
                    tmpXYPlot = static_cast<svkPlotLine*>( xyPlots->GetItemAsObject(ID) ); 
                    tmpXYPlot->SetData( vtkFloatArray::SafeDownCast(this->data->GetSpectrum(xInd, yInd, this->slice, 0, this->channel)));
                }
            }
            tmpXYPlot->polyLinePoints->Modified();
        }
    }
}
