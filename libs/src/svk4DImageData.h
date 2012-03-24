/*
 *  Copyright © 2009-2012 The Regents of the University of California.
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


#ifndef SVK_4D_IMAGE_DATA_H
#define SVK_4D_IMAGE_DATA_H


#include <vtkObjectFactory.h>
#include <vtkImageData.h>
#include <vtkDoubleArray.h>

#include <svkImageData.h>
#include <vector>

#define DEFAULT_SELECTION_TOLERANCE 0.50001

namespace svk {


/*!
 *  This class represents multi-volumentric 4D imaging data. Data is stored in its
 *  vtkCellData object and we will have one vtkDataArray per voxel/ per volume.
 *  Data can be accessed with general GetArray methods that take as input the number
 *  of volumetric indices, an array the index values, and an array of dimension sizes.
 *  From this a linear index is determined (based on the order of these afore mentioned
 *  arrays ) and the data can be accessed. As such it is extremely important not to
 *  change the order of the arrays as it implies the cell association.
 */
class svk4DImageData: public svkImageData 
{

    public:

        vtkTypeRevisionMacro( svk4DImageData, svkImageData);
        static svk4DImageData* New();
        static vtkObject*       NewObject();

        enum ActorType {PLOT_GRID, VOL_SELECTION, SAT_BANDS};

        virtual void   GetNumberOfVoxels(int numVoxels[3]);
        virtual int    GetIDFromIndex(int indexX, int indexY, int indexZ, int* indexArray = NULL);

        vtkDataArray*  GetArray( int linearIndex );
        vtkDataArray*  GetArrayFromID( int index, int* indexArray = NULL );
        vtkDataArray*  GetArray(int x, int y, int z, int* indexArray = NULL );
        vtkDataArray*  GetArray( int* indexArray );

        virtual int    GetLastSlice( svkDcmHeader::Orientation sliceOrientation = svkDcmHeader::UNKNOWN_ORIENTATION );
        virtual int    GetClosestSlice(double* posLPS, svkDcmHeader::Orientation sliceOrientation );


        void           GetTlcBrcInUserSelection( 
                            int tlcBrc[2], 
                            double userSelection[6], 
                            svkDcmHeader::Orientation orientation = svkDcmHeader::UNKNOWN_ORIENTATION, 
                            int slice = -1 
                       );

		void  		   GetZeroImage(  svkImageData* image );
        void           GetImage( svkImageData* image,
                                 int point,
                                 int* indexArray = NULL,
                                 int component = 2 );

        void           GetImage( svkImageData* image,
                                 int point,
                                 vtkstd::string seriesDescription,
                                 int* indexArray = NULL,
                                 int component = 2 );

        void           SetImage( vtkImageData* image, int point,  int* indexArray = NULL );
        virtual void   UpdateRange(int component );
        int            GetNumberOfSlices( svkDcmHeader::Orientation sliceOrientation);
        virtual void   EstimateDataRange( double range[2], int minPt, int maxPt, int component, int* tlcBrc = NULL, int* indexArray = NULL );
        //void           InitializeDataArrays();
        virtual int    GetVolumeIndexSize( int volumeIndex );
        virtual int    GetNumberOfVolumeDimensions( );
        virtual void   GetPolyDataGrid( vtkPolyData* grid );


    protected:

        svk4DImageData();
        ~svk4DImageData();

    private:



};


}   //svk


#endif //SVK_4D_IMAGE_DATA_H

