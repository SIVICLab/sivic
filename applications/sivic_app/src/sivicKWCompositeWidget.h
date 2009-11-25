/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */

#ifndef SVK_MVCKW_COMPOSITE_WIDGET_H
#define SVK_MVCKW_COMPOSITE_WIDGET_H

#include <vtkKWCompositeWidget.h>
#include <vtkObjectFactory.h>
#include <vtkKWTkUtilities.h>

#include <svkDataModel.h>
#include <svkPlotGridViewController.h>
#include <svkOverlayViewController.h>

#include <string.h>

using namespace svk; 

class vtkSivicController;


class sivicKWCompositeWidget : public vtkKWCompositeWidget
{

    friend class vtkSivicController;

    public:

        static sivicKWCompositeWidget *New();
        vtkTypeRevisionMacro(sivicKWCompositeWidget,vtkKWCompositeWidget);
        void SetSivicController( vtkSivicController* );
        void SetPlotController( svkPlotGridViewController* plotController );
        void SetOverlayController( svkOverlayViewController* overlayController );
        void SetModel( svkDataModel* );    


    protected:


        sivicKWCompositeWidget();
        ~sivicKWCompositeWidget();

        vtkSivicController*    sivicController;
        svkPlotGridViewController*  plotController;
        svkOverlayViewController*   overlayController;
        svkDataModel*               model;

};

#endif //SVK_MVCKW_COMPOSITE_WIDGET_H
