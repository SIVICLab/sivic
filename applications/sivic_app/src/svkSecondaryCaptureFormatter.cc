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


#include <svkSecondaryCaptureFormatter.h>
#include <vtkSivicController.h>


using namespace svk;


//vtkCxxRevisionMacro(svkSecondaryCaptureFormatter, "$Rev$");
vtkStandardNewMacro(svkSecondaryCaptureFormatter);


//! Constructor
svkSecondaryCaptureFormatter::svkSecondaryCaptureFormatter()
{
    imageSize[0] = 1200;
    imageSize[1] = 800;
    spectraHalfSize[0] = 500;
    spectraHalfSize[1] = 500;
    this->orientation = svkDcmHeader::AXIAL;
    this->aspect = LANDSCAPE;
}


//! Destructor
svkSecondaryCaptureFormatter::~svkSecondaryCaptureFormatter()
{
}


/*! 
 *  Pure setter method (this->x = x) 
 */     
void svkSecondaryCaptureFormatter::SetSivicController( vtkSivicController* sivicController )
{       
    this->sivicController = sivicController;
}       


/*! 
 *  Pure setter method (this->x = x) 
 */     
void svkSecondaryCaptureFormatter::SetPlotController( svkPlotGridViewController* plotController )
{       
    this->plotController = plotController;
}       


/*! 
 *  Pure setter method (this->x = x) 
 */     
void svkSecondaryCaptureFormatter::SetOverlayController( svkOverlayViewController* overlayController )
{       
    this->overlayController = overlayController;
}  


/*!
 *  Sets the model to be used which contains the data.
 */
void svkSecondaryCaptureFormatter::SetModel( svkDataModel* model )
{
    this->model = model;
}


/*!
 *
 */
void svkSecondaryCaptureFormatter::SetAspect( CaptureAspect aspect)
{
    this->aspect = aspect;
}


/*!
 *
 */
void svkSecondaryCaptureFormatter::SetOrientation( svkDcmHeader::Orientation orientation )
{
    this->orientation = orientation;
}


/*!
 *
 */
void svkSecondaryCaptureFormatter::WriteSpectraCapture( vtkImageWriter* writer, string fileNameString, int outputOption, svkImageData* outputImage, bool print ) 
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
    this->RenderSpectraImage( firstFrame, lastFrame, outputImage, flipImage );
    if( !writer->IsA("svkImageWriter") ) {  
        vtkImageData* imageCopy = vtkImageData::New();
        imageCopy->DeepCopy( outputImage );
        writer->SetInputData( imageCopy );
        imageCopy->Delete();
    } else {
        writer->SetInputData( outputImage );
    }
    writer->Write();

    if( print ) {
        this->PrintImages( fileNameString, firstFrame, lastFrame );
    }
}


void svkSecondaryCaptureFormatter::RenderSpectraImage( int firstFrame, int lastFrame, svkImageData* outputImage, bool flipImage )
{
    // First we need to prepare to renderer
    this->plotController->GetView()->TurnRendererOff( svkPlotGridView::PRIMARY );
    this->overlayController->GetView()->TurnRendererOff( svkPlotGridView::PRIMARY );
    vtkRenderWindow* window = vtkRenderWindow::New();
#if defined(linux)
    window->OffScreenRenderingOn();
#endif 
    window->AddRenderer( this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY ) );
    window->SetSize( spectraHalfSize[0],spectraHalfSize[1] );
    if(vtkActor2D::SafeDownCast(this->plotController->GetView()->GetProp( svkPlotGridView::OVERLAY_TEXT ))->GetMapper() != NULL ) {
        svkLabeledDataMapper::SafeDownCast(
              vtkActor2D::SafeDownCast(this->plotController->GetView()->GetProp( svkPlotGridView::OVERLAY_TEXT ) )
                               ->GetMapper())->GetLabelTextProperty()->SetFontSize(20);
    }

    // Now lets loop through the slices....
    vtkImageAppend* sliceAppender = vtkImageAppend::New();
    sliceAppender->SetAppendAxis(2);
    for (int m = firstFrame; m <= lastFrame; m++) {
        vtkImageData* tmpData = vtkImageData::New();
        this->sivicController->SetSlice(m);
        window->Render();
        vtkWindowToImageFilter* wtif = vtkWindowToImageFilter::New();
        wtif->SetMagnification(2);
        wtif->SetInput( window );
        wtif->Update();
        tmpData->DeepCopy( wtif->GetOutput() );
        //tmpData->Update();
        sliceAppender->SetInputData(m-firstFrame, tmpData );
        wtif->Delete();
        tmpData->Delete();
    }
    if( flipImage ) {  
        vtkImageFlip* flipper = vtkImageFlip::New();
        flipper->SetFilteredAxis( 1 );
        flipper->SetInputData( sliceAppender->GetOutput() );
        flipper->Update();
        outputImage->DeepCopy( flipper->GetOutput() );
        flipper->Delete();
    } else {
        sliceAppender->Update();
        outputImage->DeepCopy( sliceAppender->GetOutput() );
    }

    // Now we clean things up
    window->RemoveRenderer( this->plotController->GetView()->GetRenderer( svkPlotGridView::PRIMARY ) );
    this->plotController->GetView()->TurnRendererOn( svkPlotGridView::PRIMARY );
    this->overlayController->GetView()->TurnRendererOn( svkPlotGridView::PRIMARY );
    if(vtkActor2D::SafeDownCast(this->plotController->GetView()->GetProp( svkPlotGridView::OVERLAY_TEXT ))->GetMapper() != NULL ) {
        svkLabeledDataMapper::SafeDownCast(
              vtkActor2D::SafeDownCast(this->plotController->GetView()->GetProp( svkPlotGridView::OVERLAY_TEXT ) )
                               ->GetMapper())->GetLabelTextProperty()->SetFontSize(10);
    }
    window->Delete();
}


/*!
 *
 */
void svkSecondaryCaptureFormatter::WriteCombinedCapture( vtkImageWriter* writer, string fileNameString, int outputOption, svkImageData* outputImage, bool print ) 
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

    this->RenderCombinedImage( firstFrame, lastFrame, outputImage, flipImage, print );
    if( !writer->IsA("svkImageWriter") ) {  
        vtkImageData* imageCopy = vtkImageData::New();
        imageCopy->DeepCopy( outputImage );
        //imageCopy->Update();
        writer->SetInputData( imageCopy );
        imageCopy->Delete();
    } else {
        writer->SetInputData( outputImage );
    }
    writer->Write();

    if( print ) {
        this->PrintImages( fileNameString, firstFrame, lastFrame );
    }
}


/*!
 *
 */
void svkSecondaryCaptureFormatter::WriteCombinedWithSummaryCapture( vtkImageWriter* writer, string fileNameString, 
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
    sliceAppender->SetInputData(0, outputImageCopy1 );
    firstFrame = firstFrame-2 < 0 ? 0 : firstFrame-2;
    int numSummaryImages = lastFrame-firstFrame+1;
    int currentSummaryImage = 1;
    while( numSummaryImages > 6 ) {
        svkImageData* outputImageCopy2 = svkMriImageData::New();
        outputImageCopy2->SetDcmHeader( outputImage->GetDcmHeader() );
        outputImageCopy2->GetDcmHeader()->Register( outputImageCopy2 );
        this->RenderSummaryImage( firstFrame, firstFrame+5, outputImageCopy2, flipImage, print );
        firstFrame+=6;
        numSummaryImages = lastFrame-firstFrame+1;
        sliceAppender->SetInputData(currentSummaryImage, outputImageCopy2 );
        currentSummaryImage++;
        outputImageCopy2->Delete();
    }
    if( numSummaryImages > 0 ) {
        svkImageData* outputImageCopy2 = svkMriImageData::New();
        outputImageCopy2->SetDcmHeader( outputImage->GetDcmHeader() );
        outputImageCopy2->GetDcmHeader()->Register( outputImageCopy2 );
        this->RenderSummaryImage( firstFrame, lastFrame, outputImageCopy2, flipImage, print );
        sliceAppender->SetInputData(currentSummaryImage, outputImageCopy2 );
    }
    sliceAppender->Update();
    outputImage->DeepCopy( sliceAppender->GetOutput() );
    //outputImage->Update();
    if( preview ) {
        this->PreviewImage( outputImage );
    }
    if( !writer->IsA("svkImageWriter") ) {  
        vtkImageData* imageCopy = vtkImageData::New();
        imageCopy->DeepCopy( outputImage );
        //imageCopy->Update();
        writer->SetInputData( imageCopy );
        imageCopy->Delete();
    } else {
        writer->SetInputData( outputImage );
    }
    writer->Write();

}


/*!
 *
 */
void svkSecondaryCaptureFormatter::RenderCombinedImage( int firstFrame, int lastFrame, svkImageData* outputImage, bool flipImage, bool print )
{
    double plotViewport[4] =        { 0.5,
                                      0.0,
                                      1.0,
                                      0.67 };

    double overlayViewport[4] =     { 0.0,
                                      0.0, 
                                      0.5,  
                                      0.67 };

    double imageInfoViewport[4]   = { 0.0,
                                      0.67, 
                                      0.33, 
                                      0.96 };


    double spectraInfoViewport[4] = { 0.33,
                                      0.67,  
                                      1.0, 
                                      0.96 };
    double titleViewport[4] =       { 0.0,
                                      0.96,  
                                      1.0, 
                                      1.0 };
/*
    // Landscape layout #1
    double plotViewport[4] =        { (imageSize[0]-imageSize[1])/((double)imageSize[0]),
                                      0.0,
                                      1.0,
                                      1.0 };

    double overlayViewport[4] =     { 0.0,
                                      0.0, 
                                      (imageSize[0]-imageSize[1])/((double)imageSize[0]),  
                                      (imageSize[0]-imageSize[1])/((double)(imageSize[1])) };

    double imageInfoViewport[4]   = { 0.0,
                                      (2*imageSize[1]-imageSize[0])/((double)(imageSize[1])), 
                                      (imageSize[0]-imageSize[1])/((double)imageSize[0]), 
                                      (2*imageSize[1]-imageSize[0])/((double)(imageSize[1]))*1.275 };


    double spectraInfoViewport[4] = { 0.0,
                                      (2*imageSize[1]-imageSize[0])/((double)(imageSize[1]))*1.275,  
                                      (imageSize[0]-imageSize[1])/((double)imageSize[0]), 
                                      1.0 };
*/


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
    specText1->SetPosition(0.0,0.0);
    specText1->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    specText1->SetPosition2(0.48,1.0);
    specText1->GetPosition2Coordinate()->SetCoordinateSystemToNormalizedViewport();
    spectraInfoRenderer->AddActor( specText1 );

    window->AddRenderer( spectraInfoRenderer );
    vtkTextActor* specText2 = vtkTextActor::New();
    specText2->SetTextScaleModeToProp();
    specText2->SetPosition(0.5,0.0);
    specText2->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    specText2->GetPosition2Coordinate()->SetCoordinateSystemToNormalizedViewport();
    specText2->SetPosition2(0.48,1.0);
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
        //tmpData->Update();
        sliceAppender->SetInputData(m-firstFrame, tmpData );
        wtif->Delete();
    }

    if( flipImage ) {  
        vtkImageFlip* flipper = vtkImageFlip::New();
        flipper->SetFilteredAxis( 1 );
        flipper->SetInputData( sliceAppender->GetOutput() );
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

/*!
 *
 */
void svkSecondaryCaptureFormatter::WriteImageCapture( vtkImageWriter* writer, string fileNameString, int outputOption, svkImageData* outputImage, bool print, int instanceNumber ) 
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

    firstFrame = firstFrame-2 < 0 ? 0 : firstFrame;
    vtkImageAppend* sliceAppender = vtkImageAppend::New();
    sliceAppender->SetAppendAxis(2);
    firstFrame = firstFrame-2 < 0 ? 0 : firstFrame-2;
    int numSummaryImages = lastFrame-firstFrame+1;
    int remainingSummaryImages = lastFrame-firstFrame+1;
    int currentSummaryImage = 0;
    while( remainingSummaryImages > 6 ) {
        svkImageData* outputImageCopy = svkMriImageData::New();
        outputImageCopy->SetDcmHeader( outputImage->GetDcmHeader() );
        outputImageCopy->GetDcmHeader()->Register( outputImageCopy );
        this->RenderSummaryImage( firstFrame, firstFrame+5, outputImageCopy, flipImage, print );
        firstFrame+=6;
        remainingSummaryImages = lastFrame-firstFrame+1;
        sliceAppender->SetInputData(currentSummaryImage, outputImageCopy );
        currentSummaryImage++;
        outputImageCopy->Delete();
    }
    if( remainingSummaryImages > 0 ) {
        svkImageData* outputImageCopy = svkMriImageData::New();
        outputImageCopy->SetDcmHeader( outputImage->GetDcmHeader() );
        outputImageCopy->GetDcmHeader()->Register( outputImageCopy );
        this->RenderSummaryImage( firstFrame, lastFrame, outputImageCopy, flipImage, print );
        sliceAppender->SetInputData(currentSummaryImage, outputImageCopy );
    }
    sliceAppender->Update();
    outputImage->DeepCopy( sliceAppender->GetOutput() );
    //outputImage->Update();
    if( !writer->IsA("svkImageWriter") ) {  
        vtkImageData* imageCopy = vtkImageData::New();
        imageCopy->DeepCopy( outputImage );
        //imageCopy->Update();
        writer->SetInputData( imageCopy );
        imageCopy->Delete();
    } else {
        writer->SetInputData( outputImage );
    }
    writer->Write();
    if( print ) {
        this->PrintImages( fileNameString, 0, (numSummaryImages-1)/6 );
    }
}


/*!
 *
 */
void svkSecondaryCaptureFormatter::RenderSummaryImage( int firstFrame, int lastFrame, svkImageData* outputImage, bool flipImage, bool print )
{

    this->overlayController->GetView()->TurnRendererOff( svkOverlayView::PRIMARY );
    this->plotController->GetView()->TurnRendererOff( svkPlotGridView::PRIMARY );
    bool isColorBarOn = this->overlayController->GetView()->IsPropOn( svkOverlayView::COLOR_BAR );
    double titleSpace = 0.04;


    // We want to add text to the top of each image
    vtkTextActor* sliceLocationActor = vtkTextActor::New();
    this->overlayController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->AddActor( sliceLocationActor );
    if( print ) {
        sliceLocationActor->GetProperty()->SetColor(0.5,0.5,0.5);
    }

    vtkRenderWindow* window = vtkRenderWindow::New();
#if defined(linux)
    window->OffScreenRenderingOn();
#endif 
    window->AddRenderer( this->overlayController->GetView()->GetRenderer( svkPlotGridView::PRIMARY ) );
    this->overlayController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->SetRenderWindow(window);

    int numFrames = lastFrame - firstFrame + 1;
    int numRows = 2;
    if( numRows > numFrames ) {
        numRows = numFrames;
    }
    int numCols = (int)ceil( numFrames/((double)numRows) );
    int x = (imageSize[0]/numCols);
    int y = (int)((imageSize[1]*(1-titleSpace))/numRows);
    window->SetSize( x, y );
    sliceLocationActor->SetPosition(x/4,y-25);


    vtkDataSetCollection* allImages = vtkDataSetCollection::New();

    vector <vtkImageAppend*> colAppenders; 
    colAppenders.reserve( (numFrames)/numCols ); 
    vtkImageAppend* rowAppender = vtkImageAppend::New();
    rowAppender->SetAppendAxis(1);
    for (int m = firstFrame; m <= lastFrame; m++) {
        vtkRenderLargeImage* rendererToImage = vtkRenderLargeImage::New();
        double origin[3];

        if( isColorBarOn && m == firstFrame + 1 ) {
            this->overlayController->GetView()->TurnPropOff( svkOverlayView::COLOR_BAR );
        }
        // We need to reverse the slice order because of the direction of the appender.
        ostringstream position;
        this->sivicController->SetSlice( m );
        int orientationIndex = this->sivicController->GetActive4DImageData()->GetOrientationIndex( orientation );
        int sliceIndex[3] = {0,0,0};
        sliceIndex[ orientationIndex ] = this->plotController->GetSlice();
        this->sivicController->GetActive4DImageData()->GetPositionFromIndex( sliceIndex, origin );
        position << "Slice: " << this->overlayController->GetSlice()+1 << " Pos(mm): " << origin[orientationIndex];
        sliceLocationActor->SetInput( position.str().c_str() );
        vtkImageData* data = vtkImageData::New();
        allImages->AddItem( data );
    
        // Now lets use the multiwindow to get the image of the spectroscopy
        rendererToImage->SetInput( this->overlayController->GetView()->GetRenderer( svkPlotGridView::PRIMARY ) );
        window->Render();
        rendererToImage->SetMagnification(1);
        rendererToImage->Update();
        data->DeepCopy( rendererToImage->GetOutput() );
        //data->Update();
        if( (m - firstFrame) % numCols == 0 ) {
            colAppenders[(m-firstFrame)/numCols] = vtkImageAppend::New();
            colAppenders[(m-firstFrame)/numCols]->SetInputData(0, data );
            colAppenders[(m-firstFrame)/numCols]->SetAppendAxis(0);

            // In case of an odd number of slices we need to pad each
            vtkImageConstantPad* padder = vtkImageConstantPad::New();
            if( print ) {
                padder->SetConstant( 255 );
            }
            if( numCols == 1 ) { 
                padder->SetInputData(  data );
            } else {
                padder->SetInputData(  colAppenders[(m-firstFrame)/numCols]->GetOutput() );
            }
            padder->SetOutputWholeExtent(0,x*numCols-1,0,y-1,0,0);
            rowAppender->SetInputData( numRows - 1 - ((m-firstFrame)/(numCols)), padder->GetOutput() );
        } else {
            colAppenders[(m-firstFrame)/numCols]->SetInputData((m-firstFrame) % numCols, data );
        }
        rendererToImage->Delete();
    }
    rowAppender->Update();

    // Add Title
    vtkRenderer* titleRenderer = vtkRenderer::New();
    titleRenderer->SetBackground( 0.25, 0.25, 0.25 );
    vtkTextActor* titleActor = vtkTextActor::New();
    titleActor->SetInput( "SIVIC: Research Software" );
    titleActor->SetTextScaleModeToViewport();
    titleRenderer->AddActor( titleActor );
    titleActor->Delete();
    vtkRenderWindow* titleWindow = vtkRenderWindow::New();
#if defined(linux)
    titleWindow->OffScreenRenderingOn();
#endif 
    titleWindow->AddRenderer( titleRenderer );
    titleRenderer->Delete();

    vtkWindowToImageFilter* wtif = vtkWindowToImageFilter::New();
    wtif->SetMagnification(1);
    wtif->SetInput( titleWindow );
    titleWindow->SetSize( x*numCols, (int)(imageSize[1]*titleSpace) );
    titleWindow->Render();

    vtkImageAppend* titleAppender = vtkImageAppend::New();
    titleAppender->SetAppendAxis(1);
    titleAppender->SetInputData( 0, rowAppender->GetOutput() );
    titleAppender->SetInputData( 1, wtif->GetOutput() );
    wtif->Delete();
    

    vtkImageConstantPad* padder = vtkImageConstantPad::New();
    if( print ) {
        padder->SetConstant( 255 );
    }
    padder->SetInputData(  titleAppender->GetOutput() );
    padder->SetOutputWholeExtent(0,imageSize[0]-1,0,imageSize[1]-1,0,0);
    titleAppender->Delete();

    if( flipImage ) {  
        vtkImageFlip* flipper = vtkImageFlip::New();
        flipper->SetFilteredAxis( 1 );
        flipper->SetInputData( padder->GetOutput() );
        flipper->Update();
        outputImage->DeepCopy( flipper->GetOutput() );
        flipper->Delete();
    } else {
        padder->Update();
        outputImage->DeepCopy( padder->GetOutput() );
    }

    this->overlayController->GetView()->GetRenderer( svkPlotGridView::PRIMARY )->RemoveViewProp( sliceLocationActor );

    padder->Delete();
    window->Delete();
    titleWindow->Delete();
    this->overlayController->GetView()->TurnRendererOn( svkOverlayView::PRIMARY );
    this->plotController->GetView()->TurnRendererOn( svkPlotGridView::PRIMARY );
    if( isColorBarOn ) {
        this->overlayController->GetView()->TurnPropOn( svkOverlayView::COLOR_BAR );
    }
}


/*!
 *
 */
void svkSecondaryCaptureFormatter::PopulateInfoText( vtkTextActor* specText1, 
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

        // Study Date
        string studyDate = model->GetDataObject( "AnatomicalData" )->GetDcmHeader()->GetStringValue("StudyDate");

        // Temporarily removing study date just in case for phi while making screenshots
        if( studyDate.size() >= 4) {
            imageInfo << "Scan Date:  " << studyDate.substr( studyDate.size()-4, 2) << "/" 
                                     << studyDate.substr( studyDate.size()-2, 2) << "/"
                                     << studyDate.substr( 0, 4 ) << endl;
        } else {
            imageInfo << "Scan Date:  " << "?" << endl;

        }


        // Image Series
        if( model->GetDataObject( "AnatomicalData" ) ) {
            int seriesNumber = model->GetDataObject( "AnatomicalData" )->GetDcmHeader()->GetIntValue("SeriesNumber");
            imageInfo << "Image Series:  " << seriesNumber << endl;
        }

        // Image FOV
        int rows = model->GetDataObject( "AnatomicalData" )->GetDcmHeader()->GetIntValue("Rows");
        int cols = model->GetDataObject( "AnatomicalData" )->GetDcmHeader()->GetIntValue("Columns");
                        string pixelSpacing = model->GetDataObject( "AnatomicalData")->GetDcmHeader()->GetStringSequenceItemElement("PixelMeasuresSequence", 0, "PixelSpacing" , "SharedFunctionalGroupsSequence");

        pos = pixelSpacing.find_last_of("\\"); 
        if( pos == pixelSpacing.npos || pos + 1 >= pixelSpacing.npos ) {
            imageInfo << "Image FOV:  ?" << endl; 
        } else {
            imageInfo << "Image FOV:  " << atof(pixelSpacing.substr(0,pos).c_str()) * cols << " X "
                               << atof(pixelSpacing.substr(pos+1).c_str()) * rows << endl;
        }

        // Image Coil
        string imageCoil = "?";
        if(model->GetDataObject( "AnatomicalData")->GetDcmHeader()->GetNumberOfItemsInSequence( "MRReceiveCoilSequence") > 0 ) {
			imageCoil = model->GetDataObject( "AnatomicalData")->GetDcmHeader()->GetStringSequenceItemElement("MRReceiveCoilSequence", 0, "ReceiveCoilName" , "SharedFunctionalGroupsSequence");
        }


        imageInfo << "Image Coil:  " << imageCoil << endl << endl;


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
        specInfo1 << "CSI Slice No:  " << this->plotController->GetSlice() + 1 << endl;
        double sliceLocation[3];

        // Slice Position:
        int sliceIndex[3] = {0, 0, 0 };
        int orientationIndex = this->sivicController->GetActive4DImageData()->GetOrientationIndex( orientation );
        sliceIndex[ orientationIndex ] = this->plotController->GetSlice();
        this->sivicController->GetActive4DImageData()->GetPositionFromIndex( sliceIndex, sliceLocation );
        specInfo1 << "CSI Slice Pos:  " << sliceLocation[orientationIndex] << endl;
        
        if( this->sivicController->GetActive4DImageData()->IsA("svkMrsImageData")) {
			// Coil
			string specCoil = this->sivicController->GetActive4DImageData()->GetDcmHeader()->GetStringSequenceItemElement("MRReceiveCoilSequence", 0, "ReceiveCoilName" , "SharedFunctionalGroupsSequence");
			specInfo1 << "Coil:  " << specCoil << endl;
			string echoTime = this->sivicController->GetActive4DImageData()->GetDcmHeader()->GetStringSequenceItemElement("MREchoSequence", 0, "EffectiveEchoTime" , "SharedFunctionalGroupsSequence");
			specInfo1 << "TE:  " << echoTime << " ms" << endl;
        }


        pos = currentMetaboliteName.find_last_of("/"); 
        if( pos == currentMetaboliteName.npos ) {
            pos = 0;
        } else if( pos+1 < currentMetaboliteName.npos) {
            pos++;
        }
        specInfo1 << "Metabolites: " << svkUCSFUtils::GetMetaboliteName( currentMetaboliteName ).c_str() << endl;
        specInfo1 << "Metabolites File: " << endl << " " <<  currentMetaboliteName.substr(pos) << endl; 

        // Get The Spectra Name
        pos = current4DImageName.find_last_of("/");
        if( pos == current4DImageName.npos ) {
            pos = 0;
        } else if( pos+1 < current4DImageName.npos) {
            pos++;
        }
        specInfo1 << "CSI File: " << endl << " " << current4DImageName.substr(pos) << endl;

        // Selected Region 
        // Center
		double center[3];
        if( this->sivicController->GetActive4DImageData()->IsA("svkMrsImageData")) {
			svkMrsImageData::SafeDownCast(this->sivicController->GetActive4DImageData())->GetSelectionBoxCenter( center );
			specInfo2 << "Selection Center RAS: " << center[0] << " " << center[1] << " " << center[2] << "mm"<< endl;
			// Size
			float dims[3];

			svkMrsImageData::SafeDownCast(this->sivicController->GetActive4DImageData())->GetSelectionBoxDimensions( dims );
			specInfo2 << "Selected Region:  " << dims[0]*dims[1]*dims[2] / 1000.0 << "cc" << endl;

			specInfo2 << "Size RAS: " << dims[0] << " " << dims[1] << " " << dims[2] << "mm"<< endl;
        }

        double* spacing = this->sivicController->GetActive4DImageData()->GetSpacing();
        specInfo2 << "CSI Resolution:" << spacing[0] * spacing[1] * spacing[2] / 1000.0 << "cc" <<  endl;
        specInfo2 << "Size RAS: " << spacing[0] << " " << spacing[1] << " " << spacing[2] << "mm" <<  endl;
        this->sivicController->GetActive4DImageData()->GetImageCenter( center );
        specInfo2 << "Spectra Center RAS: " << center[0] << " " << center[1] << " " << center[2] << " mm"<< endl;

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

void svkSecondaryCaptureFormatter::PrintImages( string fileNameString, int startImage, int endImage ) 
{
    for (int m = startImage; m <= endImage; m++) {
        string fileNameStringTmp = fileNameString; 
        size_t pos = fileNameStringTmp.find_last_of("*");
        ostringstream frameNum;
        frameNum <<   m-startImage;
        if ( pos != string::npos) {
             fileNameStringTmp.replace(pos, 1, frameNum.str()); 
        } else {
            size_t pos = fileNameStringTmp.find_last_of(".");
            fileNameStringTmp.replace(pos, 1, frameNum.str() + ".");
        }
        stringstream printCommand;
		svkUtils::PrintFile(fileNameStringTmp.c_str(),this->sivicController->GetPrinterName().c_str()); 
        remove( fileNameStringTmp.c_str()); 
    }

}


/*!
 *  Preview an image.
 */
void svkSecondaryCaptureFormatter::PreviewImage( svkImageData* image )
{
    svkImageData* orthogonalCopy = svkMriImageData::New();
    /*
     * We are going to make a shallow copy of the data and set the dcos to unity for the preview.
     * This way we can see the way the data is ordered without the influence of the dcos, which
     * is irrelevant to the rendering of the secondary capture.
     */
    orthogonalCopy->ShallowCopy(image);
    double dcos[3][3] = {{1,0,0}, {0,1,0}, {0,0,1}};
    orthogonalCopy->SetDcos(dcos);
    vtkRenderer* ren = vtkRenderer::New();
    ren->SetBackground(0.1,0.2,0.4);
    vtkRenderWindow* window = vtkRenderWindow::New();
    vtkRenderWindowInteractor* rwi = window->MakeRenderWindowInteractor();
    svkImageViewer2* viewer = svkImageViewer2::New(); 
    viewer->SetRenderWindow( window );
    viewer->SetRenderer( ren );
    window->AddRenderer( ren);  
    window->SetSize(600,600); 
    viewer->SetInputData( orthogonalCopy );
    viewer->SetupInteractor( rwi );
    viewer->SetOrientation(orthogonalCopy->GetDcmHeader()->GetOrientationType());
    viewer->GetImageActor()->InterpolateOff();
    viewer->GetRenderer()->ResetCamera();
    viewer->ResetCamera();
    int* extent = orthogonalCopy->GetExtent();
    vtkTextActor* textActor = vtkTextActor::New();
    textActor->SetTextScaleModeToViewport();
    textActor->GetTextProperty()->SetFontSize(15);
    textActor->GetTextProperty()->SetColor(1,0.5,0);
    textActor->GetTextProperty()->BoldOn();
    ren->AddActor( textActor );
    for( int i=extent[4]; i <= extent[5]; i++ ) {
        stringstream textString;
        textString << " PREVIEW SLICE: " << i+1 << "/" << extent[5]+1 <<". Close window to continue....   " << endl;
        textActor->SetInput( textString.str().c_str() );
        viewer->SetSlice(i);
        window->Render();
        rwi->Start();
    }
    
    window->Delete();
    viewer->Delete();
    ren->Delete();
    textActor->Delete();
}    
