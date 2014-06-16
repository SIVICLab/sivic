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


#include <svkImageStatistics.h>

using namespace svk;

vtkCxxRevisionMacro(svkImageStatistics, "$Rev$");
vtkStandardNewMacro(svkImageStatistics);

//! Constructor
svkImageStatistics::svkImageStatistics()
{
    this->SetNumberOfInputPorts(25);
    this->SetNumberOfOutputPorts(1);
    bool required = true;
    bool repeatable = true;
    this->GetPortMapper()->InitializeInputPort( INPUT_IMAGE, "INPUT_IMAGE", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, required, repeatable );
    this->GetPortMapper()->InitializeInputPort( INPUT_ROI, "INPUT_ROI", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, required, repeatable );
    this->GetPortMapper()->InitializeInputPort( NUM_BINS, "NUM_BINS", svkAlgorithmPortMapper::SVK_INT);
    this->GetPortMapper()->InitializeInputPort( BIN_SIZE, "BIN_SIZE", svkAlgorithmPortMapper::SVK_DOUBLE);
    this->GetPortMapper()->InitializeInputPort( START_BIN, "START_BIN", svkAlgorithmPortMapper::SVK_DOUBLE);
    this->GetPortMapper()->InitializeInputPort( SMOOTH_BINS, "SMOOTH_BINS", svkAlgorithmPortMapper::SVK_INT, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_HISTOGRAM, "COMPUTE_HISTOGRAM", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MEAN, "COMPUTE_MEAN", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MAX, "COMPUTE_MAX", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MIN, "COMPUTE_MIN", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_STDEV, "COMPUTE_STDEV", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_VOLUME, "COMPUTE_VOLUME", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MODE, "COMPUTE_MODE", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_QUANTILES, "COMPUTE_QUANTILES", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MEDIAN, "COMPUTE_MEDIAN", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_SUM, "COMPUTE_SUM", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MOMENT_2, "COMPUTE_MOMENT_2", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MOMENT_3, "COMPUTE_MOMENT_3", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MOMENT_4, "COMPUTE_MOMENT_4", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_VARIANCE, "COMPUTE_VARIANCE", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_SAMPLE_KURTOSIS, "COMPUTE_SAMPLE_KURTOSIS", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_SAMPLE_SKEWNESS, "COMPUTE_SAMPLE_SKEWNESS", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_POPULATION_KURTOSIS, "COMPUTE_POPULATION_KURTOSIS", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_POPULATION_SKEWNESS, "COMPUTE_POPULATION_SKEWNESS", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( OUTPUT_FILE_NAME, "OUTPUT_FILE_NAME", svkAlgorithmPortMapper::SVK_STRING, !required);
    this->GetPortMapper()->InitializeOutputPort( 0, "XML_RESULTS", svkAlgorithmPortMapper::SVK_XML);
}


//! Destructor
svkImageStatistics::~svkImageStatistics()
{
}

/*!
 *  This method takes every pixel within the input image data that correspond to values > 0 in the roi image.
 *  Only looks at the current scalars. Only values >= minIncluded and <= maxIncluded are included.
 */
vtkDataArray* svkImageStatistics::GetMaskedPixels( svkMriImageData* image, svkMriImageData* roi, double minIncluded, double maxIncluded)
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


/*!
 * Generates a histogram for the given vtkDataArray. If smoothBins is greater then one then the histogram is smoothed.
 * If smoothBins is even then the next highest odd number is used.
 */
vtkFloatArray* svkImageStatistics::GetHistogram( vtkDataArray* data, double binSize, double startBin, int numBins, int smoothBins)
{
    vtkFloatArray* histogram = NULL;
    if( data != NULL){

        // Only looking at first component
        double* range = data->GetRange(0);

        histogram = vtkFloatArray::New();

        // number of bins is the bin where the maximum value will go plus the number of bins to be used for smoothing
        //int numBins = svkImageStatistics::GetBinForValue(range[1], binSize, startBin) + 1 + smoothBins;

        histogram->SetNumberOfTuples( numBins );

        // For now just support one (real) component....
        histogram->SetNumberOfComponents( 1 );
        histogram->FillComponent(0,0);
        for( int i = 0; i < data->GetNumberOfTuples(); i++ ) {
            int bin = svkImageStatistics::GetBinForValue(data->GetTuple1(i), binSize, startBin);
            if( bin > 0 && bin < histogram->GetNumberOfTuples()) {
                //Increment this bin
                histogram->SetTuple1(bin, histogram->GetTuple1(bin) + 1 );
            } else {
                cout << "ERROR: bin " << bin << " Is outside of range: " << numBins << " or " << histogram->GetNumberOfTuples() << endl;
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
int svkImageStatistics::GetBinForValue( double value, double binSize, double startBin)
{
    // Legacy code uses the nearest integer...
    //return (int)floor((value - startBin)/binSize);
    return svkUtils::NearestInt((value - startBin)/binSize);
}


/*
*/
vtkXMLDataElement* svkImageStatistics::GetOutput()
{
   vtkXMLDataElement* output = NULL;
   svkXML* outputDataObject = svkXML::SafeDownCast(this->GetOutputDataObject(0));
   if( outputDataObject != NULL ) {
       output = outputDataObject->GetValue();
   }
   return output;
}


/*!
 *  RequestData pass the input through the algorithm, and copies the dcos and header
 *  to the output.
 */
int svkImageStatistics::RequestData( vtkInformation* request,
                                              vtkInformationVector** inputVector,
                                              vtkInformationVector* outputVector )
{
    vtkXMLDataElement* results = this->GetOutput();

    results->RemoveAllNestedElements();
    results->RemoveAllAttributes();
    results->SetName("svk_image_stats_results");

    results->SetAttribute("version", string(SVK_RELEASE_VERSION).c_str());
    time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    string timeString = asctime(timeinfo);
    // Chop off the return character from asctime:
    timeString = timeString.substr(0, timeString.size()-1);
    results->SetAttribute("date", timeString.c_str());

    for (int imageIndex = 0; imageIndex < this->GetNumberOfInputConnections(INPUT_IMAGE); imageIndex++) {
        for (int roiIndex = 0; roiIndex < this->GetNumberOfInputConnections(INPUT_ROI); roiIndex++) {
            svkMriImageData* image = this->GetPortMapper()->GetMRImageInputPortValue(INPUT_IMAGE, imageIndex);
            svkMriImageData* roi   = this->GetPortMapper()->GetMRImageInputPortValue(INPUT_ROI, roiIndex);
            vtkXMLDataElement* nextResult = vtkXMLDataElement::New();
            nextResult->SetName("results");
            string imageLabel = image->GetDcmHeader()->GetStringValue("SeriesDescription");
            string roiLabel = roi->GetDcmHeader()->GetStringValue("SeriesDescription");
            svkUtils::CreateNestedXMLDataElement( nextResult, "IMAGE", imageLabel);
            svkUtils::CreateNestedXMLDataElement( nextResult, "ROI",   roiLabel);
            vtkXMLDataElement* statistics = vtkXMLDataElement::New();
            bool geometriesMatch = true;
            if( roi != NULL ) {
                svkDataValidator* validator = svkDataValidator::New();
                if( !validator->AreDataGeometriesSame( image, roi ) ) {
                    cout << "ERROR: Data geometries do not match between image and ROI. Cannot compute order statistics:" << validator->resultInfo << endl;
                    geometriesMatch = false;
                }
                validator->Delete();
            }
            statistics->SetName("measures");
            if( geometriesMatch ) {
                if( this->GetPortMapper()->GetIntInputPortValue( SMOOTH_BINS ) != NULL ){;
                    this->ComputeSmoothStatistics(image,roi, statistics);
                } else {
                    this->ComputeOrderStatistics(image,roi, statistics);
                }
                this->ComputeAccumulateStatistics(image,roi, statistics);
                this->ComputeDescriptiveStatistics(image,roi, statistics);
            }
            vtkIndent indent;
            if( statistics != NULL ) {
                nextResult->AddNestedElement( statistics );
            }
            results->AddNestedElement( nextResult );
            nextResult->Delete();
        }
    }

    vtkIndent indent;
    svkString* outputFileName = this->GetPortMapper()->GetStringInputPortValue(OUTPUT_FILE_NAME);
    if( outputFileName != NULL && outputFileName->GetValue() != "") {
        vtkXMLUtilities::WriteElementToFile( results, outputFileName->GetValue().c_str());
    }
    return 1;
}


/*!
 * Computes basic statistics using vtkImageAccumulate.
 */
void svkImageStatistics::ComputeAccumulateStatistics(svkMriImageData* image, svkMriImageData* roi, vtkXMLDataElement* results)
{
    if( image != NULL ) {
        double* spacing = image->GetSpacing();
        double pixelVolume = spacing[0] * spacing[1] * spacing[2];
        vtkImageAccumulate* accumulator = vtkImageAccumulate::New();
        accumulator->SetInput( image );
        if( roi != NULL ) {
            vtkImageToImageStencil* stencil = vtkImageToImageStencil::New();
            stencil->SetInput( roi );
            stencil->ThresholdByUpper(1);
            stencil->Update();
            accumulator->SetStencil( stencil->GetOutput() );
            stencil->Delete();
        }
        accumulator->Update( );
        accumulator->SetIgnoreZero( false );
        int numBins = this->GetPortMapper()->GetIntInputPortValue( NUM_BINS )->GetValue();
        double startBin  = this->GetPortMapper()->GetDoubleInputPortValue( START_BIN )->GetValue();
        double binSize   = this->GetPortMapper()->GetDoubleInputPortValue( BIN_SIZE )->GetValue();
        accumulator->SetComponentExtent(0,numBins-1,0,0,0,0 );
        accumulator->SetComponentOrigin(startBin, 0,0 );
        accumulator->SetComponentSpacing(binSize, 0,0);
        accumulator->Update();
        if( this->GetShouldCompute(COMPUTE_VOLUME)) {
            string volumeString = svkUtils::DoubleToString( accumulator->GetVoxelCount()*pixelVolume );
            svkUtils::CreateNestedXMLDataElement( results, "volume", volumeString);
        }

        if( this->GetShouldCompute(COMPUTE_MAX)) {
            string maxString = svkUtils::DoubleToString( *accumulator->GetMax() );
            svkUtils::CreateNestedXMLDataElement( results, "max", maxString);
        }


        if( this->GetShouldCompute(COMPUTE_MIN)) {
            string minString = svkUtils::DoubleToString( *accumulator->GetMin() );
            svkUtils::CreateNestedXMLDataElement( results, "min", minString);
        }

        if( this->GetShouldCompute(COMPUTE_MEAN)) {
            string meanString = svkUtils::DoubleToString( *accumulator->GetMean() );
            svkUtils::CreateNestedXMLDataElement( results, "mean", meanString);
        }

        if( this->GetShouldCompute(COMPUTE_STDEV)) {
            string stdevString = svkUtils::DoubleToString( *accumulator->GetStandardDeviation() );
            svkUtils::CreateNestedXMLDataElement( results, "stdev", stdevString);
        }

        if( this->GetShouldCompute(COMPUTE_MODE) && this->GetPortMapper()->GetIntInputPortValue( SMOOTH_BINS ) == NULL ) {
            vtkDataArray* histData = accumulator->GetOutput()->GetPointData()->GetScalars();
            double max = *accumulator->GetMax();
            double min = *accumulator->GetMin();
            int numBins =  histData->GetNumberOfTuples();
            accumulator->Update();
            if( numBins > 0 && histData != NULL ) {
                double mode = 0;
                double binMax = 0;
                for( int i = 0; i < numBins; i++ ) {
                    if( histData->GetTuple1(i) >= binMax ) {
                        binMax = histData->GetTuple1(i);
                        mode = binSize*i + startBin;
                    }
                }
                string valueString = svkUtils::DoubleToString(mode);
                svkUtils::CreateNestedXMLDataElement( results, "mode", valueString);
            }
        }

        // Only output the histogram here of the smoothing was not used. Otherwise the smoothed histogram will be output
        if( this->GetShouldCompute(COMPUTE_HISTOGRAM) && this->GetPortMapper()->GetIntInputPortValue( SMOOTH_BINS ) == NULL ){;
            vtkDataArray* histData = accumulator->GetOutput()->GetPointData()->GetScalars();
            this->AddHistogramTag( histData, binSize, startBin, numBins, 0, results);
        }
        accumulator->Delete();
    }
}


/*!
 * Computes statistics using the vtkOrderStatistics class.
 */
void svkImageStatistics::ComputeOrderStatistics(svkMriImageData* image, svkMriImageData* roi, vtkXMLDataElement* results)
{
    if( image != NULL ) {
        vtkDataArray* pixelsInROI = svkImageStatistics::GetMaskedPixels(image,roi);
        vtkTable* table = vtkTable::New();
        table->AddColumn( pixelsInROI );


        // Compute Quartiles
        vtkOrderStatistics* quartileStatsCalculator = vtkOrderStatistics::New();
        quartileStatsCalculator->SetInput( vtkStatisticsAlgorithm::INPUT_DATA, table );
        quartileStatsCalculator->AddColumn(pixelsInROI->GetName());
        quartileStatsCalculator->SetLearnOption(true);
        quartileStatsCalculator->SetAssessOption(false);
        quartileStatsCalculator->Update();
        vtkTable* quartileResults = quartileStatsCalculator->GetOutput(1);

        /*
         // This is usefull for debuging. It prints all the results of the algroithm out.
        for( int i = 0; i < quartileResults->GetNumberOfRows(); i++) {
            for( int j = 0; j < quartileResults->GetNumberOfColumns(); j++) {
                cout << quartileResults->GetColumnName(j) << ": " << quartileResults->GetValue(i,j) << endl;
            }
        }
        */

        // Compute deciles
        vtkOrderStatistics* decileStatsCalculator = vtkOrderStatistics::New();
        decileStatsCalculator->SetInput( vtkStatisticsAlgorithm::INPUT_DATA, table );
        decileStatsCalculator->AddColumn(pixelsInROI->GetName());
        decileStatsCalculator->SetLearnOption(true);
        decileStatsCalculator->SetAssessOption(false);
        decileStatsCalculator->SetNumberOfIntervals(10);
        decileStatsCalculator->Update();
        vtkTable* decileResults = decileStatsCalculator->GetOutput(1);

        /*
         // This is usefull for debuging. It prints all the results of the algroithm out.
        for( int i = 0; i < decileResults->GetNumberOfRows(); i++) {
            for( int j = 0; j < decileResults->GetNumberOfColumns(); j++) {
                cout << decileResults->GetColumnName(j) << ": " << decileResults->GetValue(i,j) << endl;
            }
        }
        */

        if( this->GetShouldCompute(COMPUTE_QUANTILES)) {
            string valueString = svkUtils::DoubleToString( decileResults->GetValueByName(0,"0.1-quantile").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "tenthPercentile", valueString);
            valueString = svkUtils::DoubleToString( quartileResults->GetValueByName(0,"First Quartile").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "firstQuartile", valueString);
            valueString = svkUtils::DoubleToString( quartileResults->GetValueByName(0,"Median").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "median", valueString);
            valueString = svkUtils::DoubleToString( quartileResults->GetValueByName(0,"Third Quartile").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "thirdQuartile", valueString);
            valueString = svkUtils::DoubleToString( decileResults->GetValueByName(0,"0.9-quantile").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "ninetiethPercentile", valueString);
        } else if( this->GetShouldCompute(COMPUTE_MEDIAN)) {
            string valueString = svkUtils::DoubleToString( quartileResults->GetValueByName(0,"Median").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "median", valueString);
        }


        decileStatsCalculator->Delete();
        quartileStatsCalculator->Delete();
        table->Delete();
        pixelsInROI->Delete();
    }
}


/*!
 * Computes statistics using the vtkDescriptiveStatistics class.
 */
void svkImageStatistics::ComputeDescriptiveStatistics(svkMriImageData* image, svkMriImageData* roi, vtkXMLDataElement* results)
{
    if( image != NULL ) {
        vtkDataArray* pixels = image->GetPointData()->GetScalars();
        vtkDataArray* mask   = NULL;
        if( roi != NULL ) {
            mask  = roi->GetPointData()->GetScalars();
        }
        vtkDataArray* pixelsInROI = vtkDataArray::CreateDataArray( pixels->GetDataType());
        pixelsInROI->SetName("PixelsInROI");
        pixelsInROI->SetNumberOfComponents(1);
        int count = 0;
        for( int i = 0; i < pixels->GetNumberOfTuples(); i++ ) {
            if( mask == NULL || mask->GetTuple1(i) > 0 ) {
                count ++;
                pixelsInROI->InsertNextTuple1( pixels->GetTuple1(i));
            }
        }
        //cout << "mask had " << count << " pixels" << endl;
        vtkTable* table = vtkTable::New();
        table->AddColumn( pixelsInROI );
        vtkDescriptiveStatistics* descriptiveStats = vtkDescriptiveStatistics::New();
        descriptiveStats->SetInput( vtkStatisticsAlgorithm::INPUT_DATA, table );
        descriptiveStats->AddColumn(pixelsInROI->GetName());
        descriptiveStats->SetLearnOption(true);
        descriptiveStats->SetAssessOption(false);
        descriptiveStats->Update();
        vtkTable* statResults = descriptiveStats->GetOutput(1);
        /*
        for( int i = 0; i < statResults->GetNumberOfRows(); i++) {
            for( int j = 0; j < statResults->GetNumberOfColumns(); j++) {
                cout << statResults->GetColumnName(j) << ": " << statResults->GetValue(i,j) << endl;
            }
        }
        */
        if( this->GetShouldCompute(COMPUTE_SUM)){
            string valueString = svkUtils::DoubleToString( statResults->GetValueByName(0,"Sum").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "sum", valueString);
        }
        if( this->GetShouldCompute(COMPUTE_MOMENT_2)){
            string valueString = svkUtils::DoubleToString( statResults->GetValueByName(0,"M2").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "moment2", valueString);
        }
        if( this->GetShouldCompute(COMPUTE_MOMENT_3)){
            string valueString = svkUtils::DoubleToString( statResults->GetValueByName(0,"M3").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "moment3", valueString);
        }
        if( this->GetShouldCompute(COMPUTE_MOMENT_4)){
            string valueString = svkUtils::DoubleToString( statResults->GetValueByName(0,"M4").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "moment4", valueString);
        }
        if( this->GetShouldCompute(COMPUTE_VARIANCE)){
            string valueString = svkUtils::DoubleToString( statResults->GetValueByName(0,"Variance").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "variance", valueString);
        }
        if( this->GetShouldCompute(COMPUTE_SAMPLE_KURTOSIS)){
            string valueString = svkUtils::DoubleToString( statResults->GetValueByName(0,"g2 Kurtosis").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "samplekurtosis", valueString);
        }
        if( this->GetShouldCompute(COMPUTE_SAMPLE_SKEWNESS)){
            string valueString = svkUtils::DoubleToString( statResults->GetValueByName(0,"g1 Skewness").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "sampleskewness", valueString);
        }
        if( this->GetShouldCompute(COMPUTE_POPULATION_KURTOSIS)){
            string valueString = svkUtils::DoubleToString( statResults->GetValueByName(0,"G2 Kurtosis").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "populationkurtosis", valueString);
        }
        if( this->GetShouldCompute(COMPUTE_POPULATION_SKEWNESS)){
            string valueString = svkUtils::DoubleToString( statResults->GetValueByName(0,"G1 Skewness").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "populationskewness", valueString);
        }
    }
}


/*!
 * Do smooth computation.
 */
void svkImageStatistics::ComputeSmoothStatistics(svkMriImageData* image, svkMriImageData* roi, vtkXMLDataElement* results)
{
    double startBin  = this->GetPortMapper()->GetDoubleInputPortValue( START_BIN )->GetValue();
    double binSize   = this->GetPortMapper()->GetDoubleInputPortValue( BIN_SIZE )->GetValue();
    int numBins = this->GetPortMapper()->GetIntInputPortValue( NUM_BINS )->GetValue();
    int smoothBins = 0;
    if( this->GetPortMapper()->GetIntInputPortValue( SMOOTH_BINS ) != NULL ){;
        smoothBins = this->GetPortMapper()->GetIntInputPortValue( SMOOTH_BINS )->GetValue();
    }
    if( image != NULL ) {
        vtkDataArray* pixelsInROI = svkImageStatistics::GetMaskedPixels(image,roi);
        vtkDataArray* histogram = svkImageStatistics::GetHistogram( pixelsInROI, binSize, startBin, numBins, smoothBins );
        if( pixelsInROI != NULL && histogram != NULL) {
            // Sort the data
            vtkSortDataArray::Sort( pixelsInROI );

            // Compute the mode if requested
            if( this->GetShouldCompute(COMPUTE_MODE)) {
                int numBins = histogram->GetNumberOfTuples();
                int modeBin = 0;
                // Find the moax for the mode
                for( int i = 0; i < numBins; i++ ) {
                    if( histogram->GetTuple1(i) >= histogram->GetTuple1(modeBin)) {
                        modeBin = i;
                    }
                }
                string valueString = svkUtils::DoubleToString( binSize*modeBin + startBin );
                svkUtils::CreateNestedXMLDataElement( results, "mode", valueString);
            }

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

            // Now lets find the border of the percentiles be
            int numIntervals = 20; // Compute every 5%
            vector<double> intervalUpperBin;
            vector<double> intervalLowerBin;
            //Initialize to zeros
            intervalUpperBin.assign(numIntervals,0);
            intervalLowerBin.assign(numIntervals,0);

            // Let's use the cdf to compute percentiles....

            // Approach from the lower values
            for( int i = 0; i < numBins; i++ ) {
                for( int j = 0; j < numIntervals; j++ ) {
                    double intervalMax = double(j)/numIntervals;
                    if( ncdf->GetTuple1(i) < intervalMax ) {
                        intervalLowerBin[j] = i;
                    }
                }
            }

            // Approach from the upper values
            for( int i = numBins -1; i >= 0; i-- ) {
                for( int j = 0; j < numIntervals; j++ ) {
                    double intervalMin = double(j)/numIntervals;
                    if( ncdf->GetTuple1(i) > intervalMin ) {
                        intervalUpperBin[j] = i;
                    }
                }
            }

            for( int i = 0; i < numIntervals; i++ ) {
                double percentile = binSize*((intervalUpperBin[i] + intervalLowerBin[i])/2.0) + startBin;
                if( this->GetShouldCompute(COMPUTE_QUANTILES)) {
                    if( ((double)i)/numIntervals == 0.1 ) {
                        string valueString = svkUtils::DoubleToString( percentile );
                        svkUtils::CreateNestedXMLDataElement( results, "tenthPercentile", valueString);
                    }
                    if( ((double)i)/numIntervals == 0.25 ) {
                        string valueString = svkUtils::DoubleToString( percentile );
                        svkUtils::CreateNestedXMLDataElement( results, "firstQuartile", valueString);
                    }
                    if( ((double)i)/numIntervals == 0.5 ) {
                        string valueString = svkUtils::DoubleToString( percentile );
                        svkUtils::CreateNestedXMLDataElement( results, "median", valueString);
                    }
                    if( ((double)i)/numIntervals == 0.75 ) {
                        string valueString = svkUtils::DoubleToString( percentile );
                        svkUtils::CreateNestedXMLDataElement( results, "thirdQuartile", valueString);
                    }
                    if( ((double)i)/numIntervals == 0.90 ) {
                        string valueString = svkUtils::DoubleToString( percentile );
                        svkUtils::CreateNestedXMLDataElement( results, "nintiethPercentile", valueString);
                    }
                } else if( this->GetShouldCompute(COMPUTE_MEDIAN)) {
                    if( ((double)i)/numIntervals == 0.5 ) {
                        string valueString = svkUtils::DoubleToString( percentile );
                        svkUtils::CreateNestedXMLDataElement( results, "median", valueString);
                    }

                }
            }
            if( this->GetShouldCompute(COMPUTE_HISTOGRAM)) {
                this->AddHistogramTag( histogram, binSize, startBin, numBins, 0, results);
            }
            /*
             *  This computes 10th percentile exactly.
            double numPixels = pixelsInROI->GetNumberOfTuples();
            for( int i = 0; i < numPixels; i++ ) {
                if( 100.0* ((i+1)/numPixels) > 10){
                    cout <<"No Histogram:" << 100.0* (i/numPixels) << " value= " << pixelsInROI->GetTuple1(i) << endl;
                    cout <<"No Histogram:" << 100.0* ((i+1)/numPixels) << " value= " << pixelsInROI->GetTuple1(i+1) << endl;
                break;
                }
            }
            */
            pixelsInROI->Delete();

        }
    }
}


/*!
 * Inserts the histogram tag into the result
 */
void svkImageStatistics::AddHistogramTag( vtkDataArray* histogram, double binSize, double startBin, int numBins, int smoothBins, vtkXMLDataElement* results)
{
    if( histogram != NULL ) {
        int numBins =  histogram->GetNumberOfTuples();
        vtkXMLDataElement* histTag = vtkXMLDataElement::New();
        histTag->SetName("histogram");
        histTag->SetAttribute("bins", svkUtils::IntToString(numBins).c_str());
        if( smoothBins != NULL ) {
            histTag->SetAttribute("smoothBins", svkUtils::IntToString(smoothBins).c_str());
        }
        for( int i = 0; i < numBins; i++ ) {
            vtkXMLDataElement* element = vtkXMLDataElement::New();
            element->SetName("bin");
            element->SetAttribute("index", svkUtils::IntToString(i).c_str());
            element->SetAttribute("min", svkUtils::DoubleToString(startBin + i*binSize).c_str());
            element->SetAttribute("max", svkUtils::DoubleToString(startBin + (i+1)*binSize).c_str());
            string valueString = svkUtils::DoubleToString(histogram->GetTuple1(i));
            element->SetCharacterData(valueString.c_str(), valueString.size());
            histTag->AddNestedElement(element);
            element->Delete();
        }
        results->AddNestedElement(histTag);
        histTag->Delete();
    }
}


/*!
 * Utility method to check to see if the option has been set to compute a given metric.
 */
bool svkImageStatistics::GetShouldCompute( svkImageStatistics::svkImageStatisticsParameter parameter)
{
    return (this->GetPortMapper()->GetBoolInputPortValue(parameter) && this->GetPortMapper()->GetBoolInputPortValue(parameter)->GetValue());
}
