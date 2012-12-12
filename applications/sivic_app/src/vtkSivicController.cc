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
#define DEBUG 0


#include <vtkSivicController.h>
#include <vtksys/SystemTools.hxx>
#include <vtkKWToolbarSet.h>
#include <vtkKWToolbar.h>
#include <vtkKWPushButtonWithMenu.h>


using namespace svk; 


vtkStandardNewMacro( vtkSivicController );
vtkCxxRevisionMacro( vtkSivicController, "$Revision$");

//static int nearestInt(float x); 

//! Constructor
vtkSivicController::vtkSivicController()
{
    this->spectraData = NULL;
    this->plotController    = svkPlotGridViewController::New();
    this->overlayController = svkOverlayViewController::New();
    this->imageViewWidget = NULL;
    this->spectraViewWidget = NULL;
    this->windowLevelWidget = NULL;
    this->overlayWindowLevelWidget = NULL;
    this->preferencesWidget = NULL;
    this->spectraRangeWidget = NULL;
    this->viewRenderingWidget = NULL;
    this->processingWidget = NULL;
    this->quantificationWidget = NULL;
    this->windowLevelWindow = NULL;
    this->preferencesWindow = NULL;
    this->thresholdType = "Quantity";
    this->mainWindow = NULL;
    this->secondaryCaptureFormatter = svkSecondaryCaptureFormatter::New();
    this->secondaryCaptureFormatter->SetPlotController( this->plotController );
    this->secondaryCaptureFormatter->SetOverlayController( this->overlayController );
    this->secondaryCaptureFormatter->SetSivicController( this );
    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgress );
    this->progressCallback->SetClientData( (void*)this );
    this->exitSivicCallback = vtkCallbackCommand::New();
    this->exitSivicCallback->SetCallback( ExitSivic );
    this->exitSivicCallback->SetClientData( (void*)this );
    this->synchronizeVolumes = true;

    
}


//! Destructor
vtkSivicController::~vtkSivicController()
{
    if( this->overlayController != NULL ) {
        this->overlayController->Delete();
        this->overlayController = NULL;
    }

    if( this->plotController != NULL ) {
        this->plotController->Delete();
        this->plotController = NULL;
    }

    if( this->secondaryCaptureFormatter != NULL ) {
        this->secondaryCaptureFormatter->Delete();
        this->secondaryCaptureFormatter = NULL;
    }

    if( this->windowLevelWindow != NULL ) {
        this->windowLevelWindow->Delete();
        this->windowLevelWindow = NULL;
    }

    if( this->preferencesWindow != NULL ) {
        this->preferencesWindow->Delete();
        this->preferencesWindow = NULL;
    }


}


//! Set the slice of all Controllers
void vtkSivicController::SetSlice( int slice, bool centerImage )
{
    int toggleDraw = this->GetDraw();
    if( toggleDraw ) {
    	this->DrawOff();
    }
    this->plotController->SetSlice(slice);
    this->overlayController->SetSlice(slice, centerImage);
    this->viewRenderingWidget->ResetInfoText();
    if( this->model->DataExists("AnatomicalData") && this->overlayController->IsImageInsideSpectra()) {
        if( this->orientation == "AXIAL" ) {
            this->imageViewWidget->axialSlider->SetValue( this->overlayController->GetImageSlice( svkDcmHeader::AXIAL )+1 ); 
        } else if ( this->orientation == "CORONAL" ) {
            this->imageViewWidget->coronalSlider->SetValue( this->overlayController->GetImageSlice( svkDcmHeader::CORONAL )+1 ); 
        } else if ( this->orientation == "SAGITTAL" ) {
            this->imageViewWidget->sagittalSlider->SetValue( this->overlayController->GetImageSlice( svkDcmHeader::SAGITTAL )+1 ); 
        }
    }

    if( toggleDraw ) {
    	this->DrawOn();
    }
}


void vtkSivicController::TurnOffPlotView() 
{
    this->plotController->GetView()->GetRenderer(svkPlotGridView::PRIMARY)->RemoveViewProp(
                                       this->plotController->GetView()->GetProp(svkPlotGridView::PLOT_LINES));
    this->plotController->GetView()->GetRenderer(svkPlotGridView::PRIMARY)->RemoveViewProp(
                                       this->plotController->GetView()->GetProp(svkPlotGridView::VOL_SELECTION));
    this->plotController->GetView()->GetRenderer(svkPlotGridView::PRIMARY)->RemoveViewProp(
                                       this->plotController->GetView()->GetProp(svkPlotGridView::PLOT_GRID));
    this->plotController->GetView()->GetRenderer(svkPlotGridView::PRIMARY)->RemoveViewProp(
                                       this->plotController->GetView()->GetProp(svkPlotGridView::OVERLAY_IMAGE));
    this->plotController->GetView()->GetRenderer(svkPlotGridView::PRIMARY)->RemoveViewProp(
                                       this->plotController->GetView()->GetProp(svkPlotGridView::OVERLAY_TEXT));
}


void vtkSivicController::TurnOnPlotView() 
{
    this->plotController->GetView()->GetRenderer(svkPlotGridView::PRIMARY)->AddActor(
                                       this->plotController->GetView()->GetProp(svkPlotGridView::PLOT_LINES));
    string acquisitionType = "UNKNOWN";
    svk4DImageData* activeData = this->GetActive4DImageData();
    if( activeData != NULL && activeData->IsA("svkMrsImageData") ) {
        acquisitionType = this->model->GetDataObject("SpectroscopicData")->GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
    }
    // We don't want to turn on the press box if its single voxel
    if( acquisitionType != "SINGLE VOXEL" ) {
        this->plotController->GetView()->GetRenderer(svkPlotGridView::PRIMARY)->AddActor(
                                       this->plotController->GetView()->GetProp(svkPlotGridView::VOL_SELECTION));
    }
    this->plotController->GetView()->GetRenderer(svkPlotGridView::PRIMARY)->AddActor(
                                       this->plotController->GetView()->GetProp(svkPlotGridView::PLOT_GRID));
    this->plotController->GetView()->GetRenderer(svkPlotGridView::PRIMARY)->AddActor(
                                       this->plotController->GetView()->GetProp(svkPlotGridView::OVERLAY_IMAGE));
    this->plotController->GetView()->GetRenderer(svkPlotGridView::PRIMARY)->AddActor(
                                       this->plotController->GetView()->GetProp(svkPlotGridView::OVERLAY_TEXT));

}


/*!
 *  Sets the active spectra. This is the index of the spectra as loaded into the
 *  plot grid.
 *
 * @param index
 */
void vtkSivicController::SetActive4DImageData( int index )
{
    svk4DImageData* oldData = this->GetActive4DImageData();
    if( oldData != NULL && index >= 0 ) {
        if( index <= svkPlotGridView::SafeDownCast(this->plotController->GetView())->GetNumberOfReferencePlots()) {
            int toggleDraw = this->GetDraw();
            if( toggleDraw ) {
			    this->DrawOff();
            }
            svkPlotGridView::SafeDownCast( this->plotController->GetView() )->SetActivePlotIndex( index );
            svkImageData* newData = svkPlotGridView::SafeDownCast(plotController->GetView())->GetActivePlot();
            if( newData->IsA("svkMrsImageData")) {
                this->model->ChangeDataObject("SpectroscopicData", newData );
            } else {
                this->model->ChangeDataObject("4DImageData", newData );
            }
            this->processingWidget->InitializePhaser();
            //  assume that only one dimensin is > 1 to determine number of volumes for slider range: 
            int numVolumes; 
            int channels = svkMrsImageData::SafeDownCast( newData )->GetDcmHeader()->GetNumberOfCoils();
            int timePts = svkMrsImageData::SafeDownCast( newData )->GetDcmHeader()->GetNumberOfTimePoints();
            if ( channels == 1 ) {
                numVolumes = timePts; 
            } else {                    
                numVolumes = channels; 
            }
            this->spectraViewWidget->channelSlider->SetRange( 1, channels);
            this->spectraViewWidget->channelSlider->SetValue( svkPlotGridView::SafeDownCast( this->plotController->GetView() )->GetVolumeIndex(svkMrsImageData::CHANNEL) + 1 );
            int timePoints = svkMrsImageData::SafeDownCast( newData )->GetDcmHeader()->GetNumberOfTimePoints();
            this->spectraViewWidget->timePointSlider->SetRange( 1, timePoints);
            this->spectraViewWidget->timePointSlider->SetValue( svkPlotGridView::SafeDownCast( this->plotController->GetView() )->GetVolumeIndex(svkMrsImageData::TIMEPOINT) + 1 );
            this->EnableWidgets();
            if( this->model->DataExists("AnatomicalData") ) {
                this->overlayController->SetInput( newData, svkOverlayView::MR4D );
                this->overlayController->SetTlcBrc( this->plotController->GetTlcBrc() );
            }
            this->SetPreferencesFromRegistry();
            if( toggleDraw ) {
                this->DrawOn();
            }
            this->SetComponentCallback( svkPlotGridView::SafeDownCast(this->plotController->GetView())->GetActiveComponent());
        }
    }
}


//! Set the slice of all Controllers
void vtkSivicController::SetImageSlice( int slice, string orientation )
{
    int toggleDraw = this->GetDraw();
    if( toggleDraw ) {
        this->DrawOff();
    }
    if( orientation == "AXIAL" ) {
        this->overlayController->SetSlice( slice, svkDcmHeader::AXIAL);
    } else if ( orientation == "CORONAL" ) {
        this->overlayController->SetSlice( slice, svkDcmHeader::CORONAL);
    } else if ( orientation == "SAGITTAL" ) {
        this->overlayController->SetSlice( slice, svkDcmHeader::SAGITTAL);
    }
        // Check to see if the image is inside the spectra
    if( this->model->DataExists("AnatomicalData") ) {
        if( !this->overlayController->IsImageInsideSpectra() ) {
            this->TurnOffPlotView();
        } else if( !this->plotController->GetView()->GetRenderer(svkPlotGridView::PRIMARY)->HasViewProp(
                                           this->plotController->GetView()->GetProp(svkPlotGridView::PLOT_LINES)) ) {
            this->TurnOnPlotView();
        }
    }
    if( this->overlayController->GetSlice() != this->plotController->GetSlice()) {
        this->spectraViewWidget->SetCenterImage(false);
        this->spectraViewWidget->sliceSlider->SetValue( this->overlayController->GetSlice() + 1);
        this->spectraViewWidget->SetCenterImage(true);
    }
    if( toggleDraw ) {
        this->DrawOn();
    }
}


//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetApplication( vtkKWApplication* app)
{
    this->app = app;
    
}


//! Sets the main window. All other window will be closed when this window is closed
void vtkSivicController::SetMainWindow( vtkKWWindowBase* mainWindow )
{
    this->mainWindow = mainWindow;
    if(mainWindow != NULL &&  !mainWindow->HasObserver(vtkKWWindowBase::WindowClosingEvent,exitSivicCallback )) {
        mainWindow->AddObserver(vtkKWWindowBase::WindowClosingEvent, exitSivicCallback);
    }

}


//! Sets this widget controllers view, also passes along its model
vtkKWApplication* vtkSivicController::GetApplication( )
{
    return this->app;
}


//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetViewRenderingWidget( sivicViewRenderingWidget* viewRenderingWidget)
{
    this->viewRenderingWidget = viewRenderingWidget;
    this->viewRenderingWidget->SetModel(this->model);
}


//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetProcessingWidget( sivicProcessingWidget* processingWidget)
{
    this->processingWidget = processingWidget;
    this->processingWidget->SetModel(this->model);
}


//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetPreprocessingWidget( sivicPreprocessingWidget* preprocessingWidget)
{
    this->preprocessingWidget = preprocessingWidget;
    this->preprocessingWidget->SetModel(this->model);
}


//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetDataWidget( sivicDataWidget* dataWidget)
{
    this->dataWidget = dataWidget;
    this->dataWidget->SetModel(this->model);
}

//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetImageDataWidget( sivicImageDataWidget* imageDataWidget)
{
    this->imageDataWidget = imageDataWidget;
    this->imageDataWidget->SetModel(this->model);
}


//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetQuantificationWidget( sivicQuantificationWidget* quantificationWidget)
{
    this->quantificationWidget = quantificationWidget;
    this->quantificationWidget->SetModel(this->model);
}


//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetCombineWidget( sivicCombineWidget* combineWidget)
{
    this->combineWidget = combineWidget;
    this->combineWidget->SetModel(this->model);
}


//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetDSCWidget( sivicDSCWidget* dscWidget )
{
    this->dscWidget = dscWidget;
    this->dscWidget->SetModel(this->model);
}


//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetImageViewWidget( sivicImageViewWidget* imageViewWidget )
{
    this->imageViewWidget = imageViewWidget;
    this->imageViewWidget->SetModel(this->model);
}


//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetSpectraRangeWidget( sivicSpectraRangeWidget* spectraRangeWidget )
{
    this->spectraRangeWidget = spectraRangeWidget;
    this->spectraRangeWidget->SetModel(this->model);
}

//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetSpectraViewWidget( sivicSpectraViewWidget* spectraViewWidget )
{
    this->spectraViewWidget = spectraViewWidget;
    this->spectraViewWidget->SetModel(this->model);
}


//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetWindowLevelWidget( sivicWindowLevelWidget* windowLevelWidget )
{
    this->windowLevelWidget = windowLevelWidget;
    this->windowLevelWidget->SetModel(this->model);
}

//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetPreferencesWidget( sivicPreferencesWidget* preferencesWidget )
{
    this->preferencesWidget = preferencesWidget;
    this->preferencesWidget->SetModel(this->model);
}

//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetVoxelTaggingWidget( sivicVoxelTaggingWidget* voxelTaggingWidget )
{
    this->voxelTaggingWidget = voxelTaggingWidget;
    this->voxelTaggingWidget->SetModel(this->model);
}

//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetOverlayWindowLevelWidget( sivicWindowLevelWidget* overlayWindowLevelWidget )
{
    this->overlayWindowLevelWidget = overlayWindowLevelWidget;
    this->overlayWindowLevelWidget->SetModel(this->model);
}


/*!    Open a file.    */
void vtkSivicController::ResetApplication( )
{
	int toggleDraw = this->GetDraw();
	if( toggleDraw ) {
		this->DrawOff();
	}
    model->RemoveAllDataObjects();
    this->overlayController->Reset();
    this->plotController->Reset();
    this->viewRenderingWidget->ResetInfoText();
    this->dataWidget->UpdateReferenceSpectraList();
    this->DisableWidgets();
	if( toggleDraw ) {
		this->DrawOn();
	}

}


void vtkSivicController::OpenImage( const char* fileName, bool onlyReadOneInputFile )
{
    // Lets check to see if the file exists 
	if(!svkUtils::FilePathExists(fileName)) {
        this->PopupMessage(" File does not exist!"); 
        return;
    }

    string stringFilename(fileName);
		    cout << "Attempting to read  |" << stringFilename << "|" << endl;
    svkImageData* newData = this->model->AddFileToModel( stringFilename, stringFilename, onlyReadOneInputFile );
    this->OpenImage( newData, stringFilename );

}

void vtkSivicController::OpenImage( svkImageData* data, string stringFilename )
{
    svkImageData* oldData = this->model->GetDataObject( "AnatomicalData" );
    if (data == NULL) {
        this->PopupMessage( "UNSUPPORTED FILE TYPE!");
    } else if( !data->IsA("svkMriImageData") ) {
        this->PopupMessage("ERROR: Incorrect data type, data must be an image."); 
        return;
    } else {
        string resultInfo; 
        svk4DImageData* activeData = this->GetActive4DImageData();
        if( activeData != NULL ) {
            svkDataValidator* validator = svkDataValidator::New(); 
            bool valid = validator->AreDataCompatible( data, activeData);
			if ( !valid ) {
				if ( validator->IsOnlyError(svkDataValidator::INVALID_DATA_ORIENTATION)) {
					valid = true; // We can handle orientation mismatch so lets ignore that error.
				} else {
					resultInfo = validator->resultInfo;
				}
			}
        }


        //  Precheck to see if valdation errors should be overridden:
        if( resultInfo.compare("") != 0 ) {

            string resultInfoMsg  = "WARNING: Datasets may not be compatible! \n";
            resultInfoMsg += "Do you want to attempt to display them anyway? \n";
            resultInfoMsg += "Info:\n"; 
            resultInfoMsg += resultInfo;
            int dialogStatus = this->PopupMessage( resultInfoMsg, vtkKWMessageDialog::StyleYesNo ); 

            //  If user wants to continue anyway, unset the info results 
            if ( dialogStatus == 2 ) {
                resultInfo = "";      
                this->overlayController->GetView()->ValidationOff();
                this->plotController->GetView()->ValidationOff();
            }

        } 

        if( strcmp( resultInfo.c_str(), "" ) == 0 && data != NULL ) {
            int toggleDraw = this->GetDraw();
            if( toggleDraw ) {
                this->DrawOff();
            }
            if( oldData != NULL) {
                this->model->ChangeDataObject( "AnatomicalData", data );
                this->model->SetDataFileName( "AnatomicalData", stringFilename );
                this->overlayController->SetInput( data );
            } else {
                this->model->AddDataObject( "AnatomicalData", data );
                this->model->SetDataFileName( "AnatomicalData", stringFilename );
                this->overlayController->SetInput( data );
                this->overlayController->ResetWindowLevel();
                this->overlayController->HighlightSelectionVoxels();
            }
            this->UpdateModelForReslicedImage("AnatomicalData");
            this->SetPreferencesFromRegistry();
            int* extent = data->GetExtent();
            int firstSlice;
            int lastSlice;
            firstSlice = data->GetFirstSlice( svkDcmHeader::AXIAL );
            lastSlice = data->GetLastSlice( svkDcmHeader::AXIAL );
            this->imageViewWidget->axialSlider->SetRange( firstSlice + 1, lastSlice + 1); 
            if( oldData == NULL ) {
                this->imageViewWidget->axialSlider->SetValue( ( lastSlice - firstSlice ) / 2 + 1);
            } else if( !this->GetActive4DImageData() || this->orientation != "AXIAL" ) {
                this->imageViewWidget->axialSlider->GetWidget()->InvokeEvent(vtkKWEntry::EntryValueChangedEvent); 
            }
            firstSlice = data->GetFirstSlice( svkDcmHeader::CORONAL );
            lastSlice = data->GetLastSlice( svkDcmHeader::CORONAL );
            this->imageViewWidget->coronalSlider->SetRange( firstSlice + 1, lastSlice + 1); 
            if( oldData == NULL ) {
                this->imageViewWidget->coronalSlider->SetValue( ( lastSlice - firstSlice ) / 2 + 1);
            } else if( !this->GetActive4DImageData() || this->orientation != "CORONAL" ){
                this->imageViewWidget->coronalSlider->GetWidget()->InvokeEvent(vtkKWEntry::EntryValueChangedEvent); 
            }
            firstSlice = data->GetFirstSlice( svkDcmHeader::SAGITTAL );
            lastSlice = data->GetLastSlice( svkDcmHeader::SAGITTAL );
            this->imageViewWidget->sagittalSlider->SetRange( firstSlice + 1, lastSlice + 1); 
            if( oldData == NULL ) {
                this->imageViewWidget->sagittalSlider->SetValue( ( lastSlice - firstSlice ) / 2 + 1);
            } else if( !this->GetActive4DImageData() || this->orientation != "SAGITTAL" ){
                this->imageViewWidget->sagittalSlider->GetWidget()->InvokeEvent(vtkKWEntry::EntryValueChangedEvent); 
            }
            this->imageViewWidget->volumeSlider->SetRange( 1, data->GetPointData()->GetNumberOfArrays());
            this->imageViewWidget->volumeSlider->SetValue( 1 );

            if( this->GetActive4DImageData() != NULL ) {
                this->overlayController->SetTlcBrc( plotController->GetTlcBrc() );
                this->spectraViewWidget->sliceSlider->GetWidget()->InvokeEvent( vtkKWEntry::EntryValueChangedEvent); 
            }
            if( oldData == NULL ) {
                switch( data->GetDcmHeader()->GetOrientationType() ) {
                    case svkDcmHeader::AXIAL:
                        this->SetOrientation( "AXIAL" );
                        break;
                    case svkDcmHeader::CORONAL:
                        this->SetOrientation( "CORONAL" );
                        break;
                    case svkDcmHeader::SAGITTAL:
                        this->SetOrientation( "SAGITTAL" );
                        break;
                }
            }
            double* pixelRange = this->model->GetDataObject("AnatomicalData")->GetPointData()->GetArray(0)->GetRange();
            double window = this->overlayController->GetWindow();
            double level = this->overlayController->GetLevel();
            this->windowLevelWidget->SetLevelRange( pixelRange ); 
            this->windowLevelWidget->SetLevel( level ); 
            this->windowLevelWidget->SetWindowRange( 0, pixelRange[1] - pixelRange[0] ); 
            this->windowLevelWidget->SetWindow( window ); 
            this->viewRenderingWidget->ResetInfoText();
            if( toggleDraw ) {
                this->DrawOn();
            }

        } else {

            resultInfo = "ERROR: Could not load dataset!\nInfo:\n";
            this->PopupMessage( resultInfo ); 

        }

        if( oldData == NULL ) {
            svkOverlayView::SafeDownCast( this->overlayController->GetView() )->AlignCamera();
            this->UseSelectionStyle();
        } else if( this->overlayController->GetCurrentStyle() == svkOverlayViewController::ROTATION ) {
            this->UseRotationStyle();
        }
    }
}


/*!
 *  Opens a 4D image data object. This could be spectra or a dynamic trace.
 *
 * @param newData
 * @param stringFilename
 * @param oldData
 * @param onlyReadOneInputFile
 */
void vtkSivicController::Open4DImage( svkImageData* newData,  string stringFilename, svkImageData* oldData )
{

    int toggleDraw = this->GetDraw();
    if( toggleDraw ) {
        this->DrawOff();
    }
    string objectName;
    if( newData->IsA("svkMrsImageData")) {
        objectName = "SpectroscopicData";
    } else if (newData->IsA("svkMriImageData")) {
        newData->AddObserver(vtkCommand::ProgressEvent, progressCallback);
        svk4DImageData* cellRep  = svkMriImageData::SafeDownCast(newData)->GetCellDataRepresentation();
        newData->RemoveObserver(progressCallback);
        newData = cellRep;
        objectName = "4DImageData";
        // If we are setting the
        this->spectraRangeWidget->componentSelectBox->SetValue( "real");
        this->SetComponentCallback( svkImageData::REAL);
        this->spectraRangeWidget->SetSpecUnitsCallback(svkSpecPoint::PTS);
    }
    string resultInfo;
    string plotViewResultInfo = this->plotController->GetDataCompatibility( newData, svkPlotGridView::MR4D );
    string overlayViewResultInfo = this->overlayController->GetDataCompatibility( newData, svkOverlayView::MR4D );
    svkDataValidator* validator = svkDataValidator::New(); 
    string validatorResultInfo;
    if( this->model->DataExists( "AnatomicalData" ) ) {
        bool valid = validator->AreDataCompatible( newData, this->model->GetDataObject( "AnatomicalData" )); 
        if ( !valid ) {
        	if ( validator->IsOnlyError(svkDataValidator::INVALID_DATA_ORIENTATION)) {
        		valid = true; // We can handle orientation mismatch so lets ignore that error.
			} else {
				validatorResultInfo = validator->resultInfo;
			}
        }
    }

    //  Precheck to see if valdation errors should be overridden:
    if( overlayViewResultInfo.compare("") != 0 || 
        plotViewResultInfo.compare("") != 0    || 
        validatorResultInfo.compare("") != 0 ) {

        resultInfo  = "ERROR: Dataset is not compatible! \n\n"; 
        resultInfo += "Do you want to attempt to display them anyway? \n\n"; 
        resultInfo += "Info:\n"; 
        resultInfo += plotViewResultInfo;
        resultInfo += overlayViewResultInfo;
        resultInfo += validatorResultInfo;
        int dialogStatus = this->PopupMessage( resultInfo, vtkKWMessageDialog::StyleYesNo ); 

        //  If user wants to continue anyway, unset the info results 
        if ( dialogStatus == 2 ) {
            plotViewResultInfo = "";      
            overlayViewResultInfo = "";      
            validatorResultInfo = "";      
            this->overlayController->GetView()->ValidationOff();
            this->plotController->GetView()->ValidationOff();
        } 

    }  

    if ( overlayViewResultInfo.compare("") == 0 && 
        plotViewResultInfo.compare("") == 0 && 
        validatorResultInfo.compare("") == 0 && 
        newData != NULL )  {


        // If the 4D image file is already in the model
        int* tlcBrc = NULL; 
        if( oldData != NULL ) {
            // We need to copy the tlc brc so it carries over to the new data set.
            tlcBrc = new int[2];
            memcpy( tlcBrc, this->plotController->GetTlcBrc(), 2*sizeof(int) );
            this->model->ChangeDataObject( objectName, newData );
            this->model->SetDataFileName( objectName, stringFilename );

        } else {
            this->model->AddDataObject( objectName, newData );
            this->model->SetDataFileName( objectName, stringFilename );
        }

        // Now we can update the sliders based on the image data properties
        spectraData = static_cast<vtkImageData*>(newData );
        int* extent = newData->GetExtent();

        this->plotController->SetInput( newData ); 
        this->dataWidget->UpdateReferenceSpectraList();
        this->dataWidget->SetFilename( 0, stringFilename);

        /*
         * Lets reset the orientation to match the new dataset. If there was
         * no dataset previously loaded then reset everything, otherwise we
         * only reset the orientation.
         */
        bool useFullFrequencyRange = 0;
        bool useFullAmplitudeRange = 0;
        bool resetAmplitude = 0;
        bool resetFrequency = 0;
        if( oldData != NULL ) {
            this->SetOrientation( this->orientation.c_str() );
        } else {
            resetAmplitude = 1;
            resetFrequency = 1;
            switch( newData->GetDcmHeader()->GetOrientationType() ) {
                case svkDcmHeader::AXIAL:
                    this->SetOrientation( "AXIAL" );
                    break;
                case svkDcmHeader::CORONAL:
                    this->SetOrientation( "CORONAL" );
                    break;
                case svkDcmHeader::SAGITTAL:
                    this->SetOrientation( "SAGITTAL" );
                    break;
            }
        }

        // If the tlcBrc have not been set then lets choose the center slice.
        this->overlayController->SetInput( newData, svkOverlayView::MR4D );
		this->UpdateModelForReslicedImage("AnatomicalData");
		this->UpdateModelForReslicedImage("OverlayData");
        if( tlcBrc == NULL ) {
            int firstSlice = newData->GetFirstSlice( newData->GetDcmHeader()->GetOrientationType() );
            int lastSlice = newData->GetLastSlice( newData->GetDcmHeader()->GetOrientationType() );
            this->spectraViewWidget->sliceSlider->SetRange( firstSlice + 1, lastSlice + 1); 
            this->spectraViewWidget->sliceSlider->SetValue( ( lastSlice - firstSlice ) / 2 + 1);
            int channels = newData->GetDcmHeader()->GetNumberOfCoils();
            this->spectraViewWidget->channelSlider->SetRange( 1, channels); 
            this->spectraViewWidget->channelSlider->SetValue( 1 );
            int timePoints = newData->GetDcmHeader()->GetNumberOfTimePoints();
            this->spectraViewWidget->timePointSlider->SetRange( 1, timePoints); 
            this->spectraViewWidget->timePointSlider->SetValue( 1 );
            this->plotController->SetSlice( ( lastSlice - firstSlice ) / 2 ); 
            this->overlayController->SetSlice( ( lastSlice - firstSlice ) / 2 ); 
        }

        /*
         *  In case the slice is being change we want to calculate the frequency range after the slice is chosen
         *  because the chosen range is based on the current slice.
         */
		this->ResetRange( useFullFrequencyRange, useFullAmplitudeRange, resetAmplitude, resetFrequency );
       
        
        this->SetPreferencesFromRegistry();

        this->processingWidget->InitializePhaser();

        if( newData->IsA("svkMrsImageData")) {
            this->spectraRangeWidget->point->SetDcmHeader( newData->GetDcmHeader() );
        }


        svkImageData* metData = this->model->GetDataObject("MetaboliteData");
        svkImageData* overlayData = this->model->GetDataObject("OverlayData");

        // Check to see if any currently loaded overlays are invalid in the overlayController

        string compatibility = ""; 
        if( overlayData != NULL ) {
            compatibility = this->overlayController->GetDataCompatibility( overlayData, svkOverlayView::OVERLAY );
        } else if( metData != NULL ) { // If there was no overlay data check the metabolite data
            compatibility = this->overlayController->GetDataCompatibility( metData, svkOverlayView::OVERLAY );
        }
        if( compatibility.compare("") != 0 ) {
            string message =  "Closing image overlay due to geometric incompatibility.\n";
            message.append(compatibility);
            this->PopupMessage( message );
            overlayController->GetView()->RemoveInput(svkOverlayView::OVERLAY);                    
            this->model->RemoveDataObject("OverlayData");
        } 

        // Check to see if any currently loaded metabolites are invalid in the plotController
        if( metData != NULL ) {
            compatibility = ""; 
            compatibility = this->plotController->GetDataCompatibility( metData, svkPlotGridView::MET );
            if( compatibility.compare("") != 0 ) {
                string message =  "Closing spectra overlay due to geometric incompatibility.\n";
                message.append(compatibility);
                this->PopupMessage( message );
                plotController->GetView()->RemoveInput( svkPlotGridView::MET );                    
                this->model->RemoveDataObject("MetaboliteData");
            } 
        } else if( overlayData != NULL && oldData == NULL ) {

        	// If this is the first 4D dataset lets check to see if its metabolite data.
            compatibility = "";
            compatibility = this->plotController->GetDataCompatibility( overlayData, svkPlotGridView::MET );
            if( compatibility.compare("") == 0 ) {
				this->model->AddDataObject( "MetaboliteData", overlayData );
				this->model->SetDataFileName( "MetaboliteData",  this->model->GetDataFileName("OverlayData") );
				this->plotController->SetInput( overlayData, svkPlotGridView::MET );
            }

        }

        if( tlcBrc == NULL || !svkDataView::IsTlcBrcWithinData(newData, tlcBrc[0], tlcBrc[1]) ) {
            this->plotController->HighlightSelectionVoxels();
            this->overlayController->HighlightSelectionVoxels();
        } else {
            this->plotController->SetTlcBrc( tlcBrc ); 
            this->overlayController->SetTlcBrc( tlcBrc ); 
        }
        this->spectraRangeWidget->xSpecRange->InvokeEvent(vtkKWRange::RangeValueChangingEvent );
        this->spectraRangeWidget->ySpecRange->InvokeEvent(vtkKWRange::RangeValueChangingEvent );
        string component = (this->spectraRangeWidget->componentSelectBox->GetValue( ));
        if( component == "read" ) {
            this->SetComponentCallback( 0 );
        } else if ( component == "imag" ) {
            this->SetComponentCallback( 1 );
        } else if ( component == "mag" ) {
            this->SetComponentCallback( 2 );
        }
        this->viewRenderingWidget->ResetInfoText();

		// If there is no reference image loaded lets create a default one.
        if( !this->model->DataExists( "AnatomicalData" ) ) {
        	svkMriImageData* zeroImage = svkMriImageData::New();
        	this->GetActive4DImageData()->GetZeroImage( zeroImage );
        	this->OpenImage( zeroImage, "zeroImage");
        	zeroImage->Delete();
       }
    } else {

        resultInfo = "ERROR: Could not load dataset!\nInfo:\n"; 
        this->PopupMessage( resultInfo ); 
    }

    this->DisableWidgets();
    this->EnableWidgets();
    if( toggleDraw ) {
        this->DrawOn();
    }
}



/*!
 *  Opens a 4D image data object.
 *
 * @param fileName
 * @param onlyReadOneInputFile
 */
void vtkSivicController::Open4DImage( const char* fileName, bool onlyReadOneInputFile )
{

    // Lets check to see if the file exists 
	if(!svkUtils::FilePathExists(fileName)) {
        this->PopupMessage(" File does not exist!"); 
        return;
    }
	int toggleDraw = this->GetDraw();
	if( toggleDraw ) {
		this->DrawOff();
	}

    string stringFilename(fileName);
    svkImageData* newData = model->AddFileToModel( stringFilename, stringFilename, onlyReadOneInputFile );

    string objectName;
    if( newData != NULL && newData->IsA("svkMrsImageData")) {
        objectName = "SpectroscopicData";
    } else {
        objectName = "4DImageData";
    }
    svkImageData* oldData = model->GetDataObject(objectName);

    if (newData == NULL) {

        this->PopupMessage( "UNSUPPORTED FILE TYPE!");
	    if( toggleDraw ) {
		    this->DrawOn();
	    }
        return;
    } else {
        this->Open4DImage( newData,  stringFilename, oldData );
    }
    this->DisableWidgets();
    this->EnableWidgets();
    if( this->spectraViewWidget->sliceSlider->GetEnabled() ) {
        this->spectraViewWidget->sliceSlider->GetWidget()->InvokeEvent(vtkKWEntry::EntryValueChangedEvent); 
    }

    // Update the metabolite menu for the current spectra
#if defined( UCSF_INTERNAL )

    vector<string> names = svkUCSFUtils::GetAllMetaboliteNames();
    vector<string>::iterator it = names.begin();
    string commandString;

    vtkKWMenu* metaboliteNames = static_cast<vtkKWMenu*>(
            this->GetApplication()->GetNthWindow(0)->GetMenu()->GetNthChild(3)->GetNthChild(0)); 

    metaboliteNames->DeleteAllItems();
    if( this->model != NULL ) {
        string spectraFile = this->model->GetDataFileName("SpectroscopicData");
        while(it != names.end()) {
            if( spectraFile != "" ) {
                bool includePath = true;
                string metaboliteFileName;
                metaboliteFileName = svkUCSFUtils::GetMetaboliteFileName(
                                        this->model->GetDataFileName("SpectroscopicData"), it->c_str(), includePath );
                if( svkUtils::FilePathExists(metaboliteFileName.c_str()) ) {
                    commandString = "OpenMetabolites";
                    commandString += " \"";
                    commandString += *it;
                    commandString += "\"";
                    metaboliteNames->AddRadioButton(it->c_str(), this, commandString.c_str());
                    bool onlyOneInputFile = true;
                    this->model->AddFileToModel(*it, metaboliteFileName, onlyOneInputFile );
                }
            }
            ++it;
        }
    }

#endif
	if( toggleDraw ) {
		this->DrawOn();
	}
}


/*!
 * Given a name of a dataset in the model, this method will open it as 4D image.
 */
void vtkSivicController::Open4DImageFromModel( const char* modelObjectName )
{
	if( this->model->DataExists( modelObjectName )) {
		this->Open4DImage( this->model->GetDataObject( modelObjectName), this->model->GetDataFileName( modelObjectName ));
		this->DisableWidgets();
		this->EnableWidgets();
	}
}


/*!
 * Given a name of a dataset in the model, this method will open it as an overlay.
 */
void vtkSivicController::OpenOverlayFromModel( const char* modelObjectName )
{
	if( this->model->DataExists( modelObjectName )) {
		this->OpenOverlay( this->model->GetDataObject( modelObjectName), this->model->GetDataFileName( modelObjectName ));
		this->DisableWidgets();
		this->EnableWidgets();
	}
}


/*!
 * Given a name of a dataset in the model, this method will open it as a reference image.
 */
void vtkSivicController::OpenImageFromModel( const char* modelObjectName )
{
	cout << "Openning overlay for model object " << modelObjectName << endl;
	if( this->model->DataExists( modelObjectName )) {
		this->OpenImage( this->model->GetDataObject( modelObjectName), this->model->GetDataFileName( modelObjectName ));
		this->DisableWidgets();
		this->EnableWidgets();
	}
}


/*!
 *  Updates the model to account for the reslicing of images by the overlay view.
 *  When the overlay view loads an image it may reslice the data to
 *	// TODO: Change the overlay controller to reslice in place
 */
void vtkSivicController::UpdateModelForReslicedImage( string modelObjectName )
{
	// The overlay controller may have resliced the data, we need to make sure we get the new version of the data into the model
	svkImageData* mri = NULL;
	if( modelObjectName == "AnatomicalData") {
		mri = this->overlayController->GetView()->GetInput(svkOverlayView::MRI);
	} else if ( modelObjectName == "OverlayData") {
		mri = this->overlayController->GetView()->GetInput(svkOverlayView::OVERLAY);
	} else {
		return;
	}
	if( mri != NULL && this->model->DataExists(modelObjectName) && mri != this->model->GetDataObject( modelObjectName ) ) {
		// Lets get the original filename
		string originalFilename = this->model->GetDataFileName( modelObjectName );
		// Change the current anatomical data pointer
		this->model->ChangeDataObject( modelObjectName, mri );
		// And the pointer the current loaded data set
		this->model->ChangeDataObject( originalFilename, mri );
		this->model->SetDataFileName( originalFilename,svkUtils::GetFilenameFromFullPath(originalFilename)  );
		this->model->SetDataFileName( modelObjectName,svkUtils::GetFilenameFromFullPath(originalFilename)  );
	}

}


void vtkSivicController::OpenOverlay( svkImageData* data, string stringFilename )
{
        string resultInfo = "";
		int toggleDraw = this->GetDraw();
		if( toggleDraw ) {
			this->DrawOff();
		}

        if (data == NULL) {
            this->PopupMessage( "UNSUPPORTED FILE TYPE!");
        } else {

            resultInfo = this->overlayController->GetDataCompatibility( data, svkOverlayView::OVERLAY );
			//  Precheck to see if valdation errors should be overridden:
			if( resultInfo.compare("") != 0 ) {

				string resultInfoMsg  = "WARNING: Datasets may not be compatible! \n";
				resultInfoMsg += "Do you want to attempt to display them anyway? \n";
				resultInfoMsg += "Info:\n";
				resultInfoMsg += resultInfo;
				int dialogStatus = this->PopupMessage( resultInfoMsg, vtkKWMessageDialog::StyleYesNo );

				//  If user wants to continue anyway, unset the info results
				if ( dialogStatus == 2 ) {
					resultInfo = "";
					this->overlayController->GetView()->ValidationOff();
					this->plotController->GetView()->ValidationOff();
				}

			}
            if( strcmp( resultInfo.c_str(), "" ) == 0 ) {
            	// Lets make sure the first volume is currently the active scalars
            	data->GetPointData()->SetActiveScalars( data->GetPointData()->GetArray(0)->GetName());
                this->overlayController->SetInput( data, svkOverlayView::OVERLAY );
                resultInfo = this->plotController->GetDataCompatibility( data, svkPlotGridView::MET ); 
                string overlayDataName;
                if( strcmp( resultInfo.c_str(), "" ) == 0 ) {
                    this->plotController->SetInput( data, svkPlotGridView::MET );
                    if( this->model->DataExists( "MetaboliteData" ) ) {
                        this->model->ChangeDataObject( "MetaboliteData", data );
                        this->model->SetDataFileName( "MetaboliteData", stringFilename );
                    } else {
                        this->model->AddDataObject( "MetaboliteData", data );
                        this->model->SetDataFileName( "MetaboliteData", stringFilename );
                    }
                    overlayDataName = "MetaboliteData";
                    this->spectraViewWidget->SetSyncOverlayWL( true );
                    this->overlayWindowLevelWidget->SetSyncPlotGrid( true );
                    // We are going to deselect the metabolites since we don't know where they were loaded from
                    this->DeselectMetabolites();
                    this->viewRenderingWidget->ResetInfoText();
                    // This dataset will superseded the previously loaded overlay data
                    this->model->RemoveDataObject("OverlayData");
                } else {
                    if( this->model->DataExists( "OverlayData" ) ) {
                        this->model->ChangeDataObject( "OverlayData", data );
                        this->model->SetDataFileName( "OverlayData", stringFilename );
                    } else {
                        this->model->AddDataObject( "OverlayData", data );
                        this->model->SetDataFileName( "OverlayData", stringFilename );
                    }
					this->UpdateModelForReslicedImage("OverlayData");
					data = this->model->GetDataObject("OverlayData");
                    this->viewRenderingWidget->ResetInfoText();
                    this->spectraViewWidget->SetSyncOverlayWL( false );
                    this->overlayWindowLevelWidget->SetSyncPlotGrid( false );
                    overlayDataName = "OverlayData";
                }
                string interp = (this->imageViewWidget->interpolationBox->GetWidget()->GetValue( ));
                if( interp == "nearest neighbor" ) {
                    this->SetInterpolationCallback( 0 );
                } else if ( interp == "linear" ) {
                    this->SetInterpolationCallback( 1 );
                } else if ( interp == "sinc" ) {
                    this->SetInterpolationCallback( 2 );
                }
                string lut = (this->imageViewWidget->lutBox->GetWidget()->GetValue( ));
                if( lut == "Color LUT" ) {
                    this->SetLUTCallback( svkLookupTable::COLOR );
                } else if ( lut == "Grey LUT" ) {
                    this->SetLUTCallback( svkLookupTable::GREY_SCALE );
                } else if ( lut == "Hurd LUT" ) {
                    this->SetLUTCallback( svkLookupTable::HURD );
                } else if ( lut == "Cyan LUT" ) {
                    this->SetLUTCallback( svkLookupTable::CYAN_HOT );
                } else if ( lut == "Fire LUT" ) {
                    this->SetLUTCallback( svkLookupTable::FIRE );
                } else if ( lut == "Reverse Color LUT" ) {
                    this->SetLUTCallback( svkLookupTable::REVERSE_COLOR );
                }
                int currentVolume = static_cast<int>(this->imageViewWidget->overlayVolumeSlider->GetValue());
                int numVolumes = data->GetPointData()->GetNumberOfArrays();
                bool isImageVoxelTags = svkVoxelTaggingUtils::IsImageVoxelTagData(data);
                if( isImageVoxelTags ) {
                	this->voxelTaggingWidget->GetTagsFromData(data);
                	if( this->model->DataExists("VoxelTagData")) {
						this->model->ChangeDataObject( "VoxelTagData", data );
                	} else {
						this->model->AddDataObject( "VoxelTagData", data );
                	}
                	numVolumes = 1;
                }
                this->imageViewWidget->overlayVolumeSlider->SetRange( 1, numVolumes);
                if( currentVolume - 1 < numVolumes) {
					this->imageViewWidget->overlayVolumeSlider->SetValue( currentVolume );
                } else {
					this->imageViewWidget->overlayVolumeSlider->SetValue( 1 );
                }
				this->imageViewWidget->overlayVolumeSlider->GetWidget()->InvokeEvent(vtkKWEntry::EntryValueChangedEvent);
                double* pixelRange = data->GetPointData()->GetArray(0)->GetRange();
                double window;
                double level;
                svkMriImageData::SafeDownCast(data)->GetAutoWindowLevel( window, level );
                this->overlayWindowLevelWidget->SetLevelRange( pixelRange ); 
                this->overlayWindowLevelWidget->SetLevel( level ); 
                this->overlayWindowLevelWidget->SetWindowRange( 0, pixelRange[1] - pixelRange[0] ); 
                this->overlayWindowLevelWidget->SetWindow( window ); 
                this->overlayWindowLevelWidget->SetOverlayDataName( overlayDataName ); 
                this->imageViewWidget->colorBarButton->InvokeEvent( vtkKWCheckButton::SelectedStateChangedEvent );
                this->imageViewWidget->overlayButton->InvokeEvent( vtkKWCheckButton::SelectedStateChangedEvent );
                this->imageViewWidget->overlayOpacitySlider->GetWidget()->InvokeEvent( vtkKWEntry::EntryValueChangedEvent );
                this->imageViewWidget->overlayThresholdSlider->GetWidget()->InvokeEvent( vtkKWEntry::EntryValueChangedEvent );

            } else {
                string message = "ERROR: Dataset is not compatible and will not be loaded.\nInfo:\n";
                message += resultInfo;
                this->PopupMessage( message );
            }
        }
        this->SetThresholdType( this->thresholdType );
        this->spectraViewWidget->sliceSlider->GetWidget()->InvokeEvent(vtkKWEntry::EntryValueChangedEvent); 
		if( toggleDraw ) {
			this->DrawOn();
		}
	return;
}


/*!
 * Adds an additional 4D object.
 *
 * @param stringFilename
 */
void vtkSivicController::Add4DImageData( string stringFilename, bool onlyReadOneInputFile )
{
    if ( this->GetActive4DImageData() != NULL ) {
		//string modelName = stringFilename;
        svkImageData* data = this->model->AddFileToModel( stringFilename, stringFilename, onlyReadOneInputFile );
        this->Add4DImageData( data, stringFilename );
    } else {
        this->PopupMessage( "ERROR: Currently loading of the reference spectra before the primary spectra is not supported." );
    }

}


/*!
 * Given a name of a dataset in the model, this method will add it it as 4D image.
 */
void vtkSivicController::Add4DImageDataFromModel( const char* modelObjectName )
{
	if( this->model->DataExists( modelObjectName )) {
		this->Add4DImageData( this->model->GetDataObject( modelObjectName), this->model->GetDataFileName( modelObjectName ));
		this->DisableWidgets();
		this->EnableWidgets();
	}
}

/*!
 * Adds an additional 4D object.
 *
 * @param stringFilename
 */
void vtkSivicController::Add4DImageData( svkImageData* data, string stringFilename )
{
    if (data->IsA("svkMriImageData")) {
        data->AddObserver(vtkCommand::ProgressEvent, progressCallback);
        svk4DImageData* cellRep  = svkMriImageData::SafeDownCast(data)->GetCellDataRepresentation();
        data->RemoveObserver(progressCallback);
        data = cellRep;
    }
	if( data != NULL && data->IsA("svk4DImageData") ) {
		string resultInfo = this->plotController->GetDataCompatibility( data,  svkPlotGridView::ADDITIONAL_MR4D );
		if( resultInfo.compare("") != 0 ) {
			string message =  "ERROR: Could not load reference trace.\n Info: ";
			message.append( resultInfo );
			this->PopupMessage( message );
		} else {
			svkPlotGridView::SafeDownCast(this->plotController->GetView())->AddReferenceInput( data );
			this->dataWidget->UpdateReferenceSpectraList();
			this->dataWidget->SetFilename( svkPlotGridView::SafeDownCast(this->plotController->GetView())->GetNumberOfReferencePlots(), stringFilename);
		}
	} else {
		this->PopupMessage( "ERROR: Could not load reference spectra." );
	}
}

void vtkSivicController::DeselectMetabolites( )
{
    //   This menu doesn't exist outside of the UCSF build:
    if ( this->GetApplication()->GetNthWindow(0)->GetMenu()->GetNthChild(3) == NULL ) {
        return; 
    }

    int numItems =  static_cast<vtkKWMenu*>(
                    this->GetApplication()->GetNthWindow(0)->GetMenu()->GetNthChild(3)->GetNthChild(0)
                )->GetNumberOfItems();
    for( int i = 0; i < numItems; i++ ) {
        static_cast<vtkKWMenu*>(
                    this->GetApplication()->GetNthWindow(0)->GetMenu()->GetNthChild(3)->GetNthChild(0)
                )->DeselectItem(i); 
    }
}


void vtkSivicController::OpenOverlay( const char* fileName, bool onlyReadOneInputFile )
{

    // Lets check to see if the file exists 

	if(!svkUtils::FilePathExists(fileName)) {
        this->PopupMessage(" File does not exist!"); 
        return;
    }


    string stringFilename( fileName );
    if ( this->GetActive4DImageData() || this->model->DataExists("AnatomicalData") ) {

        svkImageData* data = this->model->AddFileToModel( stringFilename, stringFilename, onlyReadOneInputFile );
        if( data != NULL && data->IsA("svkMriImageData") ) {
            this->OpenOverlay(data, stringFilename);
        } else {
            this->PopupMessage("ERROR: Incorrect data type, data must be an image to be overlayed."); 
            return;
        }
    } else {
        this->PopupMessage( "ERROR: Currently loading of overlays before image OR spectra is not supported." );
    }
    this->DisableWidgets();
    this->EnableWidgets();
}


void vtkSivicController::OpenMetabolites( const char* metabolites )
{
    string metaboliteString(metabolites);
    string metaboliteFileName;

    if ( !(metaboliteString == "None") ) {
        if( !this->model->DataExists("MetaboliteData") || 
             metaboliteString != svkUCSFUtils::GetMetaboliteName( this->model->GetDataFileName("MetaboliteData") ) ) {

            // Currently require an image and a spectra to be loading to limit testing
            if( this->GetActive4DImageData() && this->model->DataExists("AnatomicalData") ) {

                bool includePath = true;
                string metaboliteFileName;
                metaboliteFileName += svkUCSFUtils::GetMetaboliteFileName( 
                                        this->model->GetDataFileName("SpectroscopicData"), metaboliteString, includePath );
                this->OpenOverlay( metaboliteFileName.c_str() );

                if( !this->model->DataExists("MetaboliteData")) {
                    static_cast<vtkKWMenu*>(
                        this->GetApplication()->GetNthWindow(0)->GetMenu()->GetNthChild(3)->GetNthChild(0)
                    )->SelectItem(0);
                } 
                static_cast<vtkKWMenu*>(
                    this->GetApplication()->GetNthWindow(0)->GetMenu()->GetNthChild(3)->GetNthChild(0)
                )->SelectItem(
                    svkUCSFUtils::GetMetaboliteName( this->model->GetDataFileName("MetaboliteData") ).c_str()
                );

            } else {
               this->PopupMessage( "You must load an image and a spectra before loading metabolites." ); 
            } 
        }
    }
    this->DisableWidgets();
    this->EnableWidgets();
}


void vtkSivicController::SetPreferencesFromRegistry( )
{
    // This block recals sat band colors from config file
	int toggleDraw = this->GetDraw();
	if( toggleDraw ) {
		this->DrawOff();
	}
    char registryValue[100] = "";
    this->app->GetRegistryValue( 0, "defaults", "sync_volumes", registryValue );
    if( registryValue != NULL && strcmp( registryValue, "OFF" ) == 0 ) {
    	this->synchronizeVolumes = false;
    } else {
    	this->synchronizeVolumes = true;
    }
    char satBandRed[50]="";
    this->app->GetRegistryValue( 0, "sat_bands", "red", satBandRed );
    char satBandBlue[50]="";
    this->app->GetRegistryValue( 0, "sat_bands", "blue", satBandBlue );
    char satBandGreen[50]="";
    this->app->GetRegistryValue( 0, "sat_bands", "green", satBandGreen );
    if( string( satBandRed ) != "" && string(satBandBlue) != "" && string(satBandGreen) != "" ) {
        double rgb[3];
        rgb[0] = atof( satBandRed );
        rgb[1] = atof( satBandGreen );
        rgb[2] = atof( satBandBlue );
        vtkActor::SafeDownCast(this->plotController->GetView()->GetProp( svkPlotGridView::SAT_BANDS ))
                                   ->GetProperty()->SetAmbientColor( rgb );
        vtkActor::SafeDownCast(this->overlayController->GetView()->GetProp( svkOverlayView::SAT_BANDS_AXIAL ))
                                   ->GetProperty()->SetAmbientColor( rgb );
        vtkActor::SafeDownCast(this->overlayController->GetView()->GetProp( svkOverlayView::SAT_BANDS_CORONAL ))
                                   ->GetProperty()->SetAmbientColor( rgb );
        vtkActor::SafeDownCast(this->overlayController->GetView()->GetProp( svkOverlayView::SAT_BANDS_SAGITTAL ))
                                   ->GetProperty()->SetAmbientColor( rgb );


        char satBandOpacity[50]="";
        this->app->GetRegistryValue( 0, "sat_bands", "opacity", satBandOpacity );
        if( string( satBandOpacity ) != "" ) {
            vtkActor::SafeDownCast(this->plotController->GetView()->GetProp( svkPlotGridView::SAT_BANDS ))
                                   ->GetProperty()->SetOpacity( atof( satBandOpacity ) );
            vtkActor::SafeDownCast(this->overlayController->GetView()->GetProp( svkOverlayView::SAT_BANDS_AXIAL ))
                                   ->GetProperty()->SetOpacity( atof( satBandOpacity ) );
            vtkActor::SafeDownCast(this->overlayController->GetView()->GetProp( svkOverlayView::SAT_BANDS_CORONAL ))
                                   ->GetProperty()->SetOpacity( atof( satBandOpacity ) );
            vtkActor::SafeDownCast(this->overlayController->GetView()->GetProp( svkOverlayView::SAT_BANDS_SAGITTAL ))
                                   ->GetProperty()->SetOpacity( atof( satBandOpacity ) );
            
        }

        vtkActor::SafeDownCast(this->overlayController->GetView()->GetProp( svkPlotGridView::SAT_BANDS ))
                                   ->Modified();
        vtkActor::SafeDownCast(this->overlayController->GetView()->GetProp( svkOverlayView::SAT_BANDS_AXIAL ))
                                   ->Modified();
        vtkActor::SafeDownCast(this->overlayController->GetView()->GetProp( svkOverlayView::SAT_BANDS_CORONAL ))
                                   ->Modified();
        vtkActor::SafeDownCast(this->overlayController->GetView()->GetProp( svkOverlayView::SAT_BANDS_SAGITTAL ))
                                   ->Modified();
    }

    char satBandOutlineRed[50]="";
    this->app->GetRegistryValue( 0, "sat_bands_outline", "red", satBandOutlineRed );
    char satBandOutlineBlue[50]="";
    this->app->GetRegistryValue( 0, "sat_bands_outline", "blue", satBandOutlineBlue );
    char satBandOutlineGreen[50]="";
    this->app->GetRegistryValue( 0, "sat_bands_outline", "green", satBandOutlineGreen );
    if( string(satBandOutlineRed) != "" && string(satBandOutlineBlue) != "" && string(satBandOutlineGreen) != "" ) {
        double rgb[3];
        rgb[0] = atof( satBandOutlineRed );
        rgb[1] = atof( satBandOutlineGreen );
        rgb[2] = atof( satBandOutlineBlue );

        vtkActor::SafeDownCast(this->plotController->GetView()->GetProp( svkPlotGridView::SAT_BANDS_OUTLINE ))
                                   ->GetProperty()->SetAmbientColor( rgb );

        vtkActor::SafeDownCast(this->overlayController->GetView()
                                   ->GetProp( svkOverlayView::SAT_BANDS_AXIAL_OUTLINE ))
                                   ->GetProperty()->SetAmbientColor( rgb );

        vtkActor::SafeDownCast(this->overlayController->GetView()
                                   ->GetProp( svkOverlayView::SAT_BANDS_CORONAL_OUTLINE ))
                                   ->GetProperty()->SetAmbientColor( rgb );

        vtkActor::SafeDownCast(this->overlayController->GetView()
                                   ->GetProp( svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE ))
                                   ->GetProperty()->SetAmbientColor( rgb );
        char satBandOutlineOpacity[50]="";
        this->app->GetRegistryValue( 0, "sat_bands_outline", "opacity", satBandOutlineOpacity );
        if( string( satBandOutlineOpacity ) != "" ) {
            vtkActor::SafeDownCast(this->plotController->GetView()->GetProp( svkPlotGridView::SAT_BANDS_OUTLINE ))
                                   ->GetProperty()->SetOpacity( atof( satBandOutlineOpacity ) );
            vtkActor::SafeDownCast(this->overlayController->GetView()
                                       ->GetProp( svkOverlayView::SAT_BANDS_AXIAL_OUTLINE ))
                                       ->GetProperty()->SetOpacity( atof( satBandOutlineOpacity ) );

            vtkActor::SafeDownCast(this->overlayController->GetView()
                                       ->GetProp( svkOverlayView::SAT_BANDS_CORONAL_OUTLINE ))
                                       ->GetProperty()->SetOpacity( atof( satBandOutlineOpacity ) );

            vtkActor::SafeDownCast(this->overlayController->GetView()
                                       ->GetProp( svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE ))
                                       ->GetProperty()->SetOpacity( atof( satBandOutlineOpacity ) );
            
        }
        vtkActor::SafeDownCast(this->plotController->GetView()->GetProp( svkPlotGridView::SAT_BANDS_OUTLINE ))
                                   ->Modified();

        vtkActor::SafeDownCast(this->overlayController->GetView()
                                   ->GetProp( svkOverlayView::SAT_BANDS_AXIAL_OUTLINE ))
                                   ->Modified();

        vtkActor::SafeDownCast(this->overlayController->GetView()
                                   ->GetProp( svkOverlayView::SAT_BANDS_CORONAL_OUTLINE ))
                                   ->Modified();

        vtkActor::SafeDownCast(this->overlayController->GetView()
                                   ->GetProp( svkOverlayView::SAT_BANDS_SAGITTAL_OUTLINE ))
                                   ->Modified();
    }


    char plotGridRed[50]="";
    this->app->GetRegistryValue( 0, "plot_grid", "red", plotGridRed );
    char plotGridBlue[50]="";
    this->app->GetRegistryValue( 0, "plot_grid", "blue", plotGridBlue );
    char plotGridGreen[50]="";
    this->app->GetRegistryValue( 0, "plot_grid", "green", plotGridGreen );
    if( string(plotGridRed) != "" && string(plotGridBlue) != "" && string(plotGridGreen) != "" ) {
        double rgb[3];
        rgb[0] = atof( plotGridRed );
        rgb[1] = atof( plotGridGreen );
        rgb[2] = atof( plotGridBlue );
        vtkActor::SafeDownCast(this->overlayController->GetView()
                                   ->GetProp( svkOverlayView::PLOT_GRID ))
                                   ->GetProperty()->SetDiffuseColor( rgb );
    } else {
        double rgb[3] = {0,1,0};
        vtkActor::SafeDownCast(this->overlayController->GetView()
                                   ->GetProp( svkOverlayView::PLOT_GRID ))
                                   ->GetProperty()->SetDiffuseColor( rgb );
    }

    char plotGridOpacity[50]="";
    this->app->GetRegistryValue( 0, "plot_grid", "opacity", plotGridOpacity );
    if( string(plotGridOpacity) != "" ) {
    	double opacity = atof( plotGridOpacity);
        vtkActor::SafeDownCast(this->overlayController->GetView()
                                   ->GetProp( svkOverlayView::PLOT_GRID ))
                                   ->GetProperty()->SetOpacity( opacity );

    }
    char tracePlotGridRed[50]="";
    this->app->GetRegistryValue( 0, "trace_plot_grid", "red", tracePlotGridRed );
    char tracePlotGridBlue[50]="";
    this->app->GetRegistryValue( 0, "trace_plot_grid", "blue", tracePlotGridBlue );
    char tracePlotGridGreen[50]="";
    this->app->GetRegistryValue( 0, "trace_plot_grid", "green", tracePlotGridGreen );
    if( string(tracePlotGridRed) != "" && string(tracePlotGridGreen) != "" && string(tracePlotGridBlue) != "" ) {
        double rgb[3];
        rgb[0] = atof( tracePlotGridRed );
        rgb[1] = atof( tracePlotGridGreen );
        rgb[2] = atof( tracePlotGridBlue );
        vtkActor::SafeDownCast(this->plotController->GetView()
                                   ->GetProp( svkPlotGridView::PLOT_GRID ))
                                   ->GetProperty()->SetDiffuseColor( rgb );
    } else {
        double rgb[3] = {0,1,0};
        vtkActor::SafeDownCast(this->overlayController->GetView()
                                   ->GetProp( svkPlotGridView::PLOT_GRID ))
                                   ->GetProperty()->SetDiffuseColor( rgb );
    }

    char traceLineWidth[50]="";
    this->app->GetRegistryValue( 0, "trace_lines", "width", traceLineWidth );
    if( string(traceLineWidth) != "" ) {
    	double width = atof( traceLineWidth);
    	svkPlotGridView::SafeDownCast(this->plotController->GetView())->SetPlotLineWidth( width );
    }

    char plotGridWidth[50]="";
    this->app->GetRegistryValue( 0, "plot_grid", "width", plotGridWidth );
    if( string(plotGridWidth) != "" ) {
    	double width = atof( plotGridWidth);
        vtkActor::SafeDownCast(this->overlayController->GetView()
                                   ->GetProp( svkOverlayView::PLOT_GRID ))
                                   ->GetProperty()->SetLineWidth( width );

    }

	vtkActor::SafeDownCast(this->overlayController->GetView()
							   ->GetProp( svkOverlayView::PLOT_GRID ))
							   ->Modified();

    char volSelectionRed[50]="";
    this->app->GetRegistryValue( 0, "vol_selection", "red", volSelectionRed );
    char volSelectionBlue[50]="";
    this->app->GetRegistryValue( 0, "vol_selection", "blue", volSelectionBlue );
    char volSelectionGreen[50]="";
    this->app->GetRegistryValue( 0, "vol_selection", "green", volSelectionGreen );
    if(this->overlayController->GetView()->GetProp( svkOverlayView::VOL_SELECTION) != NULL ) {
		if( string(volSelectionRed) != "" && string(volSelectionBlue) != "" && string(volSelectionGreen) != "" ) {
			double rgb[3];
			rgb[0] = atof( volSelectionRed );
			rgb[1] = atof( volSelectionGreen );
			rgb[2] = atof( volSelectionBlue );
			vtkActor::SafeDownCast(this->overlayController->GetView()
									   ->GetProp( svkOverlayView::VOL_SELECTION ))
									   ->GetProperty()->SetColor( rgb );
		}

		char volSelectionOpacity[50]="";
		this->app->GetRegistryValue( 0, "vol_selection", "opacity", volSelectionOpacity );
		if( string(volSelectionOpacity) != "" ) {
			double opacity = atof( volSelectionOpacity);
			vtkActor::SafeDownCast(this->overlayController->GetView()
									   ->GetProp( svkOverlayView::VOL_SELECTION ))
									   ->GetProperty()->SetOpacity( opacity );

		}

		char volSelectionWidth[50]="";
		this->app->GetRegistryValue( 0, "vol_selection", "width", volSelectionWidth );
		if( string(volSelectionWidth) != "" ) {
			double width = atof( volSelectionWidth);
			vtkActor::SafeDownCast(this->overlayController->GetView()
									   ->GetProp( svkOverlayView::VOL_SELECTION ))
									   ->GetProperty()->SetLineWidth( width );

		}
		vtkActor::SafeDownCast(this->overlayController->GetView()
								   ->GetProp( svkOverlayView::VOL_SELECTION ))
								   ->Modified();
    }

	double rgb[3] = {-1,-1,-1};
	sivicPreferencesWidget::GetColorFromRegistry( this->app, "image_background", rgb);
	if( rgb[0] >= 0 && rgb[1] >= 0 && rgb[2] >= 0 ) {
		cout << "Setting image background" << endl;
		this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->SetBackground(rgb);
		this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->Modified();
	}
	rgb[0] = -1;
	rgb[1] = -1;
	rgb[2] = -1;
	sivicPreferencesWidget::GetColorFromRegistry(this->app, "trace_background", rgb);
	if( rgb[0] >= 0 && rgb[1] >= 0 && rgb[2] >= 0 ) {
		cout << "Setting trace background" << endl;
		this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY)->SetBackground(rgb);
		this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY)->Modified();
	}

	if( toggleDraw ) {
		this->DrawOn();
	}


}


/*! 
 *  Open an entire exam. First an a folder called "images" will be searched for.
 *  If found, that folder will be opened first. The user will the choose the image.
 *  If the folder that was chosen is called "images", then a corresponding "spectra"
 *  folder will be searched for. If found it will be used, otherwise it will open the
 *  folder above the "images" folder used. If an "images" folder was not used to open
 *  the image, then the same folder will be opened for loading spectra.
 *
 *  NOTE: This will NOT work on windows. 
*/
void vtkSivicController::OpenExam( )
{

    int status = -1;

	string imagePathName = svkUtils::GetCurrentWorkingDirectory();
	this->ResetApplication();
    // First we open an image

    // Lets check for an images folder
	if(svkUtils::FilePathExists("images")) {
        imagePathName+= "/images";
        //cout << "Switching to image path:" << imagePathName.c_str() << endl;
        status = this->OpenFile( "image", imagePathName.c_str(), true, false );
    } else {
        status = this->OpenFile( "image", NULL, true, false );
    }
    
    if( status == vtkKWDialog::StatusCanceled ) {
        return;
    } 

	char lastPath[MAXPATHLEN];
    // Lets retrieve the path used to open the image
    this->app->GetRegistryValue( 0, "RunTime", "lastPath", lastPath ); 

    // And push it into a string for parsing
    string lastPathString( lastPath );

    // Parse for directory name
    size_t found;
    found = lastPathString.find_last_of("/");

    // Lets check to see if an images folder was used
    if( lastPathString.substr(found+1) == "images" ) {
        string spectraPathName;
        spectraPathName = lastPathString.substr(0,found); 
        spectraPathName += "/spectra";
		if( svkUtils::FilePathExists(spectraPathName.c_str())) {
            status = this->OpenFile( "spectra", spectraPathName.c_str(), false, false );
        } else {
            // If an images folder was used, but there is no corresponding spectra folder
            status = this->OpenFile( "spectra", lastPathString.substr(0,found).c_str(), false, false );
        }
    } else { 
        status = this->OpenFile( "spectra", lastPathString.c_str(), false, false );
    }
     
    // If the dialog was cancelled, or if either load failed do not oven overlay
    if( status == vtkKWDialog::StatusCanceled || this->GetActive4DImageData() == NULL
                                              || !this->model->DataExists("AnatomicalData") ) {
        return;
    } 

    if( lastPathString.substr(found+1) == "images" ) {
        string spectraPathName;
        spectraPathName = lastPathString.substr(0,found); 
        spectraPathName += "/spectra";
        spectraPathName += "/" + svkUCSFUtils::GetMetaboliteDirectoryName(this->model->GetDataFileName("SpectroscopicData"));
        bool includePath = true;
        string cniFileName = svkUCSFUtils::GetMetaboliteFileName( this->model->GetDataFileName("SpectroscopicData"), "CNI-ht",includePath );
		if( this->model->DataExists("CNI-ht")) {
			int toggleDraw = this->GetDraw();
			if( toggleDraw ) {
				this->DrawOff();
			}
            this->OpenOverlay( this->model->GetDataObject("CNI-ht"), this->model->GetDataFileName("CNI-ht"));
            this->EnableWidgets(); 
            this->imageViewWidget->thresholdType->GetWidget()->SetValue( "Quantity" );
            this->imageViewWidget->overlayThresholdSlider->SetValue( 2.0 );
            this->SetOverlayThreshold( 2.0 );
			if( toggleDraw ) {
				this->DrawOn();
			}
		} else if( svkUtils::FilePathExists(spectraPathName.c_str()) ) {
            this->OpenFile( "overlay", spectraPathName.c_str(), false, true );
        } else {
            // If an images folder was used, but there is no corresponding spectra folder
            this->OpenFile( "overlay", lastPathString.substr(0,found).c_str(), false, true );
        }
    } else { 
        this->OpenFile( "overlay", lastPathString.c_str(), false, true );
    }
	return;

}


/*!    Open a file.    */
int vtkSivicController::OpenFile( char* openType, const char* startPath, bool resetBeforeLoad, bool onlyReadOneInputFile )
{
    this->viewRenderingWidget->viewerWidget->GetRenderWindowInteractor()->Disable();
    this->viewRenderingWidget->specViewerWidget->GetRenderWindowInteractor()->Disable();
    //this->viewRenderingWidget->viewerWidget->RemoveBindings();
    //this->viewRenderingWidget->specViewerWidget->RemoveBindings();
    string openTypeString( openType ); 
    size_t pos;
    vtkKWLoadSaveDialog *dlg = NULL; 
    dlg = vtkKWLoadSaveDialog::New();

    //  If it's not a command line load, then init the GUI dialog, otherwise just load the
    //  specified file: 
    if ( openTypeString.find("command_line") == string::npos ) {

        dlg->SetApplication(this->app);
        dlg->Create();
        if( startPath != NULL && strcmp( startPath, "NULL")!=0) {
            dlg->SetLastPath( startPath );
        } else {
            dlg->RetrieveLastPathFromRegistry("lastPath");
            string lastPathString("./");
            if( this->app->HasRegistryValue (0, "RunTime", "lastPath") ) {
                lastPathString = dlg->GetLastPath();
            } 
            size_t found;
            found = lastPathString.find_last_of("/");

            if( strcmp( openType, "image" ) == 0 || strcmp( openType, "image_dynamic" ) == 0 || strcmp( openType, "add_image_dynamic" ) == 0 || strcmp( openType, "overlay" ) == 0){
                lastPathString = lastPathString.substr(0,found); 
                lastPathString += "/images";
				if( svkUtils::FilePathExists(lastPathString.c_str())) {
                    dlg->SetLastPath( lastPathString.c_str());
                }
            } else if ( strcmp( openType, "spectra" ) == 0 || strcmp( openType, "add_spectra") == 0) {
                lastPathString = lastPathString.substr(0,found); 
                lastPathString += "/spectra";
				if( svkUtils::FilePathExists( lastPathString.c_str()) ) {
                    dlg->SetLastPath( lastPathString.c_str());
                }

            }

        }
    
        // Check to see which extention to filter for.
        if( strcmp( openType,"image" ) == 0 || strcmp( openType, "image_dynamic" ) == 0 || strcmp( openType, "image_dynamic" ) == 0 || strcmp( openType, "overlay" ) == 0 ) {
            dlg->SetFileTypes("{{Image Files} {.idf .fdf .dcm .DCM .IMA}} {{All files} {.*}}");
        } else if( strcmp( openType,"spectra" ) == 0 || strcmp( openType, "add_spectra") == 0) {
			char defaultSpectraExtension[100] = "";

			this->app->GetRegistryValue( 0, "defaults", "spectra_extension_filtering", defaultSpectraExtension );
			if( defaultSpectraExtension != NULL && strcmp( defaultSpectraExtension, "OFF" ) == 0 ) {
				dlg->SetFileTypes("{{All files} {.*}} {{MRS Files} {.ddf .shf .rda .dcm .DCM fid}}");
			} else {
				dlg->SetFileTypes("{{MRS Files} {.ddf .shf .rda .dcm .DCM fid}} {{All files} {.*}}");
			}
        } else {
            dlg->SetFileTypes("{{All files} {.*}} {{Image Files} {.idf .fdf .dcm .DCM .IMA}} {{MRS Files} {.ddf .shf .rda .dcm .DCM fid}}");
        }
    

        // Invoke the dialog
        dlg->Invoke();

        if ( dlg->GetStatus() == vtkKWDialog::StatusOK ) {
            if( resetBeforeLoad ) {
                this->ResetApplication();
            } 
            this->imageViewWidget->loadingLabel->SetText("Loading...");
            app->ProcessPendingEvents(); 
            this->imageViewWidget->Focus();
            string fileName("");
            fileName += dlg->GetFileName(); 
            pos = fileName.find("."); 
            if (pos == string::npos) {
                pos = 0; 
            }    
    
    
            if( DEBUG ) {
                cout<< "Extention: " << fileName.substr(pos) <<endl;   
                cout<< "Filename: " << dlg->GetFileName() << endl;
            }
        }
    } else {
        pos = openTypeString.find("command_line");  
        openTypeString.assign( openTypeString.substr( pos + 13) ); 
        cout << "OPEN TYPE STRING: " << openTypeString << endl;
        dlg->SetFileName( startPath );
    }


    if (dlg->GetFileName() != NULL) {    
        // See if it is being loaded as a metabolite
        if( openTypeString.compare( "image" ) == 0 ) {
            this->OpenImage( dlg->GetFileName(), onlyReadOneInputFile );
        } else if( openTypeString.compare( "image_dynamic" ) == 0 ) {
            this->Open4DImage( dlg->GetFileName(), onlyReadOneInputFile );
        } else if( openTypeString.compare( "add_image_dynamic" ) == 0 ) {
            this->Add4DImageData( dlg->GetFileName(), onlyReadOneInputFile );
        } else if( openTypeString.compare( "overlay" ) == 0 ) {
            this->OpenOverlay( dlg->GetFileName(), onlyReadOneInputFile );
        } else if( openTypeString.compare( "spectra" ) == 0 ) {
            this->Open4DImage( dlg->GetFileName(), onlyReadOneInputFile );
        } else if( openTypeString.compare( "add_spectra" ) == 0 ) {
            this->Add4DImageData( dlg->GetFileName(), onlyReadOneInputFile );
        } 
    }

    // If the image was loaded and a spec data set is also loaded, then activate the toggle buttons:
    this->EnableWidgets(); 
    int dlgStatus = dlg->GetStatus();
    this->imageViewWidget->loadingLabel->SetText("Done!");
    if ( dlg != NULL) {
        dlg->SaveLastPathToRegistry("lastPath");
        dlg->Delete();
    }
    app->ProcessPendingEvents();
    this->viewRenderingWidget->viewerWidget->GetRenderWindowInteractor()->Enable();
    this->viewRenderingWidget->specViewerWidget->GetRenderWindowInteractor()->Enable();
    this->imageViewWidget->loadingLabel->SetText("");
    ////this->viewRenderingWidget->viewerWidget->AddBindings();
    ////this->viewRenderingWidget->specViewerWidget->AddBindings();
    //this->GetApplication()->GetNthWindow(0)->SetStatusText("Done"  );
    //this->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 0.0 );
    return dlgStatus;
}


//! Saves Data File.
void vtkSivicController::SaveData()
{
    vtkKWFileBrowserDialog *dlg = vtkKWFileBrowserDialog::New();
    dlg->SetApplication(app);
    dlg->SaveDialogOn();
    dlg->Create();
    dlg->SetInitialFileName("E1S1I1");
    dlg->SetFileTypes("{{DICOM MR Spectroscopy} {.dcm}} {{UCSF Volume File} {.ddf}}");
    dlg->SetDefaultExtension("{{DICOM MR Spectroscoy} {.dcm}} {{UCSF Volume File} {.ddf}}");
    dlg->Invoke();
    if ( dlg->GetStatus() == vtkKWDialog::StatusOK ) {
        string filename = dlg->GetFileName(); 
        char* cStrFilename = new char [filename.size()+1];
        strcpy (cStrFilename, filename.c_str());
        this->SaveData( cStrFilename );
    } 
    dlg->Delete();
}


string vtkSivicController::GetOsiriXInDir()
{
	string inDir("/Users/" + svkUtils::GetUserName() + "/Documents/OsiriX Data/INCOMING.noindex/");
    return inDir; 
}


void vtkSivicController::SaveDataOsiriX()
{
    string fname( this->GetOsiriXInDir() + "dicom_mrs.dcm" );
    this->SaveData( const_cast<char*>( fname.c_str() ));
}


/*!
 *  Saves metabolite maps to OsiriX DB
 */
void vtkSivicController::SaveMetMapDataOsiriX()
{
    //  Loop over each of the metabolte maps and save to OsiriX: 
    for (int i = 0; i < this->quantificationWidget->modelMetNames.size(); i++ ) {
        cout << "Save mets to OsiriX: " << this->quantificationWidget->modelMetNames[i] << endl;
        string fname( this->GetOsiriXInDir() + this->quantificationWidget->modelMetNames[i] );

        this->SaveMetMapData( 
            this->model->GetDataObject( this->quantificationWidget->modelMetNames[i] ),
            const_cast<char*>( fname.c_str() ) 
        );
    }
}

/*
 *  Saves metabolite maps to user specified loction
 */
void vtkSivicController::SaveMetaboliteMaps()
{
    vtkKWFileBrowserDialog *dlg = vtkKWFileBrowserDialog::New();
    dlg->SetApplication(app);
    dlg->SaveDialogOn();
    dlg->Create();
    dlg->SetInitialFileName("");
    //  kludge to differentiate between enhanced and multi-frame (dcm versus DCM)
    dlg->SetFileTypes("{{DICOM MRI} {.DCM}} {{DICOM Enhanced MRI} {.dcm}} {{UCSF Volume File} {.idf}}");
    dlg->SetDefaultExtension("{{DICOM MRI} {.DCM}} {{DICOM Enhanced MRI} {.dcm}} {{UCSF Volume File} {.idf}}");
    dlg->Invoke();
    if ( dlg->GetStatus() == vtkKWDialog::StatusOK ) {
        string filename = dlg->GetFileName();
        char* cStrFilename = new char [filename.size()+1];
        strcpy (cStrFilename, filename.c_str());

        vtkstd::string root = svkImageReader2::GetFileRoot( cStrFilename ); 
        vtkstd::string ext = svkImageReader2::GetFileExtension( cStrFilename ); 
        int fileType; 
        if ( ext.compare("DCM") == 0 ) {
            fileType = 5;       // MRI
        } else if ( ext.compare("dcm") == 0 ) {
            fileType = 6;       // Enhanced MRI
        } else if ( ext.compare("idf") == 0 ) {
            fileType = 3;    
        }
        //  Loop over each of the metabolte maps and save to OsiriX:
        for (int i = 0; i < this->quantificationWidget->modelMetNames.size(); i++ ) {
            cout << "Save mets to local fs: " << this->quantificationWidget->modelMetNames[i] << endl;
            string fname( root + this->quantificationWidget->modelMetNames[i] );
    
            this->SaveMetMapData(
                this->model->GetDataObject( this->quantificationWidget->modelMetNames[i] ),
                const_cast<char*>( fname.c_str() ), 
                fileType
            );
        }
        for (int i = 0; i < this->dscWidget->modelDSCNames.size(); i++ ) {
            cout << "Save mets to local fs: " << this->dscWidget->modelDSCNames[i] << endl;
            string fname( root + this->dscWidget->modelDSCNames[i] );

            this->SaveMetMapData(
                this->model->GetDataObject( this->dscWidget->modelDSCNames[i] ),
                const_cast<char*>( fname.c_str() ),
                fileType
            );
        }

    }
    dlg->Delete();

}


/*
 *  Saves image data stored in model to disk.
 */
void vtkSivicController::SaveImageFromModel( const char* modelObjectName )
{
	svkImageData* data = this->model->GetDataObject( modelObjectName );
	if( data != NULL ) {
		vtkKWFileBrowserDialog *dlg = vtkKWFileBrowserDialog::New();
		dlg->SetApplication(app);
		dlg->SaveDialogOn();
		dlg->Create();
		dlg->SetInitialFileName("");
		//  kludge to differentiate between enhanced and multi-frame (dcm versus DCM)
		dlg->SetFileTypes("{{DICOM MRI} {.DCM}} {{DICOM Enhanced MRI} {.dcm}} {{UCSF Volume File} {.idf}}");
		dlg->SetDefaultExtension("{{DICOM MRI} {.DCM}} {{DICOM Enhanced MRI} {.dcm}} {{UCSF Volume File} {.idf}}");
		dlg->Invoke();
		if ( dlg->GetStatus() == vtkKWDialog::StatusOK ) {
			string filename = dlg->GetFileName();
			char* cStrFilename = new char [filename.size()+1];
			strcpy (cStrFilename, filename.c_str());

			vtkstd::string root = svkImageReader2::GetFileRoot( cStrFilename );
			vtkstd::string ext = svkImageReader2::GetFileExtension( cStrFilename );
			int fileType;
			if ( ext.compare("DCM") == 0 ) {
				fileType = 5;       // MRI
			} else if ( ext.compare("dcm") == 0 ) {
				fileType = 6;       // Enhanced MRI
			} else if ( ext.compare("idf") == 0 ) {
				fileType = 3;
			}
			string fname( root + modelObjectName );

			this->SaveMetMapData(
					data,
					const_cast<char*>( fname.c_str() ),
                fileType
				);
		}
		dlg->Delete();
	} else {
		this->PopupMessage("Could not locate dataset in model!");
	}

}


/*! 
 *  Writes metabolite image files 
 */
void vtkSivicController::SaveMetMapData( svkImageData* image, char* fileName, int writerType )
{

    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer;
    
    string fileNameString = string( fileName );
    writer = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( 
        static_cast<svkImageWriterFactory::WriterType>( writerType) ) 
    );
    if( writer->IsA("svkIdfVolumeWriter") ) {
		char doubleToFloat[100] = "";
		this->GetApplication()->GetRegistryValue( 0, "data_writing", "double_to_float", doubleToFloat );
    	if(doubleToFloat == NULL || strcmp( doubleToFloat, "" ) == 0 || strcmp( doubleToFloat, "CAST" ) == 0 ) {
			svkIdfVolumeWriter::SafeDownCast( writer )->SetCastDoubleToFloat( true );
    	}
    }
    cout << "FN: " << fileName << endl;
    cout << "type: " << writerType << endl;

    writer->SetFileName(fileName);
    writerFactory->Delete();

    writer->SetInput( image ); 
    writer->Write();

    writer->Delete();
}


/*! 
 *  Writes MRS data files 
 */
void vtkSivicController::SaveData( char* fileName )
{

    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer;

    string fileNameString = string( fileName);
    size_t pos = fileNameString.find(".");
    if( strcmp( fileNameString.substr(pos).c_str(), ".ddf" ) == 0 ) {
        writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter(svkImageWriterFactory::DDF));
    } else {
        writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter(svkImageWriterFactory::DICOM_MRS));
    }
    cout << "FN: " << fileName << endl;
    writer->SetFileName(fileName);
    writerFactory->Delete();

    writer->SetInput( this->GetActive4DImageData() );

    writer->Write();
    writer->Delete();
}


//! Saves a secondary capture.
void vtkSivicController::SaveSecondaryCapture( char* captureType )
{
    svk4DImageData* activeData = this->GetActive4DImageData();
    if( activeData == NULL ) {
        PopupMessage( "SPECTRA MUST BE LOADED TO CREATE SECONDARY CAPTURES!" );
        return; 
    }
    vtkKWFileBrowserDialog *dlg = vtkKWFileBrowserDialog::New();
    dlg->SetApplication(app);
    dlg->SaveDialogOn();
    dlg->Create();


    string defaultNamePattern;
    int seriesNumber =  svkImageWriterFactory::GetNewSeriesFilePattern( 
        activeData ,
        &defaultNamePattern
    );
    defaultNamePattern.append("*"); 
    dlg->SetInitialFileName( defaultNamePattern.c_str() );

    dlg->SetFileTypes("{{DICOM Secondary Capture} {.dcm}} {{TIFF} {.tiff}} {{JPEG} {.jpeg}} {{PS} {.ps}} {{All files} {.*}}");
    dlg->SetDefaultExtension("{{DICOM Secondary Capture} {.dcm}} {{TIFF} {.tiff}} {{JPEG} {.jpeg}} {{PS} {.ps}} {{All files} {.*}}");
#if defined( SVK_USE_GL2PS )
    if( strcmp(captureType,"SPECTRA_CAPTURE") == 0 ) {
        dlg->SetFileTypes("{{DICOM Secondary Capture} {.dcm}} {{TIFF} {.tiff}} {{JPEG} {.jpeg}} {{PS} {.ps}} {{EPS} {.eps}} {{PDF} {.pdf}} {{SVG} {.svg}} {{TeX} {.tex}} {All files} {.*}}");
        dlg->SetDefaultExtension("{{DICOM Secondary Capture} {.dcm}} {{TIFF} {.tiff}} {{JPEG} {.jpeg}} {{PS} {.ps}} {{EPS} {.eps}} {{PDF} {.pdf}} {{SVG} {.svg}} {{All files} {.*}}");
    }
#endif
    dlg->Invoke();
    if ( dlg->GetStatus() == vtkKWDialog::StatusOK ) {
        string filename = dlg->GetFileName(); 
        char* cStrFilename = new char [filename.size()+1];
        strcpy (cStrFilename, filename.c_str());
        this->SaveSecondaryCapture( cStrFilename, seriesNumber, captureType );
    } 
    dlg->Delete();
}


/*!
 *
 */
void vtkSivicController::SaveSecondaryCaptureOsiriX()
{
    string fname( this->GetOsiriXInDir() + "sc.dcm" );
    this->SaveSecondaryCapture( const_cast<char*>( fname.c_str() ), 77, "COMBINED_CAPTURE");
}


/*! 
 *  Writes a screen capture from a render window to a .dcm file
 */   
void vtkSivicController::SaveSecondaryCapture( char* fileName, int seriesNumber, char* captureType, int outputOption, bool print )
{
    cout << "PATH: " << fileName << endl;
    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    vtkImageWriter* writer = NULL;
    string printerName = "";
    bool wasCursorLocationOn = 0;
    if( this->overlayController != NULL ) {
        if( this->viewRenderingWidget->viewerWidget->GetRenderWindow()->HasRenderer(this->overlayController->GetView()->GetRenderer( svkOverlayView::MOUSE_LOCATION ))) {
            wasCursorLocationOn = 1; 
            this->overlayController->GetView()->TurnRendererOff( svkOverlayView::MOUSE_LOCATION );    
        }
    }
    string fileNameString = string( fileName);
    size_t pos = fileNameString.find_last_of(".");
    if( strcmp( fileNameString.substr(pos).c_str(), ".dcm" ) == 0 ) {
        writer = writerFactory->CreateImageWriter( svkImageWriterFactory::DICOM_SC );
        static_cast<svkImageWriter*>(writer)->SetSeriesNumber(seriesNumber);
    } else if( strcmp( fileNameString.substr(pos).c_str(), ".ps" ) == 0 ) {
        writer = writerFactory->CreateImageWriter( svkImageWriterFactory::PS );
    } else if( strcmp( fileNameString.substr(pos).c_str(), ".tiff" ) == 0 ) {
        writer = writerFactory->CreateImageWriter( svkImageWriterFactory::TIFF );
        vtkTIFFWriter* tmp = vtkTIFFWriter::SafeDownCast( writer );
        tmp->SetCompression(0);
    } else if( strcmp( fileNameString.substr(pos).c_str(), ".jpeg" ) == 0 ) {
        writer = writerFactory->CreateImageWriter( svkImageWriterFactory::JPEG );
        vtkJPEGWriter* tmp = vtkJPEGWriter::SafeDownCast( writer );
        tmp->SetQuality(100);
#if defined( SVK_USE_GL2PS )
    } else if( strcmp( fileNameString.substr(pos).c_str(), ".svg" ) == 0 ||
               strcmp( fileNameString.substr(pos).c_str(), ".eps" ) == 0 ||
               strcmp( fileNameString.substr(pos).c_str(), ".pdf" ) == 0 ||
               strcmp( fileNameString.substr(pos).c_str(), ".tex" ) == 0 ) {
        this->ExportSpectraCapture( fileNameString, outputOption, fileNameString.substr(pos));
        return;
#endif
    } else {
        this->PopupMessage("Error: Extension not supported-- FILE NOT WRITTEN!!");
        return;
    }

    writerFactory->Delete();

    //this->viewRenderingWidget->viewerWidget->GetRenderWindow()->OffScreenRenderingOn();
    //this->viewRenderingWidget->specViewerWidget->GetRenderWindow()->OffScreenRenderingOn();
    //this->viewRenderingWidget->titleWidget->GetRenderWindow()->OffScreenRenderingOn();
    //this->viewRenderingWidget->infoWidget->GetRenderWindow()->OffScreenRenderingOn();
    int currentSlice = this->plotController->GetSlice(); 



    int firstFrame = 0;
    int lastFrame = this->GetActive4DImageData()->GetDcmHeader()->GetNumberOfSlices();
    if( outputOption == svkSecondaryCaptureFormatter::CURRENT_SLICE ) { 
        firstFrame = plotController->GetSlice();
        lastFrame = firstFrame + 1;
    }

    if( print ) {
        this->ToggleColorsForPrinting( svkSecondaryCaptureFormatter::DARK_ON_LIGHT );
    }

    svkImageData* outputImage = NULL;

    /* svkImageWriter's require svkImageData with a header. So we will grab the spec header
     * and register the it so we can delete our new svkImageData object without deleting
     * the original header.
     */
    outputImage = svkMriImageData::New();

    
    if( this->model->GetDataObject( "AnatomicalData" ) != NULL ) {
        outputImage->SetDcmHeader( this->model->GetDataObject( "AnatomicalData" )->GetDcmHeader() );
        this->model->GetDataObject( "AnatomicalData" )->GetDcmHeader()->Register(this);
    } else { // If only spectra are loaded
        outputImage->SetDcmHeader( this->GetActive4DImageData()->GetDcmHeader() );
        this->GetActive4DImageData()->GetDcmHeader()->Register(this);
    }
    //Give it a default dcos so it can be viewed in an image viewer
    double dcos[3][3] = {{1,0,0}, {0,1,0}, {0,0,1}}; 
    if( this->model->GetDataObject( "AnatomicalData" ) != NULL ) {
        this->model->GetDataObject( "AnatomicalData" )->GetDcos(dcos);
    } else { //If only spectra are loaded
        this->GetActive4DImageData()->GetDcos(dcos);
    }
    outputImage->SetDcos(dcos);
    if( writer->IsA("svkImageWriter") ) {  
        static_cast<svkImageWriter*>(writer)->SetSeriesDescription( "SIVIC secondary capture" );

    }

    if( strcmp(captureType,"COMBINED_CAPTURE") == 0 ) {
        this->WriteCombinedCapture( writer, fileNameString, outputOption, outputImage, print);
    } else if( strcmp(captureType,"IMAGE_CAPTURE") == 0 ) {
        this->WriteImageCapture( writer, fileNameString, outputOption, outputImage, print);
    } else if( strcmp(captureType,"SPECTRA_CAPTURE") == 0 ) {
        this->WriteSpectraCapture( writer, fileNameString, outputOption, outputImage, print);
    } else if( strcmp(captureType,"SPECTRA_WITH_OVERVIEW_CAPTURE") == 0 ) {
        this->secondaryCaptureFormatter
            ->WriteCombinedWithSummaryCapture( writer, fileNameString, outputOption, outputImage, print );
    }


    this->SetSlice(currentSlice);

    //this->viewRenderingWidget->viewerWidget->GetRenderWindow()->OffScreenRenderingOff();
    //this->viewRenderingWidget->specViewerWidget->GetRenderWindow()->OffScreenRenderingOff();
    //this->viewRenderingWidget->titleWidget->GetRenderWindow()->OffScreenRenderingOff();
    //this->viewRenderingWidget->infoWidget->GetRenderWindow()->OffScreenRenderingOff();

    if (outputImage != NULL) {
        outputImage->Delete();
        outputImage = NULL; 
    }
    if( wasCursorLocationOn && this->overlayController != NULL ) {
        this->overlayController->GetView()->TurnRendererOn( svkOverlayView::MOUSE_LOCATION );    
    }

/*
    if ( this->model->DataExists("MetaboliteData") ) {
        svkLabeledDataMapper::SafeDownCast(
            vtkActor2D::SafeDownCast(this->plotController->GetView()->GetProp( svkPlotGridView::OVERLAY_TEXT ))->GetMapper())->GetLabelTextProperty()->SetFontSize(12);
    }
    this->viewRenderingWidget->imageInfoActor->GetTextProperty()->SetFontSize(13);
    this->viewRenderingWidget->specInfoActor->GetTextProperty()->SetFontSize(13);
*/

    if( print ){
        ToggleColorsForPrinting( svkSecondaryCaptureFormatter::LIGHT_ON_DARK );
        this->SetPreferencesFromRegistry();
    }
    writer->Delete();
    this->overlayController->GetView()->Refresh();    
    this->plotController->GetView()->Refresh();    
    this->viewRenderingWidget->infoWidget->Render();    
}


/*!
 *
 */
void vtkSivicController::WriteCombinedCapture( vtkImageWriter* writer, string fileNameString, int outputOption, svkImageData* outputImage, bool print ) 
{
    this->secondaryCaptureFormatter->WriteCombinedCapture( writer, fileNameString, outputOption, outputImage, print );
}


/*!
 *
 */
void vtkSivicController::WriteImageCapture( vtkImageWriter* writer, string fileNameString, int outputOption, svkImageData* outputImage, bool print, int instanceNumber ) 
{
    this->secondaryCaptureFormatter->WriteImageCapture( writer, fileNameString, outputOption, outputImage, print, instanceNumber );
}


/*!
 *
 */
void vtkSivicController::WriteSpectraCapture( vtkImageWriter* writer, string fileNameString, int outputOption, svkImageData* outputImage, bool print ) 
{
    this->secondaryCaptureFormatter->WriteSpectraCapture( writer, fileNameString, outputOption, outputImage, print );
}


#if defined( SVK_USE_GL2PS )
/*!
 *
 */
void vtkSivicController::ExportSpectraCapture( string fileNameString, int outputOption, string type ) 
{
    vtkGL2PSExporter* gl2psExporter = vtkGL2PSExporter::New();
    gl2psExporter->SetRenderWindow(this->viewRenderingWidget->specViewerWidget->GetRenderWindow());
    gl2psExporter->CompressOff();
    gl2psExporter->OcclusionCullOff();
    if( type == ".svg" ) {
        gl2psExporter->SetFileFormatToSVG();
    } else if( type == ".eps" ) {
        gl2psExporter->SetFileFormatToEPS();
    } else if( type == ".pdf" ) {
        gl2psExporter->SetFileFormatToPDF();
    } else if( type == ".tex" ) {
        gl2psExporter->SetFileFormatToTeX();
    }

    svkDcmHeader::Orientation dcmOrientation = svkDcmHeader::UNKNOWN_ORIENTATION;
    if( this->orientation == "AXIAL" ) {
		dcmOrientation  = svkDcmHeader::AXIAL;
    } else if ( this->orientation == "CORONAL" ) {
		dcmOrientation  = svkDcmHeader::CORONAL;
    } else if ( this->orientation == "SAGITTAL" ) {
		dcmOrientation  = svkDcmHeader::SAGITTAL;
    }

	int firstFrame = this->GetActive4DImageData()->GetFirstSlice( dcmOrientation );
	int lastFrame = this->GetActive4DImageData()->GetLastSlice( dcmOrientation );
    if( outputOption == svkSecondaryCaptureFormatter::CURRENT_SLICE ) { 
        firstFrame = plotController->GetSlice();
        lastFrame = firstFrame + 1;
    }

    int instanceNumber = 1;
    for (int m = firstFrame; m <= lastFrame; m++) {
        if( this->GetActive4DImageData()->IsA("svkMrsImageData")
            &&  static_cast<svkMrsImageData*>(this->GetActive4DImageData())->HasSelectionBox()
            && !static_cast<svkMrsImageData*>(this->GetActive4DImageData())->IsSliceInSelectionBox(m, dcmOrientation)) {
            continue;
        }
        string fileNameStringTmp = fileNameString; 

        ostringstream frameNum;
        frameNum <<  instanceNumber;

        this->SetSlice(m);

        //  Replace * with slice number in output file name: 
        size_t posStar = fileNameStringTmp.find_last_of("*");
        size_t posDot = fileNameStringTmp.find_last_of(".");
        if ( posStar != string::npos) {
            fileNameStringTmp.replace(posStar, 1, frameNum.str()); 
            posDot = fileNameStringTmp.find_last_of(".");
        } else if ( posDot != string::npos) {
            fileNameStringTmp.replace(posDot, 1, frameNum.str() + ".");
            posDot = fileNameStringTmp.find_last_of(".");
        } else if ( posDot != string::npos) {
            fileNameStringTmp+= frameNum.str();
            posDot = fileNameStringTmp.find_last_of(".");
        }

        cout << "FN: " << fileNameStringTmp.c_str() << endl;
        if( posDot != string::npos ) {
            gl2psExporter->SetFilePrefix(fileNameStringTmp.substr(0,posDot).c_str());
        } else {
            gl2psExporter->SetFilePrefix(fileNameStringTmp.c_str());
        }


        gl2psExporter->Write();
        instanceNumber++;
    }
    gl2psExporter->Delete();
}
#endif


/*!
 *
 */
void vtkSivicController::ToggleColorsForPrinting( bool colorSchema ) 
{
    double backgroundColor[3];
    double foregroundColor[3];
    int plotGridSchema;
    if( colorSchema == svkSecondaryCaptureFormatter::DARK_ON_LIGHT ) {
        backgroundColor[0] = 1;
        backgroundColor[1] = 1;
        backgroundColor[2] = 1;
        foregroundColor[0] = 0;
        foregroundColor[1] = 0;
        foregroundColor[2] = 0;
        plotGridSchema = svkSecondaryCaptureFormatter::DARK_ON_LIGHT;
    } else {
        backgroundColor[0] = 0;
        backgroundColor[1] = 0;
        backgroundColor[2] = 0;
        foregroundColor[0] = 1;
        foregroundColor[1] = 1;
        foregroundColor[2] = 1;
        plotGridSchema = svkPlotGridView::LIGHT_ON_DARK;
    }

    // Prepare the two Controllers for printing
    this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->SetBackground( backgroundColor );
    this->plotController->SetColorSchema( plotGridSchema );

    // Now lets invert the colors of the text bar 
/*
    vtkRenderer* tmpRenderer = this->viewRenderingWidget->specInfoWidget->GetRenderer();
    tmpRenderer->SetBackground( backgroundColor );
    vtkPropCollection* tmpActors = tmpRenderer->GetViewProps();
    vtkCollectionIterator* iterator = vtkCollectionIterator::New();
    iterator->SetCollection( tmpActors );
    iterator->InitTraversal();
    vtkTextActor* currentActor;
    while( !iterator->IsDoneWithTraversal() ) {
        currentActor = vtkTextActor::SafeDownCast( iterator->GetCurrentObject() );
        currentActor->GetTextProperty()->SetColor( foregroundColor );
        currentActor->Modified();
        iterator->GoToNextItem();
    }
    iterator->Delete();

    tmpRenderer = this->viewRenderingWidget->infoWidget->GetRenderer();
    tmpRenderer->SetBackground( backgroundColor );
    tmpActors = tmpRenderer->GetViewProps();
    iterator = vtkCollectionIterator::New();
    iterator->SetCollection( tmpActors );
    iterator->InitTraversal();
    while( !iterator->IsDoneWithTraversal() ) {
        currentActor = vtkTextActor::SafeDownCast( iterator->GetCurrentObject() );
        currentActor->GetTextProperty()->SetColor(foregroundColor);
        currentActor->Modified();
        iterator->GoToNextItem();
    }
    iterator->Delete();
*/

}


//! Pure setter method.
void vtkSivicController::SetModel( svkDataModel* model  ) 
{
    this->model = model;
    if( this->imageViewWidget != NULL ) {
        this->imageViewWidget->SetModel(this->model);
    }
    if( this->spectraViewWidget != NULL ) {
        this->spectraViewWidget->SetModel(this->model);
    }
    if( this->spectraRangeWidget != NULL ) {
        this->spectraRangeWidget->SetModel(this->model);
    }
    if( this->viewRenderingWidget != NULL ) {
        this->viewRenderingWidget->SetModel(this->model);
    }
    if( this->secondaryCaptureFormatter != NULL ) {
        secondaryCaptureFormatter->SetModel( model );
    }
    if(!model->HasObserver(vtkCommand::ProgressEvent, progressCallback)) {
        model->AddObserver(vtkCommand::ProgressEvent, progressCallback);
    }
}


//! Pure getter method.
svkDataModel* vtkSivicController::GetModel( ) 
{
    return model;
}


//! Returns the overlay controller
svkOverlayViewController* vtkSivicController::GetOverlayController()
{
    return this->overlayController;
}


//! Returns the plot controller
svkPlotGridViewController* vtkSivicController::GetPlotController()
{
    return this->plotController;
}


//! Changes to the window leveling mouse interactor.
void vtkSivicController::UseWindowLevelStyle() 
{
    this->overlayController->UseWindowLevelStyle();
    this->viewRenderingWidget->specViewerWidget->Render();
    this->viewRenderingWidget->viewerWidget->Render();
}

//! Changes to the window leveling mouse interactor.
void vtkSivicController::UseColorOverlayStyle() 
{
    this->overlayController->UseColorOverlayStyle();
    this->viewRenderingWidget->specViewerWidget->Render();
    this->viewRenderingWidget->viewerWidget->Render();
}


//! Changes to the drag selection mouse interactor.
void vtkSivicController::UseSelectionStyle() 
{
        // This code snaps from 3D to the closest plane. Seems unnecessary now, but we might want it later. 
        /*
    if( model->DataExists( "SpectroscopicData" ) ) {

        svkImageData* data = this->model->GetDataObject( "SpectroscopicData" );
        double* viewPlaneNormal =  this->overlayController->GetView()->
        GetRenderer(svkOverlayView::PRIMARY)->GetActiveCamera()->GetViewPlaneNormal();
        double* cameraPosition =  this->overlayController->GetView()->
        GetRenderer(svkOverlayView::PRIMARY)->GetActiveCamera()->GetPosition();
        double* cameraFocalPoint =  this->overlayController->GetView()->
        GetRenderer(svkOverlayView::PRIMARY)->GetActiveCamera()->GetFocalPoint();
        float viewNormal[3] = { viewPlaneNormal[0], viewPlaneNormal[1], viewPlaneNormal[2] };
        float axialNormal[3];
        data->GetSliceNormal( axialNormal, svkDcmHeader::AXIAL );
        float coronalNormal[3];
        data->GetSliceNormal( coronalNormal, svkDcmHeader::CORONAL );
        float sagittalNormal[3];
        data->GetSliceNormal( sagittalNormal, svkDcmHeader::SAGITTAL );
        svkDcmHeader::Orientation closestOrientation = this->overlayController->GetView()->GetOrientation();
        string newOrientation;
        float* closestNormal;
        switch ( closestOrientation ) {
            case svkDcmHeader::AXIAL:
                closestNormal = axialNormal;
                newOrientation = "AXIAL";
                break;
            case svkDcmHeader::CORONAL:
                closestNormal = coronalNormal;
                newOrientation = "CORONAL";
                break;
            case svkDcmHeader::SAGITTAL:
                closestNormal = sagittalNormal;
                newOrientation = "SAGITTAL";
                break;
        }
        if( fabs(vtkMath::Dot( axialNormal, viewNormal )) > fabs(vtkMath::Dot( closestNormal, viewNormal )) ) {
            closestNormal = axialNormal;
            closestOrientation = svkDcmHeader::AXIAL;
            newOrientation = "AXIAL";
        }  
        if( fabs(vtkMath::Dot( coronalNormal, viewNormal )) > fabs(vtkMath::Dot( closestNormal, viewNormal )) ) {
            closestNormal = coronalNormal;
            closestOrientation = svkDcmHeader::CORONAL;
            newOrientation = "CORONAL";
        }  
        if( fabs(vtkMath::Dot( sagittalNormal, viewNormal )) > fabs(vtkMath::Dot( closestNormal, viewNormal )) ) {
            closestNormal = sagittalNormal;
            closestOrientation = svkDcmHeader::SAGITTAL;
            newOrientation = "SAGITTAL";
        }
        if( this->orientation != newOrientation ) {
            this->SetOrientation( newOrientation.c_str() );
        }
    }
    */
    if( this->overlayController->GetCurrentStyle() == svkOverlayViewController::ROTATION ) {
        svkOverlayView::SafeDownCast( this->overlayController->GetView() )->AlignCamera();
    }
    this->overlayController->UseSelectionStyle();
    this->viewRenderingWidget->specViewerWidget->Render();
    this->viewRenderingWidget->viewerWidget->Render();
    this->EnableWidgets();
    this->imageViewWidget->orthImagesButton->EnabledOff();



}


//! Changes to the rotatino mouse interactor.
void vtkSivicController::UseRotationStyle() 
{
    this->overlayController->UseRotationStyle();
    this->viewRenderingWidget->specViewerWidget->Render();
    this->viewRenderingWidget->viewerWidget->Render();

    if ( model->GetDataObject("AnatomicalData") ) {

        this->imageViewWidget->orthImagesButton->EnabledOn();
        this->imageViewWidget->orthImagesButton->InvokeEvent(vtkKWCheckButton::SelectedStateChangedEvent);
        this->imageViewWidget->axialSlider->EnabledOn();
        this->imageViewWidget->coronalSlider->EnabledOn();
        this->imageViewWidget->sagittalSlider->EnabledOn();

    }
}


//! Resets the window level to full range.
void vtkSivicController::ResetWindowLevel()
{
    if( this->model->DataExists("AnatomicalData") ) {
        this->overlayController->ResetWindowLevel();
        double* pixelRange = this->model->GetDataObject("AnatomicalData")->GetPointData()->GetArray(0)->GetRange();
        double window = this->overlayController->GetWindow();
        double level = this->overlayController->GetLevel();
        int toggleDraw = this->GetDraw();
        if( toggleDraw ) {
            this->DrawOff();
        }
        this->windowLevelWidget->SetLevelRange( pixelRange ); 
        this->windowLevelWidget->SetLevel( level ); 
        this->windowLevelWidget->SetWindowRange( 0, pixelRange[1] - pixelRange[0] ); 
        this->windowLevelWidget->SetWindow( window ); 
        this->viewRenderingWidget->ResetInfoText();
        if( toggleDraw ) {
            this->DrawOn();
        }
    }

}

//! Resets the window level to full range.
void vtkSivicController::HighlightSelectionBoxVoxels()
{
    this->overlayController->HighlightSelectionVoxels();
    this->plotController->HighlightSelectionVoxels();
    this->viewRenderingWidget->specViewerWidget->Render();
    this->viewRenderingWidget->viewerWidget->Render();
    this->processingWidget->SetPhaseUpdateExtent();
}


//! Callback.
void vtkSivicController::DisplayInfo()
{
    app->DisplayHelpDialog( app->GetNthWindow(0) );
}

//! Callback.
void vtkSivicController::DisplayPreferencesWindow()
{
    if( this->preferencesWindow == NULL ) {
        this->preferencesWindow = vtkKWWindowBase::New();
        this->app->AddWindow( this->preferencesWindow );
        this->preferencesWindow->SetTitle("SIVIC Preferences");
        this->preferencesWindow->Create();
        this->preferencesWindow->SetSize( 925, 525);
        this->preferencesWidget->SetParent( this->preferencesWindow->GetViewFrame() );
        this->preferencesWidget->Create();
        this->app->Script("grid %s -in %s -row 0 -column 0 -sticky wnse -pady 2 "
                , this->preferencesWidget->GetWidgetName(), this->preferencesWindow->GetViewFrame()->GetWidgetName());

        this->app->Script("grid rowconfigure %s 0  -weight 1"
                , this->preferencesWindow->GetViewFrame()->GetWidgetName() );
        this->app->Script("grid columnconfigure %s 0  -weight 1"
                , this->preferencesWindow->GetViewFrame()->GetWidgetName() );
    }

    int toggleDraw = this->GetDraw();
    if( toggleDraw ) {
        this->DrawOff();
    }
    this->preferencesWidget->UpdateSettingsList();
    this->preferencesWindow->Display();
    if( toggleDraw ) {
        this->DrawOn();
    }

    // Check to see if the window has already been added
    bool foundPreferencesWindow = false;
    for( int i = 0; i < app->GetNumberOfWindows(); i++ ) {
        if( app->GetNthWindow(i) == this->preferencesWindow ) {
            foundPreferencesWindow = true;
        }
    }

    // Add Window if not found.
    if( !foundPreferencesWindow ) {
        app->AddWindow( this->preferencesWindow );
    }
}

//! Callback.
void vtkSivicController::DisplayVoxelTaggingWindow()
{
    if( this->voxelTaggingWindow == NULL ) {
        this->voxelTaggingWindow = vtkKWWindowBase::New();
        this->app->AddWindow( this->voxelTaggingWindow );
        this->voxelTaggingWindow->SetTitle("Voxel Tagging");
        this->voxelTaggingWindow->Create();
        this->voxelTaggingWindow->SetSize( 250, 300);
        this->voxelTaggingWidget->SetParent( this->voxelTaggingWindow->GetViewFrame() );
        this->voxelTaggingWidget->Create();
        this->app->Script("grid %s -in %s -row 0 -column 0 -sticky wnse -pady 2 "
                , this->voxelTaggingWidget->GetWidgetName(), this->voxelTaggingWindow->GetViewFrame()->GetWidgetName());

        this->app->Script("grid rowconfigure %s 0  -weight 1"
                , this->voxelTaggingWindow->GetViewFrame()->GetWidgetName() );
        this->app->Script("grid columnconfigure %s 0  -weight 1"
                , this->voxelTaggingWindow->GetViewFrame()->GetWidgetName() );
    }

    int toggleDraw = this->GetDraw();
    if( toggleDraw ) {
        this->DrawOff();
    }
    this->voxelTaggingWidget->UpdateTagsList();
    this->voxelTaggingWindow->Display();
    if( toggleDraw ) {
        this->DrawOn();
    }

    // Check to see if the window has already been added
    bool foundVoxelTaggingWindow = false;
    for( int i = 0; i < app->GetNumberOfWindows(); i++ ) {
        if( app->GetNthWindow(i) == this->voxelTaggingWindow ) {
            foundVoxelTaggingWindow = true;
        }
    }

    // Add Window if not found.
    if( !foundVoxelTaggingWindow ) {
        app->AddWindow( this->voxelTaggingWindow );
    }
}


//! Callback.
void vtkSivicController::DisplayWindowLevelWindow()
{
    if( this->windowLevelWindow == NULL ) {
        this->windowLevelWindow = vtkKWWindowBase::New();
        this->app->AddWindow( this->windowLevelWindow );
        this->windowLevelWindow->Create();
        int width;
        int height;
        this->mainWindow->GetSize(&width, &height);
        this->windowLevelWindow->SetSize( width, 175);
        this->windowLevelWidget->SetParent( this->windowLevelWindow->GetViewFrame() );
        this->windowLevelWidget->Create();
        this->overlayWindowLevelWidget->SetParent( this->windowLevelWindow->GetViewFrame() );
        this->overlayWindowLevelWidget->Create();
        this->app->Script("grid %s -in %s -row 0 -column 0 -sticky wnse -pady 2 "
                , this->windowLevelWidget->GetWidgetName(), this->windowLevelWindow->GetViewFrame()->GetWidgetName());
        this->app->Script("grid %s -in %s -row 0 -column 1 -sticky wnse -pady 2 "
                , this->overlayWindowLevelWidget->GetWidgetName(), this->windowLevelWindow->GetViewFrame()->GetWidgetName());
        this->app->Script("grid rowconfigure %s 0  -weight 1"
                , this->windowLevelWindow->GetViewFrame()->GetWidgetName() );
        this->app->Script("grid columnconfigure %s 0  -weight 1"
                , this->windowLevelWindow->GetViewFrame()->GetWidgetName() );
        this->app->Script("grid columnconfigure %s 1  -weight 1"
                , this->windowLevelWindow->GetViewFrame()->GetWidgetName() );
    }

    int toggleDraw = this->GetDraw();
    if( toggleDraw ) {
        this->DrawOff();
    }
    if( this->model->DataExists("AnatomicalData") ) {
        double* pixelRange = this->model->GetDataObject("AnatomicalData")->GetPointData()->GetArray(0)->GetRange();
        double window = this->overlayController->GetWindow(svkOverlayViewController::REFERENCE_IMAGE);
        double level = this->overlayController->GetLevel(svkOverlayViewController::REFERENCE_IMAGE);
        this->windowLevelWidget->SetLevelRange( pixelRange ); 
        this->windowLevelWidget->SetLevel( level ); 
        this->windowLevelWidget->SetWindowRange( 0, pixelRange[1] - pixelRange[0] ); 
        this->windowLevelWidget->SetWindow( window ); 
    }
    if( this->model->DataExists("OverlayData") || this->model->DataExists("MetaboliteData") ) { 
        string overlayDataName = this->overlayWindowLevelWidget->GetOverlayDataName();
        double* pixelRange = this->model->GetDataObject(overlayDataName)->GetPointData()->GetArray(0)->GetRange();
        double window = this->overlayController->GetWindow(svkOverlayViewController::IMAGE_OVERLAY);
        double level = this->overlayController->GetLevel(svkOverlayViewController::IMAGE_OVERLAY);
        this->overlayWindowLevelWidget->SetLevelRange( pixelRange ); 
        this->overlayWindowLevelWidget->SetLevel( level ); 
        this->overlayWindowLevelWidget->SetWindowRange( 0, pixelRange[1] - pixelRange[0] ); 
        this->overlayWindowLevelWidget->SetWindow( window ); 
    } 
    if( toggleDraw ) {
        this->DrawOn();
    }
    this->windowLevelWindow->Display();
    bool foundDetailedWindow = false;
    for( int i = 0; i < app->GetNumberOfWindows(); i++ ) {
        if( app->GetNthWindow(i) == this->windowLevelWindow ) {
            foundDetailedWindow = true;
        }
    }

    if( !foundDetailedWindow ) {
        app->AddWindow( this->windowLevelWindow );
    }
}


/*
 *
 */
void vtkSivicController::SetSpecUnitsCallback(int targetUnits)
{
    this->spectraRangeWidget->SetSpecUnitsCallback( static_cast<svkSpecPoint::UnitType>(targetUnits) );
    this->quantificationWidget->SetSpecUnits( static_cast<svkSpecPoint::UnitType>(targetUnits) );
}


/*
 *
 */
void vtkSivicController::SetDSCRepresentationCallback(int representation)
{
cout << "SIVIC CONTROLLER DSC CALL: " << representation << endl;
    this->dscWidget->SetDSCRepresentationCallback( static_cast<svkDSCDeltaR2::representation>(representation) );
}


/*!
 *
 */
void vtkSivicController::SetComponentCallback( int targetComponent)
{
    string acquisitionType;
    char registryValue[100] = "";
    string registryValueString = "";
    bool syncComponents = true;

    // Lets grab the printer name from the registry
    this->GetApplication()->GetRegistryValue( 0, "defaults", "sync_components", registryValue );
    if( registryValue != NULL && strcmp( registryValue, "active" ) == 0 ) {
    	syncComponents = false;
    }
    if ( targetComponent == svkPlotLine::REAL) {

    	if( syncComponents ) {
			svkPlotGridView::SafeDownCast(this->plotController->GetView())->SetComponent(svkPlotLine::REAL);
    	} else {
			svkPlotGridView::SafeDownCast(this->plotController->GetView())->SetActiveComponent(svkPlotLine::REAL);
    	}
        this->spectraRangeWidget->componentSelectBox->SetValue( "real");

    } else if ( targetComponent == svkPlotLine::IMAGINARY) {

    	if( syncComponents ) {
			svkPlotGridView::SafeDownCast(this->plotController->GetView())->SetComponent(svkPlotLine::IMAGINARY);
    	} else {
			svkPlotGridView::SafeDownCast(this->plotController->GetView())->SetActiveComponent(svkPlotLine::IMAGINARY);
    	}
        this->spectraRangeWidget->componentSelectBox->SetValue( "imag");

    } else if ( targetComponent == svkPlotLine::MAGNITUDE) {

    	if( syncComponents ) {
			svkPlotGridView::SafeDownCast(this->plotController->GetView())->SetComponent(svkPlotLine::MAGNITUDE);
    	} else {
			svkPlotGridView::SafeDownCast(this->plotController->GetView())->SetActiveComponent(svkPlotLine::MAGNITUDE);
    	}
        this->spectraRangeWidget->componentSelectBox->SetValue( "mag");

    }
    if( model->DataExists( "SpectroscopicData" ) ) {
        acquisitionType = model->GetDataObject( "SpectroscopicData" )->
            GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
        if( acquisitionType == "SINGLE VOXEL" ) {
            this->ResetRange(1,1);
        } 
    }
}


/*!
 *
 */
void vtkSivicController::SetInterpolationCallback( int interpolationType )
{
    if ( interpolationType == svkOverlayView::NEAREST) {
        static_cast<svkOverlayViewController*>(this->overlayController)->SetInterpolationType(0);
    } else if ( interpolationType == svkOverlayView::LINEAR) {
        static_cast<svkOverlayViewController*>(this->overlayController)->SetInterpolationType(1);
    } else if ( interpolationType == svkOverlayView::SINC) {
        static_cast<svkOverlayViewController*>(this->overlayController)->SetInterpolationType(2);
    }
}


/*!
 *
 */
void vtkSivicController::SetLUTCallback( int type )
{
    if ( type == svkLookupTable::COLOR) {
        static_cast<svkOverlayViewController*>( this->overlayController)->SetLUT( svkLookupTable::COLOR );
        static_cast<svkPlotGridViewController*>( this->plotController)->SetLUT( svkLookupTable::COLOR );
    } else if ( type == svkLookupTable::GREY_SCALE) {
        static_cast<svkOverlayViewController*>( this->overlayController)->SetLUT( svkLookupTable::GREY_SCALE );
        static_cast<svkPlotGridViewController*>( this->plotController)->SetLUT( svkLookupTable::GREY_SCALE );
    } else if ( type == svkLookupTable::HURD) {
        static_cast<svkOverlayViewController*>( this->overlayController)->SetLUT( svkLookupTable::HURD );
        static_cast<svkPlotGridViewController*>( this->plotController)->SetLUT( svkLookupTable::HURD );
    } else if ( type == svkLookupTable::CYAN_HOT ) {
        static_cast<svkOverlayViewController*>( this->overlayController)->SetLUT( svkLookupTable::CYAN_HOT );
        static_cast<svkPlotGridViewController*>( this->plotController)->SetLUT( svkLookupTable::CYAN_HOT );
    } else if ( type == svkLookupTable::FIRE ) {
        static_cast<svkOverlayViewController*>( this->overlayController)->SetLUT( svkLookupTable::FIRE );
        static_cast<svkPlotGridViewController*>( this->plotController)->SetLUT( svkLookupTable::FIRE );
    } else if ( type == svkLookupTable::REVERSE_COLOR ) {
        static_cast<svkOverlayViewController*>( this->overlayController)->SetLUT( svkLookupTable::REVERSE_COLOR );
        static_cast<svkPlotGridViewController*>( this->plotController)->SetLUT( svkLookupTable::REVERSE_COLOR );
    }
}


/*!
 *  Sets the overlay view in plot grid
 */
void vtkSivicController::MetMapViewCallback( int mapNumber)
{
    this->quantificationWidget->SetOverlay( 
        this->quantificationWidget->modelMetNames[mapNumber] 
    ); 
}


/*!
 *  Sets the overlay view in plot grid
 */
void vtkSivicController::DSCMapViewCallback( int mapNumber)
{
cout << "OVERLAY MAP " << mapNumber << " " << this->dscWidget->modelDSCNames[mapNumber] << endl;
    this->dscWidget->SetOverlay( 
        this->dscWidget->modelDSCNames[mapNumber] 
    ); 
}


/*!
 *
 */
void vtkSivicController::Print(char* captureType, int outputOption )
{
    if( this->GetActive4DImageData() == NULL ) {
        this->PopupMessage( "NO SPECTRA LOADED!");
        return; 
    }
    string defaultNamePattern;
    int seriesNumber =  svkImageWriterFactory::GetNewSeriesFilePattern( 
            this->GetActive4DImageData() ,
            &defaultNamePattern
            );
    this->SaveSecondaryCapture( "tmpImage.ps", seriesNumber, captureType, outputOption, 1 );
}


/*!
 *  style argument can be use to set the vtkKWMessageDialog style 
 *  type  (default, yes/no, etc.)
 *  returns the dialog status value. 
 */
int vtkSivicController::PopupMessage( string message, int style ) 
{
    vtkKWMessageDialog *messageDialog = vtkKWMessageDialog::New();
    messageDialog->SetApplication(app);
    messageDialog->SetStyle( style );
    messageDialog->Create();
    messageDialog->SetOptions( vtkKWMessageDialog::ErrorIcon );
    messageDialog->SetText(message.c_str());
    messageDialog->Invoke();
    //  yes(2), no(1)
    int status = messageDialog->GetStatus();
    messageDialog->Delete();
    return status; 
}


/*!
 *
 */
string vtkSivicController::GetPrinterName( )
{
    // Once we have the printer dialog working we can change this.
    char registryPrinterName[100] = "";
    string printerNameString = "";

    // Lets grab the printer name from the registry
    this->app->GetRegistryValue( 0, "defaults", "printer", registryPrinterName );
    if( registryPrinterName != NULL && strcmp( registryPrinterName, "" ) != 0 ) {
		printerNameString = registryPrinterName;
    } else {
    	printerNameString = "jasmine";
    }
    cout << "Using printer: " << printerNameString << endl;

    return printerNameString;

}



/*!
 * Sets the orientation to
 */
void vtkSivicController::SetOrientation( const char* orientation, bool alignOverlay ) 
{

    //  Get the orientation  
    //this->mainWindow->GetMainToolbarSet()->GetToolbarsFrame()->GetChildWidgetWithName("Main Toolbar");

    vtkKWPushButtonWithMenu* orientationButton = static_cast<vtkKWPushButtonWithMenu*>(
        this->mainWindow->GetMainToolbarSet()->GetNthToolbar(0)->GetNthWidget(11)
    ); ; 
    //orientationButton->GetMenu()->SelectItemWithSelectedValueAsInt(1);


    // Set Our orientation member variable
    svkDcmHeader::Orientation newOrientation = svkDcmHeader::UNKNOWN_ORIENTATION;
    this->orientation = orientation;
    int firstSlice;
    int lastSlice;
    int toggleDraw = this->GetDraw();
    if( toggleDraw ) {
        this->DrawOff();
    }

    int index; 
    if( this->orientation == "AXIAL" ) {
        this->plotController->GetView()->SetOrientation( svkDcmHeader::AXIAL );
        this->overlayController->GetView()->SetOrientation( svkDcmHeader::AXIAL );
        this->secondaryCaptureFormatter->SetOrientation( svkDcmHeader::AXIAL );
        this->viewRenderingWidget->SetOrientation( svkDcmHeader::AXIAL );
        if( alignOverlay || this->overlayController->GetCurrentStyle() == svkOverlayViewController::SELECTION ) {
            svkOverlayView::SafeDownCast( this->overlayController->GetView())->AlignCamera();
        }
        newOrientation = svkDcmHeader::AXIAL;
        index = orientationButton->GetMenu()->GetIndexOfItem("Axial");
    } else if ( this->orientation == "CORONAL" ) {
        this->plotController->GetView()->SetOrientation( svkDcmHeader::CORONAL );
        this->overlayController->GetView()->SetOrientation( svkDcmHeader::CORONAL );
        this->secondaryCaptureFormatter->SetOrientation( svkDcmHeader::CORONAL );
        this->viewRenderingWidget->SetOrientation( svkDcmHeader::CORONAL );
        if( alignOverlay || this->overlayController->GetCurrentStyle() == svkOverlayViewController::SELECTION ) {
            svkOverlayView::SafeDownCast( this->overlayController->GetView())->AlignCamera();
        }
        newOrientation = svkDcmHeader::CORONAL;
        index = orientationButton->GetMenu()->GetIndexOfItem("Coronal");
    } else if ( this->orientation == "SAGITTAL" ) {
        this->plotController->GetView()->SetOrientation( svkDcmHeader::SAGITTAL );
        this->overlayController->GetView()->SetOrientation( svkDcmHeader::SAGITTAL );
        this->secondaryCaptureFormatter->SetOrientation( svkDcmHeader::SAGITTAL );
        this->viewRenderingWidget->SetOrientation( svkDcmHeader::SAGITTAL );
        if( alignOverlay || this->overlayController->GetCurrentStyle() == svkOverlayViewController::SELECTION ) {
            svkOverlayView::SafeDownCast( this->overlayController->GetView())->AlignCamera();
        }
        newOrientation = svkDcmHeader::SAGITTAL;
        index = orientationButton->GetMenu()->GetIndexOfItem("Sagittal");
    }
    if( index != orientationButton->GetMenu()->GetIndexOfSelectedItem()) {
        orientationButton->GetMenu()->SelectItem( index );
    }

    if( this->GetActive4DImageData() != NULL ) {
        firstSlice = this->GetActive4DImageData()->GetFirstSlice( newOrientation );
        lastSlice =  this->GetActive4DImageData()->GetLastSlice( newOrientation );
        this->spectraViewWidget->sliceSlider->SetRange( firstSlice + 1, lastSlice + 1 );
        this->spectraViewWidget->sliceSlider->SetValue( this->plotController->GetSlice()+1 );
        this->SetSlice( this->plotController->GetSlice());
        string acquisitionType = "UNKNOWN";
        if( this->GetActive4DImageData()->IsA("svkMrsImageData")) {
            acquisitionType = model->GetDataObject( "SpectroscopicData" )->
                                GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
        }
		this->overlayController->HighlightSelectionVoxels();
		this->plotController->HighlightSelectionVoxels();
    }

    this->EnableWidgets();
    if( toggleDraw ) {
        this->DrawOn();
    }
}


void vtkSivicController::SaveSession( )
{
    this->app->SetRegistryValue( 0, "open_files", "image", 
            model->GetDataFileName("AnatomicalData").c_str());
    this->app->SetRegistryValue( 0, "open_files", "spectra",
            model->GetDataFileName("SpectroscopicData").c_str());
    this->app->SetRegistryValue( 0, "open_files", "overlay",
            model->GetDataFileName("OverlayData").c_str());
    this->app->SetRegistryValue( 0, "open_files", "metabolite",
            model->GetDataFileName("MetaboliteData").c_str());

}


void vtkSivicController::RestoreSession( )
{
    char fileName[200];
    this->ResetApplication();
    this->imageViewWidget->loadingLabel->SetText("Restoring Session...");
    this->app->ProcessPendingEvents(); 
    this->imageViewWidget->Focus();
    this->app->GetRegistryValue( 0, "open_files", "image", fileName );
    if( fileName != NULL && strcmp( fileName, "" ) != 0 ) {
        this->OpenImage( fileName );
    }
    this->app->GetRegistryValue( 0, "open_files", "spectra", fileName );
    if( fileName != NULL && strcmp( fileName, "" ) != 0 ) {
        this->Open4DImage( fileName );
    }
    this->app->GetRegistryValue( 0, "open_files", "metabolite", fileName );
    if( fileName != NULL && strcmp( fileName, "" ) != 0 ) {
        this->OpenOverlay( fileName );
    }
    this->app->GetRegistryValue( 0, "open_files", "overlay", fileName );
    if( fileName != NULL && strcmp( fileName, "" ) != 0 ) {
        this->OpenOverlay( fileName );
    }
    this->imageViewWidget->loadingLabel->SetText("");
    this->EnableWidgets();
    this->viewRenderingWidget->ResetInfoText();
}


/*!
 * Resets the x and y ranges
 */
void vtkSivicController::ResetRange( bool useFullFrequencyRange, bool useFullAmplitudeRange,
                                          bool resetAmplitude, bool resetFrequency)
{ 
    svk4DImageData* data = this->GetActive4DImageData();
    if( data != NULL ) {
        this->spectraRangeWidget->ResetRange( useFullFrequencyRange, useFullAmplitudeRange,
                                              resetAmplitude, resetFrequency );
        this->plotController->GetView()->Refresh();

    }
}


/*!
 * Resets the current channel and the number of channels 
 */
void vtkSivicController::ResetChannel( )
{
    svkImageData* data = this->GetActive4DImageData();
    if( data != NULL ) {
        int channels = data->GetDcmHeader()->GetNumberOfCoils();
        this->spectraViewWidget->channelSlider->SetRange( 1, channels); 
        this->spectraViewWidget->channelSlider->SetValue( 1 );
    }
}


/*
 *  Logic to determine whether to enable widgets. 
 */
void vtkSivicController::EnableWidgets()
{
    this->DisableWidgets();
    svk4DImageData* activeData = this->GetActive4DImageData();
    if ( activeData != NULL && model->DataExists("AnatomicalData") ) {

#if defined( UCSF_INTERNAL )
        //  Enable the UCSF metabolite widgets
        this->GetApplication()->GetNthWindow(0)->GetMenu()->GetNthChild(3)->GetNthChild(0)->EnabledOn(); 
#endif
        if( activeData->IsA("svkMrsImageData") ) {
            string acquisitionType = model->GetDataObject( "SpectroscopicData" )->
                               GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
            if( acquisitionType == "SINGLE VOXEL" ) {
                this->imageViewWidget->plotGridButton->EnabledOff();
            } else {
				this->imageViewWidget->plotGridButton->EnabledOn();
				this->imageViewWidget->plotGridButton->InvokeEvent(vtkKWCheckButton::SelectedStateChangedEvent);
            }
        } else {
            this->imageViewWidget->plotGridButton->EnabledOn();
            this->imageViewWidget->plotGridButton->InvokeEvent(vtkKWCheckButton::SelectedStateChangedEvent);
        }
    }


    if ( activeData != NULL ) {
        this->spectraRangeWidget->xSpecRange->EnabledOn();
        this->spectraRangeWidget->ySpecRange->EnabledOn();
        this->spectraViewWidget->sliceSlider->EnabledOn();
        this->spectraRangeWidget->unitSelectBox->EnabledOff();
    }

    if ( activeData != NULL && activeData->IsA("svkMrsImageData") ) {
        string domain = model->GetDataObject( "SpectroscopicData" )->GetDcmHeader()->GetStringValue("SignalDomainColumns");
        int numChannels = svkMrsImageData::SafeDownCast( model->GetDataObject("SpectroscopicData"))->GetDcmHeader()->GetNumberOfCoils();
        if( domain == "TIME" ) {
            this->preprocessingWidget->applyButton->EnabledOn(); 
            this->preprocessingWidget->zeroFillSelectorSpec->EnabledOn();
            this->preprocessingWidget->zeroFillSelectorCols->EnabledOn();
            this->preprocessingWidget->zeroFillSelectorRows->EnabledOn();
            this->preprocessingWidget->zeroFillSelectorSlices->EnabledOn();
            this->preprocessingWidget->apodizationSelectorSpec->EnabledOn();
            this->preprocessingWidget->customValueEntry->EnabledOn();
            this->processingWidget->fftButton->EnabledOn(); 
            this->processingWidget->phaseButton->EnabledOff(); 
			this->combineWidget->magnitudeCombinationButton->EnabledOff();
			this->combineWidget->additionCombinationButton->EnabledOff();
            this->spectraRangeWidget->xSpecRange->SetLabelText( "Time" );
            this->spectraRangeWidget->unitSelectBox->SetValue( "PTS" );
            this->spectraRangeWidget->SetSpecUnitsCallback(svkSpecPoint::PTS);
            this->spectraRangeWidget->unitSelectBox->EnabledOff();
        } else {
            this->processingWidget->fftButton->EnabledOff(); 
            this->processingWidget->phaseButton->EnabledOn(); 
            if( numChannels > 1 ) {
				this->combineWidget->magnitudeCombinationButton->EnabledOn();
				this->combineWidget->additionCombinationButton->EnabledOn();
            }
            this->spectraRangeWidget->xSpecRange->SetLabelText( "Frequency" );
            this->spectraRangeWidget->unitSelectBox->EnabledOn();

            this->quantificationWidget->EnableWidgets();

        }
        this->imageViewWidget->satBandButton->EnabledOn();
        this->imageViewWidget->satBandButton->InvokeEvent(vtkKWCheckButton::SelectedStateChangedEvent);
        this->imageViewWidget->satBandOutlineButton->EnabledOn();
        this->imageViewWidget->satBandOutlineButton->InvokeEvent(vtkKWCheckButton::SelectedStateChangedEvent);
        this->processingWidget->phaseSlider->EnabledOn(); 
        this->processingWidget->phaseAllChannelsButton->EnabledOn(); 
        this->processingWidget->phaseAllVoxelsButton->EnabledOn(); 
        this->spectraRangeWidget->componentSelectBox->EnabledOn();
        if( numChannels > 1 ) {
            this->spectraViewWidget->channelSlider->EnabledOn();
        }
        int numTimePoints = svkMrsImageData::SafeDownCast( model->GetDataObject("SpectroscopicData"))->GetDcmHeader()->GetNumberOfTimePoints();
        if( numTimePoints > 1 ) {
            this->spectraViewWidget->timePointSlider->EnabledOn();
        }
        this->imageViewWidget->volSelButton->EnabledOn();
        this->imageViewWidget->volSelButton->InvokeEvent(vtkKWCheckButton::SelectedStateChangedEvent);
    }

    if ( model->DataExists("AnatomicalData") ) {
        if( this->overlayController->GetCurrentStyle() == svkOverlayViewController::ROTATION ) {
            this->imageViewWidget->axialSlider->EnabledOn();
            this->imageViewWidget->coronalSlider->EnabledOn();
            this->imageViewWidget->sagittalSlider->EnabledOn();
        } else if( this->orientation == "AXIAL" ) {
            this->imageViewWidget->axialSlider->EnabledOn();
            this->imageViewWidget->coronalSlider->EnabledOff();
            this->imageViewWidget->sagittalSlider->EnabledOff();
        } else if ( this->orientation == "CORONAL" ) {
            this->imageViewWidget->axialSlider->EnabledOff();
            this->imageViewWidget->coronalSlider->EnabledOn();
            this->imageViewWidget->sagittalSlider->EnabledOff();
        } else if ( this->orientation == "SAGITTAL" ) {
            this->imageViewWidget->axialSlider->EnabledOff();
            this->imageViewWidget->coronalSlider->EnabledOff();
            this->imageViewWidget->sagittalSlider->EnabledOn();
        }
        this->windowLevelWidget->EnabledOn();
        if( model->GetDataObject("AnatomicalData")->GetDcmHeader()->GetNumberOfTimePoints() > 1) {
            this->imageViewWidget->volumeSlider->EnabledOn();
        }

    }

    if ( model->DataExists("MetaboliteData")) {
        this->spectraViewWidget->overlayImageCheck->EnabledOn();
        this->spectraViewWidget->overlayImageCheck->InvokeEvent(vtkKWCheckButton::SelectedStateChangedEvent);
        this->spectraViewWidget->overlayTextCheck->EnabledOn();
        this->spectraViewWidget->overlayTextCheck->InvokeEvent(vtkKWCheckButton::SelectedStateChangedEvent);
        this->overlayWindowLevelWidget->EnabledOn();
    } 
    if ( model->DataExists("OverlayData") ||  model->DataExists("MetaboliteData")) {
        this->imageViewWidget->interpolationBox->EnabledOn();
        this->imageViewWidget->interpolationBox->GetWidget()->InvokeEvent( vtkKWMenu::MenuItemInvokedEvent );
        this->imageViewWidget->lutBox->EnabledOn();
        this->imageViewWidget->lutBox->InvokeEvent( vtkKWMenu::MenuItemInvokedEvent );
        this->imageViewWidget->thresholdType->EnabledOn();
        this->imageViewWidget->thresholdType->InvokeEvent( vtkKWMenu::MenuItemInvokedEvent );
        this->imageViewWidget->overlayOpacitySlider->EnabledOn();
        this->imageViewWidget->overlayThresholdSlider->EnabledOn();
        this->imageViewWidget->overlayButton->EnabledOn();
        this->imageViewWidget->overlayButton->InvokeEvent(vtkKWCheckButton::SelectedStateChangedEvent);
        this->imageViewWidget->colorBarButton->EnabledOn();
        this->imageViewWidget->colorBarButton->InvokeEvent(vtkKWCheckButton::SelectedStateChangedEvent);
        this->overlayWindowLevelWidget->EnabledOn();
        this->imageViewWidget->overlayVolumeSlider->EnabledOn();
    }
}


/*!
 *
 */
void vtkSivicController::DisableWidgets()
{
    this->imageViewWidget->volSelButton->EnabledOff();
    this->imageViewWidget->plotGridButton->EnabledOff();
    this->imageViewWidget->overlayVolumeSlider->EnabledOff();

    this->spectraViewWidget->sliceSlider->EnabledOff();
    this->spectraViewWidget->channelSlider->EnabledOff();
    this->spectraViewWidget->timePointSlider->EnabledOff();
    this->imageViewWidget->axialSlider->EnabledOff();
    this->imageViewWidget->coronalSlider->EnabledOff();
    this->imageViewWidget->sagittalSlider->EnabledOff();
    this->spectraViewWidget->overlayImageCheck->EnabledOff();
    this->spectraViewWidget->overlayTextCheck->EnabledOff();
    this->spectraRangeWidget->unitSelectBox->EnabledOff();
    this->spectraRangeWidget->componentSelectBox->EnabledOff();
    this->spectraRangeWidget->xSpecRange->EnabledOff();
    this->spectraRangeWidget->ySpecRange->EnabledOff();

    this->processingWidget->phaseSlider->EnabledOff(); 
    this->processingWidget->phaseAllChannelsButton->EnabledOff(); 
    this->processingWidget->phaseAllVoxelsButton->EnabledOff(); 
    this->processingWidget->fftButton->EnabledOff(); 
    this->processingWidget->phaseButton->EnabledOff(); 
	this->combineWidget->magnitudeCombinationButton->EnabledOff();
	this->combineWidget->additionCombinationButton->EnabledOff();

    this->preprocessingWidget->applyButton->EnabledOff(); 
    this->preprocessingWidget->zeroFillSelectorSpec->EnabledOff();
    this->preprocessingWidget->zeroFillSelectorCols->EnabledOff();
    this->preprocessingWidget->zeroFillSelectorRows->EnabledOff();
    this->preprocessingWidget->zeroFillSelectorSlices->EnabledOff();
	this->preprocessingWidget->customValueEntry->EnabledOff();
    this->preprocessingWidget->apodizationSelectorSpec->EnabledOff();
    this->preprocessingWidget->apodizationSelectorCols->EnabledOff();
    this->preprocessingWidget->apodizationSelectorRows->EnabledOff();
    this->preprocessingWidget->apodizationSelectorSlices->EnabledOff();

    this->imageViewWidget->orthImagesButton->EnabledOff();

    this->imageViewWidget->interpolationBox->EnabledOff();
    this->imageViewWidget->lutBox->EnabledOff();
    this->imageViewWidget->thresholdType->EnabledOff();
    this->imageViewWidget->overlayOpacitySlider->EnabledOff();
    this->imageViewWidget->overlayThresholdSlider->EnabledOff();
    this->imageViewWidget->overlayButton->EnabledOff();
    this->imageViewWidget->colorBarButton->EnabledOff();
    this->imageViewWidget->satBandButton->EnabledOff();
    this->imageViewWidget->satBandOutlineButton->EnabledOff();
    this->overlayWindowLevelWidget->EnabledOff();
    this->windowLevelWidget->EnabledOff();
#if defined( UCSF_INTERNAL )
    //  disable the UCSF metabolite widgets
    this->GetApplication()->GetNthWindow(0)->GetMenu()->GetNthChild(3)->GetNthChild(0)->EnabledOff(); 
#endif
}


/*!
 * Update the threshold to the current value. This is used when window levelling.
 */
void vtkSivicController::UpdateThreshold( )
{
    this->SetOverlayThreshold( this->imageViewWidget->overlayThresholdSlider->GetValue() );
}


/*!
 *  This method is used to set the threshold type for changing the threshold of
 *  overlays. Threshold types can have values "Quantity" or "Percent". The 
 *  vtk wrapper will not allow us to declace an enumeration, that is why strings
 *  are used. 
 */
void vtkSivicController::SetThresholdType( string thresholdType )
{
    if( thresholdType == "Quantity" ) {
        this->SetThresholdTypeToQuantity();
    } else if (thresholdType == "Percent") {
        this->SetThresholdTypeToPercent();
    }
}


/*!
 *  
 */
string vtkSivicController::GetThresholdType()
{
    return this->thresholdType;
}


/*!
 *  
 */
int vtkSivicController::GetFrequencyType()
{
    return this->spectraRangeWidget->specUnits;
}


/*!
 *  
 */
void vtkSivicController::SetThresholdTypeToPercent()
{
    this->imageViewWidget->thresholdType->GetWidget()->SetValue("Percent");
    this->thresholdType = "Percent";
    svkImageData* overlay = this->model->GetDataObject("OverlayData");
    this->imageViewWidget->overlayThresholdSlider->SetResolution( 1 );
    this->imageViewWidget->overlayThresholdSlider->SetRange(0,100);
    if( overlay == NULL ) {
        overlay = this->model->GetDataObject("MetaboliteData");
    }
    if( overlay == NULL ) {
        this->imageViewWidget->overlayThresholdSlider->SetValue(0);
    } else {
        int threshold = (int)(100*this->overlayController->GetOverlayThreshold());
        this->imageViewWidget->overlayThresholdSlider->SetValue( threshold );
    }
}


/*!
 *  
 */
void vtkSivicController::SetThresholdTypeToQuantity()
{
    this->imageViewWidget->thresholdType->GetWidget()->SetValue("Quantity");
    this->thresholdType = "Quantity";
    svkImageData* overlay = this->model->GetDataObject("OverlayData");
    if( overlay == NULL ) {
        overlay = this->model->GetDataObject("MetaboliteData");
    }
    if( overlay == NULL ) {
        this->imageViewWidget->overlayThresholdSlider->SetValue(0);
        this->imageViewWidget->overlayThresholdSlider->SetRange(0,0);
    } else {
        double* dataRange = svkOverlayView::SafeDownCast(overlayController->GetView())->GetLookupTable()->GetRange();
        int numTableVals = svkOverlayView::SafeDownCast(overlayController->GetView())->GetLookupTable()->GetNumberOfTableValues();
        double threshold = this->overlayController->GetOverlayThreshold();
        this->imageViewWidget->overlayThresholdSlider->SetResolution( (dataRange[1] - dataRange[0])/(numTableVals*0.99) );
        this->imageViewWidget->overlayThresholdSlider->SetRange( dataRange[0] , dataRange[1] );
        this->imageViewWidget->overlayThresholdSlider->SetValue( dataRange[0] + threshold * ( dataRange[1] - dataRange[0] ));
    }
}


/*!
 *  
 */
void vtkSivicController::SetOverlayThreshold( double threshold )
{
    if( this->thresholdType == "Percent" ) {
        this->overlayController->SetOverlayThreshold( threshold/100.0 );
        this->plotController->SetOverlayThreshold( threshold/100.0 );
        this->overlayController->GetView()->Refresh();
        this->plotController->GetView()->Refresh();
    } else if ( this->thresholdType == "Quantity" ) {
        svkImageData* overlay = this->model->GetDataObject("OverlayData");
        if( overlay == NULL ) {
            overlay = this->model->GetDataObject("MetaboliteData");
        }   
        if( overlay != NULL ) {
            int numTableVals = svkOverlayView::SafeDownCast(overlayController->GetView())->GetLookupTable()->GetNumberOfTableValues();
            double* dataRange = svkOverlayView::SafeDownCast(overlayController->GetView())->GetLookupTable()->GetRange();
            double thresholdQuantity = (threshold-dataRange[0])/(dataRange[1]-dataRange[0]);
            this->overlayController->SetOverlayThreshold( thresholdQuantity );
            if ( this->model->GetDataObject("MetaboliteData") != NULL ) {
                this->plotController->SetOverlayThreshold( thresholdQuantity );
            }

            this->overlayController->GetView()->Refresh();
            this->plotController->GetView()->Refresh();
        }

    }
}



/*! 
 *   This is current for UCSF only. This method will do the following...
 *   
 *   1. Create a temporary directary in the pacsDirectory below.
 *   2. Create a local directory one directory above where the spectra was loaded from
 *   3. Verify that it can write to both of these paths.
 *   4. Create a set of secondary captures and save them to the local directory.
 *   5. Open the secondary captures in a preview window and wait for user input.
 *   6. After the user has closed all of the secondary capture windows get a confirmation to send to PACS.
 *   7. If the user confirms to send then copy all images to the PACS temporary directory.
 *   8. Reidentify these images in the temporary directory.
 *   9. Move the images to the PACS directory.
 *  10. Delete the temporary PACS directory.
 */
void vtkSivicController::PushToPACS()
{
#if defined( UCSF_INTERNAL )

    if(     this->GetActive4DImageData() == NULL
         || this->model->GetDataObject( "AnatomicalData" ) == NULL ) {

        PopupMessage( "BOTH SPECTRA AND AN IMAGE MUST BE LOADED TO CREATE SECONDARY CAPTURES!" );
        return; 
    }

    svkPACSInterface* pacsInterface = svkUCSFPACSInterface::New();

    // For testing below
    //pacsInterface->SetPACSTargetString("/data/lhst3/sivic/DICOM_REID/results");

    /*********************************** CHECK FOR FILE WRITING PERMISSIONS ****************************************/

    // Lets locate a local directory to make a copy of the images being push to pacs. We'll use the spectra directory
    string activeDataFileName;
    if( this->GetActive4DImageData()->IsA("svkMrsImageData") ) {
        activeDataFileName = this->model->GetDataFileName( "SpectroscopicData" );
    } else {
        activeDataFileName = this->model->GetDataFileName( "4DImageData" );
    }

    // Parse for directory name
    size_t found;
    found = activeDataFileName.find_last_of("/");

    string localDirectory = activeDataFileName.substr(0,found);
    found = localDirectory.find_last_of("/");

    // We want to put the SIVIC_DICOM_SC folder parallel to the spectra location
    found = localDirectory.find_last_of("/");
    localDirectory = localDirectory.substr(0,found); 

    // We will write the images to the local directory
    localDirectory.append( "/SIVIC_DICOM_SC/" );

    // Let's create the local directory 
	if(!svkUtils::FilePathExists(localDirectory.c_str())) {
		int result = vtkDirectory::MakeDirectory( localDirectory.c_str() );
        if (result == 0) { //  Was the directory created?
            string errorMessage("ERROR: Could not create to directory: ");
            errorMessage.append( localDirectory );
            PopupMessage( errorMessage );
            return; 
        } 
    }

    // Make sure we can write to the new directory
	if (!svkUtils::CanWriteToPath(localDirectory.c_str())) { // Can the user get a file handle
        string errorMessage("ERROR: Could not write to directory: ");
        errorMessage.append( localDirectory );
        PopupMessage( errorMessage );
        return; 
	} 

    string captureDirectory = svkUtils::GetDefaultSecondaryCaptureDirectory( svkMriImageData::SafeDownCast( this->model->GetDataObject("AnatomicalData"))
                                                                           , svk4DImageData::SafeDownCast( this->GetActive4DImageData()) );

    // We will need to copy the images to the capture directory
    localDirectory.append( captureDirectory );
    localDirectory.append( "/" );

    // Let's create the local directory 
	if(svkUtils::FilePathExists(localDirectory.c_str())) {
        string message = "Secondary capture directory ";
        message.append(localDirectory);
        message.append(" already exists. This directory will be removed. Do you want to continue? ");
        if( this->PopupMessage(message, vtkKWMessageDialog::StyleOkCancel ) == vtkKWDialog::StatusOK ) {
            int result = vtkDirectory::DeleteDirectory( localDirectory.c_str() );
            if( result == 0 ) {
                string errorMessage = string("ERROR: COULD NOT REMOVE DIRECTORY: ");
                errorMessage.append(localDirectory);
                this->PopupMessage(errorMessage);
                return;
            }
        } else {
            return;
        }
    }

    // Now lets make the subdirectory
    vtkDirectory::MakeDirectory( localDirectory.c_str() );

    // Make sure we can write to the new directory
	if (!svkUtils::CanWriteToPath(localDirectory.c_str())) { // Can the user get a file handle
        string errorMessage("ERROR: COULD NOT WRITE TO PATH: ");
        errorMessage.append( localDirectory );
        PopupMessage( errorMessage );
        return; 
	} 

    string filePattern = svkUtils::GetDefaultSecondaryCaptureFilePattern( svkMriImageData::SafeDownCast( this->model->GetDataObject("AnatomicalData"))
                                                                        , svk4DImageData::SafeDownCast( this->GetActive4DImageData()) );
    // Lets create a name for the images 
    string fileNameString = localDirectory + filePattern;

    /*********************************** CREATE SECONDARY CAPTURES ******************************************************/

    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    vtkImageWriter* writer = NULL;

    // If the cursor location was on, we want to turn it off for the capture
    bool wasCursorLocationOn = 0;
    if( this->overlayController != NULL ) {
        if( this->viewRenderingWidget->viewerWidget->GetRenderWindow()
                        ->HasRenderer(this->overlayController->GetView()->GetRenderer( svkOverlayView::MOUSE_LOCATION ))) {
            wasCursorLocationOn = 1; 
            this->overlayController->GetView()->TurnRendererOff( svkOverlayView::MOUSE_LOCATION );    
        }
    }

    // Lets create our writer
    writer = writerFactory->CreateImageWriter( svkImageWriterFactory::DICOM_SC );
    writerFactory->Delete();

    // Lets get a new series number...
    string notUsed;
    int seriesNumber =  svkImageWriterFactory::GetNewSeriesFilePattern(
        this->GetActive4DImageData(),
        &notUsed
    );
    static_cast<svkImageWriter*>(writer)->SetSeriesNumber( seriesNumber );

    // And set the series description
    static_cast<svkImageWriter*>(writer)->SetSeriesDescription( "SIVIC secondary capture" );

    // We will save the current slice so we can return to it after the capture is done
    int currentSlice = this->plotController->GetSlice(); 
     
    bool print = 0; 
    bool preview = 1; 

    // We need to create an svkMriImageData object to hold the secondary capture data
    svkImageData* outputImage = svkMriImageData::New();
    
    // We are going to set its header to be the header of the image. This will only be used to derive the header for the secondary capture image.
    outputImage->SetDcmHeader( this->model->GetDataObject( "AnatomicalData" )->GetDcmHeader() );
    this->model->GetDataObject( "AnatomicalData" )->GetDcmHeader()->Register(this);

    // We need to givie it a dcos so that svkImageViewer can render it for the preview
    double dcos[3][3] = {{1,0,0}, {0,1,0}, {0,0,1}}; 
    this->model->GetDataObject( "AnatomicalData" )->GetDcos(dcos);
    outputImage->SetDcos( dcos );

    // write the images to the local directary and bring up a preview window
    // TODO: If we had a secondary capture reader this could be done it two steps for clarity
    this->secondaryCaptureFormatter->WriteCombinedWithSummaryCapture( 
                               writer, fileNameString, svkSecondaryCaptureFormatter::ALL_SLICES, outputImage, print, preview );

    // Reset the slice
    this->SetSlice(currentSlice);
    stringstream textString;
    textString <<"Are you sure you want to push to PACS?\nImages will be sent to " << endl; 
    if( pacsInterface->GetPACSTargetString().compare("") == 0 ) {
        textString << " default location." << endl; 
    } else { 
        textString << pacsInterface->GetPACSTargetString() << "." << endl; 
    }   

    bool confirmSend = false;
    if( this->PopupMessage(textString.str(), vtkKWMessageDialog::StyleOkCancel ) == vtkKWDialog::StatusOK ) {
        confirmSend = true;
    }

    /********************************** SEND IMAGES TO PACS *************************************/

    // Now we copy the local images to PACS
    if( confirmSend ) {
        int firstSlice = outputImage->GetExtent()[4];
        int lastSlice = outputImage->GetExtent()[5];

        bool pacsSendSuccess = pacsInterface->SendImagesToPACS( localDirectory );
        if (!pacsSendSuccess ) { 
            string errorMessage("ERROR: Could not send to PACS: ");
            errorMessage.append(pacsInterface->GetPACSTargetString());
            PopupMessage( errorMessage );
        } 
        
    }

    // Free our output image
    if (outputImage != NULL) {
        outputImage->Delete();
        outputImage = NULL; 
    }

    // Turn mouse position renderer back on if it was on when we started
    if( wasCursorLocationOn && this->overlayController != NULL ) {
        this->overlayController->GetView()->TurnRendererOn( svkOverlayView::MOUSE_LOCATION );    
    }

    // Free writer
    writer->Delete();

    // Refresh the views
    this->overlayController->GetView()->Refresh();    
    this->plotController->GetView()->Refresh();    
    this->viewRenderingWidget->infoWidget->Render();    

#endif
}

/*!
 *  Runs tests for the application.
 */
void vtkSivicController::RunTestingSuite()
{
    sivicTestSuite* suite = new sivicTestSuite( this );
    suite->RunTests();
}

void vtkSivicController::UpdateProgress(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    if( (*(double*)(callData)) >= 1 ) {
        static_cast<vtkSivicController*>(thisObject)->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue(0);
        static_cast<vtkSivicController*>(thisObject)->GetApplication()->GetNthWindow(0)->SetStatusText("Done");
    } else {
        static_cast<vtkSivicController*>(thisObject)->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 
                  100.0*(*(double*)(callData)) );
        if( subject->IsA("svkDataModel")) {
			static_cast<vtkSivicController*>(thisObject)->GetApplication()->GetNthWindow(0)->SetStatusText(
					  static_cast<svkDataModel*>(subject)->GetProgressText().c_str() );
        }
    }
}


//! Exit the application
void vtkSivicController::ExitSivic(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    // To end a KWW application you close all of its windows.
    
    vtkKWApplication* app =static_cast<vtkSivicController*>(thisObject)->GetApplication();
    for( int i = 0; i < app->GetNumberOfWindows(); i++ ) {
        if( app->GetNthWindow(i) != subject ) {
            app->GetNthWindow(i)->Close();
        }
    }

}


/*
 *
 */
void vtkSivicController::GetMRSDefaultPPMRange( svkImageData* mrsData, float& ppmMin, float& ppmMax )
{
    vtkstd::string nucleus = mrsData->GetDcmHeader()->GetStringValue( "ResonantNucleus" );
    if ( nucleus.find("13C") !=string::npos ) {
        ppmMin = PPM_13C_DEFAULT_MIN; 
        ppmMax = PPM_13C_DEFAULT_MAX;
    } else {
        ppmMin = PPM_1H_DEFAULT_MIN; 
        ppmMax = PPM_1H_DEFAULT_MAX;
    }
}


/*!
 *
 * @return
 */
svk4DImageData* vtkSivicController::GetActive4DImageData()
{
    return svkPlotGridView::SafeDownCast(this->plotController->GetView())->GetActiveInput();
}


/*!
 *  Method will set the currently displayed volume for any data set that matches the
 *  number of volumes of the input data set. We are making an assumption here that if
 *  two datasets have the same number of volumes then those volumes represent the same
 *  data.
 */
void vtkSivicController::SyncDisplayVolumes(svkImageData* data, int volume, int volumeIndex )
{
	if( data != NULL && this->synchronizeVolumes) {

		// Lets determine how many volumes the input data set has
		int inputNumVolumes = -1;
		if( data->IsA("svk4DImageData")) {
			inputNumVolumes = svk4DImageData::SafeDownCast( data )->GetVolumeIndexSize( volumeIndex );
		} else {
			inputNumVolumes = data->GetPointData()->GetNumberOfArrays();
		}
		if( inputNumVolumes < 0) {
			cout << "ERROR: No volumes found!" << endl;
			return;
		}

		// Check if the 4D timepoints or channels match the input data
		if ( this->GetActive4DImageData() != NULL ) {
			int num4DTimepoints = this->GetActive4DImageData()->GetVolumeIndexSize( svkMrsImageData::TIMEPOINT );
			int num4DChannels = this->GetActive4DImageData()->GetVolumeIndexSize( svkMrsImageData::CHANNEL );
			if( inputNumVolumes == num4DTimepoints && this->spectraViewWidget->timePointSlider->GetValue() != volume + 1 ) {
				this->spectraViewWidget->timePointSlider->SetValue( volume + 1 );
			} else if( inputNumVolumes == num4DChannels && this->spectraViewWidget->channelSlider->GetValue() != volume + 1 ) {
				this->spectraViewWidget->channelSlider->SetValue( volume + 1 );
			}
		}

		// Check if the reference/anatomical image matches the input data
		if ( this->model->DataExists("AnatomicalData") ) {
			int anatNumVolumes = this->model->GetDataObject("AnatomicalData")->GetPointData()->GetNumberOfArrays();
			if ( inputNumVolumes == anatNumVolumes ) {
				// Now lets update the slider value if necessary
				if( this->imageViewWidget->volumeSlider->GetValue() != volume + 1 ) {
					this->imageViewWidget->volumeSlider->SetValue( volume + 1 );
				}
			}
		}

		// Check if the metabolite overlay matches the input data
		if ( this->model->DataExists("MetaboliteData") ) {
			bool isImageVoxelTags = svkVoxelTaggingUtils::IsImageVoxelTagData(this->model->GetDataObject("MetaboliteData"));
			if( !isImageVoxelTags ) {
				int metabNumVolumes = this->model->GetDataObject("MetaboliteData")->GetPointData()->GetNumberOfArrays();
				if ( inputNumVolumes == metabNumVolumes ) {
					if( !this->model->DataExists("OverlayData") ) {
						if( this->imageViewWidget->overlayVolumeSlider->GetValue() != volume + 1 ) {
							this->imageViewWidget->overlayVolumeSlider->SetValue( volume + 1 );
						}
					} else {
						svkPlotGridView::SafeDownCast( this->plotController->GetView() )->SetActiveOverlayVolume( volume );
					}
				}
			}
		}

		// Check if the image overlay matches the input data
		if ( this->model->DataExists("OverlayData") ) {
			bool isImageVoxelTags = svkVoxelTaggingUtils::IsImageVoxelTagData(this->model->GetDataObject("OverlayData"));
			if( !isImageVoxelTags ) {
				int overlayNumVolumes = this->model->GetDataObject("OverlayData")->GetPointData()->GetNumberOfArrays();
				if ( inputNumVolumes == overlayNumVolumes ) {
					if( this->imageViewWidget->overlayVolumeSlider->GetValue() != volume + 1 ) {
						this->imageViewWidget->overlayVolumeSlider->SetValue( volume + 1 );
					}
				}
			}
		}

	}
}


/*!
 * Stops the renderers from updating. This is to make transitions appear fluid.
 */
void vtkSivicController::DrawOff()
{
	this->viewRenderingWidget->specViewerWidget->GetRenderWindow()->SwapBuffersOff();
	this->viewRenderingWidget->viewerWidget->GetRenderWindow()->SwapBuffersOff();
	this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOff();
	this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->DrawOff();

}


/*!
 *  Starts the renderers updating again. This is to make transitions appear fluid.
 */
void vtkSivicController::DrawOn()
{
	this->plotController->GetView()->Refresh();
	this->overlayController->GetView()->Refresh();
	this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
	this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->DrawOn();
	this->viewRenderingWidget->specViewerWidget->GetRenderWindow()->SwapBuffersOn();
	this->viewRenderingWidget->viewerWidget->GetRenderWindow()->SwapBuffersOn();
	this->plotController->GetView()->Refresh();
	this->overlayController->GetView()->Refresh();

}


/*!
 *  Just a check to see if the renders are currently drawing.
 */
int vtkSivicController::GetDraw()
{
	// Both renderer
	int drawOverlay  = this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
	int drawPlotGrid = this->plotController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();

	if( drawOverlay != drawPlotGrid ) {
		return 0;
	} else {
		return drawOverlay;
	}
}


/*!
 * Turn on the text for the overlay.
 *
 */
void vtkSivicController::OverlayTextOn()
{
	this->spectraViewWidget->overlayTextCheck->SetSelectedState( true );
	this->spectraViewWidget->overlayTextCheck->InvokeEvent(vtkKWCheckButton::SelectedStateChangedEvent);
}

/*!
 * Turn off the text for the overlay.
 *
 */
void vtkSivicController::OverlayTextOff()
{
	this->spectraViewWidget->overlayTextCheck->SetSelectedState( false );
	this->spectraViewWidget->overlayTextCheck->InvokeEvent(vtkKWCheckButton::SelectedStateChangedEvent);
}

/*!
 *
*/
void vtkSivicController::GenerateTraces( char* sourceImage )
{
	svkMriImageData* image = NULL;
	if( strcmp( sourceImage, "reference_image" ) == 0 ) {
		image = svkMriImageData::SafeDownCast(this->model->GetDataObject("AnatomicalData"));
	} else if( strcmp( sourceImage, "overlay_image" ) == 0 || strcmp( sourceImage, "add_overlay_image" ) == 0 ) {
		if( this->model->DataExists("MetaboliteData")) {
			image = svkMriImageData::SafeDownCast(this->model->GetDataObject("MetaboliteData"));
		} else {
			image = svkMriImageData::SafeDownCast(this->model->GetDataObject("OverlayData"));
		}
	}
	if( image != NULL ) {
		if(strcmp( sourceImage, "add_overlay_image" ) == 0 ) {
			this->Add4DImageData( image, "Cell Data Representation" );
		} else {
			this->Open4DImage( image, "Cell Data Representation" );
		}
	}
}


void vtkSivicController::DisplayImageDataInfo(int row, int column, int x, int y)
{
	vtkKWMenu* rightClickMenu = vtkKWMenu::New();
    string objectNameString = this->imageDataWidget->imageList->GetWidget()->GetCellText(row,4);
	svkImageData* selectedData = this->model->GetDataObject( objectNameString );
	string invocationString;
    invocationString = "DisplayHeader ";
    invocationString.append( this->imageDataWidget->imageList->GetWidget()->GetCellText(row,4));
    rightClickMenu->SetParent( this->GetApplication()->GetNthWindow(0) );
    rightClickMenu->Create();
    rightClickMenu->AddRadioButton("Show Info", this, invocationString.c_str());
    invocationString = "OpenOverlayFromModel ";
    invocationString.append( this->imageDataWidget->imageList->GetWidget()->GetCellText(row,4));
    rightClickMenu->AddRadioButton("Set As Overlay", this, invocationString.c_str());
    invocationString = "OpenImageFromModel ";
    invocationString.append( this->imageDataWidget->imageList->GetWidget()->GetCellText(row,4));
    rightClickMenu->AddRadioButton("Set As Reference Image", this, invocationString.c_str());
    invocationString = "SaveImageFromModel ";
    invocationString.append( this->imageDataWidget->imageList->GetWidget()->GetCellText(row,4));
    rightClickMenu->AddRadioButton("Save Image", this, invocationString.c_str());

	if( selectedData->GetPointData()->GetNumberOfArrays() > 1 ) {
		invocationString = "Open4DImageFromModel ";
		invocationString.append( objectNameString );
		rightClickMenu->AddRadioButton("Set As Dynamic Traces", this, invocationString.c_str());
		invocationString = "Add4DImageDataFromModel ";
		invocationString.append(objectNameString);
		rightClickMenu->AddRadioButton("Add As Dynamic Traces", this, invocationString.c_str());
	}
    rightClickMenu->PopUp(x,y);

}


/*!
 * Pops up a window to display the header for a given object in the model.
 */
void vtkSivicController::DisplayHeader( char* objectName )
{
	string objectNameString = string(objectName);
	if( this->model->DataExists( objectNameString ) ) {
		ostringstream os;
		this->model->GetDataObject( objectNameString )->GetDcmHeader()->PrintDcmHeader(os);
		vtkKWWindowBase* infoWindow = vtkKWWindowBase::New();
		this->app->AddWindow( infoWindow );
		infoWindow->Create();
		int width;
		int height;
		//this->mainWindow->GetSize(&width, &height);
		//infoWindow->SetSize( width, height);

		vtkKWTextWithScrollbarsWithLabel* textHolder = vtkKWTextWithScrollbarsWithLabel::New();
		textHolder->SetParent( infoWindow->GetViewFrame() );
		textHolder->Create();
		textHolder->SetLabelText(objectName);
		textHolder->SetLabelPositionToTop();
		textHolder->GetWidget()->GetWidget()->ReadOnlyOn();
		textHolder->GetWidget()->VerticalScrollbarVisibilityOn();
		textHolder->GetWidget()->HorizontalScrollbarVisibilityOn();
		textHolder->GetWidget()->GetWidget()->SetWrapToNone();
		textHolder->GetWidget()->GetWidget()->AddTagMatcher("\\([a-z0-9]+,[a-z0-9]+\\)", "_fg_navy_tag_");
		textHolder->GetWidget()->GetWidget()->AddTagMatcher("# [^\n]*", "_fg_dark_green_tag_");
		textHolder->GetWidget()->GetWidget()->SetText( os.str().c_str() );
		this->app->Script("pack %s -in %s -side top -fill both -expand y "
				, textHolder->GetWidgetName(), infoWindow->GetViewFrame()->GetWidgetName());
		infoWindow->Display();
		infoWindow->Delete();
	} else {
		cout << "No object name:" << objectName << " in model " << endl;
	}
}
