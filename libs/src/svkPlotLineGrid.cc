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


#include <svkPlotLineGrid.h>

#define DEBUG 0


using namespace svk;
using namespace std;


vtkCxxRevisionMacro(svkPlotLineGrid, "$Rev$");
vtkStandardNewMacro(svkPlotLineGrid);

//! Constructor 
svkPlotLineGrid::svkPlotLineGrid()
{
    this->data = NULL;
    this->channel = 0;
    this->timePoint = 0;
    this->slice = 0;
    this->renderer = NULL;
    this->selectionBoxActor = NULL;
    this->viewBounds = NULL;
    this->xyPlots = NULL;
    this->renderer = NULL;
    this->plotGridActor = NULL;
    this->appender = NULL;
    this->freqUpToDate = NULL;
    this->ampUpToDate = NULL;
    this->orientation = svkDcmHeader::AXIAL;


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
    this->plotRangeX1 = 0;
    this->plotRangeX2 = 0;
    this->plotRangeY1 = 0;
    this->plotRangeY2 = 0;
    this->plotComponent = svkPlotLine::REAL; 
    this->tlcBrc[0] = 0;
    this->tlcBrc[1] = 0;
    this->freqSelectionUpToDate[0] = -1;
    this->freqSelectionUpToDate[1] = -1;
    this->ampSelectionUpToDate[0] = -1;
    this->ampSelectionUpToDate[1] = -1;
    this->polyDataCollection = vtkPolyDataCollection::New();

}


//! Destructor
svkPlotLineGrid::~svkPlotLineGrid()
{
    
    if( selectionBoxActor != NULL ) {
        selectionBoxActor->Delete();
        selectionBoxActor = NULL;
    }

    if( this->plotGridActor != NULL ) {
        this->plotGridActor->Delete();
        this->plotGridActor = NULL;
    }

    if( this->appender != NULL ) {
        this->appender->Delete();
        this->appender = NULL;
    }

    if( data != NULL ) {
        data->Delete();
        data = NULL;
    }
    
    if( dataModifiedCB != NULL ) {
        dataModifiedCB->Delete();
        dataModifiedCB = NULL;
    }
    if( this->xyPlots != NULL ) {
        this->xyPlots->Delete();
        this->xyPlots = NULL;
    }
    if( this->polyDataCollection != NULL ) {
        this->polyDataCollection->Delete();
        this->polyDataCollection = NULL;
    }

    if( this->renderer != NULL ) {
        this->renderer->Delete();
        this->renderer= NULL;
    }

    delete[] viewBounds;
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
    double range[2]; 
    // Getting the range modifies the object, so we get it before attaching callback
    this->data->GetDataRange( range, 0 );
    this->data->Register(this);
    this->data->AddObserver(vtkCommand::ModifiedEvent, dataModifiedCB);

    if( this->selectionBoxActor != NULL ) {
        if( this->renderer->HasViewProp( selectionBoxActor ) ) {
            this->renderer->RemoveViewProp( selectionBoxActor );
        }
        this->selectionBoxActor->Delete();
        this->selectionBoxActor = NULL;
    }
    vtkActorCollection* selectionBoxTopology = data->GetTopoActorCollection(svkMrsImageData::VOL_SELECTION);
    // Case for no selection Box
    if( selectionBoxTopology != NULL ) {
        selectionBoxTopology->InitTraversal();
        this->selectionBoxActor = selectionBoxTopology->GetNextActor();
        this->selectionBoxActor->Register(this);
        this->renderer->AddActor( selectionBoxActor );
        selectionBoxTopology->Delete();
    }
    int* extent = this->data->GetExtent(); 
    if( this->freqUpToDate != NULL ) {
        delete[] this->freqUpToDate;
    }
    this->freqSelectionUpToDate[0] = -1;
    this->freqSelectionUpToDate[1] = -1;
    this->freqUpToDate = new bool[ this->data->GetNumberOfSlices( this->orientation ) ];
    for( int i = 0; i < this->data->GetNumberOfSlices( this->orientation ); i++ ) {
        this->freqUpToDate[i] = 0;
    }
    if( this->ampUpToDate != NULL ) {
        delete[] this->ampUpToDate;
    }

    this->ampSelectionUpToDate[0] = -1;
    this->ampSelectionUpToDate[1] = -1;
    this->ampUpToDate = new bool[ this->data->GetNumberOfSlices( this->orientation ) ];
    for( int i = 0; i < this->data->GetNumberOfSlices( this->orientation ); i++ ) {
        this->ampUpToDate[i] = 0;
    }

    //  Set default plot range to full scale:
    int numFrequencyPoints = this->data->GetCellData()->GetNumberOfTuples();

    //  Get these from data or our subclass of vtkImageData tbd:
    this->plotRangeX1 = 0;
    this->plotRangeX2 = numFrequencyPoints;
    this->plotRangeY1 = range[0];
    this->plotRangeY2 = range[1];
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
    for (int zInd = extent[4] ; zInd < extent[5]; zInd++) {
        for (int yInd = extent[2] ; yInd < extent[3]; yInd++) {
            for (int xInd = extent[0]; xInd < extent[1]; xInd++) {
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
        int tlcIndex[3];
        int brcIndex[3];
        this->data->GetIndexFromID( tlcBrc[0], tlcIndex );
        this->data->GetIndexFromID( tlcBrc[1], brcIndex );
        int lastSlice  = this->data->GetLastSlice( this->orientation );
        int firstSlice = this->data->GetFirstSlice( this->orientation );
        slice = (slice > lastSlice) ? lastSlice:slice;
        slice = (slice < firstSlice) ? firstSlice:slice;
        tlcIndex[ this->data->GetOrientationIndex( this->orientation ) ] = slice;
        brcIndex[ this->data->GetOrientationIndex( this->orientation ) ] = slice;
        this->tlcBrc[0] = this->data->GetIDFromIndex( tlcIndex[0], tlcIndex[1], tlcIndex[2] );
        this->tlcBrc[1] = this->data->GetIDFromIndex( brcIndex[0], brcIndex[1], brcIndex[2] );
        this->UpdatePlotRange();
        this->Update();
        this->SetSliceAppender();

        // Case for no selection box
        if( this->selectionBoxActor != NULL ) {
            if( this->data->SliceInSelectionBox( this->slice, this->orientation ) ) {
                this->selectionBoxActor->SetVisibility(1);
            } else {
                this->selectionBoxActor->SetVisibility(0);
            }
        }

    }
}


void svkPlotLineGrid::SetSliceAppender() 
{

    int minIndex[3] = {0,0,0};
    int maxIndex[3] = {this->data->GetDimensions()[0]-2,this->data->GetDimensions()[1]-2,this->data->GetDimensions()[2]-2};
    int orientationIndex = this->data->GetOrientationIndex( this->orientation );

    minIndex[ orientationIndex ] = this->slice;
    maxIndex[ orientationIndex ] = this->slice;
    int minID = this->data->GetIDFromIndex( minIndex[0], minIndex[1], minIndex[2] );
    int maxID = this->data->GetIDFromIndex( maxIndex[0], maxIndex[1], maxIndex[2] );
    int rowRange[2] = {0,0};
    int columnRange[2] = {0,0};
    int sliceRange[2] = {0,0};
    int ID;
    this->data->GetIndexFromID( minID, &rowRange[0], &columnRange[0], &sliceRange[0] );
    this->data->GetIndexFromID( maxID, &rowRange[1], &columnRange[1], &sliceRange[1] );
    this->appender->RemoveAllInputs();
    for (int sliceIndex = sliceRange[0]; sliceIndex <= sliceRange[1]; sliceIndex++) {
        for (int rowIndex = rowRange[0]; rowIndex <= rowRange[1]; rowIndex++) {
            for (int columnIndex = columnRange[0]; columnIndex <= columnRange[1]; columnIndex++) {
                ID = this->data->GetIDFromIndex(rowIndex, columnIndex, sliceIndex );
                this->appender->AddInput(vtkPolyData::SafeDownCast(polyDataCollection->GetItemAsObject( ID )));
            }
        }
    }

}


/*! 
 *  Get the slice number to plot in the data object.
 *
 */
int svkPlotLineGrid::GetSlice()
{
    return this->slice;
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
void svkPlotLineGrid::SetTlcBrc(int tlcBrc[2])
{
    if( this->data != NULL ) { 
        int minIndex[3] = {0,0,0};
        int maxIndex[3] = {this->data->GetDimensions()[0]-2,this->data->GetDimensions()[1]-2,this->data->GetDimensions()[2]-2};
        int orientationIndex = this->data->GetOrientationIndex( this->orientation );

        minIndex[ orientationIndex ] = this->slice;
        maxIndex[ orientationIndex ] = this->slice;
        int minID = this->data->GetIDFromIndex( minIndex[0], minIndex[1], minIndex[2] );
        int maxID = this->data->GetIDFromIndex( maxIndex[0], maxIndex[1], maxIndex[2] );

        if( tlcBrc[0] <= tlcBrc[1] ) {
            if( tlcBrc[0] >= minID && tlcBrc[0] <= maxID ) {
                minID = tlcBrc[0];
            }
            if( tlcBrc[1] >= minID && tlcBrc[1] <= maxID ) {
                maxID = tlcBrc[1];
            }
        }

        this->tlcBrc[0] = minID;
        this->tlcBrc[1] = maxID;
        this->UpdatePlotRange();
        this->Update();
    } 

}


/*!
 *
 */
int* svkPlotLineGrid::GetTlcBrc() 
{
    return this->tlcBrc;
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
    tmpViewBounds[0] = VTK_DOUBLE_MAX;
    tmpViewBounds[1] = -VTK_DOUBLE_MAX;
    tmpViewBounds[2] = VTK_DOUBLE_MAX;
    tmpViewBounds[3] = -VTK_DOUBLE_MAX;
    tmpViewBounds[4] = VTK_DOUBLE_MAX;
    tmpViewBounds[5] = -VTK_DOUBLE_MAX;
    svkPlotLine* tmpXYPlot;
    string acquisitionType = data->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");

    int rowRange[2] = {0,0};
    int columnRange[2] = {0,0};
    int sliceRange[2] = {0,0};
    this->data->GetIndexFromID( tlcBrc[0], &rowRange[0], &columnRange[0], &sliceRange[0] );
    this->data->GetIndexFromID( tlcBrc[1], &rowRange[1], &columnRange[1], &sliceRange[1] );
    for (int rowIndex = rowRange[0]; rowIndex <= rowRange[1]; rowIndex++) {
        for (int columnIndex = columnRange[0]; columnIndex <= columnRange[1]; columnIndex++) {
            for (int sliceIndex = sliceRange[0]; sliceIndex <= sliceRange[1]; sliceIndex++) {
            
                ID = this->data->GetIDFromIndex( rowIndex, columnIndex, sliceIndex );
                tmpXYPlot = static_cast<svkPlotLine*>( xyPlots->GetItemAsObject(ID) ); 

                // If the data has been update since the actors- then regenerate them
                if( data->GetMTime() > tmpXYPlot->GetMTime() ) {
                    tmpXYPlot->GeneratePolyData();
                }

                if (DEBUG) {
                    cout << "Make Visible: " << ID << endl;
                }

                // We use the cellbounds to reset the camera's fov        
                if( tmpXYPlot->plotAreaBounds[0] < tmpViewBounds[0] ) {
                    tmpViewBounds[0] = tmpXYPlot->plotAreaBounds[0];
                }
                if( tmpXYPlot->plotAreaBounds[1] > tmpViewBounds[1] ) {
                    tmpViewBounds[1] = tmpXYPlot->plotAreaBounds[1];
                }
                if( tmpXYPlot->plotAreaBounds[2] < tmpViewBounds[2] ) {
                    tmpViewBounds[2] = tmpXYPlot->plotAreaBounds[2];
                }
                if( tmpXYPlot->plotAreaBounds[3] > tmpViewBounds[3] ) {
                    tmpViewBounds[3] = tmpXYPlot->plotAreaBounds[3];
                }
                if( tmpXYPlot->plotAreaBounds[4] < tmpViewBounds[4] ) {
                    tmpViewBounds[4] = tmpXYPlot->plotAreaBounds[4];
                }
                if( tmpXYPlot->plotAreaBounds[5] > tmpViewBounds[5] ) {
                    tmpViewBounds[5] = tmpXYPlot->plotAreaBounds[5];
                }
                if( acquisitionType == "SINGLE VOXEL" ) {
                    memcpy( tmpViewBounds, selectionBoxActor->GetBounds(), sizeof(double)*6 );
                }
            }
        }
    }

    // verify camera positioning
    if( viewBounds == NULL ) {
        viewBounds = tmpViewBounds;
        AlignCamera();
    } else if(  viewBounds[0] != tmpViewBounds[0] ||
                viewBounds[1] != tmpViewBounds[1] ||
                viewBounds[2] != tmpViewBounds[2] ||
                viewBounds[3] != tmpViewBounds[3] ||
                viewBounds[4] != tmpViewBounds[4] ||
                viewBounds[5] != tmpViewBounds[5] || acquisitionType == "SINGLE VOXEL") {
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


    if (DEBUG) {
        cout << "Warning(svkPlotLineGrid::GenerateActor):  Stabilize conventions for data ordering CellID ordering and x,y sign convention, tlc, brc, etc. (TODO) " << endl;
    }

    //  Note that 2D selection range is defined from tlc to brc of rubber band with y 
    //  increasing in negative direction.
    this->AllocateXYPlots();
    double origin[3] = { this->data->GetOrigin()[0],this->data->GetOrigin()[1],this->data->GetOrigin()[2] };
    double spacing[3] = { this->data->GetSpacing()[0],this->data->GetSpacing()[1],this->data->GetSpacing()[2] };

    // TODO: Generalize for oblique single voxel
    if( acquisitionType == "SINGLE VOXEL" ) {
        this->data->GetSelectionBoxSpacing( spacing );
        this->data->GetSelectionBoxOrigin( origin );
    } 
    int arrayLength = this->data->GetCellData()->GetArray(0)->GetNumberOfTuples();
    int* extent = this->data->GetExtent();
    double dcos[3][3];
    this->data->GetDcos( dcos );

    vtkPolyData* tmpPolyData =NULL; 
    vtkPoints* tmpPoints = NULL;

    if( this->appender != NULL ) {
        this->appender->Delete();
        this->appender = NULL;
    }
    this->appender = vtkAppendPolyData::New();
    this->polyDataCollection->RemoveAllItems();

    for (int zInd = extent[4]; zInd < extent[5]; zInd++) {
        for (int yInd = extent[2]; yInd < extent[3]; yInd++) {
            for (int xInd = extent[0]; xInd < extent[1]; xInd++) {
                tmpPolyData = vtkPolyData::New();
                tmpPolyData->Allocate(1,1);

                tmpPoints = vtkPoints::New();
                tmpPoints->SetNumberOfPoints( arrayLength + 1 );
                tmpPolyData->SetPoints( tmpPoints );
                //  =====================================  
                //  Load data arrays into plots: 
                //  =====================================  
                voxelIndex[0] = xInd;
                voxelIndex[1] = yInd;
                voxelIndex[2] = zInd;

                ID = this->data->GetIDFromIndex(xInd, yInd, zInd); 
                tmpXYPlot = static_cast<svkPlotLine*>( xyPlots->GetItemAsObject(ID) ); 
                tmpXYPlot->GetPointIds()->SetNumberOfIds(arrayLength);
                tmpXYPlot->SetDataPoints( tmpPoints );

                if (DEBUG) {
                    cout << "CREATE: " << xInd << " " << yInd << " " << zInd;
                    cout << " True ID " << this->data->ComputeCellId( voxelIndex ) << endl;
                }
                tmpXYPlot->SetDcos( dcos );
                tmpXYPlot->SetSpacing( spacing );
                tmpXYPlot->SetOffset( 0 );
                tmpXYPlot->SetData( 
                        vtkFloatArray::SafeDownCast(
                            this->data->GetSpectrum(xInd, yInd, zInd , 0, channel )) );

                tmpXYPlot->GeneratePolyData();
                tmpPolyData->InsertNextCell(tmpXYPlot->GetCellType(), tmpXYPlot->GetPointIds());
                tmpPolyData->Update();
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

                tmpXYPlot->SetPointRange(plotRangeX1, plotRangeX2);
                tmpXYPlot->SetValueRange(plotRangeY1, plotRangeY2);
                polyDataCollection->AddItem( tmpPolyData );
                tmpPolyData->Delete();
                tmpPoints->Delete();
            }
        }
    }
    vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
    mapper->SetInput( appender->GetOutput() );

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
   // renderer->RemoveAllViewProps();
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
        for( int i = 0; i < this->data->GetNumberOfSlices(this->orientation); i++ ) {
            this->freqUpToDate[i] = 0;
        }
    }
    this->UpdatePlotRange();
}


/*!
 *
 */
void svkPlotLineGrid::GetFrequencyWLRange(int &lower, int &upper)
{
    lower = this->plotRangeX1; 
    upper = this->plotRangeX2; 
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
        for( int i = 0; i < this->data->GetNumberOfSlices(this->orientation); i++ ) {
            this->ampUpToDate[i] = 0;
        }
    }
    this->UpdatePlotRange();
}


/*!
 *
 */
void svkPlotLineGrid::GetIntensityWLRange(double &lower, double &upper)
{
    lower = this->plotRangeY1;
    upper = this->plotRangeY2;  
}


/*!
 *  Sets the X,Y range for all plots in current slice.
 */
void svkPlotLineGrid::UpdatePlotRange()
{
    svkPlotLine* tmpXYPlot;
    int ID;
    int* extent = data->GetExtent();
    if( data != NULL && (!this->ampUpToDate[slice] || (!this->freqUpToDate[slice]) ||
        ampSelectionUpToDate[0] > tlcBrc[0] || ampSelectionUpToDate[1] < tlcBrc[1] ||
        freqSelectionUpToDate[0] > tlcBrc[0] || freqSelectionUpToDate[1] < tlcBrc[1] )) { 

        int rowRange[2] = {0,0};
        int columnRange[2] = {0,0};
        int sliceRange[2] = {0,0};

        this->data->GetIndexFromID( tlcBrc[0], &rowRange[0], &columnRange[0], &sliceRange[0] );
        this->data->GetIndexFromID( tlcBrc[1], &rowRange[1], &columnRange[1], &sliceRange[1] );

        for (int sliceIndex = sliceRange[0]; sliceIndex <= sliceRange[1]; sliceIndex++) {
            for (int rowIndex = rowRange[0]; rowIndex <= rowRange[1]; rowIndex++) {
                for (int columnIndex = columnRange[0]; columnIndex <= columnRange[1]; columnIndex++) {
                    ID = this->data->GetIDFromIndex(rowIndex, columnIndex, sliceIndex );
                    tmpXYPlot = static_cast<svkPlotLine*>( xyPlots->GetItemAsObject(ID) ); 
                    if( !this->freqUpToDate[slice] ||
                        freqSelectionUpToDate[0] > tlcBrc[0] || freqSelectionUpToDate[1] < tlcBrc[1] ) {
                        tmpXYPlot->SetPointRange(this->plotRangeX1, this->plotRangeX2);
                    }
                    if( !this->ampUpToDate[slice]  ||
                        ampSelectionUpToDate[0] > tlcBrc[0] || ampSelectionUpToDate[1] < tlcBrc[1] ) {
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
    this->ampSelectionUpToDate[0] = tlcBrc[0];
    this->ampSelectionUpToDate[1] = tlcBrc[1];
    this->freqSelectionUpToDate[0] = tlcBrc[0];
    this->freqSelectionUpToDate[1] = tlcBrc[1];
}


//! Regenerates the plot data, used when data is modified.
void svkPlotLineGrid::RegeneratePlots()
{
    int rowRange[2] = {0,0};
    int columnRange[2] = {0,0};
    int sliceRange[2] = {0,0};
    this->data->GetIndexFromID( tlcBrc[0], &rowRange[0], &columnRange[0], &sliceRange[0] );
    this->data->GetIndexFromID( tlcBrc[1], &rowRange[1], &columnRange[1], &sliceRange[1] );
    for (int rowIndex = rowRange[0]; rowIndex <= rowRange[1]; rowIndex++) {
        for (int columnIndex = columnRange[0]; columnIndex <= columnRange[1]; columnIndex++) {
            for (int sliceIndex = sliceRange[0]; sliceIndex <= sliceRange[1]; sliceIndex++) {
            static_cast<svkPlotLine*>( xyPlots->GetItemAsObject(
                        this->data->GetIDFromIndex(rowIndex, columnIndex, sliceIndex)))->GeneratePolyData(); 
            }
        }
    }

}


//! Resets the camera to look at the new selection
void svkPlotLineGrid::AlignCamera( bool invertView ) 
{  
    if( this->renderer != NULL && viewBounds != NULL ) {
        string acquisitionType = data->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
        float normal[3];
        this->data->GetSliceNormal( normal, this->orientation );
        double zoom;
        double viewWidth =  viewBounds[1] - viewBounds[0];
        double viewHeight = viewBounds[3] - viewBounds[2];
        double viewDepth =  viewBounds[5] - viewBounds[4];
        double diagonal = sqrt( pow(viewWidth,2) + pow(viewHeight,2) + pow(viewDepth,2) );
        int toggleDraw = this->renderer->GetDraw();
        if( toggleDraw ) {
            this->renderer->DrawOff();
        }

        this->renderer->ResetCamera( viewBounds );
        // if the data set is not axial, move the camera
        double* focalPoint = this->renderer->GetActiveCamera()->GetFocalPoint();
        double* cameraPosition = this->renderer->GetActiveCamera()->GetPosition();
        double distance = sqrt( pow( focalPoint[0] - cameraPosition[0], 2 ) +
                                pow( focalPoint[1] - cameraPosition[1], 2 ) +
                                pow( focalPoint[2] - cameraPosition[2], 2 ) );
        
        double newCameraPosition[3] = {0,0,0};

        // Lets calculate the distance from the focal point to the selection box
        if( this->orientation == svkDcmHeader::AXIAL && normal[2] > 0 ) {
            distance *=-1;
        } else if( this->orientation == svkDcmHeader::CORONAL && normal[1] > 0 ) { 
            distance *=-1;
        } else if( this->orientation == svkDcmHeader::SAGITTAL && normal[0] < 0 ) { 
            distance *=-1;
        }
         
        newCameraPosition[0] = focalPoint[0] + distance*normal[0]; 
        newCameraPosition[1] = focalPoint[1] + distance*normal[1];
        newCameraPosition[2] = focalPoint[2] + distance*normal[2];
        this->renderer->GetActiveCamera()->SetPosition( newCameraPosition );

        double* visibleBounds  = this->renderer->ComputeVisiblePropBounds();
        double thickness = sqrt( pow( visibleBounds[1] - visibleBounds[0], 2 ) +
                                 pow( visibleBounds[3] - visibleBounds[2], 2 ) +
                                 pow( visibleBounds[5] - visibleBounds[4], 2 ) );
        this->renderer->GetActiveCamera()->SetThickness( thickness );

        double columnNormal[3];
        float viewUp[3];
        int inverter = -1;

        switch ( this->orientation ) {
            case svkDcmHeader::AXIAL:
                this->data->GetSliceNormal( viewUp, svkDcmHeader::CORONAL );
                if( viewUp[1] < 0 ) {
                    inverter *=-1;
                }
                this->renderer->GetActiveCamera()->SetViewUp( inverter*viewUp[0], 
                                                              inverter*viewUp[1], 
                                                              inverter*viewUp[2] );
                break;
            case svkDcmHeader::CORONAL:
                this->data->GetSliceNormal( viewUp, svkDcmHeader::AXIAL );
                if( viewUp[2] > 0 ) {
                    inverter *=-1;
                }
                this->renderer->GetActiveCamera()->SetViewUp( inverter*viewUp[0], 
                                                              inverter*viewUp[1], 
                                                              inverter*viewUp[2] );
                break;
            case svkDcmHeader::SAGITTAL:
                this->data->GetSliceNormal( viewUp, svkDcmHeader::AXIAL );
                if( viewUp[2] > 0 ) {
                    inverter *=-1;
                }
                this->renderer->GetActiveCamera()->SetViewUp( inverter*viewUp[0], 
                                                              inverter*viewUp[1], 
                                                              inverter*viewUp[2] );
                break;
        }

        if( viewWidth >= viewHeight && viewWidth >= viewDepth ) {
            zoom = diagonal/viewWidth;        
        } else if( viewHeight >= viewWidth && viewHeight >= viewDepth ) {
            zoom = diagonal/viewHeight;        
        } else {
            zoom = diagonal/viewDepth;        
        }
        // We'll back off the zoom to 95% to leave some edges
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
    if( this->data != NULL ) {
        int tlcBrcImageData[2];
        this->data->GetTlcBrcInSelectionBox( tlcBrcImageData, this->orientation, this->slice );
        this->SetTlcBrc( tlcBrcImageData );
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
 *  Checks to see if the component has change, if it has it updates.
 */
void svkPlotLineGrid::UpdateOrientation()
{
    if( data != NULL ) {
        vtkCollectionIterator* iterator = vtkCollectionIterator::New();
        svkPlotLine* tmpBoxPlot;
        iterator->SetCollection(xyPlots);
        iterator->InitTraversal();
        svkDcmHeader::Orientation dataOrientation = this->data->GetDcmHeader()->GetOrientationType();
        bool mirrorPlots = false;
        bool invertPlots = false;
        double LRNormal[3];
        this->data->GetDataBasis(LRNormal, svkImageData::LR );
        double PANormal[3];
        this->data->GetDataBasis(PANormal, svkImageData::PA );
        double SINormal[3];
        this->data->GetDataBasis(SINormal, svkImageData::SI );  
        svkPlotLine::PlotDirection plotDirection;

        //TODO: Get rid of this nested switch, maybe by manipulating the dcos and/or origin of plotLines...
        switch( dataOrientation ) {
            case svkDcmHeader::AXIAL:
                switch( this->orientation ) {
                    case svkDcmHeader::AXIAL:
                        // LR/PA direction
                        if( LRNormal[0] < 0 ) {
                            mirrorPlots = true;
                        }
                        if( PANormal[1] > 0 ) {
                            invertPlots = true;
                        }
                        plotDirection = svkPlotLine::ROW_COLUMN;
                        break;
                    case svkDcmHeader::CORONAL:
                        // LR/SI
                        if( LRNormal[0] < 0 ) {
                            mirrorPlots = true;
                        }
                        if( SINormal[2] < 0 ) {
                            invertPlots = true;
                        }
                        plotDirection = svkPlotLine::ROW_SLICE;
                        break;
                    case svkDcmHeader::SAGITTAL:
                        // PA/SI
                        if( PANormal[1] < 0 ) {
                            mirrorPlots = true;
                        }
                        if( SINormal[2] < 0 ) {
                            invertPlots = true;
                        }
                        plotDirection = svkPlotLine::COLUMN_SLICE;
                        break;
                }
                break;
            case svkDcmHeader::CORONAL:
                switch( this->orientation ) {
                    case svkDcmHeader::AXIAL:
                        // LR/PA direction
                        if( LRNormal[0] < 0 ) {
                            mirrorPlots = true;
                        }
                        if( PANormal[2] > 0 ) {
                            invertPlots = true;
                        }
                        plotDirection = svkPlotLine::ROW_SLICE;
                        break;
                    case svkDcmHeader::CORONAL:
                        // LR/SI
                        if( LRNormal[0] < 0 ) {
                            mirrorPlots = true;
                        }
                        if( SINormal[1] < 0 ) {
                            invertPlots = true;
                        }
                        plotDirection = svkPlotLine::ROW_COLUMN;
                        break;
                    case svkDcmHeader::SAGITTAL:
                        plotDirection = svkPlotLine::SLICE_COLUMN;
                        // PA/SI
                        if( PANormal[2] < 0 ) {
                            mirrorPlots = true;
                        }
                        if( SINormal[1] < 0 ) {
                            invertPlots = true;
                        }
                        break;
                }
                break;
            case svkDcmHeader::SAGITTAL:
                switch( this->orientation ) {
                    case svkDcmHeader::AXIAL:
                        // LR/PA direction
                        if( LRNormal[2] < 0 ) {
                            mirrorPlots = true;
                        }
                        if( PANormal[0] > 0 ) {
                            invertPlots = true;
                        }
                        plotDirection = svkPlotLine::SLICE_ROW;
                        break;
                    case svkDcmHeader::CORONAL:
                        // LR/SI
                        if( LRNormal[2] < 0 ) {
                            mirrorPlots = true;
                        }
                        if( SINormal[1] < 0 ) {
                            invertPlots = true;
                        }
                        plotDirection = svkPlotLine::SLICE_COLUMN;
                        break;
                    case svkDcmHeader::SAGITTAL:
                        // PA/SI
                        if( PANormal[0] < 0 ) {
                            mirrorPlots = true;
                        }
                        if( SINormal[1] < 0 ) {
                            invertPlots = true;
                        }
                        break;
                }
                break;
        }

        string acquisitionType = data->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
        //if( acquisitionType == "SINGLE VOXEL" ) {
        //    mirrorPlots = false;
        //}
        while(!iterator->IsDoneWithTraversal()) {
            tmpBoxPlot = static_cast<svkPlotLine*>( iterator->GetCurrentObject()); 
            tmpBoxPlot->SetPlotDirection( plotDirection );
            tmpBoxPlot->SetMirrorPlots( mirrorPlots );
            tmpBoxPlot->GeneratePolyData();
            tmpBoxPlot->SetInvertPlots( invertPlots );
            iterator->GoToNextItem();
        }
        iterator->Delete();
        this->Update();
    }
}


/*!
 *
 */
void svkPlotLineGrid::SetChannel( int channel )
{
    this->channel = channel;
    if( this->data != NULL ) {
        int numChannels = this->data->GetDcmHeader()->GetNumberOfCoils();
        if ( channel >= numChannels ) {
            channel = numChannels -1;
        } else if ( channel < 0 ) {
            channel = 0;
        }
        int minIndex[3] = {0,0,0};
        int maxIndex[3] = {this->data->GetDimensions()[0]-2,this->data->GetDimensions()[1]-2,this->data->GetDimensions()[2]-2};
        int orientationIndex = this->data->GetOrientationIndex( this->orientation );

        minIndex[ orientationIndex ] = this->slice;
        maxIndex[ orientationIndex ] = this->slice;
        int minID = this->data->GetIDFromIndex( minIndex[0], minIndex[1], minIndex[2] );
        int maxID = this->data->GetIDFromIndex( maxIndex[0], maxIndex[1], maxIndex[2] );
        this->UpdateDataArrays( minID, maxID);
    }
}


/*!
 *
 */
void svkPlotLineGrid::SetTimePoint( int timePoint )
{
    this->timePoint = timePoint;
    int minIndex[3] = {0,0,0};
    int maxIndex[3] = {this->data->GetDimensions()[0]-2,this->data->GetDimensions()[1]-2,this->data->GetDimensions()[2]-2};
    int orientationIndex = this->data->GetOrientationIndex( this->orientation );

    minIndex[ orientationIndex ] = this->slice;
    maxIndex[ orientationIndex ] = this->slice;
    int minID = this->data->GetIDFromIndex( minIndex[0], minIndex[1], minIndex[2] );
    int maxID = this->data->GetIDFromIndex( maxIndex[0], maxIndex[1], maxIndex[2] );
    this->UpdateDataArrays( minID, maxID);
}


/*!
 *
 */
void svkPlotLineGrid::UpdateDataArrays( int tlc, int brc)
{
    svkPlotLine* tmpXYPlot;
    int ID;
    if( data != NULL ) { 
        int rowRange[2] = {0,0};
        int columnRange[2] = {0,0};
        int sliceRange[2] = {0,0};
        this->data->GetIndexFromID( tlc, &rowRange[0], &columnRange[0], &sliceRange[0] );
        this->data->GetIndexFromID( brc, &rowRange[1], &columnRange[1], &sliceRange[1] );
        for (int rowIndex = rowRange[0]; rowIndex <= rowRange[1]; rowIndex++) {
            for (int columnIndex = columnRange[0]; columnIndex <= columnRange[1]; columnIndex++) {
                for (int sliceIndex = sliceRange[0]; sliceIndex <= sliceRange[1]; sliceIndex++) {
                
                    ID = this->data->GetIDFromIndex( rowIndex, columnIndex, sliceIndex );
                    tmpXYPlot = static_cast<svkPlotLine*>( xyPlots->GetItemAsObject(ID) ); 
                    tmpXYPlot->SetData( vtkFloatArray::SafeDownCast(this->data->GetSpectrum(rowIndex, columnIndex, sliceIndex, this->timePoint, this->channel)));
                }
            }
            tmpXYPlot->polyLinePoints->Modified();
        }
    }
}


/*!
 *
 */
void svkPlotLineGrid::SetOrientation( svkDcmHeader::Orientation orientation )
{
    svkDcmHeader::Orientation oldOrientation = this->orientation;
    this->orientation = orientation;    
    if( this->data!=NULL ) {
        if( this->freqUpToDate != NULL ) {
            delete[] this->freqUpToDate;
        }
        this->freqUpToDate = new bool[ this->data->GetNumberOfSlices( this->orientation ) ];
        for( int i = 0; i < this->data->GetNumberOfSlices( this->orientation ); i++ ) {
            this->freqUpToDate[i] = 0;
        }
        if( this->ampUpToDate != NULL ) {
            delete[] this->ampUpToDate;
        }
        this->ampUpToDate = new bool[ this->data->GetNumberOfSlices( this->orientation ) ];
        for( int i = 0; i < this->data->GetNumberOfSlices( this->orientation ); i++ ) {
            this->ampUpToDate[i] = 0;
        }
        svkDataView::ResetTlcBrcForNewOrientation( this->data, this->orientation, this->tlcBrc, this->slice );

        if( !svkDataView::IsTlcBrcWithinData( this->data, tlcBrc ) ) {
            int lastSlice  = data->GetLastSlice( this->orientation );
            int firstSlice = data->GetFirstSlice( this->orientation );
            this->slice = (lastSlice-firstSlice)/2;
            this->SetSlice( this->slice );
            this->HighlightSelectionVoxels();
        }

        this->UpdateOrientation();
        this->UpdatePlotRange();
        this->AlignCamera();
    }
}


/*!
 *
 */
vtkActor* svkPlotLineGrid::GetPlotGridActor()
{
    return this->plotGridActor;
}
