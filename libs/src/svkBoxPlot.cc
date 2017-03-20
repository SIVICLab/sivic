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
 *
 *  NOTE: This class is deprecated and no longer in use.
 */


#include <svkBoxPlot.h>


using namespace svk;


//vtkCxxRevisionMacro(svkBoxPlot, "$Rev$");
vtkStandardNewMacro(svkBoxPlot);


/*!
 *  Constructor. This constructor creates new vtkObjects, and sets
 *  some instant variables.
 */
svkBoxPlot::svkBoxPlot()
{
    this->plotData = NULL;
    this->Mapper = vtkPolyDataMapper::New();
    this->Mapper->GlobalImmediateModeRenderingOn();
    this->Mapper->ReleaseDataFlagOn();
    this->plotAreaBounds = new double[6];
    this->plotAreaBounds[0] = 0;
    this->plotAreaBounds[1] = 1;
    this->plotAreaBounds[2] = 0;
    this->plotAreaBounds[3] = 1;
    this->plotAreaBounds[4] = 0;
    this->plotAreaBounds[5] = 0;
    this->startPt   = 0;
    this->endPt     = 1;
    this->minValue  = 0; 
    this->maxValue  = 1;
    this->numPoints = 0;
    this->pointData = vtkFloatArray::New();
    this->polyLinePoints = NULL;
    this->plotComponent = REAL; 
}


/*!
 *  Destructor.
 */
svkBoxPlot::~svkBoxPlot()
{

    if( this->plotData != NULL ) {
        this->plotData->Delete();
        this->plotData = NULL;
    }
    if( this->polyLinePoints != NULL ) {
        this->polyLinePoints->Delete();
        this->polyLinePoints = NULL;
    }
    if( this->pointData != NULL ) {
        this->pointData->Delete();
        this->pointData = NULL;
    }
    delete[] plotAreaBounds;
}


/*!
 *  Initialize a default object. This can be used as a test, it fully generates a
 *  functional svkBoxPlot. All elements have value zero.
 */
void svkBoxPlot::Initialize()
{ 
    // Create default object...
    double* defaultBounds = new double[6];
    vtkFloatArray* defaultPlotData = vtkFloatArray::New();
    this->numPoints = 500;
    defaultPlotData->SetNumberOfValues( numPoints );
    
    // Create 0 value array
    for( int i = 0; i < defaultPlotData->GetSize(); i++ ) {
        defaultPlotData->SetValue(i,i);
    }
    
    SetData( defaultPlotData );
    
    // Create default plotAreaBounds
    defaultBounds[0] = 0;
    defaultBounds[1] = 500;
    defaultBounds[2] = 0;
    defaultBounds[3] = 500;
    defaultBounds[4] = 0;
    defaultBounds[5] = 1;

    SetPlotAreaBounds( defaultBounds );
   
    // Create default plot parameters
    SetPointRange( 0, 500 );
    SetValueRange( 0, 500 );
    
    // Extract array range for scaling purposes
    GeneratePolyData();
    GenerateActor();
    delete[] defaultBounds;
    defaultPlotData->Delete();
}


/*!
 *  Returns the bounds of the actor.
 */
double* svkBoxPlot::GetBounds() 
{ 
    return Bounds; 
}


/*!
 *
 */
void svkBoxPlot::SetComponent( PlotComponent component )
{
    this->plotComponent = component;
}


/*!
 * Set the Data Array to be plotted.
 *
 * \param plotData a vtkFloatArray that contains the data to be plotted
 */
void svkBoxPlot::SetData( vtkFloatArray* plotData )
{
    if( this->plotData != NULL ) {
        this->plotData->Delete();
    }
    this->plotData = plotData; 
    this->plotData->Register( this );
    
    // We can now setup the polyLine and polyData

    // Get the number of points in the incoming data
    if( this->numPoints != this->plotData->GetNumberOfTuples()) {
        this->numPoints = this->plotData->GetNumberOfTuples();
        
        // Create a data object to hold the x-y-z points
        this->pointData->Initialize();

        this->pointData->SetNumberOfComponents(3); // 3 for ndim
        this->pointData->SetNumberOfTuples(numPoints+4); // four extra ponits for corners

        // Now we will need a polyData object to put the points in
        vtkPolyData* mypolyLineData=vtkPolyData::New();
        mypolyLineData->Initialize();
        mypolyLineData->Allocate(1,1);

        // And a point object so the data can be recognized as coordinates
        if( this->polyLinePoints != NULL ) {
            this->polyLinePoints->Delete();
        }

        this->polyLinePoints = vtkPoints::New();
        this->polyLinePoints->Initialize();
        this->polyLinePoints->SetDataType( plotData->GetDataType() );
        this->polyLinePoints->SetNumberOfPoints(numPoints);

        vtkPolyLine* aBoundryLine = vtkPolyLine::New();
        vtkIdList* boundryLineIds = aBoundryLine->GetPointIds();
        boundryLineIds->SetNumberOfIds( 5 );

        vtkPolyLine* aPolyLine = vtkPolyLine::New();
        vtkIdList* polyLineIds = aPolyLine->GetPointIds();
        polyLineIds->SetNumberOfIds(numPoints);

        float* tuple = new float[3];

        // We must allocate all tuples, so that we can set them later
        for( int i = 0; i < numPoints; i++ ) {
            tuple[0] = i;
            tuple[1] = 0; 
            tuple[2] = 0; 
            polyLineIds->SetId(i,i );
            this->pointData->SetTuple(i, tuple); 
        }

        // Now lets add the border around our plot with another line...
        boundryLineIds->SetId(0,numPoints+0 );
        boundryLineIds->SetId(1,numPoints+1 );
        boundryLineIds->SetId(2,numPoints+2 );
        boundryLineIds->SetId(3,numPoints+3 );
        boundryLineIds->SetId(4,numPoints+0 );
        this->pointData->SetTuple3(numPoints+0, startPt, minValue, 0); 
        this->pointData->SetTuple3(numPoints+1, startPt, maxValue, 0); 
        this->pointData->SetTuple3(numPoints+2, endPt  , maxValue, 0); 
        this->pointData->SetTuple3(numPoints+3, endPt  , minValue, 0); 

        this->polyLinePoints->SetData( this->pointData ); 
        mypolyLineData->SetPoints(polyLinePoints);

        mypolyLineData->InsertNextCell(aPolyLine->GetCellType(), aPolyLine->GetPointIds());
        mypolyLineData->InsertNextCell(aBoundryLine->GetCellType(), aBoundryLine->GetPointIds());

        vtkPolyDataMapper::SafeDownCast(Mapper)->SetInput(mypolyLineData);

        mypolyLineData->Delete();
        aPolyLine->Delete();
        aBoundryLine->Delete();
        delete[] tuple;

        // Generate poly data is where the data is sync'd
        this->GeneratePolyData();
        this->GenerateActor();
    } else {
        this->GeneratePolyData();
    }
}


/*!
 *  Sets the PHYSICAL BOUNDS of the plot, NOT the range of the data.
 *  Use SetPointRange, and SetValueRange for that! Said in another way
 *  this is the space the plot will fill.
 *
 *  \param plotAreaBounds the boundries in which you want the plot to be located [xmin, xmax, ymin, ymax, zmin, zmax]
 *
 */
void svkBoxPlot::SetPlotAreaBounds( double* plotAreaBounds)
{
    this->Bounds[0] = plotAreaBounds[0];
    this->Bounds[1] = plotAreaBounds[1];
    this->Bounds[2] = plotAreaBounds[2];
    this->Bounds[3] = plotAreaBounds[3];
    this->Bounds[4] = plotAreaBounds[4];
    this->Bounds[5] = plotAreaBounds[5];
    if( this->plotData != NULL ) {
        memcpy( this->plotAreaBounds, plotAreaBounds, sizeof(double) * 6 );
        this->TransformActor();
        this->GenerateClippingPlanes();
    } else {
        cerr<<"INPUT HAS NOT BEEN SET!!"<<endl;
    }
}


/*!
 * Set the start point and end point of the array to be plotted.
 * This essentially sets the x-range.
 *
 * \param startPt the index of the first point to plot in the data array
 *
 * \param endPt the index of the last point to plot in the data array
 *
 */
void svkBoxPlot::SetPointRange( int startPt, int endPt )
{
    if( this->plotData != NULL && ( startPt != this->startPt || endPt != this->endPt ) ) {
        this->startPt = startPt;
        this->endPt   = endPt; 
        this->pointData->SetTuple3(numPoints+0, startPt, -minValue, 0); 
        this->pointData->SetTuple3(numPoints+1, startPt, -maxValue, 0); 
        this->pointData->SetTuple3(numPoints+2, endPt  , -maxValue, 0); 
        this->pointData->SetTuple3(numPoints+3, endPt  , -minValue, 0); 
        this->polyLinePoints->Modified();
        this->TransformActor();
    } else if( this->plotData == NULL ) {
        cerr<<"INPUT HAS NOT BEEN SET!!"<<endl;
    }
}


/*!
 * Set the value (Y) range of the data set to be plotted.
 *
 * \param minValue the minimum value you wish to plot
 *
 * \param maxValue the maximum value you wish to plot 
 *
 */
void svkBoxPlot::SetValueRange( double minValue, double maxValue )
{
    // Since the data is inverted (for camera location), we need to invert the plots...
    if( this->plotData != NULL && ((-maxValue) != this->minValue || (-minValue) != this->maxValue ) ) {
        this->minValue = -maxValue;
        this->maxValue = -minValue;
        this->pointData->SetTuple3(this->numPoints+0, this->startPt, minValue, 0); 
        this->pointData->SetTuple3(this->numPoints+1, this->startPt, maxValue, 0); 
        this->pointData->SetTuple3(this->numPoints+2, this->endPt  , maxValue, 0); 
        this->pointData->SetTuple3(this->numPoints+3, this->endPt  , minValue, 0); 
        this->polyLinePoints->Modified();
        this->TransformActor();
    } else if( this->plotData == NULL ){
        cerr<<"INPUT HAS NOT BEEN SET!!"<<endl;
    }
        
}


/*!
 * Generates the clipping planes for the mMMapper. This is how the boundries
 * set are enforced, after the data is scaled, it is clipped so that data
 * outside the plot range is simply not shown.
 */
void svkBoxPlot::GenerateClippingPlanes()
{
    // We need to leave a little room around the edges, so the border does not get cut off
    float borderWidth = 0.01;
    if( plotData != NULL ) {
        vtkPlane* clipperPlane0 = vtkPlane::New();
        vtkPlane* clipperPlane1 = vtkPlane::New();
        vtkPlane* clipperPlane2 = vtkPlane::New();
        vtkPlane* clipperPlane3 = vtkPlane::New();
        clipperPlane0->SetNormal(0,1,0);
        clipperPlane0->SetOrigin( plotAreaBounds[0]-borderWidth, 
                                  plotAreaBounds[2]-borderWidth, 0 );
        clipperPlane1->SetNormal(0,-1,0);
        clipperPlane1->SetOrigin( plotAreaBounds[1]+borderWidth, 
                                  plotAreaBounds[3]+borderWidth, 0 );
        clipperPlane2->SetNormal(1,0,0);
        clipperPlane2->SetOrigin( plotAreaBounds[0]-borderWidth, 
                                  plotAreaBounds[2]-borderWidth, 0 );
        clipperPlane3->SetNormal(-1,0,0);
        clipperPlane3->SetOrigin( plotAreaBounds[1]+borderWidth, plotAreaBounds[3]+borderWidth, 0 );
        Mapper->RemoveAllClippingPlanes();
        Mapper->AddClippingPlane( clipperPlane0 ); 
        Mapper->AddClippingPlane( clipperPlane1 ); 
        Mapper->AddClippingPlane( clipperPlane2 ); 
        Mapper->AddClippingPlane( clipperPlane3 ); 
        clipperPlane0->Delete();
        clipperPlane1->Delete();
        clipperPlane2->Delete();
        clipperPlane3->Delete();
    } else {
        cerr<<"INPUT HAS NOT BEEN SET!!"<<endl;
    }
}


/*!
 * Transforms the actor so that the visible portion fills the given plotAreaBounds.
 */
void svkBoxPlot::TransformActor()
{
     
    if( plotData != NULL ) {
        double xScale = ( plotAreaBounds[1] - plotAreaBounds[0]) / ( endPt - startPt );
        double yScale = ( plotAreaBounds[3] - plotAreaBounds[2] ) /( maxValue - minValue );
        SetPosition( (plotAreaBounds[0] - xScale*startPt), plotAreaBounds[2] - yScale*minValue, plotAreaBounds[4] + ( plotAreaBounds[5] - plotAreaBounds[4] ) / 2.0);    
        SetScale( xScale, -yScale, 1 );
    } else {
        cerr<<"INPUT HAS NOT BEEN SET!!"<<endl;
    }
}


/*!
 * Generates the poly data object that is used to represent the data.
 * This method will copy the current plotData into the pointData, then
 * call modified on the polyLine object so that it nows to sync with
 * the new data. 
 *
 * TODO: Find a way to Not Copy the new data, and just change a data
 *       reference.
 */
void svkBoxPlot::GeneratePolyData()
{
    if( plotData != NULL ) {
        if (this->plotComponent == REAL || this->plotComponent == IMAGINARY) {
            for( int i = 0; i < numPoints; i++ ) {
                //  Plot Real or Imaginary component of data: 
                pointData->SetComponent(i, 1, (plotData->GetTuple(i))[this->plotComponent] );
            }
        } else {
            float mag; 
            for( int i = 0; i < numPoints; i++ ) {
                //  Plot Real or Imaginary component of data: 
                mag =  (plotData->GetTuple(i))[0] * (plotData->GetTuple(i))[0] + 
                    (plotData->GetTuple(i))[1] * (plotData->GetTuple(i))[1] ;
                mag = pow((double)mag, .5);    
                pointData->SetComponent(i, 1, mag);
            }
        }
        polyLinePoints->Modified(); 
    } else {
        cerr<<"INPUT HAS NOT BEEN SET!!"<<endl;
    }
}

/*!
 * Generates the internal actor to display the data.
 */
void svkBoxPlot::GenerateActor()
{

    if( plotData != NULL ) {
        //this->GetProperty()->SetDiffuseColor(1, 1, 1);
        this->GetProperty()->SetAmbientColor(1, 1, 1);
        this->GetProperty()->SetAmbient(1);
        this->GetProperty()->SetDiffuse(0);
        this->GetProperty()->SetSpecular(0);
        TransformActor();
        GenerateClippingPlanes();
        this->Modified();
    } else {
        cerr<<"INPUT HAS NOT BEEN SET!!"<<endl;
    }
 
}
