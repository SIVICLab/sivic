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

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageRFFT.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include <svkImageLinearPhase.h>

using namespace svk;

//vtkCxxRevisionMacro(svkImageLinearPhase, "$Revision$");
vtkStandardNewMacro(svkImageLinearPhase);


/*!
 *
 */
svkImageLinearPhase::svkImageLinearPhase() 
{
    //this->SetNumberOfThreads(1);
    //  this->SetDimensionality(1);
    this->shiftWindow[0] = 0;
    this->shiftWindow[1] = 0;
    this->shiftWindow[2] = 0;
    this->phaseArray = NULL;
    this->pie = vtkMath::Pi();
}


/*!
 *
 */
svkImageLinearPhase::~svkImageLinearPhase()
{
}

// This extent of the components changes to real and imaginary values.
int svkImageLinearPhase::IterativeRequestInformation(vtkInformation* vtkNotUsed(input), vtkInformation* output)
{
    vtkDataObject::SetPointDataActiveScalarInfo(output, VTK_DOUBLE, 2);
    return 1;
}

/*!
 *  Set an additional phase shift for example to voxel shift data. 
 */
void svkImageLinearPhase::SetShiftWindow( double shiftWindow[3] )
{
    this->shiftWindow[0] = shiftWindow[0];
    this->shiftWindow[1] = shiftWindow[1];
    this->shiftWindow[2] = shiftWindow[2];
}


void vtkImageLinearPhaseInternalRequestUpdateExtent(int *inExt, int *outExt, 
                                             int *wExt,
                                             int iteration)
{
  memcpy(inExt, outExt, 6 * sizeof(int));
  inExt[iteration*2] = wExt[iteration*2];
  inExt[iteration*2 + 1] = wExt[iteration*2 + 1];  
}


//----------------------------------------------------------------------------
// This templated execute method handles any type input, but the output
// is always doubles.
template <class T>
void vtkImageLinearPhaseExecute(svkImageLinearPhase *self,
                         vtkImageData *inData, int inExt[6], T *inPtr,
                         vtkImageData *outData, int outExt[6], double *outPtr,
                         int id)
{
  vtkImageComplex *inComplex;
  vtkImageComplex *outComplex;
  vtkImageComplex *pComplex;
  //
  int inMin0, inMax0;
  vtkIdType inInc0, inInc1, inInc2;
  T *inPtr0, *inPtr1, *inPtr2;
  //
  int outMin0, outMax0, outMin1, outMax1, outMin2, outMax2;
  vtkIdType outInc0, outInc1, outInc2;
  double *outPtr0, *outPtr1, *outPtr2;
  //
  int idx0, idx1, idx2, inSize0, numberOfComponents;
  unsigned long count = 0;
  unsigned long target;
  double startProgress;

  startProgress = self->GetIteration()/
    static_cast<double>(self->GetNumberOfIterations());
  
  // Reorder axes (The outs here are just placeholdes
  self->PermuteExtent(inExt, inMin0, inMax0, outMin1,outMax1,outMin2,outMax2);
  self->PermuteExtent(outExt, outMin0,outMax0,outMin1,outMax1,outMin2,outMax2);
  self->PermuteIncrements(inData->GetIncrements(), inInc0, inInc1, inInc2);
  self->PermuteIncrements(outData->GetIncrements(), outInc0, outInc1, outInc2);

  
  inSize0 = inMax0 - inMin0 + 1;
  vtkImageComplex* phaseArray = new vtkImageComplex[inSize0];
  self->CreatePhaseArray( inSize0, phaseArray );
  
  // Input has to have real components at least.
  numberOfComponents = inData->GetNumberOfScalarComponents();
  if (numberOfComponents < 1)
    {
    vtkGenericWarningMacro("No real components");
    return;
    }

  // Allocate the arrays of complex numbers
  inComplex = new vtkImageComplex[inSize0];
  outComplex = new vtkImageComplex[inSize0];

  target = static_cast<unsigned long>((outMax2-outMin2+1)*(outMax1-outMin1+1)
                                      * self->GetNumberOfIterations() / 50.0);
  target++;

  // loop over other axes
  inPtr2 = inPtr;
  outPtr2 = outPtr;
  for (idx2 = outMin2; idx2 <= outMax2; ++idx2)
    {
    inPtr1 = inPtr2;
    outPtr1 = outPtr2;
    for (idx1 = outMin1; !self->AbortExecute && idx1 <= outMax1; ++idx1)
      {
      if (!id) 
        {
        if (!(count%target))
          {
          self->UpdateProgress(count/(50.0*target) + startProgress);
          }
        count++;
        }
      // copy into complex numbers
      inPtr0 = inPtr1;
      pComplex = inComplex;
      for (idx0 = inMin0; idx0 <= inMax0; ++idx0)
        {
        pComplex->Real = static_cast<double>(*inPtr0);
        pComplex->Imag = 0.0;
        if (numberOfComponents > 1)
          { // yes we have an imaginary input
          pComplex->Imag = static_cast<double>(inPtr0[1]);;
          }
        inPtr0 += inInc0;
        ++pComplex;
        }
     
      // Call the method that performs the Linear Phase
      self->ExecuteLinearPhase(inComplex, outComplex, inSize0, phaseArray);

      // copy into output
      outPtr0 = outPtr1;
      pComplex = outComplex + (outMin0 - inMin0);
      for (idx0 = outMin0; idx0 <= outMax0; ++idx0)
        {
        *outPtr0 = static_cast<double>(pComplex->Real);
        outPtr0[1] = static_cast<double>(pComplex->Imag);
        outPtr0 += outInc0;
        ++pComplex;
        }
      inPtr1 += inInc1;
      outPtr1 += outInc1;
      }
    inPtr2 += inInc2;
    outPtr2 += outInc2;
    }
    
  delete [] inComplex;
  delete [] outComplex;
  delete [] phaseArray;
}




//----------------------------------------------------------------------------
// This method is passed input and output Datas, and executes the Linear Phase
// algorithm to fill the output from the input.
// Not threaded yet.
void svkImageLinearPhase::ThreadedExecute(vtkImageData *inData, vtkImageData *outData,
                                  int outExt[6], int threadId)
{
  void *inPtr, *outPtr;
  int inExt[6];

  int* wExt = this->GetOutputInformation(0)->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT());

  vtkImageLinearPhaseInternalRequestUpdateExtent(inExt,outExt,wExt,this->Iteration);
  inPtr = inData->GetScalarPointerForExtent(inExt);
  outPtr = outData->GetScalarPointerForExtent(outExt);
  
  // this filter expects that the output be doubles.
  if (vtkImageData::GetScalarType( outData->GetInformation() ) != VTK_DOUBLE)
    {
    vtkErrorMacro(<< "Execute: Output must be be type double.");
    return;
    }

  // this filter expects input to have 1 or two components
  if (outData->GetNumberOfScalarComponents() != 1 && 
      outData->GetNumberOfScalarComponents() != 2)
    {
    vtkErrorMacro(<< "Execute: Cannot handle more than 2 components");
    return;
    }

  // choose which templated function to call.
  switch ( vtkImageData::GetScalarType( inData->GetInformation() ) )
    {
    vtkTemplateMacro(
      vtkImageLinearPhaseExecute(this, inData, inExt, 
                          static_cast<VTK_TT *>(inPtr), outData, outExt, 
                          static_cast<double *>(outPtr), threadId));
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}


/*!
 *  Applies a first order phase correction to the in data. The array of phase corrections is 
 *  calcuated by CreatePhaseArray() method. 
 */
void svkImageLinearPhase::ExecuteLinearPhase( vtkImageComplex* in, vtkImageComplex* out, int N, vtkImageComplex* phaseArray )
{
    for( int i=0; i < N; i++ ) {
        out[i].Real = ( phaseArray[i].Real*in[i].Real - phaseArray[i].Imag*in[i].Imag );
        out[i].Imag = ( phaseArray[i].Real*in[i].Imag + phaseArray[i].Imag*in[i].Real );
    }

}


/*!
 *  This method takes an array of N, vtkImageComplex values, where  vtkImageComplex  
 *  is a struct with a double real and double imaginary component representing 
 *  a single complex value.  The function applies a linear phase correction to the
 *  values with pivot given by the origin (middle index of array) and linear factor
 *  that increments by 2*pi/N.  An additional phase shift may be applied (e.g. 
 *  for voxel shifting origin. 
 *  On output the vtkImageComplex variable phaseArray contains the phase factors 
 *  to be applied to each point. 
 */
void svkImageLinearPhase::CreatePhaseArray(int N, vtkImageComplex* phaseArray) 
{ 
    svkSpecUtils::CreateLinearPhaseShiftArray(N, phaseArray, this->shiftWindow[this->Iteration]);
}
