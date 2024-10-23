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


#ifndef SVK_IMAGE_TOPOLOGY_GENERATOR_H
#define SVK_IMAGE_TOPOLOGY_GENERATOR_H

#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkObject.h>
#include <svkImageData.h>
//#include <svkMriImageData.h>
//#include <svkMrsImageData.h>
#include </usr/include/vtk/vtkActorCollection.h>
#include </usr/include/vtk/vtkActor.h>
#include </usr/include/vtk/vtkActor2D.h>
#include </usr/include/vtk/vtkProperty.h>
#include </usr/include/vtk/vtkCubeSource.h>
#include </usr/include/vtk/vtkPolyDataMapper.h>
#include </usr/include/vtk/vtkPoints.h>
#include </usr/include/vtk/vtkFloatArray.h>
#include </usr/include/vtk/vtkPlanesIntersection.h>
#include <svkDcmHeader.h>
#include </usr/include/vtk/vtkHexahedron.h>
#include </usr/include/vtk/vtkUnstructuredGrid.h>
#include </usr/include/vtk/vtkRectilinearGrid.h>
#include </usr/include/vtk/vtkDataSetMapper.h>
#include </usr/include/vtk/vtkExtractEdges.h>
#include </usr/include/vtk/vtkRenderer.h>
#include </usr/include/vtk/vtkPolyDataMapper2D.h>
#include </usr/include/vtk/vtkPolyData.h>
#include </usr/include/vtk/vtkAppendPolyData.h>
#include </usr/include/vtk/vtkPolyLine.h>
#include </usr/include/vtk/vtkCellArray.h>
#include </usr/include/vtk/vtkLineSource.h>
#include </usr/include/vtk/vtkFeatureEdges.h>


namespace svk {


using namespace std;
using namespace svk;

class svkImageData;
class svkMrsImageData;

/*!
 *  Class that generates topologies for svkImageData objects. Currently it can generate
 *  two topology types: a grid that represents the cell structure of a data set and a 
 *  hexahdron that represents the selection box.  
 */
class svkImageTopologyGenerator: public vtkObject
{

    public:

        vtkTypeMacro( svkImageTopologyGenerator, vtkObject );

        svkImageTopologyGenerator();
        ~svkImageTopologyGenerator();

        virtual vtkActorCollection* GenerateVoxelGrid( svkImageData* data );
        virtual vtkActorCollection* GenerateSelectionBox ( svkImageData* data );

        virtual void           GenerateVoxelGridActor( svkImageData* data, vtkActor* targetActor );
        virtual vtkPolyData*   GenerateVoxelGridPolyData( svkImageData* data); 
        
        virtual vtkActorCollection* GetTopoActorCollection( svkImageData* data, int actorIndex = 0) = 0;

    protected:

        virtual vtkActor*           MakeGridVoxelActor( double* bounds );
        virtual vtkActor*           MakeRectGridVoxelActor( double* bounds );

};


}

#endif //SVK_IMAGE_TOPOLOGY_GENERATOR

