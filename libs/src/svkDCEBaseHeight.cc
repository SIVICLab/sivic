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
 *  $URL: svn+ssh://hawk-sf@svn.code.sf.net/p/sivic/code/trunk/libs/src/svkDCEBaseHeight.cc $
 *  $Rev: 2188 $
 *  $Author: jccrane $
 *  $Date: 2015-04-24 13:01:59 -0700 (Fri, 24 Apr 2015) $
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <svkDCEBaseHeight.h>

#include <math.h>

using namespace svk;


vtkCxxRevisionMacro(svkDCEBaseHeight, "$Rev: 2188 $");
vtkStandardNewMacro(svkDCEBaseHeight);


/*!
 *
 */
svkDCEBaseHeight::svkDCEBaseHeight()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

}



/*!
 *
 */
svkDCEBaseHeight::~svkDCEBaseHeight()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}



/*! 
 *  Integrate real spectra over specified limits. 
 */
void svkDCEBaseHeight::GenerateMap()
{
    int numVoxels[3]; 
    this->GetOutput()->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 

    //  Get the data array to initialize.  
    vtkDataArray* dceMapArray;
    dceMapArray = this->GetOutput()->GetPointData()->GetArray(0); 

    //  Add the output volume array to the correct array in the svkMriImageData object
    vtkstd::string arrayNameString("pixels");

    dceMapArray->SetName( arrayNameString.c_str() );

    double voxelValue;
    for (int i = 0; i < totalVoxels; i++ ) {

        vtkFloatArray* perfusionDynamics = vtkFloatArray::SafeDownCast( 
            svkMriImageData::SafeDownCast(this->GetImageDataInput(0))->GetCellDataRepresentation()->GetArray(i) 
        ); 
        float* dynamicVoxelPtr = perfusionDynamics->GetPointer(0);

        voxelValue = this->GetMapVoxelValue( dynamicVoxelPtr ); 

        dceMapArray->SetTuple1(i, voxelValue);
    }

    if ( this->normalize ) {

        double nawmValue = this->GetNormalizationFactor(); 
        for (int i = 0; i < totalVoxels; i++ ) {
            voxelValue = dceMapArray->GetTuple1( i );
            voxelValue /= nawmValue; 
            dceMapArray->SetTuple1(i, voxelValue);
        }

    }

}


/*!  
 *  For multi-volume data modifies header's per frame functional group sequence:
 */
double svkDCEBaseHeight::GetMapVoxelValue( float* dynamicVoxelPtr )
{
    double voxelValue; 
    voxelValue = this->GetBaseHt( dynamicVoxelPtr ); 
    return voxelValue;
}

double standardDeviation(float* signalArrayPtr, float mean, int timepoint)
{
    float stdev   = 0.0;
    float var     = 0.0;
    int   startPt = 0;
    for ( int pt = startPt; pt < timepoint; pt++ ) {
        stdev += (signalArrayPtr[pt] - mean) * (signalArrayPtr[pt] - mean);
    }
    var = stdev / timepoint;
    if (timepoint != 0) {
        return math::sqrt(var);
    }
    else {
        return 0.0;
    }
}

double svkDCEBaseHeight::GetBaselineArray(float* data)
{



/*
*  Calculates the injection point of the timeseries
*/
int svkDCEBaseHeight::GetInjectionPoint( float* baselineArray )
{
    int   startPt    = 0;
    int   endPt      = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfVolumes();
    float runningSum = 0.0;
    float runningAvg = 0.0;
    float nextBaseline, basefactor;
    for ( int pt = startPt; pt < endPt; pt ++ ) {
        runningSum += baselineArray[pt];
        runningAvg  = runingSum/(pt + 1.0);
        std_dev     = standardDeviation(baselineArray, runningAvg, pt);
        if (pt > 2) {
            nextBaseline = baselineArray[pt + 1];
            basefactor   = runningAvg + 2.5 * std_dev;
            if (nextBaseline > basefactor) {
                return pt;
            }
        }
    }
    return 1;
}

/*!  
 *  Gets baseline height of DCE curve for the current voxel, before injection point.  
 */
double svkDCEBaseHeight::GetBaseHt( float* dynamicVoxelPtr, int injectionPoint=0)
{
    // get base value
    float baseval = 0;
    int   minPt   = 3;
    for ( int pt = minPt; pt < injectionPoint; pt++) {
        baseval += dynamicVoxelPtr[ pt ];
    }
    baseval = baseval / ( injectionPoint - 1 );

    return baseval;
}
