#include "svkFactory.h"

#include "svkMriImageData.h"
#include "svkMrsImageData.h"          // comment out if not needed
#include <cstring>                    // std::strcmp

vtkStandardNewMacro(svkFactory);

// ------------------------------------------------------------------
// vtkObjectFactory interface
// ------------------------------------------------------------------
const char* svkFactory::GetVTKSourceVersion()
{
    return VTK_SOURCE_VERSION;
}

const char* svkFactory::GetDescription()
{
    return "Factory for SVK data objects (svkMriImageData, svkMrsImageData …)";
}

vtkObject* svkFactory::CreateObject(const char* vtkclassname)
{
    if (!std::strcmp(vtkclassname, "svkMriImageData"))
        return svkMriImageData::New();

    if (!std::strcmp(vtkclassname, "svkMrsImageData"))
        return svkMrsImageData::New();

    return nullptr;   // let other factories try
}

// ------------------------------------------------------------------
// Automatic registration – executed before main() in every binary
// that links the object file containing this translation unit.
// ------------------------------------------------------------------
static vtkObjectFactory* _SVKFactoryRegistration = []() -> vtkObjectFactory*
{
    vtkObjectFactory* f = svkFactory::New();
    vtkObjectFactory::RegisterFactory(f);
    f->Delete();              // ownership transferred to VTK’s global list
    return f;
}();
