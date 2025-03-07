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
 *
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


#include <svkStatistics.h>

using namespace svk;

//vtkCxxRevisionMacro(svkStatistics, "$Rev$");
vtkStandardNewMacro(svkStatistics);

//! Constructor
svkStatistics::svkStatistics()
{
}


//! Destructor
svkStatistics::~svkStatistics()
{
}


/*!
 *  This method takes every pixel within the input image data that correspond to values > 0 in the roi image.
 *  Only looks at the current scalars. Only values >= minIncluded and <= maxIncluded are included.
 */
vtkDataArray* svkStatistics::GetMaskedPixels( svkMriImageData* image, svkMriImageData* roi, double minIncluded, double maxIncluded)
{
    vtkDataArray* pixelsInROI = NULL;
    //TODO: Consider how the datasets should be validated.
    if( image != NULL && roi != NULL ) {
        vtkDataArray* pixels = image->GetPointData()->GetScalars();
        vtkDataArray* mask = roi->GetPointData()->GetScalars();
        if( pixels != NULL && roi != NULL && pixels->GetNumberOfTuples() ==  mask->GetNumberOfTuples() ) {
            pixelsInROI =  vtkDataArray::CreateDataArray( pixels->GetDataType() );
            pixelsInROI->SetName("MaskedPixels");
            pixelsInROI->SetNumberOfComponents(1);
            for( int i = 0; i < pixels->GetNumberOfTuples(); i++ ) {
                if( mask->GetTuple1(i) >= minIncluded &&  mask->GetTuple1(i) <= maxIncluded) {
                    pixelsInROI->InsertNextTuple1( pixels->GetTuple1(i) );
                }
            }
        } else {
            cout << "ERROR: Could not get masked pixels. Image and roi have a different number of pixels!" << endl;
        }
    } else {
        cout << "ERROR: Could not get masked pixels. Either image or roi is NULL!" << endl;
    }
    return pixelsInROI;
}


double svkStatistics::ComputeModeFromHistogram(vtkDataArray* histogram, double binSize, double startBin, int smoothBins)
{
    int numBins = histogram->GetNumberOfTuples();
    int modeBin = smoothBins/2 - 1;
    double modeBinHeight = histogram->GetTuple1(modeBin);
    // Find the max for the mode
    for( int i = modeBin + 1; i < numBins - (smoothBins/2 - 1); i++ ) {
        if( histogram->GetTuple1(i) >= modeBinHeight && histogram->GetTuple1(i) != 0) {
            modeBin = i;
            modeBinHeight = histogram->GetTuple1(modeBin);
        }
    }
    double value = 0;
    if( modeBinHeight > 0 ) {
        value = binSize*modeBin + startBin;
    }
    return value;

}

vector<double> svkStatistics::ComputeQuantilesFromHistogram(int numQuantiles, vtkDataArray* histogram, double binSize, double startBin, int numBins, int smoothBins)
{

    vector<double> quantiles;
    // Now let's calculate the percentiles
    vtkFloatArray* cdf = vtkFloatArray::New();
    cdf->SetNumberOfComponents(1);
    cdf->SetNumberOfTuples(numBins);

    // First lets create an cumulative distribution function
    for( int i = 0; i < numBins; i++ ) {
        if( i == 0 ) {
            // First bin special case
            cdf->SetTuple1(0, histogram->GetTuple1(0));
        } else {
            cdf->SetTuple1(i, cdf->GetTuple1(i-1) + histogram->GetTuple1(i));
        }
    }

    vtkFloatArray* ncdf = vtkFloatArray::New();
    ncdf->SetNumberOfComponents(1);
    ncdf->SetNumberOfTuples(numBins);
    // Normalize the cdf by the last bin, which contains the total sum of all bins.
    for( int i = 0; i < numBins; i++ ) {
        ncdf->SetTuple1(i, cdf->GetTuple1(i)/cdf->GetTuple1(numBins-1));
    }

    vector<double> intervalUpperBin;
    vector<double> intervalLowerBin;
    //Initialize to zeros
    intervalUpperBin.assign(numQuantiles,0);
    intervalLowerBin.assign(numQuantiles,0);

    // Let's use the cdf to compute percentiles....

    // Approach from the lower values
    for( int i = 0; i < numBins; i++ ) {
        for( int j = 0; j < numQuantiles; j++ ) {
            double intervalMax = double(j)/numQuantiles;
            if( ncdf->GetTuple1(i) < intervalMax ) {
                intervalLowerBin[j] = i;
            }
        }
    }

    // Approach from the upper values
    for( int i = numBins -1; i >= 0; i-- ) {
        for( int j = 0; j < numQuantiles; j++ ) {
            double intervalMin = double(j)/numQuantiles;
            if( ncdf->GetTuple1(i) > intervalMin ) {
                intervalUpperBin[j] = i;
            }
        }
    }
    for( int i = 0; i < numQuantiles; i++ ) {
        double quantile = binSize*((intervalUpperBin[i] + intervalLowerBin[i])/2.0) + startBin;
        quantiles.push_back(quantile);
    }
    return quantiles;
}

/*!
 * Generates a histogram for the given vtkDataArray. If smoothBins is greater then one then the histogram is smoothed.
 * If smoothBins is even then the next highest odd number is used.
 */
vtkFloatArray* svkStatistics::GetHistogram( vtkDataArray* data, double binSize, double startBin, int numBins, int smoothBins)
{
    vtkFloatArray* histogram = NULL;
    if( data != NULL){


        // Only looking at first component
        double* range = data->GetRange(0);

        histogram = vtkFloatArray::New();

        // number of bins is the bin where the maximum value will go plus the number of bins to be used for smoothing
        //int numBins = svkStatistics::GetBinForValue(range[1], binSize, startBin) + 1 + smoothBins;

        histogram->SetNumberOfTuples( numBins );

        // For now just support one (real) component....
        histogram->SetNumberOfComponents( 1 );
        histogram->FillComponent(0,0);
        for( int i = 0; i < data->GetNumberOfTuples(); i++ ) {
            if( data->GetTuple1(i) != 0 ) { // ignore zeroes
                int bin = svkStatistics::GetBinForValue(data->GetTuple1(i), binSize, startBin);
                if( bin >= 0 && bin < histogram->GetNumberOfTuples()) {
                    //Increment this bin
                    histogram->SetTuple1(bin, histogram->GetTuple1(bin) + 1 );
                }
            }
        }

        //Smooth bins must be greater than one. An odd window is always used.
        if( smoothBins > 1 ) {
            vtkFloatArray* smoothedHistogram = vtkFloatArray::New();
            // For now just support one (real) component....
            smoothedHistogram->SetNumberOfComponents( 1 );
            smoothedHistogram->SetNumberOfTuples( numBins );
            smoothedHistogram->FillComponent(0,0);

            // An even halfwidth is always used, so the actual number of bins used will always be odd
            int halfWidth = smoothBins/2;

            /*
             *  The histogram is smoothed in the following way:
             *  * The first bin in the smoothed histogram is the equal to the first bin in the original histogram.
             *  * The second bin uses the average of the first three bins.
             *  * From there the next two bins are added to the average until the halfWidth is reached
             *  * Once the the number of bins - halfwidth is reached bins are dropped from the average, two at a time
             *  * The last bin of the smoothed histogram is equal to the last bin of the original histogram
             */

            // First index of the smoothed histogram is equal to the first bin of the original histogram
            float runningSum = histogram->GetTuple1(0);
            int numBinsInSum = 1;
            smoothedHistogram->SetTuple1(0, runningSum/numBinsInSum);

            // Until halfWidth is reached add two more bins into the average, one on each side of the average
            for( int i = 1; i < halfWidth + 1; i++ ) {
                // Use all previous bins plus the next bin
                runningSum += histogram->GetTuple1(2*i -1 ) + histogram->GetTuple1(2*i);
                // We added two bins, so increment the number of bins by two.
                numBinsInSum += 2;
                smoothedHistogram->SetTuple1(i,runningSum/numBinsInSum);
            }
            for( int i = halfWidth + 1; i < numBins - halfWidth; i++ ) {
                // Remove the farthest bin used
                runningSum -= histogram->GetTuple1((i - 1 - halfWidth));

                // Add the next bin
                runningSum += histogram->GetTuple1((i + halfWidth));

                // bin sum should be equal to the number of smoothed bins
                smoothedHistogram->SetTuple1(i, runningSum/numBinsInSum);
            }

            // Now lets drop bins from the sum to compute the last bin
            // Each step the window size is made smaller so two bins are removed
            for( int i = halfWidth; i > 0; i--){
                // First Subtract the last two bins that were added
                runningSum -= histogram->GetTuple1(numBins-2*i-1);
                runningSum -= histogram->GetTuple1(numBins-2*i);
                numBinsInSum -= 2;
                smoothedHistogram->SetTuple1(numBins-i, runningSum/numBinsInSum);
            }
            histogram->DeepCopy(smoothedHistogram);
            smoothedHistogram->Delete();
        }
    }
    return histogram;
}


/*!
 * Returns the bin number in which the given value should be placed
 */
int svkStatistics::GetBinForValue( double value, double binSize, double startBin)
{
    return svkUtils::NearestInt((value - startBin)/binSize);
}


/*!
 * This method is to support a legacy feature to automatically adjust the binsize.
 */
double svkStatistics::GetAutoAdjustedBinSize( svkMriImageData* image, double startBinSize, double startBin, int numBins )
{
    double binSize = startBinSize;
    vtkImageAccumulate* accumulator = vtkImageAccumulate::New();
    accumulator->SetInputData( image );
    accumulator->Update( );
    accumulator->SetIgnoreZero( true );
    accumulator->SetComponentExtent(0,numBins-1,0,0,0,0 );
    accumulator->SetComponentOrigin(startBin, 0,0 );
    accumulator->SetComponentSpacing(binSize, 0,0);
    accumulator->Update();
    double min  = *accumulator->GetMin();
    double max  = *accumulator->GetMax();
    double mean = *accumulator->GetMean();
    vtkDataArray* hist = accumulator->GetOutput()->GetPointData()->GetScalars();
    double idealBinSize = (max-min)/((double)numBins);
    double histogramMaxBin = (double)numBins * startBinSize;
    if(histogramMaxBin < 10.0*mean) {
        histogramMaxBin = 10.0*mean;
    }
    if(max > histogramMaxBin) {
        idealBinSize = (histogramMaxBin-min)/((double)numBins);
    }
    if(max < histogramMaxBin) {
      histogramMaxBin = max;
    }
    int itemp = svkUtils::NearestInt((idealBinSize/binSize)+0.51);
    if(itemp > 1) {
        binSize= binSize * itemp;
    }
    string imageLabel = image->GetDcmHeader()->GetStringValue("SeriesDescription");
    accumulator->Delete();
    return binSize;
}

