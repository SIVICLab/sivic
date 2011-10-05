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
    this->detailedPlotController = svkDetailedPlotViewController::New();
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
    this->secondaryCaptureFormatter->SetDetailedPlotController( this->detailedPlotController );
    this->secondaryCaptureFormatter->SetSivicController( this );
    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgress );
    this->progressCallback->SetClientData( (void*)this );
    this->exitSivicCallback = vtkCallbackCommand::New();
    this->exitSivicCallback->SetCallback( ExitSivic );
    this->exitSivicCallback->SetClientData( (void*)this );

    
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

    if( this->detailedPlotController != NULL ) {
        this->detailedPlotController->Delete();
        this->detailedPlotController = NULL;
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
    int toggleDraw = this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOff();
        this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->DrawOff();
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
        this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
        this->plotController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
    }
    this->overlayController->GetView()->Refresh();
    this->plotController->GetView()->Refresh();
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
    this->plotController->GetView()->GetRenderer(svkPlotGridView::PRIMARY)->AddActor(
                                       this->plotController->GetView()->GetProp(svkPlotGridView::VOL_SELECTION));
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
void vtkSivicController::SetActiveSpectra( int index )
{
    if( this->model->DataExists("SpectroscopicData") && index >= 0 ) {
       svkImageData* oldData = model->GetDataObject("SpectroscopicData");
       if( index <= svkPlotGridView::SafeDownCast(this->plotController->GetView())->GetNumberOfReferencePlots()) {
           int toggleDraw = this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
           if( toggleDraw ) {
               this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOff();
               this->plotController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOff();
            }
           svkPlotGridView::SafeDownCast( this->plotController->GetView() )->SetActiveSpectraIndex( index );
           svkImageData* newSpec = svkPlotGridView::SafeDownCast(plotController->GetView())->GetActiveSpectra();
           this->model->ChangeDataObject("SpectroscopicData", newSpec );
           this->processingWidget->InitializePhaser();
           int channels = svkMrsImageData::SafeDownCast( newSpec )->GetDcmHeader()->GetNumberOfCoils();
           this->spectraViewWidget->channelSlider->SetRange( 1, channels);
           this->spectraViewWidget->channelSlider->SetValue( svkPlotGridView::SafeDownCast( this->plotController->GetView() )->GetChannel() + 1 );
           int timePoints = svkMrsImageData::SafeDownCast( newSpec )->GetDcmHeader()->GetNumberOfTimePoints();
           this->spectraViewWidget->timePointSlider->SetRange( 1, timePoints);
           this->spectraViewWidget->timePointSlider->SetValue( svkPlotGridView::SafeDownCast( this->plotController->GetView() )->GetTimePoint() + 1 );
           this->EnableWidgets();
           if( this->model->DataExists("AnatomicalData") ) {
               this->overlayController->SetInput( this->model->GetDataObject("SpectroscopicData"), svkOverlayView::MRS );
               this->overlayController->SetTlcBrc( this->plotController->GetTlcBrc() );
           }
           if( toggleDraw ) {
               this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
               this->plotController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
            }
           this->plotController->GetView()->Refresh();
           this->overlayController->GetView()->Refresh();
       }
    }
}


//! Set the slice of all Controllers
void vtkSivicController::SetImageSlice( int slice, string orientation )
{
    int toggleDraw = this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOff();
        this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->DrawOff();
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
        this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
        this->plotController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
    }
    this->overlayController->GetView()->Refresh();
    this->plotController->GetView()->Refresh();
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
void vtkSivicController::SetQuantificationWidget( sivicQuantificationWidget* quantificationWidget)
{
    this->quantificationWidget = quantificationWidget;
    this->quantificationWidget->SetModel(this->model);
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
void vtkSivicController::SetOverlayWindowLevelWidget( sivicWindowLevelWidget* overlayWindowLevelWidget )
{
    this->overlayWindowLevelWidget = overlayWindowLevelWidget;
    this->overlayWindowLevelWidget->SetModel(this->model);
}


/*!    Open a file.    */
void vtkSivicController::ResetApplication( )
{
    model->RemoveDataObject("AnatomicalData"); 
    model->RemoveDataObject("SpectroscopicData"); 
    model->RemoveDataObject("OverlayData"); 
    model->RemoveDataObject("MetaboliteData"); 
    this->overlayController->Reset();
    this->plotController->Reset();
    this->viewRenderingWidget->ResetInfoText();
    this->dataWidget->UpdateReferenceSpectraList();
    this->DisableWidgets();

}


void vtkSivicController::OpenImage( const char* fileName )
{


    // Lets check to see if the file exists 
	if(!svkUtils::FilePathExists(fileName)) {
        this->PopupMessage(" File does not exist!"); 
        return;
    }

    string stringFilename(fileName);
    svkImageData* oldData = this->model->GetDataObject( "AnatomicalData" );
		    cout << "Attempting to read  |" << stringFilename << "|" << endl;
    svkImageData* newData = this->model->LoadFile( stringFilename );

    if (newData == NULL) {
        this->PopupMessage( "UNSUPPORTED FILE TYPE!");
    } else if( !newData->IsA("svkMriImageData") ) {
        this->PopupMessage("ERROR: Incorrect data type, data must be an image."); 
        return;
    } else {
        string resultInfo; 
        if( this->model->GetDataObject( "SpectroscopicData" ) != NULL ) {
            svkDataValidator* validator = svkDataValidator::New(); 
            bool valid = validator->AreDataCompatible( newData, this->model->GetDataObject( "SpectroscopicData" )); 
            if ( !valid ) {
                resultInfo = validator->resultInfo; 
            }
        }


        //  Precheck to see if valdation errors should be overridden:
        if( resultInfo.compare("") != 0 ) {

            string resultInfoMsg  = "ERROR: Dataset is not compatible! \n\n"; 
            resultInfoMsg += "Do you want to attempt to display them anyway? \n\n"; 
            resultInfoMsg += "Info:\n"; 
            resultInfoMsg += resultInfo;
            int dialogStatus = this->PopupMessage( resultInfoMsg, vtkKWMessageDialog::StyleYesNo ); 

            //  If user wants to continue anyway, unset the info results 
            if ( dialogStatus == 2 ) {
                resultInfo = "";      
                this->overlayController->GetView()->ValidationOff();
            }

        } 

        if( strcmp( resultInfo.c_str(), "" ) == 0 && newData != NULL ) {
            int toggleDraw = this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
            if( toggleDraw ) {
                this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOff();
                this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->DrawOff();
            }
            if( oldData != NULL) {
                this->model->ChangeDataObject( "AnatomicalData", newData );
                this->model->SetDataFileName( "AnatomicalData", stringFilename );
                this->overlayController->SetInput( newData ); 
            } else {
                this->model->AddDataObject( "AnatomicalData", newData ); 
                this->model->SetDataFileName( "AnatomicalData", stringFilename );
                this->overlayController->SetInput( newData ); 
                this->overlayController->ResetWindowLevel();
                this->overlayController->HighlightSelectionVoxels();
            }
            this->SetPreferencesFromRegistry();
            int* extent = newData->GetExtent();
            int firstSlice;
            int lastSlice;
            firstSlice = newData->GetFirstSlice( svkDcmHeader::AXIAL );
            lastSlice = newData->GetLastSlice( svkDcmHeader::AXIAL );
            this->imageViewWidget->axialSlider->SetRange( firstSlice + 1, lastSlice + 1); 
            if( oldData == NULL ) {
                this->imageViewWidget->axialSlider->SetValue( ( lastSlice - firstSlice ) / 2);
            } else if( !model->DataExists( "SpectroscopicData") || this->orientation != "AXIAL" ) {
                this->imageViewWidget->axialSlider->GetWidget()->InvokeEvent(vtkKWEntry::EntryValueChangedEvent); 
            }
            firstSlice = newData->GetFirstSlice( svkDcmHeader::CORONAL );
            lastSlice = newData->GetLastSlice( svkDcmHeader::CORONAL );
            this->imageViewWidget->coronalSlider->SetRange( firstSlice + 1, lastSlice + 1); 
            if( oldData == NULL ) {
                this->imageViewWidget->coronalSlider->SetValue( ( lastSlice - firstSlice ) / 2);
            } else if( !model->DataExists( "SpectroscopicData") || this->orientation != "CORONAL" ){
                this->imageViewWidget->coronalSlider->GetWidget()->InvokeEvent(vtkKWEntry::EntryValueChangedEvent); 
            }
            firstSlice = newData->GetFirstSlice( svkDcmHeader::SAGITTAL );
            lastSlice = newData->GetLastSlice( svkDcmHeader::SAGITTAL );
            this->imageViewWidget->sagittalSlider->SetRange( firstSlice + 1, lastSlice + 1); 
            if( oldData == NULL ) {
                this->imageViewWidget->sagittalSlider->SetValue( ( lastSlice - firstSlice ) / 2);
            } else if( !model->DataExists( "SpectroscopicData") || this->orientation != "SAGITTAL" ){
                this->imageViewWidget->sagittalSlider->GetWidget()->InvokeEvent(vtkKWEntry::EntryValueChangedEvent); 
            }
            this->imageViewWidget->volumeSlider->SetRange( 1, newData->GetDcmHeader()->GetNumberOfTimePoints());
            this->imageViewWidget->volumeSlider->SetValue( 1 );

            if( model->DataExists("SpectroscopicData") ) {
                this->overlayController->SetTlcBrc( plotController->GetTlcBrc() );
                this->spectraViewWidget->sliceSlider->GetWidget()->InvokeEvent( vtkKWEntry::EntryValueChangedEvent); 
            }
            if( oldData == NULL ) {
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
            double* pixelRange = this->model->GetDataObject("AnatomicalData")->GetPointData()->GetArray(0)->GetRange();
            double window = this->overlayController->GetWindow();
            double level = this->overlayController->GetLevel();
            this->windowLevelWidget->SetLevelRange( pixelRange ); 
            this->windowLevelWidget->SetLevel( level ); 
            this->windowLevelWidget->SetWindowRange( 0, pixelRange[1] - pixelRange[0] ); 
            this->windowLevelWidget->SetWindow( window ); 
            this->viewRenderingWidget->ResetInfoText();
            if( toggleDraw ) {
                this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
                this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->DrawOn();
            }
            this->overlayController->GetView()->Refresh( );
            this->plotController->GetView()->Refresh();

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


void vtkSivicController::OpenSpectra( svkImageData* newData,  string stringFilename, svkImageData* oldData, bool onlyReadOneInputFile )
{

    int toggleDraw = this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOff();
        this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->DrawOff();
    }
    string resultInfo;
    string plotViewResultInfo = this->plotController->GetDataCompatibility( newData, svkPlotGridView::MRS );
    string overlayViewResultInfo = this->overlayController->GetDataCompatibility( newData, svkOverlayView::MRS );
    svkDataValidator* validator = svkDataValidator::New(); 
    string validatorResultInfo;
    if( this->model->DataExists( "AnatomicalData" ) ) {
        bool valid = validator->AreDataCompatible( newData, this->model->GetDataObject( "AnatomicalData" )); 
        if ( !valid ) {
            validatorResultInfo = validator->resultInfo; 
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
        } 

    }  

    if ( overlayViewResultInfo.compare("") == 0 && 
        plotViewResultInfo.compare("") == 0 && 
        validatorResultInfo.compare("") == 0 && 
        newData != NULL )  {

        // If the spectra file is already in the model
        int* tlcBrc = NULL; 
        if( oldData != NULL ) {
            // We need to copy the tlc brc so it carries over to the new data set.
            tlcBrc = new int[2];
            memcpy( tlcBrc, this->plotController->GetTlcBrc(), 2*sizeof(int) );
            this->model->ChangeDataObject( "SpectroscopicData", newData );
            this->model->SetDataFileName( "SpectroscopicData", stringFilename );

        } else {
            this->model->AddDataObject( "SpectroscopicData", newData );
            this->model->SetDataFileName( "SpectroscopicData", stringFilename );
        }

        // Now we can update the sliders based on the image data properties
        spectraData = static_cast<vtkImageData*>(newData );
        int* extent = newData->GetExtent();

        this->plotController->SetInput( newData ); 
        this->dataWidget->UpdateReferenceSpectraList();
        this->dataWidget->SetFilename( 0, stringFilename);

        if( tlcBrc == NULL ) {
            int firstSlice = newData->GetFirstSlice( newData->GetDcmHeader()->GetOrientationType() );
            int lastSlice = newData->GetLastSlice( newData->GetDcmHeader()->GetOrientationType() );
            this->spectraViewWidget->sliceSlider->SetRange( firstSlice + 1, lastSlice + 1); 
            this->spectraViewWidget->sliceSlider->SetValue( ( lastSlice - firstSlice ) / 2 + 1);
            int channels = svkMrsImageData::SafeDownCast( newData )->GetDcmHeader()->GetNumberOfCoils();
            this->spectraViewWidget->channelSlider->SetRange( 1, channels); 
            this->spectraViewWidget->channelSlider->SetValue( 1 );
            int timePoints = newData->GetDcmHeader()->GetNumberOfTimePoints();
            this->spectraViewWidget->timePointSlider->SetRange( 1, timePoints); 
            this->spectraViewWidget->timePointSlider->SetValue( 1 );
            this->plotController->SetSlice( ( lastSlice - firstSlice ) / 2 ); 
            this->overlayController->SetSlice( ( lastSlice - firstSlice ) / 2 ); 
        }

        this->detailedPlotController->SetInput( newData ); 
        this->overlayController->SetInput( newData, svkOverlayView::MRS ); 
       
        // THe overlay controller may reslice the data, we need te make sure we get the new version of the data 
        // TODO: Change the overlay controller to do this in place 
        svkImageData* mri = this->overlayController->GetView()->GetInput(svkOverlayView::MRI);
        if( mri != NULL && mri != this->model->GetDataObject( "AnatomicalData" ) ) {
            this->model->ChangeDataObject( "AnatomicalData", mri );
        }
        
        this->SetPreferencesFromRegistry();

        this->processingWidget->InitializePhaser();

        this->spectraRangeWidget->point->SetDcmHeader( newData->GetDcmHeader() );

        if( oldData != NULL ) {
            bool useFullFrequencyRange = 0;
            bool useFullAmplitudeRange = 0;
            bool resetAmplitude = 0;
            bool resetFrequency = 0;
            this->ResetRange( useFullFrequencyRange, useFullAmplitudeRange,
                                       resetAmplitude, resetFrequency );
            this->SetOrientation( this->orientation.c_str() );
        } else {
            bool useFullFrequencyRange = 0;
            bool useFullAmplitudeRange = 0;
            bool resetAmplitude = 1;
            bool resetFrequency = 1;
            this->ResetRange( useFullFrequencyRange, useFullAmplitudeRange,
                                       resetAmplitude, resetFrequency );
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

    } else {

        resultInfo = "ERROR: Could not load dataset!\nInfo:\n"; 
        this->PopupMessage( resultInfo ); 
    }

    if( toggleDraw ) {
        this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
        this->plotController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
    }
    this->DisableWidgets();
    this->EnableWidgets();
    this->overlayController->GetView()->Refresh( );
    this->plotController->GetView()->Refresh( );
}



void vtkSivicController::OpenSpectra( const char* fileName, bool onlyReadOneInputFile )
{

    // Lets check to see if the file exists 
	if(!svkUtils::FilePathExists(fileName)) {
        this->PopupMessage(" File does not exist!"); 
        return;
    }

    string stringFilename(fileName);
    svkImageData* oldData = model->GetDataObject("SpectroscopicData");
    svkImageData* newData = model->LoadFile( stringFilename, onlyReadOneInputFile );


    if (newData == NULL) {

        this->PopupMessage( "UNSUPPORTED FILE TYPE!");
        return;
    } else if( newData->IsA("svkMrsImageData")){
        this->OpenSpectra( newData,  stringFilename, oldData, onlyReadOneInputFile );

    } else {
        this->PopupMessage("ERROR: Incorrect data type, data must be spectra."); 
        return;
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
                }
            }
            ++it;
        }
    }

#endif
}


void vtkSivicController::OpenOverlay( svkImageData* data, string stringFilename )
{
        string resultInfo = "";

        if (data == NULL) {
            this->PopupMessage( "UNSUPPORTED FILE TYPE!");
        } else {

            resultInfo = this->overlayController->GetDataCompatibility( data, svkOverlayView::OVERLAY );
            if( strcmp( resultInfo.c_str(), "" ) == 0 ) {
                int toggleDraw = this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
                if( toggleDraw ) {
                    this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOff();
                    this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->DrawOff();
                }
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
                } else {
                    if( this->model->DataExists( "OverlayData" ) ) {
                        this->model->ChangeDataObject( "OverlayData", data );
                        this->model->SetDataFileName( "OverlayData", stringFilename );
                    } else {
                        this->model->AddDataObject( "OverlayData", data );
                        this->model->SetDataFileName( "OverlayData", stringFilename );
                    }
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
                if( lut == "color" ) {
                    this->SetLUTCallback( svkLookupTable::COLOR );
                } else if ( lut == "grey " ) {
                    this->SetLUTCallback( svkLookupTable::GREY_SCALE );
                } else if ( lut == "hurd " ) {
                    this->SetLUTCallback( svkLookupTable::HURD );
                } else if ( lut == "cyan " ) {
                    this->SetLUTCallback( svkLookupTable::CYAN_HOT );
                }
                this->imageViewWidget->colorBarButton->InvokeEvent( vtkKWCheckButton::SelectedStateChangedEvent );
                this->imageViewWidget->overlayButton->InvokeEvent( vtkKWCheckButton::SelectedStateChangedEvent );
                this->imageViewWidget->overlayOpacitySlider->GetWidget()->InvokeEvent( vtkKWEntry::EntryValueChangedEvent );
                double* pixelRange = data->GetPointData()->GetArray(0)->GetRange();
                double window = this->overlayController->GetWindow(svkOverlayViewController::IMAGE_OVERLAY);
                double level = this->overlayController->GetLevel(svkOverlayViewController::IMAGE_OVERLAY);
                this->overlayWindowLevelWidget->SetLevelRange( pixelRange ); 
                this->overlayWindowLevelWidget->SetLevel( level ); 
                this->overlayWindowLevelWidget->SetWindowRange( 0, pixelRange[1] - pixelRange[0] ); 
                this->overlayWindowLevelWidget->SetWindow( window ); 
                this->overlayWindowLevelWidget->SetOverlayDataName( overlayDataName ); 
                this->imageViewWidget->overlayVolumeSlider->SetRange( 1, data->GetDcmHeader()->GetNumberOfTimePoints());
                this->imageViewWidget->overlayVolumeSlider->SetValue( 1 );

                if( toggleDraw ) {
                    this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
                    this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->DrawOn();
                }
            } else {
                string message = "ERROR: Dataset is not compatible and will not be loaded.\nInfo:\n";
                message += resultInfo;
                this->PopupMessage( message );
            }
        }
        this->SetThresholdType( this->thresholdType );
        this->spectraViewWidget->sliceSlider->GetWidget()->InvokeEvent(vtkKWEntry::EntryValueChangedEvent); 
}

void vtkSivicController::AddSpectra( string stringFilename )
{
    if ( this->model->DataExists("SpectroscopicData") ) {
        svkImageData* data = this->model->LoadFile( stringFilename );
        if( data != NULL && data->IsA("svkMrsImageData") ) {
            string resultInfo = this->plotController->GetDataCompatibility( data,  svkPlotGridView::ADDITIONAL_MRS );
            if( resultInfo.compare("") != 0 ) {
                string message =  "ERROR: Could not load reference spectra.\n Info: ";
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
    } else {
        this->PopupMessage( "ERROR: Currently loading of the reference spectra before the primary spectra is not supported." );
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


void vtkSivicController::OpenOverlay( const char* fileName )
{

    // Lets check to see if the file exists 

	if(!svkUtils::FilePathExists(fileName)) {
        this->PopupMessage(" File does not exist!"); 
        return;
    }



    string stringFilename( fileName );
    if ( this->model->DataExists("SpectroscopicData") && this->model->DataExists("AnatomicalData") ) {

        svkImageData* data = this->model->LoadFile( stringFilename );
        if( data != NULL && data->IsA("svkMriImageData") ) {
            this->OpenOverlay(data, stringFilename);
        } else {
            this->PopupMessage("ERROR: Incorrect data type, data must be an image to be overlayed."); 
            return;
        }
    } else {
        this->PopupMessage( "ERROR: Currently loading of overlays before image AND spectra is not supported." );
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
            if( this->model->DataExists("SpectroscopicData") && this->model->DataExists("AnatomicalData") ) {

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

    // First we open an image

    // Lets check for an images folder
	if(svkUtils::FilePathExists("images")) {
        imagePathName+= "/images";
        //cout << "Switching to image path:" << imagePathName.c_str() << endl;
        status = this->OpenFile( "image", imagePathName.c_str(), true ); 
    } else {
        status = this->OpenFile( "image", NULL, true ); 
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
            status = this->OpenFile( "spectra", spectraPathName.c_str() ); 
        } else {
            // If an images folder was used, but there is no corresponding spectra folder
            status = this->OpenFile( "spectra", lastPathString.substr(0,found).c_str() ); 
        }
    } else { 
        status = this->OpenFile( "spectra", lastPathString.c_str() ); 
    }
     
    // If the dialog was cancelled, or if either load failed do not oven overlay
    if( status == vtkKWDialog::StatusCanceled || !this->model->DataExists("SpectroscopicData") 
                                              || !this->model->DataExists("AnatomicalData") ) {
        return;
    } 

    if( lastPathString.substr(found+1) == "images" ) {
        string spectraPathName;
        spectraPathName = lastPathString.substr(0,found); 
        spectraPathName += "/spectra";
        spectraPathName += "/" + svkUCSFUtils::GetMetaboliteDirectoryName(this->model->GetDataFileName("SpectroscopicData"));
        bool includePath = true;
        string cniFileName = svkUCSFUtils::GetMetaboliteFileName( this->model->GetDataFileName("SpectroscopicData"), "CNI (ht)",includePath );
		if( svkUtils::FilePathExists(cniFileName.c_str()) ) {
            this->OpenOverlay( cniFileName.c_str() ); 
            this->EnableWidgets(); 
            this->imageViewWidget->thresholdType->GetWidget()->SetValue( "Quantity" );
            this->imageViewWidget->overlayThresholdSlider->SetValue( 2.0 );
            this->SetOverlayThreshold( 2.0 );
		} else if( svkUtils::FilePathExists(spectraPathName.c_str()) ) {
            this->OpenFile( "overlay", spectraPathName.c_str() ); 
        } else {
            // If an images folder was used, but there is no corresponding spectra folder
            this->OpenFile( "overlay", lastPathString.substr(0,found).c_str() ); 
        }
    } else { 
        this->OpenFile( "overlay", lastPathString.c_str() ); 
    }


}


/*!    Open a file.    */
int vtkSivicController::OpenFile( char* openType, const char* startPath, bool resetBeforeLoad )
{
    this->viewRenderingWidget->viewerWidget->GetRenderWindowInteractor()->Disable();
    this->viewRenderingWidget->specViewerWidget->GetRenderWindowInteractor()->Disable();

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

            if( strcmp( openType, "image" ) == 0 || strcmp( openType, "overlay" ) == 0){
                lastPathString = lastPathString.substr(0,found); 
                lastPathString += "/images";
				if( svkUtils::FilePathExists(lastPathString.c_str())) {
                    dlg->SetLastPath( lastPathString.c_str());
                }
            } else if ( strcmp( openType, "spectra" ) == 0 || strcmp( openType, "spectra_one_channel") == 0 || strcmp( openType, "add_spectra") == 0) {
                lastPathString = lastPathString.substr(0,found); 
                lastPathString += "/spectra";
				if( svkUtils::FilePathExists( lastPathString.c_str()) ) {
                    dlg->SetLastPath( lastPathString.c_str());
                }

            }

        }
    
        // Check to see which extention to filter for.
        if( strcmp( openType,"image" ) == 0 || strcmp( openType, "overlay" ) == 0 ) {
            dlg->SetFileTypes("{{Image Files} {.idf .fdf .dcm .DCM .IMA}} {{All files} {.*}}");
        } else if( strcmp( openType,"spectra" ) == 0 || strcmp(openType, "spectra_one_channel") == 0 || strcmp( openType, "add_spectra") == 0) {
            dlg->SetFileTypes("{{MRS Files} {.ddf .shf .rda .dcm .DCM fid}} {{All files} {.*}}");
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
            this->OpenImage( dlg->GetFileName() );
        } else if( openTypeString.compare( "overlay" ) == 0 ) {
            this->OpenOverlay( dlg->GetFileName() );
        } else if( openTypeString.compare( "spectra" ) == 0 ) {
            this->OpenSpectra( dlg->GetFileName() );
        } else if( openTypeString.compare( "spectra_one_channel" ) == 0 ) {
            bool onlyReadOneInputFile = true;
            this->OpenSpectra( dlg->GetFileName(), onlyReadOneInputFile );
        } else if( openTypeString.compare( "add_spectra" ) == 0 ) {
            this->AddSpectra( dlg->GetFileName() );
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

    }
    dlg->Delete();

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

    writer->SetInput( this->model->GetDataObject("SpectroscopicData") );

    writer->Write();
    writer->Delete();
}


//! Saves a secondary capture.
void vtkSivicController::SaveSecondaryCapture( char* captureType )
{
    if( this->model->GetDataObject( "SpectroscopicData" ) == NULL ) {
        PopupMessage( "SPECTRA MUST BE LOADED TO CREATE SECONDARY CAPTURES!" );
        return; 
    }
    vtkKWFileBrowserDialog *dlg = vtkKWFileBrowserDialog::New();
    dlg->SetApplication(app);
    dlg->SaveDialogOn();
    dlg->Create();


    string defaultNamePattern;
    int seriesNumber =  svkImageWriterFactory::GetNewSeriesFilePattern( 
        model->GetDataObject( "SpectroscopicData" ) ,
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
    int lastFrame = this->model->GetDataObject("SpectroscopicData")->GetDcmHeader()->GetNumberOfSlices();
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
        outputImage->SetDcmHeader( this->model->GetDataObject( "SpectroscopicData" )->GetDcmHeader() );
        this->model->GetDataObject( "SpectroscopicData" )->GetDcmHeader()->Register(this);
    }
    //Give it a default dcos so it can be viewed in an image viewer
    double dcos[3][3] = {{1,0,0}, {0,1,0}, {0,0,1}}; 
    if( this->model->GetDataObject( "AnatomicalData" ) != NULL ) {
        this->model->GetDataObject( "AnatomicalData" )->GetDcos(dcos);
    } else { //If only spectra are loaded
        this->model->GetDataObject( "SpectroscopicData" )->GetDcos(dcos);
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
        vtkLabeledDataMapper::SafeDownCast(
            vtkActor2D::SafeDownCast(this->plotController->GetView()->GetProp( svkPlotGridView::OVERLAY_TEXT ))->GetMapper())->GetLabelTextProperty()->SetFontSize(12);
    }
    this->viewRenderingWidget->imageInfoActor->GetTextProperty()->SetFontSize(13);
    this->viewRenderingWidget->specInfoActor->GetTextProperty()->SetFontSize(13);
*/

    if( print ){
        ToggleColorsForPrinting( svkSecondaryCaptureFormatter::LIGHT_ON_DARK );
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

    int firstFrame = 0;
    int lastFrame = this->model->GetDataObject("SpectroscopicData")->GetDcmHeader()->GetNumberOfSlices();
    if( outputOption == svkSecondaryCaptureFormatter::CURRENT_SLICE ) { 
        firstFrame = plotController->GetSlice();
        lastFrame = firstFrame + 1;
    }
    int instanceNumber = 1;
    for (int m = firstFrame; m <= lastFrame; m++) {
        if( !static_cast<svkMrsImageData*>(this->model->GetDataObject( "SpectroscopicData" ))->IsSliceInSelectionBox(m) ) {
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


//! Returns the plot controller
svkDetailedPlotViewController* vtkSivicController::GetDetailedPlotController()
{
    return this->detailedPlotController;
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
        int toggleDraw = this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
        if( toggleDraw ) {
            this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOff();
        }
        this->windowLevelWidget->SetLevelRange( pixelRange ); 
        this->windowLevelWidget->SetLevel( level ); 
        this->windowLevelWidget->SetWindowRange( 0, pixelRange[1] - pixelRange[0] ); 
        this->windowLevelWidget->SetWindow( window ); 
        this->viewRenderingWidget->ResetInfoText();
        if( toggleDraw ) {
            this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
        }
        this->viewRenderingWidget->specViewerWidget->Render();
        this->viewRenderingWidget->viewerWidget->Render();
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
        int width;
        int height;
        this->mainWindow->GetSize(&width, &height);
        this->preferencesWindow->SetSize( width, 175);
        this->preferencesWidget->SetParent( this->preferencesWindow->GetViewFrame() );
        this->preferencesWidget->Create();
        this->app->Script("grid %s -in %s -row 0 -column 0 -sticky wnse -pady 2 "
                , this->preferencesWidget->GetWidgetName(), this->preferencesWindow->GetViewFrame()->GetWidgetName());
        this->app->Script("grid rowconfigure %s 0  -weight 1"
                , this->preferencesWindow->GetViewFrame()->GetWidgetName() );
        this->app->Script("grid columnconfigure %s 0  -weight 1"
                , this->preferencesWindow->GetViewFrame()->GetWidgetName() );
    }

    int toggleDraw = this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOff();
        this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->DrawOff();
    }
    this->preferencesWindow->Display();
    if( toggleDraw ) {
        this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
        this->plotController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
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

    int toggleDraw = this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOff();
        this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->DrawOff();
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
        this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
        this->plotController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
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
}


/*!
 *
 */
void vtkSivicController::SetComponentCallback( int targetComponent)
{
    string acquisitionType;
    if ( targetComponent == svkPlotLine::REAL) {
        this->plotController->SetComponent(svkPlotLine::REAL);
    } else if ( targetComponent == svkPlotLine::IMAGINARY) {
        this->plotController->SetComponent(svkPlotLine::IMAGINARY);
    } else if ( targetComponent == svkPlotLine::MAGNITUDE) {
        this->plotController->SetComponent(svkPlotLine::MAGNITUDE);
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
    } else if ( type == svkLookupTable::GREY_SCALE) {
        static_cast<svkOverlayViewController*>( this->overlayController)->SetLUT( svkLookupTable::GREY_SCALE );
    } else if ( type == svkLookupTable::HURD) {
        static_cast<svkOverlayViewController*>( this->overlayController)->SetLUT( svkLookupTable::HURD );
    } else if ( type == svkLookupTable::CYAN_HOT ) {
        static_cast<svkOverlayViewController*>( this->overlayController)->SetLUT( svkLookupTable::CYAN_HOT );
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
 *
 */
void vtkSivicController::Print(char* captureType, int outputOption )
{
    if( this->model->GetDataObject( "SpectroscopicData" ) == NULL ) {
        this->PopupMessage( "NO SPECTRA LOADED!");
        return; 
    }
    string defaultNamePattern;
    int seriesNumber =  svkImageWriterFactory::GetNewSeriesFilePattern( 
            model->GetDataObject( "SpectroscopicData" ) ,
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
    string printerName = "jasmine";

    return printerName;

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
    int toggleDraw = this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOff();
        this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->DrawOff();
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

    if( this->model->DataExists("SpectroscopicData") ) {
        firstSlice = this->model->GetDataObject("SpectroscopicData")->GetFirstSlice( newOrientation );
        lastSlice =  this->model->GetDataObject("SpectroscopicData")->GetLastSlice( newOrientation );
        this->spectraViewWidget->sliceSlider->SetRange( firstSlice + 1, lastSlice + 1 );
        this->spectraViewWidget->sliceSlider->SetValue( this->plotController->GetSlice()+1 );
        this->SetSlice( this->plotController->GetSlice());
        string acquisitionType = model->GetDataObject( "SpectroscopicData" )->
                                GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
        if( acquisitionType == "SINGLE VOXEL" ) {
            // For single voxel always highlight the selection box voxels
            this->overlayController->HighlightSelectionVoxels();
            this->plotController->HighlightSelectionVoxels();
        }
        this->overlayController->SetTlcBrc( this->plotController->GetTlcBrc() );
    }

    if( toggleDraw ) {
        this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
        this->plotController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
    }
    this->EnableWidgets();
    this->plotController->GetView()->Refresh();
    this->overlayController->GetView()->Refresh();
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
        this->OpenSpectra( fileName );
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
    svkImageData* data = this->model->GetDataObject( "SpectroscopicData" ); 
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
    svkImageData* data = this->model->GetDataObject( "SpectroscopicData" ); 
    if( data != NULL ) {
        int channels = svkMrsImageData::SafeDownCast( data )->GetDcmHeader()->GetNumberOfCoils();
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
    if ( model->DataExists("SpectroscopicData") && model->DataExists("AnatomicalData") ) {

#if defined( UCSF_INTERNAL )
        //  Enable the UCSF metabolite widgets
        this->GetApplication()->GetNthWindow(0)->GetMenu()->GetNthChild(3)->GetNthChild(0)->EnabledOn(); 
#endif
        string acquisitionType = model->GetDataObject( "SpectroscopicData" )->
                               GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
        if( acquisitionType == "SINGLE VOXEL" ) {
            this->imageViewWidget->plotGridButton->EnabledOff();
        } else {
            this->imageViewWidget->plotGridButton->EnabledOn();
            this->imageViewWidget->plotGridButton->InvokeEvent(vtkKWCheckButton::SelectedStateChangedEvent);
        }
    }

    if ( model->DataExists("SpectroscopicData") ) {
        this->spectraViewWidget->sliceSlider->EnabledOn();
        string domain = model->GetDataObject( "SpectroscopicData" )->GetDcmHeader()->GetStringValue("SignalDomainColumns");
        int numChannels = svkMrsImageData::SafeDownCast( model->GetDataObject("SpectroscopicData"))->GetDcmHeader()->GetNumberOfCoils();
        if( domain == "TIME" ) {
            this->preprocessingWidget->applyButton->EnabledOn(); 
            this->preprocessingWidget->zeroFillSelectorSpec->EnabledOn();
            this->preprocessingWidget->zeroFillSelectorCols->EnabledOn();
            this->preprocessingWidget->zeroFillSelectorRows->EnabledOn();
            this->preprocessingWidget->zeroFillSelectorSlices->EnabledOn();
            this->preprocessingWidget->apodizationSelectorSpec->EnabledOn();
            this->processingWidget->fftButton->EnabledOn(); 
            this->processingWidget->phaseButton->EnabledOff(); 
            this->processingWidget->combineButton->EnabledOff(); 
            this->spectraRangeWidget->xSpecRange->SetLabelText( "Time" );
            this->spectraRangeWidget->unitSelectBox->SetValue( "PTS" );
            this->spectraRangeWidget->SetSpecUnitsCallback(svkSpecPoint::PTS);
            this->spectraRangeWidget->unitSelectBox->EnabledOff();
        } else {
            this->processingWidget->fftButton->EnabledOff(); 
            this->processingWidget->phaseButton->EnabledOn(); 
            if( numChannels > 1 ) {
                this->processingWidget->combineButton->EnabledOn();
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
        this->spectraRangeWidget->xSpecRange->EnabledOn();
        this->spectraRangeWidget->ySpecRange->EnabledOn();
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
        this->imageViewWidget->volumeSlider->EnabledOn();
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

    //this->spectraViewWidget->detailedPlotButton->EnabledOff();
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
    this->processingWidget->combineButton->EnabledOff(); 

    this->preprocessingWidget->applyButton->EnabledOff(); 
    this->preprocessingWidget->zeroFillSelectorSpec->EnabledOff();
    this->preprocessingWidget->zeroFillSelectorCols->EnabledOff();
    this->preprocessingWidget->zeroFillSelectorRows->EnabledOff();
    this->preprocessingWidget->zeroFillSelectorSlices->EnabledOff();
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

    if(     this->model->GetDataObject( "SpectroscopicData" ) == NULL 
         || this->model->GetDataObject( "AnatomicalData" ) == NULL ) {

        PopupMessage( "BOTH SPECTRA AND AN IMAGE MUST BE LOADED TO CREATE SECONDARY CAPTURES!" );
        return; 
    }

    svkPACSInterface* pacsInterface = svkUCSFPACSInterface::New();

    // For testing below
    //pacsInterface->SetPACSTargetString("/data/lhst3/sivic/DICOM_REID/results");

    /*********************************** CHECK FOR FILE WRITING PERMISSIONS ****************************************/

    // Lets locate a local directory to make a copy of the images being push to pacs. We'll use the spectra directory
    string spectraFileName = this->model->GetDataFileName( "SpectroscopicData" );

    // Parse for directory name
    size_t found;
    found = spectraFileName.find_last_of("/");

    string localDirectory = spectraFileName.substr(0,found); 
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
                                                                           , svkMrsImageData::SafeDownCast( this->model->GetDataObject("SpectroscopicData")) );

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
                                                                        , svkMrsImageData::SafeDownCast( this->model->GetDataObject("SpectroscopicData")) );
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
        this->model->GetDataObject( "SpectroscopicData" ) ,
        &notUsed
    );
    static_cast<svkImageWriter*>(writer)->SetSeriesNumber( seriesNumber );

    // And set the series description
    static_cast<svkImageWriter*>(writer)->SetSeriesDescription( "SIVIC MRSI secondary capture" );

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
        static_cast<vtkSivicController*>(thisObject)->GetApplication()->GetNthWindow(0)->SetStatusText(
                  static_cast<svkDataModel*>(subject)->GetProgressText().c_str() );
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

