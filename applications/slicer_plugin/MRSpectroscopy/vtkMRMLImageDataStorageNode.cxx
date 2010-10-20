#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLImageDataStorageNode.h"
#include "vtkMRMLImageDataNode.h"
#include "vtkMRMLScene.h"

#include "svkDataModel.h"


//------------------------------------------------------------------------------
vtkMRMLImageDataStorageNode* vtkMRMLImageDataStorageNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLImageDataStorageNode");
  if(ret)
    {
      return (vtkMRMLImageDataStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLImageDataStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLImageDataStorageNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLImageDataStorageNode");
  if(ret)
    {
      return (vtkMRMLImageDataStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLImageDataStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLImageDataStorageNode::vtkMRMLImageDataStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLImageDataStorageNode::~vtkMRMLImageDataStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLImageDataStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);
  std::cout << "vtkMRMLImageDataStorageNode::WriteXML" << std::endl;
}

//----------------------------------------------------------------------------
void vtkMRMLImageDataStorageNode::ReadXMLAttributes(const char** atts)
{  
   Superclass::ReadXMLAttributes(atts);
   std::cout << "vtkMRMLImageDataStorageNode::ReadXMLAttributes" << std::endl;
}

//----------------------------------------------------------------------------
int vtkMRMLImageDataStorageNode::ReadData(vtkMRMLNode *refNode)
{
  std::cout << "vtkMRMLImageDataStorageNode::ReadData" << std::endl;
  // do not read if if we are not in the scene (for example inside snapshot)
  if ( !refNode->GetAddToScene() )
  {
    return 1;
  }

  // test whether refNode is a valid node
  if ( !( refNode->IsA("vtkMRMLImageDataNode"))) 
  {
    vtkErrorMacro("Reference node is not a proper vtkMRMLImageDataNode");
    return 0;         
  }

  if (this->GetFileName() == NULL && this->GetURI() == NULL) 
  {
    vtkErrorMacro("ReadData: file name and uri not set");
    return 0;
  }

  Superclass::StageReadData(refNode);
  if ( this->GetReadState() != this->TransferDone )
  {
    // remote file download hasn't finished
    vtkWarningMacro("ReadData: Read state is pending, returning.");
    return 0;
  }
  
  std::string fullName = this->GetFullNameFromFileName(); 

  if (fullName == std::string("")) 
  {
    vtkErrorMacro("vtkMRMLImageDataStorageNode: File name not specified");
    return 0;
  }

  // cast the input node
  vtkMRMLImageDataNode *dataNode = NULL;
  if ( refNode->IsA("vtkMRMLImageDataNode") )
  {
    dataNode = dynamic_cast <vtkMRMLImageDataNode *> (refNode);
  }

  if (dataNode == NULL)
  {
    vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID() << " to a known data node");
    return 0;
  }

  // open the file for reading input
  //TODO: model->LoadFile generates exception: basic_ios::clear
  svkDataModel* model = svkDataModel::New();
  std::cout << "reading... " << this->GetFullNameFromFileName() << std::endl;
  svkImageData* ddfData = model->LoadFile(fullName.c_str());
  std::cout << "read from " << this->GetFullNameFromFileName() << std::endl;
  dataNode->SetData(ddfData);
  
  this->SetReadStateIdle();
  
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLImageDataStorageNode::WriteData(vtkMRMLNode *refNode)
{
  std::cout << "vtkMRMLImageDataStorageNode::WriteData" << std::endl;

  // test whether refNode is a valid node to hold a volume
  if ( !( refNode->IsA("vtkMRMLImageDataNode") ) )
    {
    vtkErrorMacro("Reference node is not a proper vtkMRMLImageDataNode");
    return 0;         
    }

  if (this->GetFileName() == NULL) 
    {
    vtkErrorMacro("ReadData: file name is not set");
    return 0;
    }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("vtkMRMLImageDataStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkMRMLImageDataNode *dataNode = NULL;
  if ( refNode->IsA("vtkMRMLImageDataNode") )
    {
    dataNode = dynamic_cast <vtkMRMLImageDataNode *> (refNode);
    }

  if (dataNode == NULL)
    {
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known data node");
    return 0;
    }

  //TODO: WriteToFile has not yet implemented.
  std::cout << "writing... " << this->GetFullNameFromFileName() << std::endl;
  svkDataModel* model = svkDataModel::New();
  model->WriteToFile(dataNode->GetData(), fullName.c_str());
  std::cout << "write to " << this->GetFullNameFromFileName() << std::endl;
  return 0;
}

//----------------------------------------------------------------------------

void vtkMRMLImageDataStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLImageDataStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLImageDataStorageNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID,newID);
}

//----------------------------------------------------------------------------
void vtkMRMLImageDataStorageNode::ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  //TODO(mangpo): implement
}

//----------------------------------------------------------------------------
void vtkMRMLImageDataStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("DDF (.ddf)");
}

//----------------------------------------------------------------------------
int vtkMRMLImageDataStorageNode::SupportedFileType(const char *fileName)
{
  // check to see which file name we need to check
  std::string name;
  if (fileName)
    {
    name = std::string(fileName);
    }
  else if (this->FileName != NULL)
    {
    name = std::string(this->FileName);
    }
  else if (this->URI != NULL)
    {
    name = std::string(this->URI);
    }
  else
    {
    vtkWarningMacro("SupportedFileType: no file name to check");
    return 0;
    }
  
  std::string::size_type loc = name.find_last_of(".");
  if( loc == std::string::npos ) 
    {
    vtkErrorMacro("SupportedFileType: no file extension specified");
    return 0;
    }
  std::string extension = name.substr(loc);

  vtkWarningMacro("SupportedFileType: extension = " << extension.c_str());
  if (extension.compare(".ddf") == 0)
    {
    return 1;
    }
  else
    {
    vtkWarningMacro("SupportedFileType: can't read files with extension " << extension.c_str());
    return 0;
    }
}