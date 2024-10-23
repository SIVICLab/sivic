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
 *      
 *     
 */


#ifndef SVK_MRS_IMAGE_FOURIER_CENTER_H
#define SVK_MRS_IMAGE_FOURIER_CENTER_H

#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>

#include <svkImageInPlaceFilter.h>


namespace svk {


using namespace std;



/*! 
 *  Applies a circular shift to the specified domain (SPATIAL or SPECTRAL) and in the specified
 *  direction (FORWARD, REVERSE).  
 */
class svkMrsImageFourierCenter : public svkImageInPlaceFilter
{

    public:

        static svkMrsImageFourierCenter* New();
        vtkTypeMacro( svkMrsImageFourierCenter, svkImageInPlaceFilter);

        typedef enum {
            SPECTRAL = 0,
            SPATIAL
        } ShiftDomain;


        typedef enum {
            FORWARD = 0,
            REVERSE
        } ShiftDirection;


        void    SetShiftDomain( ShiftDomain domain ); 
        void    SetShiftDirection( ShiftDirection direction ); 


    protected:

        svkMrsImageFourierCenter();
        ~svkMrsImageFourierCenter();

        virtual int     FillInputPortInformation(int port, vtkInformation* info);


        //  Methods:
        virtual int     RequestData(
                            vtkInformation* request, 
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector
                        );


    private: 

        ShiftDomain     shiftDomain; 
        ShiftDirection  shiftDirection;

        void            ApplySpatialShift(); 
        void            ApplySpectralShift(); 



};


}   //svk


#endif //SVK_MRS_IMAGE_FOURIER_CENTER_H

