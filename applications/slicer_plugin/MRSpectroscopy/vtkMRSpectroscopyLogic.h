/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: https://intrarad.ucsf.edu/svn/rad_software/surbeck/brain/sivic/trunk/applications/slicer_plugin/MRSpectroscopy/vtkMRSpectroscopyLogic.h $
  Date:      $Date: 2009-08-10 09:58:47 -0700 (Mon, 10 Aug 2009) $
  Version:   $Revision: 14451 $

==========================================================================*/

// .NAME vtkMRSpectroscopyLogic - slicer logic class for Locator module 
// .SECTION Description
// This class manages the logic associated with tracking device for
// IGT. 


#ifndef __vtkMRSpectroscopyLogic_h
#define __vtkMRSpectroscopyLogic_h

#include "vtkMRSpectroscopyWin32Header.h"

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerApplication.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLSliceNode.h"

class vtkIGTLConnector;

class VTK_MRSpectroscopy_EXPORT vtkMRSpectroscopyLogic : public vtkSlicerModuleLogic 
{
 public:
  //BTX
  enum {  // Events
    //LocatorUpdateEvent      = 50000,
    StatusUpdateEvent       = 50001,
  };
  //ETX

 public:
  
  static vtkMRSpectroscopyLogic *New();
  
  vtkTypeRevisionMacro(vtkMRSpectroscopyLogic,vtkObject);
  void PrintSelf(ostream&, vtkIndent);

 protected:
  
  vtkMRSpectroscopyLogic();
  ~vtkMRSpectroscopyLogic();

  void operator=(const vtkMRSpectroscopyLogic&);
  vtkMRSpectroscopyLogic(const vtkMRSpectroscopyLogic&);

  static void DataCallback(vtkObject*, unsigned long, void *, void *);
  void UpdateAll();

  vtkCallbackCommand *DataCallbackCommand;

 private:


};

#endif


  
