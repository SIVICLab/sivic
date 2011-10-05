/*
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/applications/sivic_app/src/sivicDataWidget.cc $
 *  $Rev: 936 $
 *  $Author: jccrane $
 *  $Date: 2011-06-03 11:41:20 -0700 (Fri, 03 Jun 2011) $
 */



#include <sivicDataWidget.h>
#include <vtkSivicController.h>

vtkStandardNewMacro( sivicDataWidget );
vtkCxxRevisionMacro( sivicDataWidget, "$Revision: 936 $");


/*! 
 *  Constructor
 */
sivicDataWidget::sivicDataWidget()
{

    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgress );
    this->progressCallback->SetClientData( (void*)this );
    this->referenceSpectra = NULL;
    this->activeIndex = 0;
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

    //  =================================== 
    //  Zero Filling Selector
    //  =================================== 

    this->referenceSpectra = vtkKWMultiColumnListWithScrollbars::New();
    this->referenceSpectra->SetParent(this);
    this->referenceSpectra->Create();
    //this->referenceSpectra->GetWidget()->MovableColumnsOn();
    this->referenceSpectra->GetWidget()->SetWidth(0);
    this->referenceSpectra->GetWidget()->SetHeight(3);
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

    this->Script("grid %s -row 0 -column 0 -sticky nw", this->referenceSpectra->GetWidgetName(),   0);

    this->Script("grid rowconfigure %s 0 -weight 1 -maxsize 120", this->GetWidgetName() );

    this->Script("grid columnconfigure %s 0 -weight 1 ", this->GetWidgetName() );
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
    this->referenceSpectra->GetWidget()->InsertCellText(row, 4, svkUtils::GetFilenameFromFullPath(filename).c_str());
}


/*!
 *
 *  Updates the list of loaded datasets.
 */
void sivicDataWidget::UpdateReferenceSpectraList( )
{
    svkPlotGridView* plotGridView = svkPlotGridView::SafeDownCast( this->plotController->GetView() );
    int numPlots = plotGridView->GetNumberOfReferencePlots() + 1;

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
            if( i == svkPlotGridView::SafeDownCast( this->plotController->GetView())->GetActiveSpectraIndex() ) {
                this->referenceSpectra->GetWidget()->InsertCellTextAsInt(i, 3, 1);
            } else {
                this->referenceSpectra->GetWidget()->InsertCellTextAsInt(i, 3, 0);
            }
            this->referenceSpectra->GetWidget()->SetCellWindowCommandToCheckButton(i, 3);
        } else if( i== 0 ) {
        	// No input so reset
            this->referenceSpectra->GetWidget()->DeleteAllRows();
        }

    }



}

/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicDataWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    // Respond to a selection change in the overlay view
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
    int newActiveIndex = -1;
    for( int i = 0; i < this->referenceSpectra->GetWidget()->GetNumberOfRows(); i++) {

        int visible = this->referenceSpectra->GetWidget()->GetCellTextAsInt(i,1);
        int active = this->referenceSpectra->GetWidget()->GetCellTextAsInt(i,3);
        if( active == 1 && i != svkPlotGridView::SafeDownCast( this->plotController->GetView())->GetActiveSpectraIndex() ) {
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
        this->sivicController->SetActiveSpectra( newActiveIndex );
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

