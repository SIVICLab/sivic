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

#ifndef SVK_DSC_WIDGET_H 
#define SVK_DSC_WIDGET_H 

#include <vtkKWCompositeWidget.h>
#include <vtkObjectFactory.h>
#include <vtkKWTkUtilities.h>
#include <vtkKWPushButton.h>
#include <vtkKWMenuButtonWithLabel.h>

#include <svkHSVD.h>
#include <svkDataModel.h>
#include <svkDSCDeltaR2.h>
#include <svkDSCQuantify.h>
#include <svkPlotGridViewController.h>
#include <svkOverlayViewController.h>
#include <sivicKWCompositeWidget.h>

#include <string.h>

using namespace svk; 

class sivicDSCWidget : public sivicKWCompositeWidget
{

    friend class vtkSivicController;

    public:

        static sivicDSCWidget *New();
        vtkTypeMacro(sivicDSCWidget,sivicKWCompositeWidget);
        vtkstd::vector < vtkstd::string >   modelDSCNames;


    protected:


        sivicDSCWidget();
        ~sivicDSCWidget();

        vtkKWMenuButtonWithLabel*       dscRepresentationSelector;
        vtkKWPushButton*                generateMapsButton;
        vtkKWLabel*                     dscLabel; 
        vtkKWMenuButtonWithLabel*       mapViewSelector;

        
        // Description:
        // Create the widget.
        virtual void    CreateWidget();
        virtual void    ProcessCallbackCommandEvents( vtkObject*, unsigned long, void* );
        void            ResetRange();
        void            SetDSCRepresentationCallback( svkDSCDeltaR2::representation representation); 
        void            SetOverlay( vtkstd::string modelObjectName); 


    private:

        vtkCallbackCommand*                 progressCallback;

        void                                ExecuteDSC();
        void                                ExecuteQuantification();
        static void                         UpdateProgress(vtkObject* subject, unsigned long, void* thisObject, void* callData);

        svkDSCQuantify*                     dscQuant;
        vtkstd::vector < vtkstd::string >   dscNames;




        sivicDSCWidget(const sivicDSCWidget&);   // Not implemented.
        void operator=(const sivicDSCWidget&);  // Not implemented.
        svkDSCDeltaR2*                      dscRep;

        
};

#endif //SVK_DSC_WIDGET_H 
