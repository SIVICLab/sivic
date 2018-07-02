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


#ifndef SVK_IMAGE_DILATE_ERODE_H
#define SVK_IMAGE_DILATE_ERODE_H


#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkInformation.h>

#include <svkDcmHeader.h>
#include <svkImageAlgorithmExecuter.h>
#include <svkImageAlgorithmWithPortMapper.h>


namespace svk {


using namespace std;


/*! 
 *  This class handles dilating and eroding images.
 */
class svkImageDilateErode : public svkImageAlgorithmWithPortMapper
{

    public:

        enum {
            INPUT_IMAGE = 0,
            DILATE_VALUE,
            ERODE_VALUE,
            KERNEL_SIZE,
            VOLUME, 
            OUTPUT_SERIES_DESCRIPTION,
        } svkImageDilateErodeParameters;

        static svkImageDilateErode* New();
        vtkTypeMacro( svkImageDilateErode, svkImageAlgorithmWithPortMapper);

        void        SetDilateValue( int value );
        svkInt*     GetDilateValue( );

        void        SetErodeValue( int value );
        svkInt*     GetErodeValue( );

        void        SetKernelSize( int size );
        svkInt*     GetKernelSize( );

        void        SetSeriesDescription( string description );
        svkString*  GetSeriesDescription( );

        void        SetVolume( int value );
        svkInt*     GetVolume( );


    protected:

        svkImageDilateErode();
        ~svkImageDilateErode();

        virtual int RequestData( 
                        vtkInformation* request, 
                        vtkInformationVector** inputVector, 
                        vtkInformationVector* outputVector );


    private:

};


}   //svk


#endif //SVK_IMAGE_DILATE_ERODE_H

