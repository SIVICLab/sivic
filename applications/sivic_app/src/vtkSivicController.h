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

#ifndef VTK_SIVIC_CONTROLLER_H
#define VTK_SIVIC_CONTROLLER_H

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

#include <math.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <unistd.h>    
#include <stdlib.h>
#include <pwd.h>
#include <stdio.h>

#include <svkDataViewController.h>
#include <svkDetailedPlotViewController.h>
#include <svkSecondaryCaptureFormatter.h>
#include <sivicViewRenderingWidget.h>
#include <svkDataModel.h>
#include <svkLookupTable.h>
#include <svkDataValidator.h>
#include <sivicProcessingWidget.h>
#include <sivicQuantificationWidget.h>

#include <sivicTestSuite.h>
#include <sivicImageViewWidget.h>
#include <sivicSpectraViewWidget.h>
#include <sivicWindowLevelWidget.h>
#include <sivicPreferencesWidget.h>
#include <sivicSpectraRangeWidget.h>
#include <sivicGlobalWidget.h>
#if defined( SVK_USE_GL2PS )
    #include <vtkGL2PSExporter.h>
#endif

// Used for determining the default range, as a percentage of whole range
// for plotting spectra.
#define NEG_RANGE_SCALE .17
#define POS_RANGE_SCALE .70
#define PPM_DEFAULT_MIN 3.844 
#define PPM_DEFAULT_MAX 0.602 
#define SLIDER_RELATIVE_RESOLUTION 0.002 

using namespace svk;
using std::vector;


class vtkSivicController : public vtkObject
{
    public:
   
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
        void                       SetQuantificationWidget( sivicQuantificationWidget* quantificationWidget );
        void                       SetImageViewWidget( sivicImageViewWidget* imageViewWidget );
        void                       SetSpectraRangeWidget( sivicSpectraRangeWidget* spectraRangeWidget );
        void                       SetSpectraViewWidget( sivicSpectraViewWidget* spectraViewWidget );
        void                       SetWindowLevelWidget( sivicWindowLevelWidget* windowLevelWidget );
        void                       SetOverlayWindowLevelWidget( sivicWindowLevelWidget* overlayWindowLevelWidget );
        void                       SetPreferencesWidget( sivicPreferencesWidget* preferencesWidget );
        void                       SetGlobalWidget( sivicGlobalWidget* globalWidget );
        void                       SetSlice( int slice, bool centerImage = true );
        void                       SetImageSlice( int slice, string orientation );
        void                       SetModel( svkDataModel* ); 
        void                       OpenExam( );
        int                        OpenFile( char* openType, const char* startPath, bool resetBeforeLoad = 0);
        void                       OpenImage(   const char* fileName );
        void                       OpenSpectra( const char* fileName, bool onlyReadOneInputFile = false );
        void                       OpenOverlay( const char* fileName );
        void                       OpenMetabolites( const char* metabolites );
        void                       SetPreferencesFromRegistry( );
        void                       SaveData();    
        void                       SaveMetMapData( char* fileName );    
        void                       SaveData( char* fileName );    
        void                       SaveSecondaryCapture( char* captureType );    
        string                     GetUserName();
        string                     GetOsiriXInDir(); 
        void                       SaveSecondaryCaptureOsiriX();    
        void                       SaveMetMapDataOsiriX(); 
        void                       SaveDataOsiriX(); 
        void                       SaveSecondaryCapture( char* fileName, int seriesNumber, char* captureType,
                                                         int outputOption = 0, bool print = 0 );
        void                       WriteCombinedCapture( vtkImageWriter* writer, string fileNameString, int outputOption, svkImageData* outputImage, bool print );
        void                       WriteSpectraCapture( vtkImageWriter* writer, string fileNameString, int outputOption, svkImageData* outputImage, bool print );
#if defined( SVK_USE_GL2PS )
        void                       ExportSpectraCapture( string fileNameString, int outputOption, string type );
#endif
        void                       WriteImageCapture( vtkImageWriter* writer, string fileNameString, int outputOption, svkImageData* outputImage, bool print, int instanceNumber = 0 );
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
        void                       RunTestingSuite();
        void                       SetSpecUnitsCallback( int targetUnits );
        void                       SetComponentCallback( int targetComponent );
        void                       SetInterpolationCallback( int interpolationType );
        void                       SetLUTCallback( int type );
        void                       MetMapViewCallback(int mapNumber); 
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

        //svkInspectingWidget*       GetView();
        svkDataModel*              GetModel();
        svkOverlayViewController*  GetOverlayController(); 
        svkPlotGridViewController* GetPlotController(); 
        svkDetailedPlotViewController* GetDetailedPlotController(); 

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

 
    protected:

        vtkRenderWindow*            myRenderWindow;
        vtkRenderWindowInteractor*  myRenderWindowInteractor;
        //svkInspectingWidget*        inspectingWidget;
         
    private:
        static void                    UpdateProgress(vtkObject* subject, unsigned long, void* thisObject, void* callData);
        static void                    ExitSivic(vtkObject* subject, unsigned long, void* thisObject, void* callData);
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
        sivicQuantificationWidget*     quantificationWidget;
        sivicImageViewWidget*          imageViewWidget;
        sivicSpectraRangeWidget*       spectraRangeWidget;
        sivicSpectraViewWidget*        spectraViewWidget;
        sivicWindowLevelWidget*        windowLevelWidget;
        sivicWindowLevelWidget*        overlayWindowLevelWidget;
        sivicPreferencesWidget*        preferencesWidget;
        sivicGlobalWidget*             globalWidget;
        svkPlotGridViewController*     plotController;
        svkOverlayViewController*      overlayController;
        svkDetailedPlotViewController* detailedPlotController;
        svkSecondaryCaptureFormatter*  secondaryCaptureFormatter;
        vtkKWWindowBase*               windowLevelWindow;
        vtkKWWindowBase*               preferencesWindow;


};

#endif //VTK_SIVIC_CONTROLLER_H
