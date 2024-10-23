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


#ifndef SVK_SAT_BAND_SET_H
#define SVK_SAT_BAND_SET_H


#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include <vector>
#include </usr/include/vtk/vtkPlane.h>
#include </usr/include/vtk/vtkActor.h>
#include </usr/include/vtk/vtkPolyDataMapper.h>
#include </usr/include/vtk/vtkPlaneSource.h>
#include </usr/include/vtk/vtkProperty.h>
#include </usr/include/vtk/vtkClipDataSet.h>
#include </usr/include/vtk/vtkClipPolyData.h>
#include <svkMrsImageData.h>
#include <svkMriImageData.h>
#include <svkImageTopologyGenerator.h>


namespace svk {

//Is used by topo generator which uses this class so we nedd a forward dec.
class svkMrsImageData;

/*! 
 *
 */
class svkSatBandSet : public vtkObject
{

    public:

        // vtk type revision macro
        vtkTypeMacro( svkSatBandSet,vtkObject );
   
        static svkSatBandSet*       New();  
        svkSatBandSet();
        ~svkSatBandSet();


        vtkActor* GetSatBandsActor( );
        vtkActor* GetSatBandsOutlineActor( );
        void SetClipSlice( int slice );
        void SetOrientation( svkDcmHeader::Orientation orientation );
        void SetInput( svkMrsImageData* spectra );
        void SetReferenceImage( svkMriImageData* image );
        void RemoveReferenceImage( );
        void SetClipToReferenceImage( bool clipToReferenceImage );

    private:

        // methods...
        void UpdateClippingParameters();
        void GenerateSatBandsActor();
        void GenerateSliceClippingPlanes( );
        vtkActor* satBandActor;
        vtkActor* satBandOutlineActor;
        void GenerateClippingPlanes();
        void ApplyClippingPlanes( );

        // members...

        svkMrsImageData* spectra;
        svkMriImageData* image;
        int slice;
        bool clipToReferenceImage;
        svkDcmHeader::Orientation orientation; 
        vector<vtkPlane*> clippingPlanes;
        vtkPoints*  satBandSurfaceOrigins;
        vtkPoints*  satBandOrigins;
        vtkFloatArray*  satBandSurfaceNormals;
        vtkFloatArray*  satBandNormals;

        // These are stored as memeber variables to speed up slicing
        double* spacing;
        double dcos[3][3];
        double* origin;
        int* extent;
        double deltaLR; // spacing project into x/y/z
        double deltaAP;
        double deltaSI;

        static const double CLIP_TOLERANCE;
        static const double IMAGE_CLIP_TOLERANCE;
        static const int    PROJECTION_MULTIPLIER;


};


}   //svk


#endif //SVK_SAT_BAND_SET_H
