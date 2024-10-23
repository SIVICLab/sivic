/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkExtractVOI.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*
 *  Copyright © 2009-2017 The Regents of the University of California.
 *  All Rights Reserved.
 *
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions are met:
 *  •   Redistributions of source code must retain the above copyright notice, 
 *      this list of conditions and the following disclaimer.
 *  •   Redistributions in binary form must reproduce the above copyright notice, 
 *      this list of conditions and the following disclaimer in the documentation 
 *      and/or other materials provided with the distribution.
 *  •   None of the names of any campus of the University of California, the name 
 *      "The Regents of the University of California," or the names of any of its 
 *      contributors may be used to endorse or promote products derived from this 
 *      software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 *  OF SUCH DAMAGE.
 */

/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */

#include "svkExtractVOI.h"

#include "/usr/include/vtk/vtkCellData.h"
#include "/usr/include/vtk/vtkImageData.h"
#include "/usr/include/vtk/vtkInformation.h"
#include "/usr/include/vtk/vtkInformationVector.h"
#include "/usr/include/vtk/vtkObjectFactory.h"
#include "/usr/include/vtk/vtkStreamingDemandDrivenPipeline.h"
#include "/usr/include/vtk/vtkPointData.h"
using namespace svk;

//vtkCxxRevisionMacro(svkExtractVOI, "$Rev$");
vtkStandardNewMacro(svkExtractVOI);

// Construct object to extract all of the input data.
svkExtractVOI::svkExtractVOI()
{
}

svkExtractVOI::~svkExtractVOI()
{
}

int svkExtractVOI::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  int i, outDims[3], voi[6], wholeExtent[6];
  int mins[3];
  int rate[3];

  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), wholeExtent);

  double spacing[3], outSpacing[3];
  inInfo->Get(vtkDataObject::SPACING(), spacing);

  double origin[3], outOrigin[3];
  inInfo->Get(vtkDataObject::ORIGIN(), origin );

  double dcos[3][3] = {{1,0,0},{0,1,0},{0,0,1}};

  if( svkImageData::SafeDownCast(this->GetInput()) != NULL ) {
      svkImageData::SafeDownCast(this->GetInput())->GetDcos(dcos);
      svkImageData::SafeDownCast(this->GetOutputDataObject(0))->SetDcos(dcos);
  }

  // Copy because we need to take union of voi and whole extent.
  for ( i=0; i < 6; i++ )
    {
    voi[i] = this->VOI[i];
    }

  for ( i=0; i < 3; i++ )
    {
    // Empty request.
    if (voi[2*i+1] < voi[2*i] || voi[2*i+1] < wholeExtent[2*i] || 
        voi[2*i] > wholeExtent[2*i+1])
      {
      outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
                   0,-1,0,-1,0,-1);
      return 1;
      }

    // Make sure VOI is in the whole extent.
    if ( voi[2*i+1] > wholeExtent[2*i+1] )
      {
      voi[2*i+1] = wholeExtent[2*i+1];
      }
    else if ( voi[2*i+1] < wholeExtent[2*i] )
      {
      voi[2*i+1] = wholeExtent[2*i];
      }
    if ( voi[2*i] > wholeExtent[2*i+1] )
      {
      voi[2*i] = wholeExtent[2*i+1];
      }
    else if ( voi[2*i] < wholeExtent[2*i] )
      {
      voi[2*i] = wholeExtent[2*i];
      }

    if ( (rate[i] = this->SampleRate[i]) < 1 )
      {
      rate[i] = 1;
      }

    outDims[i] = (voi[2*i+1] - voi[2*i]) / rate[i] + 1;
    if ( outDims[i] < 1 )
      {
      outDims[i] = 1;
      }
    // We might as well make this work for negative extents.
    mins[i] = static_cast<int>(floor(static_cast<float>(voi[2*i])
                                     / static_cast<float>(rate[i])));
    
    outSpacing[i] = spacing[i] * rate[i];
    outOrigin[i] = origin[i];
    for( int j=0; j<3; j++) {
        outOrigin[i] += voi[2*i]*spacing[i]*dcos[j][i] - mins[i]*outSpacing[i]*dcos[j][i];
    }

    }

  // Set the whole extent of the output
  wholeExtent[0] = mins[0];
  wholeExtent[1] = mins[0] + outDims[0] - 1;
  wholeExtent[2] = mins[1];
  wholeExtent[3] = mins[1] + outDims[1] - 1;
  wholeExtent[4] = mins[2];
  wholeExtent[5] = mins[2] + outDims[2] - 1;

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               wholeExtent, 6);
  outInfo->Set(vtkDataObject::SPACING(), outSpacing, 3);
  outInfo->Set(vtkDataObject::ORIGIN(), outOrigin, 3);


  return 1;
}


/*!
 *  Default output type is same concrete sub class type as the input data.  Override with
 *  specific concrete type in sub-class if necessary.
 */
int svkExtractVOI::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData" );
    return 1;
}
