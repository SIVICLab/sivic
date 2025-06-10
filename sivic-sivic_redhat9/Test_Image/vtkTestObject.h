#ifndef vtkTestObject_H
#define vtkTestObject_H

#include <vtkObjectFactory.h>
#include <vtkDataObjectTypes.h> // Required for vtkDataObjectTypes
#include <cstring>              // Required for strcmp

// Forward declare the custom data object to avoid circular includes.
// The implementation file (vtkTestObject.cxx) will include the full header.
class MyCustomDataObject;

// It's good practice to define your custom type ID in a central header,
// perhaps alongside your custom data object's definition.
// For example, in MyCustomDataObject.h:
// #define VTK_MY_CUSTOM_DATA_OBJECT_TYPE_ID 1000 // Or some other unique ID

class vtkTestObject : public vtkObjectFactory
{
public:
  // The static New() method is the standard VTK entry point for instantiation.
  static vtkTestObject* New();

  // The vtkTypeMacro handles RTTI (Run-Time Type Information) for VTK.
  vtkTypeMacro(vtkTestObject, vtkObjectFactory);

  // --- Standard vtkObjectFactory Overrides ---

  // This method is called by VTK to create an instance of a class.
  // We override it to create our custom data object when requested.
  vtkObject* CreateObject(const char* vtkclassname) override;
  
  const char* GetVTKSourceVersion() override;
  const char* GetDescription() override;
  // These are standard virtual functions in vtkObjectFactory.
  // It's good practice to include and implement them.
 
  // --- Custom Static Methods ---

  // A static method to register the custom type with VTK's type system.
  // This typically only needs to be called once at application startup.
//   static void RegisterMyCustomType();

protected:
  vtkTestObject();
  ~vtkTestObject() override = default;
  

private:
  // Disable the copy constructor and assignment operator.
  vtkTestObject(const vtkTestObject&) = delete;
  void operator=(const vtkTestObject&) = delete;
};

#endif // vtkTestObject_H