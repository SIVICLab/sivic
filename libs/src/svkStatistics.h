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


#ifndef SVK_STATISTICS_H
#define SVK_STATISTICS_H


#include <stdio.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkDataArray.h>
#include </usr/include/vtk/vtkFloatArray.h>
#include <svkUtils.h>

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
class svkStatistics : public vtkObject
{

    public:

        // vtk type revision macro
        vtkTypeMacro( svkStatistics, vtkObject );
  
        // vtk initialization 
        static svkStatistics* New();

        static vtkDataArray* GetMaskedPixels( svkMriImageData* image, svkMriImageData* roi, double minIncluded = 1, double maxIncluded = VTK_DOUBLE_MAX);
        static vtkFloatArray* GetHistogram( vtkDataArray* data, double binSize, double startBin, int numBins,  int smoothBins = 0);
        static double ComputeModeFromHistogram(vtkDataArray* histogram, double binSize, double startBin, int smoothBins);
        static vector<double> ComputeQuantilesFromHistogram(int numQuantiles, vtkDataArray* histogram, double binSize, double startBin, int numBins, int smoothBins);
        static double GetAutoAdjustedBinSize( svkMriImageData* image, double startBinSize, double startBin, int numBins );



	protected:

        svkStatistics();
       ~svkStatistics();

	private:
        static inline int GetBinForValue( double value, double binSize, double startBin);

};


}   //svk

#endif //SVK_STATISTICS_H
