/*
 *  Copyright © 2009 The Regents of the University of California.
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
    this->plotController->SetSlice(slice);
    this->overlayController->SetSlice(slice);
    this->plotController->GetView()->Refresh();
    this->overlayController->GetView()->Refresh();
    this->viewRenderingWidget->ResetInfoText();
    if( this->model->DataExists("AnatomicalData")) {
       this->imageViewWidget->imageSlider->SetValue( this->overlayController->GetImageSlice()+1 ); 
    }
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
                this->imageViewWidget->sliceSlider->SetRange( extent[4] + 1, extent[5] + 1); 
                this->imageViewWidget->sliceSlider->SetValue( ( extent[5] - extent[4] ) / 2);
                this->imageViewWidget->orthoXSlider->SetRange( extent[0] + 1, extent[1] + 1 );
                this->imageViewWidget->orthoXSlider->SetValue( ( extent[1] - extent[0] ) / 2);
                this->imageViewWidget->orthoYSlider->SetRange( extent[2] + 1, extent[3] + 1 );
                this->imageViewWidget->orthoYSlider->SetValue( ( extent[3] - extent[2] ) / 2);
            } else {
                this->overlayController->SetTlcBrc( plotController->GetTlcBrc() );
            }
            this->imageViewWidget->imageSlider->SetRange( extent[4] + 1, extent[5] + 1); 
            this->viewRenderingWidget->ResetInfoText();
        } else {
            string message = "ERROR: Dataset is not compatible and will not be loaded!\nInfo:\n"; 
            message += resultInfo;
            this->PopupMessage( resultInfo ); 
        }
    }
}


void vtkSivicController::OpenSpectra( const char* fileName )
{
    string stringFilename(fileName);
    svkImageData* oldData = model->GetDataObject("SpectroscopicData");
    svkImageData* newData = model->LoadFile( stringFilename );

    if (newData == NULL) {
        this->PopupMessage( "UNSUPPORTED FILE TYPE!");
    } else {

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
            if( tlcBrc == NULL ) {
                int channels = svkMrsImageData::SafeDownCast( newData )->GetDcmHeader()->GetNumberOfCoils();
                this->imageViewWidget->sliceSlider->SetRange( extent[4]+1, extent[5]); 
                int centerSlice = (extent[5] - extent[4] ) / 2;
                this->imageViewWidget->sliceSlider->SetValue( centerSlice + 1);
                this->processingWidget->channelSlider->SetRange( 1, channels); 
                this->processingWidget->channelSlider->SetValue( 1 );
                this->plotController->SetSlice( centerSlice ); 
                this->overlayController->SetSlice( centerSlice ); 
            }
            this->plotController->SetInput( newData ); 
            this->spectraViewWidget->detailedPlotController->SetInput( newData ); 
            this->overlayController->SetInput( newData, svkOverlayView::MRS ); 
    
            this->processingWidget->phaseSlider->SetValue(0.0); 
            this->processingWidget->phaser->SetInput( newData );
    
            this->spectraViewWidget->point->SetDataHdr( newData->GetDcmHeader() );
   
            this->ResetRange( );
            if( tlcBrc == NULL ) {
                this->plotController->HighlightSelectionVoxels();
                this->overlayController->HighlightSelectionVoxels();
            } else {
                this->plotController->SetTlcBrc( tlcBrc ); 
                this->overlayController->SetTlcBrc( tlcBrc ); 
            }
            this->viewRenderingWidget->ResetInfoText();
        } else {
            resultInfo = "ERROR: Dataset is not compatible!\n"; 
            resultInfo += "Info:\n"; 
            resultInfo += plotViewResultInfo;
            resultInfo += overlayViewResultInfo;
            this->PopupMessage( resultInfo ); 
        }

    }

}


void vtkSivicController::OpenOverlay( const char* fileName )
{
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
    char cwd[MAXPATHLEN];
    char lastPath[MAXPATHLEN];
    bool usingImagesFolder = 0;
    getcwd(cwd, MAXPATHLEN);
    string imagePathName(cwd);

    // First we open an image

    // Lets check for an images folder
    if(stat("images",&st) == 0) {
        imagePathName+= "/images";
        //cout << "Switching to image path:" << imagePathName.c_str() << endl;
        this->OpenFile( "image", imagePathName.c_str() ); 
    } else {
        this->OpenFile( "image", NULL ); 
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
        usingImagesFolder = 1;
        string spectraPathName;
        spectraPathName = lastPathString.substr(0,found); 
        spectraPathName += "/spectra";
        if( stat(spectraPathName.c_str(),&st) == 0 ) {
            this->OpenFile( "spectra", spectraPathName.c_str() ); 
        } else {
            // If an images folder was used, but there is no corresponding spectra folder
            this->OpenFile( "spectra", lastPathString.substr(0,found).c_str() ); 
        }
    } else { 
        this->OpenFile( "spectra", lastPathString.c_str() ); 
    }

}


/*!    Open a file.    */
void vtkSivicController::OpenFile( char* openType, const char* startPath )
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

    this->imageViewWidget->loadingLabel->SetText("Done!");
    if ( dlg != NULL) {
        dlg->SaveLastPathToRegistry("lastPath");
        dlg->Delete();
    }
    app->ProcessPendingEvents(); 
    this->viewRenderingWidget->viewerWidget->GetRenderWindowInteractor()->Enable();
    this->viewRenderingWidget->specViewerWidget->GetRenderWindowInteractor()->Enable();
    this->imageViewWidget->loadingLabel->SetText("");
}


//! Saves Data File.
void vtkSivicController::SaveData()
{
    vtkKWFileBrowserDialog *dlg = vtkKWFileBrowserDialog::New();
    dlg->SetApplication(app);
    dlg->SaveDialogOn();
    dlg->Create();
    dlg->SetInitialFileName("E1S1I1");
    dlg->SetFileTypes("{{DICOM MR Spectroscopy} {.dcm}}");
    dlg->SetDefaultExtension("{{DICOM MR Spectroscoy} {.dcm}}");
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
    writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter(svkImageWriterFactory::DICOM_MRS));
    cout << "FN: " << fileName << endl;
    writer->SetFileName(fileName);
    writerFactory->Delete();

    writer->SetInput( this->model->GetDataObject("SpectroscopicData") );

    writer->Write();
    writer->Delete();
}


//! Saves a secondary capture.
void vtkSivicController::SaveSecondaryCapture()
{
    if( this->model->GetDataObject( "SpectroscopicData" ) == NULL ) {
        PopupMessage( "NO SPECTRA LOADED!" );
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
    dlg->Invoke();
    if ( dlg->GetStatus() == vtkKWDialog::StatusOK ) {
        string filename = dlg->GetFileName(); 
        char* cStrFilename = new char [filename.size()+1];
        strcpy (cStrFilename, filename.c_str());
        this->SaveSecondaryCapture( cStrFilename, seriesNumber );
    } 
    dlg->Delete();
}



void vtkSivicController::SaveSecondaryCaptureOsiriX()
{
    string fname( this->GetOsiriXInDir() + "sc.dcm" );
    this->SaveSecondaryCapture( const_cast<char*>( fname.c_str() ), 77);
}


/*! 
 *  Writes a screen capture from a render window to a .dcm file
 */   
void vtkSivicController::SaveSecondaryCapture( char* fileName, int seriesNumber, int outputOption, bool print )
{
cout << "PATH: " << fileName << endl;
    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    vtkImageWriter* writer = NULL;
    vtkImageFlip* flipper = vtkImageFlip::New();
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
        flipper->SetFilteredAxis( 1 );
    } else if( strcmp( fileNameString.substr(pos).c_str(), ".tiff" ) == 0 ) {
        writer = writerFactory->CreateImageWriter( svkImageWriterFactory::TIFF );
        vtkTIFFWriter* tmp = vtkTIFFWriter::SafeDownCast( writer );
        tmp->SetCompression(0);
        flipper->SetFilteredAxis( 1 );
    } else if( strcmp( fileNameString.substr(pos).c_str(), ".jpeg" ) == 0 ) {
        writer = writerFactory->CreateImageWriter( svkImageWriterFactory::JPEG );
        vtkJPEGWriter* tmp = vtkJPEGWriter::SafeDownCast( writer );
        tmp->SetQuality(100);
        flipper->SetFilteredAxis( 1 );
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

    int numberOfFrames = this->model->GetDataObject("SpectroscopicData")->GetDcmHeader()->GetIntValue( "NumberOfFrames" );
    int numberOfChannels = this->model->GetDataObject("SpectroscopicData")->GetDcmHeader()->GetNumberOfCoils();
    int numberOfSlices = numberOfFrames/numberOfChannels; 

    svkImageData* outputImage = NULL;

    /* svkImageWriter's require svkImageData with a header. So we will grab the spec header
     * and register the it so we can delete our new svkImageData object without deleting
     * the original header.
     */
    if( writer->IsA("svkImageWriter") ) {  
        outputImage = svkMrsImageData::New();
        outputImage->SetDcmHeader( this->model->GetDataObject( "SpectroscopicData" )->GetDcmHeader() );
        this->model->GetDataObject( "SpectroscopicData" )->GetDcmHeader()->Register(this);

    }
    

    int firstFrame = 0;
    int lastFrame = numberOfSlices;
    if( outputOption == sivicImageViewWidget::CURRENT_SLICE ) { 
        firstFrame = plotController->GetSlice();
        lastFrame = firstFrame + 1;
    }
    
    if( print ){
        this->ToggleColorsForPrinting( sivicImageViewWidget::DARK_ON_LIGHT );
        printerName = this->GetPrinterName();
    }
    for (int m = firstFrame; m < lastFrame; m++) {

        string fileNameStringTmp = fileNameString; 

        ostringstream frameNum;
        frameNum << m;

        SetSlice(m);

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
        // This guarantees no clipping at the edge
        newExtent[1] = specImageExtent[1]-1;
        newExtent[2] = prepImageExtent[2];
        newExtent[3] = prepImageExtent[3];
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
            static_cast<svkImageWriter*>(writer)->SetInstanceNumber( m + 1 );
            double dcos[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
            outputImage->CopyVtkImage( appender->GetOutput(), dcos );
            static_cast<svkImageWriter*>(writer)->SetInput( outputImage );
        } else {
            flipper->SetInput( appender->GetOutput() );
            writer->SetInput( flipper->GetOutput() );
        }

        writer->Write();

        if( print ) {

            stringstream printCommand;
            printCommand << "lpr -h -P " << printerName.c_str() <<" "<< fileNameStringTmp.c_str(); 
            cout<< printCommand.str().c_str() << endl;
            system( printCommand.str().c_str() ); 
            stringstream removeCommand;
            removeCommand << "rm " << fileNameStringTmp.c_str(); 
            system( removeCommand.str().c_str() ); 
        }
        mw2if->Delete();
    }

    this->SetSlice(currentSlice);

    //this->viewRenderingWidget->viewerWidget->GetRenderWindow()->OffScreenRenderingOff();
    //this->viewRenderingWidget->specViewerWidget->GetRenderWindow()->OffScreenRenderingOff();
    //this->viewRenderingWidget->titleWidget->GetRenderWindow()->OffScreenRenderingOff();
    //this->viewRenderingWidget->infoWidget->GetRenderWindow()->OffScreenRenderingOff();

    if (flipper != NULL) {
        flipper->Delete();
        flipper = NULL; 
    }
    if (outputImage != NULL) {
        outputImage->Delete();
        outputImage = NULL; 
    }
    if( wasCursorLocationOn && this->overlayController != NULL ) {
        this->overlayController->GetView()->TurnRendererOn( svkOverlayView::MOUSE_LOCATION );    
    }

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
    this->overlayController->UseSelectionStyle();
    this->viewRenderingWidget->specViewerWidget->Render();
    this->viewRenderingWidget->viewerWidget->Render();
    this->imageViewWidget->orthoXSlider->EnabledOff();
    this->imageViewWidget->orthoYSlider->EnabledOff();
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
        this->imageViewWidget->orthoXSlider->EnabledOn();
        this->imageViewWidget->orthoYSlider->EnabledOn();

        svkImageData* data = this->model->GetDataObject( "AnatomicalData" );
        int* extent = data->GetExtent();
        this->imageViewWidget->orthoXSlider->SetRange( extent[0] + 1, extent[1] + 1); 
        this->imageViewWidget->orthoXSlider->SetValue( ( extent[1] - extent[0] ) / 2);
        this->imageViewWidget->orthoYSlider->SetRange( extent[2] + 1, extent[3] + 1); 
        this->imageViewWidget->orthoYSlider->SetValue( ( extent[3] - extent[2] ) / 2);
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
    if ( targetComponent == svkBoxPlot::REAL) {
        this->plotController->SetComponent(svkBoxPlot::REAL);
    } else if ( targetComponent == svkBoxPlot::IMAGINARY) {
        this->plotController->SetComponent(svkBoxPlot::IMAGINARY);
    } else if ( targetComponent == svkBoxPlot::MAGNITUDE) {
        this->plotController->SetComponent(svkBoxPlot::MAGNITUDE);
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
    this->SaveSecondaryCapture( "tmpImage.ps", seriesNumber, outputOption, 1 );
}


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


string vtkSivicController::GetPrinterName( )
{
    // Once we have the printer dialog working we can change this.
    string printerName = "jasmine";
    
    return printerName;

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
void vtkSivicController::ResetRange( bool useFullRange )
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

        this->plotController->SetWindowLevelRange( lowestPoint, highestPoint, svkPlotGridView::FREQUENCY);
        this->spectraViewWidget->detailedPlotController->SetWindowLevelRange( lowestPoint, highestPoint, svkDetailedPlotView::FREQUENCY);

    }
}


/*
 *  Logic to determine whether to enable widgets. 
 */
void vtkSivicController::EnableWidgets()
{
    if ( model->DataExists("SpectroscopicData") && model->DataExists("AnatomicalData") ) {
        string acquisitionType = model->GetDataObject( "SpectroscopicData" )->
                                GetDcmHeader()->GetStringValue("MRSpectroscopyAcquisitionType");
        if( acquisitionType == "SINGLE VOXEL" ) {
            this->imageViewWidget->plotGridButton->EnabledOff();
        } else {
            this->imageViewWidget->plotGridButton->EnabledOn();
        }
        this->imageViewWidget->volSelButton->EnabledOn();
        this->imageViewWidget->satBandButton->EnabledOn();
        this->imageViewWidget->satBandOutlineButton->EnabledOn();
    }

    if ( model->DataExists("SpectroscopicData") || model->DataExists("AnatomicalData") ) {
        this->imageViewWidget->sliceSlider->EnabledOn();
    }

    if ( model->DataExists("SpectroscopicData") ) {
        string domain = model->GetDataObject( "SpectroscopicData" )->GetDcmHeader()->GetStringValue("SignalDomainColumns");
        int numChannels = svkMrsImageData::SafeDownCast( model->GetDataObject("SpectroscopicData"))->GetDcmHeader()->GetNumberOfCoils();
        if( domain == "TIME" ) {
            this->processingWidget->fftButton->EnabledOn(); 
            this->processingWidget->phaseButton->EnabledOff(); 
            this->processingWidget->combineButton->EnabledOff(); 
        } else {
            this->processingWidget->fftButton->EnabledOff(); 
            this->processingWidget->phaseButton->EnabledOn(); 
            if( numChannels > 1 ) {
                this->processingWidget->combineButton->EnabledOn();
            }
        }
        this->processingWidget->phaseSlider->EnabledOn(); 
        this->spectraViewWidget->xSpecRange->EnabledOn();
        this->spectraViewWidget->ySpecRange->EnabledOn();
        this->processingWidget->phaseAllChannelsButton->EnabledOn(); 
        this->processingWidget->phaseAllVoxelsButton->EnabledOn(); 
        this->spectraViewWidget->unitSelectBox->EnabledOn();
        this->spectraViewWidget->componentSelectBox->EnabledOn();
        if( numChannels > 1 ) {
            this->processingWidget->channelSlider->EnabledOn();
        }
    }

    if ( model->DataExists("AnatomicalData") ) {
        this->imageViewWidget->imageSlider->EnabledOn();
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

void vtkSivicController::DisableWidgets()
{
    this->imageViewWidget->volSelButton->EnabledOff();
    this->imageViewWidget->plotGridButton->EnabledOff();

    this->spectraViewWidget->detailedPlotButton->EnabledOff();
    this->imageViewWidget->sliceSlider->EnabledOff();
    this->processingWidget->channelSlider->EnabledOff();
    this->imageViewWidget->imageSlider->EnabledOff();
    this->imageViewWidget->orthoXSlider->EnabledOff();
    this->imageViewWidget->orthoYSlider->EnabledOff();
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

