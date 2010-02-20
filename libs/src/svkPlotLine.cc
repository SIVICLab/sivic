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


#include <svkPlotLine.h>


using namespace svk;


vtkCxxRevisionMacro(svkPlotLine, "$Rev$");
vtkStandardNewMacro(svkPlotLine);


/*!
 *  Constructor. This constructor creates new vtkObjects, and sets
 *  some instant variables.
 */
svkPlotLine::svkPlotLine()
{
    this->plotData = NULL;
    this->plotDataMagnitude = NULL;
    this->plotAreaBounds[0] = 0;
    this->plotAreaBounds[1] = 1;
    this->plotAreaBounds[2] = 0;
    this->plotAreaBounds[3] = 1;
    this->plotAreaBounds[4] = 0;
    this->plotAreaBounds[5] = 0;
    this->scale[0]  = 1;
    this->scale[1]  = 1;
    this->startPt   = 0;
    this->endPt     = 1;
    this->minValue  = 0; 
    this->maxValue  = 1;
    this->numPoints = 0;
    this->pointData = vtkFloatArray::New();
    this->polyLinePoints = NULL;
    this->plotComponent = REAL; 
    this->invertPlots = false;
    this->mirrorPlots = false;
    this->plotDirection = ROW_COLUMN;
    this->pointIndex = 0;
    this->amplitudeIndex =1;
    this->origin[0] = 0;
    this->origin[1] = 0;
    this->origin[2] = 0;
    this->offset = 0;
}


/*!
 *  Destructor.
 */
svkPlotLine::~svkPlotLine()
{
    if( this->plotData != NULL ) {
        this->plotData->Delete();
        this->plotData = NULL;
    }
    if( this->pointData != NULL ) {
        this->pointData->Delete();
        this->pointData = NULL;
    }
}


/*!
 *  Returns the bounds of the line.
 */
double* svkPlotLine::GetBounds() 
{ 
    return this->plotAreaBounds; 
}


/*!
 *  Set the component
 */
void svkPlotLine::SetComponent( PlotComponent component )
{
    this->plotComponent = component;
    if (this->plotComponent == REAL || this->plotComponent == IMAGINARY) {
        dataPtr = this->plotData->GetPointer(0);
        this->componentOffset = this->plotComponent;
        numComponents = 2;
    } else {
        componentOffset = 0;
        dataPtr = this->plotDataMagnitude->GetPointer(0);
        numComponents = 1;
    }
}


/*!
 * Set the Data Array to be plotted.
 *
 * \param plotData a vtkFloatArray that contains the data to be plotted
 */
void svkPlotLine::SetData( vtkFloatArray* plotData )
{
    if( this->plotData != NULL ) {
        this->plotData->Delete();
    }
    if( this->plotDataMagnitude != NULL ) {
        this->plotDataMagnitude->Delete();
    }

    this->plotData = plotData; 
    this->plotData->Register( this );

    this->plotDataMagnitude = vtkFloatArray::New();
    this->plotDataMagnitude->SetNumberOfComponents(1);
    this->plotDataMagnitude->SetNumberOfTuples( this->plotData->GetNumberOfTuples() );
    
    // We can now setup the polyLine and polyData

    // Get the number of points in the incoming data
    if( this->numPoints != this->plotData->GetNumberOfTuples()) {
        this->numPoints = this->plotData->GetNumberOfTuples();
    }

    for( int i = 0; i < this->numPoints; i++ ) {
        this->GetPointIds()->SetId(i,i+offset );
    }

    if (this->plotComponent == REAL || this->plotComponent == IMAGINARY) {
        dataPtr = this->plotData->GetPointer(0);
        this->componentOffset = this->plotComponent;
        numComponents = 2;
    } else {
        componentOffset = 0;
        dataPtr = this->plotDataMagnitude->GetPointer(0);
        numComponents = 1;
    }
    // Lets precompute the magnitudes...
    for( int i = 0; i < numPoints; i++ ) {
        double amplitude = pow( (double)((plotData->GetTuple(i))[0] * (plotData->GetTuple(i))[0] +
                (plotData->GetTuple(i))[1] * (plotData->GetTuple(i))[1]), 0.5);
        this->plotDataMagnitude->SetValue(i, amplitude);

    }

    // Generate poly data is where the data is sync'd
    this->GeneratePolyData();
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
void svkPlotLine::SetPointRange( int startPt, int endPt )
{
    this->startPt = startPt;
    this->endPt   = endPt; 
    this->RecalculateScale();
    this->GeneratePolyData();
}


/*!
 * Set the value (Y) range of the data set to be plotted.
 *
 * \param minValue the minimum value you wish to plot
 *
 * \param maxValue the maximum value you wish to plot 
 *
 */
void svkPlotLine::SetValueRange( double minValue, double maxValue )
{
    this->minValue = minValue;
    this->maxValue = maxValue;
    this->RecalculateScale();
    this->GeneratePolyData();
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
void svkPlotLine::GeneratePolyData()
{
    // delta is change in the row, column, and slice direction
    double delta[3];
    double amplitude;
    float posLPS[3];

    delta[0] = this->spacing[0]/2.0;
    delta[1] = this->spacing[1]/2.0;
    delta[2] = this->spacing[2]/2.0;

    // We are going to precalculate the start + the distance in the slice dimension for speed
    if( this->plotData != NULL && this->polyLinePoints != NULL ) {


        // First we will set all of the points before start Pt to the same position.

        amplitude = dataPtr[numComponents*(this->startPt-1) + componentOffset];
        // Often negative is up in LPS, so if this is true we invert 
        if( this->invertPlots ) {
            delta[amplitudeIndex] = (this->maxValue - amplitude)*this->scale[1];
        } else {
            delta[amplitudeIndex] = (amplitude - this->minValue)*this->scale[1];
        }
        if( this->mirrorPlots ) {
            delta[ pointIndex ] = (this->endPt - this->startPt) * this->scale[0]; 
        } else {
            delta[ pointIndex ] = 0; 
        }
        posLPS[0] = this->origin[0] + (delta[0]) * dcos[0][0] + (delta[1]) * dcos[1][0] + (delta[2]) * dcos[2][0];
        posLPS[1] = this->origin[1] + (delta[0]) * dcos[0][1] + (delta[1]) * dcos[1][1] + (delta[2]) * dcos[2][1];
        posLPS[2] = this->origin[2] + (delta[0]) * dcos[0][2] + (delta[1]) * dcos[1][2] + (delta[2]) * dcos[2][2];

        // All points before the start get moved to the same position
        for( int i = 0; i < this->startPt; i++ ) {
            this->polyLinePoints->SetPoint(i+offset, posLPS );
        }

        // Now for visible Points
        for( int i = this->startPt; i <= this->endPt; i++ ) {

            // Which component are we using...
            amplitude = dataPtr[numComponents*i + componentOffset];

            // If the value is outside the max/min
            if( amplitude > this->maxValue ) {
                amplitude = this->maxValue;
            } else if ( amplitude < this->minValue ) {
                amplitude = this->minValue;
            }

            // Often negative is up in LPS, so if this is true we invert 
            if( this->invertPlots ) {
                delta[amplitudeIndex] = (this->maxValue - amplitude)*this->scale[1];
            } else {
                delta[amplitudeIndex] = (amplitude - this->minValue)*this->scale[1];
            }


            if( this->mirrorPlots ) {
                delta[ pointIndex ] = (this->endPt - i) * this->scale[0]; 
            } else {
                delta[ pointIndex ] = (i - this->startPt) * this->scale[0]; 
            }

            // NOTE: This could be moved into the above if blocks for speed
            posLPS[0] = this->origin[0] + (delta[0]) * dcos[0][0] + (delta[1]) * dcos[1][0] + (delta[2]) * dcos[2][0];
            posLPS[1] = this->origin[1] + (delta[0]) * dcos[0][1] + (delta[1]) * dcos[1][1] + (delta[2]) * dcos[2][1];
            posLPS[2] = this->origin[2] + (delta[0]) * dcos[0][2] + (delta[1]) * dcos[1][2] + (delta[2]) * dcos[2][2];

            this->polyLinePoints->SetPoint(i+offset, posLPS);
        }

        // And finally we set all points outside the range to the last value
        amplitude = dataPtr[numComponents*(this->endPt+1) + componentOffset];

        // If the value is outside the max/min
        if( amplitude > this->maxValue ) {
            amplitude = this->maxValue;
        } else if ( amplitude < this->minValue ) {
            amplitude = this->minValue;
        }

        // Often negative is up in LPS, so if this is true we invert 
        if( this->invertPlots ) {
            delta[amplitudeIndex] = (this->maxValue - amplitude)*this->scale[1];
        } else {
            delta[amplitudeIndex] = (amplitude - this->minValue)*this->scale[1];
        }

        // If the point is outside the start/end
        if( this->mirrorPlots ) {
            delta[ pointIndex ] = 0; 
        } else {
            delta[ pointIndex ] = (this->endPt - this->startPt) * this->scale[0]; 
        }
        // NOTE: This could be moved into the above if blocks for speed
        posLPS[0] = this->origin[0] + (delta[0]) * dcos[0][0] + (delta[1]) * dcos[1][0] + (delta[2]) * dcos[2][0];
        posLPS[1] = this->origin[1] + (delta[0]) * dcos[0][1] + (delta[1]) * dcos[1][1] + (delta[2]) * dcos[2][1];
        posLPS[2] = this->origin[2] + (delta[0]) * dcos[0][2] + (delta[1]) * dcos[1][2] + (delta[2]) * dcos[2][2];
        for( int i = this->endPt+1; i < numPoints; i++ ) {
            this->polyLinePoints->SetPoint(i+offset, posLPS );
        }
        polyLinePoints->Modified(); 
    }
}


/*!
 *  Get the vtkPoints object used by this class
 */
vtkPoints* svkPlotLine::GetDataPoints() 
{
    return this->polyLinePoints;
}


/*!
 *  Set the vtkPoints object that will be used by this class
 */
void svkPlotLine::SetDataPoints(vtkPoints* polyLinePoints) 
{
    this->polyLinePoints = polyLinePoints;
}


/*!
 *  Gets the origin, 
 */
double* svkPlotLine::GetOrigin()
{
    return this->origin;
}


/*!
 *  Sets the origin
 */
void svkPlotLine::SetOrigin( double* origin )
{
    memcpy( this->origin, origin, sizeof(double) * 3 );
    this->RecalculatePlotAreaBounds();
}


/*!
 *  Get Spacing
 */
double* svkPlotLine::GetSpacing()
{
    return this->spacing;
}


/*!
 *  Set Spacing
 */
void svkPlotLine::SetSpacing( double* spacing )
{
    memcpy( this->spacing, spacing, sizeof(double) * 3 );
    this->RecalculateScale();
    this->RecalculatePlotAreaBounds();
}


/*!
 *  Get Dcos
 */
void svkPlotLine::GetDcos( double dcos[][3] )
{
    memcpy( dcos, this->dcos, sizeof(double) * 9 );
    this->RecalculatePlotAreaBounds();
}


/*!
 *  Set Dcos
 */
void svkPlotLine::SetDcos( double dcos[][3] )
{
    memcpy( this->dcos, dcos, sizeof(double) * 9 );
}


/*!
 *  Recalculates the scale based on the spacing and the ranges
 */
void svkPlotLine::RecalculateScale()
{
    this->scale[0] = ( spacing[this->pointIndex] )/((float)( this->endPt - this->startPt ));
    this->scale[1] = ( spacing[this->amplitudeIndex] )/((float)( this->maxValue - this->minValue ));
}


/*!
 *  Recalculates the bounds of the the object
 */
void svkPlotLine::RecalculatePlotAreaBounds()
{
    double xMax = origin[0] + spacing[0] * dcos[0][0]   
                            + spacing[1] * dcos[1][0]   
                            + spacing[2] * dcos[2][0];
    double xMin;
    if( xMax > origin[0] ) {
        xMin = origin[0];
    } else {
        xMin = xMax;
        xMax = origin[0];
    }
    double yMax = origin[1] + spacing[0] * dcos[0][1]   
                            + spacing[1] * dcos[1][1]   
                            + spacing[2] * dcos[2][1];
    double yMin;
    if( yMax > origin[1] ) {
        yMin = origin[1];
    } else {
        yMin = yMax;
        yMax = origin[1];
    }
    double zMax = origin[2] + spacing[0] * dcos[0][2]  
                            + spacing[1] * dcos[1][2]   
                            + spacing[2] * dcos[2][2];
    double zMin;
    if( zMax > origin[2] ) {
        zMin = origin[2];
    } else {
        zMin = zMax;
        zMax = origin[2];
    }

    this->plotAreaBounds[0] = xMin;
    this->plotAreaBounds[1] = xMax;
    this->plotAreaBounds[2] = yMin;
    this->plotAreaBounds[3] = yMax;
    this->plotAreaBounds[4] = zMin;
    this->plotAreaBounds[5] = zMax;
}


/*!
 *
 */
void svkPlotLine::SetInvertPlots( bool invertPlots ) 
{
    this->invertPlots = invertPlots;
}


/*!
 *
 */
void svkPlotLine::SetMirrorPlots( bool mirrorPlots ) 
{
    this->mirrorPlots = mirrorPlots;
}


/*!
 *
 */
void svkPlotLine::SetPlotDirection( PlotDirection plotDirection  ) 
{
    this->plotDirection = plotDirection;
    switch( this->plotDirection ) {
        case ROW_COLUMN:
            this->pointIndex     = 0;
            this->amplitudeIndex = 1;
            break;
        case COLUMN_ROW:
            this->pointIndex     = 1;
            this->amplitudeIndex = 0;
            break;
        case ROW_SLICE:
            this->pointIndex     = 0;
            this->amplitudeIndex = 2;
            break;
        case SLICE_ROW:
            this->pointIndex     = 2;
            this->amplitudeIndex = 0;
            break;
        case SLICE_COLUMN:
            this->pointIndex     = 2;
            this->amplitudeIndex = 1;
            break;
        case COLUMN_SLICE:
            this->pointIndex     = 1;
            this->amplitudeIndex = 2;
            break;
    }
    this->RecalculateScale();
}


/*!
 *
 */
void svkPlotLine::SetOffset( int offset  ) 
{
    this->offset = offset;
}
