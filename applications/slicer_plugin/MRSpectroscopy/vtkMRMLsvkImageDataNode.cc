#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLsvkImageDataNode.h"
#include "vtkMRMLScene.h"

#include "svkDataModel.h"


//------------------------------------------------------------------------------
vtkMRMLsvkImageDataNode* vtkMRMLsvkImageDataNode::New()
{
    // First try to create the object from the vtkObjectFactory
    vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLsvkImageDataNode");
    if(ret)
    {
        return (vtkMRMLsvkImageDataNode*)ret;
    }
    // If the factory was unable to create the object, then create it here.
    return new vtkMRMLsvkImageDataNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLsvkImageDataNode::CreateNodeInstance()
{
    // First try to create the object from the vtkObjectFactory
    vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLsvkImageDataNode");
    if(ret)
    {
        return (vtkMRMLsvkImageDataNode*)ret;
    }
    // If the factory was unable to create the object, then create it here.
    return new vtkMRMLsvkImageDataNode;
}


//----------------------------------------------------------------------------
vtkMRMLsvkImageDataNode::vtkMRMLsvkImageDataNode()
{
}


//----------------------------------------------------------------------------
vtkMRMLsvkImageDataNode::~vtkMRMLsvkImageDataNode()
{
    if (this->data) {
        this->data->Delete();
    }
}


//----------------------------------------------------------------------------
void vtkMRMLsvkImageDataNode::WriteXML(ostream& of, int nIndent)
{
    Superclass::WriteXML(of, nIndent);
    vtkIndent indent(nIndent);
    std::cout << "vtkMRMLsvkImageDataNode::WriteXML" << std::endl;
}


//----------------------------------------------------------------------------
void vtkMRMLsvkImageDataNode::ReadXMLAttributes(const char** atts)
{
   std::cout << "vtkMRMLsvkImageDataNode::ReadXMLAttributes" << std::endl;
   Superclass::ReadXMLAttributes(atts);
}


//----------------------------------------------------------------------------
void vtkMRMLsvkImageDataNode::Copy(vtkMRMLNode *anode)
{
    Superclass::Copy(anode);
    vtkMRMLsvkImageDataNode *node = (vtkMRMLsvkImageDataNode *) anode;
    this->data = node->data;
}


//----------------------------------------------------------------------------
void vtkMRMLsvkImageDataNode::PrintSelf(ostream& os, vtkIndent indent)
{
    Superclass::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
void vtkMRMLsvkImageDataNode::UpdateReferenceID(const char *oldID, const char *newID)
{
    Superclass::UpdateReferenceID(oldID,newID);
}


//-----------------------------------------------------------
void vtkMRMLsvkImageDataNode::UpdateReferences()
{
    Superclass::UpdateReferences();
}


//-----------------------------------------------------------
void vtkMRMLsvkImageDataNode::UpdateScene(vtkMRMLScene *scene)
{
    Superclass::UpdateScene(scene);
    //TODO(mangpo): display svkImageData
    std::cout << "UpdateScene..." <<std::endl;
}


//---------------------------------------------------------------------------
void vtkMRMLsvkImageDataNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
    Superclass::ProcessMRMLEvents(caller, event, callData);
}


//-----------------------------------------------------------
void vtkMRMLsvkImageDataNode::SetData(svkImageData* input)
{
    this->data = input;

    //TODO: do I have to include these
    //vtkMRMLStorageNode *snode = this->CreateDefaultStorageNode();
    //this->AddAndObserveStorageNode(snode);
}


//-----------------------------------------------------------
svkImageData* vtkMRMLsvkImageDataNode::GetData()
{
    return data;
}
