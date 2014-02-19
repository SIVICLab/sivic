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

#ifndef SIVIC_SPECTRA_RANGE_WIDGET_H 
#define SIVIC_SPECTRA_RANGE_WIDGET_H 

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
#include <vtkKWFrameWithLabel.h>
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
#include <svkUtils.h>
#include <sivicKWCompositeWidget.h>

#include <string.h>

using namespace svk; 

class sivicSpectraRangeWidget : public sivicKWCompositeWidget
{

    friend class vtkSivicController;

    public:

        static sivicSpectraRangeWidget *New();
        vtkTypeRevisionMacro(sivicSpectraRangeWidget,sivicKWCompositeWidget);

        void    SetSyncOverlayWL( bool syncOverlayWL );
        void    SetSpecUnitsCallback(svkSpecPoint::UnitType targetUnits);
        void    ResetRange( bool useFullFrequencyRange = 0, bool useFullAmplitudeRange = 0,
                            bool resetAmplitude = 1, bool resetFrequency = 1);
        void    ResetAmplitudeWholeRange( );
        void    ResetAmplitudeRange( bool useFullRange = 0);
        void    ResetFrequencyWholeRange( );
        void    ResetFrequencyRange( bool useFullRange = 0);
        float   ConvertPosUnits(float position, int inType, int targetType);
        
        sivicSpectraRangeWidget();
        ~sivicSpectraRangeWidget();

    protected:


        vtkKWRange*                     xSpecRange;
        vtkKWRange*                     ySpecRange;
        vtkKWMenuButton*                unitSelectBox;
        vtkKWMenuButton*                componentSelectBox;
        vtkKWFrame*                     specViewFrame;
        vtkKWFrame*                     specRangeFrame;

        svkSpecPoint::UnitType          specUnits;
        bool                            centerImage;

        
        // Description:
        // Create the widget.
        virtual void    CreateWidget();
        virtual void    ProcessCallbackCommandEvents( vtkObject*, unsigned long, void* );

    private:

        //! Holds the current full data range. Updated at ResetAmplitudeWholeRange 
        double                      dataRange[2];
        svkSpecPoint*               point;

        sivicSpectraRangeWidget(const sivicSpectraRangeWidget&);   // Not implemented.
        void operator=(const sivicSpectraRangeWidget&);  // Not implemented.
        
};

#endif //SIVIC_SPECTRA_RANGE_WIDGET_H 
