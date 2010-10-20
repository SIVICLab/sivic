#ifndef __vtkMRMLImageDataStorageNode_h
#define __vtkMRMLImageDataStorageNode_h

#include "vtkMRSpectroscopyWin32Header.h"

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"

#include "svkImageData.h"

using namespace svk; 

class vtkImageData;

class VTK_MRSpectroscopy_EXPORT vtkMRMLImageDataStorageNode : public vtkMRMLStorageNode
{
  public:
  static vtkMRMLImageDataStorageNode *New();
  vtkTypeMacro(vtkMRMLImageDataStorageNode, vtkMRMLStorageNode);
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
  virtual const char* GetNodeTagName() {return "ImageDataStorage";};
  
  /// 
  /// Read data and set it in the referenced node. 
  /// Return 1 on success, 0 on failure.
  virtual int ReadData(vtkMRMLNode *refNode);

  /// 
  /// Write data from a  referenced node
  /// Return 1 on success, 0 on failure.
  virtual int WriteData(vtkMRMLNode *refNode);

  /// 
  /// Check to see if this storage node can handle the file type in the input
  /// string. If input string is null, check URI, then check FileName. Returns
  /// 1 if is supported, 0 otherwise.
  virtual int SupportedFileType(const char *fileName);

  /// 
  /// Return a default file extension for writting
  virtual const char* GetDefaultWriteFileExtension()
    {
    return "ddf";
    };

  /// 
  /// Propagate Progress Event generated in ReadData
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  void SetInputFile(const char* fileName);
  svkImageData* GetData();

 
protected:
  vtkMRMLImageDataStorageNode();
  ~vtkMRMLImageDataStorageNode();
  vtkMRMLImageDataStorageNode(const vtkMRMLImageDataStorageNode&);
  void operator=(const vtkMRMLImageDataStorageNode&);

  char* id;
  vtkSetReferenceStringMacro(id);

  /// 
  /// Initialize all the supported write file types
  /// Subclasses should use this method to initialize SupportedWriteFileTypes.
  virtual void InitializeSupportedWriteFileTypes();

};

#endif

