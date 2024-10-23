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


#ifndef SVK_MRI_IMAGE_DATA_H
#define SVK_MRI_IMAGE_DATA_H

#define START_NUM_BINS 256
#define MAX_NUM_BINS 1048576
#define MIN_USED_BINS 10
#define EXCLUDE_FACTOR 0.05 

#include </usr/include/vtk/vtkObjectFactory.h>
#include <svkImageData.h>
#include <svk4DImageData.h>
#include </usr/include/vtk/vtkImageAccumulate.h>
#include </usr/include/vtk/vtkCallbackCommand.h>
#include </usr/include/vtk/vtkPolyData.h>
#include <svkFastCellData.h>


namespace svk {


using namespace std;


/*!
 *  Class that represents a MR Image data set. Contains methods for accessing MRI specific
 *  actors. 
 */


class svkMriImageData: public svkImageData 
{

    public:

        vtkTypeMacro( svkMriImageData, svkImageData);
        static svkMriImageData* New();
        static vtkObject*       NewObject();


        double*                 GetImagePixels( int slice );
        vtkDataArray*           GetImagePixelsArray( int slice );
        double*                 GetImagePixel( int id );
        double*                 GetImagePixel( int x, int y, int z );
        void                    SetImagePixels( double* pixels, int slice);
        void                    SetImagePixel( int x, int y, int z, double value );
        void                    SetImagePixel( int id, double value );
        void                    SetImagePixelTuple( int x, int y, int z, double* value ); 
        void                    SetImagePixelTuple( int id , double* value ); 
        virtual void            GetNumberOfVoxels(int numVoxels[3]);
        void                    GetAutoWindowLevel( double& window, double& level, int numBins = START_NUM_BINS
                                                                         , double excludeFactor = EXCLUDE_FACTOR );
        virtual int             GetVolumeIndexForFrame(int frame );
        void                    GetCenterOfMass( double centerOfMass[3], int component = 0 );
        virtual svk4DImageData* GetCellDataRepresentation();
        void                    SyncPixelDataToCellRepresentation();
        void                    SyncCellRepresentationToPixelData();


        static void             UpdatePixelData(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);


    protected:

        svkMriImageData();
        ~svkMriImageData();

        enum ActorType {PLOT_GRID};
    
    private:
        
        svk4DImageData*         cellDataRepresentation;
        double*                 pixelBuffer;
        vtkCallbackCommand*     cellRepresentationModifiedCB;
        void                    InitializeCellDataArrays();
};


}   //svk


#endif //SVK_MRI_IMAGE_DATA_H

