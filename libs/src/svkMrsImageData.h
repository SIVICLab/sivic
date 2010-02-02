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


#ifndef SVK_MRS_IMAGE_DATA_H
#define SVK_MRS_IMAGE_DATA_H


#include <vtkObjectFactory.h>
#include <svkImageData.h>
#include <svkMrsTopoGenerator.h>
#include <vtkImageData.h>


namespace svk {


using namespace std;


/*!
 *  Class that represents a MR Spectroscopy data set. Contains methods 
 *  for accessing MRS specific
 *  actors: plotGrid, volumeSelection, and satBands.
 */


class svkMrsImageData: public svkImageData 
{

    public:

        vtkTypeRevisionMacro( svkMrsImageData, svkImageData);
        static svkMrsImageData* New();
        static vtkObject*       NewObject();

        enum ActorType {PLOT_GRID, VOL_SELECTION, SAT_BANDS};

        virtual void   GetNumberOfVoxels(int numVoxels[3]);

        void           GenerateSelectionBox( vtkUnstructuredGrid* selectionBox );
        void           GetSelectionBoxCenter( float* selBoxCenter );
        void           GetSelectionBoxDimensions( float* dims );
        vtkDataArray*  GetSpectrum( int i, int j, int k, int timePoint = 0, int channel=0 );
        vtkDataArray*  GetSpectrumFromID( int index, int timePoint = 0, int channel = 0 );
        bool           SliceInSelectionBox( int slice, svkDcmHeader::Orientation orientation = svkDcmHeader::UNKNOWN );
        virtual int    GetLastSlice( svkDcmHeader::Orientation sliceOrientation = svkDcmHeader::UNKNOWN );
        void           GetSelectionBoxSpacing( double spacing[3] );
        void           GetSelectionBoxOrigin(  double origin[3] );




    protected:

        svkMrsImageData();
        ~svkMrsImageData();
    
        virtual void      UpdateRange();
};


}   //svk


#endif //SVK_MRS_IMAGE_DATA_H

