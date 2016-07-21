/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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


#ifndef SVK_DYNAMIC_MRI_ALGO_TEMPLATE_H
#define SVK_DYNAMIC_MRI_ALGO_TEMPLATE_H


#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkInformation.h>

#include <svkImageData.h>
#include <svkMriImageData.h>
#include <svkImageAlgorithm.h>
#include <svkDcmHeader.h>

#include <cminpack.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#define real __cminpack_real__



namespace svk {


using namespace std;


/*! 
 *  Demonstration template class to analyze dynamic MRI data. 
 */
class svkDynamicMRIAlgoTemplate: public svkImageAlgorithm
{

    public:

        vtkTypeMacro( svkDynamicMRIAlgoTemplate, svkImageAlgorithm);
        static                  svkDynamicMRIAlgoTemplate* New();


        void                    SetSeriesDescription(vtkstd::string newSeriesDescription);
        void                    SetOutputDataType(svkDcmHeader::DcmPixelDataFormat dataType);
        void                    SetZeroCopy(bool zeroCopy); 


    protected:

        svkDynamicMRIAlgoTemplate();
        ~svkDynamicMRIAlgoTemplate();

        virtual int             RequestInformation(
                                    vtkInformation* request,
                                    vtkInformationVector** inputVector,
                                    vtkInformationVector* outputVector
                                );
        virtual int             RequestData( 
                                    vtkInformation* request, 
                                    vtkInformationVector** inputVector, 
                                    vtkInformationVector* outputVector 
                                );

        void                    ZeroData(); 


        virtual int             FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info );
        virtual int             FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info ); 


        virtual void            UpdateProvenance();


        //  Members:
        vtkstd::string          newSeriesDescription; 


    private:
        void                    GenerateKineticParamMap();
        double                  GetKineticsMapVoxelValue( float* metKinetics0, float* metKinetics1, float* metKinetics3);
        void                    CalculateLactateKinetics( double* fittedModelParams, 
                                                          int numTimePts,
														  float* metKinetics0,
                                                          float* metKinetics1, 
                                                          float* lacKinetics ); 


        float*                  metKinetics0;
        float*                  metKinetics1;
        float*                  metKinetics2;
        int                     currentTimePoint; 

};


}   //svk


#endif //SVK_DYNAMIC_MRI_ALGO_TEMPLATE_H

