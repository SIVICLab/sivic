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


#ifndef SVK_IMAGE_THRESHOLD_H
#define SVK_IMAGE_THRESHOLD_H


#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkInformation.h>

#include <svkImageData.h>
#include <svkMriImageData.h>
#include <svkMrsImageData.h>
#include <svkImageAlgorithm.h>
#include <svkDcmHeader.h>
#include </usr/include/vtk/vtkImageThreshold.h>
#include <svkImageAlgorithmExecuter.h>
#include </usr/include/vtk/vtkCharArray.h>
#include <svkUtils.h>
#include <svkImageAlgorithmWithPortMapper.h>
#include <svkIdfVolumeWriter.h>
#include <svkStatistics.h>


namespace svk {


using namespace std;


/*! 
 *  This class handles thresholding images.
 */
class svkImageThreshold : public svkImageAlgorithmWithPortMapper
{

    public:

        enum {
            INPUT_IMAGE = 0,
            INPUT_ROI,
            EXCLUSIVE_INTEGER_MATCHING,
            THRESHOLD_MAX,
            THRESHOLD_MIN,
            OUTPUT_SERIES_DESCRIPTION,
            MASK_OUTPUT_VALUE,
            THRESHOLD_BY_MODE_IMAGE,
            THRESHOLD_BY_MODE_FACTOR,
            THRESHOLD_BY_MODE_START_BIN,
            THRESHOLD_BY_MODE_SMOOTH_BINS,
            THRESHOLD_BY_MODE_BIN_SIZE,
            THRESHOLD_BY_MODE_NUM_BINS,
            OUTPUT_SCALAR_TYPE,
            VOLUME
        } svkImageThresholdParameters;

        static svkImageThreshold* New();
        vtkTypeMacro( svkImageThreshold, svkImageAlgorithmWithPortMapper);

        void        SetThresholdMax( double max );
        svkDouble*  GetThresholdMax( );

        void        SetThresholdMin( double min );
        svkDouble*  GetThresholdMin( );

        void        SetMaskSeriesDescription( string description );
        svkString*  GetMaskSeriesDescription( );

        void        SetMaskOutputValue( int value );
        svkInt*     GetMaskOutputValue( );
    
        void        SetVolume( int value );
        svkInt*     GetVolume( );

        void        SetOutputScalarType( int outputScalarType );
        svkInt*     GetOutputScalarType( );

    protected:

        svkImageThreshold();
        ~svkImageThreshold();

        virtual int RequestData( 
                        vtkInformation* request, 
                        vtkInformationVector** inputVector, 
                        vtkInformationVector* outputVector );


    private:

};


}   //svk


#endif //SVK_IMAGE_THRESHOLD_H

