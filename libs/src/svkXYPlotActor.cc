/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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


#include <svkXYPlotActor.h>


using namespace svk;


//vtkCxxRevisionMacro(svkXYPlotActor, "$Rev$");
vtkStandardNewMacro(svkXYPlotActor);


//! Constructor
svkXYPlotActor::svkXYPlotActor()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

}


//! Destructor
svkXYPlotActor::~svkXYPlotActor()
{
    
}


/*!
 *  To make the required adjustments we needed to copy this inline method
 *  from the parent.
 *
 * @param field
 * @param tuple
 * @param component
 * @param val
 * @return
 */
static inline int vtkXYPlotActorGetComponent(vtkFieldData* field,
  vtkIdType tuple, int component, double* val)
{
  int array_comp;
  int array_index = field->GetArrayContainingComponent(component, array_comp);
  if (array_index < 0)
    {
    return 0;
    }
  vtkDataArray* da = field->GetArray(array_index);
  if (!da)
    {
    // non-numeric array.
    return 0;
    }
  *val = da->GetComponent(tuple, array_comp);
  return 1;
}


/*!
 *  Modified to support reversing x-axis.
 *
 * @param pos
 * @param pos2
 * @param xRange
 * @param yRange
 * @param lengths
 * @param numDS
 * @param numDO
 */
void svkXYPlotActor::CreatePlotData(int *pos, int *pos2, double xRange[2],
                                    double yRange[2], double *lengths,
                                    int numDS, int numDO)
{
  double xyz[3]; xyz[2] = 0.0;
  double xyzTransform[3];
  int i, numLinePts, dsNum, doNum, num;
  vtkIdType numPts, ptId, id;
  double length, x[3], xPrev[3];
  vtkDataArray *scalars;
  int component;
  vtkDataSet *ds;
  vtkCellArray *lines;
  vtkPoints *pts;
  int clippingRequired = 0;

  // Allocate resources for the polygonal plots
  //
  num = (numDS > numDO ? numDS : numDO);
  this->InitializeEntries();
  this->NumberOfInputs = num;
  this->PlotData = new vtkPolyData* [num];
  this->PlotGlyph = new vtkGlyph2D* [num];
  this->PlotAppend = new vtkAppendPolyData* [num];
  this->PlotMapper = new vtkPolyDataMapper2D* [num];
  this->PlotActor = new vtkActor2D* [num];
  for (i=0; i<num; i++)
    {
    this->PlotData[i] = vtkPolyData::New();
    this->PlotGlyph[i] = vtkGlyph2D::New();
    this->PlotGlyph[i]->SetInputData(this->PlotData[i]);
    this->PlotGlyph[i]->SetScaleModeToDataScalingOff();
    this->PlotAppend[i] = vtkAppendPolyData::New();
    this->PlotAppend[i]->AddInput(this->PlotData[i]);
    if ( this->LegendActor->GetEntrySymbol(i) != NULL &&
         this->LegendActor->GetEntrySymbol(i) != this->GlyphSource->GetOutput() )
      {
      this->PlotGlyph[i]->SetSource(this->LegendActor->GetEntrySymbol(i));
      this->PlotGlyph[i]->SetScaleFactor(this->ComputeGlyphScale(i,pos,pos2));
      this->PlotAppend[i]->AddInput(this->PlotGlyph[i]->GetOutput());
      }
    this->PlotMapper[i] = vtkPolyDataMapper2D::New();
    this->PlotMapper[i]->SetInputConnection(this->PlotAppend[i]->GetOutputPort());
    this->PlotMapper[i]->ScalarVisibilityOff();
    this->PlotActor[i] = vtkActor2D::New();
    this->PlotActor[i]->SetMapper(this->PlotMapper[i]);
    this->PlotActor[i]->GetProperty()->DeepCopy(this->GetProperty());
    if ( this->LegendActor->GetEntryColor(i)[0] < 0.0 )
      {
      this->PlotActor[i]->GetProperty()->SetColor(
        this->GetProperty()->GetColor());
      }
    else
      {
      this->PlotActor[i]->GetProperty()->SetColor(
        this->LegendActor->GetEntryColor(i));
      }
    }

  // Prepare to receive data
  this->GenerateClipPlanes(pos,pos2);
  for (i=0; i<this->NumberOfInputs; i++)
    {
    lines = vtkCellArray::New();
    pts = vtkPoints::New();

    lines->Allocate(10,10);
    pts->Allocate(10,10);
    this->PlotData[i]->SetPoints(pts);
    this->PlotData[i]->SetVerts(lines);
    this->PlotData[i]->SetLines(lines);

    pts->Delete();
    lines->Delete();
    }

  // Okay, for each input generate plot data. Depending on the input
  // we use either dataset or data object.
  //
  if ( numDS > 0 )
    {
    vtkCollectionSimpleIterator dsit;
    for ( dsNum=0, this->InputList->InitTraversal(dsit);
          (ds = this->InputList->GetNextDataSet(dsit)); dsNum++ )
      {
      clippingRequired = 0;
      numPts = ds->GetNumberOfPoints();
      scalars = ds->GetPointData()->GetScalars(this->SelectedInputScalars[dsNum]);
      if ( !scalars)
        {
        continue;
        }
      component = this->SelectedInputScalarsComponent->GetValue(dsNum);
      if ( component < 0 || component >= scalars->GetNumberOfComponents())
        {
        continue;
        }

      pts = this->PlotData[dsNum]->GetPoints();
      lines = this->PlotData[dsNum]->GetLines();
      lines->InsertNextCell(0); //update the count later

      ds->GetPoint(0, xPrev);
      for ( numLinePts=0, length=0.0, ptId=0; ptId < numPts; ptId++ )
        {
        xyz[1] = scalars->GetComponent(ptId, component);
        ds->GetPoint(ptId, x);
        switch (this->XValues)
          {
          case VTK_XYPLOT_NORMALIZED_ARC_LENGTH:
            length += sqrt(vtkMath::Distance2BetweenPoints(x,xPrev));
            xyz[0] = length / lengths[dsNum];
            xPrev[0] = x[0]; xPrev[1] = x[1]; xPrev[2] = x[2];
            break;
          case VTK_XYPLOT_INDEX:
            xyz[0] = (double)ptId;
            break;
          case VTK_XYPLOT_ARC_LENGTH:
            length += sqrt(vtkMath::Distance2BetweenPoints(x,xPrev));
            xyz[0] = length;
            xPrev[0] = x[0]; xPrev[1] = x[1]; xPrev[2] = x[2];
            break;
          case VTK_XYPLOT_VALUE:
            xyz[0] = x[this->XComponent->GetValue(dsNum)];
            break;
          default:
            vtkErrorMacro(<< "Unknown X-Component option");
          }

        if ( this->GetLogx() == 1 )
          {
          if (xyz[0] > 0)
            {
            xyz[0] = log10(xyz[0]);
            // normalize and position
            if ( xyz[0] < xRange[0] || xyz[0] > xRange[1] ||
                 xyz[1] < yRange[0] || xyz[1] > yRange[1] )
              {
              clippingRequired = 1;
              }

            numLinePts++;
            xyz[0] = pos[0] +
              (xyz[0]-xRange[0])/(xRange[1]-xRange[0])*(pos2[0]-pos[0]);
            xyz[1] = pos[1] +
              (xyz[1]-yRange[0])/(yRange[1]-yRange[0])*(pos2[1]-pos[1]);
            this->TransformPoint(pos, pos2, xyz, xyzTransform);
            id = pts->InsertNextPoint(xyzTransform);
            lines->InsertCellPoint(id);
            }
          }
        else
          {
          // normalize and position
          if ( xyz[0] < xRange[0] || xyz[0] > xRange[1] ||
               xyz[1] < yRange[0] || xyz[1] > yRange[1] )
            {
            clippingRequired = 1;
            }

          numLinePts++;
          xyz[0] = pos[0] +
            (xyz[0]-xRange[0])/(xRange[1]-xRange[0])*(pos2[0]-pos[0]);
          xyz[1] = pos[1] +
            (xyz[1]-yRange[0])/(yRange[1]-yRange[0])*(pos2[1]-pos[1]);
          this->TransformPoint(pos, pos2, xyz, xyzTransform);
          id = pts->InsertNextPoint(xyzTransform);
          lines->InsertCellPoint(id);
          }
        }//for all input points

      lines->UpdateCellCount(numLinePts);
      if ( clippingRequired )
        {
        this->ClipPlotData(pos,pos2,this->PlotData[dsNum]);
        }
      }//loop over all input data sets
    }//if plotting datasets

  else //plot data from data objects
    {
    vtkDataObject *dobj;
    int numColumns;
    vtkIdType numRows, numTuples;
    vtkDataArray *array;
    vtkFieldData *field;
    vtkCollectionSimpleIterator doit;
    for ( doNum=0, this->DataObjectInputList->InitTraversal(doit);
          (dobj = this->DataObjectInputList->GetNextDataObject(doit));
          doNum++ )
      {
      // determine the shape of the field
      field = dobj->GetFieldData();
      numColumns = field->GetNumberOfComponents(); //number of "columns"
      // numColumns also includes non-numeric array components.
      for (numRows = VTK_LARGE_ID, i=0; i<field->GetNumberOfArrays(); i++)
        {
        array = field->GetArray(i);
        if (!array)
          {
          // skip non-numeric arrays.
          continue;
          }
        numTuples = array->GetNumberOfTuples();
        if ( numTuples < numRows )
          {
          numRows = numTuples;
          }
        }

      pts = this->PlotData[doNum]->GetPoints();
      lines = this->PlotData[doNum]->GetLines();
      lines->InsertNextCell(0); //update the count later

      numPts = (this->DataObjectPlotMode == VTK_XYPLOT_ROW ?
                numColumns : numRows);

      // gather the information to form a plot
      for ( numLinePts=0, length=0.0, ptId=0; ptId < numPts; ptId++ )
        {
        int status1, status2;
        if ( this->DataObjectPlotMode == VTK_XYPLOT_ROW )
          {
          //x[0] = field->GetComponent(this->XComponent->GetValue(doNum),ptId);
          //xyz[1] = field->GetComponent(this->YComponent->GetValue(doNum),ptId);
          status1 = ::vtkXYPlotActorGetComponent(field,
            this->XComponent->GetValue(doNum), ptId, &x[0]);
          status2 = ::vtkXYPlotActorGetComponent(field,
            this->YComponent->GetValue(doNum), ptId, &xyz[1]);
          }
        else //if ( this->DataObjectPlotMode == VTK_XYPLOT_COLUMN )
          {
          //x[0] = field->GetComponent(ptId, this->XComponent->GetValue(doNum));
          //xyz[1] = field->GetComponent(ptId, this->YComponent->GetValue(doNum));

          status1 = ::vtkXYPlotActorGetComponent(field,
            ptId, this->XComponent->GetValue(doNum), &x[0]);

          if (!status1)
            {
            vtkWarningMacro(<< this->XComponent->GetValue(doNum) << " is a non-numeric component.");
            }

          status2 = ::vtkXYPlotActorGetComponent(field,
            ptId, this->YComponent->GetValue(doNum), &xyz[1]);

          if (!status2)
            {
            vtkWarningMacro(<< this->YComponent->GetValue(doNum) << " is a non-numeric component.");
            }
          }
        if (!status1 || !status2)
          {
          // component is non-numeric.
          // Skip it.
          continue;
          }

        switch (this->XValues)
          {
          case VTK_XYPLOT_NORMALIZED_ARC_LENGTH:
            length += fabs(x[0]-xPrev[0]);
            xyz[0] = length / lengths[doNum];
            xPrev[0] = x[0];
            break;
          case VTK_XYPLOT_INDEX:
            xyz[0] = (double)ptId;
            break;
          case VTK_XYPLOT_ARC_LENGTH:
            length += fabs(x[0]-xPrev[0]);
            xyz[0] = length;
            xPrev[0] = x[0];
            break;
          case VTK_XYPLOT_VALUE:
            xyz[0] = x[0];
            break;
          default:
            vtkErrorMacro(<< "Unknown X-Value option");
          }

        if ( this->GetLogx() == 1 )
          {
          if (xyz[0] > 0)
            {
            xyz[0] = log10(xyz[0]);
            // normalize and position
            if ( xyz[0] < xRange[0] || xyz[0] > xRange[1] ||
                 xyz[1] < yRange[0] || xyz[1] > yRange[1] )
              {
              clippingRequired = 1;
              }
            numLinePts++;
            xyz[0] = pos[0] +
              (xyz[0]-xRange[0])/(xRange[1]-xRange[0])*(pos2[0]-pos[0]);
            xyz[1] = pos[1] +
              (xyz[1]-yRange[0])/(yRange[1]-yRange[0])*(pos2[1]-pos[1]);
           this->TransformPoint(pos, pos2, xyz, xyzTransform);
           id = pts->InsertNextPoint(xyzTransform);
            lines->InsertCellPoint(id);
            }
          }
        else
          {
          // normalize and position
          if ( xyz[0] < xRange[0] || xyz[0] > xRange[1] ||
               xyz[1] < yRange[0] || xyz[1] > yRange[1] )
            {
            clippingRequired = 1;
            }
          numLinePts++;
          xyz[0] = pos[0] +
            (xyz[0]-xRange[0])/(xRange[1]-xRange[0])*(pos2[0]-pos[0]);
          xyz[1] = pos[1] +
            (xyz[1]-yRange[0])/(yRange[1]-yRange[0])*(pos2[1]-pos[1]);
          this->TransformPoint(pos, pos2, xyz, xyzTransform);
          id = pts->InsertNextPoint(xyzTransform);
          lines->InsertCellPoint(id);
          }
        }//for all input points

      lines->UpdateCellCount(numLinePts);
      if ( clippingRequired )
        {
        this->ClipPlotData(pos,pos2,this->PlotData[doNum]);
        }
      }//loop over all input data sets
    }

  // Remove points/lines as directed by the user
  for ( i = 0; i < num; i++)
    {
    if (!this->PlotCurveLines)
      {
      if ( !this->PlotLines )
        {
        this->PlotData[i]->SetLines(NULL);
        }
      }
    else
      {
      if ( this->GetPlotLines(i) == 0)
        {
        this->PlotData[i]->SetLines(NULL);
        }
      }

    if (!this->PlotCurvePoints)
      {
      if ( !this->PlotPoints || (this->LegendActor->GetEntrySymbol(i) &&
                                 this->LegendActor->GetEntrySymbol(i) !=
                                 this->GlyphSource->GetOutput()))
        {
        this->PlotData[i]->SetVerts(NULL);
        }
      }
    else
      {
      if ( this->GetPlotPoints(i) == 0 ||
          (this->LegendActor->GetEntrySymbol(i) &&
           this->LegendActor->GetEntrySymbol(i) !=
           this->GlyphSource->GetOutput()))
        {
        this->PlotData[i]->SetVerts(NULL);
        }
      }
    }
}


/*!
 *  Modified to adjust for reversed Axis.
 *
 * @param viewport
 * @param u
 * @param v
 */
void svkXYPlotActor::ViewportToPlotCoordinate(vtkViewport *viewport, double &u, double &v)
{
  int *p0, *p1, *p2;

  // XAxis, YAxis are in viewport coordinates already
  if( this->ReverseXAxis == false ) {
      p0 = this->XAxis->GetPositionCoordinate()->GetComputedViewportValue(viewport);
      p1 = this->XAxis->GetPosition2Coordinate()->GetComputedViewportValue(viewport);
      p2 = this->YAxis->GetPositionCoordinate()->GetComputedViewportValue(viewport);
  } else {
      p0 = this->XAxis->GetPosition2Coordinate()->GetComputedViewportValue(viewport);
      p1 = this->XAxis->GetPositionCoordinate()->GetComputedViewportValue(viewport);
      p2 = this->YAxis->GetPositionCoordinate()->GetComputedViewportValue(viewport);
  }

  u = ((u - p0[0]) / (double)(p1[0] - p0[0]))
    *(this->XComputedRange[1] - this->XComputedRange[0])
    + this->XComputedRange[0];
  v = ((v - p0[1]) / (double)(p2[1] - p0[1]))
    *(this->YComputedRange[1] - this->YComputedRange[0])
    + this->YComputedRange[0];
}
