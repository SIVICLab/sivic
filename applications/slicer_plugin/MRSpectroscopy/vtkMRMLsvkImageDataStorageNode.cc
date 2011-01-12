#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLsvkImageDataStorageNode.h"
#include "vtkMRMLsvkImageDataNode.h"
#include "vtkMRMLScene.h"

#include "svkDataModel.h"


//------------------------------------------------------------------------------
vtkMRMLsvkImageDataStorageNode* vtkMRMLsvkImageDataStorageNode::New()
{
    // First try to create the object from the vtkObjectFactory
    vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLsvkImageDataStorageNode");
    if(ret)
    {
        return (vtkMRMLsvkImageDataStorageNode*)ret;
    }
    // If the factory was unable to create the object, then create it here.
    return new vtkMRMLsvkImageDataStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLsvkImageDataStorageNode::CreateNodeInstance()
{
    // First try to create the object from the vtkObjectFactory
    vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLsvkImageDataStorageNode");
    if(ret)
    {
        return (vtkMRMLsvkImageDataStorageNode*)ret;
    }
    // If the factory was unable to create the object, then create it here.
    return new vtkMRMLsvkImageDataStorageNode;
}


//----------------------------------------------------------------------------
vtkMRMLsvkImageDataStorageNode::vtkMRMLsvkImageDataStorageNode()
{
}


//----------------------------------------------------------------------------
vtkMRMLsvkImageDataStorageNode::~vtkMRMLsvkImageDataStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLsvkImageDataStorageNode::WriteXML(ostream& of, int nIndent)
{
    Superclass::WriteXML(of, nIndent);
    vtkIndent indent(nIndent);
    std::cout << "vtkMRMLsvkImageDataStorageNode::WriteXML" << std::endl;
}

//----------------------------------------------------------------------------
void vtkMRMLsvkImageDataStorageNode::ReadXMLAttributes(const char** atts)
{  
    Superclass::ReadXMLAttributes(atts);
    std::cout << "vtkMRMLsvkImageDataStorageNode::ReadXMLAttributes" << std::endl;
}

//----------------------------------------------------------------------------
int vtkMRMLsvkImageDataStorageNode::ReadData(vtkMRMLNode *refNode)
{

    std::cout << "vtkMRMLsvkImageDataStorageNode::ReadData" << std::endl;
    // do not read if if we are not in the scene (for example inside snapshot)
    if ( !refNode->GetAddToScene() )
    {
        return 1;
    }

    // test whether refNode is a valid node
    if ( !( refNode->IsA("vtkMRMLsvkImageDataNode"))) 
    {
        vtkErrorMacro("Reference node is not a proper vtkMRMLsvkImageDataNode");
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
        vtkErrorMacro("vtkMRMLsvkImageDataStorageNode: File name not specified");
        return 0;
    }

    // cast the input node
    vtkMRMLsvkImageDataNode *dataNode = NULL;
    if ( refNode->IsA("vtkMRMLsvkImageDataNode") )
    {
        dataNode = dynamic_cast <vtkMRMLsvkImageDataNode *> (refNode);
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
int vtkMRMLsvkImageDataStorageNode::WriteData(vtkMRMLNode *refNode)
{
    std::cout << "vtkMRMLsvkImageDataStorageNode::WriteData" << std::endl;

    // test whether refNode is a valid node to hold a volume
    if ( !( refNode->IsA("vtkMRMLsvkImageDataNode") ) )
    {
        vtkErrorMacro("Reference node is not a proper vtkMRMLsvkImageDataNode");
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
        vtkErrorMacro("vtkMRMLsvkImageDataStorageNode: File name not specified");
        return 0;
    }

    // cast the input node
    vtkMRMLsvkImageDataNode *dataNode = NULL;
    if ( refNode->IsA("vtkMRMLsvkImageDataNode") )
    {
        dataNode = dynamic_cast <vtkMRMLsvkImageDataNode *> (refNode);
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

void vtkMRMLsvkImageDataStorageNode::Copy(vtkMRMLNode *anode)
{
    Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLsvkImageDataStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
    Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLsvkImageDataStorageNode::UpdateReferenceID(const char *oldID, const char *newID)
{
    Superclass::UpdateReferenceID(oldID,newID);
}

//----------------------------------------------------------------------------
void vtkMRMLsvkImageDataStorageNode::ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
    Superclass::ProcessMRMLEvents(caller, event, callData);
    //TODO(mangpo): implement
}

void vtkMRMLsvkImageDataStorageNode::SetInputFile(const char* fileName) {
    cout << "vtkMRMLsvkImageDataStorageNode:: SetInputFile  NEEDS DEFINITION " << endl;
    cout << "vtkMRMLsvkImageDataStorageNode:: SetInputFile  NEEDS DEFINITION " << endl;
    cout << "vtkMRMLsvkImageDataStorageNode:: SetInputFile  NEEDS DEFINITION " << endl;
    cout << "vtkMRMLsvkImageDataStorageNode:: SetInputFile  NEEDS DEFINITION " << endl;
    cout << "vtkMRMLsvkImageDataStorageNode:: SetInputFile  NEEDS DEFINITION " << endl;
    cout << "vtkMRMLsvkImageDataStorageNode:: SetInputFile  NEEDS DEFINITION " << endl;
    // needs definition
}


//----------------------------------------------------------------------------
void vtkMRMLsvkImageDataStorageNode::InitializeSupportedWriteFileTypes()
{
    this->SupportedWriteFileTypes->InsertNextValue("DDF (.ddf)");
}

//----------------------------------------------------------------------------
int vtkMRMLsvkImageDataStorageNode::SupportedFileType(const char *fileName)
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
