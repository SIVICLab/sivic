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


#ifndef SVK_IMAGE_DATA_H
#define SVK_IMAGE_DATA_H

#include <vtkObjectFactory.h>
#include <vtkImageData.h>
#include <vtkCell.h>
#include <vtkVertex.h>
#include <vtkLine.h>
#include <vtkPixel.h>
#include <vtkVoxel.h>
#include <vtkGenericCell.h>
#include <vtkMath.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkFloatArray.h>
#include <vtkActorCollection.h>
#include <vtkDataSetAttributes.h>

#include <svkDcmtkAdapter.h>
#include <svkDcmHeader.h>
#include <svkProvenance.h>
#include <svkImageTopologyGenerator.h>

#include <assert.h>
#include <set>


namespace svk {


using namespace std;
using namespace svk;


//  Forward declaration
class svkImageTopologyGenerator;


/*!
 *  Oblique extention to vtkImageData. This class contains a length 9 double
 *  array to contain the directional cosine matrix. Using this methods like
 *  "GetPoint" and "GetCell" can be overridden to cause the oblique nature
 *  of the dataset to be accurately represented. NOTE: Many of the methods 
 *  are not implemented currently, only those that are necessary for the 
 *  filters/actors that we are using to represent image data.
 *  
 *  Note, the svkImageData's DICOM header (svkDcmHeader) contains the following fields that 
 *  define the data orientation:
 *      1. imageOrientationPatient (the row and column vector that define the slice
 *      plane).    
 *      2. imagePositionPatient (the explicit position of each slice (frame))
 *
 *      The "right handed" normal to the plane defined by 1, gives the positive S 
 *      direction (for  an Axial Data Set).  
 *
 *      The data ordering or 3rd row of the DCOS matrix is implicitly given by the
 *      coordinates in consecutive imagePositionPatient fields for each frame. 
 *      
 *      Data ordering is only explicitly encoded in the 3x3 DCOS in the svkImageData
 *      object to indicate the spatial relation of data buffer ordering. 
 */

class svkImageData: public vtkImageData
{

    public:

        vtkTypeRevisionMacro( svkImageData, vtkImageData );
        virtual void       PrintSelf( ostream &os, vtkIndent indent );


        enum RangeComponent {
            REAL = 0,
            IMAGINARY,
            MAGNITUDE
        };

        typedef enum {
            ROW = 0,
            COLUMN,
            SLICE,
            LR,
            PA,
            SI
        } DataBasis;


        // Copy and Cast Methods
        virtual void       DeepCopy( vtkDataObject* src, svkDcmHeader::DcmPixelDataFormat castToFormat = svkDcmHeader::UNDEFINED );
        virtual void       ShallowCopy( vtkDataObject* src );
        virtual void       ZeroCopy( vtkImageData* src, svkDcmHeader::DcmPixelDataFormat castToFormat = svkDcmHeader::UNDEFINED );
        virtual void       CastDataFormat( svkDcmHeader::DcmPixelDataFormat castToFormat );
        virtual void       CopyAndCastFrom( vtkImageData* inData, int extent[6] );
        virtual void       CopyAndCastFrom( vtkImageData* inData, int x0, int x1, int y0, int y1, int z0, int z1 );
        virtual void       CopyStructure( vtkDataSet* ds );
        virtual void       CopyDcos( vtkDataObject* src );
        virtual void       CopyVtkImage( vtkImageData* sourceImage, double dcos[][3] );

        // Dcos sensitive methods:
        virtual double *   GetPoint (vtkIdType ptId);

        //  Inline for performance: 
        virtual void       GetPoint (vtkIdType id, double x[3])
        {
            const double *p = this->GetPoint(id);
            x[0] = p[0]; x[1] = p[1]; x[2] = p[2];
        }

        virtual vtkCell *  GetCell (vtkIdType cellId);
        virtual void       GetCell (vtkIdType cellId, vtkGenericCell *cell);
        virtual void       GetCellBounds (vtkIdType cellId, double bounds[6]);
        virtual vtkIdType  FindPoint (double x, double y, double z);
        virtual vtkIdType  FindPoint (double x[3]);
        virtual vtkIdType  FindCell (
                                double x[3], 
                                vtkCell *cell, 
                                vtkIdType cellId, 
                                double tol2, 
                                int &subId, 
                                double pcoords[3], 
                                double *weights
                           );
        virtual vtkIdType  FindCell (
                                double x[3], 
                                vtkCell *cell, 
                                vtkGenericCell *gencell, 
                                vtkIdType cellId, 
                                double tol2, 
                                int &subId, 
                                double pcoords[3], 
                                double *weights
                           );
        virtual vtkCell *  FindAndGetCell (
                                double x[3], 
                                vtkCell *cell, 
                                vtkIdType cellId, 
                                double tol2, 
                                int &subId, 
                                double pcoords[3], 
                                double *weights
                           );
        virtual int        GetCellType (vtkIdType cellId);
        virtual void       GetCellPoints (vtkIdType cellId, vtkIdList *ptIds);
        virtual void       GetPointCells (vtkIdType ptId, vtkIdList *cellIds);
        virtual void       ComputeBounds ();
        virtual int        ComputeStructuredCoordinates (double x[3], int ijk[3], double pcoords[3]);
        virtual void       GetVoxelGradient (int i, int j, int k, vtkDataArray *s, vtkDataArray *g);
        virtual void       GetPointGradient (int i, int j, int k, vtkDataArray *s, double g[3]);
        virtual void       SetAxisUpdateExtent (int axis, int min, int max);
        virtual void       GetAxisUpdateExtent (int axis, int &min, int &max);

        virtual void       SetDcos( double x[3][3]);
        virtual void       GetDcos( double x[3][3]);

        void               SetDcmHeader(svkDcmHeader* dcmHeader);
        svkDcmHeader*      GetDcmHeader();
        void               SetProvenance(svkProvenance* provenance);
        svkProvenance*     GetProvenance();
        int                GetIDFromIndex(int indexX, int indexY, int indexZ);
        virtual void       GetNumberOfVoxels(int numVoxels[3]) = 0;
        void               GetIndexFromID(int voxelID, int* indexX, int* indexY, int* indexZ);
        void               GetIndexFromID(int voxelID, int* index);
        void               GetIndexFromPosition(double posLPS[3], int* index);
        virtual void       GetPositionFromIndex(int* index, float* posLPS);
        virtual void       GetSliceOrigin(int slice, float* sliceOrigin, svkDcmHeader::Orientation sliceOrientation = svkDcmHeader::UNKNOWN );
        virtual void       GetSliceNormal(float* normal, svkDcmHeader::Orientation sliceOrientation = svkDcmHeader::UNKNOWN );
        virtual int        GetClosestSlice(float* posLPS, svkDcmHeader::Orientation sliceOrientation = svkDcmHeader::UNKNOWN );
        virtual int        GetNumberOfSlices( svkDcmHeader::Orientation sliceOrientation = svkDcmHeader::UNKNOWN );
        virtual int        GetFirstSlice( svkDcmHeader::Orientation sliceOrientation = svkDcmHeader::UNKNOWN );
        virtual int        GetLastSlice( svkDcmHeader::Orientation sliceOrientation = svkDcmHeader::UNKNOWN );
        virtual void       GetDataBasis( double basisVector[3], DataBasis basis );
        virtual int        GetOrientationIndex( svkDcmHeader::Orientation orientation );
        virtual void       GetImageCenter( float* center );
        virtual double     GetSliceSpacing( svkDcmHeader::Orientation sliceOrientation = svkDcmHeader::UNKNOWN );
        void               GetDataRange( double range[2], int component );
        void               SetDataRange( double range[2], int component );
        int                GetNumberOfTimePoints();

    // REQUIRED?
    virtual vtkActorCollection*   GetTopoActorCollection(int actorIndex);

        
    protected:

        svkImageData();
        ~svkImageData();

        //  Members:
        svkDcmHeader*                   dcmHeader;
        svkProvenance*                  provenance;
        // REQUIRED?
        svkImageTopologyGenerator*      topoGenerator;

        //! Range needs to be able to accomodate real, imaginary, and magnitude components
        double                          range[3][2];
        bool                            WasModified();
        virtual void                    UpdateSvkParams();
        virtual void                    UpdateRange();

    
    private:
        // dcos = Directional Cosine Matrix
        double             dcos[3][3];
        unsigned long      lastUpdateTime;

        void CastDataArrays( int vtkDataArrayType, vtkDataSetAttributes* fieldData );


};


}   //svk

#endif //SVK_IMAGE_DATA_H
