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
 *
 *  NOTE: This class is deprecated and no longer in use.
 */

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: svkAreaPicker.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/


#include "svkAreaPicker.h"
#include "/usr/include/vtk/vtkObjectFactory.h"
#include "/usr/include/vtk/vtkMapper.h"
#include "/usr/include/vtk/vtkAbstractVolumeMapper.h"
#include "/usr/include/vtk/vtkAbstractMapper3D.h"
#include "/usr/include/vtk/vtkProp.h"
#include "/usr/include/vtk/vtkLODProp3D.h"
#include "/usr/include/vtk/vtkActor.h"
#include "/usr/include/vtk/vtkPropCollection.h"
#include "/usr/include/vtk/vtkImageActor.h"
#include "/usr/include/vtk/vtkProp3DCollection.h"
#include "/usr/include/vtk/vtkAssemblyPath.h"
#include "/usr/include/vtk/vtkImageData.h"
#include "/usr/include/vtk/vtkVolume.h"
#include "/usr/include/vtk/vtkRenderer.h"
#include "/usr/include/vtk/vtkProperty.h"
#include "/usr/include/vtk/vtkCommand.h"
#include "/usr/include/vtk/vtkPlanes.h"
#include "/usr/include/vtk/vtkPlane.h"
#include "/usr/include/vtk/vtkPoints.h"
#include "/usr/include/vtk/vtkExtractSelectedFrustum.h"



//vtkCxxRevisionMacro(svkAreaPicker, "$Revision$");
vtkStandardNewMacro(svkAreaPicker);

//--------------------------------------------------------------------------
svkAreaPicker::svkAreaPicker()
{
  this->FrustumExtractor = vtkExtractSelectedFrustum::New();
  this->Frustum = this->FrustumExtractor->GetFrustum();
  this->Frustum->Register(this);

  this->ClipPoints = this->FrustumExtractor->GetClipPoints();
  this->ClipPoints->Register(this);

  this->Prop3Ds = vtkProp3DCollection::New();
  this->Mapper = NULL;
  this->DataSet = NULL;

  this->X0 = 0.0;
  this->Y0 = 0.0;
  this->X1 = 0.0;
  this->Y1 = 0.0;
}

//--------------------------------------------------------------------------
svkAreaPicker::~svkAreaPicker()
{
  this->Prop3Ds->Delete();
  this->ClipPoints->Delete();  
  this->Frustum->Delete();
  this->FrustumExtractor->Delete();
}

//--------------------------------------------------------------------------
// Initialize the picking process.
void svkAreaPicker::Initialize()
{
  this->vtkAbstractPropPicker::Initialize();
  this->Prop3Ds->RemoveAllItems();
  this->Mapper = NULL;
}

//--------------------------------------------------------------------------
void svkAreaPicker::SetRenderer(vtkRenderer *renderer)
{
  this->Renderer = renderer;
}
//--------------------------------------------------------------------------
void svkAreaPicker::SetPickCoords(double x0, double y0, double x1, double y1)
{
  this->X0 = x0;
  this->Y0 = y0;
  this->X1 = x1;
  this->Y1 = y1;
}
//--------------------------------------------------------------------------
int svkAreaPicker::Pick()
{
  return 
    this->AreaPick(this->X0, this->Y0, this->X1, this->Y1, this->Renderer);
}

//--------------------------------------------------------------------------
// Does what this class is meant to do.
int svkAreaPicker::AreaPick(double x0, double y0, double x1, double y1, 
                            vtkRenderer *renderer)
{
  this->Initialize();
  this->X0 = x0;
  this->Y0 = y0;
  this->X1 = x1;
  this->Y1 = y1;
  if (renderer)
    {
    this->Renderer = renderer;
    }

  this->SelectionPoint[0] = (this->X0+this->X1)*0.5;
  this->SelectionPoint[1] = (this->Y0+this->Y1)*0.5;
  this->SelectionPoint[2] = 0.0;

  this->InvokeEvent(vtkCommand::StartPickEvent,NULL);

  if ( this->Renderer == NULL )
    {
    vtkErrorMacro(<<"Must specify renderer!");
    return 0;
    }

  this->DefineFrustum(this->X0, this->Y0, this->X1, this->Y1, this->Renderer);

  return this->PickProps(this->Renderer);  
}

//--------------------------------------------------------------------------
//Converts the given screen rectangle into a selection frustum.
//Saves the results in ClipPoints and Frustum.
void svkAreaPicker::DefineFrustum(double x0, double y0, double x1, double y1, 
                                  vtkRenderer *renderer)
{
  this->X0 = (x0 < x1) ? x0 : x1;
  this->Y0 = (y0 < y1) ? y0 : y1;
  this->X1 = (x0 > x1) ? x0 : x1;
  this->Y1 = (y0 > y1) ? y0 : y1;

  if (this->X0 == this->X1)
    {
    this->X1 += 1.0;
    }
  if (this->Y0 == this->Y1)
    {
    this->Y1 += 1.0;
    }

  //compute world coordinates of the pick volume 
  double verts[32];
  renderer->SetDisplayPoint(this->X0, this->Y0, 0);
  renderer->DisplayToWorld();
  renderer->GetWorldPoint(&verts[0]);

  renderer->SetDisplayPoint(this->X0, this->Y0, 1);
  renderer->DisplayToWorld();
  renderer->GetWorldPoint(&verts[4]);

  renderer->SetDisplayPoint(this->X0, this->Y1, 0);
  renderer->DisplayToWorld();
  renderer->GetWorldPoint(&verts[8]);

  renderer->SetDisplayPoint(this->X0, this->Y1, 1);
  renderer->DisplayToWorld();
  renderer->GetWorldPoint(&verts[12]);

  renderer->SetDisplayPoint(this->X1, this->Y0, 0);
  renderer->DisplayToWorld();
  renderer->GetWorldPoint(&verts[16]);

  renderer->SetDisplayPoint(this->X1, this->Y0, 1);
  renderer->DisplayToWorld();
  renderer->GetWorldPoint(&verts[20]);

  renderer->SetDisplayPoint(this->X1, this->Y1, 0);
  renderer->DisplayToWorld();
  renderer->GetWorldPoint(&verts[24]);
  
  renderer->SetDisplayPoint(this->X1, this->Y1, 1);
  renderer->DisplayToWorld();
  renderer->GetWorldPoint(&verts[28]);    
    
  //a pick point is required by vtkAbstractPicker
  //return center for now until a better meaning is desired
  double sum[3] = {0.0,0.0,0.0};
  for (int i = 0; i < 8; i++)
    {
    sum[0] += verts[i*3+0];
    sum[1] += verts[i*3+1];
    sum[2] += verts[i*3+2];
    }
  this->PickPosition[0] = sum[0]/8.0;
  this->PickPosition[1] = sum[1]/8.0;
  this->PickPosition[2] = sum[2]/8.0;

  this->FrustumExtractor->CreateFrustum(verts);
}

//--------------------------------------------------------------------------
//Decides which props are within the frustum.
//Adds each to the prop3d list and fires pick events.
//Remembers the dataset, mapper, and assembly path for the nearest.
int svkAreaPicker::PickProps(vtkRenderer *renderer)
{
  vtkProp *prop;
  int picked=0;
  int pickable;
  double bounds[6];
  
  //  Initialize picking process
  this->Initialize();
  this->Renderer = renderer;
 
  // Invoke start pick method if defined
  this->InvokeEvent(vtkCommand::StartPickEvent,NULL);

  if ( renderer == NULL )
    {
    vtkErrorMacro(<<"Must specify renderer!");
    return 0;
    }

  //  Loop over all props.  
  //
  vtkPropCollection *props;
  vtkProp *propCandidate;
  if ( this->PickFromList ) 
    {
    props = this->GetPickList();
    }
  else 
    {
    props = renderer->GetViewProps();
    }

  vtkImageActor *imageActor = NULL;
  vtkAbstractMapper3D *mapper = NULL;
  vtkAssemblyPath *path;

  double mindist = VTK_DOUBLE_MAX;

  vtkCollectionSimpleIterator pit;
  for ( props->InitTraversal(pit); (prop=props->GetNextProp(pit)); )
    {
    for ( prop->InitPathTraversal(); (path=prop->GetNextPath()); )
      {
      propCandidate = path->GetLastNode()->GetViewProp();
      pickable = this->TypeDecipher(propCandidate, &imageActor, &mapper);

      //  If actor can be picked, see if it is within the pick frustum.
      if ( pickable )
        {
        if ( mapper )
          {
          mapper->GetBounds(bounds);   
          double *bds = propCandidate->GetBounds(); 
          for (int i = 0; i < 6; i++) 
            { bounds[i] = bds[i]; }
          double dist;
          //cerr << "mapper ABFISECT" << endl;
          if (this->ABoxFrustumIsect(bounds, dist))
            {
            picked = 1;
            if ( ! this->Prop3Ds->IsItemPresent(prop) )
              {
              this->Prop3Ds->AddItem(static_cast<vtkProp3D *>(prop));
              //cerr << "picked a mapper" << endl;
              if (dist < mindist) //new nearest, remember it
                {
                mindist = dist;
                this->SetPath(path);
                this->Mapper = mapper; 
                vtkMapper *map1;
                vtkAbstractVolumeMapper *vmap;
                if ( (map1=vtkMapper::SafeDownCast(mapper)) != NULL )
                  {
                  this->DataSet = map1->GetInput();
                  this->Mapper = map1;
                  }
                else if ( (vmap=vtkAbstractVolumeMapper::SafeDownCast(mapper)) != NULL )
                  {
                  this->DataSet = vmap->GetDataSetInput();
                  this->Mapper = vmap;
                  }
                else
                  {
                  this->DataSet = NULL;
                  }              
                }
              static_cast<vtkProp3D *>(propCandidate)->Pick();
              this->InvokeEvent(vtkCommand::PickEvent,NULL);
              }
            }
          }//mapper
        else if ( imageActor )
          {
          imageActor->GetBounds(bounds);
          double dist;
          //cerr << "imageA ABFISECT" << endl;
          if (this->ABoxFrustumIsect(bounds, dist))
            {
            picked = 1;          
            if ( ! this->Prop3Ds->IsItemPresent(prop) )
              {
              this->Prop3Ds->AddItem(imageActor);
              //cerr << "picked an imageactor" << endl;
              if (dist < mindist) //new nearest, remember it
                {
                mindist = dist;
                this->SetPath(path);
                this->Mapper = mapper; // mapper is null
                this->DataSet = imageActor->GetInput();
                }
              imageActor->Pick();
              this->InvokeEvent(vtkCommand::PickEvent,NULL);          
              }
            }
          }//imageActor
        }//pickable

      }//for all parts
    }//for all props

  // Invoke end pick method if defined
  this->InvokeEvent(vtkCommand::EndPickEvent,NULL);

  return picked;
}

//------------------------------------------------------------------------------
//converts the propCandidate into either a vtkImageActor or a 
//vtkAbstractMapper3D and returns its pickability
int svkAreaPicker::TypeDecipher(vtkProp *propCandidate, 
                                vtkImageActor **imageActor, 
                                vtkAbstractMapper3D **mapper)
{
  int pickable = 0;
  *imageActor = NULL;
  *mapper = NULL;

  vtkActor *actor;
  vtkLODProp3D *prop3D;
  vtkProperty *tempProperty;
  vtkVolume *volume;

  if ( propCandidate->GetPickable() && propCandidate->GetVisibility() )
    {
    pickable = 1;
    if ( (actor=vtkActor::SafeDownCast(propCandidate)) != NULL )
      {
      *mapper = actor->GetMapper();
      if ( actor->GetProperty()->GetOpacity() <= 0.0 )
        {
        pickable = 0;
        }
      }
    else if ( (prop3D=vtkLODProp3D::SafeDownCast(propCandidate)) != NULL )
      {
      int LODId = prop3D->GetPickLODID();
      *mapper = prop3D->GetLODMapper(LODId);
      if ( vtkMapper::SafeDownCast(*mapper) != NULL)
        {
        prop3D->GetLODProperty(LODId, &tempProperty);
        if ( tempProperty->GetOpacity() <= 0.0 )
          {
          pickable = 0;
          }
        }
      }
    else if ( (volume=vtkVolume::SafeDownCast(propCandidate)) != NULL )
      {
      *mapper = volume->GetMapper();
      }
    else if ( (*imageActor=vtkImageActor::SafeDownCast(propCandidate)) )
      {
      *mapper = 0;
      }
    else 
      {
      pickable = 0; //only vtkProp3D's (actors and volumes) can be picked
      }
    }
  return pickable;
}

//--------------------------------------------------------------------------
//Intersect the bbox represented by the bounds with the clipping frustum.
//Return true if partially inside.
//Also return a distance to the near plane.
int svkAreaPicker::ABoxFrustumIsect(double *bounds, double &mindist)
{
  if (bounds[0] > bounds[1] ||
      bounds[2] > bounds[3] ||
      bounds[4] > bounds[5]) 
    {
    return 0;
    }
    
  double verts[8][3];
  int x, y, z;
  int vid = 0;
  for (x = 0; x < 2; x++)
    {
    for (y = 0; y < 2; y++)
      {
      for (z = 0; z < 2; z++)
        {
        verts[vid][0] = bounds[0+x];
        verts[vid][1] = bounds[2+y];
        verts[vid][2] = bounds[4+z];
        vid++;
        }
      }
    }

  //find distance to the corner nearest the near plane for 'closest' prop
  mindist = -VTK_DOUBLE_MAX;
  vtkPlane *plane = this->Frustum->GetPlane(4); //near plane
  for (vid = 0; vid < 8; vid++)
    {
    double dist = plane->EvaluateFunction(verts[vid]);
    if (dist < 0 && dist > mindist)
      {
      mindist = dist;
      }                
    }
  mindist = -mindist;

  //leave the intersection test to the frustum extractor class
  return this->FrustumExtractor->OverallBoundsTest(bounds);
}

//--------------------------------------------------------------------------
void svkAreaPicker::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Frustum: " << this->Frustum << "\n";
  os << indent << "ClipPoints: " << this->ClipPoints << "\n";
  os << indent << "Mapper: " << this->Mapper << "\n";
  os << indent << "DataSet: " << this->DataSet << "\n";
}
