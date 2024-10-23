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


#include <svkImageStatistics.h>
#include <svkTypeUtils.h>
#include </usr/include/vtk/vtkMultiBlockDataSet.h>

using namespace svk;

//vtkCxxRevisionMacro(svkImageStatistics, "$Rev$");
vtkStandardNewMacro(svkImageStatistics);

//! Constructor
svkImageStatistics::svkImageStatistics()
{
    this->SetNumberOfInputPorts(28);
    this->SetNumberOfOutputPorts(1);
    bool required = true;
    bool repeatable = true;
    this->GetPortMapper()->InitializeInputPort( INPUT_IMAGE, "INPUT_IMAGE", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, required, repeatable );
    this->GetPortMapper()->InitializeInputPort( INPUT_ROI, "INPUT_ROI", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, required, repeatable );
    this->GetPortMapper()->InitializeInputPort( NUM_BINS, "NUM_BINS", svkAlgorithmPortMapper::SVK_INT);
    this->GetPortMapper()->InitializeInputPort( BIN_SIZE, "BIN_SIZE", svkAlgorithmPortMapper::SVK_DOUBLE, required, repeatable);
    this->GetPortMapper()->InitializeInputPort( NORMALIZATION_IMAGE_INDEX, "NORMALIZATION_IMAGE_INDEX", svkAlgorithmPortMapper::SVK_INT, !required, repeatable);
    this->GetPortMapper()->InitializeInputPort( IGNORE_NEGATIVE_NUMBERS, "IGNORE_NEGATIVE_NUMBERS", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( AUTO_ADJUST_BIN_SIZE, "AUTO_ADJUST_BIN_SIZE", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( START_BIN, "START_BIN", svkAlgorithmPortMapper::SVK_DOUBLE, !required);
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
    this->GetPortMapper()->InitializeInputPort( COMPUTE_KURTOSIS, "COMPUTE_KURTOSIS", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_SKEWNESS, "COMPUTE_SKEWNESS", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( NORMALIZATION_METHOD, "NORMALIZATION_METHOD", svkAlgorithmPortMapper::SVK_INT, !required);
    this->GetPortMapper()->InitializeInputPort( NORMALIZATION_ROI_INDEX, "NORMALIZATION_ROI_INDEX", svkAlgorithmPortMapper::SVK_INT, !required);
    this->GetPortMapper()->InitializeInputPort( OUTPUT_FILE_NAME, "OUTPUT_FILE_NAME", svkAlgorithmPortMapper::SVK_STRING, !required);
    this->GetPortMapper()->InitializeOutputPort( 0, "XML_RESULTS", svkAlgorithmPortMapper::SVK_XML);
}


//! Destructor
svkImageStatistics::~svkImageStatistics()
{
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
    vector<double> normalizationFactors;
    for (int imageIndex = 0; imageIndex < this->GetNumberOfInputConnections(INPUT_IMAGE); imageIndex++) {
        double normalizationFactor = 0;
        int normalizationImageIndex = imageIndex;
        if( this->GetPortMapper()->GetIntInputPortValue( NORMALIZATION_IMAGE_INDEX, imageIndex )){
            normalizationImageIndex = this->GetPortMapper()->GetIntInputPortValue( NORMALIZATION_IMAGE_INDEX, imageIndex )->GetValue();
            if( normalizationImageIndex > imageIndex ) {
                cout << "ERROR! Normalization Image Index cannot be greater than the image index." << endl;
                exit(1);
            }
        }
        vector<int> roiIndicies;
        int normalizationROIIndex = 0;
        int normalizationType = NONE;
        if( this->GetPortMapper()->GetIntInputPortValue(NORMALIZATION_ROI_INDEX) != NULL ) {
            normalizationROIIndex = this->GetPortMapper()->GetIntInputPortValue(NORMALIZATION_ROI_INDEX)->GetValue();
            if(this->GetPortMapper()->GetIntInputPortValue(NORMALIZATION_METHOD) != NULL ) {
                normalizationType = this->GetPortMapper()->GetIntInputPortValue(NORMALIZATION_METHOD)->GetValue();
            }
        }
        double binSize   = this->GetPortMapper()->GetDoubleInputPortValue( BIN_SIZE, imageIndex )->GetValue();
        int numBins = this->GetPortMapper()->GetIntInputPortValue( NUM_BINS )->GetValue();
        svkMriImageData* image = this->GetPortMapper()->GetMRImageInputPortValue(INPUT_IMAGE, imageIndex);
        if( this->GetShouldCompute(IGNORE_NEGATIVE_NUMBERS) ) {
            for( int i = 0; i < image->GetPointData()->GetScalars()->GetNumberOfTuples(); i++ ) {
                if( image->GetPointData()->GetScalars()->GetTuple1(i) < 0 ) {
                    image->GetPointData()->GetScalars()->SetTuple1(i,0);
                }
            }
        }
        double startBin = image->GetPointData()->GetScalars()->GetRange()[0];
        if( this->GetPortMapper()->GetDoubleInputPortValue( START_BIN ) != NULL ) {
            startBin = this->GetPortMapper()->GetDoubleInputPortValue( START_BIN )->GetValue();
        }
        if( this->GetShouldCompute(AUTO_ADJUST_BIN_SIZE)) {
            binSize = svkStatistics::GetAutoAdjustedBinSize( image, binSize, startBin, numBins );
        }
        // Let's make sure the normalization values are computed before the other statistics.
        roiIndicies.push_back( normalizationROIIndex );
        for(int i = 0; i < this->GetNumberOfInputConnections(INPUT_ROI); i++ ) {
            if( i != normalizationROIIndex ) {
                roiIndicies.push_back(i);
            }
        }
        // Look through the indicies, starting with the normalization index if present
        for (int roiIndex = 0; roiIndex < roiIndicies.size(); roiIndex++) {
            svkMriImageData* roi   = this->GetPortMapper()->GetMRImageInputPortValue(INPUT_ROI, roiIndicies[roiIndex]);
            vtkXMLDataElement* nextResult = vtkXMLDataElement::New();
            nextResult->SetName("results");
            string imageLabel = image->GetDcmHeader()->GetStringValue("SeriesDescription");
            string roiLabel = roi->GetDcmHeader()->GetStringValue("SeriesDescription");
            svkXMLUtils::CreateNestedXMLDataElement( nextResult, "IMAGE", imageLabel);
            svkXMLUtils::CreateNestedXMLDataElement( nextResult, "ROI",   roiLabel);
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
            vtkDataArray* maskedPixels = svkStatistics::GetMaskedPixels(image,roi);
            if( geometriesMatch ) {
                this->ComputeSmoothStatistics(image,roi, binSize, maskedPixels, statistics);
                this->ComputeAccumulateStatistics(image,roi, binSize, maskedPixels, statistics);
                this->ComputeDescriptiveStatistics(image,roi, maskedPixels, statistics);
            }
            if( normalizationType != NONE ) {
                if( roiIndex == 0 ) {
                    //EXTRACT NORMALIZATION FACTOR FROM XML
                    string normalizationElementName = "";
                    if( normalizationType == MODE ) {
                        normalizationElementName = "mode";
                    } else if (normalizationType == MEAN ) {
                        normalizationElementName = "mean";
                    } else {
                        cout << "ERROR: Normalization Type not recognized..." << endl;
                        exit(1);
                    }
                    vtkXMLDataElement* normalizationElement = statistics->LookupElementWithName( normalizationElementName.c_str() );
                    if( normalizationElement != NULL ) {
                        normalizationFactor = svkTypeUtils::StringToDouble(normalizationElement->GetCharacterData());
                    } else {
                        cout << "ERROR: Normalization element not found! " << endl;
                    }
                    normalizationFactors.push_back(normalizationFactor);
                }
                this->ComputeSmoothStatistics(image,roi, binSize, maskedPixels, statistics, normalizationFactors[normalizationImageIndex]);
                this->ComputeAccumulateStatistics(image,roi, binSize, maskedPixels, statistics, normalizationFactors[normalizationImageIndex]);
            }
            maskedPixels->Delete();
            vtkIndent indent;
            if( statistics != NULL ) {
                nextResult->AddNestedElement( statistics );
            }
            results->AddNestedElement( nextResult );
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
void svkImageStatistics::ComputeAccumulateStatistics(svkMriImageData* image, svkMriImageData* roi, double binSize, vtkDataArray* maskedPixels, vtkXMLDataElement* results, double normalization )
{
    if( image != NULL ) {
        double* spacing = image->GetSpacing();
        double pixelVolume = spacing[0] * spacing[1] * spacing[2];
        vtkImageAccumulate* accumulator = vtkImageAccumulate::New();
        accumulator->SetInputData( image );
        if( roi != NULL ) {
            vtkImageToImageStencil* stencil = vtkImageToImageStencil::New();
            stencil->SetInputData( roi );
            stencil->ThresholdByUpper(1);
            stencil->Update();
            accumulator->SetStencilData( stencil->GetOutput() );
            stencil->Delete();
        }
        accumulator->Update( );
        accumulator->SetIgnoreZero( true );
        int numBins = this->GetPortMapper()->GetIntInputPortValue( NUM_BINS )->GetValue();
        double startBin = image->GetPointData()->GetScalars()->GetRange()[0];
        if( this->GetPortMapper()->GetDoubleInputPortValue( START_BIN ) != NULL ) {
            startBin = this->GetPortMapper()->GetDoubleInputPortValue( START_BIN )->GetValue();
        }
        accumulator->SetComponentExtent(0,numBins-1,0,0,0,0 );
        accumulator->SetComponentOrigin(startBin, 0,0 );
        accumulator->SetComponentSpacing(binSize, 0,0);
        accumulator->Update();
        if( this->GetShouldCompute(COMPUTE_VOLUME)) {
            // Volume In Cubic Centimeters
            string volumeString = this->DoubleToString( (accumulator->GetVoxelCount()*pixelVolume)/1000.0 );
            svkXMLUtils::CreateNestedXMLDataElement( results, "volume", volumeString);
        }

        if( this->GetShouldCompute(COMPUTE_MAX)) {
            double max =  0;
            if( accumulator->GetVoxelCount() > 0 ) {
                max = *accumulator->GetMax();
            }
            if( normalization != 0 ) {
                max /= normalization;
            }
            string maxString = this->DoubleToString( max );
            vtkXMLDataElement* elem = svkXMLUtils::CreateNestedXMLDataElement( results, "max", maxString);
            if( normalization != 0 ) {
                elem->SetAttribute( "normalization", this->DoubleToString( normalization ).c_str());
            }
        }


        if( this->GetShouldCompute(COMPUTE_MIN)) {
            double min = 0;
            if( accumulator->GetVoxelCount() > 0 ) {
                min = *accumulator->GetMin();
            }
            if( normalization != 0 ) {
                min /= normalization;
            }
            string minString = this->DoubleToString( min );
            vtkXMLDataElement* elem = svkXMLUtils::CreateNestedXMLDataElement( results, "min", minString);
            if( normalization != 0 ) {
                elem->SetAttribute( "normalization", this->DoubleToString( normalization ).c_str());
            }
        }

        if( this->GetShouldCompute(COMPUTE_MEAN)) {
            double mean = *accumulator->GetMean();
            if( normalization != 0 ) {
                mean /= normalization;
            }
            string meanString = this->DoubleToString( mean );
            vtkXMLDataElement* elem = svkXMLUtils::CreateNestedXMLDataElement( results, "mean", meanString);
            if( normalization != 0 ) {
                elem->SetAttribute( "normalization", this->DoubleToString( normalization ).c_str());
            }
        }
        if( this->GetShouldCompute(COMPUTE_SUM)){
            // TODO: This is the method used it UCSF code. This should be changed to be a separate sum option.
            double mean = *accumulator->GetMean();
            double volume = (accumulator->GetVoxelCount()*pixelVolume)/1000.0;
            double sum = mean*volume;
            if( normalization != 0 ) {
                sum /= normalization;
            }
            string sumString = this->DoubleToString( sum );
            vtkXMLDataElement* elem = svkXMLUtils::CreateNestedXMLDataElement( results, "sum", sumString);
            if( normalization != 0 ) {
                elem->SetAttribute( "normalization", this->DoubleToString( normalization ).c_str());
            }
        }

        if( this->GetShouldCompute(COMPUTE_STDEV)) {
            double stdev = *accumulator->GetStandardDeviation();
            if( normalization != 0 ) {
                stdev /= normalization;
            }
            string stdevString = this->DoubleToString( stdev );
            vtkXMLDataElement* elem = svkXMLUtils::CreateNestedXMLDataElement( results, "sd", stdevString);
            if( normalization != 0 ) {
                elem->SetAttribute( "normalization", this->DoubleToString( normalization ).c_str());
            }
        }
        /*
        // This produces a different result from legacy code due to nint vs floor in finding bins
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
                string valueString = this->DoubleToString(mode);
                svkXMLUtils::CreateNestedXMLDataElement( results, "mode", valueString);
            }
        }

        // Only output the histogram here of the smoothing was not used. Otherwise the smoothed histogram will be output
        if( this->GetShouldCompute(COMPUTE_HISTOGRAM) && this->GetPortMapper()->GetIntInputPortValue( SMOOTH_BINS ) == NULL ){;
            vtkDataArray* histData = accumulator->GetOutput()->GetPointData()->GetScalars();
            this->AddHistogramTag( histData, binSize, startBin, numBins, 0, results);
        }
        */


        accumulator->Delete();
    }
}


/*!
 * Computes statistics using the vtkDescriptiveStatistics class.
 */
void svkImageStatistics::ComputeDescriptiveStatistics(svkMriImageData* image, svkMriImageData* roi, vtkDataArray* maskedPixels, vtkXMLDataElement* results )
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
        int numPixelsInROI = 0;
        //TODO: This should use the method for getting the pixels in the roi.
        for( int i = 0; i < pixels->GetNumberOfTuples(); i++ ) {
            // Get valuse in mask, if there is a mask. Ignore values of zero.
            if( (mask == NULL || mask->GetTuple1(i) > 0) &&  pixels->GetTuple1(i) != 0) {
                numPixelsInROI ++;
                pixelsInROI->InsertNextTuple1( pixels->GetTuple1(i));
            }
        }
        vtkTable* table = vtkTable::New();
        table->AddColumn( pixelsInROI );
        vtkDescriptiveStatistics* descriptiveStats = vtkDescriptiveStatistics::New();
        descriptiveStats->SetInputData( vtkStatisticsAlgorithm::INPUT_DATA, table );
        descriptiveStats->AddColumn(pixelsInROI->GetName());
        descriptiveStats->SetLearnOption(true);
        descriptiveStats->SetAssessOption(false);
        descriptiveStats->Update();
        vtkMultiBlockDataSet* statModel = vtkMultiBlockDataSet::SafeDownCast( descriptiveStats->GetOutputDataObject( vtkStatisticsAlgorithm::OUTPUT_MODEL ) );
        vtkTable* statResults = vtkTable::SafeDownCast( statModel->GetBlock( 0 ) );
        vtkTable* statDerivedResults = vtkTable::SafeDownCast( statModel->GetBlock( 1 ) );

        if( this->GetShouldCompute(COMPUTE_MOMENT_2) && numPixelsInROI > 0 ) {
            if( statResults->GetRowData()->GetArray("M2")->GetNumberOfTuples() > 0 ){
                string valueString = this->DoubleToString( statResults->GetValueByName(0,"M2").ToDouble() );
                svkXMLUtils::CreateNestedXMLDataElement( results, "moment2", valueString);
            } else {
                cout << "WARNING: Could not calculate M2 " << endl;
            }
        }
        if( this->GetShouldCompute(COMPUTE_MOMENT_3) && numPixelsInROI > 0 ){
            if( statResults->GetRowData()->GetArray("M3")->GetNumberOfTuples() > 0 ){
                string valueString = this->DoubleToString( statResults->GetValueByName(0,"M3").ToDouble() );
                svkXMLUtils::CreateNestedXMLDataElement( results, "moment3", valueString);
            } else {
                cout << "WARNING: Could not calculate M3 " << endl;
            }
        }
        if( this->GetShouldCompute(COMPUTE_MOMENT_4) && numPixelsInROI > 0 ){
            if( statResults->GetRowData()->GetArray("M4")->GetNumberOfTuples() > 0 ){
                string valueString = this->DoubleToString( statResults->GetValueByName(0,"M4").ToDouble() );
                svkXMLUtils::CreateNestedXMLDataElement( results, "moment4", valueString);
            } else {
                cout << "WARNING: Could not calculate M4 " << endl;
            }
        }
        if( this->GetShouldCompute(COMPUTE_VARIANCE)){
            string valueString = this->DoubleToString( statResults->GetValueByName(0,"Variance").ToDouble() );
            svkXMLUtils::CreateNestedXMLDataElement( results, "variance", valueString);
        }
        if( this->GetShouldCompute(COMPUTE_KURTOSIS) ){
            string valueString = this->DoubleToString( statDerivedResults->GetValueByName(0,"Kurtosis").ToDouble() );
            // TODO: Make this only happen in UCSF Compatibility mode.
            if( numPixelsInROI == 1 ) {
                valueString = "-3.0";
            }
            if( numPixelsInROI == 0 ) {
                valueString = "0.0";
            }

            svkXMLUtils::CreateNestedXMLDataElement( results, "kurtosis", valueString);
        }
        if( this->GetShouldCompute(COMPUTE_SKEWNESS)){
            string valueString = this->DoubleToString( statDerivedResults->GetValueByName(0,"Skewness").ToDouble() );
            svkXMLUtils::CreateNestedXMLDataElement( results, "skewness", valueString);
        }
        if( statModel != NULL) {
            statModel->Delete();
        }

    }
}


/*!
 * Do smooth computation.
 */
void svkImageStatistics::ComputeSmoothStatistics(svkMriImageData* image, svkMriImageData* roi, double binSize, vtkDataArray* maskedPixels, vtkXMLDataElement* results, double normalization)
{
    int numBins = this->GetPortMapper()->GetIntInputPortValue( NUM_BINS )->GetValue();
    int smoothBins = 0;
    if( this->GetPortMapper()->GetIntInputPortValue( SMOOTH_BINS ) != NULL ){;
        smoothBins = this->GetPortMapper()->GetIntInputPortValue( SMOOTH_BINS )->GetValue();
    }
    if( image != NULL ) {
        double startBin = image->GetPointData()->GetScalars()->GetRange()[0];
        if( this->GetPortMapper()->GetDoubleInputPortValue( START_BIN ) != NULL ) {
            startBin = this->GetPortMapper()->GetDoubleInputPortValue( START_BIN )->GetValue();
        }
        bool useMeanForAll = false;
        vtkDataArray* pixelsInROI = svkStatistics::GetMaskedPixels(image,roi);
        vtkDataArray* histogram = svkStatistics::GetHistogram( pixelsInROI, binSize, startBin, numBins, smoothBins );
        double mean = 0;
        if( pixelsInROI != NULL && histogram != NULL) {
            //TODO: Move calculation of sum and volume here?
            int numPixels = 0;
            for( int i= 0; i < pixelsInROI->GetNumberOfTuples(); i++) {
                if(pixelsInROI->GetTuple1(i) != 0 ) {
                    numPixels++;
                }
            }
            if( numPixels <= smoothBins ) {
                useMeanForAll = true;
                int pixelsUsed = 0;
                for( int i = 0; i < pixelsInROI->GetNumberOfTuples(); i++ ) {
                    if( pixelsInROI->GetTuple1(i) != 0) {
                        mean += pixelsInROI->GetTuple1(i);
                        pixelsUsed++;
                    }
                }
                mean /= pixelsUsed;
            }
            // Sort the data
            vtkSortDataArray::Sort( pixelsInROI );

            // Compute the mode if requested
            if( this->GetShouldCompute(COMPUTE_MODE)) {
                double value = svkStatistics::ComputeModeFromHistogram( histogram, binSize, startBin, smoothBins );
                if( useMeanForAll ) {
                    value = mean;
                }
                if( normalization != 0 ) {
                    value /= normalization;
                }
                string valueString = this->DoubleToString( value );
                vtkXMLDataElement* elem = svkXMLUtils::CreateNestedXMLDataElement( results, "mode", valueString);
                if( normalization != 0 ) {
                    elem->SetAttribute( "normalization", this->DoubleToString( normalization ).c_str());
                }
            }

            int numIntervals = 20; // Compute every 5%
            vector<double> quantiles = svkStatistics::ComputeQuantilesFromHistogram(numIntervals, histogram, binSize, startBin, numBins, smoothBins);
            for( int i = 0; i < numIntervals; i++ ) {
                double percentile = quantiles[i];
                if( useMeanForAll ) {
                    percentile = mean;
                }
                if( normalization != 0 ) {
                    percentile /= normalization;
                }
                vtkXMLDataElement* elem = NULL;
                if( this->GetShouldCompute(COMPUTE_QUANTILES)) {
                    if( ((double)i)/numIntervals == 0.1 ) {
                        string valueString = this->DoubleToString( percentile );
                        elem = svkXMLUtils::CreateNestedXMLDataElement( results, "percent10", valueString);
                    } else if( ((double)i)/numIntervals == 0.25 ) {
                        string valueString = this->DoubleToString( percentile );
                        elem = svkXMLUtils::CreateNestedXMLDataElement( results, "percent25", valueString);
                    } else if( ((double)i)/numIntervals == 0.5 ) {
                        string valueString = this->DoubleToString( percentile );
                        elem = svkXMLUtils::CreateNestedXMLDataElement( results, "median", valueString);
                    } else if( ((double)i)/numIntervals == 0.75 ) {
                        string valueString = this->DoubleToString( percentile );
                        elem = svkXMLUtils::CreateNestedXMLDataElement( results, "percent75", valueString);
                    } else if( ((double)i)/numIntervals == 0.90 ) {
                        string valueString = this->DoubleToString( percentile );
                        elem = svkXMLUtils::CreateNestedXMLDataElement( results, "percent90", valueString);
                    }
                } else if( this->GetShouldCompute(COMPUTE_MEDIAN) && ((double)i)/numIntervals == 0.5 ) {
                        string valueString = this->DoubleToString( percentile );
                        elem = svkXMLUtils::CreateNestedXMLDataElement( results, "median", valueString);
                }
                if( elem != NULL && normalization != 0) {
                    elem->SetAttribute( "normalization", this->DoubleToString( normalization ).c_str());
                }
            }
            if( this->GetShouldCompute(COMPUTE_HISTOGRAM)) {
                vtkDataArray* unSmoothedHist = svkStatistics::GetHistogram( pixelsInROI, binSize, startBin, numBins );
                this->AddHistogramTag( unSmoothedHist, binSize, startBin, numBins, 0, results);
                unSmoothedHist->Delete();
            }
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
        histTag->SetAttribute("bins", svkTypeUtils::IntToString(numBins).c_str());
        if( smoothBins > 0 ) {
            histTag->SetAttribute("smoothBins", svkTypeUtils::IntToString(smoothBins).c_str());
        }
        for( int i = 0; i < numBins; i++ ) {
            vtkXMLDataElement* element = vtkXMLDataElement::New();
            element->SetName("bin");
            element->SetAttribute("index", svkTypeUtils::IntToString(i).c_str());
            element->SetAttribute("min", this->DoubleToString(startBin + i*binSize).c_str());
            element->SetAttribute("max", this->DoubleToString(startBin + (i+1)*binSize).c_str());
            string valueString = this->DoubleToString(histogram->GetTuple1(i));
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



/*!
 *  We need a higher default precision here.
 */
string svkImageStatistics::DoubleToString( double value )
{
    return svkTypeUtils::DoubleToString(value,DOUBLE_TO_STRING_PRECISION);
}
