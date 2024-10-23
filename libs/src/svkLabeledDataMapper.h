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
 *  This code was copied from vtk-5.6.1's class svkLabeledDataMapper.
 *  There were non-virtual methods that we needed to modify to get
 *  the required behavior.
 *
 */

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkLabeledDataMapper.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkLabeledDataMapper - draw text labels at dataset points
// .SECTION Description
// vtkLabeledDataMapper is a mapper that renders text at dataset
// points. Various items can be labeled including point ids, scalars,
// vectors, normals, texture coordinates, tensors, and field data components.
//
// The format with which the label is drawn is specified using a
// printf style format string. The font attributes of the text can
// be set through the vtkTextProperty associated to this mapper. 
//
// By default, all the components of multi-component data such as
// vectors, normals, texture coordinates, tensors, and multi-component
// scalars are labeled. However, you can specify a single component if
// you prefer. (Note: the label format specifies the format to use for
// a single component. The label is creating by looping over all components
// and using the label format to render each component.)

// .SECTION Caveats
// Use this filter in combination with vtkSelectVisiblePoints if you want
// to label only points that are visible. If you want to label cells rather
// than points, use the filter vtkCellCenters to generate points at the
// center of the cells. Also, you can use the class vtkIdFilter to
// generate ids as scalars or field data, which can then be labeled.

// .SECTION See Also
// vtkMapper2D vtkActor2D vtkTextMapper vtkTextProperty vtkSelectVisiblePoints 
// vtkIdFilter vtkCellCenters

#ifndef __svkLabeledDataMapper_h
#define __svkLabeledDataMapper_h

#include </usr/include/vtk/vtkMapper2D.h>
#include <vector>
#include <map>
#include <string>
#include <svkMriImageData.h>
#include <svkVoxelTaggingUtils.h>


class vtkDataObject;
class vtkDataSet;
class vtkTextMapper;
class vtkTextProperty;
class vtkTransform;

#define VTK_LABEL_IDS        0
#define VTK_LABEL_SCALARS    1
#define VTK_LABEL_VECTORS    2
#define VTK_LABEL_NORMALS    3
#define VTK_LABEL_TCOORDS    4
#define VTK_LABEL_TENSORS    5
#define VTK_LABEL_FIELD_DATA 6
#define SVK_LABEL_DATA_TAGS  7

using namespace std;

class svkLabeledDataMapper : public vtkMapper2D
{
    public:

        // Description:
        // Instantiate object with %%-#6.3g label format. By default, point ids
        // are labeled.
        static svkLabeledDataMapper *New();

        vtkTypeMacro(svkLabeledDataMapper,vtkMapper2D);
        void PrintSelf(ostream& os, vtkIndent indent);
      
        // Description:
        // Set/Get the format with which to print the labels.  This should
        // be a printf-style format string.
        //
        // By default, the mapper will try to print each component of the
        // tuple using a sane format: %d for integers, %f for floats, %g for
        // doubles, %ld for longs, et cetera.  If you need a different
        // format, set it here.  You can do things like limit the number of
        // significant digits, add prefixes/suffixes, basically anything
        // that printf can do.  If you only want to print one component of a
        // vector, see the ivar LabeledComponent.
        vtkSetStringMacro(LabelFormat);
        vtkGetStringMacro(LabelFormat);
    
        // Description:
        // Set/Get the component number to label if the data to print has
        // more than one component. For example, all the components of
        // scalars, vectors, normals, etc. are labeled by default
        // (LabeledComponent=(-1)). However, if this ivar is nonnegative,
        // then only the one component specified is labeled.
        vtkSetMacro(LabeledComponent,int);
        vtkGetMacro(LabeledComponent,int);
    
        vtkSetMacro(DisplayTags,bool);
        vtkGetMacro(DisplayTags,bool);
    
        // Description:
        // Set/Get the field data array to label. This instance variable is
        // only applicable if field data is labeled.  This will clear
        // FieldDataName when set.
        void SetFieldDataArray(int arrayIndex);
        vtkGetMacro(FieldDataArray,int);

        // Description:
        // Set/Get the name of the field data array to label.  This instance
        // variable is only applicable if field data is labeled.  This will
        // override FieldDataArray when set.
        void SetFieldDataName(const char *arrayName);
        vtkGetStringMacro(FieldDataName);

        // Description:
        // Set the input dataset to the mapper. This mapper handles any type of data.
        virtual void SetInput(vtkDataObject*);

        // Description:
        // Use GetInputDataObject() to get the input data object for composite
        // datasets.
        vtkDataSet *GetInput();

        // Description:
        // Specify which data to plot: IDs, scalars, vectors, normals, texture coords,
        // tensors, or field data. If the data has more than one component, use
        // the method SetLabeledComponent to control which components to plot.
        // The default is VTK_LABEL_IDS.
        vtkSetMacro(LabelMode, int);
        vtkGetMacro(LabelMode, int);
        void SetLabelModeToLabelIds() {this->SetLabelMode(VTK_LABEL_IDS);};
        void SetLabelModeToLabelScalars() {this->SetLabelMode(VTK_LABEL_SCALARS);};
        void SetLabelModeToLabelVectors() {this->SetLabelMode(VTK_LABEL_VECTORS);};
        void SetLabelModeToLabelNormals() {this->SetLabelMode(VTK_LABEL_NORMALS);};
        void SetLabelModeToLabelTCoords() {this->SetLabelMode(VTK_LABEL_TCOORDS);};
        void SetLabelModeToLabelTensors() {this->SetLabelMode(VTK_LABEL_TENSORS);};
        void SetLabelModeToLabelTags() {this->SetLabelMode(SVK_LABEL_DATA_TAGS);};
        void SetLabelModeToLabelFieldData()
                    {this->SetLabelMode(VTK_LABEL_FIELD_DATA);};
    
        // Description:
        // Set/Get the text property.
        // If an integer argument is provided, you may provide different text
        // properties for different label types. The type is determined by an
        // optional type input array.
        virtual void SetLabelTextProperty(vtkTextProperty *p)
            { this->SetLabelTextProperty(p, 0); }
        virtual vtkTextProperty* GetLabelTextProperty()
            { return this->GetLabelTextProperty(0); }
        virtual void SetLabelTextProperty(vtkTextProperty *p, int type);
        virtual vtkTextProperty* GetLabelTextProperty(int type);

        // Description:
        // Draw the text to the screen at each input point.
        void RenderOpaqueGeometry(vtkViewport* viewport, vtkActor2D* actor);
        void RenderOverlay(vtkViewport* viewport, vtkActor2D* actor);

        // Description:
        // Release any graphics resources that are being consumed by this actor.
        virtual void ReleaseGraphicsResources(vtkWindow *);
      
        // Description:
        // The transform to apply to the labels before mapping to 2D.
        vtkGetObjectMacro(Transform, vtkTransform);
        void SetTransform(vtkTransform* t);
    
        //BTX
        /// Coordinate systems that output dataset may use.
        enum Coordinates
            {
            WORLD=0,           //!< Output 3-D world-space coordinates for each label anchor.
            DISPLAY=1          //!< Output 2-D display coordinates for each label anchor (3 components but only 2 are significant).
            };
        //ETX
    
        // Description:
        // Set/get the coordinate system used for output labels.
        // The output datasets may have point coordinates reported in the world space or display space.
        vtkGetMacro(CoordinateSystem,int);
            vtkSetClampMacro(CoordinateSystem,int,WORLD,DISPLAY);
        void CoordinateSystemWorld() { this->SetCoordinateSystem( svkLabeledDataMapper::WORLD ); }
        void CoordinateSystemDisplay() { this->SetCoordinateSystem( svkLabeledDataMapper::DISPLAY ); }
    
        // Description:
        // Return the modified time for this object.
        virtual unsigned long GetMTime();
    
    protected:
        svkLabeledDataMapper();
        ~svkLabeledDataMapper();
    
        vtkDataSet *Input;
    
        char* LabelFormat;
        int   LabelMode;
        int   LabeledComponent;
        int   FieldDataArray;
        char* FieldDataName;
        int   CoordinateSystem;
        bool  DisplayTags;

        vtkTimeStamp BuildTime;

        int             NumberOfLabels;
        int             NumberOfLabelsAllocated;
        vtkTextMapper** TextMappers;
        double*         LabelPositions;
        vtkTransform*   Transform;

        virtual int     FillInputPortInformation(int, vtkInformation*);

        void            AllocateLabels(int numLabels);
        void            BuildLabels();
        void            BuildLabelsInternal(vtkDataSet*);
      
        //BTX
        class       Internals;
        Internals*  Implementation;
        //ETX

    private:
        svkLabeledDataMapper(const svkLabeledDataMapper&);  // Not implemented.
        void operator=(const svkLabeledDataMapper&);  // Not implemented.
};

#endif

