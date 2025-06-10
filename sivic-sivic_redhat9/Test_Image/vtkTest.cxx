// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#include "vtkTest.h"
#include "vtkObjectFactory.h"

#include <cmath>


//vtkStandardNewMacro(vtkTest);

vtkObjectFactoryNewMacro(vtkTest);

vtkTest::vtkTest(){};

void vtkTest::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  std::cout << "test2" << std::endl;
}
