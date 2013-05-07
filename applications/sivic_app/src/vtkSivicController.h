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

#ifndef VTK_SIVIC_CONTROLLER_H
#define VTK_SIVIC_CONTROLLER_H

#ifdef WIN32
    #include <windows.h>
#endif

#include <vtkObject.h>
#include <vtkKWObject.h>
#include <vector>
#include <vtkCommand.h>
#include <vtkObjectFactory.h>
#include <vtkImageViewer2.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkKWFileBrowserDialog.h>
#include <vtkKWLoadSaveDialog.h>
#include <string>
#include <vtkSivicController.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderLargeImage.h>
#include <vtkDataSetCollection.h>
#include <vtkKWRenderWidget.h>
#include <vtkKWApplication.h>
#include <vtkKWWindowBase.h>
#include <vtkImageFlip.h>
#include <vtkKWMessageDialog.h>
#include <vtkKWTextWithScrollbars.h>
#include <vtkKWTextWithScrollbarsWithLabel.h>
#include <vtkKWText.h>
#include <svkUCSFPACSInterface.h>
#include <svkPACSInterface.h>

#include <math.h>
#ifndef WIN32
    #include <sys/stat.h>
    #include <sys/param.h>
    #include <unistd.h>
    #include <pwd.h>
#endif
#include <stdlib.h>
#include <stdio.h>

#include <svkUtils.h>
#include <svkVoxelTaggingUtils.h>
#include <svkDataViewController.h>
#include <svkSecondaryCaptureFormatter.h>
#include <svkImageWriterFactory.h> 
#include <sivicViewRenderingWidget.h>
#include <svkDataModel.h>
#include <svkLookupTable.h>
#include <svkDataValidator.h>
#include <vtkDataObjectCollection.h>
#include <sivicProcessingWidget.h>
#include <sivicPreprocessingWidget.h>
#include <sivicQuantificationWidget.h>
#include <sivicCombineWidget.h>
#include <sivicDSCWidget.h>

#include <sivicTestSuite.h>
#include <sivicImageViewWidget.h>
#include <sivicSpectraViewWidget.h>
#include <sivicWindowLevelWidget.h>
#include <svkSpecPoint.h>

#include <sivicPreferencesWidget.h>
#include <sivicVoxelTaggingWidget.h>
#include <sivicSpectraRangeWidget.h>
#include <sivicDataWidget.h>
#include <sivicImageDataWidget.h>
#include <vtkDirectory.h>
#include <vtkKWLogDialog.h>
#include <vtkKWSimpleEntryDialog.h>
#if defined( SVK_USE_GL2PS )
    #include <vtkGL2PSExporter.h>
#endif

// Used for determining the default range, as a percentage of whole range
// for plotting spectra.
#define NEG_RANGE_SCALE .17
#define POS_RANGE_SCALE .70
#define PPM_1H_DEFAULT_MIN 3.844 
#define PPM_1H_DEFAULT_MAX 0.602 
#define PPM_13C_DEFAULT_MIN 250 
#define PPM_13C_DEFAULT_MAX -50 
#define SLIDER_RELATIVE_RESOLUTION 0.002 

using namespace svk;
using std::vector;


class vtkSivicController : public vtkObject
{
    public:

        // wrapper doesn't permit enums, so... 
        static const int ANATOMY_BRAIN;
        static const int ANATOMY_PROSTATE;

   
        vtkTypeRevisionMacro( vtkSivicController, vtkObject );
        static vtkSivicController* New(); 

        vtkSivicController();
        ~vtkSivicController();


        void                       SetApplication( vtkKWApplication* app );
        vtkKWApplication*          GetApplication( );
        void                       SetMainWindow( vtkKWWindowBase* mainWindow );
        //void                       SetView( svkInspectingWidget* );
        void                       SetViewRenderingWidget( sivicViewRenderingWidget* viewRenderingWidget);
        void                       SetProcessingWidget( sivicProcessingWidget* processingWidget );
        void                       SetPreprocessingWidget( sivicPreprocessingWidget* preprocessingWidget );
        void                       SetDataWidget( sivicDataWidget* dataWidget );
        void                       SetImageDataWidget( sivicImageDataWidget* imageDataWidget );
        void                       SetQuantificationWidget( sivicQuantificationWidget* quantificationWidget );
        void                       SetCombineWidget( sivicCombineWidget* CombineWidget );
        void                       SetDSCWidget( sivicDSCWidget* dscWidget );
        void                       SetImageViewWidget( sivicImageViewWidget* imageViewWidget );
        void                       SetSpectraRangeWidget( sivicSpectraRangeWidget* spectraRangeWidget );
        void                       SetSpectraViewWidget( sivicSpectraViewWidget* spectraViewWidget );
        void                       SetWindowLevelWidget( sivicWindowLevelWidget* windowLevelWidget );
        void                       SetOverlayWindowLevelWidget( sivicWindowLevelWidget* overlayWindowLevelWidget );
        void                       SetPreferencesWidget( sivicPreferencesWidget* preferencesWidget );
        void                       SetVoxelTaggingWidget( sivicVoxelTaggingWidget* voxelTaggingWidget );
        void                       SetSlice( int slice, bool centerImage = true );
        void                       SetImageSlice( int slice, string orientation );
        void                       SetModel( svkDataModel* ); 
        void                       OpenExam( );
        int                        OpenFile( char* openType, const char* startPath, bool resetBeforeLoad = 0, bool onlyReadOneInputFile = false);
		void                       OpenImage( svkImageData* data, string stringFilename );
        void                       OpenImage(   const char* fileName, bool onlyReadOneInputFile = false );
        void                       OpenImageFromModel( const char* modelObjectName );
        void                       UpdateModelForReslicedImage(string modelObjectName);
        void                       Open4DImage( svkImageData* newData,  string stringFilename, svkImageData* oldData = NULL );
        void                       Open4DImage( const char* fileName, bool onlyReadOneInputFile = false );
        void                       Open4DImageFromModel( const char* modelObjectName );
        void                       Add4DImageData( string stringFilename, bool onlyReadOneInputFile = false );
        void                       Add4DImageData( svkImageData* data, string stringFilename );
        void                       Add4DImageDataFromModel(const char* modelObjectName );
        void                       OpenOverlayFromModel( const char* modelObjectName );
        void                       OpenOverlay( svkImageData* data, string stringFilename );
        void                       OpenOverlay( const char* fileName, bool onlyReadOneInputFile = false );
        void                       OpenMetabolites( const char* metabolites );
        void                       SetPreferencesFromRegistry( );
        void                       SaveData();    
        void                       SaveData( char* fileName );    
        void                       SaveMetaboliteMaps();    
        void                       SaveImageFromModel(const char* modelObjectName);
        void                       SaveMetMapData( svkImageData* image, char* fileName, 
                                        int writetType = 5); //svkImageWriterFactory::DICOM_MRI 
        void                       SaveSecondaryCapture( char* captureType );    
        string                     GetOsiriXInDir(); 
        void                       SaveSecondaryCaptureOsiriX();    
        void                       SaveMetMapDataOsiriX(); 
        void                       SaveDataOsiriX(); 
        void                       SaveSecondaryCapture( char* fileName, int seriesNumber, char* captureType,
                                                         int outputOption = 0, bool print = 0 );
        void                       WriteCombinedCapture( vtkImageWriter* writer, string fileNameString, 
                                                         int outputOption, svkImageData* outputImage, bool print );
        void                       WriteSpectraCapture( vtkImageWriter* writer, string fileNameString, 
                                                        int outputOption, svkImageData* outputImage, bool print );
#if defined( SVK_USE_GL2PS )
        void                       ExportSpectraCapture( string fileNameString, int outputOption, string type );
#endif
        void                       WriteImageCapture( vtkImageWriter* writer, string fileNameString, int outputOption, 
                                                      svkImageData* outputImage, bool print, int instanceNumber = 0 );
        void                       ToggleColorsForPrinting( bool colorSchema );
        void                       ResetApplication();    
        void                       UseWindowLevelStyle();
        void                       UseColorOverlayStyle();
        void                       UseSelectionStyle();
        void                       UseRotationStyle();
        void                       ResetWindowLevel();
        void                       HighlightSelectionBoxVoxels();
        void                       DisplayInfo();
        void                       DisplayWindowLevelWindow();
        void                       DisplayPreferencesWindow();
        void                       DisplayVoxelTaggingWindow();
        void                       RunTestingSuite();
        void                       SetSpecUnitsCallback( int targetUnits );
        void                       SetComponentCallback( int targetComponent );
        void                       SetInterpolationCallback( int interpolationType );
        void                       SetLUTCallback( int type );
        void                       MetMapViewCallback(int mapNumber); 
        void                       DSCMapViewCallback(int mapNumber); 
        void                       SetDSCRepresentationCallback(int representation); 
        void                       Print( char* captureType, int outputOption = 0);
        int                        PopupMessage( string message, int style = 0 );
        void                       SaveSession( );
        void                       RestoreSession( );
        void                       ResetRange( bool useFullFrequencyRange = 0, bool useFullAmplitudeRange = 0,
                                               bool resetAmplitude = 1, bool resetFrequency = 1);

        void                       ResetChannel( );
        string                     GetPrinterName( );
        void                       SetOrientation( const char*, bool alignOverlay = 0 );
        void                       TurnOffPlotView();
        void                       TurnOnPlotView();
        void                       SetActive4DImageData( int index );
        svk4DImageData*            GetActive4DImageData();
        void                       SyncDisplayVolumes(svkImageData* data, int volume, int volumeIndex = -1 );

        //svkInspectingWidget*       GetView();
        svkDataModel*              GetModel();
        svkOverlayViewController*  GetOverlayController(); 
        svkPlotGridViewController* GetPlotController(); 

        vtkKWFileBrowserDialog*    myFileBrowser;
        void                       EnableWidgets();
        void                       DisableWidgets();
        void                       UpdateThreshold( );
        void                       SetThresholdType( string thresholdType );
        string                     GetThresholdType( );
        int                        GetFrequencyType( );
        void                       SetThresholdTypeToPercent();
        void                       SetThresholdTypeToQuantity();
        void                       SetOverlayThreshold( double threshold );
        void                       PushToPACS();
        void                       GetMRSDefaultPPMRange( svkImageData* mrsData, float& ppmMin, float& ppmMax ); 
        int                        GetDraw();
        void                       DrawOff();
        void					   DrawOn();
        void					   OverlayTextOn();
        void					   OverlayTextOff();
        void					   GenerateTraces( char* sourceImage );
        void					   DisplayImageDataInfo(int row, int column, int x, int y);
        void					   DisplayHeader( char* objectName );
 
    protected:

        vtkRenderWindow*            myRenderWindow;
        vtkRenderWindowInteractor*  myRenderWindowInteractor;
        //svkInspectingWidget*        inspectingWidget;
         
    private:
        static void                    UpdateProgress(vtkObject* subject, unsigned long, void* thisObject, void* callData);
        static void                    ExitSivic(vtkObject* subject, unsigned long, void* thisObject, void* callData);
        void                           DeselectMetabolites( ); 
        vtkCallbackCommand*            progressCallback;
        vtkCallbackCommand*            exitSivicCallback;
        string                         thresholdType;
        string                         orientation; 
        svkDataModel*                  model; 
        vtkKWApplication*              app;
        vtkKWWindowBase*               mainWindow;
        vtkImageData*                  spectraData;
        sivicViewRenderingWidget*      viewRenderingWidget;
        sivicProcessingWidget*         processingWidget;
        sivicPreprocessingWidget*      preprocessingWidget;
        sivicDataWidget*               dataWidget;
        sivicImageDataWidget*          imageDataWidget;
        sivicQuantificationWidget*     quantificationWidget;
        sivicCombineWidget*            combineWidget;
        sivicDSCWidget*                dscWidget;
        sivicImageViewWidget*          imageViewWidget;
        sivicSpectraRangeWidget*       spectraRangeWidget;
        sivicSpectraViewWidget*        spectraViewWidget;
        sivicWindowLevelWidget*        windowLevelWidget;
        sivicWindowLevelWidget*        overlayWindowLevelWidget;
        sivicPreferencesWidget*        preferencesWidget;
        sivicVoxelTaggingWidget*       voxelTaggingWidget;
        svkPlotGridViewController*     plotController;
        svkOverlayViewController*      overlayController;
        svkSecondaryCaptureFormatter*  secondaryCaptureFormatter;
        vtkKWWindowBase*               windowLevelWindow;
        vtkKWWindowBase*               preferencesWindow;
        vtkKWWindowBase*               voxelTaggingWindow;
        bool						   synchronizeVolumes;


};

#endif //VTK_SIVIC_CONTROLLER_H
