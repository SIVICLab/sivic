/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */

#ifndef SVK_VIEW_RENDERING_WIDGET 
#define SVK_VIEW_RENDERING_WIDGET 

#include <vtkObjectFactory.h>
#include <vtkKWTkUtilities.h>
#include <vtkKWRenderWidget.h>
#include <vtkRenderWindowInteractor.h>

#include <svkDataModel.h>
#include <svkPlotGridViewController.h>
#include <svkOverlayViewController.h>
#include <sivicKWCompositeWidget.h>

#include <string.h>

using namespace svk; 

class sivicViewRenderingWidget : public sivicKWCompositeWidget
{

    friend class vtkSivicController;

    public:

        static sivicViewRenderingWidget *New();
        vtkTypeRevisionMacro(sivicViewRenderingWidget,sivicKWCompositeWidget);

        void ResetInfoText( );

    protected:

        sivicViewRenderingWidget();
        ~sivicViewRenderingWidget();

        vtkKWRenderWidget*              titleWidget;
        vtkKWRenderWidget*              infoWidget;
        vtkKWRenderWidget*              viewerWidget;
        vtkKWRenderWidget*              specViewerWidget;
        int                             specUnits;

        // Description:
        // Create the widget.
        virtual void    CreateWidget();
        virtual void    ProcessCallbackCommandEvents( vtkObject*, unsigned long, void* );



    private:

        vtkTextActor*               it;
        const char*                 GetMetaboliteName( string fileName );

        sivicViewRenderingWidget(const sivicViewRenderingWidget&);   // Not implemented.
        void operator=(const sivicViewRenderingWidget&);  // Not implemented.
        
};

#endif //SVK_VIEW_RENDERING_WIDGET 
