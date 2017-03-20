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
 *      Beck Olson,
 */

#ifndef SVK_MRS_FIRST_POINT_PHASE_COST_FUNCTION_H
#define SVK_MRS_FIRST_POINT_PHASE_COST_FUNCTION_H

#include <svkPhaseCostFunction.h>


using namespace svk;

/*
 *  Cost function for ITK optimizer: 
 */
class svkMRSFirstPointPhaseCostFunction : public svkPhaseCostFunction
{

    public:

        typedef svkMRSFirstPointPhaseCostFunction   Self;
        typedef svkPhaseCostFunction                Superclass;
        typedef itk::SmartPointer<Self>             Pointer;
        typedef itk::SmartPointer<const Self>       ConstPointer;

        itkNewMacro( Self );
        itkTypeMacro( svkMRSFirstPointPhaseCostFunction, svkPhaseCostFunction);
        //itkTypeRevisionMacro( svkMRSFirstPointPhaseCostFunction, svkPhaseCostFunction);


        svkMRSFirstPointPhaseCostFunction() {
        }


        /*!
         *  Cost function based on maximizing the intensity of the first FID point. 
         */
        MeasureType  GetFirstPointPhaseValue0( const ParametersType& parameters) const
        {

            //++POWELL_CALLS_TO_GET_VALUE;
        
            double phi0 = parameters[0];
        
            double intensity = FLT_MIN; 
            float cmplxPt[2];
            double tmp; 

            // apply zero order phase to data:  
            this->copySpectrum->GetTupleValue(0, cmplxPt);
            svk::svkPhaseSpec::ZeroOrderPhase(phi0, cmplxPt);

            //  maximize positive peak height (minimize negative peak ht) 
            tmp = -1*cmplxPt[0];
            //if ( tmp >= intensity) {
                intensity= tmp; 
                //cout << "new phase: " << phi0 * 180. / vtkMath::Pi() << " " << intensity << endl;
            //}

            MeasureType measure = intensity; 
            return measure;
        }


        /*  
         *  returns the cost function for the current param values: 
         *  typedef double MeasureType
         */
        virtual MeasureType  GetValue( const ParametersType & parameters ) const
        { 

            double cost;  

            // make a member variable (copy)
            //cout << "copy spectrum" << endl;
            this->copySpectrum->DeepCopy(this->spectrum); 
            cost = GetFirstPointPhaseValue0( parameters ); 

            MeasureType measure = cost; 
            //cout << "                          cost: " << measure << endl; 
            return measure;
        }


        /*
         *
         */  
        virtual unsigned int GetNumberOfParameters(void) const
        {
            return 1; 
        }


};



#endif// SVK_MRS_FIRST_POINT_PHASE_COST_FUNCTION_H
