/*
 *  Copyright © 2009-2013 The Regents of the University of California.
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

#ifndef SVK_WINDOW_LEVEL_WIDGET_H 
#define SVK_WINDOW_LEVEL_WIDGET_H 

#include <vtkKWIcon.h>
#include <vtkKWScale.h>
#include <vtkKWScaleWithEntry.h>
#include <vtkKWScaleWithEntrySet.h>
#include <vtkKWScaleWithEntrySetWithLabel.h>
#include <vtkObjectFactory.h>
#include <vtkKWTkUtilities.h>
#include <vtkKWRenderWidget.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkCollectionIterator.h>
#include <vtkCommand.h>
#include <vtkCamera.h>
#include <vtkActorCollection.h>
#include <vtkKWApplication.h>
#include <vtkKWTopLevel.h>
#include <vtkKWWindowBase.h>
#include <vtkKWEntry.h>
#include <vtkKWEntryWithLabel.h>
#include <vtkKWEntrySet.h>
#include <vtkKWWindow.h>
#include <vtkKWRange.h>
#include <vtkKWLabel.h>
#include <vtkKWRadioButton.h>
#include <vtkKWMenuButton.h>
#include <vtkKWMenuButtonWithLabel.h>
#include <vtkKWMenuButtonWithSpinButtonsWithLabel.h>
#include <vtkKWMenuButtonWithSpinButtons.h>
#include <vtkKWMenu.h>
#include <vtkKWMenuButton.h>
#include <vtkKWPushButton.h>
#include <vtkWindowToImageFilter.h>
#include <vtkImageWriter.h>
#include <vtkKWSeparator.h>
#include <vtkJPEGWriter.h>
#include <vtkTIFFWriter.h>
#include <vtkKWProgressGauge.h>
#include <vtkImageAppend.h>
#include <vtkImageConstantPad.h>
#include <vtkImageWrapPad.h>
#include <vtkImageMirrorPad.h>

#include <svkHSVD.h>
#include <svkDataView.h>
#include <svkDataModel.h>
#include <svkImageWriterFactory.h>
#include <svkPlotGridViewController.h>
#include <svkOverlayViewController.h>
#include <svkPlotGridView.h>
#include <svkImageReaderFactory.h>
#include <svkPhaseSpec.h>
#include <svkSpecPoint.h>
#include <sivicKWCompositeWidget.h>

#include <string.h>

using namespace svk; 

class sivicWindowLevelWidget : public sivicKWCompositeWidget
{

    friend class vtkSivicController;

    public:

        static sivicWindowLevelWidget *New();
        vtkTypeRevisionMacro(sivicWindowLevelWidget,sivicKWCompositeWidget);

        void   SetWindow( double window );
        void   SetLevel(  double level );
        void   SetWindowRange( double windowMin, double windowMax );
        void   SetWindowRange( double* range );
        void   SetLevelRange( double levelMin, double levelMax );
        void   SetLevelRange( double* range );
        void   EnableUpdate();
        void   DisableUpdate();
        void   SetWindowLevelTarget(svkOverlayViewController::WindowLevelTarget target);
        void   SetOverlayDataName( string overlayDataName );
        void   SetSliderLabel( string sliderTitle);
        string GetOverlayDataName( );
        void   SetSyncPlotGrid( bool syncPlotGrid );



    protected:

        sivicWindowLevelWidget();
        ~sivicWindowLevelWidget();

        svkOverlayViewController::WindowLevelTarget target;
        void                                        UpdateSliders();
        void                                        UpdateView();
        double                                      window;
        double                                      windowRange[2];
        double                                      level;
        double                                      levelRange[2];
        string                                      overlayDataName;
        bool                                        syncPlotGrid;

        vtkKWScaleWithEntry*                        levelSlider;
        vtkKWScaleWithEntry*                        windowSlider;
        vtkKWScaleWithEntry*                        minSlider;
        vtkKWScaleWithEntry*                        maxSlider;
        vtkKWScaleWithEntrySetWithLabel*            imageSliders;
        vtkKWMenuButtonWithLabel*                   sliderType;
        string                                      sliderLabel;

        
        // Description:
        // Create the widget.
        virtual void    CreateWidget();
        virtual void    ProcessCallbackCommandEvents( vtkObject*, unsigned long, void* );



    private:

        bool updateEnabled;
        bool syncOverlayWL;

        sivicWindowLevelWidget(const sivicWindowLevelWidget&);   // Not implemented.
        void operator=(const sivicWindowLevelWidget&);  // Not implemented.
        
};

#endif //SVK_WINDOW_LEVEL_WIDGET_H 
