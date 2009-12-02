/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */


#ifndef SIVIC_BUILDER_H 
#define SIVIC_BUILDER_H 

#include <vtkKWApplication.h>
#include <vtkKWWindowBase.h>
#include <vtkKWFrame.h>
#include <vtkKWWindowBase.h>
#include <vtkKWMenu.h>
#include <svkDataModel.h>
#include <vtkKWLabel.h>
#include <vtkKWToolbar.h>
#include <vtkKWToolbarSet.h>
#include <vtkKWPushButton.h>
#include <vtkKWRadioButtonSet.h>
#include <vtkKWUserInterfacePanel.h>
#include <vtkKWIcon.h>
#include <vtkKWNotebook.h>
#include <vtkPNGReader.h>
#include <vtkJPEGReader.h>

#include <svkOverlayViewController.h>
#include <vtkSivicController.h>
#include <sivicViewRenderingWidget.h>
#include <sivicImageViewWidget.h>
#include <sivicSpectraViewWidget.h>

#include <vtksys/SystemTools.hxx>
#include <vtksys/CommandLineArguments.hxx>

#define MAJOR_VERSION 0
#define MINOR_VERSION 4
#define WINDOW_SIZE_X 900
#define WINDOW_SIZE_Y 700

extern "C" int Sivickwcallbackslib_Init(Tcl_Interp *interp);



/*! 
 *  The purpose of this class is to build and start 
 *  all of the components of the application, sivic.
 */ 

class sivicApp
{
    public:
        
        sivicApp();
        ~sivicApp();

        int Start( int argc, char* argv[] );
        int GetExitStatus();
        int Build( int argc, char* argv[] );
        vtkSivicController* GetView();

    private:

        // Members
        int exitStatus;
        vtkKWWindowBase*             sivicWindow;

        vtkSivicController* sivicController;
        sivicViewRenderingWidget*  viewRenderingWidget;
        sivicProcessingWidget*     processingWidget;
        sivicImageViewWidget*    imageViewWidget;
        sivicSpectraViewWidget*  spectraViewWidget;
        vtkKWUserInterfacePanel* uiPanel;
        vtkKWNotebook*           tabbedPanel;
        svkDataModel*            model;
        vtkKWApplication*        sivicKWApp;

        // Methods
        void PopulateMainToolbar( vtkKWToolbar* toolbar );
        
};

#endif //SIVIC_BUILDER_H 
