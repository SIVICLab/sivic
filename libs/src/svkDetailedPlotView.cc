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



#include <svkDetailedPlotView.h>


using namespace svk;


vtkCxxRevisionMacro(svkDetailedPlotView, "$Rev$");
vtkStandardNewMacro(svkDetailedPlotView);


//! Constructor 
svkDetailedPlotView::svkDetailedPlotView()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    this->slice = 0;
    this->rwi = NULL;
    this->dataVector.push_back(NULL);

    this->isPropOn.assign(svkDetailedPlotView::LAST_PROP+1, false);
    this->isRendererOn.assign(svkDetailedPlotView::LAST_RENDERER+1, false);
    this->isPropVisible.assign(svkDetailedPlotView::LAST_PROP+1, false);     //Is the actor in the views FOV?

    vtkRenderer* nullRenderer = NULL;
    this->renCollection.assign(svkDetailedPlotView::LAST_RENDERER+1, nullRenderer);     //Is the actor in the views FOV?

    vtkProp* nullProp = NULL;
    this->propCollection.assign(svkDetailedPlotView::LAST_PROP+1, nullProp);     //Is the actor in the views FOV?

    vtkRenderer* ren = vtkRenderer::New();
    this->SetRenderer( svkDetailedPlotView::PRIMARY, ren );
    ren->Delete();

    this->GetRenderer( svkDetailedPlotView::PRIMARY )->SetLayer(0);
    this->plotActor = vtkXYPlotActor::New();
    
    this->point = svkSpecPoint::New();
    this->frequencyPoints = vtkFloatArray::New();
    this->specUnits = svkSpecPoint::PTS;
    this->xAxis = vtkAxisActor2D::New();
    this->xAxis->SetNumberOfMinorTicks( 10 );
    this->xAxis->SetNumberOfLabels( 10 );
    this->numPlots = 0;
    
}


//! Destructor
svkDetailedPlotView::~svkDetailedPlotView()
{
    if( this->rwi != NULL ) {
        this->rwi->Delete();
        this->rwi = NULL;
    }
    if( this->point != NULL ) {
        this->point->Delete();
        this->point = NULL;
    }
    if( this->frequencyPoints != NULL ) {
        this->frequencyPoints->Delete();
        this->frequencyPoints = NULL;
    }
    if( this->xAxis != NULL ) {
        this->xAxis->Delete();
        this->xAxis = NULL;
    }

    if( this->plotActor != NULL ) {
        this->plotActor->Delete();
        this->plotActor = NULL;
    }
       
    inputIndices.clear();
    inputComponents.clear();
    inputChannels.clear();
}


/*!
 *  Set input data and initialize default range values. 
 *  Also, should call ObserveData. 
 *
 *  \param data the candidate data for input
 *  \param index the index in which to set the data
 *
 */
void svkDetailedPlotView::SetInput(svkImageData* data, int index)
{
    if( index == 0 && data != NULL ) {
        if( this->dataVector[index] != NULL  ) {
            this->dataVector[index]->Delete();
        }
        this->ObserveData( data );
        data->Register( this );
        this->dataVector[index] = data;
        data->Update();
        this->point->SetDcmHeader( data->GetDcmHeader() );
        this->CreateFrequencyArray();
        
        this->CreateXYPlot(); 
    } 
}


/*!
 *  Currently the slice has no effect on the view.
 *  
 *  \param slice the new slice
 */
void svkDetailedPlotView::SetSlice(int slice)
{
    this->slice = slice;
}


/*!
 *  CURRENTLY NOT IMPLEMENETED
 *
 *  \param tlcID the id of the top left corner voxel
 *  \param tlcID the id of the bottom right corner voxel
 */
void svkDetailedPlotView::SetTlcBrc(int tlcID, int brcID)
{
}


/*!
 *  Sets the interactor, and attach a renderer it its window.
 *  NOTE: This will remove any/all renderers that may have been
 *  in the renderWindow.
 *  
 *  TODO: AddRenderer can throw an error if the RenderWindow has not been
 *        initialized, either add a check or re-implement.
 *
 *  \param rwi the vtkRendererWindowInteractor you wish to associate with the view
 */
void svkDetailedPlotView::SetRWInteractor( vtkRenderWindowInteractor* rwi )
{
    if( this->rwi != NULL ) {
        this->rwi->Delete();
    }
    this->rwi = rwi;
    this->rwi->Register( this );

    // Lets make sure there are no renderers in the new render window, if there are remove them
    vtkRendererCollection* myRenderers = this->rwi->GetRenderWindow()->GetRenderers();
    vtkCollectionIterator* myIterator = vtkCollectionIterator::New();
    myIterator->SetCollection( myRenderers );
    while( !myIterator->IsDoneWithTraversal() ) {
        this->rwi->GetRenderWindow()->RemoveRenderer( static_cast<vtkRenderer*>(myIterator->GetCurrentObject()) );
        myIterator->GoToNextItem();
    }
    if( !this->rwi->GetRenderWindow()->HasRenderer( this->GetRenderer( svkDetailedPlotView::PRIMARY) ) ) {
        this->rwi->GetRenderWindow()->AddRenderer( this->GetRenderer( svkDetailedPlotView::PRIMARY ) );
    }
    myIterator->Delete();

}


/*!
 *
 *  Creates an array to be used to represent the frequencies. Currently
 *  it is just consecutive integers 0-N representing points. The axis are
 *  converted later to the specific units.
 */
void svkDetailedPlotView::CreateFrequencyArray( )
{
    int arrayLength = this->dataVector[0]->GetCellData()->GetNumberOfTuples();
    this->frequencyPoints->SetNumberOfComponents(1);
    this->frequencyPoints->SetNumberOfTuples(arrayLength);
    this->frequencyPoints->SetName("frequencyPoints");
    for( int i = 0; i < arrayLength; i++ ) {
        frequencyPoints->SetTuple1(i,i);
    } 
    frequencyPoints->Modified();
}


/*!
 *  This method translates the x Axis into the correct units.
 */
void svkDetailedPlotView::CalculateXAxisRange()
{
    if( dataVector[0] == NULL ) {
        cout<< " Cannot create x axis without a data object..." << endl;
        return;
    }
    this->Refresh();
    double* range = this->plotActor->GetXAxisActor2D()->GetAdjustedRange();
    double min;
    double max;
    if( this->specUnits == svkSpecPoint::PTS ) {
        this->xAxis->SetTitle("PTS");
        min =  point->ConvertPosUnits(range[0], svkSpecPoint::PTS, this->specUnits );
        max =  point->ConvertPosUnits(range[1], svkSpecPoint::PTS, this->specUnits );
    } else if ( this->specUnits == svkSpecPoint::Hz ) {
        this->xAxis->SetTitle("Hz");
        min =  point->ConvertPosUnits(range[0], svkSpecPoint::PTS, this->specUnits );
        max =  point->ConvertPosUnits(range[1], svkSpecPoint::PTS, this->specUnits );
    } else if ( this->specUnits == svkSpecPoint::PPM ) {
        this->xAxis->SetTitle("PPM");
        min =  point->ConvertPosUnits(range[0], svkSpecPoint::PTS, this->specUnits );
        max =  point->ConvertPosUnits(range[1], svkSpecPoint::PTS, this->specUnits );
    } else {
        cout << " Bad unit type. Defaulting to Points." << endl;
        this->specUnits = svkSpecPoint::PTS;
        CalculateXAxisRange();
    }
    this->xAxis->SetRange( min, max );
    this->plotActor->Modified();
    this->Refresh();

}


/*!
 *  SetWindowLevel for spectral view;  index 0 is frequency, index 1 is intensity.
 *  Currently assumes point input, but will render in the set units.
 *
 *  \param lower the lower limit
 *  \param upper the upper limit
 *  \param index which dimension you wish to chhange, frequency or magnitude 
 *
 */
void svkDetailedPlotView::SetWindowLevelRange( double lower, double upper, int index)
{
    if( dataVector[0] == NULL ) {
            return;
        }
    if (index == FREQUENCY) {
        this->plotActor->SetXRange( lower, upper );
        // The "AdjustedRange" is the actual range of the axis, we need to render once
        // to get the correct values.
        this->Refresh();
        double* adjustedRange = this->plotActor->GetXAxisActor2D()->GetAdjustedRange();
        this->xAxis->SetRange(adjustedRange[0],adjustedRange[1]);
        this->plotActor->Modified();
        
    } else if ( index == AMPLITUDE ) {
        this->plotActor->SetYRange(lower, upper );
        this->plotActor->Modified();
    }
    this->Refresh();
}


/*
 *  Currently not implemented. Components are chosen in AddPlot. 
 *
 *  \param component the compent you wish to display, REAL, IMAGINARY, MAGNITUDE
 */
void svkDetailedPlotView::SetComponent( svkBoxPlot::PlotComponent component)
{
}


/*!
 *  Method is called when data object is Modified. 
 */
void svkDetailedPlotView::Refresh()
{
    if (this->GetDebug()) {

        cout << "svkDetailefPlotView::Refresh calls plotGrid->Update() first " << endl; 
    }

    this->svkDataView::Refresh(); 
}


/*!
 *  Adds a plot line to the view.
 *  \param index the index of the data array in the svkImageData object you wish to plot
 *  \param component the component (REAL/IMAGINARY) of the data array in the
 *                   svkImageData object you wish to plot
 *
 */
void svkDetailedPlotView::AddPlot( int index, int component, int channel, int timePoint )
{
    // Check to see if the array is already being plotted
    for ( int i = 0; i < this->numPlots; i++ ) {
        if( inputIndices[i] == index && inputComponents[i] == component && inputChannels[i] == channel ) {
            return;
        }
    }

    // Check to make sure the index is valied
    vtkDataArray* myArray = NULL;
    if( this->dataVector[0] != NULL ) { 
        myArray = svkMrsImageData::SafeDownCast(dataVector[0])->GetSpectrumFromID(index, timePoint, channel);
    }

    // Add Array
    if( myArray != NULL ) {
        this->inputIndices.push_back(index);
        this->inputComponents.push_back(component);
        this->inputChannels.push_back(channel);

        vtkFieldData *fieldData = vtkFieldData::New();
        fieldData->AllocateArrays(2);
        
        // Set Plot attributes
        int arrayLength = myArray->GetNumberOfTuples();
        vtkFloatArray* xyData = vtkFloatArray::New();
        xyData->SetNumberOfComponents(1);
        xyData->SetNumberOfTuples(arrayLength);
        xyData->SetName( "PlotData" );

        // We need to push the data into the first component of a new array or it won't plot
        for ( int i = 0; i < arrayLength; i++ ) {
            xyData->SetTuple1(i, myArray->GetComponent(i,component));
        }

        // We need push the data into field data to plot x vs y
        fieldData->AddArray(this->frequencyPoints);
        fieldData->AddArray(xyData);

        // dataObject is just a container
        vtkDataObject *dataObject = vtkDataObject::New();
        dataObject->SetFieldData(fieldData);
        fieldData->Delete();
        this->plotActor->AddDataObjectInput(dataObject);
        dataObject->Delete();
        this->plotActor->SetXValuesToValue();
        this->plotActor->SetDataObjectXComponent(this->numPlots, 0);
        this->plotActor->SetDataObjectYComponent(this->numPlots, 1);
        //this->plotActor->SetDataObjectPlotModeToColumns();
        switch (this->numPlots) {
            case 0:
                this->plotActor->SetPlotColor(this->numPlots,1,0,0);
                break;
            case 1:
                this->plotActor->SetPlotColor(this->numPlots,0,1,0);
                break;
            case 2:
                this->plotActor->SetPlotColor(this->numPlots,0,0,1);
                break;
            case 3:
                this->plotActor->SetPlotColor(this->numPlots,1,1,0);
                break;
            case 4:
                this->plotActor->SetPlotColor(this->numPlots,0,1,1);
                break;
            case 5:
                this->plotActor->SetPlotColor(this->numPlots,1,0,1);
                break;
            default:
                this->plotActor->SetPlotColor(this->numPlots,0,0,0);
                break;
        }
        this->numPlots++;
        this->CalculateXAxisRange();
    
    }

}


/*!
 *  Sets up the plot once the data has been loaded.
 */
void svkDetailedPlotView::CreateXYPlot()
{
    this->plotActor->SetYTitle("Intensity");
    // We are not using the built-in X axis...
    this->plotActor->SetXTitle("");
    this->plotActor->SetPosition( 0.01, 0.12 );
    this->plotActor->SetPosition2( 1.0, 1.0 );
    this->plotActor->GetXAxisActor2D()->SetLabelFormat("");
    this->plotActor->GetXAxisActor2D()->SetTickLength(0);
    this->xAxis->GetPositionCoordinate()->SetValue(0,0,0 );
    this->xAxis->GetPosition2Coordinate()->SetValue(0,0,0 );
    this->xAxis->GetPositionCoordinate()->SetReferenceCoordinate( 
                         this->plotActor->GetXAxisActor2D()->GetPositionCoordinate());
    this->xAxis->GetPosition2Coordinate()->SetReferenceCoordinate( 
                         this->plotActor->GetXAxisActor2D()->GetPosition2Coordinate());
    this->GetRenderer( svkDetailedPlotView::PRIMARY )->AddActor( this->plotActor );
    this->GetRenderer( svkDetailedPlotView::PRIMARY )->AddActor( this->xAxis );
    this->plotActor->Modified();
    int arrayLength = this->dataVector[0]->GetCellData()->GetNumberOfTuples();
    this->SetWindowLevelRange( 0, arrayLength, FREQUENCY );
    this->CalculateXAxisRange();
}


/*!
 *  Changes the units used.
 */
void svkDetailedPlotView::SetUnits( int units)
{
    this->specUnits = units;
    this->CalculateXAxisRange();
}


/*!
 *  Method copies the data from the image data into the plot data.
 *  Use this method to Update the plots to the current state of
 *  the data.
 */
void svkDetailedPlotView::Update()
{
    this->plotActor->RemoveAllInputs();
    int oldNumPlots = this->numPlots; 
    vector<int> oldInputIndices(inputIndices.size());
    vector<int> oldInputComponents(inputComponents.size());
    vector<int> oldInputChannels(inputChannels.size());
    copy(this->inputIndices.begin(), this->inputIndices.end(), oldInputIndices.begin());
    copy(this->inputComponents.begin(), this->inputComponents.end(), oldInputComponents.begin());
    copy(this->inputChannels.begin(), this->inputChannels.end(), oldInputChannels.begin());
    inputIndices.clear();
    inputComponents.clear();
    inputChannels.clear();
    this->numPlots = 0;
    for ( int i = 0; i < oldNumPlots; i++ ) {
        this->AddPlot( oldInputIndices[i], oldInputComponents[i], oldInputChannels[i] ); 
    }
 
}
