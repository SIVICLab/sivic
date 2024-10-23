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
 *  Adapted from uptake.c and DCEmaps_12.m/calc_dce_mat_maps_12.m, written by Sue Noworolski
 *  and Olga Starobinets at the University of California San Francisco Dept. of Radiology
 *  and Biomedical Imaging.
 *
 *  References:
 *      - 
 *      -
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

#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkInformationVector.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>

#include <svkDCEBasicFit.h>
#include <svkMathUtils.h>

#include <cmath>
#include <algorithm>

using namespace svk;


//vtkCxxRevisionMacro(svkDCEBasicFit, "$Rev$");
vtkStandardNewMacro(svkDCEBasicFit);


/*!
 *
 */
svkDCEBasicFit::svkDCEBasicFit()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    vtkInstantiator::RegisterInstantiator("svkMriImageData", svkMriImageData::NewObject);
    
    this->SetNumberOfInputPorts(3);
    bool repeatable  = true;
    bool required    = true;
    this->portMapper = NULL; 
    this->GetPortMapper()->InitializeInputPort(INPUT_IMAGE, "INPUT_IMAGE", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, required, !repeatable);
    this->GetPortMapper()->InitializeInputPort(START_TIME_PT, "START_TIME_PT", svkAlgorithmPortMapper::SVK_INT, !required);
    this->GetPortMapper()->InitializeInputPort(END_TIME_PT, "END_TIME_PT", svkAlgorithmPortMapper::SVK_INT, !required);

    this->SetNumberOfOutputPorts(6); 
    this->GetPortMapper()->InitializeOutputPort( BASE_HT_MAP, "BASE_HT_MAP", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
    this->GetPortMapper()->InitializeOutputPort( PEAK_HT_MAP, "PEAK_HT_MAP", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
    this->GetPortMapper()->InitializeOutputPort( PEAK_TIME_MAP, "PEAK_TIME_MAP", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
    this->GetPortMapper()->InitializeOutputPort( UP_SLOPE_MAP, "UP_SLOPE_MAP", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
    this->GetPortMapper()->InitializeOutputPort( WASHOUT_SLOPE_MAP, "WASHOUT_SLOPE_MAP", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
    this->GetPortMapper()->InitializeOutputPort( WASHOUT_SLOPE_POS_MAP, "WASHOUT_SLOPE_POS_MAP", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);

    this->normalize            = false; 
    this->baselineMean         = 0.;
    this->baselineStdDeviation = 0.; 
}


/*!
 *
 */
svkDCEBasicFit::~svkDCEBasicFit()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}


/*!
 * Returns the port mapper. Performs lazy initialization.
 */
svkAlgorithmPortMapper* svkDCEBasicFit::GetPortMapper()
{
    if(this->portMapper == NULL) {
        this->portMapper = svkAlgorithmPortMapper::New();
        this->portMapper->SetAlgorithm(this);
    }
    return this->portMapper;
}


/*!
 * Pass through method to the internal svkAlgorithmPortMapper
 */
int svkDCEBasicFit::FillOutputPortInformation( int port, vtkInformation* info )
{
    this->GetPortMapper()->FillOutputPortInformation(port, info );

    return 1;
}


/*!
 * Pass through method to the internal svkAlgorithmPortMapper
 */
int svkDCEBasicFit::FillInputPortInformation( int port, vtkInformation* info )
{
    this->GetPortMapper()->FillInputPortInformation(port, info );

    return 1;
}


/*!
 * Utility setter for input port: Timepoint Start
 */
void svkDCEBasicFit::SetTimepointStart(int startPt)
{
    this->GetPortMapper()->SetIntInputPortValue(START_TIME_PT, startPt);
}


/*!
 * Utility getter for input port: Timepoint Start
 */
svkInt* svkDCEBasicFit::GetTimepointStart()
{
    return this->GetPortMapper()->GetIntInputPortValue(START_TIME_PT);
}


/*!
 * Utility setter for input port: Timepoint End
 */
void svkDCEBasicFit::SetTimepointEnd(int endPt)
{
    this->GetPortMapper()->SetIntInputPortValue(END_TIME_PT, endPt);
}


/*!
 * Utility getter for input port: Timepoint End
 */
svkInt* svkDCEBasicFit::GetTimepointEnd()
{
    return this->GetPortMapper()->GetIntInputPortValue(END_TIME_PT);
}


/*! 
 *  Integrate real spectra over specified limits. 
 */
void svkDCEBasicFit::GenerateMaps()
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

    vtkDataArray* dceMapWashoutPosArray;
    dceMapWashoutPosArray = this->GetOutput(5)->GetPointData()->GetArray(0);

    //  Add the output volume array to the correct array in the svkMriImageData object
    string arrayNameString("pixels");
    dceMapBaseHtArray->SetName(arrayNameString.c_str());
    dceMapPeakHtArray->SetName(arrayNameString.c_str());
    dceMapPeakTimeArray->SetName(arrayNameString.c_str());
    dceMapUpSlopeArray->SetName(arrayNameString.c_str());
    dceMapWashoutArray->SetName(arrayNameString.c_str());
    dceMapWashoutPosArray->SetName(arrayNameString.c_str());

    // Initialize imageRate
    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader();
    float imageRate;
    if (hdr->ElementExists("TemporalResolution")) {
        imageRate = hdr->GetFloatValue("TemporalResolution");
        imageRate = imageRate / 1000;
    } else {
        int numberOfSlices = hdr->GetNumberOfSlices();
        switch (numberOfSlices) {
            case 12:
                imageRate = 6.560;
            case 14:
                imageRate = 7.360;
            case 16:
                imageRate = 8.160;
            case 20:
                imageRate = 10.42; //12.713;
            case 28:
                imageRate = 12.96;
            default:
                imageRate = numberOfSlices * 0.572; // 0.63565;
        }
    }

    double voxelValue;
    for (int i = 0; i < totalVoxels; i++) {
        vtkFloatArray* perfusionDynamics = vtkFloatArray::SafeDownCast(
            svkMriImageData::SafeDownCast(this->GetImageDataInput(0))->GetCellDataRepresentation()->GetArray(i) 
       ); 
        float* dynamicVoxelPtr = perfusionDynamics->GetPointer(0);

        this->InitializeOutputVoxelValues(dynamicVoxelPtr, i, imageRate); 
    }

    if (this->normalize) {
        double nawmValue = this->GetNormalizationFactor(); 
        for (int i = 0; i < totalVoxels; i++) {
            voxelValue  = dceMapPeakHtArray->GetTuple1(i);
            voxelValue /= nawmValue; 
            dceMapPeakHtArray->SetTuple1(i, voxelValue);
        }
    }

}

/*!  
 *  For multi-volume data modifies header's per frame functional group sequence:
 */
void svkDCEBasicFit::InitializeOutputVoxelValues(float* dynamicVoxelPtr, int voxelIndex, float imageRate) 
{

    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader();

    double voxelBaseHt;
    double voxelPeakHt; 
    double voxelPeakTime;
    double voxelUpSlope;
    double voxelWashout;
    double voxelWashoutPos;
    
    int    filterWindow = 5;
    int    arrayLength  = hdr->GetNumberOfTimePoints();
    float* unfilteredDynamicVoxelPtr = dynamicVoxelPtr;

    svkMathUtils::MedianFilter1D(dynamicVoxelPtr, arrayLength, filterWindow);
    this->GetBaseHt(dynamicVoxelPtr, &voxelBaseHt);
    this->GetPeakHt(dynamicVoxelPtr, &voxelPeakHt); 
    this->GetPeakTm(dynamicVoxelPtr, voxelPeakHt, &voxelPeakTime);
    this->GetUpSlope(unfilteredDynamicVoxelPtr, voxelPeakTime, &voxelUpSlope);
    this->GetWashout(dynamicVoxelPtr, filterWindow, voxelIndex, &voxelWashout);
    this->ScaleParams(imageRate, voxelBaseHt, &voxelPeakHt, &voxelPeakTime, &voxelUpSlope, &voxelWashout, &voxelWashoutPos);

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

    vtkDataArray* dceMapWashoutPosArray;
    dceMapWashoutPosArray  = this->GetOutput(5)->GetPointData()->GetArray(0);

    dceMapBaseHtArray->SetTuple1(voxelIndex, voxelBaseHt);
    dceMapPeakHtArray->SetTuple1(voxelIndex, voxelPeakHt);
    dceMapPeakTimeArray->SetTuple1(voxelIndex, voxelPeakTime);
    dceMapUpSlopeArray->SetTuple1(voxelIndex, voxelUpSlope);
    dceMapWashoutArray->SetTuple1(voxelIndex, voxelWashout);
    dceMapWashoutPosArray->SetTuple1(voxelIndex, voxelWashoutPos);

}

/*!
 *  Compute the mean baseline and std dev as the mean of the first 
 *  time point over all spatial points in the volume 
 */
void svkDCEBasicFit::InitializeBaseline()
{

    this->baselineMean             = this->GetTimePointMean(0); 
    vtkDataArray* timePoint0Pixels = this->GetOutput(0)->GetPointData()->GetArray(0); 
    int numSpatialPixels           = timePoint0Pixels->GetNumberOfTuples(); 
    this->baselineStdDeviation     = this->GetStandardDeviation(timePoint0Pixels, this->baselineMean, numSpatialPixels); 

}


/*
 * Compute the stdandard deviation of the array up to the specified endPt. 
 */
double svkDCEBasicFit::GetStandardDeviation(vtkDataArray* array, float mean, int endPt) 
{

    double sumOfSquareDiffs = 0.; 
    for (int i = 0; i < endPt; i++) {
        double diff       = (array->GetTuple1(i) - mean); 
        sumOfSquareDiffs += diff * diff; 
    }
    
    double variance = sumOfSquareDiffs / endPt;

    return sqrt(variance);

}

/*!
 *  Compute the mean value over all spatial locations for the specified time point. 
 */
double svkDCEBasicFit::GetTimePointMean(int timePoint)
{

    vtkDataArray* timePointPixels = this->GetImageDataInput(0)->GetPointData()->GetArray(timePoint); 

    double sum = 0.; 

    int numSpatialPixels = timePointPixels->GetNumberOfTuples(); 
    for (int i = 0; i < numSpatialPixels; i++) {
        sum += timePointPixels->GetTuple1(i); 
    }
    double mean = sum / numSpatialPixels; 

    return mean; 

}

/*!
 *  Compute the mean baseline and std dev as the mean of the first 
 *  time point over all spatial points in the volume 
 */
void svkDCEBasicFit::InitializeInjectionPoint()
{

    svkDcmHeader* hdr      = this->GetImageDataInput(0)->GetDcmHeader();
    int numberOfTimePoints = this->GetTimepointEnd()->GetValue();
    int startPt            = this->GetTimepointStart()->GetValue() - 1;

    //  Create a vtkDataArray to hold the average time kinetic 
    //  trace over the entire volume: 
    vtkFloatArray* time0Pixels = 
            static_cast<vtkFloatArray*>(
                svkMriImageData::SafeDownCast(this->GetImageDataInput(0))->GetCellDataRepresentation()->GetArray(0) 
           ); 
    vtkFloatArray* averageTrace = vtkFloatArray::New();
    averageTrace->DeepCopy(time0Pixels);

    //  For each time point compute the mean value over all spatial points  
    //  Essentially 1 voxel that represents the DCE average DCE trace.
    for (int timePt = startPt; timePt < numberOfTimePoints; timePt++) {
        double timeSpatialMean = this->GetTimePointMean(timePt); 
        averageTrace->SetTuple1(timePt, timeSpatialMean); 
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
    for (int timePt = startPt; timePt < numberOfTimePoints; timePt++) {
        runningSum   += averageTrace->GetTuple1(timePt);
        runningAvg    = runningSum/(timePt + 1.0);
        runningStdDev = this->GetStandardDeviation(averageTrace, runningAvg, timePt); 

        nextBaseline  = averageTrace->GetTuple1(timePt + 1);
        basefactor    = runningAvg + injectionPointSDThreshold * runningStdDev;
        if (nextBaseline > basefactor) {
            this->injectionPoint = timePt;
        }
    }

}

/*!  
 *  Gets base height of DCE curve for the current voxel
 */
void svkDCEBasicFit::GetBaseHt(float* dynamicVoxelPtr, double* voxelBaseHt)
{
    //  get total point range to check: 
    int    injectionPoint = this->injectionPoint; 
    double baselineVal    = 0;
    for (int pt = 1; pt < 5; pt++) {
    //for (int pt = 3; pt < injectionPoint; pt++) {          //Matlab hardcodes
        baselineVal += dynamicVoxelPtr[ pt ];
    }
    baselineVal = baselineVal / 4;
    //if (injectionPoint > 1) {                             //Matlab hardcodes
    //    baselineVal = baselineVal / (injectionPoint - 1); //Matlab hardcodes
    //}                                                       //Matlab hardcodes

    *voxelBaseHt = baselineVal;

}


/*!  
 *  Gets max peak height of DCE curve for the current voxel
 */
void svkDCEBasicFit::GetPeakHt(float* dynamicVoxelPtr, double* voxelPeakHt)
{

    int startPt   = this->injectionPoint; 
    // int endPt     = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfTimePoints();
    int endPt     = this->GetTimepointEnd()->GetValue();
    double peakHt = dynamicVoxelPtr[ startPt ];
    for (int pt = 0; pt < endPt/2; pt ++) {
        if (dynamicVoxelPtr[ pt ] > peakHt) {
            peakHt = dynamicVoxelPtr[ pt ];
        }
    }

    *voxelPeakHt = peakHt;

}

/*!  
 *  Gets time point at 90% peak height of DCE curve for the current voxel:
 *      Returns the POINT, not unit of time
 */
void svkDCEBasicFit::GetPeakTm(float* dynamicVoxelPtr, double voxelPeakHt, double* voxelPeakTime)
{

    int    pt       = this->GetTimepointStart()->GetValue(); //this->injectionPoint;
    double peakTime = 1.0;
    double height   = dynamicVoxelPtr[ pt - 1 ]; //dynamicVoxelPtr[ startPt ];
    while (height < (0.9 * voxelPeakHt)) {
        height   = dynamicVoxelPtr[ pt - 1 ];
        peakTime = pt;
        pt++;
    }

    *voxelPeakTime = peakTime;

}

/*!  
 *  Gets peak difference of the DCE curve's upslope, to 90% peak, for the current voxel
 */
void svkDCEBasicFit::GetUpSlope(float* dynamicVoxelPtr, double voxelPeakTime, double* voxelUpSlope)
{

    double diff     = 0.0;
    double peakDiff = 0.0;
    int    startPt  = this->GetTimepointStart()->GetValue();
    if (voxelPeakTime < 6) {
        peakDiff = 0.0;
    }
    else {
        for (int pt = startPt; pt <= voxelPeakTime; pt++) {
            diff = dynamicVoxelPtr[ pt ] - dynamicVoxelPtr[ pt - 1 ];
            if (diff > peakDiff) {
                peakDiff = diff;
            }
        }
    }

    *voxelUpSlope = peakDiff;

}

/*!  
 *  Gets the washout slope, via Linear Regression
 */
void svkDCEBasicFit::GetWashout(float* dynamicVoxelPtr, int filterWindow, int voxelIndex, double* voxelWashout)
{

    int offset     = (filterWindow - 1) / 2;
    // int range      = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfTimePoints();
    int range      = this->GetTimepointEnd()->GetValue();
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
 *      Washout     - Divides by Baseline, then image rate, then * 60,000
 */
void svkDCEBasicFit::ScaleParams(float imageRate, double voxelBaseHt, double* voxelPeakHt, double* voxelPeakTime, double* voxelUpSlope, double* voxelWashout, double* voxelWashoutPos)
{

    // calculate peak time
    double timeMin    = 0.0;
    double timeMax    = 2500.0;
    double peakTimePt = *voxelPeakTime;
    double peakTime   = peakTimePt * imageRate * 10;
    if (peakTime < timeMin) {
        peakTime = timeMin;
    }
    if (peakTime > timeMax) {
        peakTime = timeMax;
    }

    // scale remaining parameters
    double peakMin    = 0.0;
    double peakMax    = 5000.0;
    double slopeMin   = 0.0;
    double slopeMax   = 10000.0;
    double peakHt     = *voxelPeakHt;
    double slope      = *voxelUpSlope;
    double washout    = *voxelWashout;
    double washoutPos = *voxelWashoutPos;

    if (voxelBaseHt <= 0 || slope <= 0) {
        peakHt  = 0.0;
        slope   = 0.0;
        washout = -150;
    }
    else {
        peakHt  = 10 * peakHt / voxelBaseHt * 100;
        slope   = 10 * (slope / voxelBaseHt) / imageRate * 60 * 100;
        washout = 10 * (washout / voxelBaseHt) / imageRate * 60 * 100;
        washoutPos = washout + 150;
    }

    if (washoutPos < 0 ) {
        washoutPos = 0;
    }

    if (washout < -600) {
        washout = -600;
    }

    if (washout > 600) {
        washout = 600;
    }

    if (washoutPos > 750) {
        washoutPos = 750;
    }

    if (peakHt < peakMin ) {
        peakHt = peakMin;
    }
    else if (peakHt > peakMax) {
        peakHt = peakMax;
    }

    if (slope < slopeMin ) {
        slope = slopeMin;
    }
    else if (slope > slopeMax) {
        slope = slopeMax;
    }

    *voxelPeakTime   = peakTime;
    *voxelPeakHt     = peakHt;
    *voxelUpSlope    = slope;
    *voxelWashout    = washout;
    *voxelWashoutPos = washoutPos;

}
