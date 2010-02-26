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
 */
#define DEBUG 0


#include <vtkSivicController.h>
#include <vtksys/SystemTools.hxx>


using namespace svk; 


vtkStandardNewMacro( vtkSivicController );
vtkCxxRevisionMacro( vtkSivicController, "$Revision$");

static int nearestInt(float x); 

//! Constructor
vtkSivicController::vtkSivicController()
{
    this->spectraData = NULL;
    this->plotController    = svkPlotGridViewController::New();
    this->overlayController = svkOverlayViewController::New();
    this->imageViewWidget = NULL;
    this->spectraViewWidget = NULL;
    this->viewRenderingWidget = NULL;
    this->processingWidget = NULL;
}


//! Destructor
vtkSivicController::~vtkSivicController()
{
    char cwd[MAXPATHLEN];
    getcwd(cwd, MAXPATHLEN);
    string pathName(cwd);
    this->app->SetRegistryValue( 0, "RunTime", "lastPath", pathName.c_str());

    if( this->overlayController != NULL ) {
        this->overlayController->Delete();
        this->overlayController = NULL;
    }

    if( this->plotController != NULL ) {
        this->plotController->Delete();
        this->plotController = NULL;
    }

}


//! Set the slice of all Controllers
void vtkSivicController::SetSlice( int slice )
{
    int toggleDraw = this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOff();
        this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->DrawOff();
    }
    this->plotController->SetSlice(slice);
    this->overlayController->SetSlice(slice);
    //this->plotController->GetView()->Refresh();
    //this->overlayController->GetView()->Refresh();
    this->viewRenderingWidget->ResetInfoText();
    if( this->model->DataExists("AnatomicalData")) {
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


//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetApplication( vtkKWApplication* app)
{
    this->app = app;
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
void vtkSivicController::SetImageViewWidget( sivicImageViewWidget* imageViewWidget )
{
    this->imageViewWidget = imageViewWidget;
    this->imageViewWidget->SetModel(this->model);
}


//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetSpectraViewWidget( sivicSpectraViewWidget* spectraViewWidget )
{
    this->spectraViewWidget = spectraViewWidget;
    this->spectraViewWidget->SetModel(this->model);
}


//! Sets this widget controllers view, also passes along its model
void vtkSivicController::SetGlobalWidget( sivicGlobalWidget* globalWidget )
{
    this->globalWidget = globalWidget;
    this->globalWidget->SetModel(this->model);
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
    this->DisableWidgets();

}


void vtkSivicController::OpenImage( const char* fileName )
{
    struct stat st;
    // Lets check to see if the file exists 
    if(stat(fileName,&st) != 0) {
        this->PopupMessage(" File does not exist!"); 
        return;
    }
    string stringFilename(fileName);
    svkImageData* oldData = this->model->GetDataObject( "AnatomicalData" );
    svkImageData* newData = this->model->LoadFile( stringFilename );

    if (newData == NULL) {
        this->PopupMessage( "UNSUPPORTED FILE TYPE!");
    } else {
        string resultInfo; 
        if( this->model->GetDataObject( "SpectroscopicData" ) != NULL ) {
            svkDataValidator* validator = svkDataValidator::New(); 
            svkDataValidator::ValidationErrorStatus status = 
                validator->AreDataIncompatible( newData, this->model->GetDataObject( "SpectroscopicData" )); 
            if ( status >= 2 ) {
                resultInfo = validator->resultInfo; 
            }
        }

        if( strcmp( resultInfo.c_str(), "" ) == 0 && newData != NULL ) {
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
            int* extent = newData->GetExtent();
            if( !model->DataExists("SpectroscopicData") ) {
                int firstSlice;
                int lastSlice;
                firstSlice = newData->GetFirstSlice( svkDcmHeader::AXIAL );
                lastSlice = newData->GetLastSlice( svkDcmHeader::AXIAL );
                this->imageViewWidget->axialSlider->SetRange( firstSlice + 1, lastSlice + 1); 
                this->imageViewWidget->axialSlider->SetValue( ( lastSlice - firstSlice ) / 2);
                firstSlice = newData->GetFirstSlice( svkDcmHeader::CORONAL );
                lastSlice = newData->GetLastSlice( svkDcmHeader::CORONAL );
                this->imageViewWidget->coronalSlider->SetRange( firstSlice + 1, lastSlice + 1); 
                this->imageViewWidget->coronalSlider->SetValue( ( lastSlice - firstSlice ) / 2);
                firstSlice = newData->GetFirstSlice( svkDcmHeader::SAGITTAL );
                lastSlice = newData->GetLastSlice( svkDcmHeader::SAGITTAL );
                this->imageViewWidget->sagittalSlider->SetRange( firstSlice + 1, lastSlice + 1); 
                this->imageViewWidget->sagittalSlider->SetValue( ( lastSlice - firstSlice ) / 2);

            } else {
                this->overlayController->SetTlcBrc( plotController->GetTlcBrc() );
            }
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
            this->viewRenderingWidget->ResetInfoText();
        } else {
            string message = "ERROR: Dataset is not compatible and will not be loaded!\nInfo:\n"; 
            message += resultInfo;
            this->PopupMessage( resultInfo ); 
        }
        this->UseSelectionStyle();
    }
}


void vtkSivicController::OpenSpectra( const char* fileName )
{
    struct stat st;
    // Lets check to see if the file exists 
    if(stat(fileName,&st) != 0) {
        this->PopupMessage(" File does not exist!"); 
        return;
    }
    string stringFilename(fileName);
    svkImageData* oldData = model->GetDataObject("SpectroscopicData");
    svkImageData* newData = model->LoadFile( stringFilename );
    cout << *newData << endl;

    if (newData == NULL) {
        this->PopupMessage( "UNSUPPORTED FILE TYPE!");
    } else {
        int toggleDraw = this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
        if( toggleDraw ) {
            this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOff();
            this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->DrawOff();
        }

        string resultInfo;
        string plotViewResultInfo = this->plotController->GetDataCompatibility( newData, svkPlotGridView::MRS );
        string overlayViewResultInfo = this->overlayController->GetDataCompatibility( newData, svkPlotGridView::MRS );

        if( strcmp( overlayViewResultInfo.c_str(), "" ) == 0 && 
            strcmp( plotViewResultInfo.c_str(), "" ) == 0 && newData != NULL ) {
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

            // TODO: this will fail for sagittal/coronal data...
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
            this->plotController->SetInput( newData ); 

            this->spectraViewWidget->detailedPlotController->SetInput( newData ); 
            this->overlayController->SetInput( newData, svkOverlayView::MRS ); 
    
            this->processingWidget->phaseSlider->SetValue(0.0); 
            this->processingWidget->phaser->SetInput( newData );
    
            this->spectraViewWidget->point->SetDataHdr( newData->GetDcmHeader() );
   
            if( tlcBrc == NULL ) {
                this->plotController->HighlightSelectionVoxels();
                this->overlayController->HighlightSelectionVoxels();
            } else {
                this->plotController->SetTlcBrc( tlcBrc ); 
                this->overlayController->SetTlcBrc( tlcBrc ); 
            }
            this->ResetRange( );
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
            this->viewRenderingWidget->ResetInfoText();
        } else {
            resultInfo = "ERROR: Dataset is not compatible!\n"; 
            resultInfo += "Info:\n"; 
            resultInfo += plotViewResultInfo;
            resultInfo += overlayViewResultInfo;
            this->PopupMessage( resultInfo ); 
        }
        if( toggleDraw ) {
            this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
            this->plotController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOn();
        }
        this->overlayController->GetView()->Refresh( );
        this->plotController->GetView()->Refresh( );

    }
//this->plotController->GetRWInteractor()->SetInteractorStyle( vtkInteractorStyleTrackballCamera::New());
    // Lets update the metabolite menu for the current spectra
    this->globalWidget->PopulateMetaboliteMenu();
}


void vtkSivicController::OpenOverlay( const char* fileName )
{
    struct stat st;
    // Lets check to see if the file exists 
    if(stat(fileName,&st) != 0) {
        this->PopupMessage(" File does not exist!"); 
        return;
    }
    string resultInfo = "";
    string stringFilename( fileName );
    if ( this->model->DataExists("SpectroscopicData") && this->model->DataExists("AnatomicalData") ) {

        svkImageData* data = this->model->LoadFile( stringFilename );

        if (data == NULL) {
            this->PopupMessage( "UNSUPPORTED FILE TYPE!");
        } else {

            resultInfo = this->overlayController->GetDataCompatibility( data, svkOverlayView::OVERLAY );
            if( strcmp( resultInfo.c_str(), "" ) == 0 ) {
                this->overlayController->SetInput( data, svkOverlayView::OVERLAY );
                resultInfo = this->plotController->GetDataCompatibility( data, svkPlotGridView::MET ); 
                if( strcmp( resultInfo.c_str(), "" ) == 0 ) {
                    this->plotController->SetInput( data, svkPlotGridView::MET );
                    if( this->model->DataExists( "MetaboliteData" ) ) {
                        this->model->ChangeDataObject( "MetaboliteData", data );
                        this->model->SetDataFileName( "MetaboliteData", stringFilename );
                    } else {
                        this->model->AddDataObject( "MetaboliteData", data );
                        this->model->SetDataFileName( "MetaboliteData", stringFilename );
                    }
                    // We are going to deselect the metabolites since we don't know where they were loaded from
                    this->globalWidget->DeselectMetabolites();
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
                }
            } else {
                string message = "ERROR: Dataset is not compatible and will not be loaded.\nInfo:\n";
                message += resultInfo;
                this->PopupMessage( message );
            }
        }

    } else {
        this->PopupMessage( "ERROR: Currently loading of overlays before image AND spectra is not supported." );
    }
}


void vtkSivicController::OpenMetabolites( const char* metabolites )
{
    cout << "Attempting to open metabolites " << metabolites << endl;
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
                    this->globalWidget->metaboliteSelect->GetWidget()->SetValue( "None" );
                } 
                this->globalWidget->metaboliteSelect->GetWidget()
                          ->SetValue( svkUCSFUtils::GetMetaboliteName( this->model->GetDataFileName("MetaboliteData") ).c_str());
            } else {
               this->PopupMessage( "You must load an image and a spectra before loading metabolites." ); 
            } 
        }
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
    struct stat st;
    int status = -1;
    char cwd[MAXPATHLEN];
    char lastPath[MAXPATHLEN];
    getcwd(cwd, MAXPATHLEN);
    string imagePathName(cwd);

    // First we open an image

    // Lets check for an images folder
    if(stat("images",&st) == 0) {
        imagePathName+= "/images";
        //cout << "Switching to image path:" << imagePathName.c_str() << endl;
        status = this->OpenFile( "image", imagePathName.c_str(), true ); 
    } else {
        status = this->OpenFile( "image", NULL, true ); 
    }
    
    if( status == vtkKWDialog::StatusCanceled ) {
        return;
    } 

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
        if( stat(spectraPathName.c_str(),&st) == 0 ) {
            status = this->OpenFile( "spectra", spectraPathName.c_str() ); 
        } else {
            // If an images folder was used, but there is no corresponding spectra folder
            status = this->OpenFile( "spectra", lastPathString.substr(0,found).c_str() ); 
        }
    } else { 
        status = this->OpenFile( "spectra", lastPathString.c_str() ); 
    }

    if( status == vtkKWDialog::StatusCanceled ) {
        return;
    } 

    if( lastPathString.substr(found+1) == "images" ) {
        string spectraPathName;
        spectraPathName = lastPathString.substr(0,found); 
        spectraPathName += "/spectra";
        spectraPathName += "/" + svkUCSFUtils::GetMetaboliteDirectoryName(this->model->GetDataFileName("SpectroscopicData"));
        bool includePath = true;
        string cniFileName = svkUCSFUtils::GetMetaboliteFileName( this->model->GetDataFileName("SpectroscopicData"), "CNI (ht)",includePath );
        if( stat(cniFileName.c_str(),&st) == 0 ) {
            this->OpenOverlay( cniFileName.c_str() ); 
            this->EnableWidgets(); 
        } else if( stat(spectraPathName.c_str(),&st) == 0 ) {
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
    struct stat st;
    this->viewRenderingWidget->viewerWidget->GetRenderWindowInteractor()->Disable();
    this->viewRenderingWidget->specViewerWidget->GetRenderWindowInteractor()->Disable();

    /* The following line is a catch for what appears to be a KWWidgets bug. If we try
     * to get the last path from the registry, or even check to see if the last path
     * exists in the registry and the registry does not exist, the application returns 
     * from this function before finishing. Saving a dummy value into the registry 
     * guarantees that it exists before we try to get
     * the last path.
     */
    this->app->SetRegistryValue( 0, "DONOTUSE", "REGISTRYEXISTS","1");


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
                if( stat( lastPathString.c_str(),&st) == 0) {
                    dlg->SetLastPath( lastPathString.c_str());
                }
            } else if ( strcmp( openType, "spectra" ) == 0 ) {
                lastPathString = lastPathString.substr(0,found); 
                lastPathString += "/spectra";
                if( stat( lastPathString.c_str(),&st) == 0) {
                    dlg->SetLastPath( lastPathString.c_str());
                }
    
            }
        }
        
        // Check to see which extention to filter for.
        if( strcmp( openType,"image" ) == 0 || strcmp( openType, "overlay" ) == 0 ) {
            dlg->SetFileTypes("{{Volume Files} {.idf .fdf .dcm .DCM}} {{All files} {.*}}");
        } else if( strcmp( openType,"spectra" ) == 0 ) {
            dlg->SetFileTypes("{{Complex Data} {.ddf .shf .dcm}} {{All files} {.*}}");
        } else {
            dlg->SetFileTypes("{{All files} {.*}} {{Volume Files} {.idf .fdf .dcm .DCM}} {{Complex Data} {.ddf .shf .dcm}}");
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


string vtkSivicController::GetUserName()
{
    register struct passwd *psswd;
    register uid_t uid;
    uid = geteuid ();
    psswd = getpwuid (uid);
    string userName;  
    if (psswd) {
      userName.assign(psswd->pw_name);
    }
    return userName; 
}


string vtkSivicController::GetOsiriXInDir()
{
    string inDir("/Users/" + this->GetUserName() + "/Documents/OsiriX Data/INCOMING.noindex/");
    return inDir; 
}


void vtkSivicController::SaveDataOsiriX()
{
    string fname( this->GetOsiriXInDir() + "dicom_mrs.dcm" );
    this->SaveData( const_cast<char*>( fname.c_str() ));
}


/*! 
 *  Writes data files 
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
    if( this->model->GetDataObject( "SpectroscopicData" ) == NULL || this->model->GetDataObject( "AnatomicalData" ) == NULL ) {
        PopupMessage( "BOTH SPECTRA AND AN IMAGE MUST BE LOADED TO CREATE SECONDARY CAPTURES!" );
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
    dlg->SetFileTypes("{{DICOM Secondary Capture} {.dcm}} {{TIFF} {.tiff}} {{JPEG} {.jpeg}} {{PS} {.ps}} {{EPS} {.eps}} {{PDF} {.pdf}} {{SVG} {.svg}} {{TeX} {.tex}} {All files} {.*}}");
    dlg->SetDefaultExtension("{{DICOM Secondary Capture} {.dcm}} {{TIFF} {.tiff}} {{JPEG} {.jpeg}} {{PS} {.ps}} {{EPS} {.eps}} {{PDF} {.pdf}} {{SVG} {.svg}} {{All files} {.*}}");
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
    if( outputOption == sivicImageViewWidget::CURRENT_SLICE ) { 
        firstFrame = plotController->GetSlice();
        lastFrame = firstFrame + 1;
    }
    
    if( print ) {
        this->ToggleColorsForPrinting( sivicImageViewWidget::DARK_ON_LIGHT );
    }

    if ( this->model->DataExists("MetaboliteData") ) {
        vtkLabeledDataMapper::SafeDownCast(
            vtkActor2D::SafeDownCast(this->plotController->GetView()->GetProp( svkPlotGridView::OVERLAY_TEXT ))->GetMapper())->GetLabelTextProperty()->SetFontSize(18);
    }

    this->viewRenderingWidget->it->GetTextProperty()->SetFontSize(35);

    svkImageData* outputImage = NULL;

    /* svkImageWriter's require svkImageData with a header. So we will grab the spec header
     * and register the it so we can delete our new svkImageData object without deleting
     * the original header.
     */
    if( writer->IsA("svkImageWriter") ) {  
        outputImage = svkMriImageData::New();
        outputImage->SetDcmHeader( this->model->GetDataObject( "AnatomicalData" )->GetDcmHeader() );
        this->model->GetDataObject( "AnatomicalData" )->GetDcmHeader()->Register(this);
        
    }
    
    if( strcmp(captureType,"COMBINED_CAPTURE") == 0 ) {
        this->WriteCombinedCapture( writer, fileNameString, outputOption, outputImage, print);
    } else if( strcmp(captureType,"IMAGE_CAPTURE") == 0 ) {
        this->WriteImageCapture( writer, fileNameString, outputOption, outputImage, print);
    } else if( strcmp(captureType,"SPECTRA_CAPTURE") == 0 ) {
        this->WriteSpectraCapture( writer, fileNameString, outputOption, outputImage, print);
    } else if( strcmp(captureType,"SPECTRA_WITH_OVERVIEW_CAPTURE") == 0 ) {
        if( writer->IsA("svkDICOMSCWriter") ) {  
            svkDICOMSCWriter::SafeDownCast(writer)->SetCreateNewSeries( 0 );
        }
        this->WriteCombinedCapture( writer, fileNameString, outputOption, outputImage, print);
        if( writer->IsA("svkDICOMSCWriter") ) {  
            this->WriteImageCapture( writer, fileNameString, outputOption, outputImage, print,
                     outputImage->GetDcmHeader()->GetIntValue("InstanceNumber") + 1 );
        } else {
            this->WriteImageCapture( writer, fileNameString, outputOption, outputImage, print );
        }
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

    if ( this->model->DataExists("MetaboliteData") ) {
        vtkLabeledDataMapper::SafeDownCast(
            vtkActor2D::SafeDownCast(this->plotController->GetView()->GetProp( svkPlotGridView::OVERLAY_TEXT ))->GetMapper())->GetLabelTextProperty()->SetFontSize(12);
    }
    this->viewRenderingWidget->it->GetTextProperty()->SetFontSize(13);

    if( print ){
        ToggleColorsForPrinting( sivicImageViewWidget::LIGHT_ON_DARK );
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

    int firstFrame = 0;
    int lastFrame = this->model->GetDataObject("SpectroscopicData")->GetDcmHeader()->GetNumberOfSlices();
    if( outputOption == sivicImageViewWidget::CURRENT_SLICE ) { 
        firstFrame = plotController->GetSlice();
        lastFrame = firstFrame + 1;
    }
    int instanceNumber = 1;
    for (int m = firstFrame; m <= lastFrame; m++) {
        if( !static_cast<svkMrsImageData*>(this->model->GetDataObject( "SpectroscopicData" ))->SliceInSelectionBox(m) ) {
            continue;
        }
        string fileNameStringTmp = fileNameString; 

        ostringstream frameNum;
        frameNum <<  instanceNumber;

        this->SetSlice(m);

        //  Replace * with slice number in output file name: 
        size_t pos = fileNameStringTmp.find_last_of("*");
        if ( pos != string::npos) {
            fileNameStringTmp.replace(pos, 1, frameNum.str()); 
        } else {
            size_t pos = fileNameStringTmp.find_last_of(".");
            fileNameStringTmp.replace(pos, 1, frameNum.str() + ".");
        }

        cout << "FN: " << fileNameStringTmp.c_str() << endl;
        //  if Darwin: 
        //fileNameStringTmp.assign("\"" + fileNameStringTmp + "\""); 
        //cout << "FN: " << fileNameStringTmp.c_str() << endl;
        writer->SetFileName( fileNameStringTmp.c_str() );

        /*
         ==============================================================
         ==              FORMATTING OUTPUT IMAGE                     ==
         ==============================================================
        */

        /*
         * For now we are going to try to duplicate the output of mr.dev
         * We are goung to use the svkMultiWindowToImageFilter to concatenate
         * the image and the text, then use vtkImageAppend to concatenate the
         * combined image/text with spectrscopy. We cannot just use the
         * MultiWindowToImageFilter because it would not allow us to have
         * images side-by-side in a single "row".
         */

        // First lets concatenate the image, and the text
        svkMultiWindowToImageFilter* mw2ifprep = svkMultiWindowToImageFilter::New();
        if( print ) {
            mw2ifprep->SetPadConstant( 255 );
        }
        mw2ifprep->SetInput(     this->viewRenderingWidget->viewerWidget->GetRenderWindow(), 0, 0, 1 );
        mw2ifprep->SetInput(       this->viewRenderingWidget->infoWidget->GetRenderWindow(), 1, 0, 1 );
        mw2ifprep->Update();

        // Now lets use the multiwindow to get the image of the spectroscopy
        svkMultiWindowToImageFilter* mw2if = svkMultiWindowToImageFilter::New();
        if( print ) {
            mw2if->SetPadConstant( 255 );
        }
        //  starts index at bottom left corner of window
        //mw2if->SetInput(     viewerWidget->GetRenderWindow(), 0, 0, 1 );
        mw2if->SetInput( this->viewRenderingWidget->specViewerWidget->GetRenderWindow(), 0, 0, 2 );
        //mw2if->SetInput(       this->viewRenderingWidget->infoWidget->GetRenderWindow(), 2, 0, 1 );
        //mw2if->SetInput( this->viewRenderingWidget->titleWidget->GetRenderWindow(), 0, 3, 1 );
        mw2if->Update();

        // Now we need to pad the first image since it is smaller
        vtkImageConstantPad* padder = vtkImageConstantPad::New();
        if( print ) {
            padder->SetConstant( 255 );
        }
        int newExtent[6];
        int* prepImageExtent =  mw2ifprep->GetOutput()->GetExtent();
        int* specImageExtent =  mw2if->GetOutput()->GetExtent();
        newExtent[0] = specImageExtent[0];
        
        // The +1 -1 here is to get the resolution of the image and spectra secondary capture to match exactly.
        // TODO: Find a less manual way of making the resolution match.
        newExtent[1] = specImageExtent[1]+1;
        newExtent[2] = prepImageExtent[2];
        newExtent[3] = prepImageExtent[3]-1;
        newExtent[4] = prepImageExtent[4];
        newExtent[5] = prepImageExtent[5];
        padder->SetOutputWholeExtent( newExtent );
        padder->SetInput( mw2ifprep->GetOutput());

        // And use image append to add the specroscopy to the image+text
        vtkImageAppend* appender = vtkImageAppend::New();
        appender->SetAppendAxis( 1 );
        appender->SetInput(0, padder->GetOutput()); 
        appender->SetInput(1, mw2if->GetOutput()); 
        appender->Update();
        padder->Delete();

        /*
         ==============================================================
         ==           END FORMATTING OUTPUT IMAGE                    ==
         ==============================================================
        */

        if( writer->IsA("svkImageWriter") ) {  
            static_cast<svkImageWriter*>(writer)->SetInstanceNumber( instanceNumber );
            static_cast<svkImageWriter*>(writer)->SetSeriesDescription( "MRS_SC" );
            outputImage->GetDcmHeader()->SetValue( "InstanceNumber", instanceNumber );
            double dcos[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
            outputImage->CopyVtkImage( appender->GetOutput(), dcos );
            static_cast<svkImageWriter*>(writer)->SetInput( outputImage );
        } else {
            vtkImageFlip* flipper = vtkImageFlip::New();
            flipper->SetFilteredAxis( 1 );
            flipper->SetInput( appender->GetOutput() );
            writer->SetInput( flipper->GetOutput() );
            flipper->Delete();
            flipper = NULL;
        }
        instanceNumber++;

        writer->Write();
        
        mw2if->Delete();
        if( print ) {
            stringstream printCommand;
            printCommand << "lpr -h -P " << this->GetPrinterName().c_str() <<" "<<fileNameStringTmp.c_str(); 
            cout<< printCommand.str().c_str() << endl;
            system( printCommand.str().c_str() ); 
            stringstream removeCommand;
            removeCommand << "rm " << fileNameStringTmp.c_str(); 
            system( removeCommand.str().c_str() ); 
        }
    }
}


/*!
 *
 */
void vtkSivicController::WriteImageCapture( vtkImageWriter* writer, string fileNameString, int outputOption, svkImageData* outputImage, bool print, int instanceNumber ) 
{
    vtkTextActor* sliceLocation = vtkTextActor::New();
    this->viewRenderingWidget->viewerWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor( sliceLocation );
    int* winSize = this->viewRenderingWidget->viewerWidget->GetRenderWindow()->GetSize();
    sliceLocation->SetPosition(10,winSize[1]-25);
    
    // Lets start by determining the first frame we want to show.
    int i = 0;
    while(!static_cast<svkMrsImageData*>(this->model->GetDataObject( "SpectroscopicData" ))->SliceInSelectionBox(i) 
           && i <= this->model->GetDataObject("SpectroscopicData")->GetDcmHeader()->GetNumberOfSlices() - 1 ) {
        i++;
    }

    // We want to to start two frames before the start. This is the convention for mr.dev
    int firstFrame = i-2; 
    firstFrame = firstFrame < 0 ? 0 : firstFrame;
    i = this->model->GetDataObject("SpectroscopicData")->GetDcmHeader()->GetNumberOfSlices();
    while(!static_cast<svkMrsImageData*>(this->model->GetDataObject( "SpectroscopicData" ))->SliceInSelectionBox(i)  
           && i >= firstFrame ) {
        i--;
    }
    int lastFrame = i; 
    svkMultiWindowToImageFilter* mw2if = svkMultiWindowToImageFilter::New();
    string fileNameStringTmp = fileNameString; 

    vtkDataSetCollection* allImages = vtkDataSetCollection::New();
    vector <vtkImageAppend*> rowAppender; 
    rowAppender.reserve( (lastFrame-firstFrame)/2+1 ); 
    vtkImageAppend* colAppender = vtkImageAppend::New();
    colAppender->SetAppendAxis(1);
    for (int m = firstFrame; m <= lastFrame; m++) {
        vtkRenderLargeImage* rendererToImage = vtkRenderLargeImage::New();
        double origin[3];
        model->GetDataObject("SpectroscopicData")->GetDcmHeader()->GetOrigin(origin, lastFrame+firstFrame-m );
        // We need to reverse the slice order because of the direction of the appender.
        ostringstream position;
        position << "Slice: " << lastFrame+firstFrame-m+1 << "Pos(mm): " << origin[2];
        sliceLocation->SetInput( position.str().c_str() );
        this->SetSlice(lastFrame + firstFrame - m);
        vtkImageData* data = vtkImageData::New();
        allImages->AddItem( data );
        //  Replace * with slice number in output file name: 
        ostringstream frameNum;
        if( instanceNumber == 0 ) {
            frameNum << 0;
        } else {
            frameNum << instanceNumber;
        }
        size_t pos = fileNameStringTmp.find_last_of("*");
        if ( pos != string::npos) {
            fileNameStringTmp.replace(pos, 1, frameNum.str()); 
        } 

        cout << "FN: " << fileNameStringTmp.c_str() << endl;

        writer->SetFileName( fileNameStringTmp.c_str() );
    
        // Now lets use the multiwindow to get the image of the spectroscopy
        rendererToImage->SetInput( this->viewRenderingWidget->viewerWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer() );
        rendererToImage->SetMagnification(1);
        rendererToImage->Update();
        data->DeepCopy( rendererToImage->GetOutput() );
        data->Update();
        mw2if->SetInput( this->viewRenderingWidget->viewerWidget->GetRenderWindow(), m/2, m%2, 1 );
        if( (m - firstFrame) % 2 == 0 ) {
            rowAppender[(m-firstFrame)/2] = vtkImageAppend::New();
            rowAppender[(m-firstFrame)/2]->SetInput( 1, data );
            if( m == lastFrame - 1 ) {
                colAppender->SetInput( (m-firstFrame)/2, rowAppender[(m-firstFrame)/2]->GetOutput() );
            }
        } else {
            rowAppender[(m-firstFrame)/2]->SetInput( 0, data );
            rowAppender[(m-firstFrame)/2]->Update();
            colAppender->SetInput( (m-firstFrame)/2, rowAppender[(m-firstFrame)/2]->GetOutput() );
            rowAppender[(m-firstFrame)/2]->Delete();
        }
        //data->Delete();
        //colAppender->SetInput(m-firstFrame, data );
        rendererToImage->Delete();

    }
    colAppender->Update();
    mw2if->Update();

    if( writer->IsA("svkImageWriter") ) {  
        if( instanceNumber == 0 ) {
            static_cast<svkImageWriter*>(writer)->SetInstanceNumber( 1 );
        } else {
            static_cast<svkImageWriter*>(writer)->SetInstanceNumber( instanceNumber );
        }
        double dcos[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
        //outputImage->CopyVtkImage( mw2if->GetOutput(), dcos );
        vtkImageFlip* flipper = vtkImageFlip::New();
        flipper->SetFilteredAxis( 1 );
        flipper->SetInput( colAppender->GetOutput() );
        flipper->Update();
        outputImage->CopyVtkImage( flipper->GetOutput(), dcos );
        static_cast<svkImageWriter*>(writer)->SetInput( outputImage );
        flipper->Delete();
        flipper = NULL;
    } else {
        writer->SetInput( colAppender->GetOutput() );
    }

    writer->Write();
    //mw2if->Delete();
    this->viewRenderingWidget->viewerWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveViewProp( sliceLocation );

    if( print ) {
        stringstream printCommand;
        printCommand << "lpr -h -P " << this->GetPrinterName().c_str() <<" "<<fileNameStringTmp.c_str(); 
        cout<< printCommand.str().c_str() << endl;
        system( printCommand.str().c_str() ); 
        stringstream removeCommand;
        removeCommand << "rm " << fileNameStringTmp.c_str(); 
        system( removeCommand.str().c_str() ); 
    }
}


/*!
 *
 */
void vtkSivicController::WriteSpectraCapture( vtkImageWriter* writer, string fileNameString, int outputOption, svkImageData* outputImage, bool print ) 
{
    int firstFrame = 0;
    int lastFrame = this->model->GetDataObject("SpectroscopicData")->GetDcmHeader()->GetNumberOfSlices();
    if( outputOption == sivicImageViewWidget::CURRENT_SLICE ) { 
        firstFrame = plotController->GetSlice();
        lastFrame = firstFrame + 1;
    }
    int instanceNumber = 1;
    for (int m = firstFrame; m <= lastFrame; m++) {
        if( !static_cast<svkMrsImageData*>(this->model->GetDataObject( "SpectroscopicData" ))->SliceInSelectionBox(m) ) {
            continue;
        }
        string fileNameStringTmp = fileNameString; 

        ostringstream frameNum;
        frameNum <<  instanceNumber;

        this->SetSlice(m);

        //  Replace * with slice number in output file name: 
        size_t pos = fileNameStringTmp.find_last_of("*");
        if ( pos != string::npos) {
            fileNameStringTmp.replace(pos, 1, frameNum.str()); 
        } else {
            size_t pos = fileNameStringTmp.find_last_of(".");
            fileNameStringTmp.replace(pos, 1, frameNum.str() + ".");
        }

        cout << "FN: " << fileNameStringTmp.c_str() << endl;
        writer->SetFileName( fileNameStringTmp.c_str() );

        /*
         ==============================================================
         ==              FORMATTING OUTPUT IMAGE                     ==
         ==============================================================
        */

        /*
         * For now we are going to try to duplicate the output of mr.dev
         * We are goung to use the svkMultiWindowToImageFilter to concatenate
         * the image and the text, then use vtkImageAppend to concatenate the
         * combined image/text with spectrscopy. We cannot just use the
         * MultiWindowToImageFilter because it would not allow us to have
         * images side-by-side in a single "row".
         */

        // Now lets use the multiwindow to get the image of the spectroscopy
        svkMultiWindowToImageFilter* mw2if = svkMultiWindowToImageFilter::New();
        mw2if->SetInput( this->viewRenderingWidget->specViewerWidget->GetRenderWindow(), 0, 0, 2 );
        mw2if->Update();

        if( writer->IsA("svkImageWriter") ) {  
            static_cast<svkImageWriter*>(writer)->SetInstanceNumber( instanceNumber );
            outputImage->GetDcmHeader()->SetValue( "InstanceNumber", instanceNumber );
            double dcos[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
            outputImage->CopyVtkImage( mw2if->GetOutput(), dcos );
            static_cast<svkImageWriter*>(writer)->SetInput( outputImage );
        } else {
            vtkImageFlip* flipper = vtkImageFlip::New();
            flipper->SetFilteredAxis( 1 );
            flipper->SetInput( mw2if->GetOutput() );
            writer->SetInput( flipper->GetOutput() );
            flipper->Delete();
            flipper = NULL;
        }
        instanceNumber++;

        writer->Write();
        
        mw2if->Delete();
        if( print ) {
            stringstream printCommand;
            printCommand << "lpr -h -P " << this->GetPrinterName().c_str() <<" "<<fileNameStringTmp.c_str(); 
            cout<< printCommand.str().c_str() << endl;
            system( printCommand.str().c_str() ); 
            stringstream removeCommand;
            removeCommand << "rm " << fileNameStringTmp.c_str(); 
            system( removeCommand.str().c_str() ); 
        }
    }
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
    if( outputOption == sivicImageViewWidget::CURRENT_SLICE ) { 
        firstFrame = plotController->GetSlice();
        lastFrame = firstFrame + 1;
    }
    int instanceNumber = 1;
    for (int m = firstFrame; m <= lastFrame; m++) {
        if( !static_cast<svkMrsImageData*>(this->model->GetDataObject( "SpectroscopicData" ))->SliceInSelectionBox(m) ) {
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
    if( colorSchema == sivicImageViewWidget::DARK_ON_LIGHT ) {
        backgroundColor[0] = 1;
        backgroundColor[1] = 1;
        backgroundColor[2] = 1;
        foregroundColor[0] = 0;
        foregroundColor[1] = 0;
        foregroundColor[2] = 0;
        plotGridSchema = svkPlotGridView::DARK_ON_LIGHT;
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
    vtkRenderer* tmpRenderer = this->viewRenderingWidget->titleWidget->GetRenderer();
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
    if( this->viewRenderingWidget != NULL ) {
        this->viewRenderingWidget->SetModel(this->model);
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
        this->imageViewWidget->axialSlider->EnabledOn();
        this->imageViewWidget->coronalSlider->EnabledOn();
        this->imageViewWidget->sagittalSlider->EnabledOn();

        //svkImageData* data = this->model->GetDataObject( "AnatomicalData" );
        //int firstSlice = data->GetFirstSlice( svkDcmHeader::CORONAL );
        //int lastSlice = data->GetLastSlice( svkDcmHeader::CORONAL );
        //this->imageViewWidget->coronalSlider->SetRange( firstSlice + 1, lastSlice + 1); 
        //this->imageViewWidget->coronalSlider->SetValue( ( lastSlice - firstSlice ) / 2);
        //firstSlice = data->GetFirstSlice( svkDcmHeader::SAGITTAL );
        //lastSlice = data->GetLastSlice( svkDcmHeader::SAGITTAL );
        //this->imageViewWidget->sagittalSlider->SetRange( firstSlice + 1, lastSlice + 1); 
        //this->imageViewWidget->sagittalSlider->SetValue( ( lastSlice - firstSlice ) / 2);
    }
}


//! Resets the window level to full range.
void vtkSivicController::ResetWindowLevel()
{
    this->overlayController->ResetWindowLevel();
    this->viewRenderingWidget->specViewerWidget->Render();
    this->viewRenderingWidget->viewerWidget->Render();

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


void vtkSivicController::DisplayInfo()
{
    app->DisplayHelpDialog( app->GetNthWindow(0) );
}


/*
 *
 */
void vtkSivicController::SetSpecUnitsCallback(int targetUnits)
{

    //  Convert the current values to the target unit scale:
    float lowestPoint = spectraViewWidget->point->ConvertPosUnits(
        this->spectraViewWidget->xSpecRange->GetEntry1()->GetValueAsDouble(),
        this->spectraViewWidget->specUnits, 
        targetUnits 
    );

    float highestPoint = spectraViewWidget->point->ConvertPosUnits(
        this->spectraViewWidget->xSpecRange->GetEntry2()->GetValueAsDouble(),
        this->spectraViewWidget->specUnits, 
        targetUnits 
    );

    //  convert the Whole Range to the target unit scale:
    float lowestPointRange = spectraViewWidget->point->ConvertPosUnits(
        0,
        svkSpecPoint::PTS, 
        targetUnits 
    );

    float highestPointRange = spectraViewWidget->point->ConvertPosUnits(
        spectraData->GetCellData()->GetArray(0)->GetNumberOfTuples(), 
        svkSpecPoint::PTS, 
        targetUnits 
    );

    this->spectraViewWidget->specUnits = targetUnits; 

    //  Adjust the slider resolution for the target units:
    if ( targetUnits == svkSpecPoint::PPM ) {
        this->spectraViewWidget->xSpecRange->SetResolution( .001 );
        this->spectraViewWidget->unitSelectBox->GetWidget()->SetValue( "PPM" );
    } else if ( targetUnits == svkSpecPoint::Hz ) {
        this->spectraViewWidget->xSpecRange->SetResolution( .1 );
        this->spectraViewWidget->unitSelectBox->GetWidget()->SetValue( "HZ" );
    } else if ( targetUnits == svkSpecPoint::PTS ) {
        this->spectraViewWidget->xSpecRange->SetResolution( 1 );
        this->spectraViewWidget->unitSelectBox->GetWidget()->SetValue( "PTS" );
        lowestPoint = (float)(nearestInt(lowestPoint)); 
        highestPoint = (float)(nearestInt(highestPoint)); 
        lowestPointRange = (float)(nearestInt(lowestPointRange)); 
        highestPointRange = (float)(nearestInt(highestPointRange)); 
        if (lowestPoint == 0) {
            lowestPoint = 1; 
        }
        if (lowestPointRange == 0) {
            lowestPointRange = 1; 
        }
    }

    this->spectraViewWidget->detailedPlotController->SetUnits( this->spectraViewWidget->specUnits );
    this->spectraViewWidget->detailedPlotController->GetView()->Refresh( );
    this->spectraViewWidget->xSpecRange->SetWholeRange( lowestPointRange, highestPointRange );
    this->spectraViewWidget->xSpecRange->SetRange( lowestPoint, highestPoint ); 

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
            this->ResetRange(1);
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
 *
 */
void vtkSivicController::Print( int outputOption )
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
    this->SaveSecondaryCapture( "tmpImage.ps", seriesNumber, "COMBINED_CAPTURE", outputOption, 1 );
}


/*!
 *
 */
void vtkSivicController::PopupMessage( string message ) 
{
    vtkKWMessageDialog *messageDialog = vtkKWMessageDialog::New();
    messageDialog->SetApplication(app);
    messageDialog->Create();
    messageDialog->SetOptions( vtkKWMessageDialog::ErrorIcon );
    messageDialog->SetText(message.c_str());
    messageDialog->Invoke();
    messageDialog->Delete();
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
void vtkSivicController::SetOrientation( const char* orientation ) 
{
    // Set Our orientation member variable
    svkDcmHeader::Orientation newOrientation = svkDcmHeader::UNKNOWN;
    this->orientation = orientation;
    int firstSlice;
    int lastSlice;
    int toggleDraw = this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->GetDraw();
    if( toggleDraw ) {
        this->overlayController->GetView()->GetRenderer( svkOverlayView::PRIMARY )->DrawOff();
        this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->DrawOff();
    }

    if( this->orientation == "AXIAL" ) {
        this->plotController->GetView()->SetOrientation( svkDcmHeader::AXIAL );
        this->overlayController->GetView()->SetOrientation( svkDcmHeader::AXIAL );
        newOrientation = svkDcmHeader::AXIAL;
        int index = this->globalWidget->orientationSelect->GetWidget()->GetMenu()->GetIndexOfItem("AXIAL");
        if( index != this->globalWidget->orientationSelect->GetWidget()->GetMenu()->GetIndexOfSelectedItem()) {
            this->globalWidget->orientationSelect->GetWidget()->GetMenu()->SelectItem( index );
        }
    } else if ( this->orientation == "CORONAL" ) {
        this->plotController->GetView()->SetOrientation( svkDcmHeader::CORONAL );
        this->overlayController->GetView()->SetOrientation( svkDcmHeader::CORONAL );
        newOrientation = svkDcmHeader::CORONAL;
        int index = this->globalWidget->orientationSelect->GetWidget()->GetMenu()->GetIndexOfItem("CORONAL");
        if( index != this->globalWidget->orientationSelect->GetWidget()->GetMenu()->GetIndexOfSelectedItem()) {
            this->globalWidget->orientationSelect->GetWidget()->GetMenu()->SelectItem( index );
        }
    } else if ( this->orientation == "SAGITTAL" ) {
        this->plotController->GetView()->SetOrientation( svkDcmHeader::SAGITTAL );
        this->overlayController->GetView()->SetOrientation( svkDcmHeader::SAGITTAL );
        newOrientation = svkDcmHeader::SAGITTAL;
        int index = this->globalWidget->orientationSelect->GetWidget()->GetMenu()->GetIndexOfItem("SAGITTAL");
        if( index != this->globalWidget->orientationSelect->GetWidget()->GetMenu()->GetIndexOfSelectedItem()) {
            this->globalWidget->orientationSelect->GetWidget()->GetMenu()->SelectItem( index );
        }
    }

    if( this->model->DataExists("SpectroscopicData") ) {
        firstSlice = this->model->GetDataObject("SpectroscopicData")->GetFirstSlice( newOrientation );
        lastSlice =  this->model->GetDataObject("SpectroscopicData")->GetLastSlice( newOrientation );
        this->spectraViewWidget->sliceSlider->SetRange( firstSlice + 1, lastSlice + 1 );
        this->spectraViewWidget->sliceSlider->SetValue( this->plotController->GetSlice()+1 );
        this->SetSlice( this->plotController->GetSlice() );
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
void vtkSivicController::ResetRange( bool useFullRange, bool resetChannel )
{
    svkImageData* data = this->model->GetDataObject( "SpectroscopicData" ); 
    if( data != NULL ) {
        string domain = model->GetDataObject( "SpectroscopicData" )->GetDcmHeader()->GetStringValue("SignalDomainColumns");
        float min = 1;
        float max = data->GetCellData()->GetArray(0)->GetNumberOfTuples(); 
        this->SetSpecUnitsCallback(svkSpecPoint::PPM);
        if( domain == "FREQUENCY" ) {
            min = this->spectraViewWidget->point->ConvertPosUnits(
                            0,
                            svkSpecPoint::PTS,
                            svkSpecPoint::PPM
                                );
            max = this->spectraViewWidget->point->ConvertPosUnits(
                            data->GetCellData()->GetArray(0)->GetNumberOfTuples(),
                            svkSpecPoint::PTS,
                            svkSpecPoint::PPM
                                );

            this->spectraViewWidget->xSpecRange->SetWholeRange( min, max );
            if ( useFullRange ) {
                this->spectraViewWidget->xSpecRange->SetRange( min, max );
            } else {
                this->spectraViewWidget->xSpecRange->SetRange( PPM_DEFAULT_MIN, PPM_DEFAULT_MAX );
            }
        } else {
            this->SetSpecUnitsCallback(svkSpecPoint::PTS);
            this->spectraViewWidget->xSpecRange->SetWholeRange( min, max );
            this->spectraViewWidget->xSpecRange->SetRange( min, max );
        }

        double range[2];
        if( this->plotController->GetComponent() == svkBoxPlot::REAL ) {
            data->GetDataRange( range, svkImageData::REAL  );
        } else if( this->plotController->GetComponent() == svkBoxPlot::IMAGINARY ) {
            data->GetDataRange( range, svkImageData::IMAGINARY  );
        } else if( this->plotController->GetComponent() == svkBoxPlot::MAGNITUDE ) {
            data->GetDataRange( range, svkImageData::MAGNITUDE  );
        } else {
            cout << "UNKNOWN COMPONENT: " << this->plotController->GetComponent() << endl; 
        }
        this->spectraViewWidget->ySpecRange->SetWholeRange( range[0], range[1] );
        if ( useFullRange || domain == "TIME" ) {
            this->spectraViewWidget->ySpecRange->SetRange( range[0], range[1] );
        } else {
            this->spectraViewWidget->ySpecRange->SetRange( range[0]*NEG_RANGE_SCALE, range[1]*POS_RANGE_SCALE );
        }
        this->spectraViewWidget->ySpecRange->SetResolution( (range[1] - range[0])*SLIDER_RELATIVE_RESOLUTION );
        //We now need to reset the range of the plotController
        float lowestPoint = this->spectraViewWidget->point->ConvertPosUnits(
            this->spectraViewWidget->xSpecRange->GetEntry1()->GetValueAsDouble(),
            this->spectraViewWidget->specUnits,
            svkSpecPoint::PTS
        );

        float highestPoint = this->spectraViewWidget->point->ConvertPosUnits(
            this->spectraViewWidget->xSpecRange->GetEntry2()->GetValueAsDouble(),
            this->spectraViewWidget->specUnits,
            svkSpecPoint::PTS
        );

        // Lets also reset the number of channels:
        if( resetChannel ) { 
            int channels = svkMrsImageData::SafeDownCast( data )->GetDcmHeader()->GetNumberOfCoils();
            this->spectraViewWidget->channelSlider->SetRange( 1, channels); 
            this->spectraViewWidget->channelSlider->SetValue( 1 );
        }
        this->plotController->SetWindowLevelRange( lowestPoint, highestPoint, svkPlotGridView::FREQUENCY);
        this->spectraViewWidget->detailedPlotController->SetWindowLevelRange( lowestPoint, highestPoint, svkDetailedPlotView::FREQUENCY);

    }
}


/*
 *  Logic to determine whether to enable widgets. 
 */
void vtkSivicController::EnableWidgets()
{
    this->DisableWidgets();
    if ( model->DataExists("SpectroscopicData") && model->DataExists("AnatomicalData") ) {
        this->globalWidget->metaboliteSelect->EnabledOn();
        string acquisitionType = model->GetDataObject( "SpectroscopicData" )->
                                GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
        if( acquisitionType == "SINGLE VOXEL" ) {
            this->imageViewWidget->plotGridButton->EnabledOff();
        } else {
            this->imageViewWidget->plotGridButton->EnabledOn();
        }
        this->imageViewWidget->volSelButton->EnabledOn();
    }

    if ( model->DataExists("SpectroscopicData") ) {
        this->spectraViewWidget->sliceSlider->EnabledOn();
        string domain = model->GetDataObject( "SpectroscopicData" )->GetDcmHeader()->GetStringValue("SignalDomainColumns");
        int numChannels = svkMrsImageData::SafeDownCast( model->GetDataObject("SpectroscopicData"))->GetDcmHeader()->GetNumberOfCoils();
        if( domain == "TIME" ) {
            this->processingWidget->fftButton->EnabledOn(); 
            this->processingWidget->phaseButton->EnabledOff(); 
            this->processingWidget->combineButton->EnabledOff(); 
            this->spectraViewWidget->xSpecRange->SetLabelText( "Time" );
            this->spectraViewWidget->unitSelectBox->EnabledOff();
        } else {
            this->processingWidget->fftButton->EnabledOff(); 
            this->processingWidget->phaseButton->EnabledOn(); 
            if( numChannels > 1 ) {
                this->processingWidget->combineButton->EnabledOn();
            }
            this->spectraViewWidget->xSpecRange->SetLabelText( "Frequency" );
            this->spectraViewWidget->unitSelectBox->EnabledOn();
        }
        this->imageViewWidget->satBandButton->EnabledOn();
        this->imageViewWidget->satBandOutlineButton->EnabledOn();
        this->processingWidget->phaseSlider->EnabledOn(); 
        this->spectraViewWidget->xSpecRange->EnabledOn();
        this->spectraViewWidget->ySpecRange->EnabledOn();
        this->processingWidget->phaseAllChannelsButton->EnabledOn(); 
        this->processingWidget->phaseAllVoxelsButton->EnabledOn(); 
        this->spectraViewWidget->componentSelectBox->EnabledOn();
        if( numChannels > 1 ) {
            this->spectraViewWidget->channelSlider->EnabledOn();
        }
        int numTimePoints = svkMrsImageData::SafeDownCast( model->GetDataObject("SpectroscopicData"))->GetDcmHeader()->GetNumberOfTimePoints();
        if( numTimePoints > 1 ) {
            this->spectraViewWidget->timePointSlider->EnabledOn();
        }
    }

    if ( model->DataExists("AnatomicalData") ) {
        if( this->orientation == "AXIAL" ) {
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
    }

    if ( model->DataExists("MetaboliteData")) {
        this->spectraViewWidget->overlayImageCheck->EnabledOn();
        this->spectraViewWidget->overlayTextCheck->EnabledOn();
    } 
    if ( model->DataExists("OverlayData") ||  model->DataExists("MetaboliteData")) {
        this->imageViewWidget->interpolationBox->EnabledOn();
        this->imageViewWidget->lutBox->EnabledOn();
        this->imageViewWidget->overlayOpacitySlider->EnabledOn();
        this->imageViewWidget->overlayThresholdSlider->EnabledOn();
        this->imageViewWidget->overlayButton->EnabledOn();
        this->imageViewWidget->colorBarButton->EnabledOn();
    }
}


/*!
 *
 */
void vtkSivicController::DisableWidgets()
{
    this->imageViewWidget->volSelButton->EnabledOff();
    this->imageViewWidget->plotGridButton->EnabledOff();

    this->spectraViewWidget->detailedPlotButton->EnabledOff();
    this->spectraViewWidget->sliceSlider->EnabledOff();
    this->spectraViewWidget->channelSlider->EnabledOff();
    this->spectraViewWidget->timePointSlider->EnabledOff();
    this->imageViewWidget->axialSlider->EnabledOff();
    this->imageViewWidget->coronalSlider->EnabledOff();
    this->imageViewWidget->sagittalSlider->EnabledOff();
    this->spectraViewWidget->overlayImageCheck->EnabledOff();
    this->spectraViewWidget->overlayTextCheck->EnabledOff();
    this->spectraViewWidget->unitSelectBox->EnabledOff();
    this->spectraViewWidget->componentSelectBox->EnabledOff();
    this->spectraViewWidget->xSpecRange->EnabledOff();
    this->spectraViewWidget->ySpecRange->EnabledOff();

    this->processingWidget->phaseSlider->EnabledOff(); 
    this->processingWidget->phaseAllChannelsButton->EnabledOff(); 
    this->processingWidget->phaseAllVoxelsButton->EnabledOff(); 
    this->processingWidget->fftButton->EnabledOff(); 
    this->processingWidget->phaseButton->EnabledOff(); 
    this->processingWidget->combineButton->EnabledOff(); 

    this->imageViewWidget->orthImagesButton->EnabledOff();

    this->imageViewWidget->interpolationBox->EnabledOff();
    this->imageViewWidget->lutBox->EnabledOff();
    this->imageViewWidget->overlayOpacitySlider->EnabledOff();
    this->imageViewWidget->overlayThresholdSlider->EnabledOff();
    this->imageViewWidget->overlayButton->EnabledOff();
    this->imageViewWidget->colorBarButton->EnabledOff();
    this->imageViewWidget->satBandButton->EnabledOff();
    this->imageViewWidget->satBandOutlineButton->EnabledOff();
    this->globalWidget->metaboliteSelect->EnabledOff();
}


/*!
 *  Runs tests for the application.
 */

void vtkSivicController::RunTestingSuite()
{
    sivicTestSuite* suite = new sivicTestSuite( this );
    suite->RunTests();
}


/*
 *   Returns the nearest int.  For values at the mid-point,
 *   the value is rounded to the larger int.
 */
int nearestInt(float x) 
{
    int x_to_int;
    x_to_int = (int) x;

    /*
     *   First do positive numbers, then negative ones.
     */
    if (x>=0) {
        if ((x - x_to_int) >= 0.5) {
            x_to_int += 1;   
        }
    } else {
        if ((x_to_int - x) > 0.5) {
            x_to_int -= 1;   
        }
    }

    return (int) x_to_int;   
}

