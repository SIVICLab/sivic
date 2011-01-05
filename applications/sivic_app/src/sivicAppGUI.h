/*
 *  Copyright © 2009-2011 The Regents of the University of California.
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
