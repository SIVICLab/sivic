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

#ifndef SVK_IMAGE_FOURIER_CENTER_H
#define SVK_IMAGE_FOURIER_CENTER_H


#include </usr/include/vtk/vtkImageDecomposeFilter.h>


namespace svk {
    

using namespace svk;


/*!
 *  Version of vtkImageFourierCenter with option for reversing the operation.
 *  The reverse opteration isn't symmetric for data with an odd number of 
 *  points.  Operates on vtkImageData.  See svkMrsImageFourierCenter for
 *  version that operates on spectral data. 
 */
class svkImageFourierCenter : public vtkImageDecomposeFilter
{

    public:
        static svkImageFourierCenter *New();
        vtkTypeMacro(svkImageFourierCenter,vtkImageDecomposeFilter);
    
        void SetReverseCenter( bool reverseCenter );

    protected:
        svkImageFourierCenter();
        ~svkImageFourierCenter();

        virtual int IterativeRequestUpdateExtent(vtkInformation* in,
                                                   vtkInformation* out);

        void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,
                                           int outExt[6], int threadId);
    private:
        bool reverseCenter;
};

}   //svk

#endif //SVK_IMAGE_FOURIER_CENTER_H

