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

#include <svkSpecPoint.h>

#include <sivicPostprocessingWidget.h>
#include <vtkSivicController.h>

vtkStandardNewMacro( sivicPostprocessingWidget );
//vtkCxxRevisionMacro( sivicPostprocessingWidget, "$Revision$");


/*! 
 *  Constructor
 */
sivicPostprocessingWidget::sivicPostprocessingWidget()
{
    this->hsvdButton = NULL;
    this->selectionBoxOnlyButton = NULL;

    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgress );
    this->progressCallback->SetClientData( (void*)this );
}


/*! 
 *  Destructor
 */
sivicPostprocessingWidget::~sivicPostprocessingWidget()
{

    if( this->hsvdButton != NULL ) {
        this->hsvdButton->Delete();
        this->hsvdButton= NULL;
    }

    if( this->selectionBoxOnlyButton != NULL ) {
        this->selectionBoxOnlyButton->Delete();
        this->selectionBoxOnlyButton = NULL;
    }

}


/*! 
 *  Method in superclass to be overriden to add our custom widgets.
 */
void sivicPostprocessingWidget::CreateWidget()
{
/*  This method will create our main window. The main window is a 
    vtkKWCompositeWidget with a vtkKWRendWidget. */

    // Check if already created
    if ( this->IsCreated() )
    {
        vtkErrorMacro(<< this->GetClassName() << " already created");
        return;
    }

    // Call the superclass to create the composite widget container
    this->Superclass::CreateWidget();

    //vtkKWCheckButtonSet* checkButtons = vtkKWCheckButtonSet::New();
    //checkButtons->SetParent( this );
    //checkButtons->PackHorizontallyOn( );
    //checkButtons->UniformRowsOn( );
    //checkButtons->UniformColumnsOn( );
    //checkButtons->ExpandWidgetsOn( );
    //checkButtons->Create();

    this->selectionBoxOnlyButton = vtkKWCheckButton::New(); 
    this->selectionBoxOnlyButton->SetParent(this);
    this->selectionBoxOnlyButton->Create();
    this->selectionBoxOnlyButton->EnabledOff();
    this->selectionBoxOnlyButton->SetText("Only fit voxels in selection box");
    this->selectionBoxOnlyButton->SelectedStateOn();

    this->removeH20Button =vtkKWCheckButton::New(); 
    this->removeH20Button->SetParent(this);
    this->removeH20Button->Create();
    this->removeH20Button->EnabledOff();
    this->removeH20Button->SetText("remove H20");
    this->removeH20Button->SelectedStateOn();

    this->removeLipidButton =vtkKWCheckButton::New(); 
    this->removeLipidButton->SetParent(this);
    this->removeLipidButton->Create();
    this->removeLipidButton->EnabledOff();
    this->removeLipidButton->SetText("remove Lipid");
    this->removeLipidButton->SelectedStateOff();

    this->hsvdButton = vtkKWPushButton::New();
    this->hsvdButton->SetParent( this );
    this->hsvdButton->Create( );
    this->hsvdButton->EnabledOff();
    this->hsvdButton->SetText( "HSVD Baseline Removal");
    this->hsvdButton->SetBalloonHelpString("HSVD Baseline Removal.");

    this->Script("grid %s -row 0 -column 0 -columnspan 1 -sticky w", this->hsvdButton->GetWidgetName() );
    this->Script("grid %s -row 1 -column 0 -columnspan 1 -sticky w", this->selectionBoxOnlyButton->GetWidgetName() );
    this->Script("grid %s -row 2 -column 0 -columnspan 1 -sticky w", removeH20Button->GetWidgetName() );
    this->Script("grid %s -row 3 -column 0 -columnspan 1 -sticky w", removeLipidButton->GetWidgetName() );

    this->Script("grid rowconfigure %s 0  -weight 2", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 1  -weight 2", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 2  -weight 2", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 3  -weight 2", this->GetWidgetName() );

    this->Script("grid columnconfigure %s 0 -weight 1", this->GetWidgetName() );

    this->AddCallbackCommandObserver(
        this->hsvdButton, vtkKWPushButton::InvokedEvent );

}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicPostprocessingWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{

    if( caller == this->hsvdButton && event == vtkKWPushButton::InvokedEvent ) {
        this->ExecuteHSVD();
    } 
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}


/*!
 *  Executes HSVD.
 */
void sivicPostprocessingWidget::ExecuteHSVD() 
{
    svkImageData* data = this->model->GetDataObject("SpectroscopicData");

    if( data != NULL ) {

        svkHSVD* hsvd = svkHSVD::New();
        hsvd->AddObserver(vtkCommand::ProgressEvent, progressCallback);
        this->GetApplication()->GetNthWindow(0)->SetStatusText("Executing HSVD baseline removal");
        hsvd->SetInputData( data ); 
        //hsvd->SetModelOrder( modelOrder );

        if ( this->selectionBoxOnlyButton->GetSelectedState() ) {
            hsvd->OnlyFitSpectraInVolumeLocalization();
        }

        svkDcmHeader* hdr = data->GetDcmHeader();
        svkSpecPoint* point = svkSpecPoint::New();
        point->SetDcmHeader( hdr );

        if ( this->removeH20Button->GetSelectedState() ) {
            hsvd->RemoveH20On(); 
        }
        if ( this->removeLipidButton->GetSelectedState() ) {
            hsvd->RemoveLipidOn(); 
        }

        hsvd->Update();
        hsvd->RemoveObserver( progressCallback );

        data->Modified();

        string stringFilename = "hsvd";
        this->sivicController->Open4DImage( data, stringFilename);
        this->sivicController->EnableWidgets( );

        hsvd->Delete();
        this->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 0.0 );
        this->GetApplication()->GetNthWindow(0)->SetStatusText(" Done ");
    }

}



void sivicPostprocessingWidget::UpdateProgress(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)
        ->GetProgressGauge()->SetValue( 100.0*(*(double*)(callData)) );
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->SetStatusText(
        static_cast<vtkAlgorithm*>(subject)->GetProgressText() 
    );

}

