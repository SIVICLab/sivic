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


#ifndef SVK_IMAGE_MATHEMATICS_H
#define SVK_IMAGE_MATHEMATICS_H


#include </usr/include/vtk/vtkImageMathematics.h>
#include <svkAlgorithmPortMapper.h>
#include <svkStatistics.h>


namespace svk {


using namespace std;


/*! 
 *  Wrapper class around vtkImageMathematics that operates 
 *  on each vtkPointData array in the input vtkImageData objects.  
 *  This is a convenience for applying an operation to a multi-volume
 *  image object. 
 */

class svkImageMathematics : public vtkImageMathematics
{

    public:
        static svkImageMathematics* New();

        typedef enum {
            INPUT_IMAGE_1 = 0,
            INPUT_IMAGE_2,
            MASK,
            ADD,
            SUBTRACT,
            MULTIPLY,
            MULTIPLY_BY_SCALAR,
            ADD_SCALAR,
            SQUARE_ROOT,
            MULTIPLY_IMAGE_1_BY_IMAGE_2_MEDIAN,
            NUM_BINS_FOR_HISTOGRAM,
            BIN_SIZE_FOR_HISTOGRAM,
            START_BIN_FOR_HISTOGRAM,
            SMOOTH_BINS_FOR_HISTOGRAM,
            MASK_FOR_MEDIAN,
            OUTPUT_SERIES_DESCRIPTION,
            OUTPUT_TYPE
        } svkImageMathematicsParameters;

        typedef enum {
            UNDEFINED = 0,
            UNSIGNED_INT_2 = 1,
            SIGNED_FLOAT_4,
        } svkImageMathematicsOutputType;

        //! Uses port mapper to parse an XML element and convert it into input port data objects.
        void                    SetInputPortsFromXML( );

        //! Get the internal XML interpreter
        svkAlgorithmPortMapper* GetPortMapper();


        vtkTypeMacro( svkImageMathematics, vtkImageMathematics);

        //! Prints all input parameters set.
        void                    PrintSelf( ostream &os, vtkIndent indent );

        //  Explicitly specify float output
        void                    SetOutputType(int outputType);
        svkInt*                 GetOutputType();

    protected:

        svkImageMathematics();
        ~svkImageMathematics();

        virtual int RequestData(vtkInformation *request,
                                vtkInformationVector **inputVector,
                                vtkInformationVector *outputVector);
        virtual int         FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info );
        virtual int         FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info );

        //! The port mapper used to set the input port parameters.
        svkAlgorithmPortMapper* portMapper;

    private:
        void                SetDatatypes();


};


}   //svk


#endif //SVK_IMAGE_MATHEMATICS_H

