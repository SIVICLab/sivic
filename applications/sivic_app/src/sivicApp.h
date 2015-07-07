/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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
#include <vtkKWPushButtonWithMenu.h>
#include <vtkKWRadioButtonSet.h>
#include <vtkKWUserInterfacePanel.h>
#include <vtkKWTextWithScrollbars.h>
#include <vtkKWText.h>
#include <vtkKWIcon.h>
#include <vtkKWNotebook.h>
#include <vtkPNGReader.h>
#include <vtkJPEGReader.h>

#include <svkHSVD.h>
#include <svkOverlayViewController.h>
#include <vtkSivicController.h>
#include <sivicViewRenderingWidget.h>
#include <sivicImageViewWidget.h>
#include <sivicSpectraViewWidget.h>
#include <sivicWindowLevelWidget.h>
#include <sivicPreferencesWidget.h>
#include <sivicVoxelTaggingWidget.h>
#include <sivicSpectraRangeWidget.h>
#include <sivicDataWidget.h>
#include <sivicImageDataWidget.h>

#include <vtksys/SystemTools.hxx>
#include <vtksys/CommandLineArguments.hxx>

#define WINDOW_SIZE_X 1000
#define WINDOW_SIZE_Y 760

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

        vtkSivicController*        sivicController;
        sivicViewRenderingWidget*  viewRenderingWidget;
        sivicPreprocessingWidget*  preprocessingWidget;
        sivicPostprocessingWidget* postprocessingWidget;
        sivicDataWidget*           dataWidget;
        sivicImageDataWidget*      imageDataWidget;
        sivicProcessingWidget*     processingWidget;
        sivicPhaseWidget*          phaseWidget;
        sivicQuantificationWidget* quantificationWidget;
        sivicCombineWidget*        combineWidget;
        sivicDSCWidget*            dscWidget;
        sivicImageViewWidget*      imageViewWidget;
        sivicSpectraViewWidget*    spectraViewWidget;
        sivicWindowLevelWidget*    windowLevelWidget;
        sivicWindowLevelWidget*    overlayWindowLevelWidget;
        sivicPreferencesWidget*    preferencesWidget;
        sivicVoxelTaggingWidget*   voxelTaggingWidget;
        sivicSpectraRangeWidget*   spectraRangeWidget;
        vtkKWUserInterfacePanel*   uiPanel;
        vtkKWNotebook*             tabbedPanel;
        svkDataModel*              model;
        vtkKWApplication*          sivicKWApp;

        // Methods
        void    PopulateMainToolbar( vtkKWToolbar* toolbar );
        void    GetWelcomeMessage( vtkKWText* text );
        char**  ParseCommandLineArgs( int* argc, char* argv[] ); 
        
        vector< string >    inputFiles; 
        vector< bool >      inputFilesOnlyLoadOne; 
        vector< bool >      inputFilesDynamic; 
        
};

#endif //SIVIC_BUILDER_H 
