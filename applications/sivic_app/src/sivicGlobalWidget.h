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
 *  $URL: $
 *  $Rev: $
 *  $Author: $
 *  $Date: $
 */

#ifndef SIVIC_GLOBAL_WIDGET_H 
#define SIVIC_GLOBAL_WIDGET_H 

#include <vtkKWScale.h>
#include <vtkKWScaleWithEntry.h>
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
#include <vtkKWComboBoxWithLabel.h>
#include <vtkKWComboBox.h>
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

#include <vector>

#include <svkDataView.h>
#include <svkDataModel.h>
#include <svkImageWriterFactory.h>
#include <svkPlotGridViewController.h>
#include <svkOverlayViewController.h>
#include <svkDetailedPlotViewController.h>
#include <svkPlotGridView.h>
#include <svkImageReaderFactory.h>
#include <svkPhaseSpec.h>
#include <svkSpecPoint.h>
#include <svkMultiWindowToImageFilter.h>
#include <sivicKWCompositeWidget.h>

#include <string.h>

using namespace svk; 

class sivicGlobalWidget : public sivicKWCompositeWidget
{

    friend class vtkSivicController;

    public:

        static sivicGlobalWidget *New();
        vtkTypeRevisionMacro(sivicGlobalWidget,sivicKWCompositeWidget);
        void            PopulateMetaboliteMenu( );
        void            DeselectMetabolites( );
        
    protected:

        sivicGlobalWidget();
        ~sivicGlobalWidget();

        vtkKWMenuButtonWithLabel*    orientationSelect;
        vtkKWMenuButtonWithLabel*    metaboliteSelect;
        vtkKWFrame*                   specViewFrame;   

        
        // Description:
        // Create the widget.
        virtual void    CreateWidget();
        virtual void    ProcessCallbackCommandEvents( vtkObject*, unsigned long, void* );

    private:

        sivicGlobalWidget(const sivicGlobalWidget&);   // Not implemented.
        void operator=(const sivicGlobalWidget&);  // Not implemented.
        
};

#endif //SIVIC_GLOBAL_WIDGET_H 
