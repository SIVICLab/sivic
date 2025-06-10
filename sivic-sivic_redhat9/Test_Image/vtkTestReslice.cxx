// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#include "vtkTestReslice.h"

#include "vtkAlgorithmOutput.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include <cmath>

vtkStandardNewMacro(vtkTestReslice);

//------------------------------------------------------------------------------
// Constructor: Sets default filter to be identity.
vtkTestReslice::vtkTestReslice()
{
  this->MagnificationFactors[0] = 1.0;
  this->MagnificationFactors[1] = 1.0;
  this->MagnificationFactors[2] = 1.0;
  this->OutputSpacing[0] = 0.0; // not specified
  this->OutputSpacing[1] = 0.0; // not specified
  this->OutputSpacing[2] = 0.0; // not specified
  this->InterpolationMode = VTK_RESLICE_LINEAR;
  this->Dimensionality = 3;
}

//------------------------------------------------------------------------------
void vtkTestReslice::SetOutputSpacing(double sx, double sy, double sz)
{
  const double spacing[3] = { sx, sy, sz };
  bool modified = false;

  for (int axis = 0; axis < 3; axis++)
  {
    if (this->OutputSpacing[axis] != spacing[axis])
    {
      this->OutputSpacing[axis] = spacing[axis];
      if (spacing[axis] != 0.0)
      {
        // Delay computing the magnification factor.
        // Input might not be set yet.
        this->MagnificationFactors[axis] = 0.0; // Not computed yet.
      }
      modified = true;
    }
  }

  if (modified)
  {
    this->Modified();
  }
}

//------------------------------------------------------------------------------
void vtkTestReslice::SetAxisOutputSpacing(int axis, double s)
{
  if (axis < 0 || axis > 2)
  {
    vtkErrorMacro("Bad axis: " << axis);
    return;
  }

  double spacing[3];
  this->GetOutputSpacing(spacing);
  spacing[axis] = s;

  this->SetOutputSpacing(spacing);
}

//------------------------------------------------------------------------------
void vtkTestReslice::SetMagnificationFactors(double fx, double fy, double fz)
{
  const double factors[3] = { fx, fy, fz };
  bool modified = false;

  for (int axis = 0; axis < 3; axis++)
  {
    if (this->MagnificationFactors[axis] != factors[axis])
    {
      this->MagnificationFactors[axis] = factors[axis];
      // Spacing is no longer valid.
      this->OutputSpacing[axis] = 0.0; // Not computed yet.
      modified = true;
    }
  }

  if (modified)
  {
    this->Modified();
  }
}

//------------------------------------------------------------------------------
void vtkTestReslice::SetAxisMagnificationFactor(int axis, double factor)
{
  if (axis < 0 || axis > 2)
  {
    vtkErrorMacro("Bad axis: " << axis);
    return;
  }

  double factors[3];
  this->GetMagnificationFactors(factors);
  factors[axis] = factor;

  this->SetMagnificationFactors(factors);
}

//------------------------------------------------------------------------------
double vtkTestReslice::GetAxisMagnificationFactor(int axis, vtkInformation* inInfo)
{
  if (axis < 0 || axis > 2)
  {
    vtkErrorMacro("Bad axis: " << axis);
    return 0.0;
  }

  if (this->MagnificationFactors[axis] == 0.0)
  {
    double* inputSpacing;
    if (!this->GetInput())
    {
      vtkErrorMacro("GetMagnificationFactor: Input not set.");
      return 0.0;
    }
    this->GetInputConnection(0, 0)->GetProducer()->UpdateInformation();
    if (!inInfo)
    {
      inInfo = this->GetExecutive()->GetInputInformation(0, 0);
    }
    inputSpacing = inInfo->Get(vtkDataObject::SPACING());
    this->MagnificationFactors[axis] = inputSpacing[axis] / this->OutputSpacing[axis];
  }

  vtkDebugMacro(
    "Returning magnification factor " << this->MagnificationFactors[axis] << " for axis " << axis);

  return this->MagnificationFactors[axis];
}

//------------------------------------------------------------------------------
// Computes any global image information associated with regions.
int vtkTestReslice::RequestInformation(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  int wholeMin, wholeMax, axis, ext[6];
  double spacing[3], factor;

  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), ext);
  inInfo->Get(vtkDataObject::SPACING(), spacing);

  for (axis = 0; axis < 3; axis++)
  {
    wholeMin = ext[axis * 2];
    wholeMax = ext[axis * 2 + 1];

    // Scale the output extent
    factor = 1.0;
    if (axis < this->Dimensionality)
    {
      factor = this->GetAxisMagnificationFactor(axis, inInfo);
    }

    wholeMin = static_cast<int>(ceil(static_cast<double>(wholeMin) * factor));
    wholeMax = static_cast<int>(floor(static_cast<double>(wholeMax) * factor));

    // Change the data spacing
    spacing[axis] /= factor;

    ext[axis * 2] = wholeMin;
    ext[axis * 2 + 1] = wholeMax;

    // just in case the input spacing has changed.
    if (this->OutputSpacing[axis] != 0.0)
    {
      // Cause MagnificationFactor to recompute.
      this->MagnificationFactors[axis] = 0.0;
    }
  }

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), ext, 6);
  outInfo->Set(vtkDataObject::SPACING(), spacing, 3);

  return this->RequestInformationBase(inputVector, outputVector);
}

void vtkTestReslice::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "MagnificationFactors: " << this->MagnificationFactors[0] << " "
     << this->MagnificationFactors[1] << " " << this->MagnificationFactors[2] << "\n";
  os << indent << "Dimensionality: " << this->Dimensionality << "\n";
  os << indent << "Interpolate: " << (this->GetInterpolate() ? "On\n" : "Off\n");
}

int vtkTestReslice::FillOutputPortInformation(int vtkNotUsed(port), vtkInformation* info)
{
  // now add our info
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkTest");

  return 1;
}

//------------------------------------------------------------------------------
int vtkTestReslice::FillInputPortInformation(int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkTest");
  return 1;
}