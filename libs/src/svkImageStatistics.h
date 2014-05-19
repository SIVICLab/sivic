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


#ifndef SVK_IMAGE_STATISTICS_H
#define SVK_IMAGE_STATISTICS_H


#include <string>
#include <map>
#include <vector>
#include <stdio.h>
#include <sstream>
#include <vtkObjectFactory.h>
#include <vtkObject.h>
#include <vtkImageAccumulate.h>
#include <svkMriImageData.h>
#include <svkUtils.h>
#include <vtkImageToImageStencil.h>
#include <time.h>

namespace svk {


using namespace std;

class svkImageStatistics : public vtkObject
{

    public:

        // vtk type revision macro
        vtkTypeRevisionMacro( svkImageStatistics, vtkObject );
  
        // vtk initialization 
        static svkImageStatistics* New();  

        //! The input is the image upon which to calculate the statistics
        void SetInput( svkMriImageData* image );

        //! The roi is the region within which to calculate the statistics. Any value >= 1 is considered in the roi.
        void SetROI( svkMriImageData* roi );

        //! Sets the XML configuration that determines which statistics are to be computed and any parameters necessary.
        void SetConfig( vtkXMLDataElement* config );

        //! Updates the algrothim, computes the statistics.
        void Update( );

        //! Gets the results as xml. This method executes a DeepCopy on the provided input.
        void GetXMLResults( vtkXMLDataElement* results );

        //! Prints the statistics in XML format.
        void PrintStatistics( );



	protected:

        svkImageStatistics();
       ~svkImageStatistics();

        void Execute( );

        vtkXMLDataElement*  config;
        vtkXMLDataElement*  results;
        svkMriImageData*    image;
        svkMriImageData*    roi;
        vtkImageAccumulate* accumulator;

        

};


}   //svk



#endif //SVK_IMAGE_STATISTICS
