/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */

#ifndef SVK_IMAGE_VIEW_WIDGET_H 
#define SVK_IMAGE_VIEW_WIDGET_H 

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

class sivicImageViewWidget : public sivicKWCompositeWidget
{

    friend class vtkSivicController;

    public:

        static sivicImageViewWidget *New();
        vtkTypeRevisionMacro(sivicImageViewWidget,sivicKWCompositeWidget);
        
        enum {
            ALL_SLICES = 0,
            CURRENT_SLICE
        } OutputOption;

        enum {
            LIGHT_ON_DARK = 0,
            DARK_ON_LIGHT 
        } ColorSchema;



    protected:

        sivicImageViewWidget();
        ~sivicImageViewWidget();

        vtkKWScaleWithEntry*            sliceSlider;
        vtkKWScaleWithEntry*            orthoXSlider;
        vtkKWScaleWithEntry*            orthoYSlider;
        vtkKWScaleWithEntry*            imageSlider;
        vtkKWScaleWithEntry*            overlayOpacitySlider;
        vtkKWScaleWithEntry*            overlayThresholdSlider;
        vtkKWCheckButton*               satBandButton;
        vtkKWCheckButton*               satBandOutlineButton;
        vtkKWCheckButton*               volSelButton;
        vtkKWCheckButton*               plotGridButton;
        vtkKWCheckButton*               overlayButton;
        vtkKWCheckButton*               colorBarButton;
        vtkKWCheckButton*               orthImagesButton;
        vtkKWProgressGauge*             progressGauge;
        vtkKWLabel*                     loadingLabel;
        vtkKWMenuButtonWithLabel*       interpolationBox;
        vtkKWMenuButtonWithLabel*       lutBox;
        vtkKWFrame*                     imageViewFrame;
        vtkKWFrame*                     orthoViewFrame;
        vtkKWFrame*                     overlayViewFrame;


        
        // Description:
        // Create the widget.
        virtual void    CreateWidget();
        virtual void    ProcessCallbackCommandEvents( vtkObject*, unsigned long, void* );



    private:

        vtkTextActor*               it;

        sivicImageViewWidget(const sivicImageViewWidget&);   // Not implemented.
        void operator=(const sivicImageViewWidget&);  // Not implemented.
        
};

#endif //SVK_IMAGE_VIEW_WIDGET_H 
