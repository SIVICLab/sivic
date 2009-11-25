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
#include <sivicViewRenderingWidget.h>
#include <svkDataModel.h>
#include <svkLookupTable.h>
#include <svkDataValidator.h>
#include <sivicProcessingWidget.h>

#include <sivicTestSuite.h>
#include <sivicImageViewWidget.h>
#include <sivicSpectraViewWidget.h>

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
        //void                       SetView( svkInspectingWidget* );
        void                       SetViewRenderingWidget( sivicViewRenderingWidget* viewRenderingWidget);
        void                       SetProcessingWidget( sivicProcessingWidget* processingWidget );
        void                       SetImageViewWidget( sivicImageViewWidget* imageViewWidget );
        void                       SetSpectraViewWidget( sivicSpectraViewWidget* spectraViewWidget );
        void                       SetSlice( int slice );
        void                       SetModel( svkDataModel* ); 
        void                       OpenExam( );
        void                       OpenFile( char* openType, const char* startPath);
        void                       OpenImage(   const char* fileName );
        void                       OpenSpectra( const char* fileName );
        void                       OpenOverlay( const char* fileName );
        void                       SaveData();    
        void                       SaveData( char* fileName );    
        void                       SaveSecondaryCapture();    
        string                     GetUserName();
        string                     GetOsiriXInDir(); 
        void                       SaveSecondaryCaptureOsiriX();    
        void                       SaveDataOsiriX(); 
        void                       SaveSecondaryCapture( char* fileName, int seriesNumber, 
                                                         int outputOption = 0, bool print = 0 );
        void                       ToggleColorsForPrinting( bool colorSchema );
        void                       ResetApplication();    
        void                       UseWindowLevelStyle();
        void                       UseColorOverlayStyle();
        void                       UseSelectionStyle();
        void                       UseRotationStyle();
        void                       ResetWindowLevel();
        void                       HighlightSelectionBoxVoxels();
        void                       DisplayReleaseNotes();
        void                       RunTestingSuite();
        void                       SetSpecUnitsCallback( int targetUnits );
        void                       SetComponentCallback( int targetComponent );
        void                       SetInterpolationCallback( int interpolationType );
        void                       SetLUTCallback( int type );
        void                       Print( int outputOption = 0);
        void                       PopupMessage( string message );
        void                       SaveSession( );
        void                       RestoreSession( );
        void                       ResetRange( bool useFullRange = 0);
        string                     GetPrinterName( );

        //svkInspectingWidget*       GetView();
        svkDataModel*              GetModel();
        svkOverlayViewController*  GetOverlayController(); 
        svkPlotGridViewController* GetPlotController(); 

        vtkKWFileBrowserDialog*    myFileBrowser;
        void                       EnableWidgets();
        void                       DisableWidgets();

 
    protected:

        vtkRenderWindow*            myRenderWindow;
        vtkRenderWindowInteractor*  myRenderWindowInteractor;
        //svkInspectingWidget*        inspectingWidget;
         
    private:
    
    
        svkDataModel*              model; 
        vtkKWApplication*          app;
        vtkImageData*              spectraData;
        sivicViewRenderingWidget*    viewRenderingWidget;
        sivicProcessingWidget*       processingWidget;
        sivicImageViewWidget*      imageViewWidget;
        sivicSpectraViewWidget*    spectraViewWidget;
        svkPlotGridViewController* plotController;
        svkOverlayViewController*  overlayController;

};

#endif //VTK_SIVIC_CONTROLLER_H
