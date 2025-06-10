#ifndef SVK_FACTORY_H_
#define SVK_FACTORY_H_

#include <vtkObjectFactory.h>

/**
 * Runtime factory for SVK-specific vtkDataObject subclasses.
 * Replaces the old vtkInstantiator mechanism removed in VTK ≥ 9.2.
 */
class svkFactory : public vtkObjectFactory
{
public:
    static svkFactory* New();
    vtkTypeMacro(svkFactory, vtkObjectFactory);

    const char* GetVTKSourceVersion() override;
    const char* GetDescription()      override;
    vtkObject*  CreateObject(const char* vtkclassname) override;

protected:
    svkFactory()  = default;
    ~svkFactory() override = default;

private:                        // non-copyable
    svkFactory(const svkFactory&)            = delete;
    void operator=(const svkFactory&)        = delete;
};

#endif  // SVK_FACTORY_H_
