/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: https://intrarad.ucsf.edu/svn/rad_software/surbeck/brain/sivic/trunk/applications/slicer_plugin/MRSpectroscopy/vtkMRSpectroscopyLogic.cxx $
  Date:      $Date: 2009-08-10 09:58:47 -0700 (Mon, 10 Aug 2009) $
  Version:   $Revision: 14451 $

==========================================================================*/


#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"

#include "vtkMRSpectroscopyLogic.h"

vtkCxxRevisionMacro(vtkMRSpectroscopyLogic, "$Revision: 14451 $");
vtkStandardNewMacro(vtkMRSpectroscopyLogic);

//---------------------------------------------------------------------------
vtkMRSpectroscopyLogic::vtkMRSpectroscopyLogic()
{

  // Timer Handling

  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkMRSpectroscopyLogic::DataCallback);

}


//---------------------------------------------------------------------------
vtkMRSpectroscopyLogic::~vtkMRSpectroscopyLogic()
{

  if (this->DataCallbackCommand)
    {
    this->DataCallbackCommand->Delete();
    }

}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkMRSpectroscopyLogic:             " << this->GetClassName() << "\n";

}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyLogic::DataCallback(vtkObject *caller, 
                                       unsigned long eid, void *clientData, void *callData)
{
  vtkMRSpectroscopyLogic *self = reinterpret_cast<vtkMRSpectroscopyLogic *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkMRSpectroscopyLogic DataCallback");
  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkMRSpectroscopyLogic::UpdateAll()
{

}







