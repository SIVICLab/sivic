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


#include <svkMathUtils.h>

#include <cmath>
#include <algorithm>

using namespace svk;


//vtkCxxRevisionMacro(svkMathUtils, "$Rev$");
vtkStandardNewMacro(svkMathUtils);


/*!
 *
 */
svkMathUtils::svkMathUtils()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

}



/*!
 *
 */
svkMathUtils::~svkMathUtils()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}


/*!
 *  Computes median value of an array. 
 */
double svkMathUtils::GetMedian( vector<double>* signalWindow) 
{
    nth_element(signalWindow->begin(), signalWindow->begin() + signalWindow->size()/2, signalWindow->end());
    return (*signalWindow)[ signalWindow->size() / 2 ];
}


/*!
 *  Median filters an array, using a neighborhood window of windowSize. 
 */
void svkMathUtils::MedianFilter1D( float* dynamicVoxelPtr, int arrayLength, int windowSize=3 )
{

    // Create zero-padded array from timeseries data
    int    endPt    = arrayLength;
    int    padLength     = (int)(windowSize / 2);
    vector<double> window(windowSize);
    int totalPadLength = padLength*2;
    double* paddedArray = new double[endPt + totalPadLength ];
    for ( int pt = 0; pt < (endPt + totalPadLength); pt++ ) {
        if((pt < padLength) || (pt >= ( endPt + padLength))) {
            paddedArray[pt] = 0;
        } else {
            paddedArray[pt] = dynamicVoxelPtr[ pt - padLength ];
        }
    }
    
    // Starting from first non-padding point, create neighborhood window
    // and pass to GetMedian()
    int i;
    for ( int pt = padLength; pt < (endPt + padLength); pt++ ) {
        for (int winPt = 0; winPt < windowSize; winPt++) {
            window[winPt] = paddedArray[pt - padLength + winPt];
        }
        //cout << "window: " << window[0] <<  " " << window[1] <<  " "
        //      << window[2] <<  " "<< window[3] <<  " "<< window[4] <<  " " << endl;
        dynamicVoxelPtr[pt - padLength] = svkMathUtils::GetMedian(&window);
    }
	delete paddedArray;
}

