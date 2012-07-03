#ifndef __vtkMRMLsvkImageDataNode_h
#define __vtkMRMLsvkImageDataNode_h

#include "vtkMRSpectroscopyWin32Header.h"

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorableNode.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"

#include "svkImageData.h"
#include "vtkMRMLsvkImageDataStorageNode.h"

using namespace svk; 

class vtkImageData;

class VTK_MRSpectroscopy_EXPORT vtkMRMLsvkImageDataNode : public vtkMRMLStorableNode
{

  public:
    static vtkMRMLsvkImageDataNode *New();
    vtkTypeMacro(vtkMRMLsvkImageDataNode, vtkMRMLStorableNode);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Create instance of a GAD node.
    virtual vtkMRMLNode* CreateNodeInstance();

    // Description:
    // Set node attributes from name/value pairs
    virtual void ReadXMLAttributes( const char** atts);

    // Description:
    // Write this node's information to a MRML file in XML format.
    virtual void WriteXML(ostream& of, int indent);

    // Description:
    // Copy the node's attributes to this object
    virtual void Copy(vtkMRMLNode *node);

    // Description:
    // Get unique node XML tag name (like Volume, Model)
    virtual const char* GetNodeTagName() {return "ImageData";};

    /// 
    /// Updates this node if it depends on other nodes 
    /// when the node is deleted in the scene
    virtual void UpdateReferences();

    /// 
    /// Finds the storage node and read the data
    virtual void UpdateScene(vtkMRMLScene *scene);

    /// 
    /// alternative method to propagate events generated in Storage nodes
    virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
    // Description:
    // Update the stored reference to another node in the scene
    virtual void UpdateReferenceID(const char *oldID, const char *newID);

    /// 
    /// Create default storage node or NULL if does not have one
    virtual vtkMRMLStorageNode* CreateDefaultStorageNode()
    {
        return vtkMRMLsvkImageDataStorageNode::New();
    };

    virtual bool CanApplyNonLinearTransforms() { return false; }
    virtual void ApplyTransform(vtkAbstractTransform* vtkNotUsed(transform)) { return; };
    virtual void ApplyTransform(vtkMatrix4x4* transformMatrix)
    { Superclass::ApplyTransform(transformMatrix); }

    void SetData(svkImageData* data);
    svkImageData* GetData();

 
    protected:
    vtkMRMLsvkImageDataNode();
    ~vtkMRMLsvkImageDataNode();
    vtkMRMLsvkImageDataNode(const vtkMRMLsvkImageDataNode&);
    void operator=(const vtkMRMLsvkImageDataNode&);

    char *id;
    vtkSetReferenceStringMacro(id);
    svkImageData* data;

};

#endif

