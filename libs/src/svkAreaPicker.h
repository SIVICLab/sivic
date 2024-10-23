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
  Module:    $RCSfile: svkAreaPicker.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME svkAreaPicker - Picks props behind a selection rectangle on a viewport.
//
// .SECTION Description
// The svkAreaPicker picks all vtkProp3Ds that lie behind the screen space 
// rectangle from x0,y0 and x1,y1. The selection is based upon the bounding
// box of the prop and is thus not exact.
//
// Like vtkPicker, a pick results in a list of Prop3Ds because many props may 
// lie within the pick frustum. You can also get an AssemblyPath, which in this
// case is defined to be the path to the one particular prop in the Prop3D list
// that lies nearest to the near plane. 
//
// This picker also returns the selection frustum, defined as either a
// vtkPlanes, or a set of eight corner vertices in world space. The vtkPlanes 
// version is an ImplicitFunction, which is suitable for use with the
// vtkExtractGeometry. The six frustum planes are in order: left, right, 
// bottom, top, near, far
//
// Because this picker picks everything within a volume, the world pick point 
// result is ill-defined. Therefore if you ask this class for the world pick 
// position, you will get the centroid of the pick frustum. This may be outside
// of all props in the prop list.
//
// .SECTION See Also
// vtkInteractorStyleRubberBandPick, vtkExtractSelectedFrustum.

#ifndef __svkAreaPicker_h
#define __svkAreaPicker_h

#include "/usr/include/vtk/vtkAbstractPropPicker.h"


class vtkRenderer;
class vtkPoints;
class vtkPlanes;
class vtkProp3DCollection;
class vtkAbstractMapper3D;
class vtkDataSet;
class vtkExtractSelectedFrustum;
class vtkProp;
class vtkImageActor;

class VTK_RENDERING_EXPORT svkAreaPicker : public vtkAbstractPropPicker
{
public:
  static svkAreaPicker *New();
  vtkTypeMacro(svkAreaPicker,vtkAbstractPropPicker);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the default screen rectangle to pick in.
  void SetPickCoords(double x0, double y0, double x1, double y1);
  
  // Description:
  // Set the default renderer to pick on.
  void SetRenderer(vtkRenderer *);

  // Description:
  // Perform an AreaPick within the default screen rectangle and renderer.
  virtual int Pick();

  // Description:
  // Perform pick operation in volume behind the given screen coordinates.
  // Props intersecting the selection frustum will be accesible via GetProp3D.
  // GetPlanes returns a vtkImplicitFunciton suitable for vtkExtractGeometry.
  virtual int AreaPick(double x0, double y0, double x1, double y1, vtkRenderer *renderer = NULL);

  // Description:
  // Perform pick operation in volume behind the given screen coordinate.
  // This makes a thin frustum around the selected pixel.
  // Note: this ignores Z in order to pick everying in a volume from z=0 to z=1.
  virtual int Pick(double x0, double y0, double vtkNotUsed(z0), vtkRenderer *renderer = NULL)
    {return this->AreaPick(x0, y0, x0+1.0, y0+1.0, renderer);};

  // Description:
  // Return mapper that was picked (if any).
  vtkGetObjectMacro(Mapper,vtkAbstractMapper3D);

  // Description:
  // Get a pointer to the dataset that was picked (if any). If nothing 
  // was picked then NULL is returned.
  vtkGetObjectMacro(DataSet,vtkDataSet);

  // Description:
  // Return a collection of all the prop 3D's that were intersected
  // by the pick ray. This collection is not sorted.
  vtkProp3DCollection *GetProp3Ds() {return this->Prop3Ds;};

  // Description:
  // Return the six planes that define the selection frustum. The implicit 
  // function defined by the planes evaluates to negative inside and positive
  // outside.
  vtkGetObjectMacro(Frustum, vtkPlanes);

  // Description:
  // Return eight points that define the selection frustum.
  vtkGetObjectMacro(ClipPoints, vtkPoints);

protected:
  svkAreaPicker();
  ~svkAreaPicker();

  virtual void Initialize();
  void DefineFrustum(double x0, double y0, double x1, double y1, vtkRenderer *renderer);
  virtual int PickProps(vtkRenderer *renderer);  
  int TypeDecipher(vtkProp *, vtkImageActor **, vtkAbstractMapper3D **);

  int ABoxFrustumIsect(double bounds[], double &mindist);

  vtkPoints *ClipPoints;
  vtkPlanes *Frustum;

  vtkProp3DCollection *Prop3Ds; //candidate actors (based on bounding box)
  vtkAbstractMapper3D *Mapper; //selected mapper (if the prop has a mapper)
  vtkDataSet *DataSet; //selected dataset (if there is one)

  //used internally to do prop intersection tests
  vtkExtractSelectedFrustum *FrustumExtractor;

  double X0;
  double Y0;
  double X1;
  double Y1;

private:
  svkAreaPicker(const svkAreaPicker&);  // Not implemented.
  void operator=(const svkAreaPicker&);  // Not implemented.
};


#endif


