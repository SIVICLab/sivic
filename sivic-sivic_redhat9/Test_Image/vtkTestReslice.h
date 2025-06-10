// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
/**
 * @class   vtkTestReslice
 * @brief   Resamples an image to be larger or smaller.
 *
 * This filter produces an output with different spacing (and extent)
 * than the input.  Linear interpolation can be used to resample the data.
 * The Output spacing can be set explicitly or relative to input spacing
 * with the SetAxisMagnificationFactor method.
 */

#ifndef vtkTestReslice_h
#define vtkTestReslice_h

#include "vtkImageReslice.h"
#include "vtkImagingCoreModule.h" // For export macro

class  vtkTestReslice : public vtkImageReslice
{
public:
  static vtkTestReslice* New();
  vtkTypeMacro(vtkTestReslice, vtkImageReslice);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///@{
  /**
   * Set desired spacing.
   * Zero is a reserved value indicating spacing has not been set.
   */
  void SetOutputSpacing(double sx, double sy, double sz) override;
  void SetOutputSpacing(const double spacing[3]) override
  {
    this->SetOutputSpacing(spacing[0], spacing[1], spacing[2]);
  }
  void SetAxisOutputSpacing(int axis, double spacing);
  ///@}

  ///@{
  /**
   * Set/Get Magnification factors.
   * Zero is a reserved value indicating values have not been computed.
   */
  void SetMagnificationFactors(double fx, double fy, double fz);
  void SetMagnificationFactors(const double f[3])
  {
    this->SetMagnificationFactors(f[0], f[1], f[2]);
  }
  vtkGetVector3Macro(MagnificationFactors, double);
  void SetAxisMagnificationFactor(int axis, double factor);
  ///@}

  /**
   * Get the computed magnification factor for a specific axis.
   * The input information is required to compute the value.
   */
  double GetAxisMagnificationFactor(int axis, vtkInformation* inInfo = nullptr);

  ///@{
  /**
   * Dimensionality is the number of axes which are considered during
   * execution. To process images dimensionality would be set to 2.
   * This has the same effect as setting the magnification of the third
   * axis to 1.0
   */
  vtkSetMacro(Dimensionality, int);
  vtkGetMacro(Dimensionality, int);
  ///@}

protected:
  vtkTestReslice();
  ~vtkTestReslice() override = default;

  double MagnificationFactors[3];
  int Dimensionality;

  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  int FillOutputPortInformation(int port, vtkInformation* info) override;
  int FillInputPortInformation(int port, vtkInformation* info) override;  

private:
  vtkTestReslice(const vtkTestReslice&) = delete;
  void operator=(const vtkTestReslice&) = delete;
};

#endif