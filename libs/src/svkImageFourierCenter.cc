
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
  Module:    $RCSfile: vtkImageFourierCenter.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include <svkImageFourierCenter.h>

#include </usr/include/vtk/vtkImageData.h>
#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>

#include <math.h>

using namespace svk;

//vtkCxxRevisionMacro(svkImageFourierCenter, "$Revision$");
vtkStandardNewMacro(svkImageFourierCenter);


/*!
 *  Construct an instance of svkImageFourierCenter fitler.
 */
svkImageFourierCenter::svkImageFourierCenter()
{
    this->reverseCenter = false;
}


/*!
 *  Destruct an instance of svkImageFourierCenter fitler.
 */
svkImageFourierCenter::~svkImageFourierCenter()
{
}


/*!
 *  This method tells the superclass which input extent is needed.
 *  This gets the whole input (even though it may not be needed).
 */
int svkImageFourierCenter::IterativeRequestUpdateExtent( vtkInformation* input, vtkInformation* output)
{
    int *outExt = output->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT());
    int *wExt = input->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT());
    int inExt[6];
    memcpy(inExt, outExt, 6 * sizeof(int));
    inExt[this->Iteration*2] = wExt[this->Iteration*2];
    inExt[this->Iteration*2 + 1] = wExt[this->Iteration*2 + 1];  
    input->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),inExt,6);

    return 1;
}


/*!
 *  This method is passed input and output regions, and executes the fft
 *   algorithm to fill the output from the input.
 */
void svkImageFourierCenter::ThreadedExecute(vtkImageData *inData, vtkImageData *outData, int outExt[6], int threadId)
{
    double *inPtr0, *inPtr1, *inPtr2;
    double *outPtr0, *outPtr1, *outPtr2;
    vtkIdType inInc0, inInc1, inInc2;
    vtkIdType outInc0, outInc1, outInc2;
    int wholeMin0, wholeMax0; 
    int inIdx0, outIdx0, idx1, idx2;
    int min0, max0, min1, max1, min2, max2;
    int numberOfComponents;
    int inCoords[3];
    unsigned long count = 0;
    unsigned long target;
    double startProgress;

    startProgress = this->GetIteration()/
        static_cast<double>(this->GetNumberOfIterations());
  
    // this filter expects that the input be doubles.
    if (  vtkImageData::GetScalarType( inData->GetInformation() ) != VTK_DOUBLE) {
        vtkErrorMacro(<< "Execute: Input must be be type double.");
        return;
    }
    // this filter expects that the output be doubles.
    if (vtkImageData::GetScalarType( outData->GetInformation() ) != VTK_DOUBLE) {
        vtkErrorMacro(<< "Execute: Output must be be type double.");
        return;
    }
    // this filter expects input to have 1 or two components
    if (outData->GetNumberOfScalarComponents() != 1 && outData->GetNumberOfScalarComponents() != 2) {
        vtkErrorMacro(<< "Execute: Cannot handle more than 2 components");
        return;
    }

    // Get stuff needed to loop through the pixel
    numberOfComponents = outData->GetNumberOfScalarComponents();
    outPtr0 = static_cast<double *>(outData->GetScalarPointerForExtent(outExt));

    int* wholeExtent = this->GetOutputInformation(0)->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT());

    // permute to make the filtered axis come first
    this->PermuteExtent(outExt, min0, max0, min1, max1, min2, max2);
    this->PermuteIncrements(inData->GetIncrements(), inInc0, inInc1, inInc2);
    this->PermuteIncrements(outData->GetIncrements(), outInc0, outInc1, outInc2);
  
    // Determine the mid for the filtered axis
    wholeMin0 = wholeExtent[this->Iteration * 2];
    wholeMax0 = wholeExtent[this->Iteration * 2 + 1];  

    //  Reversing is different for odd size, but the same for even
    //  wholeMin0 + wholwMax0 is the range of indices in this
    //  dimension.  The number of points is 1 more:  e.g. 0,1,2 is 3 pts. 
    int numPts = wholeMin0 + wholeMax0 + 1; 
    float mid0 = static_cast<float>(numPts - 1) / 2.;
    int shiftSize = static_cast<int>( ceil( mid0 ) );
    int oddCorrection = numPts%2; 

    // initialize input coordinates
    inCoords[0] = outExt[0];
    inCoords[1] = outExt[2];
    inCoords[2] = outExt[4];
  
    target = static_cast<unsigned long>((max2-min2+1)*(max0-min0+1) * this->GetNumberOfIterations() / 50.0);
    target++;

    //  loop over the filtered axis first
    //  This looked at from the point of view of the input index and so the
    //  logic is reversed from that used by the spectral FFT shift algorithm. 
    for (outIdx0 = min0; outIdx0 <= max0; ++outIdx0) {

        // get the correct input pointer

        if( this->reverseCenter ) {

            if ( outIdx0 > mid0 ) {
                inIdx0 = outIdx0 - shiftSize - oddCorrection;
            } else {
                inIdx0 = outIdx0 + shiftSize ;
            }

        } else {

            if ( outIdx0 >= mid0 ) {
                inIdx0 = outIdx0 - shiftSize; 
            } else {
                inIdx0 = outIdx0 + shiftSize + oddCorrection;
            }

        }

        inCoords[this->Iteration] = inIdx0;
        inPtr0 = static_cast<double *>(inData->GetScalarPointer(inCoords));
    
        // loop over other axes
        inPtr2 = inPtr0;
        outPtr2 = outPtr0;
        for (idx2 = min2; !this->AbortExecute && idx2 <= max2; ++idx2) {
            if (!threadId) {
                if (!(count%target)) {
                    this->UpdateProgress(count/(50.0*target) + startProgress);
                }
                count++;
            }
            inPtr1 = inPtr2;
            outPtr1 = outPtr2;
            for (idx1 = min1; idx1 <= max1; ++idx1) {
                // handle components (up to 2) explicitly
                *outPtr1 = *inPtr1;
                if (numberOfComponents == 2) {
                    outPtr1[1] = inPtr1[1];
                }

                inPtr1 += inInc1;
                outPtr1 += outInc1;
            }
            inPtr2 += inInc2;
            outPtr2 += outInc2;
        }
        outPtr0 += outInc0;
    }
}


/*!
 *  If set to false the origin index [0,0,0] is moved to the center
 *  of the volume. If set to true then center index is moved to the
 *  origin. You want to reverse the center when preparing to ifft 
 *  if your origin was at the center of the volume.
 *
 */
void svkImageFourierCenter::SetReverseCenter( bool reverseCenter ) 
{
    this->reverseCenter = reverseCenter;
}
