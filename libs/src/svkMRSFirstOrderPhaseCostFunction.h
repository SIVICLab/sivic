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

#ifndef SVK_MRS_FIRST_ORDER_PHASE_COST_FUNCTION_H
#define SVK_MRS_FIRST_ORDER_PHASE_COST_FUNCTION_H


#include </usr/include/vtk/vtkMath.h>

#include <svkMRSAutoPhase.h>
#include <svkPhaseSpec.h>

#include <math.h>


using namespace svk;


/*
 *  Cost function for ITK optimizer: 
 */
class svkMRSFirstOrderPhaseCostFunction : public itk::SingleValuedCostFunction 
{

    public:

        typedef svkMRSFirstOrderPhaseCostFunction   Self;
        itkTypeMacro( svkMRSFirstOrderPhaseCostFunction, SingleValuedCostFunction );

        itkNewMacro( Self );


        svkMRSFirstOrderPhaseCostFunction() 
        {
        }


       /*!
         *  Cost function based on maximizing the intensity of the first FID point. 
         */
        MeasureType  GetFirstOrderPhaseValue0( const ParametersType& parameters) const
        {

            //++POWELL_CALLS_TO_GET_VALUE;
        
            double phi0 = parameters[0];
            double phi1 = parameters[1];
        
            double intensity = FLT_MIN; 
            float cmplxPt[2];
            double tmp; 

            // apply first order phase to data:  
            this->copySpectrum->GetTupleValue(0, cmplxPt);
            //svk::svkPhaseSpec::ZeroOrderPhase(phi0, cmplxPt);

            //  maximize positive peak height (minimize negative peak ht) 
            tmp = -1*cmplxPt[0];
                intensity= tmp; 

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
            cost = GetFirstOrderPhaseValue( parameters ); 

            MeasureType measure = cost; 
            //cout << "                          cost: " << measure << endl; 
            return measure;
        }


       /*!
         *  Zero, First, Pivot
         */  
        virtual unsigned int GetNumberOfParameters(void) const
        {
            return 3; 
        }
        

        /*
         *
         */  
        void SetLinearPhaseArrays( vtkImageComplex** linearPhaseArrays)  
        {
            this->linearPhaseArrays = linearPhaseArrays;
        }


        /*
         *
         */  
        void SetNumFirstOrderPhaseValues( int numPhaseValues )  
        {
            this->numFirstOrderPhaseValues = numPhaseValues;
        }



    protected:

            vtkImageComplex**               linearPhaseArrays;
            int                             numFirstOrderPhaseValues; 


};



#endif// SVK_MRS_FIRST_ORDER_PHASE_COST_FUNCTION_H
