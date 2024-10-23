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


#ifndef SVK_IMAGE_ALGORITHM_PIPELINE_H
#define SVK_IMAGE_ALGORITHM_PIPELINE_H


#include <stdio.h>
#include <map>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkXMLDataElement.h>
#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkMriImageData.h>
#include <svkDCEQuantify.h>
#include <svkImageThreshold.h>
#include <svkImageAlgorithmWithPortMapper.h>
#include <svkImageStatistics.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include </usr/include/vtk/vtkAlgorithmOutput.h>
#include <svkImageMathematics.h>
#include <svkImageCopy.h>

namespace svk {


using namespace std;

/*!
 *  The purpose of this class is to take in an XML element that defines a set of ROI's, a set
 *  of images/maps, filters to apply to the ROI's/images/maps, and a set of statistics to be
 *  computed. Then statistics for every combination will be computed using svkImageStatistics
 *  and an XML data element will be output containing the results of the computation.
 */
class svkImageAlgorithmPipeline : public svkImageAlgorithmWithPortMapper
{

    public:

        typedef enum {
            PIPELINE = 0
        } svkXMLImageAlgorithmParameters;

        // vtk type revision macro
        vtkTypeMacro( svkImageAlgorithmPipeline, svkImageAlgorithmWithPortMapper );
  
        // vtk initialization 
        static svkImageAlgorithmPipeline* New();

        vtkAlgorithmOutput*  GetOutputByUniquePortID(string uniquePortID);

	protected:

        svkImageAlgorithmPipeline();
       ~svkImageAlgorithmPipeline();

       //! VTK RequestData method. Pipeline is executed here
       virtual int RequestData(
                       vtkInformation* request,
                       vtkInformationVector** inputVector,
                       vtkInformationVector* outputVector );

       //! This is a factory method for getting an algorithm.
       void InitializeAlgorithmForTag( vtkXMLDataElement* tag );

	private:

       void SetInputConnections( vtkXMLDataElement* pipeline );
       void ExecutePipeline( vtkXMLDataElement* pipeline );

       map<string, vtkAlgorithmOutput*> idToPortMap;
       map<vtkXMLDataElement*, vtkAlgorithm*> xmlToAlgoMap;

};


}   //svk

#endif //SVK_IMAGE_ALGORITHM_PIPELINE_H
