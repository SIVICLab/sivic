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
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */



#include <svkMRSPeakPick.h>
#include <svkMRSNoise.h>
#include <cmath>


using namespace svk;


//vtkCxxRevisionMacro(svkMRSPeakPick, "$Rev$");
vtkStandardNewMacro(svkMRSPeakPick);


svkMRSPeakPick::svkMRSPeakPick()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    this->noiseSD = 0.0;
    this->baselineValue = FLT_MAX;
    this->snLimit = 20;
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
    noise->SetInputData( data ); 
    if ( this->onlyUseSelectionBox ) {
        noise->OnlyUseSelectionBox();
    }
    noise->Update();
    this->SetNoiseSD( noise->GetMagnitudeNoiseSD() );
    this->SetBaselineValue( noise->GetMagnitudeMeanBaseline() );

    this->averageSpectrum = noise->GetAverageMagnitudeSpectrum(); 

    this->PickPeaks();
    cout << "initial peak estimates"  << endl;
    this->PrintPeaks(); 

    this->RefinePeakRanges(); 
    cout << "refined peak estimates"  << endl;
    this->PrintPeaks(); 

    noise->Delete(); 

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->Update();

    return 1; 
}

/*!
 *  Now that the peaks have been broadly identified, refine the start/stop range.  
 *  These ranges man be used for quantification, but for other algorithms to use 
 *  as a guide when evaluating peaks, i.e. auto phasing algos.     
 */
void svkMRSPeakPick::RefinePeakRanges()
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    int numTimePoints = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );
    
    //  Define a reasonable width about the peak that defines it. 
    double tuple[2];
    float previousPt = FLT_MIN;   
    float resolveHeight = FLT_MIN; 
    float derivative; 

    //  When comparing the derivative, it should be larger in magnitude than the
    //  noiseSD: 
    float derivativeLimit = this->noiseSD; 
    float thresholdSignal = this->noiseSD * this->snLimit; 

    for ( int i = 0; i < this->peakVector.size(); i++ ) {

        //  Get the current imprecise peak range 
        int startPt = this->peakVector[i][0];
        int peakPt= this->peakVector[i][1]; 
        int endPt= this->peakVector[i][2]; 

        // Get the "resole height" for the current peak 
        this->averageSpectrum->GetTuple(peakPt, tuple);
        float signal = tuple[0] - this->baselineValue;
        float peakHeight = signal;  
        resolveHeight = peakHeight * this->resolveHeightFraction;
        bool wasStartPtReset = false; 
       
        // scan over initial range estimate
        for ( int freq = startPt; freq <= endPt;freq++ ) {

            this->averageSpectrum->GetTuple(freq, tuple);
            signal = tuple[0] - this->baselineValue;
            derivative = signal - previousPt; 
            
            //  ====================================================
            //  Refine Start Point
            //  ====================================================
            //  Reset the starting point for the peak.  The initial peak range estimates
            //  are most likely too broad.  
            if ( freq < peakPt ) {

                //  if there is a minor peak to the left of the peak, don't reset the start 
                //  on its rising edge.  If the signal drops again before the peak in question,
                //  then try to reset the start point again. 
                if ( derivative < derivativeLimit ) {
                        wasStartPtReset = false; 
                }

                //  if the start point hasn't been found, see if current point meets
                //  requirements. 
                if ( ! wasStartPtReset ) {
                    if (  signal > thresholdSignal && signal > derivativeLimit )  {
                        //if (  freq < peakPt && derivative > derivativeLimit )  {
                        //if ( signal >= resolveHeight && freq < peakPt && derivative > derivativeLimit )  {
                        //cout << "NEW START: " << " " << i << " " << freq<< endl;
                        //  reset start point
                        this->peakVector[i][0] = freq; 
                        wasStartPtReset = true; 
                    }
                }
            }

            //  ====================================================
            //  Refine End Point
            //  ====================================================
            if (  freq > peakPt && signal > thresholdSignal && signal < derivativeLimit )  {
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
    double tuple[2];
    float previousPt = FLT_MIN;   
    int   peakPt = -1; 
    float localMax = FLT_MIN; 
    float resolveHeight = FLT_MIN; 
    cout << "NOISE: " << this->noiseSD << endl;
    cout << "BL: " << this->baselineValue << endl;
    float thresholdSignal = (this->snLimit * this->noiseSD) + this->baselineValue; 
    //float derivativeLimit = 0; 
    float derivativeLimit = this->noiseSD * 2; //   ignore derivatives less than the 2*noise
    bool foundPossiblePeak = false; 

    //  Loop over points in average magnitude spectrum to locate peaks:  
    //  A peak is defined by a local maximum.  Reset once intensity drops
    //  below peak half height (fwhm resolved peaks).  
    for ( int i = 0; i < numTimePoints; i++ ) {

        this->averageSpectrum->GetTuple(i, tuple);
        float signal = tuple[0] - this->baselineValue;
        float derivative = signal - previousPt; 

        //  is the current point a local maximum?   Check that signal is: 
        //      1. > SN limit
        //      2. > previous value of localMax
        //      3. that the signal derivative is positive (this avoids situation where
        //          localMax gets reset on the falling side of a very large peak
        //cout << "CHECK PICK: " << i << " " << signal << "(" << tuple[0]<< ") vs " << thresholdSignal ;
        //cout << " and " << localMax << " D " << derivative << " vs " << derivativeLimit << endl;
        if ( signal > thresholdSignal && signal > localMax && derivative > derivativeLimit ) { 
            localMax = signal;  
            peakPt = i; 
            resolveHeight = localMax * this->resolveHeightFraction;
            //cout << "FOUND POTENTIAL PEAK: " << localMax  << " rh " << resolveHeight << endl;
            foundPossiblePeak = true; 
        }

        //  Once signal drops 
        //      1. below resolve limit of local max (e.g. half height (.5), or as set) 
        //      2. and signal > SN limit 
        //      3. and the derivative is negative, 
        //  grab the local max value and reset
        //  variables to search for next peak:     
        //  only after a possible peak has been found 
        if ( (signal <= resolveHeight) && (foundPossiblePeak == true) ) {

            while ( derivative < -1 * derivativeLimit  && signal > thresholdSignal ) {
                i = i + 1;  
                this->averageSpectrum->GetTuple(i, tuple);
                signal = tuple[0] - this->baselineValue;
                derivative = signal - previousPt; 
                previousPt = signal; 
            }
            i = i - 1; 
            this->averageSpectrum->GetTuple(i, tuple);
            signal = tuple[0] - this->baselineValue;
            derivative = signal - previousPt; 

            vector < int > peak; 
            int numPeaks = this->peakVector.size(); 
            int startPt = 0; 
            if ( numPeaks > 0 ) {
                startPt = this->peakVector[ numPeaks-1][2];  
            }
            int endPt = i; 
            //cout<< endl << "FOUND A PEAK: " << peakPt << endl << endl;
            this->InitPeakVector( &peak, startPt, peakPt, endPt); 
            this->peakVector.push_back(peak); 
            this->peakHeightVector.push_back(localMax); 
            localMax = FLT_MIN; 
            resolveHeight = FLT_MIN; 
            foundPossiblePeak = false; 
        }

        previousPt = signal; 
        
    }

    return; 
}


/*!
 *  Returns the real valued peak height in the range defined for the 
 *  specified peak number. 
 */
float svkMRSPeakPick::GetPeakHeight( vtkFloatArray* spectrum, int peakNum )
{

    float peakHeight = FLT_MIN; 
    int startPt = this->peakVector[peakNum][0];
    int endPt   = this->peakVector[peakNum][2];
    double tuple[2];     

    //int peakPt = this->peakVector[peakNum][1];
    //spectrum->GetTupleValue(peakPt, tuple);
    //return tuple[0];     

    for ( int i = startPt; i <= endPt; i++ ) {
        spectrum->GetTuple(i, tuple);
        if ( tuple[0] > peakHeight ) {
            peakHeight = tuple[0];  
        }
    }
    //cout << "range: " << startPt <<  " -> " << endPt << endl;
    return peakHeight; 
}


/*!
 *  Returns the real valued peak height in the range defined for the 
 *  specified peak number. 
 */
float svkMRSPeakPick::GetPeakHeightAtPeakPos( vtkFloatArray* spectrum, int peakNum )
{

    float peakHeight; 
    int peakPt = this->peakVector[peakNum][1];
    double tuple[2];     

    spectrum->GetTuple(peakPt, tuple);
    peakHeight = tuple[0];  

    return peakHeight; 
}


/*!
 *  Returns the real valued peak height in the range defined for the 
 *  specified peak number. 
 */
float svkMRSPeakPick::GetPeakArea( vtkFloatArray* spectrum, int peakNum )
{

    float peakArea = 0; 
    int startPt = this->peakVector[peakNum][0];
    int endPt   = this->peakVector[peakNum][2];
    double tuple[2];     
    for ( int i = startPt; i <= endPt; i++ ) {
        spectrum->GetTuple(i, tuple);
        peakArea += tuple[0];
    }

    return peakArea; 
}


/*!
 *  Returns the peak symmetry (the closer to 0, the more symmetric. 
 */
float svkMRSPeakPick::GetPeakSymmetry( vtkFloatArray* spectrum, int peakNum )
{

    float peakSym = 0; 
    int startPt = this->peakVector[peakNum][0];
    int peakPt  = this->peakVector[peakNum][1];
    int endPt   = this->peakVector[peakNum][2];
    double tuple[2];     
    float leftIntegral = 0; 
    for ( int i = startPt; i <= peakPt; i++ ) {
        spectrum->GetTuple(i, tuple);
        leftIntegral += tuple[0];
    }

    float rightIntegral = 0; 
    for ( int i = peakPt; i <= endPt; i++ ) {
        spectrum->GetTuple(i, tuple);
        rightIntegral += tuple[0];
    }

    return abs( rightIntegral - leftIntegral ); 
}



int svkMRSPeakPick::GetNumPeaks() 
{
    return this->peakVector.size(); 
}


/*!
 *  Returns the peak height for the specified peak in the average
 *  RMS spectrum.  This is the peak height that was detected in 
 *  the peak picking process. 
 */
float svkMRSPeakPick::GetAvRMSPeakHeight( int peakNum ) 
{
    return this->peakHeightVector[peakNum]; 
}


/*!
 *  prints peak definition 
 */
void svkMRSPeakPick::GetPeakDefinition( int peakNum, int* startPt, int* peakPt, int* endPt ) 
{
    *startPt = this->peakVector[peakNum][0]; 
    *peakPt = this->peakVector[peakNum][1]; 
    *endPt = this->peakVector[peakNum][2]; 
}



/*!
 *
 */
void svkMRSPeakPick::PrintPeaks()
{
    cout << endl;
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
void svkMRSPeakPick::SetNoiseSD(float noise )
{
    this->noiseSD = noise;
}


/*! 
 *  Set the fractional resolve height.  For example, does
 *  a peak need to be resolved at fwhh to be detected.  If
 *  so set to 0.5. If the peak only needs to be resolved at
 *  0.7 of max then set to 0.7.  
 */
void svkMRSPeakPick::SetResolutionHeightFraction(float resolveHeightFraction)
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
void svkMRSPeakPick::SetBaselineValue(float baseline)
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



void svkMRSPeakPick::SetSNLimit(float sn)
{
    this->snLimit = sn; 
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



