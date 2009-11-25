/*==========================================================================

Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $HeadURL: https://intrarad.ucsf.edu/svn/rad_software/surbeck/brain/sivic/trunk/applications/slicer_plugin/MRSpectroscopy/vtkMRSpectroscopyWin32Header.h $
Date:      $Date: 2009-08-10 09:58:47 -0700 (Mon, 10 Aug 2009) $
Version:   $Revision: 14451 $

==========================================================================*/

#ifndef __vtkMRSpectroscopyWin32Header_h
#define __vtkMRSpectroscopyWin32Header_h

#include <vtkMRSpectroscopyConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(MRSpectroscopy_EXPORTS)
#define VTK_MRSpectroscopy_EXPORT __declspec( dllexport ) 
#else
#define VTK_MRSpectroscopy_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_MRSpectroscopy_EXPORT 
#endif
#endif
