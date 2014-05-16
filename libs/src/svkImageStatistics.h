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
        static void GenerateDefaultConfig( vtkXMLDataElement* config );

        void SetInput( svkMriImageData* image );
        void SetROI( svkMriImageData* roi );
        void SetConfig( vtkXMLDataElement* config );
        void Update( );
        void GetImageAccumulateStats( svkMriImageData* data, svkMriImageData* roi, vtkXMLDataElement* stats );
        void GetXMLResults( vtkXMLDataElement* results );
        void PrintStatistics( );



	protected:

        svkImageStatistics();
       ~svkImageStatistics();

        string              configFlineName;
        vtkXMLDataElement*  config;
        vtkXMLDataElement*  results;
        svkMriImageData*    image;
        svkMriImageData*    roi;
        vtkImageAccumulate* accumulator;
        

};


}   //svk



#endif //SVK_IMAGE_STATISTICS
