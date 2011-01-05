/*
 *  Copyright © 2009-2011 The Regents of the University of California.
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
#include <vtkImageData.h>
#include <vtkDoubleArray.h>

#include <svkImageData.h>
#include <svkMriImageData.h>
#include <svkMrsTopoGenerator.h>


namespace svk {


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
        void           GetSelectionBoxCenter( double* selBoxCenter );
        void           GetSelectionBoxDimensions( float* dims );
        vtkDataArray*  GetSpectrum( int i, int j, int k, int timePoint = 0, int channel=0 );
        vtkDataArray*  GetSpectrumFromID( int index, int timePoint = 0, int channel = 0 );
        vtkDataArray*  GetSpectrum( int linearIndex ); 
        bool           IsSliceInSelectionBox( int slice, svkDcmHeader::Orientation orientation = svkDcmHeader::UNKNOWN_ORIENTATION );
        virtual int    GetLastSlice( svkDcmHeader::Orientation sliceOrientation = svkDcmHeader::UNKNOWN_ORIENTATION );
        void           GetSelectionBoxSpacing( double spacing[3] );
        void           GetSelectionBoxOrigin(  double origin[3] );
        int            GetClosestSlice(double* posLPS, svkDcmHeader::Orientation sliceOrientation );
        int            GetNumberOfChannels();
        void           GetTlcBrcInSelectionBox( int tlcBrc[2], svkDcmHeader::Orientation orientation = svkDcmHeader::UNKNOWN_ORIENTATION, int slice = -1 );
        void           GetSelectionBoxMaxMin( double minPoint[3], double maxPoint[3], double tolerance = 0.5 );
        void           GetTlcBrcInUserSelection( 
                            int tlcBrc[2], 
                            double userSelection[6], 
                            svkDcmHeader::Orientation orientation = svkDcmHeader::UNKNOWN_ORIENTATION, 
                            int slice = -1 
                       );
        void           GetImage( 
                            svkMriImageData* image, 
                            int point, 
                            int timePoint, 
                            int channel, 
                            int component, 
                            vtkstd::string seriesDescription 
                       ); 

        void           GetImage( 
                            svkMriImageData* image, 
                            int point, 
                            int timePoint = 0, 
                            int channel = 0, 
                            int component = 2 
                       ); 
        void           SetImage( vtkImageData* image, int point, int timePoint = 0, int channel = 0 ); 
        virtual void   UpdateRange(int component );
        int            GetNumberOfSlices( svkDcmHeader::Orientation sliceOrientation);
        void           EstimateDataRange( double range[2], int minPt, int maxPt, int component, int timePoint = 0
                                                                                              , int channel = 0   );




    protected:

        svkMrsImageData();
        ~svkMrsImageData();

    private:
        int numChannels;
    
};


}   //svk


#endif //SVK_MRS_IMAGE_DATA_H

