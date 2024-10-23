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


#ifndef SVK_IMAGE_STATISTICS_H
#define SVK_IMAGE_STATISTICS_H
#define DOUBLE_TO_STRING_PRECISION 14


#include <stdio.h>
#include <map>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkXMLDataElement.h>
#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkMriImageData.h>
#include </usr/include/vtk/vtkOrderStatistics.h>
#include <svkImageAlgorithmWithPortMapper.h>
#include <svkImageAlgorithmPipeline.h>
#include <svkIdfVolumeWriter.h>
#include <svkGenericAlgorithmWithPortMapper.h>
#include </usr/include/vtk/vtkImageToImageStencil.h>
#include </usr/include/vtk/vtkXMLUtilities.h>
#include <svkXML.h>
#include </usr/include/vtk/vtkArrayToTable.h>
#include </usr/include/vtk/vtkTable.h>
#include </usr/include/vtk/vtkDescriptiveStatistics.h>
#include <svkDataValidator.h>
#include </usr/include/vtk/vtkSortDataArray.h>
#include </usr/include/vtk/vtkVariant.h>
#include <svkXMLUtils.h>

namespace svk {


using namespace std;

/*!
 *  The purpose of this class is to take in an XML element that defines a set of ROI's, a set
 *  of images, filters to apply to the ROI's/images, and a set of statistics to be
 *  computed. Then statistics for every combination will be computed using svkImageStatistics
 *  and an XML data element will be output containing the results of the computation.
 *
 *  TODO: Should this be converted into an svkImageAlgorithmWithPortMapper?
 */
class svkImageStatistics : public svkGenericAlgorithmWithPortMapper
{

    public:

        typedef enum {
            INPUT_IMAGE = 0,
            INPUT_ROI,
            NUM_BINS,
            BIN_SIZE,
            AUTO_ADJUST_BIN_SIZE,
            IGNORE_NEGATIVE_NUMBERS,
            START_BIN,
            SMOOTH_BINS,
            COMPUTE_HISTOGRAM,
            COMPUTE_MEAN,
            COMPUTE_MAX,
            COMPUTE_MIN,
            COMPUTE_STDEV,
            COMPUTE_VOLUME,
            COMPUTE_MODE,
            COMPUTE_MEDIAN,
            COMPUTE_QUANTILES,
            COMPUTE_SUM,
            COMPUTE_MOMENT_2,
            COMPUTE_MOMENT_3,
            COMPUTE_MOMENT_4,
            COMPUTE_VARIANCE,
            COMPUTE_KURTOSIS,
            COMPUTE_SKEWNESS,
            NORMALIZATION_METHOD,
            NORMALIZATION_ROI_INDEX,
            NORMALIZATION_IMAGE_INDEX,
            OUTPUT_FILE_NAME
        } svkImageStatisticsParameter;

        typedef enum {
            NONE = 0,
            MODE,
            MEAN
        } svkNormalizationOptions;

        // vtk type revision macro
        vtkTypeMacro( svkImageStatistics, svkGenericAlgorithmWithPortMapper );
  
        // vtk initialization 
        static svkImageStatistics* New();

        //! This will grab the output object as the correct data type to avoid casting
        vtkXMLDataElement* GetOutput( );

	protected:

        svkImageStatistics();
       ~svkImageStatistics();

       //! Does the computation of the statistics.
       virtual int RequestData(
                      vtkInformation* request,
                      vtkInformationVector** inputVector,
                      vtkInformationVector* outputVector );

       void ComputeAccumulateStatistics(svkMriImageData* image, svkMriImageData* roi, double binSize, vtkDataArray* maskedPixels, vtkXMLDataElement* result, double normalization = 0 );

       void ComputeDescriptiveStatistics(svkMriImageData* image, svkMriImageData* roi, vtkDataArray* maskedPixels, vtkXMLDataElement* result );
       void ComputeSmoothStatistics(svkMriImageData* image, svkMriImageData* roi, double binSize, vtkDataArray* maskedPixel, vtkXMLDataElement* results, double normalization = 0 );
       void AddHistogramTag( vtkDataArray* histogram, double binSize, double startBin, int numBins, int smoothBins, vtkXMLDataElement* results);

	private:

       bool   GetShouldCompute( svkImageStatistics::svkImageStatisticsParameter parameter);
       string DoubleToString( double value );

};


}   //svk

#endif //SVK_IMAGE_STATISTICS_H
