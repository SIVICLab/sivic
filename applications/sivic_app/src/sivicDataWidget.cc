/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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
 */



#include <sivicDataWidget.h>
#include <vtkSivicController.h>

vtkStandardNewMacro( sivicDataWidget );
vtkCxxRevisionMacro( sivicDataWidget, "$Revision$");


/*! 
 *  Constructor
 */
sivicDataWidget::sivicDataWidget()
{

    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgress );
    this->progressCallback->SetClientData( (void*)this );
    this->referenceSpectra = NULL;
}


/*! 
 *  Destructor
 */
sivicDataWidget::~sivicDataWidget()
{
    if( this->referenceSpectra != NULL ) {
        this->referenceSpectra->Delete();
        this->referenceSpectra = NULL;
    }

}


/*! 
 *  Method in superclass to be overriden to add our custom widgets.
 */
void sivicDataWidget::CreateWidget()
{
/*  This method will create our main window. The main window is a 
    vtkKWCompositeWidget with a vtkKWRendWidget. */

    // Check if already created
    if ( this->IsCreated() )
    {
        vtkErrorMacro(<< this->GetClassName() << " already created");
        return;
    }

    // Call the superclass to create the composite widget container
    this->Superclass::CreateWidget();
    this->CreateListWidget();
}

void sivicDataWidget::CreateListWidget() 
{
    if( this->referenceSpectra != NULL ) {
        this->RemoveCallbackCommandObserver( this->referenceSpectra->GetWidget(), vtkKWMultiColumnList::CellUpdatedEvent );
        this->Script("pack forget %s", this->referenceSpectra->GetWidgetName());
        this->referenceSpectra->Delete();
    }
    this->referenceSpectra = vtkKWMultiColumnListWithScrollbars::New();
    this->referenceSpectra->SetParent(this);
    this->referenceSpectra->Create();
    //this->referenceSpectra->GetWidget()->MovableColumnsOn();
    this->referenceSpectra->GetWidget()->SetWidth(0);
    this->referenceSpectra->GetWidget()->SetHeight(3);
    this->referenceSpectra->VerticalScrollbarVisibilityOff();
    this->referenceSpectra->HorizontalScrollbarVisibilityOff();
    int col_index;
    col_index = this->referenceSpectra->GetWidget()->AddColumn("Description");
    col_index = this->referenceSpectra->GetWidget()->AddColumn("Visible");
    this->referenceSpectra->GetWidget()->SetColumnFormatCommandToEmptyOutput(col_index);
    col_index = this->referenceSpectra->GetWidget()->AddColumn("Color");
    this->referenceSpectra->GetWidget()->SetColumnFormatCommandToEmptyOutput(col_index);
    this->referenceSpectra->GetWidget()->ColumnEditableOn(col_index);
    col_index = this->referenceSpectra->GetWidget()->AddColumn("Active");
    this->referenceSpectra->GetWidget()->SetColumnFormatCommandToEmptyOutput(col_index);
    col_index = this->referenceSpectra->GetWidget()->AddColumn("Filename");

    this->Script("pack %s -expand y -fill both", this->referenceSpectra->GetWidgetName());

    //  Callbacks
    this->AddCallbackCommandObserver( this->referenceSpectra->GetWidget(), vtkKWMultiColumnList::CellUpdatedEvent );
}


/*!
 * Sets the filename for a given row.
 *
 * @param row
 * @param filename
 */
void sivicDataWidget::SetFilename( int row, string filename )
{
    while( row + 1 > filenames.size()) {
        filenames.push_back("");
    }
    this->filenames[row] = svkUtils::GetFilenameFromFullPath(filename);
    this->referenceSpectra->GetWidget()->InsertCellText(row, 4, filenames[row].c_str());
}


/*!
 *
 *  Updates the list of loaded datasets.
 */
void sivicDataWidget::UpdateReferenceSpectraList( )
{
    svkPlotGridView* plotGridView = svkPlotGridView::SafeDownCast( this->plotController->GetView() );
    int numPlots = plotGridView->GetNumberOfReferencePlots() + 1;
    this->CreateListWidget();

    // We start at one since the first plot index is the active spectra above...
    for( int i = 0; i < numPlots; i ++ ) {
        svkImageData* plotData = NULL;
        /*
         * Since the first input to the plotController is spectra, the second
         * is image, and the third-n is spectra we have to do some strange
         * indexing here.
         * TODO: Refactor plotGridView so the spectra are all in order.
         */
        if( i == 0 ) {
            plotData = this->plotController->GetView()->GetInput( i);
        } else {
            plotData = this->plotController->GetView()->GetInput( i + 1 );
        }
        if( plotData != NULL ) {

            this->referenceSpectra->GetWidget()->InsertCellText(i, 0, plotData->GetDcmHeader()->GetStringValue("SeriesDescription").c_str());
            this->referenceSpectra->GetWidget()->InsertCellTextAsInt(i, 1, plotGridView->GetPlotVisibility(i));
            this->referenceSpectra->GetWidget()->SetCellWindowCommandToCheckButton(i, 1);
            this->referenceSpectra->GetWidget()->InsertCellText(i, 2, svkUtils::ColorArrayToString( plotGridView->GetPlotColor(i)).c_str());
            this->referenceSpectra->GetWidget()->SetCellWindowCommandToColorButton(i, 2);
            if( i == svkPlotGridView::SafeDownCast( this->plotController->GetView())->GetActivePlotIndex() ) {
                this->referenceSpectra->GetWidget()->InsertCellTextAsInt(i, 3, 1);
            } else {
                this->referenceSpectra->GetWidget()->InsertCellTextAsInt(i, 3, 0);
            }
            if( filenames.size() > i ) {
                this->referenceSpectra->GetWidget()->InsertCellText(i, 4, filenames[i].c_str());
            }
            this->referenceSpectra->GetWidget()->SetCellWindowCommandToCheckButton(i, 3);
        } else if( i== 0 ) {
        	// No input so reset
            this->filenames.clear();
            this->referenceSpectra->GetWidget()->DeleteAllRows();
        }

    }
    if ( this->sivicController->GetActive4DImageData() != NULL ) {
        this->referenceSpectra->VerticalScrollbarVisibilityOn();
        this->referenceSpectra->HorizontalScrollbarVisibilityOn();
    }

}

/*!
 *  Reset the state of the widget.
 */
void sivicDataWidget::Reset( )
{
    this->filenames.clear();
    this->UpdateReferenceSpectraList();
}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicDataWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
    int newActiveIndex = -1;
    for( int i = 0; i < this->referenceSpectra->GetWidget()->GetNumberOfRows(); i++) {

        int visible = this->referenceSpectra->GetWidget()->GetCellTextAsInt(i,1);
        int active = this->referenceSpectra->GetWidget()->GetCellTextAsInt(i,3);
        if( active == 1 && i != svkPlotGridView::SafeDownCast( this->plotController->GetView())->GetActivePlotIndex() ) {
            newActiveIndex = i;
        }

        svkPlotGridView::SafeDownCast( this->plotController->GetView() )->SetPlotVisibility(i,visible );
        double color[3];
        svkUtils::StringToColorArray( color,this->referenceSpectra->GetWidget()->GetCellText(i,2) );
        svkPlotGridView::SafeDownCast( this->plotController->GetView() )->SetPlotColor(i, color );
    }

    if( newActiveIndex != -1 ) {
        for( int i = 0; i < this->referenceSpectra->GetWidget()->GetNumberOfRows(); i++) {
            if( i != newActiveIndex ) {
                this->referenceSpectra->GetWidget()->InsertCellTextAsInt(i, 3, 0);
                this->referenceSpectra->GetWidget()->SetCellWindowCommandToCheckButton(i, 3);
            }
        }
        this->sivicController->SetActive4DImageData( newActiveIndex );
    }
}


/*!
 * Updates the progress info during processing.
 *
 * @param subject
 * @param
 * @param thisObject
 * @param callData
 */
void sivicDataWidget::UpdateProgress(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 100.0*(*(double*)(callData)) );
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->SetStatusText(
    static_cast<vtkAlgorithm*>(subject)->GetProgressText() );

}

