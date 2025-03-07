/*
 *  Copyright © 2009-2017 The Regents of the University of California.
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

#ifndef SVK_COMBINE_WIDGET_H 
#define SVK_COMBINE_WIDGET_H 

#include <vtkKWCompositeWidget.h>
#include <vtkObjectFactory.h>
#include <vtkKWTkUtilities.h>
#include <vtkKWPushButton.h>
#include <vtkKWMenuButtonWithLabel.h>

#include <svkHSVD.h>
#include <svkDataModel.h>
#include <svkPhaseSpec.h>
#include <svkMrsImageFFT.h>
#include <svkMRSCombine.h>
#include <svkMultiCoilPhase.h>
#include <svkPlotGridViewController.h>
#include <svkOverlayViewController.h>
#include <svkMrsZeroFill.h>
#include <svkMrsApodizationFilter.h>
#include <sivicKWCompositeWidget.h>

#include <string.h>

using namespace svk; 

class sivicCombineWidget : public sivicKWCompositeWidget
{

    friend class vtkSivicController;

    public:

        static sivicCombineWidget *New();
        vtkTypeMacro(sivicCombineWidget,sivicKWCompositeWidget);
        //typedef svkCoilCombine::CombinationMethod CombinationMethod;

    protected:


        sivicCombineWidget();
        ~sivicCombineWidget();

        vtkKWPushButton* magnitudeCombinationButton;
        vtkKWPushButton* additionCombinationButton;
        
        // Description:
        // Create the widget.
        virtual void    CreateWidget();
        virtual void    ProcessCallbackCommandEvents( vtkObject*, unsigned long, void* );
        void            ResetRange();
        svkMRSCombine::CombinationMethod method;
        void            ExecuteCombine(svkMRSCombine::CombinationMethod method, svkMRSCombine::CombinationDimension dimension );



    private:

        vtkCallbackCommand*         progressCallback;

        static void                 UpdateProgress(vtkObject* subject, unsigned long, void* thisObject, void* callData);


        sivicCombineWidget(const sivicCombineWidget&);   // Not implemented.
        void operator=(const sivicCombineWidget&);  // Not implemented.
        
};

#endif //SVK_COMBINE_WIDGET_H 
