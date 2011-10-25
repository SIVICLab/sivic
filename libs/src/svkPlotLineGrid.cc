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


using namespace svk;
using namespace std;


vtkCxxRevisionMacro(svkPlotLineGrid, "$Rev$");
vtkStandardNewMacro(svkPlotLineGrid);

//! Constructor 
svkPlotLineGrid::svkPlotLineGrid()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    this->data = NULL;
    this->slice = 0;
    this->plotGridActor = NULL;
    this->appender = NULL;
    this->orientation = svkDcmHeader::AXIAL;
    //this->xyPlots = NULL;


    /*
     * This arrays are used for improving performance. They keep track
     * of whether modifications have been made on the current slice, so that
     * when the slice eventually does change the new slice can be updated
     * accordingly.
     * 
     * The first element of the UpToDate array is the top left corner voxel
     * of the currently updated voxels, and the second element is the bottom
     * right corner.
     */
    this->freqSelectionUpToDate[0] = -1;
    this->freqSelectionUpToDate[1] = -1;
    this->ampSelectionUpToDate[0] = -1;
    this->ampSelectionUpToDate[1] = -1;

    // These are per slice
    this->freqUpToDate = NULL;
    this->ampUpToDate = NULL;

    // This callback will catch changes to the dataset
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

    // holds the svkPlotLines
    //this->polyDataCollection = vtkPolyDataCollection::New();
    this->polyDataVector.clear();
    this->volumeUpToDate.clear();

    // By default we will look at the 0th volume.
    this->volumeIndexVector.clear();

}


//! Destructor
svkPlotLineGrid::~svkPlotLineGrid()
{
    if( this->plotGridActor != NULL ) {
        this->plotGridActor->Delete();
        this->plotGridActor = NULL;
    }

    if( this->appender != NULL ) {
        this->appender->Delete();
        this->appender = NULL;
    }

    if( this->data != NULL ) {
        this->data->Delete();
        this->data = NULL;
    }
    
    if( this->dataModifiedCB != NULL ) {
        this->dataModifiedCB->Delete();
        this->dataModifiedCB = NULL;
    }

    for( vector<svkPlotLine*>::iterator iter = this->xyPlots.begin();
        iter != this->xyPlots.end(); ++iter) {
        if( (*iter) != NULL ) {
            (*iter)->Delete();
            (*iter) = NULL;
        }
    }

    for( vector<vtkPolyData*>::iterator iter = this->polyDataVector.begin();
        iter != this->polyDataVector.end(); ++iter) {
        if( (*iter) != NULL ) {
            (*iter)->Delete();
            (*iter) = NULL;
        }
    }


    delete[] freqUpToDate;
    delete[] ampUpToDate;
}

svk4DImageData* svkPlotLineGrid::GetInput()
{
    return this->data;
}


/*! 
 * Set input data and initialize default range values. Then it generates the actors
 * for the entire dataset upfront, to allow for fast SetSlice operations.
 *
 * \param data the data to be viewed
 */
void svkPlotLineGrid::SetInput(svk4DImageData* data)
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

    // Observe the data for changes
    this->data->AddObserver(vtkCommand::ModifiedEvent, dataModifiedCB);
    int* extent = this->data->GetExtent(); 

    if( this->freqUpToDate != NULL ) {
        delete[] this->freqUpToDate;
    }
    if( this->ampUpToDate != NULL ) {
        delete[] this->ampUpToDate;
    }

    this->freqSelectionUpToDate[0] = -1;
    this->freqSelectionUpToDate[1] = -1;
    this->ampSelectionUpToDate[0] = -1;
    this->ampSelectionUpToDate[1] = -1;
    
    this->ampUpToDate = new bool[ this->data->GetNumberOfSlices( this->orientation ) ];
    this->freqUpToDate = new bool[ this->data->GetNumberOfSlices( this->orientation ) ];

    for( int i = 0; i < this->data->GetNumberOfSlices( this->orientation ); i++ ) {
        this->freqUpToDate[i] = 0;
        this->ampUpToDate[i] = 0;
    }

    //  Set default plot range to full scale:
    int numFrequencyPoints = this->data->GetCellData()->GetNumberOfTuples();

    this->volumeIndexVector.clear();
    for( int i = 0; i < this->data->GetNumberOfVolumeDimensions(); i++ ) {
        this->volumeIndexVector.push_back(0);
    }
    this->InitializeVolumeUpToDateVector();

    //  Get these from data or our subclass of vtkImageData tbd:
    this->plotRangeX1 = 0;
    this->plotRangeX2 = numFrequencyPoints-1;
    this->plotRangeY1 = range[0];
    this->plotRangeY2 = range[1];
    // Remove existing plots
    this->ClearXYPlots();
    this->AllocateXYPlots();
    this->GenerateActor();

    // In case we already had the slice set
    this->SetSlice( this->slice );
    this->Update(tlcBrc);
}


//! Allocates plot and grid vectors
void svkPlotLineGrid::AllocateXYPlots()
{
    this->ClearXYPlots();
    svkPlotLine* tmpXYPlot;
    int* extent = this->data->GetExtent();
    for (int zInd = extent[4] ; zInd < extent[5]; zInd++) {
        for (int yInd = extent[2] ; yInd < extent[3]; yInd++) {
            for (int xInd = extent[0]; xInd < extent[1]; xInd++) {
                tmpXYPlot = svkPlotLine::New();
                this->xyPlots.push_back( tmpXYPlot );
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
        this->data->GetIndexFromID( this->tlcBrc[0], tlcIndex );
        this->data->GetIndexFromID( this->tlcBrc[1], brcIndex );
        int lastSlice  = this->data->GetLastSlice( this->orientation );
        int firstSlice = this->data->GetFirstSlice( this->orientation );
        slice = (slice > lastSlice) ? lastSlice:slice;
        slice = (slice < firstSlice) ? firstSlice:slice;
        tlcIndex[ this->data->GetOrientationIndex( this->orientation ) ] = slice;
        brcIndex[ this->data->GetOrientationIndex( this->orientation ) ] = slice;
        this->tlcBrc[0] = this->data->GetIDFromIndex( tlcIndex[0], tlcIndex[1], tlcIndex[2] );
        this->tlcBrc[1] = this->data->GetIDFromIndex( brcIndex[0], brcIndex[1], brcIndex[2] );
        if( !this->IsSliceUpToDate( slice ) ) {
            int minIndex[3] = {0,0,0};
            int maxIndex[3] = {this->data->GetDimensions()[0]-2,this->data->GetDimensions()[1]-2,this->data->GetDimensions()[2]-2};
            int orientationIndex = this->data->GetOrientationIndex( this->orientation );

            minIndex[ orientationIndex ] = slice;
            maxIndex[ orientationIndex ] = slice;
            int minID = this->data->GetIDFromIndex( minIndex[0], minIndex[1], minIndex[2] );
            int maxID = this->data->GetIDFromIndex( maxIndex[0], maxIndex[1], maxIndex[2] );
            this->UpdateDataArrays( minID, maxID);
            this->SetSliceUpToDate( slice );
        }
        this->UpdatePlotRange(this->tlcBrc);
        this->Update(this->tlcBrc);
        this->SetSliceAppender();
    }
}

/*!
 *  Method appends all svkPolyLine objects into one large dataset. 
 */
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
                this->appender->AddInput(vtkPolyData::SafeDownCast(polyDataVector[ID]));
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
        // Get min/max indecies for this dataset
        int minIndex[3] = { 0,0,0 };
        int maxIndex[3] = { this->data->GetDimensions()[0]-2,
                            this->data->GetDimensions()[1]-2,
                            this->data->GetDimensions()[2]-2 };

        // Find min/max indecies for this slice
        int orientationIndex = this->data->GetOrientationIndex( this->orientation );
        minIndex[ orientationIndex ] = this->slice;
        maxIndex[ orientationIndex ] = this->slice;
        int minID = this->data->GetIDFromIndex( minIndex[0], minIndex[1], minIndex[2] );
        int maxID = this->data->GetIDFromIndex( maxIndex[0], maxIndex[1], maxIndex[2] );

        // Check for out of bounds 
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

        this->UpdatePlotRange(this->tlcBrc);
        this->Update(this->tlcBrc);
    } 

}


/*!
 *  Return the indecies of the top left corner, and bottom right corner
 */
int* svkPlotLineGrid::GetTlcBrc() 
{
    return this->tlcBrc;
}


/*!
 *  This calculates the boundaries for a given top left/ bottom right corners
 *
 *  TODO: Make this a utility method in svk4DImageData and make it use cell points
 *
 *  \param bounds this is to be populated by the method
 *  \param tlcBrc the top left/ bottom right corners of the query
*/
void svkPlotLineGrid::CalculateTlcBrcBounds( double bounds[6], int tlcBrc[2])
{
    if( data == NULL ) {
        return;
    }

    // Initialize bounds
    bounds[0] = VTK_DOUBLE_MAX;
    bounds[1] = -VTK_DOUBLE_MAX;
    bounds[2] = VTK_DOUBLE_MAX;
    bounds[3] = -VTK_DOUBLE_MAX;
    bounds[4] = VTK_DOUBLE_MAX;
    bounds[5] = -VTK_DOUBLE_MAX;
    svkPlotLine* tmpXYPlot;

    int rowRange[2]    = {0,0};
    int columnRange[2] = {0,0};
    int sliceRange[2]  = {0,0};

    // Get indecies from the tlcBrc
    this->data->GetIndexFromID( tlcBrc[0], &rowRange[0], &columnRange[0], &sliceRange[0] );
    this->data->GetIndexFromID( tlcBrc[1], &rowRange[1], &columnRange[1], &sliceRange[1] );

    // cycle through all plot objects getting their bounds
    int ID;
    for (int rowIndex = rowRange[0]; rowIndex <= rowRange[1]; rowIndex++) {
        for (int columnIndex = columnRange[0]; columnIndex <= columnRange[1]; columnIndex++) {
            for (int sliceIndex = sliceRange[0]; sliceIndex <= sliceRange[1]; sliceIndex++) {
            
                ID = this->data->GetIDFromIndex( rowIndex, columnIndex, sliceIndex );
                tmpXYPlot = static_cast<svkPlotLine*>( this->xyPlots[ID] );

                // We use the cellbounds to reset the camera's fov        
                if( tmpXYPlot->plotAreaBounds[0] < bounds[0] ) {
                    bounds[0] = tmpXYPlot->plotAreaBounds[0];
                }
                if( tmpXYPlot->plotAreaBounds[1] > bounds[1] ) {
                    bounds[1] = tmpXYPlot->plotAreaBounds[1];
                }
                if( tmpXYPlot->plotAreaBounds[2] < bounds[2] ) {
                    bounds[2] = tmpXYPlot->plotAreaBounds[2];
                }
                if( tmpXYPlot->plotAreaBounds[3] > bounds[3] ) {
                    bounds[3] = tmpXYPlot->plotAreaBounds[3];
                }
                if( tmpXYPlot->plotAreaBounds[4] < bounds[4] ) {
                    bounds[4] = tmpXYPlot->plotAreaBounds[4];
                }
                if( tmpXYPlot->plotAreaBounds[5] > bounds[5] ) {
                    bounds[5] = tmpXYPlot->plotAreaBounds[5];
                }
            }
        }
    }
}

void svkPlotLineGrid::SetColor( double rgb[3])
{
    if( this->plotGridActor != NULL ) {
        this->plotGridActor->GetProperty()->SetColor(rgb);
    }
}

double* svkPlotLineGrid::GetColor( )
{
    if( this->plotGridActor != NULL ) {
        return this->plotGridActor->GetProperty()->GetColor();
    }
    return NULL;
}

/*!
 *  Update the view based on voxelIndexTLC/BRC. This regenerates
 *  poly data if the anything has changed.
 *
 * \param tlcBrc the top left / bottom right corner range to update
 */
void svkPlotLineGrid::Update( int tlcBrc[2])
{
    if( data == NULL ) {
        return;
    }
    int ID;
    svkPlotLine* tmpXYPlot;

    int rowRange[2] = {0,0};
    int columnRange[2] = {0,0};
    int sliceRange[2] = {0,0};
    this->data->GetIndexFromID( tlcBrc[0], &rowRange[0], &columnRange[0], &sliceRange[0] );
    this->data->GetIndexFromID( tlcBrc[1], &rowRange[1], &columnRange[1], &sliceRange[1] );
    unsigned long int dataMTime = data->GetMTime();
    for (int rowIndex = rowRange[0]; rowIndex <= rowRange[1]; rowIndex++) {
        for (int columnIndex = columnRange[0]; columnIndex <= columnRange[1]; columnIndex++) {
            for (int sliceIndex = sliceRange[0]; sliceIndex <= sliceRange[1]; sliceIndex++) {
            
                ID = this->data->GetIDFromIndex( rowIndex, columnIndex, sliceIndex );
                tmpXYPlot = static_cast<svkPlotLine*>( this->xyPlots[ID] );

                // If the data has been update since the actors- then regenerate them
                if( dataMTime > tmpXYPlot->GetMTime() ) {
                    tmpXYPlot->GeneratePolyData();
                }

            }
        }
    }
}


//! Generate plot line grid actor to be used 
void svkPlotLineGrid::GenerateActor()
{
    int* voxelIndex = new int[3];
    int ID;
    double* cellBounds;
    svkPlotLine* tmpXYPlot;
    this->ClearXYPlots();
    string acquisitionType;
    if( this->data->IsA( "svkMrsImageData" )) {
        acquisitionType = data->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
    }

    if (this->GetDebug()) {
        cout << "Warning(svkPlotLineGrid::GenerateActor): " 
             << " Stabilize conventions for data ordering CellID ordering" 
             << " and x,y sign convention, tlc, brc, etc. (TODO) " << endl;
    }

    this->AllocateXYPlots();
    double origin[3] =  { this->data->GetOrigin()[0],
                          this->data->GetOrigin()[1],
                          this->data->GetOrigin()[2] };

    double spacing[3] = { this->data->GetSpacing()[0],
                          this->data->GetSpacing()[1],
                          this->data->GetSpacing()[2] };

    // TODO: Generalize for oblique single voxel
    if( acquisitionType == "SINGLE VOXEL" ) {
        svkMrsImageData::SafeDownCast(this->data)->GetSelectionBoxSpacing( spacing );
        svkMrsImageData::SafeDownCast(this->data)->GetSelectionBoxOrigin( origin );
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
    vtkIdList* pointIds = NULL;
    this->ClearPolyData();

    // Create svkPlotLines
    int* volumeIndexArray = (int*)this->volumeIndexVector.data();
    for (int zInd = extent[4]; zInd < extent[5]; zInd++) {
        //cout << "z:" << zInd << endl;
        for (int yInd = extent[2]; yInd < extent[3]; yInd++) {
            for (int xInd = extent[0]; xInd < extent[1]; xInd++) {
                tmpPolyData = vtkPolyData::New();
                tmpPolyData->Allocate(1,1);

                tmpPoints = vtkPoints::New();
                tmpPoints->SetNumberOfPoints( arrayLength );
                tmpPolyData->SetPoints( tmpPoints );
                //  =====================================  
                //  Load data arrays into plots: 
                //  =====================================  
                voxelIndex[0] = xInd;
                voxelIndex[1] = yInd;
                voxelIndex[2] = zInd;

                ID = this->data->GetIDFromIndex(xInd, yInd, zInd); 
                tmpXYPlot = static_cast<svkPlotLine*>( this->xyPlots[ID] );
                pointIds = tmpXYPlot->GetPointIds();
                pointIds->SetNumberOfIds(arrayLength);
                tmpXYPlot->SetDataPoints( tmpPoints );

                tmpXYPlot->SetDcos( dcos );
                tmpXYPlot->SetSpacing( spacing );
                tmpXYPlot->SetData( vtkFloatArray::SafeDownCast(
                                    this->data->GetArray(xInd, yInd, zInd , volumeIndexArray )) );
                tmpPolyData->InsertNextCell(VTK_POLY_LINE, pointIds );
                double plotOrigin[3];
        
                // Offset origin for current voxel
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
                this->polyDataVector.push_back( tmpPolyData );
                tmpPoints->FastDelete();
            }
        }
    }

    vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
    mapper->SetInput( appender->GetOutput() );
    if( this->plotGridActor == NULL ) {
        this->plotGridActor = vtkActor::New();
    }

    plotGridActor->SetMapper( mapper );
    mapper->Delete();

    delete[] voxelIndex;

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
void svkPlotLineGrid::SetFrequencyWLRange(int lower, int upper, int tlcBrc[2])
{
    this->plotRangeX1 = lower; 
    this->plotRangeX2 = upper; 
    if( this->data != NULL ) {
        // Make all slices out of date
        for( int i = 0; i < this->data->GetNumberOfSlices(this->orientation); i++ ) {
            this->freqUpToDate[i] = 0;
        }
    }
    this->UpdatePlotRange(tlcBrc);
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
void svkPlotLineGrid::SetIntensityWLRange(double lower, double upper, int tlcBrc[2])
{
    this->plotRangeY1 = lower;
    this->plotRangeY2 = upper;  
    if( this->data != NULL ) {
        // Make all slices out of date 
        for( int i = 0; i < this->data->GetNumberOfSlices(this->orientation); i++ ) {
            this->ampUpToDate[i] = 0;
        }
    }
    this->UpdatePlotRange(tlcBrc);
}


/*!
 *  Gets the current intensity range.
 *
 *  \param lower output value
 *  \param upper output value
 */
void svkPlotLineGrid::GetIntensityWLRange(double &lower, double &upper)
{
    lower = this->plotRangeY1;
    upper = this->plotRangeY2;  
}


/*!
 *  Sets the X,Y range for all plots in the current selection.
 *
 *  \param tlcBrc range to update
 */
void svkPlotLineGrid::UpdatePlotRange( int tlcBrc[2])
{
    if( data == NULL ) {
        return;
    }
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
                    tmpXYPlot = static_cast<svkPlotLine*>( this->xyPlots[ID] );
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

    svkPlotLine* tmpBoxPlot = NULL;
    for( vector<svkPlotLine*>::iterator iter = this->xyPlots.begin();
        iter != this->xyPlots.end(); ++iter) {
        tmpBoxPlot = static_cast<svkPlotLine*>( (*iter));
        tmpBoxPlot->SetComponent( this->plotComponent );
        tmpBoxPlot->GeneratePolyData();
    }
    int rowRange[2] = {0,0};
    int columnRange[2] = {0,0};
    int sliceRange[2] = {0,0};
    this->data->GetIndexFromID( tlcBrc[0], &rowRange[0], &columnRange[0], &sliceRange[0] );
    this->data->GetIndexFromID( tlcBrc[1], &rowRange[1], &columnRange[1], &sliceRange[1] );
    for (int rowIndex = rowRange[0]; rowIndex <= rowRange[1]; rowIndex++) {
        for (int columnIndex = columnRange[0]; columnIndex <= columnRange[1]; columnIndex++) {
            for (int sliceIndex = sliceRange[0]; sliceIndex <= sliceRange[1]; sliceIndex++) {
            static_cast<svkPlotLine*>( this->xyPlots[
                        this->data->GetIDFromIndex(rowIndex, columnIndex, sliceIndex)])->GeneratePolyData();
            }
        }
    }

}


//! Callback tied to data modified events. Regenerates plot data when the vtkImageData is modified.
void svkPlotLineGrid::UpdateData(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata)
{
    static_cast<svkPlotLineGrid*>(thisObject)->RegeneratePlots();
}


/*!
 *  Selects all voxels completely within selection box.
 *  
 */
void svkPlotLineGrid::HighlightSelectionVoxels()
{
    if( this->data != NULL && this->data->IsA( "svkMrsImageData" ) ) {
        int tlcBrcImageData[2];
        svkMrsImageData::SafeDownCast(this->data)->Get2DProjectedTlcBrcInSelectionBox( tlcBrcImageData, this->orientation, this->slice );
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
svkPlotLine::PlotComponent svkPlotLineGrid::GetComponent( )
{
    return this->plotComponent;
}


/*!
 *  Checks to see if the component has change, if it has it updates.
 */
void svkPlotLineGrid::UpdateComponent()
{

    svkPlotLine* tmpBoxPlot = NULL;
    for( vector<svkPlotLine*>::iterator iter = this->xyPlots.begin();
        iter != this->xyPlots.end(); ++iter) {
        tmpBoxPlot = static_cast<svkPlotLine*>( (*iter));
        tmpBoxPlot->SetComponent( this->plotComponent );
        tmpBoxPlot->GeneratePolyData();
    }

    if (this->GetDebug()) {
        cout << " Reset component " << plotComponent << endl;
    }
    this->Update(tlcBrc);
}


/*!
 *  Updates plot lines orientation.
 */
void svkPlotLineGrid::UpdateOrientation()
{
    if( data != NULL ) {
        svkPlotLine* tmpBoxPlot;
        svkDcmHeader::Orientation dataOrientation = this->data->GetDcmHeader()->GetOrientationType();

        // Depending on orientation of camera plots will have to be inverter, mirrored
        bool mirrorPlots = false;
        bool invertPlots = false;
        double LRNormal[3];
        double dcos[3][3];
        this->data->GetDcos( dcos );
        this->data->GetDataBasis(LRNormal, svkImageData::LR );
        double PANormal[3];
        this->data->GetDataBasis(PANormal, svkImageData::PA );
        double SINormal[3];
        this->data->GetDataBasis(SINormal, svkImageData::SI );  
        svkPlotLine::PlotDirection plotDirection;
        int amplitudeIndex;
        int pointIndex;
        
         
        int axialIndex = this->data->GetOrientationIndex( svkDcmHeader::AXIAL );
        int coronalIndex = this->data->GetOrientationIndex( svkDcmHeader::CORONAL );
        int sagittalIndex = this->data->GetOrientationIndex( svkDcmHeader::SAGITTAL );
        switch( this->orientation ) {
            case svkDcmHeader::AXIAL:

                amplitudeIndex = coronalIndex;
                if( dcos[amplitudeIndex][1] > 0 ) {
                    invertPlots = true;
                }         

                pointIndex = sagittalIndex;
                if( dcos[pointIndex][0] < 0 ) {
                    mirrorPlots = true;
                }         
                break;
            case svkDcmHeader::CORONAL:

                amplitudeIndex = axialIndex;
                if( dcos[amplitudeIndex][2] < 0 ) {
                    invertPlots = true;
                }         

                pointIndex = sagittalIndex;
                if( dcos[pointIndex][0] < 0 ) {
                    mirrorPlots = true;
                }         
                break;

            case svkDcmHeader::SAGITTAL:
                amplitudeIndex = axialIndex;
                if( dcos[amplitudeIndex][2] < 0 ) {
                    invertPlots = true;
                }         

                pointIndex = coronalIndex;
                if( dcos[pointIndex][1] < 0 ) {
                    mirrorPlots = true;
                }         
                break;
        }
         

        //string acquisitionType = data->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
        //if( acquisitionType == "SINGLE VOXEL" ) {
        //    mirrorPlots = false;
        //}

        for( vector<svkPlotLine*>::iterator iter = this->xyPlots.begin();
            iter != this->xyPlots.end(); ++iter) {
            tmpBoxPlot = static_cast<svkPlotLine*>( (*iter));
            tmpBoxPlot->SetPlotDirection( amplitudeIndex, pointIndex );
            tmpBoxPlot->SetMirrorPlots( mirrorPlots );
            tmpBoxPlot->GeneratePolyData();
            tmpBoxPlot->SetInvertPlots( invertPlots );
        }
        this->Update(tlcBrc);
    }
}


/*!
 *
 * @return
 */
vector<int> svkPlotLineGrid::GetVolumeIndexArray( )
{
    return this->volumeIndexVector;

}


/*!
 *
 * @param volumeIndex
 * @param index
 */
void svkPlotLineGrid::SetVolumeIndex( int index, int volumeIndex )
{
    this->volumeIndexVector[volumeIndex] = index;
    if( this->data != NULL ) {
        this->SetVolumeOutOfDate( volumeIndex );

        if( this->volumeIndexVector[volumeIndex] >= this->data->GetVolumeIndexSize( volumeIndex) ) {
            this->volumeIndexVector[volumeIndex] >= this->data->GetVolumeIndexSize( volumeIndex) - 1;
        } else if (this->volumeIndexVector[volumeIndex] < 0 ) {
            this->volumeIndexVector[volumeIndex] = 0;
        }

        int minIndex[3] = {0,0,0};
        int maxIndex[3] = { this->data->GetDimensions()[0]-2,
                            this->data->GetDimensions()[1]-2,
                            this->data->GetDimensions()[2]-2};

        int orientationIndex = this->data->GetOrientationIndex( this->orientation );

        minIndex[ orientationIndex ] = this->slice;
        maxIndex[ orientationIndex ] = this->slice;
        int minID = this->data->GetIDFromIndex( minIndex[0], minIndex[1], minIndex[2] );
        int maxID = this->data->GetIDFromIndex( maxIndex[0], maxIndex[1], maxIndex[2] );
        this->UpdateDataArrays( minID, maxID);
        this->SetSliceUpToDate( this->slice );
    }
}


/*!
 *
 * @param volumeIndex
 * @return
 */
int svkPlotLineGrid::GetVolumeIndex( int volumeIndex )
{
    return this->volumeIndexVector[ volumeIndex ];
}


/*!
 *  Updates arrays for each svkPlotLine in the given range.
 *
 *  \param tlc the index of the top left voxel
 *  \param brc the index of the bottom right voxel
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
                    tmpXYPlot = static_cast<svkPlotLine*>( xyPlots[ID] );
                    tmpXYPlot->SetData( vtkFloatArray::SafeDownCast(
                                         this->data->GetArray(rowIndex, columnIndex, sliceIndex, (int*)this->volumeIndexVector.data())));
                }
            }
            // Called modified on the points to trigger mapper update
            tmpXYPlot->polyLinePoints->Modified();
        }
    }
}


/*!
 *  Sets the orientation of the plots.
 *
 *  \param orientation orientation to be viewed
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

        if( !svkDataView::IsTlcBrcWithinData( this->data, this->tlcBrc ) ) {
            int lastSlice  = data->GetLastSlice( this->orientation );
            int firstSlice = data->GetFirstSlice( this->orientation );
            this->slice = (lastSlice-firstSlice)/2;
            this->SetSlice( this->slice );
            this->HighlightSelectionVoxels();
        }

        this->InitializeVolumeUpToDateVector();
        this->UpdateOrientation();
        this->UpdatePlotRange(this->tlcBrc);
    }
}


/*!
 *  Returns the plot grid actor
 *
 *  \return the plot grid actor
 */
vtkActor* svkPlotLineGrid::GetPlotGridActor()
{
    return this->plotGridActor;
}


/*!
 * Checks to see if the current volume for the given slice is up to date.
 * @param slice
 * @return
 */
bool svkPlotLineGrid::IsSliceUpToDate( int slice )
{
    bool upToDate = true;
    for( int i = 0; i < this->volumeIndexVector.size(); i ++ ) {
        upToDate = upToDate && this->volumeUpToDate[i][slice];
    }
    return upToDate;
}


/*!
 *
 * Marks the current slice us up to date.
 *
 * @param slice
 */
void svkPlotLineGrid::SetSliceUpToDate( int slice )
{
    for( int i = 0; i < this->volumeIndexVector.size(); i ++ ) {
        this->volumeUpToDate[i][slice] = true;
    }
}


/*!
 *
 * Marks the current slice as out of date.
 *
 * @param slice
 */
void svkPlotLineGrid::SetSliceOutOfDate( int slice )
{
    for( int i = 0; i < this->volumeIndexVector.size(); i ++ ) {
        this->volumeUpToDate[i][slice] = false;
    }
}

void svkPlotLineGrid::SetVolumeUpToDate( int volumeIndex )
{
    for( int i = 0; i < this->volumeUpToDate[volumeIndex].size(); i++ ) {
        this->volumeUpToDate[volumeIndex][i] = true;
    }

}

void svkPlotLineGrid::SetVolumeOutOfDate( int volumeIndex )
{
    for( int i = 0; i < this->volumeUpToDate[volumeIndex].size(); i++ ) {
        this->volumeUpToDate[volumeIndex][i] = false;
    }
}


/*!
 *  Initializes the vector that tracks whether or not a given
 *  slice for a given volume is currently up to date.
 */
void svkPlotLineGrid::InitializeVolumeUpToDateVector()
{
    this->volumeUpToDate.clear();
    while( this->volumeUpToDate.size() < this->data->GetNumberOfVolumeDimensions()) {
        vector<bool> sliceUpToDate;
        while( sliceUpToDate.size() < this->data->GetNumberOfSlices( this->orientation )) {
            sliceUpToDate.push_back( false );
        }
        this->volumeUpToDate.push_back( sliceUpToDate );
    }
}


/*!
 * Empties the vector containing the plot lines.
 */
void svkPlotLineGrid::ClearXYPlots()
{
    int count = 0;
    for( vector<svkPlotLine*>::iterator iter = this->xyPlots.begin();
        iter != this->xyPlots.end(); ++iter) {
        if( (*iter) != NULL ) {
            (*iter)->Delete();
            (*iter) = NULL;
        }
        count ++;
    }
    this->xyPlots.clear();
}


/*!
 * Empties the vector containing the poly data.
 */
void svkPlotLineGrid::ClearPolyData()
{
    int count = 0;
    for( vector<vtkPolyData*>::iterator iter = this->polyDataVector.begin();
        iter != this->polyDataVector.end(); ++iter) {
        if( (*iter) != NULL ) {
            (*iter)->Delete();
            (*iter) = NULL;
        }
        count ++;
    }
    this->polyDataVector.clear();
}
