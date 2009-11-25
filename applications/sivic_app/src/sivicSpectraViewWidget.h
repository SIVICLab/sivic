/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */

#ifndef SIVIC_SPECTRA_VIEW_WIDGET_H 
#define SIVIC_SPECTRA_VIEW_WIDGET_H 

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

class sivicSpectraViewWidget : public sivicKWCompositeWidget
{

    friend class vtkSivicController;

    public:

        static sivicSpectraViewWidget *New();
        vtkTypeRevisionMacro(sivicSpectraViewWidget,sivicKWCompositeWidget);
        
    protected:

        sivicSpectraViewWidget();
        ~sivicSpectraViewWidget();

        vtkKWRenderWidget*              detailedPlotWidget;
        vtkKWRange*                     xSpecRange;
        vtkKWRange*                     ySpecRange;
        vtkKWMenuButtonWithLabel*       unitSelectBox;
        vtkKWMenuButtonWithLabel*       componentSelectBox;
        vtkKWFrame*                     specViewFrame;
        vtkKWPushButton*                detailedPlotButton;
        vtkKWWindowBase*                detailedPlotWindow;

        int                             specUnits;

        
        // Description:
        // Create the widget.
        virtual void    CreateWidget();
        virtual void    ProcessCallbackCommandEvents( vtkObject*, unsigned long, void* );



    private:

        svkDetailedPlotViewController*  detailedPlotController;
        svkSpecPoint*               point;

        vtkTextActor*               it;

        const char*             GetMetaboliteName( string fileName );

        sivicSpectraViewWidget(const sivicSpectraViewWidget&);   // Not implemented.
        void operator=(const sivicSpectraViewWidget&);  // Not implemented.
        
};

#endif //SIVIC_SPECTRA_VIEW_WIDGET_H 
