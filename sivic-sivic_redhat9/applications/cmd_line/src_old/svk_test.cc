#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <map>
#include <string>
#include <functional>

#include "svkMriImageData.h"      //  ⟵ your class

// -----------------------------------------------------------------------------
// Simple run‑time factory
// -----------------------------------------------------------------------------
using Factory = std::map<std::string, std::function<vtkObject*()>>;
static Factory classRegistry;

void RegisterClass(const std::string& key,
                   std::function<vtkObject*()> ctor)
{
    classRegistry[key] = std::move(ctor);
}

vtkObject* CreateInstance(const std::string& key)
{
    auto it = classRegistry.find(key);
    return (it != classRegistry.end()) ? it->second() : nullptr;
}

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------
int main()
{
    // Register svkMriImageData
    RegisterClass("svkMriImageData",
                  []() -> vtkObject* {     // λ‑expression
                      return svk::svkMriImageData::New();
                  });

    // Create one:
    vtkObject* raw = CreateInstance("svkMriImageData");
    raw -> Print(cout);
    raw -> PrintSelf(cout,vtkIndent(2));
    auto* mri = svk::svkMriImageData::SafeDownCast(raw);

    // ... use *mri here ...

    if (mri) {
        mri->Delete();   // or better: vtkSmartPointer
    }
    return 0;
}

