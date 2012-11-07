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
 */



#include <sivicImageDataWidget.h>
#include <vtkSivicController.h>

vtkStandardNewMacro( sivicImageDataWidget );
vtkCxxRevisionMacro( sivicImageDataWidget, "$Revision$");


/*! 
 *  Constructor
 */
sivicImageDataWidget::sivicImageDataWidget()
{

    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgress );
    this->progressCallback->SetClientData( (void*)this );
    this->imageList = NULL;
    this->activeIndex = 0;
    this->modelModifiedCB = vtkCallbackCommand::New();
    this->modelModifiedCB->SetCallback( ModelModifiedCallback );
    this->modelModifiedCB->SetClientData( (void*)this );
}


/*! 
 *  Destructor
 */
sivicImageDataWidget::~sivicImageDataWidget()
{
    if( this->imageList != NULL ) {
        this->imageList->Delete();
        this->imageList = NULL;
    }

}

void sivicImageDataWidget::SetModel( svkDataModel* model )
{
	Superclass::SetModel(model);
	if( this->model != NULL ) {
		this->model->AddObserver(vtkCommand::ModifiedEvent, modelModifiedCB);
	}
}


/*! 
 *  Method in superclass to be overriden to add our custom widgets.
 */
void sivicImageDataWidget::CreateWidget()
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

    this->imageList = vtkKWMultiColumnListWithScrollbars::New();
    this->imageList->SetParent(this);
    this->imageList->Create();
    //this->imageList->GetWidget()->MovableColumnsOn();
    this->imageList->GetWidget()->SetWidth(0);
    this->imageList->GetWidget()->SetHeight(3);
    int col_index;
    col_index = this->imageList->GetWidget()->AddColumn("Name");
    col_index = this->imageList->GetWidget()->AddColumn("Description");
    col_index = this->imageList->GetWidget()->AddColumn("Viewed As");
    col_index = this->imageList->GetWidget()->AddColumn("Filename");
    col_index = this->imageList->GetWidget()->AddColumn("Model Name");
    //col_index = this->imageList->GetWidget()->AddColumn("Filename");

    this->Script("pack %s -expand y -fill both", this->imageList->GetWidgetName(),   0);

    //  Callbacks
    //this->AddCallbackCommandObserver( this->imageList->GetWidget(), vtkKWMultiColumnList::CellUpdatedEvent );
    //this->AddCallbackCommandObserver( this->imageList->GetWidget(), vtkKWMultiColumnList::SelectionChangedEvent );
    this->imageList->GetWidget()->SetRightClickCommand( this->sivicController, "DisplayImageDataInfo");
    this->imageList->GetWidget()->SetUneditableCellDoubleClickCommand( this->sivicController, "DisplayImageDataInfo");
}


/*!
 * Sets the filename for a given row.
 *
 * @param row
 * @param filename
 */
void sivicImageDataWidget::SetFilename( int row, string filename )
{
    this->imageList->GetWidget()->InsertCellText(row, 4, svkUtils::GetFilenameFromFullPath(filename).c_str());
}


/*!
 *
 *  Updates the list of loaded datasets.
 */
void sivicImageDataWidget::UpdateReferenceImageList( )
{
    int numImages = 0;
    // We start at one since the first plot index is the active spectra above...
    map<string, svkImageData*> allDataObjects = this->model->GetAllDataObjects();
    map<string,svkImageData*>::iterator it = allDataObjects.begin();
    int counter = 0;
    this->imageList->GetWidget()->DeleteAllRows();
    for( map<string, svkImageData*>::iterator iter = allDataObjects.begin();
        iter != allDataObjects.end(); ++iter) {
        svkImageData* image = iter->second;
        if( iter->first == "AnatomicalData" || iter->first == "MetaboliteData" || iter->first == "OverlayData"){
        	continue;
        }
        if( image != NULL && image->IsA("svkMriImageData") ) {
			string modelName = svkUtils::GetFilenameFromFullPath(iter->first);
			string seriesDescription = "";
			if( image->GetDcmHeader()->ElementExists("SeriesDescription")) {
				seriesDescription = image->GetDcmHeader()->GetStringValue("SeriesDescription");
			}
			string filename = svkUtils::GetFilenameFromFullPath(this->model->GetDataFileName( iter->first ));
            this->imageList->GetWidget()->InsertCellText(counter, 0, modelName.c_str());
            this->imageList->GetWidget()->InsertCellText(counter, 1, seriesDescription.c_str());
            string viewedAs = " ";
            if( image == this->model->GetDataObject("AnatomicalData")) {
            	viewedAs.append( "Reference Image ");
            }
            if( image == this->model->GetDataObject("OverlayData")) {
            	viewedAs.append( "Image Overlay ");
            }
            if( image == this->model->GetDataObject("MetaboliteData")) {
            	if( this->model->DataExists("OverlayData")) {
					viewedAs.append( "4D Overlay ");
            	} else {
					viewedAs.append( "Both Overlays ");
            	}
            }
            this->imageList->GetWidget()->InsertCellText(counter, 2, viewedAs.c_str());
            this->imageList->GetWidget()->InsertCellText(counter, 3, filename.c_str());
            this->imageList->GetWidget()->InsertCellText(counter, 4, iter->first.c_str());
            counter++;
        }
    }
	this->imageList->GetWidget()->SortByColumnDecreasingOrder(2);
}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicImageDataWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
    cout << "Proccessing callback command event... event:" << event << endl;
}


/*!
 * Updates the progress info during processing.
 *
 * @param subject
 * @param
 * @param thisObject
 * @param callData
 */
void sivicImageDataWidget::UpdateProgress(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 100.0*(*(double*)(callData)) );
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->SetStatusText(
    static_cast<vtkAlgorithm*>(subject)->GetProgressText() );

}


void sivicImageDataWidget::ModelModifiedCallback(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    sivicImageDataWidget* imageDataWidget = static_cast<sivicImageDataWidget*>(thisObject);
	imageDataWidget->UpdateReferenceImageList();
}
