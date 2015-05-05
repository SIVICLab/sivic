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
 */

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <svkDCEPeakHeight.h>
#include <svkMRSNoise.h>

#include <math.h>

using namespace svk;


vtkCxxRevisionMacro(svkDCEPeakHeight, "$Rev$");
vtkStandardNewMacro(svkDCEPeakHeight);


/*!
 *
 */
svkDCEPeakHeight::svkDCEPeakHeight()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");
    //  Output Port 0 = peak height map
    //  Output Port 1 = peak height time map
    this->SetNumberOfOutputPorts(2); 

    this->baselineMean = 0.;
    this->baselineStdDeviation = 0.; 
}



/*!
 *
 */
svkDCEPeakHeight::~svkDCEPeakHeight()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}



/*! 
 *  Integrate real spectra over specified limits. 
 */
void svkDCEPeakHeight::GenerateMaps()
{

    this->InitializeBaseline(); 

    int numVoxels[3]; 
    this->GetOutput(0)->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 

    //  Get the data array to initialize.  
    vtkDataArray* dceMapPeakHtArray;
    dceMapPeakHtArray = this->GetOutput(0)->GetPointData()->GetArray(0); 

    vtkDataArray* dceMapPeakTimeArray;
    dceMapPeakTimeArray = this->GetOutput(1)->GetPointData()->GetArray(0); 

    //  Add the output volume array to the correct array in the svkMriImageData object
    vtkstd::string arrayNameString("pixels");
    dceMapPeakHtArray->SetName( arrayNameString.c_str() );
    dceMapPeakTimeArray->SetName( arrayNameString.c_str() );

    double voxelValue;
    for (int i = 0; i < totalVoxels; i++ ) {

        vtkFloatArray* perfusionDynamics = vtkFloatArray::SafeDownCast( 
            svkMriImageData::SafeDownCast(this->GetImageDataInput(0))->GetCellDataRepresentation()->GetArray(i) 
        ); 
        float* dynamicVoxelPtr = perfusionDynamics->GetPointer(0);

        this->InitializeOutputVoxelValues( dynamicVoxelPtr, i ); 

    }

    if ( this->normalize ) {

        double nawmValue = this->GetNormalizationFactor(); 
        for (int i = 0; i < totalVoxels; i++ ) {
            voxelValue = dceMapPeakHtArray->GetTuple1( i );
            voxelValue /= nawmValue; 
            dceMapPeakHtArray->SetTuple1(i, voxelValue);
        }

    }

}


/*!  
 *  For multi-volume data modifies header's per frame functional group sequence:
 */
void svkDCEPeakHeight::InitializeOutputVoxelValues( float* dynamicVoxelPtr, int voxelIndex ) 
{

    double voxelPeakHt; 
    double voxelPeakTime;
    int    filterWindow = 5;
    this->MedianFilter1D( dynamicVoxelPtr,  filterWindow)
    this->GetPeakParams( dynamicVoxelPtr, &voxelPeakHt, &voxelPeakTime); 

    //  Get the data array to initialize.  
    vtkDataArray* dceMapPeakHtArray;
    dceMapPeakHtArray = this->GetOutput(0)->GetPointData()->GetArray(0); 

    vtkDataArray* dceMapPeakTimeArray;
    dceMapPeakTimeArray = this->GetOutput(1)->GetPointData()->GetArray(0); 

    dceMapPeakHtArray->SetTuple1(  voxelIndex, voxelPeakHt );
    dceMapPeakTimeArray->SetTuple1(voxelIndex, voxelPeakTime);

}



/*!
 *  Compute the mean baseline and std dev as the mean of the first 
 *  time point over all spatial points in the volume 
 */
void svkDCEPeakHeight::InitializeBaseline()
{

    this->baselineMean = this->GetTimePointMean(0); 

    vtkDataArray* timePoint0Pixels = this->GetOutput(0)->GetPointData()->GetArray( 0 ); 
    int numSpatialPixels = timePoint0Pixels->GetNumberOfTuples(); 
    this->baselineStdDeviation = this->GetStandardDeviation( timePoint0Pixels, this->baselineMean, numSpatialPixels); 
}

/*
 * Compute the stdandard deviation of the array up to the specified endPt. 
 */
double svkDCEPeakHeight::GetStandardDeviation( vtkDataArray* array, float mean, int endPt) 
{
    double sumOfSquareDiffs = 0.; 
    for ( int i = 0; i < endPt; i++ ) {
        double diff = ( array->GetTuple1(i) - mean ); 
        sumOfSquareDiffs += diff * diff; 
    }
    
    double variance = sumOfSquareDiffs / endPt;
    return math::sqrt(variance);
}


/*!
 *  Compute the mean value over all spatial locations for the specified time point. 
 */
double svkDCEPeakHeight::GetTimePointMean(int timePoint )
{

    vtkDataArray* timePointPixels = this->GetOutput(0)->GetPointData()->GetArray( timePoint ); 

    double sum = 0.; 

    int numSpatialPixels = timePointPixels->GetNumberOfTuples(); 
    for ( int i = 0; i < numSpatialPixels; i++ ) {
        sum += timePointPixels->GetTuple1(i); 
    }
    double mean = sum / numSpatialPixels; 

    return mean;     
}

/*!
 *  Compute the mean baseline and std dev as the mean of the first 
 *  time point over all spatial points in the volume 
 */
void svkDCEPeakHeight::InitializeInjectionPoint()
{

    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader();
    int numberOfTimePoints = hdr->GetNumberOfTimePoints();

    //  Create a vtkDataArray to hold the average time kinetic 
    //  trace over the entire volume: 
    vtkFloatArray* time0Pixels = 
            static_cast<vtkFloatArray*>(
                svkMriImageData::SafeDownCast(this->GetImageDataInput(0))->GetCellDataRepresentation()->GetArray(0) 
            ); 
    vtkFloatArray* averageTrace = vtkFloatArray::New();
    averageTrace->DeepCopy( time0Pixels );

    //  For each time point compute the mean value over all spatial points  
    //  Essentially 1 voxel that represents the DCE average DCE trace.
    for ( int timePt = 0; timePt < numberOfTimePoints; timePt++ ) {
        double timeSpatialMean = this->GetTimePointMean( timePt ); 
        averageTrace->SetTuple1( timePt, timeSpatialMean ); 
    }

    //  Now determine the injection point. 
    this->injectionPoint = 2;
    float runningSum     = 0.0;
    float runningAvg     = 0.0;
    float nextBaseline; 
    float basefactor;
    double runningStdDev;
    //  number of std devs above baseline for detection of the injection point. 
    float  injectionPointSDThreshold = 2.5; 
    for ( int timePt = 0; timePt < numberOfTimePoints; timePt++ ) {

        runningSum   += averageTrace->GetTuple1( timePt );
        runningAvg    = runningSum/( timePt + 1.0 );
        runningStdDev = this->GetStandardDeviation( averageTrace, runningAvg, timePt ); 

        nextBaseline = averageTrace->GetTuple1( timePt + 1 );
        basefactor   = runningAvg + injectionPointSDThreshold * runningStdDev;
        if (nextBaseline > basefactor) {
            this->injectionPoint = timePt;
        }
    }

}

/*!
 *  Computes median value of an array. 
 */
double svkDCEPeakHeight::GetMedian( double* signalWindow, int size ) 
{
    // Create sorted signal array
    double sortedWindow[size];
    for (int pt = 0; pt < size; ++pt) {
        sortedWindow[pt] = signalWindow[pt];
    }
    for (int i = iSize - 1; i > 0; --i) {
        for (int j = 0; j < i; ++j) {
            if (sortedWindow[j] > sortedWindow[j+1]) {
                double sortTemp   = sortedWindow[j];
                sortedWindow[j]   = sortedWindow[j+1];
                sortedWindow[j+1] = sortTemp;
            }
        }
    }

    // Middle or average of middle values in the sorted array
    double median = 0.0;
    if ((size % 2) == 0) {
        median = (sortedWindow[size/2] + sortedWindow[(size/2) - 1])/2.0;
    } else {
        median = sortedWindow[size/2];
    }

    return median;
}

/*!
 *  Median filters an array, using a neighborhood window of windowSize
 */
void svkDCEPeakHeight::MedianFilter1D( float* dynamicVoxelPtr, int windowSize=3 )
{
    // Create zero-padded array from timeseries data
    int    endPt    = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfTimePoints();
    int    edge     = windowSize % 2;
    double window[windowSize];
    double paddedArray[endPt + edge * 2];
    for ( int pt = 0; pt < (endPt + edge * 2); pt++ ) {
        if((pt < edge) || (pt >= endPt)) {
            paddedArray[pt] = 0;
        }
        else{
            paddedArray[pt] = dynamicVoxelPtr[pt];
        }
    }

    // Starting from first non-padding point, create neighborhood window
    // and pass to GetMedian() 
    for ( int x = edge; x < (endPt - edge); x++ ) {
        i = 0;
        for (int fx = 0; fx < windowSize; fx++) {
            window[i] = paddedArray[x + fx - edge];
        }
        i++;
        paddedArray[x] = this->GetMedian(window, windowSize)
    }

    // Put median values back into timeseries data 
    for ( int pt = edge; pt < endPt; pt++ ) {
        dynamicVoxelPtr[pt] = paddedArray[pt];
    }
}

/*!  
 *  Gets max peak height of DCE curve for the current voxel:  
 *      PeakHt is relative to the baseline value (pre uptake) 
 *      and multiplied by 1000 for scaling. 
 *
 *  Questions: auto-determine the baseine time window?  Possibly use find noise method from 
 *  spec analysis      
 */
void svkDCEPeakHeight::GetPeakParams( float* dynamicVoxelPtr, double* voxelPeakHt, double* voxelPeakTime )
{
    // get base value
    this->InitializeInjectionPoint();
    int   injectionPoint = this->injectionPoint; 
    float baselineVal    = 0;
    float imageRate      = 0.65106;
    for ( int pt = 3; pt < injectionPoint; pt++) {
        baselineVal += dynamicVoxelPtr[ pt ];
    }
    baselineVal = baselineVal / ( injectionPoint - 1 );

    //  get total point range to check:    
    int startPt     = injectionPoint; 
    int endPt       = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfTimePoints();

    // calculate peak height
    double peakHt   = dynamicVoxelPtr[ startPt ];
    for ( int pt = startPt; pt < endPt; pt ++ ) {
        if ( dynamicVoxelPtr[ pt ] > peakHt ) {
            peakHt   = dynamicVoxelPtr[ pt ];
        }       
    }

    // calculate peak time
    double peakTime = dynamicVoxelPtr[ startPt ];
    double height   = dynamicVoxelPtr[ startPt ];
    int pt          = injectionPoint;
    while ( height <= 0.9 * peakHt) {
        height   = dynamicVoxelPtr[ pt ];
        peakTime = pt;
        startPt++;
    }
    int numVoxels[3]; 
    this->GetOutput(0)->GetNumberOfVoxels(numVoxels);
    int numberOfSlices = numVoxels[2]; 
    peakTime           = (peakTime - injectionPoint) * imageRate * numberOfSlices;
    if (peakTime < 0) {
        peakTime = 0;
    }

    // scale peak height
    peakHt = peakHt * 1000 / baselineVal;

    // set peak height and peak time values
    *voxelPeakHt   = peakHt; 
    *voxelPeakTime = peakTime; 
}
