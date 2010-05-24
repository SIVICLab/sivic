/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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
 */



#include <svkMrsImageFFT.h>


using namespace svk;


vtkCxxRevisionMacro(svkMrsImageFFT, "$Rev$");
vtkStandardNewMacro(svkMrsImageFFT);


svkMrsImageFFT::svkMrsImageFFT()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    //  Start index for update extent
    this->updateExtent[0] = -1;  
    this->updateExtent[1] = -1;  
    this->updateExtent[2] = -1;  
    this->updateExtent[3] = -1;  
    this->updateExtent[4] = -1;  
    this->updateExtent[5] = -1;  
    this->domain = SPECTRAL;
    this->mode   = FORWARD;
    this->preCorrectCenter = false;
    this->postCorrectCenter = false;
    this->prePhaseShift[0] = 0;
    this->prePhaseShift[1] = 0;
    this->prePhaseShift[2] = 0;
    this->postPhaseShift[0] = 0;
    this->postPhaseShift[1] = 0;
    this->postPhaseShift[2] = 0;
}


svkMrsImageFFT::~svkMrsImageFFT()
{
}


/*!
 * Method for converting between vtkDataArrays and vtkImageComplex.
 */
void svkMrsImageFFT::ConvertArrayToImageComplex( vtkDataArray* spectrum, vtkImageComplex* imageComplexSpectrum ) 
{
    int numTuples = spectrum->GetNumberOfTuples();
    for( int i = 0; i < spectrum->GetNumberOfTuples(); i++ ) {
       double tuple[2];
       spectrum->GetTuple( i, tuple );
       imageComplexSpectrum[i].Real = tuple[0]; 
       imageComplexSpectrum[i].Imag = tuple[1]; 
    }
}


/*! 
 *
 */
int svkMrsImageFFT::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    int wholeExtent[6]; 
    this->GetInput()->GetWholeExtent( wholeExtent ); 
    
    bool useWholeExtent = false; 
    //  If the specified update extent is outside the whole extent, just use
    //  the whole extent:

    //  Lower bounds of extent:
    for (int i = 0; i < 6; i+=2) {
        if ( updateExtent[i] < wholeExtent[i] ) {
            useWholeExtent = true;
        }
    }

    //  upper bounds of extent:
    for (int i = 1; i < 6; i+=2) {
        if ( updateExtent[i] > wholeExtent[i] ) {
            useWholeExtent = true;
        }
    }

    if (useWholeExtent) {
        wholeExtent[1] =  wholeExtent[1] - 1;  
        wholeExtent[3] =  wholeExtent[3] - 1;  
        wholeExtent[5] =  wholeExtent[5] - 1;  
        for (int i = 0; i < 6; i++) {
            vtkDebugMacro(<<this->GetClassName() << " Whole Extent " << wholeExtent[i]);
            this->updateExtent[i] = wholeExtent[i];  
        }
    }
    return 1;
}

/*! 
 *
 */
int svkMrsImageFFT::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    int returnValue;
    vtkImageFourierFilter* fourierFilter = NULL;
    if( this->mode == REVERSE ) {
        fourierFilter = vtkImageRFFT::New();
    } else {
        fourierFilter = vtkImageFFT::New();
    }
    fourierFilter->SetDimensionality(3);
    if( this->domain == SPECTRAL ) {
        returnValue = RequestDataSpectral( request, inputVector, outputVector, fourierFilter ); 
    } else if ( this->domain == SPATIAL ) {
        returnValue = RequestDataSpatial( request, inputVector, outputVector, fourierFilter ); 
    }
    fourierFilter->Delete();
    return returnValue;
}


/*! 
 *
 */
int svkMrsImageFFT::RequestDataSpatial( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector, vtkImageFourierFilter* fourierFilter )
{
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0));
    int numberOfPoints = data->GetCellData()->GetArray(0)->GetNumberOfTuples();
    int numChannels  = data->GetDcmHeader()->GetNumberOfCoils();
    int numTimePoints  = data->GetDcmHeader()->GetNumberOfTimePoints();
    vtkImageData* currentData = NULL;
    svkImageLinearPhase* prePhaseShifter = svkImageLinearPhase::New();                
    svkImageLinearPhase* postPhaseShifter = svkImageLinearPhase::New();                
    svkImageFourierCenter* preIfc = svkImageFourierCenter::New(); 
    svkImageFourierCenter* postIfc = svkImageFourierCenter::New();
    double progress = 0;
    
    vtkImageData* pointImage = vtkImageData::New();
    for( int timePt = 0; timePt < numTimePoints; timePt++ ) {
        for( int channel = 0; channel < numChannels; channel++ ) {
            ostringstream progressStream;
            progressStream <<"Executing Spatial Recon for Time Point " << timePt+1 << "/"
                           << numTimePoints << " and Channel: " << channel+1 << "/" << numChannels;
            this->SetProgressText( progressStream.str().c_str() );
            for( int point = 0; point < numberOfPoints; point++ ) {
                if( point%16==0 ) {
                    progress = (point+1)/((double)numberOfPoints);
                    this->UpdateProgress( progress );
                }
                data->GetImage( pointImage, point, timePt, channel );
                pointImage->Modified();

                currentData = pointImage;

                // Lets apply a phase shift....
                if( this->prePhaseShift[0] != 0 
                    && this->prePhaseShift[1] != 0 
                    && this->prePhaseShift[2] != 0 ) {
                    prePhaseShifter->SetShiftWindow( this->prePhaseShift );
                    prePhaseShifter->SetInput( currentData ); 
                    currentData = prePhaseShifter->GetOutput();
                }

                // And correct the center....
                if( this->preCorrectCenter ) {
                    preIfc->SetReverseCenter( true );
                    preIfc->SetInput(currentData);
                    currentData = preIfc->GetOutput();
                }

                // Do the Fourier Transfort
                fourierFilter->SetInput(currentData);
                currentData = fourierFilter->GetOutput();

                // And correct the center....
                if( this->postCorrectCenter ) {
                    postIfc->SetInput(currentData);
                    currentData = postIfc->GetOutput();
                }

                // Lets apply a phase shift....
                if( this->postPhaseShift[0] != 0 
                    && this->postPhaseShift[1] != 0 
                    && this->postPhaseShift[2] != 0 ) {
                    postPhaseShifter->SetShiftWindow( this->postPhaseShift );
                    postPhaseShifter->SetInput( currentData ); 
                    currentData = postPhaseShifter->GetOutput();
                }
                currentData->Update();
                data->SetImage( currentData, point, timePt, channel );
            }
        }
    }
    if( preIfc != NULL ) {
        preIfc->Delete(); 
        preIfc = NULL; 
    }
    if( postIfc != NULL ) {
        postIfc->Delete(); 
        postIfc = NULL; 
    }
    if( prePhaseShifter != NULL ) {
        prePhaseShifter->Delete(); 
        prePhaseShifter = NULL; 
    }
    if( postPhaseShifter != NULL ) {
        postPhaseShifter->Delete(); 
        postPhaseShifter = NULL; 
    }
    pointImage->Delete();

    //  Update the DICOM header to reflect the spectral domain changes:
    if( this->mode == REVERSE ) {
        data->GetDcmHeader()->SetValue( "SVK_ColumnsDomain", "SPACE" );
        data->GetDcmHeader()->SetValue( "SVK_RowsDomain",    "SPACE" );
        data->GetDcmHeader()->SetValue( "SVK_SliceDomain",   "SPACE" );
    } else {
        data->GetDcmHeader()->SetValue( "SVK_ColumnsDomain", "KSPACE" );
        data->GetDcmHeader()->SetValue( "SVK_RowsDomain",    "KSPACE" );
        data->GetDcmHeader()->SetValue( "SVK_SliceDomain",   "KSPACE" );
    }

    //  Update Origin and Per Frame Functional Groups for voxel shift:
    this->UpdateOrigin(); 

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->GetInput()->Update();
    return 1; 
}


/*! 
 *  Given a fractional voxel shift (prePhaseShift)  along each of the axes, 
 *  calculate a new TOPLC and reset the PerFrameFunctionalGroups:
 */
void svkMrsImageFFT::UpdateOrigin() 
{

    svkImageData* data = this->GetImageDataInput(0);

    int numSlices = data->GetDcmHeader()->GetNumberOfSlices();
    int numTimePts = data->GetDcmHeader()->GetNumberOfTimePoints();
    int numCoils = data->GetDcmHeader()->GetNumberOfCoils();
    double dcos[3][3]; 
    data->GetDcmHeader()->GetDataDcos( dcos ); 

    double pixelSpacing[3]; 
    data->GetDcmHeader()->GetPixelSpacing( pixelSpacing ); 

    double toplc[3]; 
    data->GetDcmHeader()->GetOrigin( toplc, 0 ); 

    //  Calculate new toplc
    //  This is so far specific to GE RECON which has 1/2 voxel shift by default
    double shift[3]; 
    for (int i = 0; i < 3; i++ ){
        shift[i] = this->prePhaseShift[i] + 0.5; 
        for(int j = 0; j < 3; j++ ){
            toplc[i] -= ( dcos[j][i] * shift[j] * pixelSpacing[j] );
        }
    }

    data->GetDcmHeader()->InitPerFrameFunctionalGroupSequence(
        toplc, pixelSpacing, dcos, numSlices, numTimePts, numCoils 
    );

    //  Now displace by 1/2 voxel to get Cell corner for svkImageDataOrigin:
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            toplc[i] -= ( pixelSpacing[j]/2 ) * dcos[j][i];
        }
    }
    data->SetOrigin( toplc );
}


/*! 
 *
 */
int svkMrsImageFFT::RequestDataSpectral( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector, vtkImageFourierFilter* fourierFilter )
{
    //  Iterate through spectral data from all cells.  Eventually for performance I should do this by visible

    svkImageData* data = this->GetImageDataInput(0); 

    //  Extent initially, and catch up with invisible extents after rerendering (modified update).    
    int spatialDims[3]; 
    data->GetDimensions( spatialDims );
    spatialDims[0] -= 1;
    spatialDims[1] -= 1;
    spatialDims[2] -= 1;
    int numFrequencyPoints = data->GetCellData()->GetNumberOfTuples();
    int numComponents = data->GetCellData()->GetNumberOfComponents();
    int numCoils = data->GetDcmHeader()->GetNumberOfCoils();
    int numTimePts = data->GetDcmHeader()->GetNumberOfTimePoints();
    double progress = 0;
    int ranges[3];
    ranges[0] = this->updateExtent[1]-this->updateExtent[0]+1;
    ranges[1] = this->updateExtent[3]-this->updateExtent[2]+1;
    ranges[2] = this->updateExtent[5]-this->updateExtent[4]+1;
    int denominator = ranges[2] * ranges[0] * ranges[1] + ranges[1] * ranges[0] + ranges[0];

    for( int timePt = 0; timePt < numTimePts; timePt++ ) { 
        for( int coilNum = 0; coilNum < numCoils; coilNum++ ) { 
            ostringstream progressStream;
            progressStream <<"Executing FFT for Time Point " << timePt+1 << "/"
                           << numTimePts << " and Channel: " << coilNum+1 << "/" << numCoils;
            this->SetProgressText( progressStream.str().c_str() );
            for (int z = this->updateExtent[4]; z <= this->updateExtent[5]; z++) {
                progress = (((z-this->updateExtent[4]) * (ranges[0]) * (ranges[1]) ) )/((double)denominator);
                this->UpdateProgress( progress );
                for (int y = this->updateExtent[2]; y <= this->updateExtent[3]; y++) {
                    for (int x = this->updateExtent[0]; x <= this->updateExtent[1]; x++) {

                        vtkFloatArray* spectrum = static_cast<vtkFloatArray*>(
                                            svkMrsImageData::SafeDownCast(data)->GetSpectrum( x, y, z, timePt, coilNum) );

                        vtkImageComplex* imageComplexTime = new vtkImageComplex[ numFrequencyPoints ];
                        this->ConvertArrayToImageComplex( spectrum, imageComplexTime );
    
                        vtkImageComplex* imageComplexFrequency = new vtkImageComplex[ numFrequencyPoints ];
                        fourierFilter->ExecuteFft( imageComplexTime, imageComplexFrequency, numFrequencyPoints ); 
    
                        // Lets modify the data, putting 0 frequency at the center
                        for (int i = 0; i < numFrequencyPoints; i++) {
                            if( i > numFrequencyPoints/2 ) {
                                spectrum->SetTuple2( i - numFrequencyPoints/2-1, 
                                                    imageComplexFrequency[i].Real,  
                                                    imageComplexFrequency[i].Imag );
                            } else {
                                spectrum->SetTuple2( i + numFrequencyPoints/2-1, 
                                                    imageComplexFrequency[i].Real,  
                                                    imageComplexFrequency[i].Imag );
                            }
    
                        }
                    }
    
                }
            }
        }
    }

    //  Update the DICOM header to reflect the spectral domain changes:
    if( this->mode == REVERSE ) {
        string domain("TIME");
        data->GetDcmHeader()->SetValue( "SignalDomainColumns", domain );
    } else {
        string domain("FREQUENCY");
        data->GetDcmHeader()->SetValue( "SignalDomainColumns", domain );
    }

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->GetInput()->Update();
    return 1; 
} 


/*!
 *
 */
void svkMrsImageFFT::SetFFTDomain( FFTDomain domain )
{
    this->domain = domain;
}


/*!
 *
 */
void svkMrsImageFFT::SetFFTMode( FFTMode mode )
{
    this->mode = mode;
}


/*!
 *  Should we correct for an offset center before FT? 
 */
void svkMrsImageFFT::SetPreCorrectCenter( bool preCorrectCenter )
{
    this->preCorrectCenter = preCorrectCenter;
}


/*!
 *  Should we correct for an offset center after FT? 
 */
void svkMrsImageFFT::SetPostCorrectCenter( bool postCorrectCenter )
{
    this->postCorrectCenter = postCorrectCenter;
}


/*!
 *
 */
void svkMrsImageFFT::SetPrePhaseShift( double prePhaseShift[3] )
{
    this->prePhaseShift[0] = prePhaseShift[0];
    this->prePhaseShift[1] = prePhaseShift[1];
    this->prePhaseShift[2] = prePhaseShift[2];
}


/*!
 *
 */
void svkMrsImageFFT::SetPostPhaseShift( double postPhaseShift[3] )
{
    this->postPhaseShift[0] = postPhaseShift[0];
    this->postPhaseShift[1] = postPhaseShift[1];
    this->postPhaseShift[2] = postPhaseShift[2];
}


/*! 
 *  Sets the extent over which the phasing should be applied.      
 *  Takes 2 sets of x,y,z indices that specify the extent range 
 *  in 3D.  
 */
void svkMrsImageFFT::SetUpdateExtent(int* start, int* end)
{
    this->updateExtent[0] =  start[0];  
    this->updateExtent[1] =  end[0];  
    this->updateExtent[2] =  start[1];  
    this->updateExtent[3] =  end[1];  
    this->updateExtent[4] =  start[2];  
    this->updateExtent[5] =  end[2];  

    /*  
     *  set modified time so that subsequent calls to Update() call RequestInformation() 
     *  and refresh the extent 
     */
    this->Modified();
}


/*!
 *
 */
int svkMrsImageFFT::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}
