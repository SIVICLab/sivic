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
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 *
 *  TODO: REFACTOR COMMON CODE
 */


#include <svkSecondaryCaptureFormatterProstate.h>
#include <vtkSivicController.h>


using namespace svk;


vtkCxxRevisionMacro(svkSecondaryCaptureFormatterProstate, "$Rev$");
vtkStandardNewMacro(svkSecondaryCaptureFormatterProstate);


//! Constructor
svkSecondaryCaptureFormatterProstate::svkSecondaryCaptureFormatterProstate()
{
    this->aspect = PORTRAIT;

    //  landscape: 
    imageSize[0] = 1200;
    imageSize[1] = 800;
    //  portrait  
    imageSize[0] = 800;
    imageSize[1] = 1200;

    spectraHalfSize[0] = 500;
    spectraHalfSize[1] = 500;

    this->orientation = svkDcmHeader::AXIAL;
}


//! Destructor
svkSecondaryCaptureFormatterProstate::~svkSecondaryCaptureFormatterProstate()
{
}


/*!
 *
 */
void svkSecondaryCaptureFormatterProstate::RenderCombinedImage( int firstFrame, int lastFrame, svkImageData* outputImage, bool flipImage, bool print )
{
    svkPlotGridView::SafeDownCast(this->plotController->GetView())->AlignCameraOff(); 

    if ( this->aspect == LANDSCAPE ) {

        //  Viewport, specify 2 x,y tuples for each panel: 
        //  origin (blc)
        //  trc
        //  To change layout, only modify the heights and widths: 
        double imageViewHeight = .25; 
        double plotViewHeight  = .77; 
        double infoViewHeight  = .18; 
        double titleViewHeight = .05; 
        double imageWidth      = imageViewHeight; 
        double plotWidth       = plotViewHeight; 
        double blankHeight     = (plotViewHeight - imageViewHeight)/2; 

        //  Y Values: 
        double viewBottom      = 0.; 
        double imageViewTop    = blankHeight + imageViewHeight;     
        double plotViewTop     = viewBottom + plotViewHeight;     
        double infoViewBottom  = plotViewTop;                    
        double infoViewTop     = infoViewBottom + infoViewHeight;   
        double titleViewBottom = infoViewTop;                  
        double titleViewTop    = titleViewBottom + titleViewHeight; 

        //  X Values: 
        double col1Left        = 0.; 
        double col1Right       = col1Left + imageWidth; 
        double col2Left        = col1Right; 
        double col2Right       = col2Left + plotWidth; 

        //  col 1
        //  blank fills in empty space around overlay view
        double blankViewport1[4] = { 
                col1Left, 
                viewBottom, 
                col1Right, 
                blankHeight 
        };
        double overlayViewport[4] = { 
                col1Left, 
                blankHeight, 
                col1Right, 
                imageViewTop    
        };
        double blankViewport2[4] = { 
                col1Left, 
                imageViewTop, 
                col1Right, 
                1 - titleViewHeight - infoViewHeight
        };

        //  col 2
        double plotViewport[4] = { 
                col2Left, 
                viewBottom,  
                col2Right, 
                plotViewTop
        };

        //  col 1
        double imageInfoViewport[4] = { 
                col1Left, 
                infoViewBottom, 
                col1Right, 
                infoViewTop
        };

        //  col 2
        double spectraInfoViewport[4] = { 
                col2Left, 
                infoViewBottom, 
                col2Right, 
                infoViewTop
        };

        double titleViewport[4] = { 
                col1Left, 
                titleViewBottom,  
                col2Right, 
                titleViewTop
        };

        if ( this->GetDebug() ) {
            cout << "title: " << titleViewport[0] << " " << titleViewport[1] << " " 
                << titleViewport[2] << " " <<  titleViewport[3] << endl;
            cout << "INFO I: " << imageInfoViewport[0] << " " << imageInfoViewport[1] << " " 
                << imageInfoViewport[2] << " " <<  imageInfoViewport[3] << endl;
            cout << "INFO S: " << spectraInfoViewport[0] << " " << spectraInfoViewport[1] << " " 
                << spectraInfoViewport[2] << " " <<  spectraInfoViewport[3] << endl;
            cout << "Blank View 1: " << blankViewport1[0] << " " << blankViewport1[1] << " " 
                << blankViewport1[2] << " " <<  blankViewport1[3] << endl;
            cout << "View: " << overlayViewport[0] << " " << overlayViewport[1] << " " 
                << overlayViewport[2] << " " <<  overlayViewport[3] << endl;
            cout << "Blank View: " << blankViewport2[0] << " " << blankViewport2[1] << " " 
                << blankViewport2[2] << " " <<  blankViewport2[3] << endl;
            cout << "View: " << plotViewport[0] << " " << plotViewport[1] << " " 
                << plotViewport[2] << " " <<  plotViewport[3] << endl;
        }

        vtkRenderWindow* window = vtkRenderWindow::New();
    #if defined(linux)
        window->OffScreenRenderingOn();
    #endif 

        // Add Title
        vtkRenderer* titleRenderer = vtkRenderer::New();
        titleRenderer->SetBackground( 0.25, 0.25, 0.25 );
        vtkTextActor* titleActor = vtkTextActor::New();
        titleActor->SetInput( "SIVIC: Research Software" );
        titleActor->SetTextScaleModeToViewport();
        titleRenderer->AddActor( titleActor );
        titleActor->Delete();
        window->AddRenderer( titleRenderer );
        titleRenderer->SetViewport( titleViewport  );
        titleRenderer->Delete();

        vtkRenderer* blankRenderer1 = vtkRenderer::New();
        blankRenderer1->SetBackground( 0, 0, 0 );
        window->AddRenderer( blankRenderer1 );
        blankRenderer1->SetViewport( blankViewport1  );
        blankRenderer1->Delete();

        vtkRenderer* blankRenderer2 = vtkRenderer::New();
        blankRenderer2->SetBackground( 0, 0, 0 );
        window->AddRenderer( blankRenderer2 );
        blankRenderer2->SetViewport( blankViewport2  );
        blankRenderer2->Delete();
        
        // Add plot to capture window
        this->plotController->GetView()->TurnRendererOff( svkPlotGridView::PRIMARY );
        window->AddRenderer( this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY ) );
        this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->SetViewport( plotViewport );


        // Add overlay to capture window
        this->overlayController->GetView()->TurnRendererOff( svkPlotGridView::PRIMARY );
        window->AddRenderer( this->overlayController->GetView()->GetRenderer( svkPlotGridView::PRIMARY ) );
        this->overlayController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->SetViewport( overlayViewport );


        vtkRenderer* spectraInfoRenderer = vtkRenderer::New();
        vtkTextActor* specText1 = vtkTextActor::New();
        specText1->SetTextScaleModeToProp();
        specText1->SetPosition(0.0, 0.0);
        specText1->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
        specText1->SetPosition2(0.35, 1.0);
        specText1->GetPosition2Coordinate()->SetCoordinateSystemToNormalizedViewport();
        spectraInfoRenderer->AddActor( specText1 );

        window->AddRenderer( spectraInfoRenderer );
        vtkTextActor* specText2 = vtkTextActor::New();
        //specText2->SetTextScaleModeToViewport();
        specText2->SetTextScaleModeToProp();
        specText2->SetPosition(0.35, 0.00);
        specText2->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
        specText2->GetPosition2Coordinate()->SetCoordinateSystemToNormalizedViewport();
        specText2->SetPosition2(1.0, 1.0);
        specText2->GetPosition2Coordinate()->SetCoordinateSystemToNormalizedViewport();
        spectraInfoRenderer->AddActor( specText2 );
        spectraInfoRenderer->SetViewport( spectraInfoViewport );
        if( print ) {
            specText1->GetTextProperty()->SetColor(0,0,0);
            specText2->GetTextProperty()->SetColor(0,0,0);
            spectraInfoRenderer->SetBackground( 1,1,1 );
        }
        spectraInfoRenderer->Delete();

        vtkRenderer* imageInfoRenderer = vtkRenderer::New();
        window->AddRenderer( imageInfoRenderer );
        vtkTextActor* imageText = vtkTextActor::New();
        imageText->SetTextScaleModeToProp();
        imageText->SetPosition(0.05,0.0);
        imageText->SetPosition2(0.95,1.0);
        specText2->GetPosition2Coordinate()->SetCoordinateSystemToNormalizedViewport();
        imageInfoRenderer->AddActor( imageText );
        imageInfoRenderer->SetViewport( imageInfoViewport );
        if( print ) {
            vtkScalarBarActor::SafeDownCast(this->overlayController->GetView()->GetProp( svkOverlayView::COLOR_BAR )
                                        )->GetLabelTextProperty()->SetColor(0, 0, 0);
            imageText->GetTextProperty()->SetColor(0,0,0);
            imageInfoRenderer->SetBackground( 1,1,1 );
        }
        imageInfoRenderer->Delete();

        window->SetSize( imageSize[0],imageSize[1] );

        vtkImageAppend* sliceAppender = vtkImageAppend::New();
        sliceAppender->SetAppendAxis(2);
        for (int m = firstFrame; m <= lastFrame; m++) {
            vtkImageData* tmpData = vtkImageData::New();
            this->sivicController->SetSlice(m);
            this->PopulateInfoText( specText1, specText2,  imageText );
            window->Render();
            vtkWindowToImageFilter* wtif = vtkWindowToImageFilter::New();
            wtif->SetMagnification(1);
            wtif->SetInput( window );
            wtif->Update( );

            tmpData->DeepCopy( wtif->GetOutput() );
            tmpData->Update();
            sliceAppender->SetInput(m-firstFrame, tmpData );
            wtif->Delete();
        }

        if( flipImage ) {  
            vtkImageFlip* flipper = vtkImageFlip::New();
            flipper->SetFilteredAxis( 1 );
            flipper->SetInput( sliceAppender->GetOutput() );
            flipper->Update();
            outputImage->DeepCopy( flipper->GetOutput() );
            flipper->Delete();
        } else {
            sliceAppender->Update();
            outputImage->DeepCopy( sliceAppender->GetOutput() );
        }

        window->RemoveRenderer( this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY ) );
        window->RemoveRenderer( this->overlayController->GetView()->GetRenderer( svkPlotGridView::PRIMARY ) );
        this->plotController->GetView()->TurnRendererOn( svkPlotGridView::PRIMARY );
        this->overlayController->GetView()->TurnRendererOn( svkPlotGridView::PRIMARY );
        this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->SetViewport( 0.0,0.0,1.0,1.0 );
        this->overlayController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->SetViewport( 0.0,0.0,1.0,1.0 );
        specText1->Delete();
        specText2->Delete();
        imageText->Delete();
        window->Delete();

    } else {

        // portrait (3 panels) single column with 3 rows: 0 title , info + image, plot   
        
        //  Viewport, specify 2 x,y tuples for each panel: 
        //  origin (blc)
        //  trc
        //  To change layout, only modify the heights and widths: 

        int* size = this->sivicController-> GetRenderWindowSize(); 
        cout << "SIZE 0 = "   << size[0] << endl; 
        cout << "SIZE 1 = "   << size[1] << endl; 
        
        double portraitAspect =  2./3.; //portrait 1200/800
        double aspect = portraitAspect * size[1] / size[0]; 

        double plotWidth         = 1.0; 
        double plotViewHeight    = plotWidth * aspect; 
        double titleViewHeight   = .04; 
        double infoViewHeight    = .02; 
        double spacerHeight      = .01; 
        

        //set the image height to be the 1. - (plot view height + title height + infoViewHeight)
        double overlayViewHeight = 1 - plotViewHeight - titleViewHeight - infoViewHeight; 
        double overlayWidth      = overlayViewHeight / aspect; 
        if ( overlayWidth > 1 ) {
            overlayWidth      = 1;  
            overlayViewHeight = overlayWidth * aspect; 
        }

        double viewBottom        = 0.; 
        double plotViewTop       = viewBottom + plotViewHeight;     
        double overlayViewBottom = plotViewTop + spacerHeight; 
        double overlayViewTop    = overlayViewBottom + overlayViewHeight; 
        double titleViewTop      = 1.0; 
        double titleViewBottom   = titleViewTop - titleViewHeight; 
        double infoViewTop       = titleViewBottom; 
        double infoViewBottom    = infoViewTop - infoViewHeight; 

        //  X Values: width = 800
        double col1Left        = 0.0; 
        double col1Right       = 1;

        double plotViewport[4] = { 
                (1 - plotWidth)/2., 
                viewBottom, 
                plotWidth + (1 - plotWidth)/2., 
                plotViewTop 
        };

        //  blank fills in empty space around overlay view
        double blankViewport1[4] = { 
                col1Left, 
                overlayViewTop, 
                col1Right, 
                infoViewBottom
        };
        double spaceViewport1[4] = { 
                col1Left, 
                plotViewTop, 
                col1Right, 
                overlayViewBottom
        };
        double overlayViewport[4] = { 
                col1Left, 
                overlayViewBottom, 
                col1Right, 
                overlayViewTop    
        };
        double infoViewport[4] = { 
                col1Left, 
                infoViewBottom, 
                col1Right, 
                infoViewTop
        };
        double titleViewport[4] = { 
                col1Left, 
                titleViewBottom, 
                col1Right, 
                titleViewTop
        };


        if ( 1 ) { 
        //if ( this->GetDebug() ) {
            cout << "title       : " << titleViewport[0] << " " << titleViewport[1] << " " 
                << titleViewport[2] << " " <<  titleViewport[3] << endl;
            cout << "INFO I      : " << infoViewport[0] << " " << infoViewport[1] << " " 
                << infoViewport[2] << " " <<  infoViewport[3] << endl;
            cout << "OVERLAY View: " << overlayViewport[0] << " " << overlayViewport[1] << " " 
                << overlayViewport[2] << " " <<  overlayViewport[3] << endl;
            cout << "PLOT View   : " << plotViewport[0] << " " << plotViewport[1] << " " 
                << plotViewport[2] << " " <<  plotViewport[3] << endl;
        }

        vtkRenderWindow* window = vtkRenderWindow::New();
        #if defined(linux)
            window->OffScreenRenderingOn();
        #endif 

        // Add Title
        vtkRenderer* titleRenderer = vtkRenderer::New();
        titleRenderer->SetBackground( 0.25, 0.25, 0.25 );
        vtkTextActor* titleActor = vtkTextActor::New();
        titleActor->SetInput( "SIVIC: Research Software" );
        titleActor->SetTextScaleModeToViewport();
        titleActor->SetNonLinearFontScale(2.0, 10);
        titleRenderer->AddActor( titleActor );
        titleActor->Delete();
        window->AddRenderer( titleRenderer );
        titleRenderer->SetViewport( titleViewport  );
        titleRenderer->Delete();

        // if there is space between the title and the overlay view add a blank
        if ( infoViewBottom - overlayViewTop > 0 ) {
            vtkRenderer* blankRenderer1 = vtkRenderer::New();
            blankRenderer1->SetBackground( 0, 0, 0 );
            window->AddRenderer( blankRenderer1 );
            blankRenderer1->SetViewport( blankViewport1  );
            blankRenderer1->Delete();
        }
        vtkRenderer* spaceRenderer1 = vtkRenderer::New();
        spaceRenderer1->SetBackground( 0, 0, 0 );
        window->AddRenderer( spaceRenderer1 );
        spaceRenderer1->SetViewport( spaceViewport1  );
        spaceRenderer1->Delete();


        // Add plot to capture window
        this->plotController->GetView()->TurnRendererOff( svkPlotGridView::PRIMARY );
        window->AddRenderer( this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY ) );
        this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->SetViewport( plotViewport );

        // Add overlay to capture window
        this->overlayController->GetView()->TurnRendererOff( svkPlotGridView::PRIMARY );
        window->AddRenderer( this->overlayController->GetView()->GetRenderer( svkPlotGridView::PRIMARY ) );
        this->overlayController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->SetViewport( overlayViewport );

        vtkRenderer* spectraInfoRenderer = vtkRenderer::New();
        vtkTextActor* specText1 = vtkTextActor::New();
        //specText1->SetTextScaleModeToProp();
        specText1->SetTextScaleModeToViewport();
        //specText1->SetNonLinearFontScale(2.0, 10);
        specText1->SetPosition(0.0, 0.0);
        specText1->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
        specText1->SetPosition2(1.00, 1.0);
        specText1->GetPosition2Coordinate()->SetCoordinateSystemToNormalizedViewport();
        spectraInfoRenderer->AddActor( specText1 );

        window->AddRenderer( spectraInfoRenderer );
        vtkTextActor* specText2 = vtkTextActor::New();
        spectraInfoRenderer->SetViewport( infoViewport );

        vtkTextActor* imageText = vtkTextActor::New();

        window->SetSize( imageSize[0],imageSize[1] );

        vtkImageAppend* sliceAppender = vtkImageAppend::New();
        sliceAppender->SetAppendAxis(2);
        for (int m = firstFrame; m <= lastFrame; m++) {
            vtkImageData* tmpData = vtkImageData::New();
            this->sivicController->SetSlice(m);
            this->PopulateInfoText( specText1, specText2,  imageText );
            window->Render();
            vtkWindowToImageFilter* wtif = vtkWindowToImageFilter::New();
            wtif->SetMagnification(1);
            wtif->SetInput( window );
            wtif->Update( );

            tmpData->DeepCopy( wtif->GetOutput() );
            tmpData->Update();
            sliceAppender->SetInput(m-firstFrame, tmpData );
            wtif->Delete();
        }

        if( flipImage ) {  
            vtkImageFlip* flipper = vtkImageFlip::New();
            flipper->SetFilteredAxis( 1 );
            flipper->SetInput( sliceAppender->GetOutput() );
            flipper->Update();
            outputImage->DeepCopy( flipper->GetOutput() );
            flipper->Delete();
        } else {
            sliceAppender->Update();
            outputImage->DeepCopy( sliceAppender->GetOutput() );
        }

        window->RemoveRenderer( this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY ) );
        window->RemoveRenderer( this->overlayController->GetView()->GetRenderer( svkPlotGridView::PRIMARY ) );
        this->plotController->GetView()->TurnRendererOn( svkPlotGridView::PRIMARY );
        this->overlayController->GetView()->TurnRendererOn( svkPlotGridView::PRIMARY );
        this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->SetViewport( 0.0,0.0,1.0,1.0 );
        this->overlayController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->SetViewport( 0.0,0.0,1.0,1.0 );
        specText1->Delete();
        specText2->Delete();
        imageText->Delete();
        window->Delete();

    }

    svkPlotGridView::SafeDownCast(this->plotController->GetView())->AlignCameraOn(); 

}


/*!
 *
 */
void svkSecondaryCaptureFormatterProstate::PopulateInfoText( vtkTextActor* specText1, 
                                                     vtkTextActor* specText2, 
                                                     vtkTextActor* imageText   )
{
    string currentImageName = model->GetDataFileName( "AnatomicalData" );
    string current4DImageName;
    if( this->sivicController->GetActive4DImageData()->IsA("svkMrsImageData") ) {
		current4DImageName = model->GetDataFileName( "SpectroscopicData" );
    } else {
		current4DImageName = model->GetDataFileName( "4DImageData" );
    }
    string currentOverlayName = model->GetDataFileName( "OverlayData" );
    string currentMetaboliteName = model->GetDataFileName( "MetaboliteData" );

    stringstream specInfo1;
    stringstream specInfo2;
    stringstream imageInfo;
    size_t pos;
    

    if( model->DataExists( "AnatomicalData" ) ) {

        // Image Series
        if( model->GetDataObject( "AnatomicalData" ) ) {
            int seriesNumber = model->GetDataObject( "AnatomicalData" )->GetDcmHeader()->GetIntValue("SeriesNumber");
            imageInfo << "Image Series:  " << seriesNumber << endl;
        }

        if( currentOverlayName != "" ) {
            pos = currentOverlayName.find_last_of("/"); 
            if( pos == currentOverlayName.npos ) {
                pos = 0;
            } else if( pos+1 < currentOverlayName.npos) {
                pos++;
            }
            imageInfo << "Overlay File: " << endl << " " << currentOverlayName.substr(pos) << endl;
        } else if( currentMetaboliteName != "" ) {
            pos = currentMetaboliteName.find_last_of("/"); 
            if( pos == currentMetaboliteName.npos ) {
                pos = 0;
            } else if( pos+1 < currentMetaboliteName.npos) {
                pos++;
            }
            imageInfo << "Overlay File: " << endl << " " << currentMetaboliteName.substr(pos) << endl;
        }

        // Image Name
        pos = currentImageName.find_last_of("/"); 
        if( pos == currentImageName.npos ) {
            pos = 0;
        } else if( pos + 1 < currentImageName.npos) {
            pos++;
        }
        imageInfo << "Image File:  " << endl << " " <<  currentImageName.substr(pos) << endl;

    } 

    // Get Spectroscopy Parameters
    if( this->sivicController->GetActive4DImageData() != NULL ) {

        specInfo1.setf(ios::fixed,ios::floatfield);            // floatfield not set
        specInfo1.precision(1);
        specInfo2.setf(ios::fixed,ios::floatfield);            // floatfield not set
        specInfo2.precision(1);

        // Slice
        specInfo1 << "Plot Slice No:  " << this->plotController->GetSlice() + 1 << "   ";
        double sliceLocation[3];

        // Slice Position:
        int sliceIndex[3] = {0, 0, 0 };
        int orientationIndex = this->sivicController->GetActive4DImageData()->GetOrientationIndex( orientation );
        sliceIndex[ orientationIndex ] = this->plotController->GetSlice();
        this->sivicController->GetActive4DImageData()->GetPositionFromIndex( sliceIndex, sliceLocation );
        specInfo1 << "Plot Slice Pos:  " << sliceLocation[orientationIndex] << "     ";
        
        pos = currentMetaboliteName.find_last_of("/"); 
        if( pos == currentMetaboliteName.npos ) {
            pos = 0;
        } else if( pos+1 < currentMetaboliteName.npos) {
            pos++;
        }
        //specInfo1 << "Metabolites: " << svkUCSFUtils::GetMetaboliteName( currentMetaboliteName ).c_str() << endl;
        //specInfo1 << "Metabolites File: " << endl << " " <<  currentMetaboliteName.substr(pos) << endl; 

        // Get The Spectra Name
        pos = current4DImageName.find_last_of("/");
        if( pos == current4DImageName.npos ) {
            pos = 0;
        } else if( pos+1 < current4DImageName.npos) {
            pos++;
        }
        string fileName = current4DImageName.substr(pos); 
        pos = fileName.find_first_of("_"); 
        string pcNumber; 
        if( pos == fileName.npos ) {
            pos = 0;
            pcNumber = fileName; 
        } else if( pos+1 < current4DImageName.npos) {
            pcNumber = fileName.substr(0, pos); 
        }

        specInfo1 << "PC Number: " << pcNumber ;

        double ampMin;
        double ampMax;
        double freqMin;
        double freqMax;
        
        plotController->GetWindowLevelRange( ampMin, ampMax, svkPlotGridView::AMPLITUDE );
        plotController->GetWindowLevelRange( freqMin, freqMax, svkPlotGridView::FREQUENCY );
        float lowestPoint = freqMin;
        float highestPoint = freqMax;

        if( this->sivicController->GetActive4DImageData()->IsA("svkMrsImageData")) {
			svkSpecPoint* point = svkSpecPoint::New();
			point->SetDcmHeader( this->sivicController->GetActive4DImageData()->GetDcmHeader() );

			lowestPoint = point->ConvertPosUnits(
				freqMin,
				svkSpecPoint::PTS,
				this->sivicController->GetFrequencyType()
			);

			highestPoint = point->ConvertPosUnits(
				freqMax,
				svkSpecPoint::PTS,
				this->sivicController->GetFrequencyType()
			);
			point->Delete();
        }
		string units;
		if ( this->sivicController->GetFrequencyType() == svkSpecPoint::PTS ) {
			units = "Pts";
		} else if ( this->sivicController->GetFrequencyType() == svkSpecPoint::Hz ) {
			units = "Hz";
		} else if ( this->sivicController->GetFrequencyType() == svkSpecPoint::PPM ) {
			units = "PPM";
		}

        specInfo2 << "Frequency Range: " << lowestPoint << " <--> " << highestPoint << " " << units << endl;
        specInfo2.setf(ios::scientific, ios::floatfield);            // floatfield not set
        specInfo2 << "Amplitude Range: " << ampMin << " <--> " << ampMax << endl;
        if( currentOverlayName != "" || currentMetaboliteName != "") {
            if ( fabs( this->overlayController->GetOverlayThresholdValue() ) < 1000 ) {
                specInfo2.setf(ios::fixed, ios::floatfield);            // floatfield not set
            }
            specInfo2 << "Overlay Threshold: " << this->overlayController->GetOverlayThresholdValue() << endl;
        }
    } 

    imageText->SetInput( imageInfo.str().c_str() ) ;
    specText1->SetInput( specInfo1.str().c_str() ) ;
    specText2->SetInput( specInfo2.str().c_str() ) ;

}


/*!
 *
 */
void svkSecondaryCaptureFormatterProstate::WriteCombinedWithSummaryCapture( vtkImageWriter* writer, string fileNameString, 
                                                                   int outputOption, svkImageData* outputImage, 
                                                                   bool print, bool preview ) 
{
    // Here are all the frames we may want to look at....
    int firstFrame = this->sivicController->GetActive4DImageData()->GetFirstSlice( this->orientation );
    int lastFrame = this->sivicController->GetActive4DImageData()->GetLastSlice( this->orientation );


    // Lets figure out which are our starting and ending frames
    int i = firstFrame;
    if( this->sivicController->GetActive4DImageData()->IsA("svkMrsImageData") && static_cast<svkMrsImageData*>(this->sivicController->GetActive4DImageData())->HasSelectionBox() ) {
		while(!static_cast<svkMrsImageData*>(this->sivicController->GetActive4DImageData())->IsSliceInSelectionBox(i, this->orientation) && i <= lastFrame ) {
			i++;
		}
    }

    firstFrame = i; 
    firstFrame = firstFrame < 0 ? 0 : firstFrame;

    // Now lets find the last slice inside the selection box...
    i = lastFrame;
    if( this->sivicController->GetActive4DImageData()->IsA("svkMrsImageData") && static_cast<svkMrsImageData*>(this->sivicController->GetActive4DImageData())->HasSelectionBox() ) {
		while(!static_cast<svkMrsImageData*>(this->sivicController->GetActive4DImageData())->IsSliceInSelectionBox(i, this->orientation) && i >= firstFrame ) {
			i--;
		}
	}
    lastFrame = i; 

    // If we want to only look at the current slice....
    if( outputOption == CURRENT_SLICE ) {
        firstFrame = plotController->GetSlice();
        lastFrame = firstFrame;
    }

    //  Replace * with slice number in output file name: 
    size_t pos = fileNameString.find_last_of("*");
    if ( pos != string::npos) {
        fileNameString.erase(pos);
    } else {
        pos = fileNameString.find_last_of(string("."));
    }
    string filePattern = "%s%d";
    if ( pos != string::npos && pos > 0 ) {
        writer->SetFilePrefix( fileNameString.substr(0,pos).c_str() );
        filePattern.append( fileNameString.substr(pos).c_str() );
    } else {
        writer->SetFilePrefix( fileNameString.c_str() );
    }
    writer->SetFilePattern(filePattern.c_str());

    bool flipImage = 0;
    if( writer->IsA("svkImageWriter") ) {  
        flipImage = 1;
    }

    vtkImageAppend* sliceAppender = vtkImageAppend::New();
    sliceAppender->SetAppendAxis(2);
    svkImageData* outputImageCopy1 = svkMriImageData::New();
    outputImageCopy1->SetDcmHeader( outputImage->GetDcmHeader() );
    outputImageCopy1->GetDcmHeader()->Register( outputImageCopy1 );
    this->RenderCombinedImage( firstFrame, lastFrame, outputImageCopy1, flipImage, print );
    sliceAppender->SetInput(0, outputImageCopy1 );
    firstFrame = firstFrame-2 < 0 ? 0 : firstFrame-2;
    int numSummaryImages = 0; 
    int currentSummaryImage = 1;
    sliceAppender->Update();
    outputImage->DeepCopy( sliceAppender->GetOutput() );
    outputImage->Update();
    if( preview ) {
        this->PreviewImage( outputImage );
    }
    if( !writer->IsA("svkImageWriter") ) {  
        vtkImageData* imageCopy = vtkImageData::New();
        imageCopy->DeepCopy( outputImage );
        imageCopy->Update();
        writer->SetInput( imageCopy );
        imageCopy->Delete();
    } else {
        writer->SetInput( outputImage );
    }
    writer->Write();

}
