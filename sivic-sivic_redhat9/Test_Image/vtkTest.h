// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
/**
 * @class   vtkTest
 * @brief   A subclass of ImageData.
 *
 * Test is a subclass of ImageData that requires the data extent
 * to exactly match the update extent. Normal image data allows that the
 * data extent may be larger than the update extent.
 * Test
 also defines the origin differently that vtkImageData.
 * For structured points the origin is the location of first point.
 * Whereas images define the origin as the location of point 0, 0, 0.
 * Image Origin is stored in ivar, and structured points
 * have special methods for setting/getting the origin/extents.
 */

#ifndef vtkTest_h
#define vtkTest_h

#include "vtkCommonDataModelModule.h" // For export macro
#include "vtkImageData.h"
#include "vtkWrappingHints.h" // For VTK_MARSHALAUTO


//VTK_ABI_NAMESPACE_BEGIN
class vtkTest : public vtkImageData
{
public:
  //vtkStandardNewMacro(vtkTest);
  static vtkTest* New();
  vtkTypeMacro(vtkTest, vtkImageData);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * To simplify filter superclasses,
   */


protected:
  vtkTest();
  //~vtkTest() override = default;

private:
  vtkTest(const vtkTest&) = delete;
  void operator=(const vtkTest&) = delete;
};

//VTK_ABI_NAMESPACE_END
#endif