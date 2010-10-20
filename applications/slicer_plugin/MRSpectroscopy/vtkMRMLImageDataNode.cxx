#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLImageDataNode.h"
#include "vtkMRMLScene.h"

#include "svkDataModel.h"


//------------------------------------------------------------------------------
vtkMRMLImageDataNode* vtkMRMLImageDataNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLImageDataNode");
  if(ret)
    {
      return (vtkMRMLImageDataNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLImageDataNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLImageDataNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLImageDataNode");
  if(ret)
    {
      return (vtkMRMLImageDataNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLImageDataNode;
}

//----------------------------------------------------------------------------
vtkMRMLImageDataNode::vtkMRMLImageDataNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLImageDataNode::~vtkMRMLImageDataNode()
{
  if (this->data) {
    this->data->Delete();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLImageDataNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);
  std::cout << "vtkMRMLImageDataNode::WriteXML" << std::endl;
}

//----------------------------------------------------------------------------
void vtkMRMLImageDataNode::ReadXMLAttributes(const char** atts)
{
   std::cout << "vtkMRMLImageDataNode::ReadXMLAttributes" << std::endl;
   Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------

void vtkMRMLImageDataNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLImageDataNode *node = (vtkMRMLImageDataNode *) anode;
  this->data = node->data;
}

//----------------------------------------------------------------------------
void vtkMRMLImageDataNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLImageDataNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID,newID);
}

//-----------------------------------------------------------
void vtkMRMLImageDataNode::UpdateReferences()
{
  Superclass::UpdateReferences();
}

//-----------------------------------------------------------
void vtkMRMLImageDataNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
  //TODO(mangpo): display svkImageData
  std::cout << "UpdateScene..." <<std::endl;
}


//---------------------------------------------------------------------------
void vtkMRMLImageDataNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//-----------------------------------------------------------
void vtkMRMLImageDataNode::SetData(svkImageData* input)
{
  this->data = input;

  //TODO: do I have to include these
  //vtkMRMLStorageNode *snode = this->CreateDefaultStorageNode();
  //this->AddAndObserveStorageNode(snode);
}

//-----------------------------------------------------------
svkImageData* vtkMRMLImageDataNode::GetData()
{
  return data;
}