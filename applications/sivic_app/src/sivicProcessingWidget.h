/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */

#ifndef SVK_PROCESSING_WIDGET_H 
#define SVK_PROCESSING_WIDGET_H 

#include <vtkKWCompositeWidget.h>
#include <vtkObjectFactory.h>
#include <vtkKWTkUtilities.h>
#include <vtkKWScaleWithEntry.h>
#include <vtkKWCheckButton.h>
#include <vtkKWPushButton.h>

#include <svkDataModel.h>
#include <svkPhaseSpec.h>
#include <svkMrsImageFFT.h>
#include <svkCoilCombine.h>
#include <svkMultiCoilPhase.h>
#include <svkPlotGridViewController.h>
#include <svkOverlayViewController.h>
#include <sivicKWCompositeWidget.h>

#include <string.h>

using namespace svk; 

class sivicProcessingWidget : public sivicKWCompositeWidget
{

    friend class vtkSivicController;

    public:

        static sivicProcessingWidget *New();
        vtkTypeRevisionMacro(sivicProcessingWidget,sivicKWCompositeWidget);


    protected:


        sivicProcessingWidget();
        ~sivicProcessingWidget();

        vtkKWCheckButton*               phaseAllVoxelsButton;
        vtkKWCheckButton*               phaseAllChannelsButton;
        vtkKWScaleWithEntry*            channelSlider;
        vtkKWScaleWithEntry*            phaseSlider;
        vtkKWPushButton*                fftButton;
        vtkKWPushButton*                phaseButton;
        vtkKWPushButton*                combineButton;

        
        // Description:
        // Create the widget.
        virtual void    CreateWidget();
        virtual void    ProcessCallbackCommandEvents( vtkObject*, unsigned long, void* );
        void            ResetRange();



    private:

        svkPhaseSpec*               phaser;
        void                        SetPhaseUpdateExtent();
        void                        UpdatePhaseSliderBindings();
        bool                        phaseChangeInProgress;
        void                        ExecuteFFT();
        void                        ExecutePhase();
        void                        ExecuteCombine();



        sivicProcessingWidget(const sivicProcessingWidget&);   // Not implemented.
        void operator=(const sivicProcessingWidget&);  // Not implemented.
        
};

#endif //SVK_PROCESSING_WIDGET_H 
