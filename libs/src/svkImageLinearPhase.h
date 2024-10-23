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
 


#ifndef SVK_IMAGE_LINEAR_PHASE_H
#define SVK_IMAGE_LINEAR_PHASE_H

#include </usr/include/vtk/vtkImageData.h>
#include </usr/include/vtk/vtkMath.h>
#include </usr/include/vtk/vtkImageRFFT.h>
#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>
#include <svkSpecUtils.h>

#include <math.h>


#include "/usr/include/vtk/vtkImageFourierFilter.h"

namespace svk {

using namespace std;

/*!
 *  This class will apply a linear phase shift to a vtkImageData object.
 *
 *  NOTES:
 *      * This class will NOT change the origin of your dataset.
 *      * This class is a vtk Algroithm, not svk so it will not update
 *        the header. This is because it inherits from a vtk algorithm.
 *
 *  TODO:
 *      * Create an svkMriLinearPhase to wrap this algorithm so that
 *        it can produce an svk output dataset.
 */
class svkImageLinearPhase : public vtkImageFourierFilter
{
    public:
        static svkImageLinearPhase *New();
        vtkTypeMacro(svkImageLinearPhase,vtkImageFourierFilter);
        void SetShiftWindow( double shiftWindow[3] );
        void ExecuteLinearPhase( vtkImageComplex* in, vtkImageComplex* out, int N, vtkImageComplex* phaseArray );
        void CreatePhaseArray( int N, vtkImageComplex* phaseArray );
        
        // This extent of the components changes to real and imaginary values.
        int IterativeRequestInformation(vtkInformation* vtkNotUsed(input), vtkInformation* output);

    protected:
        svkImageLinearPhase();
        ~svkImageLinearPhase();

    private:

        void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,
                       int outExt[6], int threadId);
        double shiftWindow[3];
        vtkImageComplex* phaseArray;
        double pie;
};

} // svk

#endif //SVK_IMAGE_LINEAR_PHASE_H
