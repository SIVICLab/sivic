#include <vtkObjectFactory.h>
#include <vtkTestObject.h>
#include <vtkDataObjectTypes.h>
#include <vtkTest.h>
#include <vtkSmartPointer.h>


vtkStandardNewMacro(vtkTestObject)
vtkTestObject :: vtkTestObject(){}

  // Implement the CreateObject function
  vtkObject* vtkTestObject::CreateObject(const char* vtkclassname)
  {
   
    //std::cout << "vtktestobject: " << vtkclassname << std::endl;
    if (strcmp(vtkclassname, "vtkTest") == 0)
    {
      vtkSmartPointer<vtkTest> myvtktest = vtkSmartPointer<vtkTest>::New();
      std::cout << "vtktest: " << myvtktest << std::endl;
      return myvtktest;
      
    }
    return nullptr; // Indicate that this factory does not handle the class
  }

//   // Register your custom data type (if needed)
//   static void RegisterMyCustomType()
//   {
//       vtkDataObjectTypes::RegisterType("vtkTest", 645780); // Assign a unique type ID
//   }
  const char* vtkTestObject::GetVTKSourceVersion()
{
  return "hey";
}

//=========================================================================
// This method returns a human-readable description of the factory.
const char* vtkTestObject::GetDescription()
{
  // You should customize this string to describe what your factory does.
  return "A factory for creating custom project objects";
}