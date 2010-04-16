/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/libs/src/svkMrsImageFFT.cc $
 *  $Rev: 259 $
 *  $Author: beckn8tor $
 *  $Date: 2010-04-12 16:51:07 -0700 (Mon, 12 Apr 2010) $
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageRFFT.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImageRFFT -  Reverse Fast Fourier Transform.
// .SECTION Description
// vtkImageRFFT implements the reverse fast Fourier transform.  The input
// can have real or complex data in any components and data types, but
// the output is always complex doubles with real values in component0, and
// imaginary values in component1.  The filter is fastest for images that
// have power of two sizes.  The filter uses a butterfly fitlers for each
// prime factor of the dimension.  This makes images with prime number dimensions 
// (i.e. 17x17) much slower to compute.  Multi dimensional (i.e volumes) 
// FFT's are decomposed so that each axis executes in series.
// In most cases the RFFT will produce an image whose imaginary values are all
// zero's. In this case vtkImageExtractComponents can be used to remove
// this imaginary components leaving only the real image.

// .SECTION See Also
// vtkImageExtractComponenents
 


#ifndef SVK_IMAGE_RFFT_H
#define SVK_IMAGE_RFFT_H

#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkImageRFFT.h>
#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <math.h>


#include "vtkImageFourierFilter.h"

namespace svk {

using namespace std;

class svkImageRFFT : public vtkImageRFFT
{
    public:
        static svkImageRFFT *New();
        vtkTypeRevisionMacro(svkImageRFFT,vtkImageRFFT);
        
        void SetPrePhaseShift( double prePhaseShift );
        void SetPostPhaseShift( double postPhaseShift );

        double prePhaseShift;
        double postPhaseShift;
        
        void ApplyPhaseShift( vtkImageComplex* data, int N, double shift );
        void CopyComplex( vtkImageComplex* in, vtkImageComplex* out, int N );
        bool phaseOnly;
  
  // Description:
  // For streaming and threads.  Splits output update extent into num pieces.
  // This method needs to be called num times.  Results must not overlap for
  // consistent starting extent.  Subclass can override this method.  This
  // method returns the number of pieces resulting from a successful split.
  // This can be from 1 to "total".  If 1 is returned, the extent cannot be
  // split.
/*
  int SplitExtent(int splitExt[6], int startExt[6], 
                  int num, int total);
*/

    protected:
        svkImageRFFT();
        ~svkImageRFFT();
/*
  virtual int IterativeRequestInformation(vtkInformation* in,
                                          vtkInformation* out);
  virtual int IterativeRequestUpdateExtent(vtkInformation* in,
                                           vtkInformation* out);
*/
    private:


        void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,
                       int outExt[6], int threadId);
        


};

} // svk

#endif //SVK_IMAGE_RFFT_H
