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



#include <sivicProcessingWidget.h>
#include <vtkSivicController.h>

vtkStandardNewMacro( sivicProcessingWidget );
//vtkCxxRevisionMacro( sivicProcessingWidget, "$Revision$");


/*! 
 *  Constructor
 */
sivicProcessingWidget::sivicProcessingWidget()
{
    this->fftButton = NULL;
    this->spatialButton = NULL;
    this->spectralButton = NULL;
    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgress );
    this->progressCallback->SetClientData( (void*)this );


}


/*! 
 *  Destructor
 */
sivicProcessingWidget::~sivicProcessingWidget()
{

    if( this->fftButton != NULL ) {
        this->fftButton->Delete();
        this->fftButton= NULL;
    }

}


/*! 
 *  Method in superclass to be overriden to add our custom widgets.
 */
void sivicProcessingWidget::CreateWidget()
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

    vtkKWCheckButtonSet* checkButtons = vtkKWCheckButtonSet::New();
    checkButtons->SetParent( this );
    checkButtons->PackHorizontallyOn( );
    checkButtons->ExpandWidgetsOn( );
    checkButtons->Create();

    this->spatialButton = checkButtons->AddWidget(0);
    this->spatialButton->SetParent(this);
    this->spatialButton->Create();
    this->spatialButton->EnabledOff();
    this->spatialButton->SetText("spatial domain");
    this->spatialButton->SelectedStateOn();

    this->spectralButton = checkButtons->AddWidget(1);
    this->spectralButton->SetParent(this);
    this->spectralButton->Create();
    this->spectralButton->EnabledOff();
    this->spectralButton->SetText("spectral domain");
    this->spectralButton->SelectedStateOn();

    this->fftButton = vtkKWPushButton::New();
    this->fftButton->SetParent( this );
    this->fftButton->Create( );
    this->fftButton->EnabledOff();
    this->fftButton->SetText( "Transform" );
    this->fftButton->SetBalloonHelpString("Prototype Single Voxel FFT.");

    //this->Script("grid %s -row 0 -column 0 -columnspan 6 -sticky nwes", this->phaseSlider->GetWidgetName() );
    //this->Script("grid %s -row 1 -column 0 -columnspan 6 -sticky nwes", this->linearPhaseSlider->GetWidgetName() );
    //this->Script("grid %s -row 2 -column 0 -columnspan 2 -sticky nwes ", this->phasePivotEntry->GetWidgetName() );
    //this->Script("grid %s -row 2 -column 2 -columnspan 4 -sticky nwes", checkButtons->GetWidgetName() );
    this->Script("grid %s -row 0 -column 0 -columnspan 6 -sticky we -padx 4 -pady 2 ", this->fftButton->GetWidgetName() );
    this->Script("grid %s -row 1 -column 2 -columnspan 6 -sticky nwes", checkButtons->GetWidgetName() );

    this->Script("grid rowconfigure %s 0  -weight 2", this->GetWidgetName() );
    this->Script("grid rowconfigure %s 1  -weight 2", this->GetWidgetName() );
    //this->Script("grid rowconfigure %s 2  -weight 1", this->GetWidgetName() );
    //this->Script("grid rowconfigure %s 3  -weight 2", this->GetWidgetName() );

    this->Script("grid columnconfigure %s 0 -weight 1", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 1 -weight 1", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 2 -weight 1", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 3 -weight 1", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 4 -weight 1", this->GetWidgetName() );
    this->Script("grid columnconfigure %s 5 -weight 1", this->GetWidgetName() );

    this->AddCallbackCommandObserver(
        this->overlayController->GetRWInteractor(), vtkCommand::SelectionChangedEvent );
    this->AddCallbackCommandObserver(
        this->plotController->GetRWInteractor(), vtkCommand::SelectionChangedEvent );
    this->AddCallbackCommandObserver(
        this->fftButton, vtkKWPushButton::InvokedEvent );
    //this->AddCallbackCommandObserver(
        //this->spatialButton, vtkKWCheckButton::SelectedStateChangedEvent );
    //this->AddCallbackCommandObserver(
        //this->spectralButton, vtkKWCheckButton::SelectedStateChangedEvent );

    checkButtons->Delete();

}


/*! 
 *  Method responds to callbacks setup in CreateWidget
 */
void sivicProcessingWidget::ProcessCallbackCommandEvents( vtkObject *caller, unsigned long event, void *calldata )
{

    if( caller == this->fftButton && event == vtkKWPushButton::InvokedEvent ) {
        this->ExecuteRecon();
    }
    this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}



/*!
 *  Executes the FFT in place.
 */
void sivicProcessingWidget::ExecuteFFT() 
{
    svkImageData* data = this->model->GetDataObject("SpectroscopicData");
    if( data != NULL ) {
        // We'll turn the renderer off to avoid rendering intermediate steps
        this->plotController->GetView()->TurnRendererOff(svkPlotGridView::PRIMARY);
        svkMrsImageFFT* imageFFT = svkMrsImageFFT::New();
        imageFFT->SetInputData( data );
        imageFFT->Update();
        data->Modified();
        imageFFT->Delete();
        bool useFullFrequencyRange = 1;
        bool useFullAmplitudeRange = 1;
        bool resetAmplitude = 1;
        bool resetFrequency = 1;
        this->sivicController->ResetRange( useFullFrequencyRange, useFullAmplitudeRange, 
                                           resetAmplitude, resetFrequency );
        this->sivicController->EnableWidgets( );
        this->plotController->GetView()->TurnRendererOn(svkPlotGridView::PRIMARY);
        this->plotController->GetView()->Refresh();
    }

}


/*!
 *  Executes the Recon.
 */
void sivicProcessingWidget::ExecuteRecon() 
{
    svkImageData* data = this->model->GetDataObject("SpectroscopicData");
    if( data != NULL ) {


        // Get data spectral dimension, and transform appropriately:
        svkMrsImageFFT* spatialFFT = svkMrsImageFFT::New();
        cout << "SSSPATIAL: " << this->spatialButton->GetSelectedState() << endl; 
        cout << "SSSPECTRAL: " << this->spectralButton->GetSelectedState() << endl; 
        if ( this->spatialButton->GetSelectedState() ) {

            string spatialDomain0 = data->GetDcmHeader()->GetStringValue( "SVK_ColumnsDomain");
            string spatialDomain1 = data->GetDcmHeader()->GetStringValue( "SVK_RowsDomain");
            string spatialDomain2 = data->GetDcmHeader()->GetStringValue( "SVK_SliceDomain");

            spatialFFT->SetInputData( data );
            spatialFFT->SetFFTDomain( svkMrsImageFFT::SPATIAL );
            if ( spatialDomain0.compare("KSPACE") == 0 && spatialDomain1.compare("KSPACE") == 0 && spatialDomain2.compare("KSPACE") == 0 )  {
                spatialFFT->SetFFTMode( svkMrsImageFFT::REVERSE );
            } else {
                spatialFFT->SetFFTMode( svkMrsImageFFT::FORWARD);
            }
            spatialFFT->SetPreCorrectCenter( true );
            spatialFFT->SetPostCorrectCenter( true );
            spatialFFT->AddObserver(vtkCommand::ProgressEvent, progressCallback);
            this->GetApplication()->GetNthWindow(0)->SetStatusText("Executing Spatial Recon...");
            spatialFFT->Update();
            spatialFFT->RemoveObserver( progressCallback );
        }

        svkMrsImageFFT* spectralFFT = svkMrsImageFFT::New();
        if ( this->spectralButton->GetSelectedState() ) {
            string domain = data->GetDcmHeader()->GetStringValue("SignalDomainColumns");
            spectralFFT->AddObserver(vtkCommand::ProgressEvent, progressCallback);
            this->GetApplication()->GetNthWindow(0)->SetStatusText("Executing FFT...");
            if ( this->spatialButton->GetSelectedState() ) {
                spectralFFT->SetInputData( spatialFFT->GetOutput() );
            } else {
                spectralFFT->SetInputData( data );
            }
            spectralFFT->SetFFTDomain( svkMrsImageFFT::SPECTRAL );
            if ( domain.compare("TIME") == 0 ) {
                spectralFFT->SetFFTMode( svkMrsImageFFT::FORWARD );
            } else {
                spectralFFT->SetFFTMode( svkMrsImageFFT::REVERSE );
            }
            spectralFFT->Update();
            data->Modified();
            //data->Update();
            spectralFFT->RemoveObserver( progressCallback);
        }
        

        bool useFullFrequencyRange = 1;
        bool useFullAmplitudeRange = 1;
        bool resetAmplitude = 1;
        bool resetFrequency = 1;
        //this->sivicController->ResetRange( useFullFrequencyRange, useFullAmplitudeRange, 
        //                                   resetAmplitude, resetFrequency );
        string stringFilename = "Result";
        this->sivicController->Open4DImage( data, stringFilename);
        this->sivicController->EnableWidgets( );

        // We are resetting the input to make sure the actors get updated

        //this->plotController->SetInput(data);
        //this->plotController->GetView()->TurnRendererOn(svkPlotGridView::PRIMARY);
        //this->plotController->GetView()->Refresh();
        spatialFFT->Delete();
        spectralFFT->Delete();
        this->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 0.0 );
        this->GetApplication()->GetNthWindow(0)->SetStatusText(" Done ");
    }

}


void sivicProcessingWidget::UpdateProgress(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->GetProgressGauge()->SetValue( 100.0*(*(double*)(callData)) );
    static_cast<vtkKWCompositeWidget*>(thisObject)->GetApplication()->GetNthWindow(0)->SetStatusText(
                  static_cast<vtkAlgorithm*>(subject)->GetProgressText() );

}

