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

#include <cmath>
#include <svkPlotLine.h>


using namespace svk;


//vtkCxxRevisionMacro(svkPlotLine, "$Rev$");
vtkStandardNewMacro(svkPlotLine);


/*!
 *  Constructor. This constructor creates new vtkObjects, and sets
 *  some instant variables.
 */
svkPlotLine::svkPlotLine()
{
    this->plotData = NULL;
    this->dataType = -1;
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
    this->spacing[0] = 0;
    this->spacing[1] = 0;
    this->spacing[2] = 0;
    this->numComponents = 0;
    this->generatePolyData = true;
    this->dcos = NULL;
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
	if( this->plotComponent != component ) {
		this->plotComponent = component;
		if (this->plotComponent == REAL || this->plotComponent == IMAGINARY) {
			this->componentOffset = this->plotComponent;
		}
		this->Modified();
	}
}


/*!
 * Set the Data Array to be plotted.
 *
 * \param plotData a vtkFloatArray that contains the data to be plotted
 */
void svkPlotLine::SetData( vtkDataArray* plotData )
{
	if( plotData != NULL && plotData != this->plotData ) {
		if( this->plotData != NULL ) {
			this->plotData->Delete();
		}

		this->plotData = plotData;
		this->dataType = this->plotData->GetDataType();
		this->plotData->Register( this );

		// We can now setup the polyLine and polyData

		// Get the number of points in the incoming data
		if( this->numPoints != this->plotData->GetNumberOfTuples()) {
			this->numPoints = this->plotData->GetNumberOfTuples();
		}


		this->dataPtr = this->plotData->GetVoidPointer(0);
		if (this->plotComponent == REAL || this->plotComponent == IMAGINARY) {
			this->componentOffset = this->plotComponent;
		}
		this->numComponents = this->plotData->GetNumberOfComponents();
		this->Modified();
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
void svkPlotLine::SetPointRange( int startPt, int endPt )
{
    if( this->endPt != endPt || this->startPt != startPt ) {
		this->startPt = startPt;
		this->endPt   = endPt;
		if( this->endPt > numPoints -1 ) {
			this->endPt = numPoints -1;
		}
		if( this->startPt < 0 ) {
			this->startPt = 0;
		}
		this->RecalculateScale();
		this->Modified();
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
void svkPlotLine::SetValueRange( double minValue, double maxValue )
{
    if( this->maxValue != maxValue || this->minValue != minValue ) {
		this->minValue = minValue;
		this->maxValue = maxValue;
		this->RecalculateScale();
		this->Modified();
	}
}


/*!
 * The data has been modified so lets update the polyData.
 */
void svkPlotLine::Modified()
{
	if( this->generatePolyData ) {
		this->GeneratePolyData();
	}
	this->Superclass::Modified();
}


/*!
 * Modifies the input point data array that represent the data.
 * This method will transform the current plotData into the pointData array.
 *
 */
template <class T>
void svkPlotLine::GeneratePolyDataTemplated( T* castDataPtr )
{

    // We are going to precalculate the start + the distance in the slice dimension for speed
    if( this->plotData != NULL && this->polyLinePoints != NULL ) {

		// delta is change in the row, column, and slice direction
		double delta[3];
		double amplitude;
		float posLPS[3];

		delta[0] = this->spacing[0]/2.0;
		delta[1] = this->spacing[1]/2.0;
		delta[2] = this->spacing[2]/2.0;


        // First we will set all of the points before start Pt to the same position.
        if( this->plotComponent == svkPlotLine::REAL || this->plotComponent == svkPlotLine::IMAGINARY ) {
            amplitude = castDataPtr[this->numComponents*(this->startPt) + this->componentOffset];
        } else {
            amplitude = pow( static_cast<double>(castDataPtr[this->numComponents*(this->startPt)] * castDataPtr[this->numComponents*(this->startPt)] +
                             castDataPtr[this->numComponents*(this->startPt) + 1] * castDataPtr[this->numComponents*(this->startPt) + 1]), 0.5);
        }

        if( amplitude > this->maxValue ) {
            amplitude = this->maxValue;
        } else if ( amplitude < this->minValue ) {
            amplitude = this->minValue;
        }
        // Often negative is up in LPS, so if this is true we invert
        if( this->invertPlots ) {
            delta[this->amplitudeIndex] = (this->maxValue - amplitude)*this->scale[1];
        } else {
            delta[this->amplitudeIndex] = (amplitude - this->minValue)*this->scale[1];
        }
        if( this->mirrorPlots ) {
            delta[ this->pointIndex ] = (this->endPt - this->startPt) * this->scale[0];
        } else {
            delta[ this->pointIndex ] = 0;
        }
        posLPS[0] = this->origin[0] + (delta[0]) * (*this->dcos)[0][0] + (delta[1]) * (*this->dcos)[1][0] + (delta[2]) * (*this->dcos)[2][0];
        posLPS[1] = this->origin[1] + (delta[0]) * (*this->dcos)[0][1] + (delta[1]) * (*this->dcos)[1][1] + (delta[2]) * (*this->dcos)[2][1];
        posLPS[2] = this->origin[2] + (delta[0]) * (*this->dcos)[0][2] + (delta[1]) * (*this->dcos)[1][2] + (delta[2]) * (*this->dcos)[2][2];

        // All points before the start get moved to the same position
        for( int i = 0; i < this->startPt; i++ ) {
            this->polyLinePoints[ i*3 ] = posLPS[0];
            this->polyLinePoints[i*3+1] = posLPS[1];
            this->polyLinePoints[i*3+2] = posLPS[2];
        }

        // Now for visible Points
        for( int i = this->startPt; i <= this->endPt; i++ ) {

            // Which component are we using...
            if( this->plotComponent == svkPlotLine::REAL || this->plotComponent == svkPlotLine::IMAGINARY ) {
                amplitude = castDataPtr[this->numComponents*(i) + this->componentOffset];
            } else {
                amplitude = pow( static_cast<double>(castDataPtr[this->numComponents*(i)] * castDataPtr[this->numComponents*(i)] +
                                 castDataPtr[this->numComponents*(i) + 1] * castDataPtr[this->numComponents*(i) + 1]), 0.5 );
            }

            // If the value is outside the max/min
            if( amplitude > this->maxValue ) {
                amplitude = this->maxValue;
            } else if ( amplitude < this->minValue ) {
                amplitude = this->minValue;
            }

            // Often negative is up in LPS, so if this is true we invert 
            if( this->invertPlots ) {
                delta[this->amplitudeIndex] = (this->maxValue - amplitude)*this->scale[1];
            } else {
                delta[this->amplitudeIndex] = (amplitude - this->minValue)*this->scale[1];
            }


            if( this->mirrorPlots ) {
                delta[ this->pointIndex ] = (this->endPt - i) * this->scale[0];
            } else {
                delta[ this->pointIndex ] = (i - this->startPt) * this->scale[0];
            }

            // NOTE: This could be moved into the above if blocks for speed
            posLPS[0] = this->origin[0] + (delta[0]) * (*this->dcos)[0][0] + (delta[1]) * (*this->dcos)[1][0] + (delta[2]) * (*this->dcos)[2][0];
            posLPS[1] = this->origin[1] + (delta[0]) * (*this->dcos)[0][1] + (delta[1]) * (*this->dcos)[1][1] + (delta[2]) * (*this->dcos)[2][1];
            posLPS[2] = this->origin[2] + (delta[0]) * (*this->dcos)[0][2] + (delta[1]) * (*this->dcos)[1][2] + (delta[2]) * (*this->dcos)[2][2];

            this->polyLinePoints[ i*3 ]   = posLPS[0];
            this->polyLinePoints[i*3 + 1] = posLPS[1];
            this->polyLinePoints[i*3 + 2] = posLPS[2];
        }

        // And finally we set all points outside the range to the last value
        if( this->plotComponent == svkPlotLine::REAL || this->plotComponent == svkPlotLine::IMAGINARY ) {
            amplitude = castDataPtr[this->numComponents*(this->endPt) + this->componentOffset];
        } else {
            amplitude = pow( static_cast<double>(castDataPtr[this->numComponents*(this->endPt)] * castDataPtr[this->numComponents*(this->endPt)] +
                             castDataPtr[this->numComponents*(this->endPt) + 1] * castDataPtr[this->numComponents*(this->endPt) + 1]), 0.5);
        }

        // If the value is outside the max/min
        if( amplitude > this->maxValue ) {
            amplitude = this->maxValue;
        } else if ( amplitude < this->minValue ) {
            amplitude = this->minValue;
        }

        // Often negative is up in LPS, so if this is true we invert 
        if( this->invertPlots ) {
            delta[this->amplitudeIndex] = (this->maxValue - amplitude)*this->scale[1];
        } else {
            delta[this->amplitudeIndex] = (amplitude - this->minValue)*this->scale[1];
        }

        // If the point is outside the start/end
        if( this->mirrorPlots ) {
            delta[ this->pointIndex ] = 0;
        } else {
            delta[ this->pointIndex ] = (this->endPt - this->startPt) * this->scale[0];
        }
        // NOTE: This could be moved into the above if blocks for speed
        posLPS[0] = this->origin[0] + (delta[0]) * (*this->dcos)[0][0] + (delta[1]) * (*this->dcos)[1][0] + (delta[2]) * (*this->dcos)[2][0];
        posLPS[1] = this->origin[1] + (delta[0]) * (*this->dcos)[0][1] + (delta[1]) * (*this->dcos)[1][1] + (delta[2]) * (*this->dcos)[2][1];
        posLPS[2] = this->origin[2] + (delta[0]) * (*this->dcos)[0][2] + (delta[1]) * (*this->dcos)[1][2] + (delta[2]) * (*this->dcos)[2][2];
        for( int i = this->endPt+1; i < this->numPoints; i++ ) {
            this->polyLinePoints[ i*3 ] = posLPS[0];
            this->polyLinePoints[i*3+1] = posLPS[1];
            this->polyLinePoints[i*3+2] = posLPS[2];
        }
    }
}


/*!
 * Calls template method.
 */
void svkPlotLine::GeneratePolyData()
{
  // choose which templated function to call.

	switch (this->dataType) {
		vtkTemplateMacro( this->GeneratePolyDataTemplated( static_cast<VTK_TT *>(this->dataPtr) ));
	default:
		vtkErrorMacro(<< "Execute: Unknown ScalarType");
		return;
	}
}

/*!
 *  Get the vtkPoints object used by this class
 */
float* svkPlotLine::GetDataPoints()
{
    return this->polyLinePoints;
}


/*!
 *  Set the vtkPoints object that will be used by this class
 */
void svkPlotLine::SetDataPoints(float* polyLinePoints)
{
	if( this->polyLinePoints != polyLinePoints ) {
		this->polyLinePoints = polyLinePoints;
		this->Modified();
	}
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
	if( origin != NULL && (this->origin[0] != origin[0] || this->origin[1] != origin[1] || this->origin[2] != origin[2]) ){
		memcpy( this->origin, origin, sizeof(double) * 3 );
		this->RecalculatePlotAreaBounds();
		this->Modified();
	}
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
	if( spacing != NULL && (this->spacing[0] != spacing[0] || this->spacing[1] != spacing[1] || this->spacing[2] != spacing[2]) ){
		memcpy( this->spacing, spacing, sizeof(double) * 3 );
		this->RecalculateScale();
		this->RecalculatePlotAreaBounds();
		this->Modified();
	}
}


/*!
 *  Get Dcos
 */
void svkPlotLine::GetDcos( double dcos[][3] )
{
    memcpy( dcos, &(*this->dcos)[0], sizeof(double) * 9 );
}


/*!
 *  Set Dcos
 */
void svkPlotLine::SetDcos( vector< vector<double> >* dcos )
{
	if( this->dcos != dcos ) {
		this->dcos = dcos;
		this->Modified();
	}
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
    double xMax = origin[0] + spacing[0] * (*this->dcos)[0][0]
                            + spacing[1] * (*this->dcos)[1][0]
                            + spacing[2] * (*this->dcos)[2][0];
    double xMin;
    if( xMax > origin[0] ) {
        xMin = origin[0];
    } else {
        xMin = xMax;
        xMax = origin[0];
    }
    double yMax = origin[1] + spacing[0] * (*this->dcos)[0][1]
                            + spacing[1] * (*this->dcos)[1][1]
                            + spacing[2] * (*this->dcos)[2][1];
    double yMin;
    if( yMax > origin[1] ) {
        yMin = origin[1];
    } else {
        yMin = yMax;
        yMax = origin[1];
    }
    double zMax = origin[2] + spacing[0] * (*this->dcos)[0][2]
                            + spacing[1] * (*this->dcos)[1][2]
                            + spacing[2] * (*this->dcos)[2][2];
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
	if( this->invertPlots != invertPlots ) {
		this->invertPlots = invertPlots;
		this->Modified();
	}
}


/*!
 *
 */
void svkPlotLine::SetMirrorPlots( bool mirrorPlots ) 
{
	if( this->mirrorPlots != mirrorPlots ) {
		this->mirrorPlots = mirrorPlots;
		this->Modified();
	}
}

/*!
 *
 */
void svkPlotLine::SetPlotDirection( int amplitudeIndex, int pointIndex )
{
	if( this->amplitudeIndex != amplitudeIndex || this->pointIndex != pointIndex ) {
		this->amplitudeIndex = amplitudeIndex;
		this->pointIndex = pointIndex;
		this->RecalculateScale();
		this->Modified();
	}
}


/*!
 *  Set to true if you want the poly data to be regenerated when modifying.
 *  As a side effect poly data will be generated if it is changed from
 *  false to true.
 */
void svkPlotLine::SetGeneratePolyData( bool generatePolyData )
{
	if( this->generatePolyData != generatePolyData ) {
		this->generatePolyData = generatePolyData;
		if( this->generatePolyData ) {
			this->GeneratePolyData();
		}
	}
}


/*!
 *  Set to true if you want the poly data to be regenerating when modifying.
 */
bool svkPlotLine::GetGeneratePolyData( )
{
	return this->generatePolyData;
}
