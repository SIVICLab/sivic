/*
 *  Copyright © 2009-2013 The Regents of the University of California.
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



#include <svkMRSPeakPick.h>
#include <svkMRSNoise.h>


using namespace svk;


vtkCxxRevisionMacro(svkMRSPeakPick, "$Rev$");
vtkStandardNewMacro(svkMRSPeakPick);


svkMRSPeakPick::svkMRSPeakPick()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    this->noiseSD = 0.0;
    this->baselineValue = FLT_MAX;
    this->snLimit = 3;
    this->onlyUseSelectionBox = false;
    this->resolveHeightFraction = 0.7; 
}


svkMRSPeakPick::~svkMRSPeakPick()
{
}


/*! 
 *
 */
int svkMRSPeakPick::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    return 1;
}


/*! 
 *
 */
int svkMRSPeakPick::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 

    //  Initialize the noiseSD and baseline values from the average magnitude spectrum: 
    svkMRSNoise* noise = svkMRSNoise::New(); 
    noise->SetInput( data ); 
    if ( this->onlyUseSelectionBox ) {
        noise->OnlyUseSelectionBox();
    }
    noise->Update();
    this->SetNoiseSD( noise->GetMagnitudeNoiseSD() );
    this->SetBaselineValue( noise->GetMagnitudeNoiseSD() );

    this->averageSpectrum = noise->GetAverageMagnitudeSpectrum(); 

    this->PickPeaks();

    noise->Delete(); 

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->GetInput()->Update();

    return 1; 
}


/*!
 *  Returns the average value of noise standard deviation.  The noise SD is calculated for each voxel
 *  and the average of the individual SDs is returned. 
 */
void svkMRSPeakPick::PickPeaks()
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    int numTimePoints = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );
    
    //  First calculate the value for a voxel, then the SD for that same voxel
    float tuple[2];
    float previousPt = FLT_MIN;   
    int   peakPt; 
    float localMax = FLT_MIN; 
    float resolveHeight = FLT_MIN; 

    //  Loop over points in average magnitude spectrum to locate peaks:  
    //  A peak is defined by a local maximum.  Reset once intensity drops
    //  below peak half height (fwhm resolved peaks).  
    for ( int i = 0; i < numTimePoints; i++ ) {

        this->averageSpectrum->GetTupleValue(i, tuple);
        float signal = tuple[0] - this->baselineValue;
        float derivative = signal - previousPt; 

        //  is the current point a local maximum?   Check that signal is: 
        //      1. > SN limit
        //      2. > previous value of localMax
        //      3. that the signal derivative is positive (this avoids situation where
        //          localMax gets reset on the falling side of a very large peak
        if ( signal > this->snLimit * this->noiseSD && signal > localMax && derivative > 0 ) { 
            localMax = signal;  
            peakPt = i; 
            resolveHeight = localMax * this->resolveHeightFraction;
            //cout << "   new peak: " << i << " = " << tuple[0] << " " << signal << " hh: " << resolveHeight << endl;
        }

        //  Once signal drops below half height of local max, grab the local max value and reset
        //  variables to search for next peak:     
        if ( signal <= resolveHeight ) { 
            cout << "Found Peak: " << peakPt << " = " << localMax << endl;
            localMax = FLT_MIN; 
            resolveHeight = FLT_MIN; 
        }
        
        previousPt = signal; 
    }

    return; 
}



/*! 
 *  Set the noise SD
 */
float svkMRSPeakPick::SetNoiseSD(float noise )
{
    this->noiseSD = noise;
}


/*! 
 *  Set the fractional resolve height.  For example, does
 *  a peak need to be resolved at fwhh to be detected.  If
 *  so set to 0.5. If the peak only needs to be resolved at
 *  0.7 of max then set to 0.7.  
 */
float svkMRSPeakPick::SetResolutionHeightFraction(float resolveHeightFraction)
{
    if (resolveHeightFraction < 0. || resolveHeightFraction > 1. ) {
        cout << "Warning, resolve height must be between 0 and 1.  Using default value" << endl;
    }
    this->resolveHeightFraction = resolveHeightFraction;
}



/*! 
 *  Get the mean value of the baseline in the window used for 
 *  SD calc. 
 */
float svkMRSPeakPick::SetBaselineValue(float baseline)
{
    this->baselineValue = baseline;
}


/*! 
 *  Sets the linear phase to apply to spectra.
 */
void svkMRSPeakPick::OnlyUseSelectionBox()
{
    this->onlyUseSelectionBox = true;
}


/*!
 *
 */
int svkMRSPeakPick::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


/*!
 *  PrintSelf method calls parent class PrintSelf, then prints the dcos.
 *
 */
void svkMRSPeakPick::PrintSelf( ostream &os, vtkIndent indent )
{
    Superclass::PrintSelf( os, indent );
    os << "only use selection box:" << this->onlyUseSelectionBox<< endl;
    os << "noiseSD               :" << this->noiseSD<< endl;
}



