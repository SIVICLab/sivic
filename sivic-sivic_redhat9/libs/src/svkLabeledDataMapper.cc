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
 *  This code was copied from vtk-5.6.1's class vtkLabeledDataMapper.
 *  There were non-virtual methods that we needed to modify to get
 *  the required behavior.
 *
 */

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkLabeledDataMapper.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "svkLabeledDataMapper.h"

#include "/usr/include/vtk/vtkActor2D.h"
#include "/usr/include/vtk/vtkCompositeDataIterator.h"
#include "/usr/include/vtk/vtkCompositeDataSet.h"
#include "/usr/include/vtk/vtkDataArray.h"
#include "/usr/include/vtk/vtkDataSet.h"
#include "/usr/include/vtk/vtkExecutive.h"
#include "/usr/include/vtk/vtkInformation.h"
#include "/usr/include/vtk/vtkIntArray.h"
#include "/usr/include/vtk/vtkObjectFactory.h"
#include "/usr/include/vtk/vtkPointData.h"
#include "/usr/include/vtk/vtkPointSet.h"
#include "/usr/include/vtk/vtkSmartPointer.h"
#include "/usr/include/vtk/vtkStringArray.h"
#include "/usr/include/vtk/vtkTable.h"
#include "/usr/include/vtk/vtkTextMapper.h"
#include "/usr/include/vtk/vtkTextProperty.h"
#include "/usr/include/vtk/vtkTypeTraits.h"
#include "/usr/include/vtk/vtkType.h"
#include "/usr/include/vtk/vtkTransform.h"
#include "/usr/include/vtk/vtkUnicodeStringArray.h"

#include <map>

class svkLabeledDataMapper::Internals
{
    public:
        map<int, vtkSmartPointer<vtkTextProperty> > TextProperties;
};

vtkStandardNewMacro(svkLabeledDataMapper);

vtkCxxSetObjectMacro(svkLabeledDataMapper,Transform,vtkTransform);

#if defined(_WIN32) && !defined(__CYGWIN__)
# define SNPRINTF _snprintf
#else
# define SNPRINTF snprintf
#endif

// ----------------------------------------------------------------------

template<typename T>
void svkLabeledDataMapper_PrintComponent(char *output, const char *format, int index, const T *array)
{
  sprintf(output, format, array[index]);
}

// ----------------------------------------------------------------------

template<typename T>
void svkLabeledDataMapper_PrintComponent(char *output, const char *format, int index, const T *array, bool useTags, map<int, string> voxelTagDefinitions)
{
	int length = 0;
    if( useTags ) {
		length = voxelTagDefinitions[static_cast<int>(array[index])].copy( output, voxelTagDefinitions[static_cast<int>(array[index])].size());
		output[length] = '\0';
    } else {
		sprintf(output, format, array[index]);
    }

}


//----------------------------------------------------------------------------
// Creates a new label mapper

svkLabeledDataMapper::svkLabeledDataMapper()
{
  this->Implementation = new Internals;

  this->Input = NULL;
  this->LabelMode = VTK_LABEL_IDS;

  this->LabelFormat = NULL;

  this->LabeledComponent = (-1);
  this->FieldDataArray = 0;
  this->FieldDataName = NULL;

  this->NumberOfLabels = 0;
  this->NumberOfLabelsAllocated = 0;

  this->LabelPositions = 0;
  this->TextMappers = 0;
  this->AllocateLabels(50);
  this->DisplayTags = false;

  vtkSmartPointer<vtkTextProperty> prop =
    vtkSmartPointer<vtkTextProperty>::New();
  prop->SetFontSize(12);
  prop->SetBold(1);
  prop->SetItalic(1);
  prop->SetShadow(1);
  prop->SetFontFamilyToArial();
  this->Implementation->TextProperties[0] = prop;

  this->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "type");
  
  this->Transform = 0;
  this->CoordinateSystem = svkLabeledDataMapper::WORLD;
}

//----------------------------------------------------------------------------
svkLabeledDataMapper::~svkLabeledDataMapper()
{
  if (this->LabelFormat)
    {
    delete [] this->LabelFormat;
    }

  delete [] this->LabelPositions;
  if (this->TextMappers != NULL )
    {
    for (int i=0; i < this->NumberOfLabelsAllocated; i++)
      {
      this->TextMappers[i]->Delete();
      }
    delete [] this->TextMappers;
    }
  
  this->SetFieldDataName(NULL);
  this->SetTransform(NULL);
  delete this->Implementation;
}

//----------------------------------------------------------------------------
void svkLabeledDataMapper::AllocateLabels(int numLabels)
{
  if (numLabels > this->NumberOfLabelsAllocated)
    {
    int i;
    // delete old stuff
    delete [] this->LabelPositions;
    this->LabelPositions = 0;
    for (i=0; i < this->NumberOfLabelsAllocated; i++)
      {
      this->TextMappers[i]->Delete();
      }
    delete [] this->TextMappers;
    this->TextMappers = 0;

    this->NumberOfLabelsAllocated = numLabels;

    // Allocate and initialize new stuff
    this->LabelPositions = new double[this->NumberOfLabelsAllocated*3];
    this->TextMappers = new vtkTextMapper * [this->NumberOfLabelsAllocated];
    for (i=0; i<this->NumberOfLabelsAllocated; i++)
      {
      this->TextMappers[i] = vtkTextMapper::New();
      this->LabelPositions[3*i] = 0;
      this->LabelPositions[3*i+1] = 0;
      this->LabelPositions[3*i+2] = 0;
      }
    }
}

//----------------------------------------------------------------------------
void svkLabeledDataMapper::SetLabelTextProperty(vtkTextProperty* prop, int type)
{
  this->Implementation->TextProperties[type] = prop;
  this->Modified();
}

//----------------------------------------------------------------------------
vtkTextProperty* svkLabeledDataMapper::GetLabelTextProperty(int type)
{
  if (this->Implementation->TextProperties.find(type) !=
      this->Implementation->TextProperties.end())
    {
    return this->Implementation->TextProperties[type];
    }
  return 0;
}

//----------------------------------------------------------------------------
void svkLabeledDataMapper::SetInput(vtkDataObject* input)
{
  if (input)
    {
    this->SetInputDataObject(0, input);
    }
  else
    {
    // Setting a NULL input removes the connection.
    this->SetInputConnection(0, 0);
    }
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
vtkDataSet *svkLabeledDataMapper::GetInput()
{
  return vtkDataSet::SafeDownCast(this->GetInputDataObject(0, 0));
}

//----------------------------------------------------------------------------
// Release any graphics resources that are being consumed by this mapper.
void svkLabeledDataMapper::ReleaseGraphicsResources(vtkWindow *win)
{
  if (this->TextMappers != NULL )
    {
    for (int i=0; i < this->NumberOfLabelsAllocated; i++)
      {
      this->TextMappers[i]->ReleaseGraphicsResources(win);
      }
    }
}

//----------------------------------------------------------------------------
void svkLabeledDataMapper::RenderOverlay(vtkViewport *viewport,
                                         vtkActor2D *actor)
{
  for (int i=0; i<this->NumberOfLabels; i++)
    {
    double x[3];
    x[0] = this->LabelPositions[3*i];
    x[1] = this->LabelPositions[3*i + 1];
    x[2] = this->LabelPositions[3*i + 2];

    double* pos = x;
    if (this->Transform)
      {
      pos = this->Transform->TransformDoublePoint(x);
      }

    if(this->CoordinateSystem == svkLabeledDataMapper::WORLD)
      {
      actor->GetPositionCoordinate()->SetCoordinateSystemToWorld();
      actor->GetPositionCoordinate()->SetValue(pos);
      }
    else if(this->CoordinateSystem == svkLabeledDataMapper::DISPLAY)
      {
      actor->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
      actor->GetPositionCoordinate()->SetValue(pos);
      }

    this->TextMappers[i]->RenderOverlay(viewport, actor);
    }
}

//----------------------------------------------------------------------------
void svkLabeledDataMapper::RenderOpaqueGeometry(vtkViewport *viewport,
                                                vtkActor2D *actor)
{
  vtkTextProperty *tprop = this->Implementation->TextProperties[0];
  if (!tprop)
    {
    vtkErrorMacro(<<"Need default text property to render labels");
    return;
    }

  // Updates the input pipeline if needed.
  this->Update();

  vtkDataObject *inputDO = this->GetInputDataObject(0, 0);
  if ( ! inputDO )
    {
    this->NumberOfLabels = 0;
    vtkErrorMacro(<<"Need input data to render labels (2)");
    return;
    }

  // Check for property updates.
  unsigned long propMTime = 0;
  map<int, vtkSmartPointer<vtkTextProperty> >::iterator it, itEnd;
  it = this->Implementation->TextProperties.begin();
  itEnd = this->Implementation->TextProperties.end();
  for (; it != itEnd; ++it)
    {
    vtkTextProperty* prop = it->second;
    if (prop && prop->GetMTime() > propMTime)
      {
      propMTime = prop->GetMTime();
      }
    }

  // Check to see whether we have to rebuild everything
  if ( this->GetMTime() > this->BuildTime || 
       inputDO->GetMTime() > this->BuildTime ||
       propMTime > this->BuildTime)
    {
    this->BuildLabels();
    }

  for (int i=0; i<this->NumberOfLabels; i++)
    {
    double* pos = &this->LabelPositions[3*i];
    if (this->Transform)
      {
      pos = this->Transform->TransformDoublePoint(pos);
      }

    if(this->CoordinateSystem == svkLabeledDataMapper::WORLD)
      {
      actor->GetPositionCoordinate()->SetCoordinateSystemToWorld();
      actor->GetPositionCoordinate()->SetValue(pos);
      }
    else if(this->CoordinateSystem == svkLabeledDataMapper::DISPLAY)
      {
      actor->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
      actor->GetPositionCoordinate()->SetValue(pos);
      }

    this->TextMappers[i]->RenderOpaqueGeometry(viewport, actor);
    }
}

//----------------------------------------------------------------------------
void svkLabeledDataMapper::BuildLabels()
{
  vtkDebugMacro(<<"Rebuilding labels");
  vtkDataObject* inputDO = this->GetInputDataObject(0, 0);
  vtkCompositeDataSet* cd = vtkCompositeDataSet::SafeDownCast(inputDO);
  vtkDataSet* ds = vtkDataSet::SafeDownCast(inputDO);
  if (ds)
    {
    this->AllocateLabels(ds->GetNumberOfPoints());
    this->NumberOfLabels = 0;
    this->BuildLabelsInternal(ds);
    }
  else if (cd)
    {
    this->AllocateLabels(cd->GetNumberOfPoints());
    this->NumberOfLabels = 0;
    vtkCompositeDataIterator* iter = cd->NewIterator();
    for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); 
      iter->GoToNextItem())
      {
      ds = vtkDataSet::SafeDownCast(iter->GetCurrentDataObject());
      if (ds)
        {
        this->BuildLabelsInternal(ds);
        }
      }
    iter->Delete();
    }
  else
    {
    vtkErrorMacro("Unsupported data type: " << inputDO->GetClassName());
    }

  this->BuildTime.Modified();
}

//----------------------------------------------------------------------------
void svkLabeledDataMapper::BuildLabelsInternal(vtkDataSet* input)
{
  int i, j, numComp = 0, pointIdLabels = 0, activeComp = 0;
  vtkAbstractArray *abstractData = NULL;
  vtkDataArray *numericData = NULL;
  vtkStringArray *stringData = NULL;
  vtkUnicodeStringArray *uStringData = NULL;

  if (input->GetNumberOfPoints() == 0)
    {
    return;
    }

  vtkPointData *pd = input->GetPointData();
  // figure out what to label, and if we can label it
  pointIdLabels = 0;
  switch (this->LabelMode)
    {
    case VTK_LABEL_IDS:
    {
      pointIdLabels = 1;
    }; 
    break;
    case VTK_LABEL_SCALARS:
      if ( pd->GetScalars() )
        {
        numericData = pd->GetScalars();
        }
      break;
    case VTK_LABEL_VECTORS:   
      if ( pd->GetVectors() )
        {
        numericData = pd->GetVectors();
        }
      break;
    case VTK_LABEL_NORMALS:    
      if ( pd->GetNormals() )
        {
        numericData = pd->GetNormals();
        }
      break;
    case VTK_LABEL_TCOORDS:    
      if ( pd->GetTCoords() )
        {
        numericData = pd->GetTCoords();
        }
      break;
    case VTK_LABEL_TENSORS:    
      if ( pd->GetTensors() )
        {
        numericData = pd->GetTensors();
        }
      break;
    case VTK_LABEL_FIELD_DATA:
    {
    int arrayNum;
    if (this->FieldDataName != NULL)
      {
      vtkDebugMacro(<<"Labeling field data array " << this->FieldDataName);
      abstractData = pd->GetAbstractArray(this->FieldDataName, arrayNum);
      }
    else
      {
      arrayNum = (this->FieldDataArray < pd->GetNumberOfArrays() ?
                  this->FieldDataArray : pd->GetNumberOfArrays() - 1);
      abstractData = pd->GetAbstractArray(arrayNum);
      }
    numericData = vtkDataArray::SafeDownCast(abstractData);
    stringData = vtkStringArray::SafeDownCast(abstractData);
    uStringData = vtkUnicodeStringArray::SafeDownCast(abstractData);
    }; break;
    }

  // determine number of components and check input
  if ( pointIdLabels )
    {
    numComp = 1;
    }
  else if ( numericData )
    {
    numComp = numericData->GetNumberOfComponents();
    activeComp = 0;
    if ( this->LabeledComponent >= 0 )
      {
      activeComp = (this->LabeledComponent < numComp ? 
                    this->LabeledComponent : numComp - 1);
      numComp = 1;
      }
    }
  else 
    {
    if ( stringData )
      {
      numComp = stringData->GetNumberOfComponents();
      }
    else if( uStringData )
      {
      numComp = uStringData->GetNumberOfComponents();
      }
    else
      {
      if (this->FieldDataName)
        {
        vtkWarningMacro(<< "Could not find label array ("
                        << this->FieldDataName << ") "
                        << "in input.");
        }
      else 
        {
        vtkWarningMacro(<< "Could not find label array ("
                        << "index " << this->FieldDataArray << ") "
                        << "in input.");
        }

      return;
      }
    }

  vtkStdString FormatString;
  if (this->LabelFormat)
    {
    // The user has specified a format string.  
    vtkDebugMacro(<<"Using user-specified format string " << this->LabelFormat);
    FormatString = this->LabelFormat;
    }
  else
    {
    // Try to come up with some sane default.
    if (pointIdLabels)
      {
      FormatString = "%d"; 
      }
    else if (numericData)
      {
      switch (numericData->GetDataType())
        {
        case VTK_VOID: FormatString = "0x%x"; break;

        // dont use vtkTypeTraits::ParseFormat for character types as parse formats
          // aren't the same as print formats for these types.
          case VTK_BIT:
          case VTK_SHORT:
          case VTK_UNSIGNED_SHORT:
          case VTK_INT:
          case VTK_UNSIGNED_INT:
            FormatString = "%d"; break;

          case VTK_CHAR:
          case VTK_SIGNED_CHAR:
          case VTK_UNSIGNED_CHAR:
            FormatString = "%c"; break;

          case VTK_LONG:
            FormatString = vtkTypeTraits<long>::ParseFormat(); break;
          case VTK_UNSIGNED_LONG:
            FormatString = vtkTypeTraits<unsigned long>::ParseFormat(); break;

          case VTK_ID_TYPE:
            FormatString = vtkTypeTraits<vtkIdType>::ParseFormat(); break;

#if defined(VTK_TYPE_USE_LONG_LONG)
          case VTK_LONG_LONG:
            FormatString = vtkTypeTraits<long long>::ParseFormat(); break;
          case VTK_UNSIGNED_LONG_LONG:
            FormatString = vtkTypeTraits<unsigned long long>::ParseFormat(); break;
#endif

#if defined(VTK_TYPE_USE___INT64)
          case VTK___INT64:
            FormatString = vtkTypeTraits<__int64>::ParseFormat(); break;
          case VTK_UNSIGNED___INT64:
            FormatString = vtkTypeTraits<unsigned __int64>::ParseFormat(); break;
#endif

          case VTK_FLOAT:
            FormatString = vtkTypeTraits<float>::ParseFormat(); break;

          case VTK_DOUBLE:
            FormatString = vtkTypeTraits<double>::ParseFormat(); break;

        default:
          FormatString = "BUG - UNKNOWN DATA FORMAT"; break;
        }
      }
    else if (stringData)
      {
      FormatString = ""; // we'll use vtkStdString::operator+ instead of sprintf
      }
    else if (uStringData)
      {
      vtkWarningMacro( "Unicode string arrays are not adequately supported by the svkLabeledDataMapper.  Unicode strings will be converted to vtkStdStrings for rendering.");
      FormatString = "unicode"; // we'll use vtkStdString::operator+ instead of sprintf
      }
    else
      {
      FormatString = "BUG - COULDN'T DETECT DATA TYPE"; 
      }

    vtkDebugMacro(<<"Using default format string " << FormatString.c_str());

    } // Done building default format string
 
  int numCurLabels = input->GetNumberOfPoints(); 
  // We are assured that 
  // this->NumberOfLabelsAllocated >= (this->NumberOfLabels + numCurLabels)
  if (this->NumberOfLabelsAllocated < (this->NumberOfLabels + numCurLabels))
    {
    vtkErrorMacro(
      "Number of labels must be allocated before this method is called.");
    return;
    }
  
  // ----------------------------------------
  // Now we actually construct the label strings
  //

  const char *LiveFormatString = FormatString.c_str();
  char TempString[1024];

  vtkIntArray *typeArr = vtkIntArray::SafeDownCast(
    this->GetInputAbstractArrayToProcess(0, input));

  map<int, string> voxelTagDefinitions;
  if( input->IsA("svkMriImageData")) {
	  voxelTagDefinitions = svk::svkVoxelTaggingUtils::GetTagValueToNameMap( svk::svkMriImageData::SafeDownCast(input) );
  }
  for (i=0; i < numCurLabels; i++)
    {
    vtkStdString ResultString; 

    if ( pointIdLabels )
      {
      sprintf(TempString, LiveFormatString, i);
      ResultString = TempString;
      }
    else 
      {
      if ( numericData )
        {
        void *rawData = numericData->GetVoidPointer(i*numComp);
        
        if ( numComp == 1 )
          {
          if( this->DisplayTags)
            {
		    char* scalarName = pd->GetScalars()->GetName();
        	for( int j = 0; j < pd->GetNumberOfArrays(); j++)
        	  {
        	  pd->SetActiveScalars( pd->GetArray(j)->GetName());
			  rawData = pd->GetArray(j)->GetVoidPointer(i*numComp);
			  switch (numericData->GetDataType())
			    {
			      vtkTemplateMacro(svkLabeledDataMapper_PrintComponent(TempString,
			      	LiveFormatString, activeComp, static_cast<VTK_TT *>(rawData), this->DisplayTags, voxelTagDefinitions));
			    }
			  ResultString += TempString;
			  ResultString += '\n';
        	  }
        	pd->SetActiveScalars( scalarName );
			rawData = numericData->GetVoidPointer(i*numComp);
            }
          else
            {
            switch (numericData->GetDataType())
              {
              vtkTemplateMacro(svkLabeledDataMapper_PrintComponent(TempString,
                  LiveFormatString, activeComp, static_cast<VTK_TT *>(rawData)));
              }
            ResultString = TempString;
            }
          } 
        else // numComp != 1
          {
          ResultString = "(";
          
          // Print each component in turn and add it to the string.
          for (j = 0; j < numComp; ++j)
            {
            switch (numericData->GetDataType())
              {
              vtkTemplateMacro(
                svkLabeledDataMapper_PrintComponent(TempString,
                                                    LiveFormatString, 
                                                    j, 
                                                    static_cast<VTK_TT *>(rawData), this->DisplayTags, voxelTagDefinitions));
              }
            ResultString += TempString;
            
            if (j < (numComp-1))
              {
              ResultString += ' ';
              }
            else
              {
              ResultString += ')';
              }
            }
          }
        }
      else // rendering string data
        {
        // If the user hasn't given us a custom format string then
        // we'll sidestep a lot of sprintf nonsense.
        if (this->LabelFormat == NULL)
          {
          if( uStringData )
            {
            ResultString = uStringData->GetValue(i).utf8_str();
            }
          else
            {
            ResultString = stringData->GetValue(i);
            }
          }
        else // the user specified a label format
          {
          SNPRINTF(TempString, 1023, LiveFormatString, 
                   stringData->GetValue(i).c_str());
          ResultString = TempString;
          } // done printing strings with label format
        } // done printing strings
      } // done creating string 

    this->TextMappers[i+this->NumberOfLabels]->SetInput(ResultString.c_str());

    // Find the correct property type
    int type = 0;
    if (typeArr)
      {
      type = typeArr->GetValue(i);
      }
    vtkTextProperty* prop = this->Implementation->TextProperties[type];
    if (!prop)
      {
      prop = this->Implementation->TextProperties[0];
      }
    this->TextMappers[i+this->NumberOfLabels]->SetTextProperty(prop);

    double x[3];
    input->GetPoint(i, x);
    this->LabelPositions[3*(i+this->NumberOfLabels)] = x[0];
    this->LabelPositions[3*(i+this->NumberOfLabels)+1] = x[1];
    this->LabelPositions[3*(i+this->NumberOfLabels)+2] = x[2];
    }

  this->NumberOfLabels += numCurLabels;
}

//----------------------------------------------------------------------------
int svkLabeledDataMapper::FillInputPortInformation(
  int vtkNotUsed( port ), vtkInformation* info)
{
  // Can handle composite datasets.
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
  return 1;
}

//----------------------------------------------------------------------------
void svkLabeledDataMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if ( this->Input )
    {
    os << indent << "Input: (" << this->Input << ")\n";
    }
  else
    {
    os << indent << "Input: (none)\n";
    }

  map<int, vtkSmartPointer<vtkTextProperty> >::iterator it, itEnd;
  it = this->Implementation->TextProperties.begin();
  itEnd = this->Implementation->TextProperties.end();
  for (; it != itEnd; ++it)
    {
    vtkTextProperty* prop = it->second;
    if (prop)
      {
      os << indent << "LabelTextProperty " << it->first << ":\n";
      prop->PrintSelf(os, indent.GetNextIndent());
      }
    else
      {
      os << indent << "LabelTextProperty " << it->first << ": (none)\n";
      }
    }

  os << indent << "Label Mode: ";
  if ( this->LabelMode == VTK_LABEL_IDS )
    {
    os << "Label Ids\n";
    }
  else if ( this->LabelMode == VTK_LABEL_SCALARS )
    {
    os << "Label Scalars\n";
    }
  else if ( this->LabelMode == VTK_LABEL_VECTORS )
    {
    os << "Label Vectors\n";
    }
  else if ( this->LabelMode == VTK_LABEL_NORMALS )
    {
    os << "Label Normals\n";
    }
  else if ( this->LabelMode == VTK_LABEL_TCOORDS )
    {
    os << "Label TCoords\n";
    }
  else if ( this->LabelMode == VTK_LABEL_TENSORS )
    {
    os << "Label Tensors\n";
    }
  else
    {
    os << "Label Field Data\n";
    }

  os << indent << "Label Format: " << (this->LabelFormat ? this->LabelFormat : "Null") << "\n";

  os << indent << "Labeled Component: ";
  if ( this->LabeledComponent < 0 )
    {
    os << "(All Components)\n";
    }
  else
    {
    os << this->LabeledComponent << "\n";
    }

  os << indent << "Field Data Array: " << this->FieldDataArray << "\n";
  os << indent << "Field Data Name: " << (this->FieldDataName ? this->FieldDataName : "Null") << "\n";
  
  os << indent << "Transform: " << (this->Transform ? "" : "(none)") << endl;
  if (this->Transform)
    {
    this->Transform->PrintSelf(os,indent.GetNextIndent());
    }

  os << indent << "CoordinateSystem: " << this->CoordinateSystem << endl;
}

// ----------------------------------------------------------------------
void
svkLabeledDataMapper::SetFieldDataArray(int arrayIndex)
{
  if (this->FieldDataName)
    {
    delete [] this->FieldDataName;
    this->FieldDataName = NULL;
    }

  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting FieldDataArray to " << arrayIndex ); 

  if (this->FieldDataArray != (arrayIndex < 0 ? 0 : 
                               (arrayIndex > VTK_INT_MAX ? VTK_INT_MAX : arrayIndex)))
    {
    this->FieldDataArray = ( arrayIndex < 0 ? 0 : 
                             (arrayIndex > VTK_INT_MAX ? VTK_INT_MAX : arrayIndex ));
    this->Modified();
    }
}

// ----------------------------------------------------------------------
unsigned long
svkLabeledDataMapper::GetMTime()
{
  unsigned long mtime = this->Superclass::GetMTime();
  map<int, vtkSmartPointer<vtkTextProperty> >::iterator it, itEnd;
  it = this->Implementation->TextProperties.begin();
  itEnd = this->Implementation->TextProperties.end();
  for (; it != itEnd; ++it)
    {
    vtkTextProperty* p = it->second;
    unsigned long curMTime = p->GetMTime();
    if (curMTime > mtime)
      {
      mtime = curMTime;
      }
    }
  return mtime;
}

// ----------------------------------------------------------------------
void
svkLabeledDataMapper::SetFieldDataName(const char *arrayName)
{
  vtkDebugMacro(<< this->GetClassName() 
                << " (" << this << "): setting " << "FieldDataName" 
                << " to " << (arrayName?arrayName:"(null)") ); 

  if ( this->FieldDataName == NULL && arrayName == NULL) { return; } 
  if ( this->FieldDataName && arrayName && (!strcmp(this->FieldDataName,arrayName))) { return;} 
  if (this->FieldDataName) { delete [] this->FieldDataName; } 
  if (arrayName) 
    { 
    this->FieldDataName = new char[strlen(arrayName)+1]; 
    strcpy(this->FieldDataName,arrayName); 
    } 
   else 
    { 
    this->FieldDataName = NULL; 
    } 
  this->Modified(); 
}
