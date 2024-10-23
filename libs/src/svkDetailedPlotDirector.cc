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


#include <svkDetailedPlotDirector.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkGlyphSource2D.h>
#include </usr/include/vtk/vtkLegendBoxActor.h>
#include </usr/include/vtk/vtkFieldData.h>

using namespace svk;


//vtkCxxRevisionMacro(svkDetailedPlotDirector, "$Rev$");
vtkStandardNewMacro(svkDetailedPlotDirector);


//! Constructor 
svkDetailedPlotDirector::svkDetailedPlotDirector()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif
    this->xyPlotActor = svkXYPlotActor::New();
    this->xyPlotActor->SetYTitle("");
    this->xyPlotActor->SetXTitle("");
    // We are not using the built-in X axis...
    this->xyPlotActor->SetPosition( 0.0, 0.0 );
    this->xyPlotActor->SetPosition2( 1.0, 1.0 );
    // If we don't set the title as a non-empty string then the properties won't persist
    this->xyPlotActor->SetTitle(" ");
    this->xyPlotActor->GetTitleTextProperty()->SetColor(0,1,1);
    this->xyPlotActor->AdjustTitlePositionOff();
    this->xyPlotActor->SetTitlePosition(0.15, 0.9);
#if VTK_MINOR_VERSION >= 6 || VTK_MAJOR_VERSION >= 6
    this->xyPlotActor->ChartBoxOn();
    this->xyPlotActor->ChartBorderOn();
    this->xyPlotActor->GetChartBoxProperty()->SetColor(0,0,0);
#endif

    this->xyPlotActor->LegendOff();
    this->xyPlotActor->SetLegendPosition(0.70, 0.70);
    this->xyPlotActor->SetLegendPosition2(0.25, 0.2);
    this->glyphGenerator = vtkGlyphSource2D::New();
    this->glyphGenerator->SetGlyphTypeToNone();
    this->xyPlotActor->GetXAxisActor2D()->GetProperty()->SetColor(0,1,0);
    this->xyPlotActor->GetYAxisActor2D()->GetProperty()->SetColor(0,1,0);
    this->xyPlotActor->GetLegendActor()->GetEntryTextProperty()->SetFontFamilyToCourier();
    this->xyPlotActor->SetAdjustXLabels(0);
    this->xyPlotActor->SetAdjustYLabels(0);
    this->xyPlotActor->SetXLabelFormat("%0.2f");
    this->xyPlotActor->GetAxisLabelTextProperty()->BoldOn();
    this->abscissa = NULL;
    this->numPoints = -1;
    this->cursorLocationCB = NULL;
    this->ruler = vtkAxisActor2D::New();
    this->ruler->SetLabelVisibility(0);
    this->ruler->SetTickVisibility(0);
    this->ruler->GetProperty()->SetColor(0,1,0);
    // This callback will catch changes to the dataset
    this->dataModifiedCB = vtkCallbackCommand::New();
    this->dataModifiedCB->SetCallback( UpdateData );
    this->dataModifiedCB->SetClientData( (void*)this );
}


//! Destructor
svkDetailedPlotDirector::~svkDetailedPlotDirector()
{
    if( this->xyPlotActor != NULL ) {
        this->xyPlotActor->Delete();
        this->xyPlotActor = NULL;
    }

    if( this->dataModifiedCB != NULL ) {
        this->dataModifiedCB->Delete();
        this->dataModifiedCB = NULL;
    }

    if( this->glyphGenerator != NULL ) {
        this->glyphGenerator->Delete();
        this->glyphGenerator = NULL;
    }
    if( this->ruler != NULL ) {
        this->ruler->Delete();
        this->ruler = NULL;
    }
    if( this->abscissa != NULL ) {
        this->abscissa->Delete();
        this->abscissa = NULL;
    }
}


/*!
 *
 * Adds an input to the actor. This will add a plot line of the given component.
 * The vtkDataObject input is optionally and is only used to regenerate the
 * magnitude array when the given object is modified.
 *
 * @param array
 * @param component
 */
void svkDetailedPlotDirector::AddInput( vtkDataArray* array, int component, vtkDataObject* sourceToObserve )
{
    if( array != NULL  && ( this->numPoints == array->GetNumberOfTuples() || this->numPoints == -1 ) ) {
        if( this->numPoints == -1 ) {
            this->numPoints = array->GetNumberOfTuples();
        }

        vtkFieldData *fieldData = vtkFieldData::New();
        fieldData->AllocateArrays(2);

        if( this->abscissa == NULL ) {
            this->GenerateAbscissa( 0, this->numPoints );
            this->xyPlotActor->SetXRange(0, this->numPoints );
            double range[2];
            array->GetRange(range);
            this->xyPlotActor->SetYRange(range );
        }
        // We need push the data into field data to plot x vs y
        fieldData->AddArray(this->abscissa);
        fieldData->AddArray(array);

        // And we will add a magnitude array as well
        vtkDataArray* magArray = vtkFloatArray::New();
        magArray->SetNumberOfTuples( array->GetNumberOfTuples());
        magArray->SetNumberOfComponents( 1 );
        this->GenerateMagnitudeArray( array, magArray );
        fieldData->AddArray(magArray);
        magArray->Delete();

        // dataObject is just a container
        vtkDataObject *dataObject = vtkDataObject::New();
        dataObject->SetFieldData(fieldData);
        fieldData->Delete();
        this->xyPlotActor->AddDataObjectInput(dataObject);
        dataObject->Delete();
        this->xyPlotActor->SetXValuesToValue();
        //int numPlots = this->xyPlotActor->GetDataObjectInputList()->GetNumberOfItems();
        int numPlots = this->xyPlotActor->GetDataObjectInputConnectionHolder()->GetNumberOfInputConnections(0);
        this->xyPlotActor->GetLegendActor()->SetNumberOfEntries(numPlots);
        if( sourceToObserve != NULL ) {
            sourceToObserve->AddObserver(vtkCommand::ModifiedEvent, dataModifiedCB);
        }

        /*
         * Here "Component" means of the field data not of the array. The first array is the abscissa
         * and has only one component. The second array's components will be components n+1 for the
         * field data.
         */
        this->xyPlotActor->SetDataObjectXComponent(numPlots-1, 0);
        this->xyPlotActor->SetDataObjectYComponent(numPlots-1, component + 1);

        // We need to call Modified on the Legend text to get around a VTK bug.
        // Without this the font family goes back to default for added inputs
        this->xyPlotActor->GetLegendActor()->GetEntryTextProperty()->Modified();
        this->glyphGenerator->Update();
        this->xyPlotActor->GetLegendActor()->SetEntrySymbol(numPlots-1, this->glyphGenerator->GetOutput());
    }

}


/*!
 * This method regenerates the magnitude arrays for plotting.
 *
 */
void svkDetailedPlotDirector::RegenerateMagnitudeArrays()
{
    //vtkDataObjectCollection* allData = this->xyPlotActor->GetDataObjectInputList();
    vtkAlgorithm* allDataAlgo = this->xyPlotActor->GetDataObjectInputConnectionHolder();

    int numPlots = allDataAlgo->GetNumberOfInputConnections(0);
    for( int i = 0; i < numPlots; i++ ) {
        //this->GenerateMagnitudeArray( allData->GetItem(i)->GetFieldData()->GetArray(1), allData->GetItem(i)->GetFieldData()->GetArray(2) );
        this->GenerateMagnitudeArray( allDataAlgo->GetInputDataObject(0, i)->GetFieldData()->GetArray(1), allDataAlgo->GetInputDataObject(0,i)->GetFieldData()->GetArray(2) );
    }
}


/*!
 * Generates a magnitude array from an input array. It takes the root sum of the squares
 * of all the components in the input complex array.
 *
 * @param complexArray
 * @param magnitudeArray
 */
void svkDetailedPlotDirector::GenerateMagnitudeArray( vtkDataArray* complexArray, vtkDataArray* magnitudeArray)
{
    if( complexArray != NULL && magnitudeArray != NULL ) {
    	magnitudeArray->SetNumberOfTuples( complexArray->GetNumberOfTuples());
        for( int i = 0; i < complexArray->GetNumberOfTuples(); i++ ) {
            double* value = complexArray->GetTuple(i);
            double sumSquares = 0;
            for( int j = 0; j < complexArray->GetNumberOfComponents(); j++) {
                sumSquares += value[j] * value[j];
            }
            magnitudeArray->SetTuple1(i, pow(sumSquares,0.5) );
        }
    }
}


/*!
 *
 * Removes all current inputs from the plot actor. Also removes callbacks.
 *
 */
void svkDetailedPlotDirector::RemoveAllInputs( )
{
    if( this->dataModifiedCB != NULL) {
        // We want to make sure we stop monitoring the input arrays
        this->dataModifiedCB->Delete();
        this->dataModifiedCB = vtkCallbackCommand::New();
        this->dataModifiedCB->SetCallback( UpdateData );
        this->dataModifiedCB->SetClientData( (void*)this );
    }
    this->xyPlotActor->RemoveAllDataSetInputConnections();
    this->numPoints = -1;
}


/*!
 *  NOT YET IMPLEMENTED
 *
 * @param array
 */
void svkDetailedPlotDirector::RemoveInput( vtkDataArray* array )
{

}


/*!
 *  Sets the color for a given plot.
 *
 * @param plotIndex
 * @param rgb
 */
void svkDetailedPlotDirector::SetPlotColor( int plotIndex, double* rgb)
{
    this->xyPlotActor->SetPlotColor(plotIndex, rgb);
    this->Refresh();
}


void svkDetailedPlotDirector::SetBackgroundColor( double* rgb )
{
    #if VTK_MINOR_VERSION >= 6 || VTK_MAJOR_VERSION >= 6
        this->xyPlotActor->GetChartBoxProperty()->SetColor(rgb);
    #endif

}


/*!
 * Sets the background opacity
 */
void svkDetailedPlotDirector::SetBackgroundOpacity( double opacity )
{
    #if VTK_MINOR_VERSION >= 6 || VTK_MAJOR_VERSION >= 6
        this->xyPlotActor->GetChartBoxProperty()->SetOpacity(opacity);
    #endif

}


/*!
 *  Sets the visibilty of the background.
 */
void svkDetailedPlotDirector::SetBackgroundVisibility( bool visible )
{
#if VTK_MINOR_VERSION >= 6 || VTK_MAJOR_VERSION >= 6
    if( visible ) {
        this->xyPlotActor->ChartBoxOn();
    } else {
        this->xyPlotActor->ChartBoxOff();
    }
#endif
}


/*!
 *  Sets the visibilty of the annotation text.
 */
void svkDetailedPlotDirector::SetAnnotationTextVisibility( bool visible )
{
    if( visible ) {
        this->xyPlotActor->GetTitleTextProperty()->SetOpacity(1);
    } else {
        this->xyPlotActor->GetTitleTextProperty()->SetOpacity(0);
    }

}


/*!
 *  Sets the line width.
 */
void svkDetailedPlotDirector::SetLineWidth( float width )
{
	this->xyPlotActor->GetProperty()->SetLineWidth( width );
}


/*!
 *  Sets an annotation. The title is used for this.
 */
void svkDetailedPlotDirector::SetAnnotationText( string text )
{
    this->xyPlotActor->SetTitle(text.c_str());
}


/*!
 *  Sets the range for the abscissa using indecies.
 *
 * @param lower
 * @param upper
 */
void svkDetailedPlotDirector::SetIndexRange( int lower, int upper )
{
    if( this->abscissa != NULL ) {
        double lowerValue = this->abscissa->GetComponent(lower,0);
        double upperValue = this->abscissa->GetComponent(upper,0);
        if( lowerValue < upperValue ) {
            this->xyPlotActor->SetXRange( lowerValue, upperValue );
        } else {
            this->xyPlotActor->SetXRange( upperValue, lowerValue );
        }
    }
}


/*!
 *  Generates the array for the abscissa.
 *
 * @param header
 * @param type
 */
void svkDetailedPlotDirector::GenerateAbscissa( svkDcmHeader* header, svkSpecPoint::UnitType type )
{
    if( this->numPoints != -1 ) {
            float firstPointValue = 0;
            float lastPointValue = this->numPoints -1;
        if( type != svkSpecPoint::PTS ) {
            svkSpecPoint* converter = svkSpecPoint::New();
            converter->SetDcmHeader( header );
            firstPointValue = converter->ConvertPosUnits(
                0,
                svkSpecPoint::PTS,
                type
            );
            lastPointValue = converter->ConvertPosUnits(
                this->numPoints - 1,
                svkSpecPoint::PTS,
                type
            );
            converter->Delete();
        }
        this->GenerateAbscissa( firstPointValue, lastPointValue);
    }

}


/*!
 *  Generates the array for the abscissa.
 *
 * @param lower
 * @param upper
 */
void svkDetailedPlotDirector::GenerateAbscissa( double firstPointValue, double lastPointValue )
{
    if( this->abscissa == NULL ) {
        this->abscissa = vtkFloatArray::New();
    }
    abscissa->SetNumberOfComponents(1);
    abscissa->SetNumberOfTuples(this->numPoints);
    abscissa->SetName( "abscissa" );
    double scale = ( lastPointValue - firstPointValue ) / (this->numPoints-1);
    for ( int i = 0; i < this->numPoints; i++ ) {
            abscissa->SetTuple1(i, firstPointValue + i*scale );
    }
    if( firstPointValue > lastPointValue ) {
        this->xyPlotActor->ReverseXAxisOn();
    } else {
        this->xyPlotActor->ReverseXAxisOff();
    }
}


/*!
 *  Sets the vertical range of the data.
 *
 * @param lower
 * @param upper
 */
void svkDetailedPlotDirector::SetYRange( double lower, double upper )
{
    this->xyPlotActor->SetYRange( lower, upper );
}


/*!
 *  Pure Getter.
 * @return
 */
svkXYPlotActor* svkDetailedPlotDirector::GetPlotActor()
{
    return this->xyPlotActor;
}


/*!
 *  Pure getter.
 *
 * @return
 */
vtkAxisActor2D* svkDetailedPlotDirector::GetRuler()
{
    return this->ruler;
}


/*!
 * Adds the observer to watch mouse movements and updates the cursor.
 *
 * @param rwi
 */
void svkDetailedPlotDirector::AddOnMouseMoveObserver( vtkRenderWindowInteractor* rwi)
{
    // Setup Callbacks
    if( this->cursorLocationCB == NULL ) {
        this->cursorLocationCB = vtkCallbackCommand::New();
        this->cursorLocationCB->SetCallback( UpdateCursorLocation );
        this->cursorLocationCB->SetClientData( (void*)this );
    }

    if( !rwi->HasObserver(vtkCommand::MouseMoveEvent, cursorLocationCB )) {
        rwi->AddObserver(vtkCommand::MouseMoveEvent, cursorLocationCB);
    }
}


/*!
 * Remove the observer for the cursor.
 *
 * @param rwi
 */
void svkDetailedPlotDirector::RemoveOnMouseMoveObserver( vtkRenderWindowInteractor* rwi)
{
    if( this->cursorLocationCB != NULL ) {
		if( rwi->HasObserver(vtkCommand::MouseMoveEvent, cursorLocationCB )) {
			rwi->RemoveObserver(cursorLocationCB);
		}
    }
}


/*!
 * For i given x-value return the index it closes matches in the adscissa
 * array. If it is not within the abscissa, return -1.
 *
 * @param xValue
 * @return
 */
int svkDetailedPlotDirector::GetPointIndexFromXValue( double xValue )
{
    if( this->abscissa != NULL ) {
        for( int i = 0; i < this->abscissa->GetNumberOfTuples() - 1; i++) {
            double abscissaValue = this->abscissa->GetTuple1( i );
            double abscissaValueNext = this->abscissa->GetTuple1( i + 1 );
            if( ( xValue >  abscissaValue && xValue <=  abscissaValueNext )
              ||( xValue <  abscissaValue && xValue >=  abscissaValueNext ) ) {
                if( xValue == abscissaValueNext ) {
                    return i+1; 
                } else {
                    return i; 
                }
            }
        }
    }
    return -1;
}


/*!
 * For a given index in the abscissa array find the corresponding Y value
 * for the given plot index.
 *
 * @param plotIndex
 * @param pointIndex
 * @return
 */
double svkDetailedPlotDirector::GetYValueFromIndex( int plotIndex, int pointIndex )
{
    //vtkDataObjectCollection* collection = this->xyPlotActor->GetDataObjectInputList();
    //vtkDataObject* dataObject = collection->GetItem( plotIndex );

    vtkAlgorithm* collection = this->xyPlotActor->GetDataObjectInputConnectionHolder();
    vtkDataObject* dataObject = collection->GetInputDataObject(0, plotIndex);

    int component = this->xyPlotActor->GetDataObjectYComponent( plotIndex );
    int arrayComponent;
    int arrayIndex = dataObject->GetFieldData()->GetArrayContainingComponent( component, arrayComponent );
    return dataObject->GetFieldData()->GetArray(arrayIndex)->GetComponent( pointIndex, arrayComponent );
}


/*!
 * For a given X value, find the corresponding Y value.
 *
 * @param plotIndex
 * @param xValue
 * @return
 */
double svkDetailedPlotDirector::GetYValueFromXValue( int plotIndex, double xValue )
{
    int pointIndex = this->GetPointIndexFromXValue( xValue );
    return this->GetYValueFromIndex( plotIndex, pointIndex );
}


/*!
 * Simply calls modified on the actor so that it will re-render.
 * This recalculates the values in the plots.
 */
void svkDetailedPlotDirector::Refresh()
{
    this->xyPlotActor->GetLegendActor()->GetEntryTextProperty()->Modified();
    this->xyPlotActor->Modified();
}


//! Callback tied to data modified events. Regenerates plot data when the vtkImageData is modified.
void svkDetailedPlotDirector::UpdateData(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata)
{
    svkDetailedPlotDirector* director = static_cast<svkDetailedPlotDirector*>(thisObject);
    director->RegenerateMagnitudeArrays();
}


/*!
 *  This is the mouse movement callback used to update the cursor position of the array values.
 *
 * @param subject
 * @param eid
 * @param thisObject
 * @param calldata
 */
void svkDetailedPlotDirector::UpdateCursorLocation( vtkObject* subject, unsigned long eid, void* thisObject, void *calldata)
{
    double pos[2];
    svkDetailedPlotDirector* director = static_cast<svkDetailedPlotDirector*>(thisObject);
    vtkCoordinate* mousePosition = vtkCoordinate::New();
    vtkRenderWindowInteractor *rwi =
            vtkRenderWindowInteractor::SafeDownCast( subject );
    pos[0] = rwi->GetEventPosition()[0];
    pos[1] = rwi->GetEventPosition()[1];
    double u = pos[0];
    double v = pos[1];
    int* windowSize = rwi->GetRenderWindow()->GetSize();
    if( director != NULL && director->GetPlotActor() != NULL ) {
        vtkRenderer* renderer = rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
        director->GetPlotActor()->ViewportToPlotCoordinate( renderer, u, v);
        if( director->GetPlotActor()->IsInPlot(renderer,pos[0],pos[1] )) {
            double* yCoordinate  = director->GetPlotActor()->GetYAxisActor2D()->GetPosition();
            double* yCoordinate2 = director->GetPlotActor()->GetYAxisActor2D()->GetPosition2();
            if( pos[0] < windowSize[0]/2.0 ) {
                director->GetRuler()->SetPosition(pos[0]/((double)windowSize[0]),yCoordinate2[1]/((double)windowSize[1]));
                director->GetRuler()->SetPosition2(pos[0]/((double)windowSize[0]),yCoordinate[1]/((double)windowSize[1]));
                director->GetRuler()->SetTitlePosition(0.05);
            } else {
                director->GetRuler()->SetPosition2(pos[0]/((double)windowSize[0]),yCoordinate2[1]/((double)windowSize[1]));
                director->GetRuler()->SetPosition(pos[0]/((double)windowSize[0]),yCoordinate[1]/((double)windowSize[1]));
                director->GetRuler()->SetTitlePosition(0.95);
            }
            std::stringstream xValue;
            xValue.precision(4);
            xValue << u ;
            director->GetRuler()->SetTitle(xValue.str().c_str() );
            director->GetRuler()->SetVisibility( true );
            int numPlots = director->GetPlotActor()->GetDataObjectInputConnectionHolder()->GetNumberOfInputConnections(0);
            for( int i = 0; i < numPlots; i++ ) {
                double plotValue = director->GetYValueFromXValue( i, u);
                std::stringstream yValue;
                yValue.precision(3);
                if( plotValue >= 0 ) {
                    yValue << "+";
                }
                yValue << std::scientific << plotValue;
                director->GetPlotActor()->LegendOn();
                director->GetPlotActor()->GetLegendActor()->SetEntryString(i, yValue.str().c_str());
            }
        } else {
            director->GetRuler()->SetVisibility( false );
            int numPlots = director->GetPlotActor()->GetDataObjectInputConnectionHolder()->GetNumberOfInputConnections(0);
            for( int i = 0; i < numPlots; i++ ) {
                director->GetPlotActor()->LegendOff();
            }

        }
    }
}
