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
#include <svkMathUtils.h>

#include <cmath>
#include <algorithm>

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
    //  Outputports:  0 for base ht map
    //  Outputports:  1 for peak ht map
    //  Outputports:  2 for peak time map 
    //  Outputports:  3 for up slope map 
    //  Outputports:  4 for washout slope map
    this->SetNumberOfOutputPorts(5); 

    this->normalize            = false; 
    this->baselineMean         = 0.;
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

    this->InitializeInjectionPoint();
    // this->InitializeBaseline(); // Needed?

    int numVoxels[3]; 
    this->GetOutput(0)->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 

    //  Get the data array to initialize.
    vtkDataArray* dceMapBaseHtArray;
    dceMapBaseHtArray   = this->GetOutput(0)->GetPointData()->GetArray(0);

    vtkDataArray* dceMapPeakHtArray;
    dceMapPeakHtArray   = this->GetOutput(1)->GetPointData()->GetArray(0);

    vtkDataArray* dceMapPeakTimeArray;
    dceMapPeakTimeArray = this->GetOutput(2)->GetPointData()->GetArray(0);

    vtkDataArray* dceMapUpSlopeArray;
    dceMapUpSlopeArray  = this->GetOutput(3)->GetPointData()->GetArray(0);

    vtkDataArray* dceMapWashoutArray;
    dceMapWashoutArray  = this->GetOutput(4)->GetPointData()->GetArray(0);

    //  Add the output volume array to the correct array in the svkMriImageData object
    string arrayNameString("pixels");
    dceMapBaseHtArray->SetName( arrayNameString.c_str() );
    dceMapPeakHtArray->SetName( arrayNameString.c_str() );
    dceMapPeakTimeArray->SetName( arrayNameString.c_str() );
    dceMapUpSlopeArray->SetName( arrayNameString.c_str() );
    dceMapWashoutArray->SetName( arrayNameString.c_str() );

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
            voxelValue  = dceMapPeakHtArray->GetTuple1( i );
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

    double voxelBaseHt;
    double voxelPeakHt; 
    double voxelPeakTime;
    double voxelUpSlope;
    double voxelWashout;
    int    filterWindow = 5;
    int    arrayLength  = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfTimePoints();

    svkMathUtils::MedianFilter1D( dynamicVoxelPtr, arrayLength, filterWindow);
    this->GetBaseHt( dynamicVoxelPtr, &voxelBaseHt );
    this->GetPeakHt( dynamicVoxelPtr, &voxelPeakHt ); 
    this->GetPeakTm( dynamicVoxelPtr, voxelPeakHt, &voxelPeakTime );
    this->GetUpSlope( dynamicVoxelPtr, voxelPeakTime, &voxelUpSlope );
    this->GetWashout( dynamicVoxelPtr, filterWindow, voxelIndex, &voxelWashout );
    this->ScaleParams( voxelBaseHt, &voxelPeakHt, &voxelPeakTime, &voxelUpSlope, &voxelWashout );

    //  Get the data array to initialize.
    vtkDataArray* dceMapBaseHtArray;
    dceMapBaseHtArray   = this->GetOutput(0)->GetPointData()->GetArray(0);

    vtkDataArray* dceMapPeakHtArray;
    dceMapPeakHtArray   = this->GetOutput(1)->GetPointData()->GetArray(0);

    vtkDataArray* dceMapPeakTimeArray;
    dceMapPeakTimeArray = this->GetOutput(2)->GetPointData()->GetArray(0);

    vtkDataArray* dceMapUpSlopeArray;
    dceMapUpSlopeArray  = this->GetOutput(3)->GetPointData()->GetArray(0);

    vtkDataArray* dceMapWashoutArray;
    dceMapWashoutArray  = this->GetOutput(4)->GetPointData()->GetArray(0);

    dceMapBaseHtArray->SetTuple1( voxelIndex, voxelBaseHt );
    dceMapPeakHtArray->SetTuple1( voxelIndex, voxelPeakHt );
    dceMapPeakTimeArray->SetTuple1( voxelIndex, voxelPeakTime );
    dceMapUpSlopeArray->SetTuple1( voxelIndex, voxelUpSlope );
    dceMapWashoutArray->SetTuple1( voxelIndex, voxelWashout );

}

/*!
 *  Compute the mean baseline and std dev as the mean of the first 
 *  time point over all spatial points in the volume 
 */
void svkDCEPeakHeight::InitializeBaseline()
{

    this->baselineMean = this->GetTimePointMean(0); 

    vtkDataArray* timePoint0Pixels = this->GetOutput(0)->GetPointData()->GetArray( 0 ); 
    int numSpatialPixels           = timePoint0Pixels->GetNumberOfTuples(); 
    this->baselineStdDeviation     = this->GetStandardDeviation( timePoint0Pixels, this->baselineMean, numSpatialPixels); 
}


/*
 * Compute the stdandard deviation of the array up to the specified endPt. 
 */
double svkDCEPeakHeight::GetStandardDeviation( vtkDataArray* array, float mean, int endPt) 
{
    double sumOfSquareDiffs = 0.; 
    for ( int i = 0; i < endPt; i++ ) {
        double diff       = ( array->GetTuple1(i) - mean ); 
        sumOfSquareDiffs += diff * diff; 
    }
    
    double variance = sumOfSquareDiffs / endPt;
    return sqrt(variance);
}

/*!
 *  Compute the mean value over all spatial locations for the specified time point. 
 */
double svkDCEPeakHeight::GetTimePointMean( int timePoint )
{

    vtkDataArray* timePointPixels = this->GetImageDataInput(0)->GetPointData()->GetArray( timePoint ); 

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

    svkDcmHeader* hdr      = this->GetImageDataInput(0)->GetDcmHeader();
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

        nextBaseline  = averageTrace->GetTuple1( timePt + 1 );
        basefactor    = runningAvg + injectionPointSDThreshold * runningStdDev;
        if (nextBaseline > basefactor) {
            this->injectionPoint = timePt;
        }
    }

}

/*!  
 *  Gets base height of DCE curve for the current voxel
 */
void svkDCEPeakHeight::GetBaseHt( float* dynamicVoxelPtr, double* voxelBaseHt )
{
    //  get total point range to check: 
    int    injectionPoint = this->injectionPoint; 
    double baselineVal    = 0;
    for ( int pt = 1; pt < 5; pt++) {
    //for ( int pt = 3; pt < injectionPoint; pt++) {          //Matlab hardcodes
        baselineVal += dynamicVoxelPtr[ pt ];
    }
    baselineVal = baselineVal / 4;
    //if ( injectionPoint > 1 ) {                             //Matlab hardcodes
    //    baselineVal = baselineVal / ( injectionPoint - 1 ); //Matlab hardcodes
    //}                                                       //Matlab hardcodes

    *voxelBaseHt = baselineVal;

}


/*!  
 *  Gets max peak height of DCE curve for the current voxel
 */
void svkDCEPeakHeight::GetPeakHt( float* dynamicVoxelPtr, double* voxelPeakHt )
{

    //  get total point range to check:    
    int startPt = this->injectionPoint; 
    int endPt   = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfTimePoints();

    // find peak height and peak diff
    double peakHt = dynamicVoxelPtr[ startPt ];
    for ( int pt = 0; pt < endPt/2; pt ++ ) {
        if ( dynamicVoxelPtr[ pt ] > peakHt ) {
            peakHt = dynamicVoxelPtr[ pt ];
        }
    }

    *voxelPeakHt = peakHt;

}

/*!  
 *  Gets time point at 90% peak height of DCE curve for the current voxel:
 *      Returns the POINT, not unit of time
 */
void svkDCEPeakHeight::GetPeakTm( float* dynamicVoxelPtr, double voxelPeakHt, double* voxelPeakTime)
{

    int    pt       = 1; //this->injectionPoint;
    double peakTime = 1.0;
    double height   = dynamicVoxelPtr[ pt - 1 ]; //dynamicVoxelPtr[ startPt ];
    while ( height < (0.9 * voxelPeakHt)) {
        height   = dynamicVoxelPtr[ pt - 1 ];
        peakTime = pt;
        pt++;
    }

    *voxelPeakTime = peakTime;

}

/*!  
 *  Gets peak difference of the DCE curve's upslope, to 90% peak, for the current voxel
 */
void svkDCEPeakHeight::GetUpSlope( float* dynamicVoxelPtr, double voxelPeakTime, double* voxelUpSlope )
{

    double diff     = 0.0;
    double peakDiff = 0.0;
    if (voxelPeakTime < 6) {
        peakDiff = 0.0;
    }
    else {
        for ( int pt = 0; pt < voxelPeakTime; pt ++ ) {
            if ( pt > 0 ) {
                diff = dynamicVoxelPtr[ pt ] - dynamicVoxelPtr[ pt - 1 ];
                if ( diff > peakDiff ) {
                    peakDiff = diff;
                }
            }
        }
    }

    *voxelUpSlope = peakDiff;

}

/*!  
 *  Gets the washout slope, via Linear Regression
 */
void svkDCEPeakHeight::GetWashout( float* dynamicVoxelPtr, int filterWindow, int voxelIndex, double* voxelWashout )
{
    int offset     = (filterWindow - 1) / 2;
    int range      = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfTimePoints();
    int numWashPts = range / 2 - offset;
    int startPt    = range - numWashPts - offset;
    int endPt      = range - offset;
    double slope;
    double intercept;

    this->GetRegression(voxelIndex, startPt, endPt, slope, intercept);

    *voxelWashout = slope;

}

/*!  
 *  Scales/calculates DCE parameters:
 *      Peak Height - Divides by Baseline and multiplies by 1000
 *      Peak Time   - Multiplies by image rate (trigger time) * 10
 *      Up Slope    - Divides by Baseline, then image rate, then * 60,000
 */
void svkDCEPeakHeight::ScaleParams( double voxelBaseHt, double* voxelPeakHt, double* voxelPeakTime, double* voxelUpSlope, double* voxelWashout )
{

    // calculate peak time 
    int numVoxels[3];
    this->GetOutput(0)->GetNumberOfVoxels(numVoxels);
    int   numberOfSlices = numVoxels[2]; 
    float imageRate;
    switch ( numberOfSlices ) {
        case 12:
            imageRate = 6.56;
        case 14:
            imageRate = 7.36;
        case 16:
            imageRate = 8.16;
        case 20:
            imageRate = 10.42;
        case 28:
            imageRate = 12.96;
        default:
            imageRate = numberOfSlices * 0.052;
    }

    double timeMin        = 0.0;
    double timeMax        = 2500.0;
    // double *voxelPeakTime = (voxelPeakTime - injectionPoint) * imageRate * 10; 
    // matlab doesn't account for injectinon point
    double peakTime = *voxelPeakTime * imageRate * 10;
    if (peakTime < timeMin) {
        peakTime = timeMin;
    }
    if (peakTime > timeMax) {
        peakTime = timeMax;
    }

    // scale peak height and slope
    double peakMin  = 0.0;
    double peakMax  = 5000.0;
    double slopeMin = 0.0;
    double slopeMax = 10000.0;
    double peakHt   = *voxelPeakHt;
    double slope    = *voxelUpSlope;
    double washout  = *voxelWashout;

    if ( voxelBaseHt <= 0 || slope <= 0 ) {
        peakHt  = 0.0;
        slope   = 0.0;
        washout = -150;
    }
    else {
        peakHt  = 10 * peakHt / voxelBaseHt * 100;
        slope   = 10 * (slope / voxelBaseHt) / imageRate * 60 * 100;
        washout = 10 * (washout / voxelBaseHt) / imageRate * 60 * 100;
    }

    if ( peakHt < peakMin  ) {
        peakHt = peakMin;
    }
    else if ( peakHt > peakMax ) {
        peakHt = peakMax;
    }

    if ( slope < slopeMin  ) {
        slope = slopeMin;
    }
    else if ( slope > slopeMax ) {
        slope = slopeMax;
    }

    *voxelPeakTime = peakTime;
    *voxelPeakHt   = peakHt;
    *voxelUpSlope  = slope;
    *voxelWashout  = washout;
}
