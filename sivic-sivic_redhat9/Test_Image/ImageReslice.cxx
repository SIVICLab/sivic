#include <vtkImageData.h>
#include <vtkImageReslice.h>
#include <vtkNew.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTest.h>
#include <vtkTestReslice.h>
#include <vtkObjectFactory.h>
#include <vtkTestObject.h>

int main(int, char*[])
{
  vtkObjectFactory::RegisterFactory(vtkTestObject::New());

  vtkNew<vtkTest> image;
  image->SetExtent(0, 9, 0, 9, 0, 0);
  image->AllocateScalars(VTK_INT, 1);

  
  std::cout << "image: " << *image << std::endl;

  vtkNew<vtkTestReslice> reslice;
  reslice->SetOutputExtent(0, 9, 0, 100, 0, 0);

  reslice->SetInputData(image);
  reslice->Update();

  std::cout << "reslice: " << *reslice->GetOutput() << std::endl;  


  return EXIT_SUCCESS;
}
