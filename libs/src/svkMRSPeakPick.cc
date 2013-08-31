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
    this->snLimit = 40;
    this->onlyUseSelectionBox = false;
    this->resolveHeightFraction = 0.7; 
    this->resolveHeightFraction = 0.5; 
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
    this->InitPeakRanges(); 
    this->PrintPeaks(); 

    noise->Delete(); 

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->GetInput()->Update();

    return 1; 
}

/*!
 *  Now that the peaks have been identified, define some ranges.  These
 *  Ranges are not for quantification, but for other algorithms to use as a
 *  guide when evaluating peaks, i.e. auto phasing algos.     
 */
void svkMRSPeakPick::InitPeakRanges()
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    int numTimePoints = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );
    
    //  Define a reasonable width about the peak that defines it. 
    float tuple[2];
    float previousPt = FLT_MIN;   
    float resolveHeight = FLT_MIN; 

    //  when comparing the derivative, is should be larger in magnitude than the
    //  noiseSD: 
    float derivativeLimit = this->noiseSD * this->snLimit; 

    for ( int i = 0; i < this->peakVector.size(); i++ ) {

        int startPt = this->peakVector[i][0];
        int peakPt= this->peakVector[i][1]; 
        int endPt= this->peakVector[i][2]; 
        bool resetStart = false; 

        this->averageSpectrum->GetTupleValue(peakPt, tuple);
        float signal = tuple[0] - this->baselineValue;
        float peakHeight = signal;  
        resolveHeight = peakHeight * this->resolveHeightFraction;
        
        
        for ( int freq = startPt; freq <= endPt;freq++ ) {
            //cout << "freq"  << freq << endl; 

            this->averageSpectrum->GetTupleValue(freq, tuple);
            signal = tuple[0] - this->baselineValue;

            float derivative = signal - previousPt; 
            
            //  reset the starting point as far to the left as possible. 
            if ( ! resetStart ) {
                if (  freq < peakPt && derivative > derivativeLimit )  {
                //if ( signal >= resolveHeight && freq < peakPt && derivative > derivativeLimit )  {
                    //cout << "NEW START: " << " " << i << " " << freq<< endl;
                    //  reset start point
                    this->peakVector[i][0] = freq; 
                    resetStart = true; 
                }
            }
            if (  freq > peakPt && derivative < derivativeLimit )  {
            //if ( signal <= resolveHeight && freq > peakPt && derivative < derivativeLimit )  {
                //  reset end point
                this->peakVector[i][2] = freq; 
            }
            previousPt = signal; 
            
        }
    }

    return; 
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
            vector < int > peak; 
            int numPeaks = this->peakVector.size(); 
            int startPt = 0; 
            if ( numPeaks > 0 ) {
                startPt = this->peakVector[ numPeaks-1][2];  
            }
            int endPt = i; 
            this->InitPeakVector( &peak, startPt, peakPt, endPt); 
            this->peakVector.push_back(peak); 
            //cout << "Found Peak: " << peakPt << " = " << localMax << endl;
            localMax = FLT_MIN; 
            resolveHeight = FLT_MIN; 
        }
        
        previousPt = signal; 
    }

    return; 
}


/*!
 *
 */
void svkMRSPeakPick::PrintPeaks()
{
    for ( int i = 0; i < this->peakVector.size(); i++ ) {
        cout << "peak " << i << " = " << this->peakVector[i][0] << " " << this->peakVector[i][1] << " " << this->peakVector[i][2] << endl;
    }
}


/*  Initialize the peakVector, a vector of peak definitions: : 
 *      peakVector[i] = peakStartPt, peakPos, peakEndPt
 */ vector < vector < float > > peakVector;
void svkMRSPeakPick::InitPeakVector( vector<int>* peak, int startPt, int peakPt, int endPt) 
{
    peak->push_back(startPt); 
    peak->push_back(peakPt); 
    peak->push_back(endPt); 
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



