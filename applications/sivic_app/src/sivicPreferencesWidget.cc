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


#include <sivicPreferencesWidget.h>
#include <vtkSivicController.h>

vtkStandardNewMacro( sivicPreferencesWidget );
vtkCxxRevisionMacro( sivicPreferencesWidget, "$Revision$");


/*! 
 *  Constructor
 */
sivicPreferencesWidget::sivicPreferencesWidget()
{
    this->testButton = NULL;
}


/*! 
 *  Destructor
 */
sivicPreferencesWidget::~sivicPreferencesWidget()
{
    if( this->testButton != NULL ) {
        this->testButton->Delete();
        this->testButton = NULL;
    }

}


/*! 
 *  Restores preferences from your registry.
 */
void sivicPreferencesWidget::RestorePreferencesFromRegistry( )
{

}


/*! 
 *  Get Preference.
 */
bool sivicPreferencesWidget::GetReadOnlyOneInputFile()
{
    return false; 
}

/*! 
 *  Method in superclass to be overriden to add our custom widgets.
 */
void sivicPreferencesWidget::CreateWidget()
{

    // Check if already created
    if ( this->IsCreated() )
    {
        vtkErrorMacro(<< this->GetClassName() << " already created");
        return;
    }

    // Call the superclass to create the composite widget container
    this->Superclass::CreateWidget();

    this->testButton = vtkKWCheckButton::New();
    this->testButton->SetParent(this);
    this->testButton->SetText("Test Button");
    this->testButton->Create();


    //  =======================================================
    //  Now we pack the application together
    //  =======================================================
    int row = 0; 

    this->Script("grid %s -in %s -row 0 -column 0 -sticky ew ",
             this->testButton->GetWidgetName(), this->GetWidgetName() );


    this->Script("grid rowconfigure %s 0  -weight 1", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 0  -weight 1", this->GetWidgetName() );

    this->AddCallbackCommandObserver(
        this->testButton, vtkKWCheckButton::SelectedStateChangedEvent );

}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicPreferencesWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{
    if( caller == testButton ) {
        cout << "SELECTED STATE CHANGED!!! " << endl;
    } 

    // Make sure the superclass gets called for render requests
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);

}
