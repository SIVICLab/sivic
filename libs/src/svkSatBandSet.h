/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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


#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkObjectFactory.h>
#include <vector>
#include <vtkPlane.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkPlaneSource.h>
#include <vtkProperty.h>
#include <vtkClipDataSet.h>
#include <vtkClipPolyData.h>
#include <svkMrsImageData.h>
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
        vtkTypeRevisionMacro( svkSatBandSet,vtkObject );
   
        static svkSatBandSet*       New();  
        svkSatBandSet();
        ~svkSatBandSet();


        vtkActor* GetSatBandsActor( );
        vtkActor* GetSatBandsOutlineActor( );
        void SetClipSlice( int slice );
        void SetOrientation( int orientation );
        void SetInput( svkMrsImageData* spectra );

        //! Enum represents orientation of the sat bands 
        enum orientation {
            XY = 0,
            YZ,
            XZ 
        };

    
    private:


        // methods...
        void GenerateSatBandsActor();
        void GenerateSliceClippingPlanes( );
        vtkActor* satBandActor;
        vtkActor* satBandOutlineActor;
        void GenerateClippingPlanes();
        void ApplyClippingPlanes( );

        // members...

        svkMrsImageData* spectra;
        int slice;
        int orientation; 
        vector<vtkPlane*> clippingPlanes;
        vtkPoints*  satBandSurfaceOrigins;
        vtkPoints*  satBandOrigins;
        vtkFloatArray*  satBandSurfaceNormals;
        vtkFloatArray*  satBandNormals;

        // These are stored as memeber variables to speed up slicing
        double* spectraSpacing;
        double spectraDcos[3][3];
        double* spectraOrigin;
        int* spectraExtent;
        double spectraDeltaX; // spacing project into x/y/z
        double spectraDeltaY;
        double spectraDeltaZ;

};


}   //svk


#endif //SVK_SAT_BAND_SET_H
