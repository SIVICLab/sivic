/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */

#ifndef SVK_APP_GUI_H 
#define SVK_APP_GUI_H 


#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkKWFrame.h>
#include <vtkKWRange.h>
#include <vtkKWEntry.h>

#include <svkSpecPoint.h>
#include <svkImageData.h>

#include <string.h>

#define NEG_RANGE_SCALE .17
#define POS_RANGE_SCALE .70
#define PPM_DEFAULT_MIN 3.844
#define PPM_DEFAULT_MAX 0.602
#define SLIDER_RELATIVE_RESOLUTION 0.002

namespace svk {


using namespace std;


class sivicAppGUI : public vtkObject
{

    public:

        static sivicAppGUI *New();
        vtkTypeRevisionMacro(sivicAppGUI, vtkObject);

        void            CreateMRSWidgets( vtkKWWidget* parent ); 
        void            InitXSpecRangeWidget(); 
        void            InitYSpecRangeWidget( svkImageData::RangeComponent component); 
        void            XSpecRangeChanged(float& lowestPoint, float& highestPoint);
        void            YSpecRangeChanged(double& minValue, double& maxValue);
        void            SetMRSData( svkImageData* mrsData ); 
        svkSpecPoint*   GetSpecPoint();

        
    protected:

        sivicAppGUI();
        ~sivicAppGUI();


    private: 

        vtkKWRange*             xSpecRange;
        vtkKWRange*             ySpecRange;

        svkImageData*           mrsImageData; 
        svkSpecPoint*           point; 
        svkSpecPoint::UnitType  specUnits; 

        void                    CreateXSpecRangeWidget( vtkKWWidget* parent );
        void                    CreateYSpecRangeWidget( vtkKWWidget* parent );
        void                    EnableMRSWidgetCallbacks( vtkKWWidget* parent );
        void                    EnableMRSWidgets();
        void                    DisableMRSWidgets();
        
};

}   //svk namespace      

#endif //SVK_APP_GUI_H 
