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


#ifndef SVK_OVERLAY_CONTOUR_DIRECTOR_H
#define SVK_OVERLAY_CONTOUR_DIRECTOR_H


#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkDataSet.h>
#include <svkMriImageData.h>
#include </usr/include/vtk/vtkExtractVOI.h>
#include </usr/include/vtk/vtkProperty.h>
#include </usr/include/vtk/vtkPolyDataMapper.h>
#include </usr/include/vtk/vtkTransform.h>

#include <svkDcmHeader.h>
#include </usr/include/vtk/vtkContourFilter.h>
#include </usr/include/vtk/vtkLegendBoxActor.h>
#include </usr/include/vtk/vtkTubeFilter.h>
#include </usr/include/vtk/vtkActor.h>
#include <vector>


namespace svk {


using namespace std;
using namespace svk;



/*!
 *  The purpose if this class is to provide a simplified interface to
 *  the svkXYPlotActor that will work with simple vtkDataArray objects
 *  instead of requiring full fledged vtkDataObjects.
 */
class svkOverlayContourDirector : public vtkObject
{

    public:

        typedef enum {
            GREEN = 0,
            RED,
            BLUE,
            PINK,
            YELLOW,
            CYAN,
            ORANGE,
            GRAY,
            LAST_COLOR = GRAY
        } ContourColor ;

        // vtk type revision macro
        vtkTypeMacro( svkOverlayContourDirector, vtkObject );

        static svkOverlayContourDirector*    New();

        vtkActor*                            AddInput(svkMriImageData* image);
        void                                 SetSlice(int referenceSlice, svkDcmHeader::Orientation orientation);
        void                                 SetReferenceImage(svkMriImageData* referenceImage );

        //  Methods:
        void                                 SetContourColor(int overlayIndex, ContourColor color);


    protected:

        svkOverlayContourDirector();
        ~svkOverlayContourDirector();



    private:

        svkMriImageData*                 referenceImage;
        vector<vtkContourFilter*>        contourFilters;
        vector<vtkActor*>                contourActors;
        std::vector<svkMriImageData*> dataVector;
        int                              FindOverlayContourSlice( int referenceSlice, svkDcmHeader::Orientation orientation, int overlayIndex );
        vtkTransform*                    GetContourTransform(svkImageData* contourData, int contourSlice,
                                                            int referenceSlice,
                                                            svkDcmHeader::Orientation orientation);


};


}   //svk


#endif //SVK_OVERLAY_CONTOUR_DIRECTOR_H

