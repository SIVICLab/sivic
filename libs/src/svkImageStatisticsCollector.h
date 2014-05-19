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


#ifndef SVK_IMAGE_STATISTICS_COLLECTOR_H
#define SVK_IMAGE_STATISTICS_COLLECTOR_H


#include <stdio.h>
#include <map>
#include <vtkObjectFactory.h>
#include <vtkObject.h>
#include <vtkXMLDataElement.h>
#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkMriImageData.h>
#include <svkImageStatistics.h>
#include <svkImageThreshold.h>
#include <svkImageAlgorithmWithParameterMapping.h>
#include <svkIdfVolumeWriter.h>

namespace svk {


using namespace std;

/*!
 *  The purpose of this class is to take in an XML element that defines a set of ROI's, a set
 *  of images/maps, filters to apply to the ROI's/images/maps, and a set of statistics to be
 *  computed. Then statistics for every combination will be computed using svkImageStatistics
 *  and an XML data element will be output containing the results of the computation.
 */
class svkImageStatisticsCollector : public vtkObject
{

    public:

        // vtk type revision macro
        vtkTypeRevisionMacro( svkImageStatisticsCollector, vtkObject );
  
        // vtk initialization 
        static svkImageStatisticsCollector* New();  

        //! Use this method to set the XML configuration.
        void SetXMLConfiguration( vtkXMLDataElement* config );

        //! Use this method to compute the results and get them in the form of an XML element.
        void GetXMLResults( vtkXMLDataElement* results );

        vtkSetStringMacro( RootName );
        vtkGetStringMacro( RootName );

	protected:

        svkImageStatisticsCollector();
       ~svkImageStatisticsCollector();

       //! This method reads in an image from the input XML
       svkImageData* ReadImageFromXML( vtkXMLDataElement* imageElement );

       //! This method loads all Maps and rois applying any necessary filters using  svkImageStatisticsCollector::ApplyFiltersFromXML
       svkImageData* LoadMapsAndROIS( );

       //! Reads the filters for an ROI or a map, instantiates and runs the algroithm, then returns the output.
       svkImageData* ApplyFiltersFromXML( svkImageData* inputImage, vtkXMLDataElement* imageElement );

       //! Takes a filter name and instantiates an svkImageAlgorithm
       svkImageAlgorithmWithParameterMapping* GetAlgorithmForFilterName( string filterName );

       vtkXMLDataElement*                     config;
       char*                                  RootName;
       svkImageReader2*                       reader;
       map<string, svkMriImageData*>          rois;
       map<string, svkMriImageData*>          maps;

       // This just hold temporary pointers to help manage memory release.
       svkImageAlgorithmWithParameterMapping* lastFilter;

};


}   //svk

#endif //SVK_IMAGE_STATISTICS_COLLECTOR
